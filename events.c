#include "session.h"
#include "windows.h"
#include "configure.h"
#include "objects.h"
#include "drag.h"

#include <poll.h>

#pragma mark *** Event Support ***

/* For objects within objects. */
static PhxObject *
_get_object_at(PhxObject *obj, int16_t x, int16_t y) {

  PhxObject *rObj = obj;
  PhxObject **objects;
  uint16_t ndx = obj->ncount;
  if (ndx == 0)  return rObj;
  if (ndx == 1)  objects = &obj->child;
  else           objects = (PhxObject **)obj->child;
  do {
    PhxObject *inspect = objects[(--ndx)];
      /* Test to weed out children of the object to be returned. */
    if (inspect->o_mount != inspect)
      continue;

    if (visible_get(inspect)) {
      int16_t x0 = inspect->mete_box.x;
      int16_t y0 = inspect->mete_box.y;
      int16_t w_cmp = inspect->mete_box.w + x0;
      int16_t h_cmp = inspect->mete_box.h + y0;
      if ( (y0 <= y) && (y < h_cmp)
          && (x0 <= x) && (x < w_cmp) ) {
        rObj = inspect;
        break;
      }
    }
  } while (ndx != 0);
  return rObj;
}

/* Objects are 'layered', start from last in of inspection object. */
static PhxObject *
_get_object_from(PhxInterface *iface, int16_t x, int16_t y) {

  PhxObject *rObj = (PhxObject*)iface;
  uint16_t ndx = iface->ncount;
  if (ndx == 0)  return rObj;
  do {
    PhxNexus *inspect = iface->nexus[(--ndx)];
    if (visible_get((PhxObject*)inspect)) {
         /* Don't assume that a <= 0 width/height was made !visible_get() */
      int16_t w_cmp = inspect->mete_box.w;
      int16_t h_cmp = inspect->mete_box.h;
      if ((w_cmp > 0) && (h_cmp > 0)) {
        int16_t x0 = inspect->mete_box.x;
        int16_t y0 = inspect->mete_box.y;

        PhxInterface *mount = (PhxInterface*)inspect;
        while (!IS_WINDOW_TYPE((mount = mount->i_mount))) {
          x0 += mount->mete_box.x;
          y0 += mount->mete_box.y;
        }
          /* Assignment addition for debug */
        w_cmp += x0;
        h_cmp += y0;
        if ( (y0 <= y) && (y < h_cmp)
            && (x0 <= x) && (x < w_cmp) ) {
          if (OBJECT_BASE_TYPE(inspect) > PHX_NEXUS)
            return _get_object_at((PhxObject*)inspect, x - x0, y - y0);
          return _get_object_from((PhxInterface*)inspect, x, y);
        }
      }
    }
  } while (ndx != 0);
  return rObj;
}

PhxObject *
_get_object_at_pointer(PhxInterface *iface, int16_t x, int16_t y) {

    /* check if in 'window' */
    /* do not use mete_box.xy, iface will have placement values */
  if ( ((uint16_t)x >= (uint16_t)iface->mete_box.w)
      || ((uint16_t)y >= (uint16_t)iface->mete_box.h) )
    return NULL;

    /* Use this for recursive ability, avoidance of intro. */
  return _get_object_from(iface, x, y);
}

