#ifndef __SESS_DRAW_H__
#define __SESS_DRAW_H__

#include "session.h"
#include "timers.h"
#include "util/xcb_aux.h"  /* xcb_aux_sync() */

/* Avoidance of endian issues */
union rectangle_endianess {
  PhxRectangle rect;
  uint64_t     r64;
};

extern PhxRGBA RGBA_DEFAULT_BG;
extern PhxRGBA RGBA_SELECTION;
extern PhxRGBA RGBA_TEXT_FGFILL;
extern PhxRGBA RGBA_SEARCH_FGFILL;

extern bool              _interface_draw(PhxInterface *iface,
                                         xcb_expose_event_t *expose);
extern void              ui_invalidate_rectangle(PhxInterface *iface,
                                                 PhxRectangle dirty);
extern void              ui_invalidate_object(PhxObject *obj);
extern cairo_surface_t * ui_surface_create_similar(PhxInterface *iface,
                                                   int16_t w,
                                                   int16_t h);
#endif /* __SESS_DRAW_H__ */
