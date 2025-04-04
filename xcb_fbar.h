#include "textviews.h"
#include "buttons.h"
#include "labels.h"
#include "configure.h"
#include "banks.h"

/* button   [21] | 3 (15) 3,1 (15) 3 | (-2) (20,22) */
/* textview [21] | 2 (15) 4,2 (15) 4 | (0)  */

/* button   [19] | 2 (15) 2,1 (15) 3 | (-1) (19,19?) */
/* textview [19] | 2 (15) 2,1 (15) 3 | (-1)  */

/* findbar design to include a seperate window, like a dialog.
  Intent is a dialog when search of multiple files.
  Intent is a nexus when search of current file.
  file included in fdata, but no routines, all otxt based */

/* findbar is a well defined nexus, which includes its objects. */
typedef PhxNexus                                         PhxFindport;


#define BOX_HEIGHT  23
#define BOX_MIN_WIDTH  (int16_t)((double)BOX_HEIGHT / .0406)

static char *button_labels[] = {
  "Find", "Find & Replace",
  "Replace All",
  "Replace",
  "Replace & Find",
  "Done"
};

/* order, index of nexus objects */
enum {
 choose_box = 0,
 replace_all_box,
 replace_box,
 replace_find_box,
 close1_box,
 close0_box,
 textview_replace_box,
 textview_find_box,
 navigate_right_box,
 navigate_left_box,
 found_box,
 last_object
};

enum {
 RESULT_EQU = 0,
 RESULT_LT,
 RESULT_GT,
 RESULT_LE,
 RESULT_GE
};

/* XXX When goes to 'dialog' mode, member 'findbar' needs transient check. */
typedef struct phx_fsearch_t {
 PhxObject   *otxt;        /* textview attached to */
 struct {
  PhxMarkData *pairs;      /* the selection offsets of search */
  uint32_t    ncount;      /* count of _results.pair */
 } _results;
 uint32_t    state;        /* (pad) */
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

static phx_fsearch_t *  _fsearch_for_textview(PhxObjectTextview *otxt);
static phx_fsearch_t *  _fsearch_for_findport(PhxFindport *findbar);
static int32_t   _findport_search_from(char *tbuf,
                                       phx_fsearch_t *fdata,
                                       uint32_t start_offset);
static PhxNexus *  _findport_create(PhxInterface *iface,
                                    PhxRectangle configure);
static PhxFindport *      ui_findport_create(PhxInterface *iface,
                                      PhxObjectTextview *otxt);
