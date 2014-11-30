plugin_LTLIBRARIES = libQ_Zfilter.la

libQ_Zfilter_la_SOURCES = zfilter.cpp
libQ_Zfilter_la_CXXFLAGS = $(BM_CXXFLAGS)
libQ_Zfilter_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libQ_Zfilter.so $(DESTDIR)$(plugindir)/Q_Zfilter.so
