#include "buttons.h"
#include "banks.h"

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

#pragma mark *** Utilities ***

/* Allows shutting off of outlines */
void
frame_draw_set(PhxObjectButton *obj, bool draws) {
  if (draws)      obj->state &= ~OBIT_BTN_FRAME;
  else            obj->state |=  OBIT_BTN_FRAME;
}

__inline bool
frame_draw_get(PhxObjectButton *obj) {
  return ((obj->state & OBIT_BTN_FRAME) == 0);
}

/* Retreive non-clipped rectangle to draw in botton frames. */
void
_button_draw_area_request(PhxObjectButton *obj, PhxRectangle *dbox) {

  bool frame;
  int16_t line_width;
  PhxButtonStyle style;

  if ( (obj == NULL) || (dbox == NULL)
      || (OBJECT_BASE_TYPE(obj) != PHX_BUTTON) )
    return;

  frame = frame_draw_get(obj);
  line_width = (int16_t)(obj->attrib->stroke + 0.5);
  DEBUG_ASSERT((frame && (line_width <= 0)),
                       "failure: attrib->stroke invalid.");

  dbox->x = obj->draw_box.x + line_width;
  dbox->y = obj->draw_box.y + line_width;
  dbox->w = obj->draw_box.w - (line_width * 2);
  dbox->h = obj->draw_box.h - line_width;

  style = obj->type >> 8;
  if ( (style == BTN_COMBO_ARROW) || (style == BTN_COMBO_WHEEL) )
    dbox->w -= (int16_t)(((67.824176 / 152.615385) * obj->draw_box.h)
                            + line_width);

  DEBUG_ASSERT((dbox->w <= 0), "failure: arrow drawing exceeds bounds.");
}

#pragma mark *** Drawing ***

static void
_draw_button_round_corner(PhxObject *b, cairo_t *cr) {

  const double degrees = M_PI / 180.0;
  double line_width, x, y, w, h, radius;

  PhxObjectButton *obtn = (PhxObjectButton *)b;
  PhxAttr *c = obtn->attrib;

  x = obtn->mete_box.x;
  y = obtn->mete_box.y;
  w = obtn->mete_box.w;
  h = obtn->mete_box.h;

  cairo_rectangle(cr, x, y, w, h);
  cairo_clip_preserve(cr);

  cairo_set_source_rgba(cr, c->bg_fill.r,
                            c->bg_fill.g,
                            c->bg_fill.b,
                            c->bg_fill.a);
  cairo_fill(cr);

  line_width = 0.5;
  cairo_set_line_width(cr, line_width);
  x += obtn->draw_box.x + line_width;
  y += obtn->draw_box.y + line_width;
  w = obtn->draw_box.w - 1;
  h = obtn->draw_box.h;
  radius = obtn->draw_box.h / 6.5;

    /* shade bottom, overprint the following. */
  cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.4);
  cairo_rectangle(cr, x, y + h, w, 2);
  cairo_fill(cr);

  cairo_new_sub_path(cr);
  cairo_arc(cr, x + w - radius, y + radius,
                radius, -90 * degrees,   0 * degrees);
  cairo_arc(cr, x + w - radius, y + h - radius,
                radius,   0 * degrees,  90 * degrees);
  cairo_arc(cr, x + radius, y + h - radius,
                radius,  90 * degrees, 180 * degrees);
  cairo_arc(cr, x + radius, y + radius,
                radius, 180 * degrees, 270 * degrees);
  cairo_close_path(cr);

  if ((obtn->state & OBIT_BTN_PRESS) != 0)
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
  else {
    double alpha = ui_sensitive_get(obtn) ? c->fg_fill.a : (c->fg_fill.a * 0.5);
    cairo_set_source_rgba(cr, c->fg_fill.r,
                              c->fg_fill.g,
                              c->fg_fill.b, alpha);
  }
  if (frame_draw_get(obtn)) {
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, c->fg_ink.r,
                              c->fg_ink.g,
                              c->fg_ink.b,
                              c->fg_ink.a);
    cairo_set_line_width(cr, c->stroke);
    cairo_stroke(cr);
  } else {
    cairo_fill(cr);
  }
}

