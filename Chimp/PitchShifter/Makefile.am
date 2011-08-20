## $Id$

plugin_LTLIBRARIES = libChimp_PitchShifter.la

libChimp_PitchShifter_la_SOURCES = PitchShifter.cpp
libChimp_PitchShifter_la_CXXFLAGS = $(BM_CXXFLAGS)
libChimp_PitchShifter_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libChimp_PitchShifter.so $(DESTDIR)$(plugindir)/Chimp_PitchShifter.so

