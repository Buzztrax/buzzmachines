## $Id$

plugin_LTLIBRARIES = libEnsonic_CallbackTester.la

libEnsonic_CallbackTester_la_SOURCES = CallbackTester.cpp
libEnsonic_CallbackTester_la_CXXFLAGS = $(BM_CXXFLAGS)
libEnsonic_CallbackTester_la_LDFLAGS = $(BM_LIBS)

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libEnsonic_CallbackTester.so $(DESTDIR)$(plugindir)/Ensonic_CallbackTester.so
