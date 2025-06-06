#include "textuals.h"

#pragma mark *** TextMarks ***

bool
_mark_allocation(void **abstract, size_t abstract_size, uint32_t ncount) {

    /* Note: does check 1 early, so to leave a (uint32_t)~0 terminator */
  uint32_t n = (uint32_t)(ncount + 1);
  if (n != (uint32_t)~0) {
      /* n = 0xfffffffe
         n & 0xf = 0xe != 0xf no alloc, allows 0xfffffffe write */
      /* n = 0xffffffef
         n & 0xf = 0xf == 0xf alloc, available 0xffffffef - 0xfffffffe */
    if ( (n & (OBJS_ALLOC - 1)) == (OBJS_ALLOC - 1) ) {
      uint32_t oldsz = ((n & ~(OBJS_ALLOC - 1)) + OBJS_ALLOC) * abstract_size;
      uint32_t addsz = OBJS_ALLOC * abstract_size;
      char *nPtr = realloc(*abstract, (oldsz + addsz));
      if (nPtr == NULL)  goto rej;
      memset(&nPtr[oldsz], 0, addsz);
      *abstract = nPtr;
    }
    return false;
  }
rej:
  DEBUG_ASSERT(true, "realloc() rejected!");
  return true;
}

/* finds key value <= d0 */
int
_led0_compare(const void *a, const void *b) {

  uint32_t s_offset, e_offset;
  PhxMarkData *mAsk0, *mAsk1;

  uint32_t key = *((uint32_t*)a);
  mAsk0 = (PhxMarkData*)b;

  s_offset = mAsk0->d0;
  if (key < s_offset)   return -1;
  if (key == s_offset)  return 0;

  mAsk1 = mAsk0 + 1;
  e_offset = mAsk1->d0;
  if (key < e_offset)  return 0;
  return 1;
}

/* When d1 used as line value or index:
  finds key value <= d1, note: d0 == (uint32_t)~0 holds last valid d1 */
static int
_led1_compare(const void *a, const void *b) {

  uint32_t s_offset;
  uint32_t key = *((uint32_t*)a);
  PhxMarkData *mAsk0 = (PhxMarkData*)b;

  s_offset = mAsk0->d1;
  if (key < s_offset)  return -1;
  if (key == s_offset)  return 0;
  if (mAsk0->d0 == (uint32_t)~0)  return 0;
  return 1;
}

void
phxmarks_list_add(PhxTextbuffer *tbuf, PhxMark *mark) {

  uint32_t idx;
  if (tbuf->mark_list == NULL)  return;
  if (mark == NULL)  return;

    /* 0 always <newline> marks */
  idx = 0;
  if (tbuf->mark_list[idx] != NULL)
    while ((tbuf->mark_list[(++idx)]) != NULL) ;
  if ((idx + 1) == (uint32_t)~0) {
    puts("realloc failure: max objects reached.");
    return;
  }
  if ((idx & (OBJS_ALLOC - 1)) == (OBJS_ALLOC - 1)) {
    size_t newSz = (idx + 1) + (OBJS_ALLOC * sizeof(PhxMark*));
    PhxMark **newHnd = realloc(tbuf->mark_list, newSz);
    if (newHnd == NULL) {
      puts("realloc failure: text_mark_list_add");
      return;
    }
    memset(&newHnd[(idx + 1)], 0, (OBJS_ALLOC * sizeof(PhxMark*)));
    tbuf->mark_list = newHnd;
  }
  tbuf->mark_list[idx] = mark;
}

PhxMark *
phxmarks_for_type(PhxMark *marks, PhxMarkType type) {
  do
    if (marks->type == type)  break;
  while ((++marks) != NULL);
  return marks;
}

static void
phxmarks_update(PhxTextbuffer *tbuf) {

  uint32_t idx;
  PhxMark *mPtr;

  if (tbuf->mark_list == NULL)  return;
  if (tbuf->dirty_offset == (uint32_t)~0)  return;

  idx = 0;
  mPtr = tbuf->mark_list[0];
  do {
    if (mPtr->_mark_update != NULL)
      mPtr->_mark_update(tbuf);
  } while ((mPtr = tbuf->mark_list[(++idx)]) != NULL);
  tbuf->dirty_offset = (uint32_t)~0;
  tbuf->dirty_line   = (uint32_t)~0;
}

static void
phxmarks_draw(PhxTextbuffer *tbuf, cairo_t *cr) {

  uint32_t idx;
  PhxMark *mPtr;

  if (tbuf->mark_list == NULL)  return;
    /* <newline> marks [0] don't draw. */
  idx = 0;
  while ((mPtr = tbuf->mark_list[(++idx)]) != NULL) {
    if (mPtr->_mark_draw != NULL)
      mPtr->_mark_draw(tbuf, cr);
  }
}

static void
phxmarks_raze(PhxTextbuffer *tbuf) {

  uint32_t idx;
  PhxMark *mPtr;

  if (tbuf->mark_list == NULL)  return;

  idx = 0;
  mPtr = tbuf->mark_list[0];
  do {
    if (mPtr->_mark_raze != NULL)
      mPtr->_mark_raze(tbuf);
    free(mPtr);
    tbuf->mark_list[idx] = NULL;
  } while ((mPtr = tbuf->mark_list[(++idx)]) != NULL);
}

static __inline void
phxmarks_dirty_offset(PhxTextbuffer *tbuf, uint32_t offset) {
  tbuf->dirty_offset = minof(offset, tbuf->dirty_offset);
}

/* currently a bool type value */
static __inline void
phxmarks_dirty_line_set(PhxTextbuffer *tbuf) {
  tbuf->dirty_offset = 1;
}

#pragma mark *** Newline Marks ***

static void
_newlinemarks_update(PhxTextbuffer *tbuf) {

  phx_newline_t *ndata;
  PhxMarkData *mdPtr, *offset_mark;
  char *sPtr, *nPtr, *rdPtr;
  uint32_t ndx, doffset;

  if (tbuf->dirty_offset == (uint32_t)~0)  return;
  if (tbuf->mark_list == NULL)  return;

    /* newline is attached to mark's data. */
  DEBUG_ASSERT((tbuf->mark_list[0]->type != PHXNEWLINE),
                           "failure: not <newline> marks.");
  ndata = (phx_newline_t*)tbuf->mark_list[0]->data;
  mdPtr = ndata->_results.pairs;

  if ( (tbuf->dirty_offset == 0) || (ndata->_results.ncount == 0) )
    goto zero_start;

  offset_mark = bsearch(&tbuf->dirty_offset, ndata->_results.pairs,
                                             ndata->_results.ncount + 1,
                                             sizeof(PhxMarkData),
                                             _led0_compare);
  if (offset_mark != NULL) {
    if (offset_mark->d0 == (uint32_t)~0)  offset_mark--;
    ndx = (uint32_t)(offset_mark - ndata->_results.pairs);
    doffset = offset_mark->d0;
  } else {
zero_start:
    ndx = 0;
    doffset = 0;
  }

  sPtr = tbuf->string;
  rdPtr = tbuf->string + doffset;
  ndx++;

  if (tbuf->gap_start != (tbuf->str_nil + 1)) {
    char *gsPtr = &sPtr[tbuf->gap_start];
    char *gePtr = &sPtr[tbuf->gap_end];
    if (rdPtr < gsPtr) {
      do {
        if (_mark_allocation((void**)&ndata->_results.pairs,
                                    sizeof(PhxMarkData), ndx))  break;
        mdPtr = &ndata->_results.pairs[ndx];
        nPtr = strchr(rdPtr, '\n');
        if ((rdPtr <= gsPtr) && (nPtr >= gsPtr))  break;
        if (nPtr == NULL)  goto lastentry;
        mdPtr->d0 = (uint32_t)((rdPtr = (++nPtr)) - sPtr);
        mdPtr->d1 = ndx;
        ndx++;
      } while (1);
    }
    sPtr += (size_t)(gePtr - gsPtr);
    rdPtr = gePtr;
  }
  nPtr = rdPtr;
  do {
    if (_mark_allocation((void**)&ndata->_results.pairs,
                                sizeof(PhxMarkData), ndx))  break;
    mdPtr = &ndata->_results.pairs[ndx];
    if ((nPtr = strchr(nPtr, '\n')) == NULL)  break;
    mdPtr->d0 = (uint32_t)((++nPtr) - sPtr);
    mdPtr->d1 = ndx;
    ndx++;
  } while (1);
lastentry:
  mdPtr->d0 = (uint32_t)~0;
  mdPtr->d1 = ndx;
  ndata->_results.ncount = ndx;
}

static void
_newlinemarks_raze(PhxTextbuffer *tbuf) {

  if (tbuf->mark_list == NULL)  return;
  if (tbuf->mark_list[0] == NULL)  return;
  if (tbuf->mark_list[0]->data == NULL)  return;
  free(((phx_newline_t*)tbuf->mark_list[0]->data)->_results.pairs);
  ((phx_newline_t*)tbuf->mark_list[0]->data)->_results.pairs = NULL;
  free(tbuf->mark_list[0]->data);
  tbuf->mark_list[0]->data = NULL;
}

