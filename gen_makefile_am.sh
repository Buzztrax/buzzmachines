#!/bin/sh

machine=`basename $PWD`
author=`basename \`dirname $PWD\``

sources=`ls 2>/dev/null *.cpp || true`
headers=`ls 2>/dev/null *.hpp || true`

libadd=""

grep -i >/dev/null "dsplib.h" *.cpp
if [ "$?" == "0" ]; then
  libadd="$libadd \$(top_builddir)/common/dsplib/libdsplib.la"
fi 
grep -i >/dev/null "mdk.h" *.cpp
if [ "$?" == "0" ]; then
  libadd="$libadd \$(top_builddir)/common/mdk/libmdk.la"
fi 

cat >Makefile.am <<EOF
plugin_LTLIBRARIES = lib${author}_${machine}.la

lib${author}_${machine}_la_SOURCES = $sources
lib${author}_${machine}_la_CXXFLAGS = \$(BM_CXXFLAGS)
lib${author}_${machine}_la_LDFLAGS = \$(BM_LIBS)
EOF

if [ "$libadd" != "" ]; then
  cat >>Makefile.am <<EOF
lib${author}_${machine}_la_LIBADD = $libadd
EOF
fi

echo >>Makefile.am

if [ "$headers" != "" ]; then
  cat >>Makefile.am <<EOF
noinst_HEADERS = $headers

EOF
fi

cat >>Makefile.am <<EOF
install-data-hook:
	\$(RM) \$(DESTDIR)\$(plugindir)/\$(plugin_LTLIBRARIES)
	mv \$(DESTDIR)\$(plugindir)/lib${author}_${machine}.so \$(DESTDIR)\$(plugindir)/${author}_${machine}.so
EOF

