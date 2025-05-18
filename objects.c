#include "objects.h"
#include "configure.h"
#include "banks.h"

void
visible_set(PhxObject *obj, bool visible) {
  if (visible)    obj->state &= ~OBIT_VISIBLE;
  else            obj->state |=  OBIT_VISIBLE;
}

__inline bool
visible_get(PhxObject *obj) {
  return ((obj->state & OBIT_VISIBLE) == 0);
}

/* Basic use for dropdowms. Signals active and selectable.
  Uses on navigation buttons. */
void
sensitive_set(PhxObject *obj, bool sensitive) {

  if (sensitive)  obj->state &= ~OBIT_SENSITIVE;
  else {          obj->state |=  OBIT_SENSITIVE;
      /* Test if related to a combo button */
    ui_bank_insensitive_set(obj);
  }
}

__inline bool
sensitive_get(PhxObject *obj) {
  return ((obj->state & OBIT_SENSITIVE) == 0);
}

#pragma mark *** Drawing ***

void
ui_draw_vertical_line(PhxObject *b, cairo_t *cr) {

  PhxObject *mount;
  double x, y;
  PhxAttr *c = b->attrib;

  x = b->mete_box.x;
  y = b->mete_box.y;

    /* If a child, must include parent's mete.
      Child is in object's coordinates*/
  mount = b;
  while (mount != mount->o_mount) {
    mount = mount->o_mount;
    x += mount->mete_box.x;
    y += mount->mete_box.y;
  }

  cairo_rectangle(cr, x, y, b->mete_box.w, b->mete_box.h);
  cairo_clip_preserve(cr);

  cairo_set_source_rgba(cr, c->bg_fill.r,
                            c->bg_fill.g,
                            c->bg_fill.b,
                            c->bg_fill.a);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, c->fg_ink.r,
                            c->fg_ink.g,
                            c->fg_ink.b,
                            c->fg_ink.a);
  x += b->draw_box.x;
  y += b->draw_box.y;
  cairo_rectangle(cr, x, y, c->stroke, b->draw_box.h);
  cairo_fill(cr);
}

void
ui_draw_horizontal_line(PhxObject *b, cairo_t *cr) {

  PhxObject *mount;
  double x, y;
  PhxAttr *c = b->attrib;

  x = b->mete_box.x;
  y = b->mete_box.y;

    /* If a child, must include parent's mete.
      Child is in object's coordinates*/
  mount = b;
  while (mount != mount->o_mount) {
    mount = mount->o_mount;
    x += mount->mete_box.x;
    y += mount->mete_box.y;
  }

  cairo_rectangle(cr, x, y, b->mete_box.w, b->mete_box.h);
  cairo_clip_preserve(cr);

  cairo_set_source_rgba(cr, c->bg_fill.r,
                            c->bg_fill.g,
                            c->bg_fill.b,
                            c->bg_fill.a);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, c->fg_ink.r,
                            c->fg_ink.g,
                            c->fg_ink.b,
                            c->fg_ink.a);
  x += b->draw_box.x;
  y += b->draw_box.y;
  cairo_rectangle(cr, x, y, b->draw_box.w, c->stroke);
  cairo_fill(cr);
}

void
ui_draw_right_arrow(PhxObject *b, cairo_t *cr) {

  PhxObject *mount;
  double x, y, w, h, sp;
  PhxAttr *c = b->attrib;

  x = b->mete_box.x;
  y = b->mete_box.y;

    /* If a child, must include parent's mete.
      Child is in object's coordinates*/
  mount = b;
  while (mount != mount->o_mount) {
    mount = mount->o_mount;
    x += mount->mete_box.x;
    y += mount->mete_box.y;
  }

  cairo_rectangle(cr, x, y, b->mete_box.w, b->mete_box.h);
  cairo_clip_preserve(cr);

  cairo_set_source_rgba(cr, c->bg_fill.r,
                            c->bg_fill.g,
                            c->bg_fill.b,
                            c->bg_fill.a);
  cairo_fill(cr);

  cairo_set_source_rgba(cr, c->fg_ink.r,
                            c->fg_ink.g,
                            c->fg_ink.b,
                            c->fg_ink.a);

  x += b->draw_box.x + 0.5;
  y += b->draw_box.y + 0.5;
  w  = b->draw_box.w - 1.0;
  h  = b->draw_box.h - 1.0;
  sp = h / 5;

  cairo_move_to(cr, (x + w - sp + 0.5), (y + (h / 2)));
  cairo_line_to(cr,  x + sp + 0.5,       y + sp);
  cairo_line_to(cr,  x + sp + 0.5,      (y + h - sp));
  cairo_line_to(cr, (x + w - sp + 0.5), (y + (h / 2)));
  cairo_fill(cr);
}

