/*
 *
 * Copyright © 2013 Michael Stapelberg
 * Copyright © 2002 Keith Packard
 * Altered 240803 Steven Abner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the names of the authors or their
 * institutions shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the authors.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <xcb/xcb.h>
#include <xcb/render.h>

#include "endian.h"
#include "cursor.h"
#include "xcb_cursor.h"
#include "../image/xcb_image.h"

#if (__STDC_VERSION__ <= 199901L)
typedef uint8_t Boolean;
#else
typedef _Bool   Boolean;
#endif

static char *XCURSORPATH = "~/.icons:/usr/share/icons:/usr/share/pixmaps:"
                           "/usr/X11R6/lib/X11/icons";

static uint32_t
dist(const uint32_t a, const uint32_t b) {
  return (a > b ? (a - b) : (b - a));
}

static uint32_t
find_best_size(xcint_cursor_file_t *cf, const uint32_t target,
                                                       uint32_t *nsizesp) {
  uint32_t best = 0;
    /* Amount of cursors with the best size */
  uint32_t nsizes = 0;
  int n;
  for (n = 0; n < cf->header.ntoc; n++) {
    const uint32_t size = cf->tocs[n].subtype;

    if (cf->tocs[n].type != XCURSOR_IMAGE_TYPE)  continue;

      /* If the distance is less to the target size, this is a better fit. */
    if (best == 0 || dist(size, target) < dist(best, target)) {
      best = size;
      nsizes = 0;
    }

    if (size == best)
      nsizes++;
  }

  *nsizesp = nsizes;
  return best;
}

/* Returns true if and only if the read() call read the entirety of the data it
 * was supposed to read. */
static Boolean
read_entirely(int fd, void *buf, size_t count) {
  return (read(fd, buf, count) == count);
}

int
parse_cursor_file(xcb_cursor_context_t *c, const int fd,
                                          xcint_image_t **images, int *nimg) {
    /* Read the header, verify the magic value. */
  xcint_cursor_file_t cf;
  uint32_t nsizes = 0;
  uint32_t best = 0;
  uint32_t skip = 0;
  int n;
    /* The amount of images stored in 'images', used when cleaning up. */
  int cnt = 0;

  if (!read_entirely(fd, &(cf.header), sizeof(xcint_file_header_t)))
    return -EINVAL;

  cf.header.magic = le32toh(cf.header.magic);
  cf.header.header = le32toh(cf.header.header);
  cf.header.version = le32toh(cf.header.version);
  cf.header.ntoc = le32toh(cf.header.ntoc);

  if (cf.header.magic != XCURSOR_MAGIC)
    return -EINVAL;

  if ((skip = (cf.header.header - sizeof(xcint_file_header_t))) > 0)
    if (lseek(fd, skip, SEEK_CUR) == EOF)
      return -EINVAL;

  if (cf.header.ntoc > 0x10000)
    return -EINVAL;

    /* Read the table of contents */
  cf.tocs = malloc(cf.header.ntoc * sizeof(xcint_file_toc_t));
  if (!read_entirely(fd, cf.tocs, cf.header.ntoc * sizeof(xcint_file_toc_t)))
    goto error;

  for (n = 0; n < cf.header.ntoc; n++) {
    cf.tocs[n].type = le32toh(cf.tocs[n].type);
    cf.tocs[n].subtype = le32toh(cf.tocs[n].subtype);
    cf.tocs[n].position = le32toh(cf.tocs[n].position);
  }

    /* No images? Invalid file. */
  if ((best = find_best_size(&cf, c->size, &nsizes)) == 0 || nsizes == 0)
      goto error;

  *nimg = nsizes;
  if ((*images = calloc(nsizes, sizeof(xcint_image_t))) == NULL)
    goto error;

  for (n = 0; n < cf.header.ntoc; n++) {
    int j;
    xcint_chunk_header_t chunk;
      /* for convenience */
    xcint_image_t *i = &((*images)[cnt]);
    uint32_t numpixels = 0;
    uint32_t *p = NULL;

    if (cf.tocs[n].type != XCURSOR_IMAGE_TYPE ||
        cf.tocs[n].subtype != best)
      continue;

    lseek(fd, cf.tocs[n].position, SEEK_SET);
    if (!read_entirely(fd, &chunk, sizeof(xcint_chunk_header_t)))
      goto error2;
    chunk.header = le32toh(chunk.header);
    chunk.type = le32toh(chunk.type);
    chunk.subtype = le32toh(chunk.subtype);
    chunk.version = le32toh(chunk.version);
      /* Sanity check, as libxcursor does it. */
    if (chunk.type != cf.tocs[n].type ||
        chunk.subtype != cf.tocs[n].subtype)
      goto error2;
/* TODO: better type */
    if (!read_entirely(fd, i, sizeof(xcint_image_t) - sizeof(uint32_t*)))
      goto error2;
    i->width = le32toh(i->width);
    i->height = le32toh(i->height);
    i->xhot = le32toh(i->xhot);
    i->yhot = le32toh(i->yhot);
    i->delay = le32toh(i->delay);

      /* Read the actual image data and convert it to host byte order */
      /* Catch integer overflows */
    if (((uint64_t)i->width) * i->height > UINT32_MAX)
      goto error2;
    numpixels = i->width * i->height;
    i->pixels = malloc(numpixels * sizeof(uint32_t));
      /* With the malloc, one more image is eligible for cleanup later. */
    cnt++;
    if (!read_entirely(fd, i->pixels, numpixels * sizeof(uint32_t)))
        goto error2;
    p = i->pixels;
    for (j = 0; j < numpixels; j++, p++)
      *p = le32toh(*p);
  }

  free(cf.tocs);
  return 0;

error2:
    /* Free the memory for all images that were read so far. */
  for (n = 0; n < cnt; n++)
    free((*images)[n].pixels);
  free(*images);
error:
  *images = NULL;
  free(cf.tocs);
  return -EINVAL;
}