static phx_newline_t *
_newlinemarks_create(PhxInterface *iface, PhxObject *otxt) {

  PhxMark *newline_mark;
  phx_newline_t *newline;

    /* data for newline_mark */
  newline = malloc(sizeof(phx_newline_t));
  memset(newline, 0, sizeof(phx_newline_t));
  newline->_results.pairs = malloc(OBJS_ALLOC * sizeof(PhxMarkData));
  memset(newline->_results.pairs, 0, (OBJS_ALLOC * sizeof(PhxMarkData)));
  newline->otxt = otxt;

    /* Attachment of a mark to list */
  newline_mark = malloc(sizeof(PhxMark));
  newline_mark->type         = PHXNEWLINE;
  newline_mark->_mark_update = _newlinemarks_update;
  newline_mark->_mark_draw   = NULL;
  newline_mark->_mark_raze   = _newlinemarks_raze;
  newline_mark->data         = newline;
  phxmarks_list_add((PhxTextbuffer*)otxt->exclusive, newline_mark);
  return newline;
}

static void
phxmarks_initialize(PhxTextbuffer *tbuf) {

  if (tbuf->mark_list != NULL) {
    DEBUG_ASSERT(true, "re-initialize a no-no! rejected!");
    return;
  }

  tbuf->mark_list = malloc(OBJS_ALLOC * sizeof(PhxMark*));
  memset(tbuf->mark_list, 0, (OBJS_ALLOC * sizeof(PhxMark*)));

    /* setup a mark, add to above created list. */
  _newlinemarks_create(ui_interface_for(ui_window_for(tbuf->owner)), tbuf->owner);

  tbuf->dirty_offset = 0;
  phxmarks_update(tbuf);
}

#pragma mark *** Textbuffer Modifications ***

/* given a text location, positions loc->x, loc->y
   adjust bin so edit location in view */
void
location_auto_scroll(PhxTextbuffer *tbuf, location *loc) {

  int x, y, delta, font_em;
                  /* x auto-scroll */
  delta = 0;
  x = loc->x;
  font_em = tbuf->owner->attrib->font_em;
  if (x < (tbuf->bin.x + font_em)) {
    delta = (tbuf->bin.x + font_em) - x;
    if (delta > tbuf->bin.x)  delta = tbuf->bin.x;
  } else if (x >= (tbuf->bin.x + tbuf->bin.w - font_em)) {
    delta = (tbuf->bin.x + tbuf->bin.w - font_em) - x;
  }
  tbuf->bin.x -= delta;
                 /* y auto-scroll */
  y = (loc->y / font_em) * font_em;
  delta = 0;
  if (y <= tbuf->bin.y) {
    delta = tbuf->bin.y - y;
  } else if (y > (tbuf->bin.y + tbuf->bin.h - font_em)) {
    delta = (tbuf->bin.y + tbuf->bin.h - font_em) - y;
  }
  tbuf->bin.y -= delta;
}

/* given a offset to text location
   return for loc->offset positions loc->x, loc->y.
   positions in object's draw_box coordinates. */
void
location_for_offset(PhxTextbuffer *tbuf, location *loc) {

  phx_newline_t *ndata;
  PhxMarkData *line_mark;
  int32_t x;
  char *gsPtr, *gePtr, *rdPtr, *textPtr;

  phxmarks_update(tbuf);

  ndata = (phx_newline_t*)tbuf->mark_list[0]->data;
  line_mark = bsearch(&loc->offset, ndata->_results.pairs,
                                    ndata->_results.ncount + 1,
                                    sizeof(PhxMarkData),
                                    _led0_compare);
  loc->y = line_mark->d1 * tbuf->owner->attrib->font_em;
  DEBUG_ASSERT((line_mark->d0 == (uint32_t)~0),
                             "failure: location_for_offset.");
  x = 0;
  gsPtr = tbuf->string + tbuf->gap_start;
  gePtr = tbuf->string + tbuf->gap_end;
  rdPtr = tbuf->string + line_mark->d0;

  textPtr = &tbuf->string[loc->offset];
  if (textPtr >= gsPtr)  textPtr += gePtr - gsPtr;
  if (rdPtr >= gsPtr)  {
    rdPtr += gePtr - gsPtr;
    if (*rdPtr == 0) {
      loc->offset = line_mark->d0;
      loc->x = 0;
      return;
    }
    goto both_updated;
  }
  if (gsPtr < textPtr) {
    x += ui_textual_glyph_advance(tbuf->owner, &rdPtr, (gsPtr - rdPtr));
    rdPtr = gePtr;
  }
both_updated:
  if (rdPtr < textPtr)
    x += ui_textual_glyph_advance(tbuf->owner, &rdPtr, (textPtr - rdPtr));

  loc->x = x;
}

/* given positions loc->x, loc->y (object's draw_box coordinates)
   return a loc->offset for that position */
void
location_for_point(PhxTextbuffer *tbuf, location *loc) {

  int32_t x, y, font_em, sum;
  char *gsPtr, *gePtr, *rdPtr, *nPtr;

  phxmarks_update(tbuf);

  rdPtr = tbuf->string;
  gsPtr = &tbuf->string[tbuf->gap_start];
  gePtr = &tbuf->string[tbuf->gap_end];

  font_em = tbuf->owner->attrib->font_em;
  y = loc->y / font_em;  /* floor, line number */
  x = loc->x;
  loc->y = 0;

    /* used to place with character offset, instead of mouse click */
  if (y > 0) {

    PhxMarkData *line_mark;
    phx_newline_t *ndata = tbuf->mark_list[0]->data;

    line_mark = bsearch(&y, ndata->_results.pairs, ndata->_results.ncount + 1,
                           sizeof(PhxMarkPair), _led1_compare);
    if (line_mark->d0 == (uint32_t)~0) {
      line_mark--;
      x = INT_MAX;
    }
    if (y > line_mark->d0)  x = INT_MAX;
    rdPtr = tbuf->string + line_mark->d0;
    loc->y = line_mark->d1 * font_em;
  }
  if (rdPtr >= gsPtr) rdPtr += gePtr - gsPtr;
  nPtr = strchr(rdPtr, '\n');
  if ((nPtr >= gsPtr) && (rdPtr <= gsPtr))
    nPtr = strchr(gePtr, '\n');
  if (nPtr == NULL)
    nPtr = &tbuf->string[tbuf->str_nil];
  sum = 0;
  while ((nPtr - rdPtr) > 0) {
    int gw = ui_textual_glyph_advance(tbuf->owner, &rdPtr, 1);
    if ((sum + gw) > x) {
      if ((x - ((gw + 1) >> 1)) >= sum)  sum += gw;
      else do --rdPtr; while ((*rdPtr & 0x0C0) == 0x080);
      break;
    }
    if (rdPtr == gsPtr)  rdPtr = gePtr;
    if ((sum += gw) == x)  break;
  }
  if (rdPtr > gsPtr)  rdPtr -= gePtr - gsPtr;
  loc->offset = rdPtr - tbuf->string;
    /* used to place with character offset, instead of mouse click */
  loc->x = sum;
}

/* while editing, gap_buffer moved, set insert info for
   new insert position. This allows delay of having to update */
/* PhxMark(s) for line_marks */
static void
location_for_edit_update(PhxTextbuffer *tbuf, int sz) {

  char *rdPtr, *endPtr, *nPtr;
  int font_em = tbuf->owner->attrib->font_em;

  rdPtr = &tbuf->string[tbuf->insert.offset];
  if (sz == 0)  goto update_finish;
  if (sz == 1) {
    tbuf->insert.offset++;
    if (*rdPtr != '\n') {
      tbuf->insert.x += ui_textual_glyph_advance(tbuf->owner, &rdPtr, 1);
    } else {
      tbuf->insert.x = 0;
      tbuf->insert.y += font_em;
      phxmarks_dirty_line_set(tbuf);
    }
    goto update_finish;
  }
    /* delete 1 character, gap move 'down' 1 */
  if (sz == -1) {
    tbuf->insert.offset--;
    if (*(--rdPtr) != '\n') {
      char *sPtr = rdPtr + 1;
      if (*rdPtr < 0)  while ((*rdPtr & 0x0C0) == 0x080)  rdPtr--;
      tbuf->insert.x
        -= ui_textual_glyph_advance(tbuf->owner, &rdPtr, (sPtr - rdPtr));
    } else {
      int x;
      tbuf->insert.y -= font_em;
      phxmarks_dirty_line_set(tbuf);
      if (tbuf->insert.y < 0) {
        tbuf->insert.x = (tbuf->insert.y = 0); tbuf->insert.offset = 0;
        goto update_finish;
      }
      x = 0;
      while ((--rdPtr) >= tbuf->string) {
        char *dPtr;
        char *sPtr = rdPtr + 1;
        if (*rdPtr == '\n')  break;
        if (*rdPtr < 0)  while ((*rdPtr & 0x0C0) == 0x080)  rdPtr--;
          /* do not want advancement of rdPtr, use a dummy pointer */
        dPtr = rdPtr;
        x += ui_textual_glyph_advance(tbuf->owner, &dPtr, (sPtr - rdPtr));
      }
      tbuf->insert.x = x;
    }
    goto update_finish;
  }
  if (sz < 0) {
    printf("    unhandled location_update sz: %d\n", sz);
    goto update_finish;
  }
  /*if (sz > 1) */
    /* sz characters were pasted in */
  endPtr = &tbuf->string[(tbuf->insert.offset + sz)];
  nPtr = memchr(rdPtr, '\n', sz);
  if (nPtr != NULL) {
    tbuf->insert.x = 0;
    tbuf->insert.y += font_em;
    phxmarks_dirty_line_set(tbuf);
    do {
      rdPtr = nPtr + 1;
      if (rdPtr >= endPtr)  break;
      nPtr = memchr(rdPtr, '\n', (endPtr - rdPtr));
      if (nPtr == NULL)  break;
      tbuf->insert.y += font_em;
    } while (1);
  }
  tbuf->insert.x
    += ui_textual_glyph_advance(tbuf->owner, &rdPtr, (endPtr - rdPtr));
  tbuf->insert.offset += sz;

update_finish:
  location_auto_scroll(tbuf, &tbuf->insert);
  tbuf->interim = (tbuf->release = tbuf->insert);
}

