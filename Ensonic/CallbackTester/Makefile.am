plugin_LTLIBRARIES = libEnsonic_CallbackTesterE.la libEnsonic_CallbackTesterG.la

libEnsonic_CallbackTesterE_la_SOURCES = CallbackTesterE.cpp
libEnsonic_CallbackTesterE_la_CXXFLAGS = $(BM_CXXFLAGS)
libEnsonic_CallbackTesterE_la_LDFLAGS = $(BM_LIBS)
libEnsonic_CallbackTesterE_la_LIBADD = $(top_builddir)/common/dsplib/libdsplib.la

libEnsonic_CallbackTesterG_la_SOURCES = CallbackTesterG.cpp
libEnsonic_CallbackTesterG_la_CXXFLAGS = $(BM_CXXFLAGS)
libEnsonic_CallbackTesterG_la_LDFLAGS = $(BM_LIBS)
libEnsonic_CallbackTesterG_la_LIBADD = $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libEnsonic_CallbackTesterE.so $(DESTDIR)$(plugindir)/Ensonic_CallbackTesterE.so
	mv $(DESTDIR)$(plugindir)/libEnsonic_CallbackTesterG.so $(DESTDIR)$(plugindir)/Ensonic_CallbackTesterG.so
