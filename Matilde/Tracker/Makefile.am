SUBDIRS = SurfsDSPLib

SRC = BuzzInstrument.cpp BuzzInstrument.h \
 BuzzSample.cpp BuzzSample.h \
 Channel.cpp Channel.h \
 Envelope.cpp Envelope.h \
 IInstrument.cpp IInstrument.h \
 ISample.cpp ISample.h \
 Track.cpp Track.h \
 Tracker.cpp Tracker.h \
 WavetableManager.cpp WavetableManager.h

# unused
# 	ExtResampler.cpp ExtResampler.h

plugin_LTLIBRARIES = libMatilde_Tracker.la libMatilde_Tracker_Mono.la \
    libMatilde_Tracker2.la libMatilde_Tracker2_Mono.la
	
libMatilde_Tracker_la_SOURCES = $(SRC)
libMatilde_Tracker_la_CXXFLAGS = $(BM_CXXFLAGS) -DAPI_V1
libMatilde_Tracker_la_LDFLAGS = $(BM_LIBS)
libMatilde_Tracker_la_LIBADD = SurfsDSPLib/libdsplib.la $(top_builddir)/common/dsplib/libdsplib.la

libMatilde_Tracker_Mono_la_SOURCES = $(SRC)
libMatilde_Tracker_Mono_la_CXXFLAGS = $(BM_CXXFLAGS) -DMONO -DAPI_V1
libMatilde_Tracker_Mono_la_LDFLAGS = $(BM_LIBS)
libMatilde_Tracker_Mono_la_LIBADD = SurfsDSPLib/libdsplib.la $(top_builddir)/common/dsplib/libdsplib.la

libMatilde_Tracker2_la_SOURCES = $(SRC)
libMatilde_Tracker2_la_CXXFLAGS = $(BM_CXXFLAGS)
libMatilde_Tracker2_la_LDFLAGS = $(BM_LIBS)
libMatilde_Tracker2_la_LIBADD = SurfsDSPLib/libdsplib.la $(top_builddir)/common/dsplib/libdsplib.la

libMatilde_Tracker2_Mono_la_SOURCES = $(SRC)
libMatilde_Tracker2_Mono_la_CXXFLAGS = $(BM_CXXFLAGS) -DMONO
libMatilde_Tracker2_Mono_la_LDFLAGS = $(BM_LIBS)
libMatilde_Tracker2_Mono_la_LIBADD = SurfsDSPLib/libdsplib.la $(top_builddir)/common/dsplib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/libMatilde_Tracker*.la
	mv $(DESTDIR)$(plugindir)/libMatilde_Tracker.so $(DESTDIR)$(plugindir)/Matilde_Tracker.so
	mv $(DESTDIR)$(plugindir)/libMatilde_Tracker_Mono.so $(DESTDIR)$(plugindir)/Matilde_Tracker_Mono.so
	mv $(DESTDIR)$(plugindir)/libMatilde_Tracker2.so $(DESTDIR)$(plugindir)/Matilde_Tracker2.so
	mv $(DESTDIR)$(plugindir)/libMatilde_Tracker2_Mono.so $(DESTDIR)$(plugindir)/Matilde_Tracker2_Mono.so
	$(LN_S) -f Matilde_Tracker2.html  $(DESTDIR)$(userdocdir)/Matilde_Tracker.html
	$(LN_S) -f Matilde_Tracker2.html  $(DESTDIR)$(userdocdir)/Matilde_Tracker_Mono.html
	$(LN_S) -f Matilde_Tracker2.html  $(DESTDIR)$(userdocdir)/Matilde_Tracker2_Mono.html

userdoc_DATA = Matilde_Tracker2.html

