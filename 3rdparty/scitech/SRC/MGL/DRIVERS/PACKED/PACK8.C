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
* Description:  8 bit packed pixel device context routines. This code will
*				be linked in if this driver is registered for use. When
*				compiling for Windows, we will use WinG or CreateDIBSection
*				to create a DIB that we can render directly into.
*
*
****************************************************************************/

// TODO: Provide direct support in here for translating 4 bit and 8 bit
// images in assembler or at least optimised C code. This will provide
// quite a speed increase.

#include "mgl.h"
#pragma hdrstop
#include "drivers\packed\packed8.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI PACKED8_driver = {
	IDENT_FLAG,
	MGL_PACKED8NAME,
	MGL_PACKED8NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grNONE,
	NULL,
	PACKED8_initDriver,
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

#ifdef	MGLWIN

PRIVATE long AllocateSurface(MGLDC *dc)
/****************************************************************************
*
* Function:		AllocateSurface
* Parameters:	dc	- Device context.
* Returns:		Total size of the device context surface
*
* Description:	Allocates the memory for the device surface. This version
*				allocates a WinG DC and Bitmap that we can directly render
*				into under Windows.
*
****************************************************************************/
{
	winBITMAPINFO	info;

	/* Create the bitmap info with an empty palette (we will remap the palette
	 * when we do a bitBlt to the PACKED8). Note that WinG always creates
	 * bitmap surfaces that have an integral number of DWORDS for the
	 * bytesPerLine values.
	 */
	memset(&info,0,sizeof(info));
	info.header.biSize = sizeof(winBITMAPINFOHEADER);
	info.header.biWidth = dc->mi.xRes+1;
	info.header.biHeight = -(dc->mi.yRes+1);
	info.header.biPlanes = 1;
	info.header.biBitCount = 8;
	info.header.biCompression = winBI_RGB;
	info.header.biSizeImage = (long)dc->mi.bytesPerLine * (dc->mi.yRes+1);
	info.header.biClrUsed = 256;
	info.header.biClrImportant = 0;
    dc->ownMemory = true;
	return _MGL_allocateDIB(dc,&info);
}

PRIVATE void DestroySurface(MGLDC *dc)
/****************************************************************************
*
* Function:		DestroySurface
* Parameters:	dc	- Device context
*
* Description:	Destroys the packed pixel device surface.
*
****************************************************************************/
{
    if (dc->ownMemory)
		_MGL_freeDIB(dc);
}

#else

PRIVATE long AllocateSurface(MGLDC *dc)
/****************************************************************************
*
* Function:		AllocateSurface
* Parameters:	dc	- Device context.
* Returns:		Total size of the device context surface
*
* Description:	Allocate the memory for the device surface. This version
*				simply uses calloc() to allocate a buffer large enough.
*
****************************************************************************/
{
	long size = (long)dc->mi.bytesPerLine * (dc->mi.yRes+1);
	if ((dc->surface = MGL_calloc(1,size)) == NULL) {
		_MGL_result = grNoMem;
		return 0;
		}
    dc->ownMemory = true;
	return size;
}

PRIVATE void DestroySurface(MGLDC *dc)
/****************************************************************************
*
* Function:		destroyDC
* Parameters:	dc	- Device context
*
* Description:	Destroys the packed pixel device surface.
*
****************************************************************************/
{
    if (dc->ownMemory)
		MGL_free(dc->surface);
}

#endif

PRIVATE void _ASMAPI destroyDC(MGLDC *dc)
/****************************************************************************
*
* Function:		destroyDC
* Parameters:	dc	- Device context
*
* Description:	Destroys all internal memory allocated by the device
*				context. If this is the last device of it's type, we also
*				deallocate the state buffer and the colOffset buffer.
*
****************************************************************************/
{
	if (!(dc->flags & MGL_SHARED_ZBUFFER))
		MGL_free(dc->zbuffer);
	if (--dc->v->d.refCount == 0) {
		if (dc->v != g_state)
			MGL_fatalError("Internal error: dc->v != g_state!\n");
		MGL_free(dc->v);
		g_state = NULL;
		}
	DestroySurface(dc);
}

void PACKED8_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		PACKED8_initInternal
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver internal tables for 8 bit
*				packed pixel driver routines.
*
****************************************************************************/
{
	dc->flags |= MGL_LINEAR_ACCESS;
	dc->ellipseFixup = true;

	dc->r.setColor 				= PACKED8_setColor;
	dc->r.setBackColor 			= PACKED8_setBackColor;
	dc->r.setWriteMode			= DRV_setWriteMode;
	dc->r.setClipRect 			= DRV_setClipRect;
#ifndef	MGL_LITE
	dc->r.setPenStyle			= DRV_setPenStyle;
	dc->r.setPenBitmapPattern	= DRV_setPenBitmapPattern;
	dc->r.setPenPixmapPattern	= DRV_setPenPixmapPattern;
	dc->r.setLineStipple		= DRV_setLineStipple;
	dc->r.setLineStippleCount	= DRV_setLineStippleCount;
#endif

	dc->r.getPixel 				= PACKED8_getPixel;
	dc->r.putPixel 				= PACKED8_putPixel;
	dc->r.getScanLine			= PACKED8_getScanLine;
	dc->r.putScanLine			= PACKED8_putScanLine;
	_MGL_setLineVecs(dc,		  PACKED8_line);
	dc->r.solid.scanLine 		= PACKED8_solidScanLine;
	dc->r.ropSolid.scanLine 	= PACKED8_ropSolidScanLine;
	_MGL_setFillRectVecs(dc,	  __EMU__fillRect);
	dc->r.solid.fillRect 		= PACKED8_solidFillRect;
	dc->r.putMonoImage			= PACKED8_putMonoImage;
	dc->r.putMouseImage			= __EMU__putMouseImage;
	dc->r.getImage				= PACKED8_getImage;
	dc->r.putImage				= PACKED8_putImage;
	dc->r.divotSize				= PACKED8_divotSize;
	dc->r.getDivot 				= PACKED8_getDivot;
	dc->r.putDivot 				= PACKED8_putDivot;
	dc->r.stretchScanLine2x		= PACKED8_stretchScanLine2x;
	dc->r.stretchScanLine		= PACKED8_stretchScanLine;
	dc->r.stretchBlt1x2			= PACKED8_stretchBlt1x2;
	dc->r.stretchBlt2x2			= PACKED8_stretchBlt2x2;
	dc->r.stretchBlt			= PACKED8_stretchBlt;

#ifndef	MGL_LITE
	dc->r.stippleLine			= PACKED8_stippleLine;
	dc->r.dither.line         	= PACKED8_ditherLine;
	_MGL_setTrapVecs(dc,	  	  __EMU__trap);
	dc->r.solid.trap			= PACKED8_solidTrap;
	dc->r.dither.trap			= PACKED8_ditherTrap;
	dc->r.ditherPixel			= PACKED8_ditherPixel;
	dc->r.putSrcTransparent		= PACKED8_putSrcTransparent;
	dc->r.putDstTransparent		= PACKED8_putDstTransparent;
	dc->r.patt.line				= __EMU__fatPenLine;
	dc->r.colorPatt.line		= __EMU__fatPenLine;
	dc->r.fatSolid.line			= __EMU__fatPenLine;
	dc->r.fatRopSolid.line		= __EMU__fatPenLine;
	dc->r.fatPatt.line			= __EMU__fatPenLine;
	dc->r.fatColorPatt.line		= __EMU__fatPenLine;
	dc->r.patt.scanLine 		= PACKED8_pattScanLine;
	dc->r.colorPatt.scanLine 	= PACKED8_colorPattScanLine;
	dc->r.fatSolid.scanLine 	= PACKED8_solidScanLine;
	dc->r.fatRopSolid.scanLine 	= PACKED8_ropSolidScanLine;
	dc->r.fatPatt.scanLine 		= PACKED8_pattScanLine;
	dc->r.fatColorPatt.scanLine = PACKED8_colorPattScanLine;
	dc->r.dither.scanLine 		= PACKED8_ditherScanLine;
	_MGL_setDrawScanListVecs(dc,  __EMU__drawScanList);
	dc->r.solid.drawScanList 	= PACKED8_solidDrawScanList;
	_MGL_setDrawRegionVecs(dc,	  __EMU__drawRegion);
#ifdef	NO_ASSEMBLER
	_MGL_setEllipseVecs(dc,	  	  __EMU__ellipse);
#else
// TODO: Re-code the packed pixel ellipse routine with direct writes in
//		 C using the new floating point formulas for speed.
	_MGL_setEllipseVecs(dc,	  	  __EMU__ellipse);
//	_MGL_setEllipseVecs(dc,	  	  PACKED8_ellipse);
#endif
	dc->r.dither.ellipse      	= __EMU__ellipse;
	dc->r.fatSolid.ellipse		= __EMU__fatPenEllipse;
	dc->r.fatRopSolid.ellipse	= __EMU__fatPenEllipse;
	dc->r.fatPatt.ellipse		= __EMU__fatPenEllipse;
	dc->r.fatColorPatt.ellipse 	= __EMU__fatPenEllipse;
	_MGL_setFillEllipseVecs(dc,	  __EMU__fillEllipse);
	_MGL_setEllipseArcVecs(dc,	  __EMU__ellipseArc);
	dc->r.fatSolid.ellipseArc	= __EMU__fatPenEllipseArc;
	dc->r.fatRopSolid.ellipseArc= __EMU__fatPenEllipseArc;
	dc->r.fatPatt.ellipseArc	= __EMU__fatPenEllipseArc;
	dc->r.fatColorPatt.ellipseArc= __EMU__fatPenEllipseArc;
	_MGL_setFillEllipseArcVecs(dc,__EMU__fillEllipseArc);
	dc->r.getArcCoords 			= __EMU__getArcCoords;
	dc->r.drawStrBitmap			= __EMU__drawStrBitmap;
	dc->r.drawCharVec			= __EMU__drawCharVec;
	dc->r.complexPolygon 		= __EMU__complexPolygon;
	dc->r.polygon 				= __EMU__polygon;
	dc->r.ditherPolygon 		= __EMU__polygon;
	dc->r.translateImage		= PACKED8_translateImage;
	dc->r.bitBlt				= __EMU__bitBlt;
	dc->r.srcTransBlt			= PACKED8_srcTransBlt;
	dc->r.dstTransBlt			= PACKED8_dstTransBlt;
	dc->r.scanRightForColor		= PACKED8_scanRightForColor;
	dc->r.scanLeftForColor		= PACKED8_scanLeftForColor;
	dc->r.scanRightWhileColor	= PACKED8_scanRightWhileColor;
	dc->r.scanLeftWhileColor	= PACKED8_scanLeftWhileColor;

#ifdef	MGL_FIX3D
	/* 3D rasterisation */
	dc->r.cLine 				= PACKED8_cLine;
	dc->r.rgbLine 				= PACKED8_rgbLine;
	dc->r.tri 					= __EMU__tri;
	dc->r.ditherTri          	= __EMU__tri;
	dc->r.cTri 					= __EMU__cTri;
	dc->r.rgbTri 				= __EMU__rgbTri;
	dc->r.quad 					= __EMU__quad;
	dc->r.ditherQuad          	= __EMU__quad;
	dc->r.cQuad 				= __EMU__cQuad;
	dc->r.rgbQuad 				= __EMU__rgbQuad;
	dc->r.cTrap					= PACKED8_cTrap;
	dc->r.rgbTrap				= PACKED8_rgbTrap;

	/* 3D 16 bit Zbuffering */
	dc->r.zBegin				= __EMU__zBegin;
	dc->r.z16.zClear			= __EMU__zClear16;
	dc->r.z16.zLine				= PACKED8_zLine16;
	dc->r.z16.zDitherLine		= PACKED8_zDitherLine16;
	dc->r.z16.czLine			= PACKED8_czLine16;
	dc->r.z16.rgbzLine			= PACKED8_rgbzLine16;
	dc->r.z16.zTri				= __EMU__zTri;
	dc->r.z16.zDitherTri		= __EMU__zTri;
	dc->r.z16.czTri				= __EMU__czTri;
	dc->r.z16.rgbzTri			= __EMU__rgbzTri;
	dc->r.z16.zQuad				= __EMU__zQuad;
	dc->r.z16.zDitherQuad		= __EMU__zQuad;
	dc->r.z16.czQuad			= __EMU__czQuad;
	dc->r.z16.rgbzQuad			= __EMU__rgbzQuad;
	dc->r.z16.zTrap				= PACKED8_zTrap16;
	dc->r.z16.zDitherTrap		= PACKED8_zDitherTrap16;
	dc->r.z16.czTrap			= PACKED8_czTrap16;
	dc->r.z16.rgbzTrap			= PACKED8_rgbzTrap16;

	/* 3D 32 bit Zbuffering */
	dc->r.z32.zClear			= __EMU__zClear32;
	dc->r.z32.zLine				= PACKED8_zLine32;
	dc->r.z32.zDitherLine		= PACKED8_zDitherLine32;
	dc->r.z32.czLine			= PACKED8_czLine32;
	dc->r.z32.rgbzLine			= PACKED8_rgbzLine32;
	dc->r.z32.zTri				= __EMU__zTri;
	dc->r.z32.zDitherTri		= __EMU__zTri;
	dc->r.z32.czTri				= __EMU__czTri;
	dc->r.z32.rgbzTri			= __EMU__rgbzTri;
	dc->r.z32.zQuad				= __EMU__zQuad;
	dc->r.z32.zDitherQuad		= __EMU__zQuad;
	dc->r.z32.czQuad			= __EMU__czQuad;
	dc->r.z32.rgbzQuad			= __EMU__rgbzQuad;
	dc->r.z32.zTrap				= PACKED8_zTrap32;
	dc->r.z32.zDitherTrap		= PACKED8_zDitherTrap32;
	dc->r.z32.czTrap			= PACKED8_czTrap32;
	dc->r.z32.rgbzTrap			= PACKED8_rgbzTrap32;
#endif	/* MGL_FIX3D */
#endif	/* !MGL_LITE */
}

