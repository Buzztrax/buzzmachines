## $Id: Makefile.am 80 2009-05-05 15:13:01Z ensonic $

plugin_LTLIBRARIES = libCyanPhase_AtomStereoMeld.la

libCyanPhase_AtomStereoMeld_la_SOURCES = atom.cpp
libCyanPhase_AtomStereoMeld_la_CXXFLAGS = $(BM_CXXFLAGS)
libCyanPhase_AtomStereoMeld_la_LDFLAGS = $(BM_LIBS)
libCyanPhase_AtomStereoMeld_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libCyanPhase_AtomStereoMeld.so $(DESTDIR)$(plugindir)/CyanPhase_AtomStereoMeld.so

