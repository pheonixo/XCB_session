#include "../nexus.h"
#include "../nfuse.h"
#include "../gfuse.h"
#include "../draw.h"

static uint8_t layout_counter = 0;
static void  user_configure_layout(PhxInterface *iface, uint8_t layout);

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
            || (iface->mete_box.y != 200)
            || (iface->mete_box.w != 800)
            || (iface->mete_box.h != 200) );

        if (locus) {
          uint32_t values[4];
          values[0] = (iface->mete_box.x = 800);
          values[1] = (iface->mete_box.y = 200);
          values[2] = (iface->mete_box.w = 800);
          values[3] = (iface->mete_box.h = 200);
          iface->draw_box.x = (iface->draw_box.y = 0);
          iface->draw_box.w = 800;
          iface->draw_box.h = 200;
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
        if ((++layout_counter) == 26)  layout_counter = 0;
        user_configure_layout(session->iface[0], layout_counter);

        if ( (!!(session->WMstate & HAS_WM)) || (!locus) )
          ui_invalidate_object((PhxObject*)iface);
      }
      return true;
    }
  }
  return _default_interface_meter(iface, nvt, obj);
}

static void
user_configure_layout(PhxInterface *iface, uint8_t layout) {

  PhxRectangle nexus_box;
  PhxNexus *nexus;
  PhxNFuse *fuse;

  switch (layout) {

    case 0: {
      RECTANGLE(nexus_box, 0, 0, 800 - 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, y moves */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 1: {
      RECTANGLE(nexus_box, 0, 0, 800, 200 - 40);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 200 - 40, 800, 40);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, x moves */
      nexus->state |= VXPD_TOP;
      break;
    }

    case 2: {
      RECTANGLE(nexus_box, 40, 100, 800 - 80, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* fix left x position at 40. */
      nexus->min_max.x = 40;
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 40, 0, 800 - 80, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* fix left x position at 40. */
      nexus->min_max.x = 40;
        /* fix max height growth to 100. */
      nexus->min_max.h = 100;
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, y moves */
      nexus->state |= HXPD_LFT;

      RECTANGLE(nexus_box, 0, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, y moves */
      nexus->state |= HXPD_LFT;
        /* set 40 as max right movement */
      nexus->min_max.w = 40;
      break;
    }

    case 3: {
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* fix max height growth to 100. */
      nexus->min_max.h = 100;
      nexus->state |= HXPD_RGT | VXPD_BTM;

        /* set as overprint, but pushed by HXPD_LFT */
      RECTANGLE(nexus_box, 380, 20, 40, 200 - 40);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, y moves */
      nexus->state |= HXPD_LFT;
        /* Don't pull pass 420. */
      nexus->min_max.w = 420;
        /* allows lower to link with upper by stopping this from blocking */
      nexus->min_max.h = INT16_MAX - 1;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, y moves */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 4: {
      int ndx;
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* fix max height growth to 100. */
      nexus->min_max.h = 100;
      nexus->state |= HXPD_RGT | VXPD_BTM;

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 0; ndx < 300; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant height,width, y moves */
        nexus->state |= HXPD_LFT;
          /* allows lower to link with upper by stopping this from blocking */
        nexus->min_max.h = INT16_MAX - 1;
          /* each to be max.w (Don't move pass) */
        nexus->min_max.w = 140 + ndx;
      }

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, y moves */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 5: {
      RECTANGLE(nexus_box, 0, 0, 800 - 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 800 - 120, 200 - 120, 80, 120);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, move x, y */
      nexus->state |= HXPD_LFT | VXPD_TOP;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height,width, x moves */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 6: {
      int hdx;
      RECTANGLE(nexus_box, 0, 0, 800 - 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 800 - 120, 200 - 120, 80, 120);
      fuse = ui_nfuse_create(iface, nexus_box);
      /*nexus->min_max.y = -INT16_MAX + 1;*/
          /* fuse these 3 together */
        for (hdx = 0; hdx < 120; hdx += 40) {
          RECTANGLE(nexus_box, 0, hdx, 80, 40);
          nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /*nexus->min_max.h = hdx + 40;*/
        }

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width, x moves */
      nexus->state |= HXPD_LFT | VXPD_BTM;
      break;
    }

    case 7: {
      int hdx;
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->min_max.h = 100;
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 800 - 120, 200 - 120, 80, 120);
      fuse = ui_nfuse_create(iface, nexus_box);
        /* Unlike 6... cause x movement. */
      fuse->state |= HXPD_LFT;
          /* fuse these 3 together */
        for (hdx = 0; hdx < 120; hdx += 40) {
          RECTANGLE(nexus_box, 0, hdx, 80, 40);
          nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /*nexus->min_max.h = hdx + 40;*/
        }

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width, x moves */
      nexus->state |= HXPD_LFT | VXPD_BTM;
      break;
    }

    case 8: {
      RECTANGLE(nexus_box, 0, 0, 400, 200 - 40);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->min_max.w = 400;
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 400, 0, 400, 200 - 40);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 200 - 40, 800, 40);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width, y moves */
      nexus->state |= VXPD_TOP;
      break;
    }

    case 9: {
      RECTANGLE(nexus_box, 0, 100, 800, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->min_max.h = 100;
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 120);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves */
      nexus->state |= HXPD_LFT;
      nexus->min_max.h = 120;
      break;
    }

    case 10: {
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800, 100);
      nexus = ui_nexus_create(iface, nexus_box);
      nexus->min_max.h = 100;
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 800 - 40, 80, 40, 120);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x,y moves */
      nexus->state |= HXPD_LFT | VXPD_TOP;
      nexus->min_max.h = 200;
      break;
    }

    case 11: {
      RECTANGLE(nexus_box, 0, 0, 400, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400;

      RECTANGLE(nexus_box, 400, 0, 400, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 400 - 20, 200 - 40, 400 + 20, 40);
      nexus = ui_nexus_create(iface, nexus_box);
       /* Set constant width,height, x,y moves */
      nexus->state |= HXPD_LFT | VXPD_TOP;
      nexus->min_max.w = 800;
      break;
    }

    case 12: {
      RECTANGLE(nexus_box, 0, 100, 800, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 800 - 40, 40, 40, 120);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
      nexus->min_max.h = 120;
      break;
    }

    case 13: {
        /* Once width size reduced to 400, grow will have (4)
         * expand underneath (5) */
      RECTANGLE(nexus_box, 0, 0, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400;
      nexus->min_max.h = 80;

      RECTANGLE(nexus_box, 400, 0, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 80;

      RECTANGLE(nexus_box, 0, 100 - 20, 800, 40);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height, y moves, x,width static */
      nexus->state |= VXPD_TOP;
      nexus->min_max.h = 120;

        /* Intentional growth under last nexus overprint. */
      RECTANGLE(nexus_box, 0, 120, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 400, 120, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      break;
    }

    case 14: {
        /* Once width size reduced to 400, grow will have (4)
         * expand underneath (5) */
        /* center bar top-most nexus */
      RECTANGLE(nexus_box, 0, 0, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400;
      nexus->min_max.h = 80;

      RECTANGLE(nexus_box, 400, 0, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 80;

      RECTANGLE(nexus_box, 0, 120, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 400, 120, 400, 100 - 20);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 100 - 20, 800, 40);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant height, y moves, x,width static */
      nexus->state |= VXPD_TOP;
      nexus->min_max.h = 120;
      break;
    }

    case 15: {
      RECTANGLE(nexus_box, 0, 0, 400, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400;
      nexus->min_max.h = 100;

        /* (2) is under center vertical bar */
      RECTANGLE(nexus_box, 400, 0, 400, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 400 - 20, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
        /* Limit its end position to x = 420 */
      nexus->min_max.w = 420;

        /* Center vertical to pass under thiis overprint. */
      RECTANGLE(nexus_box, 0, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400 - 20;

      RECTANGLE(nexus_box, 420, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      break;
    }

    case 16: {
        /* always under bar */
      RECTANGLE(nexus_box, 400, 0, 400, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

        /* starts under vertical bar, grows pass */
      RECTANGLE(nexus_box, 0, 0, 400, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 500;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 400 - 20, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width, x moves, y static, height expands. */
      nexus->state |= HXPD_LFT | VXPD_BTM;
        /* Limit its end position to x = 420 */
      nexus->min_max.w = 420;

      RECTANGLE(nexus_box, 0, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400 - 20;

      RECTANGLE(nexus_box, 420, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      break;
    }

    case 17: {
      RECTANGLE(nexus_box, 500, 0, 300, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.x = 500;
      nexus->min_max.h = 100;

        /* starts under vertical bar, grows pass */
      RECTANGLE(nexus_box, 0, 0, 500, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 500;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 400 - 20, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width, x moves, y static, height expands. */
      nexus->state |= HXPD_LFT | VXPD_BTM;
        /* Limit its end position to x = 420 */
      nexus->min_max.w = 420;

      RECTANGLE(nexus_box, 0, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400 - 20;

      RECTANGLE(nexus_box, 420, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      break;
    }

    case 18: {
        /* As 15 but places bar after [2] */
      RECTANGLE(nexus_box, 400, 0, 400, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 0, 0, 400, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 500;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 0, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.w = 400 - 20;

      RECTANGLE(nexus_box, 400 - 20, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width, x moves, y static, height expands. */
      nexus->state |= HXPD_LFT | VXPD_BTM;
        /* Limit its end position to x = 420 */
      nexus->min_max.w = 420;

      RECTANGLE(nexus_box, 420, 100, 400 - 20, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      break;
    }

    case 19: {
      int ndx;
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 0; ndx < 300; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant width, x moves, y static, height expands. */
        nexus->state |= HXPD_LFT | VXPD_BTM;
          /* each to be max.w */
        nexus->min_max.w = 140 + ndx;
      }
      nexus->min_max.w = 100 + ndx;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width, x moves, y static, height expands. */
      nexus->state |= HXPD_LFT | VXPD_BTM;
      break;
    }

    case 20: {
      int ndx, hdx;
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant width,height, x moves, y static */
        nexus->state |= HXPD_LFT;
          /* each to be max.w */
        nexus->min_max.w = 140 + ndx;
      }
      nexus->min_max.w = 100 + ndx;

        /* These 3 should attach to bottom, allow one to only
          see 4 corners of topmost nexus. */
      for (hdx = 40; hdx < 130; hdx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, hdx, 80, 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
        nexus->min_max.w = 300;
      }

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 200; ndx < 300; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant width,height, x moves, y static */
        nexus->state |= HXPD_LFT;
          /* each to be max.w */
        nexus->min_max.w = 140 + ndx;
      }
      nexus->min_max.w = 100 + ndx;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 21: {
        /* 4 corners remain in static position except the first 2 bottommost. */
      int ndx, hdx;
      RECTANGLE(nexus_box, 40, 100, 800 - 80, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.x = 40;

      RECTANGLE(nexus_box, 40, 0, 800 - 80, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.x = 40;
      nexus->min_max.h = 100;

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant width,height, x moves, y static */
        nexus->state |= HXPD_LFT;
          /* each to only be 40 wide */
        nexus->min_max.w = 140 + ndx;
      }

      for (hdx = 40; hdx < 130; hdx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, hdx, 80, 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* nexus static */
        nexus->min_max.w = 300;
          /* each to only be 40 high */
        nexus->min_max.h = 40 + hdx;
      }

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 200; ndx < 300; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant width,height, x moves, y static */
        nexus->state |= HXPD_LFT;
          /* each to only be 40 wide */
        nexus->min_max.w = 140 + ndx;
      }

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;

      RECTANGLE(nexus_box, 0, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
        /* anchor to left */
      nexus->min_max.w = 40;
      break;
    }

    case 22: {
        /* Only vertically oriented ports have corners static. */
      int ndx, hdx;
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant width,height, x moves, y static */
        nexus->state |= HXPD_LFT;
        nexus->min_max.h = 180;
        nexus->min_max.w = 140 + ndx;
      }

      for (hdx = 40; hdx < 130; hdx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, hdx, 80, 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
        nexus->min_max.h = hdx + 40;
        nexus->min_max.w = 300;
      }

        /* set as overprint, but pushed by HXPD_LFT */
      for (ndx = 200; ndx < 300; ndx += 40) {
        RECTANGLE(nexus_box, 100 + ndx, 20, 40, 200 - 40);
        nexus = ui_nexus_create(iface, nexus_box);
          /* Set constant width,height, x moves, y static */
        nexus->state |= HXPD_LFT;
        nexus->min_max.h = 180;
        nexus->min_max.w = 140 + ndx;
      }
      nexus->min_max.w = 100 + ndx;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 23: {
        /* Central ports are 3 nfuse. Grouping internals remain static
          reguardless of state designation. Only the fuse moves according
          to designation. */
      int ndx;
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 100, 20, 120, 200 - 40);
      fuse = ui_nfuse_create(iface, nexus_box);
      fuse->min_max.w = 220;
      fuse->min_max.h = 180;
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, ndx, 0, 40, 200 - 40);
        nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
      }

      RECTANGLE(nexus_box, 220, 40, 80, 120);
      fuse = ui_nfuse_create(iface, nexus_box);
      fuse->state &= ~HXPD_LFT;
      fuse->min_max.h = 160;
      fuse->min_max.w = 300;
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, 0, ndx, 80, 40);
        nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
      }

      RECTANGLE(nexus_box, 300, 20, 120, 200 - 40);
      fuse = ui_nfuse_create(iface, nexus_box);
      fuse->min_max.w = 420;
      fuse->min_max.h = 180;
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, ndx, 0, 40, 200 - 40);
        nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
      }
      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 24: {
      int ndx;
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 180, 20, 120, 200 - 40);
      fuse = ui_nfuse_create(iface, nexus_box);
        /* Set constant width,height, x,y moves. */
      fuse->state |= HXPD_LFT | VXPD_TOP;
      fuse->min_max.w = 300;
      fuse->min_max.h = 180;
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, ndx, 0, 40, 200 - 40);
        nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
      }

      RECTANGLE(nexus_box, 100, 40, 80, 120);
      fuse = ui_nfuse_create(iface, nexus_box);
        /* Want static nfuse. */
      fuse->min_max.h = 160;
      fuse->min_max.w = 180;
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, 0, ndx, 80, 40);
        nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
      }

      RECTANGLE(nexus_box, 300, 20, 120, 200 - 40);
      fuse = ui_nfuse_create(iface, nexus_box);
        /* Set constant width,height, x,y moves. */
      fuse->state |= HXPD_LFT | VXPD_TOP;
      fuse->min_max.w = 420;
      fuse->min_max.h = 180;
      for (ndx = 0; ndx < 120; ndx += 40) {
        RECTANGLE(nexus_box, ndx, 0, 40, 200 - 40);
        nexus = ui_nexus_create((PhxInterface*)fuse, nexus_box);
          /* Normal width,height change, x,y static */
        nexus->state |= HXPD_RGT | VXPD_BTM;
      }

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
      break;
    }

    case 25: {
      RECTANGLE(nexus_box, 0, 100, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;

      RECTANGLE(nexus_box, 0, 0, 800 - 40, 100);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Normal width,height change, x,y static */
      nexus->state |= HXPD_RGT | VXPD_BTM;
      nexus->min_max.h = 100;

      RECTANGLE(nexus_box, 180, 20, 120, 200 - 40);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x,y moves. */
      nexus->state |= HXPD_LFT | VXPD_TOP;
      nexus->min_max.w = 300;
      nexus->min_max.h = 180;

      RECTANGLE(nexus_box, 100, 40, 80, 120);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, y moves, x static */
      nexus->state |= VXPD_TOP;
      nexus->min_max.w = 180;
      nexus->min_max.h = 160;

      RECTANGLE(nexus_box, 300, 20, 120, 200 - 40);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x,y moves. */
      nexus->state |= HXPD_LFT | VXPD_TOP;
      nexus->min_max.w = 420;
      nexus->min_max.h = 180;

      RECTANGLE(nexus_box, 800 - 40, 0, 40, 200);
      nexus = ui_nexus_create(iface, nexus_box);
        /* Set constant width,height, x moves, y static */
      nexus->state |= HXPD_LFT;
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
  PhxRectangle configure = { 800, 200, 800, 200 };

#if DEBUG_EVENTS_ON
    /* turn off reporting of these events */
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
  user_configure_layout(session->iface[0], 0);
    /* Map the window on the screen */
  xcb_map_window(session->connection, window);

    /* Run event loop */
  xcb_main();

    /* Clean up & disconnect from X server */
  ui_session_shutdown();

  return EXIT_SUCCESS;
}

