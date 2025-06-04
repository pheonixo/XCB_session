#include "textviews.h"
#include "labels.h"

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
    puts("error: reading file xcb_textview.c");
  fclose(rh);

  return string;
}

#pragma mark *** Window 2 *** 

static void
nexus_configure_layout(PhxInterface *iface) {

  PhxObjectLabel *label;
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

#pragma mark *** Window 1 ***

static bool
textview_left_cb(PhxInterface *iface,
                 xcb_generic_event_t *nvt,
                 PhxObject *obj) {

  if ((nvt->response_type & (uint8_t)0x7F) == XCB_CONFIGURE_NOTIFY) {

    xcb_configure_notify_event_t *configure;
    int16_t wD, hD;

    configure = (xcb_configure_notify_event_t*)nvt;
    wD = configure->width  - (obj->mete_box.x + obj->mete_box.w);
    hD = configure->height - (obj->mete_box.y + obj->mete_box.h);
    if ((obj->state & HXPD_MSK) != HXPD_NIL) {
      if ((obj->state & HXPD_MSK) == HXPD_RGT) {
        if (configure->width >= 418) {
          wD = 0;
          if (obj->mete_box.w != 400)
            wD = 400 - obj->mete_box.w;
        }
        obj->mete_box.w += wD;
        obj->draw_box.w += wD;
      } else {
        if (configure->width > 418) {
          wD = -obj->mete_box.x;
        } else {
          if (((wD -= 18) + obj->mete_box.x) > 0)
            wD = obj->mete_box.x;
        }
        obj->mete_box.x += wD;
      }
    }
    obj->mete_box.h += hD;
    obj->draw_box.h += hD;
    if (obj->exclusive != NULL) {
      PhxTextbuffer *tbuf = (PhxTextbuffer*)obj->exclusive;
      if ((obj->state & VXPD_MSK) == VXPD_BTM)
        tbuf->bin.h += hD;
      else if ((obj->state & VXPD_MSK) == VXPD_TOP) {
        if ((tbuf->bin.y + hD) < 0) {
          hD += tbuf->bin.y, tbuf->bin.y = 0;
          tbuf->bin.h += hD;
        } else {
          tbuf->bin.y += hD;
        }
      }
    }
    ui_invalidate_object(obj);
    return true;
  }

  return _default_textview_meter(iface, nvt, obj);
}

static bool
label_cb(PhxInterface *iface,
         xcb_generic_event_t *nvt,
         PhxObject *obj) {

  if ((nvt->response_type & (uint8_t)0x7F) == XCB_CONFIGURE_NOTIFY) {

    xcb_configure_notify_event_t *configure;
    int16_t wD, hD;

    configure = (xcb_configure_notify_event_t*)nvt;

    wD = configure->width  - (obj->mete_box.x + obj->mete_box.w);
    hD = configure->height - (obj->mete_box.y + obj->mete_box.h);
    if ((obj->state & HXPD_MSK) != HXPD_NIL) {
      if ((obj->state & HXPD_MSK) == HXPD_RGT) {
        if (configure->width >= 418) {
          wD = 0;
          if (obj->mete_box.w != 18)
            wD = 18 - obj->mete_box.w;
        }
        obj->mete_box.w += wD;
        obj->draw_box.w += wD;
      } else {
        if (configure->width >= 418) {
          wD = 0;
          if ((obj->mete_box.x + obj->mete_box.w) != 418)
            wD = 418 - (obj->mete_box.x + obj->mete_box.w);
        }
        obj->mete_box.x += wD;
      }
    }
      /* Ignored VXPD, want this always for demo. */
    obj->mete_box.h += hD;
    obj->draw_box.h += hD;
    ui_invalidate_object(obj);
    return true;
  }

  return _default_label_meter(iface, nvt, obj);
}

/* Note: same as default, save wD/hD includes obj->mete_box.x/y */
static bool
textview_right_cb(PhxInterface *iface,
                  xcb_generic_event_t *nvt,
                  PhxObject *obj) {

  if ((nvt->response_type & (uint8_t)0x7F) == XCB_CONFIGURE_NOTIFY) {

    xcb_configure_notify_event_t *configure;
    int16_t wD, hD;

    configure = (xcb_configure_notify_event_t*)nvt;
    wD = configure->width  - (obj->mete_box.x + obj->mete_box.w);
    hD = configure->height - (obj->mete_box.y + obj->mete_box.h);
    obj->mete_box.w += wD;
    obj->draw_box.w += wD;
    obj->mete_box.h += hD;
    obj->draw_box.h += hD;

    if (obj->exclusive != NULL) {
      PhxTextbuffer *tbuf = (PhxTextbuffer*)obj->exclusive;
      if ((obj->state & HXPD_MSK) == HXPD_LFT) {
        if ((tbuf->bin.x + wD) > 0)
          wD = -tbuf->bin.x;
        tbuf->bin.x += wD;
        wD = -wD;
      }
      tbuf->bin.w += wD;
  
      if ((obj->state & VXPD_MSK) == VXPD_TOP) {
        if ((tbuf->bin.y + hD) < 0)
          hD = -tbuf->bin.y;
        tbuf->bin.y += hD;
        hD = -hD;
      }
      tbuf->bin.h += hD;
    }

    ui_invalidate_object(obj);
    return true;
  }
  return _default_textview_meter(iface, nvt, obj);
}

static void
object_configure_layout(PhxInterface *iface) {

  PhxObjectLabel *label;
  PhxObjectTextview *otxt;
  char *text;
  PhxRectangle nexus_box;
  PhxNexus *nexus;

    /* Currently only names as window id */
  xcb_window_t window = iface->window;
  ui_window_name(window);

  RECTANGLE(nexus_box, 0, 0, 818, 200);
  nexus = ui_nexus_create(iface, nexus_box);
    /* expand like normal window */
  nexus->state |= HXPD_RGT | VXPD_BTM;

  RECTANGLE(nexus_box, 0, 0, 400, 200);
  otxt = ui_textview_create(nexus, nexus_box);
    /* add draw margins */
  otxt->draw_box.x += 10;
  otxt->draw_box.y += 10;
  otxt->draw_box.w -= 20;
  otxt->draw_box.h -= 20;
  text = user_textstring();
  ui_textview_buffer_set(otxt, text, HJST_LFT);
    /* Bottom grows height, want seperator to move left, constant width. */
  otxt->state |= HXPD_LFT | VXPD_BTM;
  otxt->_event_cb = textview_left_cb;

  RECTANGLE(nexus_box, 400, 0, 18, 200);
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
  label->_event_cb = label_cb;

  RECTANGLE(nexus_box, 418, 0, 400, 200);
  otxt = ui_textview_create(nexus, nexus_box);
    /* no draw margins */
  ui_textview_buffer_set(otxt, text, HJST_LFT);
    /* expand like normal window */
  otxt->state |= HXPD_RGT | VXPD_BTM;
  otxt->_event_cb = textview_right_cb;
  free(text);
}

#pragma mark *** Main ***

int
main(int argc, char *argv[]) {

  xcb_window_t window;

    /* window size and position */
  PhxRectangle configure = { 350, 75, 818, 200 };

#if DEBUG_EVENTS_ON
    /* turn off XCB_MOTION_NOTIFY reporting */
  debug_flags &= ~((uint64_t)1 << XCB_MOTION_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_CONFIGURE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_EXPOSE);
  debug_flags &= ~((uint64_t)1 << XCB_VISIBILITY_NOTIFY);
/*
  debug_flags &= ~((uint64_t)1 << XCB_PROPERTY_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_ENTER_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_LEAVE_NOTIFY);
  debug_flags &= ~((uint64_t)1 << XCB_CLIENT_MESSAGE);
*/
#endif

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  nexus_configure_layout(session->iface[0]);
  /*object_configure_layout(session->iface[0]);*/
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

  configure.x = 375;
  configure.y = 230;
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Since one window... instead of ui_interface_for() */
  object_configure_layout(session->iface[1]);
  /*nexus_configure_layout(session->iface[1]);*/
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
