ACLOCAL_AMFLAGS = -I m4
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gasket-0.1.pc

DISTCHECK_CONFIGURE_FLAGS = --enable-introspection
EXTRA_DIST = m4/introspection.m4

SUBDIRS = src demo gasket
CLEANFILES = *~