ibool _ASMAPI PACKED8_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		PACKED8_initDriver
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the memory device driver, allocating the
*				display surface for the context.
*
****************************************************************************/
{
	/* Allocate a single buffer for the driver specific state buffer */
	if (g_state == NULL) {
		if ((g_state = (void*)MGL_calloc(1,sizeof(PACKED8_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	dc->v = g_state;
	dc->v->m.refCount++;

	/* Load the device information	*/
	dc->deviceType = MGL_MEMORY_DEVICE;
	dc->mi.numberOfPlanes = 1;
	dc->mi.maxColor = 0xFF;
	dc->mi.maxPage = 0;
	dc->mi.aspectRatio = 1000;

	/* Set direct color mode info for 8 bit dithered operation */
	dc->mi.redMaskSize 			= 8;
	dc->mi.redFieldPosition 	= 16;
	dc->mi.greenMaskSize 		= 8;
	dc->mi.greenFieldPosition 	= 8;
	dc->mi.blueMaskSize 		= 8;
	dc->mi.blueFieldPosition 	= 0;
	dc->mi.rsvdMaskSize 		= 0;
	dc->mi.rsvdFieldPosition	= 0;

	/* Fill in required vectors */
	dc->v->m.destroy			= destroyDC;
	dc->r.getWinDC				= PACKED_getWinDC;
	dc->r.getDefaultPalette 	= DRV_getDefaultPalette;
#ifdef	MGLWIN
	dc->r.realizePalette 		= PACKED8_realizePalette;
#endif
	/* Fill in the remaining driver vectors */
	PACKED8_initInternal(dc);

	driverId = driverId;
	modeId = modeId;
	hwnd = hwnd;
	virtualX = virtualX;
	virtualY = virtualY;
	numBuffers = numBuffers;

	if (!dc->surface) {
		dc->mi.bytesPerLine = (dc->mi.xRes + 4) & ~3;	/* DWORD align data	*/
		return (AllocateSurface(dc) != 0);
		}
	return true;
}

/* Pre-expand the colors to 32-bits wide internally */

void _ASMAPI PACKED8_setColor(color_t color)
{
	DC.a.color = color;
	DC.intColor = color | (color << 8) | (color << 16) | (color << 24);
}

void _ASMAPI PACKED8_setBackColor(color_t color)
{
	DC.a.backColor = color;
	DC.intBackColor = color | (color << 8) | (color << 16) | (color << 24);
}

/****************************************************************************
 *
 * Optimized packed pixel functions all written in C for non-intel CPU's.
 * The functions are implemented in assembler for Intel CPU's, but are
 * written in C for non-intel CPU's using optimized C code. In order to
 * obtain the absolute maximum performance on target machines, some of
 * these functions could be re-coded in assembler for the target platform.
 *
 ***************************************************************************/

#if defined(NO_ASSEMBLER)

color_t _ASMAPI PACKED8_getPixel(int x,int y)
/****************************************************************************
*
* Function:		PACKED8_getPixel
* Parameters:	x,y	- Coordinate to read pixel from
* Returns:		Color of the pixel at the specified coordinate
*
****************************************************************************/
{
	return *((uchar*)PACKED8_pixelAddr(x,y));
}

void _ASMAPI PACKED8_putPixel(int x,int y)
/****************************************************************************
*
* Function:		PACKED8_putPixel
* Parameters:	x,y	- Coordinate to read pixel from
*
****************************************************************************/
{
	uchar	*s;

	s = PACKED8_pixelAddr(x,y);
	if (DC.a.writeMode == MGL_REPLACE_MODE)
		*s = (uchar)DC.intColor;
	else if (DC.a.writeMode == MGL_XOR_MODE)
		*s ^= (uchar)DC.intColor;
	else if (DC.a.writeMode == MGL_OR_MODE)
		*s |= (uchar)DC.intColor;
	else
		*s &= (uchar)DC.intColor;
}

#ifndef	MGL_LITE

void _ASMAPI PACKED8_ditherPixel(int x,int y)
/****************************************************************************
*
* Function:		PACKED8_ditherPixel
* Parameters:	x,y	- Coordinate to read pixel from
*
****************************************************************************/
{
	MGL_halfTonePixelFast2(*(uchar*)PACKED8_pixelAddr(x,y),x,y,DC.intColor);
}

#endif

void _ASMAPI PACKED8_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
	int bufx,int op)
/****************************************************************************
*
* Function:		PACKED8_getScanLine
* Parameters:	dc		- Device context
*				y		- Scanline to get data from
*				x1		- Starting coordinate in scanline
*				y2		- Ending coordinate in scanline
*				buffer	- Pointer to buffer to store data in
*				bufx	- X coordinate in buffer (ignored)
*				op		- Write mode op to use during transfer
*
* Description:  Reads a scanline of information from the device context
*				into the memory buffer. Note that this routine _always_
*				stores data in the memory buffer in packed pixel format.
*
*				The write mode operation determines how the data will be
*				written into the memory buffer.
*
****************************************************************************/
{
	int		numPixels = (x2 - x1);
	uchar	*s,*p = buffer;

	s = PACKED8_pixelAddr2(dc,x1,y);
	if (op == MGL_REPLACE_MODE)
		memcpy(p,s,numPixels);
	else if (op  == MGL_XOR_MODE) {
		while (numPixels--)
			*p++ ^= *s++;
		}
	else if (op  == MGL_OR_MODE) {
		while (numPixels--)
			*p++ |= *s++;
		}
	else {
		while (numPixels--)
			*p++ &= *s++;
		}
	bufx = bufx;
}

void _ASMAPI PACKED8_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
	int bufx,int op)
/****************************************************************************
*
* Function:		PACKED8_putScanLine
* Parameters:	dc		- Device context
*				y		- Scanline to get data from
*				x1		- Starting coordinate in scanline
*				y2		- Ending coordinate in scanline
*				buffer	- Pointer to buffer to get data from
*				bufx	- X coordinate in buffer (ignored)
*				op		- Write mode op to use during transfer
*
* Description:  Dumps a scanline of information to the device context
*				from the memory buffer. Note that this routine _always_
*				reads data from the memory buffer in packed pixel format.
*
*				The write mode operation determines how the data will be
*				written into the display context.
*
****************************************************************************/
{
	int		numPixels = (x2 - x1);
	uchar	*s = buffer,*p;

	p = PACKED8_pixelAddr2(dc,x1,y);
	if (op  == MGL_REPLACE_MODE)
		memcpy(p,s,numPixels);
	else if (op  == MGL_XOR_MODE) {
		while (numPixels--)
			*p++ ^= *s++;
		}
	else if (op  == MGL_OR_MODE) {
		while (numPixels--)
			*p++ |= *s++;
		}
	else {
		while (numPixels--)
			*p++ &= *s++;
		}
	bufx = bufx;
}

void _ASMAPI PACKED8_line(fix32_t x1,fix32_t y1,fix32_t x2,
	fix32_t y2)
/****************************************************************************
*
* Function:		PACKED8_line
* Parameters:	x1,y1	- First endpoint for line
*				x2,y2	- Second endpoint for line
*
* Description:  Renders a line using the line engine and a local putPixel
*				routine (non-clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		if(DC.a.writeMode == MGL_REPLACE_MODE) {
			while (count--) {
				*fbuf++ = (uchar)DC.intColor;
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					BINC(fbuf,fincr);	/* Increment framebuffer			*/
					}
				}
			}
		else if(DC.a.writeMode == MGL_XOR_MODE) {
			while (count--) {
				*fbuf++ ^= (uchar)DC.intColor;
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					BINC(fbuf,fincr);	/* Increment framebuffer			*/
					}
				}
			}
		else if(DC.a.writeMode == MGL_OR_MODE) {
			while (count--) {
				*fbuf++ |= (uchar)DC.intColor;
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					BINC(fbuf,fincr);	/* Increment framebuffer			*/
					}
				}
			}
		else {
			while (count--) {
				*fbuf++ &= (uchar)DC.intColor;
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					BINC(fbuf,fincr);	/* Increment framebuffer			*/
					}
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		if(DC.a.writeMode == MGL_REPLACE_MODE) {
			while (count--) {
				*fbuf = (uchar)DC.intColor;
				BINC(fbuf,fincr);		/* Increment framebuffer			*/
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					fbuf++;
					}
				}
			}
		else if(DC.a.writeMode == MGL_XOR_MODE) {
			while (count--) {
				*fbuf ^= (uchar)DC.intColor;
				BINC(fbuf,fincr);		/* Increment framebuffer			*/
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					fbuf++;
					}
				}
			}
		else if(DC.a.writeMode == MGL_OR_MODE) {
			while (count--) {
				*fbuf |= (uchar)DC.intColor;
				BINC(fbuf,fincr);		/* Increment framebuffer			*/
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					fbuf++;
					}
				}
			}
		else {
			while (count--) {
				*fbuf &= (uchar)DC.intColor;
				BINC(fbuf,fincr);		/* Increment framebuffer			*/
				if (d <= 0) {
					d += Eincr;			/* Choose the Eastern Pixel			*/
					}
				else {
					d += NEincr;		/* Choose the North Eastern Pixel	*/
					fbuf++;
					}
				}
			}
		}
}

