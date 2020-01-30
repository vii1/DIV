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
* Description:	Device driver helper routines common to all display
*				device drivers.
*
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\common\common.h"

/*------------------------- Implementation --------------------------------*/

void DRV_stub(void) {}

PRIVATE void _MGL_setRVecs(rVecs *v)
/****************************************************************************
*
* Function:		_MGL_setRVecs
*
* Description:	Sets common rendering vectors to stub vector.
*
****************************************************************************/
{
	v->line                    = DRV_stubVector;
	v->scanLine                = DRV_stubVector;
	v->fillRect                = DRV_stubVector;
#ifndef MGL_LITE
	v->drawScanList            = DRV_stubVector;
	v->trap                    = DRV_stubVector;
	v->drawRegion              = DRV_stubVector;
	v->ellipse                 = DRV_stubVector;
	v->fillEllipse             = DRV_stubVector;
	v->ellipseArc              = DRV_stubVector;
	v->fillEllipseArc          = DRV_stubVector;
#endif
}

#ifdef	__WINDOWS__
void _ASMAPI PACKED8_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,
	int waitVRT)
/****************************************************************************
*
* Function:		PACKED8_realizePalette
* Parameters:	dc		- Device context
*				pal		- Palette to realize
*				num		- Number of colors to realise
*				index	- First color to realize
*
* Description:	Realizes the palette for an 8 bit memory device context.
*				Because the actual bits in the bitmap are stored in the DIB
*				color table, which is a separate color table to the normal
*				MGL color table, we need to re-build the DIB color table
*				correctly to make the DIB color table into an identity
*				palette mapping.
*
*				If the system is changed to or from SYSPAL_NOSTATIC mode,
*				all the memory DC bitmaps will have to have their palettes
*				realized again to ensure they are mapped to identity
*				palettes.
*
****************************************************************************/
{
	HDC				hdc;
	RGBQUAD			colors[256];
	PALETTEENTRY	pe[20];
	int				i;

	/* Copy all palette colors to begin with */
	memcpy(colors,pal,256 * sizeof(RGBQUAD));

    if (!_MGL_fullScreen) {
		hdc = GetDC(NULL);
		if (GetDeviceCaps(hdc,BITSPIXEL) <= 8) {
			if (GetSystemPaletteUse(hdc) == SYSPAL_NOSTATIC || _MGL_isBackground) {
				/* In SYSPAL_NOSTATIC mode we can assign all colors except for
				 * black and white which must be 0 and 255 respectively.
				 */
				colors[0].rgbRed = 0;
				colors[0].rgbGreen = 0;
				colors[0].rgbBlue = 0;
				colors[255].rgbRed = 0xFF;
				colors[255].rgbGreen = 0xFF;
				colors[255].rgbBlue = 0xFF;
				}
			else {
				/* Make sure we get the system palette entries to ensure an
				 * identity palette mapping.
				 */
				GetSystemPaletteEntries(hdc, 0, 10, &pe[0]);
				GetSystemPaletteEntries(hdc, 246, 10, &pe[10]);
				for (i = 0; i < 10; i++) {
					colors[i].rgbRed = pe[i].peRed;
					colors[i].rgbGreen = pe[i].peGreen;
					colors[i].rgbBlue = pe[i].peBlue;
					colors[i+246].rgbRed = pe[i+10].peRed;
					colors[i+246].rgbGreen = pe[i+10].peGreen;
					colors[i+246].rgbBlue = pe[i+10].peBlue;
					}
				}
			}
		ReleaseDC(NULL, hdc);
		}

	/* Now set the DIB color table entries */
	switch (_MGL_bltType) {
		case BLT_CreateDIBSection:
			SetDIBColorTable(dc->wm.memdc.hdc,index,num,colors);
			break;
		case BLT_WinG:
			if (dc->wm.memdc.hbm)
				WinGSetDIBColorTable(dc->wm.memdc.hdc,index,num,colors);
			break;
		}
}
#endif

