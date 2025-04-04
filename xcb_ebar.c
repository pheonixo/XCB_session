#include "nexus.h"
#include "buttons.h"
#include "banks.h"
#include "configure.h"

static char *strings[] = {
  "editor_bar.c", "editor_bar.h", "lci_sessions.c",
  "parser.c", "lci_parser_C.c", "lci_symbols.c", NULL
};

static void
result_cb(PhxBank *ibank) {

  char *was, *is;
  PhxNexus *nexus = ibank->nexus[0];
  PhxVault *vault = (PhxVault*)ibank->exclusive;
  PhxObjectLabel *olbl;

  olbl = (PhxObjectLabel*)nexus->objects[vault->was_idx];
  if (olbl->type == PHX_DRAWING)
    was = "object";
  else
    was = ((PhxLabelbuffer*)olbl->exclusive)->string;
  olbl = (PhxObjectLabel*)nexus->objects[vault->on_idx];
  if (olbl->type == PHX_DRAWING)
    is = "object";
  else
    is  = ((PhxLabelbuffer*)olbl->exclusive)->string;
  printf("from %s to %s\n", was, is);
}

static bool
btn_delete_file(PhxInterface *iface,
                xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse = (xcb_button_press_event_t*)nvt;
  if ((mouse->response_type & (uint8_t)0x7F) == (uint8_t)XCB_BUTTON_RELEASE) {

    PhxNexus *nexus = (PhxNexus*)obj->i_mount;
    PhxObject *actuator = nexus->objects[2];
    PhxBank *ibank = ui_dropdown_from(actuator);
    PhxVault *vault = (PhxVault*)ibank->exclusive;
    ui_bank_remove_object(ibank, vault->on_idx);
  }
  return _default_button_meter(iface, nvt, obj);
}

static bool
btn_navigate_left(PhxInterface *iface,
                  xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse = (xcb_button_press_event_t*)nvt;
  if ((mouse->response_type & (uint8_t)0x7F) == (uint8_t)XCB_BUTTON_RELEASE) {

    PhxNexus *nexus = (PhxNexus*)obj->i_mount;
    PhxObject *actuator = nexus->objects[2];
    PhxBank *ibank = ui_dropdown_from(actuator);
    PhxVault *vault = (PhxVault*)ibank->exclusive;
    uint16_t err = vault->on_idx;

    nexus = ibank->nexus[0];
    vault->was_idx = vault->on_idx;

    if (vault->on_idx == (uint16_t)~0)
      return _default_button_meter(iface, nvt, obj);

    if (vault->on_idx != 0) {
      do {
        PhxObject *inspect = nexus->objects[(--vault->on_idx)];
        if (sensitive_get(inspect)) {
          ui_actuator_content_update(ibank);
          result_cb(ibank);
          ui_invalidate_object(actuator);
          return _default_button_meter(iface, nvt, obj);
        }
      } while (vault->on_idx != 0);
    }
    vault->on_idx = nexus->ncount;
    do {
      PhxObject *inspect = nexus->objects[(--vault->on_idx)];
      if (sensitive_get(inspect)) {
        ui_actuator_content_update(ibank);
        result_cb(ibank);
        ui_invalidate_object(actuator);
        return _default_button_meter(iface, nvt, obj);
      }
    } while (vault->on_idx != 0);
    vault->on_idx = err;
  }
  return _default_button_meter(iface, nvt, obj);
}

static bool
btn_navigate_right(PhxInterface *iface,
                   xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse = (xcb_button_press_event_t*)nvt;
  if ((mouse->response_type & (uint8_t)0x7F) == (uint8_t)XCB_BUTTON_RELEASE) {

    PhxNexus *nexus = (PhxNexus*)obj->i_mount;
    PhxObject *actuator = nexus->objects[2];
    PhxBank *ibank = ui_dropdown_from(actuator);
    PhxVault *vault = (PhxVault*)ibank->exclusive;
    uint16_t err = vault->on_idx;

    nexus = ibank->nexus[0];
    vault->was_idx = vault->on_idx;

    if (vault->on_idx == (uint16_t)~0)
      return _default_button_meter(iface, nvt, obj);

    if ((vault->on_idx + 1) < nexus->ncount) {
      do {
        PhxObject *inspect = nexus->objects[(++vault->on_idx)];
        if (sensitive_get(inspect)) {
          ui_actuator_content_update(ibank);
          result_cb(ibank);
          ui_invalidate_object(actuator);
          return _default_button_meter(iface, nvt, obj);
        }
      } while ((vault->on_idx + 1) < nexus->ncount);
    }
    vault->on_idx = ~0;
    do {
      PhxObject *inspect = nexus->objects[(++vault->on_idx)];
      if (sensitive_get(inspect)) {
        ui_actuator_content_update(ibank);
        result_cb(ibank);
        ui_invalidate_object(actuator);
        return _default_button_meter(iface, nvt, obj);
      }
    } while ((vault->on_idx + 1) < nexus->ncount);
    vault->on_idx = err;
  }

  return _default_button_meter(iface, nvt, obj);
}

