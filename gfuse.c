#include "gfuse.h"
#include "configure.h"

#define DND_DEBUG 0
#if DND_DEBUG
 #define DND_DEBUG_PUTS(a) puts((a))
 #define DND_DEBUG_PRINT(a,b) \
   printf("%-26s  target %d\n", (a), (b))
 #define DND_DEBUG_CURSOR(a) puts((a))
#else
 #define DND_DEBUG_PUTS(a)
 #define DND_DEBUG_PRINT(a,b)
 #define DND_DEBUG_CURSOR(a)
#endif

#pragma mark *** Drawing ***

static void
_draw_grab_grip(PhxObject *b, cairo_t *cr) {

  PhxNexus *nexus = (PhxNexus*)b;
  xcb_gravity_t gravity = (nexus->state >> GRAVITY_SHIFT) & 0x0F;

  PhxRectangle mbox = nexus->mete_box;

  PhxAttr *c = nexus->attrib;
  cairo_set_source_rgba(cr, 0.82, 0.82, 0.82, 1);
  cairo_set_line_width(cr, c->stroke);

  switch (gravity) {
    case XCB_GRAVITY_NORTH_WEST:
      if ( ((mbox.y + mbox.h) > 0) && ((mbox.x + mbox.w) > 0) ) {
        cairo_rectangle(cr, 0, 0, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_rectangle(cr, 0, 0, GRIPSZ, mbox.h);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0 + mbox.w, 0);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, mbox.h);
        cairo_stroke(cr);

        cairo_move_to(cr, GRIPSZ, 0 + GRIPSZ);
        cairo_line_to(cr, 0 + mbox.w, 0 + GRIPSZ);
        cairo_stroke(cr);
        cairo_move_to(cr, 0 + GRIPSZ, GRIPSZ);
        cairo_line_to(cr, 0 + GRIPSZ, 0 + mbox.h);
        cairo_stroke(cr);
          /* cairo uses intersect for clip */
        cairo_rectangle(cr, GRIPSZ, GRIPSZ,
                            mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_NORTH:
      if ( (mbox.y > -GRIPSZ) && ((mbox.x + mbox.w) > 0) ) {
        cairo_rectangle(cr, 0, 0, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0 + mbox.w, 0);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, 0 + GRIPSZ);
        cairo_line_to(cr, 0 + mbox.w, 0 + GRIPSZ);
        cairo_stroke(cr);
          /* cairo uses intersect for clip */
        cairo_rectangle(cr, 0, 0 + GRIPSZ, mbox.w, mbox.h);
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_NORTH_EAST:
      if ( ((mbox.y + mbox.h) > 0) && ((mbox.x + mbox.w) > 0) ) {
        cairo_rectangle(cr, 0, 0, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_rectangle(cr, mbox.w - GRIPSZ, 0, mbox.w, mbox.h);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, mbox.w, 0);
        cairo_stroke(cr);
        cairo_move_to(cr, mbox.w, 0);
        cairo_line_to(cr, mbox.w, mbox.h);
        cairo_stroke(cr);

        cairo_move_to(cr, 0, GRIPSZ);
        cairo_line_to(cr, mbox.w - GRIPSZ, GRIPSZ);
        cairo_stroke(cr);
        cairo_move_to(cr, mbox.w - GRIPSZ, GRIPSZ);
        cairo_line_to(cr, mbox.w - GRIPSZ, mbox.h);
        cairo_stroke(cr);

          /* cairo uses intersect for clip */
        cairo_rectangle(cr, 0, GRIPSZ, mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_WEST:
      if ( (mbox.x > -GRIPSZ) && ((mbox.y + mbox.h) > 0) ) {
        cairo_rectangle(cr, 0, 0, GRIPSZ, mbox.h);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, mbox.h);
        cairo_stroke(cr);
        cairo_move_to(cr, 0 + GRIPSZ, 0);
        cairo_line_to(cr, 0 + GRIPSZ, 0 + mbox.h);
        cairo_stroke(cr);
          /* cairo uses intersect for clip */
        cairo_rectangle(cr, GRIPSZ, 0, mbox.w - GRIPSZ, mbox.h);
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_CENTER:
      if ( ((mbox.y + mbox.h) > 0) && ((mbox.x + mbox.w) > 0) ) {
        cairo_rectangle(cr, 0, 0, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_rectangle(cr, 0, 0, GRIPSZ, mbox.h);
        cairo_fill(cr);
        cairo_rectangle(cr, 0, 0 + mbox.h - GRIPSZ, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_rectangle(cr, mbox.w - GRIPSZ, 0, mbox.w, mbox.h);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, mbox.w, 0);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, mbox.h);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, mbox.h);
        cairo_line_to(cr, mbox.w, mbox.h);
        cairo_stroke(cr);
        cairo_move_to(cr, mbox.w, 0);
        cairo_line_to(cr, mbox.w, mbox.h);
        cairo_stroke(cr);

        cairo_move_to(cr, GRIPSZ, GRIPSZ);
        cairo_line_to(cr, mbox.w - GRIPSZ, GRIPSZ);
        cairo_stroke(cr);
        cairo_move_to(cr, GRIPSZ, GRIPSZ);
        cairo_line_to(cr, GRIPSZ, mbox.h - GRIPSZ);
        cairo_stroke(cr);
        cairo_move_to(cr, GRIPSZ, mbox.h - GRIPSZ);
        cairo_line_to(cr, mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_stroke(cr);
        cairo_move_to(cr, mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_line_to(cr, mbox.w - GRIPSZ, GRIPSZ);
        cairo_stroke(cr);
          /* clip */
        cairo_rectangle(cr, GRIPSZ, GRIPSZ,
                            mbox.w - (GRIPSZ << 1),
                            mbox.h - (GRIPSZ << 1));
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_EAST:
      if ( (mbox.x > -GRIPSZ) && ((mbox.y + mbox.h) > 0) ) {
        cairo_rectangle(cr, mbox.w - GRIPSZ, 0, mbox.w, mbox.h);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, mbox.w - GRIPSZ, 0);
        cairo_line_to(cr, mbox.w - GRIPSZ, mbox.h);
        cairo_stroke(cr);
        cairo_move_to(cr, mbox.w, 0);
        cairo_line_to(cr, mbox.w, 0 + mbox.h);
        cairo_stroke(cr);
          /* cairo uses intersect for clip */
        cairo_rectangle(cr, 0, 0, mbox.w - GRIPSZ, mbox.h);
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_SOUTH_WEST:
      if ( ((mbox.y + mbox.h) > 0) && ((mbox.x + mbox.w) > 0) ) {
        cairo_rectangle(cr, 0, mbox.h - GRIPSZ, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_rectangle(cr, 0, 0, GRIPSZ, mbox.h);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, mbox.h);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, mbox.h);
        cairo_line_to(cr, mbox.w, mbox.h);
        cairo_stroke(cr);
        cairo_move_to(cr, GRIPSZ, 0);
        cairo_line_to(cr, GRIPSZ, mbox.h - GRIPSZ);
        cairo_stroke(cr);
        cairo_move_to(cr, GRIPSZ, mbox.h - GRIPSZ);
        cairo_line_to(cr, mbox.w, mbox.h - GRIPSZ);
        cairo_stroke(cr);
          /* cairo uses intersect for clip */
        cairo_rectangle(cr, GRIPSZ, 0, mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_SOUTH:
      if ( ((mbox.y + mbox.h) > 0) && ((mbox.x + mbox.w) > 0) ) {
        cairo_rectangle(cr, 0, 0 + mbox.h - GRIPSZ, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, 0 + mbox.h - GRIPSZ);
        cairo_line_to(cr, 0 + mbox.w, 0 + mbox.h - GRIPSZ);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, 0 + mbox.h);
        cairo_line_to(cr, 0 + mbox.w, 0 + mbox.h);
        cairo_stroke(cr);
          /* cairo uses intersect for clip */
        cairo_rectangle(cr, 0, 0, mbox.w, mbox.h - GRIPSZ);
        cairo_clip(cr);
      }
      break;
    case XCB_GRAVITY_SOUTH_EAST:
      if ( ((mbox.y + mbox.h) > 0) && ((mbox.x + mbox.w) > 0) ) {
        cairo_rectangle(cr, 0, 0 + mbox.h - GRIPSZ, mbox.w, GRIPSZ);
        cairo_fill(cr);
        cairo_rectangle(cr, mbox.w - GRIPSZ, 0, mbox.w, mbox.h);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, c->fg_ink.r,
                                  c->fg_ink.g,
                                  c->fg_ink.b,
                                  c->fg_ink.a);
        cairo_move_to(cr, 0, mbox.h);
        cairo_line_to(cr, mbox.w, mbox.h);
        cairo_stroke(cr);
        cairo_move_to(cr, mbox.w, 0);
        cairo_line_to(cr, mbox.w, mbox.h);
        cairo_stroke(cr);

        cairo_move_to(cr, 0, mbox.h - GRIPSZ);
        cairo_line_to(cr, mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_stroke(cr);

        cairo_move_to(cr, mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_line_to(cr, mbox.w - GRIPSZ, 0);
        cairo_stroke(cr);

          /* cairo uses intersect for clip */
        cairo_rectangle(cr, 0, 0, mbox.w - GRIPSZ, mbox.h - GRIPSZ);
        cairo_clip(cr);
      }
      break;
    default:
      break;
  }
}

#pragma mark *** Cursor ***

struct _cursor_id {
  const char *gnames[5];
  uint8_t id;
};
static struct _cursor_id phx_cursor_id = {
 {  "fleur",               /*   5  */
    "sb_h_double_arrow",   /* 4, 6 */
    "size_bdiag",          /* 3, 7 */
    "sb_v_double_arrow",   /* 2, 8 */
    "size_fdiag"           /* 1, 9 */
 },0
};

static const char *
_cursor_name_by_gravity(xcb_gravity_t gravity) {
    /* center = 5 */
  uint16_t key = absof((int)gravity - 5);
  if (key > (uint16_t)4)  return NULL;
  return phx_cursor_id.gnames[key];
}

/* must enter with window coordindinates */
static xcb_gravity_t
_cursor_gravity_at(PhxNFuse *fuse, int16_t x, int16_t y) {

  PhxRectangle r;
  bool has_north, has_east, has_south, has_west;
  uint16_t gravity;

  if (fuse->type != PHX_GFUSE)
    return XCB_NONE;

  gravity = (fuse->state >> GRAVITY_SHIFT) & 0x0F;
  r = fuse->mete_box;
  if ( (y < r.y) || (y >= (r.y + r.h))
      || (x < r.x) || (x >= (r.x + r.w)) )
    return XCB_NONE;

  has_north = false, has_east = false,
  has_south = false, has_west = false;
  if (y < (r.y + GRIPSZ))
    has_north = true;
  if (y >= (r.y + r.h - GRIPSZ))
    has_south = true;

  if (x < (r.x + GRIPSZ))
    has_west = true;
  if (x >= (r.x + r.w - GRIPSZ))
    has_east = true;

  if (gravity == XCB_GRAVITY_NORTH)
    return ((has_north) ? XCB_GRAVITY_NORTH : XCB_NONE);
  if (gravity == XCB_GRAVITY_SOUTH)
    return ((has_south) ? XCB_GRAVITY_SOUTH : XCB_NONE);
  if (gravity == XCB_GRAVITY_EAST)
    return ((has_east) ? XCB_GRAVITY_EAST : XCB_NONE);
  if (gravity == XCB_GRAVITY_WEST)
    return ((has_west) ? XCB_GRAVITY_WEST : XCB_NONE);

  if (gravity == XCB_GRAVITY_NORTH_EAST)
    return ((has_north) && (has_east))
            ? XCB_GRAVITY_NORTH_EAST : (has_north)
            ? XCB_GRAVITY_NORTH : (has_east)
            ? XCB_GRAVITY_EAST : XCB_NONE;
  if (gravity == XCB_GRAVITY_NORTH_WEST)
    return ((has_north) && (has_west))
            ? XCB_GRAVITY_NORTH_WEST : (has_north)
            ? XCB_GRAVITY_NORTH : (has_west)
            ? XCB_GRAVITY_WEST : XCB_NONE;
  if (gravity == XCB_GRAVITY_SOUTH_EAST)
    return ((has_south) && (has_east))
            ? XCB_GRAVITY_SOUTH_EAST : (has_south)
            ? XCB_GRAVITY_SOUTH : (has_east)
            ? XCB_GRAVITY_EAST : XCB_NONE;
  if (gravity == XCB_GRAVITY_SOUTH_WEST)
    return ((has_south) && (has_west))
            ? XCB_GRAVITY_SOUTH_WEST : (has_south)
            ? XCB_GRAVITY_SOUTH : (has_west)
            ? XCB_GRAVITY_WEST : XCB_NONE;

  if (gravity == XCB_GRAVITY_CENTER)
    return ((has_south) && (has_west))
            ? XCB_GRAVITY_SOUTH_WEST : ((has_north) && (has_east))
            ? XCB_GRAVITY_NORTH_EAST : ((has_north) && (has_west))
            ? XCB_GRAVITY_NORTH_WEST : ((has_south) && (has_east))
            ? XCB_GRAVITY_SOUTH_EAST : (has_north)
            ? XCB_GRAVITY_NORTH : (has_east)
            ? XCB_GRAVITY_EAST : (has_south)
            ? XCB_GRAVITY_SOUTH : (has_west)
            ? XCB_GRAVITY_WEST : XCB_NONE;

  return XCB_NONE;
}

/* mount coordinates in event_x, event_y */
static xcb_gravity_t
_set_cursor_from_gravity(PhxInterface *iface,
                         PhxNFuse *fuse, int16_t x, int16_t y) {

  xcb_gravity_t gravity;
  const char *gravity_cursor;

  if ( (fuse == NULL) || (fuse->type != PHX_GFUSE) ) {
    ui_cursor_set_named(NULL, iface->window);
    return XCB_NONE;
  }

  gravity = _cursor_gravity_at(fuse, x, y);
  gravity_cursor = _cursor_name_by_gravity(gravity);
  ui_cursor_set_named(gravity_cursor, iface->window);
  return gravity;
}

#pragma mark *** Events ***

static bool
_drag_motion_gfuse(PhxInterface *iface,
                   xcb_generic_event_t *nvt,
                   PhxObject *obj) {

  int16_t x, y;
  xcb_motion_notify_event_t *motion
    = (xcb_motion_notify_event_t*)nvt;
  bool bp = ((motion->state & XCB_BUTTON_MASK_1) != 0);
  x = motion->event_x, y = motion->event_y;

    /* Either start drag and/or set cursor */
  if (ui_active_drag_get() == NULL) {

    xcb_gravity_t gravity;

      /* Cursor was set by enter notice. */
    DEBUG_ASSERT((ui_cursor_get_named() == NULL),
                         "Cursor not set from entered or missing diagonals.");
      /* Since drag is NULL, 'obj' will be gfuse.
         Also must have gravity since entered this cb. */
    gravity = _set_cursor_from_gravity(iface, (PhxNFuse*)obj, x, y);
      /* Claim drag */
    if (bp) {
      ui_active_drag_set(obj);
      phx_cursor_id.id = gravity;
    }
  }
  if (bp) {
    int16_t xD, yD;
    uint8_t pos_gravity, cursor_id;
    bool is_all, braked;

    PhxNFuse *fuse = (PhxNFuse*)ui_active_drag_get();
    PhxRectangle rD = fuse->mete_box;
    int16_t mouse_x = session->last_event_x,
            mouse_y = session->last_event_y;

      /* Test if was inside grip region prior to current pointer. */
    if ( (mouse_x < rD.x) || (mouse_x >= (rD.x + rD.w)) )
      return true;
    if ( (mouse_y < rD.y) || (mouse_y >= (rD.y + rD.h)) )
      return true;

      /* Test if in currect gravity region. */
    pos_gravity = _cursor_gravity_at(fuse, x, y);
    cursor_id = phx_cursor_id.id;
    if ( (cursor_id != pos_gravity) && ((cursor_id & 1) == 0) )
      if (pos_gravity != 0)
        return true;

    braked = false;
    is_all = ((fuse->state & GRAVITY_MASK) == GRAVITY_CENTER);

    yD = y - mouse_y;
    if (yD != 0) {
      if ( (cursor_id == 4) || (cursor_id == 6) ) {
          /* 4 west, 6 east (==4==8)*/
        yD = 0;
      } else {

        int16_t min, ylmtr, rh;

          /* When cursor outside, below grip and returning, wait until
             .5 grip to begin drag again. */
        if ( (yD < 0) && (mouse_y > (rD.y + rD.h - (GRIPSZ/2))) )
          return true;

               /* Directional Breakdown */

        if (cursor_id < 4) {
            /* north component */
          if ( (mouse_y < rD.y) || (mouse_y >= (rD.y + GRIPSZ)) )
            return true;
            /* When cursor outside, above window and returning, wait until
               .5 grip to begin drag again. */
          if (yD > 0) {
                 if (mouse_y < (rD.y + (GRIPSZ/2)))  return true;
          } else if (mouse_y > (rD.y + (GRIPSZ/2)))  return true;

          min = (!is_all) ? 0 : GRIPSZ;
          if ((fuse->state & VXPD_MSK) == VXPD_TOP)
            ylmtr = minof(iface->mete_box.h - GRIPSZ,
                          fuse->min_max.h - rD.h);
          else
            ylmtr = minof(iface->mete_box.h, fuse->min_max.h)
                   - GRIPSZ - min;
          if ((rD.y + yD) >= ylmtr)
            yD = ylmtr - rD.y;
          ylmtr = maxof(0, fuse->min_max.y);
          if ((rD.y + yD) <= ylmtr)
            yD = ylmtr - rD.y;
          rh = rD.h - min;
          if ( (yD > 0) && ((rh - yD) < GRIPSZ) ) {
            braked = true;
            rD.y += yD - (rh - GRIPSZ);
            yD = rh - GRIPSZ;
          }
        } else {
            /* south component */
          if ( (mouse_y >= (rD.y + rD.h))
              || (mouse_y < ((rD.y + rD.h) - GRIPSZ)) )
            return true;
            /* When cursor outside, above south grip and returning, wait
               until .5 grip to begin drag again. */
          if ( (yD > 0) && (mouse_y < (rD.y + rD.h - (GRIPSZ/2))) )
            return true;

          ylmtr = minof(iface->mete_box.h, fuse->min_max.h);
          if ((rD.y + rD.h + yD) >= ylmtr)
            yD = ylmtr - (rD.y + rD.h);
          ylmtr = maxof(0, fuse->min_max.y);
          min = ( (!is_all) || ((fuse->state & VXPD_MSK) == VXPD_TOP) )
               ? 0 : GRIPSZ;
          rh = rD.h - min;
          if ((rD.y + rh - GRIPSZ + yD) <= ylmtr)
            yD = ylmtr - (rD.y + rh - GRIPSZ);
          if ( (yD < 0) && ((rh + yD) < GRIPSZ)
              && ((rD.y + yD) >= ylmtr) ) {
            braked = true;
            rD.y += rh - GRIPSZ + yD;
            yD = GRIPSZ - rh;
          }
        }
      } /* end (cursor_id == 4) || (cursor_id == 6) */
    } /* end (yD != 0) */

    xD = x - mouse_x;
    if (xD != 0) {
      if ( (cursor_id == 2) || (cursor_id == 8) ) {
          /* 2 north, 8 south  (<3) */
        xD = 0;
      } else {

        int16_t min, xlmtr, rw;

          /* When cursor outside, right of east grip and returning, wait
             until .5 grip to begin drag again. */
        if ( (xD < 0) && (mouse_x > (rD.x + rD.w - (GRIPSZ/2))) )
          return true;

               /* Directional Breakdown */

        if ((cursor_id % 3) != 0) {
            /* ! ne, e, se. (west based) */
          if ( (mouse_x < rD.x) || (mouse_x > (rD.x + GRIPSZ)) )
            return true;
            /* When cursor outside, left of window and returning, wait until
               .5 grip to begin drag again. */
          if (xD > 0) {
                 if (mouse_x < (rD.x + (GRIPSZ/2)))  return true;
          } else if (mouse_x > (rD.x + (GRIPSZ/2)))  return true;

          min = ( (!is_all) || ((fuse->state & HXPD_MSK) == HXPD_LFT) )
               ? 0 : GRIPSZ;
          if ((fuse->state & HXPD_MSK) == HXPD_LFT)
            xlmtr = minof(iface->mete_box.w - GRIPSZ - min,
                          fuse->min_max.w - rD.w);
          else
            xlmtr = minof(iface->mete_box.w,
                          fuse->min_max.w) - GRIPSZ - min;
          if ((rD.x + xD) >= xlmtr)
            xD = xlmtr - rD.x;
          xlmtr = maxof(0, fuse->min_max.x);
          if ((rD.x + xD) <= xlmtr)
            xD = xlmtr - rD.x;
          rw = rD.w - min;
          if ( (xD > 0) && ((rw - xD) < GRIPSZ) ) {
            braked = true;
            rD.x += xD - (rw - GRIPSZ);
            xD = rw - GRIPSZ;
          }
        } else {
            /* 3 6 9 east */
          if ( (mouse_x >= (rD.x + rD.w))
              || (mouse_x < ((rD.x + rD.w) - GRIPSZ)) )
            return true;
            /* When cursor outside, left of east grip and returning, wait
               until .5 grip to begin drag again. */
          if ( (xD > 0) && (mouse_x < (rD.x + rD.w - (GRIPSZ/2))) )
            return true;

          xlmtr = minof(iface->mete_box.w, fuse->min_max.w);
          if ((rD.x + rD.w + xD) >= xlmtr)
            xD = xlmtr - (rD.x + rD.w);
          xlmtr = maxof(0, fuse->min_max.x);
          min = ( (!is_all) || ((fuse->state & HXPD_MSK) == HXPD_LFT) )
               ? 0 : GRIPSZ;
          rw = rD.w - min;
          if ((rD.x + rw - GRIPSZ + xD) <= xlmtr)
            xD = xlmtr - (rD.x + rw - GRIPSZ);
          if ( (xD < 0) && ((rw + xD) < GRIPSZ)
              && ((rD.x + xD) >= xlmtr) ) {
            braked = true;
            rD.x += rw - GRIPSZ + xD;
            xD = GRIPSZ - rw;
          }
        }

      } /* end (cursor_id == 2) || (cursor_id == 8) */
    } /* end (xD != 0) */

   if ( (yD != 0) || (xD != 0) || braked ) {
      if ((fuse->state & HXPD_MSK) != HXPD_LFT) {
        if ((cursor_id % 3) == 0)
          rD.w += xD;
        else
          rD.x += xD, rD.w -= xD;
      } else {
        rD.x += xD;
      }
      if ((fuse->state & VXPD_MSK) != VXPD_TOP) {
        if (cursor_id >= 4)
          rD.h += yD;
        else
          rD.y += yD, rD.h -= yD;
      } else {
        rD.y += yD;
      }
      ui_nexus_resize((PhxNexus*)fuse, &rD);
    }
  }
  return true;
}

bool
_default_gfuse_meter(PhxInterface *iface,
                     xcb_generic_event_t *nvt,
                     PhxObject *obj) {

  switch (nvt->response_type & (uint8_t)0x7F) {

    case XCB_BUTTON_RELEASE: {   /* response_type 5 */
      PhxObject *has_drag = ui_active_drag_get();
      if (has_drag != NULL) {
        if (has_drag != obj) {
          xcb_button_press_event_t *mouse
            = (xcb_button_press_event_t*)nvt;
          ui_active_within_set(obj, mouse->state);
          DND_DEBUG_PUTS("ui_active_within_set(obj) _default_gfuse_meter()");
        }
        DND_DEBUG_PUTS("ui_active_drag_set(NULL) _default_gfuse_meter()");
        ui_active_drag_set(NULL);
        return true;
      }
      break;
    }

    case XCB_MOTION_NOTIFY: {    /* response_type 6 */
      xcb_motion_notify_event_t *motion
        = (xcb_motion_notify_event_t*)nvt;
      if (obj == NULL) {
          /* drag went outside window, shutdown drag. */
        iface->state |= SBIT_RELEASE_IGNORE;
        DND_DEBUG_CURSOR("ui_cursor_set_named(NULL) _default_gfuse_meter()");
        ui_cursor_set_named(NULL, iface->window);
        DND_DEBUG_PUTS("ui_active_drag_set(NULL) _default_gfuse_meter()");
        ui_active_drag_set(NULL);
        return true;
      }
      if ((motion->state & XCB_BUTTON_MASK_1) != 0)
        return _drag_motion_gfuse(iface, nvt, obj);
        /* Motion to another point checks for correct cursor. */
      _set_cursor_from_gravity(iface, (PhxNFuse*)obj,
                               motion->event_x, motion->event_y);
      break;
    }

    case XCB_ENTER_NOTIFY: {     /* response_type 7 */
      if (ui_active_drag_get() == NULL) {
        xcb_enter_notify_event_t *enter
         = (xcb_enter_notify_event_t*)nvt;
        _set_cursor_from_gravity(iface, (PhxNFuse*)obj,
                                 enter->event_x, enter->event_y);
      } else {
          /* do not respond to dnd, we can receive no drops. */
        return true;
      }
      break;
    }

    case XCB_LEAVE_NOTIFY: {     /* response_type 8 */
        /* only bother with cursor if have control of it */
      DND_DEBUG_CURSOR("ui_cursor_set_named(NULL) _default_gfuse_meter()");
      ui_cursor_set_named(NULL, iface->window);
      break;
    }

    default:
      return false;
      break;
  }
  return true;
}

#pragma mark *** Creation ***

/* Add a rectanglar grip to a nexus positioned and defined by gravity.
   Reduces nexus' drawing size by GRIPSZ. */
bool
_gfuse_add_grip(PhxNexus *nexus, xcb_gravity_t gravity) {

  PhxRectangle *dbox;

  if (nexus->type != PHX_GFUSE)
    return false;

  dbox = &nexus->draw_box;
  nexus->state &= ~GRAVITY_MASK;

  switch (gravity) {
    case XCB_GRAVITY_NORTH_WEST:
      nexus->state |= GRAVITY_NORTH_WEST;
      dbox->x += GRIPSZ, dbox->y += GRIPSZ,
      dbox->w -= GRIPSZ, dbox->h -= GRIPSZ;
      break;
    case XCB_GRAVITY_NORTH:
      nexus->state |= GRAVITY_NORTH;
      dbox->y += GRIPSZ, dbox->h -= GRIPSZ;
      break;
    case XCB_GRAVITY_NORTH_EAST:
      nexus->state |= GRAVITY_NORTH_EAST;
      dbox->y += GRIPSZ, dbox->w -= GRIPSZ, dbox->h -= GRIPSZ;
      break;
    case XCB_GRAVITY_WEST:
      nexus->state |= GRAVITY_WEST;
      dbox->x += GRIPSZ, dbox->w -= GRIPSZ;
      break;
    case XCB_GRAVITY_CENTER:
      nexus->state |= GRAVITY_CENTER;
      dbox->x += GRIPSZ, dbox->y += GRIPSZ,
      dbox->w -= (GRIPSZ << 1), dbox->h -= (GRIPSZ << 1);
      break;
    case XCB_GRAVITY_EAST:
      nexus->state |= GRAVITY_EAST;
      dbox->w -= GRIPSZ;
      break;
    case XCB_GRAVITY_SOUTH_WEST:
      nexus->state |= GRAVITY_SOUTH_WEST;
      dbox->x += GRIPSZ, dbox->w -= GRIPSZ, dbox->h -= GRIPSZ;
      break;
    case XCB_GRAVITY_SOUTH:
      nexus->state |= GRAVITY_SOUTH;
      dbox->h -= GRIPSZ;
      break;
    case XCB_GRAVITY_SOUTH_EAST:
      nexus->state |= GRAVITY_SOUTH_EAST;
      dbox->w -= GRIPSZ, dbox->h -= GRIPSZ;
      break;
    default:
      break;
  }
  return true;
}

PhxNFuse *
ui_gfuse_create(PhxInterface *iface, PhxRectangle configure,
                                             xcb_gravity_t gravity) {
  PhxNexus *gfuse, *nfuse;

    /* attaches to active_iface */
  gfuse = ui_nexus_create(iface, configure);
  if (gfuse == NULL)  return NULL;
    /* redefine its purpose */
  gfuse->type = PHX_GFUSE;
  _gfuse_add_grip(gfuse, gravity);

  gfuse->_draw_cb = _draw_grab_grip;
  gfuse->_event_cb = _default_gfuse_meter;

  configure = gfuse->draw_box;
  nfuse = ui_nexus_create((PhxInterface*)gfuse, configure);
  if (nfuse == NULL)  return NULL;
    /* redefine its purpose */
  nfuse->type = PHX_NFUSE;

    /* Default for gfuse. Behave like a window. */
  gfuse->state |= HXPD_RGT | VXPD_BTM;
    /* Default for nfuse for gfuse parent. Constant x,y, movable w,h.
      Must match gfuse! */
  nfuse->state |= HXPD_RGT | VXPD_BTM;

  return (PhxNFuse*)gfuse;
}
