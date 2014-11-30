plugin_LTLIBRARIES = libMadBrain_4fm2f.la

libMadBrain_4fm2f_la_SOURCES = 4fm2f.cpp
libMadBrain_4fm2f_la_CXXFLAGS = $(BM_CXXFLAGS)
libMadBrain_4fm2f_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libMadBrain_4fm2f.so $(DESTDIR)$(plugindir)/MadBrain_4fm2f.so