#ifdef	MGL_FIX3D
PRIVATE void _MGL_setZVecs(zVecs *v)
/****************************************************************************
*
* Function:		_MGL_setRVecs
*
* Description:	Sets common 3D rendering vectors to stub vector.
*
****************************************************************************/
{
	v->zClear                  = DRV_stubVector;
	v->zLine                   = DRV_stubVector;
	v->zDitherLine             = DRV_stubVector;
	v->czLine                  = DRV_stubVector;
	v->rgbzLine                = DRV_stubVector;
	v->zTri                    = DRV_stubVector;
	v->zDitherTri              = DRV_stubVector;
	v->czTri                   = DRV_stubVector;
	v->rgbzTri                 = DRV_stubVector;
	v->zQuad                   = DRV_stubVector;
	v->zDitherQuad             = DRV_stubVector;
	v->czQuad                  = DRV_stubVector;
	v->rgbzQuad                = DRV_stubVector;
	v->zTrap                   = DRV_stubVector;
	v->zDitherTrap             = DRV_stubVector;
	v->czTrap                  = DRV_stubVector;
	v->rgbzTrap                = DRV_stubVector;
}
#endif

void _MGL_setCommonVectors(MGLDC *dc)
/****************************************************************************
*
* Function:		_MGL_setCommonVectors
* Parameters:	dc	- MGL device context to create HDC for
*
* Description:	Sets all the internal rendering vectors in the MGL
*				device context to point to a stub vector by default.
*
****************************************************************************/
{
	dc->r.getWinDC				  = DRV_stubVector;
	dc->r.getDefaultPalette       = DRV_stubVector;
	dc->r.realizePalette          = DRV_stubVector;
	dc->r.setColor                = DRV_stubVector;
	dc->r.setBackColor            = DRV_stubVector;
	dc->r.setClipRect             = DRV_stubVector;
	dc->r.beginDirectAccess       = DRV_stubVector;
	dc->r.endDirectAccess         = DRV_stubVector;
	dc->r.beginPixel              = DRV_stubVector;
	dc->r.getPixel                = DRV_stubVector;
	dc->r.putPixel                = DRV_stubVector;
	dc->r.endPixel                = DRV_stubVector;
	dc->r.getScanLine             = DRV_stubVector;
	dc->r.putScanLine             = DRV_stubVector;
	dc->r.stretchScanLine2x       = DRV_stubVector;
	dc->r.stretchScanLine         = DRV_stubVector;
#ifndef	MGL_LITE
	dc->r.setWriteMode            = DRV_stubVector;
	dc->r.setPenStyle             = DRV_stubVector;
	dc->r.setLineStipple          = DRV_stubVector;
	dc->r.setLineStippleCount     = DRV_stubVector;
	dc->r.setPenBitmapPattern     = DRV_stubVector;
	dc->r.setPenPixmapPattern     = DRV_stubVector;
	dc->r.ditherPixel             = DRV_stubVector;
	dc->r.putSrcTransparent       = DRV_stubVector;
	dc->r.putDstTransparent       = DRV_stubVector;
#endif
	dc->r.putMonoImage            = DRV_stubVector;
	dc->r.putMouseImage           = DRV_stubVector;
	dc->r.getImage                = DRV_stubVector;
	dc->r.putImage                = DRV_stubVector;
	dc->r.divotSize               = DRV_stubVector;
	dc->r.getDivot                = DRV_stubVector;
	dc->r.putDivot                = DRV_stubVector;
	dc->r.stretchBlt1x2           = DRV_stubVector;
	dc->r.stretchBlt2x2           = DRV_stubVector;
	dc->r.stretchBlt              = DRV_stubVector;
	_MGL_setRVecs(&dc->r.solid);
	_MGL_setRVecs(&dc->r.ropSolid);
	_MGL_setRVecs(&dc->r.patt);
	_MGL_setRVecs(&dc->r.colorPatt);
	_MGL_setRVecs(&dc->r.fatSolid);
	_MGL_setRVecs(&dc->r.fatRopSolid);
	_MGL_setRVecs(&dc->r.fatPatt);
	_MGL_setRVecs(&dc->r.fatColorPatt);
	_MGL_setRVecs(&dc->r.dither);

#ifndef	MGL_LITE
	dc->r.stippleLine             = DRV_stubVector;
	dc->r.getArcCoords            = DRV_stubVector;
	dc->r.drawStrBitmap           = DRV_stubVector;
	dc->r.drawCharVec             = DRV_stubVector;
	dc->r.complexPolygon          = DRV_stubVector;
	dc->r.polygon                 = DRV_stubVector;
	dc->r.ditherPolygon           = DRV_stubVector;
	dc->r.translateImage          = DRV_stubVector;
	dc->r.bitBlt                  = DRV_stubVector;
	dc->r.srcTransBlt             = DRV_stubVector;
	dc->r.dstTransBlt             = DRV_stubVector;
	dc->r.scanRightForColor       = DRV_stubVector;
	dc->r.scanLeftForColor        = DRV_stubVector;
	dc->r.scanRightWhileColor     = DRV_stubVector;
	dc->r.scanLeftWhileColor      = DRV_stubVector;
	dc->r.bitBltOff               = DRV_stubVector;
	dc->r.srcTransBltOff          = DRV_stubVector;
	dc->r.dstTransBltOff          = DRV_stubVector;
	dc->r.bitBltLin               = DRV_stubVector;
	dc->r.srcTransBltLin          = DRV_stubVector;
	dc->r.dstTransBltLin          = DRV_stubVector;
#ifdef	MGL_FIX3D
	dc->r.cLine                   = DRV_stubVector;
	dc->r.rgbLine                 = DRV_stubVector;
	dc->r.tri                     = DRV_stubVector;
	dc->r.ditherTri               = DRV_stubVector;
	dc->r.cTri                    = DRV_stubVector;
	dc->r.rgbTri                  = DRV_stubVector;
	dc->r.quad                    = DRV_stubVector;
	dc->r.ditherQuad              = DRV_stubVector;
	dc->r.cQuad                   = DRV_stubVector;
	dc->r.rgbQuad                 = DRV_stubVector;
	dc->r.cTrap                   = DRV_stubVector;
	dc->r.rgbTrap                 = DRV_stubVector;
	dc->r.zBegin                  = DRV_stubVector;
	_MGL_setZVecs(&dc->r.z16);
	_MGL_setZVecs(&dc->r.z32);
#endif
#endif
}

