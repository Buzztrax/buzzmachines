plugin_LTLIBRARIES = libMakk_M3.la

libMakk_M3_la_SOURCES = M3.cpp
libMakk_M3_la_CXXFLAGS = $(BM_CXXFLAGS)
libMakk_M3_la_LDFLAGS = $(BM_LIBS)
libMakk_M3_la_LIBADD = $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libMakk_M3.so $(DESTDIR)$(plugindir)/Makk_M3.so

preset_DATA = Makk_M3.prs

