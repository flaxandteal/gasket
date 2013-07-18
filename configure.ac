AC_PREREQ(2.62)
AC_INIT([gasket], [0.1], [phil@philtweir.co.uk])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.9.6 -Wall -Werror -Wno-extra-portability dist-bzip2])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL

LT_INIT

AC_CONFIG_MACRO_DIR([m4])

GOBJECT_INTROSPECTION_CHECK([1.30.0])

PKG_CHECK_MODULES(GLIB, gobject-2.0)
PKG_CHECK_MODULES(GOBJECT, glib-2.0)
PKG_CHECK_MODULES(CAIRO, cairo-xlib)
PKG_CHECK_MODULES(RSVG, librsvg-2.0)
PKG_CHECK_MODULES(UUID, uuid)
PKG_CHECK_MODULES(GIR, gobject-introspection-1.0)
PKG_CHECK_MODULES(LIBXML, libxml-2.0)
PKG_CHECK_MODULES(GIO_UNIX, gio-unix-2.0)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    gasket/Makefile
    src/Makefile
    src/client/Makefile
    src/utils/Makefile
    test/Makefile
    gasket-0.1.pc
])
AC_OUTPUT
