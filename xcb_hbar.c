#include "textviews.h"
#include "buttons.h"

extern void  ext_cairo_blur_surface(cairo_surface_t *, int, int);

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

#define BTN_HEADER_CLOSE    ((PhxButtonStyle)8)
#define BTN_HEADER_MINIMIZE ((PhxButtonStyle)9)
#define BTN_HEADER_MAXIMIZE ((PhxButtonStyle)10)
#define BTN_HEADER_MANAGER  ((PhxButtonStyle)11)

uint16_t HEADER_HEIGHT;

#pragma mark *** Drawing ***

static void
_draw_symbol_close(PhxObject *b, cairo_t *cr) {

  cairo_matrix_t matrix;
  double xc, yc;

  cairo_save(cr);

  xc = b->mete_box.x + b->draw_box.x + (b->draw_box.w / 2);
  yc = b->mete_box.y + b->draw_box.y + (b->draw_box.h / 2);

  cairo_set_source_rgba(cr, 0, 0, 0, 1);

  cairo_get_matrix(cr, &matrix);

  cairo_translate(cr, xc, yc);
  cairo_rotate(cr, M_PI/4);
  cairo_translate(cr, -xc, -yc);

  cairo_move_to(cr, b->mete_box.x + 3, yc);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 3, yc);
  cairo_set_line_width(cr, 1);
  cairo_stroke(cr);

  cairo_move_to(cr, xc, b->mete_box.y + 3);
  cairo_line_to(cr, xc, b->mete_box.y + b->draw_box.h - 3);
  cairo_set_line_width(cr, 1);
  cairo_stroke(cr);

  cairo_set_matrix(cr, &matrix);

  cairo_restore(cr);
}

static void
_draw_symbol_minimize(PhxObject *b, cairo_t *cr) {

  double yc;

  cairo_save(cr);

  yc = b->mete_box.y + b->draw_box.y + (b->draw_box.h / 2);

  cairo_set_source_rgba(cr, 0, 0, 0, 1);

  cairo_move_to(cr, b->mete_box.x + 3, yc);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 3, yc);
  cairo_set_line_width(cr, 1);
  cairo_stroke(cr);

  cairo_restore(cr);
}

static void
_draw_symbol_maximize(PhxObject *b, cairo_t *cr) {

  double xc, yc;

  cairo_save(cr);

  xc = b->mete_box.x + b->draw_box.x + (b->draw_box.w / 2);
  yc = b->mete_box.y + b->draw_box.y + (b->draw_box.h / 2);

  cairo_set_source_rgba(cr, 0, 0, 0, 1);

  cairo_move_to(cr, b->mete_box.x + 3, yc);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 3, yc);
  cairo_set_line_width(cr, 1);
  cairo_stroke(cr);

  cairo_move_to(cr, xc, b->mete_box.y + 3);
  cairo_line_to(cr, xc, b->mete_box.y + b->draw_box.h - 3);
  cairo_set_line_width(cr, 1);
  cairo_stroke(cr);

  cairo_restore(cr);
}

static void
_draw_symbol_move(PhxObject *b, cairo_t *cr) {

  double xc, yc;

  cairo_save(cr);

  cairo_set_source_rgba(cr, 0, 0, 0, 1);
  cairo_set_line_width(cr, 0.5);

  xc = b->mete_box.x + b->draw_box.x + (b->draw_box.w / 2);
  yc = b->mete_box.y + b->draw_box.y + (b->draw_box.h / 2);

    /* horizontal */
  cairo_move_to(cr, b->mete_box.x + 4, yc);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 4, yc);
  cairo_stroke(cr);
    /* right */
  cairo_move_to(cr, b->mete_box.x + b->draw_box.w - 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 4.5, yc + 1.75);
  cairo_stroke(cr);
  cairo_move_to(cr, b->mete_box.x + b->draw_box.w - 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 4.5, yc - 1.75);
  cairo_stroke(cr);
    /* left */
  cairo_move_to(cr, b->mete_box.x + 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + 4.5, yc + 2.25);
  cairo_stroke(cr);
  cairo_move_to(cr, b->mete_box.x + 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + 4, yc - 2.25);
  cairo_stroke(cr);

    /* vertical */
  cairo_move_to(cr, xc, b->mete_box.y + 4);
  cairo_line_to(cr, xc, b->mete_box.y + b->draw_box.h - 4);
  cairo_stroke(cr);
    /* top */
  cairo_move_to(cr, xc, b->mete_box.y + 2);
  cairo_line_to(cr, xc + 1.5, b->mete_box.y + 4);
  cairo_stroke(cr);
  cairo_move_to(cr, xc, b->mete_box.y + 2);
  cairo_line_to(cr, xc - 1.5, b->mete_box.y + 4);
  cairo_stroke(cr);
    /* bottom */
  cairo_move_to(cr, xc, b->mete_box.y + b->draw_box.h - 2);
  cairo_line_to(cr, xc - 1.5, b->mete_box.y + b->draw_box.h - 4);
  cairo_stroke(cr);
  cairo_move_to(cr, xc, b->mete_box.y + b->draw_box.h - 2);
  cairo_line_to(cr, xc + 1.5, b->mete_box.y + b->draw_box.h - 4);
  cairo_stroke(cr);

  cairo_restore(cr);
}

