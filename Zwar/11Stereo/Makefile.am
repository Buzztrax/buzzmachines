plugin_LTLIBRARIES = libZwar_11stereo.la

libZwar_11stereo_la_SOURCES = 11Stereo.cpp
libZwar_11stereo_la_CXXFLAGS = $(BM_CXXFLAGS)
libZwar_11stereo_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libZwar_11stereo.so $(DESTDIR)$(plugindir)/Zwar_11stereo.so

demosong_DATA = 11Stereo.bmx
