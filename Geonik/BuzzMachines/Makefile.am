plugin_LTLIBRARIES = libGeonik_s_Compressor.la libGeonik_s_Omega_1.la libGeonik_s_PrimiFun.la

libGeonik_s_Compressor_la_SOURCES = Compressor.cpp
libGeonik_s_Compressor_la_CXXFLAGS = $(BM_CXXFLAGS)
libGeonik_s_Compressor_la_LDFLAGS = $(BM_LIBS)

libGeonik_s_Omega_1_la_SOURCES = Omega_1.cpp
libGeonik_s_Omega_1_la_CXXFLAGS = $(BM_CXXFLAGS)
libGeonik_s_Omega_1_la_LDFLAGS = $(BM_LIBS)

libGeonik_s_PrimiFun_la_SOURCES = PrimiFun.cpp
libGeonik_s_PrimiFun_la_CXXFLAGS = $(BM_CXXFLAGS)
libGeonik_s_PrimiFun_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	for lib in $(plugin_LTLIBRARIES); do $(RM) $(DESTDIR)$(plugindir)/$$lib; done
	mv $(DESTDIR)$(plugindir)/libGeonik_s_Compressor.so $(DESTDIR)$(plugindir)/Geonik_s_Compressor.so
	mv $(DESTDIR)$(plugindir)/libGeonik_s_Omega_1.so $(DESTDIR)$(plugindir)/Geonik_s_Omega_1.so
	mv $(DESTDIR)$(plugindir)/libGeonik_s_PrimiFun.so $(DESTDIR)$(plugindir)/Geonik_s_PrimiFun.so