PhxInterface *
_coordinates_for_object(PhxInterface *iface, xcb_generic_event_t *nvt,
                        PhxObject *obj, int16_t *xPtr, int16_t *yPtr) {

  int16_t x, y;
  PhxInterface *rMount = iface;

  uint8_t response_type = (nvt->response_type & (uint8_t)0x7F);
  if ((uint8_t)(response_type - (uint8_t)XCB_KEY_PRESS)
      >= (uint8_t)((uint8_t)XCB_FOCUS_IN - (uint8_t)XCB_KEY_PRESS)) {
      /* All windows same root, so 'iface->window' is just fine. */
    xcb_query_pointer_cookie_t c0
      = xcb_query_pointer(session->connection, iface->window);
    xcb_query_pointer_reply_t *r0
      = xcb_query_pointer_reply(session->connection, c0, NULL);
    x = r0->root_x;
    y = r0->root_y;
    free(r0);
  } else {
    xcb_motion_notify_event_t *r0
      = (xcb_motion_notify_event_t *)nvt;
    x = r0->root_x;
    y = r0->root_y;
  }

      /* Translation of global to window. */
  if (obj != NULL) {
        /* All type 1 interface are global window positions */
    x -= rMount->mete_box.x;
    y -= rMount->mete_box.y;
        /* Translation to Nexus coordinates. */
    if (!IS_WINDOW_TYPE(obj)) {
      if (obj->type == PHX_GFUSE)  rMount = (PhxInterface*)obj;
      else {
        PhxInterface *mount = obj->i_mount;
        if (IS_IFACE_TYPE(obj))  mount = (PhxInterface*)obj;
        if (rMount->window == mount->window)
          rMount = mount;
        do
          x -= mount->mete_box.x, y -= mount->mete_box.y;
        while (!IS_WINDOW_TYPE((mount = mount->i_mount)));
      }
    }
  }
  *xPtr = x, *yPtr = y;
  return rMount;
}

#pragma mark *** Communication ***

/* Needed for muliple clicks, and determining drag_begin start up. */
static xcb_timestamp_t bptime = 0;

static bool
_event_keyboard(xcb_generic_event_t *nvt) {

  xcb_key_press_event_t *kp = (xcb_key_press_event_t*)nvt;
  PhxInterface *iface       = _interface_for(kp->event);
  PhxObject *focus;

    /* Accidentically sent to us ? or deleted window */
  if ( (iface == NULL)
      || (iface->vid_buffer == NULL) )  return false;

    /* Default gets first shot at this. Don't allow overrride. */
  if (_drag_keyboard(iface, nvt))  return true;

    /* A single iface object can have NULL _event_cb.
      User may wish to add 'app' keys for override of do nothing. */
    /* Address for case where focus follows pointer. no-wm */
  if ((focus = ui_active_focus_get()) != NULL) {
    if ( (focus->_event_cb == NULL)
        || !(focus->_event_cb(iface, nvt, focus)) )
      if (focus != (PhxObject*)iface)
        return _default_interface_meter(iface, nvt, focus);
  }
  return false;
}

/*#define DEBUG_BUTTON(a,b) \
  printf("       %d, SBIT_%s.\n", (a), (b)) */
#define DEBUG_BUTTON(a,b) (void)(a), (void)(b)

