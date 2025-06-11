#include "findport.h"

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

static int32_t   _findport_search_from(char *tbuf,
                                       phx_fsearch_t *fdata,
                                       uint32_t start_offset);

#pragma mark *** Session Findboard ***

/* Create session phx_findboard_t */
static phx_findboard_t *
_findboard_create(void) {

  phx_findboard_t *find;

  session->xfindboard = malloc(sizeof(phx_findboard_t));
  find = session->xfindboard;
  memset(find, 0, sizeof(phx_findboard_t));

  find->fsearches = malloc(OBJS_ALLOC * sizeof(phx_fsearch_t*));
  memset(find->fsearches, 0, (OBJS_ALLOC * sizeof(phx_fsearch_t*)));

  return find;
}

#pragma mark *** Fsearch Accessors ***

/* Finds the last occurring fsearch for a textview.
  XXX Currently just set up for one search per otxt, no previous
  search data retained. */
static phx_fsearch_t *
_fsearch_for_textview(PhxObjectTextview *otxt) {

  phx_findboard_t *findboard = session->xfindboard;
  phx_fsearch_t *fdata = NULL;
  if (findboard == NULL)
    findboard = _findboard_create();
  if (findboard->ncount != 0) {
    uint16_t fdx = findboard->ncount;
    do {
      phx_fsearch_t *inspect = findboard->fsearches[(--fdx)];
      if (inspect->otxt == otxt) {
        fdata = inspect;
        break;
      }
    } while (fdx != 0);
  }
  return fdata;
}

static phx_fsearch_t *
_fsearch_for_findport(PhxFindport *findbar) {

  phx_findboard_t *findboard = session->xfindboard;
  phx_fsearch_t *fdata = NULL;
  if (findboard == NULL)
    findboard = _findboard_create();
  if (findboard->ncount != 0) {
    uint16_t fdx = findboard->ncount;
    do {
      phx_fsearch_t *inspect = findboard->fsearches[(--fdx)];
      if (inspect->findbar == findbar) {
        fdata = inspect;
        break;
      }
    } while (fdx != 0);
  }
  return fdata;
}


#pragma mark *** Marks ***

/* direction 32bits due to c type enumeration */
/* Given an insert location, set search sdx based on directional request. */
static void
_fsearchmarks_move_to(phx_fsearch_t *fdata, int32_t direction) {

  PhxTextbuffer *tbuf;
  uint32_t locus;
  int32_t ins;
  PhxMarkData *mdPtr, *offset_mark;

  if (fdata->_results.ncount == 0)  return;
  mdPtr = fdata->_results.pairs;
  tbuf = (PhxTextbuffer*)fdata->otxt->exclusive;
  ins = tbuf->insert.offset;
  if (mdPtr->d0 > ins) {
    locus = fdata->_results.ncount - 1;
    if ((direction == RESULT_GE)
        || (direction == RESULT_GT)
        || (fdata->_results.ncount == 1)) {
      locus = 0;
    }
    goto tbuf_update;
  }
  offset_mark = bsearch(&ins, mdPtr, fdata->_results.ncount + 1,
                                sizeof(PhxMarkData), _led0_compare);
  if (offset_mark->d0 == (uint32_t)~0)  offset_mark--;
  locus = offset_mark - mdPtr;
  if (ins == offset_mark->d0) {
    if ((direction == RESULT_GE)
        || (direction == RESULT_LE))
      goto tbuf_update;
    if (direction == RESULT_LT) {
      if (locus == 0)
            locus = fdata->_results.ncount - 1;
      else  locus--;
      goto tbuf_update;
    }
    if (direction == RESULT_GT) {
        /* if not selected */
      if (offset_mark->d1 != tbuf->release.offset)
        goto tbuf_update;
rgt:  if ((offset_mark + 1)->d0 != (uint32_t)~0)
            locus++;
      else  locus = 0;
      goto tbuf_update;
    }
  }
  if (direction == RESULT_GT)  goto rgt;

tbuf_update:
  mdPtr = &fdata->_results.pairs[locus];
  tbuf->insert.offset = mdPtr->d0;
  location_for_offset(tbuf, &tbuf->insert);
  location_auto_scroll(tbuf, &tbuf->insert);
  tbuf->interim = tbuf->insert;
  tbuf->release.offset = mdPtr->d1;
  location_for_offset(tbuf, &tbuf->release);
}

