#include "textviews.h"

static char *
user_textstring(void) {

  char *string;
  size_t rdSz;
  long filesize;
  FILE *rh = fopen("./libctype/locale/rwSystems/numbers.c", "r");
  if (rh == NULL) {  puts("file not found"); return NULL;  }
  fseek(rh, 0 , SEEK_END);
  filesize = ftell(rh);
  fseek(rh, 0 , SEEK_SET);
  rdSz = (filesize + TGAP_ALLOC) & ~(TGAP_ALLOC - 1);
  string = malloc(rdSz);
  memset(&string[filesize], 0, (rdSz - filesize));
  if (fread(string, filesize, 1, rh) == 0)
    puts("error: reading file xcb_textview_drag.c");
  fclose(rh);

  return string;
}

#pragma mark *** Window 2 ***

static void
nexus_configure_layout(PhxInterface *iface) {

  PhxObjectTextview *otxt;
  char *text;
  PhxRectangle nexus_box;
  PhxNexus *nexus;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

  RECTANGLE(nexus_box, 0, 0, 400, 200);
  nexus = ui_nexus_create(iface, nexus_box);
    /* Bottom grows height, want seperator to move left, constant width. */
  nexus->state |= HXPD_LFT | VXPD_BTM;
  nexus->min_max.w = 400;
  otxt = ui_textview_create(nexus, nexus_box);
#if  0
  otxt->draw_box.x += 10;
  otxt->draw_box.y += 10;
  otxt->draw_box.w -= 20;
  otxt->draw_box.h -= 20;
#endif
  text = user_textstring();
  ui_textview_buffer_set(otxt, text, HJST_LFT);
  free(text);
}

#pragma mark *** Main ***

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 100, 200, 400, 200 };

#if DEBUG_EVENTS_ON
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
  nexus_configure_layout(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
