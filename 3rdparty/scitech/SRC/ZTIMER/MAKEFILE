#############################################################################
#
#								 Zen Timer
#
#							   From the book
#						 "Zen of Assembly Language"
#							Volume 1, Knowledge
#
#							 by Michael Abrash
#
# Descripton:   Generic makefile for the Zen Timer Library. Requires the
#               SciTech Software Makefile Utilities package to be
#               installed.
#
# $Date:   20 Feb 1996 14:35:34  $ $Author:   KendallB  $
#
#############################################################################

# DOS extender dependant flags

CFLAGS          += $(DX_CFLAGS)
ASFLAGS         += $(DX_ASFLAGS)

# Name of library and object files required to build it

.IF $(STKCALL)
LIBFILE         = sztimer$L
.ELSE
LIBFILE         = ztimer$L
.ENDIF
OBJECTS         = lztimer$O ulztimer$O ztimer$O

# Name of executable files built

EXEFILES        = test$E testc$E

all: $(EXEFILES)

test$E: test$O $(LIBFILE)
testc$E: testc$O $(LIBFILE)

# Define the list of object files to create dependency information for

DEPEND_OBJ      = $(OBJECTS) test$O testc$O

.INCLUDE: "$(SCITECH)\makedefs\common.mk"

