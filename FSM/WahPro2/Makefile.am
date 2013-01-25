## $Id: Makefile.am 76 2009-05-02 21:22:04Z ensonic $

plugin_LTLIBRARIES = libFSM_WahManPro2.la

libFSM_WahManPro2_la_SOURCES = WahManPro2.cpp
libFSM_WahManPro2_la_CXXFLAGS = $(BM_CXXFLAGS)
libFSM_WahManPro2_la_LDFLAGS = $(BM_LIBS)
libFSM_WahManPro2_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libFSM_WahManPro2.so $(DESTDIR)$(plugindir)/FSM_WahManPro2.so

preset_DATA = FSM_WahManPro2.prs
