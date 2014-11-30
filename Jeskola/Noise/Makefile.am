plugin_LTLIBRARIES = libJeskola_Noise.la

libJeskola_Noise_la_SOURCES = Noise.cpp
libJeskola_Noise_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_Noise_la_LDFLAGS = $(BM_LIBS)
libJeskola_Noise_la_LIBADD =  $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_Noise.so $(DESTDIR)$(plugindir)/Jeskola_Noise.so