void _ASMAPI PACKED8_solidScanLine(int y,int x1,int x2)
/****************************************************************************
*
* Function:		PACKED8_solidScanLine
* Parameters:	y		- Scanline to draw line at
*				x1,x2	- Extent of the scanline to draw
*
* Description:	Draws a scanline by plotting each pixel on the scanline
*				separately.
*
****************************************************************************/
{
	int		i,count,numPixels;
	uchar	*d;

	if (x1 == x2)
		return;
	if (x2 < x1)
		SWAPT(x1,x2,i);
	d = PACKED8_pixelAddr(x1,y);
	if ((numPixels = (x2-x1)) >= 8) {
		/* Force DWORD alignment */
		while (((uint)d & 3) != 0) {
			*d++ = (uchar)DC.intColor;
			numPixels--;
			}
		count = numPixels >> 2;
		numPixels &= 3;
		while (count--) {
			*((ulong*)d) = DC.intColor;
			d += 4;
			}
		}
	while (numPixels--)
		*d++ = (uchar)DC.intColor;
}

void _ASMAPI PACKED8_ropSolidScanLine(int y,int x1,int x2)
/****************************************************************************
*
* Function:		PACKED8_ropSolidScanLine
* Parameters:	y		- Scanline to draw line at
*				x1,x2	- Extent of the scanline to draw
*
* Description:	Draws a scanline by plotting each pixel on the scanline
*				separately.
*
****************************************************************************/
{
	int		i,numPixels;
	uchar	*d;

	if (x1 == x2)
		return;
	if (x2 < x1)
		SWAPT(x1,x2,i);
	d = PACKED8_pixelAddr(x1,y);
	numPixels = (x2-x1);
	if (DC.a.writeMode == MGL_XOR_MODE) {
		while (numPixels--)
			*d++ ^= (uchar)DC.intColor;
		}
	else if (DC.a.writeMode == MGL_OR_MODE) {
		while (numPixels--)
			*d++ |= (uchar)DC.intColor;
		}
	else {
		while (numPixels--)
			*d++ &= (uchar)DC.intColor;
		}
}

void _ASMAPI PACKED8_pattScanLine(int y,int x1,int x2)
/****************************************************************************
*
* Function:		PACKED8_pattScanLine
* Parameters:	y		- Scanline to draw line at
*				x1,x2	- Extent of the scanline to draw
*
* Description:	Draws a scanline by plotting each pixel on the scanline
*				separately.
*
****************************************************************************/
{
	int		i;
	uchar	*d,p,pattern;

	if (x1 == x2)
		return;
	if (x2 < x1)
		SWAPT(x1,x2,i);
	d = PACKED8_pixelAddr(x1,y);
	pattern = DC.a.penPat[y & 7];
	p = pattern << (x1 & 7);
	if (DC.a.penStyle == MGL_BITMAP_TRANSPARENT) {
		/* Transparent bitmap pattern fill */
		if (DC.a.writeMode == MGL_REPLACE_MODE) {
			/* Replace write mode */
			for (i = x1; i < x2; i++) {
				if ((i & 7) == 0)
					p = pattern;
				if (p & 0x80)
					*d = (uchar)DC.intColor;
				d++;
				p <<= 1;
				}
			}
		else {
			/* Other write modes */
			for (i = x1; i < x2; i++) {
				if ((i & 7) == 0)
					p = pattern;
				if (p & 0x80) {
					if (DC.a.writeMode == MGL_XOR_MODE)
						*d ^= (uchar)DC.intColor;
					else if (DC.a.writeMode == MGL_OR_MODE)
						*d |= (uchar)DC.intColor;
					else
						*d &= (uchar)DC.intColor;
					}
				d++;
				p <<= 1;
				}
			}
		}
	else {
		/* Opaque bitmap pattern fill */
		if (DC.a.writeMode == MGL_REPLACE_MODE) {
			/* Replace write mode */
			for (i = x1; i < x2; i++) {
				if ((i & 7) == 0)
					p = pattern;
				if (p & 0x80)
					*d++ = (uchar)DC.intColor;
				else
					*d++ = (uchar)DC.intBackColor;
				p <<= 1;
				}
			}
		else {
			/* Other write modes */
			for (i = x1; i < x2; i++) {
				if ((i & 7) == 0)
					p = pattern;
				if (p & 0x80) {
					if (DC.a.writeMode == MGL_XOR_MODE)
						*d++ ^= (uchar)DC.intColor;
					else if (DC.a.writeMode == MGL_OR_MODE)
						*d++ |= (uchar)DC.intColor;
					else
						*d++ &= (uchar)DC.intColor;
					}
				else {
					if (DC.a.writeMode == MGL_XOR_MODE)
						*d++ ^= (uchar)DC.intBackColor;
					else if (DC.a.writeMode == MGL_OR_MODE)
						*d++ |= (uchar)DC.intBackColor;
					else
						*d++ &= (uchar)DC.intBackColor;
					}
				p <<= 1;
				}
			}
		}
}

void _ASMAPI PACKED8_ditherScanLine(int y,int x1,int x2)
/****************************************************************************
*
* Function:		PACKED8_ditherScanLine
* Parameters:	y		- Scanline to draw line at
*				x1,x2	- Extent of the scanline to draw
*
* Description:	Draws a scanline by plotting each pixel on the scanline
*				separately. Works with patterns fills etc.
*
****************************************************************************/
{
	int		i;
	uchar	*d;

	if (x1 == x2)
		return;
	if (x2 < x1)
		SWAPT(x1,x2,i);
	d = PACKED8_pixelAddr(x1,y);
	for (i = x1; i < x2; i++) {
		MGL_halfTonePixelFast2(*d,i,y,DC.intColor);
		d++;
		}
}

void _ASMAPI PACKED8_solidFillRect(int x1,int y1,int x2,int y2)
/****************************************************************************
*
* Function:		PACKED8_solidFillRect
* Parameters:	x1,y1	- Top left coordinate of rectangle
*				x2,y2	- Bottom right coordinate of rectangle
*
* Description:	Fills a rectangle.
*
****************************************************************************/
{
	int		y,dInc = DC.mi.bytesPerLine - (x2-x1);
	uchar	*d;

	d = PACKED8_pixelAddr(x1,y1);
	for (y = y1; y < y2; y++) {
		int count,numPixels;
		numPixels = (x2-x1);
		if (numPixels >= 8) {
			/* Force DWORD alignment */
			while (((uint)d & 3) != 0) {
				*d++ = (uchar)DC.intColor;
				if ((numPixels--) == 0)
					return;
				}
			count = numPixels >> 2;
			numPixels &= 3;
			while (count--) {
				*((ulong*)d) = DC.intColor;
				d += 4;
				}
			}
		while (numPixels--)
			*d++ = (uchar)DC.intColor;
		d += dInc;
		}
}

void _ASMAPI PACKED8_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,
	int height,uchar *image)
/****************************************************************************
*
* Function:		PACKED8_putMonoImage
* Parameters:	dc		- Device context
* Parameters:	x			- X coordinate to place image at
*				y			- Y coordinate to place image at
*				byteWidth	- Width of the image in bytes (8 pixels)
*				height		- Height of the image
*				image		- Pointer to the image data
*
* Description:	Draws a monochromatic image at the position (x,y) using
*				the definition provided by the data pointed to by 'image'.
*				The bitmap image definition should be width*height bytes in
*				size. Where a bit is a 1 in the image defintion, a pixel
*				will be plotted in the current foreground color. Where a bit
*				is a zero, the data is left unchanged.
*
*				We emulate this by repeatedly calling the MGL_pixelCoord
*				routine (to ensure correct clipping).
*
*				The image will be drawn in the current drawing color and
*				in the current write mode.
*
****************************************************************************/
{
	rect_t	clipRect;
	int		width,bottom,right,dInc,op;
	uchar	*d,*dScanEnd,*dEnd,byte,color;

	/* Setup initial variables */
	clipRect = dc->intClipRect;
	right = x + (width = byteWidth << 3);
	bottom = y + height;
	color = (uchar)dc->intColor;
	d = PACKED8_pixelAddr2(dc,x,y);
	dEnd = d + ((height-1) * dc->mi.bytesPerLine) + width;
	dInc = dc->mi.bytesPerLine - width;
	op = dc->a.writeMode;

	/* Check if image needs to be clipped */
	if ((y < clipRect.top) || (bottom > clipRect.bottom) ||
		(x < clipRect.left) || (right > clipRect.right)) {
		/* Draw clipped version of image (all ROP's) */
		while (d < dEnd) {
			int	i = x;
			dScanEnd = d + width;
			while (d < dScanEnd) {
				int	j = 0;
				byte = *image++;
				while (j < 8) {
					if (byte & 0x80) {
						if (MGL_ptInRectCoord(i,y,clipRect)) {
							if (op == MGL_REPLACE_MODE)
								*d = color;
							else if (op == MGL_XOR_MODE)
								*d ^= color;
							else if (op == MGL_OR_MODE)
								*d |= color;
							else
								*d &= color;
							}
						}
					byte <<= 1;
					j++;
					i++;
					d++;
					}
				}
			d += dInc;
			y++;
			}
		}
	else {
		/* Draw unclipped version of image */
		if (op == MGL_REPLACE_MODE) {
			/* Fast replace mode version */
			while (d < dEnd) {
				dScanEnd = d + width;
				while (d < dScanEnd) {
					int	j = 0;
					byte = *image++;
					while (j < 8) {
						if (byte & 0x80)
							*d = color;
						byte <<= 1;
						j++;
						d++;
						}
					}
				d += dInc;
				}
			}
		else {
			/* Slower XOR/AND/OR mode version */
			while (d < dEnd) {
				dScanEnd = d + width;
				while (d < dScanEnd) {
					int	j = 0;
					byte = *image++;
					while (j < 8) {
						if (byte & 0x80) {
							if (op == MGL_XOR_MODE)
								*d ^= color;
							else if (op == MGL_OR_MODE)
								*d |= color;
							else
								*d &= color;
							}
						byte <<= 1;
						j++;
						d++;
						}
					}
				d += dInc;
				}
			}
		}
}

void _ASMAPI PACKED8_putImage(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,void *surface,int bytesPerLine, MGLDC *src)
/****************************************************************************
*
* Function:		PACKED8_putImage
* Parameters:	dc				- Device context to Blt to
* 				left			- Left coordinate of are to Blt from
*				top				- Top coordinate of image to Blt from
*				right			- Right coordinate of image to Blt from
*				bottom			- Bottom coordinate of image to Blt from
*				dstLeft			- Left coordinate to Blt to
*				dstTop			- Right coordinate to Blt to
*				op				- Write mode to use during Blt
*				surface			- Surface to blt image from
*               bytesPerLine	- bytesPerLine value for surface
*
* Description:	Blt an image from a memory buffer onto the specified
*				device context. The information in the memory buffer is in
*				packed pixel format, so the underlying device driver may
*				need to do some translation if the mode is in a planar
*				format.
*
****************************************************************************/
{
	uchar	*s,*d;
	int		sInc,dInc;
	int		width = (right - left);
	int		height = (bottom - top);

	s = PACKED8_pixelAddr3(src,left,top,surface,bytesPerLine);
	sInc = bytesPerLine;
	d = PACKED8_pixelAddr2(dc,dstLeft,dstTop);
	dInc = dc->mi.bytesPerLine;

	if (op == MGL_REPLACE_MODE) {
		/* Fast replace mode blit */
		while (height--) {
			memcpy(d,s,width);
			s += sInc;
			d += dInc;
			}
		}
	else {
		/* Slower ROP code blits */
		sInc -= width;
		dInc -= width;
		while (height--) {
			int	numPixels = width;
			if (op  == MGL_XOR_MODE) {
				while (numPixels--)
					*d++ ^= *s++;
				}
			else if (op  == MGL_OR_MODE) {
				while (numPixels--)
					*d++ |= *s++;
				}
			else {
				while (numPixels--)
					*d++ &= *s++;
				}
			s += sInc;
			d += dInc;
			}
		}
	src = src;
}

