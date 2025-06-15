
# XCB_session

GUI and Application creator for Linux.

## Requires

X11/xcb  -lxcb -lxcb-shm -lxcb-render\
Cairo    -lcairo\
Freetype -lfreetype

## Usage

```C
int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 100, 100, 500, 500 };

#if DEBUG_EVENTS_ON
    /* Shut off events you don't want reported. */
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
#endif

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  user_configure_layout(session->iface[0]);
    /* Extra window configure adds. */
  user_add_text(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
```
## Notes

Development state. Originally designed for creating 'widgets' or objects that you couldn't with toolkits. Turned into an application designer once DND, clipboard and other features added. Hopefully DND will become an XDNDServer extension for basic Linux functionality.

## Features
Current state is a multi-window application with DND, clipboard, dropdown windows for combo buttons, textviews, findbar, and creation of headerbars.

The findbar is what started this project, since Linux was without one, and the toolkit refused to allow. Text is supplimented with full UTF-8 with proper or 'different' than other. It uses simple key combos of ^e for enter of selected as search string, ^E for enter of selected as replacement string and ^f for find. Both are editable and scrollable. Options of goto previous, next, replace, replace with moving to next, and replace all. Dialog popup still needed for directory searches and when textview being searched is too small for good viewable findbar.

## Examples
Design/creations are located in forge. Currently all 'xcb_*'.c files are design and workable applications. The makefile can build all or look into to see individual builds and names for each. These are examples of creation of applications or individuals pieces of object design prior to incorporation into XCB_session. The TODO has some notes on concepts.

## Concept
Create a window or interface. Add drawing ports or nexus to interface. Add objects to drawing ports. Objects are intended to respond to events. There are 3 basic callbacks to an object: draw, all others, and raze(destroy). Any window will quit with ^q. This is also intended for the long haul so binary compat is of major import.