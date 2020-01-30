#############################################################################
#
#               Copyright (C) 1991-1997 SciTech Software, Inc.
#                            All rights reserved.
#
# Descripton:   Specialised definitions for building the MGLWIN target
#               version of the MGL.
#
# $Date:   07 Nov 1997 16:08:26  $ $Author:   KendallB  $
#
#############################################################################

# Define the name of the library file

.IF $(BUILD_DLL)
MGL_DLL         = 1
.ELIF $(IMPORT_DLL)
MGL_DLL         = 1
.ENDIF

.IF $(MGL_DLL)
.IF $(WC_DLL)

# Build Watcom C++ specific DLL (register calling conventions).

.IF $(MGL_LITE)                 # Build the Watcom C++ specific DLL
CFLAGS          += -DMGL_LITE
ASFLAGS         += -DMGL_LITE
LIBFILE         = mglltwi$L
DLLFILE         = mglltw$D
.ELIF $(MGL_PRO)
CFLAGS          += -DMGL_PRO
ASFLAGS         += -DMGL_PRO
LIBFILE         = mglprwi$L
DLLFILE         = mglprw$D
.ELIF $(MGL_FLT3D)
CFLAGS          += -DMGL_FLT3D
ASFLAGS         += -DMGL_FLT3D
LIBFILE         = mglflwi$L     # Default for MGL 2.0
DLLFILE         = mglflw$D
.ELSE
LIBFILE         = mglfxwi$L     # Default for MGL 2.0
DLLFILE         = mglfxw$D
.ENDIF
.ELSE

# Build Standard Windows DLL (C calling conventions)

CFLAGS          += -DMGL_DLL    # Build the standard DLL
.IF $(MGL_LITE)
CFLAGS          += -DMGL_LITE
ASFLAGS         += -DMGL_LITE
LIBFILE         = mgllti$L
DLLFILE         = mgllt$D
.ELIF $(MGL_PRO)
CFLAGS          += -DMGL_PRO
ASFLAGS         += -DMGL_PRO
LIBFILE         = mglpri$L
DLLFILE         = mglpr$D
.ELIF $(MGL_FLT3D)
CFLAGS          += -DMGL_FLT3D
ASFLAGS         += -DMGL_FLT3D
LIBFILE         = mglfli$L
DLLFILE         = mglfl$D
.ELSE
LIBFILE         = mglfxi$L      # Default for MGL 2.0
DLLFILE         = mglfx$D
.ENDIF
.ENDIF

.ELSE

# Build standard library

.IF $(MGL_LITE)
CFLAGS          += -DMGL_LITE
ASFLAGS         += -DMGL_LITE
.IF $(STKCALL)
LIBFILE         = smgllt$L
.ELSE
LIBFILE         = mgllt$L
.ENDIF
.ELIF $(MGL_PRO)
CFLAGS          += -DMGL_PRO
ASFLAGS         += -DMGL_PRO
.IF $(STKCALL)
LIBFILE         = smglpr$L
.ELSE
LIBFILE         = mglpr$L
.ENDIF
.ELIF $(MGL_FLT3D)
CFLAGS          += -DMGL_FLT3D
ASFLAGS         += -DMGL_FLT3D
.IF $(STKCALL)
LIBFILE         = smglfl$L
.ELSE
LIBFILE         = mglfl$L
.ENDIF
.ELSE
.IF $(STKCALL)
LIBFILE         = smglfx$L
.ELSE
LIBFILE         = mglfx$L           # Default for MGL 2.0
.ENDIF
.ENDIF

.ENDIF

.IF $(STKCALL)
LIBFILECPP      = smglcpp$L
.ELSE
LIBFILECPP      = mglcpp$L
.ENDIF

# Target to build the library file

all: $(LIBFILE)

# The MGLWIN version uses PC based 386 assembler code

PCASM           = 1

# List of all MGLWIN C & assembler object files

LOCALCOBJ       = mglwin$O event$O mouse$O opengl$O

# Add all extra Win386 files to the libraries. This include all the
# Win386 linked in code for the WinDirect and PM/Pro libraries, so the
# user does not need to link with these libraries.

WIN386COBJ      = wdirect$O wdir386$O wing386$O pmlite$O pmpro$O pmpro32$O
WIN386ASMOBJ    = _pmlite$O _pmpro$O

.IF $(USE_WIN386)
LOCALCOBJ       += $(WIN386COBJ)
LOCALASMOBJ     += $(WIN386ASMOBJ)
.ENDIF

# List of all MGLWIN driver C & assembler object files

LOCALDRVCOBJ    = common$O vgapal$O windc$O vga$O ports$O vga4$O vga8$O     \
                  vgax$O svga$O svga4$O svga8$O svga16$O svga24$O svga32$O  \
				  linear8$O linear16$O linear24$O linear32$O				\
                  vesavbe$O vbeaf$O gaccel$O gfulldib$O gddraw$O            \
                  gtfcalc$O                                                 \
                  accel8$O accel16$O accel24$O accel32$O                    \
                  ddraw8$O ddraw16$O ddraw24$O ddraw32$O openglhw$O         \
                  fsogl8$O fsogl16$O fsogl24$O fsogl32$O                    \
                  fdib8$O fdib16$O fdib24$O fdib32$O

LOCALDRVASMOBJ  = _vga$O _ports$O _vga4$O _vgax$O                           \
                  _svga$O _svga8$O _svga16$O _svga24$O _svga32$O            \
                  _vbeaf$O _afstubs$O _ddstubs$O _glstubs$O

# Define the directories to search for local source files

LOCALSRC        = mglwin drivers\common drivers\vga drivers\svga            \
                  drivers\accel drivers\win drivers\opengl

# Create the local include file dependencies

LDEPEND_OBJ     = $(LOCALCOBJ) $(LOCALDRVCOBJ) $(LOCALASMOBJ) $(LOCALDRVASMOBJ) $(WIN386COBJ) $(WIN386ASMOBJ)
LDEPEND_SRC     = $(DEPEND_SRC);mglwin;drivers\common;drivers\vga;drivers\svga;drivers\accel;drivers\win;drivers\opengl

depend:
    @rm -f mglwin\mglwin.dep
    @makedep -amglwin\mglwin.dep -r -s -S$(MGLDEPEND_SRC:s/\/\\) @$(mktmp $(MGLDEPEND_OBJ:t"\n")\n)
    @makedep -amglwin\mglwin.dep -r -s -S@$(mktmp $(LDEPEND_SRC:s/\/\\)) @$(mktmp $(LDEPEND_OBJ:t"\n")\n)
    +@echo MGLWIN Object file dependency information generated.

# Local include file dependencies

.INCLUDE .IGNORE: "mglwin\mglwin.dep"

