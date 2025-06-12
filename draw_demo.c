#include "nexus.h"

/* For testing/design setup, not to be part of finished code */
static uint16_t colour_select = 0;
/* For testing, visual backgounds for display port individuality */
/* Used to configure nexus, and in its draw */
static const int16_t rsz = 14;

/* For testing/design setup, not to be part of finished code */
static const PhxRGBA  colours[12] = {
  { 1, 1, 0, 1 },
  { 1, 0, 0, 1 },
  { 1, 0, 1, 1 },
  { 0, 0, 1, 1 },
  { 0, 1, 1, 1 },
  { 0, 1, 0, 1 },
  { 1, .5, 0, 1 },
  { 1, 0, .5, 1 },
  { .5, 0, 1, 1 },
  { 0, .5, 1, 1 },
  { 0, 1, .5, 1 },
  { .5, 1, 0, 1 }
};
static cairo_surface_t *pix[12] = { NULL };

/* For testing/design setup, not to be part of finished code */
static void
_draw_header_button(PhxRectangle *dbox, const PhxRGBA *c, cairo_t *cr) {

  cairo_matrix_t matrix;
  cairo_pattern_t *r1;

  double lw;
  double top = dbox->y;
  double bottom = dbox->y + dbox->h;
  double radius = dbox->h / 2;
  double xc = dbox->x + (dbox->w / 2);
  double yc = dbox->y + (dbox->h / 2);

  cairo_save(cr);

  cairo_new_sub_path(cr);
  cairo_arc(cr, xc, yc, radius, 0, M_PI * 2);
  cairo_clip_preserve(cr);

    /* fill of button draw_box area */
  cairo_set_source_rgba(cr, 1, 1, 1, 1);
  cairo_fill_preserve(cr);

    /* fill in actual content */
  cairo_get_matrix(cr, &matrix);

    /* fill in circle */
  cairo_translate(cr, xc, bottom + 6.5);
  cairo_scale(cr, 1, 0.7);
  cairo_translate(cr, -xc, -(bottom + 6.5));
  r1 = cairo_pattern_create_radial(xc, yc, 6,
                                   xc, yc, (double)(dbox->h + 8.0));
  cairo_pattern_add_color_stop_rgba(r1, 0, c->r, c->g, c->b, c->a);
  cairo_pattern_add_color_stop_rgba(r1, 1, (c->r - 0.45),
                                           (c->g - 0.45),
                                           (c->b - 0.45),
                                           (c->a - 0.15));
  cairo_set_source(cr, r1);
  cairo_fill_preserve(cr);
  cairo_pattern_destroy(r1);

  cairo_set_matrix(cr, &matrix);

  r1 = cairo_pattern_create_radial(xc - 0.7, top + 3.5, .35,
                                   xc - 0.7, top + 3.5, 8);
  cairo_pattern_add_color_stop_rgba(r1, 0, (c->r + 0.65),
                                           (c->g + 0.65),
                                           (c->b + 0.65),
                                            c->a);

  cairo_pattern_add_color_stop_rgba(r1, 1, c->r,  c->g,  c->b, .08);
  cairo_set_source(cr, r1);
  cairo_fill_preserve(cr);
  cairo_pattern_destroy(r1);

    /* colour of button border */
  cairo_set_source_rgba(cr, 0, 0, 0, 1);
  lw = 0.5;
  cairo_set_line_width(cr, lw);
  cairo_stroke(cr);

  cairo_restore(cr);
}

void
_demo_draw(PhxObject *obj, cairo_t *cr) {

  PhxNexus *nexus = (PhxNexus*)obj;

  int16_t cdx = colour_select;
  const PhxRGBA *c = &colours[cdx];

  PhxRectangle *dbox = &nexus->draw_box;

    /* Clear backgound surface. */
  cairo_save(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint(cr);
  cairo_restore(cr);

    /* Clip to drawing area. */
  cairo_rectangle(cr, 0, 0, dbox->w, dbox->h);
  cairo_clip(cr);

  cairo_rectangle(cr, dbox->x, dbox->y, dbox->w, dbox->h);
  cairo_set_source_rgba(cr, (1 - c->r),
                            (1 - c->g),
                            (1 - c->b), 1);
  cairo_fill(cr);

  cairo_set_source_surface(cr, pix[cdx],
                               dbox->x + 4,
                               dbox->y + 4);
  cairo_paint(cr);
  cairo_set_source_surface(cr, pix[cdx],
                               dbox->x + dbox->w - (4 + rsz),
                               dbox->y + 4);
  cairo_paint(cr);
  cairo_set_source_surface(cr, pix[cdx],
                               dbox->x + dbox->w - (4 + rsz),
                               dbox->y + dbox->h - (4 + rsz));
  cairo_paint(cr);
  cairo_set_source_surface(cr, pix[cdx],
                               dbox->x + 4,
                               dbox->y + dbox->h - (4 + rsz));
  cairo_paint(cr);

  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
    puts("error: _demo_draw()");
}

static void
DrawDemoInitialize(void) {
    /* pix[] for testing/design setup, not to be part of finished code */
  uint8_t pdx;
  for (pdx = 0; pdx < 12; pdx++) {
    PhxRectangle bbox;
    cairo_t *cr;
    pix[pdx] = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, rsz, rsz);
    cr = cairo_create(pix[pdx]);
    RECTANGLE(bbox, 0, 0, rsz, rsz);
    _draw_header_button(&bbox, &colours[pdx], cr);
    cairo_destroy(cr);
    cairo_surface_flush(pix[pdx]);
  }
}
