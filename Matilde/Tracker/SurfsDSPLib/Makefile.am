## $Id$

noinst_LTLIBRARIES = libdsplib.la

libdsplib_la_SOURCES = \
	SRF_2PFilter.cpp SRF_2PFilter.h \
	SRF_Amp.cpp SRF_Amp.h \
	SRF_AutoGain.cpp \
	SRF_DSP.cpp SRF_DSP.h \
	SRF_Resampler.cpp SRF_Resampler.h \
	SRF_Types.h
libdsplib_la_CXXFLAGS = $(BM_CXXFLAGS) -DBUZZ
libdsplib_la_LDFLAGS = $(BM_LIBS)
