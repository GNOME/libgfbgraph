TESTS = gtestutils autoptr

AM_CPPFLAGS = -I$(top_srcdir) $(LIBGFBGRAPH_CFLAGS)
AM_LDFLAGS = $(top_builddir)/gfbgraph/libgfbgraph-@API_VERSION@.la $(LIBGFBGRAPH_LIBS)

noinst_PROGRAMS = $(TESTS)

gtestutils_SOURCES = gtestutils.c

autoptr_SOURCES = autoptr.c

-include $(top_srcdir)/git.mk
