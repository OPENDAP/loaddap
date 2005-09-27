
# Automake file for asciival
#
# 5/17/2005 jhrg
#
# $Id: Makefile.am 12148 2005-09-12 21:30:51Z jimg $

AUTOMAKE_OPTIONS = foreign dejagnu

AM_CPPFLAGS = -I$(top_srcdir)
ACLOCAL_AMFLAGS = -I conf

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
	LoaddodsProcessing.cc LoaddodsProcessing.h MLVars.h		\
	MetadataProcessing.h config_writedap.h nan_hack.h name_map.h

LD_SRCS = loaddap.c variable.c queue.c extend.c error.c MLVars.c \
	process_values.c

# AttributeInterface.h    ClientStructure.h       defines.h
# ClientArray.h           ClientTypefactory.h     dods_memory.h
# ClientByte.h            ClientUInt16.h          error.h
# ClientFloat32.h         ClientUInt32.h          extend.h
# ClientFloat64.h         ClientUrl.h             name_map.h
# ClientGrid.h            LoaddodsProcessing.h    nan_hack.h
# ClientInt16.h           MLVars.h                queue.h
# ClientInt32.h           MetadataProcessing.h    variable.h
# ClientSequence.h        config.h                version.h
# ClientStr.h             config_writedap.h

writedap_SOURCES = $(CLIENTSRCS) $(WV_SRCS)

DEJATOOL = writedap
RUNTESTDEFAULTFLAGS = --tool $$tool --srcdir testsuite

EXTRA_DIST = 

CLEANFILES = *~ *.log *.sum
