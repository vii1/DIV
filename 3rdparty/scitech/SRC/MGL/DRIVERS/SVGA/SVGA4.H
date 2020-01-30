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
* Description:	Main VGA4 device driver header file. This file contains all
*				of the C language declarations and definitions for the
*				SVGA4 device driver file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_SVGA_SVGA4_H
#define	__DRIVERS_SVGA_SVGA4_H

#ifndef	__DRIVERS_VGA_VGA4_H
#include "drivers\vga\vga4.h"
#endif

#ifndef	__DRIVERS_COMMON_SVGA_H
#include "drivers\common\svga.h"
#endif

/*------------------------- Function Prototypes ---------------------------*/

/* VGA C based routines */

ibool 	_ASMAPI SVGA4_detect(int id,int force,int *driver,int *mode,modetab availableModes);
ibool 	_ASMAPI SVGA4_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
ibool 	_ASMAPI SVGA4_restoreGraphMode(MGLDC *dc);

#endif	/* __DRIVERS_SVGA_SVGA4_H */
