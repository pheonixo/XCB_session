#include "banks.h"
#include "buttons.h"
#include "configure.h"

#pragma mark *** Utilities ***

PhxBank *
ui_dropdown_from(PhxObject *obj) {

  uint16_t        idx;
  PhxButtonStyle  style;

    /* A child object may be passed. Combo desibn of only one level depth. */
  if (obj != obj->o_mount)
    obj = obj->o_mount;
  else {
    PhxInterface *inspect = _interface_for(_window_for(obj));
    if (OBJECT_BASE_TYPE(inspect) == PHX_IBANK)
    return inspect;
  }

  if (OBJECT_BASE_TYPE(obj) != PHX_BUTTON) {
    DEBUG_ASSERT(true, "failure: no designed dropdown.");
    return NULL;
  }
  style = obj->type >> 8;
  if (!((style == BTN_COMBO_ARROW) || (style == BTN_COMBO_WHEEL))) {
    DEBUG_ASSERT(true, "failure: no designed dropdown.");
    return NULL;
  }

    /* Scan session for dropdown that matches object. */
  idx = session->ncount;
  do {
    PhxInterface *inspect = session->iface[(--idx)];
    if (inspect->type == ((DDL_COMBO << 8) | PHX_IBANK)) {
      if (((PhxVault*)inspect->exclusive)->actuator == obj)
        return inspect;
    }
  } while (idx != 0);
  return NULL;
}

static void
_attrib_colours_swap(PhxAttr *dst, PhxAttr *src) {
  PhxRGBA swap[3];
  memmove(&swap,         &dst->bg_fill, (3 * sizeof(PhxRGBA)));
  memmove(&dst->bg_fill, &src->bg_fill, (3 * sizeof(PhxRGBA)));
  memmove(&src->bg_fill, &swap,         (3 * sizeof(PhxRGBA)));
}

/* Called when object added to bank. */
void
ui_bank_content_update(PhxBank *ibank) {

  PhxRectangle configure = { 0 };
  uint16_t     odx, wD, hD;
  PhxNexus     *nexus = ibank->nexus[0];

  for (odx = 0; odx < nexus->ncount; odx++) {
    PhxObject *inspect = nexus->objects[odx];
    if (!visible_get(inspect))  continue;
    configure.w = maxof(configure.w, inspect->mete_box.w);
    configure.h = maxof(configure.h, (inspect->mete_box.y
                                     + inspect->mete_box.h));
  }

  if (ui_window_minimum_get(ibank->window, &wD, &hD)) {
    configure.w = maxof(configure.w, wD);
    configure.h = maxof(configure.h, hD);
  } else {
    configure.w = maxof(configure.w, 1);
    configure.h = maxof(configure.h, 1);
  }
  if (ui_window_maximum_get(ibank->window, &wD, &hD)) {
    configure.w = minof(configure.w, wD);
    configure.h = minof(configure.h, hD);
  }

  wD = configure.w - ibank->mete_box.w;
  hD = configure.h - ibank->mete_box.h;

  if ((wD != 0) || (hD != 0))
    _interface_configure(ibank, wD, hD);

  ui_actuator_content_update(ibank);
}

void
ui_actuator_content_update(PhxBank *ibank) {

  PhxObject *new_obj = NULL;
  PhxNexus *nexus = ibank->nexus[0];
  PhxVault *vault = (PhxVault*)ibank->exclusive;

  if (vault->on_idx != (uint16_t)~0) {
    PhxRectangle configure;
set_actuator:
    new_obj = malloc(sizeof(PhxObject));
    memmove(new_obj, nexus->objects[vault->on_idx], sizeof(PhxObject));

    _button_draw_area_request(vault->actuator, &configure);
    new_obj->mete_box = configure;

      /* actuator 'features' may be different from bank's, adjust. */
    new_obj->state = (new_obj->state
                       & ~(HXPD_MSK | VXPD_MSK | HJST_MSK | VJST_MSK))
                    | (vault->actuator->state
                        & (HXPD_MSK | VXPD_MSK | HJST_MSK | VJST_MSK));
      /* adjust new to reflect ownership of its placement. */
    new_obj->i_mount = vault->actuator->i_mount;
    new_obj->o_mount = vault->actuator;
  } else {
    if (nexus->ncount != 0) {
      uint16_t idx = 0;
      do {
        PhxObject *inspect = nexus->objects[idx];
        if ( (visible_get(inspect))
            && (sensitive_get(inspect)) ) {
          vault->on_idx = idx;
          vault->actuator->ncount = 1;
          goto set_actuator;
        }
      } while ((++idx) < nexus->ncount);
    }
    vault->actuator->ncount = 0;
  }
  if (vault->actuator->child != NULL)
    free(vault->actuator->child);
  vault->actuator->child = new_obj;
}

