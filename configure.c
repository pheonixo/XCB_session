#include "configure.h"
#include "gfuse.h"

#pragma mark *** Image ***

static bool
_cImage_initialize(Image_s *cImage) {

  cImage->icount = 0;
  cImage->idata = malloc(OBJS_ALLOC * sizeof(IData_s));
  if (cImage->idata == NULL) {
    DEBUG_ASSERT(true, "failure: malloc _interface_configure()");
    return true;
  }
  memset(cImage->idata, 0, (OBJS_ALLOC * sizeof(IData_s)));
  return false;
}

static bool  _mark_all(PhxInterface *iface, Image_s *image);

/* returns true if error */
static bool
_image_add(PhxNexus *nexus, Image_s *image) {

  IData_s *idata;
  uint16_t icount;
  bool err;

  icount = image->icount;
  err = _obj_alloc_test((void**)&image->idata, sizeof(IData_s), icount);
  if (err)  return err;
  idata = &image->idata[icount];
  image->icount += 1;
  idata->nexus = nexus;
  idata->mete_box = nexus->mete_box;
  idata->draw_box = nexus->draw_box;

    /* A fusion? mark all within. */
  if (OBJECT_BASE_TYPE(nexus) == PHX_NFUSE)
    if (_mark_all((PhxInterface*)nexus, image))  return true;
  return false;
}

/* Marks and records state before any changes/destruction of surfaces.
   Destruction delayed, configure delayed */
static bool
_mark_all(PhxInterface *iface, Image_s *image) {

  uint16_t ndx;
  for (ndx = 0; ndx < iface->ncount; ndx++)
    if (_image_add(iface->nexus[ndx], image))  return true;
  return false;
}

static bool
_mark_nexus(PhxNexus *nexus, Image_s *image) {

  uint16_t icount;
    /* don't add one that's in image */
  for (icount = image->icount; icount != 0;) {
    PhxNexus *inspect = (image->idata[(--icount)]).nexus;
    if (inspect == nexus)  return false;
  }
  return (_image_add(nexus, image));
}

#pragma mark *** Sweeps ***

static int16_t
_vcheck(PhxNexus *inspect, int16_t offset) {

  int16_t iy = inspect->mete_box.y;
  if ((inspect->state & VXPD_TOP) != 0) {
    if ((iy + offset) < inspect->min_max.y)
      offset = inspect->min_max.y - iy;
  }

  iy += inspect->mete_box.h;
  if ((iy + offset) > inspect->min_max.h)
    offset = inspect->min_max.h - iy;

  return offset;
}

static int16_t
_rgt_hcheck(PhxNexus *inspect, int16_t offset) {

  int16_t ix = inspect->mete_box.x;
    /* Does X move? */
  if ((inspect->state & HXPD_LFT) != 0) {
    if ((ix + offset) < inspect->min_max.x)
      offset = inspect->min_max.x - ix;
  }

  ix += inspect->mete_box.w;
  if ((ix + offset) > inspect->min_max.w)
    offset = inspect->min_max.w - ix;

  return offset;
}


static int16_t
_lft_hcheck(PhxNexus *inspect, int16_t offset) {

  int16_t ix = inspect->mete_box.x;

  if ((ix + offset) < inspect->min_max.x)
    offset = inspect->min_max.x - ix;

  if ((ix + offset) > inspect->min_max.w)
    offset = inspect->min_max.w - ix;

  return offset;
}

