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

/* Verified that input clip from expose is controled by window managers
  and that the clip may be whole window rather than invalid area. Compiz
  returned whole window on resize of height, were twm returned the newly
  exposed area of window.
*/
/*
  double x1, y1, x2, y2;
  cairo_clip_extents(cr, &x1, &y1, &x2, &y2);
  printf("%.2f %.2f %.2f %.2f\n", x1, y1, x2, y2);
*/
static bool
_intersection(PhxNexus *nexus, PhxRectangle *clip_box) {

  PhxRectangle mbox = nexus->mete_box;
  PhxInterface *mount = nexus->i_mount;
  if (!IS_WINDOW_TYPE(mount))
    do
      mbox.x += mount->mete_box.x,
      mbox.y += mount->mete_box.y;
    while (!IS_WINDOW_TYPE((mount = mount->i_mount)));

  if (mbox.x >= (clip_box->x + clip_box->w))  return false;
  if (mbox.y >= (clip_box->y + clip_box->h))  return false;
  if (mbox.x < clip_box->x)
    if (clip_box->x >= (mbox.x + mbox.w))     return false;
  if (mbox.y < clip_box->y)
    if (clip_box->y >= (mbox.y + mbox.h))     return false;

  *clip_box = mbox;
  return true;
}

/* For demodraw. */
static uint16_t
_nexus_count(PhxNexus *nexus) {
  if (nexus->type == PHX_GFUSE) {
    return 2 + ((PhxNFuse*)nexus)->nexus[0]->ncount;
  }
  if (nexus->type != PHX_NFUSE)
    return 1;
    /* Nfuse only set for 1 deep for now. */
  return (1 + nexus->ncount);
}

static void
_nexus_walk(PhxInterface *iface, cairo_t *face_cr, PhxRectangle clip_box) {

  uint16_t ndx = 0;
    /* walk all iface nexus, ascending since layered system */
  do {
    bool draw_it;
    cairo_t *cr;
    PhxRectangle cbox = clip_box;  /* Unsure if needed to speed up. */
    PhxNexus *nexus = iface->nexus[ndx];

      /* Must clear OBIT_SUR_TOUCH first! */
    draw_it = (!!(nexus->state & OBIT_SUR_TOUCH));
    nexus->state &= ~OBIT_SUR_TOUCH;

      /* Is nexus in visible draw area? */
    DEBUG_ASSERT((nexus->surface == NULL),
                               "error: NULL surface... _nexus_walk()");
      /* Seperated for demodraw. Immediate below should be joined. */
    if (!ui_visible_get((PhxObject*)nexus)) {
      continue;
    }
    if (!_intersection(nexus, &cbox)) {
      colour_select += _nexus_count(nexus);
      colour_select %= 12;
      continue;
    }

      /* Was it changed from last time it was drawn? */
    if (!draw_it) {
        /* Demodraw code. */
      colour_select += _nexus_count(nexus);
      colour_select %= 12;
        /* Paint untouched nexus->surface to iface->surface. */
      goto painted_surface;
    }
      /* Demodraw code.*/
    colour_select++, colour_select %= 12;

        /* Start drawing nexus and its contents. */
    cr = cairo_create(nexus->surface);
      /* each nexus can draw a background, or user's desire */
    if (nexus->_draw_cb != NULL)
      nexus->_draw_cb((PhxObject*)nexus, cr);
    else if (nexus->attrib->bg_fill.a != 0) {
      PhxRGBA *c = &nexus->attrib->bg_fill;
        /* Clear backgound surface. */
      cairo_save(cr);
      cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
      cairo_paint(cr);
      cairo_restore(cr);
        /* Clip to drawing area. */
      cairo_rectangle(cr, 0, 0, nexus->mete_box.w, nexus->mete_box.h);
      cairo_clip(cr);
        /* Paint viewable nexus user's desire or default. */
      cairo_set_source_rgba(cr, c->r, c->g, c->b, c->a);
      cairo_paint(cr);
    }

      /* Deal with 2 special nexus types. */
    if (nexus->type == PHX_GFUSE) {
        /* Bypass internal nfuse surface, use gfuse surface as face_cr.
          Keep 'nexus' as gfuse for surface drawn. */
      PhxNexus *subfuse;
      subfuse = ((PhxNFuse*)nexus)->nexus[0];
      cairo_translate(cr, subfuse->mete_box.x, subfuse->mete_box.y);
        /* Demodraw code.*/
      colour_select++, colour_select %= 12;
        /* Drawing start of subnexus and its contents. */
      if (subfuse->_draw_cb != NULL)
        subfuse->_draw_cb((PhxObject*)subfuse, cr);
      else if (subfuse->attrib->bg_fill.a != 0) {
        PhxRGBA *c = &subfuse->attrib->bg_fill;
        cairo_set_source_rgba(cr, c->r, c->g, c->b, c->a);
        cairo_paint(cr);
      }
      if (subfuse->ncount != 0)
        _nexus_walk((PhxInterface*)subfuse, cr, clip_box);
      goto drawn_surface;
    }
    if (nexus->type == PHX_NFUSE) {
        /* Note difference: nexus objects use cr as 'face_cr'. */
      if (nexus->ncount != 0)
        _nexus_walk((PhxInterface*)nexus, cr, clip_box);
      goto drawn_surface;
    }

      /* Normal nexus contents. */
    if (nexus->ncount != 0)
      _object_walk(nexus, cr);

drawn_surface:
    cairo_destroy(cr);
    cairo_surface_flush(nexus->surface);
painted_surface:
    cairo_set_source_surface(face_cr, nexus->surface,
                                      nexus->mete_box.x,
                                      nexus->mete_box.y);
    cairo_paint(face_cr);

  } while ((++ndx) < iface->ncount);
}

