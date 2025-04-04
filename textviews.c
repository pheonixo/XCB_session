#include "textviews.h"
#include "textviews_drag.h"
#include "findport.h"

#pragma mark *** Drawing ***

void
_textview_draw(PhxObject *b, cairo_t *cr) {

  PhxRGBA *colour;
  PhxRectangle bounds;
  PhxObjectTextview *otxt = (PhxObjectTextview*)b;

  bounds = otxt->mete_box;
  if ( ( (bounds.w <= 0) || (bounds.h <= 0) )
      || ((bounds.x + bounds.w) <= 0)
      || ((bounds.y + bounds.h) <= 0) )
    return;

    /* Will reset matrix to current on restore(). */
  cairo_save(cr);

    /* XXX: Draw background. Plain backdrop only set up for.
      Occurs even if empty buffer content. */
  colour = &otxt->attrib->bg_fill;
  cairo_set_source_rgba(cr, colour->r, colour->g, colour->b, colour->a);
  if (bounds.x < 0)  bounds.x = 0;
  if (bounds.y < 0)  bounds.y = 0;
  cairo_rectangle(cr, bounds.x, bounds.y, bounds.w, bounds.h);
  cairo_fill(cr);

  if (otxt->exclusive != NULL) {
    if ( (otxt->draw_box.w > 0) && (otxt->draw_box.h > 0) ) {
        /* mete offset by draw margins */
      bounds.x = otxt->mete_box.x + otxt->draw_box.x;
      bounds.y += otxt->draw_box.y;
      cairo_rectangle(cr, bounds.x, bounds.y,
                          otxt->draw_box.w, otxt->draw_box.h);
      cairo_clip(cr);

      colour = &otxt->attrib->fg_fill;
      if (colour->a != 0.0) {
        cairo_set_source_rgba(cr, colour->r, colour->g, colour->b, colour->a);
        cairo_paint(cr);
      }

      cairo_translate(cr, bounds.x, bounds.y);
      _default_textbuffer_draw(b, cr);
    }
  }

  cairo_restore(cr);
}

#pragma mark *** Events ***