void
location_shift_click(PhxTextbuffer *tbuf, int16_t x, int16_t y) {

  location *active;
  location temp;

  temp.x = x + tbuf->bin.x;
  temp.y = y + tbuf->bin.y;
  location_for_point(tbuf, &temp);
  location_auto_scroll(tbuf, &temp);

  active = &tbuf->release;
  if (temp.offset < tbuf->release.offset) {
    active = &tbuf->insert;
    if ( (temp.offset > tbuf->insert.offset)
        && (tbuf->release.offset == tbuf->interim.offset) )
      active = &tbuf->release;
  }
  if (active->offset != temp.offset) {
    tbuf->interim.x = (active->x = temp.x);
    tbuf->interim.y = (active->y = temp.y);
    tbuf->interim.offset = (active->offset = temp.offset);
  }
}

/* XXX
   Currently pspace and ppunct are the same, except for
  the testing wide functions. This will change! Need seperate
  or refactored branching for user to override if need be.
  Plus this to be added to ppunct:
     if clicked on '{', select to forward enclosing '}'
     if clicked on '}', select to backward enclosing '{'
     if clicked on '(', select to forward enclosing ')'
     if clicked on ')', select to backward enclosing '('
     if clicked on '[', select to forward enclosing ']'
     if clicked on ']', select to backward enclosing '['
*/
void
location_double_click(PhxTextbuffer *tbuf, bool shift_click) {

  location *temp;
  int32_t cdx, offset;
  char ch0, *cPtr, *iPtr;
  union { uint8_t u8code[4]; uint32_t u32code; } utf;
  uint8_t *uPtr;

  iPtr = &tbuf->string[tbuf->interim.offset];
  ch0 = *(cPtr = iPtr);
  if (ch0 == 0)  return;

    /* load utf8 code */
  cdx = 0;
  uPtr = utf.u8code;
  utf.u32code = 0;

    /* specific c languauge parsing, do before utf8 parse */
  if (ch0 == '_')  goto palnum;

    /* Do not advance cPtr, act like the following didn't happen. */
  *uPtr = ch0, uPtr++, cdx++;
  if ((ch0 & (char)0x0C0) == (char)0x0C0) {
    while ( (cdx < 4)
             && (((ch0 = cPtr[cdx]) & (char)0x0C0) == (char)0x080) )
      *uPtr = ch0, uPtr++, cdx++;
    ch0 = *(cPtr = iPtr);
  }
    /* pick a card, send wchar_t to functions */
  if (iswspace(utf.u32code) != 0)  goto pspace;
  if (iswalnum(utf.u32code) != 0)  goto palnum;
  if (iswpunct(utf.u32code) != 0)  goto ppunct;
  return;

pspace:
  do {
    cdx = 0;
    uPtr = utf.u8code;
    utf.u32code = 0;
    *uPtr = ch0, uPtr++, cdx++;
    if ((ch0 & (char)0x0C0) == (char)0x0C0) {
      while ( (cdx < 4)
               && (((ch0 = cPtr[cdx]) & (char)0x0C0) == (char)0x080) )
        *uPtr = ch0, uPtr++, cdx++;
    }
    if ( (ch0 == 0) || (iswspace(utf.u32code) == 0) )  break;
    ch0 = *(cPtr += cdx);
  } while (1);
    /* assign release, default is to be release */
    /* default covers '(shift_click && (offset == tbuf->release.offset))' */
    /* note: if unselected previous state,
      release will change to initially found <space> */
  temp = &tbuf->release;
  offset = (int32_t)(cPtr - tbuf->string);
  if ( (shift_click && (offset > tbuf->release.offset))
     || !shift_click ) {
    tbuf->release.offset = offset;
    location_for_offset(tbuf, &tbuf->release);
    temp = &tbuf->release;
  }
    /* parse for backward looking codes to place insert */
  if ((iPtr - 1) >= tbuf->string) {
    cPtr = iPtr;
    do {
      cdx = -1;
      uPtr = utf.u8code;
      utf.u32code = 0;
      ch0 = cPtr[cdx];
      do {
        *uPtr = ch0;
        if ((ch0 & 0x0C0) != 0x080)  break;
        if (((cdx - 1) < -4) || (&cPtr[(cdx - 1)] < tbuf->string))  break;
          /* safe to retrieve utf8 code */
        ch0 = cPtr[(--cdx)];
          /* make room to assign on loop */
        utf.u32code <<= 8;
      } while (1);
      if (iswspace(utf.u32code) == 0)  break;
      cPtr += cdx;
    } while (1);
      /* if <space> code found, assign insert */
    if (cPtr != iPtr) {
        /* default covers '(shift_click && (offset == tbuf->insert.offset))' */
      temp = &tbuf->insert;
      offset = (int32_t)(cPtr - tbuf->string);
      if ( (shift_click && (offset < tbuf->insert.offset))
         || !shift_click ) {
        tbuf->insert.offset = offset;
        location_for_offset(tbuf, &tbuf->insert);
        temp = &tbuf->insert;
      }
    }
  }
  tbuf->interim = *temp;
  location_auto_scroll(tbuf, temp);
  return;

palnum:
  do {
    if (ch0 == '_') {  ch0 = *(++cPtr); continue;  }
    cdx = 0;
    uPtr = utf.u8code;
    utf.u32code = 0;
    *uPtr = ch0, uPtr++, cdx++;
    if ((ch0 & (char)0x0C0) == (char)0x0C0) {
      while ( (cdx < 4)
               && (((ch0 = cPtr[cdx]) & (char)0x0C0) == (char)0x080) )
        *uPtr = ch0, uPtr++, cdx++;
    }
    if ( (ch0 == 0) || (iswalnum(utf.u32code) == 0) )  break;
    ch0 = *(cPtr += cdx);
  } while (1);
    /* assign release, default is to be release */
  temp = &tbuf->release;
  offset = (int32_t)(cPtr - tbuf->string);
  if ( (shift_click && (offset > tbuf->release.offset))
     || !shift_click ) {
    tbuf->release.offset = offset;
    location_for_offset(tbuf, &tbuf->release);
    temp = &tbuf->release;
  }
    /* parse for backward looking codes to place insert */
  if ((iPtr - 1) >= tbuf->string) {
    cPtr = iPtr;
    do {
      cdx = -1;
      uPtr = utf.u8code;
      utf.u32code = 0;
      ch0 = cPtr[cdx];
      if (ch0 == '_') {
        if (((--cPtr) - 1) < tbuf->string)  break;
        continue;
      }
      do {
        *uPtr = ch0;
        if ((ch0 & 0x0C0) != 0x080)  break;
        if (((cdx - 1) < -4) || (&cPtr[(cdx - 1)] < tbuf->string))  break;
          /* safe to retrieve utf8 code */
        ch0 = cPtr[(--cdx)];
          /* make room to assign on loop */
        utf.u32code <<= 8;
      } while (1);
      if (iswalnum(utf.u32code) == 0)  break;
      cPtr += cdx;
    } while (1);
      /* if <space> code found, assign insert */
    if (cPtr != iPtr) {
        /* default covers '(shift_click && (offset == tbuf->insert.offset))' */
      temp = &tbuf->insert;
      offset = (int32_t)(cPtr - tbuf->string);
      if ( (shift_click && (offset < tbuf->insert.offset))
         || !shift_click ) {
        tbuf->insert.offset = offset;
        location_for_offset(tbuf, &tbuf->insert);
        temp = &tbuf->insert;
      }
    }
  }
  tbuf->interim = *temp;
  location_auto_scroll(tbuf, temp);
  return;

ppunct:
  do {
    cdx = 0;
    uPtr = utf.u8code;
    utf.u32code = 0;
    *uPtr = ch0, uPtr++, cdx++;
    if ((ch0 & (char)0x0C0) == (char)0x0C0) {
      while ( (cdx < 4)
               && (((ch0 = cPtr[cdx]) & (char)0x0C0) == (char)0x080) )
        *uPtr = ch0, uPtr++, cdx++;
    }
    if ( (ch0 == 0) || (iswpunct(utf.u32code) == 0) )  break;
    ch0 = *(cPtr += cdx);
  } while (1);
    /* assign release, default is to be release */
    /* default covers '(shift_click && (offset == tbuf->release.offset))' */
    /* note: if unselected previous state,
      release will change to initially found <space> */
  temp = &tbuf->release;
  offset = (int32_t)(cPtr - tbuf->string);
  if ( (shift_click && (offset > tbuf->release.offset))
     || !shift_click ) {
    tbuf->release.offset = offset;
    location_for_offset(tbuf, &tbuf->release);
    temp = &tbuf->release;
  }
    /* parse for backward looking codes to place insert */
  if ((iPtr - 1) >= tbuf->string) {
    cPtr = iPtr;
    do {
      cdx = -1;
      uPtr = utf.u8code;
      utf.u32code = 0;
      ch0 = cPtr[cdx];
      do {
        *uPtr = ch0;
        if ((ch0 & 0x0C0) != 0x080)  break;
        if (((cdx - 1) < -4) || (&cPtr[(cdx - 1)] < tbuf->string))  break;
          /* safe to retrieve utf8 code */
        ch0 = cPtr[(--cdx)];
          /* make room to assign on loop */
        utf.u32code <<= 8;
      } while (1);
      if (iswpunct(utf.u32code) == 0)  break;
      cPtr += cdx;
    } while (1);
      /* if <space> code found, assign insert */
    if (cPtr != iPtr) {
        /* default covers '(shift_click && (offset == tbuf->insert.offset))' */
      temp = &tbuf->insert;
      offset = (int32_t)(cPtr - tbuf->string);
      if ( (shift_click && (offset < tbuf->insert.offset))
         || !shift_click ) {
        tbuf->insert.offset = offset;
        location_for_offset(tbuf, &tbuf->insert);
        temp = &tbuf->insert;
      }
    }
  }
  tbuf->interim = *temp;
  location_auto_scroll(tbuf, temp);
  return;
}

