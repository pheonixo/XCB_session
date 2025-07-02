/*
 * resize gfuse issues
 * Steven J Abner 2025
 */
#include "../windows.h"
#include "../gfuse.h"
#include "../draw.h"

static uint8_t layout_counter = 0;
static void  user_configure_layout(PhxInterface *iface, uint8_t layout);

#pragma mark *** Events ***

static bool
_cfg_interface_meter(PhxInterface *iface,
                     xcb_generic_event_t *nvt,
                     PhxObject *obj) {

  uint8_t response = nvt->response_type & (uint8_t)0x7F;
  if ( (response == XCB_KEY_PRESS)
      || (response == XCB_KEY_RELEASE) ) {

    xcb_key_press_event_t *kp;
    xcb_keysym_t keyval;
    kp = (xcb_key_press_event_t*)nvt;
    keyval = _xcb_keysym_for(kp->detail, kp->state);

    if (keyval == 0xffbf) {  /* F2 key */
      if (response == XCB_KEY_RELEASE) {
        bool locus;
        xcb_window_t window = kp->event;

          /* Don't know where we'll end up, but current
            will be destroyed. */
        ui_active_within_set(NULL, 0, 0, 0);

        if (iface->ncount != 0) {
          uint16_t ndx = iface->ncount;
          do {
            PhxNexus *nexus = iface->nexus[(--ndx)];
            _default_nexus_raze((PhxObject*)nexus);
          } while (ndx != 0);
          iface->ncount = 0;
        }

          /* On WMs this is more than likely true. They do what they want. */
        locus = ( (iface->mete_box.x != 800)
            || (iface->mete_box.y != 100)
            || (iface->mete_box.w != 400)
            || (iface->mete_box.h != 400) );

        if (locus) {
          uint32_t values[4];
          values[0] = (iface->mete_box.x = 800);
          values[1] = (iface->mete_box.y = 100);
          values[2] = (iface->mete_box.w = 400);
          values[3] = (iface->mete_box.h = 400);
          iface->draw_box.x = (iface->draw_box.y = 0);
          iface->draw_box.w = 400;
          iface->draw_box.h = 400;
          xcb_configure_window(session->connection, window,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
                     | XCB_CONFIG_WINDOW_WIDTH
                     | XCB_CONFIG_WINDOW_HEIGHT, values);
          xcb_flush(session->connection);
            /* reset vid_buffer since usiing before _event_configure arrives. */
          cairo_xcb_surface_set_size(iface->vid_buffer,
                                     iface->mete_box.w,
                                     iface->mete_box.h);
        }
          /* Set new layout. */
        if ((++layout_counter) == 6)  layout_counter = 0;
        user_configure_layout(session->iface[0], layout_counter);

        if ( (!!(session->WMstate & HAS_WM)) || (!locus) )
          ui_invalidate_object((PhxObject*)iface);
      }
      return true;
    }
  }
  return _default_interface_meter(iface, nvt, obj);
}

#pragma mark *** Creation ***

static void
user_configure_layout(PhxInterface *iface, uint8_t layout) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNFuse *nfuse;
  uint16_t quad_height = iface->mete_box.h / 4;
  uint16_t quad_width  = iface->mete_box.w / 4;

  switch (layout) {

    case 0: {
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
      break;
    }

    case 1: {
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
      break;
    }

    case 2: {
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
      break;
    }

    case 3: {
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
      break;
    }

    case 4: {
        /* Want ports to overlay with farthest to headerbar being topmost */
      RECTANGLE(nexus_box, 0, 0, iface->mete_box.w,
                                 iface->mete_box.h - (2*quad_height));
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_SOUTH);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.y = 0;

      RECTANGLE(nexus_box, 0, iface->mete_box.h - (2*quad_height),
                              iface->mete_box.w,
                              quad_height);
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_SOUTH);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.y = GRIPSZ;

      RECTANGLE(nexus_box, 0, iface->mete_box.h - quad_height,
                              iface->mete_box.w,
                              quad_height);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.y = iface->mete_box.h - quad_height;

      ui_window_minimum_set(iface->window, 1, 2*GRIPSZ);
      break;
    }

    case 5: {
        /* Want ports to overlay with closest to headerbar being topmost */
      RECTANGLE(nexus_box, 0, iface->mete_box.h - quad_height,
                              iface->mete_box.w,
                              quad_height);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, iface->mete_box.h - (2*quad_height),
                              iface->mete_box.w,
                              quad_height);
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_SOUTH);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.y = GRIPSZ;

      RECTANGLE(nexus_box, 0, 0, iface->mete_box.w,
                                 iface->mete_box.h - (2*quad_height));
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_SOUTH);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.y = 0;

      ui_window_minimum_set(iface->window, 1, 2*GRIPSZ);
      break;
    }

    case 6: {
        /* Want ports to overlay with rightmost being topmost */
      RECTANGLE(nexus_box, 0, 0, iface->mete_box.w - (2*quad_width),
                                 iface->mete_box.h);
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_EAST);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.x = 0;

      RECTANGLE(nexus_box, iface->mete_box.w - (2*quad_width), 0,
                              quad_width,
                              iface->mete_box.h);
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_EAST);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.x = GRIPSZ;

      RECTANGLE(nexus_box, iface->mete_box.w - quad_width, 0,
                              quad_width,
                              iface->mete_box.h);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.x = iface->mete_box.w - quad_width;

      ui_window_minimum_set(iface->window, 2*GRIPSZ, 1);
      break;
    }

    case 7: {
        /* Want ports to overlay with leftmost being topmost */
      RECTANGLE(nexus_box, iface->mete_box.w - quad_width, 0,
                              quad_width,
                              iface->mete_box.h);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.x = iface->mete_box.w - quad_width;

      RECTANGLE(nexus_box, iface->mete_box.w - (2*quad_width), 0,
                              quad_width,
                              iface->mete_box.h);
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_EAST);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.x = GRIPSZ;

      RECTANGLE(nexus_box, 0, 0, iface->mete_box.w - (2*quad_width),
                                 iface->mete_box.h);
      nfuse = ui_gfuse_create(iface, nexus_box, XCB_GRAVITY_EAST);
      nfuse->state |= HXPD_RGT | VXPD_BTM;
      nfuse->min_max.x = 0;

      ui_window_minimum_set(iface->window, 2*GRIPSZ, 1);
      break;
    }

    default: {
      puts("unknown layout");
      break;
    }
  } /* end switch() */
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

    /* A 'topmost' decorated window */
  window = ui_window_create(configure);
  if (window == 0)  exit(EXIT_FAILURE);
    /* Because of the way user_configure_layout() is being used,
      place this here. */
  session->iface[0]->_event_cb = _cfg_interface_meter;
    /* Since one window... instead of ui_interface_for() */
  user_configure_layout(session->iface[0], 5);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}
