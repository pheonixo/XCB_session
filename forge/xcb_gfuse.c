#include "../session.h"
#include "../windows.h"
#include "../nexus.h"
#include "../nfuse.h"
#include "../gfuse.h"

/* Name window, use configure_test.c:case 23 as background. */
static void
user_configure_layout(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNFuse *fuse, *subfuse, *subsubfuse;
  int ndx;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 220 - GRIPSZ,        150 - GRIPSZ,
                       360 + (GRIPSZ << 1), 300 + (GRIPSZ << 1));
    /* returns user area, 0,0 referenced */
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
    /* lock grip 'w', 'h', 50 each */
  subfuse = (PhxNFuse*)fuse->nexus[0];
  subfuse->min_max.w = 630;
  subfuse->min_max.h = 500;

          /* normal unit area, behaves as inner window */
  RECTANGLE(nexus_box, 0, 150, 360 - 40, 150);
  nexus = ui_nexus_create((PhxInterface*)subfuse, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_TOP;

  RECTANGLE(nexus_box, 0, 0, 360 - 40, 150);
  nexus = ui_nexus_create((PhxInterface*)subfuse, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.h = 150;

      /* creating nfuse instead of just nexus */
    RECTANGLE(nexus_box, 40, 40, 80, 300 - 80);
    subsubfuse = ui_nfuse_create((PhxInterface*)subfuse, nexus_box);
    subsubfuse->min_max.w = 40 + 2*40;
    subsubfuse->min_max.h = 300 - 40;
        /* internal nexus */
      for (ndx = 0; ndx < 80; ndx += 40) {
        RECTANGLE(nexus_box, ndx, 0, 40, 300 - 80);
        nexus = ui_nexus_create((PhxInterface*)subsubfuse, nexus_box);
        nexus->min_max.w = 40 + ndx;
      }

      /* creating nfuse instead of just nexus */
    RECTANGLE(nexus_box, 40 + 2*40, 90, 80, 120);
    subsubfuse = ui_nfuse_create((PhxInterface*)subfuse, nexus_box);
    subsubfuse->state &= ~(HXPD_MSK | VXPD_MSK);
    subsubfuse->state |= HXPD_LFT | VXPD_TOP;
    subsubfuse->min_max.w = 40+2*40 + 80;
    subsubfuse->min_max.h = 90 + 120;
        /* internal nexus */
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, 0, ndx, 80, 40);
        nexus = ui_nexus_create((PhxInterface*)subsubfuse, nexus_box);
        nexus->state |= HXPD_LFT | VXPD_TOP;
      }

      /* creating nfuse instead of just nexus */
    RECTANGLE(nexus_box, 40+2*40+80, 40, 80, 300 - 80);
    subsubfuse = ui_nfuse_create((PhxInterface*)subfuse, nexus_box);
    subsubfuse->min_max.w = 40+2*40+80 + 80;
    subsubfuse->min_max.h = 300 - 40;
        /* internal nexus */
      for (ndx = 0; ndx < 80; ndx += 40) {
        RECTANGLE(nexus_box,ndx, 0, 40, 300 - 80);
        nexus = ui_nexus_create((PhxInterface*)subsubfuse, nexus_box);
        nexus->min_max.w = 40 + ndx;
      }

  RECTANGLE(nexus_box, 360 - 40, 0, 40, 300);
  nexus = ui_nexus_create((PhxInterface*)subfuse, nexus_box);
  nexus->state |= HXPD_LFT | VXPD_BTM;
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 200, 200, 800, 600 };

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
