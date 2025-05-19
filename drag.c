#include "drag.h"
#include "windows.h"
#include "objects.h"

#define DND_DEBUG 0
#if DND_DEBUG
 static bool status_reply = false;
 #define DND_DEBUG_PUTS(a) puts((a))
 #define DND_DEBUG_PRINT(a,b) \
   printf("%-26s  target %d\n", (a), (b))
 #define DND_DEBUG_CURSOR(a) puts((a))
 #define DND_DEBUG_IGNORE(a,b,c) \
   printf("ignore %-13s reply, server target %d != status target   %d\n", \
                         (a), (b), (c))
 #define DND_DEBUG_INTERNAL(a,b) \
   printf("%-26s  target %p\n", (a), (void*)(b))
#else
 #define DND_DEBUG_PUTS(a)
 #define DND_DEBUG_PRINT(a,b)
 #define DND_DEBUG_CURSOR(a)
 #define DND_DEBUG_IGNORE(a,b,c)
 #define DND_DEBUG_INTERNAL(a,b)
#endif

/* rare cases from events.c */
PhxObject *      _get_object_at_pointer(PhxInterface *iface,
                                        int16_t x,
                                        int16_t y);
PhxInterface *   _coordinates_for_object(PhxInterface *iface,
                                         void *nvt,
                                         PhxObject *obj,
                                         int16_t *xPtr,
                                         int16_t *yPtr);
bool             _enter_leave_notices(PhxInterface *iface,
                                      xcb_generic_event_t *nvt,
                                      PhxObject *obj,
                                      bool type);
bool             _within_event(PhxInterface *iface,
                               xcb_generic_event_t *nvt,
                               PhxObject *obj);

#pragma mark *** Drawing ***

  /* XXX TODO a dash-lined rectangle for selection */


#pragma mark *** Keyboard ***

bool
_drag_keyboard(PhxInterface *iface, xcb_generic_event_t *nvt) {

  xcb_key_press_event_t *kp = (xcb_key_press_event_t*)nvt;
  xcb_keysym_t keyval       = _xcb_keysym_for(kp->detail, kp->state);
  bool locus = ((kp->response_type & (uint8_t)0x7F) == XCB_KEY_PRESS);
  const char *cursor;

  PhxObject *has_drag = ui_active_drag_get();

  if (keyval == 0xFF1B) {
    if (has_drag != NULL) {
      iface->state |= SBIT_RELEASE_IGNORE;
      if (!locus) {
       #if DND_EXTERNAL_ON
        if (xdndActivated_get(session->xdndserver))
          xdnd_drag_cancel(session->xdndserver);
       #endif
        if (has_drag != ui_active_within_get()) {
            /* within stays the same */
          PhxObject *obj = ui_active_within_get();
          _enter_leave_notices(iface, nvt, obj, 0);
        }
        has_drag->_event_cb(iface, nvt, NULL);
        DND_DEBUG_PUTS("ui_active_drag_set(NULL) _drag_keyboard()");
        ui_active_drag_set(NULL);
      }
      return true;
    }
   #if DND_EXTERNAL_ON
    if (xdndActivated_get(session->xdndserver)) {
      if (!locus)  xdnd_drag_cancel(session->xdndserver);
      return true;
    }
   #endif
    return false;
  }

  if (   (keyval != 0x0ffe3)                       /* XK_Control_L */
      && (keyval != 0x0ffe4) )                     /* XK_Control_R */
    return false;

  if (has_drag == NULL)  return false;

  cursor = ui_cursor_get_named();
  if ( (cursor == NULL) || (strcmp(cursor, "dnd-no-drop") == 0) )
    return true;

  if (locus)  kp->state |= XCB_KEY_BUT_MASK_CONTROL;
  else        kp->state &= ~XCB_KEY_BUT_MASK_CONTROL;
#if DND_INTERNAL_ON
  session->idndserver->owner_state = kp->state;
#endif
#if DND_EXTERNAL_ON
  session->xdndserver->xdndSource.source_state = kp->state;
#endif

  if (ui_active_within_get() != NULL) {
      /* initial setup, not implemented */
    locus ^= (has_drag != ui_active_within_get());
    if (locus) {
      (ui_active_within_get())->state |= OBIT_DND_COPY;
      DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-copy)  _drag_keyboard()");
      ui_cursor_set_named("dnd-copy", iface->window);
    } else {
      (ui_active_within_get())->state &= ~OBIT_DND_COPY;
      DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-move)  _drag_keyboard()");
      ui_cursor_set_named("dnd-move", iface->window);
    }
  }
#if DND_EXTERNAL_ON
    else {
    if (session->xdndserver->xdndSource.sa_count > 1) {
      if (!locus) {
        DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-copy)  _drag_keyboard()");
        ui_cursor_set_named("dnd-copy", iface->window);
      } else {
        DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-move)  _drag_keyboard()");
        ui_cursor_set_named("dnd-move", iface->window);
      }
    }
  }
#endif
  return true;
}

#pragma mark *** Mouse ***

bool
_drag_selection_box(PhxInterface *iface, xcb_generic_event_t *nvt) {

/* should this be setting within? relevant??? */

  /* Setup do nothing for now, assign has_drag.
   Reminder to use root_xy, window values changed. */
puts("_drag_selection_box");
  ui_active_drag_set((PhxObject*)iface);
  return true;
}

/* bp-motion event. Suppliment to main_loop().
  Not intended to be called when target of DND.
  Only handles button 1 motion. */
bool
_drag_motion(PhxInterface *iface, xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_motion_notify_event_t *motion;
  PhxObject *has_drag = ui_active_drag_get();

    /* Interface-type special handing. */
  if (has_drag != NULL) {
      /* Currently a button, but should user elect PhxNexus dragging.
        Uncertainty on case of possible external region dragging. */
    if ((iface->state & SBIT_HBR_DRAG) != 0)
      return has_drag->_event_cb(iface, nvt, obj);
    if (IS_IFACE_TYPE(has_drag)) {
      if (has_drag->type == PHX_GFUSE)
        return has_drag->_event_cb(iface, nvt, obj);
      return _drag_selection_box(iface, nvt);
    }
  }

  motion = (xcb_motion_notify_event_t*)nvt;

#if DND_EXTERNAL_ON
    /* -m "Remove excessive work on non-src drags." */
  if ( (has_drag != NULL)
      && (has_drag->dnd_quirk != NULL) ) {
    if ( !xdndActivated_get(session->xdndserver)
        && (session->xdndserver->xdndSource.source == 0) )
        /* This does not start server, just loads source drag info. */
      xdnd_quirk_src_load(session->xdndserver, iface->window,
                                         has_drag->dnd_quirk);
    xdnd_drag_motion(session->xdndserver, motion);
    if (xdndActivated_get(session->xdndserver))
      return true;
  }
#endif

    /* Occurs on a drag_selection(!has_drag) or has_drag */
  if (obj == NULL) {
      /* Most likely scenario. We become source for multi-window. */
    if (has_drag != NULL) {
      DEBUG_ASSERT((ui_active_within_get() != NULL),
             "failure: LEAVE didn't set within correctly... _drag_motion().");
      return has_drag->_event_cb(iface, nvt, obj);
    }
  }
    /* Even if drag, objects need to be informed of enter/leave */
  if (ui_active_within_get() != obj)
    _within_event(iface, nvt, obj);
    /* Case: drag cancel
      has_drag == NULL, SBIT_RELEASE_IGNORE is set
      want within updates, and exit jump to setting points. */
  if ( (has_drag == NULL) && ((iface->state & SBIT_RELEASE_IGNORE) != 0) )
    return true;

  if (has_drag != NULL) {
    int16_t *xPtr, *yPtr;
      /* Assumes when an object, instead of iface, initiated
       ownership of has_drag, it knows coordinates or in window terms.. */
    xPtr = &motion->event_x;
    yPtr = &motion->event_y;
    _coordinates_for_object(iface, nvt, obj, xPtr, yPtr);
    return has_drag->_event_cb(iface, nvt, obj);
  }

    /* Ownership test: No _event_cb or master should block. */
  if (obj != NULL)
    if ( (obj->type == PHX_IFACE)
        || ( (!IS_IFACE_TYPE(obj)) && ((obj->state & OBIT_DND_AWARE) == 0) )
        || (obj->_event_cb == NULL) ) {
        /* Special: headerbar allows drag move/resize. */
      if (obj->i_mount->type != PHX_HEADERBAR)
        return _drag_selection_box(iface, (xcb_generic_event_t*)motion);
    }
  return false;
}