static const char *
cursor_path(struct xcb_cursor_context_t *c) {
  if (c->path == NULL) {
    c->path = getenv("XCURSOR_PATH");
    if (c->path == NULL)
      c->path = XCURSORPATH;
  }
  return c->path;
}

static const char *
next_path(const char *path) {
  const char *colon = strchr(path, ':');
  return (colon ? colon + 1 : NULL);
}

/*
 * _XcursorThemeInherits was directly copied from libxcursor so as to not break
 * compatibility.
 *
 */
#define XcursorWhite(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')
#define XcursorSep(c) ((c) == ';' || (c) == ',')

static char *
_XcursorThemeInherits(const char *full) {

  char    line[8192];
  char    *result = NULL;
  FILE    *f;

  if (full == NULL)  return NULL;

  f = fopen(full, "r");
  if (f) {
    while (fgets(line, sizeof(line), f)) {
      if (strncmp(line, "Inherits", 8) == 0) {
        char    *l = line + 8;
        char    *r;
        while (*l == ' ')  l++;
        if (*l != '=')  continue;
        l++;
        while (*l == ' ') l++;
        result = malloc(strlen(l));
        if (result != NULL) {
          r = result;
          while (*l) {
            while (XcursorSep(*l) || XcursorWhite(*l))  l++;
            if (*l == 0)  break;
            if (r != result)  *r++ = ':';
            while (*l && !XcursorWhite(*l) && !XcursorSep(*l))
              *r++ = *l++;
          }
          *r++ = '\0';
        }
        break;
      }
    }
    fclose(f);
  }
  return result;
}

/*
 * Tries to open the cursor file “name” in the “theme”/cursors subfolder of
 * each component of cursor_path(). When the file cannot be found, but a file
 * “index.theme” in the component is present, the Inherits= key will be
 * extracted and open_cursor_file calls itself recursively to search the
 * specified inherited themes, too.
 *
 */
static int
open_cursor_file(xcb_cursor_context_t *c, const char *theme,
                                          const char *name, int *scan_core) {
  int fd = -1;
  char *inherits = NULL;
  char *full;
  const char *path;
  const char *pend;
  const char *sep;
  int themelen;

  *scan_core = -1;

  if ( (strcmp(theme, "core") == 0)
      && ((*scan_core = cursor_shape_to_id(name)) >= 0) )
    return -1;

  if ( (c->home == NULL) && ((c->home = getenv("HOME")) == NULL) )
    return -1;

  path = cursor_path(c);
  pend = path + strlen(path);
  themelen = strlen(theme) + 1;
    /* use Linux PATH_MAX */
  full = malloc(4096);

  do {
    int pathlen;
    sep = strchr(path, ':');
    if (sep == NULL)  sep = pend;
    pathlen = sep - path;
    if (*path == '~') {
      snprintf(full, 4096, "%s%.*s/%s/%s/%s",
                    c->home, pathlen - 1, path + 1, theme, "cursors", name);
      pathlen += strlen(c->home) - 1;
    } else {
      snprintf(full, 4096, "%.*s/%s/%s/%s",
                    pathlen, path, theme, "cursors", name);
    }
    if ((fd = open(full, O_RDONLY)) != -1)  break;
    if (inherits == NULL) {
      snprintf((full + pathlen + themelen), 4096, "/index.theme");
      inherits = _XcursorThemeInherits(full);
    }
  } while ((path = sep + 1) < pend);
  free(full);

  for (path = inherits;
       (path != NULL && fd == -1);
       (path = next_path(path))) {
    fd = open_cursor_file(c, path, name, scan_core);
  }

  if (inherits != NULL)
      free(inherits);

  return fd;
}