static void
_draw_symbol_resize(PhxObject *b, cairo_t *cr) {

  double xc, yc;
  cairo_matrix_t matrix;

  cairo_save(cr);

  cairo_set_source_rgba(cr, 0, 0, 0, 1);
  cairo_set_line_width(cr, 0.5);

  xc = b->mete_box.x + b->draw_box.x + (b->draw_box.w / 2);
  yc = b->mete_box.y + b->draw_box.y + (b->draw_box.h / 2);

  cairo_get_matrix(cr, &matrix);

  cairo_translate(cr, xc, yc);
  cairo_rotate(cr, M_PI/4);
  cairo_translate(cr, -xc, -yc);

    /* horizontal */
  cairo_move_to(cr, b->mete_box.x + 3, yc);
  cairo_line_to(cr, xc - 1.5, yc);
  cairo_stroke(cr);
  cairo_move_to(cr, xc + 1.5, yc);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w -3, yc);
  cairo_stroke(cr);
    /* right */
  cairo_move_to(cr, b->mete_box.x + b->draw_box.w - 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 4.5, yc + 1.75);
  cairo_stroke(cr);
  cairo_move_to(cr, b->mete_box.x + b->draw_box.w - 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + b->draw_box.w - 4.5, yc - 1.75);
  cairo_stroke(cr);
    /* left */
  cairo_move_to(cr, b->mete_box.x + 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + 4.5, yc + 2.25);
  cairo_stroke(cr);
  cairo_move_to(cr, b->mete_box.x + 2, yc + 0.25);
  cairo_line_to(cr, b->mete_box.x + 4, yc - 2.25);
  cairo_stroke(cr);

    /* vertical */
  cairo_move_to(cr, xc, b->mete_box.y + 3);
  cairo_line_to(cr, xc, yc - 1.5);
  cairo_stroke(cr);
  cairo_move_to(cr, xc, yc + 1.5);
  cairo_line_to(cr, xc, b->mete_box.y + b->draw_box.h - 3);
  cairo_stroke(cr);
    /* top */
  cairo_move_to(cr, xc, b->mete_box.y + 2);
  cairo_line_to(cr, xc + 1.5, b->mete_box.y + 4);
  cairo_stroke(cr);
  cairo_move_to(cr, xc, b->mete_box.y + 2);
  cairo_line_to(cr, xc - 1.5, b->mete_box.y + 4);
  cairo_stroke(cr);
    /* bottom */
  cairo_move_to(cr, xc, b->mete_box.y + b->draw_box.h - 2);
  cairo_line_to(cr, xc - 1.5, b->mete_box.y + b->draw_box.h - 4);
  cairo_stroke(cr);
  cairo_move_to(cr, xc, b->mete_box.y + b->draw_box.h - 2);
  cairo_line_to(cr, xc + 1.5, b->mete_box.y + b->draw_box.h - 4);
  cairo_stroke(cr);

  cairo_get_matrix(cr, &matrix);

  cairo_restore(cr);
}