bool
_drag_finish(PhxInterface *iface, xcb_generic_event_t *nvt, PhxObject *obj) {

  PhxObject *has_drag = ui_active_drag_get();
  if (has_drag == (PhxObject*)iface) {
      /* Special case where blocked others from ownership. */
    DND_DEBUG_CURSOR("ui_cursor_set_named(NULL)  _drag_finish()");
    ui_cursor_set_named(NULL, iface->window);
    DND_DEBUG_PUTS("ui_active_drag_set(NULL) _drag_finish()");
    ui_active_drag_set(NULL);
    return true;
  }

  if ( (has_drag->type == PHX_GFUSE)
      || ((iface->state & SBIT_HBR_DRAG) != 0) )
    return has_drag->_event_cb(iface, nvt, obj);

  if (obj != NULL) {
    if ((obj->state & OBIT_DND_AWARE) != 0) {
        /* 2 cases: in object that has drag, in object other than has_drag */
        /* handle 'other than has_drag', fall thru on has_drag */
      if (obj != has_drag) {
          /* Do not convert event to object's i_mount */
        if (obj->_event_cb != NULL)
          if (obj->_event_cb(iface, nvt, obj))  return true;
          /* fall-thru letting has_drag handle finish */
      } else {
        if (obj->_event_cb(iface, nvt, obj))  return true;
      }
    }
  }
 #if DND_EXTERNAL_ON
  if (obj == NULL) {
    xdnd_drag_drop(session->xdndserver);
    return true;
  }
 #endif
    /* Reset or drop finish for has_drag */
  if (!has_drag->_event_cb(iface, nvt, obj)) {
    DEBUG_ASSERT((obj != ui_active_within_get()),
                       "failure: within object not correctly set.");
  }
  DND_DEBUG_PUTS("ui_active_drag_set(NULL) _drag_finish()");
  ui_active_drag_set(NULL);
  return true;
}

#pragma mark *** DNDI ***

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)

xcb_idndserver_t *
_dnd_initialize(void) {

  xcb_idndserver_t *dndserver;
  dndserver = malloc(sizeof(xcb_idndserver_t));
  memset(dndserver, 0, sizeof(xcb_idndserver_t));
  return dndserver;
}

static uint8_t
dnd_get_twa_count(PhxObject *obj) {
  if ( (obj != NULL) && (obj->dnd_quirk != NULL) )
    return obj->dnd_quirk->watypes_count;
  return 0;
}

static xcb_atom_t *
dnd_get_twa(PhxObject *obj) {
  if ( (obj != NULL) && (obj->dnd_quirk != NULL) )
    return obj->dnd_quirk->watypes;
  return NULL;
}

/*
   An object can accept multiple types from a source. Once
  source-target are in agreement, 'accepted' will stay accepted
  as long as within the object's draw_box region. The margin area
  is consider a no drop region.
   Logic behind idndState:
  'owner' will always be the same once drag started. 'within' will
  change after leaving 'owner'. 'within' can even change more than
  once during the drag. On entry to a 'within', we must agree upon
  what 'owner' can send and 'within' can receive. This agreement
  lasts the entirety of 'within''s content or draw_box.
   If no agreement, a no fly zone is set as if it's not XdndAware.
*/
bool
_dnd_status(xcb_dnd_notify_event_t *dnd) {

  PhxRectangle mbox, dbox;
  xcb_atom_t accepted = dnd->within_type;
  PhxInterface *iface = (PhxInterface*)dnd->within;
  if (!IS_IFACE_TYPE(iface))  iface = iface->i_mount;

  mbox.y = (mbox.x = 0);
  mbox.w = dnd->within->mete_box.w;
  mbox.h = dnd->within->mete_box.h;
  dbox = dnd->within->draw_box;
  if ( (mbox.x != dbox.x) || (mbox.y != dbox.y)
      || (mbox.w != dbox.w) || (mbox.h != dbox.h) ) {
      /* Requesting type based on location. Coorindates in object space.
        Need to consider no drop areas like margins. */
    dbox.w += dbox.x;
    dbox.h += dbox.y;
    if (   (dnd->within_x < dbox.x)  || (dnd->within_y < dbox.y)
        || (dnd->within_x >= dbox.w) || (dnd->within_y >= dbox.h) ) {
      if (accepted != XCB_NONE) {
        DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-no-drop) _dnd_status()");
        ui_cursor_set_named("dnd-no-drop", iface->window);
      }
      dnd->within_type = (accepted = XCB_NONE);
    } else if (accepted == XCB_NONE)
      accepted = XDND_ENTER;
      /* Since probable call is drawing update based on object space,
        drawing occurs with drawing space 0,0 translation. */
    dnd->within_x -= dbox.x;
    dnd->within_y -= dbox.y;
  }

  if (accepted == XDND_ENTER) {
    uint8_t wdx, wcount, odx, ocount;
    xcb_atom_t *owner_types  = dnd_get_twa(dnd->owner);
    xcb_atom_t *within_types = dnd_get_twa(dnd->within);
    wcount = dnd_get_twa_count(dnd->within);
    ocount = dnd_get_twa_count(dnd->owner);
    accepted = within_types[0];
    for (wdx = 0; wdx < wcount; wdx++)
      for (odx = 0; odx < ocount; odx++)
        if (within_types[wdx] == owner_types[odx]) {
          accepted = within_types[wdx];
          break;
        }
    dnd->within_type = accepted;
      /* Should only occur if source doesn't have what we want.
        Should be status return then of no fly zone. Also no draw_caret. */
    if (accepted == XCB_NONE) {
      DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-no-drop) _dnd_status()");
      ui_cursor_set_named("dnd-no-drop", iface->window);
      return false;
    } else {
      dnd->within->state |= OBIT_DND_COPY;
      if ( (dnd->owner_actions == 0)
          || ((dnd->owner_state & XCB_KEY_BUT_MASK_CONTROL) == 0) ) {
        DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-copy) _dnd_status()");
        ui_cursor_set_named("dnd-copy", iface->window);
      } else {
        DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-move) _dnd_status()");
        ui_cursor_set_named("dnd-move", iface->window);
      }
    }
  }
    /* user adjustments to display of object */
  if (dnd->within->_obj_status_cb == NULL)
    return false;
  return dnd->within->_obj_status_cb(dnd);
}

void
_dnd_drop(xcb_dnd_notify_event_t *dnd) {

  PhxInterface *iface;
  xcb_selection_data_t sdata = { 0 };
  xcb_motion_notify_event_t motion = { 0 };

    /* request last status update */
  _dnd_status(dnd);

  if (dnd->within_type != XCB_NONE) {
      /* get data from owner */
    sdata.type = dnd->within_type;
    dnd->owner->dnd_quirk->_sel_get_cb(&sdata);
      /* send data to within, acts as notify leave also */
    dnd->within->dnd_quirk->_sel_set_cb(&sdata);
      /* selection data used and invalid for next drag */
    if (sdata.data != NULL)
      free(sdata.data);
      /* implement source action, for move, source needs to delete. */
    if (dnd->owner->dnd_quirk->_sel_act_cb != NULL)
      dnd->owner->dnd_quirk->_sel_act_cb(dnd->owner_state);
  }

  dnd->within->state &= ~(OBIT_DND_CARET | OBIT_DND_COPY | OBIT_DND);
  ui_invalidate_object(dnd->within);

    /* resend an ENTER to reset cursor/state (window coordinates) */
  motion.event_x = dnd->within_x + dnd->within->i_mount->mete_box.x;
  motion.event_y = dnd->within_y + dnd->within->i_mount->mete_box.y;

  iface = _interface_for((dnd->within->i_mount)->window);
  _enter_leave_notices(iface, (xcb_generic_event_t*)&motion, dnd->within, 1);
}

