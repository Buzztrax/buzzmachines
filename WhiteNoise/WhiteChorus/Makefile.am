plugin_LTLIBRARIES = libWhiteNoise_WhiteChorus.la

libWhiteNoise_WhiteChorus_la_SOURCES = Chorus.cpp
libWhiteNoise_WhiteChorus_la_CXXFLAGS = $(BM_CXXFLAGS)
libWhiteNoise_WhiteChorus_la_LDFLAGS = $(BM_LIBS)
libWhiteNoise_WhiteChorus_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libWhiteNoise_WhiteChorus.so $(DESTDIR)$(plugindir)/WhiteNoise_WhiteChorus.so
