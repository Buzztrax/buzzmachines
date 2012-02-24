## $Id$

plugin_LTLIBRARIES = libLd_Gate.la

libLd_Gate_la_SOURCES = gate.cpp
libLd_Gate_la_CXXFLAGS = $(BM_CXXFLAGS)
libLd_Gate_la_LDFLAGS = $(BM_LIBS)
libLd_Gate_la_LIBADD =  $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libLd_Gate.so $(DESTDIR)$(plugindir)/Ld_Gate.so
