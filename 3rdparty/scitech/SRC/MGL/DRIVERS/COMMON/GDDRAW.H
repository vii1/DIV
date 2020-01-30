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
* Description:	Common header file to all DirectDraw display drivers.
*
*               This module interfaces with DirectDraw and handles all of
*				the common DirectDraw specific code for the DirectDraw
*				drivers.
*
*
****************************************************************************/

#ifndef	__DRIVERS_COMMON_GDDRAW_H
#define	__DRIVERS_COMMON_GDDRAW_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)				/* Pack structures to byte granularity		*/

/* The following defines the internal DDRAW device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	display_vec	v;				/* Display device vectors and info		*/
	} DDRAW_state;

#pragma pack()

/*------------------------- Function Prototypes ---------------------------*/

ibool	MGLAPI DDRAW_detect(int *driver,char *driverId);
void 	MGLAPI DDRAW_useMode(modetab modes,int mode,int id,gmode_t *modeInfo);
ibool 	MGLAPI DDRAW_setMode(MGLDC *dc,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void	MGLAPI DDRAW_setAccelVectors(MGLDC *dc);
void 	_ASMAPI DDRAW_restoreTextMode(MGLDC *dc);
void 	_ASMAPI DDRAW_setActivePage(MGLDC *dc,int page);
void 	_ASMAPI DDRAW_setVisualPage(MGLDC *dc,int page,int waitVRT);
void 	_ASMAPI DDRAW_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);
void 	_ASMAPI DDRAW_vSync(MGLDC *dc);
ibool	_ASMAPI DDRAW_makeOffscreenDC(MGLDC *dc);
ibool	_ASMAPI DDRAW_makeLinearOffscreenDC(MGLDC *dc);
void 	* _ASMAPI DDRAW_getDefaultPalette(MGLDC *dc);

/* Fullscreen hardware OpenGL interfacing */

ibool 	MGLAPI DDOPENGL_detect(void);
void 	MGLAPI DDOPENGL_useMode(modetab modes,int mode,int id,gmode_t *modeInfo);

/* Hardware device driver support routines */

void	_ASMAPI DDRAW_beginDrawing(void);
void	_ASMAPI DDRAW_endDrawing(void);
void	_ASMAPI DDRAW_beginDirectAccess(void);
void	_ASMAPI DDRAW_endDirectAccess(void);
void	_ASMAPI DDRAW_beginPixel(MGLDC *dc);
void	_ASMAPI DDRAW_endPixel(MGLDC *dc);

/* Hardware rendering routines */

void	_ASMAPI DDRAW_fillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDRAW_ropFillRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI DDRAW_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI DDRAW_bitBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op);
void 	_ASMAPI DDRAW_srcTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI DDRAW_dstTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);

/* Offscreen memory hardware rendering routines */

void	_ASMAPI DDRAW_OFF_fillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDRAW_OFF_ropFillRect(int x1,int y1,int x2,int y2);
void 	_ASMAPI DDRAW_OFF_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);

/* Stubs to do lock/unlock calls while drawing to linear framebuffer */

void 	_ASMAPI DDRAW_lock(void);
void 	_ASMAPI DDRAW_unlock(void);

color_t	_ASMAPI DDSTUB_getPixel(int x,int y);
void    _ASMAPI DDSTUB_putPixel(int x,int y);
void    _ASMAPI DDSTUB_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI DDSTUB_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
void	_ASMAPI DDSTUB_getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI DDSTUB_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void 	_ASMAPI DDSTUB_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI DDSTUB_putDivot(MGLDC *dc,void *divot);
void 	_ASMAPI DDSTUB_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI DDSTUB_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI DDSTUB_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool translate);

void    _ASMAPI DDSTUB_solidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_ropSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_pattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_colorPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_fatSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_fatRopSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_fatPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_fatColorPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI DDSTUB_ditherLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);

void    _ASMAPI DDSTUB_solidScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_ropSolidScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_pattScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_colorPattScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_fatSolidScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_fatRopSolidScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_fatPattScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_fatColorPattScanLine(int y,int x1,int x2);
void    _ASMAPI DDSTUB_ditherScanLine(int y,int x1,int x2);