static void
_fsearchmarks_draw(PhxTextbuffer *tbuf, cairo_t *cr) {

  phx_fsearch_t *fdata;
  PhxMarkData *mdPtr;
  bool has_focus;
  double font_em;

  fdata = _fsearch_for_textview(tbuf->owner);
  if ( (fdata == NULL) || (fdata->_results.ncount == 0) )  return;
  if (!ui_visible_get((PhxObject*)fdata->findbar))   return;

  has_focus = (ui_active_focus_get() == tbuf->owner);
  font_em = tbuf->owner->attrib->font_em;

  mdPtr = fdata->_results.pairs;
  if (mdPtr->d0 < tbuf->draw_top_offset) {
    mdPtr = bsearch(&tbuf->draw_top_offset, mdPtr,
                                    fdata->_results.ncount + 1,
                                    sizeof(PhxMarkData),
                                    _led0_compare);
    if (mdPtr->d1 < tbuf->draw_top_offset)
      mdPtr += 1;
  }
  do {
    double x0, y0, x1, y1, alpha;
    location tlpt, brpt;
    tlpt.offset = mdPtr->d0;
    brpt.offset = mdPtr->d1;
    location_for_offset(tbuf, &tlpt);
    location_for_offset(tbuf, &brpt);
    x0 = tlpt.x - tbuf->bin.x;
    y0 = tlpt.y - tbuf->bin.y;
    x1 = brpt.x - tbuf->bin.x;
    y1 = brpt.y - tbuf->bin.y;

    alpha = 0.7;
    if (!has_focus)  alpha = 0.35;
    cairo_set_source_rgba(cr, RGBA_SEARCH_FGFILL.r,
                              RGBA_SEARCH_FGFILL.g, 
                              RGBA_SEARCH_FGFILL.b, alpha);
    if (y0 == y1) {
        /* single line selection rectangle */
      cairo_rectangle(cr, x0, y0, (x1 - x0), font_em);
    } else {
      double width = tbuf->owner->draw_box.w;
      if (x0 < 0)  x0 = 0;
      cairo_rectangle(cr, x0, y0, width, font_em);
      if ((y0 += font_em) < y1)
        cairo_rectangle(cr, 0, y0, width, (y1 - y0));
      if (x1 > tbuf->bin.x)
        cairo_rectangle(cr, 0, y1, x1, font_em);
    }
    cairo_fill(cr);

  } while ((++mdPtr)->d0 < tbuf->draw_end_offset);
}

static void
_fsearchmarks_update(PhxTextbuffer *tbuf) {
  phx_fsearch_t *fdata = _fsearch_for_textview(tbuf->owner);
  _findport_search_from(tbuf->string, fdata, tbuf->dirty_offset);
}

#pragma mark *** Fsearch ***

static void
_fsearch_found_update(phx_fsearch_t *fdata) {

  bool set;
  char rbuf[32];
  uint32_t found_count = fdata->_results.ncount;
  sprintf(rbuf, "%d found matches", found_count);
  ui_label_text_set(fdata->findbar->objects[found_box], rbuf);

  set = (found_count > 1);
  if (found_count == 1) {
    PhxTextbuffer *tbuf = (PhxTextbuffer*)fdata->otxt->exclusive;
    set = ((tbuf->insert.offset != fdata->_results.pairs->d0)
           || (tbuf->release.offset != fdata->_results.pairs->d1));
  }
  ui_sensitive_set(fdata->findbar->objects[navigate_right_box], set);
  ui_sensitive_set(fdata->findbar->objects[navigate_left_box], set);
}

/* Updates 'pairs' from start of current selected by what was replaced.
  Determines if search_from() or updating of offsets method is used. */
static void
_fsearch_entry_update(char *tbuf,
                      phx_fsearch_t *fdata,
                      uint32_t replace_size,
                      uint32_t locus) {

  PhxMarkData *mdPtr = &fdata->_results.pairs[locus];
  int32_t delta = replace_size - (mdPtr->d1 - mdPtr->d0);
  if (delta > 0) {
      /* test for replacement string containing search string. */
    PhxObjectTextview *robj = fdata->findbar->objects[textview_replace_box];
    PhxTextbuffer *rbuf = (PhxTextbuffer*)robj->exclusive;
    if (strstr(rbuf->string, fdata->string) != NULL) {
      _findport_search_from(tbuf, fdata, mdPtr->d0);
      return;
    }
  }
  while ((mdPtr + 1)->d0 != (uint32_t)~0) {
    mdPtr->d0 = (mdPtr + 1)->d0 + delta;
    mdPtr->d1 = (mdPtr + 1)->d1 + delta;
    mdPtr++;
  }
  mdPtr->d0 = (uint32_t)~0;
  mdPtr->d1 = (uint32_t)~0;
  fdata->_results.ncount--;
  _fsearch_found_update(fdata);
}

/* Use to clear search info related to otxt/findbar.
  Can re-use for new search. */
static void
_fsearch_reset(phx_fsearch_t *fdata) {

  fdata->state &= ~1;
  if (fdata->string != NULL)
    free(fdata->string), fdata->string = NULL;
  if (fdata->file != NULL)
    free(fdata->file), fdata->file = NULL;
  if (fdata->_results.pairs != NULL) {
    fdata->_results.pairs = realloc(fdata->_results.pairs,
                                       (OBJS_ALLOC * sizeof(PhxMarkData)));
    memset(fdata->_results.pairs, 0, (OBJS_ALLOC * sizeof(PhxMarkData)));
    fdata->_results.ncount = 0;
  }
    /* leave otxt, findbar alone. references of objects related to fdata. */
  _fsearch_found_update(fdata);

}

static void
_fsearch_raze(void *obj) {

  phx_fsearch_t *search = (phx_fsearch_t*)obj;

  if (search->string != NULL)
    free(search->string), search->string = NULL;
  if (search->file != NULL)
    free(search->file), search->file = NULL;
  if (search->_results.pairs != NULL) {
    free(search->_results.pairs);
    search->_results.pairs = NULL;
    search->_results.ncount = 0;
  }
    /* leave otxt, findbar alone. references of objects related to search. */
}