static void
_draw_header_button(PhxObject *b, cairo_t *cr) {

  double bottom, top, radius, xc, yc;
  PhxAttr focus_out, *attrib;
  PhxRGBA *c;

  cairo_save(cr);

  bottom = b->mete_box.y + b->draw_box.y + b->draw_box.h;
  top = b->mete_box.y + b->draw_box.y;
  radius = b->draw_box.h / 2;
  xc = b->mete_box.x + b->draw_box.x + (b->draw_box.w / 2);
  yc = b->mete_box.y + b->draw_box.y + (b->draw_box.h / 2);

  cairo_new_sub_path(cr);
  cairo_arc(cr, xc, yc, radius, 0, M_PI * 2);
  cairo_clip_preserve(cr);

  attrib = b->attrib;

  if (!sensitive_get(b)) {
    focus_out.bg_fill.r = 0.0;
    focus_out.bg_fill.g = 0.0;
    focus_out.bg_fill.b = 0.0;
    focus_out.bg_fill.a = 0.0;
    focus_out.fg_fill.r = 0.8;
    focus_out.fg_fill.g = 0.8;
    focus_out.fg_fill.b = 0.8;
    focus_out.fg_fill.a = 1.0;
    focus_out.fg_ink.r  = 0.3;
    focus_out.fg_ink.g  = 0.3;
    focus_out.fg_ink.b  = 0.3;
    focus_out.fg_ink.a  = 1.0;
    focus_out.stroke  = 0.5;
    attrib = &focus_out;
    if ((b->state & 1) != 0)
     focus_out.fg_fill = b->attrib->fg_fill;
  }

    /* fill of button draw_box area */
  if (attrib->bg_fill.a != 0) {
    c = &attrib->bg_fill;
    cairo_set_source_rgba(cr, c->r, c->g, c->b, c->a);
    cairo_fill_preserve(cr);
  }

   /* fill in actual content */
  if (attrib->fg_fill.a != 0) {
    cairo_matrix_t matrix;
    cairo_pattern_t *r1;

    c = &attrib->fg_fill;
    cairo_get_matrix(cr, &matrix);

      /* fill in circle */
    cairo_translate(cr, xc, bottom + 6.5);
    cairo_scale(cr, 1, 0.7);
    cairo_translate(cr, -xc, -(bottom + 6.5));
    r1 = cairo_pattern_create_radial(xc, yc, 6,
                                     xc, yc, (double)(b->draw_box.h + 8.0));
    cairo_pattern_add_color_stop_rgba(r1, 0, c->r, c->g, c->b, c->a);
    cairo_pattern_add_color_stop_rgba(r1, 1, (c->r - 0.45),
                                             (c->g - 0.45),
                                             (c->b - 0.45),
                                             (c->a - 0.15));
    cairo_set_source(cr, r1);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(r1);

    cairo_set_matrix(cr, &matrix);

    r1 = cairo_pattern_create_radial(xc - 0.7, top + 3.5, .35,
                                     xc - 0.7, top + 3.5, 8);
    cairo_pattern_add_color_stop_rgba(r1, 0, (c->r + 0.65),
                                             (c->g + 0.65),
                                             (c->b + 0.65),
                                              c->a);

    cairo_pattern_add_color_stop_rgba(r1, 1, c->r,  c->g,  c->b, .08);
    cairo_set_source(cr, r1);
    cairo_fill(cr);
    cairo_pattern_destroy(r1);
  }
    /* colour of button border */
  if (attrib->fg_fill.a != 0) {
    if (frame_draw_get(b)) {
      double lw;
      c = &attrib->fg_fill;
      cairo_set_source_rgba(cr, c->r, c->g, c->b, c->a);
      lw = (attrib->stroke != 0.0) ? attrib->stroke : 0.5;
      cairo_set_line_width(cr, lw);
      cairo_stroke(cr);
    }
  }
  cairo_restore(cr);
}

static void
_draw_hdr_background(PhxObject *b, cairo_t *cr) {

  cairo_surface_t *tmp;
  cairo_t *cr_tmp;

  cairo_rectangle(cr, b->mete_box.x, b->mete_box.y,
                      b->mete_box.w, b->mete_box.h);
  cairo_clip(cr);

  tmp = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                             b->mete_box.w, 10);
  if (cairo_surface_status(tmp))  return;
  cr_tmp = cairo_create(tmp);
  cairo_move_to(cr_tmp, b->mete_box.x, 10);
  cairo_line_to(cr_tmp, b->mete_box.w, 10);

  cairo_set_source_rgba(cr_tmp, 0.2, 0.2, 0.2, 1);
  cairo_set_line_width(cr_tmp, 1);
  cairo_stroke(cr_tmp);

  ext_cairo_blur_surface(tmp, 0, 8);

  cairo_set_source_surface(cr, tmp, 0, b->mete_box.h - 9);
  cairo_paint(cr);
  cairo_destroy(cr_tmp);

    /* clean up */
  cairo_surface_destroy(tmp);
}

#pragma mark *** WM functions ***

static int16_t xroot, yroot;