static bool
_event_mouse(xcb_generic_event_t *nvt) {

  PhxInterface *iface, *imount;
  PhxObject *obj, *focus;
  int16_t x, *xPtr, y, *yPtr;
  bool handled;
  bool locus = ((nvt->response_type & (uint8_t)0x7F) == XCB_BUTTON_PRESS);
  xcb_button_press_event_t *mouse = (xcb_button_press_event_t*)nvt;

  x = *(xPtr = &mouse->event_x);
  y = *(yPtr = &mouse->event_y);
  session->last_event_x = x;
  session->last_event_y = y;

  iface = _interface_for(mouse->event);
  if (!!(iface->state & SBIT_RELEASE_IGNORE)) {
    iface->state &= ~SBIT_RELEASE_IGNORE;
    return true;
  }
  obj = _get_object_at_pointer(iface, x, y);

    /* Scroll wheel doesn't deal with focus, is 'within' based.
      More than likely, all buttons other than 1. */
  if ((mouse->detail >= 4) && (mouse->detail <= 7)) {
    PhxObject *within = ui_active_within_get();
    DEBUG_ASSERT(( (within == NULL) || (obj != within) ),
                              "failure: no within _event_mouse().");
    if (within->_event_cb != NULL)  within->_event_cb(iface, nvt, obj);
    return true;
  }

  if ((focus = ui_active_focus_get()) == NULL) {
      /* obj can't be NULL on 'press'. release will have a focus. */
    ui_active_focus_set(obj);
    iface = _interface_for(_window_for(obj));
    iface->state |= SBIT_RELEASE_IGNORE;
    _window_stack_topmost(iface);
    bptime = mouse->time;
    return true;
  }

    /* Transient, if focus is a transient. Want
      even non-focused 'parent' events. */
  if (ui_window_is_transient(_window_for(focus))) {
    xcb_window_t window = _window_for(focus);
    iface = _interface_for(window);
    obj = (PhxObject*)iface->nexus[0];
      /* conversion to transient '->event'. */
    _coordinates_for_object(iface, nvt, obj, xPtr, yPtr);
      /* object relative to '->event' coordinates. */
    obj = _get_object_at_pointer(iface, *xPtr, *yPtr);
    iface->_event_cb(iface, nvt, obj);
    return true;
  }

  if (!locus) {
    if (ui_active_drag_get() != NULL)
      return _drag_finish(iface, nvt, obj);
    if ((iface->state & SBIT_SELECTING) != 0) {
      iface->state &= ~SBIT_SELECTING;
      return true;
    }
  } else {
    uint8_t num_clicks;
      /* Feature: click to bring forward a window does not alter state. */
      /* rethink if window and iface click? */
    DEBUG_ASSERT(( (focus != NULL) && (_window_for(focus) != mouse->event) ),
                         "failure: mouse_event bad window.");

    if (obj->i_mount->type != PHX_HEADERBAR) {
      if (focus != obj) {
        iface->state &= ~SBIT_CLICKS;
        ui_active_focus_set(obj);
          /* Need both invalidates prior to next test for possible
            behavioral updates to focus. */
        if (focus != NULL)
          ui_invalidate_object(focus);
          /* if an object and wants FOCUS_ONCLICK, return */
        if ( (!IS_IFACE_TYPE(obj))
            && ((obj->state & OBIT_FOCUS_ONCLICK) != 0) ) {
          puts("using as focus click");
          bptime = mouse->time;
            /* Some objects may draw different based on focus. */
          ui_invalidate_object(obj);
          return true;
        }
      }
    }
      /* Make available to all: single, double, triple button press count. */
    num_clicks = iface->state & SBIT_CLICKS;
    if (mouse->time - bptime > (uint32_t)275)
      num_clicks = 1;
    else if ((++num_clicks) > 3)
      num_clicks = 1;
    iface->state = (iface->state & ~SBIT_CLICKS) | num_clicks;
    bptime = mouse->time;
  }

  DEBUG_ASSERT((ui_active_within_get() != obj),
                         "failure: mouse_event within object.");

  handled = false;
  imount = _coordinates_for_object(iface, nvt, obj, xPtr, yPtr);
  if (imount->_event_cb != NULL)
    handled = imount->_event_cb(iface, nvt, obj);

  focus = ui_active_focus_get();
  if ( !handled && (ui_active_within_get() != focus) )
    ui_active_focus_set((PhxObject*)iface);

  return true;
}

