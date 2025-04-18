#include "windows.h"
#include "nexus.h" /* For default '_raze_cb' */

#pragma mark *** Utilities ***

/* For reference:
  enum WMSizeHintsFlag {
    WM_SIZE_HINT_US_POSITION   = 1U << 0,
    WM_SIZE_HINT_US_SIZE       = 1U << 1,
    WM_SIZE_HINT_P_POSITION    = 1U << 2,
    WM_SIZE_HINT_P_SIZE        = 1U << 3,
    WM_SIZE_HINT_P_MIN_SIZE    = 1U << 4,
    WM_SIZE_HINT_P_MAX_SIZE    = 1U << 5,
    WM_SIZE_HINT_P_RESIZE_INC  = 1U << 6,
    WM_SIZE_HINT_P_ASPECT      = 1U << 7,
    WM_SIZE_HINT_BASE_SIZE     = 1U << 8,
    WM_SIZE_HINT_P_WIN_GRAVITY = 1U << 9
  };
*/

/* Appears only those with flags set are honored */
static void
_xcb_size_hints_set(xcb_window_t window,
                    uint16_t x, uint16_t y, uint8_t shift) {

  struct WMSizeHints hints;
  memset(&hints, 0, sizeof(struct WMSizeHints));
  hints.flags = 1U << shift;
  hints.min_width = x;
  hints.min_height = y;
  xcb_change_property(session->connection, XCB_PROP_MODE_REPLACE, window,
                      XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS,
                      32, sizeof(struct WMSizeHints) >> 2, &hints);
}

/* returns true if valid */
static bool
_xcb_size_hints_get(xcb_window_t window,
                    uint16_t *x, uint16_t *y, uint8_t shift) {

  struct WMSizeHints hints;
  xcb_get_property_cookie_t c0;
  xcb_get_property_reply_t *r0;
  bool valid = false;

  memset(&hints, 0, sizeof(struct WMSizeHints));
  c0 = xcb_get_property(session->connection, 0, window, (1U << shift),
                 XCB_ATOM_WM_SIZE_HINTS, 0, sizeof(struct WMSizeHints));
  r0 = xcb_get_property_reply(session->connection, c0, NULL);

  if (r0 != NULL) {
    if ( (r0->type == XCB_ATOM_WM_SIZE_HINTS)
        && (r0->format == 32)
        && (r0->length != 0) ) {
      if (shift == 4) {
        *x = hints.min_width;
        *y = hints.min_height;
        valid = true;
      } else if (shift == 5) {
        *x = hints.max_width;
        *y = hints.max_height;
        valid = true;
      }
    }
    free(r0);
  }
  return valid;
}

void
ui_window_minimum_set(xcb_window_t window, uint16_t x, uint16_t y) {
  _xcb_size_hints_set(window, x, y, 4);
}

bool
ui_window_minimum_get(xcb_window_t window, uint16_t *x, uint16_t *y) {
  return _xcb_size_hints_get(window, x, y, 4);
}

void
ui_window_maximum_set(xcb_window_t window, uint16_t x, uint16_t y) {
  _xcb_size_hints_set(window, x, y, 5);
}

bool
ui_window_maximum_get(xcb_window_t window, uint16_t *x, uint16_t *y) {
  return _xcb_size_hints_get(window, x, y, 5);
}

bool
ui_window_is_transient(xcb_window_t window) {

  xcb_get_property_cookie_t cookie
    = xcb_get_property(session->connection, 0, window,
                       XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 0, 1);
  xcb_get_property_reply_t *reply
    = xcb_get_property_reply(session->connection, cookie, NULL);
  bool transient = false;
  if (reply != NULL) {
    if ( (reply->type == XCB_ATOM_WINDOW)
        && (reply->format == 32)
        && (reply->length != 0) ) {
      transient = true;
    }
    free(reply);
  }
  return transient;
}

/* Use of int to match enum size. */
void
ui_attributes_set(PhxObject *obj,
                  char *font_name,
                  int font_slant,
                  int font_weight,
                  int line_height) {

  cairo_t *cro;
  cairo_font_extents_t font_extents;
  double pixel_line_height, font_size;
    /* Cast as, objects unknown at window level. */
  PhxAttr *attrib = ((PhxInterface*)obj)->attrib;

  if (strcmp(font_name, attrib->font_name) != 0) {
    free(attrib->font_name);
    attrib->font_name = strdup(font_name);
  }
  attrib->font_var = (font_slant << 8) | font_weight;

  cro = cairo_create(((PhxInterface*)obj)->i_mount->surface);
  cairo_select_font_face(cro, font_name, font_slant, font_weight);

  pixel_line_height = line_height;
  font_size = line_height;
  cairo_set_font_size(cro, pixel_line_height);
  cairo_font_extents(cro, &font_extents);
  while (pixel_line_height
          < (int)(font_extents.ascent + font_extents.descent)) {
    font_size -= 0.5;
    cairo_set_font_size(cro, font_size);
    cairo_font_extents(cro, &font_extents);
  }
  attrib->font_size   = font_size;
  attrib->font_origin = font_extents.ascent;
  attrib->font_em     = line_height;
  cairo_destroy(cro);
}

