plugin_LTLIBRARIES = libBigyo_FrequencyShifter.la

libBigyo_FrequencyShifter_la_SOURCES = Bigyo_FrequencyShifter.cpp \
  Allpass2.h ComplexFloat.h denormals.h FastCosSin.h HilbertPair.h LinLog.h
libBigyo_FrequencyShifter_la_CXXFLAGS = $(BM_CXXFLAGS)
libBigyo_FrequencyShifter_la_LDFLAGS = $(BM_LIBS)
libBigyo_FrequencyShifter_la_LIBADD = $(top_builddir)/common/mdk/libmdk.la

install-data-hook:
	$(RM) $(DESTDIR)$(plugindir)/$(plugin_LTLIBRARIES)
	mv $(DESTDIR)$(plugindir)/libBigyo_FrequencyShifter.so $(DESTDIR)$(plugindir)/Bigyo_FrequencyShifter.so
