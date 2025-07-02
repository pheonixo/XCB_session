/*
 * resize gfuse issues
 * Steven J Abner 2025
 */
#include "../windows.h"
#include "../gfuse.h"


#pragma mark *** Creation ***

static void
user_configure_layout_NS_bt(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNFuse *nfuse;
  uint16_t quad_height = iface->mete_box.h / 4;

    /* Want ports to overlay with farthest to headerbar being topmost */
  RECTANGLE(nexus_box, 0, 0, iface->mete_box.w,
                             iface->mete_box.h - (2*quad_height));
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.h = iface->mete_box.h - quad_height;


  RECTANGLE(nexus_box, 0, iface->mete_box.h - (2*quad_height),
                          iface->mete_box.w,
                          quad_height);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_NORTH);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.y = 0;

  RECTANGLE(nexus_box, 0, iface->mete_box.h - quad_height,
                          iface->mete_box.w,
                          quad_height);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_NORTH);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.y = GRIPSZ;

  ui_window_minimum_set(iface->window, 1, 2*GRIPSZ);
}

static void
user_configure_layout_NS_tb(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNFuse *nfuse;
  uint16_t quad_height = iface->mete_box.h / 4;

    /* Want ports to overlay with closest to headerbar being topmost */
  RECTANGLE(nexus_box, 0, iface->mete_box.h - quad_height,
                          iface->mete_box.w,
                          quad_height);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_NORTH);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.y = GRIPSZ;

  RECTANGLE(nexus_box, 0, iface->mete_box.h - (2*quad_height),
                          iface->mete_box.w,
                          quad_height);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_NORTH);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.y = 0;

  RECTANGLE(nexus_box, 0, 0, iface->mete_box.w,
                             iface->mete_box.h - (2*quad_height));
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.h = iface->mete_box.h - quad_height;

  ui_window_minimum_set(iface->window, 1, 2*GRIPSZ);
}

/* rightmost is the topmost */
static void
user_configure_layout_EW_rl(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNFuse *nfuse;
  uint16_t quad_width = iface->mete_box.w / 4;

    /* Want ports to overlay with rightmost being topmost */
  RECTANGLE(nexus_box, 0, 0, iface->mete_box.w - (2*quad_width),
                             iface->mete_box.h);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.w = iface->mete_box.w - quad_width;

  RECTANGLE(nexus_box, iface->mete_box.w - (2*quad_width), 0,
                          quad_width,
                          iface->mete_box.h);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_WEST);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.x = 0;

  RECTANGLE(nexus_box, iface->mete_box.w - quad_width, 0,
                          quad_width,
                          iface->mete_box.h);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_WEST);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.x = GRIPSZ;

  ui_window_minimum_set(iface->window, 2*GRIPSZ, 1);
}

/* leftmost is the topmost */
static void
user_configure_layout_EW_lr(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNFuse *nfuse;
  uint16_t quad_width = iface->mete_box.w / 4;

    /* Want ports to overlay with leftmost being topmost */
  RECTANGLE(nexus_box, iface->mete_box.w - quad_width, 0,
                          quad_width,
                          iface->mete_box.h);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_WEST);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.x = GRIPSZ;

  RECTANGLE(nexus_box, iface->mete_box.w - (2*quad_width), 0,
                          quad_width,
                          iface->mete_box.h);
  nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_WEST);
  nfuse->state |= HXPD_RGT | VXPD_BTM;
  nfuse->min_max.x = 0;

  RECTANGLE(nexus_box, 0, 0, iface->mete_box.w - (2*quad_width),
                             iface->mete_box.h);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.w = iface->mete_box.w - quad_width;

  ui_window_minimum_set(iface->window, 2*GRIPSZ, 1);
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 800, 100, 400, 400 };

#if DEBUG_EVENTS_ON
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

#if 0
    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  user_configure_layout_NS_bt(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);
#endif

#if 0
  configure.x -= 500;
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  user_configure_layout_EW_rl(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);
#endif

#if 0
  configure.y += 450;
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  user_configure_layout_EW_lr(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);
#endif

#if 1
  configure.x += 500;
    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  user_configure_layout_NS_tb(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);
#endif

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