static bool
_event_motion(xcb_generic_event_t *nvt) {

  PhxInterface *iface, *imount;
  PhxObject *obj, *focus;
  xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t*)nvt;
  int16_t x, *xPtr, y, *yPtr;
    /* Note: other than button 1 gets passed to nexus for obj distribution. */
  bool handled, bp = ((motion->state & XCB_BUTTON_MASK_1) != 0);

  x = *(xPtr = &motion->event_x);
  y = *(yPtr = &motion->event_y);
  if ((session->last_event_x == x) && (session->last_event_y == y))
    return true;
    /* Delay for probable hand shift. */
  if (bp && ((motion->time - bptime) < (uint32_t)225))
    return true;

  focus = ui_active_focus_get();

    /* Transient, if focus is a transient. Want
      even non-focused 'parent' events. */
  if (focus != NULL) {
    xcb_window_t window = _window_for(focus);
    if (ui_window_is_transient(window)) {
      iface = _interface_for(window);
      obj = (PhxObject*)iface->nexus[0];
        /* conversion to transient '->event'. */
      _coordinates_for_object(iface, nvt, obj, xPtr, yPtr);
        /* object relative to '->event' coordinates. */
      obj = _get_object_at_pointer(iface, *xPtr, *yPtr);
      iface->_event_cb(iface, nvt, obj);
      return true;
    }
  }

  iface = _interface_for(motion->event);
  obj = _get_object_at_pointer(iface, x, y);

    /* First check if a drag-type was in progress. */
  if (bp) {
      /* Check if internal selecting of 'data'. */
    if ((iface->state & SBIT_SELECTING) != 0) {
      if (obj != NULL)
        _coordinates_for_object(iface, nvt, focus, xPtr, yPtr);
      focus->_event_cb(iface, nvt, obj);
      goto motion_exit;
    }
    if (_drag_motion(iface, nvt, obj))  goto motion_exit;
  }

    /* Drag selection is still possiblity. Send locus notices if changed. */
  if (ui_active_within_get() != obj)
    ui_active_within_set(obj);

    /* Find/set for mount under pointer. */
  imount = _coordinates_for_object(iface, nvt, obj, xPtr, yPtr);
  handled = false;

    /* User/default for object given chance to respond. */
  if (imount->_event_cb != NULL) {
    handled = imount->_event_cb(iface, nvt, obj);
    if (!handled && (imount == iface) && (obj != NULL))
      if (obj->type == PHX_GFUSE)
        handled = obj->_event_cb(iface, nvt, obj);
  }
    /* default action of unhandled start of 'drag motion' */
  if (bp && !handled)
    _drag_selection_box(iface, nvt);

motion_exit:
  session->last_event_x = x;
  session->last_event_y = y;
  return true;
}

static bool
_event_enter(xcb_generic_event_t *nvt) {

  xcb_enter_notify_event_t *xing
    = (xcb_enter_notify_event_t*)nvt;
  PhxInterface *iface;
  PhxObject *obj;

  iface = _interface_for(xing->event);
    /* Using WM's _NET_WM_MOVERESIZE this is only means of determining
      _NET_WM_MOVERESIZE_CANCEL. We regain focus even after we had to
      call ungrab_pointer() to start drag. */
  if (!!(iface->state & SBIT_HBR_DRAG)) {
    if (xing->mode == XCB_NOTIFY_MODE_UNGRAB) {
      if ( (_NET_WM_STRING == NULL)
          || (strcmp(_NET_WM_STRING, "Compiz") != 0) )
        return true;
      puts(" finished drag XCB_ENTER_NOTIFY");
      iface->state &= ~SBIT_HBR_DRAG;
      ui_active_drag_set(NULL);
    }
  }
#if DND_EXTERNAL_ON
  if ( (ui_active_within_get() != NULL)
      && (session->xdndserver->xdndSource.source == 0) )
#else
  if (ui_active_within_get() != NULL)
#endif
    DEBUG_ASSERT(( (ui_active_within_get() == ui_active_focus_get())
                    && ((iface->state & SBIT_HBR_DRAG) == 0) ),
                         "failure: within already set.");

  obj = _get_object_at_pointer(iface, xing->event_x, xing->event_y);
  if (obj == NULL) {
    DEBUG_ASSERT((!ui_window_is_transient(iface->window)),
                        "failure: entered NULL object.");
    return false;
  }
    /* Need basic on entry, objects set to their desire. */
  ui_cursor_set_named("left_ptr", xing->event);
  ui_active_within_set(obj);

  return true;
}

    /* within will not get motion event on window exit */
static bool
_event_leave(xcb_generic_event_t *nvt) {

  xcb_enter_notify_event_t *xing = (xcb_enter_notify_event_t*)nvt;
  if (xing->mode == XCB_NOTIFY_MODE_NORMAL)
    ui_active_within_set(NULL);
  return true;
}

/* if window gains focus, a content focus in event sets iface not an object.
 XXX: test if only one object, a title focus then should set object as focus. */
