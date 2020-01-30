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
* Description:	Common header file for Graphics Accelerator routines used
*				by all accelerated device drivers.
*
*
****************************************************************************/

#ifndef	__DRIVERS_COMMON_GACCEL_H
#define	__DRIVERS_COMMON_GACCEL_H

#ifndef	__DRIVERS_COMMON_SVGA_H
#include "drivers\common\svga.h"
#endif

#ifndef	__VBEAF_H
#include "vbeaf.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)				/* Pack structures to byte granularity		*/

/* The following defines the internal ACCEL device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	display_vec	v;				/* Display device vectors and info		*/
	short		oldBIOSMode;	/* Old video mode before graphics		*/
	ibool		old50Lines;		/* 1 if old mode was 43/50 line mode	*/
	ibool		fastDrawing;	/* True if fast drawing is enabled		*/
	} ACCEL_state;

#pragma pack()

/*--------------------------- Global Variables ----------------------------*/

extern	AF_devCtx   * _VARAPI _MGL_afPtr;/* Pointer to loaded driver	*/

#define	AF		_MGL_afPtr

/*------------------------- Function Prototypes ---------------------------*/

ibool	MGLAPI ACCEL_detect(int *driver,char *driverId);
void 	MGLAPI ACCEL_useMode(modetab modes,int mode,int id,gmode_t *modeInfo);
ibool 	MGLAPI ACCEL_setMode(MGLDC *dc,AF_int16 mode,short *oldBIOSMode,ibool *old50Lines,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void	_ASMAPI ACCEL_startStereo(MGLDC *dc);
void	_ASMAPI ACCEL_stopStereo(MGLDC *dc);
void 	_ASMAPI ACCEL_restoreTextMode(MGLDC *dc);
ibool 	_ASMAPI ACCEL_restoreGraphMode(MGLDC *dc);
void 	_ASMAPI ACCEL_setActivePage(MGLDC *dc,int page);
void 	_ASMAPI ACCEL_setVisualPage(MGLDC *dc,int page,int waitVRT);
void 	_ASMAPI ACCEL_setDisplayStart(MGLDC *dc,int x,int y,int waitFlag);
void 	_ASMAPI ACCEL_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);
void 	_ASMAPI ACCEL_vSync(MGLDC *dc);
void	_ASMAPI ACCEL_setCursor(cursor_t *curs);
void    _ASMAPI ACCEL_setCursorPos(int x,int y);
void    _ASMAPI ACCEL_showCursor(ibool visible);
void 	_ASMAPI ACCEL_setCursorColor(MGLDC *dc,color_t cursorColor);
ibool	_ASMAPI ACCEL_makeOffscreenDC(MGLDC *dc);
ibool	_ASMAPI ACCEL_makeLinearOffscreenDC(MGLDC *dc);

/* Hardware device driver support routines */

void 	_ASMAPI ACCEL_setWriteMode(int mode);
void 	_ASMAPI ACCEL_setPenStyle(int style);
void 	_ASMAPI ACCEL_setPenBitmapPattern(const pattern_t *pat);
void 	_ASMAPI ACCEL_setPenPixmapPattern(const pixpattern_t *pat);
void	_ASMAPI ACCEL_beginDirectAccess(void);
void	_ASMAPI ACCEL_endDirectAccess(void);
void	_ASMAPI ACCEL_beginDirectAccessSLOW(void);
void	_ASMAPI ACCEL_endDirectAccessSLOW(void);
void	_ASMAPI ACCEL_beginPixel(MGLDC *dc);
void	_ASMAPI ACCEL_endPixel(MGLDC *dc);
void	_ASMAPI ACCEL_beginPixelSLOW(MGLDC *dc);
void	_ASMAPI ACCEL_endPixelSLOW(MGLDC *dc);

/* Hardware rendering routines */

void 	_ASMAPI ACCEL_setClipRect(MGLDC *dc,int left,int top,int right,int bottom);
void    _ASMAPI ACCEL_scanLine(int y,int x1,int x2);
void    _ASMAPI ACCEL_pattScanLine(int y,int x1,int x2);
void    _ASMAPI ACCEL_colorPattScanLine(int y,int x1,int x2);
void	_ASMAPI ACCEL_fillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACCEL_fillPattRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACCEL_fillColorPattRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI ACCEL_drawScanList(int y,int length,short *scans);
void 	_ASMAPI ACCEL_drawPattScanList(int y,int length,short *scans);
void 	_ASMAPI ACCEL_drawColorPattScanList(int y,int length,short *scans);
void    _ASMAPI ACCEL_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI ACCEL_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
void 	_ASMAPI ACCEL_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI ACCEL_bitBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op);
void 	_ASMAPI ACCEL_bitBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op);
void 	_ASMAPI ACCEL_srcTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI ACCEL_srcTransBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op,color_t transparent);
void 	_ASMAPI ACCEL_dstTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI ACCEL_dstTransBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op,color_t transparent);
void    _ASMAPI ACCEL_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI ACCEL_setLineStipple(ushort stipple);
void 	_ASMAPI ACCEL_setLineStippleCount(uint count);
void 	_ASMAPI ACCEL_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACCEL_trap(void);
void	_ASMAPI ACCEL_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACCEL_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);

