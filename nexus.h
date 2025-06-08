#ifndef __SESS_NEXUS_H__
#define __SESS_NEXUS_H__

#include "windows.h"

struct _PhxNFuse {
 PhxObjectType      type;
 uint16_t           ncount;
 uint32_t           state;           /* flags/state, see below */
 PhxRectangle       mete_box;        /* allocate box */
 PhxRectangle       draw_box;        /* clip box */
 PhxDrawHandler     _draw_cb;        /* background, seperated expose event */
 PhxActionHandler   _event_cb;       /* actions meter */
 PhxRazeHandler     _raze_cb;        /* deconstruct internals */
 PhxNexus           **nexus;         /* list of alloted contained objects */
 PhxInterface       *i_mount;        /* Interface or NFuse connection */
 PhxAttr            *attrib;         /* attribute defaults */
 PhxRectangle       min_max;
 cairo_surface_t    *surface;
 uint16_t           sur_width;       /* for determining re-surfacing */
 uint16_t           sur_height;      /* for determining re-surfacing */
 xcb_window_t       window;
   /* information this object may need for operation */
 void               *exclusive;
};

struct _PhxNexus {
 PhxObjectType      type;
 uint16_t           ncount;
 uint32_t           state;           /* flags/state, see below */
 PhxRectangle       mete_box;        /* allocate box */
 PhxRectangle       draw_box;        /* clip box */
 PhxDrawHandler     _draw_cb;        /* background, seperated expose event */
 PhxActionHandler   _event_cb;       /* actions meter */
 PhxRazeHandler     _raze_cb;        /* deconstruct internals */
/* differs from nfuse in name only */
 PhxObject          **objects;       /* list of alloted contained objects */
 PhxInterface       *i_mount;        /* Interface or NFuse connection */
 PhxAttr            *attrib;         /* attribute defaults */
 PhxRectangle       min_max;
 cairo_surface_t    *surface;
 uint16_t           sur_width;       /* for determining re-surfacing */
 uint16_t           sur_height;      /* for determining re-surfacing */
 xcb_window_t       window;
   /* information this object may need for operation */
 void               *exclusive;
};

extern PhxNexus *        ui_nexus_create(PhxInterface *iface,
                                         PhxRectangle configure);
extern void              _default_nexus_raze(void *nexus);
extern bool              _default_nexus_meter(PhxInterface *iface,
                                              xcb_generic_event_t *event,
                                              PhxObject *obj);
extern uint16_t          _default_nexus_remove_object(PhxNexus *nexus,
                                                      PhxObject *obj);

#endif /* __SESS_NEXUS_H__ */