static bool
_textview_keyboard(PhxInterface *iface,
                   xcb_generic_event_t *nvt,
                   PhxObject *obj)  {

  PhxTextbuffer *tbuf;
  xcb_key_press_event_t *kp;
  xcb_keysym_t keyval;
  char ch0;
  uint16_t state;

  if (_textview_drag_keyboard(iface, nvt, obj))
    return true;

    /* Any key we acted upon ignores XCB_KEY_RELEASE. */
  if ((iface->state & SBIT_RELEASE_IGNORE) != 0) {
    iface->state &= ~SBIT_RELEASE_IGNORE;
    return true;
  }

  if ((tbuf = (PhxTextbuffer*)obj->exclusive) == NULL) {
    DEBUG_ASSERT(true, "textview has no textbuffer. _textview_keyboard().");
    return true;
  }

  kp     = (xcb_key_press_event_t*)nvt;
  keyval = _xcb_keysym_for(kp->detail, kp->state);
  state = kp->state & (  XCB_MOD_MASK_1
                       | XCB_MOD_MASK_CONTROL
                       | XCB_MOD_MASK_SHIFT);

  if ( (keyval >= 0x0020) && (keyval <= 0x007e) ) {
    if ((state & XCB_MOD_MASK_CONTROL) != 0) {
      if (keyval == 'a') {  location_select_all(tbuf);  goto redraw;  }
      if (keyval == 'c') {  _textbuffer_copy(tbuf);     goto sbit;  }
      if (keyval == 'v') {  _textbuffer_paste(tbuf);    goto redraw;  }
      if (keyval == 'x') {  _textbuffer_cut(tbuf);      goto redraw;  }
      if ( (keyval == 'e') || (keyval == 'E') ) {
        if (kp->response_type == XCB_KEY_RELEASE)
          ui_findport_keyboard(obj, keyval);
        return true;
      }
      if ( (keyval == 'f') || (keyval == 'F') ) {
          /* XXX not seperate since dialog not started code. */
        if (kp->response_type == XCB_KEY_RELEASE)
          ui_findport_search(iface, obj);
        return true;
      }
        /* unhandled */
      return false;
    }
    _textbuffer_insert(tbuf, (char*)&keyval, 1);
    goto redraw;
  }

  switch (keyval) {

    case 0x0ff08:                        /* XK_BackSpace */
      _textbuffer_delete(tbuf);
      goto redraw;

    case 0x0ff09:                              /* XK_Tab */
      ch0 = keyval;
      _textbuffer_insert(tbuf, &ch0, 1);
      goto redraw;

    case 0x0ff0d:                           /* XK_Return */
    case 0x0ff8d:                         /* XK_KP_Enter */
      ch0 = '\n';
      _textbuffer_insert(tbuf, &ch0, 1);
      goto redraw;

    case 0x0ff1b:                           /* XK_Escape */
      return true;

    case 0x0ff50:                             /* XK_Home */
    case 0x0ff95:                          /* XK_KP_Home */
      location_key_motion(tbuf, (state | (short)0x8020));
      goto redraw;

    case 0x0ff51:                             /* XK_Left */
    case 0x0ff96:                          /* XK_KP_Left */
      location_key_motion(tbuf, (state | (short)0x8000));
      goto redraw;

    case 0x0ff52:                               /* XK_Up */
    case 0x0ff97:                            /* XK_KP_Up */
      location_key_motion(tbuf, (state | (short)0x8010));
      goto redraw;

    case 0x0ff53:                            /* XK_Right */
    case 0x0ff98:                         /* XK_KP_Right */
      location_key_motion(tbuf, (state | (short)0x0000));
      goto redraw;

    case 0x0ff54:                             /* XK_Down */
    case 0x0ff99:                          /* XK_KP_Down */
      location_key_motion(tbuf, (state | (short)0x0010));
      goto redraw;

    case 0x0ff55:                          /* XK_Page_Up */
    case 0x0ff9a:                       /* XK_KP_Page_Up */
    /*case XK_KP_Prior: */                    /* 0x0ff9a */
      location_key_motion(tbuf, (state | (short)0x8030));
      goto redraw;

    case 0x0ff56:                        /* XK_Page_Down */
    case 0x0ff9b:                     /* XK_KP_Page_Down */
    /*case XK_KP_Next: */                     /* 0x0ff9b */
      location_key_motion(tbuf, (state | (short)0x0030));
      goto redraw;

    case 0x0ff57:                              /* XK_End */
    case 0x0ff9c:                           /* XK_KP_End */
      location_key_motion(tbuf, (state | (short)0x0020));
      goto redraw;

    case 0x0ff9d:                         /* XK_KP_Begin */
      return false;

    case 0x0ff63:                           /* XK_Insert */
    case 0x0ff9e:                        /* XK_KP_Insert */
      tbuf->state ^= TBIT_KEY_INSERT;
      goto redraw;

    case 0x0ff67:                             /* XK_Menu */
    case 0x0ff7f:                         /* XK_Num_Lock */
      return false;

    case 0x0ffaa:                 /* * 2a XK_KP_Multiply */
    case 0x0ffab:                      /* + 2b XK_KP_Add */
    case 0x0ffac:                /* , 2c XK_KP_Separator */
    case 0x0ffad:                 /* - 2d XK_KP_Subtract */
    case 0x0ffae:                  /* . 2e XK_KP_Decimal */
    case 0x0ffaf:                   /* / 2f XK_KP_Divide */
    case 0x0ffb0:                             /* XK_KP_0 */
    case 0x0ffb1:                             /* XK_KP_1 */
    case 0x0ffb2:                             /* XK_KP_2 */
    case 0x0ffb3:                             /* XK_KP_3 */
    case 0x0ffb4:                             /* XK_KP_4 */
    case 0x0ffb5:                             /* XK_KP_5 */
    case 0x0ffb6:                             /* XK_KP_6 */
    case 0x0ffb7:                             /* XK_KP_7 */
    case 0x0ffb8:                             /* XK_KP_8 */
    case 0x0ffb9:                             /* XK_KP_9 */
      ch0 = keyval & 0x3F;
      _textbuffer_insert(tbuf, &ch0, 1);
      goto redraw;

    case 0x0ffe1:                          /* XK_Shift_L */
    case 0x0ffe2:                          /* XK_Shift_R */
    case 0x0ffe3:                        /* XK_Control_L */
    case 0x0ffe4:                        /* XK_Control_R */
    case 0x0ffe5:                        /* XK_Caps_Lock */
    case 0x0ffe6:                       /* XK_Shift_Lock */
    case 0x0ffe9:                            /* XK_Alt_L */
    case 0x0ffea:                            /* XK_Alt_R */
    case 0x0ffeb:                          /* XK_Super_L */
      return false;

    case 0x0ffff:                           /* XK_Delete */
    case 0x0ff9f:                        /* XK_KP_Delete */
      tbuf->state ^= TBIT_KEY_DELETE;
      _textbuffer_delete(tbuf);
      goto redraw;

    default:
printf("unhandled key (0x%X)\n", keyval);
  }
  return false;

redraw:
  ui_invalidate_object(obj);
sbit:
  iface->state |= SBIT_RELEASE_IGNORE;
  return true;
}

