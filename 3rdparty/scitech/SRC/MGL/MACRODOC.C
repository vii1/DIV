/****************************************************************************
*
*						MegaGraph Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Module containing documentation and fake prototypes for
*				macros and assembler functions defined in the MGRAPH.H
*				header but without real C code to extract docs from. 
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

/***************************************************************************
DESCRIPTION:
Changes the active bank on banked framebuffer devices.

HEADER:
mgraph.h

REMARKS:
This assembler callable function changes the active SuperVGA bank in banked
framebuffer modes, so that you can implement your own high performance direct
framebuffer rasterizing code for banked modes. To call this function you simply
load the new bank value into the DL register and then call it from your assembler
rasterizing code. All registers including the DL register will be preserved across the
call.

Note that if you are doing direct rasterizing in a bank framebuffer, the device
context surface pointer will be a pointer to the start of the banked framebuffer and
will never change.

SEE ALSO:
SVGA_setBankC
***************************************************************************/
void ASMAPI SVGA_setBank(void);

/***************************************************************************
DESCRIPTION:
Changes the active bank on banked framebuffer devices.

HEADER:
mgraph.h

PARAMETERS:
bank	- New bank number to change to

REMARKS:
This C callable function changes the active SuperVGA bank in banked framebuffer
modes, so that you can implement your own high performance direct framebuffer
rasterizing code for banked modes. To call this function you simply pass the new
bank value then call it from your C rasterizing code.

Note that if you are doing direct rasterizing in a bank framebuffer, the device
context surface pointer will be a pointer to the start of the banked framebuffer and
will never change.

SEE ALSO:
SVGA_setBank
***************************************************************************/
void ASMAPI SVGA_setBankC(int bank);