void
location_triple_click(PhxTextbuffer *tbuf, int16_t x, int16_t y,
                                                 bool shift_click) {
  location temp;
  char *nPtr;
  int32_t offsetI, offsetR;

  temp.x = x + tbuf->bin.x;
  temp.y = y + tbuf->bin.y;
  location_for_point(tbuf, &temp);

  nPtr = strchr(&tbuf->string[temp.offset], '\n');
  offsetR = (int32_t)((nPtr + 1) - tbuf->string);
  if (nPtr == NULL)  offsetR = tbuf->str_nil;

  nPtr = memrchr(tbuf->string, '\n', temp.offset);
  nPtr = (nPtr == NULL) ? tbuf->string : (nPtr + 1);
  offsetI = (int32_t)(nPtr - tbuf->string);

  if ( (shift_click && (offsetR > tbuf->release.offset))
      || !shift_click ) {
    tbuf->release.offset = offsetR;
    location_for_offset(tbuf, &tbuf->release);
    temp = tbuf->release;
  }
  if ( (shift_click && (offsetI < tbuf->insert.offset))
      || !shift_click ) {
    tbuf->insert.offset = offsetI;
    location_for_offset(tbuf, &tbuf->insert);
    temp = tbuf->insert;
  }
  tbuf->interim = temp;
}

/* horizontal scroll is based on amount longest line is beyond
   bin's width. */
static int16_t
_longest_x_advance(PhxTextbuffer *tbuf) {

  uint32_t oStrt, oEnd;
  uint16_t max_advance;
  const char *rdPtr;
  PhxAttr *attrib = tbuf->owner->attrib;
  cairo_surface_t *drawable;
  cairo_t *cr;
  phx_newline_t *ndata;
  PhxMarkData *pPtr;

  drawable = tbuf->owner->i_mount->surface;
  DEBUG_ASSERT((drawable == NULL), "SEGFAULT: _longest_x_advance()");
  cr = cairo_create(drawable);

  ndata = tbuf->mark_list[0]->data;
  pPtr = ndata->_results.pairs;

  max_advance = 0;
  rdPtr = (const char*)tbuf->string;
  oEnd = 0;
  do {
    uint32_t advance, line_length;
    const char *sPtr;
    oStrt = oEnd;
    oEnd = (++pPtr)->d0;
    if (oEnd == (uint32_t)~0)  break;
    sPtr = &rdPtr[oStrt];
    line_length = oEnd - oStrt;
    advance = ext_cairo_glyphs_advance((char**)&sPtr,
                              line_length, cr,
                              attrib->font_name,
                              attrib->font_var & 0x0FF,
                             (attrib->font_var >> 8) & 0x0FF,
                              attrib->font_size);
    if (advance > max_advance)  max_advance = advance;
  } while (1);
  cairo_destroy(cr);
  return max_advance;
}

static int32_t
_longest_y_advance(PhxTextbuffer *tbuf, uint16_t font_em) {

  phx_newline_t *ndata = tbuf->mark_list[0]->data;
  PhxMarkData *pPtr = ndata->_results.pairs + ndata->_results.ncount - 1;
    /* .line starts with 0, need + 1 */
  return (int32_t)(((pPtr->d1) * font_em) + font_em);
}

void
location_scroll_click(PhxTextbuffer *tbuf, int16_t x, int16_t y) {

  uint16_t font_em = tbuf->owner->attrib->font_em;

    /* Needed to piece back for advance measurements.
      Also not included at called point in mouse(). */
  _textbuffer_flush(tbuf);

  if (x != 0) {
    if (x > 0) {
        /* For editing, always allow 1 char block past width */
      int16_t width = _longest_x_advance(tbuf);
      if (tbuf->bin.w >= width)  return;
      if (((tbuf->bin.x + tbuf->bin.w) + x) >= width) {
        int16_t delta = width - (tbuf->bin.x + tbuf->bin.w);
        if (delta < 0)  delta = 0;
        x = delta;
      }
    } else if ((tbuf->bin.x + x) < 0)  x = -tbuf->bin.x;
    tbuf->bin.x += x;
    return;
  }

  if (y != 0) {
    int32_t y_propose;
    if (y > 0) {
        /* moving text upward, expose more at bottom  SCROLL_DOWN */
      int32_t height = _longest_y_advance(tbuf, font_em);
        /* ceiling of last exposed line */
      y_propose = y + tbuf->bin.y + tbuf->bin.h + (font_em - 1);
      y_propose = ((uint32_t)y_propose / font_em) * font_em;
      if (y_propose > height)
        y_propose = height - (tbuf->bin.y + tbuf->bin.h);
      if (y_propose > 0)
        tbuf->bin.y += y_propose - (tbuf->bin.y + tbuf->bin.h);
      return;
    }
      /* moving text downward, expose more at topline   SCROLL_UP */
    y_propose = tbuf->bin.y + y;
    if (y_propose < 0)
          tbuf->bin.y = 0;
    else  tbuf->bin.y = ((uint32_t)y_propose / font_em) * font_em;
  }
}

void
location_select_all(PhxTextbuffer *tbuf) {

  _textbuffer_flush(tbuf);
  tbuf->insert.offset = 0;
  tbuf->release.offset = tbuf->str_nil;
  location_for_offset(tbuf, &tbuf->insert);
  location_for_offset(tbuf, &tbuf->release);
  ui_invalidate_object(tbuf->owner);
}

/* Seperate because parses buffer for movement. This is similar
  to location_[double/triple]_clicks(). */
static bool
location_key_cntl_motion(PhxTextbuffer *tbuf,
                         location *active,
                         location *inactive,
                         int16_t eflag) {
  int16_t action;
  bool shift_click;

  int32_t cdx, offset;
  char ch0, *cPtr;
  union { uint8_t u8code[4]; uint32_t u32code; } utf;
  uint8_t *uPtr;

  shift_click = ((eflag & XCB_MOD_MASK_SHIFT) != 0);
  ch0 = *(cPtr = &tbuf->string[active->offset]);

  action = eflag >> 4;
  if ((action & 3) != 0)  return false;

  if (action < 0) {  /* XK_Left */
    do {
      if ((cPtr - 1) < tbuf->string)  return true;
      if ((ch0 = cPtr[(cdx = -1)]) == (char)'_')  break;
      uPtr = utf.u8code;
      utf.u32code = 0;
      do {
        *uPtr = ch0;
        if ((ch0 & 0x0C0) != 0x080)  break;
        if (((cdx - 1) < -4) || (&cPtr[(cdx - 1)] < tbuf->string))  break;
          /* safe to retrieve utf8 code */
        ch0 = cPtr[(--cdx)];
          /* make room to assign on loop */
        utf.u32code <<= 8;
      } while (1);
      if (iswalnum(utf.u32code) != 0)  break;
      cPtr += cdx;
    } while (1);
    if ((cPtr - 1) < tbuf->string)  return true;
    do {
      if ((ch0 = cPtr[(cdx = -1)]) == (char)'_') {
        if (((--cPtr) - 1) < tbuf->string)  break;
        continue;
      }
      uPtr = utf.u8code;
      utf.u32code = 0;
      do {
        *uPtr = ch0;
        if ((ch0 & 0x0C0) != 0x080)  break;
        if (((cdx - 1) < -4) || (&cPtr[(cdx - 1)] < tbuf->string))  break;
          /* safe to retrieve utf8 code */
        ch0 = cPtr[(--cdx)];
          /* make room to assign on loop */
        utf.u32code <<= 8;
      } while (1);
      if (iswalnum(utf.u32code) == 0)  break;
    } while (((cPtr += cdx) - 1) >= tbuf->string);
    offset = (int32_t)(cPtr - tbuf->string);
    if ( (shift_click && (offset < active->offset))
        || !shift_click )
    goto assign_active;
  } else {           /* XK_Right */
    do {
      if (ch0 == 0)  return true;
      if (ch0 == '_')  break;
      uPtr = utf.u8code;
      utf.u32code = 0;
      *uPtr = ch0, uPtr++, cdx = 1;
      if ((ch0 & (char)0x0C0) == (char)0x0C0) {
        while ( (cdx < 4)
                 && (((ch0 = cPtr[cdx]) & (char)0x0C0) == (char)0x080) )
          *uPtr = ch0, uPtr++, cdx++;
      }
      if (iswalnum(utf.u32code) != 0)  break;
      ch0 = *(cPtr += cdx);
    } while (1);
    ch0 = *cPtr;
    do {
      if (ch0 == '_') {
        if ((ch0 = *(++cPtr)) == 0)  break;
        continue;
      }
      uPtr = utf.u8code;
      utf.u32code = 0;
      *uPtr = ch0, uPtr++, cdx = 1;
      if ((ch0 & (char)0x0C0) == (char)0x0C0) {
        while ( (cdx < 4)
                 && (((ch0 = cPtr[cdx]) & (char)0x0C0) == (char)0x080) )
          *uPtr = ch0, uPtr++, cdx++;
      }
      if (iswalnum(utf.u32code) == 0)  break;
        /* Note: inside loop because of <underscore>'s continue. */
      if ((ch0 = *(cPtr += cdx)) == 0)  break;
    } while (1);
    offset = (int32_t)(cPtr - tbuf->string);
    if ( (shift_click && (offset > active->offset))
        || !shift_click ) {
assign_active:
      active->offset = offset;
      location_for_offset(tbuf, active);
      location_auto_scroll(tbuf, active);
    }
  }
  tbuf->interim = *active;
  if (!shift_click || (inactive->offset == active->offset))
    *inactive = *active;
  return true;
}

