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
* Description:	Common header file for internal device driver code used
*				by all device drivers.
*
*
****************************************************************************/

#ifndef	__DRIVERS_COMMON_COMMON_H
#define	__DRIVERS_COMMON_COMMON_H

/*---------------------- Macro and type definitions -----------------------*/

#define	DRV_useMode(modes,mode,id,pgs,flgs)		\
   {modes[mode].driver = id;					\
	modes[mode].pages = pgs;					\
	modes[mode].flags = flgs;}

#define	DRV_removeMode(modes,mode)			\
   {modes[mode].driver = 0xFF;				\
	modes[mode].pages = 0;					\
    modes[mode].flags = 0;}

#define	DRV_stubVector	(void*)DRV_stub

#define	PACKED24_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)DC.surface + ((long)y * DC.mi.bytesPerLine) + x*3))

#define	PACKED24_pixelAddr2(dc,x,y)	\
	 ((void*)((uchar _HUGE *)(dc)->surface + ((long)y * (dc)->mi.bytesPerLine) + x*3))

#define	MGL_packColorRGB(R,G,B)										\
 (((R) & 0xFF0000L) | (((G) >> 8) & 0xFF00L) | ((B) >> 16))

#define	MGL_packColorBGR(R,G,B)										\
 (((B) & 0xFF0000L) | (((G) >> 8) & 0xFF00L) | ((R) >> 16))

extern palette_t _VARAPI _VGA8_defPal[256];
extern palette_t _VARAPI _VGA4_defPal[16];

/*------------------------- Function Prototypes ---------------------------*/

void	DRV_stub(void);				/* Empty stub routine				*/

/* Generic device driver C helper routines */

void	_MGL_setLineVecs(MGLDC *dc,lineFunc f);
void	_MGL_setScanLineVecs(MGLDC *dc,scanLineFunc f);
void	_MGL_setFillRectVecs(MGLDC *dc,fillRectFunc f);
void	_MGL_setDrawScanListVecs(MGLDC *dc,drawScanListFunc f);
void	_MGL_setTrapVecs(MGLDC *dc,trapFunc f);
void	_MGL_setDrawRegionVecs(MGLDC *dc,drawRegionFunc f);
void	_MGL_setEllipseVecs(MGLDC *dc,ellipseFunc f);
void	_MGL_setEllipseArcVecs(MGLDC *dc,ellipseArcFunc f);
void	_MGL_setFillEllipseVecs(MGLDC *dc,fillEllipseFunc f);
void	_MGL_setFillEllipseArcVecs(MGLDC *dc,fillEllipseArcFunc f);
ulong	_ASMAPI DRV_getWinDC(MGLDC *dc);
ulong 	_ASMAPI PACKED_getWinDC(MGLDC *dc);
ulong 	_ASMAPI FULLSCREEN_getWinDC(MGLDC *dc);
void	_ASMAPI FULLSCREEN_createHDC(MGLDC *dc);
void	_ASMAPI FULLSCREEN_destroyHDC(MGLDC *dc);
void 	_ASMAPI DRV_setColor(color_t color);
void 	_ASMAPI DRV_setBackColor(color_t color);
void 	_ASMAPI DRV_setWriteMode(int mode);
void 	_ASMAPI DRV_setPenStyle(int style);
void 	_ASMAPI DRV_setPenBitmapPattern(const pattern_t *pat);
void 	_ASMAPI DRV_setPenPixmapPattern(const pixpattern_t *pat);
void 	_ASMAPI DRV_setLineStipple(ushort stipple);
void 	_ASMAPI DRV_setLineStippleCount(uint stippleCount);
void 	_ASMAPI DRV_setClipRect(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI DRV_copyImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,ibool yinc,int bytesPerRow,int op,
		void (_ASMAPI *readImage)(MGLDC *dc,int left,int top,int right,int bottom,void *buffer),
		void (_ASMAPI *writeImage)(MGLDC *dc,int dstLeft,int dstTop,void *buffer,int op));
void 	* _ASMAPI DRV_getDefaultPalette(MGLDC *dc);
void 	_ASMAPI PACKED8_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);

#endif	/* __DRIVERS_COMMON_COMMON_H */
