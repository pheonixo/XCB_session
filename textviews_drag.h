#ifndef __SESS_TEXTDRAG_H__
#define __SESS_TEXTDRAG_H__

#include "textviews.h"

extern bool  _textview_drag_cancel(PhxInterface *iface,
                                   xcb_generic_event_t *nvt,
                                   PhxObject *obj);
extern bool  _textview_drag_keyboard(PhxInterface *iface,
                                     xcb_generic_event_t *nvt,
                                     PhxObject *obj);
extern bool  _textview_drag_crossing(PhxInterface *,
                                     xcb_generic_event_t *,
                                     PhxObject *);
extern bool  _textview_drag_begin(PhxInterface *,
                                  xcb_generic_event_t *,
                                  PhxObject *);
extern bool  _textview_drag_motion(PhxInterface *,
                                   xcb_generic_event_t *,
                                   PhxObject *);
extern bool  _textview_drag_finish(PhxInterface *iface,
                                   xcb_generic_event_t *nvt,
                                   PhxObject *obj);

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
extern void  _textview_selection_data_set(xcb_selection_data_t *gdata);
extern void  _textview_selection_data_get(xcb_selection_data_t *sdata);
extern void  _textview_selection_action(uint16_t);
/* drawing update for pointer position, with request for continous updates */
extern bool  _dnd_status_cb(xcb_dnd_notify_event_t *dnd);
#endif

#endif /* __SESS_TEXTDRAG_H__ */