void _ASMAPI PACKED8_getDivot(MGLDC *dc,int left,int top,int right,int bottom,
	void *divot)
/****************************************************************************
*
* Function:		PACKED8_getDivot
* Parameters:	dc		- Device context
* 				left	- Left coordinate of image
*				top		- Top coordinate of image
*				right	- Right coordinate of image
*				bottom	- Bottom coordinate of image
*               divot	- Place to store the image data
*
* Description:	Gets a block of video data from the current video page into
*				system RAM. 'divot' MUST have been allocated with enough
*				room to store the image. Use MGL_divotSize() to compute
*				the size of the image. This routine saves the entire
*				byte boundary block of video data.
*
****************************************************************************/
{
	uchar		*s,*d;
	int			sInc,dInc;
	int			height = (bottom - top);
	divotHeader	hdr;

	/* Store header information */
	hdr.left = left;	hdr.top = top;
	hdr.right = right;	hdr.bottom = bottom;
	hdr.bytesPerLine = (right - left);
	s = PACKED8_pixelAddr2(dc,left,top);
	sInc = dc->mi.bytesPerLine;
	d = divot;
	dInc = hdr.bytesPerLine;
	memcpy(d,&hdr,sizeof(hdr));
	d += sizeof(hdr);

	while (height--) {
		memcpy(d,s,hdr.bytesPerLine);
		s += sInc;
		d += dInc;
		}
}

void _ASMAPI PACKED8_putDivot(MGLDC *dc,void *divot)
/****************************************************************************
*
* Function:		PACKED8_putDivot
* Parameters:	dc		- Device context
* 				divot	- Buffer containing image block to use
*
* Description:	Moves a block of video data from system RAM onto the current
*				display page. The image is restored to exactly the same
*				position that it was saved from.
*
*				NOTE: These coordinate CANNOT be viewport relative.
*
****************************************************************************/
{
	uchar		*s,*d;
	int			sInc,dInc;
	divotHeader	hdr;
	int			height;

	/* Extract header information */
	s = divot;
	memcpy(&hdr,s,sizeof(hdr));
	s += sizeof(hdr);
	sInc = hdr.bytesPerLine;
	height = (hdr.bottom - hdr.top);
	d = PACKED8_pixelAddr2(dc,hdr.left,hdr.top);
	dInc = dc->mi.bytesPerLine;

	while (height--) {
		memcpy(d,s,hdr.bytesPerLine);
		s += sInc;
		d += dInc;
		}
}

#ifndef	MGL_LITE

void _ASMAPI PACKED8_ditherLine(fix32_t x1,fix32_t y1,fix32_t x2,
	fix32_t y2)
/****************************************************************************
*
* Function:		PACKED8_ditherLine
* Parameters:	x1,y1	- First endpoint for line
*				x2,y2	- Second endpoint for line
*
* Description:  Renders a line using the line engine and a local putPixel
*				routine (non-clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		while (count--) {
			MGL_halfTonePixelFast2(*fbuf,x,y,DC.intColor);
			fbuf++; 				/* Increment x coordinate			*/
			x++;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		while (count--) {
			MGL_halfTonePixelFast2(*fbuf,x,y,DC.intColor);
			y += yincr;				/* Increment y coordinate			*/
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				x++;
				}
			}
		}
}

void _ASMAPI PACKED8_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,
	fix32_t y2,fix32_t c2)
/****************************************************************************
*
* Function:		PACKED8_cLine
* Parameters:	x1,y1,c1	- First endpoint for line
*				x2,y2,c2	- Second endpoint for line
*
* Description:  Renders a shaded line (non clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		dc;					/* Color gradients					*/
	fix32_t		cSlope;				/* Color slopes					*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment values					*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	dc = c2 - c1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);

		while (count--) {
			*fbuf++ = MGL_FIXTOINT(c1);
			c1 += cSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf += fincr;		/* Increment framebuffer			*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);

		while (count--) {
			*fbuf = MGL_FIXTOINT(c1);
			fbuf += fincr;			/* Incremene framebuffer			*/
			c1 += cSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,
	fix32_t y2,zfix32_t z2)
/****************************************************************************
*
* Function:		PACKED8_zLine16
* Parameters:	x1,y1,z1	- First endpoint for line
*				x2,y2,z2	- Second endpoint for line
*
* Description:  Scan convert a 3D line segment using the MidPoint Digital
*				Differential Analyser algorithm, including Zbuffer
*				coordinates. We call the DC.putPixel code directly to
*				render each pixel.
*
*				Lines must ALWAYS be passed with the X1 value less than the
*				X2 value!!
*
*				This routine expects the endpoints to be in 32 bit fixed
*				point 16.16 format and Z coordinates to be in 4.28 fixed
*				point format.
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf16_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf16_t	z,zSlope;			/* Current position in Z buffer		*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr16(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute coordinate slopes */
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				*fbuf = (uchar)DC.intColor;
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				*fbuf = (uchar)DC.intColor;
				}
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_zLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,
	fix32_t y2,zfix32_t z2)
/****************************************************************************
*
* Function:		PACKED8_zLine32
* Parameters:	x1,y1,z1	- First endpoint for line
*				x2,y2,z2	- Second endpoint for line
*
* Description:  Scan convert a 3D line segment using the MidPoint Digital
*				Differential Analyser algorithm, including Zbuffer
*				coordinates. We call the DC.putPixel code directly to
*				render each pixel.
*
*				Lines must ALWAYS be passed with the X1 value less than the
*				X2 value!!
*
*				This routine expects the endpoints to be in 32 bit fixed
*				point 16.16 format and Z coordinates to be in 4.28 fixed
*				point format.
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf32_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf32_t	zSlope;				/* Current position in Z buffer		*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr32(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				*fbuf = (uchar)DC.intColor;
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				*fbuf = (uchar)DC.intColor;
				}
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_czLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,
	fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2)
/****************************************************************************
*
* Function:		PACKED8_czLine16
* Parameters:	x1,y1,z1,c1	- First endpoint for line
*				x2,y2,z2,c2	- Second endpoint for line
*
* Description:  Renders a shaded zbuffered line (non clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		dc;					/* Color gradients					*/
	fix32_t		cSlope;				/* Color slopes						*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf16_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf16_t	z,zSlope;			/* Current position in Z buffer		*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	dc = c2 - c1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr16(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				*fbuf = (uchar)MGL_FIXTOINT(c1);
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			c1 += cSlope;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				*fbuf = (uchar)MGL_FIXTOINT(c1);
				}
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			c1 += cSlope;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_czLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,
	fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2)
/****************************************************************************
*
* Function:		PACKED8_czLine32
* Parameters:	x1,y1,z1,c1	- First endpoint for line
*				x2,y2,z2,c2	- Second endpoint for line
*
* Description:  Renders a shaded zbuffered line (non clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		dc;					/* Color gradients					*/
	fix32_t		cSlope;				/* Color slopes						*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf32_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf32_t	zSlope;				/* Current position in Z buffer		*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	dc = c2 - c1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr32(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				*fbuf = (uint)MGL_FIXTOINT(c1);
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			c1 += cSlope;
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				*fbuf = (uchar)MGL_FIXTOINT(c1);
				}
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			c1 += cSlope;
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_solidDrawScanList(int y,int length,short *scans)
/****************************************************************************
*
* Function:		PACKED8_solidDrawScanList
* Parameters:	y		- Y coordinate to begin rendering
*				length	- Number of scanlines in the list
*				scans	- Pointer to array of scanlines to render
*
* Description:	Renders the passed in list of scanlines beginning at
*				the specified y coordinate in the current drawing
*				attributes. This routine performs NO clipping.
*
****************************************************************************/
{
	uchar	*dStart;

	dStart = PACKED8_pixelAddr(0,y);
	while (length--) {
		int		i,x1 = scans[0],x2 = scans[1];
		int		count,numPixels;
		uchar	*d;

		if (x1 == x2)
			continue;
		if (x2 < x1)
			SWAPT(x1,x2,i);
		d = dStart + x1;
		if ((numPixels = (x2-x1)) >= 8) {
			/* Force DWORD alignment */
			while (((uint)d & 3) != 0) {
				*d++ = (uchar)DC.intColor;
				numPixels--;
				}
			count = numPixels >> 2;
			numPixels &= 3;
			while (count--) {
				*((ulong*)d) = DC.intColor;
				d += 4;
				}
			}
		while (numPixels--)
			*d++ = (uchar)DC.intColor;

		dStart += DC.mi.bytesPerLine;
		scans += 2;
		}
}

void _ASMAPI PACKED8_translateImage(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src)
/****************************************************************************
*
* Function:		PACKED8_translateImage
* Parameters:	dc				- Device context to Blt to
* 				left			- Left coordinate of are to Blt from
*				top				- Top coordinate of image to Blt from
*				right			- Right coordinate of image to Blt from
*				bottom			- Bottom coordinate of image to Blt from
*				dstLeft			- Left coordinate to Blt to
*				dstTop			- Right coordinate to Blt to
*				op				- Write mode to use during Blt
*				surface			- Surface to blt image from
*               bytesPerLine	- bytesPerLine value for surface
*				bitsPerPixel	- Pixel depth for source buffer
*				pal				- Palette for the source image
*				pf				- Pixel format for the image
*
* Description:	Blt and image from a memory buffer onto the specified
*				device context, translating the pixel format on the fly.
*				The information in the memory buffer is always in packed
*				pixel format.
*
*				NOTE: We only ever get to this routine if the pixel formats
*					  for the source and destination contexts are different.
*					  If they are the same it will be handled by directly
*					  copying the pixel information.
*
****************************************************************************/
{
	uchar	*s,*d;
	int		y,height,width;
	int		sInc,dInc;

	if (bitsPerPixel != 24 || pf->redPos == 0) {
		/* Pass to emulation code for non 24bpp RGB format images */
		__EMU__translateImage(dc,left,top,right,bottom,dstLeft,dstTop,op,
			surface,bytesPerLine,bitsPerPixel,pal,pf,src);
		return;
		}

	d = PACKED8_pixelAddr2(dc,dstLeft,dstTop);
	s = PACKED24_pixelAddr2(src,left,top);
	height = bottom - top;
	width = right - left;
	sInc = src->mi.bytesPerLine - width*3;
	dInc = dc->mi.bytesPerLine - width;
	y = dstTop;
	while (height--) {
		int count = width;
		while (count--) {
			MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
			s += 3;
			}
		y++;
		s += sInc;
		d += dInc;
		}
}