/* Configure of a NFuse with grip(s), external faces, not its inners */
static bool
_gfuse_delta_adjust(PhxNFuse *inspect, int16_t *hD, int16_t *vD,
                    PhxRectangle hbox, PhxRectangle vbox, Image_s *cImage) {
  PhxRectangle *ibox = &inspect->mete_box;
  int16_t icmp, ibgn, adj;
  int16_t xD = *hD, yD = *vD;
  int16_t xOffset = 0, yOffset = 0;
  static const uint8_t _bit_gravity[12] =
    /* Northern & 1, Southern & 2, Eastern & 4, Western & 8 */
    /*  NW, N, NE, W,  C, E, SW, S, SE, static */
    { 0, 9, 1,  5, 8, 15, 4, 10, 2,  6, 0, 0 };
  bool pp = false;
  uint8_t gravity = (inspect->state >> GRAVITY_SHIFT) & 0x0F;
  gravity = _bit_gravity[gravity];

  if ( (xD != 0) && ((inspect->state & NBIT_HORZ_TOUCH) == 0) ) {
    ibgn = (ibox->y < 0) ? 0 : ibox->y;
    if ( (ibgn >= hbox.y) && (ibox->h <= hbox.h) ) {
      icmp = ibox->x + ibox->w;
      if ( (icmp >= hbox.x) && (icmp <= hbox.w) ) {
        xOffset = hbox.w;
        if (xD < 0)  xOffset = hbox.x;
        xOffset = _rgt_hcheck((PhxNexus*)inspect, (xOffset - icmp));
          /* extra check against min with xD < 0 */
        if (xD < 0) {
          adj = (gravity == 15) ? (GRIPSZ << 1) : GRIPSZ;
          if ((hbox.x - adj) < inspect->min_max.x)
            xOffset = inspect->min_max.x + adj - hbox.w;
        }
      } else if ((gravity & 8) != 0) {
        adj = ((gravity == 15) && ((inspect->state & HXPD_LFT) == 0))
                     ? (GRIPSZ << 1) : GRIPSZ;
        icmp = ibox->x + adj;
        if ( (icmp >= hbox.x) && (icmp <= hbox.w) ) {
          xOffset = hbox.w;
          if (xD < 0)  xOffset = hbox.x;
          xOffset = _rgt_hcheck((PhxNexus*)inspect, (xOffset - icmp));
        }
      }
    }
  }
  if ( (yD != 0) && ((inspect->state & NBIT_VERT_TOUCH) == 0) ) {
    ibgn = (ibox->x < 0) ? 0 : ibox->x;
    if ( (ibgn >= vbox.x) && (ibox->w <= vbox.w) ) {
      icmp = ibox->y + ibox->h;
      if ( (icmp >= vbox.y) && (icmp <= vbox.h) ) {
        yOffset = vbox.h;
        if (yD < 0)  yOffset = vbox.y;
        yOffset = _vcheck((PhxNexus*)inspect, (yOffset - icmp));
          /* extra check against min with yD < 0 */
        if (yD < 0) {
          adj = (gravity == 15) ? (GRIPSZ << 1) : GRIPSZ;
          if ((vbox.y - adj) <= inspect->min_max.y)
            yOffset = inspect->min_max.y + adj - vbox.h;
        }
      } else if ((gravity & 1) != 0) {
        adj = ((gravity == 15) && ((inspect->state & VXPD_TOP) == 0))
                     ? (GRIPSZ << 1) : GRIPSZ;
        icmp = ibox->y + adj;
        if ( (icmp >= vbox.y) && (icmp <= vbox.h) ) {
          yOffset = vbox.h;
          if (yD < 0)  yOffset = vbox.y;
          yOffset = _vcheck((PhxNexus*)inspect, (yOffset - icmp));
        }
      }
    }
  }

  if ((xOffset != 0) || (yOffset != 0)) {
    _mark_nexus((PhxNexus*)inspect, cImage);

    if (xOffset != 0) {
      inspect->state |= NBIT_HORZ_TOUCH;
      adj = (gravity == 15) ? (GRIPSZ << 1) : GRIPSZ;
      if (ibox->w + xOffset < adj) {
        if (ibox->x > 0) {
          if ((ibox->x + xOffset) < 0) {
            inspect->mete_box.w = adj;
            inspect->draw_box.w = 0;
            xOffset = -ibox->x;
          }
          inspect->mete_box.x += xOffset;
            /* alter to allow x at left of grip */
          xOffset -= adj;
          pp = true;
        } else {
          xOffset = 0;
        }
        goto y_test;
      }
      if (ibox->w == adj) {
        if ((gravity & 4) != 0)
          inspect->mete_box.w += xOffset,
          inspect->draw_box.w += xOffset;
        else {
          inspect->mete_box.x += xOffset;
          pp = true;
        }
      } else {
        if ((inspect->state & HXPD_MSK) == HXPD_RGT) {
          inspect->mete_box.w += xOffset;
          inspect->draw_box.w += xOffset;
          if ((gravity & 4) != 0)
            pp = true;
          else
            xOffset = 0;
        } else {
          inspect->mete_box.x += xOffset;
          pp = true;
        }
      }
    } /* end (xOffset != 0) */

  y_test:

    if (yOffset != 0) {
      inspect->state |= NBIT_VERT_TOUCH;
      adj = (gravity == 15) ? (GRIPSZ << 1) : GRIPSZ;

        /* Height with applied offset must be minimum of grip sizes. */
      if ((ibox->h + yOffset) < adj) {
          /* adding yOffset causes height < grip. */
        if (ibox->y > 0) {  /* can grip move towards 0? */
          if ((ibox->y + yOffset) < 0) {
              /* partial move toward 0. */
            inspect->mete_box.h = adj;
            inspect->draw_box.h = 0;
            yOffset = -ibox->y;
          }
          inspect->mete_box.y += yOffset;
          yOffset -= adj;
          pp = true;
        } else {
          yOffset = 0;
        }
        goto assign;
      }
        /* For grip to push adjacent, rather than grow. */
      if (ibox->h == adj) {
        if ((gravity & 2) != 0)
          inspect->mete_box.h += yOffset,
          inspect->draw_box.h += yOffset;
        else {
          inspect->mete_box.y += yOffset;
          pp = true;
        }
      } else {
        if ((inspect->state & VXPD_MSK) == VXPD_BTM) {
          inspect->mete_box.h += yOffset;
          inspect->draw_box.h += yOffset;
          if ((gravity & 2) != 0)
            pp = true;
          else
            yOffset = 0;
        } else {
          inspect->mete_box.y += yOffset;
          pp = true;
        }
      }
    } /* end (yOffset != 0) */

  }  /* end ((xOffset != 0) || (yOffset != 0)) */

assign:
  *hD = xOffset, *vD = yOffset;
  return pp;
}