#pragma mark *** Events ***

bool
_default_bank_run(PhxBank *ibank, xcb_generic_event_t *nvt, PhxObject *obj) {

  uint8_t response = (nvt->response_type & (uint8_t)0x7F);
  PhxNexus *nexus = ibank->nexus[0];
  PhxVault *vault = ibank->exclusive;
  PhxObject *vObj;

  if (response == (uint8_t)XCB_MOTION_NOTIFY) {
    if (obj == NULL) {
      if (vault->in_idx != (uint16_t)~0) {
        vObj = nexus->objects[vault->in_idx];
        _attrib_colours_swap(vObj->attrib, (PhxAttr*)&vault->bg_fill);
        vault->in_idx = (uint16_t)~0;
        ui_invalidate_object((PhxObject*)ibank);
      }
    } else {
      if (!sensitive_get(obj)) {
        if (vault->in_idx != (uint16_t)~0) {
          vObj = nexus->objects[vault->in_idx];
          _attrib_colours_swap(vObj->attrib, (PhxAttr*)&vault->bg_fill);
          vault->in_idx = (uint16_t)~0;
          ui_invalidate_object((PhxObject*)ibank);
        }
        return true;
      }
      if (vault->in_idx == (uint16_t)~0)
        goto start;
      if (nexus->objects[vault->in_idx] != obj) {
        if (vault->in_idx != (uint16_t)~0) {
          vObj = nexus->objects[vault->in_idx];
          _attrib_colours_swap(vObj->attrib, (PhxAttr*)&vault->bg_fill);
        }
  start:
        vault->in_idx = nexus->ncount;
        while (nexus->objects[(--vault->in_idx)] != obj) ;
        vObj = nexus->objects[vault->in_idx];
        _attrib_colours_swap(vObj->attrib, (PhxAttr*)&vault->bg_fill);
        ui_invalidate_object((PhxObject*)ibank);
      }
    }
    return true;
  }

  if (response == (uint8_t)XCB_BUTTON_PRESS) {
    xcb_ungrab_pointer(session->connection, XCB_CURRENT_TIME);
    xcb_unmap_window(session->connection, ibank->window);
    xcb_flush(session->connection);
    if (obj == NULL) {
      if (vault->in_idx != (uint16_t)~0) {
        vObj = nexus->objects[vault->in_idx];
        _attrib_colours_swap(vObj->attrib, (PhxAttr*)&vault->bg_fill);
        vault->in_idx = (uint16_t)~0;
        vault->was_idx = vault->on_idx;
      }
    } else  if (sensitive_get(obj)) {
      vault->in_idx = nexus->ncount;
      while (nexus->objects[(--vault->in_idx)] != obj) ;
      vObj = nexus->objects[vault->in_idx];
      _attrib_colours_swap(vObj->attrib, (PhxAttr*)&vault->bg_fill);
      vault->was_idx = vault->on_idx;
      vault->on_idx  = vault->in_idx;
      if (vault->was_idx != vault->on_idx)
        ui_actuator_content_update(ibank);
    }
    ui_invalidate_object((PhxObject*)ibank->i_mount);
      /* send results of run to caller */
    if (vault->_result_cb != NULL)  vault->_result_cb(ibank);

    return false;
  }
  return false;
}

