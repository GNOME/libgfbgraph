lib_sources = \
	gfbgraph-album.c		\
	gfbgraph-authorizer.c		\
	gfbgraph-common.c		\
	gfbgraph-connectable.c		\
	gfbgraph-goa-authorizer.c	\
	gfbgraph-node.c			\
	gfbgraph-photo.c		\
	gfbgraph-user.c
lib_headers = \
	gfbgraph.h 			\
	gfbgraph-album.h		\
	gfbgraph-authorizer.h		\
	gfbgraph-common.h		\
	gfbgraph-connectable.h		\
	gfbgraph-goa-authorizer.h	\
	gfbgraph-node.h			\
	gfbgraph-photo.h		\
	gfbgraph-user.h

lib_LTLIBRARIES = libgfbgraph-@API_VERSION@.la

libgfbgraph_@API_VERSION@_la_CFLAGS = \
	-I$(top_srcdir)	-Wall -g -DG_LOG_DOMAIN=\"GFBGraph\"	\
	$(LIBGFBGRAPH_CFLAGS)					\
	$(GOA_API_CHANGE_CPPFLAGS) 				\
	$(SOUP_UNSTABLE_CPPFLAGS)				\
	$(GOA_CFLAGS)						\
	$(SOUP_CFLAGS)

libgfbgraph_@API_VERSION@_la_LDFLAGS = -no-undefined

libgfbgraph_@API_VERSION@_la_LIBADD = \
	$(LIBGFBGRAPH_LIBS)	\
	$(SOUP_LIBS)		\
	$(GOA_LIBS)

libgfbgraph_@API_VERSION@_la_SOURCES = $(lib_sources) $(lib_headers)

libgfbgraph_@API_VERSION@_la_HEADERS = $(lib_headers)

libgfbgraph_@API_VERSION@_ladir = $(includedir)/gfbgraph-@API_VERSION@/gfbgraph

# Introspection
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(libgfbgraph_@API_MAJOR@_@API_MINOR@_la_SOURCES)

GFBGraph-@API_VERSION@.gir: libgfbgraph-@API_VERSION@.la Makefile


GFBGraph_@API_MAJOR@_@API_MINOR@_gir_INCLUDES = \
	GObject-2.0	\
	Rest-0.7	\
	Json-1.0	\
	Gio-2.0		\
	Soup-2.4

GFBGraph_@API_MAJOR@_@API_MINOR@_gir_CFLAGS = \
	$(GOA_API_CHANGE_CPPFLAGS)	\
	$(SOUP_UNSTABLE_CPPFLAGS)	\
	$(LIBGFBGRAPH_CFLAGS)		\
	$(SOUP_CFLAGS)			\
	$(GOA_CFLAGS)			\
	-I$(top_srcdir)

GFBGraph_@API_MAJOR@_@API_MINOR@_gir_LIBS = libgfbgraph-@API_VERSION@.la
GFBGraph_@API_MAJOR@_@API_MINOR@_gir_FILES = $(lib_sources) $(lib_headers)
GFBGraph_@API_MAJOR@_@API_MINOR@_gir_NAMESPACE = GFBGraph
GFBGraph_@API_MAJOR@_@API_MINOR@_gir_EXPORT_PACKAGES = libgfbgraph.@API_VERSION@
GFBGraph_@API_MAJOR@_@API_MINOR@_gir_SCANNER_FLAGS = \
	--warn-all				\
	--nsversion=$(API_VERSION)		\
	--symbol-prefix=gfbgraph		\
	--identifier-prefix = GFBGraph		\
	--c-include="gfbgraph/gfbgraph.h"

INTROSPECTION_GIRS += GFBGraph-$(API_VERSION).gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = ../girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = $(gir_DATA) $(typelib_DATA)

endif

-include $(top_srcdir)/git.mk
