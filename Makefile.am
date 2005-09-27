
# Automake file for asciival
#
# 5/17/2005 jhrg
#
# $Id: Makefile.am 12148 2005-09-12 21:30:51Z jimg $

AUTOMAKE_OPTIONS = foreign dejagnu

MATLAB_ROOT = /Applications/MATLAB71
# @MATLAB_ROOT@

# This is a little sloppy, but until I figure out how to set up
# automake to build loaddap (and thus can use loaddap_CFLAGS), I'm
# just going to dump the include here and override this for writedap
# (so it's not looking in the matlab includes...). jhrg 9/26/05
AM_CPPFLAGS = -I$(top_srcdir) -I$(MATLAB_ROOT)/extern/include
ACLOCAL_AMFLAGS = -I conf
CMEX = $(MATLAB_ROOT)/bin/mex

# These are not used by automake but are often useful for certain types of
# debugging.
CXXFLAGS_DEBUG = -g3 -O0 -Wall -fno-defer-pop
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

DIST_SUBDIRS = testsuite win32

bin_PROGRAMS = writedap

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
	process_values.c MLVars.h defines.h dods_memory.h error.h	\
	extend.h queue.h variable.h 

LD_OBJS = loaddap.o variable.o queue.o extend.o error.o MLVars.o	\
	process_values.o

writedap_SOURCES = $(CLIENTSRCS) $(WV_SRCS)
writedap_CFLAGS = -I$(top_srcdir)

loaddap: 
	$(CMEX) $(LD_SRCS)

DEJATOOL = writedap
RUNTESTDEFAULTFLAGS = --tool $$tool --srcdir testsuite

EXTRA_DIST = 

CLEANFILES = *~ *.log *.sum
