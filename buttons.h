#ifndef __SESS_BUTTONS_H__
#define __SESS_BUTTONS_H__

#include "objects.h"
#include "draw.h"
#include "labels.h"

typedef PhxObject                          PhxObjectButton;

 /* specific to buttons */
#define SBIT_BTN_FRAME   (0x00000001 << 14) /*14 */
#define SBIT_BTN_BG      (0x00000001 << 15) /*15 */

typedef uint16_t                           PhxButtonStyle;
#define BTN_ROUND_CORNER    ((PhxButtonStyle)0)
#define BTN_NAVIGATE_LEFT   ((PhxButtonStyle)1)
#define BTN_NAVIGATE_RIGHT  ((PhxButtonStyle)2)
#define BTN_NAVIGATE_UP     ((PhxButtonStyle)3)
#define BTN_NAVIGATE_DOWN   ((PhxButtonStyle)4)
#define BTN_STADIUM         ((PhxButtonStyle)5)
#define BTN_COMBO_ARROW     ((PhxButtonStyle)6)
#define BTN_COMBO_WHEEL     ((PhxButtonStyle)7)


extern void              frame_draw_set(PhxObjectButton *obtn,
                                        bool draws);
extern bool              frame_draw_get(PhxObjectButton *obtn);
extern void              frame_remove(PhxObjectButton *obtn);
extern void              frame_define(PhxObjectButton *obtn,
                                      PhxRectangle dbox);

extern void              _default_button_raze(void *obtn);
extern bool              _default_button_meter(PhxInterface *iface,
                                               xcb_generic_event_t *event,
                                               PhxObject *obj);
extern PhxObjectButton * ui_button_create(PhxNexus *nexus,
                                          PhxButtonStyle style,
                                          PhxRectangle configure);
extern void              _button_draw_area_request(PhxObjectButton *obtn,
                                                   PhxRectangle *dbox);
extern void              ui_draw_button(PhxObject *b, cairo_t *cr);
extern PhxObjectLabel *  ui_button_label_create(PhxObjectButton *obtn,
                                                char *str,
                                                uint32_t jstfy);
extern PhxObject *       ui_button_object_create(PhxObjectButton *obtn,
                                                 PhxObjectType type,
                                                 PhxDrawHandler draw);

#endif /* __SESS_BUTTONS_H__ */