static bool
_event_focus(xcb_generic_event_t *nvt) {

  bool locus = ((nvt->response_type & (uint8_t)0x7F) == XCB_FOCUS_IN);
  xcb_focus_in_event_t *focus = (xcb_focus_in_event_t*)nvt;
  PhxInterface *iface = _interface_for(focus->event);

  if (iface != NULL) {
    iface->state &= ~SBIT_CLICKS;
      /* On non-dropdown windows, first content click focuses. */
    if ( (locus) && (!ui_window_is_transient(focus->event)) ) {
#if 0
      if (!!(iface->state & SBIT_MAPPED))
        _window_stack_topmost(iface);
#else
      if (!!(iface->state & SBIT_MAPPED)) {
        uint16_t idx;
        const static uint32_t values[] = { XCB_STACK_MODE_ABOVE };
        xcb_configure_window(session->connection, iface->window,
                             XCB_CONFIG_WINDOW_STACK_MODE, values);
        for (idx = 0; idx < session->ncount; idx++)
          if (iface == session->stack_order[idx])  break;
        for (; idx < session->ncount; idx++)
          session->stack_order[idx] = session->stack_order[(idx + 1)];
        session->stack_order[(session->ncount - 1)] = iface;
      }
#endif
    }
  }
  if (!locus) {
    iface->state &= ~SBIT_CLICKS;
    iface = NULL;
  }
  ui_active_focus_set((PhxObject*)iface);
  return true;
}

bool
_event_expose(xcb_generic_event_t *nvt) {

  xcb_expose_event_t *expose = (xcb_expose_event_t*)nvt;
  PhxInterface *iface = _interface_for(expose->window);
  cairo_t *cr;

  DEBUG_ASSERT( ( ((int16_t)expose->x < 0) || ((int16_t)expose->y < 0) ),
                   "probable failure: _event_expose().");

  cr = cairo_create(iface->vid_buffer);
  cairo_rectangle(cr, expose->x, expose->y,
                      expose->width, expose->height);
  cairo_clip(cr);

  _interface_draw(iface);

  cairo_set_source_surface(cr, iface->surface, 0, 0);
  cairo_paint(cr);

  DEBUG_ASSERT((cairo_status(cr) != CAIRO_STATUS_SUCCESS),
                                "failure: _event_expose().");

  cairo_destroy(cr);
  cairo_surface_flush(iface->vid_buffer);

  return true;
}

/* Note: activated dropdown on button release. When activated on
  button press, code needed for one WM to send XCB_ENTER. */
static bool
_event_visibility(xcb_generic_event_t *nvt) {

  xcb_visibility_notify_event_t *seen
    = (xcb_visibility_notify_event_t*)nvt;

  if (ui_window_is_transient(seen->window)) {
    xcb_grab_pointer_cookie_t c0;
    xcb_grab_pointer_reply_t *r0;
      /* On non-WM, need to raise transients. */
    if (session->has_WM == 0) {
      const static uint32_t values[] = { XCB_STACK_MODE_ABOVE };
      xcb_configure_window(session->connection, seen->window,
                           XCB_CONFIG_WINDOW_STACK_MODE, values);
    }
      /* does not send focus in/out events. */
    xcb_set_input_focus(session->connection, XCB_INPUT_FOCUS_PARENT,
                                     seen->window, XCB_CURRENT_TIME);
    ui_active_focus_set((PhxObject*)_interface_for(seen->window));
    c0 = xcb_grab_pointer(session->connection, 1, seen->window,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
           | XCB_EVENT_MASK_POINTER_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
        XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);
    r0 = xcb_grab_pointer_reply(session->connection, c0, NULL);
    if ((r0 == NULL) || (r0->status != XCB_GRAB_STATUS_SUCCESS)) {
      puts("grab failed");
      return false;
    }
  }
  return true;
}

