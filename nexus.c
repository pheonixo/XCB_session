#include "nexus.h"
#include "objects.h"

/* draw_demo.c */
extern void  _demo_draw(PhxObject *obj, cairo_t *cr);

#pragma mark *** Events ***

/* XXX add visiblity */
/* Nexus has only default action for configure. */
bool
_default_nexus_meter(PhxInterface *iface,
                     xcb_generic_event_t *nvt,
                     PhxObject *obj) {

     /* Nexus does nothing on leave event */
  if (obj == NULL)  return false;

    /* Nexus' object needs calling, our default is do nothing. */
  if (!IS_IFACE_TYPE(obj)) {
    if (!sensitive_get(obj))
      return true;
      /* PhxObject's default of false, forces PhxNexus to handle. */
    if (obj->_event_cb != NULL)
      if (obj->_event_cb(iface, nvt, obj))
        return true;
      /* Object may have been a child of an object. */
    if (obj->o_mount != obj)
      if (obj->o_mount->_event_cb(iface, nvt, obj))
        return true;
    return false;
  }

    /* Nexus was sent an iface type. Should only be 'self'.
      Case of configure has handled, but internal objects may adjust. */
  if (obj->ncount != 0) {
    uint16_t odx = obj->ncount;
    xcb_configure_notify_event_t *configure;
    configure = (xcb_configure_notify_event_t*)nvt;
    if ((configure->response_type & (uint8_t)0x7F) != XCB_CONFIGURE_NOTIFY)
        /* on NFUSE, bare surface exposed, can receive events, they do nil */
      return false;

      /* For generic nexus default */
    if (obj->type == PHX_NEXUS)
      do {
        PhxObject *inspect = ((PhxNexus*)obj)->objects[(--odx)];
  /*      if ( (visible_get(inspect)) && (inspect->_event_cb != NULL) )*/
        if (inspect->_event_cb != NULL)
          inspect->_event_cb(iface, nvt, inspect);
      } while (odx != 0);
  }
  return false;
}

#pragma mark *** Creation ***

uint16_t
_default_nexus_remove_object(PhxNexus *nexus, PhxObject *obj) {

  uint16_t odx;
  if ((odx = nexus->ncount) == 0) {
    DEBUG_ASSERT(true, "error: no objects in nexus.");
    return (uint16_t)~0;
  }
  do {
    PhxObject *inspect = nexus->objects[(--odx)];
    if (inspect == obj) {
      if (obj->_raze_cb != NULL)  obj->_raze_cb((void*)obj);
      else                 _default_object_raze((void*)obj);
      if ((nexus->ncount - 1) != odx)
        memmove(&nexus->objects[odx], &nexus->objects[(odx + 1)],
                   (nexus->ncount - odx) * sizeof(PhxObject*));
      nexus->ncount -= 1;
      nexus->objects[nexus->ncount] = NULL;
      return nexus->ncount;
    }
  } while (odx != 0);
  DEBUG_ASSERT(true, "error: object not found for removal.");
  return (uint16_t)~0;
}

/* Add NULL assignments for debugging. */
void
_default_nexus_raze(void *in_obj) {

  PhxNexus *nexus = (PhxNexus*)in_obj;
    /* nexus->objects */
  if (nexus->ncount != 0) {
    uint16_t odx = nexus->ncount;
    do {
      PhxObject *obj = nexus->objects[(--odx)];
      if (IS_IFACE_TYPE(obj)) {
        _default_nexus_raze((void*)obj);
      } else {
          /* raze and free object */
        if (obj->_raze_cb != NULL)  obj->_raze_cb((void*)obj);
        else               _default_object_raze((void*)obj);
          /* update nexus */
        nexus->objects[odx] = NULL;
        nexus->ncount -= 1;
      }
    } while (odx != 0);
  }
  if (nexus->objects != NULL) {
    free(nexus->objects);
    nexus->objects = NULL;
  }

  free(nexus->attrib->font_name);
  free(nexus->attrib);
  nexus->attrib = NULL;               /* For debug */

    /* can be NULL from 'configure' */
  if (nexus->surface != NULL)  {
    cairo_surface_destroy(nexus->surface);
    nexus->surface = NULL;
  }

  DEBUG_ASSERT((nexus == (PhxNexus*)ui_active_drag_get()),
                     "failure: has_drag on nexus termination.");
  DEBUG_ASSERT((nexus == (PhxNexus*)ui_active_within_get()),
                     "failure: within on nexus termination.");
  DEBUG_ASSERT((nexus == (PhxNexus*)ui_active_focus_get()),
                     "failure: has_focus on nexus termination.");
  free(nexus);
}