void _MGL_setLineVecs(MGLDC *dc,lineFunc f)
{
	dc->r.solid.line 			= f;
	dc->r.ropSolid.line 		= f;
	dc->r.patt.line 			= f;
	dc->r.colorPatt.line 		= f;
	dc->r.fatSolid.line 		= f;
	dc->r.fatRopSolid.line 		= f;
	dc->r.fatPatt.line 			= f;
	dc->r.fatColorPatt.line 	= f;
	dc->r.dither.line 			= f;
}

void _MGL_setScanLineVecs(MGLDC *dc,scanLineFunc f)
{
	dc->r.solid.scanLine 			= f;
	dc->r.ropSolid.scanLine 		= f;
	dc->r.patt.scanLine 			= f;
	dc->r.colorPatt.scanLine 		= f;
	dc->r.fatSolid.scanLine 		= f;
	dc->r.fatRopSolid.scanLine 		= f;
	dc->r.fatPatt.scanLine 			= f;
	dc->r.fatColorPatt.scanLine 	= f;
	dc->r.dither.scanLine 			= f;
}

void _MGL_setFillRectVecs(MGLDC *dc,fillRectFunc f)
{
	dc->r.solid.fillRect 			= f;
	dc->r.ropSolid.fillRect 		= f;
	dc->r.patt.fillRect 			= f;
	dc->r.colorPatt.fillRect 		= f;
	dc->r.fatSolid.fillRect 		= f;
	dc->r.fatRopSolid.fillRect 		= f;
	dc->r.fatPatt.fillRect 			= f;
	dc->r.fatColorPatt.fillRect 	= f;
	dc->r.dither.fillRect 			= f;
}

