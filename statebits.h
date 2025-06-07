#ifndef __SESS_STATE_BITS_H__
#define __SESS_STATE_BITS_H__

/* first 16 bits for specifics of an entity */

/* specific PHX_IFACE only bits [0-31] */
#define SBIT_CLICKS          3            /* bits 0,1 click count */
#define SBIT_TRANSIENT       (1 << 3)     /* Special window type */
#define SBIT_MAPPED          (1 << 4)     /* Same as Object's visible */
#define SBIT_SENSITIVE       (1 << 5)     /* Alignment with objects */

#define SBIT_RELEASE_IGNORE  (1 << 7)     /* Button/key release ignore */
#define SBIT_SELECTING       (1 << 8)     /* Drag selection of content */

#define SBIT_UNDECORATED     (1 << 9)     /* For undecorated */
#define SBIT_HEADERBAR       (1 << 10)    /* For undecorated, adds nexus */
#define SBIT_HBR_DRAG        (1 << 11)    /* WM type funcs, move/resize */
#define SBIT_MAXIMIZED       (1 << 12)    /* For WM, maximized */
#define SBIT_MINIMIZED       (1 << 13)    /* For WM, minimized */
#define SBIT_NET_FRAME       (1 << 14)    /* For WM, allow move/resize */

#define SBIT_SUR_TOUCH       (1 << 15)

/* specific to type of object, bits [0-7] */
  /* nexus: configure updating */  /* other objects not to use for persistant */
#define NBIT_HORZ_TOUCH      (1 << 0)
#define NBIT_VERT_TOUCH      (1 << 1)
  /* buttons: draw instructions */
#define OBIT_BTN_PRESS       (1 << 0)
#define OBIT_BTN_FRAME       (1 << 2)
  /* textviews: location drop used to draw caret insert point */
#define OBIT_DND_CARET       (1 << 0) /* dnd caret draw */

#define OBIT_VISIBLE         (1 << 4)
#define OBIT_SENSITIVE       (1 << 5)
#define OBIT_FOCUS_ONCLICK   (1 << 6) /* Button/key release ignore */

/* specific to all objects,    bits [8-15] */
#define OBIT_DND_AWARE       (1 << 10) /* Has dnd handler, 0 == unaware */
#define OBIT_DND             (1 << 11) /* highlight, drag is within object */
#define DRAG_CURSOR_BIT      12
#define OBIT_DND_COPY        (1 << DRAG_CURSOR_BIT) /* dnd action move/copy */

#define 0BIT_SUR_TOUCH       (1 << 15)

/* XPD/JST [16-23] */
/* Applies to all, save PHX_IFACE */
#define HXPD_NIL  (0 << 16) /*16 */ /* static */
#define HXPD_LFT  (1 << 16) /*16 */ /* x moves (constant w) */
#define HXPD_RGT  (2 << 16) /*17 */ /* w moves */
#define HXPD_MSK  (3 << 16) /*17 */
#define VXPD_NIL  (0 << 18) /*18 */ /* static */
#define VXPD_TOP  (1 << 18) /*18 */ /* y moves (constant h) */
#define VXPD_BTM  (2 << 18) /*19 */ /* h moves */
#define VXPD_MSK  (3 << 18) /*19 */
/* Applies to all, save PHX_IFACE (should be textually used) */
#define HJST_CTR  (0 << 20) /*20 */
#define HJST_LFT  (1 << 20) /*20 */
#define HJST_RGT  (2 << 20) /*21 */
#define HJST_MSK  (3 << 20) /*21 */
#define VJST_CTR  (0 << 22) /*22 */
#define VJST_TOP  (1 << 22) /*22 */
#define VJST_BTM  (2 << 22) /*23 */
#define VJST_MSK  (3 << 22) /*23 */

/* Matches XCB, save shifted 24. */
/* Unique to PHX_GFUSE */
#define GRAVITY_SHIFT             24
#define GRAVITY_MASK       (15 << 24)
#define GRAVITY_NORTH_WEST  (1 << 24) /* 24 - 31 */
#define GRAVITY_NORTH       (2 << 24)
#define GRAVITY_NORTH_EAST  (3 << 24)
#define GRAVITY_WEST        (4 << 24)
#define GRAVITY_CENTER      (5 << 24)
#define GRAVITY_EAST        (6 << 24)
#define GRAVITY_SOUTH_WEST  (7 << 24)
#define GRAVITY_SOUTH       (8 << 24)
#define GRAVITY_SOUTH_EAST  (9 << 24)

/* Unique to textbuffer, in exclusive section */
#define TBIT_KEY_INSERT (1 << 0)
#define TBIT_KEY_DELETE (1 << 1)

#endif /* __SESS_STATE_BITS_H__ */