void _ASMAPI PACKED8_srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
/****************************************************************************
*
* Function:		PACKED8_srcTransBlt
* Parameters:	dst			- Destination Device context
*				src			- Source device context
* 				left		- Left coordinate of are to Blt from
*				top			- Top coordinate of image to Blt from
*				right		- Right coordinate of image to Blt from
*				bottom		- Bottom coordinate of image to Blt from
*				dstLeft		- Left coordinate to Blt to
*				dstTop		- Right coordinate to Blt to
*				transparent	- Transparent color
*
****************************************************************************/
{
	uchar	*s,*d;
	int		height,width;
	int		sInc,dInc;

	d = PACKED8_pixelAddr2(dst,dstLeft,dstTop);
	s = PACKED8_pixelAddr2(src,left,top);
	height = bottom - top;
	width = right - left;
	sInc = src->mi.bytesPerLine - width;
	dInc = dst->mi.bytesPerLine - width;
	while (height--) {
		int count = width;
		while (count--) {
			if (*s != (uchar)transparent)
				*d = *s;
			d++;
			s++;
			}
		s += sInc;
		d += dInc;
		}
	op = op;
}

void _ASMAPI PACKED8_dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
/****************************************************************************
*
* Function:		PACKED8_dstTransBlt
* Parameters:	dst			- Destination Device context
*				src			- Source device context
* 				left		- Left coordinate of are to Blt from
*				top			- Top coordinate of image to Blt from
*				right		- Right coordinate of image to Blt from
*				bottom		- Bottom coordinate of image to Blt from
*				dstLeft		- Left coordinate to Blt to
*				dstTop		- Right coordinate to Blt to
*				transparent	- Transparent color
*
****************************************************************************/
{
	uchar	*s,*d;
	int		height,width;
	int		sInc,dInc;

	d = PACKED8_pixelAddr2(dst,dstLeft,dstTop);
	s = PACKED8_pixelAddr2(src,left,top);
	height = bottom - top;
	width = right - left;
	sInc = src->mi.bytesPerLine - width;
	dInc = dst->mi.bytesPerLine - width;
	while (height--) {
		int count = width;
		while (count--) {
			if (*d == (uchar)transparent)
				*d = *s;
			d++;
			s++;
			}
		s += sInc;
		d += dInc;
		}
	op = op;
}

int _ASMAPI PACKED8_scanRightForColor(int x,int y,color_t color)
/****************************************************************************
*
* Function:		PACKED8_scanRightForColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to scan for
* Returns:      Coordinate of pixel, or XRes+1 if not found
*
* Description:	Scans right in the device context from the specified
*				location for the specified color value. If a pixel is
*				found, the x coordinate is returned, otherwise a value
*				of XRes+1 will be returned.
*
****************************************************************************/
{
	uchar	*p;

	p = PACKED8_pixelAddr(x,y);
	while (x <= DC.mi.xRes) {
		if (*p == (uchar)color)
			return x;
		x++;
		p++;
		}
	return x;
}

int _ASMAPI PACKED8_scanLeftForColor(int x,int y,color_t color)
/****************************************************************************
*
* Function:		PACKED8_scanLeftForColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to scan for
* Returns:      Coordinate of pixel, or -1 if not found
*
* Description:	Scans left in the device context from the specified
*				location for the specified color value. If a pixel is
*				found, the x coordinate is returned, otherwise a value
*				of -1 will be returned.
*
****************************************************************************/
{
	uchar	*p;

	p = PACKED8_pixelAddr(x,y);
	while (x >= 0) {
		if (*p == (uchar)color)
			return x;
		x--;
		p--;
		}
	return -1;
}

int _ASMAPI PACKED8_scanRightWhileColor(int x,int y,color_t color)
/****************************************************************************
*
* Function:		PACKED8_scanRightWhileColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to continue scanning on
* Returns:      Coordinate of pixel, or XRes+1 if not found
*
* Description:	Scans right in the device context from the specified
*				location looking for a change in the pixel color. If a
*				different pixel color is found, the x coordinate is
*				returned, otherwise a value of XRes+1 will be returned.
*
****************************************************************************/
{
	uchar	*p;

	p = PACKED8_pixelAddr(x,y);
	while (x <= DC.mi.xRes && (*p == (uchar)color)) {
		x++;
		p++;
		}
	return x;
}

int _ASMAPI PACKED8_scanLeftWhileColor(int x,int y,color_t color)
/****************************************************************************
*
* Function:		PACKED8_scanLeftWhileColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to continue scanning on
* Returns:      Coordinate of pixel, or -1 if not found
*
* Description:	Scans left in the device context from the specified
*				location looking for a change in the pixel color. If a
*				different pixel color is found, the x coordinate is
*				returned, otherwise a value of -1 will be returned.
*
****************************************************************************/
{
	uchar	*p;

	p = PACKED8_pixelAddr(x,y);
	while (x >= 0 && (*p == (uchar)color)) {
		x--;
		p--;
		}
	return x;
}

void _ASMAPI PACKED8_solidTrap(void)
/****************************************************************************
*
* Function:		PACKED8_solidTrap
*
* Description:	Renders a flat shaded trapezoid. The parameters for the
*				trapezoid are found in the DC.tr structure.
*
****************************************************************************/
{
	int 		dX,count,ix1,ix2;
	uchar		*fbufstart,*fbuf;

	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			}
		if (dX != 0) {
			fbuf = fbufstart + ix1;
			if (dX >= 8) {
				/* Force DWORD alignment */
				while (((uint)fbuf & 3) != 0) {
					*fbuf++ = (uchar)DC.intColor;
					dX--;
					}
				count = dX >> 2;
				dX &= 3;
				while (count--) {
					*((ulong*)fbuf) = DC.intColor;
					fbuf += 4;
					}
				}
			while (dX--)
				*fbuf++ = (uchar)DC.intColor;
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_ditherTrap(void)
/****************************************************************************
*
* Function:		PACKED8_ditherTrap
*
* Description:	Renders a flat shaded trapezoid. The parameters for the
*				trapezoid are found in the DC.tr structure.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;

	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			}
		if (dX != 0) {
			fbuf = fbufstart + ix1;
			while (dX--) {
				MGL_halfTonePixelFast2(*fbuf,(int)fbuf,DC.tr.y,DC.intColor);
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_cTrap(void)
/****************************************************************************
*
* Function:		PACKED8_cTrap
*
* Description:	Renders a smooth shaded color index trapezoid. The
*				parameters for the trapezoid are found in the DC.tr
*				structure.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	fix32_t 	dx,ic1,ic2,cSlope;

	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		ic1 = DC.tr.c1;	ic2 = DC.tr.c2;
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			SWAPT(ic1,ic2,dx);
			}
		if (dX != 0) {
			fbuf = fbufstart + ix1;
			dx = MGL_TOFIX(dX+1);
			cSlope = MGL_FixDiv(ic2 - ic1,dx);
			while (dX--) {
				*fbuf = (uchar)MGL_FIXTOINT(ic1);
				ic1 += cSlope;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.c1 += DC.tr.cSlope1;	DC.tr.c2 += DC.tr.cSlope2;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_zTrap16(void)
/****************************************************************************
*
* Function:		PACKED8_zTrap16
*
* Description:	Renders a flat shaded zbuffered trapezoid. The parameters
*				for the trapezoid are found in the DC.tr structure.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	zbuf16_t	z,z0,*zbufstart,*zbuf,*zoff;

	zbufstart = _MGL_zbufAddr16(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			}
		if (dX != 0) {
			zbuf = zbufstart + (ix1 - DC.size.left);
			fbuf = fbufstart + ix1;
			zoff = &(((zbuf16_t*)DC.zOffset)[ix1]);
			z0 = MGL_TOZBUF16(DC.tr.z0);
			while (dX--) {
				if ((z = z0 + *zoff++) > *zbuf) {
					*zbuf = z;
					*fbuf = (uchar)DC.intColor;
					}
				zbuf++;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_zTrap32(void)
/****************************************************************************
*
* Function:		PACKED8_zTrap32
*
* Description:	Renders a flat shaded zbuffered trapezoid. The parameters
*				for the trapezoid are found in the DC.tr structure.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	zbuf32_t	z,*zbufstart,*zbuf,*zoff;

	zbufstart = _MGL_zbufAddr32(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			}
		if (dX != 0) {
			zbuf = zbufstart + (ix1 - DC.size.left);
			fbuf = fbufstart + ix1;
			zoff = &(((zbuf32_t*)DC.zOffset)[ix1]);
			while (dX--) {
				if ((z = DC.tr.z0 + *zoff++) > *zbuf) {
					*zbuf = z;
					*fbuf = (uchar)DC.intColor;
					}
				zbuf++;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_czTrap16(void)
/****************************************************************************
*
* Function:		PACKED8_czTrap16
*
* Description:	Renders a smooth shaded color index zbuffered trapezoid. The
*				parameters for the trapezoid are found in the DC.tr
*				structure.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	fix32_t 	dx,ic1,ic2,cSlope;
	zbuf16_t	z,z0,*zbufstart,*zbuf,*zoff;

	zbufstart = _MGL_zbufAddr16(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		ic1 = DC.tr.c1;	ic2 = DC.tr.c2;
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			SWAPT(ic1,ic2,dx);
			}
		if (dX != 0) {
			zbuf = zbufstart + (ix1 - DC.size.left);
			fbuf = fbufstart + ix1;
			zoff = &(((zbuf16_t*)DC.zOffset)[ix1]);
			z0 = MGL_TOZBUF16(DC.tr.z0);
			dx = MGL_TOFIX(dX+1);
			cSlope = MGL_FixDiv(ic2 - ic1,dx);
			while (dX--) {
				if ((z = z0 + *zoff++) > *zbuf) {
					*zbuf = z;
					*fbuf = (uchar)MGL_FIXTOINT(ic1);
					}
				ic1 += cSlope;
				zbuf++;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.c1 += DC.tr.rSlope1;	DC.tr.c2 += DC.tr.rSlope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_czTrap32(void)
/****************************************************************************
*
* Function:		PACKED8_czTrap32
*
* Description:	Renders a smooth shaded color index zbuffered trapezoid. The
*				parameters for the trapezoid are found in the DC.tr
*				structure.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	fix32_t 	dx,ic1,ic2,cSlope;
	zbuf32_t	z,*zbufstart,*zbuf,*zoff;

	zbufstart = _MGL_zbufAddr32(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		ic1 = DC.tr.c1;	ic2 = DC.tr.c2;
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			SWAPT(ic1,ic2,dx);
			}
		if (dX != 0) {
			zbuf = zbufstart + (ix1 - DC.size.left);
			fbuf = fbufstart + ix1;
			zoff = &(((zbuf32_t*)DC.zOffset)[ix1]);
			dx = MGL_TOFIX(dX+1);
			cSlope = MGL_FixDiv(ic2 - ic1,dx);
			while (dX--) {
				if ((z = DC.tr.z0 + *zoff++) > *zbuf) {
					*zbuf = z;
					*fbuf = (uchar)MGL_FIXTOINT(ic1);
					}
				ic1 += cSlope;
				zbuf++;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.c1 += DC.tr.rSlope1;	DC.tr.c2 += DC.tr.rSlope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

#endif	/* !MGL_LITE	*/

void _ASMAPI PACKED8_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		PACKED8_stretchBlt2x2
* Parameters:	dst		- Destination Device context
*				src		- Source device context
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*
****************************************************************************/
{
	uchar	*s,*d;
	int		height,width,count;
	int		sInc,dInc;

	/* Determine special cases that require translation */
#ifndef	MGL_LITE
	if (src->mi.bitsPerPixel != dst->mi.bitsPerPixel) {
		if (src->mi.bitsPerPixel == 24 && src->pf.redPos != 0)
			PACKED8_stretchBlt2x2_24(dst,src,left,top,right,bottom,dstLeft,
				dstTop);
		else
			__EMU__stretchBlt2x2(dst,src,left,top,right,bottom,dstLeft,
				dstTop,pal,idPal);
		return;
		}
#endif

	width = right - left;
	height = bottom - top;
	s = PACKED8_pixelAddr2(src,left,top);
	sInc = src->mi.bytesPerLine;
	d = PACKED8_pixelAddr2(dst,dstLeft,dstTop);
	dInc = dst->mi.bytesPerLine - width*2;

	while (height--) {
		uchar *sp = s;

		/* Double all pixels in the first scanline */
		count = width;
		while (count--) {
			d[0] = d[1] = *sp;
			d += 2;
			sp++;
			}
		d += dInc;
		sp = s;

		/* Double all pixels in the second scanline */
		count = width;
		while (count--) {
			d[0] = d[1] = *sp;
			d += 2;
			sp++;
			}
		d += dInc;
		s += sInc;
		}
}

#endif	/* NO_ASSEMBLER */

/****************************************************************************
 *
 * Optimized packed pixel functions all written in C. These functions are
 * either not that time critical, or have extremely complicated inner
 * loops and a good C compiler can do just as good a job as we can
 * optimizing the code. Hence all the following routines are written in
 * C for all processors.
 *
 ***************************************************************************/

long _ASMAPI PACKED8_divotSize(MGLDC *dc,int left,int top,int right,
	int bottom)
/****************************************************************************
*
* Function:     PACKED8_divotSize
* Returns:		Memory required to store divot
*
* Description:	For maximum speed in the packed pixel divot code, we align
*               the left and right edges of the divot to 32 bit boundaries.
*
****************************************************************************/
{
	dc = dc;
	return (long)(((right + 3) & ~0x3) - (left & ~0x3)) *
		(long)(bottom - top) + sizeof(divotHeader);
}

/* Slow scanline stretching and transparent blt'ing code. We dont code
 * this in assembler as we code the entire stretch and transparent blt
 * code in 32 bit assembler for speed.
 */

void _ASMAPI PACKED8_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,
	void *buffer)