static bool
_movesize_message(xcb_motion_notify_event_t *motion) {

  xcb_connection_t *connection = session->connection;
  xcb_screen_t *screen
    = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
  xcb_intern_atom_cookie_t c0;
  xcb_intern_atom_reply_t *r0;
  xcb_client_message_event_t *message;

  c0 = xcb_intern_atom(connection, 1, 18, "_NET_WM_MOVERESIZE");
  r0 = xcb_intern_atom_reply(connection, c0, NULL);

  xcb_ungrab_pointer(connection, motion->time);
  xcb_flush(connection);

  message = calloc(32, 1);
  message->response_type  = XCB_CLIENT_MESSAGE;
  message->format         = 32;
  message->window         = motion->event;
  message->type           = r0->atom;
  message->data.data32[0] = motion->root_x;
  message->data.data32[1] = motion->root_y;
  message->data.data32[2] = 8; /* movement only */
  message->data.data32[3] = XCB_BUTTON_MASK_1;
  message->data.data32[4] = 1; /* source indication normal application */
  xcb_send_event(connection, false, screen->root,
                     XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                     XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (char*)message);
  xcb_flush(connection);
  free(r0);
  return true;
}

static bool
_drag_motion_hbtn(PhxInterface *iface,
                  xcb_motion_notify_event_t *motion) {
  int32_t values[2];
  xcb_query_pointer_cookie_t c0
    = xcb_query_pointer(session->connection, iface->window);
  xcb_query_pointer_reply_t *r0
    = xcb_query_pointer_reply(session->connection, c0, NULL);
  xcb_flush(session->connection);
  values[0] = (iface->mete_box.x += (r0->root_x - xroot));
  values[1] = (iface->mete_box.y += (r0->root_y - yroot));
  xroot = r0->root_x;
  yroot = r0->root_y;
  free(r0);
#if USE_XLIB
  XMoveWindow(display, motion->event, values[0], values[1]);
#else
  xcb_configure_window(session->connection, motion->event,
               XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);
#endif
  xcb_flush(session->connection);
  return true;
}

static bool
_drag_begin_hbtn(PhxInterface *iface,
                 xcb_motion_notify_event_t *motion) {

  const char *named;
  puts(" start drag XCB_MOTION_NOTIFY");
  if (!!(motion->state & XCB_MOD_MASK_CONTROL))
        named = "top_right_corner";
  else  named = "move";
  ui_cursor_set_named(named, motion->event);
/* During testing, block resize for now. */
if (!(motion->state & XCB_MOD_MASK_CONTROL)) {
  if ( (_NET_WM_STRING != NULL)
      && (strcmp(_NET_WM_STRING, "Compiz") == 0) ) {
    return _movesize_message(motion);
  } else {
    xcb_grab_pointer_cookie_t c0;
    xcb_grab_pointer_reply_t *r0;
    c0 = xcb_grab_pointer(session->connection, 1, motion->event,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
           | XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        XCB_NONE, XCB_NONE, motion->time);
    r0 = xcb_grab_pointer_reply(session->connection, c0, NULL);
    if ((r0 == NULL) || (r0->status != XCB_GRAB_STATUS_SUCCESS)) {
      puts("grab failed");
      return false;
    }
    return true;
  }
}
  return true;
}

#pragma mark *** Events ***

static bool
_hbtn_close_event(PhxInterface *iface,
                  xcb_generic_event_t *nvt,
                  PhxObject *obj) {

  uint8_t response = nvt->response_type & (uint8_t)0x7F;
  if (response == XCB_BUTTON_RELEASE) {
    xcb_button_press_event_t *bp = (xcb_button_press_event_t*)nvt;
    xcb_client_message_event_t *message = calloc(32, 1);

    message->response_type  = XCB_CLIENT_MESSAGE;
    message->format         = 32;
    message->window         = bp->event;
    message->type           = WM_PROTOCOLS;
    message->data.data32[0] = WM_DELETE_WINDOW;
    message->data.data32[1] = bp->time;
    xcb_send_event(session->connection, false, message->window,
                       XCB_EVENT_MASK_NO_EVENT, (char*)message);
    xcb_flush(session->connection);
  } else if (response == XCB_ENTER_NOTIFY) {
    sensitive_set(obj, true);
    visible_set(obj->child, true);
    ui_invalidate_object(obj);
  } else if (response == XCB_LEAVE_NOTIFY) {
    sensitive_set(obj, (ui_active_focus_get() != NULL));
    visible_set(obj->child, false);
    ui_invalidate_object(obj);
  }
  return _default_button_meter(iface, nvt, obj);
}

