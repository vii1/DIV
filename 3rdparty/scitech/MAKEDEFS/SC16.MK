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
#               Symantec C++ 6.x/7.x 16 bit version. Supports 16 bit DOS
#               and 16 bit Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : SC_LIBBASE

# Default commands for compiling, assembling linking and archiving
   CC           := sc       # C-compiler and flags
   CFLAGS       := -ml -Jm
.IF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   ASFLAGS      := /t /mx /m /D__COMM__ /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := sc       # Loader and flags
   LDFLAGS       = $(CFLAGS)
   RC           := rcc      # WIndows resource compiler
   RCFLAGS      :=          # Mark as Win32 compatible resources
   LIB          := lib      # Librarian
   LIBFLAGS     := /N /B
   ILIB         := implib   # Import librarian
   ILIBFLAGS    :=

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -g       # Turn on debugging for C compiler
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -5 -o+all
.ELIF $(OPT_SIZE)
   CFLAGS       += -5 -o+space
.END

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += -ff -DFPU387
   ASFLAGS      += -DFPU387 -DFPU_REG_RTN
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

# User a larger stack if requested

.IF $(STKSIZE)
    LDFLAGS     += =$(STKSIZE)
.ENDIF

# Optionally compile for 16 bit Windows
.IF $(USE_WIN16)
.IF $(BUILD_DLL)
   CFLAGS       += -WD -DBUILD_DLL
   ASFLAGS      += -DBUILD_DLL
.ELSE
   CFLAGS       += -WA
.ENDIF
   DX_ASFLAGS   += -D__WINDOWS16__
   LIB_OS       = WIN16
.ELSE
   LIB_OS       = DOS16
.END

# Place to look for PMODE library files

.IF $(MK_PMODE)
.ELSE
PMLIB           := pmode.lib
.END

# Define where to install library files
   LIB_BASE     := $(SCITECH_LIB)\$(LIB_DIR)\$(LIB_OS)\$(SC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Implicit generation rules for making object files
.SWAP %$O: %.c ; +$(CC) $(CFLAGS) -c $<
.SWAP %$O: %$P ; +$(CC) $(CFLAGS) -c $<
.SWAP %$O: %$A ; +$(AS) @$(mktmp $(ASFLAGS)) $(<:s,/,\)

# Implicit rule for building resource files
.SWAP %$R: %.rc ; +$(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
.SWAP %$D: ; +$(LD) $(LDFLAGS) @$(mktmp $(&:s/\/\\) $(EXELIBS))

# Implicit rule for building a library file using response file
.IF $(BUILD_DLL)
.SWAP %$L: ; $(ILIB) $(ILIBFLAGS) $@ $?
.ELIF $(IMPORT_DLL)
.SWAP %$L: ; $(ILIB) $(ILIBFLAGS) $@ $?
.ELSE
.SWAP %$L: ; +$(LIB) $(LIBFLAGS) $@ @$(mktmp -+$(?:t" &\n-+")\n)
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_WIN16)
.SWAP %$E: ; +$(LD) $(LDFLAGS) @$(mktmp $(&:s/\/\\) $(EXELIBS))
.ELSE
.SWAP %$E: ; +$(LD) $(LDFLAGS) @$(mktmp $(&:s/\/\\) $(PMLIB) $(EXELIBS))
.ENDIF
