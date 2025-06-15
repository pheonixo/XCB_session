FORGE = forge/
# XTRA: xcb code needed but not included with xcb
XXTRA = \
  cursor/cursor.c \
  cursor/endian.c \
  cursor/load_cursor.c \
  cursor/shape_to_id.c \
  renderutil/cache.c \
  renderutil/glyph.c \
  renderutil/util.c \
  image/xcb_image.c \
  util/xcb_aux.c

LXTRA = -lxcb-shm -lxcb-render

LLIBS = -lxcb -lcairo $(LXTRA)

# HDR/SCR: Pheonix code creation
XHDR = \
  session.h \
  statebits.h \
  atoms.h \
  windows.h \
  events.h \
  clipboard.c \
  drag.h \
  timers.h \
  draw.h \
  configure.h \
  nexus.h \
  nfuse.h \
  gfuse.h \
  objects.h \
  banks.h \
  buttons.h \
  labels.h \
  textuals.h \
  textviews.h \
  textviews_drag.h \
  findport.h

XSRC = \
  session.c \
  atoms.c \
  windows.c \
  events.c \
  clipboard.c \
  drag.c \
  timers.c \
  events_debug.c \
  draw.c \
  configure.c \
  nexus.c \
  nfuse.c \
  gfuse.c \
  objects.c \
  banks.c \
  buttons.c \
  labels.c \
  textuals.c \
  textviews.c \
  textviews_drag.c \
  findport.c \
  $(XXTRA)

CFLAGS := -ansi -pedantic -Wall -Wno-unknown-pragmas -O0 -g

# XLIB: add on code for UTF8 handling
SRC := $(XSRC) ext_cairo.c
LIBS := $(LLIBS) -lm -I/usr/include/freetype2 -lfreetype
# because of freetype
CFLAGS := $(CFLAGS) -Wno-long-long

libctype.a:
	make -C ./libctype
	@cp ./libctype/build/libctype.a ./

# session: window creation
xxssn: $(FORGE)xcb_session.c $(SRC) $(XHDR) makefile
	@echo "  CC -o xxssn"
	@gcc $(CFLAGS) -o xxssn $(FORGE)xcb_session.c  $(SRC) $(LIBS)

# session: two window creation
xxss2: $(FORGE)xcb_session2.c $(SRC) $(XHDR) makefile
	@echo "  CC -o xxss2"
	@gcc $(CFLAGS) -o xxss2 $(FORGE)xcb_session2.c $(SRC) $(LIBS)

# nexus: display port creation
xxnex: $(FORGE)xcb_nexus.c   $(SRC) $(XHDR) makefile
	@echo "  CC -o xxnex"
	@gcc $(CFLAGS) -o xxnex $(FORGE)xcb_nexus.c    $(SRC) $(LIBS)

# gfuse: 2 window test of gravities north and east abilites
xxgne: $(FORGE)xcb_gfne.c    $(SRC) $(XHDR) makefile 
	@echo "  CC -o xxgne"
	@gcc $(CFLAGS) -o xxgne $(FORGE)xcb_gfne.c     $(SRC) $(LIBS)

# gfuse: grippable display port
xxfse: $(FORGE)xcb_gfuse.c   $(SRC) $(XHDR) makefile 
	@echo "  CC -o xxfse"
	@gcc $(CFLAGS) -o xxfse $(FORGE)xcb_gfuse.c    $(SRC) $(LIBS)

# object button (multiple button objects)
xxbtn: $(FORGE)xcb_buttons.c $(SRC) $(XHDR) makefile
	@echo "  CC -o xxbtn"
	@gcc $(CFLAGS) -o xxbtn $(FORGE)xcb_buttons.c  $(SRC) $(LIBS)

# object textview (single for internal drag testing)
xxtv1: $(FORGE)xcb_textview_drag.c  $(SRC) $(XHDR) libctype.a makefile
	@echo "  CC -o xxtv1"
	@gcc $(CFLAGS) -o xxtv1 $(FORGE)xcb_textview_drag.c $(SRC) $(LIBS) -L./ -lctype

# object textview (3 internal entities: textview, label, textview)
# 2 windows: one as nexus w/ 3 objects, other as
#            three nexus each with 1 object.
xxtxt: $(FORGE)xcb_textview.c  $(SRC) $(XHDR) libctype.a makefile
	@echo "  CC -o xxtxt"
	@gcc $(CFLAGS) -o xxtxt $(FORGE)xcb_textview.c $(SRC) $(LIBS) -L./ -lctype

# object gfused textview (3 internal entities: textview, label, textview)
# gfuse containing three nexus each with 1 object.
xxgtx: $(FORGE)xcb_gfuse_textview.c  $(SRC) $(XHDR) libctype.a makefile
	@echo "  CC -o xxgtx"
	@gcc $(CFLAGS) -o xxgtx $(FORGE)xcb_gfuse_textview.c $(SRC) $(LIBS) -L./ -lctype

# object combo button (creates bank)
xxbnk: $(FORGE)xcb_banks.c   $(SRC) $(XHDR) makefile
	@echo "  CC -o xxbnk"
	@gcc $(CFLAGS) -o xxbnk $(FORGE)xcb_banks.c     $(SRC) $(LIBS)

# objects as 'editor bar', navigatable combo button (4 variations)
xxebr: $(FORGE)xcb_ebar.c    $(SRC) $(XHDR) makefile
	@echo "  CC -o xxebr"
	@gcc $(CFLAGS) -o xxebr $(FORGE)xcb_ebar.c      $(SRC) $(LIBS)

#  Design of Findport
# nexus findbar (textview add on, 2 nexus: 1 w/textview, findbar)
xxfbr: $(FORGE)xcb_fbar.c $(FORGE)xcb_fbar.h  $(SRC) $(XHDR) libctype.a makefile
	@echo "  CC -o xxfbr"
	@gcc $(CFLAGS) -o xxfbr $(FORGE)xcb_fbar.c $(SRC) $(LIBS) -L./ -lctype
# nexus findbar (multiple for size relationships)
xxfbr2: $(FORGE)xcb_fbar2.c $(FORGE)xcb_fbar.h $(SRC) $(XHDR) libctype.a makefile
	@echo "  CC -o xxfbr2"
	@gcc $(CFLAGS) -o xxfbr2 $(FORGE)xcb_fbar2.c $(SRC) $(LIBS) -L./ -lctype

#  Design of Headerport
# nexus hearderbar (textview add on)
xxhbr: $(FORGE)xcb_hbar.c $(SRC) $(XHDR) libctype.a makefile
	@echo "  CC -o xxhbr"
	@gcc $(CFLAGS) -o xxhbr $(FORGE)xcb_hbar.c $(SRC) $(LIBS) -L./ -lctype

xxhbr2: $(FORGE)xcb_hbar2.c $(SRC) $(XHDR) libctype.a makefile
	@echo "  CC -o xxhbr2"
	@gcc $(CFLAGS) -o xxhbr2 $(FORGE)xcb_hbar2.c $(SRC) $(LIBS) -L./ -lctype

all: xxssn xxss2 xxnex xxgne xxfse xxbtn xxtv1 xxtxt xxgtx xxbnk xxebr \
xxfbr xxfbr2 xxhbr