/* Offscreen memory hardware rendering routines */

void 	_ASMAPI ACCEL_OFF_setClipRect(MGLDC *dc,int left,int top,int right,int bottom);
void    _ASMAPI ACCEL_OFF_scanLine(int y,int x1,int x2);
void    _ASMAPI ACCEL_OFF_pattScanLine(int y,int x1,int x2);
void    _ASMAPI ACCEL_OFF_colorPattScanLine(int y,int x1,int x2);
void	_ASMAPI ACCEL_OFF_fillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACCEL_OFF_fillPattRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACCEL_OFF_fillColorPattRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI ACCEL_OFF_drawScanList(int y,int length,short *scans);
void 	_ASMAPI ACCEL_OFF_drawPattScanList(int y,int length,short *scans);
void 	_ASMAPI ACCEL_OFF_drawColorPattScanList(int y,int length,short *scans);
void    _ASMAPI ACCEL_OFF_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void 	_ASMAPI ACCEL_OFF_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void    _ASMAPI ACCEL_OFF_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI ACCEL_OFF_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACCEL_OFF_trap(void);
void	_ASMAPI ACCEL_OFF_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACCEL_OFF_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);

/* Fast stubs to wait for direct access to video memory before drawing */

color_t	_ASMAPI ACSTUB_getPixel(int x,int y);
void    _ASMAPI ACSTUB_putPixel(int x,int y);
void    _ASMAPI ACSTUB_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI ACSTUB_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
void	_ASMAPI ACSTUB_getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI ACSTUB_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void 	_ASMAPI ACSTUB_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI ACSTUB_putDivot(MGLDC *dc,void *divot);
void 	_ASMAPI ACSTUB_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI ACSTUB_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI ACSTUB_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool translate);

void    _ASMAPI ACSTUB_solidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_ropSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_pattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_colorPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatRopSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatColorPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_ditherLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);

void    _ASMAPI ACSTUB_solidScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_ropSolidScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_pattScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_colorPattScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatSolidScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatRopSolidScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatPattScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatColorPattScanLine(int y,int x1,int x2);
void    _ASMAPI ACSTUB_ditherScanLine(int y,int x1,int x2);

void	_ASMAPI ACSTUB_solidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_ropSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_pattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_colorPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatRopSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatColorPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_ditherFillRect(int x1,int y1,int x2,int y2);

void    _ASMAPI ACSTUB_solidDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_ropSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_pattDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_colorPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatRopSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatColorPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI ACSTUB_ditherDrawScanList(int y,int length,short *scans);

void    _ASMAPI ACSTUB_solidTrap(void);
void    _ASMAPI ACSTUB_ropSolidTrap(void);
void    _ASMAPI ACSTUB_pattTrap(void);
void    _ASMAPI ACSTUB_colorPattTrap(void);
void    _ASMAPI ACSTUB_fatSolidTrap(void);
void    _ASMAPI ACSTUB_fatRopSolidTrap(void);
void    _ASMAPI ACSTUB_fatPattTrap(void);
void    _ASMAPI ACSTUB_fatColorPattTrap(void);
void    _ASMAPI ACSTUB_ditherTrap(void);

void    _ASMAPI ACSTUB_solidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_ropSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_pattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_colorPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatRopSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatColorPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_ditherDrawRegion(int x,int y,const region_t *r);

void    _ASMAPI ACSTUB_solidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_ropSolidEllipse(int left,int top,int A,int B,int clip);
void	_ASMAPI ACSTUB_pattEllipse(int left,int top,int A,int B,int clip);
void	_ASMAPI ACSTUB_colorPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatSolidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatRopSolidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatColorPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_ditherEllipse(int left,int top,int A,int B,int clip);

void    _ASMAPI ACSTUB_solidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_ropSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_pattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_colorPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatRopSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatColorPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_ditherFillEllipse(int left,int top, int A, int B,int clip);

