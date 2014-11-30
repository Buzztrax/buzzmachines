plugin_LTLIBRARIES = libCyanPhase_DTMF_1.la

libCyanPhase_DTMF_1_la_SOURCES = DTMF-1.cpp
libCyanPhase_DTMF_1_la_CXXFLAGS = $(BM_CXXFLAGS)
libCyanPhase_DTMF_1_la_LDFLAGS = $(BM_LIBS)
libCyanPhase_DTMF_1_la_LIBADD = $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libCyanPhase_DTMF_1.so $(DESTDIR)$(plugindir)/CyanPhase_DTMF_1.so

demosong_DATA = CyanPhase_DTMF-1_Demo.bmx
userdoc_DATA = CyanPhase_DTMF_1.txt
