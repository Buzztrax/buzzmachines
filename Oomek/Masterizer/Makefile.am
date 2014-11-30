plugin_LTLIBRARIES = libOomek_Masterizer.la

libOomek_Masterizer_la_SOURCES = Masterizer.cpp
libOomek_Masterizer_la_CXXFLAGS = $(BM_CXXFLAGS)
libOomek_Masterizer_la_LDFLAGS = $(BM_LIBS)
libOomek_Masterizer_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libOomek_Masterizer.so $(DESTDIR)$(plugindir)/Oomek_Masterizer.so

demosong_DATA =Masterizer_test.bmx