/****************************************************************************
*
* Function:		PACKED8_stretchScanline2x
* Parameters:	dc			- Device context
*				y			- Scanline to get data from
*				x1			- Starting coordinate in scanline
*				count		- Number of source pixels
*				buffer		- Pointer to buffer to get data from
*
* Description:	Dumps a scanline with transparency.
*
****************************************************************************/
{
	uchar	*s = buffer,*p;

	p = PACKED8_pixelAddr2(dc,x1,y);
	while (count--) {
		p[0] = p[1] = *s;
		p += 2;
		s++;
		}
}

void _ASMAPI PACKED8_stretchScanLine(MGLDC *dc,int y,int x1,int count,
	void *buffer,int *repCounts)
/****************************************************************************
*
* Function:		PACKED8_stretchScanline
* Parameters:	dc			- Device context
*				y			- Scanline to get data from
*				x1			- Starting coordinate in scanline
*				count		- Number of source pixels
*				buffer		- Pointer to buffer to get data from
*				repCounts	- Pointer to pixel replication counts buffer
*
* Description:	Dumps a scanline with transparency.
*
****************************************************************************/
{
	uchar	*s = buffer,*p;

	p = PACKED8_pixelAddr2(dc,x1,y);
	while (count--) {
		int i = *repCounts++;
		while (i--)
			*p++ = *s;
		s++;
		}
}

#ifndef	MGL_LITE

void _ASMAPI PACKED8_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,
	void *buffer,color_t transparent)
/****************************************************************************
*
* Function:		PACKED8_putSrcTransparent
* Parameters:	dc			- Device context
*				y			- Scanline to get data from
*				x1			- Starting coordinate in scanline
*				y2			- Ending coordinate in scanline
*				buffer		- Pointer to buffer to get data from
*				transparent	- Transparent color to skip
*
* Description:	Dumps a scanline with source transparency.
*
****************************************************************************/
{
	int		count = (x2 - x1);
	uchar	*s = buffer,*p,t = transparent;

	p = PACKED8_pixelAddr2(dc,x1,y);
	while (count--) {
		if (*s != t)
			*p = *s;
		p++;
		s++;
		}
}

void _ASMAPI PACKED8_putDstTransparent(MGLDC *dc,int y,int x1,int x2,
	void *buffer,color_t transparent)
/****************************************************************************
*
* Function:		PACKED8_putDstTransparent
* Parameters:	dc			- Device context
*				y			- Scanline to get data from
*				x1			- Starting coordinate in scanline
*				y2			- Ending coordinate in scanline
*				buffer		- Pointer to buffer to get data from
*				transparent	- Transparent color to skip
*
* Description:	Dumps a scanline with destination transparency.
*
****************************************************************************/
{
	int		count = (x2 - x1);
	uchar	*s = buffer,*p,t = transparent;

	p = PACKED8_pixelAddr2(dc,x1,y);
	while (count--) {
		if (*p == t)
			*p = *s;
		p++;
		s++;
		}
}

#endif	/* !MGL_LITE */

void _ASMAPI PACKED8_getImage(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,void *surface,int bytesPerLine)
/****************************************************************************
*
* Function:		PACKED8_getImage
* Parameters:	dc				- Device context to Blt from
* 				left			- Left coordinate of are to Blt from
*				top				- Top coordinate of image to Blt from
*				right			- Right coordinate of image to Blt from
*				bottom			- Bottom coordinate of image to Blt from
*				dstLeft			- Left coordinate to Blt to
*				dstTop			- Right coordinate to Blt to
*				op				- Write mode to use during Blt
*				surface			- Surface to blt image to
*               bytesPerLine	- bytesPerLine value for surface
*
* Description:  Blt an image from the device context into the specified
*				memory device context. The memory device context will be
*				the same pixel depth as the display device in packed pixel
*				format. This routine may be required to perform translation
*				from planar to packed pixel format internally.
*
****************************************************************************/
{
	uchar	*s,*d;
	int		sInc,dInc;
	int		width = (right - left);
	int		height = (bottom - top);

	s = PACKED8_pixelAddr2(dc,left,top);
	sInc = dc->mi.bytesPerLine;
	d = PACKED8_pixelAddr3(src,dstLeft,dstTop,surface,bytesPerLine);
	dInc = bytesPerLine;

	if (op == MGL_REPLACE_MODE) {
		/* Fast replace mode blit */
		while (height--) {
			memcpy(d,s,width);
			s += sInc;
			d += dInc;
			}
		}
	else {
		/* Slower ROP code blits */
		sInc -= width;
		dInc -= width;
		while (height--) {
			int	numPixels = width;
			if (op  == MGL_XOR_MODE) {
				while (numPixels--)
					*d++ ^= *s++;
				}
			else if (op  == MGL_OR_MODE) {
				while (numPixels--)
					*d++ |= *s++;
				}
			else {
				while (numPixels--)
					*d++ &= *s++;
				}
			s += sInc;
			d += dInc;
			}
		}
}

#ifndef	MGL_LITE

void _ASMAPI PACKED8_stretchBlt2x2_24(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop)
/****************************************************************************
*
* Function:		PACKED8_stretchBlt2x2_24
* Parameters:	dst		- Destination Device context
*				src		- Source device context
* 				left	- Left coordinate of are to Blt from
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*
* Description:	Fast 2x2 stretch blt for 24 bit pixels to the 8 bit packed
*				pixel device. We are doing a lot of dithering, and a good
*				32 bit C compiler can actually produce faster code than I
*				have been able to do directly in assembler!
*
****************************************************************************/
{
	int		y,count,xcount = right - left;
	int		srcBytes = src->mi.bytesPerLine;
	int		dstBytes = dst->mi.bytesPerLine;
	uchar	*dBegin,*sBegin;

	/* Stretch each scanline */
	y = dstTop;
	count = bottom - top;
	dBegin = PACKED8_pixelAddr2(dst,dstLeft,dstTop);
	sBegin = PACKED24_pixelAddr2(src,left,top);
	while (count--) {
		uchar *s = sBegin;
		uchar *d = dBegin;
		int count = xcount;
		while (count--) {
			MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
			MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
			s += 3;
			}
		y++;
		dBegin += dstBytes;
		s = sBegin;
		d = dBegin;
		count = xcount;
		while (count--) {
			MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
			MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
			s += 3;
			}
		y++;
		dBegin += dstBytes;
		sBegin += srcBytes;
		}
}

void _ASMAPI PACKED8_colorPattScanLine(int y,int x1,int x2)
/****************************************************************************
*
* Function:		PACKED8_colorPattScanLine
* Parameters:	y		- Scanline to draw line at
*				x1,x2	- Extent of the scanline to draw
*
* Description:	Draws a scanline by plotting each pixel on the scanline
*				separately. Color pattern fill version.
*
****************************************************************************/
{
	int		i;
	uchar	*d;
	color_t	*p,*pattern;

	if (x1 == x2)
		return;
	if (x2 < x1)
		SWAPT(x1,x2,i);
	pattern = &DC.a.penPixPat[y&7][0];
	d = PACKED8_pixelAddr(x1,y);
	p = &pattern[x1&7];
	for (i = x1; i < x2; i++) {
		if ((i & 7) == 0)
			p = pattern;
		*d++ = *p++;
		}
}

void _ASMAPI PACKED8_stretchBlt1x2_24(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop)
/****************************************************************************
*
* Function:		PACKED8_stretchBlt1x2_24
* Parameters:	dst		- Destination Device context
*				src		- Source device context
* 				left	- Left coordinate of are to Blt from
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*
* Description:	Fast 1x2 stretch blt for 24 bit pixels to the 8 bit packed
*				pixel device. We are doing a lot of dithering, and a good
*				32 bit C compiler can actually produce faster code than I
*				have been able to do directly in assembler!
*
****************************************************************************/
{
	int		y,count,xcount = right - left;
	int		srcBytes = src->mi.bytesPerLine;
	int		dstBytes = dst->mi.bytesPerLine;
	uchar	*dBegin,*sBegin;

	/* Stretch each scanline */
	y = dstTop;
	count = bottom - top;
	dBegin = PACKED8_pixelAddr2(dst,dstLeft,dstTop);
	sBegin = PACKED24_pixelAddr2(src,left,top);
	while (count--) {
		uchar *s = sBegin;
		uchar *d = dBegin;
		int count = xcount;
		while (count--) {
			MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
			s += 3;
			}
		y++;
		dBegin += dstBytes;
		s = sBegin;
		d = dBegin;
		count = xcount;
		while (count--) {
			MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
			s += 3;
			}
		y++;
		dBegin += dstBytes;
		sBegin += srcBytes;
		}
}

void _ASMAPI PACKED8_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
/****************************************************************************
*
* Function:		PACKED8_stippleLine
* Parameters:	x1,y1	- First endpoint for line
*				x2,y2	- Second endpoint for line
*
* Description:  Renders a single pixel wide stippled line direct to the
*				framebuffer. Eventually this should be recoded in assembler,
*				but this will do for the time being.
*
****************************************************************************/
{
	fix32_t	d;						/* Decision variable				*/
	fix32_t	dx,dy;					/* Dx and Dy values for the line	*/
	fix32_t	Eincr,NEincr;			/* Decision variable increments		*/
	int		x,y,count;				/* Current (x,y) and count value	*/
	int		yincr,fincr;			/* Increment for values				*/
	ushort	mask,stipple;			/* Current stipple pattern			*/
	ibool	lowslope = true;		/* True if slope <= 1				*/
	uchar	color,*fbuf;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	color = DC.intColor;
	stipple = DC.a.lineStipple;
	mask = (stipple >> (DC.a.stippleCount & 0xF));
	DC.a.stippleCount += count;

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		while (count--) {
			if (mask & 1)
				*fbuf = color;		/* Store pixel value 				*/
			if ((mask >>= 1) == 0)
				mask = stipple;
			fbuf++;					/* Increment x coordinate			*/

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				fbuf += fincr;		/* Increment framebuffer			*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		while (count--) {
			if (mask & 1)
				*fbuf = color;		/* Store pixel value 				*/
			if ((mask >>= 1) == 0)
				mask = stipple;
			y += yincr;				/* Increment y coordinate			*/
			fbuf += fincr;			/* Incremene framebuffer			*/

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				}
			}
		}
}

