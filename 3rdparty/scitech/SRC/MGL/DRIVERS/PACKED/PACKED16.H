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
* Description:	16 bit packed pixel memory device driver header file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_PACKED_PACKED16_H
#define	__DRIVERS_PACKED_PACKED16_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

typedef struct {
	mem_vec	v;					/* Memory device vectors and info		*/
	} PACKED16_state;

#define	PACKED16_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)DC.surface + ((long)y * DC.mi.bytesPerLine) + x*2))

#define	PACKED16_pixelAddr2(dc,x,y)	\
	 ((void*)((uchar _HUGE *)(dc)->surface + ((long)y * (dc)->mi.bytesPerLine) + x*2))

#define	PACKED16_pixelAddr3(dc,x,y,surface,bytesPerLine)	\
	 ((void*)((uchar _HUGE *)(surface) + ((long)y * (bytesPerLine)) + x*2))

#define	MGL_packColor15FX(R,G,B)										\
 (((R) >> 9) & 0x7C00) | (((G) >> 14) & 0x3E0) | (((B) >> 19) & 0x1F)

#define	MGL_packColor16FX(R,G,B)										\
 (((R) >> 8) & 0xF800) | (((G) >> 13) & 0x7E0) | (((B) >> 19) & 0x1F)

#define	MGL_pack24to15(s)				\
	((ushort)((s)[0]) >> 3) |			\
	(((ushort)((s)[1]) << 2) & 0x3E0) |	\
	(((ushort)((s)[2]) << 7) & 0x7C00)

#define	MGL_pack24to16(s)				\
	((ushort)((s)[0]) >> 3) |			\
	(((ushort)((s)[1]) << 3) & 0x7E0) |	\
	(((ushort)((s)[2]) << 8) & 0xF800)

/*------------------------- Function Prototypes ---------------------------*/

/* 16 bit C based routines */

void 	PACKED16_initInternal(MGLDC *dc);
ibool 	_ASMAPI PACKED16_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	_ASMAPI PACKED16_setColor(color_t color);
void 	_ASMAPI PACKED16_setBackColor(color_t color);

/* 16 bit optimized packed pixel routines */

void    _ASMAPI PACKED16_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI PACKED16_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI PACKED16_solidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED16_ropSolidScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED16_pattScanLine(int y,int x1,int x2);
void    _ASMAPI PACKED16_colorPattScanLine(int y,int x1,int x2);
void	_ASMAPI PACKED16_ellipse(int left,int top,int A,int B,int clip);
color_t _ASMAPI PACKED16_getPixel(int x,int y);
void 	_ASMAPI PACKED16_putPixel(int x,int y);
void	_ASMAPI PACKED16_solidFillRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI PACKED16_solidDrawScanList(int y,int length,short *scans);
int     _ASMAPI PACKED16_scanRightForColor(int x,int y,color_t color);
int     _ASMAPI PACKED16_scanLeftForColor(int x,int y,color_t color);
int     _ASMAPI PACKED16_scanRightWhileColor(int x,int y,color_t color);
int     _ASMAPI PACKED16_scanLeftWhileColor(int x,int y,color_t color);
void    _ASMAPI PACKED16_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void	_ASMAPI PACKED16_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI PACKED16_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void 	_ASMAPI PACKED16_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED16_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED16_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool idPal);
void 	_ASMAPI PACKED16_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI PACKED16_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void	_ASMAPI PACKED16_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer);
void	_ASMAPI PACKED16_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts);
void	_ASMAPI PACKED16_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void	_ASMAPI PACKED16_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent);
void 	_ASMAPI PACKED16_getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI PACKED16_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void	_ASMAPI PACKED16_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
long 	_ASMAPI PACKED16_divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI PACKED16_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI PACKED16_putDivot(MGLDC *dc,void *divot);
void	_ASMAPI PACKED16_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI PACKED15_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI PACKED16_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void    _ASMAPI PACKED16_solidTrap(void);
void	_ASMAPI PACKED16_cTrap(void);
void	_ASMAPI PACKED15_rgbTrap(void);
void	_ASMAPI PACKED16_rgbTrap(void);
void	_ASMAPI PACKED16_zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED16_zLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI PACKED16_czLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED16_czLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI PACKED15_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED15_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED16_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED16_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI PACKED16_zTrap16(void);
void	_ASMAPI PACKED16_zTrap32(void);
void 	_ASMAPI PACKED16_czTrap16(void);
void 	_ASMAPI PACKED16_czTrap32(void);
void 	_ASMAPI PACKED15_rgbzTrap16(void);
void 	_ASMAPI PACKED15_rgbzTrap32(void);
void 	_ASMAPI PACKED16_rgbzTrap16(void);
void 	_ASMAPI PACKED16_rgbzTrap32(void);

/* Internal helper functions */

void 	_ASMAPI PACKED16_stretchBlt1x2_24(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop);
void 	_ASMAPI PACKED16_stretchBlt2x2_24(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop);
#ifndef	NO_ASSEMBLER
void 	_ASMAPI PACKED16_stretchScan(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED15_stretchScan24(void *d,void *s,int xcount,int *repCounts);
void 	_ASMAPI PACKED16_stretchScan24(void *d,void *s,int xcount,int *repCounts);
#endif

#endif	/* __DRIVERS_PACKED_PACKED16_H */