#ifndef	MGL_LITE
void _MGL_setDrawScanListVecs(MGLDC *dc,drawScanListFunc f)
{
	dc->r.solid.drawScanList 			= f;
	dc->r.ropSolid.drawScanList 		= f;
	dc->r.patt.drawScanList 			= f;
	dc->r.colorPatt.drawScanList 		= f;
	dc->r.fatSolid.drawScanList 		= f;
	dc->r.fatRopSolid.drawScanList 		= f;
	dc->r.fatPatt.drawScanList 			= f;
	dc->r.fatColorPatt.drawScanList 	= f;
	dc->r.dither.drawScanList 			= f;
}

void _MGL_setTrapVecs(MGLDC *dc,trapFunc f)
{
	dc->r.solid.trap 			= f;
	dc->r.ropSolid.trap 		= f;
	dc->r.patt.trap 			= f;
	dc->r.colorPatt.trap 		= f;
	dc->r.fatSolid.trap 		= f;
	dc->r.fatRopSolid.trap 		= f;
	dc->r.fatPatt.trap 			= f;
	dc->r.fatColorPatt.trap 	= f;
	dc->r.dither.trap 			= f;
}

void _MGL_setDrawRegionVecs(MGLDC *dc,drawRegionFunc f)
{
	dc->r.solid.drawRegion 			= f;
	dc->r.ropSolid.drawRegion 		= f;
	dc->r.patt.drawRegion 			= f;
	dc->r.colorPatt.drawRegion 		= f;
	dc->r.fatSolid.drawRegion 		= f;
	dc->r.fatRopSolid.drawRegion 	= f;
	dc->r.fatPatt.drawRegion 		= f;
	dc->r.fatColorPatt.drawRegion 	= f;
	dc->r.dither.drawRegion 		= f;
}

void _MGL_setEllipseVecs(MGLDC *dc,ellipseFunc f)
{
	dc->r.solid.ellipse 			= f;
	dc->r.ropSolid.ellipse 			= f;
	dc->r.patt.ellipse 				= f;
	dc->r.colorPatt.ellipse 		= f;
	dc->r.fatSolid.ellipse 			= f;
	dc->r.fatRopSolid.ellipse 		= f;
	dc->r.fatPatt.ellipse 			= f;
	dc->r.fatColorPatt.ellipse 		= f;
	dc->r.dither.ellipse 			= f;
}

void _MGL_setEllipseArcVecs(MGLDC *dc,ellipseArcFunc f)
{
	dc->r.solid.ellipseArc 				= f;
	dc->r.ropSolid.ellipseArc 			= f;
	dc->r.patt.ellipseArc 				= f;
	dc->r.colorPatt.ellipseArc 			= f;
	dc->r.fatSolid.ellipseArc 			= f;
	dc->r.fatRopSolid.ellipseArc 		= f;
	dc->r.fatPatt.ellipseArc 			= f;
	dc->r.fatColorPatt.ellipseArc 		= f;
	dc->r.dither.ellipseArc 			= f;
}

void _MGL_setFillEllipseVecs(MGLDC *dc,fillEllipseFunc f)
{
	dc->r.solid.fillEllipse 			= f;
	dc->r.ropSolid.fillEllipse 			= f;
	dc->r.patt.fillEllipse 				= f;
	dc->r.colorPatt.fillEllipse 		= f;
	dc->r.fatSolid.fillEllipse 			= f;
	dc->r.fatRopSolid.fillEllipse 		= f;
	dc->r.fatPatt.fillEllipse 			= f;
	dc->r.fatColorPatt.fillEllipse 		= f;
	dc->r.dither.fillEllipse 			= f;
}

void _MGL_setFillEllipseArcVecs(MGLDC *dc,fillEllipseArcFunc f)
{
	dc->r.solid.fillEllipseArc 			= f;
	dc->r.ropSolid.fillEllipseArc 		= f;
	dc->r.patt.fillEllipseArc 			= f;
	dc->r.colorPatt.fillEllipseArc 		= f;
	dc->r.fatSolid.fillEllipseArc 		= f;
	dc->r.fatRopSolid.fillEllipseArc 	= f;
	dc->r.fatPatt.fillEllipseArc 		= f;
	dc->r.fatColorPatt.fillEllipseArc 	= f;
	dc->r.dither.fillEllipseArc 		= f;
}
#endif