static void
_draw_button_combo_arrow(PhxObject *b, cairo_t *cr) {

  double line_width, x, y, w, h, alpha;

  PhxObjectButton *obtn = (PhxObjectButton*)b;
  PhxAttr *c = obtn->attrib;

  _draw_button_round_corner(b, cr);

  line_width = 0.5;
  x = obtn->mete_box.x + obtn->draw_box.x;
  y = obtn->mete_box.y + obtn->draw_box.y - line_width;
  w = (67.824176 / 152.615385) * obtn->draw_box.h;
  h = obtn->draw_box.h + line_width - 3;
  x += obtn->draw_box.w - (w + 1.5);
  y += 1.5;

  alpha = ui_sensitive_get(obtn) ? c->fg_ink.a : (c->fg_ink.a * 0.5);
  cairo_set_source_rgba(cr, c->fg_ink.r,
                            c->fg_ink.g,
                            c->fg_ink.b,
                            alpha);

  cairo_new_path(cr);  /* current path is not consumed by another */
  cairo_move_to(cr, (x + (w/2)),  y);
  cairo_line_to(cr, (x + w),     (y + (h/2) - .5));
  cairo_line_to(cr,  x,          (y + (h/2) - .5));
  cairo_line_to(cr, (x + (w/2)),  y);
  cairo_move_to(cr, (x + (w/2)), (y + h));
  cairo_line_to(cr, (x + w),     (y + (h/2) + .5));
  cairo_line_to(cr,  x,          (y + (h/2) + .5));
  cairo_line_to(cr, (x + (w/2)), (y + h));
  cairo_fill(cr);
}

static void
_draw_button_navigate_right(PhxObject *b, cairo_t *cr) {

  const double degrees = M_PI / 180.0;
  double line_width, x, y, w, h, radius, sp, alpha;

  PhxObjectButton *obtn = (PhxObjectButton*)b;
  PhxAttr *c = obtn->attrib;

  x = obtn->mete_box.x;
  y = obtn->mete_box.y;
  w = obtn->mete_box.w;
  h = obtn->mete_box.h;

  cairo_rectangle(cr, x, y, w, h);
  cairo_clip_preserve(cr);

  cairo_set_source_rgba(cr, c->bg_fill.r,
                            c->bg_fill.g,
                            c->bg_fill.b,
                            c->bg_fill.a);
  cairo_fill(cr);

  line_width = 0.5;
  cairo_set_line_width(cr, line_width);
  x += obtn->draw_box.x + line_width;
  y += obtn->draw_box.y + line_width;
  w = obtn->draw_box.w;
  h = obtn->draw_box.h;
  radius = obtn->draw_box.h / 6.5;
  sp = h / 5;

    /* shade bottom, overprint the following. */
  cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.4);
  cairo_rectangle(cr, x, y + h, w, 2);
  cairo_fill(cr);

  cairo_new_sub_path(cr);
  cairo_arc(cr, x + w - radius, y + radius,
                radius, -90 * degrees,   0 * degrees);
  cairo_line_to(cr, x + w, y + h - radius);
  cairo_arc(cr, x + w - radius, y + h - radius,
                radius,   0 * degrees,  90 * degrees);
  cairo_line_to(cr, x, y + h);
  cairo_line_to(cr, x, y);
  cairo_line_to(cr, x + w - radius, y);
  cairo_close_path(cr);

  if ((obtn->state & OBIT_BTN_PRESS) != 0)
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
  else {
    alpha = ui_sensitive_get(obtn) ? c->fg_fill.a : (c->fg_fill.a * 0.5);
    cairo_set_source_rgba(cr, c->fg_fill.r,
                              c->fg_fill.g,
                              c->fg_fill.b, alpha);
  }
  if (frame_draw_get(obtn)) {
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, c->fg_ink.r,
                              c->fg_ink.g,
                              c->fg_ink.b,
                              c->fg_ink.a);
    cairo_set_line_width(cr, c->stroke);
    cairo_stroke(cr);
  } else {
    cairo_fill(cr);
  }

  alpha = ui_sensitive_get(obtn) ? c->fg_ink.a : (c->fg_ink.a * 0.5);
  cairo_set_source_rgba(cr, c->fg_ink.r + 0.2,
                            c->fg_ink.g + 0.2,
                            c->fg_ink.b + 0.2,
                            alpha);
  cairo_move_to(cr, (x + w - sp + 0.5), (y + (h / 2)));
  cairo_line_to(cr,  x + sp + 0.5,       y + sp);
  cairo_line_to(cr,  x + sp + 0.5,      (y + h - sp));
  cairo_line_to(cr, (x + w - sp + 0.5), (y + (h / 2)));
  cairo_fill(cr);
}

