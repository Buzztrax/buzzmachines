## $Id$

plugin_LTLIBRARIES = libFSM_Infector.la

libFSM_Infector_la_SOURCES = Channel.cpp Filters.cpp Infector.cpp Track.cpp Vegetable.cpp
libFSM_Infector_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_Infector_la_LDFLAGS = $(BM_LIBS)

noinst_HEADERS = Infector.h

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_Infector.so $(DESTDIR)$(plugindir)/FSM_Infector.so