void
ui_attributes_font_em_set(PhxObject *obj, int16_t font_em) {

  PhxAttr *attrib = ((PhxInterface*)obj)->attrib;
  ui_attributes_set(obj, attrib->font_name,
                         attrib->font_var & 0x0FF,
                        (attrib->font_var >> 8) & 0x0FF,
                         font_em);
}

#pragma mark *** Creation ***

uint16_t
_default_interface_remove_nexus(PhxInterface *iface, PhxNexus *nexus) {

  uint16_t ndx;
  if ((ndx = iface->ncount) == 0) {
    DEBUG_ASSERT(true, "error: no nexus in interface.");
    return (uint16_t)~0;
  }
  do {
    PhxNexus *inspect = iface->nexus[(--ndx)];
    if (inspect == nexus) {
      if (nexus->_raze_cb != NULL)  nexus->_raze_cb((void*)nexus);
      else                      _default_nexus_raze((void*)nexus);
      if ((iface->ncount - 1) != ndx)
        memmove(&iface->nexus[ndx], &iface->nexus[(ndx + 1)],
                   (iface->ncount - ndx) * sizeof(PhxNexus*));
      iface->ncount -= 1;
      iface->nexus[iface->ncount] = NULL;
      return iface->ncount;
    }
  } while (ndx != 0);
  DEBUG_ASSERT(true, "error: nexus not found for removal.");
  return (uint16_t)~0;
}

void
_default_interface_raze(void *obj) {

  uint16_t ndx;
  PhxInterface *iface = (PhxInterface*)obj;

  if (!IS_WINDOW_TYPE(iface)) {
    DEBUG_ASSERT(true, "Destruction of non-interface denied.");
    return;
  }

  if ((ndx = iface->ncount) != 0) {
    do {
      PhxNexus *nexus = iface->nexus[(--ndx)];
        /* _raze_cb intended for user additions */
      if (nexus->_raze_cb != NULL)  nexus->_raze_cb((void*)nexus);
      else                      _default_nexus_raze((void*)nexus);
        /* update iface */
      iface->nexus[ndx] = NULL;
      iface->ncount -= 1;
    } while (ndx != 0);
  }
  if (iface->nexus != NULL) {
    free(iface->nexus);
    iface->nexus = NULL;
  }

  free(iface->attrib->font_name);
  free(iface->attrib);
  iface->attrib = NULL;

  if (iface->surface != NULL) {
    cairo_surface_destroy(iface->surface);
    iface->surface = NULL;
  }

  if (iface->vid_buffer != NULL) {
    cairo_surface_destroy(iface->vid_buffer);
    iface->vid_buffer = NULL;
  }

  free(iface);
}

    /* Attribute defaults */
static PhxAttr *
_interface_attrib_default(PhxInterface *iface) {

  cairo_t *cr;
  cairo_font_extents_t font_extents;
  int pixel_line_height;
  double font_size;

  PhxAttr *attrib = (PhxAttr*)malloc(sizeof(PhxAttr));
  if (attrib == NULL)  return NULL;
  memset(attrib, 0, sizeof(PhxAttr));

  cr = cairo_create(iface->surface);
  attrib->font_name = strdup(FONT_NAME);
  attrib->font_var = FONT_VAR;
  attrib->font_em = FONT_EM;
  cairo_select_font_face(cr, attrib->font_name,
                             ((attrib->font_var >> 8) & 0x0FF),
                             (attrib->font_var & 0x0FF));
  attrib->bg_fill.r = (double)0x0F0/0x0FF;
  attrib->bg_fill.g = (double)0x0F0/0x0FF;
  attrib->bg_fill.b = (double)0x0F0/0x0FF;
  attrib->bg_fill.a = 1.0;
  attrib->fg_fill.a = 1.0;
  attrib->fg_ink.a  = 1.0;
  attrib->stroke    = 0.5;

  pixel_line_height = attrib->font_em;
  font_size = (double)attrib->font_em + 0.5;
  do {
    font_size -= 0.5;
    cairo_set_font_size(cr, font_size);
    cairo_font_extents(cr, &font_extents);
  } while (pixel_line_height < (int)(font_extents.ascent
                                     + font_extents.descent));
  attrib->font_size   = font_size;
  attrib->font_origin = font_extents.ascent;
  cairo_destroy(cr);
  return attrib;
}

