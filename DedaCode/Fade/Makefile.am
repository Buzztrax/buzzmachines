plugin_LTLIBRARIES = libDedaCode_Fade.la

libDedaCode_Fade_la_SOURCES = DedaCode_Fade_v2.cpp
libDedaCode_Fade_la_CXXFLAGS = $(BM_CXXFLAGS)
libDedaCode_Fade_la_LDFLAGS = $(BM_LIBS)
libDedaCode_Fade_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libDedaCode_Fade.so $(DESTDIR)$(plugindir)/DedaCode_Fade.so
