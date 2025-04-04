#ifndef __SESS_LABELS_H__
#define __SESS_LABELS_H__

#include "objects.h"
#include "textuals.h"

typedef PhxObject                          PhxObjectLabel;
typedef struct phx_labelbuffer             PhxLabelbuffer;

struct phx_labelbuffer {
 char             *string;           /* c-str pointer (text buffer) */
 uint16_t         str_nil;           /* c-str pointer (text buffer) */
 PhxRectangle     adv_box;           /* extents of drawn string */
};

extern PhxObjectLabel * ui_label_create(PhxNexus *nexus,
                                        PhxRectangle configure,
                                        char *text,
                                        uint32_t jstfy);
extern void              _default_label_raze(void *obj);
extern bool              _default_label_meter(PhxInterface *iface,
                                              xcb_generic_event_t *event,
                                              PhxObject *obj);
extern void              ui_label_text_set(PhxObjectLabel *olbl,
                                           char *str);
extern void              ui_label_font_em_set(PhxObjectLabel *obj,
                                              int16_t line_height);
extern void              _label_text_max_fit(PhxObjectLabel *olbl);


#endif /* __SESS_LABELS_H__ */
