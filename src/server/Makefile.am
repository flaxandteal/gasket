lib_LTLIBRARIES = libgasket-0.1.la

libgasket_0_1_la_LIBADD = $(GLIB_LIBS) $(GOBJECT_LIBS) $(GIO_UNIX_LIBS) $(LIBXML_LIBS) $(GIR_LIBS) $(CAIRO_LIBS) $(RSVG_LIBS) $(UUID_LIBS)
libgasket_0_1_la_CFLAGS = -g -O0 $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GIO_UNIX_CFLAGS) $(LIBXML_CFLAGS) $(GIR_CFLAGS) $(CAIRO_CFLAGS) $(RSVG_CFLAGS) $(UUID_CFLAGS)
libgasket_0_1_la_SOURCES = server.c cairo.c
AM_CPPFLAGS = -I${top_srcdir}/common

CLEANFILES = *~