static bool
_hbtn_minimize_event(PhxInterface *iface,
                     xcb_generic_event_t *nvt,
                     PhxObject *obj) {

  uint8_t response = nvt->response_type & (uint8_t)0x7F;
  if (response == XCB_BUTTON_RELEASE) {
    if (session->has_WM != 0) {
      xcb_button_press_event_t *bp = (xcb_button_press_event_t*)nvt;
      xcb_screen_t *screen
        = xcb_setup_roots_iterator(xcb_get_setup(session->connection)).data;
      xcb_intern_atom_cookie_t c0;
      xcb_intern_atom_reply_t *r0;
      xcb_client_message_event_t *message = calloc(32, 1);
      message->response_type  = XCB_CLIENT_MESSAGE;
      message->format         = 32;
      message->window         = bp->event;
      c0 = xcb_intern_atom(session->connection, 0, 15, "WM_CHANGE_STATE");
      r0 = xcb_intern_atom_reply(session->connection, c0, NULL);
      message->type           = r0->atom;
      message->data.data32[0] = 3;  /* IconicState */
      xcb_send_event(session->connection, false, screen->root,
                         XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                         XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (char*)message);
      xcb_flush(session->connection);
      free(r0);
    } else if (!(iface->state & SBIT_MAXIMIZED)) {
      uint32_t values[4];
        /* For now, implement a window shade type of minimize. */
      if (!!(iface->state & SBIT_MINIMIZED)) {
        iface->state &= ~SBIT_MINIMIZED;
        values[0] = iface->wmgr_box.x;
        values[1] = iface->wmgr_box.y;
        values[2] = iface->wmgr_box.w;
        values[3] = iface->wmgr_box.h;
        xcb_configure_window(session->connection, iface->window,
                     XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                   | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                     values);
      } else {
        iface->state |= SBIT_MINIMIZED;
        iface->wmgr_box = iface->mete_box;
        values[0] = iface->mete_box.x;
        values[1] = iface->mete_box.y;
        values[2] = iface->mete_box.w;
        values[3] = HEADER_HEIGHT;
      }
      xcb_configure_window(session->connection, iface->window,
                   XCB_CONFIG_WINDOW_X     | XCB_CONFIG_WINDOW_Y
                 | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                   values);
      xcb_flush(session->connection);
    }
  } else if (response == XCB_ENTER_NOTIFY) {
    sensitive_set(obj, true);
    visible_set(obj->child, true);
    ui_invalidate_object(obj);
  } else if (response == XCB_LEAVE_NOTIFY) {
    sensitive_set(obj, (ui_active_focus_get() != NULL));
    visible_set(obj->child, false);
    ui_invalidate_object(obj);
  }
  return _default_button_meter(iface, nvt, obj);
}

static bool
_hbtn_maximize_event(PhxInterface *iface,
                     xcb_generic_event_t *nvt,
                     PhxObject *obj) {

  uint8_t response = nvt->response_type & (uint8_t)0x7F;
  if (response == XCB_BUTTON_RELEASE) {
    xcb_button_press_event_t *bp = (xcb_button_press_event_t*)nvt;
    uint32_t mbit = !!(iface->state & SBIT_MAXIMIZED);
    xcb_connection_t *connection = session->connection;
    xcb_screen_t *screen
      = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    if (session->has_WM != 0) {
      xcb_intern_atom_cookie_t c0, c1, c2;
      xcb_intern_atom_reply_t *r0, *r1, *r2;
      xcb_client_message_event_t *message = calloc(32, 1);
      message->response_type  = XCB_CLIENT_MESSAGE;
      message->format         = 32;
      message->window         = bp->event;
      c0 = xcb_intern_atom(connection, 0, 13, "_NET_WM_STATE");
      c1 = xcb_intern_atom(connection, 0, 28, "_NET_WM_STATE_MAXIMIZED_HORZ");
      c2 = xcb_intern_atom(connection, 0, 28, "_NET_WM_STATE_MAXIMIZED_VERT");
      r0 = xcb_intern_atom_reply(connection, c0, NULL);
      r1 = xcb_intern_atom_reply(connection, c1, NULL);
      r2 = xcb_intern_atom_reply(connection, c2, NULL);
      message->type           = r0->atom;
      message->data.data32[0] = !mbit;
      iface->state ^= SBIT_MAXIMIZED;
      message->data.data32[1] = r1->atom;
      message->data.data32[2] = r2->atom;
      xcb_send_event(connection, false, screen->root,
                         XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
                         XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY, (char*)message);
      xcb_flush(connection);
      free(r0);
      free(r1);
      free(r2);
    } else if (!(iface->state & SBIT_MINIMIZED)) {
      uint32_t values[4];
      if (mbit) {
        iface->state &= ~SBIT_MAXIMIZED;
        values[0] = iface->wmgr_box.x;
        values[1] = iface->wmgr_box.y;
        values[2] = iface->wmgr_box.w;
        values[3] = iface->wmgr_box.h;
      } else {
          /* When button was pressed, window became topmost. */
        iface->state |= SBIT_MAXIMIZED;
        iface->wmgr_box = iface->mete_box;
        values[0] = 0;
        values[1] = 0;
        values[2] = screen->width_in_pixels;
        values[3] = screen->height_in_pixels;
      }
      xcb_configure_window(connection, iface->window,
                   XCB_CONFIG_WINDOW_X     | XCB_CONFIG_WINDOW_Y
                 | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                   values);
      xcb_flush(connection);
    }
  } else if (response == XCB_ENTER_NOTIFY) {
    sensitive_set(obj, true);
    visible_set(obj->child, true);
    ui_invalidate_object(obj);
  } else if (response == XCB_LEAVE_NOTIFY) {
    sensitive_set(obj, (ui_active_focus_get() != NULL));
    visible_set(obj->child, false);
    ui_invalidate_object(obj);
  }
  return _default_button_meter(iface, nvt, obj);
}

