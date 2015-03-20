plugin_LTLIBRARIES = libFSM_KickXP.la

libFSM_KickXP_la_SOURCES = KickXP.cpp
libFSM_KickXP_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_KickXP_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_KickXP.so $(DESTDIR)$(plugindir)/FSM_KickXP.so