static void
_rays(PhxNexus *nexus, int16_t hD, int16_t vD,
      PhxRectangle *rH, PhxRectangle *rV, bool type) {

  PhxRectangle hbox, vbox;
  const PhxRectangle *ibox;
  ibox = &nexus->mete_box;       /* current size */

    /* Initial ray regions are, (w, h as point locations):
      hbox: nexus x,y,w values, h:nexus max
      vbox: nexus x,y,h values, w:nexus max */
  hbox.x = (vbox.x = ibox->x);   /* x ray endings */
  hbox.y = (vbox.y = ibox->y);   /* y ray endings */
  hbox.w = hbox.x + ibox->w;     /* create points, w ray ending for hbox */
  hbox.h = nexus->min_max.h;     /* min_max are points,   h ray for hbox */
  vbox.w = nexus->min_max.w;     /* min_max are points,   w ray for vbox */
  vbox.h = vbox.y + ibox->h;     /* create points, h ray ending for vbox */

    /* based off directional resize, adjust initial ray points.
      This will also make 0 motion when none in a direction. */
  if (type) {
    if (hD < 0)  hbox.w = hbox.x - hD;          /* left, use w as origin pt */
    else         hbox.w = hbox.x, hbox.x -= hD; /* right, swap, x as origin */
    if ((nexus->state & VXPD_MSK) == VXPD_TOP)
      hbox.h = ibox->y + ibox->h;               /* limit h ray */
    if (vD < 0)  vbox.h = vbox.y - vD;          /* up, h as origin */
    else         vbox.h = vbox.y, vbox.y -= vD; /* down, swap, y as origin */
    if ((nexus->state & HXPD_MSK) == HXPD_LFT)
      vbox.w = ibox->x + ibox->w;               /* limit w ray */
  } else {
    if (hD < 0)  hbox.x = hbox.w, hbox.w -= hD;
    else         hbox.x = hbox.w - hD;
    if (vD < 0)  vbox.y = vbox.h, vbox.h -= vD;
    else         vbox.y = vbox.h - vD;
  }

/* Need case where right motion forces grip(s) against window's width. */

  if (nexus->type == PHX_GFUSE) {
    uint16_t gravity = (nexus->state >> GRAVITY_SHIFT) & 0x0F;
    switch (gravity) {
      case XCB_GRAVITY_NORTH_WEST:
        break;
      case XCB_GRAVITY_NORTH:
          /* shift y,h to southside of grip if only a grip. */
        if ( (vD > 0)
            && (nexus->mete_box.h == GRIPSZ) )
          vbox.y += GRIPSZ, vbox.h += GRIPSZ;
        break;
      case XCB_GRAVITY_NORTH_EAST:
        vbox.w = ibox->x + ibox->w;
        break;
      case XCB_GRAVITY_WEST:
          /* shift x,w to eastside of grip if only a grip. */
        if ( (hD > 0)
            && (nexus->mete_box.w == GRIPSZ) )
          hbox.x += GRIPSZ, hbox.w += GRIPSZ;
        break;
      case XCB_GRAVITY_CENTER:
          /* This should transfer to far side if together. Should
            zero if not internal delta sweep */
        hbox.h = ibox->y + ibox->h;
        vbox.w = ibox->x + ibox->w;
        break;
      case XCB_GRAVITY_EAST:
        break;
      case XCB_GRAVITY_SOUTH_WEST:
        hbox.h = ibox->y + ibox->h;
        break;
      case XCB_GRAVITY_SOUTH:
        break;
      case XCB_GRAVITY_SOUTH_EAST:
        hbox.h = ibox->y + ibox->h;
        vbox.w = ibox->x + ibox->w;
        break;
      default:
        break;
    }
  }
  *rH = hbox;
  *rV = vbox;
}

/* Called by ui_nexus_resize() and itself.
  push right/btm pull (occurs when 'within' port has resize) */
static void
_prpb_sweep(PhxInterface *iface, uint16_t ndx, int16_t hD, int16_t vD,
                       Image_s *cImage) {

  PhxRectangle hbox, vbox;
#if 0
  bool is_gfuse;
#endif

  if (ndx == 0)  return;

    /* Set 'ray' regions to determine moves. 0==use prpb adjusts. */
  _rays(iface->nexus[ndx], hD, vD, &hbox, &vbox, 0);

#if 0
  is_gfuse = (iface->nexus[ndx]->type == PHX_GFUSE);
#endif

  do {
    const PhxRectangle *ibox;
    PhxNexus *inspect;
    int16_t icmp, iend, ibgn;
    int16_t hOffset = 0, vOffset = 0;
    inspect = iface->nexus[(--ndx)];
    ibox = &inspect->mete_box;

    if ( ((inspect->state & HXPD_MSK) != 0) && (hD != 0) ) {
      iend = inspect->min_max.h;
      if ((inspect->state & VXPD_MSK) != 0)
        iend = ibox->y + ibox->h;
      ibgn = (ibox->y < 0) ? 0 : ibox->y;
      if ( (ibgn >= hbox.y) && (iend <= hbox.h) ) {
        icmp = ibox->x;
        if ( (icmp >= hbox.x) && (icmp <= hbox.w) ) {
          hOffset = hbox.w;
          if (hD < 0)  hOffset = hbox.x;
          hOffset = _lft_hcheck(inspect, (hOffset - icmp));
        }
      }
    }
    if ( ((inspect->state & VXPD_MSK) != 0) && (vD != 0) ) {
      iend = inspect->min_max.w;
      if ((inspect->state & HXPD_MSK) != 0)
        iend = ibox->x + ibox->w;
      ibgn = (ibox->x < 0) ? 0 : ibox->x;
      if ( (ibgn >= vbox.x) && (iend <= vbox.w) ) {
        icmp = ibox->y;
        if ( (icmp >= vbox.y) && (icmp <= vbox.h) ) {
          vOffset = vbox.h;
          if (vD < 0)  vOffset = vbox.y;
          vOffset = _vcheck(inspect, (vOffset - icmp));
        }
      }
    }

    if ((hOffset != 0) || (vOffset != 0)) {
      bool pp = false;
      _mark_nexus(inspect, cImage);
      if (hOffset != 0) {
        inspect->state |= NBIT_HORZ_TOUCH;
        if ((inspect->state & HXPD_MSK) == HXPD_RGT) {
          inspect->mete_box.x += hOffset;
          if ( (inspect->type == PHX_GFUSE)
              && (inspect->mete_box.w >= GRIPSZ) ) {
            int16_t delta;
            delta = minof((inspect->mete_box.w - GRIPSZ), hOffset);
            inspect->mete_box.w -= delta;
            inspect->draw_box.w -= delta;
            pp = (inspect->mete_box.w == GRIPSZ);
          } else {
            inspect->mete_box.w -= hOffset;
            inspect->draw_box.w -= hOffset;
          }
        } else {
          inspect->mete_box.x += hOffset;
          pp = true;
        }
      }
      if (vOffset != 0) {
        inspect->state |= NBIT_VERT_TOUCH;
        if ((inspect->state & VXPD_MSK) == VXPD_BTM) {
          inspect->mete_box.y += vOffset;
          if ( (inspect->type == PHX_GFUSE)
              && (inspect->mete_box.h >= GRIPSZ) ) {
            int16_t delta;
            delta = minof((inspect->mete_box.h - GRIPSZ), vOffset);
            inspect->mete_box.h -= delta;
            inspect->draw_box.h -= delta;
            pp = (inspect->mete_box.h == GRIPSZ);
          } else {
            inspect->mete_box.h -= vOffset;
            inspect->draw_box.h -= vOffset;
          }
        } else {
          inspect->mete_box.y += vOffset;
          inspect->mete_box.h += vOffset;
          inspect->draw_box.h += vOffset;
          pp = true;
        }
      }
      DEBUG_ASSERT(( (inspect->type == PHX_GFUSE)
                    && ( (inspect->mete_box.h < GRIPSZ)
                        || (inspect->mete_box.w < GRIPSZ) ) ),
                   "error: Crushed grip.");
      if (pp) _prpb_sweep(iface, ndx, hOffset, vOffset, cImage);
    }
  } while (ndx != 0);
}

