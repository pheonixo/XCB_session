#include "session.h"
#include "windows.h"
#include "objects.h"
#include <unistd.h>

PhxSession *session = NULL;
FILE *_debug_wh = NULL;

#pragma mark *** Accessors ***

PhxObject *
ui_active_focus_get(void) {
  return session->has_focus;
}

/* When changing focus between objects, send in/out for them to respond. */
void
ui_active_focus_set(PhxObject *obj) {

  xcb_focus_in_event_t notify = { 0 };
  PhxInterface *iface;
  PhxObject *focused;

  notify.detail = XCB_NOTIFY_DETAIL_NONE;
#if DND_EXTERNAL_ON
  if ( (ui_active_drag_get() != NULL)
      || (xdndActivated_get(session->xdndserver)) )
#else
  if (ui_active_drag_get() != NULL)
#endif
    notify.mode = XCB_NOTIFY_MODE_WHILE_GRABBED;
  if ((focused = session->has_focus) != NULL) {
    DEBUG_ASSERT((focused->_event_cb == NULL),
                 "SEGFAULT: undefined _event_cb... ui_active_focus_set()");
    notify.response_type = XCB_FOCUS_OUT;
    notify.event = ui_window_for(focused);
      /* Window may have been destroyed. */
    if ((iface = ui_interface_for(notify.event)) != NULL) {
        /* Do not remove focus from iface, unless lost to another iface,
          or NULL. Attachment(s), such as headerbar, can be sensitive
          to 'window' focus. Objects within iface do lose focus. */
      if ( !IS_WINDOW_TYPE(focused)
           && (focused->type != PHX_HEADERBAR) )
        focused->_event_cb(iface, (xcb_generic_event_t*)&notify, focused);
      if (obj == NULL)
        iface->_event_cb(iface, (xcb_generic_event_t*)&notify, focused);
    }
  }
  if (obj != NULL) {
    notify.response_type = XCB_FOCUS_IN;
    notify.event = ui_window_for(obj);
    iface = ui_interface_for(notify.event);
    if (obj->_event_cb == NULL) {
      DEBUG_ASSERT(true, "error: undefined _event_cb... ui_active_focus_set()");
      session->has_focus = (PhxObject*)iface;
      return;
    }
    obj->_event_cb(iface, (xcb_generic_event_t*)&notify, obj);
  }
  session->has_focus = obj;
}

PhxObject *
ui_active_within_get(void) {
  return session->obj_within;
}

/* Used to know active object.
  Similar to a focus event, but mouse related. Detail will be
  XCB_NOTIFY_DETAIL_NONE. This signals object only being informed we've
  entered or left the object. Any detailed information of entry should be
  gleaned from mouse motion event handling. Allows adjustments such as
  highlite, text prep/reset, etc if object needs it.
  Mode will indicate if in drag. */
/* Add state to allow notify with mod keys. */
void
ui_active_within_set(PhxObject *obj, uint16_t state) {

  xcb_enter_notify_event_t notify = { 0 };
  PhxInterface *iface;
  PhxObject *within;

  notify.detail = XCB_NOTIFY_DETAIL_NONE;
  notify.state = state;
#if DND_EXTERNAL_ON
  if ( (ui_active_drag_get() != NULL)
      || (xdndActivated_get(session->xdndserver)) )
#else
  if (ui_active_drag_get() != NULL)
#endif
    notify.mode = XCB_NOTIFY_MODE_WHILE_GRABBED;
  if ( ((within = session->obj_within) != NULL)
      && (within != obj) ) {
      /* We can be in an object that doesn't deal with mouse events.
        More than likely is handled farther up the chain. */
    PhxInterface *imount = (PhxInterface*)within;
    if (!IS_WINDOW_TYPE(within)) {
      while (imount->_event_cb == NULL) {
        imount = imount->i_mount;
        if (IS_WINDOW_TYPE(imount))  break;
      }
    }
    if (imount->_event_cb != NULL) {
      notify.response_type = XCB_LEAVE_NOTIFY;
      notify.event = ui_window_for((PhxObject*)imount);
      iface = ui_interface_for(notify.event);
      imount->_event_cb(iface,
                        (xcb_generic_event_t*)&notify,
                        (PhxObject*)imount);
    }
  }
  if (obj != NULL) {
    bool handled = false;
    PhxInterface *imount = (PhxInterface*)obj;
    if (!IS_WINDOW_TYPE(obj)) {
      while (imount->_event_cb == NULL) {
        imount = imount->i_mount;
        if (IS_WINDOW_TYPE(imount))  break;
      }
    }
    if (imount->_event_cb != NULL) {
      notify.response_type = XCB_ENTER_NOTIFY;
      notify.event = ui_window_for((PhxObject*)imount);
      iface = ui_interface_for(notify.event);
      handled
        = imount->_event_cb(iface, (xcb_generic_event_t*)&notify, obj);
    }
    if (!handled)
      ui_cursor_set_named(NULL, ui_window_for((PhxObject*)imount));
    obj = (PhxObject*)imount;
  }
  session->obj_within = obj;
}