static bool
_event_configure(xcb_generic_event_t *nvt) {

  xcb_configure_notify_event_t *configure
    = (xcb_configure_notify_event_t*)nvt;
  xcb_window_t window = configure->event;
  PhxInterface *iface = _interface_for(window);
  int16_t hD, vD;

  if ( (iface->mete_box.x == configure->x)
      && (iface->mete_box.y == configure->y)
      && (iface->mete_box.w == configure->width)
      && (iface->mete_box.h == configure->height) )
    return true;

  hD = configure->width - iface->mete_box.w;
  vD = configure->height - iface->mete_box.h;
  if ((hD != 0) || (vD != 0))
    _interface_configure(iface, hD, vD);

  if ((configure->x | configure->y) != 0) {
    if ( (iface->mete_box.x != configure->x)
        || (iface->mete_box.y != configure->y) ) {
        /* Needed, if want accurate positioning. WM is unreliable.
          Use separingly, causes severe delays. */
      xcb_connection_t *connection = session->connection;
      xcb_screen_t *screen
        = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
      xcb_translate_coordinates_reply_t *cfg
        = xcb_translate_coordinates_reply(connection,
                          xcb_translate_coordinates(connection,
                                      window, screen->root, 0, 0), NULL);
      iface->mete_box.x = cfg->dst_x;
      iface->mete_box.y = cfg->dst_y;
      free(cfg);
    }
  }
  return true;
}

static bool
_event_selection(xcb_generic_event_t *nvt) {

  uint8_t response_type;

#if DND_EXTERNAL_ON
  if (_dnd_selection_event(nvt))  return true;
#endif
  response_type = (nvt->response_type & (uint8_t)0x7F);

  if (response_type == XCB_SELECTION_CLEAR) {
    xcb_selection_clear_event_t *clear
      = (xcb_selection_clear_event_t*)nvt;
    if (clear->selection == CLIPBOARD)
      _xclb_process_clear(session->xclipboard);
    else
      DEBUG_ASSERT(true, "unhandled XCB_SELECTION_CLEAR.");
    return true;
  }

  if (response_type == XCB_SELECTION_REQUEST) {
    xcb_selection_request_event_t *request
      = (xcb_selection_request_event_t*)nvt;
    if (request->selection == CLIPBOARD) {
      PhxObject *obj = ui_active_focus_get();
      if (obj != NULL) {
        PhxInterface *iface = _interface_for(obj->i_mount->window);
          /* send to object, contents of clipboard is valid. */
        if (obj->_event_cb != NULL)
          return obj->_event_cb(iface, nvt, obj);
      }
         /* Should this be fall-thru? */
      _xclb_process_request(session->xclipboard, request);
    } else {
      DEBUG_ASSERT(true, "unhandled XCB_SELECTION_REQUEST.");
    }
    return true;
  }

  if (response_type == XCB_SELECTION_NOTIFY) {
    xcb_selection_notify_event_t *notify
      = (xcb_selection_notify_event_t*)nvt;
    if ( (notify->selection == CLIPBOARD)
        && (_xclb_process_notify(session->xclipboard, notify)) ) {
      PhxObject *obj = ui_active_focus_get();
      PhxInterface *iface = _interface_for(obj->i_mount->window);
        /* send to object, contents of clipboard is valid. */
      if (obj != NULL) {
        if (obj->_event_cb != NULL)
          return obj->_event_cb(iface, nvt, obj);
      }
      DEBUG_ASSERT(true, "failure: no handler for clipboard.");
    } else {
      DEBUG_ASSERT(true, "unhandled XCB_SELECTION_NOTIFY.");
    }
    return true;
  }

  if (response_type == XCB_CLIENT_MESSAGE) {
    DEBUG_ASSERT(true, "unhandled XCB_CLIENT_MESSAGE.");
  } else {
    DEBUG_ASSERT(true, "unhandled processing of _event_selection().");
  }
  return false;
}

static bool
_event_protocols(xcb_generic_event_t *nvt) {

  xcb_client_message_event_t *cm
    = (xcb_client_message_event_t*)nvt;

  PhxInterface *iface = _interface_for(cm->window);

  if (cm->data.data32[0] != WM_DELETE_WINDOW) {
    DEBUG_ASSERT(true, "unhandled WM_PROTOCOLS.");
    return false;
  }
  xcb_unmap_window(session->connection, cm->window);
    /* done to appear responsive, incase saving delays */
  xcb_flush(session->connection);
    /* send message to iface, allowing flatten if desired. */
    /* need iface raze and free, done during destroy */
  cairo_surface_destroy(iface->vid_buffer);
  xcb_destroy_window(session->connection, cm->window);
  iface->vid_buffer = NULL;

  return false;
}

