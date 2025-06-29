#include "../nexus.h"
#include "../buttons.h"
#include "../labels.h"

static void
_draw_test(PhxObject *b, cairo_t *cr) {

  PhxRectangle dbox;
  PhxObjectButton *button = (PhxObjectButton *)b;
  ui_draw_button(b, cr);

  _button_draw_area_request(button, &dbox);

  cairo_rectangle(cr, dbox.x, dbox.y, dbox.w, dbox.h);
  cairo_set_source_rgba(cr, .23, .36, .66, .4);
  cairo_fill(cr);
}

static void
user_configure_layout(PhxInterface *iface) {

  PhxObjectButton *obtn;
  PhxButtonStyle style;
  PhxRectangle nexus_box_base, within_box;
  PhxNexus *nexus;
  int ndx;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

                 /* Navigate type buttons */

  RECTANGLE(nexus_box_base, 175, 90, 20, 20);
  style = BTN_NAVIGATE_LEFT;
  for (ndx = 0; ndx < 10; ndx++) {
    nexus = ui_nexus_create(iface, nexus_box_base);
    nexus->_draw_cb = NULL;
      /* Position of object within nexus */
    RECTANGLE(within_box, 0, 0, 20, 20);
    obtn = ui_button_create(nexus, style, within_box);
    if ((ndx & 2) == 0) {
      PhxRGBA *c = &obtn->attrib->fg_ink;
      c->r = 0, c->g = .2, c->b = .4, c->a = 1;
    }
       /* Upate next nexus positioning */
    nexus_box_base.x += 20;
    if (style == BTN_NAVIGATE_LEFT)
          style = BTN_NAVIGATE_RIGHT;
    else  style = BTN_NAVIGATE_LEFT;
  }

                 /* Combo type buttons */

  RECTANGLE(nexus_box_base, 175, 70, 50, 20);
  style = BTN_COMBO_ARROW;

  for (ndx = 0; ndx < 10; ndx++) {
    nexus = ui_nexus_create(iface, nexus_box_base);
    nexus->_draw_cb = NULL;
      /* Position of object within nexus */
    RECTANGLE(within_box, 0, 0, 50, 20);
    obtn = ui_button_create(nexus, style, within_box);
      /* Add label to third one */
    if (ndx == 0)
    ui_button_label_create(obtn, "Button", (HJST_CTR | VJST_CTR));

    if (ndx == 0)
      obtn->_draw_cb = _draw_test;
    else
         /* Upate next nexus positioning */
      if ((ndx & 1) == 0) {
        PhxRGBA *c = &obtn->attrib->fg_fill;
        c->r = 0.94, c->g = 0.4, c->b = 0.94, c->a = 1;
      }
    if (ndx == 5)
      obtn->_event_cb = NULL;
    nexus_box_base.x += 50;
  }

  RECTANGLE(nexus_box_base, 175, 110, 50, 20);
  style = BTN_ROUND_CORNER;

  for (ndx = 0; ndx < 10; ndx++) {
    nexus = ui_nexus_create(iface, nexus_box_base);
    nexus->_draw_cb = NULL;
      /* Position of object within nexus */
    RECTANGLE(within_box, 0, 0, 50, 20);
    obtn = ui_button_create(nexus, style, within_box);
      /* Add label to first one */
    if (ndx == 0)
      ui_button_label_create(obtn, "Button", (HJST_CTR | VJST_CTR));
    if ((ndx & 1) != 0) {
      PhxRGBA *c = &obtn->attrib->fg_fill;
      c->r = 0.2, c->g = 0.94, c->b = 0.94, c->a = 1;
    }

if (ndx == 8) {
  PhxObject *obj;
  obj = ui_button_object_create(obtn, PHX_DRAWING, ui_draw_right_arrow);
  RECTANGLE(obj->draw_box, (obj->mete_box.w / 2) - (obj->mete_box.h / 2),
                           0, obj->mete_box.h, obj->mete_box.h);
}

if (ndx == 9) {
  PhxObject *obj;
#if 1
  obj = ui_button_object_create(obtn, PHX_DRAWING, ui_draw_horizontal_line);
  RECTANGLE(obj->draw_box, 2, (obj->mete_box.h / 2) - 1,
                           obj->mete_box.w - 4, 2);
  obj->attrib->stroke = 2;
#else

/*
   Initial code designing above 'ui_button_object_create()'
*/

  PhxRectangle mbox;
  int16_t margin, hctr;

  _button_draw_area_request(obtn, &mbox);
  margin = mbox.h / 4;
  mbox.x += margin, mbox.w -= margin * 2;
  mbox.y += margin, mbox.h -= margin * 2;
  obj = ui_object_create(nexus, PHX_DRAWING, ui_draw_horizontal_line, mbox);
  obj->attrib->bg_fill.a = 0;
  obj->attrib->fg_fill.a = 0;
  hctr = mbox.h / 2;
  RECTANGLE(obj->draw_box, 2, hctr - 1, mbox.w - 4, 2);
  obj->attrib->stroke = 2;

  nexus->ncount -= 1;
  nexus->objects[nexus->ncount] = NULL;

  obtn->child = (PhxObject*)obj;
  obtn->ncount += 1;
  obtn->_raze_cb = _default_object_raze;
  obj->o_mount = obtn;
#endif
}
       /* Upate next nexus positioning */
    nexus_box_base.x += 50;
  }

                 /* Round corner button with label */

  RECTANGLE(nexus_box_base, 20, 20, 150, 80);
  style = BTN_ROUND_CORNER;

  nexus = ui_nexus_create(iface, nexus_box_base);
  nexus->_draw_cb = NULL;
    /* Position of object within nexus */
  RECTANGLE(within_box, 0, 0, 150, 80);
  obtn = ui_button_create(nexus, style, within_box);
    /* Test of draw_box retreival */
  obtn->_draw_cb = _draw_test;
    /* frame_draw_set(obtn, false); */
  ui_button_label_create(obtn, "Button", HJST_CTR);

                 /* Combo button with label */

  RECTANGLE(nexus_box_base, 20, 110, 150, 80);
  style = BTN_COMBO_ARROW;

  nexus = ui_nexus_create(iface, nexus_box_base);
  nexus->_draw_cb = NULL;
    /* Position of object within nexus */
  RECTANGLE(within_box, 0, 0, 150, 80);
  obtn = ui_button_create(nexus, style, within_box);
    /* Test of draw_box retreival */
  obtn->_draw_cb = _draw_test;
    /* frame_draw_set(obtn, false); */
  ui_button_label_create(obtn, "Button", HJST_CTR);
  _label_text_max_fit((PhxObjectLabel*)obtn->child);
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 100, 200, 800, 200 };

#if DEBUG_EVENTS_ON
    /* turn off reporting of these events */
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_CONFIGURE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_EXPOSE);
  debug_flags &= ~((uint64_t)1 << XCB_KEY_PRESS);
  debug_flags &= ~((uint64_t)1 << XCB_KEY_RELEASE);
  debug_flags &= ~((uint64_t)1 << XCB_BUTTON_PRESS);
  debug_flags &= ~((uint64_t)1 << XCB_BUTTON_RELEASE);
  debug_flags &= ~((uint64_t)1 << XCB_ENTER_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_LEAVE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_FOCUS_IN);
  debug_flags &= ~((uint64_t)1 << XCB_FOCUS_OUT);
  debug_flags &= ~((uint64_t)1 << XCB_VISIBILITY_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_PROPERTY_NOTIFY);
#endif

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  user_configure_layout(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
