# Makefile for loaddap (under VC++ on win32)

# Setup directories for intermediate files and built components
OBJDIR		= .\objects
SRCDIR		= ..
INSTALLLIBDIR	= ..\..\prepkg\lib
INSTALLDLLDIR	= ..\..\prepkg\dll
INSTALLBINDIR	= ..\..\prepkg\bin
INSTALLEXEDIR	= ..\..\prepkg\bin
INSTALLETCDIR	= ..\..\prepkg\etc
INSTALLINCDIR	= ..\..\prepkg\include

# Setup basic compiler flags for all .obj's
CC		= cl
CFLAGS		= /nologo /MT /W0 /GR /EHsc /c
#CFLAGS		= /MTd /W3 /GR /EHsc /c		# debug

# Linker flags
LINK		= link
LIBPATH		= /libpath:$(INSTALLLIBDIR)
DODSLIBS	= libdapMT.lib pthreadVC.lib libcurl.lib zlib.lib libxdr.lib
SYSLIBS		= ws2_32.lib wsock32.lib advapi32.lib
LINKLIBS	= $(DODSLIBS) $(SYSLIBS)
LFLAGS		= /nologo /subsystem:console $(LINKLIBS) /machine:I386 $(LIBPATH)
#LFLAGS		= /DEBUG /subsystem:console $(LINKLIBS) /machine:I386 $(LIBPATH) # debug

# Setup cmex flags
CMEX		= mex
CMEXFLAGS	=
CMEXLIBS	= libcurl.lib zlib.lib
CMEXDODSLIBS	= $(CMEXLIBS) $(INSTALLLIBDIR)\libdapMT.lib libxdr.lib
CMEXLINKLIBS	= $(CMEXDODSLIBS) user32.lib

# Setup compiler includes and defines for libraries and .exe's
INCS		= /I . /I $(INSTALLINCDIR) /I $(INSTALLINCDIR)\GNU /I $(INSTALLINCDIR)\gl /I ..
DEFS		= /D HAVE_CONFIG_H /D WIN32 /D WIN32_LEAN_AND_MEAN /D _CONSOLE /D V5_COMPAT
CPPFLAGS	= $(DEFS) $(INCS)

# Combine compiler flags
OBJFLAGS	= $(CFLAGS) $(CPPFLAGS)

# Define the targets
TARGETS		= $(OBJDIR)\loaddap.dll $(OBJDIR)\writedap.exe

# Clear built-in rules and set new ones
.SUFFIXES :
.SUFFIXES :	.obj .cc .c

{..}.c{$(OBJDIR)}.obj:
	$(CC) $(OBJFLAGS) /Fo$@ /Tc $<

.c{$(OBJDIR)}.obj:
	$(CC) $(OBJFLAGS) /Fo$@ /Tc $<

{..}.cc{$(OBJDIR)}.obj:
	$(CC) $(OBJFLAGS) /Fo$@ /Tp $<

DLLOBJS		= $(OBJDIR)\loaddap.obj \
		$(OBJDIR)\variable.obj \
		$(OBJDIR)\extend.obj \
		$(OBJDIR)\error.obj \
		$(OBJDIR)\MLVars.obj \
		$(OBJDIR)\process_values.obj \
		$(OBJDIR)\conpipe.obj

WD_OBJS		= $(OBJDIR)\write_dap.obj \
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
		$(OBJDIR)\ClientSequence.obj \
		$(OBJDIR)\ClientTypeFactory.obj \
		$(OBJDIR)\ClientGrid.obj \
		$(OBJDIR)\ClientUInt32.obj \
		$(OBJDIR)\ClientInt16.obj \
		$(OBJDIR)\ClientUInt16.obj \
		$(OBJDIR)\ClientFloat32.obj

all: setup $(TARGETS)

# As long as loaddap.dll exists, it appears nothing went wrong despite
# any message to the contrary.
$(OBJDIR)\loaddap.dll: $(DLLOBJS)
	@-$(CMEX) $(CMEXFLAGS) -output loaddap.dll -outdir $(OBJDIR) $(DLLOBJS) $(CMEXLINKLIBS)
	@echo "Ignore any cmex link error for loaddap.dll (Matlab bug)"

$(OBJDIR)\writedap.exe: $(WD_OBJS) $(CLIENTOBJS)
	$(LINK) $(LFLAGS) /OUT:$(OBJDIR)\writedap.exe $(WD_OBJS) $(CLIENTOBJS) $(LINKLIBS)

install:
	copy $(OBJDIR)\loaddap.dll $(INSTALLEXEDIR)
	copy $(OBJDIR)\writedap.exe $(INSTALLEXEDIR)

setup:
	@-md $(OBJDIR)
	@-md $(INSTALLLIBDIR)
	@-md $(INSTALLEXEDIR)

clean:
	@-del $(OBJDIR)\*.dll
	@-del $(OBJDIR)\*.exe
	@-del $(OBJDIR)\*.obj
	@-del $(OBJDIR)\*.map
	@-rd $(OBJDIR)

#include "dependancies"