bool
_send_dnd_event(xcb_idndserver_t *iserv,
                xcb_generic_event_t *nvt,
                xcb_atom_t event_type) {

  PhxInterface *iface, *mount;
  xcb_motion_notify_event_t *motion;
  int16_t x, y;

  if (iserv->owner == NULL) {
    iserv->owner   = ui_active_drag_get();
      /* external default is copy only */
    iserv->owner_actions = 0;
    if (iserv->owner != NULL)
      iserv->owner_actions = iserv->owner->dnd_quirk->src_actions;
    goto set_within;
  } else if (iserv->within != ui_active_within_get()) {
set_within:
    DND_DEBUG_INTERNAL("XDND_ENTER", ui_active_within_get());
    iserv->within  = ui_active_within_get();
      /* atom to signal MUST set within_type */
    iserv->within_type = XDND_ENTER;
  }

  motion = (xcb_motion_notify_event_t*)nvt;
  iserv->owner_state = motion->state;

    /* begin translation of coordinates */
  mount = (PhxInterface*)iserv->within;
  if (!IS_IFACE_TYPE(mount))  mount = mount->i_mount;
  iface = _interface_for(mount->window);
    /* event_x, event_y are in has_drag coordinates */
    /* but rather than use, calculate within's using root. */
  x = motion->root_x - iface->mete_box.x;
  y = motion->root_y - iface->mete_box.y;
  if (mount->type != PHX_IFACE)
    do
      x -= mount->mete_box.x, y -= mount->mete_box.y;
    while ((mount = mount->i_mount)->type != PHX_IFACE);
    /* x, y are now in nexus space, set to object space */
  iserv->within_x = x - iserv->within->mete_box.x;
  iserv->within_y = y - iserv->within->mete_box.y;
    /* end translation of coordinates */

  if ( (event_type == XDND_STATUS) || (event_type == XDND_POSITION) )
    return _dnd_status(iserv);

  _dnd_drop(iserv);
  DND_DEBUG_INTERNAL("XDND_FINISHED", ui_active_within_get());
  memset(session->idndserver, 0, sizeof(xcb_dnd_notify_event_t));
  return true;
}

/* Not part of XDNDServer, but a must call-back.
  void (*_xdnd_status_cb)(xcb_window_t, xcb_client_message_data_t *))
  Translates XdndPosition coordinates, allows window to update needed,
  and returns necessary XdndStatus reply data.
  'param' sent from XDNDServer only sends data32[2] position,
  rest are zeroed. Expects a qualified XdndStaus data_t returned for
  its status reply. */
void
xdnd_status_event(xcb_window_t window, xcb_client_message_data_t *param) {

  PhxInterface *iface, *mount;
  PhxObject *obj;
  xcb_motion_notify_event_t motion = { 0 };
  int16_t x, y, *xPtr, *yPtr;

    /* root position coordinates, data[2] only viable input. */
  x = (uint16_t)((uint32_t)param->data32[2] >> 16);
  y = (uint16_t)((uint32_t)param->data32[2]);
  motion.root_x = x;
  motion.root_y = y;

  motion.response_type = XCB_MOTION_NOTIFY;
  motion.event = window;
  if ((iface = _interface_for(window)) == NULL) {
    DEBUG_ASSERT(true, "failure: Not our window. xdnd_status_event()");
    return;
  }
    /* convert global to window coordinates */
  motion.event_x = (x -= iface->mete_box.x);
  motion.event_y = (y -= iface->mete_box.y);
    /* get object under window coordinates */
  obj = _get_object_at_pointer(iface, x, y);

  if (ui_active_within_get() != obj)
    _within_event(iface, (xcb_generic_event_t*)&motion, obj);

  if (IS_IFACE_TYPE(obj)) {
      /* At this time, interfaces aren't dragable receivers. */
      /* no type, no acceptance and no action */
      /* Can ignore other entries */
    return;
  }

    /* Two major checks, is object dnd aware, and did user connect to events */
    /* Possibly may not use our XDNDServer, so don't assume 'do_not_disturb' */
  if ( ((obj->state & OBIT_DND_AWARE) != 0)
      && (obj->_event_cb != NULL) ) {
    bool want_updates;
    xcb_idndserver_t *iserv = session->idndserver;
      /* Create motion, uses root_x, root_y. Expects send_event to convert. */
    want_updates = _send_dnd_event(iserv,
                                   (xcb_generic_event_t*)&motion, XDND_STATUS);
    param->data32[0] = iserv->within_type;
    if (iserv->within_type != XCB_NONE) {
      param->data32[1] = 1;
        /* only copy for targets */
      param->data32[4] = XDND_ACTIONCOPY;
    }
    if (want_updates)  return;
      /* fall-thru if don't need updates of position */
  }
  DND_DEBUG_INTERNAL("XDND_ENTER", ui_active_within_get());
    /* do no disturb, since not a dnd aware object, or none acceptable types. */
    /* must be in root coordinates */
  param->data16[4] = obj->mete_box.x;
  param->data16[5] = obj->mete_box.y;
  param->data16[6] = obj->mete_box.w;
  param->data16[7] = obj->mete_box.h;
  xPtr = (int16_t*)&param->data16[4];
  yPtr = (int16_t*)&param->data16[5];
  if ((mount = obj->i_mount)->type != PHX_IFACE)
    do
      *xPtr += mount->mete_box.x, *yPtr += mount->mete_box.y;
    while ((mount = mount->i_mount)->type != PHX_IFACE);
  *xPtr += mount->mete_box.x, *yPtr += mount->mete_box.y;
  return;
}

#endif /* (DND_INTERNAL_ON || DND_EXTERNAL_ON) */

#if DND_EXTERNAL_ON

bool
_dnd_selection_event(xcb_generic_event_t *nvt) {

/* PhxInterface *iface; */
  uint8_t response_type = (nvt->response_type & (uint8_t)0x7F);
  switch (response_type) {
    case XCB_SELECTION_CLEAR: {
      xcb_selection_clear_event_t *clear
        = (xcb_selection_clear_event_t*)nvt;
      if (clear->selection == XDND_SELECTION) {
        xdnd_selection_clear(session->xdndserver);
        return true;
      }
      break;
    }
    case XCB_SELECTION_REQUEST: {
      xcb_selection_request_event_t *request
        = (xcb_selection_request_event_t*)nvt;
      if (request->selection == XDND_SELECTION) {
        xdnd_process_selection(session->xdndserver, request);
        return true;
      }
      break;
    }
    case XCB_SELECTION_NOTIFY: {
      xcb_selection_notify_event_t *notify
        = (xcb_selection_notify_event_t*)nvt;
      if (notify->property == XDND_SELECTION) {
        xdnd_process_selection(session->xdndserver, NULL);
        return true;
      }
      break;
    }
    case XCB_CLIENT_MESSAGE: {
      xcb_client_message_event_t *cm
        = (xcb_client_message_event_t*)nvt;

        /* The processing of XdndLeave. For external source drags,
         XdndLeave is considered shutdown of machine. For mult-window
         application, XdndLeave is the exiting of a target. */
      if (cm->type == XDND_LEAVE) {
          /* send an XCB_LEAVE_NOTIFY to 'within' */
        PhxInterface *iface = _interface_for(cm->window);
        DND_DEBUG_INTERNAL("XCB_LEAVE_NOTIFY", ui_active_within_get());
        _enter_leave_notices(iface, nvt, ui_active_within_get(), 0);
        DND_DEBUG_PUTS("ui_active_within_set(NULL) _dnd_selection_event()");
        ui_active_within_set(NULL);
          /* Check if we own source. */
        if (_interface_for(cm->data.data32[0]) != NULL) {
          DND_DEBUG_PRINT("received XDND_LEAVE,", cm->window);
          return true;
        }
      }
      if (xdnd_process_message(session->xdndserver, cm)) {
          /* XdndFinished is when we end setting to no drag state */
        DND_DEBUG_PUTS("ui_active_drag_set(NULL) _dnd_selection_event()");
          /* Base on drop accepted, set topmost */
        if (ui_active_within_get() != NULL) {
          PhxObject *within = ui_active_within_get();
          if (_window_for(within) != cm->window)
            _window_stack_topmost(_interface_for(_window_for(within)));
        } else {
          PhxObject *has_drag = ui_active_drag_get();
          _window_stack_topmost(_interface_for(_window_for(has_drag)));  
        }
        ui_cursor_set_named(NULL, cm->window);
        ui_active_drag_set(NULL);
      }
      return true;
    }
    default:
      break;
  }
  return false;
}
#endif /* DND_EXTERNAL_ON */

