#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.68)
AC_INIT([loaddap], [3.7.2], [support@opendap.org])
AC_CONFIG_AUX_DIR([conf])
AC_CONFIG_MACRO_DIR([conf])

AM_INIT_AUTOMAKE
AC_CONFIG_TESTDIR([testsuite], [.])

LT_INIT
AM_PROG_LIBTOOL

AC_CONFIG_SRCDIR([AttributeInterface.h])
AC_CONFIG_HEADER([config.h])

AX_REQUIRE_MATLAB
AX_MATLAB_VERSION

if test $MATLAB_MAJOR  -eq 7 -a $MATLAB_MINOR -lt 8
then
    MEXFLAGS="$MEXFLAGS -V5"
    AC_SUBST([MEXFLAGS])
else
    AC_DEFINE([MATLAB_R2009], [1], [Define this if using Matlab 7.8 or newer])
    AC_SUBST([MATLAB_R2009])
fi

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_YACC

AX_PATH_MEX

dnl Extra checks.
AX_MEXEXT

MEX="$MEX CC='\$(COMPILE)'"
AC_SUBST([MEX])

AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([putenv strcspn strerror strpbrk strrchr strstr])

dnl Checks for specific libraries
AC_CHECK_LIB([dl], [dlopen], [MEXLDADD="$MEXLDADD -ldl"])
AC_SUBST([MEXLDADD])

AC_CHECK_LIBDAP([3.8.0],
 [
  LIBS="$LIBS $DAP_CLIENT_LIBS"
  CPPFLAGS="$CPPFLAGS $DAP_CFLAGS"
 ],
 [ AC_MSG_ERROR([Cannot find libdap])
])

OPENDAP_DEBUG_OPTION

dnl autoheader macros; tack some text at the top and bottom of config.h.in
AH_TOP([#ifndef _config_h
#define _config_h])

AH_BOTTOM([/* Shorthand for gcc's unused attribute feature */
#if defined(__GNUG__) || defined(__GNUC__)
#define not_used __attribute__ ((unused))
#else
#define not_used 
#endif /* __GNUG__ || __GNUC__ */

#endif /* _config_h */])

AC_CONFIG_FILES([Makefile testsuite/Makefile testsuite/atlocal])

AC_OUTPUT
