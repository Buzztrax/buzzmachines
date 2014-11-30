plugin_LTLIBRARIES = libMakk_M4.la

libMakk_M4_la_SOURCES = M4.cpp waves/waves.cpp
libMakk_M4_la_CXXFLAGS = $(BM_CXXFLAGS)
libMakk_M4_la_LDFLAGS = $(BM_LIBS)
libMakk_M4_la_LIBADD = $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libMakk_M4.so $(DESTDIR)$(plugindir)/Makk_M4.so

preset_DATA = Makk_M4.prs
demosong_DATA = Makk_M4.bmw

