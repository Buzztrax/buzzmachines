plugin_LTLIBRARIES = libJeskola_Delay.la

libJeskola_Delay_la_SOURCES = Delay.cpp
libJeskola_Delay_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_Delay_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_Delay.so $(DESTDIR)$(plugindir)/Jeskola_Delay.so