ulong _ASMAPI DRV_getWinDC(MGLDC *dc)
{ dc = dc; return 0; }

ulong _ASMAPI PACKED_getWinDC(MGLDC *dc)
{
#ifdef	__WINDOWS__
	return (ulong)dc->wm.memdc.hdc;
#else
	dc = dc;
	return NULL;
#endif
}

ulong _ASMAPI FULLSCREEN_getWinDC(MGLDC *dc)
/****************************************************************************
*
* Function:		FULLSCREEN_getWinDC
* Parameters:	dc	- MGL device context to create HDC for
*
* Description:	This function uses our MGL DIB driver to create a valid
*				Windows HDC for our display memory surface in both WinDirect
*				and DirectDraw modes. Using this HDC applications can then
*				use GDI functions to render to display memory, and we can
*				use OpenGL to draw directly to our display memory as well.
*
****************************************************************************/
{
#ifdef	__WINDOWS__
	if (!dc->wm.fulldc.hdc) {
		MGL_dibInfo	dibInfo;

		memset(&dibInfo,0,sizeof(dibInfo));
		dibInfo.h.biSize = sizeof(BITMAPINFOHEADER);
		dibInfo.h.biWidth = dc->mi.xRes+1;
		dibInfo.h.biHeight = -(dc->mi.yRes+1);
		dibInfo.h.biPlanes = 1;
		dibInfo.h.biClrUsed = 0;
		dibInfo.h.biClrImportant = 0;
		dibInfo.h.biCompression = BI_RGB;
		dibInfo.h.biSizeImage = (long)dc->mi.bytesPerLine * (dc->mi.yRes+1);
		dibInfo.h.biBitCount = dc->mi.bitsPerPixel;
		switch (dc->mi.bitsPerPixel) {
			case 8:
				dibInfo.h.biClrUsed = 256;
				memcpy(dibInfo.p.pal,dc->colorTab,sizeof(dibInfo.p.pal));
				break;
			case 15:
				dibInfo.h.biBitCount = 16;
				break;
			case 16:
				dibInfo.h.biCompression = BI_BITFIELDS;
				dibInfo.p.m.maskRed 	= 0x0000F800;
				dibInfo.p.m.maskGreen 	= 0x000007E0;
				dibInfo.p.m.maskBlue 	= 0x0000001F;
				break;
			case 24:
				if (dc->pf.redPos == 0) {
					/* Bitmap is in the extended 24 bit BGR format */
					dibInfo.h.biCompression = BI_BITFIELDS;
					dibInfo.p.m.maskRed 	= 0x0000FF;
					dibInfo.p.m.maskGreen 	= 0x00FF00;
					dibInfo.p.m.maskBlue 	= 0xFF0000;
					}
				break;
			case 32:
				if (dc->pf.rsvdPos != 0) {
					if (dc->pf.redPos == 0) {
						/* Bitmap is in the extended 32 bit ABGR format */
						dibInfo.h.biCompression = BI_BITFIELDS;
						dibInfo.p.m.maskRed 	= 0x000000FFUL;
						dibInfo.p.m.maskGreen 	= 0x0000FF00UL;
						dibInfo.p.m.maskBlue 	= 0x00FF0000UL;
						}
					}
				else {
					if (dc->pf.redPos != 8) {
						/* Bitmap is in the extended 32 bit RGBA format */
						dibInfo.h.biCompression = BI_BITFIELDS;
						dibInfo.p.m.maskRed 	= 0xFF000000UL;
						dibInfo.p.m.maskGreen 	= 0x00FF0000UL;
						dibInfo.p.m.maskBlue 	= 0x0000FF00UL;
						}
					else {
						/* Bitmap is in the extended 32 bit BGRA format */
						dibInfo.h.biCompression = BI_BITFIELDS;
						dibInfo.p.m.maskRed 	= 0x0000FF00UL;
						dibInfo.p.m.maskGreen 	= 0x00FF0000UL;
						dibInfo.p.m.maskBlue 	= 0xFF000000UL;
						}
					}
				break;
			}
		dibInfo.dwSurface = (DWORD)dc->surface;
		dc->wm.fulldc.hdc = CreateDC("MGLDIB",NULL,NULL,(LPDEVMODE)&dibInfo);
		}
	return (ulong)dc->wm.fulldc.hdc;
#else
	dc = dc;
	return NULL;
#endif
}

