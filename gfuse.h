#ifndef __SESS_GFUSE_H__
#define __SESS_GFUSE_H__

#include "nexus.h"

 /* specific to gnexus */
#define GRIPSZ                     8

extern PhxNFuse *        ui_gfuse_create(PhxInterface *iface,
                                         PhxRectangle configure,
                                         xcb_gravity_t gravity);
extern void              _draw_grab_grip(PhxNexus *nexus, cairo_t *cr);

#endif /* __SESS_GFUSE_H__ */