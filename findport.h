#ifndef __SESS_FINDPORT_H__
#define __SESS_FINDPORT_H__

#include "textviews.h"
#include "buttons.h"
#include "labels.h"
#include "configure.h"
#include "banks.h"

/* XXX Currently nexus based. When a textview included in multiple
  object based nexus, should auto-switch to dialog. */
/* Findport is a well defined nexus, which includes its objects.
  It's activated when a textview is 'focus' and keyboard uses correct keyval.
  Eventually to allow when files selected to search activate a dialog,
  making it 'session'-oriented find.  */
typedef PhxNexus                                         PhxFindport;

/* XXX When goes to 'dialog' mode, member 'findbar' needs transient check. */
typedef struct phx_fsearch_t {
 PhxObject   *otxt;        /* textview attached to */
 struct {
  PhxMarkData *pairs;      /* the selection offsets of search */
  uint32_t    ncount;      /* count of _results.pair */
 } _results;
 uint32_t    state;        /* bit 0 (changed) */
   /* basically a spelled out exclusive */
 char        *string;      /* string found or seaching for */
 PhxFindport *findbar;     /* findbar for attached textview */
 char        *file;        /* file attached to */
 void        *exclusive; /* for gtk, others... extra data */
} phx_fsearch_t;

struct phx_findboard_t {
  uint16_t        ncount;
  uint16_t        sdsz;
  uint16_t        rdsz;
  uint16_t        pad0;
  phx_fsearch_t   **fsearches;    /* array of file searches */
  char            *sdata;         /* 'entered' search_string */
  char            *rdata;         /* 'entered' replace_string */
  void            *exclusive;     /* possible add ons */
};

extern void              ui_findport_search(PhxInterface *iface,
                                            PhxObjectTextview *otxt);
extern void              ui_findport_keyboard(PhxObjectTextview *otxt,
                                              uint8_t key);
extern PhxFindport *     ui_findport_create(PhxInterface *iface,
                                            PhxObjectTextview *otxt);

#endif /* __SESS_FINDPORT_H__ */