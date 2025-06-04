#include "session.h"
#include "windows.h"

/* Name title of window, set baground colour. */
static void
user_configure_port_layout(PhxInterface *iface) {

  PhxRGBA *c;
  xcb_window_t window = iface->window;

    /* Currently only names as window id */
  ui_window_name(window);

    /* Override default's background colour. */
  c = &iface->attrib->bg_fill;
  c->r -= .5, c->g -= .5, c->b -= .5;
}

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 400, 400, 800, 200 };

#if DEBUG_EVENTS_ON
    /* turn off XCB_MOTION_NOTIFY reporting */
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
/*
  debug_flags &= ~((uint64_t)1 << XCB_PROPERTY_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_CONFIGURE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_EXPOSE);
  debug_flags &= ~((uint64_t)1 << XCB_VISIBILITY_NOTIFY);
*/
#endif

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);

    /* Since one window... instead of ui_interface_for() */
  user_configure_port_layout(session->iface[0]);

    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

  configure.x -= 200;
  configure.y -= 200;
  window = ui_window_create(configure);
  ui_window_name(window);
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
