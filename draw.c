#include "draw.h"
#include "windows.h"
#include "objects.h"
#include "gfuse.h"

#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif

PhxRGBA RGBA_DEFAULT_BG = {
  (double)0x0F0/0x0FF, (double)0x0F0/0x0FF, (double)0x0F0/0x0FF, 1.0 };
PhxRGBA RGBA_SELECTION = {
  (double)0x091/0x0FF, (double)0x0C2/0x0FF, (double)0x0F2/0x0FF, 1.0 };
PhxRGBA RGBA_TEXT_FGFILL = {
  (double)0x0F7/0x0FF, (double)0x0F7/0x0FF, (double)0x0F7/0x0FF, 1.0 };
PhxRGBA RGBA_SEARCH_FGFILL = {
  (double)0x0FB/0x0FF, (double)0x0E7/0x0FF, (double)0x07C/0x0FF, 1.0 };

#include "draw_demo.c"

#pragma mark *** Draw ***

static void
_object_walk(PhxNexus *nexus, cairo_t *cr) {

  uint16_t odx = 0;
  do {
    PhxObject *obj = nexus->objects[odx];
      /* initial object must be visible to draw it and
         any visible children */
    if (ui_visible_get(obj)) {
      cairo_save(cr);
      if (obj->_draw_cb != NULL)  obj->_draw_cb(obj, cr);
      if (obj->child != NULL) {
          /* walk through any children */
        PhxObject *add = obj->child;
        do {
            /* user may adjust which children at any given time should draw */
          if (ui_visible_get(add))
            if (add->_draw_cb != NULL)  add->_draw_cb(add, cr);
          add = add->child;
        } while (add != NULL);
      }
      cairo_restore(cr);
    }
  } while ((++odx) < nexus->ncount);
}

static void
_nexus_walk(PhxInterface *iface) {

  uint16_t ndx = 0;
    /* walk all iface nexus, ascending since layered system */
  do {
    PhxNexus *nexus = iface->nexus[ndx];
    if (!ui_visible_get((PhxObject*)nexus)) continue;
    if (nexus->surface != NULL) {
      PhxInterface *mount;
      cairo_t *cr;

      colour_select++, colour_select %= 12;
      cr = cairo_create(nexus->surface);

                   /* clipping code for surfaces */
      mount = nexus->i_mount;
      if (!IS_WINDOW_TYPE(mount)) {
        int16_t wD, hD,
                xD = nexus->mete_box.x,
                yD = nexus->mete_box.y;
        if (nexus->type == PHX_NEXUS) {
          xD += (mount->mete_box.x < 0) ? mount->mete_box.x : 0;
          yD += (mount->mete_box.y < 0) ? mount->mete_box.y : 0;
        }
          /* wD, hD aren't concerned with iface extents */
          /* Since a mount, w, h considered points from origin 0, 0 */
        wD = minof(mount->mete_box.w, nexus->mete_box.x + nexus->mete_box.w);
        hD = minof(mount->mete_box.h, nexus->mete_box.y + nexus->mete_box.h);
        cairo_rectangle(cr, -xD, -yD, wD, hD);
        cairo_clip(cr);
      }
                      /* end clipping code */

        /* each nexus can draw a background, or user's desire */
      if (nexus->_draw_cb != NULL)
        nexus->_draw_cb((PhxObject*)nexus, cr);
      else if (nexus->attrib->bg_fill.a != 0) {
        PhxRGBA *c = &nexus->attrib->bg_fill;
        cairo_set_source_rgba(cr, c->r, c->g, c->b, c->a);
        cairo_paint(cr);
      }
        /* check if nexus fusion, has surface, check children */
      if (nexus->ncount != 0) {
        DEBUG_ASSERT((!IS_IFACE_TYPE(nexus)), "logic error _nexus_walk()");
        if (OBJECT_BASE_TYPE(nexus) >= PHX_NEXUS)
          _object_walk(nexus, cr);
        else
          _nexus_walk((PhxInterface*)nexus);
      }

      cairo_destroy(cr);
      cairo_surface_flush(nexus->surface);
    }
  } while ((++ndx) < iface->ncount);
}

bool
_interface_draw(PhxInterface *iface) {

  cairo_t *cr;

  if (iface->surface == NULL) {
    DEBUG_ASSERT(true, "severe error, no iface surface");
    return false;
  }

    /* This is for testing/design setup. Draws something to allow visual. */
  if (pix[0] == NULL)  DrawDemoInitialize();

  cr = cairo_create(iface->surface);

    /* iface to always redaw self */
  if (iface->_draw_cb != NULL)
    iface->_draw_cb((PhxObject*)iface, cr);
  else if (iface->attrib->bg_fill.a != 0) {
    PhxRGBA *c = &iface->attrib->bg_fill;
    cairo_set_source_rgba(cr, c->r, c->g, c->b, c->a);
    cairo_paint(cr);
  }

  if (iface->ncount != 0)  _nexus_walk(iface);

  cairo_destroy(cr);
  cairo_surface_flush(iface->surface);
  colour_select = 0;
  return true;
}

#pragma mark *** Redraw ***

