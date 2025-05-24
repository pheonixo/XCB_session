#include "textviews_drag.h"

#define DND_DEBUG 0
#if DND_DEBUG
 #define DND_DEBUG_PUTS(a) puts((a))
 #define DND_DEBUG_PRINT(a,b) \
   printf("%-26s  target %d\n", (a), (b))
 #define DND_DEBUG_CURSOR(a) puts((a))
 #define DND_DEBUG_INTERNAL(a,b) \
   printf("%-26s  target %p\n", (a), (void*)(b))
#else
 #define DND_DEBUG_PUTS(a)
 #define DND_DEBUG_PRINT(a,b)
 #define DND_DEBUG_CURSOR(a)
 #define DND_DEBUG_INTERNAL(a,b)
#endif

#pragma mark *** Drawing ***

/* _drag_caret() used by internal and external */
static void
_drag_caret(PhxTextbuffer *tbuf, int16_t x, int16_t y) {

  location temp;
  int16_t  font_em;

  if ( (x < 0) || (x > tbuf->bin.w) )  goto outside;
  if ( (y < 0) || (y > tbuf->bin.h) )  goto outside;

  temp.x = x + tbuf->bin.x;
  temp.y = y + tbuf->bin.y;
  location_for_point(tbuf, &temp);

  tbuf->owner->state |= OBIT_DND;
  tbuf->owner->state |= OBIT_DND_CARET;
  font_em = tbuf->owner->attrib->font_em;

    /* 3 cases for y's position (want a full line above or below caret):
      in top line that's not fully visible
      in bottom line that's not fully visible
      in fully visible line, but questionable x */
  if (temp.y <= (tbuf->bin.y + font_em)) {
    location loc;
    loc.x = temp.x, loc.y = temp.y - font_em;
    if ((tbuf->bin.y - font_em) < 0)  loc.y = 0;
    location_auto_scroll(tbuf, &loc);
  } else if (temp.y >= (tbuf->bin.y + tbuf->bin.h - font_em)) {
    location loc;
    loc.x = temp.x, loc.y = temp.y + font_em;
    location_auto_scroll(tbuf, &loc);
  } else {
    location_auto_scroll(tbuf, &temp);
  }

    /* 3 cases of drop position:
      at ends of selection
      outside ends of selection
      inside ends of selection */
  if ( (temp.offset == tbuf->release.offset)
      || (temp.offset == tbuf->insert.offset) ) {
      /* when at ends, use caret on dnd copy */
    if ((tbuf->owner->state & OBIT_DND_COPY) != 0) {
      tbuf->drop = temp;
      goto redraw;
    }
    tbuf->owner->state &= ~OBIT_DND_CARET;
  } else if ( (temp.offset > tbuf->release.offset)
      || (temp.offset < tbuf->insert.offset) ) {
    if (temp.offset != tbuf->drop.offset) {
      tbuf->drop = temp;
      goto redraw;
    }
  } else {
    /* also occurs on startup of drag, not just outside */
outside:
    tbuf->owner->state &= ~OBIT_DND_CARET;
    tbuf->drop = tbuf->interim;
redraw:
    ui_invalidate_object(tbuf->owner);
  }
}

#pragma mark *** Keyboard ***

bool
_textview_drag_cancel(PhxInterface *iface,
                      xcb_generic_event_t *nvt,
                      PhxObject *obj) {

  PhxTextbuffer *tbuf;
  PhxObject *within;

  (void)nvt;

  if ((obj = ui_active_drag_get()) == NULL)  return false;

  if ((within = ui_active_within_get()) != NULL) {
/* Should be according to actual object, not generic. */
    if (within->type == PHX_TEXTVIEW) {
      within->state &= ~(OBIT_DND_CARET | OBIT_DND_COPY | OBIT_DND);
      ui_cursor_set_named("text", iface->window);
      ui_invalidate_object(within);
    } else {
      ui_cursor_set_named(NULL, iface->window);
    }
  }
    /* This is has_drag, cursor is in 'within'. */
  obj->state &= ~(OBIT_DND_CARET | OBIT_DND_COPY | OBIT_DND);
  tbuf = (PhxTextbuffer*)obj->exclusive;
    /* This is auto_scroll */
  tbuf->bin = tbuf->drag_sbin;
    /* Set focus to within's window (cursor's position).
      Since different windows, used DNDX. */
  if (_window_for(within) != _window_for(obj)) {
    iface = (within != NULL) ? _interface_for(_window_for(within))
                             : _interface_for(_window_for(obj));
    iface->state &= ~SBIT_CLICKS;
    xcb_set_input_focus(session->connection,
                        XCB_INPUT_FOCUS_PARENT,
                        iface->window, XCB_CURRENT_TIME);
    ui_active_focus_set((PhxObject*)iface);
 #if DND_EXTERNAL_ON
    if (xdndActivated_get(session->xdndserver))
      xdnd_selection_clear(session->xdndserver);
 #endif
  }
    /* Finished with has_drag. */
  DND_DEBUG_PUTS("ui_active_drag_set(NULL) _textview_drag_cancel()");
  ui_active_drag_set(NULL);
  ui_invalidate_object(obj);
  return true;
}

