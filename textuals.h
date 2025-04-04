#ifndef __SESS_TEXTUALS_H__
#define __SESS_TEXTUALS_H__

#include "objects.h"

#ifndef _WINT_T  /* needed for mouse_double_click() */
 #define _WINT_T
 typedef wchar_t  wint_t;
#endif
 /* Supplied by Pheonix since libc doesn't work */
extern int    iswalnum(wint_t c);
extern int    iswpunct(wint_t c);
extern int    iswspace(wint_t c);
  /* needed for key_movement_control() */
/* extern int   iswalnum(wint_t c);  see above include */
extern int    iswblank(wint_t c);
  /* Supplied by Pheonix since libc doesn't contain */
extern char * memrchr(const char *, int, size_t);
  /* Supplied by Pheonix since cairo doesn't handle */
extern void  ext_cairo_show_glyphs(char **, unsigned, cairo_t *, char *,
                                                           int, int, double);
extern int   ext_cairo_glyph_advance(char **, cairo_t *, char *,
                                                           int, int, double);
extern int   ext_cairo_glyphs_advance(char **, unsigned, cairo_t *, char *,
                                                           int, int, double);

#define MARK_ALLOC  4096
#define TGAP_ALLOC  4096
#define BUFF_ALLOC  4096
/* defined by number of <space> characters */
#define TAB_TEXT_WIDTH 4
/* mostly for button text indents */
#define BUTTON_TEXT_MIN 3

typedef struct { int32_t x, y, offset; }   location;
typedef struct { int32_t x, y, w, h; }     PhxBin;
typedef struct phx_textbuffer              PhxTextbuffer;

 /* This is still under development. But should apply also to non-texual. */
typedef enum {
 PHXNEWLINE = 1,  /*information, <newline> count, semi y positional */
               /* if single font size, y postioning */
 PHXSEARCH,    /* rect fill */
 PHXFONT,        /* font info */
 PHXFOREGROUND,  /* pen drawing */
 PHXBACKGROUND,  /* rect fill */
 PHXLASTMARK
} PhxMarkType;

  /* data for certain objects */
typedef struct PhxMarkData {  uint32_t d0, d1;  }       PhxMarkData;
typedef struct PhxMarkPair {  uint32_t offset, line;  } PhxMarkPair;
typedef struct PhxMark {
 PhxMarkType  type;
  /* case: update may need some data not provided by Textview */
 void (*_mark_update)(PhxTextbuffer*);
 void (*_mark_draw)(PhxTextbuffer*, cairo_t*);
 void (*_mark_raze)(PhxTextbuffer*);
 void         *data;
} PhxMark;

typedef struct phx_newline_t {
 PhxObject   *otxt;        /* textview attached to */
 struct {
  PhxMarkData *pairs;      /* 2 data points */
  uint32_t    ncount;      /* count of _results.pair */
 } _results;
 uint32_t    state;        /* (pad) */
} phx_newline_t;

struct phx_textbuffer {
 char             *string;               /* c-str pointer (text buffer) */
             /* flags, eg. Insert, Delete keys */
 uint16_t         state;
             /* draw_buffer allotment in 4096 multiples. */
 uint16_t         dbSz;
             /* editing management */
 int32_t          str_nil,               /* simular to strlen return */
                  gap_start,
                  gap_end,
                  gap_delta;
             /* cursor management */
 location         insert,
                  release,
                  interim,
                  drop;
             /* x, y plotting of glyphs, + other attributes */
 PhxObject        *owner;                 /* reference to object */
 char             *glyph_widths;          /* ascii, create quick access */
 PhxBin           bin;
             /* newlines, and others tracking */
 uint32_t         dirty_offset;
 uint32_t         dirty_line;     /* currently used like boolean. */
 PhxMark          **mark_list;
  /* buffer (copy of string inside bin) used to avoid 'lock'(s).
    Used/created for _draw_cb. */
 uint32_t         draw_top_offset;
 uint32_t         draw_end_offset;
 char             *draw_buffer;
  /* drag n drop */
 PhxBin           drag_sbin;
};

/* Applies to textbuffer only */
#define TBIT_KEY_INSERT (1 << 0)
#define TBIT_KEY_DELETE (1 << 1)

extern int16_t           ui_textual_glyph_advance(PhxObject *obj,
                                                  char **stream,
                                                  int16_t byte_count);
extern char *            ui_textual_glyph_table(PhxObject *obj);
extern void              _default_textbuffer_draw(PhxObject *b,
                                                  cairo_t *cr);
extern void              _default_textbuffer_raze(void *obj);
extern PhxTextbuffer *   ui_textbuffer_create(PhxObject *obj,
                                              char *data);

/* textual marks... adding textbuffer functionality. */
extern void  phxmarks_list_add(PhxTextbuffer *, PhxMark *);
extern bool  _mark_allocation(void **, size_t, uint32_t);
extern int   _led0_compare(const void *a, const void *b);

 /* PhxTextbuffer routines */
extern void  location_auto_scroll(PhxTextbuffer *, location *);
extern void  location_for_offset(PhxTextbuffer *, location *);
extern void  location_for_point(PhxTextbuffer *, location *);
extern void  location_shift_click(PhxTextbuffer *, int16_t, int16_t);
extern void  location_double_click(PhxTextbuffer *, bool);
extern void  location_triple_click(PhxTextbuffer *, int16_t, int16_t, bool);
extern void  location_scroll_click(PhxTextbuffer *, int16_t, int16_t);
extern void  location_select_all(PhxTextbuffer *);
extern void  location_key_motion(PhxTextbuffer *, int16_t);
extern void  location_interim_equalize(PhxTextbuffer *, int32_t, int32_t);
extern void  location_selection_equalize(PhxTextbuffer *);
extern bool  location_drag_begin(PhxTextbuffer *, int16_t, int16_t);
extern void  _textbuffer_flush(PhxTextbuffer *);
extern void  _textbuffer_edit_set(PhxTextbuffer *, int32_t);
extern void  _textbuffer_replace(PhxTextbuffer *, char *, int32_t);
extern void  _textbuffer_insert(PhxTextbuffer *, char *, int32_t);
extern void  _textbuffer_delete(PhxTextbuffer *);
extern void  _textbuffer_drag_release(PhxTextbuffer *, bool);
 /* clipboard related */
extern void  _textbuffer_copy(PhxTextbuffer *);
extern void  _textbuffer_cut(PhxTextbuffer *);
extern void  _textbuffer_paste_notify(PhxTextbuffer *);
extern void  _textbuffer_paste(PhxTextbuffer *);

#endif /* __SESS_TEXTUALS_H__ */