plugin_LTLIBRARIES = libWhiteNoise_StereoDist.la

libWhiteNoise_StereoDist_la_SOURCES = Dist.cpp
libWhiteNoise_StereoDist_la_CXXFLAGS = $(BM_CXXFLAGS)
libWhiteNoise_StereoDist_la_LDFLAGS = $(BM_LIBS)
libWhiteNoise_StereoDist_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libWhiteNoise_StereoDist.so $(DESTDIR)$(plugindir)/WhiteNoise_StereoDist.so
