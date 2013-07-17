bin_PROGRAMS = gasket-reset gasket-test

lib_LTLIBRARIES = libgasket-0.1.la
library_includedir = $(includedir)/gasket-0.1/gasket
dnl library_include_HEADERS = gasket/gasket-train.h

libgasket_0_1_la_LIBADD = $(GLIB_LIBS) $(GOBJECT_LIBS) $(GIO_UNIX_LIBS) $(LIBXML_LIBS) $(GIR_LIBS)
libgasket_0_1_la_CFLAGS = $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GIO_UNIX_CFLAGS) $(LIBXML_CFLAGS) $(GIR_CFLAGS)
libgasket_0_1_la_SOURCES = gasket-train.c gasket-train.h

gasket_reset_LDADD = libgasket-0.1.la $(GASKET_LIBS) $(GLIB_LIBS) $(GOBJECT_LIBS) $(GIO_UNIX_LIBS) $(LIBXML_LIBS) $(GIR_LIBS)
gasket_reset_CFLAGS = $(GASKET_CFLAGS) $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GIO_UNIX_CFLAGS) $(LIBXML_CFLAGS) $(GIR_CFLAGS)
gasket_reset_SOURCES = main.c

gasket_test_LDADD = libgasket-0.1.la $(GASKET_LIBS) $(GLIB_LIBS) $(GOBJECT_LIBS) $(GIO_UNIX_LIBS) $(LIBXML_LIBS) $(GIR_LIBS)
gasket_test_CFLAGS = $(GASKET_CFLAGS) $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GIO_UNIX_CFLAGS) $(LIBXML_CFLAGS) $(GIR_CFLAGS)
gasket_test_SOURCES = test-main.c

CLEANFILES = *~

-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = $(libgasket_0_1_la_SOURCES)
Gasket-0.1.gir: libgasket-0.1.la
Gasket_0_1_gir_INCLUDES = GObject-2.0
Gasket_0_1_gir_CFLAGS = $(INCLUDES)
Gasket_0_1_gir_LIBS = libgasket-0.1.la
Gasket_0_1_gir_FILES = $(introspection_sources)
Gasket_0_1_gir_EXPORT_PACKAGES = gasket-0.1
INTROSPECTION_GIRS += Gasket-0.1.gir

girdir = $(datadir)/gir-1.0
nodist_gir_DATA = Gasket-0.1.gir

typelibdir = $(libdir)/girepository-1.0
nodist_typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(nodist_gir_DATA) $(nodist_typelib_DATA)
endif
