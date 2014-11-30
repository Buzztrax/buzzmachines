plugin_LTLIBRARIES = libFSM_WahMan.la

libFSM_WahMan_la_SOURCES = WahMan.cpp
libFSM_WahMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_WahMan_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_WahMan.so $(DESTDIR)$(plugindir)/FSM_WahMan.so
