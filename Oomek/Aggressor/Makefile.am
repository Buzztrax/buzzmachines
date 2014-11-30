plugin_LTLIBRARIES = libOomek_Aggressor_303.la

libOomek_Aggressor_303_la_SOURCES = 303.cpp
libOomek_Aggressor_303_la_CXXFLAGS = $(BM_CXXFLAGS)
libOomek_Aggressor_303_la_LDFLAGS = $(BM_LIBS)
libOomek_Aggressor_303_la_LIBADD =  $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libOomek_Aggressor_303.so $(DESTDIR)$(plugindir)/Oomek_Aggressor_303.so

demosong_DATA = Oomek_Aggressor_ctrack.bmx Oomek_Aggressor_demo.bmx
