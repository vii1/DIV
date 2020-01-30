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
* Description:	Main VGAX device driver header file. This file contains all
*				of the C language declarations and definitions for the
*				VGAX device driver file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_VGA_VGAX_H
#define	__DRIVERS_VGA_VGAX_H

#ifndef	__DRIVERS_COMMON_VGA_H
#include "drivers\common\vga.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

/* The following defines the internal VGAX device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	display_vec	v;				/* Display device vectors and info		*/
	short		oldBIOSMode;	/* Old video mode before graphics		*/
	ibool		old50Lines;		/* 1 if old mode was 50 line VGA8 mode	*/
	ibool		fastDrawing;
	} VGAX_state;

/*------------------------- Function Prototypes ---------------------------*/

/* VGAX C based routines */

ibool 	_ASMAPI VGAX_detect(int id,int force,int *driver,int *mode,modetab availableModes);
ibool 	_ASMAPI VGAX_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	_ASMAPI VGAX_restoreTextMode(MGLDC *dc);
ibool 	_ASMAPI VGAX_restoreGraphMode(MGLDC *dc);
void 	* _ASMAPI VGAX_getDefaultPalette(MGLDC *dc);
void 	VGAX_initInternal(MGLDC *dc);

/* VGAX assembler based routines */

void    _ASMAPI VGAX_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI VGAX_scanLine(int y,int x1,int x2);
void	_ASMAPI VGAX_ellipse(int left,int top,int A,int B,int clip);
void    _ASMAPI VGAX_putMonoImage(MGLDC *dc, int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI VGAX_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
void	_ASMAPI VGAX_beginPixel(MGLDC *dc);
color_t _ASMAPI VGAX_getPixel(int x,int y);
void 	_ASMAPI VGAX_putPixel(int x,int y);
void 	_ASMAPI VGAX_endPixel(MGLDC *dc);
void 	_ASMAPI VGAX_drawScanList(int y,int length,short *scans);
void	_ASMAPI VGAX_fillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI VGAX_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI VGAX_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI VGAX_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer);
void	_ASMAPI VGAX_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts);
void	_ASMAPI VGAX_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void	_ASMAPI VGAX_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void 	_ASMAPI VGAX_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI VGAX_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
long 	_ASMAPI VGAX_divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI VGAX_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI VGAX_putDivot(MGLDC *dc,void *divot);

/* Dithering routines */

void 	_ASMAPI VGAX_ditherPixel(int x,int y);

#endif	/* __DRIVERS_VGA_VGAX_H */
