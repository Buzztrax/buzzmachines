## $I$

plugin_LTLIBRARIES = libElak_Dist.la

libElak_Dist_la_SOURCES = dist2.cpp
libElak_Dist_la_CXXFLAGS = $(BM_CXXFLAGS)
libElak_Dist_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libElak_Dist.so $(DESTDIR)$(plugindir)/Elak_Dist.so

