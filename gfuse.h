#ifndef __SESS_GFUSE_H__
#define __SESS_GFUSE_H__

#include "nexus.h"

 /* specific to gnexus */
#define GRIPSZ                     8

 /* gfuse will create a nfuse that's offset by grip size amounts.
   This is the mount 0,0 for all attaching to gfuse. */
extern PhxNFuse *        ui_gfuse_create(PhxInterface *iface,
                                         PhxRectangle configure,
                                         xcb_gravity_t gravity);

#endif /* __SESS_GFUSE_H__ */