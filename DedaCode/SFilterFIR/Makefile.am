plugin_LTLIBRARIES = libDedaCode_SFilterFIR.la

libDedaCode_SFilterFIR_la_SOURCES = DedaCode_SFilterFIR.cpp
libDedaCode_SFilterFIR_la_CXXFLAGS = $(BM_CXXFLAGS)
libDedaCode_SFilterFIR_la_LDFLAGS = $(BM_LIBS)
libDedaCode_SFilterFIR_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libDedaCode_SFilterFIR.so $(DESTDIR)$(plugindir)/DedaCode_SFilterFIR.so
