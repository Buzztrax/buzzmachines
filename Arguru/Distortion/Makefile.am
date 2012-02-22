## $Id$

plugin_LTLIBRARIES = libArguru_Distortion.la

libArguru_Distortion_la_SOURCES = Distortion.cpp
libArguru_Distortion_la_CXXFLAGS = $(BM_CXXFLAGS)
libArguru_Distortion_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libArguru_Distortion.so $(DESTDIR)$(plugindir)/Arguru_Distortion.so
