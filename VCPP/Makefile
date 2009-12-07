# Makefile for loaddap (using VC++ 8.x on win32)

# Source code, samples, docs and target build areas
OBJDIR			= .\objects
SRCDIR			= ..
DOCDIR			= .\doc
SAMPLEDIR		= .\sample

# This is the prepackaging area.  Contents should never be on the build path.
# Fill in the VERSION (e.g., "= 3.5.2") to generate a final, version stamped
# release of the product.
PRODUCT			= ml-structs
VERSION			=
PKGDIR			= ..\..\prepkg\opendap\$(PRODUCT)$(VERSION)
PKGLIBDIR		= $(PKGDIR)\lib
PKGDLLDIR		= $(PKGDIR)\dll
PKGBINDIR		= $(PKGDIR)\bin
PKGETCDIR		= $(PKGDIR)\etc
PKGINCDIR		= $(PKGDIR)\include
PKGSAMPLEDIR	= $(PKGDIR)\sample

# This is the installation area.  Select directories should be on the build
# in order to pick up the Core headers and libraries only.
INSTALLDIR		= ..\..
INSTALLLIBDIR	= $(INSTALLDIR)\lib
INSTALLDLLDIR	= $(INSTALLDIR)\dll
INSTALLBINDIR	= $(INSTALLDIR)\bin
INSTALLETCDIR	= $(INSTALLDIR)\etc
INSTALLINCDIR	= $(INSTALLDIR)\include

# This is where we get Core (libdap) related building materials.
LIBDIR			= $(INSTALLLIBDIR)  # To pickup Core libraries
INCLUDEDIR		= $(INSTALLINCDIR)  # To pickup Core headers

# Compiler, Linker
CC		= cl
LINK	= link

# Setup linker flags for executables
EMDLFLAGS		= /nologo /libpath:$(LIBDIR) /NODEFAULTLIB:LIBCMT.lib

# Setup link libraries for executables
EMDLLIBS		= libdapclientMD.lib libdapMD.lib libxml2.lib libcurl.lib zlib.lib libxdr.lib pthreadVC.lib ws2_32.lib wsock32.lib advapi32.lib

# Setup compiler flags
INCS			= /I . /I $(INCLUDEDIR) /I $(INCLUDEDIR)\GNU /I $(INCLUDEDIR)\gl /I ..
DEFS			= /D HAVE_CONFIG_H /D WIN32 /D WIN32_LEAN_AND_MEAN
MDCFLAGS		= /MD /D HAVE_PTHREAD_H
CFLAGS			= $(DEFS) $(INCS) /nologo /W0 /GR /EHsc /c

# Setup cmex flags
CMEX			= mex
CMEXFLAGS		= -I. -I.. -DHAVE_CONFIG_H -DWIN32 -DNO_MX_FREE -DMATLAB_R2009
CMEXLIBS		= libmx.lib libmat.lib
# MEXLLIBS		= libcurl.lib zlib.lib libdapMD.lib libxdr.lib user32.lib

default:
	@echo "usage: nmake all; nmake install; nmake package"

all:	modelMD

modelMD:
	@-md $(OBJDIR)\MD
	@echo "Building MD model Matlab cmd-line client"
        @nmake /NOLOGO OBJDIR="$(OBJDIR)\MD" loaddap.mexw32
	@nmake /NOLOGO CFLAGS="$(CFLAGS) $(MDCFLAGS)" LFLAGS="$(EMDLFLAGS)" LIBS="$(EMDLLIBS)" OBJDIR=$(OBJDIR)\MD writedap
	@echo "MD model Matlab cmd-line client is complete"

# Convenience dependancies
writedap			:	$(OBJDIR)\writedap.exe
loaddap.mexw32		:	$(OBJDIR)\loaddap.mexw32

# Clear built-in rules and set new ones
.SUFFIXES :
.SUFFIXES :	.obj .cc .c

{..}.c{$(OBJDIR)}.obj:
	$(CC) $(CFLAGS) /Fo$@ /Tc $<

.c{$(OBJDIR)}.obj:
	$(CC) $(CFLAGS) /Fo$@ /Tc $<

{..}.cc{$(OBJDIR)}.obj:
	$(CC) $(CFLAGS) /Fo$@ /Tp $<

MEXSRCS		= $(SRCDIR)\loaddap.c \
			$(SRCDIR)\variable.c \
			$(SRCDIR)\extend.c \
			$(SRCDIR)\error.c \
			$(SRCDIR)\MLVars.c \
			$(SRCDIR)\process_values.c \
			conpipe.c

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

# As long as loaddap.mexw32 exists, it appears nothing went wrong despite
# any message to the contrary.
$(OBJDIR)\loaddap.mexw32: $(MESXRCS)
	@-md $(OBJDIR)
	$(CMEX) $(CMEXFLAGS) -outdir $(OBJDIR) $(MEXSRCS) $(CMEXLIBS)

$(OBJDIR)\writedap.exe: $(WD_OBJS) $(CLIENTOBJS)
	$(LINK) $(LFLAGS) /OUT:$@ $(WD_OBJS) $(CLIENTOBJS) $(LIBS)

# I'm not sure why Rob M installed the mex file in a DDL directory, but it doesn't seem to
# hurt anything. jhrg
install: all mkinstdirs
    copy $(OBJDIR)\MD\loaddap.mex* $(INSTALLDLLDIR)
    copy $(SRCDIR)\loaddap.m $(INSTALLBINDIR)
    copy $(SRCDIR)\whodap.m $(INSTALLBINDIR)
	copy $(OBJDIR)\MD\writedap.exe* $(INSTALLBINDIR)
	mt -nologo -manifest $(INSTALLBINDIR)\writedap.exe.manifest -outputresource:$(INSTALLBINDIR)\writedap.exe;1
	del $(INSTALLBINDIR)\writedap.exe.manifest

# mt -nologo -manifest $(INSTALLDLLDIR)\loaddap.mexw32.manifest -outputresource:$(INSTALLDLLDIR)\loaddap.mexw32;2
# del $(INSTALLDLLDIR)\loaddap.mexw32.manifest

package: all mkpkgdirs document
    copy $(OBJDIR)\MD\loaddap.mex* $(PKGDLLDIR)
    copy $(SRCDIR)\loaddap.m $(PKGBINDIR)
    copy $(SRCDIR)\whodap.m $(PKGBINDIR)
	copy $(OBJDIR)\MD\writedap.exe* $(PKGBINDIR)
	mt -nologo -manifest $(PKGBINDIR)\writedap.exe.manifest -outputresource:$(PKGBINDIR)\writedap.exe;1
	del $(PKGBINDIR)\writedap.exe.manifest

#        mt -nologo -manifest $(PKGDLLDIR)\loaddap.mexw32.manifest -outputresource:$(PKGDLLDIR)\loaddap.mexw32;2
#        del $(PKGDLLDIR)\loaddap.mexw32.manifest

mkinstdirs:
	@-md $(INSTALLBINDIR)
	@-md $(INSTALLDLLDIR)
	
mkpkgdirs:
	@-md $(PKGBINDIR)
	@-md $(PKGDLLDIR)

document:
	@-md $(PKGSAMPLEDIR)
	copy $(SAMPLEDIR)\* $(PKGSAMPLEDIR)
	copy $(DOCDIR)\readme.txt $(PKGDIR)

clean:
	@-rd /Q /S $(OBJDIR) $(PKGDIR)

!INCLUDE "dependancies"