/* First non-scroll button used as set focus. Feature to
  foreground a window or object without deselecting or position change. */
static bool
_textview_mouse(PhxInterface *iface,
                xcb_generic_event_t *nvt,
                PhxObject *obj)  {

  xcb_button_press_event_t *bp;
  bool shift_click, locus;
  uint8_t num_clicks;
  int16_t xbp, ybp;
  PhxObjectTextview *otxt;
  PhxTextbuffer     *tbuf;

  bp = (xcb_button_press_event_t*)nvt;
  locus = ((bp->response_type & (uint8_t)0x7F) == XCB_BUTTON_PRESS);

  if (!locus) {
      /* check if release occured on drag state. */
    if (_textview_drag_finish(iface, nvt, obj))
      return true;
  }

  if ( (bp->detail >= 4) && (bp->detail <= 7) ) {
    uint8_t button;
    bool kpcntl;
    if (!locus)  return true;

    otxt = (PhxObjectTextview*)ui_active_within_get();
    DEBUG_ASSERT((otxt->type != PHX_TEXTVIEW),
                       "within->type != PHX_TEXTVIEW. _textview_mouse().");
    if ((tbuf = (PhxTextbuffer*)otxt->exclusive) == NULL) {
      DEBUG_ASSERT(true, "textview has no textbuffer. _textview_mouse().");
      return true;
    }
    button = bp->detail;
    kpcntl = ((bp->state & XCB_MOD_MASK_CONTROL) != 0);
    if ( kpcntl || (button >= 6) ) {
      xbp = (uint16_t)otxt->draw_box.w / (uint16_t)10;
      ybp = 0;
      if ((button & 1) == 0) xbp = -xbp;
    } else {
      xbp = 0;
      ybp = (uint16_t)otxt->draw_box.h / (uint16_t)10;
      if ((button & 1) == 0) ybp = -ybp;
    }
    location_scroll_click(tbuf, xbp, ybp);
    obj = (PhxObject*)otxt;
    goto redraw;
  }

  num_clicks = iface->state & SBIT_CLICKS;
    /* Textview on press only accepts left button, others unhandled.
      On release multiple clicks dealt with on press, nothing to do. */
  if (locus) {
    if (bp->detail != 1)       return false;
  } else if (num_clicks != 1)  return true;

  otxt = (PhxObjectTextview*)obj;
  if ((tbuf = (PhxTextbuffer*)otxt->exclusive) == NULL) {
    DEBUG_ASSERT(true, "textview has no textbuffer. _textview_mouse().");
    return true;
  }
  xbp = bp->event_x - (otxt->mete_box.x + otxt->draw_box.x);
  ybp = bp->event_y - (otxt->mete_box.y + otxt->draw_box.y);
  if (xbp < 0)  xbp = 0;
  if (ybp < 0)  ybp = 0;

    /* flatten text_buffer, if was editing */
  _textbuffer_flush(tbuf);

  shift_click = ((bp->state & XCB_MOD_MASK_SHIFT) != 0);
  if (locus) {
    if      (num_clicks == 2)
      location_double_click(tbuf, shift_click);
    else if (num_clicks == 3)
      location_triple_click(tbuf, xbp, ybp, shift_click);
    else if (!shift_click)
      location_interim_equalize(tbuf, xbp, ybp);
    goto redraw;
  }

  DEBUG_ASSERT(locus, "failure: program logic. _textview_mouse(RELEASE).");

  if (shift_click) {
      /* Do nothing until release event, or motion */
    location_shift_click(tbuf, xbp, ybp);
  } else if (bp->detail == 1) {
    location_selection_equalize(tbuf);
  }

redraw:
  ui_invalidate_object(obj);
  return true;
}

