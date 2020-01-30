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
#               Microsoft Visual C++ 2.x 32 bit version. Supports Phar Lap
#               TNT DOS Extender and 32 bit Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : TNT_PATH VC_LIBBASE

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := cl       # C-compiler and flags
   CFLAGS       := /YX /w
.IF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   ASFLAGS      := /t /mx /m /D__FLAT__ /D__COMM__ /DSTDCALL_MANGLE /iINCLUDE /i$(SCITECH)\INCLUDE
.IF $(USE_WIN32)
   LD           := cl
   LDFLAGS       = $(CFLAGS)
   LDENDFLAGS   := -link /SUBSYSTEM:windows
.ELSE
   LD           := 386link  # Loader and flags
   LDFLAGS       = $(CFLAGS)
.ENDIF
   RC           := rc       # Watcom resource compiler
   RCFLAGS      :=          # Mark as Win32 compatible resources
   LIB          := lib      # Librarian
   LIBFLAGS     :=
   ILIB         := lib      # Import librarian
   ILIBFLAGS    := /MACHINE:IX86

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += /Yd /Zi  # Turn on debugging for C compiler
   ASFLAGS      += /zi      # Turn on debugging for assembler
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += /G5 /O2 /Ox
.ELIF $(OPT_SIZE)
   CFLAGS       += /G5 /O1
.END

# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += /DFPU387
   ASFLAGS      += /DFPU387
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

# Use a larger stack during linking if requested, or use a default stack
# of 50k. The usual default stack provided by Visual C++ is *way* to small
# for real 32 bit code development.

.IF $(USE_WIN32)
    # Not necessary for Win32 code.
.ELSE
.IF $(STKSIZE)
    LDENDFLAGS  += /STACK:$(STKSIZE)
.ELSE
    LDENDFLAGS  += /STACK:51200
.ENDIF
.ENDIF

# DOS extender dependant flags
.IF $(USE_WIN32)                # Build 32 bit Windows NT app
.IF $(BUILD_DLL)
   CFLAGS       += /MT /LD -DBUILD_DLL
   ASFLAGS      += -DBUILD_DLL
.ELSE
   CFLAGS       += /MT
.ENDIF
   DX_ASFLAGS   += -D__WINDOWS32__
   LIB_OS       = WIN32
.ELSE
   USE_TNT      := 1
   DX_CFLAGS    += /DTNT
   DX_ASFLAGS   += -DTNT
   LIB_OS       = DOS32
.ENDIF

# Place to look for PMODE library files
.IF $(MK_PMODE)
.ELSE
PMLIB           := tnt\pmode.lib
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
.SWAP %$D: ; +rclink $(LD) $(RC) $@ $(mktmp $(LDFLAGS) /Fe$@ $(&:t"\n"s/\/\\) $(EXELIBS) kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib comdlg32.lib)

# Implicit rule for building a library file using response file. Note that
# we use a special .VCD file that contains the EXPORT definitions for the
# Microsoft compiler, since the LIB utility automatically adds leading
# underscores to exported functions.
.IF $(BUILD_DLL)
#  .SWAP %$L: ; $(ILIB) $(ILIBFLAGS) /DEF:$(?:b).vcd /OUT:$@
.ELIF $(IMPORT_DLL)
.SWAP %$L: ;
@[
    makedef -v $(?:b)
    $(ILIB) $(ILIBFLAGS) /DEF:$(?:b).def /OUT:$@
]
.ELSE
.SWAP %$L: ; $(LIB) $(LIBFLAGS) /out:$@ @$(mktmp $(?:t"\n")\n)
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_WIN32)
.SWAP %$E: ; +$(LD) $(LDFLAGS) /Fe$@ @$(mktmp $(&:t"\n"s/\/\\) $(EXELIBS) kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib comdlg32.lib $(LDENDFLAGS))
.ELSE
.SWAP %$E: ; +$(LD) @msvc32.dos -exe $@ @$(mktmp,$*.lnk $(&:t"\n"s/\/\\) $(PMLIB) $(EXELIBS) $(LDENDFLAGS))
.ENDIF
