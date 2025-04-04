#include "session.h"
#include "windows.h"

PhxSession *session = NULL;

#pragma mark *** Accessors ***

PhxObject *
ui_active_focus_get(void) {
  return session->has_focus;
}

void
ui_active_focus_set(PhxObject *obj) {
  session->has_focus = obj;
}

PhxObject *
ui_active_within_get(void) {
  return session->obj_within;
}

void
ui_active_within_set(PhxObject *obj) {
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

/* iface to hold global values */
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
ui_cursor_set_named(const char *named, xcb_window_t window) {

  if (named == NULL) {
    if (session->cursor_id != 0) {
      xcb_free_cursor(session->connection, session->cursor_id);
      session->cursor_id = 0;
      free((void*)session->cursor_named);
      session->cursor_named = NULL;
      xcb_change_window_attributes(session->connection, window,
                                       XCB_CW_CURSOR, &session->cursor_id);
    }
    return;
  }
    /* case: a system does not have a named cursor, so it uses 0.
      Keep named, but associate with id 0. */
  if (session->cursor_named == NULL)  goto load_cursor;
    /* same name identifiers, use previous found id */
  if (strcmp(named, session->cursor_named) == 0)
    goto change_cursor;   /* window may/may not be the same, so change. */
    /* other than id 0, free cursor */
  if (session->cursor_id != 0)
    xcb_free_cursor(session->connection, session->cursor_id);
    /* known from above tests that cursor_named != NULL */
  free((void*)session->cursor_named);
load_cursor:
  session->cursor_id = xcb_cursor_load_cursor(session->cursor_ctx, named);

    /* gnome wm hack */
  if ( (session->cursor_id == 0)
      && (strcmp(named, "dnd-no-drop") == 0) )
    session->cursor_id
      = xcb_cursor_load_cursor(session->cursor_ctx, "dnd-none");

  session->cursor_named = strdup(named);
change_cursor:
  xcb_change_window_attributes(session->connection, window,
                                     XCB_CW_CURSOR, &session->cursor_id);
}

__inline const char *
ui_cursor_get_named(void) {
  return session->cursor_named;
}

#pragma mark *** Session ***

PhxSession *
_session_create(xcb_connection_t *connection) {

  xcb_screen_t *screen;
  size_t aSz;

  if (session != NULL)
    return session;

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

    /* Global cursor, get the first screen */
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  xcb_cursor_context_new(connection, screen, &session->cursor_ctx);

    /* list of windows/iface */
  aSz = OBJS_ALLOC * sizeof(PhxInterface*);
  session->iface = malloc(aSz);
  if (session->iface == NULL) {
    free(session);
    return NULL;
  }
  memset(session->iface, 0, aSz);

    /* Global atoms for Clipboard and DND use. */
  ui_atoms_initialize(connection);

  return session;
}

xcb_window_t
_window_for(PhxObject *obj) {

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
_interface_for(xcb_window_t window) {

  uint16_t sdx;

  if (window == 0) {
    DEBUG_ASSERT(true, "Invalid window... _interface_for()");
    return NULL;
  }
  if (session == NULL) {
    DEBUG_ASSERT(true, "Session not created.");
    return NULL;
  }
  if (session->ncount == 0) {
    DEBUG_ASSERT(true, "Session contains no windows... _interface_for()");
    return NULL;
  }

  sdx = session->ncount;
  do
    if (session->iface[(--sdx)]->window == window)
      return session->iface[sdx];
  while (sdx != 0);

  DEBUG_ASSERT(true, "Session does not contain window... _interface_for()");
  return NULL;
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
#if DND_EXTERNAL_ON
  if (session->xdndserver != NULL)
    free(session->xdndserver);
#endif
  if (session->xclipboard != NULL)
    free(session->xclipboard);
  free(session->iface);
  free(session);
}
