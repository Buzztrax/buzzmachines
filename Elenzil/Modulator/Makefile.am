## $Id$

plugin_LTLIBRARIES = libElenzil_Modulator.la

libElenzil_Modulator_la_SOURCES = amplitude_modulator.cpp
libElenzil_Modulator_la_CXXFLAGS = $(BM_CXXFLAGS)
libElenzil_Modulator_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libElenzil_Modulator.so $(DESTDIR)$(plugindir)/Elenzil_Modulator.so
