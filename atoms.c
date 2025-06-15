#include "atoms.h"

/* Window properties, not related to a window manager. */
xcb_atom_t WM_PROTOCOLS;
xcb_atom_t WM_DELETE_WINDOW;

/* X clipboard protocol. */
xcb_atom_t CLIPBOARD;
xcb_atom_t UTF8_STRING;
xcb_atom_t XCBD_DATA;
xcb_atom_t ATOM_PAIR;
xcb_atom_t MULTIPLE;
xcb_atom_t STRING;
xcb_atom_t TARGETS;
xcb_atom_t TIMESTAMP;

/* Window Manager atoms */
xcb_atom_t WM_TAKE_FOCUS;
xcb_atom_t _NET_ACTIVE_WINDOW;
xcb_atom_t _MOTIF_WM_HINTS;
xcb_atom_t WM_CHANGE_STATE;
xcb_atom_t _NET_FRAME_EXTENTS;
char *     _NET_WM_STRING = NULL;

#if 0
   /* Window Manager atoms */
/* WM_PROTOCOLS: receive through client messages */
xcb_atom_t WM_TAKE_FOCUS;
xcb_atom_t _NET_WM_PING;
xcb_atom_t _NET_WM_SYNC_REQUEST;
/* window properties */
xcb_atom_t _MOTIF_WM_HINTS;
xcb_atom_t _NET_FRAME_EXTENTS;
/* sent to root window */
xcb_atom_t WM_CHANGE_STATE;
xcb_atom_t _NET_ACTIVE_WINDOW;
xcb_atom_t _NET_WM_MOVERESIZE;
xcb_atom_t _NET_WM_STATE;
xcb_atom_t _NET_WM_STATE_MAXIMIZED_HORZ;
xcb_atom_t _NET_WM_STATE_MAXIMIZED_VERT;
/* Window manager name for overriding bad or inactive protocols. */
char *     _NET_WM_STRING = NULL;
#endif

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
/* X drag n drop protocol. */
xcb_atom_t XDND_ACTIONASK;
xcb_atom_t XDND_ACTIONCOPY;
xcb_atom_t XDND_ACTIONLINK;
xcb_atom_t XDND_ACTIONLIST;
xcb_atom_t XDND_ACTIONMOVE;
xcb_atom_t XDND_ACTIONPRIVATE;
xcb_atom_t XDND_AWARE;
xcb_atom_t XDND_DROP;
xcb_atom_t XDND_ENTER;
xcb_atom_t XDND_FINISHED;
xcb_atom_t XDND_LEAVE;
xcb_atom_t XDND_POSITION;
xcb_atom_t XDND_SELECTION;
xcb_atom_t XDND_STATUS;
xcb_atom_t XDND_TYPELIST;
#endif

struct _acr {
 char *name;
 uint16_t nsz;
 xcb_intern_atom_cookie_t cookie;
 xcb_intern_atom_reply_t *reply;
};

