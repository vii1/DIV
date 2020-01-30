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
* Description:	Main SVGA8 device driver header file. This file contains all
*				of the C language declarations and definitions for the
*				SVGA8 device driver file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_SVGA_SVGA8_H
#define	__DRIVERS_SVGA_SVGA8_H

#ifndef	__DRIVERS_COMMON_SVGA_H
#include "drivers\common\svga.h"
#endif

/*------------------------- Function Prototypes ---------------------------*/

/* SVGA C based routines */

ibool 	_ASMAPI SVGA8_detect(int id,int force,int *driver,int *mode,modetab availableModes);
ibool 	_ASMAPI SVGA8_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	SVGA8_initInternal(MGLDC *dc);

/* SVGA assembler based routines */

void    _ASMAPI SVGA8_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI SVGA8_scanLine(int y,int x1,int x2);
void    _ASMAPI SVGA8_putMonoImage(MGLDC *dc, int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI SVGA8_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
color_t _ASMAPI SVGA8_getPixel(int x,int y);
void 	_ASMAPI SVGA8_putPixel(int x,int y);
void 	_ASMAPI SVGA8_drawScanList(int y,int length,short *scans);
void	_ASMAPI SVGA8_fillRect(int x1,int y1,int x2,int y2);
int     _ASMAPI SVGA8_scanRightForColor(int x,int y,color_t color);
int     _ASMAPI SVGA8_scanLeftForColor(int x,int y,color_t color);
int     _ASMAPI SVGA8_scanRightWhileColor(int x,int y,color_t color);
int     _ASMAPI SVGA8_scanLeftWhileColor(int x,int y,color_t color);
void	_ASMAPI SVGA8_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI SVGA8_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI SVGA8_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer);
void	_ASMAPI SVGA8_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts);
void	_ASMAPI SVGA8_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void	_ASMAPI SVGA8_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void 	_ASMAPI SVGA8_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
long 	_ASMAPI SVGA8_divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI SVGA8_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI SVGA8_putDivot(MGLDC *dc,void *divot);

/* Dithering routines */

void 	_ASMAPI SVGA8_ditherPixel(int x,int y);

#endif	/* __DRIVERS_SVGA_SVGA8_H */
