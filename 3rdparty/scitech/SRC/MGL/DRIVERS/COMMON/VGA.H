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
* Description:	Header file for all common VGA compatible device driver
*				code.
*
*
****************************************************************************/

#ifndef	__DRIVERS_COMMON_VGA_H
#define	__DRIVERS_COMMON_VGA_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*------------------------- Function Prototypes ---------------------------*/

ibool	MGLAPI VGA_detect(int *driver);
ibool 	MGLAPI VGA_setMode(MGLDC *dc,int AX,int BX,short *oldBIOSMode,ibool *old43Lines,void (*saveVideoMemory)(int mode));
void 	MGLAPI VGA_useMode(modetab modes,int mode,int id,gmode_t *modeInfo);
void 	MGLAPI VGA_restoreMode(ibool oldBIOSMode,ibool old43Lines,void (*restoreVideoMemory)(void));
void 	_ASMAPI VGA_setActivePage(MGLDC *dc,int page);
void 	_ASMAPI VGA_setVisualPage(MGLDC *dc,int page,int waitVRT);
void 	_ASMAPI VGA_vSync(MGLDC *dc);
void 	_ASMAPI VGA_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);
void	_ASMAPI VGA_blastPalette(uchar *pal,int num,int index,int maxProgram,int waitVRT);
void	_ASMAPI VGA_setCRTStart(int crtStart,int page,int waitVRT);

#endif	/* __DRIVERS_COMMON_VGA_H */
