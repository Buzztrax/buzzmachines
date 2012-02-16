## $Id$

plugin_LTLIBRARIES = libAsDev_a4pFilter01.la

libAsDev_a4pFilter01_la_SOURCES = a4pFilter01.cpp
libAsDev_a4pFilter01_la_CXXFLAGS = $(BM_CXXFLAGS)
libAsDev_a4pFilter01_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libAsDev_a4pFilter01.so $(DESTDIR)$(plugindir)/AsDev_a4pFilter01.so
