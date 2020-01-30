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
*				VGA4 device driver file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_VGA_VGA4_H
#define	__DRIVERS_VGA_VGA4_H

#ifndef	__DRIVERS_COMMON_VGA_H
#include "drivers\common\vga.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

/* The following defines the internal VGA4 device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	display_vec	v;				/* Display device vectors and info		*/
	short		oldBIOSMode;	/* Old video mode before graphics		*/
	ibool		old43Lines;		/* 1 if old mode was 43 line VGA4 mode	*/
	ibool		fastDrawing;
	} VGA4_state;

/*------------------------- Function Prototypes ---------------------------*/

/* VGA C based routines */

ibool 	_ASMAPI VGA4_detect(int id,int force,int *driver,int *mode,modetab availableModes);
ibool 	_ASMAPI VGA4_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	VGA4_initMode(int mode);
void 	VGA4_initInternal(MGLDC *dc);
void 	_ASMAPI VGA4_restoreTextMode(MGLDC *dc);
ibool 	_ASMAPI VGA4_restoreGraphMode(MGLDC *dc);
void 	* _ASMAPI VGA4_getDefaultPalette(MGLDC *dc);

/* VGA assembler based routines */

void    _ASMAPI VGA4_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI VGA4_scanLine(int y,int x1,int x2);
void	_ASMAPI VGA4_ellipse(int left,int top,int A,int B,int clip);
void    _ASMAPI VGA4_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI VGA4_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
void	_ASMAPI VGA4_beginPixel(MGLDC *dc);
color_t _ASMAPI VGA4_getPixel(int x,int y);
void 	_ASMAPI VGA4_putPixel(int x,int y);
void 	_ASMAPI VGA4_endPixel(MGLDC *dc);
void 	_ASMAPI VGA4_drawScanList(int y,int length,short *scans);
void	_ASMAPI VGA4_fillRect(int x1,int y1,int x2,int y2);
int     _ASMAPI VGA4_scanRightForColor(int x,int y,color_t color);
int     _ASMAPI VGA4_scanLeftForColor(int x,int y,color_t color);
int     _ASMAPI VGA4_scanRightWhileColor(int x,int y,color_t color);
int     _ASMAPI VGA4_scanLeftWhileColor(int x,int y,color_t color);
void	_ASMAPI VGA4_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI VGA4_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void 	_ASMAPI VGA4_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI VGA4_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
long 	_ASMAPI VGA4_divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI VGA4_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI VGA4_putDivot(MGLDC *dc,void *divot);
void	_ASMAPI VGA4_alignedBitBlt(MGLDC *dc,int left8,int top,int right8,int bottom,int dstLeft8,int dstTop,int incx,int incy,int op);
void 	_ASMAPI VGA4_readImage(MGLDC *dc,int left,int top,int right,int bottom,void *buffer);
void 	_ASMAPI VGA4_writeImage(MGLDC *dc,int dstLeft,int dstTop,void *buffer,int op);

#endif	/* __DRIVERS_VGA_VGA4_H */
