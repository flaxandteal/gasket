#!/bin/sh
# $Id$

if [ "x$(uname)" = "xOpenBSD" ]; then
	[ -z "$AUTOMAKE_VERSION" ] && export AUTOMAKE_VERSION=1.10
	[ -z "$AUTOCONF_VERSION" ] && export AUTOCONF_VERSION=2.65
fi

die()
{
    echo "$@" >&2
    exit 1
}

mkdir -p build-aux
aclocal || die "aclocal failed"
libtoolize -f || die "libtoolize failed"
autoheader || die "autoheader failed"
automake --add-missing --force-missing --copy --foreign || die "automake failed"
autoreconf || die "autoreconf failed"
