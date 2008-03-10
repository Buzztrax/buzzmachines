dnl $Id: configure.ac,v 1.3 2007-11-13 14:48:46 ensonic Exp $

dnl Initialise the autoconf/automake tool
AC_PREREQ(2.59)
AC_INIT([buzzmachines], [0.1.0], [buzztard-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([ChangeLog])
AC_CONFIG_HEADER([config.h])

dnl Initialise the automake tool
AM_INIT_AUTOMAKE([check-news])

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
AC_PATH_X
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h malloc.h stddef.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset pow sqrt strchr])

plugindir="\$(libdir)/Gear"
AC_SUBST(plugindir)

# Compilation and Linker flags
# the -Wno-non-virtual-dtor is needed to suppress : virtual methods with no virtual destructor
BM_CXXFLAGS="-I\$(top_srcdir)/common -Wno-non-virtual-dtor"
BM_LIBS="-module -avoid-version"
AC_SUBST(BM_CXXFLAGS)
AC_SUBST(BM_LIBS)

AC_CONFIG_FILES(\
  Makefile \
  common/Makefile \
  common/mdk/Makefile \
  Elak/Makefile \
  Elak/Dist2/Makefile \
  Elak/SVF/Makefile \
  FSM/Makefile \
  FSM/ArpMan/Makefile \
  FSM/Chorus/Makefile \
  FSM/Infector/Makefile \
  FSM/Kick/Makefile \
  FSM/PhatMan/Makefile \
  FSM/PanzerDelay/Makefile
)

AC_OUTPUT
