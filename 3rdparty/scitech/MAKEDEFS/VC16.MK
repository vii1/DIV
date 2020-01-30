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
#               Microsoft Visual C++ 1.x 16 bit version. Supports 16 bit
#               DOS and Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : VC_LIBBASE

# Default commands for compiling, assembling linking and archiving
   CC           := cl       # C-compiler and flags
   CFLAGS       := /YX /w /G3 /Gs
.IF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   ASFLAGS      := /t /mx /m /D__COMM__ /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := cl       # Loader and flags
   LDFLAGS       = $(CFLAGS)
   RC           := rc       # WIndows resource compiler
   RCFLAGS      :=
   LIB          := lib      # Librarian
   LIBFLAGS     := /NOI /NOE
   ILIB         := implib   # Import librarian
   ILIBFLAGS    := /noignorecase

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += /Yd /Zi  # Turn on debugging for C compiler
   ASFLAGS      += /zi      # Turn on debugging for assembler
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += /Ox
.END

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += /FPi87 /DFPU387
   ASFLAGS      += /DFPU387 /DFPU_REG_RTN
.END

# Optionally compile a shareware version of a product
.IF $(SHW)
   LIB_DIR      := LIBSW
   CFLAGS       += /DSHAREWARE
   ASFLAGS      += -DSHAREWARE
.ELSE
   LIB_DIR      := LIB
.END

# Optionally compile a beta release version of a product
.IF $(BETA)
   CFLAGS       += /DBETA
   ASFLAGS      += /DBETA
.END

# Use a larger stack during linking if requested ???? How the fuck do you
# specify linker options on the CL command line?????

.IF $(STKSIZE)
.ENDIF

# Optionally compile for 16 bit Windows
.IF $(USE_WIN16)
.IF $(BUILD_DLL)
   CFLAGS       += /GD /Alfw /DBUILD_DLL
   ASFLAGS      += -DBUILD_DLL
.ELSE
   CFLAGS       += /GA /AL
.ENDIF
   DX_ASFLAGS   += -D__WINDOWS16__
   LIB_OS       = WIN16
.ELSE
   CFLAGS       += /AL
   LIB_OS       = DOS16
.END

# Place to look for PMODE library files

.IF $(MK_PMODE)
.ELSE
PMLIB           := pmode.lib
.END

# Define where to install library files
   LIB_BASE     := $(SCITECH_LIB)\$(LIB_DIR)\$(LIB_OS)\$(VC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Implicit generation rules for making object files
.SWAP %$O: %.c ; +$(CC) /nologo $(CFLAGS) /c $<
.SWAP %$O: %$P ; +$(CC) /nologo $(CFLAGS) /c $<
.SWAP %$O: %$A ; +$(AS) @$(mktmp $(ASFLAGS)) $(<:s,/,\)

# Implicit rule for building resource files
.SWAP %$R: %.rc ; +$(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
.SWAP %$D: ; +rclink $(LD) $(RC) $@ $(mktmp $(LDFLAGS) -e$@\n$(&:t"\n":s/\/\\)\n$(EXELIBS))

# Implicit rule for building a library file using response file
.IF $(BUILD_DLL)
.SWAP %$L:      ; $(ILIB) $(ILIBFLAGS) $@ $?
.ELIF $(IMPORT_DLL)
.SWAP %$L:      ; $(ILIB) $(ILIBFLAGS) $@ $?
.ELSE
.SWAP %$L: ; $(LIB) /nologo $(LIBFLAGS) $@ @$(mktmp -+$(?:t" &\n-+") &\n,,\n)
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_WIN16)
.SWAP %$E: ; +rclink $(LD) $(RC) $@ $(mktmp $(LDFLAGS) /Fe$@ $(&:s/\/\\) $(EXELIBS))
#.SWAP %$E: ; $(LD) @$(mktmp $(LDFLAGS) /Fe$@ $(&:s/\/\\) $(EXELIBS))
.ELSE
.SWAP %$E: ; $(LD) @$(mktmp $(LDFLAGS) /Fe$@ $(&:s/\/\\) $(PMLIB) $(EXELIBS))
.ENDIF
