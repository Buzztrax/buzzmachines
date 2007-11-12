## $I$

plugin_LTLIBRARIES = libFSM_Infector.la

libFSM_Infector_la_SOURCES = Channel.cpp FFT.cpp Filters.cpp NewFilters.cpp Track.cpp Vegetable.cpp
libFSM_Infector_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_Infector_la_LDFLAGS = $(BM_LIBS)
libFSM_Infector_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_Infector.so $(DESTDIR)$(plugindir)/FSM_Infector.so
