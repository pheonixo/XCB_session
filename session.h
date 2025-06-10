#ifndef __SESS_APPLICATION_H__
#define __SESS_APPLICATION_H__

#define USE_XLIB 0
#if USE_XLIB
#include <X11/Xlib-xcb.h>   /* -lxcb -lX11 -lX11-xcb */
extern Display *display;
#else
#include <xcb/xcb.h>        /* -lxcb */
#endif

/*#include <xcb/xcb.h>*/
#include <cairo/cairo-xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
/* xcb supplemental code (not included on all OS(s)) */
#include "cursor/xcb_cursor.h"
/* Sets default cursor when in window. For window managers NULL
  normally uses "left_ptr". For no WM we set "left_ptr" for 'NULL'. */
#define CURSOR_DEFAULT ((const char*)NULL)  /* const char* */

typedef struct _PhxInterface                             PhxInterface;
typedef struct _PhxObject                                PhxObject;
typedef struct _PhxRectangle { int16_t x, y, w, h; }     PhxRectangle;
typedef struct phx_findboard_t                           phx_findboard_t;
/* mapping of state bits to avoid conflicts */
#include "statebits.h"

/* If wish to include dnd between objects and/or windows */
#define DND_INTERNAL_ON   1
#define DND_EXTERNAL_ON   1
/* above affects atom globals */
#include "atoms.h"
/* event loop attached to session, clipboard.h, and drag.h */
#include "events.h"

#if (__STDC_VERSION__ <= 199901L)
extern char *strdup(const char*);
 #define minof(a,b) \
  (((a) < (b)) ? (a) : (b))
 #define maxof(a,b) \
  (((a) > (b)) ? (a) : (b))
 #define RECTANGLE(a,b,c,d,e) \
  (a).x = b, (a).y = c, (a).w = d, (a).h = e
#else
 #define minof(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
 #define maxof(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
 #define RECTANGLE(a,b,c,d,e) \
  a = (PhxRectangle){ b, c, d, e }
#endif

#define absof(a) ((a) < 0 ? -(a) : (a))

#define DEBUG_EVENTS_ON 1
#if DEBUG_EVENTS_ON
 extern void _debug_event(xcb_generic_event_t *nvt, const char *caller);
/* #define DEBUG_EVENTS(a)    _debug_event(nvt, (a))*/
 #define DEBUG_EVENTS(a)    (void)(a)
   /* during debug, allowed to turn off/on specific events */
 extern uint64_t debug_flags;
 #define DEBUG_ASSERT(a, b) \
   if ((a)) fprintf(stderr, "%s\n", (b))
#else
 #define DEBUG_EVENTS(a)     (void)(a)
 #define DEBUG_ASSERT(a, b)  (void)(a), (void)(b)
#endif

/* globals */
#define OBJS_ALLOC  16

typedef uint16_t                                         PhxObjectType;
            /* Interface (xcb <=> windows) */
#define PHX_IFACE    ((PhxObjectType)1)
#define PHX_ITDLG    ((PhxObjectType)((1 << 8) + 1))
#define PHX_ITDDL    ((PhxObjectType)((2 << 8) + 1))
#define PHX_IBANK    ((PhxObjectType)2)       /* ITDDL with PhxDDLStyle */
#define PHX_WNDLAST  PHX_IBANK
          /* Interface (windows <=> group of (windows <=> display ports)) */
#define PHX_NFUSE    ((PhxObjectType)3)
#define PHX_GFUSE    ((PhxObjectType)((1 << 8) + 3))  /* gripped fuse */
          /* Interface (windows <=> display ports) */
#define PHX_NEXUS    ((PhxObjectType)4)
#define PHX_FPORT     ((PhxObjectType)((1 << 8) + 4))  /* nexus with objects */
#define PHX_HEADERBAR ((PhxObjectType)((2 << 8) + 4))  /* nexus with objects */
          /* Objects */
 /* These are just drawings, all same base object struct */
#define PHX_OBJECT   ((PhxObjectType)5)
#define PHX_DRAWING  ((PhxObjectType)5)
#define PHX_BUTTON   ((PhxObjectType)6)
 /* These expand base object struct */
#define PHX_LABEL    ((PhxObjectType)7)
#define PHX_TEXTVIEW ((PhxObjectType)8)
#define PHX_OLAST    PHX_TEXTVIEW

#define IS_WINDOW_TYPE(a) \
  ((((a->type) - PHX_IFACE) & (uint16_t)0x0FF) < PHX_WNDLAST)
            /* type test (interface) */
#define IS_IFACE_TYPE(a) \
  ((((a->type) - PHX_IFACE) & (uint16_t)0x0FF) < PHX_NEXUS)
            /* type test (object) */
#define OBJECT_BASE_TYPE(a)  ( ((a)->type)       & (uint16_t)0x0FF)
#define OBJECT_STYLE(a)      ((((a)->type) >> 8) & (uint16_t)0x0FF)

typedef struct _PhxSession {
 xcb_connection_t     *connection;
 xcb_keysym_t         *keysyms;        /* based off XGetKeyboardMapping() */
 xcb_cursor_context_t *cursor_ctx;     /* xcb supplemental code */
 const char           *cursor_named;   /* strdup of named cursor using */
 PhxInterface         **iface;         /* list of windows */
 PhxObject            *has_focus;      /* object receiving keyboard */
 PhxObject            *obj_within;     /* object under mouse pointer */
 PhxObject            *has_drag;       /* object that intiated a dnd grab */
 xcb_clipboard_t      *xclipboard;     /* session-wide clipboard */
 phx_findboard_t      *xfindboard;     /* session-wide findboard */
 xcb_cursor_t         cursor_default;  /* cursor id of "NULL" */
 xcb_cursor_t         cursor_id;       /* cursor id of named */
 uint8_t              sym_stride;
 uint8_t              sym_min;
 uint16_t             ncount;          /* number of windows */
 int16_t              last_event_x;
 int16_t              last_event_y;
#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
 xcb_idndserver_t     *idndserver;
#endif
#if DND_EXTERNAL_ON
 xcb_xdndserver_t     *xdndserver;
#endif
  /* addressment of WM issues. */
 PhxInterface         **stack_order;
  /* adding second bit, sync. */
#define HAS_WM   1
#define HAS_SYNC 2
 uint16_t             WMstate;
} PhxSession;

/* defines within session.c */
extern PhxSession *session;

extern bool              _obj_alloc_test(void **abstract,
                                         size_t abstract_size,
                                         uint16_t ncount);
extern xcb_keysym_t      _xcb_keysym_for(xcb_keycode_t keycode,
                                         uint16_t modifiers);
extern uint16_t          _interface_remove_for(xcb_window_t window);
extern PhxSession *      _session_create(xcb_connection_t *connection);

extern void              ui_session_shutdown(void);

extern void              ui_cursor_initialize(const char *named);
extern void              ui_cursor_set_named(const char *named,
                                             xcb_window_t window);
extern const char *      ui_cursor_get_named(void);

extern PhxInterface *    ui_interface_for(xcb_window_t window);
extern xcb_window_t      ui_window_for(PhxObject *obj);

extern PhxObject *       ui_active_focus_get(void);
extern void              ui_active_focus_set(PhxObject *obj);
extern PhxObject *       ui_active_within_get(void);
extern void              ui_active_within_set(PhxObject *obj,
                                              uint16_t state);
extern PhxObject *       ui_active_drag_get(void);
extern void              ui_active_drag_set(PhxObject *obj);

#endif /* __SESS_APPLICATION_H__ */