static phx_fsearch_t *
_fsearch_create(PhxInterface *iface, PhxObjectTextview *otxt) {

  PhxMark *search_mark;
  phx_findboard_t *fboard;
  phx_fsearch_t *fsearch;

  fsearch = malloc(sizeof(phx_fsearch_t));
  memset(fsearch, 0, sizeof(phx_fsearch_t));
  fsearch->_results.pairs = malloc(OBJS_ALLOC * sizeof(PhxMarkData));
  memset(fsearch->_results.pairs, 0, (OBJS_ALLOC * sizeof(PhxMarkData)));

    /* leave string until outside of function. */
    /* connect to textview */
  fsearch->otxt = otxt;
  search_mark = malloc(sizeof(PhxMark));
  search_mark->type = PHXSEARCH;
  search_mark->_mark_update = _fsearchmarks_update;
  search_mark->_mark_draw   = _fsearchmarks_draw;
    /* when otxt dies, search_mark needs death. no reference in findport.
      No data for our use. otxt will free mark. */
  search_mark->_mark_raze   = NULL;
  search_mark->data         = NULL;
  phxmarks_list_add((PhxTextbuffer*)otxt->exclusive, search_mark);
    /* create user's interface */
  fsearch->findbar = ui_findport_create(iface, otxt);
    /* currently no file attachments, single port, single text demo. */
    /* rest filled by other methods. */

/* XXX need realloc */

  fboard = session->xfindboard;
  fboard->fsearches[fboard->ncount] = fsearch;
  fboard->ncount += 1;

  if (fboard->sdata != NULL) {
    PhxObjectTextview *search;
    fsearch->string = strdup(fboard->sdata);
    search = fsearch->findbar->objects[textview_find_box];
    ui_textview_buffer_set(search, fsearch->string, (HJST_LFT | VJST_BTM));
  }
  if (fboard->rdata != NULL) {
    PhxObjectTextview *replace;
    replace = fsearch->findbar->objects[textview_replace_box];
    ui_textview_buffer_set(replace, fboard->rdata, (HJST_LFT | VJST_BTM));
  }

  return fsearch;
}

#pragma mark *** Findport Events ***

static void
result_cb(PhxBank *ibank) {

  PhxVault *vault = (PhxVault*)ibank->exclusive;
  PhxFindport *findbar;
  phx_fsearch_t *fdata;
  PhxObjectTextview *otxt, *search;
  PhxTextbuffer *stxt;
  PhxRectangle rbox;
  int16_t vD;
  uint16_t box_height, sep_reduction;
  bool set;

    /* reguardless of selection, do a find if find_box has content
      and is different than fdata->string. */
  findbar = (PhxNexus*)vault->actuator->i_mount;
  box_height = findbar->objects[choose_box]->mete_box.h;
  sep_reduction = (box_height < 21) ? 1 : 0;
  set = (vault->on_idx == 0);
    /* Don't assume findbar is connected to a searchable.
      If no connection, we just 'resize' based on results. */
  fdata = _fsearch_for_findport(findbar);
  if (fdata == NULL) {
    if (vault->was_idx == vault->in_idx)  return;
    rbox = findbar->mete_box;
    if (set)  rbox.h = box_height + 2;
    else      rbox.h = (box_height * 2) + sep_reduction;
    ui_nexus_resize((PhxNexus*)findbar, &rbox);
    goto final_adjust;
  }

  search = findbar->objects[textview_find_box];
  stxt = (PhxTextbuffer*)search->exclusive;
  if ( (stxt->string != NULL) || (*stxt->string != 0) ) {
    phx_fsearch_t *fdata = _fsearch_for_findport(findbar);
    if ( (fdata->string == NULL) || (*fdata->string == 0)
        || (strcmp(stxt->string, fdata->string) != 0)
        || ((fdata->state & 1) != 0) ) {
      PhxTextbuffer *tbuf = (PhxTextbuffer*)fdata->otxt->exclusive;
      if (fdata->string != NULL)
        free(fdata->string);
      fdata->string = strdup(stxt->string);
      _findport_search_from(tbuf->string, fdata, 0);
      _fsearchmarks_move_to(fdata, RESULT_EQU);
    }
  }

  if (vault->was_idx == vault->in_idx)  return;

  otxt = _fsearch_for_findport(findbar)->otxt;

  if (set) {
    findbar->mete_box.h = box_height + 2;
    rbox = otxt->i_mount->mete_box;
    vD = findbar->mete_box.h - rbox.y;
    rbox.y += vD;
    rbox.h -= vD;
  } else {
    findbar->mete_box.h = (box_height * 2) + sep_reduction;
    rbox = otxt->i_mount->mete_box;
    vD = findbar->mete_box.h - rbox.y;
    rbox.y += vD;
    rbox.h -= vD;
  }
  ui_nexus_resize((PhxNexus*)otxt->i_mount, &rbox);

final_adjust:
  ui_visible_set(findbar->objects[close0_box], set);
  ui_visible_set(findbar->objects[replace_all_box], !set);
  ui_visible_set(findbar->objects[replace_box], !set);
  ui_visible_set(findbar->objects[replace_find_box], !set);
  ui_visible_set(findbar->objects[close1_box], !set);
  ui_visible_set(findbar->objects[textview_replace_box], !set);
  vault->was_idx = vault->in_idx;
  ui_invalidate_object((PhxObject*)findbar->i_mount);
}

