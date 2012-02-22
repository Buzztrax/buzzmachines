## $Id$

plugin_LTLIBRARIES = libFrequencyUnknown_O-Delay.la

libFrequencyUnknown_O_Delay_la_SOURCES = O-Delay.cpp
libFrequencyUnknown_O_Delay_la_CXXFLAGS = $(BM_CXXFLAGS)
libFrequencyUnknown_O_Delay_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFrequencyUnknown_O-Delay.so $(DESTDIR)$(plugindir)/FrequencyUnknown_O-Delay.so

userdoc_DATA = FrequencyUnknown_O-Delay.html
demosong_DATA = FrequencyUnknown_O-Delay_delaytest.bmx FrequencyUnknown_O-Delay_sparkle.bmx