static xcb_cursor_t
_cursor_from_file(xcb_cursor_context_t *c, int fd) {

  xcint_image_t *images;
  int nimg = 0;
  xcb_pixmap_t pixmap = XCB_NONE;
  xcb_gcontext_t gc = 0;
  xcb_render_picture_t pic;
  uint32_t last_width = 0;
  uint32_t last_height = 0;
  xcb_cursor_t cid;
  int n;
  xcb_render_animcursorelt_t *elements;

  if (parse_cursor_file(c, fd, &images, &nimg) < 0) {
    close(fd);
    return XCB_NONE;
  }
  close(fd);

    /* create a cursor from it */
  /* xcb_render_animcursorelt_t elements[nimg]; */
  elements = calloc(nimg, sizeof(xcb_render_animcursorelt_t));
  pic = xcb_generate_id(c->conn);

  for (n = 0; n < nimg; n++) {
    xcint_image_t *i = &(images[n]);
    xcb_image_t *img = xcb_image_create_native(c->conn, i->width, i->height,
                                               XCB_IMAGE_FORMAT_Z_PIXMAP, 32,
                                               NULL,
                                    (i->width * i->height * sizeof(uint32_t)),
                                               (uint8_t*)i->pixels);

    if ( (i->width != last_width) || (i->height != last_height) ) {
      last_width = i->width;
      last_height = i->height;
      if (pixmap != XCB_NONE) {
        xcb_free_pixmap(c->conn, pixmap);
        xcb_free_gc(c->conn, gc);
      }
      pixmap = xcb_generate_id(c->conn);
      gc = xcb_generate_id(c->conn);
      xcb_create_pixmap(c->conn, 32, pixmap, c->root, i->width, i->height);
      xcb_create_gc(c->conn, gc, pixmap, 0, NULL);
    }

    xcb_image_put(c->conn, pixmap, gc, img, 0, 0, 0);

    xcb_render_create_picture(c->conn, pic, pixmap, c->pict_format->id, 0, NULL);

    elements[n].cursor = xcb_generate_id(c->conn);
    elements[n].delay = i->delay;

    xcb_render_create_cursor(c->conn, elements[n].cursor, pic, i->xhot, i->yhot);

    xcb_render_free_picture(c->conn, pic);
    xcb_image_destroy(img);
    free(i->pixels);
  }

  xcb_free_pixmap(c->conn, pixmap);
  if (gc != 0)
    xcb_free_gc(c->conn, gc);
  free(images);

  if (nimg == 1 || c->render_version == RV_CURSOR) {
      /* non-animated cursor or no support for animated cursors */
    cid = elements[0].cursor;
    free(elements);
    return cid;
  }

  cid = xcb_generate_id(c->conn);
  xcb_render_create_anim_cursor(c->conn, cid, nimg, elements);

  for (n = 0; n < nimg; n++)
    xcb_free_cursor(c->conn, elements[n].cursor);

  free(elements);
  return cid;
}


/* The character id of the X11 "cursor" font when falling back to un-themed
 * cursors. */
xcb_cursor_t
xcb_cursor_load_cursor(xcb_cursor_context_t *c, const char *name) {

  int core_char = -1;
  int fd = -1;

    /* NB: if !render_present, fd will be -1 and thus the next if statement
       will trigger the fallback. */
  if (c->render_version != RV_NONE) {
    if (c->rm[RM_XCURSOR_THEME])
      fd = open_cursor_file(c, c->rm[RM_XCURSOR_THEME], name, &core_char);

    if (fd == -1 && core_char == -1)
      fd = open_cursor_file(c, "default", name, &core_char);
  }

  if (fd == -1 || core_char > -1) {
    xcb_cursor_t cid;
    if ( (core_char == -1) && (name != NULL) )
      core_char = cursor_shape_to_id(name);
    if (core_char == -1)
      return XCB_NONE;

    cid = xcb_generate_id(c->conn);
    xcb_create_glyph_cursor(c->conn, cid, c->cursor_font,
                            c->cursor_font, core_char, core_char + 1,
                            0, 0, 0, 65535, 65535, 65535);
    return cid;
  }
  return _cursor_from_file(c, fd);
}