static bool
btn_close_event(PhxInterface *iface,
                xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse;
  PhxFindport *findbar;
  phx_fsearch_t *fdata;
  PhxBank *ibank;
  PhxVault *vault;

  if ((nvt->response_type & (uint8_t)0x7F) != XCB_BUTTON_RELEASE)
    return _default_button_meter(iface, nvt, obj);
  mouse = (xcb_button_press_event_t*)nvt;
  if (mouse->detail != 1)
    return _default_button_meter(iface, nvt, obj);

  findbar = (PhxNexus*)obj->i_mount;
  ui_visible_set((PhxObject*)findbar, false);
  ui_invalidate_object((PhxObject*)findbar);

    /* Added to allow interface without being attached to content. */
  fdata = _fsearch_for_findport(findbar);
  if (fdata != NULL) {
    PhxObjectTextview *otxt = _fsearch_for_findport(findbar)->otxt;
    PhxNexus *txt_nexus = (PhxNexus*)otxt->i_mount;
    PhxRectangle rbox = txt_nexus->mete_box;
    rbox.y -= findbar->mete_box.h;
    rbox.h += findbar->mete_box.h;
    ui_nexus_resize(txt_nexus, &rbox);
    ui_active_focus_set(otxt);
  }

    /* Set up for re-openning findbar. */
  ibank = ui_dropdown_from(findbar->objects[choose_box]);
  vault = (PhxVault*)ibank->exclusive;
  vault->was_idx = (vault->on_idx = 0);
  ui_actuator_content_update(ibank);
  ui_visible_set(findbar->objects[close0_box], true);

  return _default_button_meter(iface, nvt, obj);
}

/* XXX need dirty_offset else always search from 0. */
static bool
btn_navigate_right_event(PhxInterface *iface,
                         xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse;
  phx_fsearch_t *fdata;

  if ((nvt->response_type & (uint8_t)0x7F) != XCB_BUTTON_RELEASE)
    return _default_button_meter(iface, nvt, obj);
  mouse = (xcb_button_press_event_t*)nvt;
  if (mouse->detail != 1)
    return _default_button_meter(iface, nvt, obj);

  fdata = _fsearch_for_findport((PhxFindport*)obj->i_mount);
  if (fdata != NULL) {
    _fsearchmarks_move_to(fdata, RESULT_GT);
    ui_invalidate_object(fdata->otxt);
  }

  return _default_button_meter(iface, nvt, obj);
}

static bool
btn_navigate_left_event(PhxInterface *iface,
                        xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse;
  phx_fsearch_t *fdata;

  if ((nvt->response_type & (uint8_t)0x7F) != XCB_BUTTON_RELEASE)
    return _default_button_meter(iface, nvt, obj);
  mouse = (xcb_button_press_event_t*)nvt;
  if (mouse->detail != 1)
    return _default_button_meter(iface, nvt, obj);

  fdata = _fsearch_for_findport((PhxFindport*)obj->i_mount);
  if (fdata != NULL) {
    _fsearchmarks_move_to(fdata, RESULT_LT);
    ui_invalidate_object(fdata->otxt);
  }

  return _default_button_meter(iface, nvt, obj);
}

static void
_findport_replace(phx_fsearch_t *fdata, uint8_t mode) {

  int32_t ins, delta;
  uint32_t locus, ndx;
  PhxMarkData *mdPtr, *offset_mark;
  PhxTextbuffer *tbuf, *rbuf;
  PhxObjectTextview *replace;

  if (fdata->_results.ncount == 0)  return;
  mdPtr = fdata->_results.pairs;
  tbuf = (PhxTextbuffer*)fdata->otxt->exclusive;
  ins = tbuf->insert.offset;
  if (mdPtr->d0 > ins)  return;
  offset_mark = bsearch(&ins, mdPtr, fdata->_results.ncount + 1,
                                sizeof(PhxMarkData), _led0_compare);
  if (offset_mark->d0 == (uint32_t)~0)  offset_mark--;

    /* check for button pressed with an actual selection of 'find' */
  if (ins != offset_mark->d0)  return;
  if (tbuf->release.offset != offset_mark->d1)  return;
  if (memcmp(fdata->string, &tbuf->string[ins],
                     (tbuf->release.offset - ins)) != 0)
    return;
    /* replace string get from findbar, can be empty string */
  replace = fdata->findbar->objects[textview_replace_box];
  rbuf = (PhxTextbuffer*)replace->exclusive;
    /* set rdx for findport_update_entry() */
  locus = offset_mark - mdPtr;

   /* no test for 1, possible replacement with contained search string */
  if ( (mode == 0)
      || ((mode == 3) && (fdata->_results.ncount == 1)) ) {
    _textbuffer_replace(tbuf, rbuf->string, rbuf->str_nil);
    _fsearch_entry_update(tbuf->string, fdata, rbuf->str_nil, locus);
    _textbuffer_edit_set(tbuf, (ins + rbuf->str_nil));
    return;
  }
  if (mode == 1) {
      /* replace does not alter locations, nor set up for editing */
    _textbuffer_replace(tbuf, rbuf->string, rbuf->str_nil);
    _fsearch_entry_update(tbuf->string, fdata, rbuf->str_nil, locus);
      /* adjusment of locations */
    if (fdata->_results.ncount == 0) {
      _textbuffer_edit_set(tbuf, (ins + rbuf->str_nil));
    } else {
      _fsearchmarks_move_to(fdata, RESULT_GT);
    }
    return;
  }
    /* replace_all mode 3, does not 'Find' in replacement
      want difference for placement of insert mark after done */
  delta = rbuf->str_nil - (mdPtr->d1 - mdPtr->d0);
  ndx = fdata->_results.ncount;      /* holds count, not idx */
  offset_mark = mdPtr + (ndx - 1);   /* -1 for idx */
    /* start from tail, preserves forward mark positions */
  do {
    tbuf->insert.offset  = offset_mark->d0;
    tbuf->release.offset = offset_mark->d1;
    _textbuffer_replace(tbuf, rbuf->string, rbuf->str_nil);
    if ((--ndx) == 0)  break;
      /* sdx = pre-button press location */
    if (ndx <= locus)  ins += delta;
    offset_mark--;
  } while (1);
  _fsearch_reset(fdata);
    /* adjusment of locations */
  _textbuffer_edit_set(tbuf, (ins + rbuf->str_nil));
}

