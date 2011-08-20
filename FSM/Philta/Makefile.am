## $Id$

plugin_LTLIBRARIES = libFSM_Philta.la

libFSM_Philta_la_SOURCES = Philta.cpp
libFSM_Philta_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_Philta_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_Philta.so $(DESTDIR)$(plugindir)/FSM_Philta.so