#endif	/* !MGL_LITE */

void _ASMAPI PACKED8_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		PACKED8_stretchBlt1x2
* Parameters:	dst		- Destination Device context
*				src		- Source device context
* 				left	- Left coordinate of are to Blt from
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*
* Description:	Fast 2x stretch blt for 24 bit pixels to the 8 bit packed
*				pixel device. We are doing a lot of dithering, and a good
*				32 bit C compiler can actually produce faster code than I
*				have been able to do directly in assembler!
*
****************************************************************************/
{
	int		count,xcount = right - left;
	int		srcBytes = src->mi.bytesPerLine;
	int		dstBytes = dst->mi.bytesPerLine;
	uchar	*dBegin,*sBegin;

#ifndef	MGL_LITE
	if (src->mi.bitsPerPixel != dst->mi.bitsPerPixel) {
		if (src->mi.bitsPerPixel == 24 && src->pf.redPos != 0)
			PACKED8_stretchBlt1x2_24(dst,src,left,top,right,bottom,dstLeft,dstTop);
		else
			__EMU__stretchBlt1x2(dst,src,left,top,right,bottom,dstLeft,dstTop,
				pal,idPal);
		return;
		}
#endif

	/* Double each scanline */
	count = bottom - top;
	dBegin = PACKED8_pixelAddr2(dst,dstLeft,dstTop);
	sBegin = PACKED8_pixelAddr2(src,left,top);
	if (dst->flags & MGL_VIRTUAL_ACCESS) {
		while (count--) {
			MGL_memcpyVIRTDST(dBegin,sBegin,xcount);
			dBegin += dstBytes;
			MGL_memcpyVIRTDST(dBegin,sBegin,xcount);
			dBegin += dstBytes;
			sBegin += srcBytes;
			}
		}
	else {
		while (count--) {
			memcpy(dBegin,sBegin,xcount);
			dBegin += dstBytes;
			memcpy(dBegin,sBegin,xcount);
			dBegin += dstBytes;
			sBegin += srcBytes;
			}
		}
}

void _ASMAPI PACKED8_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,
	palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		PACKED8_stretchBlt
* Parameters:	dst		- Destination Device context
*				src		- Source device context
* 				left	- Left coordinate of are to Blt from
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*				zoomx	- X axis zoom factor
*				zoomy	- Y axis zoom factor
*
****************************************************************************/
{
	int		next,prev,count,xcount = right - left;
	int		srcBytes = src->mi.bytesPerLine;
	int		dstBytes = dst->mi.bytesPerLine;
	fix32_t	fx;
	uchar	*sBegin,*dBegin;
	int		*rc,*repCounts = _MGL_buf;

#ifndef	MGL_LITE
	if (!((src->mi.bitsPerPixel == 8) ||
			(src->mi.bitsPerPixel == 24 && src->pf.redPos != 0)) || !idPal) {
		__EMU__stretchBlt(dst,src,left,top,right,bottom,dstLeft,dstTop,
			zoomx,zoomy,pal,idPal);
		return;
		}
#endif

	/* Pre-render the pixel replication counts for each scanline */
	fx = zoomx;
	prev = 0;
	count = xcount;
	rc = repCounts;
	while (count--) {
		*rc++ = (next = MGL_FIXROUND(fx)) - prev;
		prev = next;
		fx += zoomx;
		}

	/* Stretch each scanline */
	fx = zoomy;
	prev = 0;
	count = bottom - top;
	dBegin = PACKED8_pixelAddr2(dst,dstLeft,dstTop);
	if (src->mi.bitsPerPixel == 8) {
		sBegin = PACKED8_pixelAddr2(src,left,top);
		while (count--) {
			int dy = (next = MGL_FIXROUND(fx)) - prev;
			prev = next;
			fx += zoomy;
			while (dy--) {
#if	defined(NO_ASSEMBLER)
				uchar *s = sBegin;
				uchar *d = dBegin;
				int count = xcount;
				int *rc = repCounts;
				while (count--) {
					int i = *rc++;
					while (i--)
						*d++ = *s;
					s++;
					}
#else
				PACKED8_stretchScan(dBegin,sBegin,xcount,repCounts);
#endif
				dBegin += dstBytes;
				}
			sBegin += srcBytes;
			}
		}
#ifndef	MGL_LITE
	else {
		int	y = dstTop;
		sBegin = PACKED24_pixelAddr2(src,left,top);
		while (count--) {
			int dy = (next = MGL_FIXROUND(fx)) - prev;
			prev = next;
			fx += zoomy;
			while (dy--) {
				uchar *s = sBegin;
				uchar *d = dBegin;
				int count = xcount;
				int *rc = repCounts;
				while (count--) {
					int i = *rc++;
					while (i--)
						MGL_halfTonePixelFast2(*d++,(int)d,y,*s);
					s += 3;
					}
				y++;
				dBegin += dstBytes;
				}
			sBegin += srcBytes;
			}
		}
#endif
}

#ifdef	MGL_FIX3D

void _ASMAPI PACKED8_rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,
	fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2)
/****************************************************************************
*
* Function:		PACKED8_rgbLine
* Parameters:	x1,y1,r1,g1,b1	- First endpoint for line
*				x2,y2,r2,g2,b2	- Second endpoint for line
*
* Description:  Renders an RGB shaded line (non clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		dr,dg,db;			/* Color gradients					*/
	fix32_t		rSlope,gSlope,bSlope;/* Color slopes					*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment values					*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;
	int			r,g,b;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	dr = r2 - r1;
	dg = g2 - g1;
	db = b2 - b1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		rSlope = MGL_FixDiv(dr,dx);
		gSlope = MGL_FixDiv(dg,dx);
		bSlope = MGL_FixDiv(db,dx);

		while (count--) {
			r = MGL_FIXTOINT(r1);
			g = MGL_FIXTOINT(g1);
			b = MGL_FIXTOINT(b1);
			MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
			fbuf++;					/* Increment x coordinate			*/
			r1 += rSlope;
			g1 += gSlope;
			b1 += bSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				fbuf += fincr;		/* Increment framebuffer			*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		rSlope = MGL_FixDiv(dr,dx);
		gSlope = MGL_FixDiv(dg,dx);
		bSlope = MGL_FixDiv(db,dx);

		while (count--) {
			r = MGL_FIXTOINT(r1);
			g = MGL_FIXTOINT(g1);
			b = MGL_FIXTOINT(b1);
			MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
			y += yincr;				/* Increment y coordinate			*/
			fbuf += fincr;			/* Incremene framebuffer			*/
			r1 += rSlope;
			g1 += gSlope;
			b1 += bSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_zDitherLine16(fix32_t x1,fix32_t y1,zfix32_t z1,
	fix32_t x2,fix32_t y2,zfix32_t z2)
/****************************************************************************
*
* Function:		PACKED8_zDitherLine16
* Parameters:	x1,y1,z1	- First endpoint for line
*				x2,y2,z2	- Second endpoint for line
*
* Description:  Scan convert a 3D line segment using the MidPoint Digital
*				Differential Analyser algorithm, including Zbuffer
*				coordinates. We call the DC.putPixel code directly to
*				render each pixel.
*
*				Lines must ALWAYS be passed with the X1 value less than the
*				X2 value!!
*
*				This routine expects the endpoints to be in 32 bit fixed
*				point 16.16 format and Z coordinates to be in 4.28 fixed
*				point format.
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf16_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf16_t	z,zSlope;			/* Current position in Z buffer		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;
	int			r,g,b;

	r = MGL_rgbRed(DC.intColor);
	g = MGL_rgbGreen(DC.intColor);
	b = MGL_rgbBlue(DC.intColor);
	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr16(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute Z buffer coordinates and slope */
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute Z buffer coordinates and slope */
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			y += yincr;				/* Increment y coordinate			*/
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_zDitherLine32(fix32_t x1,fix32_t y1,zfix32_t z1,
	fix32_t x2,fix32_t y2,zfix32_t z2)
/****************************************************************************
*
* Function:		PACKED8_zDitherLine32
* Parameters:	x1,y1,z1	- First endpoint for line
*				x2,y2,z2	- Second endpoint for line
*
* Description:  Scan convert a 3D line segment using the MidPoint Digital
*				Differential Analyser algorithm, including Zbuffer
*				coordinates. We call the DC.putPixel code directly to
*				render each pixel.
*
*				Lines must ALWAYS be passed with the X1 value less than the
*				X2 value!!
*
*				This routine expects the endpoints to be in 32 bit fixed
*				point 16.16 format and Z coordinates to be in 4.28 fixed
*				point format.
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf32_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf32_t	zSlope;				/* Current position in Z buffer		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;
	int			r,g,b;

	r = MGL_rgbRed(DC.intColor);
	g = MGL_rgbGreen(DC.intColor);
	b = MGL_rgbBlue(DC.intColor);
	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr32(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute Z buffer coordinates and slope */
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute Z buffer coordinates and slope */
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			y += yincr;				/* Increment y coordinate			*/
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,
	fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,
	fix32_t b2,fix32_t g2)
/****************************************************************************
*
* Function:		PACKED8_rgbzLine16
* Parameters:	x1,y1,r1,g1,b1	- First endpoint for line
*				x2,y2,r2,g2,b2	- Second endpoint for line
*
* Description:  Renders an RGB shaded line (non clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		dr,dg,db;			/* Color gradients					*/
	fix32_t		rSlope,gSlope,bSlope;/* Color slopes					*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf16_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf16_t	z,zSlope;			/* Current position in Z buffer		*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;
	int			r,g,b;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	dr = r2 - r1;
	dg = g2 - g1;
	db = b2 - b1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr16(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		rSlope = MGL_FixDiv(dr,dx);
		gSlope = MGL_FixDiv(dg,dx);
		bSlope = MGL_FixDiv(db,dx);
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				r = MGL_FIXTOINT(r1);
				g = MGL_FIXTOINT(g1);
				b = MGL_FIXTOINT(b1);
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			r1 += rSlope;
			g1 += gSlope;
			b1 += bSlope;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		rSlope = MGL_FixDiv(dr,dx);
		gSlope = MGL_FixDiv(dg,dx);
		bSlope = MGL_FixDiv(db,dx);
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				r = MGL_FIXTOINT(r1);
				g = MGL_FIXTOINT(g1);
				b = MGL_FIXTOINT(b1);
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			y += yincr;				/* Increment y coordinate			*/
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			r1 += rSlope;
			g1 += gSlope;
			b1 += bSlope;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_rgbzLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,
	fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,
	fix32_t b2,fix32_t g2)
