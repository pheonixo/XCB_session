#ifndef __SESS_ATOMS_H__
#define __SESS_ATOMS_H__

#include "session.h"

/* Used by events.c, windows.c */
extern xcb_atom_t WM_PROTOCOLS;
extern xcb_atom_t WM_DELETE_WINDOW;
extern xcb_atom_t WM_TAKE_FOCUS;

 /* Used by clipboard.c, events.c, and texuals.c */
extern xcb_atom_t CLIPBOARD;
 /* Used by clipboard.c, textviews.c */
extern xcb_atom_t UTF8_STRING;
extern xcb_atom_t XCBD_DATA;  /* Note: probably should be CLIPBOARD */

 /* Used by clipboard.c */
extern xcb_atom_t ATOM_PAIR;
extern xcb_atom_t MULTIPLE;           /* ICCCM Required */
extern xcb_atom_t STRING;
extern xcb_atom_t TARGETS;            /* ICCCM Required */
extern xcb_atom_t TIMESTAMP;          /* ICCCM Required */

extern xcb_atom_t _NET_ACTIVE_WINDOW;
extern xcb_atom_t _MOTIF_WM_HINTS;
extern char *     _NET_WM_STRING;     /* Used to determine features. */

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
 /* Used by dndx.c, some used by dndi.c */
 /* dndi.c includes textual.c usage. */
extern xcb_atom_t XDND_ACTIONASK;
extern xcb_atom_t XDND_ACTIONCOPY;
extern xcb_atom_t XDND_ACTIONLINK;
extern xcb_atom_t XDND_ACTIONLIST;
extern xcb_atom_t XDND_ACTIONMOVE;
extern xcb_atom_t XDND_ACTIONPRIVATE;
extern xcb_atom_t XDND_AWARE;
extern xcb_atom_t XDND_DROP;
extern xcb_atom_t XDND_ENTER;
extern xcb_atom_t XDND_FINISHED;
extern xcb_atom_t XDND_LEAVE;
extern xcb_atom_t XDND_POSITION;
extern xcb_atom_t XDND_SELECTION;
extern xcb_atom_t XDND_STATUS;
extern xcb_atom_t XDND_TYPELIST;
#endif

 /* Used by sessions.c */
extern void  ui_atoms_initialize(xcb_connection_t *);

#endif /* __SESS_ATOMS_H__ */
