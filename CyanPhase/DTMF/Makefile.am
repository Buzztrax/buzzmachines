## $Id$

plugin_LTLIBRARIES = libCyanPhase_DTMF.la

libCyanPhase_DTMF_la_SOURCES = DTMF-1.cpp
libCyanPhase_DTMF_la_CXXFLAGS = $(BM_CXXFLAGS)
libCyanPhase_DTMF_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libCyanPhase_DTMF.so $(DESTDIR)$(plugindir)/CyanPhase_DTMF.so

demosong_DATA = CyanPhase_DTMF-1_Demo.bmx

