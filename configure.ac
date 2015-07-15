m4_define([api_major], [0])
m4_define([api_minor], [2])
m4_define([api_micro], [4])

m4_define([gfbgraph_version], [api_major.api_minor.api_micro])

AC_PREREQ([2.63])
AC_INIT([gfbgraph], [gfbgraph_version],
        [https://bugzilla.gnome.org/enter_bug.cgi?product=gfbgraph],
        [gfbgraph],
        [https://github.com/alvaropg/libgfbgraph.git])

AC_CONFIG_SRCDIR([gfbgraph/gfbgraph.h])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 -Wno-portability])

AM_SILENT_RULES([yes])

API_MAJOR=api_major
API_MINOR=api_minor
AC_SUBST([API_MAJOR],[$API_MAJOR])
AC_SUBST([API_MINOR],[$API_MINOR])
AC_SUBST([API_VERSION],[$API_MAJOR.$API_MINOR])
AC_SUBST([API_VERSION_AM],[$API_MAJOR\_$API_MINOR])
AC_DEFINE_UNQUOTED(API_VERSION, [$API_VERSION], [API version])

AC_PROG_CC

LT_INIT

# check for gtk-doc
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

GOBJECT_INTROSPECTION_CHECK([1.30.0])

PKG_CHECK_MODULES(LIBGFBGRAPH, [glib-2.0 gio-2.0 gobject-2.0 rest-0.7 json-glib-1.0])

PKG_CHECK_MODULES(SOUP, [libsoup-2.4])
SOUP_UNSTABLE_CPPFLAGS=-DLIBSOUP_USE_UNSTABLE_REQUEST_API
AC_SUBST(SOUP_UNSTABLE_CPPFLAGS)

PKG_CHECK_MODULES(GOA, [goa-1.0])
GOA_API_CHANGE_CPPFLAGS=-DGOA_API_IS_SUBJECT_TO_CHANGE
AC_SUBST(GOA_API_CHANGE_CPPFLAGS)

AC_OUTPUT([
Makefile
libgfbgraph.pc
docs/Makefile
docs/reference/Makefile
docs/reference/apiversion.xml
docs/reference/version.xml
gfbgraph/Makefile
tests/Makefile])