void    _ASMAPI ACSTUB_solidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ropSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_pattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_colorPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatRopSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatColorPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ditherEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI ACSTUB_solidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ropSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_pattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_colorPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatRopSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatColorPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ditherFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI ACSTUB_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void	_ASMAPI ACSTUB_drawStrBitmap(int x,int y,const char *str);
void	_ASMAPI ACSTUB_drawCharVec(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy);
void	_ASMAPI ACSTUB_complexPolygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_polygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_ditherPolygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
void 	_ASMAPI ACSTUB_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI ACSTUB_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI ACSTUB_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
int  	_ASMAPI ACSTUB_scanRightForColor(int x,int y,color_t color);
int   	_ASMAPI ACSTUB_scanLeftForColor(int x,int y,color_t color);
int   	_ASMAPI ACSTUB_scanRightWhileColor(int x,int y,color_t color);
int   	_ASMAPI ACSTUB_scanLeftWhileColor(int x,int y,color_t color);

void	_ASMAPI ACSTUB_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI ACSTUB_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI ACSTUB_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_ditherTri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_ditherQuad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_cTrap(void);
void	_ASMAPI ACSTUB_rgbTrap(void);

void	_ASMAPI ACSTUB_z16_zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z16_zDitherLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z16_czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI ACSTUB_z16_rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI ACSTUB_z16_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zDitherTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zDitherQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zTrap(void);
void	_ASMAPI ACSTUB_z16_zDitherTrap(void);
void 	_ASMAPI ACSTUB_z16_czTrap(void);
void 	_ASMAPI ACSTUB_z16_rgbzTrap(void);

void	_ASMAPI ACSTUB_z32_zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z32_zDitherLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z32_czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI ACSTUB_z32_rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI ACSTUB_z32_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zDitherTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zDitherQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zTrap(void);
void	_ASMAPI ACSTUB_z32_zDitherTrap(void);
void 	_ASMAPI ACSTUB_z32_czTrap(void);
void 	_ASMAPI ACSTUB_z32_rgbzTrap(void);

/* Slower stubs to call EnableDirectAccess/DisableDirectAccess between
 * direct rendering to video memory.
 */