void _ASMAPI FULLSCREEN_destroyHDC(MGLDC *dc)
{
#ifdef	__WINDOWS__
	DeleteDC(dc->wm.fulldc.hdc);
#else
	dc = dc;
#endif
}

void _ASMAPI DRV_setColor(color_t color)
{ DC.a.color = DC.intColor = color; }

void _ASMAPI DRV_setBackColor(color_t color)
{ DC.a.backColor = DC.intBackColor = color; }

void _ASMAPI DRV_setClipRect(MGLDC *dc,int left,int top,int right,int bottom)
/****************************************************************************
*
* Function:		DRV_setClipRect
* Parameters:	left,top	- Left top coordinate
*				right,botom	- Right bottom coordinate
*
* Description:	Generic routine to set the clipping rectangle. This routine
*				simply sets both the viewport space and screen space
*				clipping rectangles. We also ensure that the clip rectangle
*               does not go outside of the full screen clip space.
*
*				We also set the internal clipping rectangle to be translated
*				into screen space.
*
****************************************************************************/
{
	rect_t	r;
	int		adjustX = dc->a.viewPort.left - dc->a.viewPortOrg.x;
	int		adjustY = dc->a.viewPort.top - dc->a.viewPortOrg.y;

	/* Clip to screen clipping boundary */
	r.left = left;	r.right = right;
	r.top = top;    r.bottom = bottom;
	MGL_offsetRect(r,adjustX,adjustY);
	MGL_sectRectFast(r,dc->size,&r);
	MGL_offsetRect(r,-adjustX,-adjustY);

	/* Now set internal clipping rectangles */
	dc->a.clipRect.left = r.left;
	dc->a.clipRect.top = r.top;
	dc->a.clipRect.right = r.right;
	dc->a.clipRect.bottom = r.bottom;
	dc->clipRectFX.left = MGL_TOFIX(r.left);
	dc->clipRectFX.top = MGL_TOFIX(r.top);
	dc->clipRectFX.right = MGL_TOFIX(r.right);
	dc->clipRectFX.bottom = MGL_TOFIX(r.bottom);
	dc->intClipRect.left = r.left + adjustX;
	dc->intClipRect.top = r.top + adjustY;
	dc->intClipRect.right = r.right + adjustX;
	dc->intClipRect.bottom = r.bottom + adjustY;
	dc->intClipRectFX.left = MGL_TOFIX(dc->intClipRect.left);
	dc->intClipRectFX.top = MGL_TOFIX(dc->intClipRect.top);
	dc->intClipRectFX.right = MGL_TOFIX(dc->intClipRect.right);
	dc->intClipRectFX.bottom = MGL_TOFIX(dc->intClipRect.bottom);
}

void _ASMAPI DRV_setWriteMode(int mode)
{ DC.a.writeMode = mode; _MGL_setRenderingVectors(); }

#ifndef	MGL_LITE
void _ASMAPI DRV_setPenStyle(int style)
{ DC.a.penStyle = style; _MGL_setRenderingVectors(); }

void _ASMAPI DRV_setPenBitmapPattern(const pattern_t *pat)
{ memcpy(&DC.a.penPat,pat,sizeof(pattern_t)); }

void _ASMAPI DRV_setPenPixmapPattern(const pixpattern_t *pat)
{ memcpy(&DC.a.penPixPat,pat,sizeof(pixpattern_t)); }

void _ASMAPI DRV_setLineStipple(ushort stipple)
{ DC.a.lineStipple = stipple; }

void _ASMAPI DRV_setLineStippleCount(uint stippleCount)
{ DC.a.stippleCount = stippleCount; }

