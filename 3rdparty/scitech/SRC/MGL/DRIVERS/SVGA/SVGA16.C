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
* Description:	Main SuperVGA 64k color device driver file. This file
*				will be linked in when a call to MGL_registerDriver()
*				is made	to register this device driver for use.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\svga\svga16.h"
#include "drivers\packed\packed16.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI SVGA16_driver = {
	IDENT_FLAG,
	MGL_SVGA16NAME,
	MGL_SVGA16NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grVESA,
	SVGA16_detect,
	SVGA16_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t SVGA15_modes[] = {
	{319,199,15,1,0x7FFF,0,0,1200,0,0x4F02},
	{319,239,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{319,399,15,1,0x7FFF,0,0,600,0,0x4F02},
	{319,479,15,1,0x7FFF,0,0,500,0,0x4F02},
	{399,299,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{511,383,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{639,349,15,1,0x7FFF,0,0,1371,0,0x4F02},
	{639,399,15,1,0x7FFF,0,0,1200,0,0x4F02},
	{639,479,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{799,599,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{1023,767,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{1151,863,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{1279,959,15,1,0x7FFF,0,0,1000,0,0x4F02},
	{1279,1023,15,1,0x7FFF,0,0,938,0,0x4F02},
	{1599,1199,15,1,0x7FFF,0,0,1000,0,0x4F02},
	};

PRIVATE gmode_t SVGA16_modes[] = {
	{319,199,16,1,0xFFFF,0,0,1200,0,0x4F02},
	{319,239,16,1,0x7FFF,0,0,1000,0,0x4F02},
	{319,399,16,1,0x7FFF,0,0,600,0,0x4F02},
	{319,479,16,1,0x7FFF,0,0,500,0,0x4F02},
	{399,299,16,1,0xFFFF,0,0,1000,0,0x4F02},
	{511,383,16,1,0xFFFF,0,0,1000,0,0x4F02},
	{639,349,16,1,0xFFFF,0,0,1371,0,0x4F02},
	{639,399,16,1,0xFFFF,0,0,1200,0,0x4F02},
	{639,479,16,1,0xFFFF,0,0,1000,0,0x4F02},
	{799,599,16,1,0xFFFF,0,0,1000,0,0x4F02},
	{1023,767,16,1,0xFFFF,0,0,1000,0,0x4F02},
	{1151,863,16,1,0xFFFF,0,0,1000,0,0x4F02},
	{1279,959,16,1,0xFFFF,0,0,1000,0,0x4F02},
	{1279,1023,16,1,0xFFFF,0,0,938,0,0x4F02},
	{1599,1199,16,1,0xFFFF,0,0,1000,0,0x4F02},
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI SVGA16_detect(int id,int force,int *driver,int *mode,modetab modes)
/****************************************************************************
*
* Function:		SVGA16_detect
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
	if (force >= grVESA && SVGA_detect(driver)) {
		/* Fill in information here for the available video modes by
		 * querying the SVGA VBE BIOS for the information.
		 */
		SVGA_useMode(modes,grSVGA_320x200x32k,id,&SVGA15_modes[0]);
		SVGA_useMode(modes,grSVGA_320x240x32k,id,&SVGA15_modes[1]);
		SVGA_useMode(modes,grSVGA_320x400x32k,id,&SVGA15_modes[2]);
		SVGA_useMode(modes,grSVGA_320x480x32k,id,&SVGA15_modes[3]);
		SVGA_useMode(modes,grSVGA_400x300x32k,id,&SVGA15_modes[4]);
		SVGA_useMode(modes,grSVGA_512x384x32k,id,&SVGA15_modes[5]);
		SVGA_useMode(modes,grSVGA_640x350x32k,id,&SVGA15_modes[6]);
		SVGA_useMode(modes,grSVGA_640x400x32k,id,&SVGA15_modes[7]);
		SVGA_useMode(modes,grSVGA_640x480x32k,id,&SVGA15_modes[8]);
		SVGA_useMode(modes,grSVGA_800x600x32k,id,&SVGA15_modes[9]);
		SVGA_useMode(modes,grSVGA_1024x768x32k,id,&SVGA15_modes[10]);
		SVGA_useMode(modes,grSVGA_1152x864x32k,id,&SVGA15_modes[11]);
		SVGA_useMode(modes,grSVGA_1280x960x32k,id,&SVGA15_modes[12]);
		SVGA_useMode(modes,grSVGA_1280x1024x32k,id,&SVGA15_modes[13]);
		SVGA_useMode(modes,grSVGA_1600x1200x32k,id,&SVGA15_modes[14]);

		SVGA_useMode(modes,grSVGA_320x200x64k,id,&SVGA16_modes[0]);
		SVGA_useMode(modes,grSVGA_320x240x64k,id,&SVGA16_modes[1]);
		SVGA_useMode(modes,grSVGA_320x400x64k,id,&SVGA16_modes[2]);
		SVGA_useMode(modes,grSVGA_320x480x64k,id,&SVGA16_modes[3]);
		SVGA_useMode(modes,grSVGA_400x300x64k,id,&SVGA16_modes[4]);
		SVGA_useMode(modes,grSVGA_512x384x64k,id,&SVGA16_modes[5]);
		SVGA_useMode(modes,grSVGA_640x350x64k,id,&SVGA16_modes[6]);
		SVGA_useMode(modes,grSVGA_640x400x64k,id,&SVGA16_modes[7]);
		SVGA_useMode(modes,grSVGA_640x480x64k,id,&SVGA16_modes[8]);
		SVGA_useMode(modes,grSVGA_800x600x64k,id,&SVGA16_modes[9]);
		SVGA_useMode(modes,grSVGA_1024x768x64k,id,&SVGA16_modes[10]);
		SVGA_useMode(modes,grSVGA_1152x864x64k,id,&SVGA16_modes[11]);
		SVGA_useMode(modes,grSVGA_1280x960x64k,id,&SVGA16_modes[12]);
		SVGA_useMode(modes,grSVGA_1280x1024x64k,id,&SVGA16_modes[13]);
		SVGA_useMode(modes,grSVGA_1600x1200x64k,id,&SVGA16_modes[14]);
		return true;
		}
	mode = mode;
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

void SVGA16_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		SVGA16_initInternal
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver tables.
*
****************************************************************************/
{
	/* Fill in required device context vectors */
	dc->ellipseFixup = true;
#ifdef	__WINDOWS__
	dc->wm.fulldc.hdc 			= NULL;
#endif
	dc->r.getWinDC				= DRV_getWinDC;
	dc->r.getDefaultPalette 	= SVGA_getDefaultPalette;
	dc->r.setColor 				= DRV_setColor;
	dc->r.setBackColor 			= DRV_setBackColor;
	dc->r.setWriteMode			= DRV_setWriteMode;
	dc->r.setClipRect 			= DRV_setClipRect;
	dc->r.getPixel 				= SVGA16_getPixel;
	dc->r.putPixel 				= SVGA16_putPixel;
	dc->r.getScanLine			= SVGA16_getScanLine;
	dc->r.putScanLine			= SVGA16_putScanLine;
	dc->r.stretchScanLine		= SVGA16_stretchScanLine;
	dc->r.stretchScanLine2x		= SVGA16_stretchScanLine2x;

#ifndef	MGL_LITE
	dc->r.setPenStyle			= DRV_setPenStyle;
	dc->r.setPenBitmapPattern	= DRV_setPenBitmapPattern;
	dc->r.setPenPixmapPattern	= DRV_setPenPixmapPattern;
	dc->r.setLineStipple		= DRV_setLineStipple;
	dc->r.setLineStippleCount	= DRV_setLineStippleCount;
	dc->r.putSrcTransparent		= SVGA16_putSrcTransparent;
	dc->r.putDstTransparent		= SVGA16_putDstTransparent;
#endif

	/* Fill in remaining device vectors */
	_MGL_setLineVecs(dc,	  SVGA16_line);
	_MGL_setScanLineVecs(dc,  SVGA16_scanLine);
	_MGL_setFillRectVecs(dc,  SVGA16_fillRect);
	dc->r.putMonoImage			= SVGA16_putMonoImage;
	dc->r.putMouseImage			= __EMU__putMouseImage;
	dc->r.getImage				= __EMU__getImage;
#ifdef  __16BIT__
	dc->r.putImage				= __EMU__putImage;
#else
	dc->r.putImage				= SVGA16_putImage;
#endif
	dc->r.divotSize				= SVGA16_divotSize;
	dc->r.getDivot 				= SVGA16_getDivot;
	dc->r.putDivot 				= SVGA16_putDivot;
	dc->r.stretchBlt1x2			= __EMU__stretchBlt1x2;
	dc->r.stretchBlt2x2			= __EMU__stretchBlt2x2;
	dc->r.stretchBlt			= __EMU__stretchBlt;

#ifndef	MGL_LITE
	dc->r.stippleLine			= __EMU__stippleLine;
	dc->r.dither.line         	= __EMU__line;
	_MGL_setTrapVecs(dc,	  __EMU__trap);
	dc->r.patt.line				= __EMU__fatPenLine;
	dc->r.colorPatt.line		= __EMU__fatPenLine;
	dc->r.dither.scanLine		= __EMU__scanLine;
	dc->r.fatSolid.line			= __EMU__fatPenLine;
	dc->r.fatRopSolid.line		= __EMU__fatPenLine;
	dc->r.fatPatt.line			= __EMU__fatPenLine;
	dc->r.fatColorPatt.line		= __EMU__fatPenLine;
	_MGL_setDrawScanListVecs(dc,SVGA16_drawScanList);
	dc->r.dither.drawScanList 	= __EMU__drawScanList;
	_MGL_setDrawRegionVecs(dc,__EMU__drawRegion);
	_MGL_setEllipseVecs(dc,	  __EMU__ellipse);
	dc->r.dither.ellipse      	= __EMU__ellipse;
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
	dc->r.ditherPolygon 		= __EMU__polygon;
	dc->r.translateImage		= __EMU__translateImage;
	dc->r.bitBlt				= __EMU__bitBlt;
	dc->r.srcTransBlt			= __EMU__srcTransBlt;
	dc->r.dstTransBlt			= __EMU__dstTransBlt;
	dc->r.scanRightForColor		= SVGA16_scanRightForColor;
	dc->r.scanLeftForColor		= SVGA16_scanLeftForColor;
	dc->r.scanRightWhileColor	= SVGA16_scanRightWhileColor;
	dc->r.scanLeftWhileColor	= SVGA16_scanLeftWhileColor;

#ifdef	MGL_FIX3D
	/* 3D rasterisation */
	dc->r.tri 					= __EMU__tri;
	dc->r.ditherTri          	= __EMU__tri;
	dc->r.quad 					= __EMU__quad;
	dc->r.ditherQuad          	= __EMU__quad;

	/* 3D Zbuffering */
	dc->r.zBegin				= __EMU__zBegin;
#endif
#endif
}

ibool _ASMAPI SVGA16_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		SVGA16_initDriver
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
	SVGA_state	*state;
	display_vec	*d;

	/* Allocate a single buffer for the driver specific state buffer */
	if (modeId >= grSVGA_320x200x64k)
		dc->mi = SVGA16_modes[modeId - grSVGA_320x200x64k];
	else
		dc->mi = SVGA15_modes[modeId - grSVGA_320x200x32k];
	if (g_state == NULL) {
		if ((g_state = (void*)MGL_calloc(1,sizeof(SVGA_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	state = (SVGA_state*)(dc->v = g_state);
	d = &dc->v->d;
	d->refCount++;
	d->hwnd = hwnd;
	d->graphDriver = driverId;
	d->graphMode = modeId;

	/* Load the device context with the device information for the
	 * selected video mode
	 */
	dc->deviceType 			= MGL_DISPLAY_DEVICE;
	dc->xInch 				= 9000;
	dc->yInch 				= 7000;
	d->hardwareCursor 		= FALSE;
	d->restoreTextMode 		= SVGA_restoreTextMode;
    d->restoreGraphMode		= SVGA_restoreGraphMode;
	d->destroy				= destroyDC;
	d->makeOffscreenDC 		= NULL;
	d->makeLinearOffscreenDC = NULL;

	_MGL_computePF(dc);
	SVGA16_initInternal(dc);

	/* Set the video mode */
	return SVGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,
		&state->oldBIOSMode,&state->old50Lines,virtualX,virtualY,numBuffers,
		stereo,refreshRate);
}

long _ASMAPI SVGA16_divotSize(MGLDC *dc,int left,int top,int right,
	int bottom)
/****************************************************************************
*
* Function:		SVGA16_divotSize
* Returns:		Memory required to store divot
*
* Description:	For maximum speed in the packed pixel divot code, we align
*               the left and right edges of the divot to 32 bit boundaries.
*
****************************************************************************/
{
	dc = dc;
	return (long)(((right + 1) & ~0x1) - (left & ~0x1)) *
		(long)(bottom - top) * 2 + sizeof(divotHeader);
}