#pragma mark *** DNDX ***

#if DND_EXTERNAL_ON

/* Current state/design of this code. */
static const uint32_t XDND_PROTOCOL_VERSION = 5;

#if DND_DEBUG
/* strictly for debugging */
static bool position_request = true;
#endif

/* ISO C90 doesn't handle fields */
/* Originally used source with outside variables to determine activated.
  Code worked, but seemed to limit abilities and make code more complicated. */
/* defines look too much like function pointers. Making them functions. */
/* All but xdndActivated_get() intended as internal(static), but may need? */

__inline uint8_t
xdndVersion_get(xcb_xdndserver_t *dserv) {
  return (uint8_t)(dserv->xdndState.state & (uint16_t)0x0FF);
}

__inline void
xdndVersion_set(xcb_xdndserver_t *dserv, uint8_t value) {
  dserv->xdndState.state
    = (dserv->xdndState.state & (uint16_t)~0x0FF) | (value & (uint16_t)0x0FF);
}

__inline bool
xdndActivated_get(xcb_xdndserver_t *dserv) {
  return !!(dserv->xdndState.state & (uint16_t)0x100);
}

__inline void
xdndActivated_set(xcb_xdndserver_t *dserv, bool set) {
  dserv->xdndState.state
    = (dserv->xdndState.state & (uint16_t)~0x100) | (set << 8);
}

__inline bool
xdndExchanges_get(xcb_xdndserver_t *dserv) {
  return !!(dserv->xdndState.state & (uint16_t)0x200);
}

__inline void
xdndExchanges_set(xcb_xdndserver_t *dserv, bool set) {
  dserv->xdndState.state
    = (dserv->xdndState.state & (uint16_t)~0x200) | (set << 9);
}

__inline bool
xdndEnclose_get(xcb_xdndserver_t *dserv) {
  return !!(dserv->xdndState.state & (uint16_t)0x400);
}

__inline void
xdndEnclose_set(xcb_xdndserver_t *dserv, bool set) {
  dserv->xdndState.state
    = (dserv->xdndState.state & (uint16_t)~0x400) | (set << 10);
}

__inline bool
xdndIOchange_get(xcb_xdndserver_t *dserv) {
  return !!(dserv->xdndState.state & (uint16_t)0x0800);
}

__inline void
xdndIOchange_set(xcb_xdndserver_t *dserv, bool set) {
  dserv->xdndState.state
    = (dserv->xdndState.state & (uint16_t)~0x0800) | (set << 11);
}

__inline bool
xdndDropable_get(xcb_xdndserver_t *dserv) {
  return !!(dserv->xdndState.state & (uint16_t)0x1000);
}

__inline void
xdndDropable_set(xcb_xdndserver_t *dserv, bool set) {
  dserv->xdndState.state
    = (dserv->xdndState.state & (uint16_t)~0x1000) | (set << 12);
}
    /* Viewable drop code. */
__inline bool
xdndRaised_get(xcb_xdndserver_t *dserv) {
  return !!(dserv->xdndState.state & (uint16_t)0x2000);
}

__inline void
xdndRaised_set(xcb_xdndserver_t *dserv, bool set) {
  dserv->xdndState.state
    = (dserv->xdndState.state & (uint16_t)~0x2000) | (set << 13);
}

xcb_xdndserver_t *
xdnd_initialize(xcb_connection_t *connection,
        void (*_xdnd_status_cb)(xcb_window_t, xcb_client_message_data_t *)) {

  xcb_xdndserver_t *dndserver;

  dndserver = malloc(sizeof(xcb_xdndserver_t));
  memset(dndserver, 0, sizeof(xcb_xdndserver_t));
  dndserver->connection = connection;
  dndserver->_xdnd_status_cb = _xdnd_status_cb;
  return dndserver;
}

void
xdnd_window_awareness(xcb_connection_t *connection, xcb_window_t window) {

    /* Add XdndAware property */
  xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                      window,
                      XDND_AWARE, XCB_ATOM_ATOM,
                      sizeof(uint32_t) * 8,
                      1, &XDND_PROTOCOL_VERSION);
}

    /* Viewable drop code. */
/* The connection c and the window win are supposed to be defined*/
static void
_raise_target_window(xcb_connection_t *connection, xcb_window_t window) {
  const static uint32_t values[] = { XCB_STACK_MODE_ABOVE };
  xcb_configure_window(connection, window,
                         XCB_CONFIG_WINDOW_STACK_MODE, values);
}

static void
_xdnd_shutdown(xcb_xdndserver_t *dserv) {

  xcb_connection_t *connection;
  void (*_xdnd_status_cb)(xcb_window_t, xcb_client_message_data_t *);
  DND_DEBUG_PUTS("returned to source... shutting down server.");
  connection      = dserv->connection;
  _xdnd_status_cb = dserv->_xdnd_status_cb;
  if (dserv->xdnddata.data != NULL)
    free(dserv->xdnddata.data);
  if (dserv->xdndSource.source_offer != NULL)
   free(dserv->xdndSource.source_offer);
  memset(dserv, 0, sizeof(xcb_xdndserver_t));
  dserv->connection      = connection;
  dserv->_xdnd_status_cb = _xdnd_status_cb;
}

/* currently only action copy between windows */
static void
_xdnd_cursor_event(xcb_xdndserver_t *dserv,
                   xcb_window_t window,
                   xcb_atom_t action) {

  if (action == XCB_NONE) {
    DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-no-drop) xdnd_cursor_event()");
    ui_cursor_set_named("dnd-no-drop", window);
  } else {
    if ((dserv->xdndSource.source_state & XCB_MOD_MASK_CONTROL) != 0) {
      DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-move) xdnd_cursor_event()");
      ui_cursor_set_named("dnd-move", window);
    } else {
      DND_DEBUG_CURSOR("ui_cursor_set_named(dnd-copy) xdnd_cursor_event()");
      ui_cursor_set_named("dnd-copy", window);
    }
  }
}

/* We are source, external targets get this information through
 X Server via xcb_get_property_reply(XDND_TYPELIST). We set this
 up when our internal drag first becomes an external one.
   As far as data_in/out info, currently just textual so no conversion
 needed, just buffers to move data from/to source/target. Desire to
 later add conversion routines to XDNDServer where internal can access
 and fulfill target's requests. */
/* when server starts up, want object's handlers to have
  as if server could drop on itself, being source and target */
/* WRONG! */

