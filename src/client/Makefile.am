lib_LTLIBRARIES = libgasket-client-0.1.la
AM_CPPFLAGS = -I${top_srcdir}/common

libgasket_client_0_1_la_LIBADD = $(GLIB_LIBS) $(GOBJECT_LIBS) $(GIO_UNIX_LIBS) $(LIBXML_LIBS) $(GIR_LIBS)
libgasket_client_0_1_la_CFLAGS = $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) $(GIO_UNIX_CFLAGS) $(LIBXML_CFLAGS) $(GIR_CFLAGS)
libgasket_client_0_1_la_SOURCES = gasket-train.c ${top_srcdir}/common/gasket/gasket-train.h

CLEANFILES = *~

-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --add-include-path=${top_srcdir}/common/gasket
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir) --includedir=${top_srcdir}/common/gasket

introspection_sources = $(libgasket_client_0_1_la_SOURCES)
Gasket-0.1.gir: libgasket-client-0.1.la
Gasket_0_1_gir_INCLUDES = GObject-2.0
Gasket_0_1_gir_CFLAGS = $(INCLUDES)
Gasket_0_1_gir_LIBS = libgasket-client-0.1.la
Gasket_0_1_gir_FILES = $(introspection_sources)
Gasket_0_1_gir_EXPORT_PACKAGES = gasket-0.1
INTROSPECTION_GIRS += Gasket-0.1.gir

girdir = $(datadir)/gir-1.0
nodist_gir_DATA = Gasket-0.1.gir

typelibdir = $(libdir)/girepository-1.0
nodist_typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(nodist_gir_DATA) $(nodist_typelib_DATA)
endif
