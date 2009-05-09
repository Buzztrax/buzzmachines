## $Id$

plugin_LTLIBRARIES = libElak_SVF.la

libElak_SVF_la_SOURCES = svf.cpp
libElak_SVF_la_CXXFLAGS = $(BM_CXXFLAGS)
libElak_SVF_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libElak_SVF.so $(DESTDIR)$(plugindir)/Elak_SVF.so

demosong_DATA = Elak_SVF.bmx

