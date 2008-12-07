## $Id$

# TODO we can build a mono version by -DMONO

SUBDIRS = SurfsDSPLib

SOURCES= \
	BuzzInstrument.cpp BuzzInstrument.h \
	BuzzSample.cpp BuzzSample.h \
	Channel.cpp Channel.h \
	Envelope.cpp Envelope.h \
	ExtResampler.cpp ExtResampler.h \
	IInstrument.cpp IInstrument.h \
	ISample.cpp ISample.h \
	Track.cpp Track.h \
	Tracker.cpp Tracker.h \
	WavetableManager.cpp WavetabelManager.h

plugin_LTLIBRARIES = libMatilde_Tracker.la libMatilde_Tracker_Mono.la
libMatilde_Tracker_la_SOURCES = $(SOURCES)
libMatilde_Tracker_la_CXXFLAGS = $(BM_CXXFLAGS)
libMatilde_Tracker_la_LDFLAGS = $(BM_LIBS)
libMatilde_Tracker_la_LIBADD = SurfsDSPLib/libdsplib.la

libMatilde_Tracker_Mono_la_SOURCES = $(SOURCES)
libMatilde_Tracker_Mono_la_CXXFLAGS = $(BM_CXXFLAGS) -DMONO
libMatilde_Tracker_Mono_la_LDFLAGS = $(BM_LIBS)
libMatilde_Tracker_Mono_la_LIBADD = SurfsDSPLib/libdsplib.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/libMatilde_Tracker*.la
	mv $(DESTDIR)$(plugindir)/libMatilde_Tracker.so $(DESTDIR)$(plugindir)/Matilde_Tracker.so
	mv $(DESTDIR)$(plugindir)/libMatilde_Tracker_Mono.so $(DESTDIR)$(plugindir)/Matilde_Tracker_Mono.so