static void
_event_invalidate(struct _sigtimer *tmr) {

  xcb_expose_event_t *expose;
  union rectangle_endianess *rect_u;
  PhxInterface *iface;

  if (strcmp(tmr->id, "frame") != 0)  return;

    /* Because of server lag, verify iface wasn't deleted. */
  iface = (ui_interface_for(tmr->iface->window));
  if (iface != NULL) {
      /* Could send direct to expose, but then need an interupt
        signal for event loop to wake on rare instances of
        direct expose occurs when server is idle. During idle
        xcb_aux_sync() does not wake server, leaving direct drawn
        not being displayed until some unrelated event occurs. */
    expose = calloc(32, 1);
    expose->response_type = XCB_EXPOSE;
    expose->window = tmr->iface->window;
    rect_u = (union rectangle_endianess*)&tmr->data;
    rect_u->rect.w -= rect_u->rect.x,
    rect_u->rect.h -= rect_u->rect.y;
    expose->x = rect_u->rect.x;
    expose->y = rect_u->rect.y;
    expose->width = rect_u->rect.w;
    expose->height = rect_u->rect.h;
    xcb_send_event(session->connection, false, tmr->iface->window,
                           XCB_EVENT_MASK_EXPOSURE, (char *)expose);
    xcb_flush(session->connection);
  }
  ui_timer_delete(tmr);
}

/* Expects input rectangle, x,y,w,h not x0,y0,x1,y1 */
void
ui_invalidate_rectangle(PhxInterface *iface, PhxRectangle dirty) {

  union rectangle_endianess rect_u;
  struct _sigtimer *tmr;

  while (!IS_WINDOW_TYPE(iface)) {
    dirty.x += iface->mete_box.x;
    dirty.y += iface->mete_box.y;
    iface = iface->i_mount;
  }
  dirty.w += dirty.x, dirty.h += dirty.y;
  if ((dirty.w <= 0) || (dirty.h <= 0))  return;
  if (dirty.x < 0)  dirty.x = 0;
  if (dirty.y < 0)  dirty.y = 0;
  rect_u.rect = dirty;

  tmr = ui_timer_get(iface, "frame");
  if (tmr == NULL) {
    struct timespec ts = { 0, 16000000 };
    tmr = ui_timer_create(iface, "frame", &ts, _event_invalidate);
    DEBUG_ASSERT((tmr == NULL), "error: NULL timer ui_invalidate_rectangle()");
    if (tmr != NULL)  tmr->data = rect_u.r64;
  } else {
    union rectangle_endianess *inv_u;
    inv_u = (union rectangle_endianess*)&tmr->data;
    inv_u->rect.x = minof(inv_u->rect.x, rect_u.rect.x);
    inv_u->rect.y = minof(inv_u->rect.y, rect_u.rect.y);
    inv_u->rect.w = maxof(inv_u->rect.w, rect_u.rect.w);
    inv_u->rect.h = maxof(inv_u->rect.h, rect_u.rect.h);
  }
}

void
ui_invalidate_object(PhxObject *obj) {

  union rectangle_endianess dirty_u = { { 0, 0, 0, 0 } };
  PhxInterface *mount = (PhxInterface*)obj;
  struct _sigtimer *tmr;

  if (!IS_WINDOW_TYPE(mount))
    do
      dirty_u.rect.x += mount->mete_box.x,
      dirty_u.rect.y += mount->mete_box.y;
    while (!IS_WINDOW_TYPE((mount = mount->i_mount)));

  dirty_u.rect.w = dirty_u.rect.x + obj->mete_box.w;
  if (dirty_u.rect.w <= 0)  return;
  dirty_u.rect.h = dirty_u.rect.y + obj->mete_box.h;
  if (dirty_u.rect.h <= 0)  return;

  if (dirty_u.rect.x < 0)  dirty_u.rect.x = 0;
  if (dirty_u.rect.y < 0)  dirty_u.rect.y = 0;

  tmr = ui_timer_get(mount, "frame");
  if (tmr == NULL) {
    struct timespec ts = { 0, 16000000 }; /* 60 fps */
    tmr = ui_timer_create(mount, "frame", &ts, _event_invalidate);
    DEBUG_ASSERT((tmr == NULL), "error: NULL timer ui_invalidate_object()");
    if (tmr != NULL)  tmr->data = dirty_u.r64;
  } else {
    union rectangle_endianess *inv_u;
    inv_u = (union rectangle_endianess*)&tmr->data;
    inv_u->rect.x = minof(inv_u->rect.x, dirty_u.rect.x);
    inv_u->rect.y = minof(inv_u->rect.y, dirty_u.rect.y);
    inv_u->rect.w = maxof(inv_u->rect.w, dirty_u.rect.w);
    inv_u->rect.h = maxof(inv_u->rect.h, dirty_u.rect.h);
  }
}

#pragma mark *** Surface ***

cairo_surface_t *
ui_surface_create_similar(PhxInterface *iface, int16_t w, int16_t h) {

  while (!IS_WINDOW_TYPE(iface))
    iface = iface->i_mount;
  return cairo_surface_create_similar(iface->vid_buffer,
                                      CAIRO_CONTENT_COLOR_ALPHA, w, h);
}
