#ifndef __SESS_DRAG_H__
#define __SESS_DRAG_H__

/* Need before session.h loaded */
typedef struct xcb_xdndserver_t  xcb_xdndserver_t;
typedef struct xcb_dnd_notify_event_t xcb_dnd_notify_event_t;
typedef struct xcb_dnd_notify_event_t xcb_idndserver_t;

#include "session.h"

 /* These are the basics for dnd, the handling when within an object. */
bool  _drag_selection_box(PhxInterface *, xcb_generic_event_t *);
bool  _drag_motion(PhxInterface *, xcb_generic_event_t *, PhxObject *);
bool  _drag_finish(PhxInterface *, xcb_generic_event_t *, PhxObject *);
bool  _drag_keyboard(PhxInterface *, xcb_generic_event_t *);

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)

/* Kind of the 'client_message' for inter-window dnd. */
struct xcb_dnd_notify_event_t {
  PhxObject       *owner;
  uint16_t        owner_state;     /* (uint16_t)xcb_key_but_mask_t */
  uint8_t         owner_actions;
  PhxObject       *within;
  int16_t         within_x, within_y;
  int16_t         within_w, within_h;
  xcb_atom_t      within_type;
};

/* To use internal server, API */
xcb_idndserver_t *    _dnd_initialize(void);
bool  _dnd_status(xcb_dnd_notify_event_t *);             /*_drag_motion*/
void  _dnd_drop(xcb_dnd_notify_event_t *);               /*_drag_finish*/
bool  _send_dnd_event(xcb_idndserver_t *, xcb_generic_event_t *, xcb_atom_t);

/* Typed struct used by clipboard and xdnd. */
#ifndef XCBSELDATA
 #define XCBSELDATA
 typedef struct {
  xcb_atom_t type;        /* type of data */
  uint32_t   dsz;         /* byte size of data */
  uint8_t    *data;
 } xcb_selection_data_t;
#endif

/* An object's connection to a dnd event, as source and target.
  'src_actions': upper byte holds actions understood
                 lower byte has state of keys to select action desired.
  'watypes_count': we accept types count for list.
  'watypes': we accept types list.
  '..._cb': function pointers to actions.
  Notes: we accept types can take an address of an atom. The 'raze' handler
  checks _count to identify if free() is used. If an object accepts more
  than one, an object is to malloc() 'watypes' with 'watypes_count'
  containing the number of xcb_atom_t. */
typedef struct xcb_dnd_property_t {
  uint16_t   src_actions;
  uint8_t    watypes_count;
  xcb_atom_t *watypes;
    /* selection data retreival/placement from/to object for drop. */
  void       (*_sel_get_cb)(xcb_selection_data_t *);
  void       (*_sel_set_cb)(xcb_selection_data_t *);
    /* selection actions source performs after drop. */
  void       (*_sel_act_cb)(uint16_t);
} xcb_dnd_property_t;

#endif /* (DND_INTERNAL_ON || DND_EXTERNAL_ON) */

#if DND_EXTERNAL_ON  /* These are for window to window transfers. XDND */

typedef struct {
  uint16_t         source_state;     /* (uint16_t)xcb_key_but_mask_t */
  uint8_t          so_count;
  uint8_t          sa_count;
  xcb_window_t     source;
  xcb_atom_t       *source_offer;    /* type of data source can provide */
  xcb_atom_t       *source_action;   /* type of actions source can perform */
  xcb_client_message_data_t  src_entermsg;
} XDNDSorceData;

typedef struct {
  uint16_t          state;
  xcb_window_t      target;
  xcb_atom_t        acceptedType;     /* target wants this from provided */
  xcb_timestamp_t   xdndLastPositionTimestamp; /* required, and unused. */
    /* Used for XdndStatus 'do not send until you leave rectangle' */
  int16_t           x0, y0, x1, y1;
} XDNDStateMachine;

struct xcb_xdndserver_t {
                   /* application */
  xcb_connection_t *connection;
  void (*_xdnd_status_cb)(xcb_window_t, xcb_client_message_data_t *);
                   /* running */
    /* source callbacks */
  void (*xdnddata_in)(xcb_selection_data_t *);
  void (*xdnddata_act)(uint16_t);
    /* target callback */
  void (*xdnddata_out)(xcb_selection_data_t *);
    /* dnd data */
  xcb_selection_data_t        xdnddata;
  XDNDSorceData               xdndSource;
  XDNDStateMachine            xdndState;
};

 /* Needed internal events */
 /* in xcb_main() of events.c,         code in drag.c (DNDX section) */
extern xcb_xdndserver_t *
              xdnd_initialize(xcb_connection_t *,
      void (*_xdnd_status_cb)(xcb_window_t, xcb_client_message_data_t *));
 /* input of xdnd_initialize(),        code in drag.c (DNDI section) */
extern void   xdnd_status_event(xcb_window_t, xcb_client_message_data_t *);
 /* in _window_create() of windows.c,  code in drag.c (DNDX section) */
extern void   xdnd_window_awareness(xcb_connection_t *, xcb_window_t);
 /* in _drag_motion() of drag.c,       code in drag.c (DNDX section) */
extern bool   xdnd_quirk_src_load(xcb_xdndserver_t *,
                                  xcb_window_t, xcb_dnd_property_t *);
 /* in xdnd_status_event() of drag.c,  code in drag.c (DNDX section) */
 /* in _textview_drag_crossing() of textviews_drag.c                 */
extern bool   xdnd_quirk_dst_load(xcb_xdndserver_t *,
                                  xcb_window_t, xcb_dnd_property_t *);
 /* in _event_selection() of events.c, code in drag.c (DNDI section) */
extern bool   _dnd_selection_event(xcb_generic_event_t *);

     /* Basic XDND events, motion, button, key (As drag source) */

 /* in _drag_motion() of drag.c (DNDI section),
                                       code in drag.c (DNDX section) */
extern bool   xdnd_drag_motion(xcb_xdndserver_t *,
                               xcb_motion_notify_event_t *);
 /* in _drag_finish() of drag.c (DNDI section),
                                       code in drag.c (DNDX section)  */
extern bool   xdnd_drag_drop(xcb_xdndserver_t *);
 /* in _event_keyboard() of events.c,  code in drag.c (DNDX section) */
extern void   xdnd_drag_cancel(xcb_xdndserver_t *);

     /* Selection/XDND messaging system of events */

 /* in _dnd_selection_event() of drag.c (DNDI section),
                                       code in drag.c (DNDX section) */
extern void   xdnd_selection_clear(xcb_xdndserver_t *);
 /* in _dnd_selection_event() of drag.c (DNDI section) */
extern void   xdnd_process_selection(xcb_xdndserver_t *,
                                     xcb_selection_request_event_t *);
 /* in _dnd_selection_event() of drag.c (DNDI section) */
extern bool   xdnd_process_message(xcb_xdndserver_t *,
                                   xcb_client_message_event_t *);

     /* Accessor of xdndState's state */
extern bool   xdndActivated_get(xcb_xdndserver_t *);

#endif /* DND_EXTERNAL_ON */

#endif /* __SESS_DRAG_H__ */
