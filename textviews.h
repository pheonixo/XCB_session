#ifndef __SESS_TEXTVIEWS_H__
#define __SESS_TEXTVIEWS_H__

#include "objects.h"
#include "textuals.h"

typedef PhxObject                          PhxObjectTextview;

extern void              _default_textview_raze(void *obj);
extern bool              _default_textview_meter(PhxInterface *iface,
                                                 xcb_generic_event_t *event,
                                                 PhxObject *obj);
extern PhxObjectTextview *
                         ui_textview_create(PhxNexus *nexus,
                                            PhxRectangle configure);
extern void              ui_textview_buffer_set(PhxObjectTextview *otxt,
                                                char *data,
                                                int jstfy);
void                     ui_textview_font_set(PhxObjectTextview *otxt,
                                              char *font_name);
void                     ui_textview_slant_set(PhxObjectTextview *otxt,
                                               int font_slant);
void                     ui_textview_weight_set(PhxObjectTextview *otxt,
                                                int font_weight);
void                     ui_textview_font_em_set(PhxObjectTextview *otxt,
                                            int line_height);

#endif /* __SESS_TEXTVIEWS_H__ */
