#ifndef __SESS_BANKS_H__
#define __SESS_BANKS_H__

#include "windows.h"
#include "labels.h"

typedef struct _PhxInterface               PhxBank;
typedef struct _PhxVault                   PhxVault;
typedef void  (*PhxResultHandler)(PhxBank *);

typedef uint16_t                           PhxDDLStyle;
#define DDL_DROPDOWN_MENU    ((PhxDDLStyle)0)
#define DDL_POPUP_MENU       ((PhxDDLStyle)1)
#define DDL_TOOLTIP          ((PhxDDLStyle)2)
#define DDL_COMBO            ((PhxDDLStyle)3)

struct _PhxVault {
 uint16_t         in_idx;       /* object's index under pointer */
 uint16_t         on_idx;       /* object's index of actuator's display */
 uint16_t         was_idx;      /* object's index prior selection */
 uint16_t         pad0;
 PhxRGBA  bg_fill;              /* bound draw_box */
 PhxRGBA  fg_fill;              /* bound stroke path's fill */
 PhxRGBA  fg_ink;               /* stroke colour */
 PhxObject        *actuator;    /* activates popup, can be NULL if context */
 PhxResultHandler _result_cb;   /* TRUE if display_object changes
                                  allows configure alters on popup complete */
};


extern PhxBank *        ui_bank_create(PhxObject *actuator,
                                       PhxDDLStyle window_type,
                                       PhxResultHandler result_cb);
extern void             ui_bank_add_result_cb(PhxObject *actuator,
                                              PhxResultHandler rcb);

extern PhxBank *        ui_dropdown_from(PhxObject *obj);
extern void             ui_bank_content_update(PhxBank *ibank);
extern void             ui_actuator_content_update(PhxBank *ibank);
extern void             ui_bank_insensitive_set(PhxObject *obj);

extern PhxObject *      ui_bank_object_create(PhxBank *ibank,
                                              PhxObjectType type,
                                              PhxDrawHandler draw,
                                              PhxRectangle configure);
extern PhxObjectLabel * ui_bank_label_create(PhxBank *ibank,
                                             PhxRectangle configure,
                                             char *text,
                                             uint32_t jstfy);
extern void             ui_bank_remove_object(PhxBank *ibank,
                                              uint16_t idx);

#endif /* __SESS_BANKS_H__ */
