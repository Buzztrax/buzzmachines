## $Id$

plugin_LTLIBRARIES = libFSM_TapMan.la

libFSM_TapMan_la_SOURCES = TapMan.cpp
libFSM_TapMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_TapMan_la_LDFLAGS = $(BM_LIBS)
libFSM_TapMan_la_LIBADD = $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_TapMan.so $(DESTDIR)$(plugindir)/FSM_TapMan.so

