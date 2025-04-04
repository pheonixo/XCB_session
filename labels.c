#include "labels.h"
#include "configure.h"

#pragma mark *** Drawing ***

void
ui_label_draw(PhxObject *b, cairo_t *cr) {

  PhxObject *mount;
  PhxAttr *attrib;
  PhxRGBA colour;
  double x, y;
  int x_advance, width;
  unsigned char ch, *blPtr, *nlPtr;
  PhxObjectLabel *olbl = (PhxObjectLabel*)b;
  PhxLabelbuffer *lbuf = (PhxLabelbuffer*)olbl->exclusive;

  if ((lbuf->string == NULL) || (*lbuf->string == 0))  return;

  cairo_save(cr);

  attrib = olbl->attrib;
  cairo_select_font_face(cr, attrib->font_name,
                             attrib->font_var & 0x0FF,
                            (attrib->font_var >> 8) & 0x0FF);
  cairo_set_font_size(cr, attrib->font_size);

    /* Use mete, user may alter draw to be outside mete for positioning. */
  x = olbl->mete_box.x;
  y = olbl->mete_box.y;

    /* If a child, must include parent's mete.
      Child is in object's coordinates*/
  mount = b;
  while (mount != mount->o_mount) {
    mount = mount->o_mount;
    x += mount->mete_box.x;
    y += mount->mete_box.y;
  }

  cairo_rectangle(cr, x, y, olbl->mete_box.w, olbl->mete_box.h);
  cairo_clip(cr);

    /* colour mete instead of draw, see above note. */
  cairo_set_source_rgba(cr, attrib->bg_fill.r,
                            attrib->bg_fill.g,
                            attrib->bg_fill.b,
                            attrib->bg_fill.a);
  cairo_paint(cr);

  x += olbl->draw_box.x;
  y += olbl->draw_box.y;

    /* draw_box.w is largest glyph advance. */
    /* draw_box.h is the sum of font_em(s) olbl spans. */

  colour = attrib->fg_fill;
  if (colour.a != 0) {
    cairo_set_source_rgba(cr, colour.r, colour.g, colour.b, colour.a);
    cairo_rectangle(cr, x, y, olbl->draw_box.w, olbl->draw_box.h);
    cairo_fill(cr);
  }
/* for centered vertical */
  x += lbuf->adv_box.x;
  if ((olbl->state & VJST_MSK) == VJST_CTR)
    y += (olbl->draw_box.h - lbuf->adv_box.h) / 2;
  else if ((olbl->state & VJST_MSK) == VJST_BTM)
    y += (olbl->draw_box.h - lbuf->adv_box.h);

  colour = attrib->fg_ink;
  cairo_set_source_rgba(cr, colour.r, colour.g, colour.b, colour.a);

    /* if multi-line need each advance */
    /* Note: single line labels already incorporated any tabs */
  x_advance = 0, width = 0;
  blPtr = (unsigned char*)lbuf->string;
  nlPtr = (unsigned char*)strchr(lbuf->string, '\n');
  if (nlPtr == NULL) {
      /* not multi-line, 1 pass, width already known */
    nlPtr = blPtr + strlen((char*)blPtr);
    x_advance = lbuf->adv_box.w;
    ch = 0;
    goto nlp;
  }
  nlPtr = blPtr;
  ch = *nlPtr;
  do {
    int bx, whtsp;
    char *dummy = (char*)nlPtr;
    while (ch >= 0x020)  ch = *(++nlPtr);
    x_advance += ext_cairo_glyphs_advance(&dummy, ((char*)nlPtr - dummy), cr,
                              attrib->font_name,
                              attrib->font_var & 0x0FF,
                             (attrib->font_var >> 8) & 0x0FF,
                              attrib->font_size);
    if ((ch != 0) && (ch != '\n')) {  /* 0x00, 0x0A */
      if (ch == '\t') {  /* 0x09 */
        if (width == 0) {
          cairo_text_extents_t search_extents;
          char utf_str[4] = { 0x20, 0, 0, 0 };
          cairo_text_extents(cr, (const char*)&utf_str, &search_extents);
          width = TAB_TEXT_WIDTH * (unsigned)(search_extents.x_advance + 0.5);
        }
        x_advance += width;
      }
        /* NOTE: does not handle these 'space' classifications, nor 'cntrl' */
      /*else if (ch == '\r') */  /* 0x0D */
      /*else if (ch == '\f') */  /* 0x0C */
      /*else if (ch == '\v') */  /* 0x0B */
      ch = *(++nlPtr);
      continue;
    }
nlp:
    bx = 0;
    whtsp = olbl->draw_box.w - x_advance;
    if (whtsp > 0) {
      uint32_t jstfy = olbl->state & HJST_MSK;
      if  (jstfy == HJST_CTR) {
        whtsp /= 2;
      } else {
        if (jstfy == HJST_LFT) {
          whtsp = olbl->draw_box.x + BUTTON_TEXT_MIN;
        } else {
          whtsp -= (olbl->draw_box.x + BUTTON_TEXT_MIN);
        }
      }
      bx -= whtsp;
    }
    cairo_move_to(cr, x - bx, y + attrib->font_origin);
    ext_cairo_show_glyphs((char**)&blPtr, (nlPtr - blPtr), cr,
                              attrib->font_name,
                              attrib->font_var & 0x0FF,
                             (attrib->font_var >> 8) & 0x0FF,
                              attrib->font_size);

    if (ch == 0)  break;
    y += attrib->font_em;
    ch = *(blPtr = (++nlPtr));
    x_advance = 0;
  } while (1);

  cairo_restore(cr);
}

