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

#ifndef	__DRIVERS_COMMON_GOPENGL_H
#define	__DRIVERS_COMMON_GOPENGL_H

#ifndef	__DRIVERS_COMMON_GDDRAW_H
#include "drivers\common\gddraw.h"
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
	} FSOPENGL_state;

#pragma pack()

/*-------------------------- Global Variables -----------------------------*/

extern FGL_hwInfo	_MGL_glHWInfo;

/*------------------------- Function Prototypes ---------------------------*/

ibool	MGLAPI FSOPENGL_detect(int *driver,char *driverId,char *realname);
void 	MGLAPI FSOPENGL_useMode(modetab modes,int mode,int id,gmode_t *modeInfo);
ibool 	MGLAPI FSOPENGL_setMode(MGLDC *dc,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void 	MGLAPI FSOPENGL_setAccelVectors(MGLDC *dc);
void 	_ASMAPI FSOPENGL_restoreTextMode(MGLDC *dc);
void 	_ASMAPI FSOPENGL_setActivePage(MGLDC *dc,int page);
void 	_ASMAPI FSOPENGL_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);
void 	_ASMAPI FSOPENGL_vSync(MGLDC *dc);
void * 	_ASMAPI FSOPENGL_getDefaultPalette(MGLDC *dc);
ulong 	_ASMAPI FSOPENGL_getWinDC(MGLDC *dc);

/* Hardware device driver support routines */

void	_ASMAPI FSOPENGL_beginDirectAccess(void);
void	_ASMAPI FSOPENGL_endDirectAccess(void);
void	_ASMAPI FSOPENGL_beginPixel(MGLDC *dc);
void	_ASMAPI FSOPENGL_endPixel(MGLDC *dc);

/* Hardware rendering routines */

// TODO: Implement this using fgl calls
//void	_ASMAPI FSOPENGL_fillRect(int x1,int y1,int x2,int y2);
//void	_ASMAPI FSOPENGL_ropFillRect(int x1,int y1,int x2,int y2);
// bitBlt
// stretchBlt
// transBlt

/* Stubs to do lock/unlock calls while drawing to linear framebuffer */

void 	_ASMAPI FSOPENGL_lock(void);
void 	_ASMAPI FSOPENGL_unlock(void);

color_t	_ASMAPI OGLSTUB_getPixel(int x,int y);
void    _ASMAPI OGLSTUB_putPixel(int x,int y);
void    _ASMAPI OGLSTUB_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI OGLSTUB_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
void	_ASMAPI OGLSTUB_getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void 	_ASMAPI OGLSTUB_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src);
void 	_ASMAPI OGLSTUB_getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI OGLSTUB_putDivot(MGLDC *dc,void *divot);
void 	_ASMAPI OGLSTUB_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI OGLSTUB_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool translate);
void 	_ASMAPI OGLSTUB_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool translate);

void    _ASMAPI OGLSTUB_solidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_ropSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_pattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_colorPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_fatSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_fatRopSolidLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_fatPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_fatColorPattLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void    _ASMAPI OGLSTUB_ditherLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);

void    _ASMAPI OGLSTUB_solidScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_ropSolidScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_pattScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_colorPattScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_fatSolidScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_fatRopSolidScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_fatPattScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_fatColorPattScanLine(int y,int x1,int x2);
void    _ASMAPI OGLSTUB_ditherScanLine(int y,int x1,int x2);

void	_ASMAPI OGLSTUB_solidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_ropSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_pattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_colorPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_fatSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_fatRopSolidFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_fatPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_fatColorPattFillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI OGLSTUB_ditherFillRect(int x1,int y1,int x2,int y2);

void    _ASMAPI OGLSTUB_solidDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_ropSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_pattDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_colorPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_fatSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_fatRopSolidDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_fatPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_fatColorPattDrawScanList(int y,int length,short *scans);
void    _ASMAPI OGLSTUB_ditherDrawScanList(int y,int length,short *scans);

