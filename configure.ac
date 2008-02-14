#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([loaddap], [3.5.2], [opendap-tech@opendap.org])
AC_CONFIG_AUX_DIR([conf])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([AttributeInterface.h])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_RANLIB
AC_PROG_YACC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcppunit':
# AC_CHECK_LIB([cppunit], [main])
# FIXME: Replace `main' with a function in `-lmx':
# AC_CHECK_LIB([mx], [main])

# Checks for header files.
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

AC_CHECK_LIBDAP([3.7.10],
 [
  LIBS="$LIBS $DAP_CLIENT_LIBS"
  CPPFLAGS="$CPPFLAGS $DAP_CFLAGS"
 ],
 [ AC_MSG_ERROR([Cannot find libdap])
])

OPENDAP_DEBUG_OPTION

dnl MEX_INIT([MEX],[1],[1],[2])

dnl echo "Build OS: $build_os"

dnl linux="no"
dnl winnt="no"
dnl macos="no"
dnl case $build_os in
dnl *linux*)
dnl     linux="yes";;
dnl dnl    EXEEXT=.mexglx;;
dnl *mingw*)
dnl     winnt="yes";;
dnl dnl    EXEEXT=.dll;;
dnl *cygwin*)
dnl     winnt="yes";;
dnl dnl    EXEEXT=.dll;;
dnl *darwin*)
dnl     macos="yes";
dnl dnl    EXEEXT=.mexmac;;
dnl esac

dnl AM_CONDITIONAL(LINUX, test "$linux" = "yes")
dnl AM_CONDITIONAL(WINNT, test "$winnt" = "yes")
dnl AM_CONDITIONAL(MACOS, test "$macos" = "yes")

dnl dnl
dnl dnl Matlab Library
dnl dnl

dnl CHECK_MATLAB

dnl if test "$have_matlab" = "no"
dnl then
dnl     AC_MSG_ERROR([Must specify matlab directory using --with-matlab])
dnl fi

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

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
