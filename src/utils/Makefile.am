bin_PROGRAMS = gasket-reset

library_includedir = $(includedir)/gasket-0.1/gasket
## library_include_HEADERS = gasket/gasket-train.h

gasket_reset_LDADD = ${top_builddir}/src/client/libgasket-client-0.1.la $(GASKET_LIBS) $(GLIB_LIBS) $(GOBJECT_LIBS) $(GIO_UNIX_LIBS) $(LIBXML_LIBS) $(GIR_LIBS)
gasket_reset_CFLAGS = -I.. $(GASKET_CFLAGS) $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GIO_UNIX_CFLAGS) $(LIBXML_CFLAGS) $(GIR_CFLAGS)
gasket_reset_SOURCES = main_reset.c
AM_CPPFLAGS = -I${top_srcdir}/common

CLEANFILES = *~
