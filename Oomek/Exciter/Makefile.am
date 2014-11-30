plugin_LTLIBRARIES = libOomek_Exciter.la

libOomek_Exciter_la_SOURCES = Exciter.cpp
libOomek_Exciter_la_CXXFLAGS = $(BM_CXXFLAGS)
libOomek_Exciter_la_LDFLAGS = $(BM_LIBS)
libOomek_Exciter_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libOomek_Exciter.so $(DESTDIR)$(plugindir)/Oomek_Exciter.so

demosong_DATA = 909.bmx 909_excite.bmx

