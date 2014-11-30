plugin_LTLIBRARIES = libDedaCode_StereoGain.la

libDedaCode_StereoGain_la_SOURCES = DedaCode_StereoGain_v2.cpp
libDedaCode_StereoGain_la_CXXFLAGS = $(BM_CXXFLAGS)
libDedaCode_StereoGain_la_LDFLAGS = $(BM_LIBS)
libDedaCode_StereoGain_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libDedaCode_StereoGain.so $(DESTDIR)$(plugindir)/DedaCode_StereoGain.so
