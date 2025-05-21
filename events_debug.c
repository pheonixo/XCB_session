#include "session.h"
#include "windows.h"

/* session.c */
extern char *    _xkey_names(xcb_keysym_t sym);

#define DEBUG(a,b) \
  printf("%d %s\n", a, b);

/* set all events on */
uint64_t debug_flags = ~0;

static char *extension_errors[] = {
  "MIT-SHM:""BadSeg",                             /* 128 */
  "XInputExtension:""Device",                     /* 129 */
  "XInputExtension:""Event",                      /* 130 */
  "XInputExtension:""Mode",                       /* 131 */
  "XInputExtension:""DeviceBusy",                 /* 132 */
  "XInputExtension:""Class",                      /* 133 */
  "SYNC:""Counter",                               /* 134 */
  "SYNC:""Alarm",                                 /* 135 */
  "Unused",                                       /* 136 */
  "XKEYBOARD:""Keyboard",                         /* 137 */
  "SECURITY:""error",                             /* 138 */
  "Unused",                                       /* 139 */
  "XFIXES:""BadRegion",                           /* 140 */
  "Unused",                                       /* 141 */
  "RENDER:""PictFormat",                          /* 142 */
  "RENDER:""Picture",                             /* 143 */
  "RENDER:""PictOp",                              /* 144 */
  "RENDER:""GlyphSet",                            /* 145 */
  "RENDER:""Glyph",                               /* 146 */
  "RANDR:""BadOutput",                            /* 147 */
  "RANDR:""BadCrtc",                              /* 148 */
  "RANDR:""BadMode",                              /* 149 */
  "RANDR:""BadProvider",                          /* 150 */
  "Unused",                                       /* 151 */
  "DAMAGE:""BadDamage",                           /* 152 */
  "DOUBLE-BUFFER:""BadBuffer",                    /* 153 */
  "RECORD:""BadContext",                          /* 154 */
  "XVideo:""BadPort",                             /* 155 */
  "XVideo:""BadEncoding",                         /* 156 */
  "XVideo:""BadControl",                          /* 157 */
  "GLX:""BadContext",                             /* 158 */
  "GLX:""BadContextState",                        /* 159 */
  "GLX:""BadDrawable",                            /* 160 */
  "GLX:""BadPixmap",                              /* 161 */
  "GLX:""BadContextTag",                          /* 162 */
  "GLX:""BadCurrentWindow",                       /* 163 */
  "GLX:""BadRenderRequest",                       /* 164 */
  "GLX:""BadLargeRequest",                        /* 165 */
  "GLX:""UnsupportedPrivateRequest",              /* 166 */
  "GLX:""BadFBConfig",                            /* 167 */
  "GLX:""BadPbuffer",                             /* 168 */
  "GLX:""BadCurrentDrawable",                     /* 169 */
  "GLX:""BadWindow",                              /* 170 */
  "GLX:""GLXBadProfileARB",                       /* 171 */
  "XFree86-VidModeExtension:""BadClock",          /* 172 */
  "XFree86-VidModeExtension:""BadHTimings",       /* 173 */
  "XFree86-VidModeExtension:""BadVTimings",       /* 174 */
  "XFree86-VidModeExtension:""ModeUnsuitable",    /* 175 */
  "XFree86-VidModeExtension:""ExtensionDisabled", /* 176 */
  "XFree86-VidModeExtension:""ClientNotLocal",    /* 177 */
  "XFree86-VidModeExtension:""ZoomLocked",        /* 178 */
  "XFree86-DGA:""error"                           /* 179 */
};

