plugin_LTLIBRARIES = libJeskola_Freeverb.la

libJeskola_Freeverb_la_SOURCES = allpass.cpp comb.cpp main.cpp revmodel.cpp
libJeskola_Freeverb_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_Freeverb_la_LDFLAGS = $(BM_LIBS)
libJeskola_Freeverb_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

noinst_HEADERS = allpass.h comb.h denormals.h main.h revmodel.h tuning.h

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_Freeverb.so $(DESTDIR)$(plugindir)/Jeskola_Freeverb.so
