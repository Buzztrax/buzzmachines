plugin_LTLIBRARIES = libGraue_SoftSat.la

libGraue_SoftSat_la_SOURCES = softsat.cpp
libGraue_SoftSat_la_CXXFLAGS = $(BM_CXXFLAGS)
libGraue_SoftSat_la_LDFLAGS = $(BM_LIBS)
libGraue_SoftSat_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libGraue_SoftSat.so $(DESTDIR)$(plugindir)/Graue_SoftSat.so