bool
xdnd_quirk_src_load(xcb_xdndserver_t *dserv,
                    xcb_window_t owner,
                    xcb_dnd_property_t *quirk) {
  XDNDSorceData *sdata;
  uint8_t adx, wacnt, bits;

  DND_DEBUG_PRINT("xdnd_quirk_src_load()", owner);

  if (quirk == NULL) {
    dserv->xdnddata_in  = NULL;
    dserv->xdnddata_act = NULL;
    return false;
  }

  dserv->xdnddata_in = quirk->_sel_get_cb;
    /* Required by XdndEnter, both a count flag and first 3 entries */
  sdata = &dserv->xdndSource;
  sdata->src_entermsg.data32[0] = owner;
  sdata->src_entermsg.data32[1] = XDND_PROTOCOL_VERSION << 24;
  sdata->src_entermsg.data32[1] |= ((wacnt = quirk->watypes_count) > 3);
  if (wacnt > 3)  wacnt = 3;
  sdata->src_entermsg.data32[2] = XCB_NONE;
  sdata->src_entermsg.data32[3] = XCB_NONE;
  sdata->src_entermsg.data32[4] = XCB_NONE;
  for (adx = 0; adx < wacnt; adx++)
    sdata->src_entermsg.data32[(adx + 2)] = quirk->watypes[adx];
    /* May be used with sub-windows, or user defined?
      Set up just like an external owner. */
  sdata->so_count = (wacnt = quirk->watypes_count);
  sdata->source = owner;
  sdata->source_offer = malloc(sizeof(xcb_atom_t) * wacnt);
  memmove(sdata->source_offer, quirk->watypes,
                                         (sizeof(xcb_atom_t) * wacnt));

    /* Update XdndTypeList requardless of count. Possible XDND could
      get rid of XdndEnter requirement. */
  xcb_change_property(dserv->connection, XCB_PROP_MODE_REPLACE,
                      owner,
                      XDND_TYPELIST, XCB_ATOM_ATOM,
                      sizeof(xcb_atom_t) * 8,
                      sizeof(xcb_atom_t) * quirk->watypes_count,
                      quirk->watypes);

  dserv->xdnddata_act = quirk->_sel_act_cb;
  sdata->source_action = malloc(sizeof(xcb_atom_t) * 5);
  memset(sdata->source_action, 0, (sizeof(xcb_atom_t) * 5));
  adx = 0;
  bits = quirk->src_actions;
  do {
      sdata->source_action[adx] = XDND_ACTIONCOPY, adx++;
    if (bits == 0)  break;
    if ((bits & 1) != 0)
      sdata->source_action[adx] = XDND_ACTIONMOVE, adx++, bits >>= 1;
    if (bits == 0)  break;
    if ((bits & 1) != 0)
      sdata->source_action[adx] = XDND_ACTIONLINK, adx++, bits >>= 1;
    if (bits == 0)  break;
    if ((bits & 1) != 0)
      sdata->source_action[adx] = XDND_ACTIONASK, adx++, bits >>= 1;
    if (bits == 0)  break;
    if ((bits & 1) != 0)
      sdata->source_action[adx] = XDND_ACTIONPRIVATE, adx++, bits >>= 1;
    DEBUG_ASSERT((bits != 0), "failure: XdndAction not supported.");
    break;
  } while (1);
  sdata->sa_count = adx;
    /* Add XdndActionList property */
  xcb_change_property(dserv->connection, XCB_PROP_MODE_REPLACE,
                      owner,
                      XDND_ACTIONLIST, XCB_ATOM_ATOM,
                      sizeof(uint32_t) * 8,
                      sizeof(xcb_atom_t) * adx,
                      (char*)sdata->source_action);
  xcb_flush(dserv->connection);
  return false;
}

/* This should, once evolved, be a XDNDTargetData. */
bool
xdnd_quirk_dst_load(xcb_xdndserver_t *dserv,
                    xcb_window_t owner,
                    xcb_dnd_property_t *quirk) {

  DND_DEBUG_PRINT("xdnd_quirk_dst_load()", owner);
  if (dserv->xdndSource.source == owner) {
      /* Attempt to drop on self. Revert to internal handling. */
      /* Viewable drop code. */
    _raise_target_window(dserv->connection, owner);

    _xdnd_shutdown(dserv);
    return true;
  }
  dserv->xdnddata_out = (quirk != NULL) ? quirk->_sel_set_cb : NULL;
  return false;
}

/* Used when we are the source.
   Sends XdndDrop, XdndEnter, XdndPosition, XdndLeave messages to target. */
static void
_xdnd_send_event(xcb_xdndserver_t *dserv,
                 xcb_motion_notify_event_t *motion,
                 xcb_atom_t event_type) {

  xcb_client_message_event_t *message = calloc(32, 1);

  message->response_type  = XCB_CLIENT_MESSAGE;
  message->format         = 32;
  message->window         = dserv->xdndState.target;
  message->type           = event_type;
  message->data.data32[0] = dserv->xdndSource.source;

  if (event_type == XDND_LEAVE) {
    DND_DEBUG_PRINT("send XDND_LEAVE,", dserv->xdndState.target);
send:
    xcb_send_event(dserv->connection, false, message->window,
                       XCB_EVENT_MASK_NO_EVENT, (char*)message);
    xcb_flush(dserv->connection);
    return;
  }

  if (event_type == XDND_ENTER) {
    DND_DEBUG_PRINT("send XDND_ENTER,", dserv->xdndState.target);
      /* Attach data from drag that was obtained on entry to XDNDServer. */
    memmove(&message->data, &dserv->xdndSource.src_entermsg,
                             sizeof(xcb_client_message_data_t));
    goto send;
  }

  if (event_type == XDND_POSITION) {
  #if DND_DEBUG
       /* Once for mulpitle messages */
    if (position_request) {
      DND_DEBUG_PRINT("send XDND_POSITION,", dserv->xdndState.target);
      position_request = false;
      status_reply = false;
    }
  #endif
    dserv->xdndState.xdndLastPositionTimestamp = motion->time;
    message->data.data32[2] = (motion->root_x << 16) | motion->root_y;
      /* version >=1 */
    message->data.data32[3] = motion->time;
      /* Always send what we want to do for target.
         Received status tells us what they accept. */
      /* version >= 2 */
    message->data.data32[4] = XDND_ACTIONCOPY;
    goto send;
  }

  if (event_type != XDND_DROP) {
    DEBUG_ASSERT(true, "Developer screw up, Slap him!");
    free(message);
    return;
  }

  if (xdndDropable_get(dserv)) {
   #if DND_DEBUG
    printf("send XDND_DROP,     target %d\n", dserv->xdndState.target);
    printf("              ,     xdnddata_in(&data)\n");
   #endif
    dserv->xdnddata.type = dserv->xdndState.acceptedType;
    dserv->xdnddata_in(&dserv->xdnddata);
      /* time stamp for retrieving the data, version >=1.
        As this is not a cache of selections, unused. */
    message->data.data32[2] = dserv->xdndState.xdndLastPositionTimestamp;
    goto send;
  }

#if DND_DEBUG
  printf("send XDND_LEAVE instead of XDND_DROP,    target %d\n",
                                  dserv->xdndState.target);
#endif
    /* This is now our send XDND_DROP. */
  message->type          = XDND_LEAVE;
  xcb_send_event(dserv->connection, false, message->window,
                         XCB_EVENT_MASK_NO_EVENT, (char*)message);

    /* Since no XDND_DROP, target won't send XDND_FINISHED. */
    /* simulate XDND_FINISHED target to source. */
  message = calloc(32, 1);
  message->response_type  = XCB_CLIENT_MESSAGE;
  message->format         = 32;
  message->window         = dserv->xdndSource.source;
  message->type           = XDND_FINISHED;
  message->data.data32[0] = dserv->xdndState.target;
    /* successfully performed the accepted drop action version >=5 */
    /* [1] == 0 if not accepted */
  message->data.data32[1] = 0;
  message->data.data32[2] = XCB_NONE;

  xcb_send_event(dserv->connection, false, message->window,
                         XCB_EVENT_MASK_NO_EVENT, (char*)message);
  xcb_flush(dserv->connection);
}

/* Returns (uint8_t)-1 of not aware */
static uint8_t
_xdnd_get_aware_value(xcb_connection_t *connection, xcb_window_t window) {

  uint32_t version = ~0;
  xcb_get_property_reply_t *reply;
  xcb_get_property_cookie_t cookie
    = xcb_get_property(connection, 0, window,
                       XDND_AWARE, XCB_ATOM_ATOM, 0, 1);
  xcb_flush(connection);
  reply = xcb_get_property_reply(connection, cookie, NULL);
  xcb_flush(connection);
    /* If a greater version, still can handle ours, don't reject. */
  if ( (reply != NULL) && (reply->value_len != 0) ) {
    version = *(uint32_t*)xcb_get_property_value(reply);
    free(reply);
  }
  return (uint8_t)version;
}

