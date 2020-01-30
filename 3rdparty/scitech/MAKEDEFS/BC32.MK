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
#               Borland C++ 4.0 32 bit version. Supports Borland's DOS Power
#               Pack DPMI32 DOS extender, Phar Lap's TNT DOS Extender and
#               32 bit Windows development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Import enivornment variables that we use
.IMPORT .IGNORE : USE_TNT USE_WIN32 USE_BC5 USE_VXD BC_LIBBASE

# We are compiling for a 32 bit envionment
   _32BIT_      := 1

# Default commands for compiling, assembling linking and archiving
   CC           := bcc32
   CFLAGS       := -w- -4 -H=bcc32.sym -i60 -d
.IF $(USE_TASMX)
   AS           := tasmx
.ELSE
   AS           := tasm
.ENDIF
   ASFLAGS      := /t /mx /m /D__FLAT__ /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := bclink tlink32.exe
   LDFLAGS      := -c
   RC           := brc32
.IF $(USE_BC5)
   WIN_VERSION  := -V4.0
   RCFLAGS      := -32
.ELSE
   RCFLAGS      := -w32
.ENDIF
   LIB          := tlib
   LIBFLAGS     := /C /P32
   ILIB         := implib
   ILIBFLAGS    := -c

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -v
   LDFLAGS      += -v
   LIBFLAGS     += /P128
   ASFLAGS      += /zi
.ELSE
   LDFLAGS      += -x
   ASFLAGS      += /q
.END

# Optionally turn on optimisations (-5 -O2 breaks BC++ 4.0-4.5 sometimes)
.IF $(OPT)
   CFLAGS       += -5 -O2 -k-
.ELIF $(OPT_SIZE)
   CFLAGS       += -5 -O1 -k-
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

# Optionally use Phar Lap's TNT DOS Extender, otherwise use the DOS Power Pack
.IF $(USE_TNT)
   CFLAGS       += -D__MSDOS__
   DX_CFLAGS    += -DTNT
   DX_ASFLAGS   += -DTNT
   LIB_OS       = DOS32
.ELIF $(USE_VXD)
.IF $(BUILD_DLL)
   CFLAGS       += -WD -DBUILD_DLL
   ASFLAGS      += -DBUILD_DLL
.ELSE
   CFLAGS       += -W
.ENDIF
   CFLAGS       += -RT- -x- -Oi -DIS_32 -DWANTVXDWRAPS -DVTOOLSD -DWIN40 -DDEFSEG=1 -zC_LTEXT -zALCODE -zR_LDATA -zTLCODE
   DX_ASFLAGS   += -D__WIN32_VXD__
   LIB_OS       = VXD
.ELIF $(USE_WIN32)
.IF $(BUILD_DLL)
   CFLAGS       += -WD -DBUILD_DLL
   ASFLAGS      += -DBUILD_DLL
.ELSE
   CFLAGS       += -W -WM
.ENDIF
   DEF_LIBS		:= import32.lib cw32mt.lib
   DX_ASFLAGS   += -D__WINDOWS32__
   LIB_OS       = WIN32
.ELSE
   USE_DPMI32   := 1
   CFLAGS       += -D__MSDOS__
   DX_CFLAGS    += -WX -DDPMI32
   DX_ASFLAGS   += -DDPMI32
   LIB_OS       = DOS32
.END

# Place to look for PMODE library files

.IF $(MK_PMODE)
.ELIF $(USE_TNT)
PMLIB           := tnt\pmode.lib
.ELSE
PMLIB           := dpmi32\pmode.lib
.END

# Define where to install library files
   LIB_BASE     := $(SCITECH_LIB)\$(LIB_DIR)\$(LIB_OS)\$(BC_LIBBASE)
   LIB_DEST     := $(LIB_BASE)

# Implicit generation rules for making object files
.SWAP %$O: %.c ; +$(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $<
.SWAP %$O: %$P ; +$(CC) @$(mktmp $(CFLAGS:s/\/\\)) -c $<
.SWAP %$O: %$A ; +$(AS) @$(mktmp $(ASFLAGS:s/\/\\)) $(<:s,/,\)

# Implicit rule for building resource files
.SWAP %$R: %.rc ; +$(RC) $(RCFLAGS) -r $<

# Implicit rule for building a DLL using a response file
.SWAP %$D: ;
@[
    makedef $(@:b)
	$(LD) $(mktmp $(LDFLAGS) -Tpd -aa c0d32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(EXELIBS)\n$*.def)
]

# Implicit rule for building a library file using response file
.IF $(BUILD_DLL)
.SWAP %$L: ; $(ILIB) $(ILIBFLAGS) $@ $?
.ELIF $(IMPORT_DLL)
.SWAP %$L: ; $(ILIB) $(ILIBFLAGS) $@ $?
.ELSE
.SWAP %$L: ; $(LIB) $(LIBFLAGS) $@ @$(mktmp -+$(?:t" &\n-+")\n)
.ENDIF

# Implicit rule for building an executable file using response file

.IF $(USE_TNT)
.SWAP %$E: ;
	@echo Not supported yet!
#	TODO! This needs to be updated when we get TNT support working again!
#@[
#	@cp $(mktmp stub 'gotnt.exe') $*.def
#	$(LD) @$(mktmp $(LDFLAGS) -e$@ $(&:s/\/\\) $(PMLIB) $(EXELIBS) dosx32.lib)
#    @rm $*.def
#]
.ELIF $(USE_WIN32)
.IF $(WIN32_GUI)
.SWAP %$E: ; +$(LD) $(mktmp $(LDFLAGS) -Tpe -aa $(WIN_VERSION) c0w32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(EXELIBS)\n$*.def)
.ELSE
.SWAP %$E: ; +$(LD) $(mktmp $(LDFLAGS) -Tpe -ap c0x32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(EXELIBS)\n$*.def)
.ENDIF
.ELSE
.SWAP %$E: ; +$(LD) $(mktmp $(LDFLAGS) -Tpe -ap c0x32.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(EXELIBS)\n$*.def)
.END
