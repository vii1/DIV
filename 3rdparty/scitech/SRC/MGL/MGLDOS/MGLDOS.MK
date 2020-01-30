#############################################################################
#
#               Copyright (C) 1991-1997 SciTech Software, Inc.
#                            All rights reserved.
#
# Descripton:   Specialised definitions for building the MGLDOS target
#               version of the MGL.
#
# $Date:   07 Nov 1997 16:09:26  $ $Author:   KendallB  $
#
#############################################################################

# Optionally turn on NO_IDE_BUG for BC++ 3.1 IDE

.IF $(NOIDEBUG)
CFLAGS          += -DNO_IDE_BUG
LIBFILE         = mgldb$L
.ELSE

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

# The MGLDOS version uses PC based 386 assembler code

PCASM           = 1

# List of all MGLDOS C & assembler object files

LOCALCOBJ       = mgldos$O maskcode$O event$O mouse$O opengl$O
LOCALASMOBJ     = _event$O

# List of all MGLDOS driver C & assembler object files

LOCALDRVCOBJ    = common$O vgapal$O vga$O ports$O vga4$O vga8$O vgax$O      \
				  svga$O svga4$O svga8$O svga16$O svga24$O svga32$O			\
				  linear8$O linear16$O linear24$O linear32$O				\
                  vesavbe$O vbeaf$O gaccel$O gtfcalc$O                      \
                  accel8$O accel16$O accel24$O accel32$O

LOCALDRVASMOBJ  = _vga$O _ports$O _vga4$O _vgax$O                           \
                  _svga$O _svga8$O _svga16$O _svga24$O _svga32$O            \
                  _vbeaf$O _afstubs$O

# Define the directories to search for local source files

LOCALSRC        = mgldos drivers\common drivers\vga drivers\svga            \
                  drivers\accel

# Create the local include file dependencies

LDEPEND_OBJ     = $(LOCALCOBJ) $(LOCALDRVCOBJ) $(LOCALASMOBJ) $(LOCALDRVASMOBJ)
LDEPEND_SRC     = $(DEPEND_SRC);mgldos;drivers\common;drivers\vga;drivers\svga;drivers\accel

depend:
    @rm -f mgldos\mgldos.dep
    @makedep -amgldos\mgldos.dep -r -s -S$(MGLDEPEND_SRC:s/\/\\) @$(mktmp $(MGLDEPEND_OBJ:t"\n")\n)
    @makedep -amgldos\mgldos.dep -r -s -S@$(mktmp $(LDEPEND_SRC:s/\/\\)) @$(mktmp $(LDEPEND_OBJ:t"\n")\n)
    +@echo MGLDOS Object file dependency information generated.

# Local include file dependencies

.INCLUDE .IGNORE: "mgldos\mgldos.dep"
