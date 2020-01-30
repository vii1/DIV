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
#               Metaware High C/C++ 3.21 32 bit version. Supports Phar Lap's
#               TNT DOS Extender.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := hc386    # C-compiler and flags
   CFLAGS       :=
.IF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   ASFLAGS      := /t /mx /m /D__FLAT__ /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := hc386
   LDFLAGS       = $(CFLAGS)
   LIB          := 386lib   # TNT 386|lib Librarian
   LIBFLAGS     := -TC

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -g       # Turn on debugging for C compiler
   ASFLAGS      += /zi      # Turn on debugging for assembler
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -586 -O
.ELIF $(OPT_SIZE)
   CFLAGS       += -586 -O1
.ELSE
   CFLAGS       += -O0
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
   USE_TNT      := 1
   DX_CFLAGS    += -DTNT
   DX_ASFLAGS   += -DTNT
   LDFLAGS      += -LH:\TNT\LIB

# Place to look for PMODE library files

.IF NOT $(MK_PMODE)
PMLIB           := tnt\pmode.lib
.END

# Define where to install library files
   LIB_BASE     := $(SCITECH_LIB)\$(LIB_DIR)\DOS32\HC
   LIB_DEST     := $(LIB_BASE)

# Implicit generation rules for making object files
.SWAP %$O: %.c ; +$(CC) $(CFLAGS) -c $<
.SWAP %$O: %$P ; +$(CC) $(CFLAGS) -c $<
.SWAP %$O: %$A ; +$(AS) $(ASFLAGS) $(<:s,/,\)

# Implicit rule for building a library file using response file
.SWAP %$L:      ; +$(LIB) $(LIBFLAGS) $@ @$(mktmp,$*.rsp -R $?)

# Implicit rule for building an executable file using response file
.SWAP %$E:      ; +$(LD) $(LDFLAGS) -o $@ @$(mktmp $(&:s/\/\\) $(PMLIB) $(EXELIBS) -ldosx32.lib)