/* x moves if pushed against gravity bar, when hbox pushes on x for hD>0
  or when hbox pushes on width == GRIPSZ for hD<0. */
static bool
_plpt_x_moves(PhxNexus *inspect, PhxRectangle hbox, int16_t hD) {

  int16_t width, gravity;

  width = 0;
  gravity = (inspect->state >> GRAVITY_SHIFT) & 0x0F;
    /* If westward component. Need push against grip. */
  if ( (gravity == (int16_t)XCB_GRAVITY_WEST)
      || (gravity == (int16_t)XCB_GRAVITY_NORTH_WEST)
      || (gravity == (int16_t)XCB_GRAVITY_SOUTH_WEST) ) {
    if ( ((inspect->mete_box.w + hD) < GRIPSZ)
        && (inspect->mete_box.w == GRIPSZ) )
      return true;
    if ( (hD > 0)
        && (inspect->mete_box.x >= hbox.x)
        && (inspect->mete_box.x <= hbox.w) )
      return true;
    return false;
  }

  if ( (gravity != (int16_t)XCB_GRAVITY_NORTH)
      && (gravity != (int16_t)XCB_GRAVITY_SOUTH) ) {
    width = GRIPSZ;
    if (gravity == (int16_t)XCB_GRAVITY_CENTER)  width *= 2;
  }
  if ( (inspect->mete_box.w == width)
      || ((inspect->mete_box.w + hD) < 0) )
    return true;
  return false;
}

static bool
_plpt_y_moves(PhxNexus *inspect, PhxRectangle vbox, int16_t vD) {

  int16_t height, gravity;

  height = 0;
  gravity = (inspect->state >> GRAVITY_SHIFT) & 0x0F;
    /* If northward component. Need push against grip. */
  if ( (gravity == (int16_t)XCB_GRAVITY_NORTH)
      || (gravity == (int16_t)XCB_GRAVITY_NORTH_WEST)
      || (gravity == (int16_t)XCB_GRAVITY_NORTH_EAST) ) {
    if ( ((inspect->mete_box.h + vD) < GRIPSZ)
        && (inspect->mete_box.h == GRIPSZ) )
      return true;
    if ( (vD > 0)
        && (inspect->mete_box.y >= vbox.y)
        && (inspect->mete_box.y <= vbox.h) )
      return true;
    return false;
  }

  if ( (gravity != (int16_t)XCB_GRAVITY_WEST)
      && (gravity != (int16_t)XCB_GRAVITY_EAST) ) {
    height = GRIPSZ;
    if (gravity == (int16_t)XCB_GRAVITY_CENTER)  height *= 2;
  }
  if ( (inspect->mete_box.h == height)
      || ((inspect->mete_box.h + vD) < 0) )
    return true;
  return false;
}

/* From ui_nexus_resize(): entry when hD or vD change. hOffset, vOffset
  changes dealt with in prpb(). Because hD,vD moved, the name lt
  for left/top. */