static bool
_process_event(xcb_generic_event_t *nvt) {

  DEBUG_EVENTS("_process_event");

  switch (nvt->response_type & (uint8_t)0x7F) {

    case XCB_KEY_PRESS:           /* response_type 2 */
    case XCB_KEY_RELEASE: {       /* response_type 3 */
      _event_keyboard(nvt);
      break;
    }

    case XCB_BUTTON_PRESS:        /* response_type 4 */
    case XCB_BUTTON_RELEASE: {    /* response_type 5 */
      _event_mouse(nvt);
      break;
    }

    case XCB_MOTION_NOTIFY: {     /* response_type 6 */
      _event_motion(nvt);
      break;
    }

    case XCB_ENTER_NOTIFY: {      /* response_type 7 */
      _event_enter(nvt);
      break;
    }

    case XCB_LEAVE_NOTIFY: {      /* response_type 8 */
      _event_leave(nvt);
      break;
    }

    case XCB_FOCUS_IN:            /* response_type 9 */
    case XCB_FOCUS_OUT: {         /* response_type 10 */
      _event_focus(nvt);
      break;
    }

    case XCB_EXPOSE: {            /* response_type 12 */
      _event_expose(nvt);
      break;
    }

    case XCB_VISIBILITY_NOTIFY: { /* response_type 15 */
      _event_visibility(nvt);
      break;
    }

    case XCB_DESTROY_NOTIFY: {    /* response_type 17 */
        /* assume a kill, delete all windows (do their thing)
           send false to exit xcb_run() loop */
      xcb_destroy_notify_event_t *destroy
        = (xcb_destroy_notify_event_t*)nvt;
      if (_interface_remove_for(destroy->window) == 0) {
        free(nvt);
        return false;
      }
      break;
    }

      /* these 4 because of XCB_EVENT_MASK_STRUCTURE_NOTIFY */
    case XCB_UNMAP_NOTIFY: {      /* response_type 18 */
        /* Do not assume umappping is that of the topmost! */
      uint16_t sdx;
      xcb_map_notify_event_t *map = (xcb_map_notify_event_t*)nvt;
      PhxInterface *iface = _interface_for(map->event);
      iface->state &= ~SBIT_MAPPED;
      if ( ((sdx = session->ncount - 1) != 0)
          && (!((session->stack_order[sdx])->state & SBIT_MAPPED)) ) {
        do {
          iface = session->stack_order[(--sdx)];
          if (!!(iface->state & SBIT_MAPPED)) {
            _window_stack_topmost(iface);
            break;
          }
        } while (sdx != 0);
      }
      break;
    }

    case XCB_MAP_NOTIFY: {        /* response_type 19 */
      xcb_map_notify_event_t *map = (xcb_map_notify_event_t*)nvt;
      PhxInterface *iface = _interface_for(map->event);
      iface->state |= SBIT_MAPPED;
      _window_stack_topmost(iface);
      break;
    }

    case XCB_REPARENT_NOTIFY: {   /* response_type 21 */
        /* position after map, to design user requested of window
         * set once and when window manager asks to reparent
         * dont need during icon/deiconify
         * doesnt stop WM configures, but will be sent after map notify */
      xcb_reparent_notify_event_t *rp
        = (xcb_reparent_notify_event_t*)nvt;
      PhxInterface *iface = _interface_for(rp->window);

      uint32_t values[2];
      values[0] = iface->mete_box.x;
      values[1] = iface->mete_box.y;
      xcb_configure_window(session->connection, iface->window,
                   XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, values);

      if ( (!!session->has_WM)
          && (!!(iface->state & SBIT_UNDECORATED)) ) {
        struct MWMHints {
          uint32_t   flags, functions, decorations, input_mode, status;
        } hints = { 2, 0, 0, 0, 0 };
        xcb_change_property(session->connection, XCB_PROP_MODE_REPLACE,
                            rp->window, _MOTIF_WM_HINTS, _MOTIF_WM_HINTS, 32,
                            sizeof(hints) >> 2, &hints);
      }
      break;
    }
    case XCB_CONFIGURE_NOTIFY: {  /* response_type 22 */
      _event_configure(nvt);
      break;
    }

      /* needed for SELECTION == dnd, clipboard ??? */
    case XCB_PROPERTY_NOTIFY: {   /* response_type 28 */
      xcb_property_notify_event_t *prop
        = (xcb_property_notify_event_t*)nvt;
      if (prop->atom == CLIPBOARD) {
        if (prop->state == 0)
          _xclb_set_ownership(session->xclipboard, prop->window, prop->time);
      }
      break;
    }

      /* SELECTION == dnd, clipboard */
    case XCB_SELECTION_CLEAR:     /* response_type 29 */
    case XCB_SELECTION_REQUEST:   /* response_type 30 */
    case XCB_SELECTION_NOTIFY: {  /* response_type 31 */
      _event_selection(nvt);
      break;
    }

    case XCB_CLIENT_MESSAGE: {    /* response_type 33 */
      xcb_client_message_event_t *cm
        = (xcb_client_message_event_t*)nvt;
      if (cm->type != WM_PROTOCOLS)  _event_selection(nvt);
      else                           _event_protocols(nvt);
      break;
    }

    default:
      if (nvt->response_type == 0) {
        xcb_generic_error_t *err = (xcb_generic_error_t*)nvt;
        printf("error: %"PRIu8"\n", err->error_code);
        break;
      }
        /* Unknown event type, use to track coding problems */
      printf("Unknown event: %"PRIu8"\n", nvt->response_type);
      break;

  } /* end switch(response_type) */
    /* nvt no longer needed */
  free(nvt);
    /* critical! otherwise wont process anything done */
  xcb_flush(session->connection);
  return true;
}

