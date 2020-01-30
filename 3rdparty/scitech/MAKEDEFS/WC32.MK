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
#               Watcom C++ 10.x 32 bit version. Supports Rational's DOS4GW
#               DOS Extender, PMODE/W, Causeway, FlashTek's X32-VM,
#               Phar Lap's TNT DOS Extender and 32 bit Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : USE_TNT USE_X32 USE_X32VM USE_PMODEW STKCALL USE_CAUSEWAY
.IMPORT .IGNORE : USE_WIN386 WC_LIBBASE NOOPT

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := wcc386   # C-compiler and flags
   CP           := wpp386   # C++ compiler and flags
   CFLAGS       := -zq-j-w2-s
.IF $(USE_TASMX)
   AS           := tasmx    # Assembler and flags
.ELSE
   AS           := tasm     # Assembler and flags
.ENDIF
   ASFLAGS      := /t /mx /m /D__FLAT__ /DSTDCALL_MANGLE /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := wlink    # Loader and flags
   LDFLAGS       =
   RC           := wrc      # Watcom resource compiler
.IF $(USE_WIN32)
   RCFLAGS      := -q /bt=nt # Mark as Win32 compatible resources
.ELSE
   RCFLAGS      := -q       # Mark as Win16 compatible resources
.ENDIF
   LIB          := wlib     # Librarian
   LIBFLAGS     := -q
   ILIB         := wlib     # Import librarian
   ILIBFLAGS    := -c

# Optionally turn on pre-compiled headers
.IF $(NO_PCH)
.ELSE
   CFLAGS       += -fh -fhq
.ENDIF

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -d2      # Turn on debugging for C compiler
   LIBFLAGS     += -p=128   # Larger page size for libraries with debug info!
   ASFLAGS      += /zi      # Turn on debugging for assembler
   LDFLAGS      += D A      # Turn on debugging for linker
.ELSE
   ASFLAGS      += /q       # Suppress object records not needed for linking
.END

# Optionally turn on optimisations (with or without stack conventions)
.IF $(STKCALL)
.IF $(OPT)
   CFLAGS       += -onatx-5s
.ELIF $(OPT_SIZE)
   CFLAGS       += -onaslmr-5s
.ELIF $(NOOPT)
   CFLAGS       += -od-5s
.ELSE
   CFLAGS       += -3s
.END
.ELSE
.IF $(OPT)
   CFLAGS       += -onatx-5r
.ELIF $(OPT_SIZE)
   CFLAGS       += -onaslmr-5r
.ELIF $(NOOPT)
   CFLAGS       += -od-5r
.END
.END

# Optionally turn on direct i387 FPU instructions optimised for Pentium
.IF $(FPU)
   CFLAGS       += -fpi87-fp5-DFPU387
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

# Use a larger stack during linking if requested, or use a default stack
# of 50k. The usual default stack provided by Watcom C++ is *way* to small
# for real 32 bit code development.
.IF $(USE_WIN32)
    # Not necessary for Win32 code.
.ELSE
.IF $(STKSIZE)
    LDFLAGS     += OP STACK=$(STKSIZE)
.ELSE
    LDFLAGS     += OP STACK=51200
.ENDIF
.ENDIF

.IF $(USE_TNT)                  # Use Phar Lap's TNT DOS Extender
    LDFLAGS      += SYS NT OP STUB=GOTNT.EXE
    DX_CFLAGS    += -DTNT
    DX_ASFLAGS   += -DTNT
    LIB_OS       = DOS32
.ELIF $(USE_X32VM)              # Use FlashTek X-32VM DOS extender
    LDFLAGS      += SYS X32RV
    DX_CFLAGS    += -DX32VM
    DX_ASFLAGS   += -DX32VM
    LIB_OS       = DOS32
.ELIF $(USE_X32)                # Use FlashTek X-32 DOS extender
    LDFLAGS      += SYS X32R
    DX_CFLAGS    += -DX32VM
    DX_ASFLAGS   += -DX32VM
    LIB_OS       = DOS32
.ELIF $(USE_WIN32)              # Build 32 bit Windows NT app
.IF $(BUILD_DLL)
	CFLAGS       += -bm-bd-bt=nt-DBUILD_DLL
    ASFLAGS      += -DBUILD_DLL
.ELSE
	CFLAGS       += -bm-bt=nt
.ENDIF
    DX_ASFLAGS   += -D__WINDOWS32__
    LIB_OS       = WIN32
.ELIF $(USE_WIN386)             # Build 32 bit Win386 extended app
.IF $(BUILD_DLL)
    CFLAGS       += -bd-bt=windows-DBUILD_DLL
    ASFLAGS      += -DBUILD_DLL
.ELSE
    CFLAGS       += -bt=windows
.ENDIF
    DX_ASFLAGS   += -D__WIN386__
    LIB_OS       = WIN386
.ELIF $(USE_PMODEW)             # PMODE/W
    USE_DOS4GW   := 1
    LDFLAGS      += SYS PMODEW
    DX_CFLAGS    += -DDOS4GW
    DX_ASFLAGS   += -DDOS4GW
    LIB_OS       = DOS32