PhxObject *
ui_active_drag_get(void) {
  return session->has_drag;
}

void
ui_active_drag_set(PhxObject *obj) {
  session->has_drag = obj;
}

xcb_window_t
ui_window_for(PhxObject *obj) {

  PhxInterface *iface;
  if ( (session == NULL) || (obj == NULL) ){
    DEBUG_ASSERT(true, "No object, No window.");
    return 0;
  }
    /* Since using basics, and objects unknown at this level of code. */
  iface = (PhxInterface*)obj;
  if (IS_IFACE_TYPE(iface))
    return iface->window;
  return iface->i_mount->window;
}

PhxInterface *
ui_interface_for(xcb_window_t window) {

  uint16_t sdx;

  if (window == 0) {
    DEBUG_ASSERT(true, "Invalid window... ui_interface_for()");
    return NULL;
  }
  if (session == NULL) {
    DEBUG_ASSERT(true, "Session not created.");
    return NULL;
  }
  if (session->ncount == 0) {
    DEBUG_ASSERT(true, "Session contains no windows... ui_interface_for()");
    return NULL;
  }

  sdx = session->ncount;
  do
    if (session->iface[(--sdx)]->window == window)
      return session->iface[sdx];
  while (sdx != 0);

  DEBUG_ASSERT(true, "Session does not contain window... ui_interface_for()");
  return NULL;
}

#pragma mark *** Realloc ***

bool
_obj_alloc_test(void **abstract, size_t abstract_size, uint16_t ncount) {

  if (ncount == UINT16_MAX) {
rej:
    DEBUG_ASSERT(true, "realloc() rejected!");
    return true;
  }
    /* Note: does check 1 early, so to leave a NULL terminator */
  if (((ncount + 1) & (OBJS_ALLOC - 1)) == (OBJS_ALLOC - 1)) {
    uint32_t n = (unsigned)(ncount + 1);
    if ((n + (uint32_t)OBJS_ALLOC) <= (uint32_t)UINT16_MAX) {
      size_t oldsz
          = (size_t)((ncount & ~(OBJS_ALLOC - 1)) + OBJS_ALLOC)
                                 * abstract_size;
      size_t addsz = OBJS_ALLOC * abstract_size;
      char *nPtr, *cptr;
      nPtr = realloc(*abstract, (oldsz + addsz));
      if (nPtr == NULL)  goto rej;
      cptr = nPtr + oldsz;
      memset(cptr, 0, addsz);
      *abstract = nPtr;
    }
  }
  return false;
}

#pragma mark *** Keyboard ***

#include "XKeyNames.h"

char *
_xkey_names(xcb_keysym_t sym) {

  uint32_t has_data;

  if (sym < 0x0020U)  return "Unmapped";
  if (sym <= 0x007eU) {
    sym -= 0x0020U;
    return knames[sym].name;
  }

  if (sym < 0xfd01U)  return "Unmapped";
  has_data = 0x007fU - 0x0020U;
  if (sym <= 0xfd1eU) {
    sym -= 0xfd01U - has_data;
    return knames[sym].name;
  }

  if (sym < 0xfe01U)  return "Unmapped";
  has_data += 0xfd1fU - 0xfd01U;
  if (sym <= 0xfe34U) {
    sym -= 0xfe01U - has_data;
    return knames[sym].name;
  }
  if (sym < 0xfe50U)  return "Unmapped";
  has_data += 0xfe35U - 0xfe01U;
  if (sym <= 0xfea5U) {
    sym -= 0xfe50U - has_data;
    return knames[sym].name;
  }

  if (sym < 0xfed0U)  return "Unmapped";
  sym -= 0xfed0U - (has_data + (0xfea6U - 0xfe50U));
  return knames[sym].name;
}

