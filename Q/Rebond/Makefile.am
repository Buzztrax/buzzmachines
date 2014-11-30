plugin_LTLIBRARIES = libQ_Rebond.la

libQ_Rebond_la_SOURCES = rebond.cpp
libQ_Rebond_la_CXXFLAGS = $(BM_CXXFLAGS)
libQ_Rebond_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libQ_Rebond.so $(DESTDIR)$(plugindir)/Q_Rebond.so