/* As a textview, only respond to motion if focus was set.
  This implies a button clicked on, and cursor has't left (xing). */
static bool
_textview_motion(PhxInterface *iface,
                 xcb_generic_event_t *nvt,
                 PhxObject *obj)  {

  xcb_motion_notify_event_t *motion;
  PhxObjectTextview *otxt;

  motion = (xcb_motion_notify_event_t*)nvt;
    /* check for mouse passing through */
  if ((motion->state & XCB_BUTTON_MASK_1) == 0)
    return false;
    /* Special case: _textview_drag_cancel() with pointer above us.
      Can have button depressed, but may not have focus. */
  otxt = (PhxObjectTextview*)ui_active_focus_get();
  if (otxt == NULL)  return true;

  if (IS_IFACE_TYPE(otxt)) {
    DEBUG_ASSERT(true, "unhandled meter event... _textview_motion().");
    return false;
  }

  if (obj != (PhxObject*)otxt) {
      /* Has to be in a drag state. Test for developer errors. */
    if (obj == NULL) {
       /* Left PhxInterface, drag state. */
      if ((iface->state & SBIT_SELECTING) == 0)
        DEBUG_ASSERT((ui_active_drag_get() != (PhxObject*)otxt),
                     "unhandled NULL... _textview_motion().");
    } else if ( IS_IFACE_TYPE(obj)
               || ( ((obj->state & OBIT_DND_AWARE) != 0)
                   && ((obj->state & OBIT_DND) == 0) ) ) {
        /* Rule out in 'drag selection' entering another object
          before passing to 'dnd' handler. */
      return true;
    }
      /* An internal dnd event, motion coordinates from here use root */
    DEBUG_ASSERT(( (ui_active_drag_get() == NULL)
                  && ((iface->state & SBIT_SELECTING) == 0) ),
                     "unhandled drag motion... _textview_motion().");
  }

    /* Start selection or drag process */
  if (ui_active_drag_get() == NULL)
      /* For brevity of code, also used for SBIT_SELECTING. */
    return _textview_drag_begin(iface, nvt, obj);

  return _textview_drag_motion(iface, nvt, obj);
}

static bool
_textview_crossing(PhxInterface *iface,
                   xcb_generic_event_t *nvt,
                   PhxObject *obj)  {
  char *named;
  bool locus;

  if (_textview_drag_crossing(iface, nvt, obj))
    return true;

  named = NULL;
  locus = ((nvt->response_type & (uint8_t)0x7F) == XCB_ENTER_NOTIFY);
  if (locus) {
    obj->state &= ~(OBIT_DND_CARET | OBIT_DND_COPY | OBIT_DND);
    named = "text";
  }
  ui_cursor_set_named(named, iface->window);
  return true;
}

static bool
_textview_configure(PhxInterface *iface,
                    xcb_generic_event_t *nvt,
                    PhxObject *obj)  {

  xcb_configure_notify_event_t *configure;
  int16_t wD, hD;

  if ((obj->state & (HXPD_MSK | VXPD_MSK)) == 0)  return true;

  configure = (xcb_configure_notify_event_t*)nvt;
  wD = configure->width  - obj->mete_box.w;
  hD = configure->height - obj->mete_box.h;
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
  return true;
}