void
location_key_motion(PhxTextbuffer *tbuf, int16_t eflag) {

  location *active, *inactive;
  int16_t action, font_em;

  _textbuffer_flush(tbuf);
  font_em = tbuf->owner->attrib->font_em;

  action = eflag >> 4;
  if ((action & 3) == 3)  goto bin_scroll;

    /* deselection of selection */
  if ( ((eflag & 0x000F) == 0)
      && (tbuf->insert.offset != tbuf->release.offset) ) {
    if (action < 0)
          active = &tbuf->insert, inactive = &tbuf->release;
    else  active = &tbuf->release, inactive = &tbuf->insert;
    tbuf->interim = (*inactive = *active);
    location_auto_scroll(tbuf, active);
    return;
  }
  active = &tbuf->release, inactive = &tbuf->insert;
  if (tbuf->interim.offset == tbuf->insert.offset) {
    active = &tbuf->insert, inactive = &tbuf->release;
    if ( (tbuf->interim.x == tbuf->insert.x)
        && (tbuf->interim.y == tbuf->insert.y) ) {
NWSE:   if (action < 0)
            active = &tbuf->insert, inactive = &tbuf->release;
      else  active = &tbuf->release, inactive = &tbuf->insert;
      tbuf->interim = *active;
    }
  } else if ( (tbuf->interim.x == tbuf->release.x)
             && (tbuf->interim.y == tbuf->release.y) ) {
    goto NWSE;
  }

  if (eflag & XCB_MOD_MASK_CONTROL) {
    if (location_key_cntl_motion(tbuf, active, inactive, eflag))
      return;
  }

  if ((action & 3) == 0) {
    if (action < 0) {  /* XK_Left */
      if (active->offset != 0)
        while ((tbuf->string[(--active->offset)] & 0x0C0) == 0x080)
          if (active->offset == 0)  break;
    } else {           /* XK_Right */
      if (tbuf->string[active->offset] != 0)
        while ((tbuf->string[(++active->offset)] & 0x0C0) == 0x080)
          if (tbuf->string[(active->offset + 1)] == 0)  break;
    }
    location_for_offset(tbuf, active);
    location_auto_scroll(tbuf, active);
horiz_motion:
    tbuf->interim = *active;
    if ( ((eflag & XCB_MOD_MASK_SHIFT) == 0)
        || (inactive->offset == active->offset) )
      *inactive = *active;
    return;
  }

  if ((action & 3) == 1) {
    if (action < 0) {  /* XK_Up */
      if (active->y == 0)  goto vert_motion;
      active->y -= font_em;
    } else {           /* XK_Down */
      active->y += font_em;
    }
vert_motion:
    active->x = tbuf->interim.x;
    location_for_point(tbuf, active);
    location_auto_scroll(tbuf, active);/**/
    if ( ((eflag & XCB_MOD_MASK_SHIFT) != 0)
        && (inactive->offset != active->offset) )  {
      tbuf->interim.y = active->y;
      tbuf->interim.offset = active->offset;
    } else {
      inactive->x = active->x;
      tbuf->interim.y = (inactive->y = active->y);
      tbuf->interim.offset = (inactive->offset = active->offset);
    }
    return;
  }

  if ((action & 3) == 2) {
    char *nPtr;
    if (action < 0) {  /* XK_Home */
      if (active->offset != 0) {
        nPtr = memrchr(tbuf->string, '\n', active->offset);
        nPtr = (nPtr == NULL) ? tbuf->string : (nPtr + 1);
        active->offset = (int)(nPtr - tbuf->string);
          /* auto-scrolls 'tbuf->bin.x' amount (reduced for begin only) */
        active->x = 0;
        tbuf->bin.x = 0;
      }
    } else {           /* XK_End */
      int16_t x;
      nPtr = strchr(&tbuf->string[active->offset], '\n');
      active->offset = (int)(nPtr - tbuf->string);
      if (nPtr == NULL)  active->offset = tbuf->str_nil;
        /* sum of glyph advances */
      location_for_offset(tbuf, active);
        /* x auto-scroll (reduced for end only) */
      x = active->x + tbuf->bin.x;
      if (x >= (tbuf->bin.w - font_em))
        tbuf->bin.x += x - (tbuf->bin.w - font_em);
    }
    goto horiz_motion;
  }

    /* These do no cursor movement, just bin adjust. */
bin_scroll:
  if (eflag & XCB_MOD_MASK_CONTROL) {
    bool shift_click = ((eflag & XCB_MOD_MASK_SHIFT) != 0);
    if (action < 0) {  /* XK_Page_Up */
      tbuf->bin.y = 0;
      if (shift_click) {
        tbuf->interim.x = (tbuf->interim.y = (tbuf->interim.offset = 0));
        tbuf->insert = tbuf->interim;
      }
    } else {           /* XK_Page_Down */
      location temp;
      temp.offset = tbuf->str_nil;
      location_for_offset(tbuf, &temp);
      if (shift_click)
        tbuf->release = (tbuf->interim = temp);
      temp.y -= tbuf->bin.h - font_em;
      if (temp.y < 0)  temp.y = 0;
      tbuf->bin.y = temp.y;
    }
    return;
  }
  if (action < 0) {  /* XK_Page_Up */
    if (tbuf->bin.y != 0) {
        /* check for case of less than two line viewing */
      if (tbuf->bin.h < (font_em << 1)) {
        tbuf->bin.y -= font_em;
      } else {
          /* move partial exposed below view */
        int delta = tbuf->bin.y - ((tbuf->bin.y / font_em) * font_em);
        if (delta != 0)  tbuf->bin.y -= delta - font_em;
          /* move full viewed top line to bottom of viewed page */
        tbuf->bin.y -= tbuf->bin.h - font_em;
        delta = tbuf->bin.h - ((tbuf->bin.h / font_em) * font_em);
        if (delta != font_em)  tbuf->bin.y += delta;
      }
      if (tbuf->bin.y < 0)  tbuf->bin.y = 0;
    }
  } else {           /* XK_Page_Down */
    location temp;
    temp.offset = tbuf->str_nil;
    location_for_offset(tbuf, &temp);
    if ((tbuf->bin.y + tbuf->bin.h - font_em) <= temp.y) {
      if (tbuf->bin.h < (font_em << 1))
            tbuf->bin.y += font_em;
      else  tbuf->bin.y += ((tbuf->bin.h / font_em) * font_em) - font_em;
    }
  }
}

void
location_interim_equalize(PhxTextbuffer *tbuf, int32_t x, int32_t y) {

    /* x, y in system of draw_bow = 0,0 */
  tbuf->interim.x = x + tbuf->bin.x;
  tbuf->interim.y = y + tbuf->bin.y;
  location_for_point(tbuf, &tbuf->interim);
  location_auto_scroll(tbuf, &tbuf->interim);

    /* if clicked in selection do nothing until motion or release */
  if ( (tbuf->insert.offset != tbuf->release.offset)
      && ( (tbuf->interim.offset >= tbuf->insert.offset)
          && (tbuf->interim.offset <= tbuf->release.offset)) )
    return;

  tbuf->insert = (tbuf->release = tbuf->interim);
}

void
location_selection_equalize(PhxTextbuffer *tbuf) {

  if (tbuf->insert.offset != tbuf->release.offset)
    tbuf->insert = (tbuf->release = tbuf->interim);
}

bool
location_drag_begin(PhxTextbuffer *tbuf, int16_t x, int16_t y) {

  if (tbuf->insert.offset != tbuf->release.offset) {
      /* need to test for special case, selection is of 1 character
       with user wanting to drag intead of selection. We need to test
       insert.x < motion's x < release.x. */
    if ((tbuf->release.offset - tbuf->insert.offset) == 1) {
      location temp;
      temp.x = x, temp.y = y;
      location_for_point(tbuf, &temp);
      if ( (tbuf->insert.offset == temp.offset)
          || (tbuf->release.offset == temp.offset) )
        goto begin_drag;
      return false;
    }
    if ( ( (tbuf->interim.offset >= tbuf->insert.offset)
          && (tbuf->interim.offset != tbuf->release.offset) )
        && ( (tbuf->interim.offset <= tbuf->release.offset)
            && (tbuf->interim.offset != tbuf->insert.offset) ) ) {
  begin_drag:
      tbuf->drop = tbuf->interim;
      tbuf->drag_sbin = tbuf->bin;
      return true;
    }
  }
  return false;
}

