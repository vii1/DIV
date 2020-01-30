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
#               Borland C++ 3.1 version. Supports 16 bit DOS development.
#
#############################################################################

# Include standard startup script definitions
.IMPORT: SCITECH
.INCLUDE: "$(SCITECH)\makedefs\startup.mk"

# Default commands for compiling, assembling linking and archiving
   CC           := bcc
   CFLAGS       := -ml -H=bcc.sym -i60 -d
.IF $(USE_TASMX)
   AS           := tasmx
.ELSE
   AS           := tasm
.ENDIF
   ASFLAGS      := /t /mx /m /iINCLUDE /i$(SCITECH)\INCLUDE
   LD           := bclink tlink.exe
   LDFLAGS      := -c
   LIB          := tlib
   LIBFLAGS     := /C

# Optionally turn on debugging information
.IF $(DBG)
   CFLAGS       += -v
   LDFLAGS      += -v
   ASFLAGS      += /zi
   LIBFLAGS     += /P128
.ELSE
   LDFLAGS      += -x
   ASFLAGS      += /q
.END

# Optionally turn on optimisations
.IF $(OPT)
   CFLAGS       += -3 -O2
.ELIF $(OPT_SIZE)
   CFLAGS       += -3 -O1
.END
                                         
# Optionally turn on direct i387 FPU instructions

.IF $(FPU)
   CFLAGS       += -f287 -DFPU387
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

# Define the default libraries to link with
   DEF_LIBS		:= mathl.lib cl.lib

# Define where to install library files
   LIB_DEST     := $(SCITECH_LIB)\$(LIB_DIR)\DOS16\BC3

# Implicit generation rules for making object files
.SWAP %$O: %.c ; +$(CC) @$(mktmp $(CFLAGS)) -c $<
.SWAP %$O: %$P ; +$(CC) @$(mktmp $(CFLAGS)) -c $<
.SWAP %$O: %$A ; +$(AS) @$(mktmp $(ASFLAGS)) $(<:s,/,\)

# Implicit rule for building a library file using response file
.SWAP %$L:      ; $(LIB) $(LIBFLAGS) $@ @$(mktmp -+$(?:t" &\n-+")\n)

# Implicit rule for building an executable file using response file
.SWAP %$E: ; +$(LD) $(mktmp $(LDFLAGS) -Tde c0l.obj+\n$(&:s/\/\\)\n$@\n$*.map\n$(DEF_LIBS) $(EXELIBS))
