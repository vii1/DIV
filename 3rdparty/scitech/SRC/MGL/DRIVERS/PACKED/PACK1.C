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
* Description:  1 bit packed pixel device context routines. This code will
*				be linked in if this driver is registered for use.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\packed\packed1.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI PACKED1_driver = {
	IDENT_FLAG,
	MGL_PACKED1NAME,
	MGL_PACKED1NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grNONE,
	NULL,
	PACKED1_initDriver,
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

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
	if (dc->ownMemory)
		MGL_free(dc->surface);
}

ibool _ASMAPI PACKED1_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		PACKED1_initDriver
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
		if ((g_state = (void*)MGL_calloc(1,sizeof(PACKED1_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	dc->v = g_state;
	dc->v->m.refCount++;

	/* Load the device information	*/
	dc->deviceType = MGL_MEMORY_DEVICE;
	dc->mi.bitsPerPixel = 1;
	dc->mi.numberOfPlanes = 1;
	dc->mi.maxColor = 0x1;
	dc->mi.maxPage = 0;
	dc->mi.aspectRatio = 1000;
	dc->ellipseFixup = true;
	dc->zbuffer = NULL;

	/* Fill in required vectors */
	dc->v->m.destroy			= destroyDC;
	dc->r.getWinDC				= DRV_getWinDC;
	dc->r.getDefaultPalette 	= DRV_getDefaultPalette;
	dc->r.realizePalette 		= DRV_stubVector;
	dc->r.setColor 				= DRV_setColor;
	dc->r.setBackColor 			= DRV_setBackColor;
	dc->r.setWriteMode			= DRV_setWriteMode;
	dc->r.setClipRect 			= DRV_setClipRect;
	dc->r.beginDirectAccess 	= DRV_stubVector;
	dc->r.endDirectAccess 		= DRV_stubVector;
	dc->r.beginPixel 			= DRV_stubVector;
	dc->r.getPixel 				= PACKED1_getPixel;
	dc->r.putPixel 				= PACKED1_putPixel;
	dc->r.endPixel 				= DRV_stubVector;
	dc->r.getScanLine			= PACKED1_getScanLine;
	dc->r.putScanLine			= PACKED1_putScanLine;
	dc->r.stretchScanLine		= DRV_stubVector;
	dc->r.stretchScanLine2x		= DRV_stubVector;

#ifndef	MGL_LITE
	dc->r.setPenStyle			= DRV_setPenStyle;
	dc->r.setPenBitmapPattern	= DRV_setPenBitmapPattern;
	dc->r.setPenPixmapPattern	= DRV_setPenPixmapPattern;
	dc->r.setLineStipple		= DRV_setLineStipple;
	dc->r.setLineStippleCount	= DRV_setLineStippleCount;
	dc->r.ditherPixel			= DRV_stubVector;
	dc->r.putSrcTransparent		= DRV_stubVector;
	dc->r.putDstTransparent		= DRV_stubVector;
#endif

	/* Fill in remaining device vectors */
	_MGL_setLineVecs(dc,__EMU__line);
	_MGL_setScanLineVecs(dc,PACKED1_scanLine);
	_MGL_setFillRectVecs(dc,__EMU__fillRect);
	dc->r.putMonoImage			= __EMU__putMonoImage;
	dc->r.putMouseImage			= DRV_stubVector;
	dc->r.getImage				= __EMU__getImage;
	dc->r.putImage				= __EMU__putImage;
	dc->r.divotSize				= __EMU__divotSize;
	dc->r.getDivot 				= __EMU__getDivot;
	dc->r.putDivot 				= __EMU__putDivot;
	dc->r.stretchBlt1x2			= __EMU__stretchBlt1x2;
	dc->r.stretchBlt2x2			= __EMU__stretchBlt2x2;
	dc->r.stretchBlt			= __EMU__stretchBlt;

#ifndef	MGL_LITE
	dc->r.stippleLine			= __EMU__stippleLine;
	_MGL_setTrapVecs(dc,__EMU__trap);
	dc->r.patt.line				= __EMU__fatPenLine;
	dc->r.colorPatt.line		= __EMU__fatPenLine;
	dc->r.fatSolid.line			= __EMU__fatPenLine;
	dc->r.fatRopSolid.line		= __EMU__fatPenLine;
	dc->r.fatPatt.line			= __EMU__fatPenLine;
	dc->r.fatColorPatt.line		= __EMU__fatPenLine;
	_MGL_setDrawScanListVecs(dc,__EMU__drawScanList);
	_MGL_setDrawRegionVecs(dc,__EMU__drawRegion);
	_MGL_setEllipseVecs(dc,__EMU__ellipse);
	dc->r.fatSolid.ellipse		= __EMU__fatPenEllipse;
	dc->r.fatRopSolid.ellipse	= __EMU__fatPenEllipse;
	dc->r.fatPatt.ellipse		= __EMU__fatPenEllipse;
	dc->r.fatColorPatt.ellipse 	= __EMU__fatPenEllipse;
	_MGL_setFillEllipseVecs(dc,__EMU__fillEllipse);
	_MGL_setEllipseArcVecs(dc,__EMU__ellipseArc);
	dc->r.fatSolid.ellipseArc	= __EMU__fatPenEllipseArc;
	dc->r.fatRopSolid.ellipseArc = __EMU__fatPenEllipseArc;
	dc->r.fatPatt.ellipseArc	= __EMU__fatPenEllipseArc;
	dc->r.fatColorPatt.ellipseArc = __EMU__fatPenEllipseArc;
	_MGL_setFillEllipseArcVecs(dc,__EMU__fillEllipseArc);
	dc->r.getArcCoords 			= __EMU__getArcCoords;
	dc->r.drawStrBitmap			= __EMU__drawStrBitmap;
	dc->r.drawCharVec			= __EMU__drawCharVec;
	dc->r.complexPolygon 		= __EMU__complexPolygon;
	dc->r.polygon 				= __EMU__polygon;
	dc->r.translateImage		= __EMU__translateImage;
	dc->r.bitBlt				= __EMU__bitBlt;
	dc->r.srcTransBlt			= __EMU__srcTransBlt;
	dc->r.dstTransBlt			= __EMU__dstTransBlt;
	dc->r.scanRightForColor		= __EMU__scanRightForColor;
	dc->r.scanLeftForColor		= __EMU__scanLeftForColor;
	dc->r.scanRightWhileColor	= __EMU__scanRightWhileColor;
	dc->r.scanLeftWhileColor	= __EMU__scanLeftWhileColor;

#ifdef	MGL_FIX3D
	/* 3D rasterisation */
	dc->r.tri 					= __EMU__tri;
	dc->r.quad 					= __EMU__quad;

	/* 3D zbuffering */
	dc->r.zBegin				= __EMU__zBegin;
#endif
#endif

	driverId = driverId;
	modeId = modeId;
	hwnd = hwnd;
	virtualX = virtualX;
	virtualY = virtualY;
	numBuffers = numBuffers;

	/* Allocate the device context surface */
	if (!dc->surface) {
		dc->mi.bytesPerLine = (((dc->mi.xRes+1+7)/8) + 3) & ~3;
		return (AllocateSurface(dc) != 0);
		}
	return true;
}

void _ASMAPI PACKED1_scanLine(int y,int x1,int x2)
/****************************************************************************
*
* Function:		PACKED1_scanLine
* Parameters:	y	- Y coordinate for scanline
*				x1	- First x coordinate for scanline
*				x2	- Second x coordinate for scanline
*
* Description:	Renders a scanline into the device context at the
*				specified location.
*
****************************************************************************/
{
	int		count = (x2-1)/8 - x1/8;
	uchar	mask1,mask2,mask3,color1,color2,color3,pattern,*s;

	if (x1 == x2)				/* Ignore lines with xstart = xend	*/
		return;

	s = (uchar*)DC.surface + (y * DC.mi.bytesPerLine) + x1/8;
	mask1 = 0xFE << ((x1 & 0x7) ^ 0x7);
	mask2 = 0x7F >> ((x2-1) & 0x7);
	color1 = (DC.a.penStyle == MGL_BITMAP_OPAQUE) ? DC.intBackColor : DC.intColor;
	if (color1 & 0x1) {
		color1 = ~mask1;
		color2 = ~mask2;
		color3 = 0xFF;
		}
	else color1 = color2 = color3 = 0;
	if (DC.a.penStyle != MGL_BITMAP_TRANSPARENT) {
		switch (DC.a.writeMode) {
			case MGL_REPLACE_MODE:
				if (count) {
					*s &= mask1;
					*s++ |= color1;
					count--;
					while (count--)
						*s++ = color3;
					}
				else mask2 |= mask1;
				*s &= mask2;
				*s |= color2;
				break;
			case MGL_XOR_MODE:
				if (count) {
					*s++ ^= color1;
					count--;
					while (count--)
						*s++ ^= color3;
					}
				else mask2 |= mask1;
				*s ^= color2;
				break;
			case MGL_OR_MODE:
				if (count) {
					*s++ |= color1;
					count--;
					while (count--)
						*s++ |= color3;
					}
				else mask2 |= mask1;
				*s |= color2;
				break;
			case MGL_AND_MODE:
				if (count) {
					*s++ &= (mask1 | color1);
					count--;
					while (count--)
						*s++ &= color3;
					}
				else mask2 |= mask1;
				*s &= (mask2 | color2);
				break;
			}
		}
	if (DC.a.penStyle != MGL_BITMAP_SOLID) {
		pattern = DC.a.penPat[y & 7];
		mask3 = ~pattern;
		mask1 |= mask3;
		mask2 |= mask3;
		if (DC.intColor & 0x1) {
			color1 = ~mask1;
			color2 = ~mask2;
			color3 = pattern;
			}
		else color1 = color2 = color3 = 0;
		if (DC.a.writeMode == MGL_XOR_MODE) {
			if (count) {
				pattern = *s ^ color1;
				*s &= mask1;
				*s++ |= pattern;
				count--;
				while (count--) {
					pattern = *s ^ color3;
					*s &= mask3;
					*s++ |= pattern;
					}
				}
			else mask2 |= mask1;
			pattern = *s ^ color2;
			*s &= mask2;
			*s |= pattern;
			}
		else {
			if (count) {
				*s &= mask1;
				*s++ |= color1;
				count--;
				while (count--) {
					*s &= mask3;
					*s++ |= color3;
					}
				}
			else mask2 |= mask1;
			*s &= mask2;
			*s |= color2;
			}
		}
}

color_t _ASMAPI PACKED1_getPixel(int x,int y)
/****************************************************************************
*
* Function:		PACKED1_getPixel
* Parameters:	x,y	- Coordinate to read pixel from
* Returns:		Color of the pixel at the specified coordinate
*
****************************************************************************/
{
	uchar	_HUGE *s;

	s = (uchar _HUGE *)DC.surface + ((long)y * DC.mi.bytesPerLine) + x/8;
	return (*s >> (x & 0x7)) & 0x1;
}

void _ASMAPI PACKED1_putPixel(int x,int y)
/****************************************************************************
*
* Function:		PACKED1_putPixel
* Parameters:	x,y	- Coordinate to read pixel from
*
****************************************************************************/
{
	uchar	mask,color,_HUGE *s;

	s = (uchar _HUGE *)DC.surface + ((long)y * DC.mi.bytesPerLine) + x/8;
	mask = ~(1 << ((x & 0x7) ^ 0x7));
	color = (DC.intColor & 0x1) ? ~mask : 0;
	switch (DC.a.writeMode) {
		case MGL_REPLACE_MODE:
			*s &= mask;
			*s |= color;
			break;
		case MGL_XOR_MODE:
			*s ^= color;
			break;
		case MGL_OR_MODE:
			*s |= color;
			break;
		case MGL_AND_MODE:
			*s &= (mask | color);
			break;
		}
}

void _ASMAPI PACKED1_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
	int bufx,int op)
/****************************************************************************
*
* Function:		PACKED1_getScanLine
* Parameters:	dc		- Device context
*				y		- Scanline to get data from
*				x1		- Starting coordinate in scanline
*				y2		- Ending coordinate in scanline
*				buffer	- Pointer to buffer to store data in
*				bufx	- X coordinate in buffer 
*				op		- Write mode op to use during transfer
*
* Description:  Reads a scanline of information from the device context
*				into the memory buffer. Note that this routine _always_
*				stores data in the memory buffer in packed pixel format.
*				The 'bufx' value is used to determine how many bits each
*				pixel value must be shifted over when dumping into the
*				output buffer.
*
*				The write mode operation determines how the data will be
*				written into the memory buffer.
*
****************************************************************************/
{
	dc = dc;
	y = y;
	x1 = x1;
	x2 = x2;
	buffer = buffer;
	bufx = bufx;
	op = op;
/*	int		i,middleBytes;
	uchar	*s,*p = buffer,lmask,rmask,shift;

	s = (uchar*)((uchar _HUGE*)dc->surface + ((long)y * dc->mi.bytesPerLine) + x1/8);
	lmask = 0xFF >> ((x1 & 0x7) ^ 0x7);
	rmask = 0xFF << ((x2 & 0x7) ^ 0x7);
	switch (op) {
		case MGL_REPLACE_MODE:
			memcpy(p,s,middleBytes);
			break;
		case MGL_XOR_MODE:
			for (i = 0; i < middleBytes; i++)
				*p++ ^= *s++;
			break;
		case MGL_OR_MODE:
			for (i = 0; i < middleBytes; i++)
				*p++ |= *s++;
			break;
		case MGL_AND_MODE:
			for (i = 0; i < middleBytes; i++)
				*p++ &= *s++;
			break;
		}*/
}

void _ASMAPI PACKED1_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
	int bufx,int op)
/****************************************************************************
*
* Function:		PACKED1_putScanLine
* Parameters:	dc		- Device context
*				y		- Scanline to get data from
*				x1		- Starting coordinate in scanline
*				y2		- Ending coordinate in scanline
*				buffer	- Pointer to buffer to get data from
*				bufx	- X coordinate in buffer
*				op		- Write mode op to use during transfer
*
* Description:  Dumps a scanline of information to the device context
*				from the memory buffer. Note that this routine _always_
*				reads data from the memory buffer in packed pixel format.
*				The 'bufx' value is used to determine how many bits each
*				pixel value must be shifted over when reading from the
*				input buffer.
*
*				The write mode operation determines how the data will be
*				written into the display context.
*
****************************************************************************/
{
	dc = dc;
	y = y;
	x1 = x1;
	x2 = x2;
	buffer = buffer;
	bufx = bufx;
	op = op;
/*	int		i,middleBytes = (x2 - x1);
	uchar	*s = buffer,*p;

	p = (uchar*)((uchar _HUGE*)dc->surface + ((long)y * dc->mi.bytesPerLine) + x1);
	switch (op) {
		case MGL_REPLACE_MODE:
			memcpy(p,s,middleBytes);
			break;
		case MGL_XOR_MODE:
			for (i = 0; i < middleBytes; i++)
				*p++ ^= *s++;
			break;
		case MGL_OR_MODE:
			for (i = 0; i < middleBytes; i++)
				*p++ |= *s++;
			break;
		case MGL_AND_MODE:
			for (i = 0; i < middleBytes; i++)
				*p++ &= *s++;
			break;
		}*/
}
