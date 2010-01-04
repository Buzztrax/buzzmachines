## $Id$

plugin_LTLIBRARIES = libGeonik_s_Omega1.la libGeonik_s_PrimiFun.la

libGeonik_s_Omega1_la_SOURCES = Omega1.cpp
libGeonik_s_Omega1_la_CXXFLAGS = $(BM_CXXFLAGS)
libGeonik_s_Omega1_la_LDFLAGS = $(BM_LIBS)

libGeonik_s_PrimiFun_la_SOURCES = PrimiFun.cpp
libGeonik_s_PrimiFun_la_CXXFLAGS = $(BM_CXXFLAGS)
libGeonik_s_PrimiFun_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libGeonik_s_Omega1.so $(DESTDIR)$(plugindir)/Geonik_s_Omega1.so
	mv $(DESTDIR)$(plugindir)/libGeonik_s_PrimiFun.so $(DESTDIR)$(plugindir)/Geonik_s_PrimiFun.so