/* 'obj' should be actuator */
bool
_default_bank_meter(PhxInterface *iface,
                    xcb_generic_event_t *nvt,
                    PhxObject *obj) {

  xcb_button_press_event_t *mouse;
  int16_t x, y;
  PhxRectangle dbox;

  if ( ((nvt->response_type & (uint8_t)0x7F) != (uint8_t)XCB_BUTTON_RELEASE)
      || ((obj->state & OBIT_BTN_PRESS) == 0) )
    return _default_button_meter(iface, nvt, obj);
  mouse = (xcb_button_press_event_t*)nvt;
  if (mouse->detail != 1)
    return _default_button_meter(iface, nvt, obj);

  if (obj->ncount == 0)
    return _default_button_meter(iface, nvt, obj);

    /* Make sure pressed in 'event' area, the button draw_box. */
  x = mouse->event_x;
  y = mouse->event_y;
  dbox = obj->draw_box;
  dbox.x += obj->mete_box.x;
  dbox.y += obj->mete_box.y;
  if ( (x >= dbox.x) && (x < (dbox.x + dbox.w))
      && (y >= dbox.y) && (y < (dbox.y + dbox.h)) ) {
    uint32_t values[4];
    PhxBank *inspect, *dropdown = ui_dropdown_from(obj);
    PhxRectangle configure = dropdown->draw_box;
    PhxVault *vault = (PhxVault*)dropdown->exclusive;

      /* iface is that of the actuator's. */
      /* Place draw_box at upper-left of actuator's content. */
    _button_draw_area_request((PhxObjectButton*)obj, &dbox);
      /* Adjust dbox tp nexus coordinates */
    dbox.x += obj->mete_box.x;
    dbox.y += obj->mete_box.y;
      /* window + button + draw positioning + depth into bank */
    configure.x += iface->mete_box.x + dbox.x;
    configure.y += iface->mete_box.y + dbox.y;
    inspect = (PhxInterface*)obj;
    while (!IS_WINDOW_TYPE((inspect = inspect->i_mount))) {
      configure.x += inspect->mete_box.x;
      configure.y += inspect->mete_box.y;
    }
      /* make last check for changes. */
    ui_bank_content_update(dropdown);
      /* Base dropdown on active button selection. */
    configure.y -= dropdown->nexus[0]->objects[vault->on_idx]->mete_box.y;
      /* XXX need positional adjusts for ibank placement at screen limits */
    dropdown->mete_box = configure;
    values[0] = configure.x;
    values[1] = configure.y;
    values[2] = dropdown->mete_box.w;
    values[3] = dropdown->mete_box.h;
    xcb_configure_window(session->connection, dropdown->window,
                 XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                 XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(session->connection);

      /* dbox was changed to displayed region, same as dropdown 'in' region. */
    vault->in_idx = (uint16_t)~0;
    vault->was_idx = vault->on_idx;
    if ( (x >= dbox.x) && (x < (dbox.x + dbox.w))
        && (y >= dbox.y) && (y < (dbox.y + dbox.h)) ) {
      PhxObject *vObj;
      vault->in_idx = vault->on_idx;
      vObj = dropdown->nexus[0]->objects[vault->in_idx];
      _attrib_colours_swap(vObj->attrib, (PhxAttr*)&vault->bg_fill);
    }
    xcb_map_window(session->connection, dropdown->window);
  }
  return _default_button_meter(iface, nvt, obj);
}

#pragma mark *** Alterations ***

void
ui_bank_add_result_cb(PhxObject *actuator, PhxResultHandler rcb) {

  PhxBank *ibank = ui_dropdown_from(actuator);
  if (ibank == NULL) {
    DEBUG_ASSERT(true, "error: Bad interface. ui_bank_add_result_cb()");
    return;
  }
  ((PhxVault*)ibank->exclusive)->_result_cb = rcb;
}

void
ui_bank_remove_object(PhxBank *ibank, uint16_t idx) {

  int16_t yD;
  bool is_combo;
  PhxVault *vault = (PhxVault*)ibank->exclusive;
  PhxNexus *nexus = ibank->nexus[0];
  PhxObject *on_obj;

  if (idx >= nexus->ncount) {
    DEBUG_ASSERT(true, "error: member not in bank. ui_bank_remove_object()");
    return;
  }

  on_obj = nexus->objects[idx];
  yD = on_obj->mete_box.h;
    /* Adjust window and drawing port. Done for debugging. Actual size
      calculations done pre-mapping of dropdown (ibank). */
  ibank->mete_box.h -= yD;
  ibank->draw_box.h -= yD;
  nexus->mete_box.h -= yD;
  nexus->draw_box.h -= yD;

  _default_nexus_remove_object(nexus, on_obj);

  is_combo = (OBJECT_STYLE(ibank) == DDL_COMBO);

    /* By design, vault->on_idx can not be hidden, nor insensitive. */
  vault->on_idx = (uint16_t)~0;
  if (nexus->ncount == 0) {
    if (is_combo) {
      free(vault->actuator->child);
      vault->actuator->child = NULL;
      vault->actuator->ncount = 0;
      ui_invalidate_object(vault->actuator);
    }
    return;
  }
    /* Walk to top of stack, then down the stack. */
    /* when idx == ncount then was last obj in stack removed */
  if (idx < nexus->ncount) {
    uint16_t tdx;
      /* Firstly, remove on_obj's height from all follwing positions.
        Reminder: _default_nexus_remove_object() ran so adjustments
        start at idx. 'ncount' was also reduced. */
    for (tdx = idx; tdx < nexus->ncount; tdx++)
      nexus->objects[tdx]->mete_box.y -= yD;
      /* Find the new 'vault->on_idx' value. */
    tdx = idx;
    do {
      PhxObject *tObj = nexus->objects[tdx];
      if ( (visible_get(tObj)) && (sensitive_get(tObj)) ) {
        vault->on_idx = tdx;
        goto ctest;
      }
    } while ((++tdx) < nexus->ncount);
  }
  if (idx != 0) {
    do {
      PhxObject *tObj = nexus->objects[(--idx)];
      if ( (visible_get(tObj)) && (sensitive_get(tObj)) ) {
        vault->on_idx = idx;
        goto ctest;
      }
    } while (idx != 0);
  }
ctest:
  if (is_combo) {
    ui_actuator_content_update(ibank);
    ui_invalidate_object(vault->actuator);
  }
}

/* Normally not directly accessed. Is add on to sensitive_set(). */
void
ui_bank_insensitive_set(PhxObject *obj) {

  PhxBank *ibank;
  PhxNexus *nexus;
  PhxVault *vault;
  uint16_t idx;

    /* Assume that not called from sensitve_set(). */
  obj->state |= OBIT_SENSITIVE;

  if (obj->o_mount != obj) {
    PhxObject *parent = obj->o_mount;
    if ( (parent->type != ((BTN_COMBO_ARROW << 8) | PHX_BUTTON))
        && (parent->type != ((BTN_COMBO_WHEEL << 8) | PHX_BUTTON)) )
      return;
      /* Must get actual object this child represents. */
    ibank = ui_dropdown_from(parent);
    nexus = ibank->nexus[0];
    vault = (PhxVault*)ibank->exclusive;
    obj = nexus->objects[vault->on_idx];
      /* set the actual object to requested. */
    obj->state |= OBIT_SENSITIVE;
      /* object is now the bank member in need of attention. */
  } else if (ui_window_is_transient(_window_for(obj))) {
    ibank = _interface_for(_window_for(obj));
    if ( (ibank == NULL)
        || (OBJECT_STYLE(ibank) != DDL_COMBO) ) return;
      /* object is bank member and of testing type. */
    nexus = ibank->nexus[0];
    vault = (PhxVault*)ibank->exclusive;
    if (nexus->objects[vault->on_idx] != obj)  return;
      /* object is now the bank member in need of attention. */
  } else {
      /* not a bank member */
    return;
  }
    /* obj is displayed combo button content. It has become
      insensitive, so remove from combo content and display
      the next sequential sensitive object. */
  idx = vault->on_idx;
  vault->on_idx = (uint16_t)~0;
    /* Walk to top of stack, then down the stack. */
  if (idx < nexus->ncount) {
    uint16_t tdx = idx;
    do {
      PhxObject *tObj = nexus->objects[tdx];
      if ( (visible_get(tObj)) && (sensitive_get(tObj)) ) {
        vault->on_idx = tdx;
        goto ctest;
      }
    } while ((++tdx) < nexus->ncount);
  }
  if (idx != 0) {
    do {
      PhxObject *tObj = nexus->objects[(--idx)];
      if ( (visible_get(tObj)) && (sensitive_get(tObj)) ) {
        vault->on_idx = idx;
        goto ctest;
      }
    } while (idx != 0);
  }
ctest:
  ui_actuator_content_update(ibank);
  ui_invalidate_object(vault->actuator);
}

#pragma mark *** Creation ***

/* Places newly created object at requested 'configure'. */
/* Note: extra updating code for bank-type in ui_object_create(). */
PhxObjectLabel *
ui_bank_label_create(PhxBank *ibank,
                      PhxRectangle configure,
                      char *text,
                      uint32_t jstfy) {

  bool transfer = ( (OBJECT_STYLE(ibank) == DDL_COMBO)
               && (((PhxVault*)ibank->exclusive)->actuator->child == NULL) );
    /* During creation, an object gets added to bank. If it's the
      first object, and of DDL_COMBO, object gets added as actuator's child.
      We convert object to label, but after it's been added to
      actuator. Compensate! */
  PhxObjectLabel *olbl
    = ui_label_create(ibank->nexus[0], configure, text, jstfy);
  if (transfer) {
    ((PhxVault*)ibank->exclusive)->actuator->child->exclusive
      = olbl->exclusive;
  }
  return olbl;
}

/* Places newly created object at requested 'configure'. */
/* Note: extra updating code for bank-type in ui_object_create(). */
PhxObject *
ui_bank_object_create(PhxBank *ibank,
                      PhxObjectType type,
                      PhxDrawHandler draw,
                      PhxRectangle configure) {
  PhxObject *obj;
    /* Create requested object. */
  obj = ui_object_create(ibank->nexus[0], type, draw, configure);
    /* Logic simular to label, default to transparent fills. */
  obj->attrib->bg_fill.a = 0;
  obj->attrib->fg_fill.a = 0;

  return obj;
}

/* This is for the child of an actuator button, used for display.
  Is a copy of actual bank member with minor changes. This only
  free(s) the copy. Internals belong to actual member. */
static void
_actuator_raze_cb(void *obj) {

  PhxObjectButton *obtn = (PhxObjectButton*)obj;
  if (obtn->child != NULL) {
    free(obtn->child);
    obtn->ncount -= 1;
  }
  obtn->child = NULL;
  _default_button_raze(obj);
}

/* Should be called on combo button destroy or application shutdown. */
/* Active combo child is a copy of one of bank's members. We free our copy
  as normal raze, set actuator's child to NULL. */
void
_default_bank_raze(void *obj) {

  PhxBank     *bank = (PhxBank*)obj;
  PhxVault    *vault  = (PhxVault*)bank->exclusive;
  PhxObject   *actuator = vault->actuator;
  xcb_window_t window;

  if ( (actuator != NULL)
      && ( (actuator->type == ((BTN_COMBO_ARROW << 8) | PHX_BUTTON))
          || (actuator->type == ((BTN_COMBO_WHEEL << 8) | PHX_BUTTON)) ) ) {
    if (actuator->child != NULL) {
      free(actuator->child);
      actuator->ncount -= 1;
    }
    actuator->child = NULL;
  }

  free(vault);
  bank->exclusive = NULL;

  bank->_raze_cb = _default_interface_raze;
  window = bank->window;
  _interface_remove_for(window);
  xcb_destroy_window(session->connection, window);
}

/*
  switch type
  _NET_WM_WINDOW_TYPE_DROPDOWN_MENU:
      actuator != NULL use button LL corner anchor
  _NET_WM_WINDOW_TYPE_POPUP_MENU:
      right-click: content area, pointer position, TR anchor
  _NET_WM_WINDOW_TYPE_TOOLTIP:
      hover timed, location marked, TR anchor
  _NET_WM_WINDOW_TYPE_COMBO:
      actuator != NULL, button draw_box LR restricted, position on last active

Note: Possibility of another dropdown type. One that has content that can
 drag objects out of bank to copy. Is this a desired dropdown???
      Probably should be just a window.
*/

PhxBank *
ui_bank_create(PhxObject *actuator,
               PhxDDLStyle window_type,
               PhxResultHandler result_cb) {

  PhxRectangle configure = { 0, 0, 1, 1 };
  xcb_window_t window;
  PhxBank      *ibank;
  PhxVault     *vault;
  PhxNexus     *nexus;

            /* Needs pre-existing conditions to create. */
  if (actuator == NULL) {
    DEBUG_ASSERT(true, "error: no actuator connection. ui_bank_create()");
    return NULL;
  }

            /* Current type of design only coded for */
  if (window_type != DDL_COMBO) {
    DEBUG_ASSERT(true, "error: Bad window_type. ui_bank_create()");
    return NULL;
  }
  if ( (actuator->type != ((BTN_COMBO_ARROW << 8) | PHX_BUTTON))
      && (actuator->type != ((BTN_COMBO_WHEEL << 8) | PHX_BUTTON)) ) {
    DEBUG_ASSERT(true, "error: Bad actuator. ui_bank_create()");
    return NULL;
  }

  ibank = ui_dropdown_from(actuator);
  if (ibank != NULL) {
    DEBUG_ASSERT(true, "warning: bank already exists. ui_bank_create()");
    if ( (((PhxVault*)ibank->exclusive)->_result_cb == NULL)
        && (result_cb != NULL) )
      ((PhxVault*)ibank->exclusive)->_result_cb = result_cb;
    return ibank;
  }

    /* Any object added to a DDL_COMBO is dependant on button content.
      As good a place to start for DDL_COMBO type. */
  _button_draw_area_request(actuator, &configure);
  configure.x = (configure.y = 0);

    /* Create window/interface. */
  window = ui_dropdown_create(configure, _window_for(actuator));
  ibank = _interface_for(window);
    /* Dropdowns belong or are created for an object. Take on
      object's attributes. */
  free(ibank->attrib->font_name);
  memmove(ibank->attrib, actuator->attrib, sizeof(PhxAttr));
  ibank->attrib->font_name = strdup(actuator->attrib->font_name);
    /* Need a port to display content. */
  nexus = ui_nexus_create(ibank, configure);
  nexus->state |= HXPD_RGT | VXPD_BTM;
  nexus->_draw_cb = NULL;
  nexus->attrib->bg_fill.r = (double)0x0F0/0x0FF;
  nexus->attrib->bg_fill.g = (double)0x0F0/0x0FF;
  nexus->attrib->bg_fill.b = (double)0x0F0/0x0FF;
  nexus->attrib->bg_fill.a = 1.0;
    /* No foreground fill on base nexus as default. */
  nexus->attrib->fg_fill.a = 0;

  vault = (PhxVault*)(ibank->exclusive = malloc(sizeof(PhxVault)));
  memset(vault, 0, sizeof(PhxVault));
  vault->actuator = actuator;
  vault->_result_cb = result_cb;
  vault->in_idx  = (uint16_t)~0;
  vault->on_idx  = (uint16_t)~0;
  vault->was_idx = (uint16_t)~0;
  vault->bg_fill = nexus->attrib->bg_fill;
  vault->fg_fill = RGBA_SELECTION;
  vault->fg_ink  = nexus->attrib->fg_ink;

            /* Specific to combo type button */

    /* Set generic dropdown to our use (combo type button). */
  ibank->type = (DDL_COMBO << 8) | PHX_IBANK;
  ibank->_event_cb = _default_bank_run;
  ibank->_raze_cb  = _default_bank_raze;
  actuator->_raze_cb = _actuator_raze_cb;
  actuator->_event_cb = _default_bank_meter;

  return ibank;
}
