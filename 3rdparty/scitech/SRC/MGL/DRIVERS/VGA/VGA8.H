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
* Description:	Main VGA8 device driver header file. This file contains all
*				of the C language declarations and definitions for the
*				VGA8 device driver file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_VGA_VGA8_H
#define	__DRIVERS_VGA_VGA8_H

#ifndef	__DRIVERS_SVGA_LINEAR8_H
#include "drivers\svga\linear8.h"
#endif

#ifndef	__DRIVERS_COMMON_VGA_H
#include "drivers\common\vga.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

/* The following defines the internal VGA8 device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	display_vec	v;				/* Display device vectors and info		*/
	short		oldBIOSMode;	/* Old video mode before graphics		*/
	ibool		old50Lines;		/* 1 if old mode was 50 line VGA8 mode	*/
	} VGA8_state;

/*------------------------- Function Prototypes ---------------------------*/

/* VGA C based routines */

ibool 	_ASMAPI VGA8_detect(int id,int force,int *driver,int *mode,modetab availableModes);
ibool 	_ASMAPI VGA8_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	VGA8_initInternal(MGLDC *dc);
void 	_ASMAPI VGA8_restoreTextMode(MGLDC *dc);
ibool 	_ASMAPI VGA8_restoreGraphMode(MGLDC *dc);
void 	* _ASMAPI VGA8_getDefaultPalette(MGLDC *dc);

#endif	/* __DRIVERS_VGA_VGA8_H */