void	_ASMAPI DDSTUB_solidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_ropSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_pattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_colorPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_fatSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_fatRopSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_fatPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_fatColorPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI DDSTUB_ditherFillRect(int x1,int y1,int x2,int y2);

void    _ASMAPI DDSTUB_solidDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_ropSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_pattDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_colorPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_fatSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_fatRopSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_fatPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_fatColorPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI DDSTUB_ditherDrawScanList(int y,int length,short *scans);

void    _ASMAPI DDSTUB_solidTrap(void);
void    _ASMAPI DDSTUB_ropSolidTrap(void);
void    _ASMAPI DDSTUB_pattTrap(void);
void    _ASMAPI DDSTUB_colorPattTrap(void);
void    _ASMAPI DDSTUB_fatSolidTrap(void);
void    _ASMAPI DDSTUB_fatRopSolidTrap(void);
void    _ASMAPI DDSTUB_fatPattTrap(void);
void    _ASMAPI DDSTUB_fatColorPattTrap(void);
void    _ASMAPI DDSTUB_ditherTrap(void);

void    _ASMAPI DDSTUB_solidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_ropSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_pattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_colorPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_fatSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_fatRopSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_fatPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_fatColorPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI DDSTUB_ditherDrawRegion(int x,int y,const region_t *r);

void    _ASMAPI DDSTUB_solidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI DDSTUB_ropSolidEllipse(int left,int top,int A,int B,int clip);
void	_ASMAPI DDSTUB_pattEllipse(int left,int top,int A,int B,int clip);
void	_ASMAPI DDSTUB_colorPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI DDSTUB_fatSolidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI DDSTUB_fatRopSolidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI DDSTUB_fatPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI DDSTUB_fatColorPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI DDSTUB_ditherEllipse(int left,int top,int A,int B,int clip);

void    _ASMAPI DDSTUB_solidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_ropSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_pattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_colorPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_fatSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_fatRopSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_fatPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_fatColorPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI DDSTUB_ditherFillEllipse(int left,int top, int A, int B,int clip);

void    _ASMAPI DDSTUB_solidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_ropSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_pattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_colorPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatRopSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatColorPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_ditherEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI DDSTUB_solidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_ropSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_pattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_colorPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatRopSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_fatColorPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI DDSTUB_ditherFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI DDSTUB_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void	_ASMAPI DDSTUB_drawStrBitmap(int x,int y,const char *str);
void	_ASMAPI DDSTUB_drawCharVec(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy);
void	_ASMAPI DDSTUB_complexPolygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_polygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_ditherPolygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
void 	_ASMAPI DDSTUB_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI DDSTUB_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI DDSTUB_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
int  	_ASMAPI DDSTUB_scanRightForColor(int x,int y,color_t color);
int   	_ASMAPI DDSTUB_scanLeftForColor(int x,int y,color_t color);
int   	_ASMAPI DDSTUB_scanRightWhileColor(int x,int y,color_t color);
int   	_ASMAPI DDSTUB_scanLeftWhileColor(int x,int y,color_t color);

void	_ASMAPI DDSTUB_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI DDSTUB_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI DDSTUB_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_ditherTri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_ditherQuad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI DDSTUB_cTrap(void);
void	_ASMAPI DDSTUB_rgbTrap(void);

void	_ASMAPI DDSTUB_z16_zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI DDSTUB_z16_zDitherLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI DDSTUB_z16_czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI DDSTUB_z16_rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI DDSTUB_z16_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_zDitherTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_zDitherQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z16_zTrap(void);
void	_ASMAPI DDSTUB_z16_zDitherTrap(void);
void 	_ASMAPI DDSTUB_z16_czTrap(void);
void 	_ASMAPI DDSTUB_z16_rgbzTrap(void);

void	_ASMAPI DDSTUB_z32_zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI DDSTUB_z32_zDitherLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI DDSTUB_z32_czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI DDSTUB_z32_rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI DDSTUB_z32_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_zDitherTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_zDitherQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI DDSTUB_z32_zTrap(void);
void	_ASMAPI DDSTUB_z32_zDitherTrap(void);
void 	_ASMAPI DDSTUB_z32_czTrap(void);
void 	_ASMAPI DDSTUB_z32_rgbzTrap(void);

#endif	/* __DRIVERS_COMMON_GDDRAW_H */

