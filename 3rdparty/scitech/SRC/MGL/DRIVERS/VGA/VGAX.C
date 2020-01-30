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
* Description:	Main VGA ModeX style 256 color device driver file. This
*				file will be linked in when a call to MGL_registerDriver()
*				is made to register this device driver for use.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\vga\vgax.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI VGAX_driver = {
	IDENT_FLAG,
	MGL_VGAXNAME,
	MGL_VGAXNAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grVGA,
	VGAX_detect,
	VGAX_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t VGAX_modes[] = {
	{319,199,8,1,0xFF,3,80,1200,0x3E80L,0x13},
	{319,239,8,1,0xFF,2,80,1000,0x4B00L,0x13},
	{319,399,8,1,0xFF,1,80,600,0x7D00L,0x13},
	{319,479,8,1,0xFF,0,80,500,0x9600L,0x13},
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/* Mode X CRTC register tweaks for the various resolutions supported by
 * this library. These values assume that the BIOS mode 13h has been
 * previously programmed.
 */

PRIVATE ushort X320Y200[] = {
	0x63,					/* Dot clock							*/
	2,						/* Number of CRTC Registers to update	*/
	0x0014,					/* Turn off dword mode					*/
	0xE317,					/* Turn on byte mode					*/
	};

PRIVATE ushort X320Y240[] = {
	0xE3,					/* Dot clock							*/
	10,						/* Number of CRTC Registers to update	*/
	0x0D06,					/* Vertical total						*/
	0x3E07,					/* Overflow (bit 8 of vertical counts)	*/
	0x4109,					/* Cell height (2 to double-scan)		*/
	0xEA10,					/* V sync start							*/
	0xAC11,					/* V sync end and protect cr0-cr7		*/
	0xDF12,					/* Vertical displayed					*/
	0x0014,					/* Turn off dword mode					*/
	0xE715,					/* V blank start						*/
	0x0616,					/* V blank end							*/
	0xE317,					/* Turn on byte mode					*/
	};

PRIVATE ushort X320Y400[] = {
	0x63,					/* Dot clock							*/
	3,						/* Number of CRTC Registers to update	*/
	0x4009,					/* Cell height							*/
	0x0014,					/* Turn off dword mode					*/
	0xE317,					/* Turn on byte mode					*/
	};

PRIVATE ushort X320Y480[] = {
	0xE3,					/* Dot clock							*/
	10,						/* Number of CRTC Registers to update	*/
	0x0D06,					/* Vertical total						*/
	0x3E07,					/* Overflow (bit 8 of vertical counts)	*/
	0x4009,					/* Cell height (2 to double-scan)		*/
	0xEA10,					/* V sync start							*/
	0xAC11,					/* V sync end and protect cr0-cr7		*/
	0xDF12,					/* Vertical displayed					*/
	0x0014,					/* Turn off dword mode					*/
	0xE715,					/* V blank start						*/
	0x0616,					/* V blank end							*/
	0xE317,					/* Turn on byte mode					*/
	};

/* Lookup table to find the correct CRTC programming table for each mode */

PRIVATE	ushort	*VGAX_tweaks[] = {
	X320Y200,
	X320Y240,
	X320Y400,
	X320Y480,
	};

/* Plane masks for clipping left and right edges of scanlines */

uchar	_VARAPI VGAX_leftMask[] =	{0x0F,0x0E,0x0C,0x08};
uchar	_VARAPI VGAX_rightMask[] =	{0x01,0x03,0x07,0x0F};

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI VGAX_detect(int id,int force,int *driver,int *mode,modetab modes)
/****************************************************************************
*
* Function:		VGAX_detect
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
		VGA_useMode(modes,grVGAX_320x200x256,id,&VGAX_modes[0]);
		VGA_useMode(modes,grVGAX_320x240x256,id,&VGAX_modes[1]);
		VGA_useMode(modes,grVGAX_320x400x256,id,&VGAX_modes[2]);
		VGA_useMode(modes,grVGAX_320x480x256,id,&VGAX_modes[3]);
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

void VGAX_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		VGAX_initInternal
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver tables.
*
****************************************************************************/
{
	display_vec	*d = &dc->v->d;

	/* Fill in required device context vectors */
	dc->ellipseFixup = true;
#ifdef	__WINDOWS__
	dc->wm.fulldc.hdc = NULL;
#endif
	d->maxProgram				= 256;
	dc->r.getWinDC				= DRV_getWinDC;
	dc->r.getDefaultPalette 	= VGAX_getDefaultPalette;
	dc->r.realizePalette 		= VGA_realizePalette;
	dc->r.setColor 				= DRV_setColor;
	dc->r.setBackColor 			= DRV_setBackColor;
	dc->r.setWriteMode			= DRV_setWriteMode;
	dc->r.setClipRect 			= DRV_setClipRect;
	dc->r.beginPixel 			= VGAX_beginPixel;
	dc->r.getPixel 				= VGAX_getPixel;
	dc->r.putPixel 				= VGAX_putPixel;
	dc->r.endPixel 				= VGAX_endPixel;
	dc->r.getScanLine			= VGAX_getScanLine;
	dc->r.putScanLine			= VGAX_putScanLine;
	dc->r.stretchScanLine		= VGAX_stretchScanLine;
	dc->r.stretchScanLine2x		= VGAX_stretchScanLine2x;

#ifndef	MGL_LITE
	dc->r.setPenStyle			= DRV_setPenStyle;
	dc->r.setPenBitmapPattern	= DRV_setPenBitmapPattern;
	dc->r.setPenPixmapPattern	= DRV_setPenPixmapPattern;
	dc->r.setLineStipple		= DRV_setLineStipple;
	dc->r.setLineStippleCount	= DRV_setLineStippleCount;
	dc->r.ditherPixel			= VGAX_ditherPixel;
	dc->r.putSrcTransparent		= VGAX_putSrcTransparent;
	dc->r.putDstTransparent		= VGAX_putDstTransparent;
	dc->r.stretchBlt1x2			= __EMU__stretchBlt1x2;
	dc->r.stretchBlt2x2			= __EMU__stretchBlt2x2;
	dc->r.stretchBlt			= __EMU__stretchBlt;
#endif

	/* Fill in remaining device vectors */
	_MGL_setLineVecs(dc,	  VGAX_line);
	_MGL_setScanLineVecs(dc,  VGAX_scanLine);
	_MGL_setFillRectVecs(dc,  VGAX_fillRect);
	dc->r.putMonoImage			= VGAX_putMonoImage;
	dc->r.putMouseImage			= __EMU__putMouseImage;
	dc->r.getImage				= __EMU__getImage;
	dc->r.putImage				= VGAX_putImage;
	dc->r.divotSize				= VGAX_divotSize;
	dc->r.getDivot 				= __EMU__getDivot;
	dc->r.putDivot 				= __EMU__putDivot;

#ifndef	MGL_LITE
	dc->r.stippleLine			= __EMU__stippleLine;
	dc->r.dither.line         	= __EMU__line;
	_MGL_setTrapVecs(dc,	  __EMU__trap);
	dc->r.patt.line				= __EMU__fatPenLine;
	dc->r.colorPatt.line		= __EMU__fatPenLine;
	dc->r.fatSolid.line			= __EMU__fatPenLine;
	dc->r.fatRopSolid.line		= __EMU__fatPenLine;
	dc->r.fatPatt.line			= __EMU__fatPenLine;
	dc->r.fatColorPatt.line		= __EMU__fatPenLine;
	dc->r.dither.scanLine		= __EMU__scanLine;
	_MGL_setDrawScanListVecs(dc,VGAX_drawScanList);
	dc->r.dither.drawScanList 	= __EMU__drawScanList;
	_MGL_setDrawRegionVecs(dc,__EMU__drawRegion);
	_MGL_setEllipseVecs(dc,	  VGAX_ellipse);
	dc->r.dither.ellipse    	= __EMU__ellipse;
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
	dc->r.scanRightForColor		= __EMU__scanRightForColor;
	dc->r.scanLeftForColor		= __EMU__scanLeftForColor;
	dc->r.scanRightWhileColor	= __EMU__scanRightWhileColor;
	dc->r.scanLeftWhileColor	= __EMU__scanLeftWhileColor;

#ifdef	MGL_FIX3D
	/* 3D rasterisation */
	dc->r.rgbLine 				= __EMU__rgbLine;
	dc->r.tri 					= __EMU__tri;
	dc->r.ditherTri          	= __EMU__tri;
	dc->r.cTri 					= __EMU__cTri;
	dc->r.rgbTri 				= __EMU__rgbTri;
	dc->r.quad 					= __EMU__quad;
	dc->r.ditherQuad          	= __EMU__quad;
	dc->r.cQuad 				= __EMU__cQuad;
	dc->r.rgbQuad 				= __EMU__rgbQuad;
	dc->r.cTrap					= __EMU__cTrap;
	dc->r.rgbTrap				= __EMU__rgbTrap;

	/* 3D Zbuffering (not supported) */
	dc->r.zBegin				= __EMU__zBegin;
#endif
#endif
}

void EnterModeX(display_vec	*d)
/****************************************************************************
*
* Function:		EnterModeX
*
* Description:	Puts the graphics controller into ModeX mode.
*
****************************************************************************/
{
	int			i;
	ushort		*t;

	/* Tweak the CRTC controller into the appropriate ModeX video mode */
	UV_outpw(0x3C4,0x0604);		/* Disable chain-4					*/
	UV_outpw(0x3C4,0x0100);		/* Synchronous reset for clock chg.	*/
	t = VGAX_tweaks[d->graphMode - grVGAX_320x200x256];
	UV_outp(0x3C2,*t++);		/* Set new dot clock value			*/
	UV_outpw(0x3C4,0x0300);		/* Restart sequencer again			*/

	UV_clrinx(0x3D4,0x11,0x80);	/* No write protect on CRTC regs	*/
	for (i = *t++; i > 0; i--)
		UV_outpw(0x3D4,*t++);	/* Program CRTC tweaks				*/
	UV_outp(0x3C4,0x2);			/* Set sequencer 2 point 2 map mask	*/
}

ibool _ASMAPI VGAX_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		VGAX_initDriver
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
	VGAX_state	*state;
	display_vec	*d;

	/* This driver does not support scrolling surfaces */
	if (virtualX != -1 || virtualY != -1)
		return false;
	if (stereo)
    	return false;
	numBuffers = numBuffers;

	/* Allocate a single buffer for the driver specific state buffer */
	dc->mi = VGAX_modes[modeId - grVGAX_320x200x256];
	if (g_state == NULL) {
		if ((g_state = MGL_calloc(1,sizeof(VGAX_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	state = (VGAX_state*)(dc->v = g_state);
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
	d->restoreTextMode 		= VGAX_restoreTextMode;
	d->restoreGraphMode		= VGAX_restoreGraphMode;
	d->destroy				= destroyDC;
	d->makeOffscreenDC 		= NULL;
	d->makeLinearOffscreenDC = NULL;
	_MGL_computePF(dc);
	VGAX_initInternal(dc);

	/* Set the video mode */
	if (!VGA_setMode(dc,dc->mi.scratch1,0,&state->oldBIOSMode,
			&state->old50Lines,NULL))
		return false;
	EnterModeX(d);
	return true;
}

ibool _ASMAPI VGAX_restoreGraphMode(MGLDC *dc)
{
	short t1; ibool t2;
	if (!VGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,&t1,&t2,NULL))
		return false;
	EnterModeX(&dc->v->d);
    return true;
}

void _ASMAPI VGAX_restoreTextMode(MGLDC *dc)
{
	VGAX_state *state = (VGAX_state*)dc->v;
	VGA_restoreMode(state->oldBIOSMode,state->old50Lines,NULL);
}

void * _ASMAPI VGAX_getDefaultPalette(MGLDC *dc)
{ dc = dc; return _VGA8_defPal; }

long _ASMAPI VGAX_divotSize(MGLDC *dc,int left,int top,int right,
	int bottom)
/****************************************************************************
*
* Function:		VGAX_divotSize
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

#ifndef	MGL_LITE

void _ASMAPI VGAX_ditherPixel(int x,int y)
/****************************************************************************
*
* Function:		VGAX_ditherPixel
* Parameters:	dc	- Device context
*				x,y	- Coordinate to read pixel from
*
****************************************************************************/
{
	color_t	old = DC.intColor;
	MGL_halfTonePixelFast2(DC.intColor,x,y,old);
	DC.r.putPixel(x,y);
	DC.intColor = old;
}

#endif	/* !MGL_LITE */

/* The following scanline stretching functions are yet to be written */

#pragma warn -par
void _ASMAPI VGAX_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer)
{}

void _ASMAPI VGAX_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,int *repCounts)
{}

void _ASMAPI VGAX_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent)
{}

void _ASMAPI VGAX_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,color_t transparent)
{}
