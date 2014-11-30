## $Id$

plugin_LTLIBRARIES = libLd_VocXP.la

libLd_VocXP_la_SOURCES = vocxp.cpp
libLd_VocXP_la_CXXFLAGS = $(BM_CXXFLAGS)
libLd_VocXP_la_LDFLAGS = $(BM_LIBS)
libLd_VocXP_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libLd_VocXP.so $(DESTDIR)$(plugindir)/Ld_VocXP.so
