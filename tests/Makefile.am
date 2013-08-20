TESTS = gtestutils test test-async

COMMON_SRC = credentials.h

AM_CPPFLAGS = -I$(top_srcdir) $(LIBGFBGRAPH_CFLAGS)
AM_LDFLAGS = $(top_builddir)/gfbgraph/libgfbgraph-@API_VERSION@.la $(LIBGFBGRAPH_LIBS)

noinst_PROGRAMS = $(TESTS)

gtestutils_SOURCES = $(COMMON_SRC) \
	gtestutils.c

test_SOURCES = $(COMMON_SRC) \
	test.c

test_async_SOURCES = $(COMMON_SRC) \
	test-async.c

-include $(top_srcdir)/git.mk
