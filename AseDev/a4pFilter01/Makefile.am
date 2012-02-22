## $Id$

plugin_LTLIBRARIES = libAseDev_a4pFilter01.la

libAseDev_a4pFilter01_la_SOURCES = a4pFilter01.cpp
libAseDev_a4pFilter01_la_CXXFLAGS = $(BM_CXXFLAGS)
libAseDev_a4pFilter01_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libAseDev_a4pFilter01.so $(DESTDIR)$(plugindir)/AseDev_a4pFilter01.so

userdoc_DATA = AseDev_a4pFilter01.html