bool
_textview_drag_keyboard(PhxInterface *iface,
                        xcb_generic_event_t *nvt,
                        PhxObject *obj)  {

  xcb_key_press_event_t *kp;
  xcb_keysym_t keyval;

  if (ui_active_drag_get() == NULL)
    return false;

  kp = (xcb_key_press_event_t*)nvt;
  keyval = _xcb_keysym_for(kp->detail, kp->state);

  if (keyval == 0x0ff1b)
    return _textview_drag_cancel(iface, nvt, obj);

  return _drag_keyboard(iface, nvt);
}

#pragma mark *** Mouse ***

bool
_textview_drag_crossing(PhxInterface *iface,
                        xcb_generic_event_t *nvt,
                        PhxObject *obj) {
  PhxTextbuffer *tbuf;
  bool locus;

  if ((obj->state & OBIT_DND_AWARE) == 0)  return true;

  if (ui_active_drag_get() == NULL) {
#if DND_EXTERNAL_ON
    if (!xdndActivated_get(session->xdndserver))
#endif
    return false;
  }

  locus = ((nvt->response_type & (uint8_t)0x7F) == XCB_ENTER_NOTIFY);
  if (locus) {
      /* motion decides other bits */
    obj->state |= OBIT_DND;
#if DND_EXTERNAL_ON
     if (xdndActivated_get(session->xdndserver))
      xdnd_quirk_dst_load(session->xdndserver, iface->window, obj->dnd_quirk);
#endif
      /* Setup for possible leave or cancel. */
    DND_DEBUG_INTERNAL("Setting tbuf for", obj);
    tbuf = (PhxTextbuffer*)obj->exclusive;
    tbuf->drag_sbin = tbuf->bin;
    return true;
  }

    /* motion should decide cursor */
  DND_DEBUG_INTERNAL("Restoring tbuf for", obj);
  tbuf = (PhxTextbuffer*)obj->exclusive;
  tbuf->bin = tbuf->drag_sbin;
  obj->state &= ~(OBIT_DND_CARET | OBIT_DND_COPY | OBIT_DND);
  ui_invalidate_object(obj);
  return true;
}

bool
_textview_drag_begin(PhxInterface *iface,
                     xcb_generic_event_t *nvt,
                     PhxObject *obj) {

  xcb_motion_notify_event_t *motion;
  PhxObjectTextview *otxt;
  PhxTextbuffer     *tbuf;
  int16_t xmm, ymm;

  motion = (xcb_motion_notify_event_t*)nvt;

  otxt = (PhxObjectTextview*)ui_active_focus_get();
  if ((tbuf = (PhxTextbuffer*)otxt->exclusive) == NULL) {
    DEBUG_ASSERT(true, "textview has no textbuffer. _textview_drag_begin().");
    return true;
  }
  xmm = motion->event_x - (otxt->mete_box.x + otxt->draw_box.x);
  ymm = motion->event_y - (otxt->mete_box.y + otxt->draw_box.y);

  if (xmm < 0)  xmm = 0;
  if (ymm < 0)  ymm = 0;
    /* Interim used for button press location.
      First time bp-motion to set SBIT or has_drag.
      From above 'if' we know has_drag not set. */
  if ((iface->state & SBIT_SELECTING) != 0) {
selecting:
    location_shift_click(tbuf, xmm, ymm);
    ui_invalidate_object(otxt);
    return true;
  }

  if (location_drag_begin(tbuf, xmm, ymm)) {
    bool kpcntl = ((motion->state & XCB_MOD_MASK_CONTROL) != 0);
    otxt->state |= OBIT_DND | (kpcntl << DRAG_CURSOR_BIT);
    DND_DEBUG_INTERNAL("Setting tbuf for", otxt);
    DND_DEBUG_PUTS("ui_active_drag_set(otxt) _textview_drag_begin()");
    ui_active_drag_set(otxt);
    ui_cursor_set_named(((kpcntl) ? "dnd-copy" : "dnd-move"), iface->window);
    return true;
  }
    /* Since a selection didn't exist, has to br selection drag. */
  iface->state |= SBIT_SELECTING;
  goto selecting;
}

