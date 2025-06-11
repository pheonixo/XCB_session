#ifndef __SESS_OBJECTS_H__
#define __SESS_OBJECTS_H__

#include "nexus.h"
#include "draw.h"

/* Objects probably need surfaces too. For redraw and animation */
struct _PhxObject {
 PhxObjectType      type;
 uint16_t           ncount;
 uint32_t           state;           /* flags/state, see below */
 PhxRectangle       mete_box;        /* allocate box */
 PhxRectangle       draw_box;        /* clip box */
 PhxDrawHandler     _draw_cb;        /* background, seperated expose event */
 PhxActionHandler   _event_cb;       /* actions meter */
 PhxRazeHandler     _raze_cb;        /* deconstruct internals */
   /* Intent is a PhxObject type, but user free to recast.
     Stated because user reponsible for 'raze' of child, and
     control of ncount. This leaves oprn possible use of child
     as a handle, pointer to pointer(s). */
 PhxObject          *child;          /* non-specific addon */
 PhxInterface       *i_mount;        /* port mounting (window connector) */
 PhxAttr            *attrib;         /* drawing/text attribute data */
 PhxObject          *o_mount;        /* object mounting (belongs to) */

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
 xcb_dnd_property_t *dnd_quirk;      /* dnd support */
 bool              (*_obj_status_cb)(xcb_dnd_notify_event_t *);
#endif
 void               *exclusive;
};

extern PhxObject *       ui_object_create(PhxNexus *nexus,
                                          PhxObjectType type,
                                          PhxDrawHandler draw,
                                          PhxRectangle configure);
extern PhxObject *       ui_object_child_create(PhxObject *obj,
                                                PhxObjectType type,
                                                PhxDrawHandler draw,
                                                PhxRectangle configure);
extern void              _default_object_raze(void *obj);

extern void              ui_draw_vertical_line(PhxObject *b,
                                               cairo_t *cr);
extern void              ui_draw_horizontal_line(PhxObject *b,
                                                 cairo_t *cr);
extern void              ui_draw_right_arrow(PhxObject *b,
                                             cairo_t *cr);


#endif /* __SESS_OBJECTS_H__ */