void    _ASMAPI OGLSTUB_solidTrap(void);
void    _ASMAPI OGLSTUB_ropSolidTrap(void);
void    _ASMAPI OGLSTUB_pattTrap(void);
void    _ASMAPI OGLSTUB_colorPattTrap(void);
void    _ASMAPI OGLSTUB_fatSolidTrap(void);
void    _ASMAPI OGLSTUB_fatRopSolidTrap(void);
void    _ASMAPI OGLSTUB_fatPattTrap(void);
void    _ASMAPI OGLSTUB_fatColorPattTrap(void);
void    _ASMAPI OGLSTUB_ditherTrap(void);

void    _ASMAPI OGLSTUB_solidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_ropSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_pattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_colorPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_fatSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_fatRopSolidDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_fatPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_fatColorPattDrawRegion(int x,int y,const region_t *r);
void    _ASMAPI OGLSTUB_ditherDrawRegion(int x,int y,const region_t *r);

void    _ASMAPI OGLSTUB_solidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI OGLSTUB_ropSolidEllipse(int left,int top,int A,int B,int clip);
void	_ASMAPI OGLSTUB_pattEllipse(int left,int top,int A,int B,int clip);
void	_ASMAPI OGLSTUB_colorPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI OGLSTUB_fatSolidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI OGLSTUB_fatRopSolidEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI OGLSTUB_fatPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI OGLSTUB_fatColorPattEllipse(int left,int top,int A,int B,int clip);
void    _ASMAPI OGLSTUB_ditherEllipse(int left,int top,int A,int B,int clip);

void    _ASMAPI OGLSTUB_solidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_ropSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_pattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_colorPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_fatSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_fatRopSolidFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_fatPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_fatColorPattFillEllipse(int left,int top, int A, int B,int clip);
void    _ASMAPI OGLSTUB_ditherFillEllipse(int left,int top, int A, int B,int clip);

void    _ASMAPI OGLSTUB_solidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_ropSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_pattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_colorPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatRopSolidEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatColorPattEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_ditherEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI OGLSTUB_solidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_ropSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_pattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_colorPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatRopSolidFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_fatColorPattFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void    _ASMAPI OGLSTUB_ditherFillEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);

void    _ASMAPI OGLSTUB_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void	_ASMAPI OGLSTUB_drawStrBitmap(int x,int y,const char *str);
void	_ASMAPI OGLSTUB_drawCharVec(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy);
void	_ASMAPI OGLSTUB_complexPolygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_polygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_ditherPolygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
void 	_ASMAPI OGLSTUB_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI OGLSTUB_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI OGLSTUB_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
int  	_ASMAPI OGLSTUB_scanRightForColor(int x,int y,color_t color);
int   	_ASMAPI OGLSTUB_scanLeftForColor(int x,int y,color_t color);
int   	_ASMAPI OGLSTUB_scanRightWhileColor(int x,int y,color_t color);
int   	_ASMAPI OGLSTUB_scanLeftWhileColor(int x,int y,color_t color);

void	_ASMAPI OGLSTUB_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI OGLSTUB_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI OGLSTUB_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_ditherTri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_ditherQuad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI OGLSTUB_cTrap(void);
void	_ASMAPI OGLSTUB_rgbTrap(void);

void	_ASMAPI OGLSTUB_z16_zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI OGLSTUB_z16_zDitherLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI OGLSTUB_z16_czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI OGLSTUB_z16_rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI OGLSTUB_z16_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_zDitherTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_zDitherQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z16_zTrap(void);
void	_ASMAPI OGLSTUB_z16_zDitherTrap(void);
void 	_ASMAPI OGLSTUB_z16_czTrap(void);
void 	_ASMAPI OGLSTUB_z16_rgbzTrap(void);

void	_ASMAPI OGLSTUB_z32_zLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI OGLSTUB_z32_zDitherLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI OGLSTUB_z32_czLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI OGLSTUB_z32_rgbzLine(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI OGLSTUB_z32_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_zDitherTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_zDitherQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI OGLSTUB_z32_zTrap(void);
void	_ASMAPI OGLSTUB_z32_zDitherTrap(void);
void 	_ASMAPI OGLSTUB_z32_czTrap(void);
void 	_ASMAPI OGLSTUB_z32_rgbzTrap(void);

#endif	/* __DRIVERS_COMMON_GOPENGL_H */