static bool
_hbtn_manager_event(PhxInterface *iface,
                    xcb_generic_event_t *nvt,
                    PhxObject *obj) {

  uint8_t response = nvt->response_type & (uint8_t)0x7F;

  if ( (response == XCB_KEY_PRESS)
      || (response == XCB_KEY_RELEASE) ) {
      /* Allow modifier keys to alter functionality when no drag. */
    if (ui_active_drag_get() == NULL) {
      xcb_key_press_event_t *kp = (xcb_key_press_event_t*)nvt;
      if (response == XCB_KEY_PRESS) {
        xcb_keysym_t keyval = _xcb_keysym_for(kp->detail, kp->state);
        if (   (keyval == 0x0ffe3)                       /* XK_Control_L */
            || (keyval == 0x0ffe4) ) {                   /* XK_Control_R */
          obj->child->_draw_cb = _draw_symbol_resize;
          ui_invalidate_object(obj);
          return true;
        }
        return false;
      }
      if (!!(kp->state & XCB_MOD_MASK_CONTROL)) {
        obj->child->_draw_cb = _draw_symbol_move;
        ui_invalidate_object(obj);
        return true;
      }
    }
      /* Even thou we respond to modifier, pass false to allow
        key combinations '^q' to exist. */
    return false;
  }

  if (response == XCB_ENTER_NOTIFY) {
    sensitive_set(obj, true);
    visible_set(obj->child, true);
    ui_invalidate_object(obj);
  } else if (response == XCB_LEAVE_NOTIFY) {
    sensitive_set(obj, (ui_active_focus_get() != NULL));
    visible_set(obj->child, false);
    ui_invalidate_object(obj);
  }
  return _default_button_meter(iface, nvt, obj);
}

