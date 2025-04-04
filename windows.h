#ifndef __SESS_WINDOWS_H__
#define __SESS_WINDOWS_H__

#include "session.h"

/*windows*/
typedef struct _PhxNFuse                                 PhxNFuse;
typedef struct _PhxNexus                                 PhxNexus;
typedef void  (*PhxDrawHandler)(PhxObject *, cairo_t *);
typedef bool (*PhxActionHandler)
               (PhxInterface *, xcb_generic_event_t *, PhxObject *);
typedef void  (*PhxRazeHandler)(void *);
typedef struct _PhxRGBA { double r, g, b, a; }           PhxRGBA;
typedef struct _PhxTextAttr                              PhxAttr;

/* Window controller, metes out to nexus. */
struct _PhxInterface {
 PhxObjectType      type;
 uint16_t           ncount;
 uint32_t           state;           /* flags/state, see below */
 PhxRectangle       mete_box;        /* allocate box */
 PhxRectangle       draw_box;        /* clip box */
 PhxDrawHandler     _draw_cb;        /* background, seperated expose event */
 PhxActionHandler   _event_cb;       /* actions meter */
 PhxRazeHandler     _raze_cb;        /* deconstruct internals */
  /* named 'nexus' changes on others, iface/objects */
 PhxNexus           **nexus;         /* alloted display ports / transients */
 PhxInterface       *i_mount;        /* Self */
 PhxAttr            *attrib;         /* attribute defaults */
  /* from here, differs from objects */
 PhxRectangle       min_max;         /* configure limit 'rays' */
 cairo_surface_t    *surface;
 xcb_window_t       window;
  /* from here, differs from other iface */
 cairo_surface_t    *vid_buffer;
  /* Add on to make specialized interfaces */
 void               *exclusive;
};

 /* textual defaults, name, block size */
#define FONT_NAME   "DejaVu Sans"
#define FONT_EM     16
#define FONT_VAR    ((CAIRO_FONT_SLANT_NORMAL << 8) | CAIRO_FONT_WEIGHT_NORMAL)

 /* 16 byte additional on non-textual objects */
struct _PhxTextAttr {
 PhxRGBA  bg_fill;       /* bound draw_box */
 PhxRGBA  fg_fill;       /* bound stroke path's fill */
 PhxRGBA  fg_ink;        /* stroke colour */
 double   stroke;        /* width of stroke */
  /* textual (32 bytes) */
 char     *font_name;    /* name of font */
 double   font_size;     /* point size */
 double   font_origin;   /* baseline of drawing in em space */
 uint32_t font_em;       /* line height (vertical advance), block size */
 uint32_t font_var;      /* for pango (variant:stretch:weight:style) */
                         /* cairo diff than pango(needs offsets to fit) */
                         /* values diff on basics. code set to cairo */
                         /* for cairo (0 = WEIGHT_NORMAL:SLANT_NORMAL) */
};

struct WMSizeHints { /* 18 32s */
 uint32_t flags;                           /* X has as long */
 int32_t  x, y;                            /* Obsolete */
 int32_t  width, height;                   /* Obsolete */
 int32_t  min_width, min_height;
 int32_t  max_width, max_height;
 int32_t  width_inc, height_inc;
 int32_t  min_aspect_num, min_aspect_den;  /* numerator */
 int32_t  max_aspect_num, max_aspect_den;  /* denominator */
 int32_t  base_width, base_height;
 uint32_t win_gravity;
};


extern void            ui_window_name(xcb_window_t window);
extern xcb_window_t    ui_window_create(PhxRectangle configure);
extern xcb_window_t    ui_dropdown_create(PhxRectangle configure,
                                          xcb_window_t transient_for_window);
extern void            _default_interface_raze(void *iface);
extern uint16_t        _default_interface_remove_nexus(PhxInterface *iface,
                                                       PhxNexus *nexus);

extern bool            ui_window_is_transient(xcb_window_t window);
extern void            ui_window_minimum_set(xcb_window_t window,
                                             uint16_t x, uint16_t y);
extern bool            ui_window_minimum_get(xcb_window_t window,
                                             uint16_t *x, uint16_t *y);
extern void            ui_window_maximum_set(xcb_window_t window,
                                             uint16_t x, uint16_t y);
extern bool            ui_window_maximum_get(xcb_window_t window,
                                             uint16_t *x, uint16_t *y);

extern void            ui_attributes_set(PhxObject *obj,
                                         char *font_name,
                                         int font_slant,
                                         int font_weight,
                                         int line_height);
extern void            ui_attributes_font_em_set(PhxObject *obj,
                                                 int16_t line_height);


#endif /* __SESS_WINDOWS_H__ */