color_t	_ASMAPI ACSTUB_getPixelSLOW(int x,int y);
void    _ASMAPI ACSTUB_putPixelSLOW(int x,int y);
void    _ASMAPI ACSTUB_putMonoImageSLOW(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI ACSTUB_putMouseImageSLOW(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
void	_ASMAPI ACSTUB_getImageSLOW(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI ACSTUB_putImageSLOW(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void 	_ASMAPI ACSTUB_getDivotSLOW(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI ACSTUB_putDivotSLOW(MGLDC *dc,void *divot);
void 	_ASMAPI ACSTUB_stretchBlt1x2SLOW(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI ACSTUB_stretchBlt2x2SLOW(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI ACSTUB_stretchBltSLOW(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool translate);

void    _ASMAPI ACSTUB_solidLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_ropSolidLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_pattLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_colorPattLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatSolidLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatRopSolidLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatPattLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_fatColorPattLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI ACSTUB_ditherLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);

void    _ASMAPI ACSTUB_solidScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_ropSolidScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_pattScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_colorPattScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatSolidScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatRopSolidScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatPattScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_fatColorPattScanLineSLOW(int y,int x1,int x2);
void    _ASMAPI ACSTUB_ditherScanLineSLOW(int y,int x1,int x2);

void	_ASMAPI ACSTUB_solidFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_ropSolidFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_pattFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_colorPattFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatSolidFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatRopSolidFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatPattFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_fatColorPattFillRectSLOW(int x1,int y1,int x2,int y2);
void	_ASMAPI ACSTUB_ditherFillRectSLOW(int x1,int y1,int x2,int y2);

void    _ASMAPI ACSTUB_solidDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_ropSolidDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_pattDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_colorPattDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatSolidDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatRopSolidDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatPattDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_fatColorPattDrawScanListSLOW(int y,int length,short *scans);
void    _ASMAPI ACSTUB_ditherDrawScanListSLOW(int y,int length,short *scans);

void    _ASMAPI ACSTUB_solidTrapSLOW(void);
void    _ASMAPI ACSTUB_ropSolidTrapSLOW(void);
void    _ASMAPI ACSTUB_pattTrapSLOW(void);
void    _ASMAPI ACSTUB_colorPattTrapSLOW(void);
void    _ASMAPI ACSTUB_fatSolidTrapSLOW(void);
void    _ASMAPI ACSTUB_fatRopSolidTrapSLOW(void);
void    _ASMAPI ACSTUB_fatPattTrapSLOW(void);
void    _ASMAPI ACSTUB_fatColorPattTrapSLOW(void);
void    _ASMAPI ACSTUB_ditherTrapSLOW(void);

void    _ASMAPI ACSTUB_solidDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_ropSolidDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_pattDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_colorPattDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatSolidDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatRopSolidDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatPattDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_fatColorPattDrawRegionSLOW(int x,int y,const region_t *r);
void    _ASMAPI ACSTUB_ditherDrawRegionSLOW(int x,int y,const region_t *r);

void    _ASMAPI ACSTUB_solidEllipseSLOW(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_ropSolidEllipseSLOW(int left,int top,int A,int B,int clip);
void	_ASMAPI ACSTUB_pattEllipseSLOW(int left,int top,int A,int B,int clip);
void	_ASMAPI ACSTUB_colorPattEllipseSLOW(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatSolidEllipseSLOW(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatRopSolidEllipseSLOW(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatPattEllipseSLOW(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_fatColorPattEllipseSLOW(int left,int top,int A,int B,int clip);
void    _ASMAPI ACSTUB_ditherEllipseSLOW(int left,int top,int A,int B,int clip);

void    _ASMAPI ACSTUB_solidFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_ropSolidFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_pattFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_colorPattFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatSolidFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatRopSolidFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatPattFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_fatColorPattFillEllipseSLOW(int left,int top, int A, int B,int clip);
void    _ASMAPI ACSTUB_ditherFillEllipseSLOW(int left,int top, int A, int B,int clip);

void    _ASMAPI ACSTUB_solidEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ropSolidEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_pattEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_colorPattEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatSolidEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatRopSolidEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatPattEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatColorPattEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ditherEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI ACSTUB_solidFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ropSolidFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_pattFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_colorPattFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatSolidFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatRopSolidFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatPattFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_fatColorPattFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI ACSTUB_ditherFillEllipseArcSLOW(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI ACSTUB_stippleLineSLOW(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void	_ASMAPI ACSTUB_drawStrBitmapSLOW(int x,int y,const char *str);
void	_ASMAPI ACSTUB_drawCharVecSLOW(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy);
void	_ASMAPI ACSTUB_complexPolygonSLOW(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_polygonSLOW(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_ditherPolygonSLOW(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_translateImageSLOW(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
void 	_ASMAPI ACSTUB_bitBltSLOW(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI ACSTUB_srcTransBltSLOW(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI ACSTUB_dstTransBltSLOW(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
int  	_ASMAPI ACSTUB_scanRightForColorSLOW(int x,int y,color_t color);
int   	_ASMAPI ACSTUB_scanLeftForColorSLOW(int x,int y,color_t color);
int   	_ASMAPI ACSTUB_scanRightWhileColorSLOW(int x,int y,color_t color);
int   	_ASMAPI ACSTUB_scanLeftWhileColorSLOW(int x,int y,color_t color);

void	_ASMAPI ACSTUB_cLineSLOW(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI ACSTUB_rgbLineSLOW(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI ACSTUB_triSLOW(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_ditherTriSLOW(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_cTriSLOW(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_rgbTriSLOW(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_quadSLOW(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_ditherQuadSLOW(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_cQuadSLOW(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_rgbQuadSLOW(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI ACSTUB_cTrapSLOW(void);
void	_ASMAPI ACSTUB_rgbTrapSLOW(void);

void	_ASMAPI ACSTUB_z16_zLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z16_zDitherLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z16_czLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI ACSTUB_z16_rgbzLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI ACSTUB_z16_zTriSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zDitherTriSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_czTriSLOW(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_rgbzTriSLOW(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zQuadSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zDitherQuadSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_czQuadSLOW(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_rgbzQuadSLOW(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z16_zTrapSLOW(void);
void	_ASMAPI ACSTUB_z16_zDitherTrapSLOW(void);
void 	_ASMAPI ACSTUB_z16_czTrapSLOW(void);
void 	_ASMAPI ACSTUB_z16_rgbzTrapSLOW(void);

void	_ASMAPI ACSTUB_z32_zLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z32_zDitherLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI ACSTUB_z32_czLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI ACSTUB_z32_rgbzLineSLOW(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI ACSTUB_z32_zTriSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zDitherTriSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_czTriSLOW(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_rgbzTriSLOW(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zQuadSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zDitherQuadSLOW(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_czQuadSLOW(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_rgbzQuadSLOW(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI ACSTUB_z32_zTrapSLOW(void);
void	_ASMAPI ACSTUB_z32_zDitherTrapSLOW(void);
void 	_ASMAPI ACSTUB_z32_czTrapSLOW(void);
void 	_ASMAPI ACSTUB_z32_rgbzTrapSLOW(void);

#endif	/* __DRIVERS_COMMON_GACCEL_H */
