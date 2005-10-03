
# Automake file for asciival
#
# 5/17/2005 jhrg
#
# $Id: Makefile.am 12148 2005-09-12 21:30:51Z jimg $

AUTOMAKE_OPTIONS = foreign dejagnu

AM_CPPFLAGS = -I$(top_srcdir)
ACLOCAL_AMFLAGS = -I conf

# Automake doesn't know about the mex script; we have to jury-rig some
# things. 9/29/05 jhrg
#
MEXFLAGS = $(AM_CPPFLAGS) -V5 -DNO_MX_FREE -DHAVE_CONFIG_H

# These are not used by automake but are often useful for certain types of
# debugging.
#
CXXFLAGS_DEBUG = -g3 -O0 -Wall -fno-defer-pop
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

bin_PROGRAMS = writedap
pkgdata_SCRIPTS = whodap.m
dist_pkgdata_DATA = loaddap.m

CLIENTSRCS = ClientByte.cc ClientInt32.cc ClientFloat64.cc ClientStr.cc	\
	ClientUrl.cc ClientArray.cc ClientStructure.cc			\
	ClientSequence.cc ClientGrid.cc ClientUInt32.cc			\
	ClientInt16.cc ClientUInt16.cc ClientFloat32.cc			\
	ClientTypeFactory.cc ClientTypeFactory.h AttributeInterface.h	\
	ClientStructure.h ClientArray.h ClientByte.h ClientUInt16.h	\
	ClientFloat32.h ClientUInt32.h ClientFloat64.h ClientUrl.h	\
	ClientGrid.h ClientInt16.h ClientInt32.h ClientSequence.h	\
	ClientStr.h

WV_SRCS = write_dap.cc name_map.cc MetadataProcessing.cc		\
	LoaddodsProcessing.cc LoaddodsProcessing.h			\
	MetadataProcessing.h config_writedap.h nan_hack.h name_map.h

LD_SRCS = loaddap.c variable.c queue.c extend.c error.c MLVars.c	\
	process_values.c 

LD_HDRS = MLVars.h defines.h dods_memory.h error.h extend.h queue.h	\
	variable.h 

writedap_SOURCES = $(CLIENTSRCS) $(WV_SRCS)

# It's possible, according to some web info, to build mex files without
# using the Mathworks 'mex' script, but I think that it's just too much 
# extra maintenance work. I'm leaving these here, but I build using mex.
# jhrg 9/29/05
#
# loaddap_SOURCES = $(LD_SRCS)
# loaddap_CFLAGS  = $(MATLAB_FLAGS) -DNO_MX_FREE -DV5_COMPAT
# loaddap_LDFLAGS = $(WRAPS) $(MATLAB_LINK)
# loaddap_LDADD   = $(MATLAB_LIB)
# loaddap_EXEEXT  = $(MEXEXT)

# A better way to handle building the mex file is to separate the mex file's
# source code and build so it's in its own directory. Then use the automake
# variables COMPILE and LINK to force the use of 'mex' when building. Then it
# should be possible to use loaddap_SOURCES, et c., and get the full power of
# automake. jhrg 9/29/05

all-local: loaddap-stamp

loaddap-stamp:
	mex $(CPPFLAGS) $(MEXFLAGS) -output $@ $(LD_SRCS) \
		&& touch loaddap-stamp

clean-local:
	-rm -f loaddap.mex*
	-rm loaddap-stamp

install-exec-local:
	$(INSTALL_PROGRAM) loaddap.mex* $(DESTDIR)$(bindir)

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/loaddap.mex*       

DEJATOOL = writedap
RUNTESTDEFAULTFLAGS = --tool $$tool --srcdir testsuite

EXTRA_DIST = win32 testsuite $(LD_SRCS) $(LD_HDRS) loaddap.m whodap.m \
	README INSTALL NEWS

CLEANFILES = *~ *.log *.sum

###########################################################################

# Build linux RPMs

srpm:
	rpmbuild -ts @PACKAGE@-@PACKAGE_VERSION@.tar.gz

rpm:
	rpmbuild -tb @PACKAGE@-@PACKAGE_VERSION@.tar.gz

# LocalWords:  mex Mathworks automake loaddap jhrg