void
_debug_event(xcb_generic_event_t *nvt, const char *caller) {

  uint8_t response = nvt->response_type & (uint8_t)0x7F;
  uint64_t on_bit = (uint64_t)1 << response;
  if ((debug_flags & on_bit) == 0)  return;

  switch (response) {

    PhxInterface *test;
    xcb_button_press_event_t *bp;

    case XCB_KEY_PRESS: {         /* response_type 2 */
      xcb_key_press_event_t *kp = (xcb_key_press_event_t*)nvt;
      xcb_keysym_t keyval       = _xcb_keysym_for(kp->detail, kp->state);
      test = _interface_for(kp->event);
      printf("window %"PRIu32", KEY_PRESS.  "
             " code %"PRIu8" modifers %"PRIu16""
             " keyval %"PRIx32" key_name %s  %s\n",
             test->window, kp->detail, kp->state,
             keyval, _xkey_names(keyval), caller);
      break;
    }
    case XCB_KEY_RELEASE: {       /* response_type 3 */
      xcb_key_press_event_t *kp = (xcb_key_press_event_t*)nvt;
      xcb_keysym_t keyval       = _xcb_keysym_for(kp->detail, kp->state);
      test = _interface_for(kp->event);
      printf("window %"PRIu32", KEY_RELEASE."
             " code %"PRIu8" modifers %"PRIu16""
             " keyval %"PRIx32" key_name %s  %s\n",
             test->window, kp->detail, kp->state,
             keyval, _xkey_names(keyval), caller);
      break;
    }
    case XCB_BUTTON_PRESS: {      /* response_type 4 */
      bp = (xcb_button_press_event_t*)nvt;
      test = _interface_for(bp->event);
      printf("window %"PRIu32", BUTTON_PRESS   ", test->window);
  bp_detail:
      switch (bp->detail) {
        case 4:  printf("Wheel Scroll Up   ");  break;
        case 5:  printf("Wheel Scroll Down ");  break;
        case 6:  printf("Wheel Scroll Left ");  break;
        case 7:  printf("Wheel Scroll Right");  break;
        default:  /* 8, 9, 1, 2, 3 */
          printf(" Button %"PRIu8"", bp->detail);
          break;
      }
      printf(" modifier %"PRIu16" (%"PRIi16",%"PRIi16")  %s\n",
             bp->state, bp->event_x, bp->event_y, caller);
      break;
    }
    case XCB_BUTTON_RELEASE: {    /* response_type 5 */
      bp = (xcb_button_press_event_t*)nvt;
      test = _interface_for(bp->event);
      printf("window %"PRIu32", BUTTON_RELEASE ", test->window);
      goto bp_detail;
    }
    case XCB_MOTION_NOTIFY: {     /* response_type 6 */
      xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t*)nvt;
      test = _interface_for(motion->event);
      printf("window %"PRIu32", MOTION_NOTIFY."
             " (%"PRIi16",%"PRIi16")  %s\n",
             test->window, motion->event_x, motion->event_y, caller);
      break;
    }
    case XCB_ENTER_NOTIFY: {      /* response_type 7 */
      xcb_enter_notify_event_t *xing = (xcb_enter_notify_event_t*)nvt;
      test = _interface_for(xing->event);
      printf("window %"PRIu32", ENTER_NOTIFY."
             " (%"PRIi16",%"PRIi16") detail=%"PRIu8" mode=%"PRIu8" %s\n",
             test->window, xing->event_x, xing->event_y,
             xing->detail, xing->mode, caller);
      break;
    }
    case XCB_LEAVE_NOTIFY: {      /* response_type 8 */
      xcb_enter_notify_event_t *xing = (xcb_enter_notify_event_t*)nvt;
      test = _interface_for(xing->event);
      printf("window %"PRIu32", LEAVE_NOTIFY."
             " (%"PRIi16",%"PRIi16") detail=%"PRIu8" mode=%"PRIu8" %s\n",
             test->window, xing->event_x, xing->event_y,
             xing->detail, xing->mode, caller);
      break;
    }
    case XCB_FOCUS_IN:            /* response_type 9 */
    case XCB_FOCUS_OUT: {         /* response_type 10 */
      xcb_focus_in_event_t *focus = (xcb_focus_in_event_t*)nvt;
      test = _interface_for(focus->event);
      printf("window %"PRIu32", FOCUS_", test->window);
      if ((nvt->response_type & (uint8_t)0x7F) == XCB_FOCUS_IN) {
        printf("IN. %s\n", caller);
      } else {
        printf("OUT. %s\n", caller);
      }
      break;
    }
    case XCB_EXPOSE: {            /* response_type 12 */
      xcb_expose_event_t *expose = (xcb_expose_event_t*)nvt;
      test = _interface_for(expose->window);
      printf("window %"PRIu32", EXPOSE. Region"
         " (%"PRIu16",%"PRIu16",%"PRIu16",%"PRIu16")  %s\n",
         test->window, expose->x, expose->y,
         expose->width, expose->height, caller);
      break;
    }
    case XCB_VISIBILITY_NOTIFY: { /* response_type 15 */
      xcb_visibility_notify_event_t *seen
        = (xcb_visibility_notify_event_t*)nvt;
      test = _interface_for(seen->window);
      printf("window %"PRIu32", VISIBILITY_NOTIFY. state %"PRIu8"  %s\n",
             test->window, seen->state, caller);
      break;
    }
    case XCB_DESTROY_NOTIFY: {    /* response_type  17 */
        /* assume a kill, delete all windows (do their thing)
           send false to exit loop */
      xcb_destroy_notify_event_t *destroy
        = (xcb_destroy_notify_event_t*)nvt;
      DEBUG(destroy->window, "XCB_DESTROY_NOTIFY");
      break;
    }
    case XCB_UNMAP_NOTIFY: {      /* response_type 18 */
      xcb_unmap_notify_event_t *unmap
        = (xcb_unmap_notify_event_t*)nvt;
      PhxInterface *iface = _interface_for(unmap->window);
      DEBUG(iface->window, "XCB_UNMAP_NOTIFY");
      break;
    }
    case XCB_MAP_NOTIFY: {        /* response_type 19 */
      xcb_map_notify_event_t *map
        = (xcb_map_notify_event_t*)nvt;
      PhxInterface *iface = _interface_for(map->window);
      DEBUG(iface->window, "XCB_MAP_NOTIFY");
      break;
    }
    case XCB_REPARENT_NOTIFY: {   /* response_type 21 */
      xcb_reparent_notify_event_t *rp
        = (xcb_reparent_notify_event_t*)nvt;
      PhxInterface *iface = _interface_for(rp->window);
      DEBUG(iface->window, "XCB_REPARENT_NOTIFY");
      break;
    }
    case XCB_CONFIGURE_NOTIFY: {  /* response_type 22 */
      xcb_configure_notify_event_t *configure
        = (xcb_configure_notify_event_t*)nvt;
      PhxInterface *iface = _interface_for(configure->event);
      printf("window %"PRIu32", CONFIGURE_NOTIFY."
             " (%"PRIi16",%"PRIi16",%"PRIi16",%"PRIi16")  %s\n",
             iface->window, configure->x, configure->y,
             configure->width, configure->height, caller);
      break;
    }
    case XCB_PROPERTY_NOTIFY: {   /* response_type 28 */
      char buffer[64];
      xcb_window_t window;
      xcb_property_notify_event_t *prop
        = (xcb_property_notify_event_t*)nvt;
      xcb_get_atom_name_reply_t *reply
        = xcb_get_atom_name_reply(session->connection,
                     xcb_get_atom_name(session->connection, prop->atom), NULL);
      memmove(buffer, xcb_get_atom_name_name(reply), reply->name_len);
      buffer[reply->name_len] = 0;
      free(reply);
      window = prop->window;
      if (prop->state != XCB_PROPERTY_DELETE) {
          /* For validation of iface. */
        PhxInterface *iface = _interface_for(window);
        window = (iface == NULL) ? 0 : iface->window;
      }
      printf("window %"PRIu32", XCB_PROPERTY_NOTIFY."
         " atom %"PRIu32" %s, state %"PRIu8"  %s\n",
         window, prop->atom, buffer, prop->state, caller);
      break;
    }
    case XCB_SELECTION_CLEAR: {   /* response_type 29 */
      xcb_selection_clear_event_t *clear
        = (xcb_selection_clear_event_t*)nvt;
      xcb_window_t window = clear->owner;
      PhxInterface *iface = _interface_for(window);
      DEBUG(iface->window, "XCB_SELECTION_CLEAR");
      break;
    }
    case XCB_SELECTION_REQUEST: { /* response_type 30 */
      xcb_selection_request_event_t *request
        = (xcb_selection_request_event_t*)nvt;
      xcb_window_t window = request->owner;
      PhxInterface *iface = _interface_for(window);
      DEBUG(iface->window, "XCB_SELECTION_REQUEST");
      break;
    }
    case XCB_SELECTION_NOTIFY: {  /* response_type 31 */
      xcb_selection_notify_event_t *selection
        = (xcb_selection_notify_event_t*)nvt;
      xcb_window_t window = selection->requestor;
      PhxInterface *iface = _interface_for(window);
      DEBUG(iface->window, "XCB_SELECTION_NOTIFY");
      break;
    }
    case XCB_CLIENT_MESSAGE: {    /* response_type 33 */
      xcb_client_message_event_t *message
        = (xcb_client_message_event_t*)nvt;
      char buffer[64];
      xcb_get_atom_name_reply_t *reply
        = xcb_get_atom_name_reply(session->connection,
                  xcb_get_atom_name(session->connection, message->type), NULL);
      memmove(buffer, xcb_get_atom_name_name(reply), reply->name_len);
      buffer[reply->name_len] = 0;
      free(reply);
      printf("%"PRIu32" XCB_CLIENT_MESSAGE %s\n", message->window, buffer);
      break;
    }

    default:
      if (nvt->response_type == 0) {
        xcb_generic_error_t *err = (xcb_generic_error_t*)nvt;
        uint8_t code = err->error_code - (uint8_t)128;
        if (code < (uint8_t)(180 - 128)) {
          printf("error: %"PRIu8" %s %s\n",
                err->error_code, extension_errors[code], caller);
        } else {
          printf("unknown error: %"PRIu8" (%"PRIu8":%"PRIu16") %"PRIu32" %s\n",
                 err->error_code, err->major_code, err->minor_code,
                 err->resource_id, caller);
        }
        break;
      }
        /* Unknown event type, use to track coding problems */
      printf("Unknown event: %"PRIu8"  %s\n", nvt->response_type, caller);
      break;
  }
}