static xcb_visualtype_t *
_find_visual(xcb_connection_t *c) {

  xcb_screen_iterator_t screen_iter
    = xcb_setup_roots_iterator(xcb_get_setup(c));

  xcb_screen_t *screen = screen_iter.data;
  xcb_visualid_t visual = screen->root_visual;

  for (; screen_iter.rem; xcb_screen_next(&screen_iter)) {
    xcb_depth_iterator_t depth_iter;
    depth_iter = xcb_screen_allowed_depths_iterator(screen_iter.data);
    for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
      xcb_visualtype_iterator_t visual_iter;
      visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
      for (; visual_iter.rem; xcb_visualtype_next(&visual_iter))
        if (visual == visual_iter.data->visual_id)
          return visual_iter.data;
    }
  }
  return NULL;
}

static PhxInterface *
_interface_create(xcb_connection_t *connection,
                  PhxRectangle configure, void *generic_window) {

  cairo_status_t err;
  xcb_visualtype_t *visual;
  xcb_window_t window = *(xcb_window_t*)generic_window;

  PhxInterface *iface = malloc(sizeof(PhxInterface));
  if (iface == NULL) {
    DEBUG_ASSERT(true, "Out of memory... _interface_create()");
    return NULL;
  }
  memset(iface, 0, sizeof(PhxInterface));
    /* Display port list, not combined alloc, this can realloc */
  iface->nexus = malloc((OBJS_ALLOC * sizeof(PhxNexus*)));
  if (iface->nexus == NULL) {
    DEBUG_ASSERT(true, "Out of memory... _interface_create()");
    free(iface);
    return NULL;
  }
  memset(iface->nexus, 0, (OBJS_ALLOC * sizeof(PhxNexus*)));

  iface->i_mount = iface;
  iface->type = PHX_IFACE;
    /* Assign window values */
  iface->mete_box = configure;
  iface->draw_box.x = 0;
  iface->draw_box.y = 0;
  iface->draw_box.w = configure.w;
  iface->draw_box.h = configure.h;
  iface->min_max.x = -INT16_MAX;
  iface->min_max.y = -INT16_MAX;
  iface->min_max.w = INT16_MAX;
  iface->min_max.h = INT16_MAX;

  iface->_raze_cb = _default_interface_raze;

    /* create window's drawing surface */
  if ((visual = _find_visual(connection)) == NULL) {
    free(iface);
    DEBUG_ASSERT(true, "Some weird internal error...?!");
    return NULL;
  }
  iface->window = window;
  iface->vid_buffer = cairo_xcb_surface_create(connection,
                                               window, visual,
                                               iface->draw_box.w,
                                               iface->draw_box.h);
  err = cairo_surface_status(iface->vid_buffer);
  if (err != CAIRO_STATUS_SUCCESS) {
    DEBUG_ASSERT(true, "Some weird internal error...?!");
    return NULL;
  }
  iface->surface = cairo_surface_create_similar(iface->vid_buffer,
                                      CAIRO_CONTENT_COLOR_ALPHA,
                                              iface->draw_box.w,
                                              iface->draw_box.h);
  err = cairo_surface_status(iface->surface);
  if (err != CAIRO_STATUS_SUCCESS) {
    cairo_surface_finish(iface->vid_buffer);
    free(iface);
    DEBUG_ASSERT(true, "Some weird internal error...?!");
    return NULL;
  }

    /* now have surface, fill in default text attributes */
  iface->attrib = _interface_attrib_default(iface);
  if (iface->attrib == NULL) {
    DEBUG_ASSERT(true, "Out of memory... _interface_create()");
    cairo_surface_finish(iface->surface);
    cairo_surface_finish(iface->vid_buffer);
    free(iface);
    return NULL;
  }
  return iface;
}

void
ui_window_undecorate_set(xcb_window_t window) {

  struct MWMHints {
    uint32_t   flags, functions, decorations, input_mode, status;
  } hints = { 2, 0, 0, 0, 0 };

  xcb_intern_atom_cookie_t c0;
  xcb_intern_atom_reply_t *r0;
  c0 = xcb_intern_atom(session->connection, 0, 15, "_MOTIF_WM_HINTS");
  r0 = xcb_intern_atom_reply(session->connection, c0, NULL);
  xcb_change_property(session->connection, XCB_PROP_MODE_REPLACE, window,
                      r0->atom, r0->atom, 32, sizeof(hints) >> 2,
                      &hints);
  free(r0);
}

static void
_window_event_delete(xcb_connection_t *connection, xcb_window_t window) {

    /* code to allow delete window instead of quit */
  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
                      WM_PROTOCOLS, XCB_ATOM_ATOM, 32, 1,
                      &WM_DELETE_WINDOW);
}

