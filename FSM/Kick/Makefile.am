## $Id$

plugin_LTLIBRARIES = libFSM_Kick.la

libFSM_Kick_la_SOURCES = Kick.cpp
libFSM_Kick_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_Kick_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_Kick.so $(DESTDIR)$(plugindir)/FSM_Kick.so

preset_DATA = FSM_Kick.prs
demosong_DATA = FSM_Kick_Demo.bmx