/* reset textbuffer prestine, a c-string followed by gap area */
void
_textbuffer_flush(PhxTextbuffer *tbuf) {

  if ((tbuf->str_nil + 1) != tbuf->gap_start) {
    int gapSz;
      /* Added to allow marks to refresh. */
    phxmarks_dirty_offset(tbuf, tbuf->gap_start);
    memmove(&tbuf->string[tbuf->gap_start], &tbuf->string[tbuf->gap_end],
                                  (size_t)(tbuf->str_nil + 1 - tbuf->gap_end));
    gapSz = tbuf->gap_end - tbuf->gap_start;
    tbuf->str_nil -= gapSz;
    tbuf->gap_start = tbuf->str_nil + 1;
    tbuf->gap_end = tbuf->gap_start + gapSz;
    tbuf->gap_delta = 0;
  }
  phxmarks_update(tbuf);
}

/* check if gap size has enough editing area. resize if needed
  return gap size or -1 on error */
static int
_textbuffer_gap_size(PhxTextbuffer *tbuf, int sz) {

  int gapSz = tbuf->gap_end - tbuf->gap_start;
  if (sz >= gapSz) {
    int addSz;
    size_t newSz;
    char *newPtr;
      /* make sure gap at end */
    _textbuffer_flush(tbuf);
    addSz = (sz + TGAP_ALLOC) & ~(TGAP_ALLOC - 1);
    newSz = tbuf->str_nil + (gapSz += addSz);
    newPtr = realloc(tbuf->string, newSz);
    if (newPtr == NULL)  return -1;
    tbuf->string = newPtr;
    memset(&newPtr[tbuf->str_nil], 0, gapSz);
    tbuf->gap_end = tbuf->gap_start + gapSz;
  }
  return gapSz;
}

/* companion to ui_text_buffer_replace().
 * With replace, sometimes don't set to editing location.
 * This allows one to set that spot.
 * ui_text_buffer_insert() always will set up editing. */
void
_textbuffer_edit_set(PhxTextbuffer *tbuf, int32_t offset) {

  tbuf->insert.offset = offset;
  location_for_offset(tbuf, &tbuf->insert);
  location_auto_scroll(tbuf, &tbuf->insert);
  tbuf->interim = (tbuf->release = tbuf->insert);
}

void
_textbuffer_replace(PhxTextbuffer *tbuf, char *data, int32_t sz) {

  int32_t selSz, delta, gapSz;
  selSz = tbuf->release.offset - tbuf->insert.offset;
  if ((sz < 0) || (selSz == 0))  return;

  delta = (sz - selSz);
  gapSz = _textbuffer_gap_size(tbuf, delta);
  if (gapSz < 0)  return;

    /* 3 possible scenerios if gap_end < str_nil:
         insert before gap_start && release after gap_end
         insert && release before gap_start
         insert && release after gap_end
       simplicity sake use: */
  _textbuffer_flush(tbuf);

    /* open space for replace (non-editing movement) */
  memmove(&tbuf->string[(tbuf->insert.offset + sz)],
          &tbuf->string[tbuf->release.offset],
          (tbuf->str_nil - tbuf->release.offset));
  if (sz != 0)
    memmove(&tbuf->string[tbuf->insert.offset], data, sz);
  tbuf->string[(tbuf->str_nil += delta)] = 0;
  tbuf->gap_start = tbuf->str_nil + 1;
  tbuf->gap_delta = 0;

    /* inform marks that buffer changed */
  phxmarks_dirty_offset(tbuf, tbuf->insert.offset);
}

/* Insert, in addition to normal insert, can do insert into
 * a selection. Normally that is considered a delete than insert.
 * The delete/insert ability is also needed for 'insert' mode.
 * Instead it currently moves gap_end.
 */
void
_textbuffer_insert(PhxTextbuffer *tbuf, char *data, int sz) {

  int selSz, gapSz;

    /* check if inserting into a selection */
  selSz = tbuf->release.offset - tbuf->insert.offset;
  if ((sz == 0) && (selSz == 0))  return;

  gapSz = _textbuffer_gap_size(tbuf, (sz - selSz));
  if (gapSz < 0)  return;

    /* determine if selection has <newline>, 'force update' of marks if so */
  if (selSz != 0) {
    if (memchr(&tbuf->string[tbuf->insert.offset], '\n', selSz) != NULL)
      phxmarks_dirty_line_set(tbuf);
  } else if ((tbuf->state & TBIT_KEY_INSERT) != 0) {
    if (tbuf->string[tbuf->insert.offset] == '\n')
      phxmarks_dirty_line_set(tbuf);
  }

    /* case: buffer reset not used */
  if (tbuf->gap_end < tbuf->str_nil) {
    int delta = tbuf->gap_start - tbuf->release.offset;
    if (delta >= 0) {
      memmove(&tbuf->string[(tbuf->gap_end - delta)],
              &tbuf->string[tbuf->release.offset], delta);
    } else {
      memmove(&tbuf->string[tbuf->gap_start],
              &tbuf->string[tbuf->gap_end],
                     tbuf->gap_end - tbuf->insert.offset);
    }
    goto moved;
  }
    /* if editing at end, don't move gap but move/add nil byte and gap_start */
  if ( (tbuf->release.offset != tbuf->str_nil)
      && (tbuf->insert.offset != tbuf->gap_start) ) {
    memmove(&tbuf->string[(tbuf->release.offset + gapSz)],
            &tbuf->string[tbuf->release.offset],
              (tbuf->str_nil + 1 - tbuf->release.offset));
    tbuf->str_nil += gapSz;
moved:
    tbuf->gap_delta = selSz;
    tbuf->gap_end = tbuf->release.offset + gapSz;
  }
  if (sz == 1)
    tbuf->string[tbuf->insert.offset] = *data;
  else
    memmove(&tbuf->string[tbuf->insert.offset], data, sz);

  phxmarks_dirty_offset(tbuf, tbuf->insert.offset);
  tbuf->gap_start = tbuf->insert.offset + sz;
  if (tbuf->release.offset == tbuf->str_nil) {
    tbuf->str_nil += (sz - selSz);
    tbuf->string[tbuf->str_nil] = 0;
    tbuf->gap_start = tbuf->str_nil + 1;
  } else if ((tbuf->state & TBIT_KEY_INSERT) != 0) {
/* insert mode
         if selection in insert mode, becomes replace seletion with char
         insert offset does not change
         if no selection, insert offset changes, as does gap_start
         deletion removes chars, which changes gap_end's position
         gap_delta is amount the gap changes, used to determine
         positions for get display */
    if (selSz != 0)  sz = 0;
    tbuf->gap_delta += sz;
    tbuf->gap_end += sz;
  }
  location_for_edit_update(tbuf, sz);
}

/* For selection, use release instead of insert. */
/* As with ui_text_buffer_insert(), an action, other then editing,
   will 'reset' buffer. */
void
_textbuffer_delete(PhxTextbuffer *tbuf) {

    /* get info on selection */
  int sz = tbuf->release.offset - tbuf->insert.offset;
    /* cases ruled out by size == 0 */
  if (sz == 0) {
      /* nothing to <backspace> */
    if (tbuf->insert.offset == 0)  return;
      /* added for utf8 */
    while ((tbuf->string[(tbuf->insert.offset - 1)] & 0x0C0) == 0x080)
      tbuf->insert.offset--;
  }
    /* debugging verify of selection reversal 'no-no' */
  if (sz < 0) {  puts("error, ui_text_buffer_delete()"); return;  }

    /* if editing at end, don't move gap but move/add nil byte and gap_start */
  if (tbuf->release.offset == tbuf->str_nil) {
      /* on 'delete' moves nil sz amount, 0 if sz == 0 */
    if ((tbuf->state & TBIT_KEY_DELETE) != 0) {
        /* remove flag */
      tbuf->state ^= TBIT_KEY_DELETE;
        /* nothing to 'delete' at end of buffer, leave locations as is */
      if (sz == 0)  return;
    }
    location_for_edit_update(tbuf, -(!sz));
    phxmarks_dirty_offset(tbuf, tbuf->insert.offset);
    tbuf->str_nil += -(sz + !sz);
    tbuf->string[tbuf->str_nil] = 0;
    tbuf->gap_start = tbuf->str_nil + 1;
    return;
  }

  if (tbuf->insert.offset != tbuf->gap_start) {
    int gapSz = tbuf->gap_end - tbuf->gap_start;
      /* move release the distance of gapSz, include nil with move */
    memmove(&tbuf->string[(tbuf->release.offset + gapSz)],
            &tbuf->string[tbuf->release.offset],
            (tbuf->gap_start - tbuf->release.offset));
    tbuf->gap_delta = sz;
    tbuf->gap_end = tbuf->release.offset + gapSz;
    tbuf->str_nil += gapSz;
    if (sz != 0) {
      if (memchr(&tbuf->string[tbuf->insert.offset], '\n', sz) != NULL)
        phxmarks_dirty_line_set(tbuf);
    }
  }

  if ((tbuf->state & TBIT_KEY_DELETE) != 0) {
    tbuf->state ^= TBIT_KEY_DELETE;
    if (!sz)  tbuf->gap_end++, tbuf->gap_delta++, sz++;
  }

  tbuf->gap_start = tbuf->insert.offset - !sz;
  location_for_edit_update(tbuf, -(!sz));
  phxmarks_dirty_offset(tbuf, tbuf->insert.offset);
}