static bool
_textview_selection(PhxInterface *iface,
                    xcb_generic_event_t *nvt,
                    PhxObject *obj)  {

  uint8_t response_type;

  response_type = (nvt->response_type & (uint8_t)0x7F);

  if (response_type == XCB_SELECTION_CLEAR) {
    DEBUG_ASSERT(true, "unhandled XCB_SELECTION_CLEAR.");
    return false;
  }

  if (response_type == XCB_SELECTION_REQUEST) {
    xcb_selection_request_event_t *request
      = (xcb_selection_request_event_t*)nvt;
    if (request->selection == CLIPBOARD)
      _xclb_process_request(session->xclipboard, request);
    else
      DEBUG_ASSERT(true, "unhandled XCB_SELECTION_REQUEST.");
    return true;
  }

  if (response_type == XCB_SELECTION_NOTIFY) {
    xcb_selection_notify_event_t *notify
      = (xcb_selection_notify_event_t*)nvt;
    if (notify->property == XCBD_DATA) {
      PhxObjectTextview *otxt = (PhxObjectTextview*)obj;
      _textbuffer_paste_notify((PhxTextbuffer*)otxt->exclusive);
    }
#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
      else if (notify->property != XDND_SELECTION)
#else
      else
#endif
        /* default for XDND_SELECTION is do nothing */
      DEBUG_ASSERT(true, "unhandled XCB_SELECTION_NOTIFY.");
    return true;
  }

  if (response_type == XCB_CLIENT_MESSAGE)
    DEBUG_ASSERT(true, "unhandled XCB_CLIENT_MESSAGE.");
  return false;
}

bool
_default_textview_meter(PhxInterface *iface,
                        xcb_generic_event_t *nvt,
                        PhxObject *obj) {

  DEBUG_EVENTS("_default_textview_meter");

  switch (nvt->response_type & (uint8_t)0x7F) {

    case XCB_KEY_PRESS:           /* response_type 2 */
    case XCB_KEY_RELEASE:         /* response_type 3 */
      return _textview_keyboard(iface, nvt, obj);

    case XCB_BUTTON_PRESS:        /* response_type 4 */
    case XCB_BUTTON_RELEASE:      /* response_type 5 */
      return _textview_mouse(iface, nvt, obj);

    case XCB_MOTION_NOTIFY:       /* response_type 6 */
      return _textview_motion(iface, nvt, obj);

    case XCB_ENTER_NOTIFY:        /* response_type 7 */
    case XCB_LEAVE_NOTIFY:        /* response_type 8 */
      return _textview_crossing(iface, nvt, obj);

    case XCB_FOCUS_IN:            /* response_type 9 */
    case XCB_FOCUS_OUT:           /* response_type 10 */
    case XCB_EXPOSE: {            /* response_type 12 */
      break;
    }

    case XCB_DESTROY_NOTIFY:      /* response_type 17 */
    case XCB_UNMAP_NOTIFY:        /* response_type 18 */
    case XCB_MAP_NOTIFY:          /* response_type 19 */
    case XCB_REPARENT_NOTIFY: {   /* response_type 21 */
      break;
    }

    case XCB_CONFIGURE_NOTIFY:    /* response_type 22 */
      return _textview_configure(iface, nvt, obj);

    case XCB_PROPERTY_NOTIFY: {   /* response_type 28 */
      break;
    }

      /* SELECTION == dnd, clipboard */
    case XCB_SELECTION_CLEAR:     /* response_type 29 */
    case XCB_SELECTION_REQUEST:   /* response_type 30 */
    case XCB_SELECTION_NOTIFY:    /* response_type 31 */
    case XCB_CLIENT_MESSAGE: {    /* response_type 33 */
     _textview_selection(iface, nvt, obj);
      break;
    }

    default:
        /* Unknown event type, use to track coding problems */
      printf("Unknown event: %"PRIu8"\n", nvt->response_type);
      break;

  } /* end switch(response_type) */
  return false;
}

#pragma mark *** Exclusive ***

void
ui_textview_font_set(PhxObjectTextview *obj, char *font_name) {

  PhxAttr *attrib = obj->attrib;
  ui_attributes_set((PhxObject*)obj, font_name,
                            attrib->font_var & 0x0FF,
                           (attrib->font_var >> 8) & 0x0FF,
                            attrib->font_em);
  if ( (obj->exclusive != NULL)
      && (obj->type == PHX_TEXTVIEW) ) {
    free(((PhxTextbuffer*)obj->exclusive)->glyph_widths);
    ((PhxTextbuffer*)obj->exclusive)->glyph_widths
      = ui_textual_glyph_table((PhxObject*)obj);
  }
}

