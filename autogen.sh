#!/bin/sh

set -e

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

olddir=`pwd`
cd "$srcdir"

GTKDOCIZE=`which gtkdocize`
if test -z $GTKDOCIZE; then
        echo "*** No GTK-Doc found, please install it ***"
        exit 1
else
        gtkdocize || exit $?
fi

ACLOCAL="${ACLOCAL-aclocal} $ACLOCAL_FLAGS" autoreconf -v -i

cd "$olddir"

test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