/****************************************************************************
*
* Function:		PACKED8_rgbzLine32
* Parameters:	x1,y1,r1,g1,b1	- First endpoint for line
*				x2,y2,r2,g2,b2	- Second endpoint for line
*
* Description:  Renders an RGB shaded line (non clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		dr,dg,db;			/* Color gradients					*/
	fix32_t		rSlope,gSlope,bSlope;/* Color slopes					*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf32_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf32_t	zSlope;				/* Current position in Z buffer		*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr,fincr;		/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	uchar		*fbuf;
	int			r,g,b;

	yincr = 1;
	fincr = DC.mi.bytesPerLine;
	zincr = DC.zwidth*2;
	dz = z2 - z1;
	dr = r2 - r1;
	dg = g2 - g1;
	db = b2 - b1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		fincr = -fincr;
		zincr = -zincr;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;
	fbuf = PACKED8_pixelAddr(x,y);
	zbuf = _MGL_zbufAddr32(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		rSlope = MGL_FixDiv(dr,dx);
		gSlope = MGL_FixDiv(dg,dx);
		bSlope = MGL_FixDiv(db,dx);
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				r = MGL_FIXTOINT(r1);
				g = MGL_FIXTOINT(g1);
				b = MGL_FIXTOINT(b1);
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			fbuf++;					/* Increment x coordinate			*/
			zbuf++;
			r1 += rSlope;
			g1 += gSlope;
			b1 += bSlope;
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				BINC(fbuf,fincr);	/* Increment framebuffer			*/
				BINC(zbuf,zincr);	/* Increment zbuffer pointer		*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		rSlope = MGL_FixDiv(dr,dx);
		gSlope = MGL_FixDiv(dg,dx);
		bSlope = MGL_FixDiv(db,dx);
		zSlope = MGL_ZFixDiv(dz,dx);

		while (count--) {
			if (z1 > *zbuf) {
				*zbuf = z1;
				r = MGL_FIXTOINT(r1);
				g = MGL_FIXTOINT(g1);
				b = MGL_FIXTOINT(b1);
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,y,r,g,b);
				}
			y += yincr;				/* Increment y coordinate			*/
			BINC(fbuf,fincr);		/* Increment framebuffer			*/
			BINC(zbuf,zincr);		/* Increment zbuffer pointer		*/
			r1 += rSlope;
			g1 += gSlope;
			b1 += bSlope;
			z1 += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				fbuf++;
				zbuf++;
				}
			}
		}
}

void _ASMAPI PACKED8_rgbTrap(void)
/****************************************************************************
*
* Function:		PACKED8_rgbTrap
*
* Description:	Renders a trapezoid which is a flat topped/bottom polygon
*				with two edges.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	fix32_t 	dx,ir1,ir2,rSlope,ig1,ig2,gSlope,ib1,ib2,bSlope;
	int			r,g,b;

	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		ir1 = DC.tr.r1;	ir2 = DC.tr.r2;
		ig1 = DC.tr.g1;	ig2 = DC.tr.g2;
		ib1 = DC.tr.b1;	ib2 = DC.tr.b2;
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			SWAPT(ir1,ir2,dx);
			SWAPT(ig1,ig2,dx);
			SWAPT(ib1,ib2,dx);
			}
		if (dX != 0) {
			fbuf = fbufstart + ix1;
			dx = MGL_TOFIX(dX+1);
			rSlope = MGL_FixDiv(ir2 - ir1,dx);
			gSlope = MGL_FixDiv(ig2 - ig1,dx);
			bSlope = MGL_FixDiv(ib2 - ib1,dx);
			while (dX--) {
				r = MGL_FIXTOINT(ir1);
				g = MGL_FIXTOINT(ig1);
				b = MGL_FIXTOINT(ib1);
				MGL_halfTonePixelFast(*fbuf,(int)fbuf,DC.tr.y,r,g,b);
				ir1 += rSlope;	ig1 += gSlope;	ib1 += bSlope;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.r1 += DC.tr.rSlope1;	DC.tr.r2 += DC.tr.rSlope2;
		DC.tr.g1 += DC.tr.gSlope1;	DC.tr.g2 += DC.tr.gSlope2;
		DC.tr.b1 += DC.tr.bSlope1;	DC.tr.b2 += DC.tr.bSlope2;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_zDitherTrap16(void)
/****************************************************************************
*
* Function:		PACKED8_zDitherTrap16
*
* Description:	Renders a trapezoid which is a flat topped/bottom polygon
*				with two edges.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf,*dp;
	zbuf16_t	z,z0,*zbufstart,*zbuf,*zoff;
	int     	Rdiv51,Rmod51;
	int     	Gdiv51,Gmod51;
	int     	Bdiv51,Bmod51;

	zbufstart = _MGL_zbufAddr16(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			}
		zbuf = zbufstart + (ix1 - DC.size.left);
		fbuf = fbufstart + ix1;
		zoff = &(((zbuf16_t*)DC.zOffset)[ix1]);
		z0 = MGL_TOZBUF16(DC.tr.z0);
		MGL_halfTonePixelSetup(DC.tr.y,DC.intColor,dp,Rdiv51,Rmod51,Gdiv51,
			Gmod51,Bdiv51,Bmod51);
		while (dX--) {
			if ((z = z0 + *zoff++) > *zbuf) {
				*zbuf = z;
				MGL_halfTonePixelFast3(*fbuf,(int)fbuf,dp,Rdiv51,Rmod51,
					Gdiv51,Gmod51,Bdiv51,Bmod51);
				}
			zbuf++;
			fbuf++;
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_zDitherTrap32(void)
/****************************************************************************
*
* Function:		PACKED8_zDitherTrap32
*
* Description:	Renders a trapezoid which is a flat topped/bottom polygon
*				with two edges.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf,*dp;
	zbuf32_t	z,*zbufstart,*zbuf,*zoff;
	int     	Rdiv51,Rmod51;
	int     	Gdiv51,Gmod51;
	int     	Bdiv51,Bmod51;

	zbufstart = _MGL_zbufAddr32(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			}
		zbuf = zbufstart + (ix1 - DC.size.left);
		fbuf = fbufstart + ix1;
		zoff = &(((zbuf32_t*)DC.zOffset)[ix1]);
		MGL_halfTonePixelSetup(DC.tr.y,DC.intColor,dp,Rdiv51,Rmod51,Gdiv51,
			Gmod51,Bdiv51,Bmod51);
		while (dX--) {
			if ((z = DC.tr.z0 + *zoff++) > *zbuf) {
				*zbuf = z;
				MGL_halfTonePixelFast3(*fbuf,(int)fbuf,dp,Rdiv51,Rmod51,
					Gdiv51,Gmod51,Bdiv51,Bmod51);
				}
			zbuf++;
			fbuf++;
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_rgbzTrap16(void)
/****************************************************************************
*
* Function:		PACKED8_rgbzTrap16
*
* Description:	Renders a trapezoid which is a flat topped/bottom polygon
*				with two edges.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	fix32_t 	dx,ir1,ir2,rSlope,ig1,ig2,gSlope,ib1,ib2,bSlope;
	zbuf16_t	z,z0,*zbufstart,*zbuf,*zoff;
	int			r,g,b;

	zbufstart = _MGL_zbufAddr16(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		ir1 = DC.tr.r1;	ir2 = DC.tr.r2;
		ig1 = DC.tr.g1;	ig2 = DC.tr.g2;
		ib1 = DC.tr.b1;	ib2 = DC.tr.b2;
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			SWAPT(ir1,ir2,dx);
			SWAPT(ig1,ig2,dx);
			SWAPT(ib1,ib2,dx);
			}
		if (dX != 0) {
			zbuf = zbufstart + (ix1 - DC.size.left);
			fbuf = fbufstart + ix1;
			zoff = &(((zbuf16_t*)DC.zOffset)[ix1]);
			z0 = MGL_TOZBUF16(DC.tr.z0);
			dx = MGL_TOFIX(dX+1);
			rSlope = MGL_FixDiv(ir2 - ir1,dx);
			gSlope = MGL_FixDiv(ig2 - ig1,dx);
			bSlope = MGL_FixDiv(ib2 - ib1,dx);
			while (dX--) {
				if ((z = z0 + *zoff++) > *zbuf) {
					*zbuf = z;
					r = MGL_FIXTOINT(ir1);
					g = MGL_FIXTOINT(ig1);
					b = MGL_FIXTOINT(ib1);
					MGL_halfTonePixelFast(*fbuf,(int)fbuf,DC.tr.y,r,g,b);
					}
				ir1 += rSlope;	ig1 += gSlope;	ib1 += bSlope;
				zbuf++;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.r1 += DC.tr.rSlope1;	DC.tr.r2 += DC.tr.rSlope2;
		DC.tr.g1 += DC.tr.gSlope1;	DC.tr.g2 += DC.tr.gSlope2;
		DC.tr.b1 += DC.tr.bSlope1;	DC.tr.b2 += DC.tr.bSlope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

void _ASMAPI PACKED8_rgbzTrap32(void)
/****************************************************************************
*
* Function:		PACKED8_rgbzTrap32
*
* Description:	Renders a trapezoid which is a flat topped/bottom polygon
*				with two edges.
*
****************************************************************************/
{
	int 		dX,ix1,ix2;
	uchar		*fbufstart,*fbuf;
	fix32_t 	dx,ir1,ir2,rSlope,ig1,ig2,gSlope,ib1,ib2,bSlope;
	zbuf32_t	z,*zbufstart,*zbuf,*zoff;
	int			r,g,b;

	zbufstart = _MGL_zbufAddr32(0,DC.tr.y);
	fbufstart = PACKED8_pixelAddr(0,DC.tr.y);
	while (DC.tr.count--) {
		ir1 = DC.tr.r1;	ir2 = DC.tr.r2;
		ig1 = DC.tr.g1;	ig2 = DC.tr.g2;
		ib1 = DC.tr.b1;	ib2 = DC.tr.b2;
		if ((dX = (ix2 = MGL_FIXROUND(DC.tr.x2)) - (ix1 = MGL_FIXROUND(DC.tr.x1))) < 0) {
			ix1 = ix2;
			dX = -dX;
			SWAPT(ir1,ir2,dx);
			SWAPT(ig1,ig2,dx);
			SWAPT(ib1,ib2,dx);
			}
		if (dX != 0) {
			zbuf = zbufstart + (ix1 - DC.size.left);
			fbuf = fbufstart + ix1;
			zoff = &(((zbuf32_t*)DC.zOffset)[ix1]);
			dx = MGL_TOFIX(dX+1);
			rSlope = MGL_FixDiv(ir2 - ir1,dx);
			gSlope = MGL_FixDiv(ig2 - ig1,dx);
			bSlope = MGL_FixDiv(ib2 - ib1,dx);
			while (dX--) {
				if ((z = DC.tr.z0 + *zoff++) > *zbuf) {
					*zbuf = z;
					r = MGL_FIXTOINT(ir1);
					g = MGL_FIXTOINT(ig1);
					b = MGL_FIXTOINT(ib1);
					MGL_halfTonePixelFast(*fbuf,(int)fbuf,DC.tr.y,r,g,b);
					}
				ir1 += rSlope;	ig1 += gSlope;	ib1 += bSlope;
				zbuf++;
				fbuf++;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.r1 += DC.tr.rSlope1;	DC.tr.r2 += DC.tr.rSlope2;
		DC.tr.g1 += DC.tr.gSlope1;	DC.tr.g2 += DC.tr.gSlope2;
		DC.tr.b1 += DC.tr.bSlope1;	DC.tr.b2 += DC.tr.bSlope2;
		DC.tr.z0 += DC.tr.dZdY;
		zbufstart += DC.zwidth;
		fbufstart += DC.mi.bytesPerLine;
		DC.tr.y++;
		}
}

#endif	/* MGL_FIX3D */
