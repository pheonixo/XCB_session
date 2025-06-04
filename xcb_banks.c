#include "nexus.h"
#include "buttons.h"
#include "banks.h"
#include "configure.h"

static void
result_cb(PhxBank *ibank) {
  char *was, *is;
  PhxNexus *nexus = ibank->nexus[0];
  PhxVault *vault = (PhxVault*)ibank->exclusive;
  PhxObjectLabel *olbl;

  olbl = (PhxObjectLabel*)nexus->objects[vault->was_idx];
  was = ((PhxLabelbuffer*)olbl->exclusive)->string;
  olbl = (PhxObjectLabel*)nexus->objects[vault->on_idx];
  is  = ((PhxLabelbuffer*)olbl->exclusive)->string;
  printf("from %s to %s\n", was, is);
}

static void
user_configure_layout(PhxInterface *iface) {

  PhxObjectButton *obtn;
  PhxButtonStyle style;
  PhxRectangle nexus_box, within_box;
  PhxNexus *nexus;
  PhxBank  *bank;
  PhxObject  *obj;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

  RECTANGLE(nexus_box, 25, 50, 250, 100);
  style = BTN_COMBO_ARROW;

  nexus = ui_nexus_create(iface, nexus_box);
  nexus->_draw_cb = NULL;
    /* Position of object within nexus */
  RECTANGLE(within_box, 0, 0, 250, 100);
  obtn = ui_button_create(nexus, style, within_box);
/* Test verifying no bank creation.
  ui_bank_create(DDL_COMBO, obtn, result_cb);
*/
/* Test with button content
  ui_button_add_label(obtn, "Button", (HJST_CTR | VJST_CTR));
  ui_bank_create(DDL_COMBO, obtn, result_cb);
*/
/* Test adding bank content */
  ui_button_label_create(obtn, "Button", (HJST_CTR | VJST_CTR));
  bank = ui_bank_create(obtn, DDL_COMBO, result_cb);
    /* to add an object:
      need bank for object attaching to nexus
      needs to append list, increases mete of nexus.
      display of bank allows larger than display,
        but actuator display a constant */

  nexus = bank->nexus[0];
  nexus_box = nexus->mete_box;
  nexus_box.y += nexus_box.h;
  obj = (PhxObject*)ui_button_label_create(obtn, "Button 2",
                                                (HJST_CTR | VJST_CTR));
  sensitive_set(obj, false);
  nexus_box.y += nexus_box.h;
  ui_button_label_create(obtn, "Button 3", (HJST_CTR | VJST_CTR));
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 800, 200, 300, 200 };

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