void
_textbuffer_drag_release(PhxTextbuffer *tbuf, bool is_move) {

  int32_t sz;

  if (tbuf->interim.offset == tbuf->drop.offset) {
    location_auto_scroll(tbuf, &tbuf->insert);
    return;
  }

  sz = tbuf->release.offset - tbuf->insert.offset;
    /* add 1 to sz for nil at end of copy to gap */
  if (_textbuffer_gap_size(tbuf, (sz + 1)) < 0)  return;

  memmove(&tbuf->string[tbuf->gap_start],
            &tbuf->string[tbuf->insert.offset], (size_t)sz);
    /* make copied a c-string */
  tbuf->string[(tbuf->gap_start + sz)] = 0;

  if (!is_move) {
    tbuf->gap_start += sz;
    memmove(&tbuf->string[(tbuf->drop.offset + sz)],
            &tbuf->string[tbuf->drop.offset],
            (size_t)(tbuf->gap_start - tbuf->drop.offset));
    memmove(&tbuf->string[tbuf->drop.offset],
              &tbuf->string[tbuf->gap_start], (size_t)sz);
    tbuf->str_nil += sz;

    phxmarks_dirty_offset(tbuf, tbuf->drop.offset);
    phxmarks_update(tbuf);
  } else {
    int32_t offset;
    if (tbuf->drop.offset < tbuf->insert.offset) {
      memmove(&tbuf->string[(tbuf->drop.offset + sz)],
              &tbuf->string[tbuf->drop.offset],
              (size_t)(tbuf->insert.offset - tbuf->drop.offset));
      memmove(&tbuf->string[tbuf->drop.offset],
                &tbuf->string[tbuf->gap_start], (size_t)sz);
    } else if (tbuf->drop.offset > tbuf->release.offset) {
      memmove(&tbuf->string[tbuf->insert.offset],
              &tbuf->string[tbuf->release.offset],
              (size_t)(tbuf->drop.offset - tbuf->release.offset));
      memmove(&tbuf->string[(tbuf->drop.offset - sz)],
                &tbuf->string[tbuf->gap_start], (size_t)sz);
    }
    offset = tbuf->drop.offset;
    if (offset > tbuf->insert.offset)
      offset = tbuf->insert.offset;
    phxmarks_dirty_offset(tbuf, offset);
    phxmarks_update(tbuf);
    if (tbuf->drop.offset > tbuf->release.offset)
      tbuf->drop.offset -= sz;
  }

  tbuf->insert.offset = tbuf->drop.offset;
  location_for_offset(tbuf, &tbuf->insert);
  tbuf->release.offset = tbuf->insert.offset + sz;
  location_for_offset(tbuf, &tbuf->release);
  tbuf->interim = tbuf->release;
}

void
_textbuffer_copy(PhxTextbuffer *tbuf) {

  int32_t sz = tbuf->release.offset - tbuf->insert.offset;
  if (sz == 0)  return;
  DEBUG_ASSERT((sz < 0), "trying to copy negative amount.");
    /* Informs ICCCM that clipboard content needs refresh.
       We stored in xcb_selection_data_t. */
  _xclb_copy(session->xclipboard, tbuf->owner->i_mount->window,
                    &tbuf->string[tbuf->insert.offset], sz);
}

void
_textbuffer_cut(PhxTextbuffer *tbuf) {

  int32_t sz = tbuf->release.offset - tbuf->insert.offset;
  if (sz == 0)  return;
  DEBUG_ASSERT((sz < 0), "trying to copy negative amount.");
    /* Informs ICCCM that clipboard content needs refresh.
       We stored in xcb_selection_data_t. */
  _xclb_copy(session->xclipboard, tbuf->owner->i_mount->window,
                    &tbuf->string[tbuf->insert.offset], sz);
    /* Remove content from text_buffer. */
  _textbuffer_delete(tbuf);
}

void
_textbuffer_paste_notify(PhxTextbuffer *tbuf) {

  char *contents;
  int32_t  c_size;
  _xclb_paste_reply(session->xclipboard, &contents, &c_size);
  if (c_size != 0) {
      /* insert used to allow pasted text to be auto-selected */
    location insert = tbuf->insert;
    _textbuffer_insert(tbuf, contents, c_size);
    tbuf->insert = insert;
  }
}

void
_textbuffer_paste(PhxTextbuffer *tbuf) {
   /* Tell owner of clipboard data, we want its contents. */
   /* Will get XCB_SELECTION_NOTIFY when data ready. */
  _xclb_paste_request(session->xclipboard, tbuf->owner->i_mount->window);
}

#pragma mark *** Drawing ***

static char *
_textbuffer_get(PhxTextbuffer *tbuf, int start, int end) {

  char *dst;
  size_t sz = end - start;

  if (tbuf->dbSz == 0) {
    size_t dbSz;
dbrealloc:
    dbSz = (sz + (BUFF_ALLOC - 1)) / BUFF_ALLOC;
    tbuf->dbSz = dbSz;
    tbuf->draw_buffer = malloc(dbSz * 4096);
  } else if ((tbuf->dbSz * BUFF_ALLOC) < sz) {
    free(tbuf->draw_buffer);
    goto dbrealloc;
  }

  dst = tbuf->draw_buffer;
    /* translate to storage system */
  if (start >= tbuf->gap_start) {
    start += tbuf->gap_end - tbuf->gap_start;
  } else if (end > tbuf->gap_start) {
      /* split copy, accesses gap points */
    end += tbuf->gap_end - tbuf->gap_start;
    sz = tbuf->gap_start - start;
    memmove(dst, &tbuf->string[start], sz);
    dst += sz;
    start = tbuf->gap_end;
    sz = end - start;
  }
  memmove(dst, &tbuf->string[start], sz);
  dst[sz] = 0;
  return tbuf->draw_buffer;
}

/* Create/Retrieve a buffer based off 'bin' viewing. */
char *
_textbuffer_for_display(PhxTextbuffer *tbuf,
                         location *tempS,
                         location *tempE) {
  phx_newline_t *ndata;
  PhxMarkData *line_mark;
  uint32_t font_em, y;

  if (tbuf->dirty_line != (uint32_t)~0)
    phxmarks_update(tbuf);

  font_em = tbuf->owner->attrib->font_em;
  y = tbuf->bin.y / font_em;
    /* used to place with character offset, instead of mouse click */
  ndata = tbuf->mark_list[0]->data;
  line_mark = ndata->_results.pairs;
  if (y != 0) {
    line_mark = bsearch(&y, ndata->_results.pairs, ndata->_results.ncount + 1,
                                           sizeof(PhxMarkData), _led1_compare);
    if (line_mark == NULL) {
      DEBUG_ASSERT(true, "error: line_mark == NULL");
      return NULL;
    }
    if (line_mark->d0 == (uint32_t)~0)  line_mark--;
  }
  tempS->x      = 0;
  tempS->y      = line_mark->d1 * font_em;
  tempS->offset = line_mark->d0;
  y = (tbuf->bin.y + tbuf->bin.h + font_em) / font_em;
  line_mark = bsearch(&y, line_mark,
                          ndata->_results.ncount + 1 - line_mark->d1,
                          sizeof(PhxMarkData), _led1_compare);
  tempE->x      = 0;  /* unused */
  tempE->y      = line_mark->d1 * font_em;
  if (line_mark->d0 == (uint32_t)~0)
    tempE->offset = tbuf->str_nil;
  else {
    int offset = line_mark->d0;
    if (tbuf->dirty_offset != (uint32_t)~0)
      offset += tbuf->gap_start - tbuf->dirty_offset - tbuf->gap_delta;
    tempE->offset = offset;
  }
  return _textbuffer_get(tbuf, tempS->offset, tempE->offset);
}

