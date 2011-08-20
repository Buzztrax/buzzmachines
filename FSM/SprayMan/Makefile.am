## $Id$

plugin_LTLIBRARIES = libFSM_SprayMan.la

libFSM_SprayMan_la_SOURCES = SprayMan.cpp
libFSM_SprayMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_SprayMan_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_SprayMan.so $(DESTDIR)$(plugindir)/FSM_SprayMan.so

