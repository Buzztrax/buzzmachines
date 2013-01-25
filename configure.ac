dnl $Id$

dnl Initialise the autoconf/automake tool
AC_PREREQ(2.61)
AC_INIT([buzzmachines],[20100517],[buzztard-devel@lists.sourceforge.net])
AM_INIT_AUTOMAKE([-Wall -Werror check-news std-options])
AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR(m4)
LT_PREREQ([2.2])
# We don't want *.a files
LT_INIT([disable-static])

# Support silent build rules, requires at least automake-1.11. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lmsvcrt':
AC_CHECK_LIB([msvcrt], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h malloc.h stddef.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset pow sqrt strchr])

dnl enable runtime debugging code (this is more like a developer build mode)
AC_MSG_CHECKING(whether to enable runtime debugging code)
AC_ARG_ENABLE(
	debug,
	AS_HELP_STRING([--enable-debug],[enable runtime debugging code (default=no)]),
	,
	[enable_debug="no"])
AC_MSG_RESULT($enable_debug)
if test "$enable_debug" = "yes"; then
	AC_DEFINE(USE_DEBUG, [1], [enable runtime debugging code])
	DEBUG_CFLAGS="-O0 -Wall -Werror -g"
else
	DEBUG_CFLAGS=""
fi

plugindir="\$(libdir)/Gear"
AC_SUBST(plugindir)
presetdir="\$(datadir)/Gear"
AC_SUBST(presetdir)
userdocdir="\$(datadir)/Gear"
AC_SUBST(userdocdir)
demosongdir="\$(datadir)/buzztard/songs/Gear"
AC_SUBST(demosongdir)

# Compilation and Linker flags
# -Wno-non-virtual-dtor suppresses: virtual methods with no virtual destructor
# -Wno-unknown-pragmas  suppresses: ignoring #pragma xxx
BM_CXXFLAGS="-I\$(top_srcdir)/common $DEBUG_CFLAGS -Wno-non-virtual-dtor -Wno-unknown-pragmas"
BM_LIBS="-module -avoid-version"
AC_SUBST(BM_CXXFLAGS)
AC_SUBST(BM_LIBS)

AC_CONFIG_FILES(Makefile \
  common/Makefile \
    common/dsplib/Makefile \
    common/mdk/Makefile \
    Arguru/Distortion/Makefile \
  AseDev/Makefile \
    AseDev/a4pFilter01/Makefile \
  Chimp/Makefile \
    Chimp/PitchShifter/Makefile \
  CyanPhase/Makefile \
    CyanPhase/AtomStereoMeld/Makefile \
    CyanPhase/DTMF/Makefile \
  Elak/Makefile \
    Elak/Dist2/Makefile \
    Elak/SVF/Makefile \
  Elenzil/Makefile \
    Elenzil/FrequencyBomb/Makefile \
    Elenzil/Modulator/Makefile \
  Ensonic/Makefile \
    Ensonic/CallbackTester/Makefile \
  FrequencyUnknown/Makefile \
    FrequencyUnknown/O-Delay/Makefile \
  FSM/Makefile \
    FSM/ArpMan/Makefile \
    FSM/Chorus/Makefile \
    FSM/Chorus2/Makefile \
    FSM/Infector/Makefile \
    FSM/Kick/Makefile \
    FSM/PanzerDelay/Makefile \
    FSM/PhatMan/Makefile \
    FSM/Philta/Makefile \
    FSM/ScatMan/Makefile \
    FSM/ScrapMan/Makefile \
    FSM/SprayMan/Makefile \
    FSM/TapMan/Makefile \
    FSM/TunaMan/Makefile \
    FSM/WahMan/Makefile \
    FSM/WahManPro/Makefile \
    FSM/WahPro2/Makefile \
  Geonik/Makefile \
    Geonik/BuzzMachines/Makefile \
  Jeskola/Makefile \
    Jeskola/CrossDelay/Makefile \
    Jeskola/Delay/Makefile \
    Jeskola/Distortion/Makefile \
    Jeskola/Freeverb/Makefile \
    Jeskola/Mul/Makefile \
    Jeskola/Ninja/Makefile \
    Jeskola/Noise/Makefile \
    Jeskola/Trilok/Makefile \
  Ld/Makefile \
    Ld/Gate/Makefile \
    Ld/SLimit/Makefile \
  MadBrain/Makefile \
    MadBrain/Dynamite6/Makefile \
    MadBrain/4fm2f/Makefile \
  Makk/Makefile \
    Makk/M3/Makefile \
    Makk/M4/Makefile \
  Matilde/Makefile \
    Matilde/Tracker/Makefile \
    Matilde/Tracker/SurfsDSPLib/Makefile \
  Oomek/Makefile \
    Oomek/Aggressor/Makefile \
  Q/Makefile \
    Q/Rebond/Makefile \
    Q/Zfilter/Makefile \
  WhiteNoise/Makefile \
    WhiteNoise/StereoDist/Makefile \
  Zwar/Makefile \
    Zwar/11Stereo/Makefile \
    Zwar/Csi/Makefile \
)

AC_OUTPUT

echo "
Configuration
	Version                    : ${VERSION}
	Source code location       : ${srcdir}
	Prefix                     : ${prefix}
	Compiler                   : ${CC}

	Debug                      : ${enable_debug}
"
