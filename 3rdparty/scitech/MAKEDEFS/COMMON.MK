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
#               current directory, and maintaining the source files with
#               RCS.
#
#############################################################################

# Define where to install all compiled DLL files

.IF $(SHW)
DLL_DEST    := $(SCITECH_LIB)\DLLSW
.ELSE
DLL_DEST    := $(SCITECH_LIB)\REDIST
.ENDIF

# Target to build the library and DLL file if specified

.IF $(LIBFILE)

lib: $(LIBFILE)

.IF $(DLLFILE)

# Build and install a DLL file, or simply build import library and install

.IF $(BUILD_DLL)

$(DLLFILE): $(OBJECTS)
$(LIBFILE): $(DLLFILE)
.SWAP install: $(LIBFILE) $(DLLFILE)
    $(INSTALL) $(LIBFILE) $(LIB_DEST)$(LIB_EXTENDER)
    $(INSTALL) $(DLLFILE) $(DLL_DEST)
.ELSE

$(LIBFILE): $(DLL_DEST)\$(DLLFILE)
.SWAP install: $(LIBFILE)
    $(INSTALL) $(LIBFILE) $(LIB_DEST)$(LIB_EXTENDER)

.ENDIF
.ELSE

# Build and install a normal library file

.IF $(USE_DLL)
.ELSE
$(LIBFILE): $(OBJECTS)

.SWAP install: $(LIBFILE)
    $(INSTALL) $(LIBFILE) $(LIB_DEST)$(LIB_EXTENDER)
.ENDIF
.ENDIF
.ENDIF

# Clean up directory removing all files not needed to make the library.
# There seems to be a bug in the version of RM that we are using, that
# causes it to not delete all the files if there are lots of them, hence
# we issue the rm command twice which should solve the problem (until we
# get a fixed version of RM).

.SWAP clean:
    -@rm -f *.obj *.o *.sym *.bak *.tdk *.swp *.map *.err *.csm *.lib *.aps
    -@rm -f *.~* *.td *.tr *.tr? *.td? *.rws *.res *.exp *.ilk *.pdb *.pch *.a bcc32.*
.IF $(LIBFILE)
    -@rm -f $(LIBCLEAN)
.END

.SWAP cleanexe:   clean
.IF $(UNIX)
    -@rm -f $(EXEFILES)
.ELSE
    -@rm -f *$E *.drv *.rex
.ENDIF

.SWAP realclean:  cleanexe
    -@rm -f *.exe *.dll *.dsk *.obr *.mbt *.mrt *.dsw

# Create the include file dependencies using the MKUTIL makedep program if
# the list of dependent object files is defined

.IF $(DEPEND_OBJ)
.SWAP depend:
    @rm -f makefile.dep
.IF $(DEPEND_SRC)
.IF $(DEPEND_INC)
	@makedep -amakefile.dep -r -s -I@$(mktmp $(DEPEND_INC:s/\/\\)) -S@$(mktmp $(DEPEND_SRC:s/\/\\)) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ELSE
	@makedep -amakefile.dep -r -s -S@$(mktmp $(DEPEND_SRC:s/\/\\)) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ENDIF
.ELSE
.IF $(DEPEND_INC)
	@makedep -amakefile.dep -r -s -I@$(mktmp $(DEPEND_INC:s/\/\\)) @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ELSE
	@makedep -amakefile.dep -r -s @$(mktmp $(DEPEND_OBJ:t"\n")\n)
.ENDIF
.ENDIF
    +@echo Object file dependency information generated.
.ENDIF

# Include file dependencies

.INCLUDE .IGNORE: "makefile.dep"