static void
_draw_button_navigate_left(PhxObject *b, cairo_t *cr) {

  const double degrees = M_PI / 180.0;
  double line_width, x, y, w, h, radius, sp, alpha;

  PhxObjectButton *obtn = (PhxObjectButton*)b;
  PhxAttr *c = obtn->attrib;

  x = obtn->mete_box.x;
  y = obtn->mete_box.y;
  w = obtn->mete_box.w;
  h = obtn->mete_box.h;

  cairo_rectangle(cr, x, y, w, h);
  cairo_clip_preserve(cr);

  cairo_set_source_rgba(cr, c->bg_fill.r,
                            c->bg_fill.g,
                            c->bg_fill.b,
                            c->bg_fill.a);
  cairo_fill(cr);

  line_width = 0.5;
  cairo_set_line_width(cr, line_width);
  x += obtn->draw_box.x + line_width;
  y += obtn->draw_box.y + line_width;
  w = obtn->draw_box.w;
  h = obtn->draw_box.h;
  radius = obtn->draw_box.h / 6.5;
  sp = h / 5;

    /* shade bottom, overprint the following. */
  cairo_set_source_rgba(cr, 0.7, 0.7, 0.7, 0.4);
  cairo_rectangle(cr, x, y + h, w, 2);
  cairo_fill(cr);

  cairo_new_sub_path(cr);
  cairo_arc(cr, x + radius, y + radius,
                radius, 180 * degrees, 270 * degrees);
  cairo_line_to(cr, x + w, y);
  cairo_line_to(cr, x + w , y + h);
  cairo_line_to(cr, x + radius , y + h);
  cairo_arc(cr, x + radius, y + h - radius,
                radius,  90 * degrees, 180 * degrees);
  cairo_line_to(cr, x, y + radius);
  cairo_close_path(cr);

  if ((obtn->state & OBIT_BTN_PRESS) != 0)
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
  else {
    alpha = ui_sensitive_get(obtn) ? c->fg_fill.a : (c->fg_fill.a * 0.5);
    cairo_set_source_rgba(cr, c->fg_fill.r,
                              c->fg_fill.g,
                              c->fg_fill.b, alpha);
  }
  if (frame_draw_get(obtn)) {
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, c->fg_ink.r,
                              c->fg_ink.g,
                              c->fg_ink.b,
                              c->fg_ink.a);
    cairo_set_line_width(cr, c->stroke);
    cairo_stroke(cr);
  } else {
    cairo_fill(cr);
  }

  alpha = ui_sensitive_get(obtn) ? c->fg_ink.a : (c->fg_ink.a * 0.5);
  cairo_set_source_rgba(cr, c->fg_ink.r + 0.2,
                            c->fg_ink.g + 0.2,
                            c->fg_ink.b + 0.2,
                            alpha);
  cairo_move_to(cr,  x + sp - 0.5,      (y + (h / 2)));
  cairo_line_to(cr, (x + w - sp - 0.5),  y + sp);
  cairo_line_to(cr, (x + w - sp - 0.5), (y + h) - sp);
  cairo_line_to(cr,  x + sp - 0.5,      (y + (h / 2)));
  cairo_fill(cr);
}

void
ui_draw_button(PhxObject *b, cairo_t *cr) {

  PhxObjectButton *obtn = (PhxObjectButton *)b;
  PhxButtonStyle style = obtn->type >> 8;

    /* default, unknown, unimplemented fall to round corner */
  if      (style == BTN_NAVIGATE_LEFT)
    _draw_button_navigate_left(b, cr);
  else if (style == BTN_NAVIGATE_RIGHT)
    _draw_button_navigate_right(b, cr);
  else if (style == BTN_COMBO_ARROW)
    _draw_button_combo_arrow(b, cr);
  else
    _draw_button_round_corner(b, cr);
/*
  else {
    puts("unimplemented, using ROUND_CORNER");
    obj->_draw_cb = ui_draw_button;
  }
*/
}

#pragma mark *** Events ***

/* XXX add visiblity */
bool
_default_button_meter(PhxInterface *iface,
                      xcb_generic_event_t *nvt,
                      PhxObject *obj) {

  switch (nvt->response_type & (uint8_t)0x7F) {

    case XCB_BUTTON_PRESS: {      /* response_type 4 */

        /* Want 'activate' look if in draw area and has _event_cb. */
      if (obj->_event_cb != NULL) {
        xcb_button_press_event_t *mouse = (xcb_button_press_event_t*)nvt;
        int16_t x = mouse->event_x;
        int16_t y = mouse->event_y;
        PhxRectangle pbox = obj->draw_box;
        pbox.x += obj->mete_box.x;
        pbox.y += obj->mete_box.y;
        if ( (x >= pbox.x) && (x < (pbox.x + pbox.w))
            && (y >= pbox.y) && (y < (pbox.y + pbox.h)) ) {
          obj->state |= OBIT_BTN_PRESS;
          ui_invalidate_object(obj);
        }
      }
      return true;
    }
    case XCB_ENTER_NOTIFY: {      /* response_type 7 */
      ui_cursor_set_named(NULL, obj->i_mount->window);
      return true;
    }
    case XCB_LEAVE_NOTIFY: {      /* response_type 8 */
      return true;
    }

    case XCB_BUTTON_RELEASE: {    /* response_type 5 */
      if ((obj->state & OBIT_BTN_PRESS) != 0) {
        obj->state &= ~OBIT_BTN_PRESS;
        ui_invalidate_object(obj);
      }
      return true;
    }

    case XCB_CONFIGURE_NOTIFY: {  /* response_type 22 */

        /* Button configure of moving x | y only default we handle. */
      xcb_configure_notify_event_t *configure;
      configure = (xcb_configure_notify_event_t*)nvt;

      if ((obj->state & HXPD_LFT) != 0)
        obj->mete_box.x += (configure->width  - obj->mete_box.w);
      if ((obj->state & VXPD_TOP) != 0)
        obj->mete_box.y += (configure->height - obj->mete_box.h);

      return true;
    }

    default:
      break;
  }
  return false;
}

