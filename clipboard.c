#include "clipboard.h"
#include "atoms.h"

static xcb_connection_t *connection;

#define XCLB_ASSERT  DEBUG_ASSERT

/*
  Moved into atoms.h atoms.c. Also see xclb_initialize() change.
static xcb_atom_t ATOM_PAIR;
       xcb_atom_t CLIPBOARD;
static xcb_atom_t MULTIPLE;
static xcb_atom_t STRING;
static xcb_atom_t TARGETS;
static xcb_atom_t TIMESTAMP;
static xcb_atom_t UTF8_STRING;
       xcb_atom_t XCBD_DATA;
*/

/* if owner of clipboard, setting xcb_selection_data_t makes
   content available immediately, but timestamp maybe off.
   If not the owner, setting xcb_selection_data_t makes
   content available immediately, but timestamp will set correctly
   when available clipboard. */


#pragma mark *** Clear ***

/* This is called by xcb_main() in response to:
   XCB_SELECTION_CLEAR with a selection == CLIPBOARD */
void
_xclb_process_clear(xcb_clipboard_t *clbd) {

  xcb_selection_data_t *seldata = &clbd->xseldata;

  XCLB_ASSERT(true, "_xclb_process_clear()");

  if (seldata->data != NULL)
    free(seldata->data);
  memset(clbd, 0, sizeof(xcb_clipboard_t));
}

#pragma mark *** Copy ***

/* This is a two part process of 'copy to clipboard'.
   This is called by xcb_main() in response to:
   XCB_PROPERTY_NOTIFY with an atom == CLIPBOARD.
   XCB_PROPERTY_NOTIFY was a response to xcb_clipboard_copy_ini()
   which is done like this for ICCCM compliance. */
void
_xclb_set_ownership(xcb_clipboard_t *clbd,
                    xcb_window_t owner,
                    xcb_timestamp_t time) {

  XCLB_ASSERT(true, "_xclb_set_ownership()");

  if (clbd->xseldata.dsz == 0) {
    puts("Ownership of clipboard refused! Must have content for clipboard.");
    return;
  }

  xcb_set_selection_owner(connection, owner, CLIPBOARD, time);

  clbd->owner     = owner;
  clbd->time      = time;
  clbd->selection = CLIPBOARD;
  clbd->target    = TARGETS;
  clbd->property  = XCB_NONE;
}

void
_xclb_copy(xcb_clipboard_t *clbd, xcb_window_t requestor, char *src, int sz) {

  xcb_selection_data_t *seldata = &clbd->xseldata;

  XCLB_ASSERT(true, "_xclb_copy()");

    /* How to get time for clipboard, ICCCM compliance */
    /* Reguardless, if already owned, ICCCM says reassert on data changes */
  xcb_change_property(connection, XCB_PROP_MODE_APPEND, requestor,
                    CLIPBOARD, XCB_ATOM_ATOM, 32, 0, NULL);

  seldata->type = STRING;
  if (seldata->data != NULL)
    free(seldata->data);
  seldata->dsz = sz;
  seldata->data = malloc(seldata->dsz + 1);
  memmove(seldata->data, src, sz);
  seldata->data[sz] = 0;
}

#pragma mark *** Paste ***

/* Ask for clipboard contents. */
void
_xclb_paste_request(xcb_clipboard_t *clbd, xcb_window_t requestor) {

  XCLB_ASSERT(true, "_xclb_paste_request()");

  clbd->owner     = requestor;
  clbd->selection = CLIPBOARD;
  clbd->target    = STRING;
  clbd->property  = XCBD_DATA;

    /* Send request for contents to owner of clipboard. */
    /* When owner of clipboard is ready to send data, we
       receive a selection notify event. */
  xcb_convert_selection(connection,
                        requestor,
                        clbd->selection,
                        clbd->target,
                        clbd->property,
                        XCB_CURRENT_TIME);
  xcb_flush(connection);
}

