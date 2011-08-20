## $Id: Makefile.am 81 2009-05-05 19:15:03Z ensonic $

plugin_LTLIBRARIES = libJeskola_Distortion.la

libJeskola_Distortion_la_SOURCES = Distortion.cpp
libJeskola_Distortion_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_Distortion_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_Distortion.so $(DESTDIR)$(plugindir)/Jeskola_Distortion.so
