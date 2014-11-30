plugin_LTLIBRARIES = libZwar_Csi.la

libZwar_Csi_la_SOURCES = Csi.cpp
libZwar_Csi_la_CXXFLAGS = $(BM_CXXFLAGS)
libZwar_Csi_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libZwar_Csi.so $(DESTDIR)$(plugindir)/Zwar_Csi.so