static void
_plpt_sweep(PhxInterface *iface, uint16_t ndx, int16_t hD, int16_t vD,
                       Image_s *cImage) {

  PhxRectangle hbox, vbox;

  if (ndx == 0)  return;

    /* Set 'ray' regions to determine moves. 1==use plpt adjusts. */
  _rays(iface->nexus[ndx], hD, vD, &hbox, &vbox, 1);

  do {
    const PhxRectangle *ibox;
    PhxNexus *inspect;
    bool x_moves = false, y_moves = false;
    int16_t icmp, iend, ibgn;
    int16_t hOffset = 0, vOffset = 0;

    inspect = iface->nexus[(--ndx)];
    ibox = &inspect->mete_box;

      /* Because of overlapping nexus, don't elimate by NBIT_HORZ_TOUCH */
    if ( ((inspect->state & HXPD_MSK) != 0) && (hD != 0) ) {
      iend = inspect->min_max.h;
      if ((inspect->state & VXPD_MSK) != 0)
        iend = ibox->y + ibox->h;
      ibgn = (ibox->y < 0) ? 0 : ibox->y;
      if ( (ibgn >= hbox.y) && (iend <= hbox.h) ) {
        icmp = ibox->x + ibox->w;
        if ( (inspect->type == PHX_GFUSE)
            && (x_moves = _plpt_x_moves(inspect, hbox, hD))
            && (ibox->w != GRIPSZ) )
          icmp -= ibox->w;
        if ( (icmp >= hbox.x) && (icmp <= hbox.w) ) {
          hOffset = hbox.w;
          if (hD < 0)  hOffset = hbox.x;
          hOffset = _rgt_hcheck(inspect, (hOffset - icmp));
        }
      }
    }

      /* Because of overlapping nexus, don't elimate by NBIT_VERT_TOUCH */
    if ( ((inspect->state & VXPD_MSK) != 0) && (vD != 0) ) {
      iend = inspect->min_max.w;
      if ((inspect->state & HXPD_MSK) != 0)
        iend = ibox->x + ibox->w;
      ibgn = (ibox->x < 0) ? 0 : ibox->x;
      if ( (ibgn >= vbox.x) && (iend <= vbox.w) ) {
        icmp = ibox->y + ibox->h;
        if ( (inspect->type == PHX_GFUSE)
            && (y_moves = _plpt_y_moves(inspect, vbox, vD))
            && (ibox->h != GRIPSZ) )
          icmp -= ibox->h;
        if ( (icmp >= vbox.y) && (icmp <= vbox.h) ) {
          vOffset = vbox.h;
          if (vD < 0)  vOffset = vbox.y;
          vOffset = _vcheck(inspect, (vOffset - icmp));
        }
      }
    }

    if ((hOffset != 0) || (vOffset != 0)) {
      bool pp = false;
      _mark_nexus(inspect, cImage);
      if (hOffset != 0) {
        inspect->state |= NBIT_HORZ_TOUCH;
        if ( ((inspect->state & HXPD_MSK) == HXPD_RGT)
            && (!x_moves) ) {
          if ( (inspect->type != PHX_GFUSE)
              || (hOffset >= (GRIPSZ - inspect->mete_box.w)) ) {
            inspect->mete_box.w += hOffset;
            inspect->draw_box.w += hOffset;
          } else {
            goto xdelta;
          }
        } else {
          if ( (inspect->type != PHX_GFUSE)
              || (hOffset >= (GRIPSZ - inspect->mete_box.w)) ) {
            inspect->mete_box.x += hOffset;
            if ( (inspect->type == PHX_GFUSE)
                && ((inspect->state & HXPD_MSK) == HXPD_RGT) ) {
              inspect->mete_box.w -= hOffset;
              inspect->draw_box.w -= hOffset;
            }
          } else {
            int16_t delta;
xdelta:     delta = GRIPSZ - inspect->mete_box.w;
            inspect->mete_box.x += hOffset - delta;
            inspect->mete_box.w += delta;
            inspect->draw_box.w += delta;
          }
          pp = true;
        }
      }
      if (vOffset != 0) {
        inspect->state |= NBIT_VERT_TOUCH;
        if ( ((inspect->state & VXPD_MSK) == VXPD_BTM)
            && (!y_moves) ) {
          if ( (inspect->type != PHX_GFUSE)
              || (vOffset >= (GRIPSZ - inspect->mete_box.h)) ) {
            inspect->mete_box.h += vOffset;
            inspect->draw_box.h += vOffset;
          } else {
            goto ydelta;
          }
        } else {
          if ( (inspect->type != PHX_GFUSE)
              || (vOffset >= (GRIPSZ - inspect->mete_box.h)) ) {
            inspect->mete_box.y += vOffset;
            if ( (inspect->type == PHX_GFUSE)
                && ((inspect->state & VXPD_MSK) == VXPD_BTM) ) {
              inspect->mete_box.h -= vOffset;
              inspect->draw_box.h -= vOffset;
            }
          } else {
            int16_t delta;
ydelta:     delta = GRIPSZ - inspect->mete_box.h;
            inspect->mete_box.y += vOffset - delta;
            inspect->mete_box.h += delta;
            inspect->draw_box.h += delta;
          }
          pp = true;
        }
      }

      DEBUG_ASSERT(( (inspect->type == PHX_GFUSE)
                    && ( (inspect->mete_box.h < GRIPSZ)
                        || (inspect->mete_box.w < GRIPSZ) ) ),
                   "error: Crushed grip.");
      if (pp) _plpt_sweep(iface, ndx, hOffset, vOffset, cImage);
    }
  } while (ndx != 0);
}