bool
_textview_drag_motion(PhxInterface *iface,
                   xcb_generic_event_t *nvt,
                   PhxObject *obj) {

  xcb_motion_notify_event_t *motion;
  bool kpcntl;

  int16_t xmm, ymm;
  PhxObject *has_drag = ui_active_drag_get();
  PhxObjectTextview *otxt = (PhxObjectTextview*)has_drag;
  PhxObject *within = ui_active_within_get();

  if ( (obj == NULL) || (within == NULL) ) {
no_drop:
    otxt->state &= ~OBIT_DND_CARET;
    ui_cursor_set_named("dnd-no-drop", iface->window);
                   /* XXX tbuf in otxt */
    if (((PhxTextbuffer*)otxt->exclusive)->drop.offset
         != ((PhxTextbuffer*)otxt->exclusive)->interim.offset) {
      ((PhxTextbuffer*)otxt->exclusive)->drop
        = ((PhxTextbuffer*)otxt->exclusive)->interim;
      ui_invalidate_object((PhxObject*)otxt);
    }
    return true;
  }

#if (!DND_INTERNAL_ON)
  if (has_drag != within)  goto no_drop;
#endif

    /* During drag can enter an interface section or non-aware object. */
  if ( !!(within->state & OBIT_DND_AWARE) != (!IS_IFACE_TYPE(within)) )
    goto no_drop;

  motion = (xcb_motion_notify_event_t*)nvt;
    /* Note: Applies to all objects (within), not just textview (has_drag). */
  xmm = motion->event_x - (within->mete_box.x + within->draw_box.x);
  ymm = motion->event_y - (within->mete_box.y + within->draw_box.y);
  if (   (xmm < 0) || (xmm >= within->draw_box.w)
      || (ymm < 0) || (ymm >= within->draw_box.h) ) {
    goto no_drop;
  }

/* currently in keyboard and here */
    /* kpcntl & within location control pointer icon */
  kpcntl = ((motion->state & XCB_MOD_MASK_CONTROL) != 0);
    /* initial setup, not implemented */
  kpcntl ^= (has_drag != within);
  if (kpcntl)  {
    otxt->state |= OBIT_DND_COPY;
    ui_cursor_set_named("dnd-copy", iface->window);
  } else {
    otxt->state &= ~OBIT_DND_COPY;
    ui_cursor_set_named("dnd-move", iface->window);
  }

  if (within != has_drag) {
#if DND_INTERNAL_ON
    xcb_idndserver_t *iserv = session->idndserver;
    _send_dnd_event(iserv, nvt, XDND_POSITION);
#else
    goto no_drop;
#endif
    return true;
  }

    /* At here: within == has_drag so is textview. */
  _drag_caret((PhxTextbuffer*)within->exclusive, xmm, ymm);
  ui_invalidate_object(within);
  return true;
}

bool
_textview_drag_finish(PhxInterface *iface,
                      xcb_generic_event_t *nvt,
                      PhxObject *obj) {

  const char *cursor;
  PhxObjectTextview *otxt = (PhxObjectTextview*)ui_active_drag_get();

    /* if NULL, developer am bad! */
  if (otxt == NULL)  return false;

    /* NULL indicates not in window. Catch if object can handle. */
  if ( (obj == NULL) || ((obj->state & OBIT_DND_AWARE) == 0) )
    return _textview_drag_cancel(iface, nvt, obj);

  if (ui_active_within_get() != (PhxObject*)otxt) {
#if DND_INTERNAL_ON
    xcb_idndserver_t *iserv = session->idndserver;
    _send_dnd_event(iserv, nvt, XDND_DROP);
#endif
    return _textview_drag_cancel(iface, nvt, obj);
  }

  otxt->state &= ~(OBIT_DND_CARET | OBIT_DND_COPY | OBIT_DND);
  cursor = ui_cursor_get_named();
  DEBUG_ASSERT((cursor == NULL), "failure: NULL cursor drop.");
  _textbuffer_drag_release((PhxTextbuffer*)obj->exclusive,
                               (strcmp(cursor, "dnd-move") == 0));
  ui_cursor_set_named("text", iface->window);
  DND_DEBUG_PUTS("ui_active_drag_set(NULL) _textview_drag_finish()");
  ui_active_drag_set(NULL);
  ui_invalidate_object(obj);
#if DND_INTERNAL_ON
  DND_DEBUG_INTERNAL("XDND_FINISHED", ui_active_within_get());
  memset(session->idndserver, 0, sizeof(xcb_dnd_notify_event_t));
#endif
  return true;
}