static xcb_selection_data_t *
_xclb_get_reply(xcb_connection_t *connection,
                        xcb_clipboard_t *clbd) {

  xcb_get_property_cookie_t c0;
  xcb_get_property_reply_t *r0;
  xcb_selection_data_t *clip_data;

  xcb_selection_data_t *seldata = &clbd->xseldata;
  if (seldata->data != NULL) {
    free(seldata->data);
    seldata->data = NULL;
    seldata->dsz = 0;
  }

  XCLB_ASSERT(true, "_xclb_get_reply()");

  c0 = xcb_get_property(connection, 0,
                            clbd->owner, clbd->property, clbd->target,
                            0, UINT_MAX/4);
  r0  = xcb_get_property_reply(connection, c0, NULL);
  clip_data = NULL;
  if (r0 != NULL) {
    clip_data = seldata;
    clip_data->type = clbd->target;
    clip_data->dsz = xcb_get_property_value_length(r0);
      /* Appears that 'STRING' are non-nil c-strings, just text. */
    clip_data->data = malloc(clip_data->dsz + 1);
    memmove(clip_data->data, xcb_get_property_value(r0), clip_data->dsz);
    clip_data->data[clip_data->dsz] = 0;
    free(r0);
/* why? */
    xcb_delete_property(connection, clbd->owner, clbd->property);
  }
  return clip_data;
}

void
_xclb_paste_reply(xcb_clipboard_t *clbd, char **contents, int *data_size) {

  xcb_selection_data_t *clip_data;

  clip_data = _xclb_get_reply(connection, clbd);
  if (clip_data != NULL) {
    *contents = (char*)clip_data->data;
    *data_size = clip_data->dsz;
    XCLB_ASSERT(true, "_xclb_paste_reply()");
    return;
  }
  XCLB_ASSERT(true, "_xclb_paste_reply() no data");
}

static bool
_xclb_set_data(xcb_clipboard_t *clbd,
               xcb_selection_request_event_t *request) {

  char buffer[32];
  xcb_get_atom_name_cookie_t cookie
    = xcb_get_atom_name(connection, request->target);
  xcb_get_atom_name_reply_t *reply
    = xcb_get_atom_name_reply(connection, cookie, NULL);
  memmove(buffer, xcb_get_atom_name_name(reply), reply->name_len);
  buffer[reply->name_len] = 0;
  free(reply);

  XCLB_ASSERT(true, "_xclb_set_data()");

  if (request->property == XCB_NONE) {
      /* per ICCCM */
    if (request->target == MULTIPLE)  goto no_conversion;
    request->property = request->target;
  }
  if (request->target == TARGETS) {
    xcb_atom_t targets[4];
    targets[0] = MULTIPLE;
    targets[1] = STRING;
    targets[2] = TIMESTAMP;
    targets[3] = UTF8_STRING;
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                        request->requestor,
                        request->property, XCB_ATOM_ATOM,
                        sizeof(xcb_atom_t) * 8,  /* number of bits / element */
                        sizeof(targets) / sizeof(xcb_atom_t), targets);
    goto conversion;
  }
  if (request->target == MULTIPLE) {
    xcb_atom_t *aPtr;
    uint32_t count, idx;
    xcb_get_property_cookie_t cookie;
    xcb_get_property_reply_t *reply;

      /* atom pairs: [0] = target, [1] = property */
    cookie = xcb_get_property(connection, 0, request->requestor,
                         request->property, ATOM_PAIR, 0, UINT_MAX/4);
    reply = xcb_get_property_reply(connection, cookie, NULL);
    xcb_flush(connection);
    aPtr = xcb_get_property_value(reply);
    count = xcb_get_property_value_length(reply) / sizeof(xcb_atom_t);
    for (idx = 0; idx < count; idx += 2) {
      if (aPtr[idx] == STRING) {
        xcb_selection_data_t *seldata = &clbd->xseldata;
          /* change atom[i+1] contents, not the atom */
        xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                            request->requestor,
                            aPtr[idx], aPtr[(idx + 1)],
                            sizeof(uint8_t) * 8,
                            seldata->dsz, seldata->data);
      } else {
          /* change the atom to none */
        aPtr[(idx + 1)] = XCB_NONE;
      }
    }
      /* pass back atom pairs, reflecting our 'none' setting */
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                        request->requestor,
                        request->property, XCB_ATOM_ATOM,
                        sizeof(xcb_atom_t) * 8,
                        count, aPtr);
    free(reply);
    goto conversion;
  }
  if ( (request->target == STRING) || (request->target == UTF8_STRING) ) {
    xcb_selection_data_t *seldata = &clbd->xseldata;
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                        request->requestor,
                        request->property, request->target,
                        sizeof(uint8_t) * 8,
                        seldata->dsz, seldata->data);
    goto conversion;
  }
  if (request->target == TIMESTAMP) {
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                        request->requestor,
                        request->property, request->target,
                        sizeof(xcb_timestamp_t) * 8,
                        1, &clbd->time);
    goto conversion;
  }
