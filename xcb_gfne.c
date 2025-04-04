#include "session.h"
#include "windows.h"
#include "nexus.h"
#include "gfuse.h"

static void
background_l2r_layout(PhxInterface *iface, int16_t offset) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  int ndx;

  for (ndx = 150; ndx <= 230; ndx += 40) {
    RECTANGLE(nexus_box, 100 + ndx, 20 + offset, 40, 200 - 40);
    nexus = ui_nexus_create(iface, nexus_box);
      /* Don't care about 'h' for test. */
      /* set x moves for top set, w moves for bottom set */
    if (offset < 200) {
        /* 'x' */
      nexus->state |= HXPD_LFT | VXPD_BTM;
      nexus->min_max.x = 100 + ndx;
    } else {
        /* 'w' */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 140 + ndx;
    }
  }
}

static void
background_r2l_layout(PhxInterface *iface, int16_t offset) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  int ndx;

  for (ndx = 280+325; ndx >= 200+325; ndx -= 40) {
    RECTANGLE(nexus_box, 100 + ndx, 20 + offset, 40, 200 - 40);
    nexus = ui_nexus_create(iface, nexus_box);
      /* Don't care about 'h' for test. */
      /* set x moves for top set, w moves for bottom set */
    if (offset < 200)
          nexus->state |= HXPD_LFT | VXPD_BTM;  /* 'x' */
    else  nexus->state |= HXPD_RGT | VXPD_BTM;  /* 'w' */
    nexus->min_max.w = 140+325 + ndx;
  }
}

static void
background_t2b_layout(PhxInterface *iface, int16_t offset) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  int ndx;

  for (ndx = 0; ndx <= 80; ndx += 40) {
    RECTANGLE(nexus_box, 90, ndx + offset, 200 - 40, 40);
    nexus = ui_nexus_create(iface, nexus_box);
      /* Don't care about 'w' for test. */
      /* set y moves for top set, h moves for bottom set */
    if (offset < 400)
          nexus->state |= HXPD_LFT | VXPD_TOP;  /* 'y' */
    else  nexus->state |= HXPD_RGT | VXPD_BTM;  /* 'h' */
    nexus->min_max.h = 40 + ndx + offset;
  }
}

/* NOTE: on last, views all 3, but colour stays same due to no
  redraw signal (depends on pull speed) on leave since unaltered. */
static void
background_b2t_layout(PhxInterface *iface, int16_t offset) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  int ndx;

  for (ndx = 80; ndx >= 0; ndx -= 40) {
    RECTANGLE(nexus_box, 90+255, ndx + offset, 200 - 40, 40);
    nexus = ui_nexus_create(iface, nexus_box);
     /* Don't care about 'w' for test. */
     /* set y moves for top set, h moves for bottom set */
    if (offset < 400)
          nexus->state |= HXPD_LFT | VXPD_TOP;  /* 'y' */
    else  nexus->state |= HXPD_RGT | VXPD_BTM;  /* 'h' */
    nexus->min_max.h = 40 + ndx + offset;
  }
}

/* Name window, use configure_test.c:case 23 as background. */
static void
user_configure_0_layout(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNFuse *fuse;
  int ndx;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

#if DEBUG_EVENTS_ON
    /* turn off XCB_MOTION_NOTIFY reporting */
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
#endif

                     /* Set up a background */
  ndx = 75;
  background_l2r_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 50 - GRIPSZ,         ndx - GRIPSZ,
                       180 + (GRIPSZ << 1), 200 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 300 + ndx;
                     /* Set up a background */
  ndx = 325;
  background_l2r_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 50 - GRIPSZ,         ndx - GRIPSZ,
                       180 + (GRIPSZ << 1), 200 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 300 + ndx;

                     /* Set up a background */
  ndx = 75;
  background_r2l_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 425 - GRIPSZ,        ndx - GRIPSZ,
                       180 + (GRIPSZ << 1), 200 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 300 + ndx;
                     /* Set up a background */
  ndx = 325;
  background_r2l_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 425 - GRIPSZ,        ndx - GRIPSZ,
                       180 + (GRIPSZ << 1), 200 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 300 + ndx;
}

static void
user_configure_1_layout(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNFuse *fuse;
  int ndx;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

                     /* Set up a background */
  ndx = 40;
  background_t2b_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 65 - GRIPSZ,         140 + ndx - GRIPSZ,
                       205 + (GRIPSZ << 1), 180 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 500;
                     /* Set up a background */
  ndx = 420;
  background_t2b_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 65 - GRIPSZ,         140 + ndx - GRIPSZ,
                       205 + (GRIPSZ << 1), 180 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 500;

                     /* Set up a background */
  ndx = 40;
  background_b2t_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 325 - GRIPSZ,        140 + ndx - GRIPSZ,
                       200 + (GRIPSZ << 1), 180 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 300 + ndx;
                     /* Set up a background */
  ndx = 420;
  background_b2t_layout(iface, ndx);
                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 325 - GRIPSZ,        140 + ndx - GRIPSZ,
                       200 + (GRIPSZ << 1), 180 + (GRIPSZ << 1));
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  fuse->min_max.w = 610;
  fuse->min_max.h = 300 + ndx;
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 150, 200, 800, 600 };

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
  user_configure_0_layout(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

  RECTANGLE(configure, 1000, 100, 600, 800);
    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of _interface_for() */
  user_configure_1_layout(session->iface[1]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