#pragma mark *** DNDI DNDX ***

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
/*reply*/
void
_textview_selection_data_set(xcb_selection_data_t *gdata) {

    /* XXX should verify gdata type */

    /* Needed for shutdown or this XdndFinish. */
  xcb_window_t window;
    /* within should be set when handling XdndPosition. */
  PhxTextbuffer *tbuf;
  PhxBin saved_bin;
  PhxObjectTextview *otxt = (PhxObjectTextview*)ui_active_within_get();
  if ( (otxt == NULL) || (otxt->type != PHX_TEXTVIEW) ) {
    DEBUG_ASSERT(true, "failure: within is NULL for insert.");
    return;
  }
  tbuf = (PhxTextbuffer*)otxt->exclusive;
    /* Do not split a selection. We do not replace at this time. */
  if ( (tbuf->insert.offset != tbuf->release.offset)
      && ( (tbuf->drop.offset >= tbuf->insert.offset)
          && (tbuf->drop.offset <= tbuf->release.offset) ) ) {
    if (tbuf->drop.offset != tbuf->insert.offset)
      tbuf->drop.offset = tbuf->release.offset;
  }
    /* Before inserting, set locations to drop.offset. */
  tbuf->insert.offset  = tbuf->drop.offset;
  tbuf->release.offset = tbuf->drop.offset;

    /* want to preserve how user sees drop point, no scrolling on insert. */
  saved_bin = tbuf->bin;

  _textbuffer_insert(tbuf, (char*)gdata->data, gdata->dsz);
    /* set drop as highlighted selection */
  tbuf->insert.offset  = tbuf->release.offset - gdata->dsz;
  location_for_offset(tbuf, &tbuf->insert);
  location_for_offset(tbuf, &tbuf->release);
  tbuf->interim = tbuf->release;
  _textbuffer_flush(tbuf);

  tbuf->bin = saved_bin;
    /* make sure drop caret not visible */
  otxt->state &= ~(OBIT_DND_CARET | OBIT_DND_COPY | OBIT_DND);
    /* This was also our XdndFinished, set cursor, SBIT. */
  window = otxt->i_mount->window;
  ui_cursor_set_named("text", window);
  ui_invalidate_object((PhxObject*)otxt);
    /* do not free, it's XDND property */
}

/* this should be called on start of drag motion?
  called at drop send message. */
/*request*/
void
_textview_selection_data_get(xcb_selection_data_t *sdata) {

  char *rbuf = NULL;
  int sz;
  PhxObjectTextview *otxt = (PhxObjectTextview*)ui_active_drag_get();
  PhxTextbuffer *tbuf = (PhxTextbuffer*)otxt->exclusive;
  sz = tbuf->release.offset - tbuf->insert.offset;
  if (sz != 0) {
    rbuf = malloc(sz + 1);
    memmove(rbuf, &tbuf->string[tbuf->insert.offset], sz);
    rbuf[sz] = 0;
  }
  if (sdata->data != NULL)
    free(sdata->data);
  sdata->data = (uint8_t*)rbuf;
  sdata->dsz = sz;
}

/* As source, XDND_ACTIONCOPY should be ignored, other than
  XDND_ACTIONMOVE are currently also ignored. */
void
_textview_selection_action(uint16_t key_but_state) {
  PhxObjectTextview *otxt = (PhxObjectTextview*)ui_active_drag_get();
  if ((key_but_state & XCB_KEY_BUT_MASK_CONTROL) != 0) {
    _textbuffer_delete((PhxTextbuffer*)otxt->exclusive);
    _textbuffer_flush((PhxTextbuffer*)otxt->exclusive);
    ui_invalidate_object((PhxObject*)otxt);
  }
}

/* drawing update for pointer position, with request for continous updates */
bool
_dnd_status_cb(xcb_dnd_notify_event_t *dnd) {

    /* acceptance of drag type was state flag adjusted */
    /* adjust drawing routine */
  _drag_caret((PhxTextbuffer*)dnd->within->exclusive,
                                  dnd->within_x, dnd->within_y);
    /* true = want to be continuously updated */
  return true;
}
#endif
