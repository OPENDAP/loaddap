
# Base Dods distribution Directory
DODSDIR		= ..\..\..\..

# Setup directories for intermediate files and built components
OBJDIR		= objects
INSTALLLIBDIR	= $(DODSDIR)\lib
INSTALLEXEDIR	= $(DODSDIR)\bin

# Directories of prebuilt libraries
LIBDIR		= $(DODSDIR)\lib

# Header Directories
INCDIR		= $(DODSDIR)\include

# Setup basic compiler flags for all .obj's
CC		= cl
CFLAGS		= /nologo /MT /W0 /GR /GX /c
#CFLAGS		= /MTd /Zi /W3 /GR /GX /c			# debug

# Linker flags
LINK		= link
LIBPATH		= /libpath:$(LIBDIR)
DODSLIBS	= libdap++.lib pthreadVCE.lib libcurl.lib librx.lib libz.lib libxdr.lib
SYSLIBS		= ws2_32.lib wsock32.lib advapi32.lib
LINKLIBS	= $(DODSLIBS) $(SYSLIBS)
LFLAGS		= /nologo /subsystem:console $(LINKLIBS) /machine:I386 $(LIBPATH)
#LFLAGS		= /DEBUG /subsystem:console $(LINKLIBS) /machine:I386 $(LIBPATH)	# debug


# Setup cmex flags
CMEX		= cmex
CMEXFLAGS	=
CMEXLIBS	= $(LIBDIR)\libcurl.lib $(LIBDIR)\librx.lib $(LIBDIR)\libz.lib
CMEXDODSLIBS	= $(CMEXLIBS) $(LIBDIR)\libdap++.lib $(LIBDIR)\libxdr.lib
CMEXLINKLIBS	= $(CMEXDODSLIBS) user32.lib

# Setup compiler includes and defines for libraries and .exe's
INCS		= /I . /I .. /I $(INCDIR) /I $(INCDIR)\w3c-libwww /I $(INCDIR)
DEFS		= /D WIN32 /D _CONSOLE /D DODS_VERSION=\"3.2.2\" /D VERSION=\"3.2.2\"
#DEFS		= /D DODS_DEBUG /D DODS_DEBUG2 /D WIN32 /D _CONSOLE /D DODS_VERSION=\"3.2.2\" /D VERSION=\"3.2.2\"    # debug
CPPFLAGS	= $(DEFS) $(INCS)

# Combine compiler flags
OBJFLAGS	= $(CFLAGS) $(CPPFLAGS)

# Define the targets
TARGETS		= $(OBJDIR)\loaddods.dll $(OBJDIR)\writeval.exe

# Clear built-in rules and set new ones
.SUFFIXES :
.SUFFIXES :	.obj .cc .c

{..}.c{$(OBJDIR)}.obj:
	@$(CC) $(OBJFLAGS) /Fo$@ /Tc $<

.c{$(OBJDIR)}.obj:
	@$(CC) $(OBJFLAGS) /Fo$@ /Tc $<

{..}.cc{$(OBJDIR)}.obj:
	@$(CC) $(OBJFLAGS) /Fo$@ /Tp $<

DLLOBJS		= $(OBJDIR)\loaddods.obj \
		$(OBJDIR)\variable.obj \
		$(OBJDIR)\extend.obj \
		$(OBJDIR)\error.obj \
		$(OBJDIR)\MLVars.obj \
		$(OBJDIR)\process_values.obj \
		$(OBJDIR)\conpipe.obj

WV_OBJS		= $(OBJDIR)\write_val.obj \
		$(OBJDIR)\name_map.obj \
		$(OBJDIR)\MetadataProcessing.obj \
		$(OBJDIR)\LoaddodsProcessing.obj

CLIENTOBJS	= $(OBJDIR)\ClientByte.obj \
		$(OBJDIR)\ClientInt32.obj \
		$(OBJDIR)\ClientFloat64.obj \
		$(OBJDIR)\ClientStr.obj \
		$(OBJDIR)\ClientUrl.obj \
		$(OBJDIR)\ClientArray.obj \
		$(OBJDIR)\ClientStructure.obj \
		$(OBJDIR)\ClientList.obj \
		$(OBJDIR)\ClientSequence.obj \
		$(OBJDIR)\ClientGrid.obj \
		$(OBJDIR)\ClientUInt32.obj \
		$(OBJDIR)\ClientInt16.obj \
		$(OBJDIR)\ClientUInt16.obj \
		$(OBJDIR)\ClientFloat32.obj

all: setup $(TARGETS)

# As long as loaddods.dll exists, it appears nothing went wrong despite the messages
# to the contrary.  I expect this problem doesn't show up under Win95/98.
$(OBJDIR)\loaddods.dll: $(DLLOBJS)
	@$(CMEX) $(CMEXFLAGS) -output loaddods -outdir $(OBJDIR) $(DLLOBJS) $(CMEXLINKLIBS)
	@echo "Ignore any cmex link error for loaddods.dll (Matlab bug)"

$(OBJDIR)\writeval.exe: $(WV_OBJS) $(CLIENTOBJS)
	@$(LINK) $(LFLAGS) /OUT:$(OBJDIR)\writeval.exe $(WV_OBJS) $(CLIENTOBJS) $(LINKLIBS)

install:
	copy $(OBJDIR)\loaddods.dll $(INSTALLEXEDIR)
	copy $(OBJDIR)\writeval.exe $(INSTALLEXEDIR)

# Don't use the built-in "extended DOS" commands with all the
# fancy flags.  Win95/98 systems don't have those so we must
# keep it simple.  Sticking with no-flag versions of copy, mkdir,
# rmdir and del are a safe bet.
setup:
	@-mkdir $(OBJDIR)
	@-mkdir $(INSTALLLIBDIR)
	@-mkdir $(INSTALLEXEDIR)

clean:
	@-del $(OBJDIR)\*.dll
	@-del $(OBJDIR)\*.exe
	@-del $(OBJDIR)\*.obj
	@-del $(OBJDIR)\*.map
	@-rmdir $(OBJDIR)

#include "dependancies"





