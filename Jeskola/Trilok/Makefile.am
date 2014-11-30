plugin_LTLIBRARIES = libJeskola_Trilok.la

libJeskola_Trilok_la_SOURCES = Trilok.cpp
libJeskola_Trilok_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_Trilok_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_Trilok.so $(DESTDIR)$(plugindir)/Jeskola_Trilok.so
