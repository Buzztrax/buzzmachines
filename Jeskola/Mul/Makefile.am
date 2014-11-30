plugin_LTLIBRARIES = libJeskola_Multiplier.la

libJeskola_Multiplier_la_SOURCES = Mul.cpp
libJeskola_Multiplier_la_CXXFLAGS = $(BM_CXXFLAGS)
libJeskola_Multiplier_la_LDFLAGS = $(BM_LIBS)
# FIXME: need custom auxbus library that can trigger AB_ShowEditor in the UI

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libJeskola_Multiplier.so $(DESTDIR)$(plugindir)/Jeskola_Multiplier.so