/*
static void
_window_event_focus(xcb_connection_t *connection, xcb_window_t window) {

  xcb_change_property(connection, XCB_PROP_MODE_APPEND, window,
                      WM_PROTOCOLS, XCB_ATOM_ATOM, 32, 1,
                      &WM_TAKE_FOCUS);
}
*/

static xcb_window_t
_window_create(PhxRectangle configure) {

  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  uint32_t mask;
  uint32_t values[2];

  if (session == NULL) {
    connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(connection)) {
      DEBUG_ASSERT(true, "Could not connect to X11 server");
      return 0;
    }
    _session_create(connection);
  } else {
    connection = session->connection;
  }

    /* Create the window */
  window = xcb_generate_id(connection);
  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

  mask      = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
              XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
              XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW   |
              XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE    |
              XCB_EVENT_MASK_FOCUS_CHANGE   | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
              XCB_EVENT_MASK_PROPERTY_CHANGE |
              XCB_EVENT_MASK_VISIBILITY_CHANGE;

    /* ignores x, y, border_width (virus) */
  xcb_create_window(connection,
                    screen->root_depth,            /* depth          */
                    window,                        /* generated id   */
                    screen->root,                  /* parent window  */
                    configure.x,                   /* x, y           */
                    configure.y,
                    configure.w,                   /* width, height  */
                    configure.h,
                    0,                             /* border_width   */
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class          */
                    screen->root_visual,           /* visual         */
                    mask, values);                 /* masks */


  _window_event_delete(connection, window);
/*  _window_event_focus(connection, window);*/

#if DND_EXTERNAL_ON
  xdnd_window_awareness(connection, window);
#endif

  return window;
}

xcb_window_t
ui_window_create(PhxRectangle configure) {

  xcb_connection_t *connection;
  PhxInterface *iface;
  xcb_window_t window = _window_create(configure);
  if (window == 0)  return 0;

    /* Global connector, values and router */
  connection = session->connection;
  iface = _interface_create(connection, configure, &window);
  if (iface == NULL)  return 0;
  session->iface[session->ncount] = iface;
  session->ncount += 1;

  return window;
}

/* A transient window is a window(iface) with its i_mount of
  the parent it's attached to. */
/* Has window manager decorations. */
xcb_window_t
ui_dialog_create(PhxRectangle configure, xcb_window_t transient_for_window) {

  xcb_connection_t *connection;
  PhxInterface *iface;
  xcb_window_t window = _window_create(configure);
  if (window == 0)  return 0;

  connection = session->connection;
  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
                             XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 32,
                             1, &transient_for_window);

  iface = _interface_create(connection, configure, &window);
  if (iface == NULL)  return 0;
  iface->type = PHX_ITDLG;
  session->iface[session->ncount] = iface;
  session->ncount += 1;
  iface->i_mount = _interface_for(transient_for_window);
  return window;
}

/* A transient window is a window(iface) */
/* 'configure' in global coordinates */
/* No window manager decorations. To be configured with a window (moved).
  Presents on button pressed objects or hover. These include
  menu label, toolbar icon, combo button, right-click context, etc. */
xcb_window_t
ui_dropdown_create(PhxRectangle configure, xcb_window_t transient_for_window) {

  xcb_connection_t *connection;
  PhxInterface *iface;
  uint32_t values[] = {  true  };
  xcb_window_t window = _window_create(configure);
  if (window == 0)  return 0;

  connection = session->connection;
  xcb_change_window_attributes(connection, window,
                               XCB_CW_OVERRIDE_REDIRECT, values);

  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
                             XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WINDOW, 32,
                             1, &transient_for_window);

  iface = _interface_create(connection, configure, &window);
  if (iface == NULL)  return 0;
  iface->type = PHX_ITDDL;
  session->iface[session->ncount] = iface;
  session->ncount += 1;
  iface->i_mount = _interface_for(transient_for_window);
  return window;
}

void
ui_window_name(xcb_window_t window) {

  xcb_connection_t *connection = session->connection;

  xcb_intern_atom_cookie_t cookie
    = xcb_intern_atom(connection, 1, 7, "WM_NAME");
  xcb_intern_atom_reply_t *reply
    = xcb_intern_atom_reply(connection, cookie, NULL);
  xcb_intern_atom_cookie_t cookie2
    = xcb_intern_atom(connection, 0, 6, "STRING");
  xcb_intern_atom_reply_t *reply2
    = xcb_intern_atom_reply(connection, cookie2, NULL);

  char title[16];
  sprintf(title, "%d", window);
  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window,
                       reply->atom, reply2->atom, 8,
                       strlen(title), title);
  free(reply);
  free(reply2);
}

