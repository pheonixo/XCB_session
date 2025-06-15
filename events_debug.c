#include "session.h"
#include "windows.h"

/* set all events on */
uint64_t debug_flags = ~0;

#if DEBUG_EVENTS_ON

static bool file_openned = false;
FILE *_debug_wh;

void
_debug_assert(bool test, const char *message) {

  if (!file_openned) {
    _debug_wh = stderr;
    if (!(session->WMstate & HAS_WM)) {
      _debug_wh = fopen("./xcb_session.debug", "w");
      if (_debug_wh == NULL)  _debug_wh = stderr;
    }
    file_openned = true;
  }
  if (test) {
      /* Incase user debugging with XTerm */
    if (_debug_wh != stderr)
      fprintf(stderr, "%s\n", message);
    fprintf(_debug_wh, "%s\n", message);
  }
}

/* session.c */
extern char *    _xkey_names(xcb_keysym_t sym);

#define DEBUG(a,b) \
  if (_debug_wh != stderr)  fprintf(stderr, "%d %s\n", a, b); \
  fprintf(_debug_wh, "%d %s\n", a, b);

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
    /* Used to initialize _debug_wh */
  if (!file_openned)  _debug_assert(false, caller);

  switch (response) {

    PhxInterface *test;
    xcb_button_press_event_t *bp;

    case XCB_KEY_PRESS: {         /* response_type 2 */
      xcb_key_press_event_t *kp = (xcb_key_press_event_t*)nvt;
      xcb_keysym_t keyval       = _xcb_keysym_for(kp->detail, kp->state);
      test = ui_interface_for(kp->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", KEY_PRESS.  "
               " code %"PRIu8" modifers %"PRIu16""
               " keyval %"PRIx32" key_name %s  %s\n",
               test->window, kp->detail, kp->state,
               keyval, _xkey_names(keyval), caller);
      fprintf(_debug_wh, "window %"PRIu32", KEY_PRESS.  "
             " code %"PRIu8" modifers %"PRIu16""
             " keyval %"PRIx32" key_name %s  %s\n",
             test->window, kp->detail, kp->state,
             keyval, _xkey_names(keyval), caller);
      break;
    }
    case XCB_KEY_RELEASE: {       /* response_type 3 */
      xcb_key_press_event_t *kp = (xcb_key_press_event_t*)nvt;
      xcb_keysym_t keyval       = _xcb_keysym_for(kp->detail, kp->state);
      test = ui_interface_for(kp->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", KEY_RELEASE."
               " code %"PRIu8" modifers %"PRIu16""
               " keyval %"PRIx32" key_name %s  %s\n",
               test->window, kp->detail, kp->state,
               keyval, _xkey_names(keyval), caller);
      fprintf(_debug_wh, "window %"PRIu32", KEY_RELEASE."
             " code %"PRIu8" modifers %"PRIu16""
             " keyval %"PRIx32" key_name %s  %s\n",
             test->window, kp->detail, kp->state,
             keyval, _xkey_names(keyval), caller);
      break;
    }
    case XCB_BUTTON_PRESS: {      /* response_type 4 */
      bp = (xcb_button_press_event_t*)nvt;
      test = ui_interface_for(bp->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", BUTTON_PRESS   ", test->window);
      fprintf(_debug_wh, "window %"PRIu32", BUTTON_PRESS   ", test->window);
  bp_detail:
      switch (bp->detail) {
        case 4:
          if (_debug_wh != stderr)
            fprintf(stderr, "Wheel Scroll Up   ");
          fprintf(_debug_wh, "Wheel Scroll Up   ");
          break;
        case 5:
          if (_debug_wh != stderr)
            fprintf(stderr, "Wheel Scroll Down ");
          fprintf(_debug_wh, "Wheel Scroll Down ");
          break;
        case 6:
          if (_debug_wh != stderr)
            fprintf(stderr, "Wheel Scroll Left ");
          fprintf(_debug_wh, "Wheel Scroll Left ");
          break;
        case 7:
          if (_debug_wh != stderr)
            fprintf(stderr, "Wheel Scroll Right");
          fprintf(_debug_wh, "Wheel Scroll Right");
          break;
        default:  /* 8, 9, 1, 2, 3 */
          if (_debug_wh != stderr)
            fprintf(stderr, " Button %"PRIu8"", bp->detail);
          fprintf(_debug_wh, " Button %"PRIu8"", bp->detail);
          break;
      }
      if (_debug_wh != stderr)
        fprintf(stderr, " modifier %"PRIu16" (%"PRIi16",%"PRIi16")  %s\n",
             bp->state, bp->event_x, bp->event_y, caller);
      fprintf(_debug_wh, " modifier %"PRIu16" (%"PRIi16",%"PRIi16")  %s\n",
             bp->state, bp->event_x, bp->event_y, caller);
      break;
    }
    case XCB_BUTTON_RELEASE: {    /* response_type 5 */
      bp = (xcb_button_press_event_t*)nvt;
      test = ui_interface_for(bp->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", BUTTON_RELEASE ", test->window);
      fprintf(_debug_wh, "window %"PRIu32", BUTTON_RELEASE ", test->window);
      goto bp_detail;
    }
    case XCB_MOTION_NOTIFY: {     /* response_type 6 */
      xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t*)nvt;
      test = ui_interface_for(motion->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", MOTION_NOTIFY."
             " (%"PRIi16",%"PRIi16") state=%"PRIu16" %s\n",
             test->window, motion->event_x, motion->event_y,
             motion->state, caller);
      fprintf(_debug_wh, "window %"PRIu32", MOTION_NOTIFY."
             " (%"PRIi16",%"PRIi16") state=%"PRIu16" %s\n",
             test->window, motion->event_x, motion->event_y,
             motion->state, caller);
      break;
    }
    case XCB_ENTER_NOTIFY: {      /* response_type 7 */
      xcb_enter_notify_event_t *xing = (xcb_enter_notify_event_t*)nvt;
      test = ui_interface_for(xing->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", ENTER_NOTIFY."
             " (%"PRIi16",%"PRIi16") detail=%"PRIu8" mode=%"PRIu8" %s\n",
             test->window, xing->event_x, xing->event_y,
             xing->detail, xing->mode, caller);
      fprintf(_debug_wh, "window %"PRIu32", ENTER_NOTIFY."
             " (%"PRIi16",%"PRIi16") detail=%"PRIu8" mode=%"PRIu8" %s\n",
             test->window, xing->event_x, xing->event_y,
             xing->detail, xing->mode, caller);
      break;
    }
    case XCB_LEAVE_NOTIFY: {      /* response_type 8 */
      xcb_enter_notify_event_t *xing = (xcb_enter_notify_event_t*)nvt;
      test = ui_interface_for(xing->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", LEAVE_NOTIFY."
             " (%"PRIi16",%"PRIi16") detail=%"PRIu8" mode=%"PRIu8" %s\n",
             test->window, xing->event_x, xing->event_y,
             xing->detail, xing->mode, caller);
      fprintf(_debug_wh, "window %"PRIu32", LEAVE_NOTIFY."
             " (%"PRIi16",%"PRIi16") detail=%"PRIu8" mode=%"PRIu8" %s\n",
             test->window, xing->event_x, xing->event_y,
             xing->detail, xing->mode, caller);
      break;
    }
    case XCB_FOCUS_IN:            /* response_type 9 */
    case XCB_FOCUS_OUT: {         /* response_type 10 */
      xcb_focus_in_event_t *focus = (xcb_focus_in_event_t*)nvt;
      test = ui_interface_for(focus->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", FOCUS_", test->window);
      fprintf(_debug_wh, "window %"PRIu32", FOCUS_", test->window);
      if ((nvt->response_type & (uint8_t)0x7F) == XCB_FOCUS_IN) {
        if (_debug_wh != stderr)
          fprintf(stderr, "IN. detail=%"PRIu8" mode=%"PRIu8" %s\n",
               focus->detail, focus->mode, caller);
        fprintf(_debug_wh, "IN. detail=%"PRIu8" mode=%"PRIu8" %s\n",
               focus->detail, focus->mode, caller);
      } else {
        if (_debug_wh != stderr)
          fprintf(stderr, "OUT. detail=%"PRIu8" mode=%"PRIu8" %s\n",
               focus->detail, focus->mode, caller);
        fprintf(_debug_wh, "OUT. detail=%"PRIu8" mode=%"PRIu8" %s\n",
               focus->detail, focus->mode, caller);
      }
      break;
    }
    case XCB_EXPOSE: {            /* response_type 12 */
      xcb_expose_event_t *expose = (xcb_expose_event_t*)nvt;
      test = ui_interface_for(expose->window);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", EXPOSE. Region"
         " (%"PRIu16",%"PRIu16",%"PRIu16",%"PRIu16")  %s\n",
         test->window, expose->x, expose->y,
         expose->width, expose->height, caller);
      fprintf(_debug_wh, "window %"PRIu32", EXPOSE. Region"
         " (%"PRIu16",%"PRIu16",%"PRIu16",%"PRIu16")  %s\n",
         test->window, expose->x, expose->y,
         expose->width, expose->height, caller);
      break;
    }
    case XCB_VISIBILITY_NOTIFY: { /* response_type 15 */
      xcb_visibility_notify_event_t *seen
        = (xcb_visibility_notify_event_t*)nvt;
      test = ui_interface_for(seen->window);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32","
             " VISIBILITY_NOTIFY. state %"PRIu8"  %s\n",
             test->window, seen->state, caller);
      fprintf(_debug_wh, "window %"PRIu32","
             " VISIBILITY_NOTIFY. state %"PRIu8"  %s\n",
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
      PhxInterface *iface = ui_interface_for(unmap->window);
      DEBUG(iface->window, "XCB_UNMAP_NOTIFY");
      break;
    }
    case XCB_MAP_NOTIFY: {        /* response_type 19 */
      xcb_map_notify_event_t *map
        = (xcb_map_notify_event_t*)nvt;
      PhxInterface *iface = ui_interface_for(map->window);
      DEBUG(iface->window, "XCB_MAP_NOTIFY");
      break;
    }
    case XCB_REPARENT_NOTIFY: {   /* response_type 21 */
      xcb_reparent_notify_event_t *rp
        = (xcb_reparent_notify_event_t*)nvt;
      PhxInterface *iface = ui_interface_for(rp->window);
      DEBUG(iface->window, "XCB_REPARENT_NOTIFY");
      break;
    }
    case XCB_CONFIGURE_NOTIFY: {  /* response_type 22 */
      xcb_configure_notify_event_t *configure
        = (xcb_configure_notify_event_t*)nvt;
      PhxInterface *iface = ui_interface_for(configure->event);
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", CONFIGURE_NOTIFY."
             " (%"PRIi16",%"PRIi16",%"PRIi16",%"PRIi16")  %s\n",
             iface->window, configure->x, configure->y,
             configure->width, configure->height, caller);
      fprintf(_debug_wh, "window %"PRIu32", CONFIGURE_NOTIFY."
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
        PhxInterface *iface = ui_interface_for(window);
        window = (iface == NULL) ? 0 : iface->window;
      }
      if (_debug_wh != stderr)
        fprintf(stderr, "window %"PRIu32", XCB_PROPERTY_NOTIFY."
         " atom %"PRIu32" %s, state %"PRIu8"  %s\n",
         window, prop->atom, buffer, prop->state, caller);
      fprintf(_debug_wh, "window %"PRIu32", XCB_PROPERTY_NOTIFY."
         " atom %"PRIu32" %s, state %"PRIu8"  %s\n",
         window, prop->atom, buffer, prop->state, caller);
      break;
    }
    case XCB_SELECTION_CLEAR: {   /* response_type 29 */
      xcb_selection_clear_event_t *clear
        = (xcb_selection_clear_event_t*)nvt;
      xcb_window_t window = clear->owner;
      PhxInterface *iface = ui_interface_for(window);
      DEBUG(iface->window, "XCB_SELECTION_CLEAR");
      break;
    }
    case XCB_SELECTION_REQUEST: { /* response_type 30 */
      xcb_selection_request_event_t *request
        = (xcb_selection_request_event_t*)nvt;
      xcb_window_t window = request->owner;
      PhxInterface *iface = ui_interface_for(window);
      DEBUG(iface->window, "XCB_SELECTION_REQUEST");
      break;
    }
    case XCB_SELECTION_NOTIFY: {  /* response_type 31 */
      xcb_selection_notify_event_t *selection
        = (xcb_selection_notify_event_t*)nvt;
      xcb_window_t window = selection->requestor;
      PhxInterface *iface = ui_interface_for(window);
      DEBUG(iface->window, "XCB_SELECTION_NOTIFY");
      break;
    }
    case XCB_CLIENT_MESSAGE: {    /* response_type 33 */
      xcb_client_message_event_t *message
        = (xcb_client_message_event_t*)nvt;
      char buffer0[64];
      xcb_get_atom_name_reply_t *reply
        = xcb_get_atom_name_reply(session->connection,
                  xcb_get_atom_name(session->connection, message->type), NULL);
      memmove(buffer0, xcb_get_atom_name_name(reply), reply->name_len);
      buffer0[reply->name_len] = 0;
      free(reply);
      if (strcmp(buffer0, "WM_PROTOCOLS") == 0) {
        char buffer1[64];
        reply
          = xcb_get_atom_name_reply(session->connection,
         xcb_get_atom_name(session->connection, message->data.data32[0]), NULL);
        memmove(buffer1, xcb_get_atom_name_name(reply), reply->name_len);
        buffer1[reply->name_len] = 0;
        free(reply);
        if (_debug_wh != stderr)
          fprintf(stderr, "%"PRIu32" XCB_CLIENT_MESSAGE %s:%s\n",
               message->window, buffer0, buffer1);
        fprintf(_debug_wh, "%"PRIu32" XCB_CLIENT_MESSAGE %s:%s\n",
               message->window, buffer0, buffer1);
      } else {
        if (_debug_wh != stderr)
          fprintf(stderr, "%"PRIu32" XCB_CLIENT_MESSAGE %s\n",
               message->window, buffer0);
        fprintf(_debug_wh, "%"PRIu32" XCB_CLIENT_MESSAGE %s\n",
               message->window, buffer0);
      }
      break;
    }

    default:
      if (nvt->response_type == 0) {
        xcb_generic_error_t *err = (xcb_generic_error_t*)nvt;
        uint8_t code = err->error_code - (uint8_t)128;
        if (code < (uint8_t)(180 - 128)) {
          if (_debug_wh != stderr)
            fprintf(stderr, "error: %"PRIu8" %s %s\n",
                err->error_code, extension_errors[code], caller);
          fprintf(_debug_wh, "error: %"PRIu8" %s %s\n",
                err->error_code, extension_errors[code], caller);
        } else {
          if (_debug_wh != stderr)
            fprintf(stderr, "unknown error: %"PRIu8" (%"PRIu8":%"PRIu16")"
                 " %"PRIu32" %s\n",
                 err->error_code, err->major_code, err->minor_code,
                 err->resource_id, caller);
          fprintf(_debug_wh, "unknown error: %"PRIu8" (%"PRIu8":%"PRIu16")"
                 " %"PRIu32" %s\n",
                 err->error_code, err->major_code, err->minor_code,
                 err->resource_id, caller);
        }
        break;
      }
        /* Unknown event type, use to track coding problems */
      if (_debug_wh != stderr)
        fprintf(stderr, "Unknown event:"
             " %"PRIu8"  %s\n", nvt->response_type, caller);
      fprintf(_debug_wh, "Unknown event:"
             " %"PRIu8"  %s\n", nvt->response_type, caller);
      break;
  }
}

#endif /* DEBUG_EVENTS_ON */