static void
_interface_delta_sweep(PhxInterface *iface, int16_t hD, int16_t vD,
                                                        Image_s *cImage) {
  PhxRectangle *ibox, hbox, vbox;
  PhxNexus *inspect = (PhxNexus*)iface;
  uint16_t ndx;

    /* interface push/pull rays are to inf */
  ibox = &inspect->mete_box;
  hbox.x = (vbox.x = -INT16_MAX);
  hbox.y = (vbox.y = -INT16_MAX);
  hbox.w = ibox->w;              /* create points */
  hbox.h = INT16_MAX;            /* min_max are points */
  vbox.w = INT16_MAX;            /* min_max are points */
  vbox.h = ibox->h;              /* create points */

  if (iface->type != PHX_GFUSE) {
    if (hD < 0)  hbox.x = hbox.w, hbox.w -= hD;
    else         hbox.x = hbox.w - hD;
    if (vD < 0)  vbox.y = vbox.h, vbox.h -= vD;
    else         vbox.y = vbox.h - vD;
  } else {
    if (hD < 0)  hbox.x = iface->draw_box.w + iface->draw_box.x,
                 hbox.w = hbox.x - hD;
    else         hbox.w = iface->draw_box.w + iface->draw_box.x,
                 hbox.x = hbox.w - hD;
    if (vD < 0)  vbox.y = iface->draw_box.h + iface->draw_box.y,
                 vbox.h = vbox.y - vD;
    else         vbox.h = iface->draw_box.h + iface->draw_box.y,
                 vbox.y = vbox.h - vD;
  }

  ndx = iface->ncount;
  do {
    int16_t icmp, ibgn;
    int16_t hOffset = 0, vOffset = 0;
    inspect = iface->nexus[(--ndx)];

      /* Differs from NFuse/Nexus, has internal (grip) min/max to consider. */
      /* Since rare case, seperated to keep normal operation small. */
    if (inspect->type == PHX_GFUSE) {
      bool r;
      hOffset = hD, vOffset = vD;
      r = _gfuse_delta_adjust((PhxNFuse*)inspect, &hOffset, &vOffset,
                                                  hbox, vbox, cImage);
      if (r) _plpt_sweep(iface, ndx, hOffset, vOffset, cImage);
      continue;
    }

    ibox = &inspect->mete_box;

    if ((inspect->state & HXPD_MSK) != 0) {
      if ( (hD != 0) && ((inspect->state & NBIT_HORZ_TOUCH) == 0) ) {
        ibgn = (ibox->y < 0) ? 0 : ibox->y;
        if ( (ibgn >= hbox.y) && (ibox->h <= hbox.h) ) {
          icmp = ibox->x + ibox->w;
          if ( (icmp >= hbox.x) && (icmp <= hbox.w) ) {
            hOffset = hbox.w;
            if (hD < 0)  hOffset = hbox.x;
            hOffset = _rgt_hcheck(inspect, (hOffset - icmp));
          }
        }
      }
    }
    if ((inspect->state & VXPD_MSK) != 0) {
      if ( (vD != 0) && ((inspect->state & NBIT_VERT_TOUCH) == 0) ) {
        ibgn = (ibox->x < 0) ? 0 : ibox->x;
        if ( (ibgn >= vbox.x) && (ibox->w <= vbox.w) ) {
          icmp = ibox->y + ibox->h;
          if ( (icmp >= vbox.y) && (icmp <= vbox.h) ) {
            vOffset = vbox.h;
            if (vD < 0)  vOffset = vbox.y;
            vOffset = _vcheck(inspect, (vOffset - icmp));
          }
        }
      }
    }

    if ((hOffset != 0) || (vOffset != 0)) {
      bool pp = false;
      _mark_nexus(inspect, cImage);

      if (hOffset != 0) {
        inspect->state |= NBIT_HORZ_TOUCH;
        if ((inspect->state & HXPD_MSK) == HXPD_RGT) {
          inspect->mete_box.w += hOffset;
          inspect->draw_box.w += hOffset;
            /* no push/pull ability */
          hOffset = 0;
        } else {
          inspect->mete_box.x += hOffset;
          pp = true;
        }
      }
      if (vOffset != 0) {
        inspect->state |= NBIT_VERT_TOUCH;
        if ((inspect->state & VXPD_MSK) == VXPD_BTM) {
          inspect->mete_box.h += vOffset;
          inspect->draw_box.h += vOffset;
            /* no push/pull ability */
          vOffset = 0;
        } else {
          inspect->mete_box.y += vOffset;
          pp = true;
        }
      }
      if (pp)  _plpt_sweep(iface, ndx, hOffset, vOffset, cImage);
    }
  } while (ndx != 0);
}

/* Run NFuse as a mini-window, as simulated push/pull lower right grab */
/* Configure of NFuse inners. */
static void
_nfuse_delta_sweep(PhxInterface *iface, Image_s *cImage) {

  PhxNexus *inspect;
  uint16_t ndx, idx = cImage->icount;
  if (idx == 0)  return;

  ndx = iface->ncount;
  do {
    inspect = iface->nexus[(--ndx)];
    if (OBJECT_BASE_TYPE(inspect) == PHX_NFUSE) {
      PhxRectangle *img_rect;
        /* determine if creates a hD, vD or both */
        /* no members, move on */
      if (inspect->ncount == 0)  continue;
        /* find inspect in cImage, compare, calculate deltas */
      img_rect = NULL;
      for (idx = 0; idx < cImage->icount; idx++)
        if (inspect == cImage->idata[idx].nexus) {
          img_rect = &cImage->idata[idx].mete_box;
          break;
        }
      if (img_rect != NULL) {
        uint32_t *img_wh = (uint32_t*)&img_rect->w;
        uint32_t *nsp_wh = (uint32_t*)&inspect->mete_box.w;
        if (*img_wh != *nsp_wh) {
          int16_t hD = inspect->mete_box.w - img_rect->w;
          int16_t vD = inspect->mete_box.h - img_rect->h;
          _interface_delta_sweep((PhxInterface*)inspect, hD, vD, cImage);
          _nfuse_delta_sweep((PhxInterface*)inspect, cImage);
        }
      }
    } /* end (inspect->type == PHX_NFUSE) */
  } while (ndx != 0);
}

#pragma mark *** Signal events ***

/* A nexus was resized. Any surface touching its mete_box must be
  redrawn. This has nothing to do with adjustments based on configure
  notify events. */