#pragma mark *** Creation ***

void
frame_remove(PhxObjectButton *obj) {
  obj->state |= OBIT_BTN_FRAME;
  obj->draw_box.x = (obj->draw_box.y = 0);
  obj->draw_box.w = obj->mete_box.w;
  obj->draw_box.h = obj->mete_box.h;
}

__inline void
frame_define(PhxObjectButton *obtn, PhxRectangle dbox) {
  obtn->draw_box = dbox;
}

static PhxObject *
_button_label_combo(PhxObjectButton *obtn,
                    PhxRectangle configure,
                    char *text,
                    uint32_t jstfy) {
  PhxBank  *ibank;
  PhxNexus *nexus;

    /* Retreive or create DDL_COMBO. */
  ibank = ui_dropdown_from(obtn);
  if (ibank == NULL) {
    ibank = ui_bank_create(obtn, DDL_COMBO, NULL);
    obtn->exclusive = ibank;
  }

    /* Because combo request, we control positioning. */
  nexus = ibank->nexus[0];
  configure.x = (configure.y = 0);
  if (nexus->ncount != 0)
    configure.y = nexus->objects[(nexus->ncount - 1)]->mete_box.y
                + nexus->objects[(nexus->ncount - 1)]->mete_box.h;

    /* Create requested object. */
  return ui_bank_label_create(ibank, configure, text, jstfy);
}

void
_button_label_raze(void *obj) {
  PhxObject *obtn = (PhxObject*)obj;
  _default_label_raze(obtn->child);
  obtn->ncount -= 1;
  obtn->child = NULL;
  _default_button_raze(obj);
}

PhxObjectLabel *
ui_button_label_create(PhxObjectButton *obtn, char *str, uint32_t jstfy) {

  PhxObjectLabel *olbl;
  PhxRectangle dbox;

  _button_draw_area_request(obtn, &dbox);

  if ( (obtn->type == ((BTN_COMBO_ARROW << 8) | PHX_BUTTON))
      || (obtn->type == ((BTN_COMBO_WHEEL << 8) | PHX_BUTTON)) ) {
    return _button_label_combo(obtn, dbox, str, jstfy);
  }

  if (obtn->child != NULL) {
    DEBUG_ASSERT(true, "warning: replacing child. ui_button_label_create()");
    if (obtn->child->_raze_cb != NULL)
         obtn->child->_raze_cb((void*)obtn->child);
    else  _default_object_raze((void*)obtn->child);
  }

  olbl = ui_label_create((PhxNexus*)obtn->i_mount, dbox, str, jstfy);

    /* label needs to be removed from nexus, added as a child */
  obtn->i_mount->ncount -= 1;
  obtn->i_mount->nexus[obtn->i_mount->ncount] = NULL;

  obtn->child = olbl;
  obtn->ncount += 1;
  obtn->_raze_cb = _button_label_raze;
  olbl->o_mount = obtn;

  return olbl;
}

static PhxObject *
_button_object_combo(PhxObjectButton *obtn,
                     PhxObjectType obj_type,
                     PhxDrawHandler obj_draw,
                     PhxRectangle configure) {
  PhxBank   *ibank;
  PhxNexus  *nexus;
  PhxObject *obj;

    /* Retreive or create DDL_COMBO. */
  ibank = ui_dropdown_from(obtn);
  if (ibank == NULL)
    ibank = ui_bank_create(obtn, DDL_COMBO, NULL);

    /* Because combo request, we control positioning. */
  nexus = ibank->nexus[0];
  configure.x = (configure.y = 0);
  if (nexus->ncount != 0)
    configure.y = nexus->objects[(nexus->ncount - 1)]->mete_box.y
                + nexus->objects[(nexus->ncount - 1)]->mete_box.h;

    /* Create requested object. */
  obj = ui_object_create(ibank->nexus[0], obj_type, obj_draw, configure);
    /* Logic simular to label, default to transparent fills. */
  obj->attrib->bg_fill.a = 0;
  obj->attrib->fg_fill.a = 0;

  return obj;
}