static bool
btn_replace_all_event(PhxInterface *iface,
                      xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse;
  phx_fsearch_t *fdata;

  if ((nvt->response_type & (uint8_t)0x7F) != XCB_BUTTON_RELEASE)
    return _default_button_meter(iface, nvt, obj);
  mouse = (xcb_button_press_event_t*)nvt;
  if (mouse->detail != 1)
    return _default_button_meter(iface, nvt, obj);

  fdata = _fsearch_for_findport((PhxFindport*)obj->i_mount);
  if (fdata != NULL) {
    _findport_replace(fdata, 3);
    ui_invalidate_object((PhxObject*)fdata->findbar->i_mount);
  }

  return _default_button_meter(iface, nvt, obj);
}

static bool
btn_replace_event(PhxInterface *iface,
                  xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse;
  phx_fsearch_t *fdata;

  if ((nvt->response_type & (uint8_t)0x7F) != XCB_BUTTON_RELEASE)
    return _default_button_meter(iface, nvt, obj);
  mouse = (xcb_button_press_event_t*)nvt;
  if (mouse->detail != 1)
    return _default_button_meter(iface, nvt, obj);

  fdata = _fsearch_for_findport((PhxFindport*)obj->i_mount);
  if (fdata != NULL) {
    _findport_replace(fdata, 0);
    ui_invalidate_object((PhxObject*)fdata->findbar->i_mount);
  }

  return _default_button_meter(iface, nvt, obj);
}

static bool
btn_replace_find_event(PhxInterface *iface,
                       xcb_generic_event_t *nvt, PhxObject *obj) {

  xcb_button_press_event_t *mouse;
  phx_fsearch_t *fdata;

  if ((nvt->response_type & (uint8_t)0x7F) != XCB_BUTTON_RELEASE)
    return _default_button_meter(iface, nvt, obj);
  mouse = (xcb_button_press_event_t*)nvt;
  if (mouse->detail != 1)
    return _default_button_meter(iface, nvt, obj);

  fdata = _fsearch_for_findport((PhxFindport*)obj->i_mount);
  if (fdata != NULL) {
    _findport_replace(fdata, 1);
    ui_invalidate_object((PhxObject*)fdata->findbar->i_mount);
  }

  return _default_button_meter(iface, nvt, obj);
}

static int32_t
_findport_search_from(char *tbuf,
                      phx_fsearch_t *fdata,
                      uint32_t start_offset) {

  char *sPtr, *rdPtr;
  uint32_t ndx, key_len;
  PhxMarkData *mdPtr, *offset_mark;

  PhxObjectTextview *stxt = fdata->findbar->objects[textview_find_box];
  PhxTextbuffer *sbuf = (PhxTextbuffer*)stxt->exclusive;
  sPtr = sbuf->string;
  if ( (sPtr == NULL) || (*sPtr == 0) )  return 0;

    /* Shouldn't need because of current 'focus'. */
  _textbuffer_flush(sbuf);
  fdata->state &= ~1; /* searched */
  fdata->string = strdup(sbuf->string);
  key_len = sbuf->str_nil;
  mdPtr = fdata->_results.pairs;

  if ( (start_offset == 0) || (fdata->_results.ncount == 0) )
    goto zero_start;

  offset_mark = bsearch(&start_offset, fdata->_results.pairs,
                                       fdata->_results.ncount + 1,
                                       sizeof(PhxMarkData), _led0_compare);
  if (offset_mark != NULL) {
    if (offset_mark->d0 == (uint32_t)~0)  offset_mark--;
    ndx = (uint32_t)(offset_mark - fdata->_results.pairs);
    start_offset = offset_mark->d0;
  } else {
zero_start:
    ndx = 0;
    start_offset = 0;
  }

  sPtr = tbuf;
  rdPtr = tbuf + start_offset;
  do {
    if (_mark_allocation((void**)&fdata->_results.pairs,
                                sizeof(PhxMarkData), ndx))  break;
    mdPtr = &fdata->_results.pairs[ndx];
    if ((rdPtr = strstr(rdPtr, fdata->string)) == NULL)  break;
    mdPtr->d0 = rdPtr - sPtr;
    mdPtr->d1 = mdPtr->d0 + key_len;
    rdPtr += key_len;
    ndx++;
  } while (1);
  mdPtr->d0 = (uint32_t)~0;
  mdPtr->d1 = (uint32_t)~0;
  fdata->_results.ncount = ndx;
  _fsearch_found_update(fdata);
  return ndx;
}