xcb_keysym_t
_xcb_keysym_for(xcb_keycode_t keycode, uint16_t modifiers) {

  xcb_keysym_t sym;
  xcb_keysym_t *keysyms = session->keysyms;
  uint8_t    sym_stride = session->sym_stride;
  uint8_t    sym_min    = session->sym_min;

  uint8_t column = 0;
  if (modifiers & 1)  column++;

  keysyms = &keysyms[((keycode - sym_min) * sym_stride)];
  sym = keysyms[column];
    /* keypad syms (NumLock) */
  if ((sym >= 0xff80) && (sym <= 0xffbd))
    if (modifiers & 0x10)  sym = keysyms[(column + 1)];
    /* alphabetic syms (CapsLock) */
  if ((modifiers & 2) != 0) {
    if ( ((sym >= 0x41) && (sym <= 0x5A))
        || ((sym >= 0x61) && (sym <= 0x7A)) )
      sym = keysyms[(column + 1)];
  }
    /* modifier syms (missing codes) */
  if ((sym == 0) && (column != 0))
    sym = keysyms[(column - 1)];

  return sym;
}

/* session to hold global values */
void
_xcb_keysym_alloc(xcb_connection_t *connection) {

  const struct xcb_setup_t *setup = xcb_get_setup(connection);

  xcb_keycode_t min_keycode = setup->min_keycode;
  xcb_keycode_t max_keycode = setup->max_keycode;

  xcb_get_keyboard_mapping_reply_t *sym_table;
  xcb_get_keyboard_mapping_cookie_t cookie;
  cookie = xcb_get_keyboard_mapping(connection,
                                    min_keycode,
                                    max_keycode - min_keycode + 1);
    /* AFAICT owned by server */
  sym_table
    = xcb_get_keyboard_mapping_reply(connection, cookie, NULL);
    /* owned by server */
  session->keysyms    = xcb_get_keyboard_mapping_keysyms(sym_table);
  session->sym_stride = sym_table->keysyms_per_keycode;
  session->sym_min    = min_keycode;
}

#pragma mark *** Cursor ***

void
ui_cursor_initialize(xcb_connection_t *connection) {

  xcb_screen_t *screen;
  const char *named;

    /* Global cursor, get the first screen */
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    /* Set a default or 'NULL' cursor for our windows. */
  xcb_cursor_context_new(connection, screen, &session->cursor_ctx);
  named = (CURSOR_DEFAULT != NULL) ? CURSOR_DEFAULT : "left_ptr";
  session->cursor_default
    = xcb_cursor_load_cursor(session->cursor_ctx, named);
}

void
ui_cursor_set_named(const char *named, xcb_window_t window) {

    /* Request for default or WM control */
  if (named == NULL) {
    if (session->cursor_id != 0) {
      xcb_free_cursor(session->connection, session->cursor_id);
      session->cursor_id = 0;
      free((void*)session->cursor_named);
      session->cursor_named = NULL;
default_cursor:
      xcb_change_window_attributes(session->connection, window,
                                   XCB_CW_CURSOR, &session->cursor_default);
    }
    return;
  }
    /* Has 'named', but is it an alias, currently loaded, or not found. */
    /* Two main alias, "text" and gnome hack, "dnd-no-drop". */
    /* use default name for "text" */
  if (strcmp(named, "text") == 0)         named = "xterm";
    /* See if currently loaded. Load cursor attribute anyway,
      since we don't cache xcb_window_t */
  if (session->cursor_named != NULL) {
    if (strcmp(named, session->cursor_named) == 0)  goto change_cursor;
    free((void*)session->cursor_named);
  }
    /* Find requested 'named'. 0 id can be returned. If so, use 'named'
      and like above, change since xcb_window_t not cached. */
  session->cursor_named = strdup(named);
  if (session->cursor_id != 0)
    xcb_free_cursor(session->connection, session->cursor_id);
  session->cursor_id = xcb_cursor_load_cursor(session->cursor_ctx, named);
    /* Check if found. */
change_cursor:
  if (session->cursor_id == 0) {
    if (strcmp(named, "dnd-no-drop") == 0) {
      session->cursor_id
        = xcb_cursor_load_cursor(session->cursor_ctx, "dnd-none");
      if (session->cursor_id != 0)  goto change_attrib;
    }
    goto default_cursor;
  }
change_attrib:
  xcb_change_window_attributes(session->connection, window,
                                     XCB_CW_CURSOR, &session->cursor_id);
}

__inline const char *
ui_cursor_get_named(void) {
  return session->cursor_named;
}

