plugin_LTLIBRARIES = libElenzil_FrequencyBomb.la

libElenzil_FrequencyBomb_la_SOURCES = freqbomb.cpp
libElenzil_FrequencyBomb_la_CXXFLAGS = $(BM_CXXFLAGS)
libElenzil_FrequencyBomb_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libElenzil_FrequencyBomb.so $(DESTDIR)$(plugindir)/Elenzil_FrequencyBomb.so
