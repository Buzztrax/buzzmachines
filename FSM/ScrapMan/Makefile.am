plugin_LTLIBRARIES = libFSM_ScrapMan.la

libFSM_ScrapMan_la_SOURCES = ScrapMan.cpp
libFSM_ScrapMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_ScrapMan_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_ScrapMan.so $(DESTDIR)$(plugindir)/FSM_ScrapMan.so