/* These we do not create an atom if doesn't exist. */
static void
_atoms_window_managers(xcb_connection_t *connection) {

  const uint16_t nAtoms = 7;
  xcb_atom_t _NET_SUPPORTING_WM_CHECK;
  xcb_atom_t _NET_WM_NAME;

  uint16_t adx;
  struct _acr atoms[7] = {
    { "WM_TAKE_FOCUS",            13, {0}, NULL },  /* currently unused */
    { "_NET_ACTIVE_WINDOW",       18, {0}, NULL },
    { "_MOTIF_WM_HINTS",          15, {0}, NULL },
    { "WM_CHANGE_STATE",          15, {0}, NULL },
    { "_NET_FRAME_EXTENTS",       18, {0}, NULL },
    { "_NET_SUPPORTING_WM_CHECK", 24, {0}, NULL },
    { "_NET_WM_NAME",             12, {0}, NULL }
  };

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].cookie = xcb_intern_atom(connection, 1,
                                        atoms[adx].nsz, atoms[adx].name);
  xcb_flush(connection);

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].reply = xcb_intern_atom_reply(connection,
                                             atoms[adx].cookie, NULL);
  xcb_flush(connection);

  WM_TAKE_FOCUS      = atoms[ 0].reply->atom;  free(atoms[ 0].reply);
  _NET_ACTIVE_WINDOW = atoms[ 1].reply->atom;  free(atoms[ 1].reply);
  _MOTIF_WM_HINTS    = atoms[ 2].reply->atom;  free(atoms[ 2].reply);
  WM_CHANGE_STATE    = atoms[ 3].reply->atom;  free(atoms[ 3].reply);
  _NET_FRAME_EXTENTS = atoms[ 4].reply->atom;  free(atoms[ 4].reply);

  _NET_SUPPORTING_WM_CHECK  = atoms[ 5].reply->atom;  free(atoms[5].reply);
  _NET_WM_NAME              = atoms[ 6].reply->atom;  free(atoms[6].reply);

  if ( (_NET_SUPPORTING_WM_CHECK != XCB_ATOM_NONE)
      && (_NET_WM_NAME != XCB_ATOM_NONE) ) {
    xcb_screen_t *screen
      = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    xcb_get_property_cookie_t c0
      = xcb_get_property(connection, 0, screen->root,
                         _NET_SUPPORTING_WM_CHECK,
                         XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX/4);
    xcb_get_property_reply_t *r0
      = xcb_get_property_reply(connection, c0, NULL);
    if (r0 != NULL) {
      if (r0->value_len > 0) {
        xcb_window_t cWindow = *((xcb_window_t*)xcb_get_property_value(r0));
        xcb_get_property_cookie_t c1
          = xcb_get_property(connection, 0, cWindow, _NET_WM_NAME,
                          XCB_GET_PROPERTY_TYPE_ANY, 0, UINT_MAX/4);
        xcb_get_property_reply_t *r1
          = xcb_get_property_reply(connection, c1, NULL);
        if (r1 != NULL) {
          if ( (r1->value_len > 0)
               && ( (r1->type == XCB_ATOM_STRING)
                   || (r1->type == UTF8_STRING) )  )
            _NET_WM_STRING = strdup((char *)xcb_get_property_value(r1));
          free(r1);
        }
      }
      free(r0);
    }
  }
}

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
static void
_atoms_drag_n_drop(xcb_connection_t *connection) {

  const uint16_t nAtoms = 15;
  uint16_t adx;
  struct _acr  atoms[15] = {
    { "XdndAware",       9, {0}, NULL },
    { "XdndDrop",        8, {0}, NULL },
    { "XdndEnter",       9, {0}, NULL },
    { "XdndFinished",   12, {0}, NULL },
    { "XdndLeave",       9, {0}, NULL },
    { "XdndPosition",   12, {0}, NULL },
    { "XdndSelection",  13, {0}, NULL },
    { "XdndStatus",     10, {0}, NULL },
    { "XdndTypeList",   12, {0}, NULL },
    { "XdndActionAsk",     13, {0}, NULL },
    { "XdndActionCopy",    14, {0}, NULL },
    { "XdndActionLink",    14, {0}, NULL },
    { "XdndActionList",    14, {0}, NULL },
    { "XdndActionMove",    14, {0}, NULL },
    { "XdndActionPrivate", 17, {0}, NULL }
  };

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].cookie = xcb_intern_atom(connection, 0,
                                        atoms[adx].nsz, atoms[adx].name);
  xcb_flush(connection);

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].reply = xcb_intern_atom_reply(connection,
                                             atoms[adx].cookie, NULL);
  xcb_flush(connection);

  XDND_AWARE         = atoms[ 0].reply->atom;  free(atoms[ 0].reply);
  XDND_DROP          = atoms[ 1].reply->atom;  free(atoms[ 1].reply);
  XDND_ENTER         = atoms[ 2].reply->atom;  free(atoms[ 2].reply);
  XDND_FINISHED      = atoms[ 3].reply->atom;  free(atoms[ 3].reply);
  XDND_LEAVE         = atoms[ 4].reply->atom;  free(atoms[ 4].reply);
  XDND_POSITION      = atoms[ 5].reply->atom;  free(atoms[ 5].reply);
  XDND_SELECTION     = atoms[ 6].reply->atom;  free(atoms[ 6].reply);
  XDND_STATUS        = atoms[ 7].reply->atom;  free(atoms[ 7].reply);
  XDND_TYPELIST      = atoms[ 8].reply->atom;  free(atoms[ 8].reply);
  XDND_ACTIONASK     = atoms[ 9].reply->atom;  free(atoms[ 9].reply);
  XDND_ACTIONCOPY    = atoms[10].reply->atom;  free(atoms[10].reply);
  XDND_ACTIONLINK    = atoms[11].reply->atom;  free(atoms[11].reply);
  XDND_ACTIONLIST    = atoms[12].reply->atom;  free(atoms[12].reply);
  XDND_ACTIONMOVE    = atoms[13].reply->atom;  free(atoms[13].reply);
  XDND_ACTIONPRIVATE = atoms[14].reply->atom;  free(atoms[14].reply);
}
#endif