no_conversion:
  printf("no coversion for atom %d = %s\n", request->target, buffer);
  return false;
conversion:
  xcb_flush(connection);
/*  printf("   coversion for atom %d = %s\n", request->target, buffer);*/
  return true;
}

/* For paste, get converted clipboard data.
   In reponse to xcb_convert_selection(). Search for
   an acceptable data format for paste. A supplier of
   clipboard data may have multiple types. */
/* Current setup up for ICCCM compliance and STRING */
void
_xclb_process_request(xcb_clipboard_t *clbd,
                      xcb_selection_request_event_t *request) {

  xcb_selection_notify_event_t *notify = calloc(32, 1);

  XCLB_ASSERT(true, "_xclb_process_request()");

  notify->response_type = XCB_SELECTION_NOTIFY;
  notify->time      = request->time;
  notify->requestor = request->requestor;
  notify->selection = request->selection;
  notify->target    = request->target;
  notify->property  =
             _xclb_set_data(clbd, request) ? request->property : XCB_NONE;
  xcb_send_event(connection, false, request->requestor,
                 XCB_EVENT_MASK_PROPERTY_CHANGE, (char *)notify);
  xcb_flush(connection);
}

bool
_xclb_process_notify(xcb_clipboard_t *clbd,
                     xcb_selection_notify_event_t *notify) {

  XCLB_ASSERT(true, "_xclb_process_notify()");

  if (notify->property == XCBD_DATA)  return true;
    /* Test if has data from deleted window */
  if (clbd->property == XCBD_DATA) {
    xcb_selection_data_t *seldata = &clbd->xseldata;
    xcb_window_t owner = notify->requestor;
    xcb_set_selection_owner(connection, owner, CLIPBOARD, clbd->time);
    notify->property = clbd->property;
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE,
                        notify->requestor,
                        notify->property, notify->target,
                        sizeof(uint8_t) * 8,
                        seldata->dsz, seldata->data);
    return true;
  }
  return false;
}

#pragma mark *** Creation ***

xcb_clipboard_t *
xclb_initialize(xcb_connection_t *c) {

  xcb_clipboard_t *clbd;

/*
                 This is now in atoms.c

  uint16_t adx;
  struct {
    char *name;
    uint16_t nsz;
    xcb_intern_atom_cookie_t cookie;
    xcb_intern_atom_reply_t *reply;
  } atoms[8] = {
    { "ATOM_PAIR", 9, {0}, NULL }, { "CLIPBOARD", 9, {0}, NULL },
    { "MULTIPLE", 8, {0}, NULL },
    { "STRING", 6, {0}, NULL }, { "TARGETS", 7, {0}, NULL},
    { "TIMESTAMP", 9, {0}, NULL}, { "UTF8_STRING", 11, {0}, NULL },
    { "XCBD_DATA", 9, {0}, NULL }
  };


  for (adx = 0; adx < 8; adx++)
    atoms[adx].cookie = xcb_intern_atom(c, 0, atoms[adx].nsz, atoms[adx].name);
  xcb_flush(c);

  for (adx = 0; adx < 8; adx++)
    atoms[adx].reply = xcb_intern_atom_reply(c, atoms[adx].cookie, NULL);
  xcb_flush(c);

  ATOM_PAIR         = atoms[0].reply->atom;  free(atoms[0].reply);
  CLIPBOARD         = atoms[1].reply->atom;  free(atoms[1].reply);
  MULTIPLE          = atoms[2].reply->atom;  free(atoms[2].reply);
  STRING            = atoms[3].reply->atom;  free(atoms[3].reply);
  TARGETS           = atoms[4].reply->atom;  free(atoms[4].reply);
  TIMESTAMP         = atoms[5].reply->atom;  free(atoms[5].reply);
  UTF8_STRING       = atoms[6].reply->atom;  free(atoms[6].reply);
  XCBD_DATA         = atoms[7].reply->atom;  free(atoms[7].reply);
*/

  connection = c;
  clbd = malloc(sizeof(xcb_clipboard_t));
  memset(clbd, 0, sizeof(xcb_clipboard_t));
  return clbd;
}


