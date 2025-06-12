#include "atoms.h"

xcb_atom_t WM_PROTOCOLS;
xcb_atom_t WM_DELETE_WINDOW;

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

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
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

/* These we do not create an atom if doesn't exist. */
static void
_atoms_window_managers(xcb_connection_t *connection) {

  const uint16_t nAtoms = 7;
  xcb_atom_t _NET_SUPPORTING_WM_CHECK;
  xcb_atom_t _NET_WM_NAME;

  uint16_t adx;
  struct _acr {
    char *name;
    uint16_t nsz;
    xcb_intern_atom_cookie_t cookie;
    xcb_intern_atom_reply_t *reply;
  } atoms[7] = {
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

void
ui_atoms_initialize(xcb_connection_t *connection) {

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
  const uint16_t nAtoms = 25;
  const uint16_t nBase  = 10;
#else
  const uint16_t nAtoms = 10;
#endif

    /* for ISO C90 use max array size */
  uint16_t adx;
  struct _acr {
    char *name;
    uint16_t nsz;
    xcb_intern_atom_cookie_t cookie;
    xcb_intern_atom_reply_t *reply;
  } atoms[25] = {

    { "WM_PROTOCOLS",     12, {0}, NULL },
    { "WM_DELETE_WINDOW", 16, {0}, NULL },

    { "CLIPBOARD", 9, {0}, NULL }, { "UTF8_STRING", 11, {0}, NULL },
    { "XCBD_DATA", 9, {0}, NULL }, { "ATOM_PAIR",    9, {0}, NULL },
    { "MULTIPLE",  8, {0}, NULL }, { "STRING",       6, {0}, NULL },
    { "TARGETS",   7, {0}, NULL }, { "TIMESTAMP",    9, {0}, NULL },

    { "XdndActionAsk",     13, {0}, NULL },
    { "XdndActionCopy",    14, {0}, NULL },
    { "XdndActionLink",    14, {0}, NULL },
    { "XdndActionList",    14, {0}, NULL },
    { "XdndActionMove",    14, {0}, NULL },
    { "XdndActionPrivate", 17, {0}, NULL },

    { "XdndAware",       9, {0}, NULL },
    { "XdndDrop",        8, {0}, NULL },
    { "XdndEnter",       9, {0}, NULL },
    { "XdndFinished",   12, {0}, NULL },
    { "XdndLeave",       9, {0}, NULL },
    { "XdndPosition",   12, {0}, NULL },
    { "XdndSelection",  13, {0}, NULL },
    { "XdndStatus",     10, {0}, NULL },
    { "XdndTypeList",   12, {0}, NULL }
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
  CLIPBOARD       = atoms[ 2].reply->atom;  free(atoms[ 2].reply);
  UTF8_STRING     = atoms[ 3].reply->atom;  free(atoms[ 3].reply);
  XCBD_DATA       = atoms[ 4].reply->atom;  free(atoms[ 4].reply);
  ATOM_PAIR       = atoms[ 5].reply->atom;  free(atoms[ 5].reply);
  MULTIPLE        = atoms[ 6].reply->atom;  free(atoms[ 6].reply);
  STRING          = atoms[ 7].reply->atom;  free(atoms[ 7].reply);
  TARGETS         = atoms[ 8].reply->atom;  free(atoms[ 8].reply);
  TIMESTAMP       = atoms[ 9].reply->atom;  free(atoms[ 9].reply);

    /* uses above atoms */
  _atoms_window_managers(connection);

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
  XDND_ACTIONASK     = atoms[(nBase +  0)].reply->atom;
                  free(atoms[(nBase +  0)].reply);
  XDND_ACTIONCOPY    = atoms[(nBase +  1)].reply->atom;
                  free(atoms[(nBase +  1)].reply);
  XDND_ACTIONLINK    = atoms[(nBase +  2)].reply->atom;
                  free(atoms[(nBase +  2)].reply);
  XDND_ACTIONLIST    = atoms[(nBase +  3)].reply->atom;
                  free(atoms[(nBase +  3)].reply);
  XDND_ACTIONMOVE    = atoms[(nBase +  4)].reply->atom;
                  free(atoms[(nBase +  4)].reply);
  XDND_ACTIONPRIVATE = atoms[(nBase +  5)].reply->atom;
                  free(atoms[(nBase +  5)].reply);
  XDND_AWARE         = atoms[(nBase +  6)].reply->atom;
                  free(atoms[(nBase +  6)].reply);
  XDND_DROP          = atoms[(nBase +  7)].reply->atom;
                  free(atoms[(nBase +  7)].reply);
  XDND_ENTER         = atoms[(nBase +  8)].reply->atom;
                  free(atoms[(nBase +  8)].reply);
  XDND_FINISHED      = atoms[(nBase +  9)].reply->atom;
                  free(atoms[(nBase +  9)].reply);
  XDND_LEAVE         = atoms[(nBase + 10)].reply->atom;
                  free(atoms[(nBase + 10)].reply);
  XDND_POSITION      = atoms[(nBase + 11)].reply->atom;
                  free(atoms[(nBase + 11)].reply);
  XDND_SELECTION     = atoms[(nBase + 12)].reply->atom;
                  free(atoms[(nBase + 12)].reply);
  XDND_STATUS        = atoms[(nBase + 13)].reply->atom;
                  free(atoms[(nBase + 13)].reply);
  XDND_TYPELIST      = atoms[(nBase + 14)].reply->atom;
                  free(atoms[(nBase + 14)].reply);
#endif
}
