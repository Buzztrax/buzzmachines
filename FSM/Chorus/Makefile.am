## $Id$

plugin_LTLIBRARIES = libFSM_Chorus.la

libFSM_Chorus_la_SOURCES = Chorus.cpp
libFSM_Chorus_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_Chorus_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_Chorus.so $(DESTDIR)$(plugindir)/FSM_Chorus.so
