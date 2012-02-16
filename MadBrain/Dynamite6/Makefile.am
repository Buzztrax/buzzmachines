## $Id$

plugin_LTLIBRARIES = libMadBrain_Dynamite6.la

libMadBrain_Dynamite6_la_SOURCES = Dynamite6.cpp
libMadBrain_Dynamite6_la_CXXFLAGS = $(BM_CXXFLAGS)
libMadBrain_Dynamite6_la_LDFLAGS = $(BM_LIBS)
libMadBrain_Dynamite6_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libMadBrain_Dynamite6.so $(DESTDIR)$(plugindir)/MadBrain_Dynamite6.so