bool
_interface_draw(PhxInterface *iface, xcb_expose_event_t *expose) {

  PhxRectangle clip_box;
  cairo_t *cr;

  if (iface->surface == NULL) {
    DEBUG_ASSERT(true, "severe error, no iface surface");
    return false;
  }

    /* This is for testing/design setup. Draws something to allow visual. */
  if (pix[0] == NULL)  DrawDemoInitialize();

  cr = cairo_create(iface->surface);

  clip_box.x = expose->x;
  clip_box.y = expose->y;
  clip_box.w = expose->width;
  clip_box.h = expose->height;
  cairo_rectangle(cr, expose->x, expose->y,
                      expose->width, expose->height);
  cairo_clip(cr);

    /* Clear clipped area. */
  cairo_save(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint(cr);
  cairo_restore(cr);

    /* iface to always redaw self ('touch' by all) */
  if (iface->_draw_cb != NULL)
    iface->_draw_cb((PhxObject*)iface, cr);
  else if (iface->attrib->bg_fill.a != 0) {
    PhxRGBA *c = &iface->attrib->bg_fill;
    cairo_set_source_rgba(cr, c->r, c->g, c->b, c->a);
    cairo_paint(cr);
  }

  if (iface->ncount != 0)  _nexus_walk(iface, cr, clip_box);

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
    if (!(iface->state & SBIT_HBR_DRAG)) {
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
  }
  ui_timer_delete(tmr);
}

/* Expects input rectangle, x,y,w,h not x0,y0,x1,y1 */
void
ui_invalidate_rectangle(PhxInterface *iface, PhxRectangle dirty) {

  union rectangle_endianess rect_u;
  struct _sigtimer *tmr;

  while (!IS_WINDOW_TYPE(iface)) {
    iface->state |= OBIT_SUR_TOUCH;
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

  if (!IS_WINDOW_TYPE(mount)) {
    if (IS_IFACE_TYPE(mount))  mount->state |= OBIT_SUR_TOUCH;
    do {
      dirty_u.rect.x += mount->mete_box.x,
      dirty_u.rect.y += mount->mete_box.y;
      if (IS_WINDOW_TYPE((mount = mount->i_mount)))  break;
      mount->state |= OBIT_SUR_TOUCH;
    } while (1);
  }

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
