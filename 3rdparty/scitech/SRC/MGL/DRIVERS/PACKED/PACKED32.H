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
* Description:	32 bit packed pixel memory device driver header file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_PACKED_PACKED32_H
#define	__DRIVERS_PACKED_PACKED32_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

typedef struct {
	mem_vec	v;					/* Memory device vectors and info		*/
	} PACKED32_state;

#define	PACKED32_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)DC.surface + ((long)y * DC.mi.bytesPerLine) + x*4))

#define	PACKED32_pixelAddr2(dc,x,y)	\
	 ((void*)((uchar _HUGE *)(dc)->surface + ((long)y * (dc)->mi.bytesPerLine) + x*4))

#define	PACKED32_pixelAddr3(dc,x,y,surface,bytesPerLine)	\
	 ((void*)((uchar _HUGE *)(surface) + ((long)y * (bytesPerLine)) + x*4))

#define	MGL_packColorARGB(R,G,B)										\
 (((R) & 0xFF0000L) | (((G) >> 8) & 0xFF00L) | ((B) >> 16))

#define	MGL_packColorABGR(R,G,B)										\
 (((B) & 0xFF0000L) | (((G) >> 8) & 0xFF00L) | ((R) >> 16))

#define	MGL_packColorRGBA(R,G,B)										\
 ((((R) << 8) & 0xFF000000L) | ((G) & 0xFF0000L) | (((B) >> 8) & 0xFF00))

#define	MGL_packColorBGRA(R,G,B)										\
 ((((B) << 8) & 0xFF000000L) | ((G) & 0xFF0000L) | (((R) >> 8) & 0xFF00))

#define	MGL_packRGBtoARGB(s)	\
	((ulong)((s)[0])) |			\
	((ulong)((s)[1]) << 8) |	\
	((ulong)((s)[2]) << 16)

#define	MGL_packRGBtoABGR(s)	\
	((ulong)((s)[0]) << 16) |	\
	((ulong)((s)[1]) << 8) |	\
	((ulong)((s)[2]))

#define	MGL_packRGBtoRGBA(s)	\
	((ulong)((s)[0]) << 8) |	\
	((ulong)((s)[1]) << 16) |	\
	((ulong)((s)[2]) << 24)

#define	MGL_packRGBtoBGRA(s)	\
	((ulong)((s)[0]) << 24) |	\
	((ulong)((s)[1]) << 16) |	\
	((ulong)((s)[2]) << 8)

/*------------------------- Function Prototypes ---------------------------*/

/* 32 bit C based routines */

void 	PACKED32_initInternal(MGLDC *dc);
ibool 	_ASMAPI PACKED32_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	_ASMAPI PACKED32_setColor(color_t color);
void 	_ASMAPI PACKED32_setBackColor(color_t color);

/* 32 bit optimized packed pixel routines */

void    _ASMAPI PACKED32_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI PACKED32_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI PACKED32_solidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED32_ropSolidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED32_pattScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED32_colorPattScanLine(int y,int x1,int x2);
void	_ASMAPI PACKED32_ellipse(int left,int top,int A,int B,int clip);
color_t _ASMAPI PACKED32_getPixel(int x,int y);
void 	_ASMAPI PACKED32_putPixel(int x,int y);
void	_ASMAPI PACKED32_solidFillRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI PACKED32_solidDrawScanList(int y,int length,short *scans);
int     _ASMAPI PACKED32_scanRightForColor(int x,int y,color_t color);
int     _ASMAPI PACKED32_scanLeftForColor(int x,int y,color_t color);
int     _ASMAPI PACKED32_scanRightWhileColor(int x,int y,color_t color);
int     _ASMAPI PACKED32_scanLeftWhileColor(int x,int y,color_t color);
void    _ASMAPI PACKED32_putMonoImage(MGLDC *dc, int x,int y,int byteWidth,int height,uchar *image);
void	_ASMAPI PACKED32_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI PACKED32_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void 	_ASMAPI PACKED32_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED32_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED32_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED32_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI PACKED32_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void	_ASMAPI PACKED32_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer);
void	_ASMAPI PACKED32_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts);
void	_ASMAPI PACKED32_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void	_ASMAPI PACKED32_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void 	_ASMAPI PACKED32_getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI PACKED32_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void	_ASMAPI PACKED32_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
long 	_ASMAPI PACKED32_divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI PACKED32_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI PACKED32_putDivot(MGLDC *dc,void *divot);
void	_ASMAPI PACKED32_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI PACKED32ARGB_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI PACKED32ABGR_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI PACKED32RGBA_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI PACKED32BGRA_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void    _ASMAPI PACKED32_solidTrap(void);
void	_ASMAPI PACKED32_cTrap(void);
void	_ASMAPI PACKED32ARGB_rgbTrap(void);
void	_ASMAPI PACKED32ABGR_rgbTrap(void);
void	_ASMAPI PACKED32RGBA_rgbTrap(void);
void	_ASMAPI PACKED32BGRA_rgbTrap(void);
void	_ASMAPI PACKED32_zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED32_zLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED32_czLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED32_czLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED32ARGB_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32ARGB_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32ABGR_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32ABGR_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32RGBA_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32RGBA_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32BGRA_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32BGRA_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED32_zTrap16(void);
void	_ASMAPI PACKED32_zTrap32(void);
void 	_ASMAPI PACKED32_czTrap16(void);
void 	_ASMAPI PACKED32_czTrap32(void);
void 	_ASMAPI PACKED32ARGB_rgbzTrap16(void);
void 	_ASMAPI PACKED32ARGB_rgbzTrap32(void);
void 	_ASMAPI PACKED32ABGR_rgbzTrap16(void);
void 	_ASMAPI PACKED32ABGR_rgbzTrap32(void);
void 	_ASMAPI PACKED32RGBA_rgbzTrap16(void);
void 	_ASMAPI PACKED32RGBA_rgbzTrap32(void);
void 	_ASMAPI PACKED32BGRA_rgbzTrap16(void);
void 	_ASMAPI PACKED32BGRA_rgbzTrap32(void);

/* Internal helper functions */

void 	_ASMAPI PACKED32_stretchBlt1x2_24(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop);
void 	_ASMAPI PACKED32_stretchBlt2x2_24(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop);
#ifndef	NO_ASSEMBLER
void 	_ASMAPI PACKED32_stretchScan(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED32_stretchScanARGB(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED32_stretchScanABGR(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED32_stretchScanRGBA(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED32_stretchScanBGRA(void *d,void *s,int xcount,int *repCounts);
#endif

#endif	/* __DRIVERS_PACKED_PACKED32_H */