static void
_invalidate_configured(PhxInterface *iface, Image_s *cImage) {

  uint16_t idx;
  PhxRectangle invalid;
  RECTANGLE(invalid, INT16_MAX, INT16_MAX, -INT16_MAX, -INT16_MAX);

  idx = cImage->icount;
  do {
    IData_s *nd;
    uint64_t *ib, *nb;
    PhxRectangle bbox;
    PhxNexus *inspect = cImage->idata[(--idx)].nexus;
    PhxInterface *mount = (PhxInterface*)inspect;
    if (inspect->surface == NULL)  continue;

    nd = &cImage->idata[idx];
    ib = (uint64_t*)&inspect->mete_box.x;
    nb = (uint64_t*)&nd->mete_box.x;
    if (*ib == *nb)  continue;
    bbox.x = minof(inspect->mete_box.x, nd->mete_box.x);
    bbox.y = minof(inspect->mete_box.y, nd->mete_box.y);
    bbox.w = maxof(inspect->mete_box.x + inspect->mete_box.w,
                        nd->mete_box.x +      nd->mete_box.w);
    bbox.h = maxof(inspect->mete_box.y + inspect->mete_box.h,
                        nd->mete_box.y +      nd->mete_box.h);

    bbox.x = maxof(bbox.x, 0);
    bbox.y = maxof(bbox.y, 0);
    while (!IS_WINDOW_TYPE((mount = mount->i_mount))) {
      bbox.x += mount->mete_box.x;
      bbox.y += mount->mete_box.y;
      bbox.w += mount->mete_box.x;
      bbox.h += mount->mete_box.y;
    }
    invalid.x = minof(invalid.x, bbox.x);
    invalid.y = minof(invalid.y, bbox.y);
    invalid.w = maxof(invalid.w, bbox.w);
    invalid.h = maxof(invalid.h, bbox.h);

  } while (idx != 0);

  invalid.x = maxof(invalid.x, 0);
  invalid.y = maxof(invalid.y, 0);
  invalid.w = maxof(invalid.w, 0);
  invalid.h = maxof(invalid.h, 0);
  invalid.w = minof(invalid.w, iface->mete_box.w);
  invalid.h = minof(invalid.h, iface->mete_box.h);

  invalid.w -= invalid.x;
  invalid.h -= invalid.y;

  ui_invalidate_rectangle(iface, invalid);
}

/* User gets sent moved/resized configure notice directly. They
  decide if nexus/contents of needs to be invalidated. */
static void
_interface_configure_callbacks(PhxInterface *iface, Image_s *cImage) {

  uint16_t idx;
  xcb_configure_notify_event_t configure;
  memset(&configure, 0, sizeof(xcb_configure_notify_event_t));
  configure.response_type = XCB_CONFIGURE_NOTIFY;

  idx = cImage->icount;
  do {
    xcb_generic_event_t *nvt = (xcb_generic_event_t*)&configure;
    PhxNexus *inspect = cImage->idata[(--idx)].nexus;
    IData_s *nd = &cImage->idata[idx];
    uint64_t *ib = (uint64_t*)&inspect->mete_box;
    uint64_t *nb = (uint64_t*)&nd->mete_box;
    if (*ib == *nb)  continue;
    if (inspect->_event_cb != NULL) {
      bool handled;
        /* This is done to protect from user messing with */
      PhxRectangle swap_mete = inspect->mete_box;
      PhxRectangle swap_draw = inspect->draw_box;
      inspect->mete_box = nd->mete_box;
      inspect->draw_box = nd->draw_box;
      configure.event  = inspect->window;
      configure.window = inspect->window;
      configure.x      = swap_mete.x;
      configure.y      = swap_mete.y;
      configure.width  = swap_mete.w;
      configure.height = swap_mete.h;

#if DEBUG_EVENTS_ON
      DEBUG_EVENTS("_interface_configure_callbacks");
#endif
      handled = inspect->_event_cb(iface, (void*)nvt, (PhxObject*)inspect);
        /* set mete_box to _interface_configure() results */
      if (!handled)
        inspect->mete_box = swap_mete,
        inspect->draw_box = swap_draw;
    }
  } while (idx != 0);
}

#pragma mark *** Configure ***

/* Must do topmost iface before children. */
static void
_interface_resurface(PhxInterface *iface) {

  uint16_t idx = 0;
  do {
    PhxNexus *inspect = iface->nexus[idx];
    if (!!(inspect->state & (NBIT_HORZ_TOUCH | NBIT_VERT_TOUCH))) {
      PhxRectangle r;
      bool visible;
      inspect->state &= ~(NBIT_HORZ_TOUCH | NBIT_VERT_TOUCH);
      inspect->state |= OBIT_SUR_TOUCH;
      r = inspect->mete_box;
      visible = ( (r.w > 0) && ((r.x + r.w) > 0)
                 && (r.h > 0) && ((r.y + r.h) > 0) );
      ui_visible_set((PhxObject*)inspect, visible);
      if ( (r.w > (int16_t)inspect->sur_width)
          || (r.h > (int16_t)inspect->sur_height) ) {
        cairo_status_t error;
        DEBUG_ASSERT((inspect->surface == NULL),
                                           "error(1): _interface_resurface");
        cairo_surface_destroy(inspect->surface);
        if (r.w < (int16_t)inspect->sur_width)   r.w = inspect->sur_width;
        if (r.h < (int16_t)inspect->sur_height)  r.h = inspect->sur_height;
        inspect->surface = ui_surface_create_similar(iface, r.w, r.h);
        error = cairo_surface_status(inspect->surface);
        DEBUG_ASSERT((error != CAIRO_STATUS_SUCCESS),
                                           "error(2): _interface_resurface()");
        inspect->sur_width  = r.w;
        inspect->sur_height = r.h;
      }
        /* At minimum, reset internal state flag. */
      if ( (OBJECT_BASE_TYPE(inspect) == PHX_NFUSE)
          && (inspect->ncount != 0) )
        _interface_resurface((PhxInterface*)inspect);
    }
  } while ((++idx) < iface->ncount);
}

