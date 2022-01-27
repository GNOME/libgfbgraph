SUBDIRS = gfbgraph docs tests
ACLOCAL_AMFLAGS = -I m4

doc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

%-$(API_VERSION).pc: %.pc
	$(AM_V_GEN)cp $< $@

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = libgfbgraph-$(API_VERSION).pc

CLEANFILES = $(pkgconfig_DATA)

all-local: libgfbgraph-$(API_VERSION).pc

EXTRA_DIST = $(libgfbgraphdoc_DATA)

# Introspection
DISTCHECK_CONFIGURE_FLAGS = --enable-introspection
EXTRA_DIST += m4/introspection.m4

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(libgfbgraphdocdir)

-include $(top_srcdir)/git.mk
