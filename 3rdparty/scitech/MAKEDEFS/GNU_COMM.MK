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
# Descripton:   Common makefile targets used by all SciTech Software
#               makefiles. This file includes targets for cleaning the
#               current directory etc.
#
#############################################################################

# Target to build the library 

ifdef LIBFILE
lib: $(LIBFILE)

$(LIBFILE): $(OBJECTS)

install: $(LIBFILE)
	$(INSTALL) $(LIBFILE) $(LIB_DEST)
endif

# Clean up directory removing all files not needed to make the library.
# There seems to be a bug in the version of RM that we are using, that
# causes it to not delete all the files if there are lots of them, hence
# we issue the rm command twice which should solve the problem (until we
# get a fixed version of RM).

clean:
	-@rm -f *.o *.bak *.a core *.a *.lib

cleanexe: clean
	-@rm -f *$E $(EXEFILES)

# Create the include file dependencies using the MKUTIL makedep program if
# the list of dependent object files is defined
#
# TODO: Need to either port the DOS makedep program to Linux, or find an 
#       alternative method of generating automatic dependencies (perhaps
#		using the gcc -MM method to a file).

ifdef DEPEND_OBJ
depend:
	@rm -f makefile.dep
ifdef DEPEND_SRC
	@makedep -amakefile.dep -r -s -S$(DEPEND_SRC:s/\/\\) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
else
	@makedep -amakefile.dep -r -s @$(mktmp $(DEPEND_OBJ:t"\n")\n)
endif
	+@echo Object file dependency information generated.
endif

# Include file dependencies (ignore if the file does not exist)

ifeq (makefile.dep,$(wildcard,makefile.dep))
include makefile.dep
endif
