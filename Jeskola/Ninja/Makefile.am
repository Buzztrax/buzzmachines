plugin_LTLIBRARIES = libJeskola_NiNjA_dElaY.la

libJeskola_NiNjA_dElaY_la_SOURCES = Ninja.cpp
libJeskola_NiNjA_dElaY_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_NiNjA_dElaY_la_LDFLAGS = $(BM_LIBS)
libJeskola_NiNjA_dElaY_la_LIBADD = $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_NiNjA_dElaY.so $(DESTDIR)$(plugindir)/Jeskola_NiNjA_dElaY.so

preset_DATA = Jeskola_NiNjA_dElaY.prs
demosong_DATA = Jeskola_NiNjA_dElaY.bmx