static bool
nexus_events(PhxInterface *iface,
             xcb_generic_event_t *nvt, PhxObject *obj) {

  int16_t wD, grp_sz;
  PhxNexus *nexus;
  xcb_configure_notify_event_t *configure
    = (xcb_configure_notify_event_t*)nvt;
  if ((configure->response_type & (uint8_t)0x7F)
                               != (uint8_t)XCB_CONFIGURE_NOTIFY)
    return _default_nexus_meter(iface, nvt, obj);

    /* Make use of sequential non-overlap object listing. */
  nexus = (PhxNexus*)obj;
  grp_sz = nexus->objects[(nexus->ncount - 1)]->mete_box.x
          + nexus->objects[(nexus->ncount - 1)]->mete_box.w
          - nexus->objects[0]->mete_box.x;
  wD = (obj->mete_box.w - grp_sz) / 2;
  wD -= nexus->objects[0]->mete_box.x;
  grp_sz = nexus->ncount;
  do
    nexus->objects[(--grp_sz)]->mete_box.x += wD;
  while (grp_sz != 0);

  wD = configure->width - nexus->mete_box.w;
  nexus->mete_box.w += wD;
  nexus->draw_box.w += wD;

  return true;
}

static void
_draw_scroll_up(PhxObject *b, cairo_t *cr) {

  double x, y, w, h, sp;
  PhxAttr *c = b->attrib;
  PhxRectangle dbox;

  x = b->mete_box.x;
  y = b->mete_box.y;

  cairo_rectangle(cr, x, y, b->mete_box.w, b->mete_box.h);
  cairo_clip_preserve(cr);

  cairo_set_source_rgba(cr, c->bg_fill.r,
                            c->bg_fill.g,
                            c->bg_fill.b,
                            c->bg_fill.a);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, c->fg_ink.r,
                            c->fg_ink.g,
                            c->fg_ink.b,
                            c->fg_ink.a);

  cairo_rectangle(cr, 0, b->draw_box.h - 0.5,
                      b->draw_box.w, c->stroke);
  cairo_fill(cr);

  RECTANGLE(dbox, (b->mete_box.w / 2) - (b->mete_box.h / 2),
                           0, b->mete_box.h, b->mete_box.h);

  x += dbox.x + 0.5;
  y += dbox.y + 0.5;
  w  = dbox.w - 1.0;
  h  = dbox.h - 1.0;
  sp = h / 5;

  cairo_move_to(cr, (x + w - sp + 0.5), (y + h - sp));
  cairo_line_to(cr,  x + sp + 0.5,      (y + h - sp));
  cairo_line_to(cr, (x + (w / 2)),      (y + sp));
  cairo_line_to(cr, (x + w - sp + 0.5), (y + h - sp));
  cairo_fill(cr);

}