static void
_findport_visible_set(PhxObjectTextview *otxt, PhxFindport *findbar) {

  phx_findboard_t *findboard;

  if (!ui_visible_get((PhxObject*)findbar)) {
    PhxRectangle rbox;
    uint16_t box_height;
    ui_visible_set((PhxObject*)findbar, true);
    ui_visible_set(findbar->objects[replace_all_box], false);
    ui_visible_set(findbar->objects[replace_box], false);
    ui_visible_set(findbar->objects[replace_find_box], false);
    ui_visible_set(findbar->objects[close1_box], false);
    ui_visible_set(findbar->objects[textview_replace_box], false);
      /* Set to view row 0 only. */
    box_height = findbar->objects[choose_box]->mete_box.h;
    findbar->mete_box.h = box_height + 2;
      /* Move/resize otxt/nexus towards bottom to accommodate findbar. */
    rbox = otxt->i_mount->mete_box;
    rbox.y += box_height + 2;
    rbox.h -= box_height + 2;
    ui_nexus_resize((PhxNexus*)otxt->i_mount, &rbox);
  }

  findboard = session->xfindboard;
    /* Place current xfindboard data into search entry. */
  if (findboard->rdata != NULL) {
    PhxObjectTextview *ftxt = findbar->objects[textview_replace_box];
    PhxTextbuffer *tbuf = (PhxTextbuffer*)ftxt->exclusive;
    char *string = tbuf->string;
    if ( (string == NULL)
        || (strcmp(string, findboard->rdata) != 0) ) {
/* XXX use obj->state. */
      ui_textview_buffer_set(ftxt, findboard->rdata, (HJST_LFT | VJST_BTM));
    }
  }

  if (findboard->sdata != NULL) {
    PhxObjectTextview *ftxt = findbar->objects[textview_find_box];
    PhxTextbuffer *tbuf = (PhxTextbuffer*)ftxt->exclusive;
    char *string = tbuf->string;
    if ( (string == NULL)
        || (strcmp(string, findboard->sdata) != 0) ) {
/* XXX use obj->state. */
      ui_textview_buffer_set(ftxt, findboard->sdata, (HJST_LFT | VJST_BTM));
    }
  }
}

void
ui_findport_search(PhxInterface *iface, PhxObjectTextview *otxt) {

  PhxFindport *findbar;
  char *tbuf;
  phx_fsearch_t *fdata = _fsearch_for_textview(otxt);

  if (fdata == NULL)
    fdata = _fsearch_create(iface, otxt);
  findbar = fdata->findbar;
  _findport_visible_set(otxt, findbar);

  tbuf = ((PhxTextbuffer*)otxt->exclusive)->string;
  _findport_search_from(tbuf, fdata, 0);
  ui_invalidate_object((PhxObject*)iface);
}

void
ui_findport_keyboard(PhxObjectTextview *otxt, uint8_t key) {

  PhxTextbuffer *tbuf;
  phx_findboard_t *findboard;
  uint16_t *szPtr;
  char     **dataPtr;
  int32_t sz, bdx;

  if ( (otxt == NULL) || (OBJECT_BASE_TYPE(otxt) != PHX_TEXTVIEW)
      || (otxt->exclusive == NULL) )
    return;

  tbuf = (PhxTextbuffer*)otxt->exclusive;
  sz = tbuf->release.offset - tbuf->insert.offset;
  if (sz == 0)  return;
  DEBUG_ASSERT((sz < 0), "trying to copy negative amount.");

  if (session->xfindboard == NULL)
    _findboard_create();

  findboard = session->xfindboard;
  szPtr   = (key == 'e') ? &findboard->sdsz : &findboard->rdsz;
  dataPtr = (key == 'e') ? &findboard->sdata : &findboard->rdata;
  bdx     = (key == 'e') ? textview_find_box : textview_replace_box;

  if (*dataPtr != NULL)
    free(*dataPtr);
  *dataPtr = malloc(sz + 1);
  memmove(*dataPtr, &tbuf->string[tbuf->insert.offset], sz);
  (*dataPtr)[sz] = 0;
  *szPtr = sz;

    /* All findbars to get updated data. */
  if (findboard->ncount != 0) {
    uint16_t ndx = findboard->ncount;
    do {
      phx_fsearch_t *inspect = findboard->fsearches[(--ndx)];
      PhxNexus *findbar = inspect->findbar;
      PhxObjectTextview *ftxt = findbar->objects[bdx];
      ui_textview_buffer_set(ftxt, *dataPtr, (HJST_LFT | VJST_BTM));
      if (ui_visible_get((PhxObject*)findbar)) {
          /* active visible to be only one that gets fdata string updated? */
        phx_fsearch_t *fdata = _fsearch_for_findport(findbar);
        fdata->state |= 1; /* changed/not-searched */
        if (fdata->string != NULL)  free(fdata->string);
        fdata->string = strdup(*dataPtr);
        ui_invalidate_object((PhxObject*)findbar);
      }
    } while (ndx != 0);
  }
}

static bool
_findport_meter(PhxInterface *iface,
                xcb_generic_event_t *nvt, PhxObject *obj) {

  if ((nvt->response_type & (uint8_t)0x7F) == XCB_CONFIGURE_NOTIFY) {
    int16_t wD;
    PhxNexus *nexus = (PhxNexus*)obj;
    xcb_configure_notify_event_t *configure, notify;
    configure = (xcb_configure_notify_event_t*)nvt;

      /* Want ateration of textview widths, coupled with x movement
        of 'done' buttons. During creation set object's xXPD bits. */
    wD = configure->width - nexus->mete_box.w;
    if (wD != 0) {
      uint16_t idx;
      nexus->mete_box.w += wD;
      nexus->draw_box.w += wD;
      memmove(&notify, configure, sizeof(xcb_configure_notify_event_t));
      for (idx = close1_box; idx <= textview_find_box; idx++) {
        PhxObject *inspect = nexus->objects[idx];
        notify.width  = inspect->mete_box.w + wD;
        notify.height = inspect->mete_box.h;
        inspect->_event_cb(iface, (xcb_generic_event_t*)&notify, inspect);
        ui_invalidate_object(inspect);
      }
    }
    return true;
  }
  return _default_nexus_meter(iface, nvt, obj);
}

#pragma mark *** Findport ***

  /* Code only creates a findbar on call for a search. */
