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
# Descripton:   Generic GNU MAKE startup makefile definitions file. All the
#               makefiles look for a startup makefile in the 
#               $(SCITECH)\makedefs directory, and one of the startup files
#               for the appropriate target OS and compiler should be either
#               copied to or symbolic linked to startup.mk.
#
#               Linux version for GNU C/C++.
#
#############################################################################

ifndef MK_EXE
.PHONY all:
	@echo "Type 'make lib'      to build the library"
	@echo "     'make install'  to build the library and install it"
	@echo "     'make tests'    to build any relevant test programs"
	@echo "     'make clean'    to clean all objects and libraries"
	@echo "     'make cleanexe' to clean everything including executeables"
	@echo ""
	@echo "The following modify the compile environment:"
	@echo "  DBG=1      - Enable debugging information"
	@echo "  OPT=1      - Enable speed optimisations"
	@echo "  OPT_SIZE=1 - Enable size optimisations"
	@echo "  OPT_MAX=1  - Enable max optimisations (-O6 for some compilers)"
	@echo ""
	@echo "For example 'make OPT=1 install' will compile with optimisations and"
	@echo "install the compiled library into the library install directories"
endif

# The following are expected to be defined as shell environment variables
# by the user using the SciTech libraries. For bash add this to your
# .bashrc file (assuming you installed the libraries to /usr/scitech):
#
# export SCITECH=/usr/scitech
# export SCITECH_LIB=/usr/scitech
#
# The SCITECH_LIB variable is used to define where the built libraries
# files should be 'installed' to, and for a local system will always be the
# same as the SCITECH variable. However if the libraries are located on a
# shared network drive, or more specifically if the libraries are being 
# built over the network this should point to the location where the files
# will end up (ie: on your file server).

# The following are expected to be either undefined or defined by the 
# high level makefile before this configution makefile is included:
#
# INCLUDE       = -Ilocal_include

# The following are defined by the startup makefile and allow the common
# makefiles to determine the target build environment (if anything different
# needs to be done):
#
# UNIX  - Building for a Unix like system target
# DOS   - Building for a DOS target
# WIN32 - Building for a 32-bit Windows target
# OS2   - Building for a 32-bit OS/2 target
  UNIX          := 1

# File suffix definitions. The following suffixes are defiend (comments afer
# the suffix include whitespace in the commands with GNU Make):
#
# L - Library files
# E - Executable files
# O - Object files
# A - Assembler source files
# S - GNU assembler source files
# P - C++ source files
#
# NOTE: Linux does not require any extenion for executeable files, but you
#       can use an extension if you wish. We use the .x extension for building
#       executeable files so that we can use implicit rules to make the 
#       makefiles simpler and more portable between systems. When you install
#       the files to a local bin directory, you will probably want to remove
#       the .x extension.
   L            := .a
   E            := .x
   O            := .o
   A            := .asm
   S            := .s
   P            := .cpp

# File prefix/suffic definitions. The following prefixes are defined, and are
# used primarily to abstract between the Unix style libXX.a naming convention 
# and the DOS/Windows/OS2 naming convention of XX.lib.
#
# LP - Library file prefix (name of file on disk)
# LL - Library link prefix (name of library on link command line)
# LE - Library link suffix (extension of library on link command line)
   LP           := lib
   LL           := -l
   LE           :=

# Default commands for compiling, assembling linking and archiving. 
   CC           := gcc
   CFLAGS       := -Wall -I. -Iinclude -I$(SCITECH)/include $(INCLUDE)
   CXX          := g++
   AS           := tasm
   ASFLAGS      := /t /mx /m /D__FLAT__ /iINCLUDE /i$(SCITECH)\INCLUDE /DDJGPP
   LD           := gcc
   LDFLAGS      := -L. -L$(SCITECH)/lib/linux/gcc
   LIB          := ar
   LIBFLAGS     := rs

# Optionally turn on debugging information
ifdef DBG
   CFLAGS       += -g
else
   ASFLAGS      += /q
endif

# Optionally turn on optimisations
ifdef OPT_MAX
   CFLAGS       += -O6
endif
ifdef OPT
   CFLAGS       += -O2
else
ifdef OPT_SIZE
   CFLAGS       += -O1
endif
endif

# Optionally compile a beta release version of a product
ifdef BETA
   CFLAGS       += -DBETA
   ASFLAGS      += -DBETA
endif

# Target environment dependant flags
   CFLAGS       += -D__LINUX__
   ASFLAGS      += -D__LINUX__

# Define where to install library files
   LIB_DEST     := $(SCITECH_LIB)/lib/linux/gcc

# Macro to install a library file
   INSTALL      := cp

# Implicit generation rules for making object files from source files
%$O: %.c ; $(CC) $(CFLAGS) -c $< 
%$O: %$P ; $(CXX) $(CFLAGS) -c $<

# TODO: Figure out how to call the TASM assembler via a DOSEMU shell. For the
#       moment we simply copy foo.o to the target file so that the libraries
#		will build, albeit without assembler code.
%$O: %$A ; cp $(SCITECH)/makedefs/foo.o $@
#@[
#    $(AS) $(ASFLAGS) $<
#    @obj2bfd -u -O coff-go32 $*.obj
#    @rm $*.obj
#]

# Implicit rule for building a library file
%$L:     ; $(LIB) $(LIBFLAGS) $@ $^

# Implicit rule for building an executable file
%$E:     ; $(LD) $(LDFLAGS) -o $@ $^ $(EXELIBS) -lpmode -lm

