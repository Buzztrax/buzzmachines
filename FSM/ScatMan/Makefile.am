## $Id$

plugin_LTLIBRARIES = libFSM_ScatMan.la

libFSM_ScatMan_la_SOURCES = ScatMan.cpp
libFSM_ScatMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_ScatMan_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_ScatMan.so $(DESTDIR)$(plugindir)/FSM_ScatMan.so

