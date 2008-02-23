## $I$

plugin_LTLIBRARIES = libFSM_PhatMan.la

libFSM_PhatMan_la_SOURCES = Chorus.cpp
libFSM_PhatMan_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_PhatMan_la_LDFLAGS = $(BM_LIBS)
libFSM_PhatMan_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_PhatMan.so $(DESTDIR)$(plugindir)/FSM_PhatMan.so