/* Cast as PhxObjectTextview. It has the correct object layout. */
void
_default_textbuffer_draw(PhxObject *b, cairo_t *cr) {

  PhxTextbuffer *tbuf;
  PhxAttr *attrib;
  PhxRGBA *colour;
  unsigned char *tPtr, *nPtr, ch0;
  char *draw_buffer;
  location tempS, tempE;
  double font_em, glyph_origin, tab, alpha;
  double x0, y0, endline, caret_width;
  bool has_focus = (ui_active_focus_get() == b);

  attrib = b->attrib;
  font_em = attrib->font_em;

  tbuf = (PhxTextbuffer*)b->exclusive;
  draw_buffer = _textbuffer_for_display(tbuf, &tempS, &tempE);
    /* for external mark drawing */
  tbuf->draw_top_offset = tempS.offset;
  tbuf->draw_end_offset = tempE.offset;

    /* Use locations for verify draw of selection and search marks */
  if ( (tempS.offset < tbuf->release.offset)
      && (tempE.offset > tbuf->insert.offset)
      && (tbuf->insert.offset != tbuf->release.offset) ) {

    double x1, y1;
    x0 = tbuf->insert.x - tbuf->bin.x;
    y0 = tbuf->insert.y - tbuf->bin.y;
    x1 = tbuf->release.x - tbuf->bin.x;
    y1 = tbuf->release.y - tbuf->bin.y;

    alpha = 0.8;
    if (!has_focus)  alpha = 0.4;
    cairo_set_source_rgba(cr, RGBA_SELECTION.r,
                              RGBA_SELECTION.g,
                              RGBA_SELECTION.b, alpha);
    if (y0 == y1) {
        /* single line selection rectangle */
      cairo_rectangle(cr, x0, y0, (x1 - x0), font_em);
    } else {
      if (x0 < 0)  x0 = 0;
      cairo_rectangle(cr, x0, y0, b->draw_box.w, font_em);
      if ((y0 += font_em) < y1)
        cairo_rectangle(cr, 0, y0, b->draw_box.w, (y1 - y0));
      if (x1 > tbuf->bin.x)
        cairo_rectangle(cr, 0, y1, x1, font_em);
    }
    cairo_fill(cr);
  }

    /* draw in search marks */
  phxmarks_draw(tbuf, cr);

    /* close selection */
  cairo_new_sub_path(cr);

    /* Set basics */
  cairo_select_font_face(cr, attrib->font_name,
                             attrib->font_var & 0x0FF,
                            (attrib->font_var >> 8) & 0x0FF);
  cairo_set_font_size(cr, attrib->font_size);

  colour = &attrib->fg_ink;
  alpha = colour->a;
  if (!has_focus)  alpha *= 0.75;
  cairo_set_source_rgba(cr, colour->r, colour->g, colour->b, alpha);
  glyph_origin = attrib->font_origin;

    /* Create image of text for current view location. */
    /* 'temp' returns are origins 0,0 of glyph draw boxes, based off offsets. */
  tab = tbuf->glyph_widths[0x20] * TAB_TEXT_WIDTH;
    /* set loop variables */
  ch0 = *(nPtr = (tPtr = (unsigned char*)draw_buffer));
  x0 = -tbuf->bin.x;
  y0 = glyph_origin + (tempS.y - tbuf->bin.y);
  endline = tempE.y + font_em;
    /* draw text */
  do {
    cairo_move_to(cr, x0, y0);
rescan:
    while (ch0 >= 0x020)  ch0 = *(++nPtr);
    ext_cairo_show_glyphs((char**)&tPtr, (nPtr - tPtr),
                          cr, attrib->font_name,
                              attrib->font_var & 0x0FF,
                             (attrib->font_var >> 8) & 0x0FF,
                              attrib->font_size);
    if (ch0 == 0)  break;
    if (ch0 != '\n') {
      if (ch0 == '\t')  cairo_rel_move_to(cr, tab, 0);
      /*else if (ch == '\r') */
      /*else if (ch == '\f') */
      ch0 = *(tPtr = (++nPtr));
      goto rescan;
    }
    if ((y0 += font_em) >= endline)  break;
    ch0 = *(tPtr = (++nPtr));
  } while (1);

         /* caret/selection section */

    /* close text drawing */
  cairo_new_sub_path(cr);

    /* Caret has 2 locations, editing and 'drop'. OBIT_DND from 'b'.
      It can also have 2 states, normal and insert mode. */
  x0 = tbuf->insert.x;
  y0 = tbuf->insert.y;
  caret_width = 0.5;
  if ((b->state & OBIT_DND) != 0) {
    x0 = tbuf->drop.x;
    y0 = tbuf->drop.y;
  } else if ((tbuf->state & TBIT_KEY_INSERT) != 0) {
    char *query = &draw_buffer[(tbuf->insert.offset - tempS.offset)];
    ch0 = *query;
    if ((char)ch0 < 0)
      caret_width = ui_textual_glyph_advance(tbuf->owner, &query, 1);
    else
      caret_width = tbuf->glyph_widths[((int)ch0)];
    if (caret_width == 0)  caret_width = 2;
    alpha *= 0.5;
  }
  x0 -= tbuf->bin.x;
  y0 -= tbuf->bin.y;
  if (tbuf->insert.offset == tbuf->release.offset)  {
    cairo_set_source_rgba(cr, 0, 0, 0, alpha);
    cairo_rectangle(cr, x0, y0, caret_width, font_em);
    cairo_fill(cr);
    return;
  }
         /* selection section */

  if ((b->state & OBIT_DND_CARET) != 0) {
      /* During dnd, caret draws when outside a selection. */
    cairo_set_source_rgba(cr, 0, 0, 0, 1);
    cairo_rectangle(cr, x0, y0, caret_width, font_em);
    cairo_fill(cr);
  }

}

#pragma mark *** Creation ***

void
_default_textbuffer_raze(void *obj) {

  PhxTextbuffer *tbuf;
  if ((tbuf = (PhxTextbuffer*)obj) != NULL) {
    free(tbuf->draw_buffer);   tbuf->draw_buffer = NULL;
    phxmarks_raze(tbuf);
    free(tbuf->mark_list);     tbuf->mark_list = NULL;
    free(tbuf->glyph_widths);  tbuf->glyph_widths = NULL;
    free(tbuf->string);        tbuf->string = NULL;
  }
}

/* Calculates advance of 'byte_count' bytes or until <newline> or <nil>. */
int16_t
ui_textual_glyph_advance(PhxObject *obj,
                         char **stream,
                         int16_t byte_count) {

  cairo_t *cr = NULL;
  char *glyph_table;
  int16_t advance = 0;
  bool has_table = (obj->type == PHX_TEXTVIEW);

  if (has_table)
    glyph_table = ((PhxTextbuffer*)obj->exclusive)->glyph_widths;
  else
    glyph_table = ui_textual_glyph_table(obj);

  while (byte_count > 0) {
    char ch0 = *(*stream);
    if (ch0 >= 0) {
      (*stream) += 1;
      advance += glyph_table[(unsigned)ch0];
      if ( (ch0 == 0) || (ch0 == '\n') || ((--byte_count) == 0) )
        break;
    } else {
      char *rdPtr = *stream;
      if (cr == NULL) {
        cairo_surface_t *drawable = obj->i_mount->surface;
        DEBUG_ASSERT((drawable == NULL),
                         "SEGFAULT: ui_textual_glyph_advance()");
        cr = cairo_create(drawable);
      }
      advance += ext_cairo_glyph_advance(stream, cr,
                                         obj->attrib->font_name,
                                         obj->attrib->font_var & 0x0FF,
                                        (obj->attrib->font_var >> 8) & 0x0FF,
                                         obj->attrib->font_size);
      byte_count -= (*stream) - rdPtr;
    }
  }

  if (!has_table)  free(glyph_table);
  if (cr != NULL)  cairo_destroy(cr);
  return advance;
}

/* Create an ascii table of widths for quick access to advances. */
char *
ui_textual_glyph_table(PhxObject *obj) {

  cairo_surface_t *drawable;
  cairo_t *cro;
  PhxAttr *attrib;
  char *glyph_widths;
  uint16_t idx;  /* ABSOLUTE MUST! gcc can't handle using char for idx. */

  drawable = obj->i_mount->surface;
  DEBUG_ASSERT((drawable == NULL),
                         "SEGFAULT: ui_textual_glyph_table()");
  cro = cairo_create(drawable);

  attrib = obj->attrib;
  cairo_select_font_face(cro, attrib->font_name,
                              attrib->font_var & 0x0FF,
                             (attrib->font_var >> 8) & 0x0FF);
  cairo_set_font_size(cro, attrib->font_size);

  glyph_widths = malloc(128 * sizeof(char));
  memset(glyph_widths, 0, 0x20);

  for (idx = 0x20; idx <= 0x7e; idx++) {
    cairo_text_extents_t search_extents;
    cairo_text_extents(cro, (const char*)&idx, &search_extents);
    glyph_widths[idx] = (uint8_t)(search_extents.x_advance + 0.5);
  }
  glyph_widths[0x7f] = 1;
    /* want a size for good representation with block_caret */
  glyph_widths[0]    = glyph_widths[0x20];
  glyph_widths['\t'] = glyph_widths[0x20] * TAB_TEXT_WIDTH;

  cairo_destroy(cro);
  return glyph_widths;
}

/* Create for an object. Attaches to object's 'exclusive'.
  Need object's draw_box for attributes and PhxBin.
  Expects a c-string for 'data'. */
PhxTextbuffer *
ui_textbuffer_create(PhxObject *obj, char *data) {

  PhxTextbuffer *tbuf;
  size_t rdSz, bufSz;
  bool newly;

    /* For now, use as required.. */
  if (obj->type != PHX_TEXTVIEW)  return NULL;

  tbuf = (PhxTextbuffer*)obj->exclusive;
  newly = (tbuf == NULL);
  if (newly) {
    tbuf = malloc(sizeof(PhxTextbuffer));
    memset(tbuf, 0, sizeof(PhxTextbuffer));
    obj->exclusive = tbuf;
    tbuf->owner = obj;
    tbuf->glyph_widths = ui_textual_glyph_table(obj);
  }

  tbuf->insert.offset = (tbuf->insert.y = (tbuf->insert.x = 0));
  tbuf->drop = (tbuf->interim = (tbuf->release = tbuf->insert));
  tbuf->bin.w = obj->draw_box.w;
  tbuf->bin.h = obj->draw_box.h;

  if (tbuf->string != NULL)
    free(tbuf->string);
  rdSz = (data != NULL) ? strlen(data) : 0;
  bufSz = (rdSz + TGAP_ALLOC) & ~(TGAP_ALLOC - 1);
  if ((bufSz - rdSz) < (TGAP_ALLOC >> 1))  bufSz += TGAP_ALLOC;
  tbuf->string = malloc(bufSz);
  memset(&tbuf->string[rdSz], 'a', (bufSz - rdSz));
  if (rdSz)  memmove(tbuf->string, data, rdSz);

  tbuf->string[rdSz] = 0;
  tbuf->str_nil = rdSz;
  tbuf->gap_start = rdSz + 1;
  tbuf->gap_end = bufSz - 1;

  if (newly) {
    phxmarks_initialize(tbuf);
  } else {
    tbuf->dirty_offset = 0;
    phxmarks_update(tbuf);
  }
  return tbuf;
}
