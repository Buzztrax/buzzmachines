plugin_LTLIBRARIES = libCyanPhase_Notch.la

libCyanPhase_Notch_la_SOURCES = notch.cpp
libCyanPhase_Notch_la_CXXFLAGS = $(BM_CXXFLAGS)
libCyanPhase_Notch_la_LDFLAGS = $(BM_LIBS)
libCyanPhase_Notch_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libCyanPhase_Notch.so $(DESTDIR)$(plugindir)/CyanPhase_Notch.so
