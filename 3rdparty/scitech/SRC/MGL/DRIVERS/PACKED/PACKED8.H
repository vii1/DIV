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
* Description:	8 bit packed pixel device driver header file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_PACKED_PACKED8_H
#define	__DRIVERS_PACKED_PACKED8_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

typedef struct {
	mem_vec	v;					/* Memory device vectors and info		*/
	} PACKED8_state;

#define	PACKED8_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)DC.surface + ((long)y * DC.mi.bytesPerLine) + x))

#define	PACKED8_pixelAddr2(dc,x,y)	\
	 ((void*)((uchar _HUGE *)(dc)->surface + ((long)y * (dc)->mi.bytesPerLine) + x))

#define	PACKED8_pixelAddr3(dc,x,y,surface,bytesPerLine)	\
	 ((void*)((uchar _HUGE *)(surface) + ((long)y * (bytesPerLine)) + x))

/*------------------------- Function Prototypes ---------------------------*/

/* 8 bit C based routines */

void 	PACKED8_initInternal(MGLDC *dc);
ibool 	_ASMAPI PACKED8_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	_ASMAPI PACKED8_setColor(color_t color);
void 	_ASMAPI PACKED8_setBackColor(color_t color);

/* 8 bit optimized packed pixel routines */

void    _ASMAPI PACKED8_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI PACKED8_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI PACKED8_solidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED8_ropSolidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED8_pattScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED8_colorPattScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED8_ditherScanLine(int y,int x1,int x2);
void	_ASMAPI PACKED8_ellipse(int left,int top,int A,int B,int clip);
color_t _ASMAPI PACKED8_getPixel(int x,int y);
void 	_ASMAPI PACKED8_putPixel(int x,int y);
void 	_ASMAPI PACKED8_ditherPixel(int x,int y);
void	_ASMAPI PACKED8_solidFillRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI PACKED8_solidDrawScanList(int y,int length,short *scans);
int     _ASMAPI PACKED8_scanRightForColor( int x,int y,color_t color);
int     _ASMAPI PACKED8_scanLeftForColor( int x,int y,color_t color);
int     _ASMAPI PACKED8_scanRightWhileColor( int x,int y,color_t color);
int     _ASMAPI PACKED8_scanLeftWhileColor( int x,int y,color_t color);
void    _ASMAPI PACKED8_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void	_ASMAPI PACKED8_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI PACKED8_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void 	_ASMAPI PACKED8_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI PACKED8_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI PACKED8_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool translate);
void 	_ASMAPI PACKED8_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI PACKED8_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void	_ASMAPI PACKED8_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer);
void	_ASMAPI PACKED8_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts);
void	_ASMAPI PACKED8_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void	_ASMAPI PACKED8_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void 	_ASMAPI PACKED8_getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI PACKED8_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void	_ASMAPI PACKED8_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
long 	_ASMAPI PACKED8_divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI PACKED8_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI PACKED8_putDivot(MGLDC *dc,void *divot);
void	_ASMAPI PACKED8_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI PACKED8_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void    _ASMAPI PACKED8_solidTrap(void);
void    _ASMAPI PACKED8_ditherTrap(void);
void	_ASMAPI PACKED8_cTrap(void);
void	_ASMAPI PACKED8_rgbTrap(void);
void	_ASMAPI PACKED8_zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED8_zLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED8_zDitherLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED8_zDitherLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED8_czLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED8_czLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED8_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED8_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED8_zTrap16(void);
void	_ASMAPI PACKED8_zTrap32(void);
void    _ASMAPI PACKED8_zDitherTrap16(void);
void    _ASMAPI PACKED8_zDitherTrap32(void);
void 	_ASMAPI PACKED8_czTrap16(void);
void 	_ASMAPI PACKED8_czTrap32(void);
void 	_ASMAPI PACKED8_rgbzTrap16(void);
void 	_ASMAPI PACKED8_rgbzTrap32(void);
void 	_ASMAPI PACKED8_ditherPixel(int x,int y);
void    _ASMAPI PACKED8_ditherLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);

/* Internal assembler helper functions */

void 	_ASMAPI PACKED8_stretchBlt2x2_24(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop);
void 	_ASMAPI PACKED8_stretchBlt1x2_24(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop);
#ifndef	NO_ASSEMBLER
void 	_ASMAPI PACKED8_stretchScan(uchar *d,uchar *s,int count,int *repCounts);
#endif

#endif	/* __DRIVERS_PACKED_PACKED8_H */
