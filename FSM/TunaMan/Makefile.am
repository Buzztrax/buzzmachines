## $Id$

plugin_LTLIBRARIES = libFSM_TunaMan.la

libFSM_TunaMan_la_SOURCES = TunaMan.cpp
libFSM_TunaMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_TunaMan_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_TunaMan.so $(DESTDIR)$(plugindir)/FSM_TunaMan.so