/*
  When 'within' headerbar, focus clicks shouldn't apply. Additionally
  keyboard can hold meaning to objects.
*/
static bool
_default_headerbar_meter(PhxInterface *iface,
                         xcb_generic_event_t *nvt,
                         PhxObject *obj) {

  uint8_t response;

    /* We handle only == PHX_IFACE. Not its variants. */
  if (iface->type != PHX_IFACE)  return false;

  DEBUG_EVENTS("_default_headerbar_meter");

  response = nvt->response_type & (uint8_t)0x7F;

    /* Must have focus to respond to other events. */
  if (session->has_focus == NULL) {
    if (response == XCB_FOCUS_IN)  goto focus_set;
    return false;
  }

  if ( (response == XCB_ENTER_NOTIFY)
      || (response == XCB_LEAVE_NOTIFY) ) {
    return _default_nexus_meter(iface, nvt, obj);
  }

  if ( (response == XCB_FOCUS_IN)
      || (response == XCB_FOCUS_OUT) ) {
    PhxNexus *hbar;
    uint16_t ndx;
focus_set:
    hbar = (PhxNexus*)obj;
    ndx = hbar->ncount;
    while (ndx != 0) {
      PhxObject *inspect = hbar->objects[(--ndx)];
      sensitive_set(inspect, (response == XCB_FOCUS_IN));
      ui_invalidate_object(inspect);
    }
    return false;
  }

  if ( (response == XCB_KEY_PRESS)
      || (response == XCB_KEY_RELEASE) ) {
    if (obj->type == ((BTN_HEADER_MANAGER << 8) | PHX_BUTTON))
      return _hbtn_manager_event(iface, nvt, obj);
    return _default_nexus_meter(iface, nvt, obj);
  }

  if (response == XCB_BUTTON_PRESS) {
    xcb_button_press_event_t *mouse
      = (xcb_button_press_event_t*)nvt;
    xroot = mouse->root_x;
    yroot = mouse->root_y;
    return _default_nexus_meter(iface, nvt, obj);
  }

    /* Compiz semi-notifies exits thru process_events()' xing events.
      _drag_motion_hbtn() exits thru here. */
  if (response == XCB_BUTTON_RELEASE) {
    if (!!(iface->state & SBIT_HBR_DRAG)) {
      xcb_button_press_event_t *mouse
        = (xcb_button_press_event_t*)nvt;
      xcb_ungrab_pointer(session->connection, mouse->time);
      xcb_flush(session->connection);
      puts(" finished drag XCB_BUTTON_RELEASE");
      iface->state &= ~SBIT_HBR_DRAG;
      ui_active_drag_set(NULL);
      ui_active_within_set(obj);
      return true;
    }
    return _default_nexus_meter(iface, nvt, obj);
  }

  if (response == XCB_MOTION_NOTIFY) {
    xcb_motion_notify_event_t *motion
      = (xcb_motion_notify_event_t*)nvt;

    if (!!(iface->state & SBIT_HBR_DRAG))
      return _drag_motion_hbtn(iface, motion);

    if (!!(motion->state & XCB_BUTTON_MASK_1)) {
        /* 'obj' != NULL at this point. */
      if (obj->type == ((BTN_HEADER_MANAGER << 8) | PHX_BUTTON)) {
          /* Not returning normally from drag, so kill
            button default of release. */
        obj->state &= ~OBIT_BTN_PRESS;
        iface->state |= SBIT_HBR_DRAG;
        ui_active_drag_set((PhxObject*)obj->i_mount);
        return _drag_begin_hbtn(iface, motion);
      }
      return false;
    }
    return _default_nexus_meter(iface, nvt, obj);
  }

  if (response == XCB_CONFIGURE_NOTIFY) {
    xcb_configure_notify_event_t *configure
      = (xcb_configure_notify_event_t*)nvt;

    PhxNexus *hbar = (PhxNexus*)obj;
    int16_t wD = configure->width - hbar->mete_box.w;
    if (wD != 0) {
      uint16_t idx;
      PhxObject *wm_button = NULL;
      if ((idx = hbar->ncount) != 0)
        do {
          PhxObject *inspect = hbar->objects[(--idx)];
          if (inspect->type == ((BTN_HEADER_MANAGER << 8) | PHX_BUTTON)) {
            wm_button = inspect;
            break;
          }
        } while (idx != 0);
      if (wm_button != NULL) {
        wm_button->mete_box.x += wD;
        ui_invalidate_object(obj);
      }
    }
  }

  return _default_nexus_meter(iface, nvt, obj);
}

#pragma mark *** Main ***

PhxNexus *
ui_headerbar_for(PhxInterface *iface) {

  uint16_t idx;
  if ((idx = iface->ncount) != 0)
    do {
      PhxNexus *inspect = iface->nexus[(--idx)];
      if (inspect->type == PHX_HEADERBAR)  return inspect;
    } while (idx != 0);
  return NULL;
}

static char *
user_textstring(void) {

  char *string;
  size_t rdSz;
  long filesize;
  FILE *rh = fopen("./libctype/locale/rwSystems/numbers.c", "r");
  if (rh == NULL) {  puts("file not found"); return NULL;  }
  fseek(rh, 0 , SEEK_END);
  filesize = ftell(rh);
  fseek(rh, 0 , SEEK_SET);
  rdSz = (filesize + TGAP_ALLOC) & ~(TGAP_ALLOC - 1);
  string = malloc(rdSz);
  memset(&string[filesize], 0, (rdSz - filesize));
  if (fread(string, filesize, 1, rh) == 0)
    puts("error: reading file xcb_textview_drag.c");
  fclose(rh);

  return string;
}

static void
user_add_text(PhxInterface *iface) {

  PhxObjectTextview *otxt;
  char *text;
  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNexus *hbar = ui_headerbar_for(iface);

  RECTANGLE(nexus_box, 0, hbar->mete_box.h,
                          iface->mete_box.w,
                          iface->mete_box.h - hbar->mete_box.h);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;

  nexus_box.y = 0;
  otxt = ui_textview_create(nexus, nexus_box);
  text = user_textstring();
  ui_textview_buffer_set(otxt, text, HJST_LFT);
  free(text);
}