void _ASMAPI DRV_copyImage(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,ibool yinc,int bytesPerRow,int op,
	void (_ASMAPI *readImage)(MGLDC *dc,int left,int top,int right,int bottom,
		void *buffer),
	void (_ASMAPI *writeImage)(MGLDC *dc,int dstLeft,int dstTop,
		void *buffer,int op))
/****************************************************************************
*
* Function:		DRV_copyImage
* Parameters:	dc						- Device context
* 				left,top,right,bottom	- Rectangle to copy from
*				dstLeft,dstTop			- Point to copy to
*				yinc					- True if y should be incremented
*				bytesPerRow				- Number of bytes in a scanline row
*				op						- Write mode operation for dest.
*				readImage				- Routine to call to read data
*				writeImage				- Routine to call to write data
*
* Description:	Copy a block of video memory from one location to another
*				by moving as much data as possible from video RAM to
*				system RAM and back to video RAM.
*
****************************************************************************/
{
	int		cHeight;
	rect_t	c,d;

	cHeight = (_MGL_bufSize-16) / bytesPerRow;
	if (cHeight > (bottom - top))
		cHeight = bottom - top;
	c.left = left;
	c.right = right;
	d.left = dstLeft;
	d.right = dstLeft + (right - left);
	if (yinc) {
		c.top = top;
		c.bottom = top + cHeight;
		d.top = dstTop;
		d.bottom = dstTop + cHeight;
		}
	else {
		c.bottom = bottom;
		c.top = bottom - cHeight;
		d.bottom = dstTop + (bottom - top);
		d.top = d.bottom - cHeight;
		}

	while (c.top < c.bottom) {
		readImage(dc,c.left,c.top,c.right,c.bottom,_MGL_buf);
		writeImage(dc,d.left,d.top,_MGL_buf,op);
		if (yinc) {
			c.top += cHeight;
			c.bottom += cHeight;
			d.top += cHeight;
			d.bottom += cHeight;
			if (c.bottom > bottom) {
				c.bottom = bottom;
				d.bottom = d.top + (c.bottom - c.top);
				}
			}
		else {
			c.top -= cHeight;
			c.bottom -= cHeight;
			d.top -= cHeight;
			d.bottom -= cHeight;
			if (c.top < top) {
				c.top = top;
				d.top = d.bottom - (c.bottom - c.top);
				}
			}
		}
}
#endif

void * _ASMAPI DRV_getDefaultPalette(MGLDC *dc)
{ dc = dc; return NULL; }

ulong MGLAPI _MGL_getOriginOffset(MGLDC *dc)
{ return dc->originOffset; }

int	MGLAPI _MGL_getBankOffset(MGLDC *dc)
{ return dc->bankOffset; }

#if	defined(NO_ASSEMBLER) && !defined(__REALDOS__)

/* Dummy header blocks for drivers that are not available. Rather than make
 * application code re-compile and not register these missing drivers, we
 * have stubs in here so existing code can compile and link fine but the
 * drivers will not be present.
 */

#ifdef __BORLANDC__
#pragma warn -par
#endif
ibool _ASMAPI DRV_stubDriver(void)
{ return false; }

#define	DECLARE_STUB(name)					\
	drivertype	_VARAPI name##_driver = {   \
		IDENT_FLAG,                         \
		MGL_##name##NAME,					\
		MGL_##name##NAME,					\
		"",0,0,0,          					\
		(void*)DRV_stubDriver,              \
		(void*)DRV_stubDriver,              \
		}

DECLARE_STUB(VGA4);
DECLARE_STUB(VGA8);
DECLARE_STUB(VGAX);
DECLARE_STUB(SVGA4);
DECLARE_STUB(SVGA8);
DECLARE_STUB(SVGA16);
DECLARE_STUB(SVGA24);
DECLARE_STUB(SVGA32);
DECLARE_STUB(LINEAR8);
DECLARE_STUB(LINEAR16);
DECLARE_STUB(LINEAR24);
DECLARE_STUB(LINEAR32);
DECLARE_STUB(ACCEL8);
DECLARE_STUB(ACCEL16);
DECLARE_STUB(ACCEL24);
DECLARE_STUB(ACCEL32);

#endif