static xcb_get_geometry_reply_t *
_is_visible(xcb_connection_t *connection, xcb_window_t inspect) {

  bool visible;
  xcb_get_window_attributes_cookie_t attributesCookie
    = xcb_get_window_attributes(connection, inspect);
  xcb_get_geometry_cookie_t geometryCookie
    = xcb_get_geometry(connection, inspect);
  xcb_get_window_attributes_reply_t *attributes;
  xcb_get_geometry_reply_t *geometry;
  xcb_flush(connection);
  attributes
    = xcb_get_window_attributes_reply(connection, attributesCookie, NULL);
  geometry
    = xcb_get_geometry_reply(connection, geometryCookie, NULL);
  xcb_flush(connection);

  if (attributes == NULL) {
    if (geometry != NULL)  free(geometry);
    return NULL;
  }
  if (geometry == NULL) {
    free(attributes);
    return NULL;
  }

  visible = ( (attributes->map_state & XCB_MAP_STATE_VIEWABLE)
             == XCB_MAP_STATE_VIEWABLE);
  free(attributes);
  if (!visible) {
    free(geometry);
    geometry = NULL;
  }
  return geometry;
}

/* recursive */
static xcb_window_t
_get_window_at_pointer(xcb_xdndserver_t *dserv, xcb_window_t start_window,
                                     int16_t root_x, int16_t root_y,
                                     int16_t zero_x, int16_t zero_y) {
  int32_t ccount;
  xcb_window_t rWindow = start_window;
  xcb_query_tree_cookie_t treeCookie
    = xcb_query_tree(dserv->connection, start_window);
  xcb_query_tree_reply_t *tree
    = xcb_query_tree_reply(dserv->connection, treeCookie, NULL);
  xcb_flush(dserv->connection);
    /* The children are listed in bottom-to-top stacking order. */
  ccount = xcb_query_tree_children_length(tree);
  if (ccount != 0) {
    xcb_window_t *wList = xcb_query_tree_children(tree);
    do {
      xcb_window_t inspect = wList[(--ccount)];
        /* This return to us geometry if visible. */
      xcb_get_geometry_reply_t *geometry;
      geometry = _is_visible(dserv->connection, inspect);
      if (geometry != NULL) {
        int16_t x0 = zero_x + geometry->x;
        int16_t y0 = zero_y + geometry->y;
        int16_t x1 = x0 + geometry->width;
        int16_t y1 = y0 + geometry->height;
        free(geometry);
        if ( (y0 <= root_y) && (root_y < y1)
            && (x0 <= root_x) && (root_x < x1) ) {
          free(tree);
          return _get_window_at_pointer(dserv, inspect, root_x, root_y, x0, y0);
        }
      }
    } while (ccount != 0);
  }
  free(tree);
  return rWindow;
}

static xcb_window_t
_get_pointer_window(xcb_xdndserver_t *dserv,
                    xcb_window_t root,
                    int16_t root_x,
                    int16_t root_y) {

  xcb_query_pointer_cookie_t c0
    = xcb_query_pointer(dserv->connection, root);
  xcb_query_pointer_reply_t *r0
    = xcb_query_pointer_reply(dserv->connection, c0, NULL);

  xcb_window_t rWindow = 0;

  if (r0->child == 0) {
    ui_cursor_set_named("dnd-no-drop", dserv->xdndSource.source);
  } else if ( (r0->root_x == root_x) && (r0->root_y == root_y) ) {
    xcb_get_geometry_cookie_t c1
      = xcb_get_geometry(dserv->connection, r0->child);
    xcb_get_geometry_reply_t *r1
      = xcb_get_geometry_reply(dserv->connection, c1, NULL);
    rWindow = _get_window_at_pointer(dserv, r0->child,
                                     r0->root_x, r0->root_y,
                                     r1->x, r1->y);
      /* source also stores pointer's state, needed for actions. */
    if (dserv->xdndSource.source_state != r0->mask) {
      xdndIOchange_set(dserv, true);
      dserv->xdndSource.source_state = r0->mask;
    }
    free(r1);
  }

  free(r0);
  return rWindow;
}

/* As source, when mouse motion */
bool
xdnd_drag_motion(xcb_xdndserver_t *dserv, xcb_motion_notify_event_t *motion) {

  xcb_window_t owner  = motion->event;
  xcb_window_t target = _get_pointer_window(dserv,
                                            motion->root,
                                            motion->root_x,
                                            motion->root_y);
  if (target == 0) {
    /*puts("old motion, rejecting");*/
    return true;
  }

  if (!xdndExchanges_get(dserv)) {
      /* Filter case where target did not reply to XdndAware
         but traversing its window. */
    if (dserv->xdndState.target == target)  return true;
      /* Filter case where we returned to ourself, target NULL can't happen */
    if (owner == target) {
      if (xdndActivated_get(dserv))  xdnd_selection_clear(dserv);
      return true;
    }
    dserv->xdndSource.source = owner;
    if ( (dserv->_xdnd_status_cb == NULL)
        || (dserv->xdnddata_in == NULL) ) {
      fprintf(stderr, "Missing handlers for XDNDServer... Aborting\n");
      return false;
    }
    xcb_set_selection_owner(dserv->connection, owner,
                               XDND_SELECTION, motion->time);
restart:
    xdndActivated_set(dserv, true);
    dserv->xdndState.target = target;
    xdndVersion_set(dserv, _xdnd_get_aware_value(dserv->connection, target));
    if (xdndVersion_get(dserv) == (uint8_t)~0) {
      DND_DEBUG_PRINT("abort on xdndVersion.", target);
      _xdnd_cursor_event(dserv, owner, XCB_NONE);
      return true;
    }
    xdndExchanges_set(dserv, true);
    _xdnd_send_event(dserv, NULL, XDND_ENTER);
#if DND_DEBUG
    position_request = true;
#endif
    goto enter_positioning;
  }

  if (target != dserv->xdndState.target) {
#if DND_DEBUG
    position_request = true;
#endif
    if (xdndVersion_get(dserv) != (uint8_t)~0)
      _xdnd_send_event(dserv, NULL, XDND_LEAVE);
      /* Do not reset source information, only state of target.
       The typelist of source should not need query/reply for each
       new target. It does need to be checked if a target's status
       should change acceptance type, or on first status reply against
       typelist source supplies. */
    memset(&dserv->xdndState, 0, sizeof(XDNDStateMachine));
      /* Now that sent XdndLeave, try to send XdndEnter to new target. */
    goto restart;
  }

    /* If XdndStatus requested a 'do not send until' notice, check here. */
  if (xdndEnclose_get(dserv)) {
    int16_t mx = motion->root_x, my = motion->root_y;
    if ( (mx >= dserv->xdndState.x0) && (mx < dserv->xdndState.x1)
        && (my >= dserv->xdndState.y0) && (my < dserv->xdndState.y1) )
      return true;
    xdndEnclose_set(dserv, false);
  }

enter_positioning:
  _xdnd_send_event(dserv, motion, XDND_POSITION);
  return true;
}

/* As source, when mouse button released, send XCB_CLIENT_MESSAGE */
/* Just a pubic wrapper for our private method. */
bool
xdnd_drag_drop(xcb_xdndserver_t *dserv) {
  _xdnd_send_event(dserv, NULL, XDND_DROP);
  return true;
}

/* Just a pubic wrapper for our private method. */
void
xdnd_drag_cancel(xcb_xdndserver_t *dserv) {
  DND_DEBUG_PUTS("CANCEL");
  _xdnd_send_event(dserv, NULL, XDND_LEAVE);
  xdnd_selection_clear(dserv);
}

void
xdnd_selection_clear(xcb_xdndserver_t *dserv) {
  DND_DEBUG_PUTS("received XDND_SELECTION_CLEAR");
  _xdnd_shutdown(dserv);
}

