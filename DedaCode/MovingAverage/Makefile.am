plugin_LTLIBRARIES = libDedaCode_MovingAverage.la

libDedaCode_MovingAverage_la_SOURCES = DedaCode_MovingAverage.cpp
libDedaCode_MovingAverage_la_CXXFLAGS = $(BM_CXXFLAGS)
libDedaCode_MovingAverage_la_LDFLAGS = $(BM_LIBS)
libDedaCode_MovingAverage_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libDedaCode_MovingAverage.so $(DESTDIR)$(plugindir)/DedaCode_MovingAverage.so
