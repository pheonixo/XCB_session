#include "session.h"
#include "windows.h"
#include "nexus.h"

/* Name window, use configure_test.c:case 23. */
static void
user_configure_port_layout(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  int ndx, hdx;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

                     /* Set up display ports */

  RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;

  RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.h = 100;

    /* set as overprint, but pushed by HXPD_LFT */
  for (ndx = 0; ndx < 120; ndx += 40) {
    RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
    nexus = ui_nexus_create(iface, nexus_box);
      /* Set constant height, x moves */
    nexus->state |= HXPD_LFT | VXPD_BTM;
    nexus->min_max.h = 180;
    nexus->min_max.w = 140 + ndx;
  }

  for (hdx = 40; hdx < 130; hdx += 40) {
    RECTANGLE(nexus_box, 100 + ndx, hdx, 80, 40);
    nexus = ui_nexus_create(iface, nexus_box);
    nexus->state |= HXPD_RGT | VXPD_BTM;
    nexus->min_max.h = hdx + 40;
    nexus->min_max.w = 300;
  }

    /* set as overprint, but pushed by HXPD_LFT */
  for (ndx = 200; ndx < 300; ndx += 40) {
    RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
    nexus = ui_nexus_create(iface, nexus_box);
      /* Set constant height, x moves */
    nexus->state |= HXPD_LFT | VXPD_BTM;
    nexus->min_max.h = 180;
    nexus->min_max.w = 140 + ndx;
  }
  nexus->min_max.w = 100 + ndx;

  RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
  nexus = ui_nexus_create(iface, nexus_box);
    /* Set constant height, x moves */
  nexus->state |= HXPD_LFT | VXPD_BTM;
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 100, 200, 800, 200 };

#if DEBUG_EVENTS_ON
    /* turn off XCB_MOTION_NOTIFY reporting */
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_PROPERTY_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_CONFIGURE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_EXPOSE);
  debug_flags &= ~((uint64_t)1 << XCB_VISIBILITY_NOTIFY);
#endif

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);

    /* Since one window... instead of _interface_for() */
  user_configure_port_layout(session->iface[0]);

    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