static void
_atoms_clipboard(xcb_connection_t *connection) {

  const uint16_t nAtoms = 8;
  uint16_t adx;
  struct _acr  atoms[8] = {
    { "CLIPBOARD", 9, {0}, NULL }, { "UTF8_STRING", 11, {0}, NULL },
    { "XCBD_DATA", 9, {0}, NULL }, { "ATOM_PAIR",    9, {0}, NULL },
    { "MULTIPLE",  8, {0}, NULL }, { "STRING",       6, {0}, NULL },
    { "TARGETS",   7, {0}, NULL }, { "TIMESTAMP",    9, {0}, NULL }
  };

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].cookie = xcb_intern_atom(connection, 0,
                                        atoms[adx].nsz, atoms[adx].name);
  xcb_flush(connection);

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].reply = xcb_intern_atom_reply(connection,
                                             atoms[adx].cookie, NULL);
  xcb_flush(connection);

  CLIPBOARD       = atoms[0].reply->atom;  free(atoms[0].reply);
  UTF8_STRING     = atoms[1].reply->atom;  free(atoms[1].reply);
  XCBD_DATA       = atoms[2].reply->atom;  free(atoms[2].reply);
  ATOM_PAIR       = atoms[3].reply->atom;  free(atoms[3].reply);
  MULTIPLE        = atoms[4].reply->atom;  free(atoms[4].reply);
  STRING          = atoms[5].reply->atom;  free(atoms[5].reply);
  TARGETS         = atoms[6].reply->atom;  free(atoms[6].reply);
  TIMESTAMP       = atoms[7].reply->atom;  free(atoms[7].reply);
}

static void
_atoms_basic(xcb_connection_t *connection) {

  const uint16_t nAtoms = 2;
  uint16_t adx;
  struct _acr  atoms[2] = {
    { "WM_PROTOCOLS",     12, {0}, NULL },
    { "WM_DELETE_WINDOW", 16, {0}, NULL }
  };

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].cookie = xcb_intern_atom(connection, 0,
                                        atoms[adx].nsz, atoms[adx].name);
  xcb_flush(connection);

  for (adx = 0; adx < nAtoms; adx++)
    atoms[adx].reply = xcb_intern_atom_reply(connection,
                                             atoms[adx].cookie, NULL);
  xcb_flush(connection);

  WM_PROTOCOLS    = atoms[ 0].reply->atom;  free(atoms[ 0].reply);
  WM_DELETE_WINDOW = atoms[1].reply->atom;  free(atoms[ 1].reply);
}

void
ui_atoms_initialize(xcb_connection_t *connection) {

  xcb_screen_t *screen;
  xcb_get_window_attributes_cookie_t c0;
  xcb_get_window_attributes_reply_t *r0;

  screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    /* Test if a window manager exists. Should application be used
      as window manager, screen->root gets altered prior to here. */
  c0 = xcb_get_window_attributes(connection, screen->root);
  r0 = xcb_get_window_attributes_reply(connection, c0, NULL);
    /* HAS_WM is bit 0 */
  session->WMstate |=
       !!(r0->all_event_masks & XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT);
  free(r0);

  _atoms_basic(connection);
  _atoms_clipboard(connection);
#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
  _atoms_drag_n_drop(connection);
#endif
  if (!!(session->WMstate & HAS_WM))
    _atoms_window_managers(connection);
}