#pragma mark *** Session ***

/* XXX setup for Linux only, buffers, name poorly handled.
  Quickly thrown together to have multiple apps at once. */
static void
_debug_file_handle_create(void) {

  pid_t  pid;
  char cline[128];
  FILE *rh;

  pid = getpid();
  sprintf(cline, "/proc/%d/cmdline", pid);
  if ((rh = fopen(cline, "r")) != NULL) { 
    char file[32];
    char *delimiter;

    fgets(cline, 128, rh);
    fclose(rh);
    delimiter = strrchr(cline, '/');
    sprintf(file, ".%s.debug", delimiter);
    _debug_wh = fopen(file, "w");
  }
  if (_debug_wh == NULL)  _debug_wh = stderr;

}

PhxSession *
_session_create(xcb_connection_t *connection) {

  size_t aSz;

  if (session != NULL)
    return session;

    /* Initialize a FILE* to write messages to. */
    /* Depending on 'DEBUG' setting, file/stderr will
      get run loop errors, additional developer errors, or
      both plus debug event info. */
  _debug_file_handle_create();

  session = (PhxSession*)malloc(sizeof(PhxSession));
  if (session == NULL)  return NULL;
  memset(session, 0, sizeof(PhxSession));

  session->connection = connection;

  _xcb_keysym_alloc(connection);
  if (session->keysyms == NULL) {
    free(session);
    DEBUG_ASSERT(true, "Could not connect to keyboard.");
    return NULL;
  }

    /* list of windows/iface */
  aSz = OBJS_ALLOC * sizeof(PhxInterface*);
  session->iface = malloc(aSz);
  if (session->iface == NULL) {
    free(session);
    return NULL;
  }
  memset(session->iface, 0, aSz);

  session->stack_order = malloc(aSz);
  if (session->stack_order == NULL) {
    free(session->iface);
    free(session);
    return NULL;
  }
  memset(session->stack_order, 0, aSz);

    /* Global atoms for WM existance, Clipboard, DND use, et al. */
  ui_atoms_initialize(connection);

    /* Set up for cursor, and window XCB_CW_CURSOR default. */
  ui_cursor_initialize(connection);

  return session;
}

static void
_stack_remove(PhxInterface *iface) {

  uint16_t idx;

  for (idx = 0; idx < session->ncount; idx++)
    if (iface == session->stack_order[idx])  break;

    /* Debugging only. */
  if (idx == session->ncount) {
    uint16_t cdx;
    for (cdx = 0; cdx < session->ncount; cdx++)
      if (!!((session->stack_order[cdx])->state & SBIT_MAPPED))
        DEBUG_ASSERT(true, "error: no interface found.");
  }

    /* idx + 1 will be NULL when idx == (session->ncount - 1) */
  for (; idx < session->ncount; idx++)
    session->stack_order[idx] = session->stack_order[(idx + 1)];
}

uint16_t
_interface_remove_for(xcb_window_t window) {

  uint16_t sdx;

  if (window == 0) {
    DEBUG_ASSERT(true, "Invalid window... _interface_remove_for()");
    return 0;
  }
  if (session == NULL) {
    DEBUG_ASSERT(true, "Session doesn't exist.");
    return 0;
  }
  if (session->ncount == 0) {
    DEBUG_ASSERT(true,
          "Session contains no windows... _interface_remove_for()");
    return 0;
  }

  sdx = session->ncount;
  do {
    PhxInterface *iface = session->iface[(--sdx)];
    if (iface->window == window) {
      _stack_remove(iface);
      if (iface->_raze_cb != NULL)  iface->_raze_cb(iface);
      if ((session->ncount - 1) != sdx)
        memmove(&session->iface[sdx], &session->iface[(sdx + 1)],
                   (session->ncount - sdx) * sizeof(PhxInterface*));
      session->ncount -= 1;
      session->iface[session->ncount] = NULL;
      break;
    }
  } while (sdx != 0);

  return session->ncount;
}

void
ui_session_shutdown(void) {
  xcb_disconnect(session->connection);
  if (_debug_wh != stderr)
    fclose(_debug_wh);
  if (_NET_WM_STRING != NULL)
    free(_NET_WM_STRING);
#if DND_EXTERNAL_ON
  if (session->xdndserver != NULL)
    free(session->xdndserver);
#endif
  if (session->xclipboard != NULL)
    free(session->xclipboard);
  free(session->stack_order);
  free(session->iface);
  free(session);
}