/* Response to XdndDrop, when us as target sent CONVERT. */
/* Responce to XdndDrop, when us as source sent XdndDrop Client Message */
/* Though we sent 2 different types of calls, 1 type of reply. */
void
xdnd_process_selection(xcb_xdndserver_t *dserv,
                       xcb_selection_request_event_t *request) {

  xcb_atom_t performedActionType;
  xcb_selection_data_t *data = &dserv->xdnddata;

    /* Check if sending selection, comes from request */
  if ( (request != NULL)
      && (dserv->xdndState.target != request->owner) ) {
    xcb_selection_notify_event_t *notify;

      /* It appears that a window manager can send XDND_SELECTION for even
        dnd unaware windows. And can send request at any event. */
    DND_DEBUG_PRINT("source XDND_SELECTION for,", dserv->xdndState.target);

      /* It's external's content, they free when done. */
      /* They might for internal, so should for external. */
      /* When drop initiated we retreived data, this is us
        sending data per their request for it. */
    if (data->dsz == 0) {
      if (dserv->xdnddata_in == NULL) {
        DEBUG_ASSERT(true, "failure: no input handler for data.");
          /* Should be valid to send no size with NULL data. */
      } else {
        dserv->xdnddata_in(&dserv->xdnddata);
      }
    }
      /* Note: even though STRING is non-nil terminated,
         we do send a nil terminated c-string. */
    xcb_change_property(dserv->connection, XCB_PROP_MODE_REPLACE,
                        request->requestor,
                        request->property, request->target,
                        sizeof(uint8_t) * 8, data->dsz, data->data);
      /* Send signal to target their data is available. */
    notify = calloc(32, 1);
    notify->response_type = XCB_SELECTION_NOTIFY;
    notify->time      = request->time;
    notify->requestor = request->requestor;
    notify->selection = request->selection;
    notify->target    = request->target;
    notify->property  = request->property;
    xcb_send_event(dserv->connection, false, request->requestor,
                   XCB_EVENT_MASK_PROPERTY_CHANGE, (char *)notify);
    xcb_flush(dserv->connection);
    return;
  }

    /* 'Tis a XCB_SELECTION_NOTIFY, so get data from source's attached
      property. We send XdndFinished after successful or unsuccessful
      attempt at retreival. */
  DND_DEBUG_PUTS("received XDND_SELECTION as target");

  performedActionType = XCB_NONE;
  if (dserv->xdnddata_out == NULL) {
    DEBUG_ASSERT(true, "failure: XDND_SELECTION has no receiver function.");
    goto finish_send;
  }
    /* != 0 occurs when moves to another window (unused and of type) */
  DEBUG_ASSERT(( (data->dsz != 0)
                && (data->type != dserv->xdndState.acceptedType) ),
                       "failure: fixme, didn't expect to happen.");
  if (data->dsz == 0) {
    xcb_get_property_cookie_t c0;
    xcb_get_property_reply_t *r0;
      /* Handle receiving selection, comes from notify */
    c0 = xcb_get_property(dserv->connection, 0, dserv->xdndState.target,
                              XDND_SELECTION, XCB_GET_PROPERTY_TYPE_ANY,
                                                          0, UINT_MAX/4);
    r0 = xcb_get_property_reply(dserv->connection, c0, NULL);
    xcb_flush(dserv->connection);
    if (r0 == NULL) {
      DEBUG_ASSERT(true, "failure: XDND_SELECTION has no data.");
      goto finish_send;
    }
    if ((data->dsz = xcb_get_property_value_length(r0)) == 0) {
      DEBUG_ASSERT(true, "failure: XDND_SELECTION has no data.");
      free(r0);
      goto finish_send;
    }
      /* Appears that 'STRING' are non-nil c-strings, just text. */
    data->data = malloc(data->dsz + 1);
    memmove(data->data, xcb_get_property_value(r0), data->dsz);
    data->data[data->dsz] = 0;
    free(r0);
  }
    /* This is us, as target, receiving data.
      It is our only inform of server shut down. */
  dserv->xdnddata_out(&dserv->xdnddata);

  performedActionType = XDND_ACTIONCOPY;

finish_send:

    /* Inform source it's over. This occurs even when data retreive
      failure from external source. I don't see resend as a solution. */

    /* 2 actions can be performed by a target, XDND_ACTIONCOPY, XCB_NONE. */
    /* The action list only applies to source. It performs when
      XDND_FINISHED is received (versions >= 2). */
  if (xdndVersion_get(dserv) >= 2) {
    xcb_client_message_event_t *message = calloc(32, 1);

    DND_DEBUG_PRINT("send XDND_FINISHED,", dserv->xdndState.target);

    message->response_type  = XCB_CLIENT_MESSAGE;
    message->format         = 32;
    message->window         = dserv->xdndSource.source;
    message->type           = XDND_FINISHED;
    message->data.data32[0] = dserv->xdndState.target;
      /* successfully performed the accepted drop action version >=5 */
    if (performedActionType != XCB_NONE) {
      message->data.data32[1] = 1;
      message->data.data32[2] = performedActionType;
    }

    xcb_send_event(dserv->connection, false, message->window,
                                   XCB_EVENT_MASK_NO_EVENT, (char*)message);
    xcb_flush(dserv->connection);
  } else {
     /* Since no XDND_FINISHED, must shutdown here. */
     /* Implies only source actions XDND_ACTIONCOPY, XCB_NONE. */
    _xdnd_shutdown(dserv);
  }
  return;
}

static void
_get_source_typelist(xcb_xdndserver_t *dserv,
                     xcb_client_message_event_t *request) {

    /* for XDND_TYPELIST */
  xcb_get_property_reply_t *r0 = NULL;
    /* Get format type we want data converted to (send data as) */
    /* Check for XdndTypeList, use backup of request message otherwise */
    /* Initialize as fallback variables */
  uint32_t lcount = 3, ldx;
  xcb_atom_t *a_list = &request->data.data32[2];
    /* When retreival of XdndAware (data32), bit 1 informs if XdndTypeList */
  if ((request->data.data32[1] & 1) != 0) {
      /* retreive XdndTypeList information, adjust variables */
    xcb_get_property_cookie_t c0
      = xcb_get_property(dserv->connection, 0, dserv->xdndSource.source,
                         XDND_TYPELIST, XCB_GET_PROPERTY_TYPE_ANY,
                         0, UINT_MAX/4);
    r0 = xcb_get_property_reply(dserv->connection, c0, NULL);
    if (r0 != NULL) {
      lcount = xcb_get_property_value_length(r0) / sizeof(xcb_atom_t);
      a_list = xcb_get_property_value(r0);
    }
  }

    /* Grab a copy of source's XDND_TYPELIST. We use this once
     we get XdndPosition updates to determine acceptance of dnd.
     Enter provides no clue as to where in window drop might
     occur. That's why XdndPosition sent immediately after XdndEnter. */
  dserv->xdndSource.source_offer = malloc(sizeof(xcb_atom_t) * lcount);
  memset(dserv->xdndSource.source_offer, 0, (sizeof(xcb_atom_t) * lcount));
  ldx = 0;
  do {
    xcb_atom_t atom = a_list[ldx];
    if (atom == XCB_NONE)  break;
    dserv->xdndSource.source_offer[ldx] = atom;
    if ((ldx + 1) == lcount)  break;
    ldx++;
  } while (1);
  dserv->xdndSource.so_count = ldx + 1;

  if (lcount > 3)  free(r0);
}

/* When we are the target: XdndEnter, XdndPosition, XdndDrop, XdndLeave. */
/* When we are the source: XdndStatus, XdndFinished. */
/* Returns true if XdndLeave or XdndFinished received. Signals
   application that target/source dnd finished. */
