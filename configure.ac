dnl $Id$

dnl Initialise the autoconf/automake tool
AC_PREREQ(2.61)
AC_INIT([buzzmachines],[0.1.0],[buzztard-devel@lists.sourceforge.net])
AM_INIT_AUTOMAKE([-Wall -Werror check-news std-options])
AC_CONFIG_SRCDIR([ChangeLog])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR(m4)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

# We don't want *.a files
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

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

# Compilation and Linker flags
# -Wno-non-virtual-dtor suppresses: virtual methods with no virtual destructor
# -Wno-unknown-pragmas  suppresses: ignoring #pragma xxx
BM_CXXFLAGS="-I\$(top_srcdir)/common $DEBUG_CFLAGS -Wno-non-virtual-dtor -Wno-unknown-pragmas"
BM_LIBS="-module -avoid-version"
AC_SUBST(BM_CXXFLAGS)
AC_SUBST(BM_LIBS)

SHAVE_INIT([.])
AC_CONFIG_FILES(Makefile shave shave-libtool \
  common/Makefile \
    common/dsplib/Makefile \
    common/mdk/Makefile \
  Elak/Makefile \
    Elak/Dist2/Makefile \
    Elak/SVF/Makefile \
  Ensonic/Makefile \
    Ensonic/CallbackTester/Makefile \
  FSM/Makefile \
    FSM/ArpMan/Makefile \
    FSM/Chorus/Makefile \
    FSM/Chorus2/Makefile \
    FSM/Infector/Makefile \
    FSM/Kick/Makefile \
    FSM/PhatMan/Makefile \
    FSM/PanzerDelay/Makefile
    FSM/WahMan/Makefile
  Jeskola/Makefile \
    Jeskola/Ninja/Makefile \
    Jeskola/Trilok/Makefile \
  Matilde/Makefile \
    Matilde/Tracker/Makefile \
    Matilde/Tracker/SurfsDSPLib/Makefile \
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
