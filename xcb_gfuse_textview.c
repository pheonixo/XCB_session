#include "textviews.h"
#include "labels.h"
#include "gfuse.h"

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
    puts("error: reading file xcb_gfuse_textview.c");
  fclose(rh);

  return string;
}

#pragma mark *** Configure ***

static void
nexus_configure_layout(PhxInterface *iface) {

  PhxObjectLabel *label;
  PhxObjectTextview *otxt;
  char *text;
  PhxRectangle nexus_box;
  PhxNexus *nexus;

  RECTANGLE(nexus_box, 0, 0, 400, 200);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.w = 400;
  RECTANGLE(nexus_box, 0, 0, 400, 200);
  otxt = ui_textview_create(nexus, nexus_box);
  otxt->draw_box.x += 10;
  otxt->draw_box.y += 10;
  otxt->draw_box.w -= 20;
  otxt->draw_box.h -= 20;
  text = user_textstring();
  ui_textview_buffer_set(otxt, text, HJST_LFT);

  RECTANGLE(nexus_box, 400, 0, 18, 200);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_LFT | VXPD_BTM;
  nexus->min_max.w = 418;
  RECTANGLE(nexus_box, 0, 0, 18, 200);
  label = ui_label_create(nexus, nexus_box,
                           "S\nE\nP\nA\nR\nA\nT\nO\nR", (HJST_CTR | VJST_CTR));
  label->attrib->bg_fill.a = 1.0;
  label->attrib->fg_ink = label->attrib->bg_fill;
  label->attrib->fg_fill.r = 0.5;
  label->attrib->fg_fill.g = 0.5;
  label->attrib->fg_fill.b = 0.5;
  label->attrib->fg_fill.a = 0.5;
  ui_label_font_em_set(label, 16);
    /* Bottom grows height, want seperator to move left, constant width. */
  label->state |= HXPD_LFT | VXPD_BTM;

  RECTANGLE(nexus_box, 418, 0, 400, 200);
  nexus = ui_nexus_create(iface, nexus_box);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->min_max.x = 418;
  RECTANGLE(nexus_box, 0, 0, 400, 200);
  otxt = ui_textview_create(nexus, nexus_box);
  ui_textview_buffer_set(otxt, text, HJST_LFT);
  free(text);
}

static void
gfuse_configure_layout(PhxInterface *iface) {

  PhxRectangle nexus_box;
  PhxNFuse *fuse;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

                     /* Set up a gfuse */
  RECTANGLE(nexus_box, 100 - GRIPSZ,        44 - GRIPSZ,
                       818 + (GRIPSZ << 1), 200 + (GRIPSZ << 1));
    /* returns user area, 0,0 referenced */
  fuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_CENTER);
  nexus_configure_layout((PhxInterface*)fuse);

}

#pragma mark *** Main ***

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 900, 75, 1000, 400 };

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
  gfuse_configure_layout(session->iface[0]);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