bool
xdnd_process_message(xcb_xdndserver_t *dserv,
                     xcb_client_message_event_t *request) {

  xcb_client_message_event_t *message;
  xcb_atom_t event_type = request->type;

    /* Here only if external source notify of shutdown. */
  if (event_type == XDND_LEAVE) {
    DND_DEBUG_PRINT("received XDND_LEAVE,", dserv->xdndState.target);
    xdnd_selection_clear(dserv);
    return true;
  }
    /* same as XDND_LEAVE but received since we're source */
  if (event_type == XDND_FINISHED) {
      /* This should delay our data buffer delete until received.
        On versions <2, once sent, delete, since no XdndFinished existed. */
    DND_DEBUG_PRINT("received XDND_FINISHED,", dserv->xdndState.target);

      /* Finished sent possibly by us, or by some WM on dnd unaware. */
    if (xdndVersion_get(dserv) != (uint8_t)~0) {
        /* Under version 5, forced to perform action. Above can signal
          that no drop was performed, so take no action. */
      if ( (xdndVersion_get(dserv) < (uint16_t)5)
          || ((request->data.data32[1] & 1) == 1) ) {
          /* As source, if XdndAction(s) perform required, else XdndActionCopy */
        if (dserv->xdndSource.sa_count > 1) {
          if (dserv->xdnddata_act != NULL) {
            dserv->xdnddata_act(dserv->xdndSource.source_state);
          } else
            DEBUG_ASSERT(true, "failure: no action handler for actions.");
        }
      }
    }
    xdnd_selection_clear(dserv);
    return true;
  }

  if (event_type == XDND_ENTER) {
   #if DND_DEBUG
    bool snz = (dserv->xdndSource.source != 0);
    printf("received XDND_ENTER,        target %d source %d\n",
                    dserv->xdndState.target, dserv->xdndSource.source);
   #endif
      /* Check to see if application is source (!=0). */
    if (dserv->xdndSource.source == 0) {
      /* We are target for an external source. */
        /* Initialize our dnd state */
      xdndActivated_set(dserv, true);
      xdndExchanges_set(dserv, true);
        /* Get the version they support */
      xdndVersion_set(dserv, (uint8_t)(request->data.data32[1] >> 24));
      dserv->xdndSource.source = request->data.data32[0];
      _get_source_typelist(dserv, request);
     #if DND_DEBUG
      printf("start server,               target %d source %d\n",
                      request->window, dserv->xdndSource.source);
     #endif
    }
   #if DND_DEBUG
    if (snz)
      printf("altered server,             target %d source %d\n",
                    request->window, dserv->xdndSource.source);
   #endif
    dserv->xdndState.target  = request->window;
    return false;
  }

    /* Depending on version, may send CONVERT instead of message */
  if (event_type == XDND_DROP) {
    DND_DEBUG_PRINT("received XDND_DROP,", dserv->xdndState.target);
    if (dserv->xdndState.acceptedType != XCB_NONE) {
      xcb_time_t time = request->data.data32[2];
      xcb_convert_selection(dserv->connection, dserv->xdndState.target,
                            XDND_SELECTION,
                            dserv->xdndState.acceptedType,
                            XDND_SELECTION,
                            time);
      xcb_flush(dserv->connection);
      return false;
    } else if (xdndVersion_get(dserv) < 2)
      return false;
    /* if no acceptedType for versions >= 2, send finished event */
  }

    /* Works with xdndSource.source
       because XDND_ENTER is always first message. */
  message = calloc(32, 1);
  message->response_type  = XCB_CLIENT_MESSAGE;
  message->format         = 32;                /* Always response atom type */
  message->window         = dserv->xdndSource.source;
  message->data.data32[0] = dserv->xdndState.target;  /* our window */

    /* Special case: Version >= 2 && acceptedType == XCB_NONE */
  if (event_type == XDND_DROP) {
    xcb_client_message_event_t *notify;
    DND_DEBUG_PUTS("send XDND_FINISHED from received XDND_DROP");
    message->type = XDND_FINISHED;
    xcb_send_event(dserv->connection, false, message->window,
                                   XCB_EVENT_MASK_NO_EVENT, (char*)message);

    notify = calloc(32, 1);
    memmove(notify, message, 32);
    xcb_send_event(dserv->connection, false, dserv->xdndState.target,
                                   XCB_EVENT_MASK_NO_EVENT, (char*)notify);
    xcb_flush(dserv->connection);
    return false;
  }

  if (event_type == XDND_POSITION) {
  #if DND_DEBUG
    if (position_request) {
      if (request->window == dserv->xdndState.target)
        DND_DEBUG_PRINT("received XDND_POSITION,", request->window);
      position_request = false;
    }
  #endif
    message->type = XDND_STATUS;
      /* retreive pertainate 'data'. */
    dserv->xdndState.xdndLastPositionTimestamp = request->data.data32[3];
     /* Since we are in our window, no use xcb_translate_coordinates().
       We pass global coordinates in 'param'. Also check for target change. */
    message->data.data32[2] = request->data.data32[2];
      /* get feedback from target window */
    dserv->_xdnd_status_cb(request->window, &message->data);

      /* data[0] needs to set dserv's acceptedType */
      /* this may seem redundant, but there are 2 types of sources,
        external and internal */
    if (dserv->xdndState.acceptedType != message->data.data32[0]) {
        /* check that source can provide new type */
      xcb_atom_t can_supply = XCB_NONE;
      xcb_atom_t within_wants = message->data.data32[0];
      if (within_wants != XCB_NONE) {
        uint8_t wdx;
        for (wdx = 0; wdx < dserv->xdndSource.so_count; wdx++)
          if (dserv->xdndSource.source_offer[wdx] == within_wants) {
            can_supply = within_wants;
            break;
          }
        message->data.data32[0] = can_supply;
      }
    }
    dserv->xdndState.acceptedType = message->data.data32[0];
      /* now set data[0] correctly */
    message->data.data32[0] = request->window;

    xcb_send_event(dserv->connection, false, message->window,
                                   XCB_EVENT_MASK_NO_EVENT, (char*)message);
    xcb_flush(dserv->connection);
    return false;
  }

  free(message);

  DEBUG_ASSERT((event_type != XDND_STATUS),  "failure: Developer huh? dooh!");

    /* Currently no way to tell if later position was sent,
      so could ignore this one. */
  if (dserv->xdndState.target != request->data.data32[0]) {
    DND_DEBUG_IGNORE("XDND_STATUS",
                       dserv->xdndState.target, request->data.data32[0]);
    return false;
  }
 #if DND_DEBUG
    else {
        /* Once for mulpitle messages */
      if (!status_reply)
        DND_DEBUG_PRINT("received XDND_STATUS,", request->data.data32[0]);
      status_reply = true;
  }
 #endif

    /* An object can maybe accept piecemeal drop locations,
       if any location in a window can receive, our response must can
       accept with an action other than None. */
    /* Only if window can't accept can we short circut process */

    /* Viewable drop code. */
  if (xdndDropable_get(dserv) && !xdndRaised_get(dserv)) {
    xdndRaised_set(dserv, true);
    _raise_target_window(dserv->connection, dserv->xdndState.target);
  }

    /* version >=2: allows XCB_NONE for rejection. */
  if ( (xdndDropable_get(dserv) != (request->data.data32[1] & 1))
      || ( (xdndDropable_get(dserv)) && (xdndIOchange_get(dserv)) ) ) {
    _xdnd_cursor_event(dserv, request->window, request->data.data32[4]);
    xdndDropable_set(dserv, request->data.data32[1] & 1);
  }
  xdndIOchange_set(dserv, false);

    /* Check if wants us not to send positions for certain region */
    /* Only handle exclusion requests. */
  if ( (request->data.data32[3] != 0)
      && ((request->data.data32[1] & 2) == 0) ) {
    DND_DEBUG_PUTS("received xdndEnclose");
    xdndEnclose_set(dserv, true);
    dserv->xdndState.x0 = request->data.data16[4];
    dserv->xdndState.y0 = request->data.data16[5];
    dserv->xdndState.x1 = dserv->xdndState.x0 + request->data.data16[6];
    dserv->xdndState.y1 = dserv->xdndState.y0 + request->data.data16[7];
  }

  return false;
}

#endif /* DND_EXTERNAL_ON */
