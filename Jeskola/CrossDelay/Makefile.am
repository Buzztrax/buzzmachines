## $Id$

plugin_LTLIBRARIES = libJeskola_Cross_Delay.la

libJeskola_Cross_Delay_la_SOURCES = CrossDelay.cpp
libJeskola_Cross_Delay_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_Cross_Delay_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_Cross_Delay.so $(DESTDIR)$(plugindir)/Jeskola_Cross_Delay.so