void
ui_nexus_resize(PhxNexus *nexus, PhxRectangle *rD) {

  Image_s cImage;
  int16_t hD, vD, hOffset, vOffset;
  const PhxRectangle *mbox;
  PhxInterface *mount;
  uint16_t ndx;

  if (IS_WINDOW_TYPE(nexus)) {
    DEBUG_ASSERT(true,
               "failure: PhxInterface can't be resized ui_nexus_resize()");
    return;
  }

  if (_cImage_initialize(&cImage))  return;

  mbox = &nexus->mete_box;
  hD = rD->x - mbox->x;
  if ((mbox->x + hD) < nexus->min_max.x)
    hD = nexus->min_max.x - mbox->x;
  hOffset = rD->w - mbox->w;
  if ((mbox->x + hD + mbox->w + hOffset) > nexus->min_max.w)
    hOffset = nexus->min_max.w - (mbox->x + hD + mbox->w);

  vD = rD->y - mbox->y;
  if ((mbox->y + vD) < nexus->min_max.y)
    vD = nexus->min_max.y - mbox->y;
  vOffset = rD->h - mbox->h;
  if ((mbox->y + vD + mbox->h + vOffset) > nexus->min_max.h)
    vOffset = nexus->min_max.h - (mbox->y + vD + mbox->h);

    /* Size nexus, and its surrounding nexus on mount level */
    /* As usual, layered, so can only move those below */
  mount = nexus->i_mount;
  ndx = mount->ncount;
  do {
    PhxNexus *inspect = mount->nexus[(--ndx)];
    if (inspect == nexus) {
      bool plpt = false, prpb = false;
        /* Will mark internals also */
      _mark_nexus(inspect, &cImage);
      if (hD != 0) {  /* move operation, doesn't effect width, nor draw */
        inspect->state |= NBIT_HORZ_TOUCH;
        inspect->mete_box.x += hD;
        plpt = true;
      }
      if (hOffset != 0) {  /* resize operation */
        inspect->state |= NBIT_HORZ_TOUCH;
        inspect->mete_box.w += hOffset;
        inspect->draw_box.w += hOffset;
        prpb = true;
      }
      if (vD != 0) {  /* move operation, doesn't effect width, nor draw */
        inspect->state |= NBIT_VERT_TOUCH;
        inspect->mete_box.y += vD;
        plpt = true;
      }
      if (vOffset != 0) {  /* resize operation */
        inspect->state |= NBIT_VERT_TOUCH;
        inspect->mete_box.h += vOffset;
        inspect->draw_box.h += vOffset;
        prpb = true;
      }
        /* using left/top as a normal iface reconfigure */
      if (plpt)  _plpt_sweep(mount, ndx, hD, vD, &cImage);
        /* using right/btm as reconfigure 'grab' */
      if (prpb)  _prpb_sweep(mount, ndx, hOffset, vOffset, &cImage);
      break;
    }
  } while (ndx != 0);

    /* With externals changed, now handle internals */
  if ( ((hOffset | vOffset) != 0)
      && (OBJECT_BASE_TYPE(nexus) == PHX_NFUSE)
      && (nexus->ncount != 0) )
    _interface_delta_sweep((PhxInterface*)nexus, hOffset, vOffset, &cImage);

    /* At this point, if had PHX_NFUSE types, they may have been sized
      and marked. Need altered fusion internals sized if was a hD or vD. */
  _nfuse_delta_sweep(mount, &cImage);
    /* At minimum, send nexus->_event_cb signal */
  _interface_resurface(mount);
    /* Each nexus that changed sends a CONFIGURE_NOTIFY its
       callback. This allows user to modify its objects as they see fit */
  _interface_configure_callbacks(mount, &cImage);
    /* Send 'expose' to bounding box of altered nexus. */
  _invalidate_configured(mount, &cImage);

  free(cImage.idata);
}

void
_interface_configure(PhxInterface *iface, int16_t hD, int16_t vD) {

  Image_s cImage;
  IData_s *idata;
  uint16_t icount;

  if (!IS_WINDOW_TYPE(iface)) {
    DEBUG_ASSERT(true, "error: entry type _interface_configure().");
    return;
  }

  if (_cImage_initialize(&cImage))  return;

    /* Creates before image, for notify of altered configure size */
  icount = cImage.icount;
  idata = &cImage.idata[icount];
  cImage.icount += 1;
  idata->nexus = (PhxNexus*)iface;
  idata->mete_box = iface->mete_box;
  idata->mete_box.x = (idata->mete_box.y = 0);
  idata->draw_box = iface->draw_box;

  iface->mete_box.w += hD;
  iface->draw_box.w += hD;
  iface->mete_box.h += vD;
  iface->draw_box.h += vD;

    /* resize video buffer */
  cairo_xcb_surface_set_size(iface->vid_buffer,
                             iface->mete_box.w,
                             iface->mete_box.h);

    /* Determine if resizing of iface's surface is needed. */
  if ( (iface->mete_box.w > iface->sur_width)
      || (iface->mete_box.h > iface->sur_height) ) {

    cairo_status_t error;

    if (iface->surface != NULL)
      cairo_surface_destroy(iface->surface);

    iface->sur_width  = maxof(iface->draw_box.w, iface->sur_width);
    iface->sur_height = maxof(iface->draw_box.h, iface->sur_height);

    iface->surface
      = ui_surface_create_similar(iface, iface->sur_width, iface->sur_height);
    error = cairo_surface_status(iface->surface);
    if (error != CAIRO_STATUS_SUCCESS) {
      DEBUG_ASSERT(true, "Some weird cairo error...?!");
      goto rejected;
    }
  }

  if (iface->ncount != 0) {
    _interface_delta_sweep(iface, hD, vD, &cImage);

      /* At this point, if had PHX_NFUSE types they hace been sized
         and marked. Need internals of now sized if was a hD or vD found.
         Normal for nfuse is x,y moves, but user may have altered flags */
    _nfuse_delta_sweep(iface, &cImage);

      /* Checks to see if a surface needs adjusting. */
    _interface_resurface(iface);
  }
    /* Each nexus that changed sends a CONFIGURE_NOTIFY its
       callback. This allows user to modify its objects as they see fit */
  _interface_configure_callbacks(iface, &cImage);

rejected:
  free(cImage.idata);
}