/* Returns false if configure if fully within iface.
  false equivalent to 'not an issue', within bounds. */
static bool
_check_intersect(PhxInterface *iface, PhxRectangle configure) {

  PhxRectangle ibox = iface->mete_box;
  ibox.x = (ibox.y = 0);
  ibox.w += ibox.x, ibox.h += ibox.y;

  if ( (configure.x < ibox.x) || (configure.x > ibox.w)
      || (configure.y < ibox.y) || (configure.y > ibox.h) ) {
    return true;
  }
  if ( ((configure.x + configure.w) > ibox.w)
      || ((configure.y + configure.h) > ibox.h) ) {
    return true;
  }
  return false;
}

/* Nexus can not be created for objects >= PHX_NEXUS,
  exception: guts of a gfuse. */
PhxNexus *
ui_nexus_create(PhxInterface *iface, PhxRectangle configure) {

  PhxNexus *nexus;
  PhxAttr  *attrib;
  size_t   aSz;

  if (OBJECT_BASE_TYPE(iface) >= PHX_NEXUS) {
    DEBUG_ASSERT(true, "error: mismatch type ui_nexus_create()");
    return NULL;
  }
  if (_obj_alloc_test((void**)&iface->nexus, sizeof(PhxNexus*), iface->ncount))
    return NULL;

  DEBUG_ASSERT((_check_intersect(iface, configure)),
                      "WARNING: Nexus creation outside of Interface.");

  nexus = malloc(sizeof(PhxNexus));
  if (nexus == NULL)  return NULL;
  memset(nexus, 0, sizeof(PhxNexus));

  nexus->type = PHX_NEXUS;
    /* default: XPD static */
  nexus->state = 0;
    /* Assign values, placement in iface's coordinates */
  *((PhxRectangle*)&nexus->mete_box) = configure;
  *((PhxRectangle*)&nexus->draw_box) = configure;
  nexus->draw_box.x = 0;
  nexus->draw_box.y = 0;

  nexus->_draw_cb  = _demo_draw;
  nexus->_event_cb = _default_nexus_meter;
  nexus->_raze_cb  = _default_nexus_raze;

    /* Attribute defaults */
  attrib = (PhxAttr*)malloc(sizeof(PhxAttr));
  if (attrib == NULL) {
    free(nexus);
    return NULL;
  }
  memmove(attrib, iface->attrib, sizeof(PhxAttr));
  attrib->font_name = strdup(iface->attrib->font_name);
  nexus->attrib = attrib;
    /* object list, not combined alloc, this can realloc */
  aSz = OBJS_ALLOC * sizeof(PhxObject*);
  nexus->objects = malloc(aSz);
  if (nexus->objects == NULL) {
    free(attrib->font_name);
    free(attrib);
    free(nexus);
    return NULL;
  }
  memset(nexus->objects, 0, aSz);

    /* inherits iface's/nfuse's min_max */
  nexus->min_max.x = iface->min_max.x;
  nexus->min_max.y = iface->min_max.y;
  nexus->min_max.w = iface->min_max.w;
  nexus->min_max.h = iface->min_max.h;

    /* This is mapping. With a special nexus, headerbar, if adding
      it will place as topmost. If not adding a headerbar, we must check
      if one exists. We place new nexus under headerbar, the topmost
      before adding this one. We also have no idea if new nexus will
      be a headerbar. */
  if (iface->ncount == 0)  goto topmost;
  if (iface->nexus[(iface->ncount - 1)]->type == PHX_HEADERBAR) {
    PhxNexus *swap = iface->nexus[(iface->ncount - 1)];
    iface->nexus[(iface->ncount - 1)] = nexus;
    iface->nexus[iface->ncount] = swap;
  } else {
topmost:
    iface->nexus[iface->ncount] = nexus;
  }
  iface->ncount += 1;

    /* drawing ability */
  if (iface->surface != NULL) {
    cairo_status_t error;
    nexus->surface = cairo_surface_create_for_rectangle(
                                    iface->surface,
                                    nexus->mete_box.x,
                                    nexus->mete_box.y,
                                    nexus->mete_box.w,
                                    nexus->mete_box.h);
    error = cairo_surface_status(nexus->surface);
    DEBUG_ASSERT((error != CAIRO_STATUS_SUCCESS),
                      "error: _ui_nexus_create_for() surface creation");
    nexus->sur_width  = nexus->draw_box.w;
    nexus->sur_height = nexus->draw_box.h;
  }

  nexus->i_mount = iface;
  nexus->window  = iface->window;

  return nexus;
}