.ELIF $(USE_CAUSEWAY)           # Causeway
    USE_DOS4GW   := 1
    LDFLAGS      += SYS CAUSEWAY
    DX_CFLAGS    += -DDOS4GW
    DX_ASFLAGS   += -DDOS4GW
    LIB_OS       = DOS32
.ELSE                           # Use DOS4GW
    USE_DOS4GW   := 1
    DX_CFLAGS    += -DDOS4GW
    DX_ASFLAGS   += -DDOS4GW
    LIB_OS       = DOS32
.END

# Place to look for PMODE library files

.IF $(MK_PMODE)
.ELIF $(USE_TNT)
.IF $(STKCALL)
PMLIB           := tnt\spmode.lib,
.ELSE
PMLIB           := tnt\pmode.lib,
.ENDIF
.ELIF $(USE_X32)
.IF $(STKCALL)
PMLIB           := x32\spmode.lib,
.ELSE
PMLIB           := x32\pmode.lib,
.ENDIF
.ELSE
.IF $(STKCALL)
PMLIB           := dos4gw\spmode.lib,
.ELSE
PMLIB           := dos4gw\pmode.lib,
.ENDIF
.END

# Define where to install library files
   LIB_BASE     := $(SCITECH_LIB)\$(LIB_DIR)\$(LIB_OS)\$(WC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Implicit generation rules for making object files
.SWAP %$O: %.c ; +$(CC) @$(mktmp $(CFLAGS)) $<
.SWAP %$O: %$P ; +$(CP) @$(mktmp $(CFLAGS)) $<
.SWAP %$O: %$A ; +$(AS) @$(mktmp $(ASFLAGS)) $(<:s,/,\)

# Implicit rule for building resource files
.SWAP %$R: %.rc ; +$(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
.IF $(USE_WIN32)
.SWAP %$D: ;
@[
    @trimlib $(mktmp $(LDFLAGS) OP quiet SYS nt_dll\nN $@\nF $(&:t",\n":s/\/\\)\nLIBR $(EXELIBS:t",")) $*.lnk
    rclink $(LD) $(RC) $@ $*.lnk
    @rm $*.lnk
]
.ELSE
.SWAP %$D: ;
@[
    @trimlib $(mktmp $(LDFLAGS) OP quiet SYS win386\nN $*.rex\nF $(&:t",\n":s/\/\\)\nLIBR $(EXELIBS:t",")) $*.lnk
    rclink $(LD) $(RC) $@ $*.lnk
    wbind $* -d -q -n
    @rm $*.lnk
]
.ENDIF

# Implicit rule for building a library file using response file
.IF $(BUILD_DLL)
.SWAP %$L: ; $(ILIB) $(ILIBFLAGS) $@ +$?
.ELIF $(IMPORT_DLL)
.SWAP %$L: ; $(ILIB) $(ILIBFLAGS) $@ +$?
.ELSE
.SWAP %$L: ; +$(LIB) $(LIBFLAGS) $@ @$(mktmp,$*.rsp -+$(?:t"\n-+":s/\/\\)\n)
.ENDIF

# Implicit rule for building an executable file using response file
.IF $(USE_TNT)
.SWAP %$E: ; +$(LD) $(LDFLAGS) @$(mktmp,$*.lnk OP quiet\nN $@\nF $(&:t",":s/\/\\)\nLIBR dosx32.lib,$(PMLIB)$(EXELIBS:t","))
.ELIF $(USE_X32)
.SWAP %$E: ;
@[
    @trimlib $(mktmp OP quiet\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(EXELIBS:t",")) $*.lnk
    $(LD) $(LDFLAGS) @$*.lnk
    x32fix $@
    @rm $*.lnk
]
.ELIF $(USE_WIN32)
.IF $(WIN32_GUI)
.SWAP %$E: ;
@[
    @trimlib $(mktmp $(LDFLAGS) OP quiet SYS win95\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(EXELIBS:t",")) $*.lnk
    rclink $(LD) $(RC) $@ $*.lnk
    @rm $*.lnk
]
.ELSE
.SWAP %$E: ;
@[
    @trimlib $(mktmp $(LDFLAGS) OP quiet SYS nt\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(EXELIBS:t",")) $*.lnk
    rclink $(LD) $(RC) $@ $*.lnk
    @rm $*.lnk
]
.ENDIF
.ELIF $(USE_WIN386)
.SWAP %$E: ;
@[
    @trimlib $(mktmp $(LDFLAGS) OP quiet SYS win386\nN $*.rex\nF $(&:t",":s/\/\\)\nLIBR $(EXELIBS:t",")) $*.lnk
    rclink $(LD) wbind $*.rex $*.lnk
    @rm $*.lnk
]
.ELSE
.SWAP %$E: ;
@[
    @trimlib $(mktmp OP quiet\nN $@\nF $(&:t",":s/\/\\)\nLIBR $(PMLIB)$(EXELIBS:t",")) $*.lnk
    $(LD) $(LDFLAGS) @$*.lnk
    @rm $*.lnk
]
.ENDIF
