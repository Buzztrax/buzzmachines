## $I$

plugin_LTLIBRARIES = libFSM_Chorus2.la

libFSM_Chorus2_la_SOURCES = Chorus2.cpp
libFSM_Chorus2_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_Chorus2_la_LDFLAGS = $(BM_LIBS)
libFSM_Chorus2_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_Chorus2.so $(DESTDIR)$(plugindir)/FSM_Chorus2.so
