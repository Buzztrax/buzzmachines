plugin_LTLIBRARIES = libFSM_ArpMan.la

libFSM_ArpMan_la_SOURCES = ArpMan.cpp
libFSM_ArpMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_ArpMan_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_ArpMan.so $(DESTDIR)$(plugindir)/FSM_ArpMan.so
