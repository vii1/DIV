#############################################################################
#
#                       MegaGraph Graphics Library
#
#  ========================================================================
#
#    The contents of this file are subject to the SciTech MGL Public
#    License Version 1.0 (the "License"); you may not use this file
#    except in compliance with the License. You may obtain a copy of
#    the License at http://www.scitechsoft.com/mgl-license.txt
#
#    Software distributed under the License is distributed on an
#    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    rights and limitations under the License.
#
#    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
#
#    The Initial Developer of the Original Code is SciTech Software, Inc.
#    All Rights Reserved.
#
#  ========================================================================
#
# Descripton:   Generic DMAKE startup makefile definitions file. Assumes
#               that the SCITECH environment variable has been set to point
#               to where all our stuff is installed. You should not need
#               to change anything in this file.
#
#               DJGPP V2 port of GNU C/C++ to DOS with DPMI only.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Override some file suffix definitions
   L            := .a     # Libraries
   O            := .o     # Objects

# Import enivornment variables that we use
.IMPORT .IGNORE : DJ_LIBBASE

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := gcc      # C-compiler and flags
   CFLAGS       :=
.IF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   ASFLAGS      := /t /mx /m /D__FLAT__ /iINCLUDE /i$(SCITECH)\INCLUDE /DDJGPP
   LD           := dj_ld    # Loader and flags
   LDFLAGS       = $(CFLAGS)
   LIB          := ar       # Librarian
   LIBFLAGS     := rs

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -g       # Turn on debugging for C compiler
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -O2
.ELIF $(OPT_SIZE)
   CFLAGS       += -O1
.END

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += -DFPU387
   ASFLAGS      += -DFPU387
.END

# Optionally compile a shareware version of a product
.IF $(SHW)
   LIB_DIR      := LIBSW
   CFLAGS       += -DSHAREWARE
   ASFLAGS      += -DSHAREWARE
.ELSE
   LIB_DIR      := LIB
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += -DBETA
   ASFLAGS      += -DBETA
.END

# DOS extender dependant flags
   DX_CFLAGS    +=
   DX_ASFLAGS   +=

# Define where to install library files
   LIB_DEST     := $(SCITECH_LIB)\$(LIB_DIR)\DOS32\$(DJ_LIBBASE)

# Place to look for PMODE library files

.IF $(MK_PMODE)
.ELSE
PMLIB           := pmode.a
.END

# Implicit generation rules for making object files
.SWAP %$O: %.c ; $(CC) $(CFLAGS) -c $<
.SWAP %$O: %$P ; $(CC) $(CFLAGS) -c $<
.SWAP %$O: %$A ;
@[
    $(AS) @$(mktmp $(ASFLAGS)) $(<:s,/,\)
    @obj2bfd -u -O coff-go32 $*.obj
    @rm $*.obj
]

# Implicit rule for building a library file using response file
.SWAP %$L:      ; $(LIB) $(LIBFLAGS) $@ @$(mktmp $(&:s/\/\\)\n)

# Implicit rule for building an executable file using response file
.SWAP %$E:      ; $(LD) $(LDFLAGS) $@ @$(mktmp $(&:s/\/\\) $(EXELIBS) $(PMLIB) -lgpp -lstdcx -lm -lpc)