void
xcb_main(void) {

  xcb_connection_t *connection = session->connection;

    /* Guard against running main without a purpose. */
  if (session->ncount == 0) {
    DEBUG_ASSERT(true, "error: nothing for xcb_main() to do.");
    return;
  }
    /* Test if Window manager exists. Might even be us? */
  if ( (session->has_WM == 0)
      && (_MOTIF_WM_HINTS != XCB_ATOM_NONE) ) {
    xcb_screen_t *screen;
    xcb_get_window_attributes_cookie_t c0;
    xcb_get_window_attributes_reply_t *r0;
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    c0 = xcb_get_window_attributes(connection, screen->root);
    r0 = xcb_get_window_attributes_reply(connection, c0, NULL);
    session->has_WM =
       !!(r0->all_event_masks & XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
    free(r0);
  }

  if ( (CURSOR_DEFAULT != NULL) || (session->has_WM == 0) ) {
    const char *named
      = (CURSOR_DEFAULT != NULL) ? CURSOR_DEFAULT : "left_ptr";
    ui_cursor_initialize(named);
  }

    /* application-wide clipboard */
  session->xclipboard = xclb_initialize(connection);
#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
  session->idndserver = _dnd_initialize();
#endif
#if DND_EXTERNAL_ON
    /* application-wide window dnd */
  session->xdndserver = xdnd_initialize(connection, xdnd_status_event);
#endif
    /* set all before through to Xserver */
  xcb_flush(connection);

  do {
    int timeout;
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(connection)) != NULL)
        /* _process_event() returns 'running' for main loop. */
      if (_process_event(event) == false)  goto cleanup;
    do {
        /* _process_timers() runs any timers that are immediately pending,
           and returns the number of milliseconds until the next timer
           in the queue is ready.
           _process_timers() returns INT_MAX if there are no timers. */
        /* _process_timers() also removes from timeout its run time. */
      timeout = _process_timers();
      if ((event = xcb_poll_for_queued_event(connection)) == NULL)  break;
        if (_process_event(event) == false)  goto cleanup;
    } while (1);
    if (timeout > 6) {
      struct pollfd pfd[1];
      pfd->fd = xcb_get_file_descriptor(connection);
      pfd->events = POLLIN;
      pfd->revents = POLLOUT;
      poll(pfd, 1, timeout);
    }
  } while (1);
    /* received XCB_DESTROY_NOTIFY */
cleanup:
  return;
}