void
_button_object_raze(void *obj) {
  PhxObject *obtn = (PhxObject*)obj;
  _default_object_raze(obtn->child);
  obtn->ncount -= 1;
  obtn->child = NULL;
  _default_button_raze(obj);
}

PhxObject *
ui_button_object_create(PhxObjectButton *obtn,
                        PhxObjectType type,
                        PhxDrawHandler draw) {

  PhxObject *obj;
  PhxRectangle dbox;

  _button_draw_area_request(obtn, &dbox);

  if ( (obtn->type == ((BTN_COMBO_ARROW << 8) | PHX_BUTTON))
      || (obtn->type == ((BTN_COMBO_WHEEL << 8) | PHX_BUTTON)) ) {
    return _button_object_combo(obtn, type, draw, dbox);
  }

  if (obtn->child != NULL) {
    DEBUG_ASSERT(true, "warning: replacing child. ui_button_label_create()");
    if (obtn->child->_raze_cb != NULL)
         obtn->child->_raze_cb((void*)obtn->child);
    else  _default_object_raze((void*)obtn->child);
  }

  obj = ui_object_create((PhxNexus*)obtn->i_mount, type, draw, dbox);
    /* Logic simular to label, default to transparent fills. */
  obj->attrib->bg_fill.a = 0;
  obj->attrib->fg_fill.a = 0;

    /* object needs to be removed from nexus, added as a child */
  obtn->i_mount->ncount -= 1;
  obtn->i_mount->nexus[obtn->i_mount->ncount] = NULL;

  obtn->child = obj;
  obtn->ncount += 1;
  obtn->_raze_cb = _button_object_raze;
  obj->o_mount = obtn;

  return obj;
}

void
_default_button_raze(void *obj) {

  PhxObject *obtn = (PhxObject*)obj;
  PhxButtonStyle  style = obtn->type >> 8;
  if ( (style == BTN_COMBO_ARROW) || (style == BTN_COMBO_WHEEL) ) {
      /* Scan session for dropdown that matches actuator. */
    uint16_t idx = session->ncount;
    do {
      PhxInterface *inspect = session->iface[(--idx)];
      if (inspect->type == ((DDL_COMBO << 8) | PHX_IBANK)) {
        if (((PhxVault*)inspect->exclusive)->actuator == obtn) {
          inspect->_raze_cb((void*)inspect);
          break;
        }
      }
    } while (idx != 0);
  }

  free(obtn->attrib->font_name);
  free(obtn->attrib);
  obtn->attrib = NULL;               /* For debug */

  DEBUG_ASSERT((obtn == ui_active_drag_get()),
                     "failure: has_drag on object termination.");
  DEBUG_ASSERT((obtn == ui_active_within_get()),
                     "failure: within on object termination.");
  DEBUG_ASSERT((obtn == ui_active_focus_get()),
                     "failure: has_focus on object termination.");
  free(obtn);
}

PhxObjectButton *
ui_button_create(PhxNexus *nexus,
                 PhxButtonStyle style,
                 PhxRectangle configure) {

  PhxObjectButton *obtn;
  int16_t bfm;  /* Button Frame Margin */
  PhxDrawHandler  button_draw;
  PhxRGBA *c;

  if      (style == BTN_NAVIGATE_LEFT)
    button_draw = _draw_button_navigate_left;
  else if (style == BTN_NAVIGATE_RIGHT)
    button_draw = _draw_button_navigate_right;
  else if (style == BTN_COMBO_ARROW)
    button_draw = _draw_button_combo_arrow;
  else  /* default */
    button_draw = _draw_button_round_corner;

  bfm = (configure.h < 21) ? 2 : 3;
  obtn = ui_object_create(nexus, ((style << 8) | PHX_BUTTON),
                                          button_draw, configure);
  RECTANGLE(obtn->draw_box, bfm, bfm,
                            configure.w - (bfm << 1),
                            configure.h - (bfm << 1));
  obtn->_event_cb = _default_button_meter;
  obtn->_raze_cb  = _default_button_raze;
  c = &obtn->attrib->fg_fill;
  c->r = 0.94, c->g = 0.94, c->b = 0.94, c->a = 1;

  return obtn;
}