static void
_findport_raze(void *obj) {

  phx_fsearch_t *fdata = _fsearch_for_findport((PhxFindport*)obj);
  if (fdata != NULL)  _fsearch_raze((void*)fdata);
  _default_nexus_raze((PhxFindport*)obj);
}

  /* From buttons.c, frame drawing is 'bfm = (configure.h < 21) ? 2 : 3;' */
  /* At BOX_HEIGHT = 24, font_em desired is 24 - (6 + 4), 2 for line widths,
    2 for white space margin. */
static void
_button_size_max(PhxNexus *nexus, PhxRectangle *button_mete) {

  int16_t button_size;
  PhxRectangle dbox;
  PhxObject *obj;

    /* Create dummy conbo button. */
  obj = ui_button_create(nexus, BTN_COMBO_ARROW, *button_mete);
  _button_draw_area_request(obj, &dbox);
    /* Remove the dummy. */
  obj->_raze_cb((void*)obj);
  nexus->objects[0] = NULL;
  nexus->ncount -= 1;

    /* add in 1px white space margin. */
  ui_attributes_font_em_set((PhxObject*)nexus, dbox.h);

    /* Create dummy label of largest text. Extract the text advance. */
  obj = ui_label_create(nexus, dbox,
                             button_labels[1], (HJST_CTR | VJST_CTR));
  if (obj == NULL)  return;
  button_size
    = ((PhxLabelbuffer*)((PhxObjectLabel*)obj)->exclusive)->adv_box.w;
    /* Remove the dummy. */
  obj->_raze_cb((void*)obj);
  nexus->objects[0] = NULL;
  nexus->ncount -= 1;

    /* Add in textview visual margin */
  button_size += BUTTON_TEXT_MIN * 2;
    /* Return to sender. */
  button_mete->w -= dbox.w - button_size;
}