void
ui_textview_slant_set(PhxObjectTextview *obj, int font_slant) {

  PhxAttr *attrib = obj->attrib;
  ui_attributes_set((PhxObject*)obj, attrib->font_name,
                            attrib->font_var & 0x0FF,
                            font_slant,
                            attrib->font_em);
  if ( (obj->exclusive != NULL)
      && (obj->type == PHX_TEXTVIEW) ) {
    free(((PhxTextbuffer*)obj->exclusive)->glyph_widths);
    ((PhxTextbuffer*)obj->exclusive)->glyph_widths
      = ui_textual_glyph_table((PhxObject*)obj);
  }
}

void
ui_textview_weight_set(PhxObjectTextview *obj, int font_weight) {

  PhxAttr *attrib = obj->attrib;
  ui_attributes_set((PhxObject*)obj, attrib->font_name,
                            font_weight,
                           (attrib->font_var >> 8) & 0x0FF,
                            attrib->font_em);
  if ( (obj->exclusive != NULL)
      && (obj->type == PHX_TEXTVIEW) ) {
    free(((PhxTextbuffer*)obj->exclusive)->glyph_widths);
    ((PhxTextbuffer*)obj->exclusive)->glyph_widths
      = ui_textual_glyph_table((PhxObject*)obj);
  }
}

void
ui_textview_font_em_set(PhxObjectTextview *obj, int line_height) {

  PhxAttr *attrib = obj->attrib;
  ui_attributes_set((PhxObject*)obj, attrib->font_name,
                            attrib->font_var & 0x0FF,
                           (attrib->font_var >> 8) & 0x0FF,
                            line_height);
  if ( (obj->exclusive != NULL)
      && (obj->type == PHX_TEXTVIEW) ) {
    free(((PhxTextbuffer*)obj->exclusive)->glyph_widths);
    ((PhxTextbuffer*)obj->exclusive)->glyph_widths
      = ui_textual_glyph_table((PhxObject*)obj);
  }
}

void
ui_textview_buffer_set(PhxObjectTextview *otxt, char *data, int jstfy) {

  (void)jstfy; /* not implemented */
  ui_textbuffer_create(otxt, data);
}

#pragma mark *** Creation ***

void
_default_textview_raze(void *obj) {

  PhxObjectTextview *otxt = (PhxObjectTextview*)obj;
  _default_textbuffer_raze((PhxTextbuffer*)otxt->exclusive);
#if DND_INTERNAL_ON
  if (otxt->dnd_quirk->watypes_count > 1)
    free(otxt->dnd_quirk->watypes);
#endif
  free(otxt->exclusive);
}

PhxObjectTextview *
ui_textview_create(PhxNexus *nexus, PhxRectangle configure) {

  PhxObjectTextview *obj
    = (PhxObjectTextview*)ui_object_create(nexus, PHX_TEXTVIEW,
                                            _textview_draw, configure);

  obj->state |= OBIT_DND_AWARE;
  obj->state |= OBIT_FOCUS_ONCLICK;
  obj->_event_cb = _default_textview_meter;
  obj->_raze_cb  = _default_textview_raze;

    /* default colours: bg_fill, fg_fill transparent. */
  obj->attrib->fg_fill.a = 0;

    /* set a default, 14px line height */
  ui_textview_font_em_set(obj, 14);

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
  obj->dnd_quirk = malloc(sizeof(xcb_dnd_property_t));
    /* send/receive of data */
  obj->dnd_quirk->_sel_get_cb = _textview_selection_data_get;
  obj->dnd_quirk->_sel_set_cb = _textview_selection_data_set;
    /* This handles XdndAction... types source can do. */
  obj->dnd_quirk->_sel_act_cb = _textview_selection_action;
    /* does copy/move: 1  copy only : 0 */
  obj->dnd_quirk->src_actions = 1;
    /* only sends/receives UTF8_STRING data */
  obj->dnd_quirk->watypes_count = 1;
  obj->dnd_quirk->watypes = &UTF8_STRING;
    /* POSITION supplied updates of display */
  obj->_obj_status_cb = _dnd_status_cb;
#endif

  return obj;
}