static PhxNexus *
user_configure_layout(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *hbar;
  PhxObjectButton *obtn;
  int16_t xpos, ypos, sz;
  PhxRGBA none          = { 0, 0, 0, 0 };
  PhxRGBA header_red    = { 1, .6, .6, 1 };
  PhxRGBA header_yellow = { .95, .95, 0, 1 };
  PhxRGBA header_green  = { 0, 1, 0, 1 };
  PhxRGBA header_blue   = { 0.6, 0.58, 1, 1 };
  double  header_stroke = 0.5;

    /* Remove wm decorations */
  xcb_window_t window = iface->window;
  ui_window_undecorate_set(window);

    /* On an undecorated window, allow only 1 headerbar.
      Do not restrict to being first nexus created. */
  if ((iface->state & SBIT_HEADERBAR) != 0)
    return ui_headerbar_for(iface);

  RECTANGLE(nexus_box, 0, 0, 500, 14 + 10);
  hbar = ui_nexus_create(iface, nexus_box);
  hbar->state = HXPD_RGT;
  hbar->type = PHX_HEADERBAR;
  iface->state |= SBIT_HEADERBAR;
  hbar->_draw_cb = _draw_hdr_background;
  hbar->_event_cb = _default_headerbar_meter;

  HEADER_HEIGHT = (14 + 10);

    /* Creation of close button */
  ypos = (int16_t)(((double)(14 + 10) * 0.208333) + 0.499999);
  xpos = ypos + 3;
  sz = (14 + 10) - (ypos << 1);
  nexus_box.x = xpos;
  nexus_box.y = ypos;
  nexus_box.w = (nexus_box.h = sz);
  obtn = ui_button_create(hbar, BTN_HEADER_CLOSE, nexus_box);
  frame_remove(obtn);
  obtn->_draw_cb = _draw_header_button;
  obtn->_event_cb = _hbtn_close_event;
  obtn->attrib->fg_ink = (obtn->attrib->bg_fill = none);
  obtn->attrib->fg_fill = header_red;
  obtn->attrib->fg_ink.a  = 1.0;
  obtn->attrib->stroke  = header_stroke;
  obtn->child = ui_object_child_create(obtn, PHX_DRAWING, NULL, nexus_box);
  obtn->child->_draw_cb = _draw_symbol_close;
  visible_set(obtn->child, false);

    /* Creation of minimize button */
  nexus_box.x = sz + (2 * xpos) - 1;
  nexus_box.y = ypos;
  obtn = ui_button_create(hbar, BTN_HEADER_MINIMIZE, nexus_box);
  frame_remove(obtn);
  obtn->_draw_cb = _draw_header_button;
  obtn->_event_cb = _hbtn_minimize_event;
  obtn->attrib->fg_ink = (obtn->attrib->bg_fill = none);
  obtn->attrib->fg_fill = header_yellow;
  obtn->attrib->fg_ink.a  = 1.0;
  obtn->attrib->stroke  = header_stroke;
  obtn->child = ui_object_child_create(obtn, PHX_DRAWING, NULL, nexus_box);
  obtn->child->_draw_cb = _draw_symbol_minimize;
  visible_set(obtn->child, false);

    /* Creation of maximize button */
  nexus_box.x = (2 * (sz - 1)) + (3 * xpos);
  nexus_box.y = ypos;
  obtn = ui_button_create(hbar, BTN_HEADER_MAXIMIZE, nexus_box);
  frame_remove(obtn);
  obtn->_draw_cb = _draw_header_button;
  obtn->_event_cb = _hbtn_maximize_event;
  obtn->attrib->fg_ink = (obtn->attrib->bg_fill = none);
  obtn->attrib->fg_fill = header_green;
  obtn->attrib->fg_ink.a  = 1.0;
  obtn->attrib->stroke  = header_stroke;
  obtn->child = ui_object_child_create(obtn, PHX_DRAWING, NULL, nexus_box);
  obtn->child->_draw_cb = _draw_symbol_maximize;
  visible_set(obtn->child, false);

    /* Creation of move/resize button */
  nexus_box.x = iface->mete_box.w - (sz + 1 + xpos);
  nexus_box.y = ypos;
  obtn = ui_button_create(hbar, BTN_HEADER_MANAGER, nexus_box);
  frame_remove(obtn);
  obtn->state = (obtn->state & ~HJST_MSK) | HJST_RGT;
  obtn->_draw_cb = _draw_header_button;
  obtn->_event_cb = _hbtn_manager_event;
  obtn->attrib->fg_ink = (obtn->attrib->bg_fill = none);
  obtn->attrib->fg_fill = header_blue;
  obtn->attrib->fg_ink.a  = 1.0;
  obtn->attrib->stroke  = header_stroke;
  obtn->child = ui_object_child_create(obtn, PHX_DRAWING, NULL, nexus_box);
    /* default state is to move, keyboard alters to allow resize. */
  obtn->child->_draw_cb = _draw_symbol_move;
  visible_set(obtn->child, false);

  return hbar;
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 100, 100, 500, 500 };

#if DEBUG_EVENTS_ON
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_CONFIGURE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_ENTER_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_LEAVE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_PROPERTY_NOTIFY);
#endif

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of _interface_for() */
  user_configure_layout(session->iface[0]);
  user_add_text(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