#pragma mark *** Events ***

static bool
_label_configure(PhxInterface *iface,
                    xcb_generic_event_t *nvt,
                    PhxObject *obj)  {

  xcb_configure_notify_event_t *configure;
  int16_t wD, hD;

  if ((obj->state & (HXPD_MSK | VXPD_MSK)) == 0)  return true;

  configure = (xcb_configure_notify_event_t*)nvt;
  wD = configure->width  - obj->mete_box.w;
  hD = configure->height - obj->mete_box.h;

  if ((obj->state & HXPD_MSK) == HXPD_LFT) {
    if ((obj->mete_box.x + wD) > 0)
      wD = -obj->mete_box.x;
    obj->mete_box.x += wD;
    obj->mete_box.x += wD;
    wD = -wD;
  }
  obj->mete_box.w += wD;
  obj->mete_box.w += wD;

  if ((obj->state & VXPD_MSK) == VXPD_TOP) {
    if ((obj->mete_box.y + hD) < 0)
      hD = -obj->mete_box.y;
    obj->mete_box.y += hD;
    obj->draw_box.y += hD;
    hD = -hD;
  }
  obj->mete_box.h += hD;
  obj->draw_box.h += hD;

  return true;
}

bool
_default_label_meter(PhxInterface *iface,
                     xcb_generic_event_t *nvt,
                     PhxObject *obj) {

  uint8_t response;

  DEBUG_EVENTS("_default_label_meter");

  response = nvt->response_type & (uint8_t)0x7F;

  if (response == XCB_ENTER_NOTIFY) {
    ui_cursor_set_named(NULL, obj->i_mount->window);
    return true;
  }

  if (response == XCB_LEAVE_NOTIFY)
    return true;

  if (response == XCB_CONFIGURE_NOTIFY)
    return _label_configure(iface, nvt, obj);

  return false;
}

#pragma mark *** Creation ***

static void
_text_font_reeval(cairo_t *cr, PhxAttr *attrib, int line_height) {

  cairo_font_extents_t font_extents;
  double font_size;

  cairo_select_font_face(cr, attrib->font_name,
                             attrib->font_var & 0x0FF,
                             (attrib->font_var >> 8) & 0x0FF);

  font_size = (double)line_height;
  cairo_set_font_size(cr, (double)line_height);
  cairo_font_extents(cr, &font_extents);
  while (line_height < (int)(font_extents.ascent + font_extents.descent)) {
    font_size -= 0.5;
    cairo_set_font_size(cr, font_size);
    cairo_font_extents(cr, &font_extents);
  }
  attrib->font_size   = font_size;
  attrib->font_origin = font_extents.ascent;
  attrib->font_em     = line_height;
}