static bool
_findport_layout(PhxNexus *nexus) {

  PhxRectangle top_button_mete, bottom_button_mete;
  PhxObject *obj;
  char *stream;
  int16_t width_from_advance;

  int16_t textview_start, combo_end;
  uint16_t box_height = nexus->mete_box.h / 2;
  uint16_t sep_reduction = (box_height < 21) ? 1 : 2;
  uint16_t txt_margin = (box_height < 21) ? 3 : 4;

  if (box_height < 10)  return false;

    /* Findbar is based on button sizes. Height based on desired size.
      Width based off longest text label of button.  */
  top_button_mete = nexus->mete_box;
  top_button_mete.x = (top_button_mete.y = 0);
  top_button_mete.h /= 2;
  _button_size_max(nexus, &top_button_mete);

  bottom_button_mete = top_button_mete;
  combo_end = top_button_mete.w;

                   /* choose_box 0,0  [0] */
  obj = ui_button_create(nexus, BTN_COMBO_ARROW, top_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  ui_button_label_create(obj, button_labels[0], (HJST_CTR | VJST_CTR));
  ui_button_label_create(obj, button_labels[1], (HJST_CTR | VJST_CTR));
    /* Creates '_event_cb' which designer responds to added objects
      thru 'result_cb'. Basically multiple '_event_cb's. */
  ui_bank_add_result_cb(obj, result_cb);

                   /* replace_all_box 0,1 [1] */
  bottom_button_mete.y += box_height;
  obj = ui_button_create(nexus, BTN_ROUND_CORNER, bottom_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  obj->draw_box.y -= sep_reduction;
  obj->_event_cb = btn_replace_all_event;
  ui_button_label_create(obj, button_labels[2], (HJST_CTR | VJST_CTR));

                   /* replace_box 1,1 [2] */
    /* Want a progressive pixel margin for 'Replace'. Get advance,
      add box_height plus txt_margin for button mete_box width. Use 'nexus'
      as object, since only use for cairo_surface_t. */
  stream = button_labels[3];
  width_from_advance = ui_textual_glyph_advance(
                     (PhxObject*)nexus, &stream, strlen(button_labels[3]));
  bottom_button_mete.x += bottom_button_mete.w;
  bottom_button_mete.w = width_from_advance + box_height + txt_margin;
  obj = ui_button_create(nexus, BTN_ROUND_CORNER, bottom_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  obj->draw_box.y -= sep_reduction;
  obj->draw_box.x -= 1;
  obj->draw_box.w += 1;
  obj->_event_cb = btn_replace_event;
  ui_button_label_create(obj, button_labels[3], (HJST_CTR | VJST_CTR));

                   /* replace_find_box 2,1 [3] */
  bottom_button_mete.x += bottom_button_mete.w;
  bottom_button_mete.w = top_button_mete.w;
  obj = ui_button_create(nexus, BTN_ROUND_CORNER, bottom_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  obj->draw_box.y -= sep_reduction;
  obj->draw_box.x -= 1;
  obj->draw_box.w += 1;
  obj->_event_cb = btn_replace_find_event;
  ui_button_label_create(obj, button_labels[4], (HJST_CTR | VJST_CTR));

  textview_start = bottom_button_mete.x + bottom_button_mete.w;

                   /* close1_box 4,1 [4] */
    /* Want a progressive pixel margin for 'Replace'. Get advance,
      add box_height plus txt_margin for button mete_box width. Use 'nexus'
      as object, since only use for cairo_surface_t. */
  stream = button_labels[5];
  width_from_advance = ui_textual_glyph_advance(
                     (PhxObject*)nexus, &stream, strlen(button_labels[5]));
  width_from_advance += box_height + txt_margin;
  bottom_button_mete.x = nexus->mete_box.w - width_from_advance;
  bottom_button_mete.w = width_from_advance;
  obj = ui_button_create(nexus, BTN_ROUND_CORNER, bottom_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  obj->state |= HXPD_LFT;
  obj->draw_box.y -= sep_reduction;
  obj->mete_box.x += 2;
  obj->draw_box.x -= 2;
  bottom_button_mete.x += 2;
  obj->_event_cb = btn_close_event;
  ui_button_label_create(obj, button_labels[5], (HJST_CTR | VJST_CTR));

                   /* close0_box 5,0 [5] */
  top_button_mete.x = nexus->mete_box.w - width_from_advance;
  top_button_mete.w = width_from_advance;
  obj = ui_button_create(nexus, BTN_ROUND_CORNER, top_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  obj->state |= HXPD_LFT;
  obj->mete_box.x += 2;
  obj->draw_box.x -= 2;
  top_button_mete.x += 2;
  obj->_event_cb = btn_close_event;
  ui_button_label_create(obj, button_labels[5], (HJST_CTR | VJST_CTR));

                   /* textview_replace_box 3,1 [6] */
  bottom_button_mete.w = bottom_button_mete.x - textview_start;
  bottom_button_mete.x = textview_start;
  obj = ui_textview_create(nexus, bottom_button_mete);
  obj->state &= ~VXPD_MSK;
    /* match button draw region top */
  obj->draw_box.y += txt_margin - sep_reduction;
  obj->draw_box.x += 2;
  obj->draw_box.w -= 6;
  obj->draw_box.h -= txt_margin + sep_reduction;
  obj->attrib->fg_fill.r = 1;
  obj->attrib->fg_fill.g = 1;
  obj->attrib->fg_fill.b = 1;
  obj->attrib->fg_fill.a = 1;
  ui_textview_font_em_set(obj, obj->draw_box.h);
  ui_textview_buffer_set(obj, "replacing_text", (HJST_LFT | VJST_CTR));

                   /* textview_search_box 4,0 [7] */
  top_button_mete.x = bottom_button_mete.x;
  top_button_mete.w = bottom_button_mete.w;
  obj = ui_textview_create(nexus, top_button_mete);
  obj->state &= ~VXPD_MSK;
  obj->draw_box.y += txt_margin;  /* match button draw region top */
  obj->draw_box.x += 2;
  obj->draw_box.w -= 6;
  obj->draw_box.h -= txt_margin + sep_reduction;
  obj->attrib->fg_fill.r = 1;
  obj->attrib->fg_fill.g = 1;
  obj->attrib->fg_fill.b = 1;
  obj->attrib->fg_fill.a = 1;
  ui_textview_font_em_set(obj, obj->draw_box.h);
  ui_textview_buffer_set(obj, "searched_text", (HJST_LFT | VJST_CTR));

                   /* navigate_right_box 3,0 [8] */
  top_button_mete.x -= box_height;
  top_button_mete.w = box_height;
  obj = ui_button_create(nexus, BTN_NAVIGATE_RIGHT, top_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  obj->draw_box.x -= 3;
  obj->draw_box.w += 1;
  obj->_event_cb = btn_navigate_right_event;

                   /* navigate_left_box 2,0 [9] */
  top_button_mete.x -= box_height;
  obj = ui_button_create(nexus, BTN_NAVIGATE_LEFT, top_button_mete);
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  obj->draw_box.x += 1;
  obj->draw_box.w += 1;
  obj->_event_cb = btn_navigate_left_event;

                   /* found_box 1,0 [10] */
  top_button_mete.w = top_button_mete.x - combo_end;
  top_button_mete.x = combo_end;
  obj = ui_label_create(nexus, top_button_mete,
                                 "0 found matches", (HJST_RGT | VJST_BTM));
  obj->state &= ~(HXPD_MSK | VXPD_MSK);
  ui_label_font_em_set(obj, obj->attrib->font_em - 1);
  obj->draw_box.y -= 1;
  obj->attrib->bg_fill.a = 1;

  return true;
}

static PhxFindport *
_findport_create(PhxInterface *iface, PhxRectangle configure) {

  PhxNexus *nexus;
  nexus = ui_nexus_create(iface, configure);
    /* Note: could do this after objects created. All objects would
      then start as 'static'. */
  nexus->type = PHX_FPORT;
  nexus->state = HXPD_RGT;
  nexus->_draw_cb  = NULL;
  nexus->_event_cb = _findport_meter;
  nexus->_raze_cb  = _findport_raze;
  nexus->attrib->bg_fill.r = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.g = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.b = (double)0x0D4/0x0FF;
  nexus->attrib->bg_fill.a = 1;

  if (!_findport_layout(nexus)) {
    nexus->_raze_cb((void*)nexus);
    iface->ncount--;
    iface->nexus[iface->ncount] = NULL;
    return NULL;
  }
  ui_visible_set((PhxObject*)nexus, false);
  return nexus;
}

/* Creates invisible nexus at '0,0' of textview's mete_box. */
/* XXX findport limited to top of textview */
PhxFindport *
ui_findport_create(PhxInterface *iface, PhxObjectTextview *otxt) {

  PhxInterface *mount;
  PhxRectangle configure;

  configure = otxt->i_mount->mete_box;
  configure.h = otxt->attrib->font_em;
  configure.h += (configure.h < 16) ? 5 : 7;
  configure.h *= 2;

  mount = (PhxInterface*)otxt;
  while (OBJECT_BASE_TYPE((mount = mount->i_mount)) >= PHX_NEXUS) ;

  return _findport_create(mount, configure);
}

