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
* Description:	Main VGA4 device driver file. This file will be linked in
*				when a call to MGL_registerDriver() is made to register
*				this device driver for use. This driver only supports the
*				standard VGA mode - the SVGA4 driver supports these same
*				modes with extended page flipping enabled (and 800x600).
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\vga\vga4.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI VGA4_driver = {
	IDENT_FLAG,
	MGL_VGA4NAME,
	MGL_VGA4NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grVGA,
	VGA4_detect,
	VGA4_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t VGA4_modes[] = {
	{319,199,4,4,15,7,40,1200,0x2000,0x0D},
	{639,199,4,4,15,3,80,2400,0x4000,0x0E},
	{639,349,4,4,15,1,80,1371,0x8000,0x10},
	{639,399,4,4,15,1,80,1200,0x8000,0x10},
	{639,479,4,4,15,0,80,1000,0x10000L,0x12},
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI VGA4_detect(int id,int force,int *driver,int *mode,modetab modes)
/****************************************************************************
*
* Function:		VGA4_detect
* Parameters:	id		- Id of this driver for storing in mode table
*				force	- Highest performance driver to be used
*               driver	- Place to store detected driver id
*				mode	- Place to store recommended video mode
*               modes	- Place to store list of supported modes
* Returns:		TRUE if the device was found, FALSE if not.
*
* Description:	Determines if an VGA or compatible adapter is present, and
*				if so fills in the mode table with all of the modes
*				supported by this device driver.
*
*				If the value of 'driver' is less than the value of our
*               device driver number, then we return false since the user
*				has requested to ignore our devices modes.
*
****************************************************************************/
{
	if (force >= grVGA && VGA_detect(driver)) {
		VGA_useMode(modes,grVGA_320x200x16,id,&VGA4_modes[0]);
		VGA_useMode(modes,grVGA_640x200x16,id,&VGA4_modes[1]);
		VGA_useMode(modes,grVGA_640x350x16,id,&VGA4_modes[2]);
		VGA_useMode(modes,grVGA_640x400x16,id,&VGA4_modes[3]);
		VGA_useMode(modes,grVGA_640x480x16,id,&VGA4_modes[4]);
		if (*mode == grDETECT)
			*mode = grVGA_640x480x16;
		return true;
		}
	return false;
}

PRIVATE void _ASMAPI destroyDC(MGLDC *dc)
/****************************************************************************
*
* Function:		destroyDC
* Parameters:	dc	- Device context
*
* Description:	Destroys all internal memory allocated by the device
*				context.
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
}

void VGA4_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		VGA4_initInternal
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver internal tables for 4 bit
*				VGA style modes.
*
****************************************************************************/
{
	display_vec	*d = &dc->v->d;

	/* Load the device context with the device information for the
	 * selected video mode
	 */
	dc->deviceType = MGL_DISPLAY_DEVICE;
	dc->xInch = 9000;
	dc->yInch = 7000;
	d->hardwareCursor = FALSE;
	d->makeOffscreenDC = NULL;
	d->makeLinearOffscreenDC = NULL;
	dc->ellipseFixup = true;
#ifdef	__WINDOWS__
	dc->wm.fulldc.hdc = NULL;
#endif

	/* Fill in required device context vectors */
	d->maxProgram				= 256;
	d->restoreTextMode 			= VGA4_restoreTextMode;
	d->restoreGraphMode			= VGA4_restoreGraphMode;
	d->setActivePage 			= VGA_setActivePage;
	d->setVisualPage 			= VGA_setVisualPage;
	d->vSync 					= VGA_vSync;

	/* Fill in required vectors */
	dc->r.getWinDC				= DRV_getWinDC;
	dc->r.getDefaultPalette 	= VGA4_getDefaultPalette;
	dc->r.realizePalette 		= VGA_realizePalette;
	dc->r.setColor 				= DRV_setColor;
	dc->r.setBackColor 			= DRV_setBackColor;
	dc->r.setWriteMode			= DRV_setWriteMode;
	dc->r.setClipRect 			= DRV_setClipRect;
	dc->r.beginPixel 			= VGA4_beginPixel;
	dc->r.getPixel 				= VGA4_getPixel;
	dc->r.putPixel 				= VGA4_putPixel;
	dc->r.endPixel 				= VGA4_endPixel;
	dc->r.getScanLine			= VGA4_getScanLine;
	dc->r.putScanLine			= VGA4_putScanLine;

#ifndef	MGL_LITE
	dc->r.setPenStyle			= DRV_setPenStyle;
	dc->r.setPenBitmapPattern	= DRV_setPenBitmapPattern;
	dc->r.setPenPixmapPattern	= DRV_setPenPixmapPattern;
	dc->r.setLineStipple		= DRV_setLineStipple;
	dc->r.setLineStippleCount	= DRV_setLineStippleCount;
#endif

	/* Fill in remaining device vectors */
	_MGL_setLineVecs(dc,	  VGA4_line);
	_MGL_setScanLineVecs(dc,  VGA4_scanLine);
	_MGL_setFillRectVecs(dc,  VGA4_fillRect);
	dc->r.putMonoImage			= VGA4_putMonoImage;
	dc->r.putMouseImage			= __EMU__putMouseImage;
	dc->r.getImage				= __EMU__getImage;
	dc->r.putImage				= VGA4_putImage;
	dc->r.divotSize				= VGA4_divotSize;
	dc->r.getDivot 				= VGA4_getDivot;
	dc->r.putDivot 				= VGA4_putDivot;
	dc->r.stretchBlt1x2			= __EMU__stretchBlt1x2;
	dc->r.stretchBlt2x2			= __EMU__stretchBlt2x2;
	dc->r.stretchBlt			= __EMU__stretchBlt;

#ifndef	MGL_LITE
	dc->r.stippleLine			= __EMU__stippleLine;
	_MGL_setTrapVecs(dc,	  __EMU__trap);
	dc->r.patt.line				= __EMU__fatPenLine;
	dc->r.colorPatt.line		= __EMU__fatPenLine;
	dc->r.fatSolid.line			= __EMU__fatPenLine;
	dc->r.fatRopSolid.line		= __EMU__fatPenLine;
	dc->r.fatPatt.line			= __EMU__fatPenLine;
	dc->r.fatColorPatt.line		= __EMU__fatPenLine;
	_MGL_setDrawScanListVecs(dc,VGA4_drawScanList);
	_MGL_setDrawRegionVecs(dc,__EMU__drawRegion);
	_MGL_setEllipseVecs(dc,	  VGA4_ellipse);
	dc->r.fatSolid.ellipse		= __EMU__fatPenEllipse;
	dc->r.fatRopSolid.ellipse	= __EMU__fatPenEllipse;
	dc->r.fatPatt.ellipse		= __EMU__fatPenEllipse;
	dc->r.fatColorPatt.ellipse 	= __EMU__fatPenEllipse;
	_MGL_setFillEllipseVecs(dc,__EMU__fillEllipse);
	_MGL_setEllipseArcVecs(dc,__EMU__ellipseArc);
	dc->r.fatSolid.ellipseArc	= __EMU__fatPenEllipseArc;
	dc->r.fatRopSolid.ellipseArc = __EMU__fatPenEllipseArc;
	dc->r.fatPatt.ellipseArc	= __EMU__fatPenEllipseArc;
	dc->r.fatColorPatt.ellipseArc	= __EMU__fatPenEllipseArc;
	_MGL_setFillEllipseArcVecs(dc,__EMU__fillEllipseArc);
	dc->r.getArcCoords 			= __EMU__getArcCoords;
	dc->r.drawStrBitmap			= __EMU__drawStrBitmap;
	dc->r.drawCharVec			= __EMU__drawCharVec;
	dc->r.complexPolygon 		= __EMU__complexPolygon;
	dc->r.polygon 				= __EMU__polygon;
	dc->r.translateImage		= __EMU__translateImage;
	dc->r.bitBlt				= VGA4_bitBlt;
	dc->r.srcTransBlt			= __EMU__srcTransBlt;
	dc->r.dstTransBlt			= __EMU__dstTransBlt;
	dc->r.scanRightForColor		= VGA4_scanRightForColor;
	dc->r.scanLeftForColor		= VGA4_scanLeftForColor;
	dc->r.scanRightWhileColor	= VGA4_scanRightWhileColor;
	dc->r.scanLeftWhileColor	= VGA4_scanLeftWhileColor;

#ifdef	MGL_FIX3D
	/* 3D rasterisation */
	dc->r.tri 					= __EMU__tri;
	dc->r.quad 					= __EMU__quad;

	/* 3D Zbuffering */
	dc->r.zBegin				= __EMU__zBegin;
#endif
#endif
}

void VGA4_initMode(int mode)
/****************************************************************************
*
* Function:		VGA4_initMode
* Parameters:	mode	- Mode that has been initialised
*
****************************************************************************/
{
	int		i;
	RMREGS	regs;

	if (mode == grVGA_640x400x16) {
		/* This is a custom 640x400 VGA mode. In order to set this mode,
		 * we set the normal VGA 640x350 mode, and then tweak the CRTC
		 * parameters to get the new emulated 640x400 video mode.
		 */

		UV_outp(0x3C2, (UV_inp(0x3CC) & 0x3F) | 0x40);
		UV_wrinx(0x3D4, 0x10, 0x9C);	/* Vertical sync start		*/
		UV_wrinx(0x3D4, 0x11, 0x8E);	/* Vertical sync end		*/
		UV_wrinx(0x3D4, 0x12, 0x8F);	/* Vertical display end		*/
		UV_wrinx(0x3D4, 0x15, 0x96);	/* Vertical blank start		*/
		UV_wrinx(0x3D4, 0x16, 0xB9);	/* Vertical blank end		*/

		regs.x.ax = 0x1124;				/* Load BIOS 8x16 font		*/
		regs.h.bl = 0x2;
		PM_int86(0x10, &regs, &regs);
		}

	/* Re-program the EGA palette with values from 0 through 15 so that we
	 * can simply re-program the VGA color registers directly
	 */
	UV_inp(0x3DA);				/* Reset attribute controller flip flop	*/
	for (i = 0; i < 16; i++) {
		UV_outp(0x3C0, i);
		UV_outp(0x3C0, i);
		}
	UV_outp(0x3C0, 0x20);		/* Turn display output back on again	*/
}

ibool _ASMAPI VGA4_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		VGA4_initDriver
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver, and starts the specified
*				graphics mode. This is also where we fill in all of the
*				vectors in the device context to initialise our device
*				context properly.
*
****************************************************************************/
{
	VGA4_state	*state;
	display_vec	*d;

	/* This driver does not support scrolling surfaces */
	if (virtualX != -1 || virtualY != -1)
		return false;
	if (stereo)
    	return false;
	numBuffers = numBuffers;

	/* Allocate a single buffer for the driver specific state buffer */
	dc->mi = VGA4_modes[modeId - grVGA_320x200x16];
	if (g_state == NULL) {
		if ((g_state = (void*)MGL_calloc(1,sizeof(VGA4_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	state = (VGA4_state*)(dc->v = g_state);
	d = &dc->v->d;
	d->refCount++;
	d->hwnd = hwnd;
	d->graphDriver = driverId;
	d->graphMode = modeId;
	d->destroy = destroyDC;

	VGA4_initInternal(dc);

	/* Set the video mode */
	if (!VGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,&state->oldBIOSMode,
			&state->old43Lines,NULL))
		return false;

	/* Do extra mode initilisation */
	VGA4_initMode(d->graphMode);
	return true;
}

ibool _ASMAPI VGA4_restoreGraphMode(MGLDC *dc)
{
	short t1; ibool t2;
	if (!VGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,&t1,&t2,NULL))
		return false;
	VGA4_initMode(dc->v->d.graphMode);
	return true;
}

void _ASMAPI VGA4_restoreTextMode(MGLDC *dc)
{
	VGA4_state *state = (VGA4_state*)dc->v;
	VGA_restoreMode(state->oldBIOSMode,state->old43Lines,NULL);
}

void * _ASMAPI VGA4_getDefaultPalette(MGLDC *dc)
{ dc = dc; return _VGA4_defPal; }

void _ASMAPI VGA4_putImage(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,MGLDC *src)
/****************************************************************************
*
* Function:		VGA4_putImage
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
*				device context.
*
****************************************************************************/
{
	int		i,dstRight = (dstLeft + (right - left));
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *p;

	/* Display data by calling device driver to dump scanlines	*/
	p = (uchar _HUGE *)surface + ((long)top * bytesPerLine) + left / 2;
	for (i = dstTop; i < dstBottom; i++, p += bytesPerLine) {
		NORMALISE_PTR(p);
		VGA4_putScanLine(dc,i,dstLeft,dstRight,p,left,op);
		}
	src = src;
}

#ifndef	MGL_LITE

void _ASMAPI VGA4_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,int incx,int incy)
/****************************************************************************
*
* Function:		VGA4_bitBlt
* Parameters:	dc		- Device context
* 				left	- Left coordinate of are to Blt from
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*				op		- Write mode to use during Blt
*				incx	- Do we increment or decrement in x
*				incy	- Do we increment or decrement in y
*
* Description:	Blts a block of image data from one place on the device
*				context to another place on the device context. The values
*				of 'incx' and 'incy' will have been set by the high level
*				code to ensure that the transfer will take place correctly
*				for overlapping regions.
*
*				If the source and destination x coordinates are both aligned
*				on the same pixel boundary within the byte containing the
*				pixel, we use the fast VGA4_alignedBitBlt routine, otherwise
*				we use a slower scheme to move the data through a system
*				memory buffer.
*
****************************************************************************/
{
	if ((left & 0x7) == (dstLeft & 0x7)) {
		/* Break up the blt into a byte aligned blit for the middle section
		 * which can be done very fast, and calls to DRV_copyImage for the
		 * left and right unaligned portions of the image
		 */
		int	left8 = (left+7) & ~0x7,right8 = right & ~0x7;

		if (incx && (left != left8))
			DRV_copyImage(dc,left,top,left8,bottom,dstLeft,dstTop,incy,
				(left8-left+1)/2,op,VGA4_readImage,VGA4_writeImage);
		if (!incx && (right != right8))
			DRV_copyImage(dc,right8,top,right,bottom,dstLeft+(right8-left),dstTop,incy,
				(right-right8+1)/2,op,VGA4_readImage,VGA4_writeImage);
		VGA4_alignedBitBlt(dc,left8,top,right8,bottom,(dstLeft+7) & ~0x7,
			dstTop,incx,incy,op);
		if (!incx && (left != left8))
			DRV_copyImage(dc,left,top,left8,bottom,dstLeft,dstTop,incy,
				(left8-left+1)/2,op,VGA4_readImage,VGA4_writeImage);
		if (incx && (right != right8))
			DRV_copyImage(dc,right8,top,right,bottom,dstLeft+(right8-left),dstTop,incy,
				(right-right8+1)/2,op,VGA4_readImage,VGA4_writeImage);
		}
	else {
		DRV_copyImage(dc,left,top,right,bottom,dstLeft,dstTop,incy,
			(right-left+1)/2,op,VGA4_readImage,VGA4_writeImage);
		}
}

#endif	/* !MGL_LITE */

long _ASMAPI VGA4_divotSize(MGLDC *dc,int left,int top,int right,int bottom)
/****************************************************************************
*
* Function:		VGA4_divotSize
* Returns:		Memory required to store divot
*
* Description:	For maximum speed in the 16 color divot code, we align
*               the left and right edges of the divot to byte boundaries.
*
****************************************************************************/
{
	dc = dc;
	return (long)((((right-1) / 8) + 1) - (left/8)) *
		(long)(bottom - top) * 4 + sizeof(divotHeader);
}
