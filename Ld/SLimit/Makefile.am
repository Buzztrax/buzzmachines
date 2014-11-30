plugin_LTLIBRARIES = libLd_SLimit.la

libLd_SLimit_la_SOURCES = slimit.cpp
libLd_SLimit_la_CXXFLAGS = $(BM_CXXFLAGS)
libLd_SLimit_la_LDFLAGS = $(BM_LIBS)
libLd_SLimit_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libLd_SLimit.so $(DESTDIR)$(plugindir)/Ld_SLimit.so

