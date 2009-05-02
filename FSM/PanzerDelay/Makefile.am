## $Id$

plugin_LTLIBRARIES = libFSM_PanzerDelay.la

libFSM_PanzerDelay_la_SOURCES = PanzerDelay.cpp
libFSM_PanzerDelay_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_PanzerDelay_la_LDFLAGS = $(BM_LIBS)
libFSM_PanzerDelay_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_PanzerDelay.so $(DESTDIR)$(plugindir)/FSM_PanzerDelay.so

preset_DATA = FSM_PanzerDelay.prs

