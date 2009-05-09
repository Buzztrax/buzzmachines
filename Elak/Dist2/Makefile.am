## $Id$

plugin_LTLIBRARIES = libElak_Dist2.la

libElak_Dist2_la_SOURCES = dist2.cpp
libElak_Dist2_la_CXXFLAGS = $(BM_CXXFLAGS)
libElak_Dist2_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libElak_Dist2.so $(DESTDIR)$(plugindir)/Elak_Dist2.so

demosong_DATA = Elak_Dist2.bmx