static int
_text_label_fit(PhxObjectLabel *olbl, bool max) {

  char *sPtr;
  unsigned char ch0, *nlPtr;
  int line_height, ncnt, x_advance, max_advance, tab_width;
  cairo_t *cro;
  bool had_eval;
  PhxAttr *attrib;
  PhxLabelbuffer *lbuf = (PhxLabelbuffer*)olbl->exclusive;

  cro = cairo_create(olbl->i_mount->surface);
  attrib = olbl->attrib;
  line_height = attrib->font_em;
    /* get <newline> count to determine pixel line height of font */
  ncnt = 0;
  sPtr = lbuf->string;
  while (*sPtr != 0) {
    if (*sPtr == '\n') ncnt++;
    sPtr++;
  }

  if (ncnt != 0)
    line_height *= (ncnt + 1);
  if ((!max) && (line_height <= olbl->draw_box.h)) {
    cairo_select_font_face(cro, attrib->font_name,
                                attrib->font_var & 0x0FF,
                                (attrib->font_var >> 8) & 0x0FF);
    cairo_set_font_size(cro, line_height);
  } else {
    line_height = olbl->draw_box.h;
    if (ncnt != 0)  line_height /= (ncnt + 1);
    _text_font_reeval(cro, attrib, line_height);
  }

  had_eval = false;

reeval:
    /* note on reeval, tab width change. */
  x_advance = 0, max_advance = 0, tab_width = 0;
  sPtr = lbuf->string;
  nlPtr = (unsigned char*)strchr(lbuf->string, '\n');
  if (nlPtr == NULL)  nlPtr = (unsigned char*)(lbuf->string + lbuf->str_nil);
  ch0 = *sPtr;
  do {
    char *dummy = (char*)sPtr;
    while ((ch0 >= 0x020) && (sPtr < (char*)nlPtr))  ch0 = *(++sPtr);
    x_advance += ext_cairo_glyphs_advance(&dummy, ((char*)nlPtr - dummy), cro,
                              attrib->font_name,
                              attrib->font_var & 0x0FF,
                             (attrib->font_var >> 8) & 0x0FF,
                              attrib->font_size);
    if ((ch0 == 0) || (ch0 == '\n'))  break;
        /* NOTE: does not handle these 'space' classifications, nor 'cntrl' */
      /*else if (ch == '\r') */  /* 0x0D */
      /*else if (ch == '\f') */  /* 0x0C */
      /*else if (ch == '\v') */  /* 0x0B */
    if (ch0 == '\t') {  /* 0x09 */
      if (tab_width == 0) {
        cairo_text_extents_t search_extents;
        char utf_str[4] = { 0x20, 0, 0, 0 };
        cairo_text_extents(cro, (const char*)&utf_str, &search_extents);
        tab_width = TAB_TEXT_WIDTH * (unsigned)(search_extents.x_advance + 0.5);
      }
      x_advance += tab_width;
    }
    ch0 = *(++sPtr);
  } while (1);

    /* now verify x_advance fits olbl->draw_box.w */
    /* if exceeds, reset font variables based on x_advance */
  if (max_advance > x_advance)  x_advance = max_advance;
  if (x_advance > olbl->draw_box.w) {
      /* use ratio: line_height is to desired_width
                 as font_em is to x_advance */
    if (!had_eval) {
      double num;
      num = (olbl->draw_box.w - (BUTTON_TEXT_MIN * 1.5)) * attrib->font_em;
      line_height = (int)((num / (double)x_advance) + 0.9);
      had_eval = true;
    } else {
      line_height -= 1;
    }
    _text_font_reeval(cro, attrib, line_height);
    goto reeval;
  }
  lbuf->adv_box.x = 0;
  lbuf->adv_box.w = x_advance;

    /* assume possible resize, and since no vertical justify */
    /* want vertical centered */
  line_height = attrib->font_em;
  if (ncnt != 0)
    line_height += attrib->font_em * ncnt;
  if (olbl->draw_box.h < line_height) {
    line_height -= 1;
    _text_font_reeval(cro, attrib, line_height);
    goto reeval;
  }
  lbuf->adv_box.y = 0;
  lbuf->adv_box.h = line_height;

  cairo_destroy(cro);
  return x_advance;
}

void
 _label_text_max_fit(PhxObjectLabel *olbl) {
  _text_label_fit(olbl, true);
}

void
ui_label_text_set(PhxObjectLabel *olbl, char *str) {

  PhxLabelbuffer *lbuf = (PhxLabelbuffer*)olbl->exclusive;

  if (lbuf == NULL) {
    lbuf = (PhxLabelbuffer*)(olbl->exclusive = malloc(sizeof(PhxLabelbuffer)));
    memset(lbuf, 0, sizeof(PhxLabelbuffer));
  }
  if (lbuf->string != NULL)
    free(lbuf->string), lbuf->string = NULL;
  lbuf->str_nil = 0;
  if (str != NULL) {
    lbuf->string = strdup(str);
    lbuf->str_nil = (int)strlen(str);
  }
}

void
ui_label_font_em_set(PhxObjectLabel *obj, int16_t line_height) {

  PhxAttr *attrib = obj->attrib;
  ui_attributes_set((PhxObject*)obj, attrib->font_name,
                                     attrib->font_var & 0x0FF,
                                    (attrib->font_var >> 8) & 0x0FF,
                                     line_height);

  if (OBJECT_BASE_TYPE(obj) == PHX_LABEL) {
    char *text = ((PhxLabelbuffer*)obj->exclusive)->string;
    if ((text != NULL) && (*text != 0))
      _text_label_fit(obj, false);
  }
}

void
_default_label_raze(void *obj) {

  PhxObjectLabel *olbl = (PhxObjectLabel*)obj;
  PhxLabelbuffer *lbuf = (PhxLabelbuffer*)olbl->exclusive;
  if (lbuf->string != NULL)  free(lbuf->string);
  free(lbuf);
  olbl->exclusive = NULL;    /* For debug */
  _default_object_raze(obj);
}

PhxObjectLabel *
ui_label_create(PhxNexus *nexus, PhxRectangle configure,
                             char *text, uint32_t jstfy) {
  PhxObjectLabel *obj
    = (PhxObjectLabel*)ui_object_create(nexus, PHX_LABEL,
                                         ui_label_draw, configure);
  if (obj != NULL) {
    obj->state |= jstfy;
    obj->_event_cb = _default_label_meter;
    obj->_raze_cb  = _default_label_raze;
      /* default is foreground over transparent fills */
    obj->attrib->bg_fill.a = 0;
    obj->attrib->fg_fill.a = 0;

    ui_label_text_set(obj, text);
    if ((text != NULL) && (*text != 0))
      _text_label_fit(obj, false);
  }
  return obj;
}
