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
* Description:	24 bit packed pixel memory device driver header file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_PACKED_PACKED24_H
#define	__DRIVERS_PACKED_PACKED24_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

typedef struct {
	mem_vec	v;					/* Memory device vectors and info		*/
	} PACKED24_state;

#define	PACKED24_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)DC.surface + ((long)y * DC.mi.bytesPerLine) + x*3))

#define	PACKED24_pixelAddr2(dc,x,y)	\
	 ((void*)((uchar _HUGE *)(dc)->surface + ((long)y * (dc)->mi.bytesPerLine) + x*3))

#define	PACKED24_pixelAddr3(dc,x,y,surface,bytesPerLine)	\
	 ((void*)((uchar _HUGE *)(surface) + ((long)y * (bytesPerLine)) + x*3))

/*------------------------- Function Prototypes ---------------------------*/

/* 24 bit C based routines */

void 	PACKED24_initInternal(MGLDC *dc);
ibool 	_ASMAPI PACKED24_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	_ASMAPI PACKED24_setColor(color_t color);
void 	_ASMAPI PACKED24_setBackColor(color_t color);

/* 24 bit optimized packed pixel routines */

void    _ASMAPI PACKED24_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI PACKED24_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI PACKED24_solidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED24_ropSolidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED24_pattScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED24_colorPattScanLine(int y,int x1,int x2);
void	_ASMAPI PACKED24_ellipse(int left,int top,int A,int B,int clip);
color_t _ASMAPI PACKED24_getPixel(int x,int y);
void 	_ASMAPI PACKED24_putPixel(int x,int y);
void	_ASMAPI PACKED24_solidFillRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI PACKED24_solidDrawScanList(int y,int length,short *scans);
int     _ASMAPI PACKED24_scanRightForColor(int x,int y,color_t color);
int     _ASMAPI PACKED24_scanLeftForColor(int x,int y,color_t color);
int     _ASMAPI PACKED24_scanRightWhileColor(int x,int y,color_t color);
int     _ASMAPI PACKED24_scanLeftWhileColor(int x,int y,color_t color);
void    _ASMAPI PACKED24_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void	_ASMAPI PACKED24_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI PACKED24_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void 	_ASMAPI PACKED24_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED24_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED24_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED24_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI PACKED24_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void	_ASMAPI PACKED24_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer);
void	_ASMAPI PACKED24_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts);
void	_ASMAPI PACKED24_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void	_ASMAPI PACKED24_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void 	_ASMAPI PACKED24_getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI PACKED24_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void	_ASMAPI PACKED24_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
long 	_ASMAPI PACKED24_divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI PACKED24_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI PACKED24_putDivot(MGLDC *dc,void *divot);
void	_ASMAPI PACKED24_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI PACKED24RGB_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI PACKED24BGR_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void    _ASMAPI PACKED24_solidTrap(void);
void	_ASMAPI PACKED24_cTrap(void);
void	_ASMAPI PACKED24RGB_rgbTrap(void);
void	_ASMAPI PACKED24BGR_rgbTrap(void);
void	_ASMAPI PACKED24_zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED24_zLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED24_czLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED24_czLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED24RGB_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED24RGB_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED24BGR_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED24BGR_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED24_zTrap16(void);
void	_ASMAPI PACKED24_zTrap32(void);
void 	_ASMAPI PACKED24_czTrap16(void);
void 	_ASMAPI PACKED24_czTrap32(void);
void 	_ASMAPI PACKED24RGB_rgbzTrap16(void);
void 	_ASMAPI PACKED24RGB_rgbzTrap32(void);
void 	_ASMAPI PACKED24BGR_rgbzTrap16(void);
void 	_ASMAPI PACKED24BGR_rgbzTrap32(void);

/* Internal helper functions */

#ifndef	NO_ASSEMBLER
void 	_ASMAPI PACKED24_stretchScan2x(void *d,void *s,int xcount);
void 	_ASMAPI PACKED24_stretchScanBGR2x(void *d,void *s,int xcount);
void 	_ASMAPI PACKED24_stretchScan(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED24_stretchScanBGR(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED24_srcTransScan(void *d,void *s,int xcount,color_t transparent);
void 	_ASMAPI PACKED24_dstTransScan(void *d,void *s,int xcount,color_t transparent);
void 	_ASMAPI PACKED24_translateScanBGR(void *d,void *s,int xcount);
#endif

#endif	/* __DRIVERS_PACKED_PACKED24_H */