static int16_t
_configure_bar(PhxNexus *nexus, bool frame, bool max_text) {

  PhxObjectButton *obtn;
  PhxButtonStyle style;
  PhxRectangle within_box;
  PhxObject *obj;
  uint16_t sdx = 0;
  int16_t grp_sz, xD;

  RECTANGLE(within_box, 0, 0, 28, 28);
  style = BTN_NAVIGATE_LEFT;
  obtn = ui_button_create(nexus, style, within_box);
  if (!frame) {
    frame_remove(obtn);
    frame_draw_set(obtn, false);
  }
  obtn->_event_cb = btn_navigate_left;

  RECTANGLE(within_box, (28 + 1), 0, 1, 28);
  obj = ui_object_create(nexus, PHX_DRAWING, ui_draw_vertical_line, within_box);
  sensitive_set(obj, false);

  RECTANGLE(within_box, (28 + 1 + 1), 0, 150, 28);
  style = BTN_COMBO_ARROW;
  obtn = ui_button_create(nexus, style, within_box);
  if (!frame) {
    frame_remove(obtn);
    frame_draw_set(obtn, false);
  }

#define DESIGN 0

#if !DESIGN
  obj = ui_button_object_create(obtn, PHX_DRAWING, _draw_scroll_up);
  obj->mete_box.h /= 2;
  obj->draw_box.h /= 2;
  sensitive_set(obj, false);
  /*sensitive_set(obtn->child, false);*/

  for (sdx = 0; sdx < 2; sdx++) {
    obj = ui_button_label_create(obtn, strings[sdx], (HJST_CTR | VJST_CTR));
    if (max_text)  _label_text_max_fit(obj);
  }

  obj = ui_button_object_create(obtn, PHX_DRAWING, ui_draw_horizontal_line);
  RECTANGLE(obj->draw_box, 2, (obj->mete_box.h / 2) - 1,
                           obj->mete_box.w - 4, 2);
  obj->attrib->stroke = 2;
  sensitive_set(obj, false);

  for (; strings[sdx] != NULL; sdx++) {
    obj = ui_button_label_create(obtn, strings[sdx], (HJST_CTR | VJST_CTR));
    if (max_text)  _label_text_max_fit(obj);
  }

  ui_bank_add_result_cb(obtn, result_cb);
#else
  {
  PhxNexus *bank_nexus;
  PhxBank  *bank;
  ui_button_label_create(obtn, strings[sdx], (HJST_CTR | VJST_CTR));

  _button_draw_area_request(obtn, &within_box);
  within_box.x = (within_box.y = 0);
  bank = ui_bank_create(obtn, DDL_COMBO, within_box, result_cb);

  bank_nexus = bank->nexus[0];
  within_box = bank_nexus->mete_box;
  within_box.y += within_box.h;
  ui_label_create(bank_nexus, within_box,
                    strings[(++sdx)], (HJST_CTR | VJST_CTR));
  within_box.y += within_box.h;
  obj = ui_button_object_create(obtn, PHX_DRAWING, ui_draw_horizontal_line);
  RECTANGLE(obj->draw_box, 2, (obj->mete_box.h / 2) - 1,
                           obj->mete_box.w - 4, 2);
  obj->attrib->stroke = 2;
  for ((++sdx); strings[sdx] != NULL; sdx++) {
    within_box.y += within_box.h;
    ui_label_create(bank_nexus, within_box,
                      strings[sdx], (HJST_CTR | VJST_CTR));
  }
  }
#endif

  RECTANGLE(within_box, (28 + 1 + 1 + 150 + 1), 0, 1, 28);
  obj = ui_object_create(nexus, PHX_DRAWING, ui_draw_vertical_line, within_box);
  sensitive_set(obj, false);

  RECTANGLE(within_box, (28 + 1 + 1 + 150 + 1 + 1), 0, 28, 28);
  style = BTN_NAVIGATE_RIGHT;
  obtn = ui_button_create(nexus, style, within_box);
  if (!frame) {
    frame_remove(obtn);
    frame_draw_set(obtn, false);
  }
  obtn->_event_cb = btn_navigate_right;


    /* Add 'delete' button to simulate removal of object from list.
      Tack on as part of grouped objects. */
  RECTANGLE(within_box, (28 + 1 + 1 + 150 + 1 + 1 + 28) + 10, 0, 75, 28);
  style = BTN_ROUND_CORNER;
  obtn = ui_button_create(nexus, style, within_box);
  ui_button_label_create(obtn, "Delete", (HJST_CTR | VJST_CTR));
  if (!frame) {
    frame_remove(obtn);
    frame_draw_set(obtn, false);
  }
  obtn->_event_cb = btn_delete_file;

    /* For this demo, restrict window size to above, treat as unit. */
  grp_sz = nexus->objects[(nexus->ncount - 1)]->mete_box.x
          + nexus->objects[(nexus->ncount - 1)]->mete_box.w
          - nexus->objects[0]->mete_box.x;

    /* Center group. */
  xD = (600 - grp_sz) / 2;
  sdx = nexus->ncount;
  do
    nexus->objects[(--sdx)]->mete_box.x += xD;
  while (sdx != 0);

  return grp_sz;
}

static void
user_configure_layout(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  int16_t grp_sz;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

  RECTANGLE(nexus_box, 0, 0, 600, 28);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state = HXPD_RGT;
  nexus->_draw_cb = NULL;
  nexus->_event_cb = nexus_events;
  nexus->attrib->bg_fill.r = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.g = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.b = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.a = 1.0;

  grp_sz = _configure_bar(nexus, false, false);
  ui_window_minimum_set(window, grp_sz, 112);

  RECTANGLE(nexus_box, 0, 28, 600, 28);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state = HXPD_RGT;
  nexus->_draw_cb = NULL;
  nexus->_event_cb = nexus_events;
  nexus->attrib->bg_fill.r = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.g = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.b = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.a = 1.0;

  _configure_bar(nexus, true, false);

  RECTANGLE(nexus_box, 0, 56, 600, 28);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state = HXPD_RGT;
  nexus->_draw_cb = NULL;
  nexus->_event_cb = nexus_events;
  nexus->attrib->bg_fill.r = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.g = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.b = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.a = 1.0;

  _configure_bar(nexus, false, true);

  RECTANGLE(nexus_box, 0, 84, 600, 28);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state = HXPD_RGT;
  nexus->_draw_cb = NULL;
  nexus->_event_cb = nexus_events;
  nexus->attrib->bg_fill.r = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.g = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.b = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.a = 1.0;

  _configure_bar(nexus, true, true);
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 800, 200, 600, 112 };

#if DEBUG_EVENTS_ON
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_PROPERTY_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_CONFIGURE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_ENTER_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_LEAVE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_FOCUS_IN);
  debug_flags &= ~((uint64_t)1 << XCB_FOCUS_OUT);
#endif

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of _interface_for() */
  user_configure_layout(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
