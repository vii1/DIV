#############################################################################
#
#                   Copyright (C) 1996 SciTech Software
#                           All rights reserved.
#
# Descripton:   Generic makefile for the SuperVGA Kit. Requires the SciTech
#               Software makefile definitions package to be installed.
#
# $Date:   23 Apr 1996 02:09:30  $ $Author:   KendallB  $
#
#############################################################################

# DOS extender dependant flags

CFLAGS          += $(DX_CFLAGS)
ASFLAGS         += $(DX_ASFLAGS)

# Version number for this release

VER             = 60

# Names of all executable files built

.IF $(USE_WIN16)
EXEFILES        = wdtest16$E wdvga16$E
EXELIBS         = ztimer$L wdir$(VER)$L pmpro$(VER)$L
.IF $(USE_CTL3D)
CFLAGS          += -DUSE_CTL3D
EXELIB          += ctl3dv2$L
.ENDIF
.ELIF $(USE_WIN32)
EXEFILES        = wdtest32$E wdvga32$E
EXELIBS         = ztimer$L wdir$(VER)F$L pmpro$(VER)F$L
.IF $(USE_CTL3D)
CFLAGS          += -DUSE_CTL3D
EXELIB          += ctl3d32$L
.ENDIF
.ELSE
EXEFILES        = svtest$E profile$E
.IF $(STKCALL)
EXELIBS         = sztimer$L swdir$L
.ELSE
EXELIBS         = ztimer$L wdir$L
.ENDIF
.ENDIF

# Name of library and object files required to build it

.IF $(BUILD_DLL)
USE_DLL        = 1
.ENDIF
.IF $(IMPORT_DLL)
USE_DLL         = 1
.ENDIF

.IF $(USE_DLL)
.IF $(USE_WIN16)
DLLFILE         = svga$(VER)$D
LIBFILE         = svga$(VER)$L
.ELSE
DLLFILE         = svga$(VER)f$D
LIBFILE         = svga$(VER)f$L
.ENDIF
.ELSE
.IF $(STKCALL)
LIBFILE         = ssvga$L
.ELSE
LIBFILE         = svga$L
.ENDIF
.ENDIF

OBJECTS         = vesavbe$O svgasdk$O _svgasdk$O _linsdk$O              \
                  font8x16$O cpu$O vgapal$O vbeaf$O _vbeaf$O

# Link with UVBELib(tm) device support library if desired

.IF $(USE_UVBELIB)
CFLAGS          += -DUSE_UVBELIB
.IF $(STKCALL)
EXELIBS         += suvbelib$L
.ELSE
EXELIBS         += uvbelib$L
.ENDIF
.ENDIF

# Link with UVBELib/Accel device support library if desired

.IF $(USE_UVBELIB_ACCEL)
CFLAGS          += -DUSE_UVBELIB_ACCEL
.IF $(STKCALL)
EXELIBS         += swdpro$L
.ELSE
EXELIBS         += wdpro$L
.ENDIF
.ENDIF

all: $(EXEFILES)

svtest$E: svtest$O tests$O aftests$O getopt$O $(LIBFILE)
profile$E: profile$O _profile$O getopt$O $(LIBFILE)
hellovbe$E: hellovbe$O

.IF $(USE_WIN16)

wdtest16$E: wdtest$O tests$O wdtest16$R $(LIBFILE)
wdvga16$E: wdvga$O wdvga16$R

.ELIF $(USE_WIN32)

wdtest32$E: wdtest$O tests$O aftests$O wdtest32$R $(LIBFILE)
wdvga32$E: wdvga$O wdvga32$R

.ELSE

wdvga$E: wdvga$O $(LIBFILE)

.ENDIF


# The following is 4Dos specific!!

smallclean:
    @del *.sym *.bak *.tdk *.map *.dsk *.log
    @except (cpu.* font8x16.* svga.*) del *.obj *.o

# Define the list of object files to create dependency information for

DEPEND_OBJ      = $(OBJECTS) svtest$O getopt$O cpu$O profile$O 		\
                  _profile$O hellovbe$O wdtest$O wdvga$O vbeaf$O    \
                  _vbeaf$O tests$O aftests$O

.INCLUDE: "$(SCITECH)\makedefs\common.mk"

