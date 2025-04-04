#ifndef __SESS_CLIPBOARD_H__
#define __SESS_CLIPBOARD_H__

#include <xcb/xcb.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

#ifndef XCBSELDATA
 #define XCBSELDATA
 typedef struct {
   xcb_atom_t    type;        /* type of data */
   uint32_t      dsz;         /* byte size of data */
   uint8_t       *data;
 } xcb_selection_data_t;
#endif

typedef struct {
  xcb_window_t            owner;
  xcb_timestamp_t         time;
  xcb_atom_t              selection;
  xcb_atom_t              target;
  xcb_atom_t              property;
  xcb_selection_data_t    xseldata;
} xcb_clipboard_t;

/* used in main event loop */
extern xcb_clipboard_t *
                xclb_initialize(xcb_connection_t *connection);
extern void     _xclb_process_clear(xcb_clipboard_t *clbd);
extern void     _xclb_set_ownership(xcb_clipboard_t *clbd,
                                    xcb_window_t owner,
                                    xcb_timestamp_t time);
/* used in main event loop, and objects' meter  */
extern void     _xclb_process_request(xcb_clipboard_t *clbd,
                                      xcb_selection_request_event_t *request);
extern bool     _xclb_process_notify(xcb_clipboard_t *clbd,
                                      xcb_selection_notify_event_t *notify);

/* used in Textbuffer */
extern void     _xclb_copy(xcb_clipboard_t *clbd,
                           xcb_window_t requestor,
                           char *src,
                           int sz);
extern void     _xclb_paste_request(xcb_clipboard_t *clbd,
                                    xcb_window_t requestor);
extern void     _xclb_paste_reply(xcb_clipboard_t *clbd,
                                  char **contents,
                                  int *data_size);

#endif /* __SESS_CLIPBOARD_H__ */