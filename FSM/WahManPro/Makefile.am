## $Id$

plugin_LTLIBRARIES = libFSM_WahManPro.la

libFSM_WahManPro_la_SOURCES = WahManPro.cpp
libFSM_WahManPro_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_WahManPro_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_WahManPro.so $(DESTDIR)$(plugindir)/FSM_WahManPro.so