#pragma mark *** Creation ***

void
_default_object_raze(void *in_obj) {

  PhxObject *obj = (PhxObject*)in_obj;

  free(obj->attrib->font_name);
  free(obj->attrib);
  obj->attrib = NULL;               /* For debug */

  DEBUG_ASSERT((obj == ui_active_drag_get()),
                     "failure: has_drag on object termination.");
  DEBUG_ASSERT((obj == ui_active_within_get()),
                     "failure: within on object termination.");
  DEBUG_ASSERT((obj == ui_active_focus_get()),
                     "failure: has_focus on object termination.");
  free(obj);
}

PhxObject *
ui_object_child_create(PhxObject *obj,
                       PhxObjectType type,
                       PhxDrawHandler draw,
                       PhxRectangle configure) {

  PhxObject *object;

  if (IS_IFACE_TYPE(obj)) {
    DEBUG_ASSERT(true, "error: non-object attachment."
                            "try ui_object_create()");
    return NULL;
  }
  if (obj->child != NULL) {
    DEBUG_ASSERT(true, "error: has child. try deeper nesting? or raze?");
    return NULL;
  }
  if (obj->ncount != 0) {
    DEBUG_ASSERT(true, "error: has ncount. ?!?");
    return NULL;
  }
  if ( (configure.w <= 0) || (configure.h <= 0) ) {
    if (draw != NULL) {
      DEBUG_ASSERT(true, "error: nil size object request.");
      return NULL;
    }
      /* Possible event or data attachment. */
    DEBUG_ASSERT(true, "Warning: non-drawing object request.");
  }

  object = malloc(sizeof(PhxObject));
  if (object == NULL)  return NULL;
  memset(object, 0, sizeof(PhxObject));

  object->type = type;
  object->state = obj->state;
  object->mete_box = configure;
  RECTANGLE(object->draw_box, 0, 0, configure.w, configure.h);

  object->_draw_cb = draw;
  /* object->_event_cb = NULL; */
  object->_raze_cb  = _default_object_raze;

  object->i_mount = obj->i_mount;
  object->o_mount = obj;
    /* Attribute defaults */
  object->attrib = (PhxAttr*)malloc(sizeof(PhxAttr));
  if (object->attrib == NULL) {  free(object); return NULL;  }
  memmove(object->attrib, obj->attrib, sizeof(PhxAttr));
  object->attrib->font_name = strdup(obj->attrib->font_name);

  obj->ncount = 1;

  return object;
}

PhxObject *
ui_object_create(PhxNexus *nexus,
                 PhxObjectType type,
                 PhxDrawHandler draw,
                 PhxRectangle configure) {

  PhxObject *object;
  PhxInterface *iface;

  if (OBJECT_BASE_TYPE(nexus) != PHX_NEXUS) {
    DEBUG_ASSERT(true, "error: nexus unknown. ui_object_create()");
    return NULL;
  }
  if (_obj_alloc_test((void**)&nexus->objects,
                      sizeof(PhxObject*), nexus->ncount))
    return NULL;

  if ( (configure.w <= 0) || (configure.h <= 0) ) {
    DEBUG_ASSERT(true, "error: nil size object request. ui_object_create()");
    return NULL;
  }

  object = malloc(sizeof(PhxObject));
  if (object == NULL)  return NULL;
  memset(object, 0, sizeof(PhxObject));

  object->type = type;
  object->state = nexus->state;
  object->mete_box = configure;
  RECTANGLE(object->draw_box, 0, 0, configure.w, configure.h);

  object->_draw_cb = draw;
  /* object->_event_cb = NULL; */
  object->_raze_cb  = _default_object_raze;

  object->i_mount = (PhxInterface*)nexus;
    /* Attribute defaults */
  object->attrib = (PhxAttr*)malloc(sizeof(PhxAttr));
  if (object->attrib == NULL) {  free(object); return NULL;  }
  memmove(object->attrib, nexus->attrib, sizeof(PhxAttr));
  object->attrib->font_name = strdup(nexus->attrib->font_name);
  object->o_mount = object;

  nexus->objects[nexus->ncount] = object;
  nexus->ncount += 1;

    /* If a PhxBank, special updating */
  iface = _interface_for(nexus->window);
  if (OBJECT_BASE_TYPE(iface) == PHX_IBANK) {
      /* resizes both window and its nexus.
        For default, added objects of banks do not follow nexus resize.
        Force 'static' expand of new object. */
    object->state &= ~(HXPD_MSK | VXPD_MSK);
      /* XXX: one directional bank set up. */
    ui_bank_content_update(iface);
  }

  return object;
}
