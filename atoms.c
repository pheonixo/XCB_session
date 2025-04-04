#include "atoms.h"

xcb_atom_t WM_PROTOCOLS;
xcb_atom_t WM_DELETE_WINDOW;
xcb_atom_t WM_TAKE_FOCUS;

xcb_atom_t CLIPBOARD;
xcb_atom_t UTF8_STRING;
xcb_atom_t XCBD_DATA;
xcb_atom_t ATOM_PAIR;
xcb_atom_t MULTIPLE;
xcb_atom_t STRING;
xcb_atom_t TARGETS;
xcb_atom_t TIMESTAMP;

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

void
ui_atoms_initialize(xcb_connection_t *connection) {

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
  const uint16_t nAtoms = 26;
#else
  const uint16_t nAtoms = 11;
#endif

    /* for ISO C90 use max array size */
  uint16_t adx;
  struct _acr {
    char *name;
    uint16_t nsz;
    xcb_intern_atom_cookie_t cookie;
    xcb_intern_atom_reply_t *reply;
  } atoms[26] = {

    { "WM_PROTOCOLS",     12, {0}, NULL },
    { "WM_DELETE_WINDOW", 16, {0}, NULL },
    { "WM_TAKE_FOCUS",    13, {0}, NULL },

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

  WM_PROTOCOLS     = atoms[0].reply->atom;  free(atoms[ 0].reply);
  WM_DELETE_WINDOW = atoms[1].reply->atom;  free(atoms[ 1].reply);
  WM_TAKE_FOCUS    = atoms[2].reply->atom;  free(atoms[ 2].reply);
  CLIPBOARD       = atoms[ 3].reply->atom;  free(atoms[ 3].reply);
  UTF8_STRING     = atoms[ 4].reply->atom;  free(atoms[ 4].reply);
  XCBD_DATA       = atoms[ 5].reply->atom;  free(atoms[ 5].reply);
  ATOM_PAIR       = atoms[ 6].reply->atom;  free(atoms[ 6].reply);
  MULTIPLE        = atoms[ 7].reply->atom;  free(atoms[ 7].reply);
  STRING          = atoms[ 8].reply->atom;  free(atoms[ 8].reply);
  TARGETS         = atoms[ 9].reply->atom;  free(atoms[ 9].reply);
  TIMESTAMP       = atoms[10].reply->atom;  free(atoms[10].reply);

#if (DND_INTERNAL_ON || DND_EXTERNAL_ON)
  XDND_ACTIONASK     = atoms[11].reply->atom;  free(atoms[11].reply);
  XDND_ACTIONCOPY    = atoms[12].reply->atom;  free(atoms[12].reply);
  XDND_ACTIONLINK    = atoms[13].reply->atom;  free(atoms[13].reply);
  XDND_ACTIONLIST    = atoms[14].reply->atom;  free(atoms[14].reply);
  XDND_ACTIONMOVE    = atoms[15].reply->atom;  free(atoms[15].reply);
  XDND_ACTIONPRIVATE = atoms[16].reply->atom;  free(atoms[16].reply);
  XDND_AWARE         = atoms[17].reply->atom;  free(atoms[17].reply);
  XDND_DROP          = atoms[18].reply->atom;  free(atoms[18].reply);
  XDND_ENTER         = atoms[19].reply->atom;  free(atoms[19].reply);
  XDND_FINISHED      = atoms[20].reply->atom;  free(atoms[20].reply);
  XDND_LEAVE         = atoms[21].reply->atom;  free(atoms[21].reply);
  XDND_POSITION      = atoms[22].reply->atom;  free(atoms[22].reply);
  XDND_SELECTION     = atoms[23].reply->atom;  free(atoms[23].reply);
  XDND_STATUS        = atoms[24].reply->atom;  free(atoms[24].reply);
  XDND_TYPELIST      = atoms[25].reply->atom;  free(atoms[25].reply);
#endif
}