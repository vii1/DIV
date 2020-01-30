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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Main WINDC device driver for rendering directly into a
*				Windows device context. The Windowed DC device driver does
*				not directly product output using MGL rendering commands,
*				except for the BitBlt operations between a MEMORYDC and
*				a WINDC. Using the MGL under Windows requires creating a
*				WINDC for communicating with the GDI, and creating a
*				MEMORYDC (which is in fact a WinG DC) for handling all
*				MGL output. When the MEMORYDC is constructed, it can then
*				be Blt'ed to the display (which does this using WinG or
*				the appropriate GDI functions).
*
*				Hence the only thing that the WINDC handles is BitBlt's and
*				palette management (to ensure our palettes are updated
*				correctly, so that we dont get BitBlt slowdown's due to
*				palette translation).
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\win\windc.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI WINDC_driver = {
	IDENT_FLAG,
	"WINDC.DRV",
	"WINDC.DRV",
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
	grNONE,
	NULL,
	WINDC_initDriver,
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

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
	/* If we have a palette associated with this DC, then destroy it */
	if (dc->wm.windc.hpal)
		DeleteObject(dc->wm.windc.hpal);
	if (!_MGL_haveWin31 && dc->wm.windc.hdc)
		ReleaseDC(dc->wm.windc.hwnd,dc->wm.windc.hdc);
}

void WINDC_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		WINDC_initInternal
* Parameters:	dc	- Device context.
*
* Description:	Initialises the device driver internal tables for 8 bit
*				unaccelerated driver routines.
*
****************************************************************************/
{
	// TODO: Flesh these functions out with versions that render via
    // 		 GDI so the MGL drawing functions can be used directly with
    //		 GDI surfaces.
	dc->r.setColor 				= DRV_setColor;
	dc->r.setBackColor 			= DRV_setBackColor;
	dc->r.setWriteMode			= DRV_setWriteMode;
	dc->r.setClipRect 			= DRV_setClipRect;
#ifndef	MGL_LITE
	dc->r.setPenStyle			= DRV_setPenStyle;
	dc->r.setPenBitmapPattern	= DRV_setPenBitmapPattern;
	dc->r.setPenPixmapPattern	= DRV_setPenPixmapPattern;
	dc->r.setLineStipple		= DRV_setLineStipple;
	dc->r.setLineStippleCount	= DRV_setLineStippleCount;
#endif

//	dc->r.getPixel 				= WIN_getPixel;
//	dc->r.putPixel 				= WIN_putPixel;
//	dc->r.getScanLine			= WIN_getScanLine;
//	dc->r.putScanLine			= WIN_putScanLine;
//	_MGL_setLineVecs(dc,		  WIN_line);
//	dc->r.solid.scanLine 		= WIN_scanLine;
//	dc->r.ropSolid.scanLine 	= WIN_scanLine;
//	_MGL_setFillRectVecs(dc,	  WIN_fillRect);
//	dc->r.putMonoImage			= WIN_putMonoImage;
//	dc->r.putMouseImage			= WIN_putMouseImage;
//	dc->r.getImage				= WIN_getImage;
	dc->r.putImage				= WIN_putImage;
//	dc->r.divotSize				= WIN_divotSize;
//	dc->r.getDivot 				= WIN_getDivot;
//	dc->r.putDivot 				= WIN_putDivot;
//	dc->r.stretchScanLine2x		= WIN_stretchScanLine2x;
//	dc->r.stretchScanLine		= WIN_stretchScanLine;
	dc->r.stretchBlt1x2			= WIN_stretchBlt1x2;
	dc->r.stretchBlt2x2			= WIN_stretchBlt2x2;
	dc->r.stretchBlt			= WIN_stretchBlt;

#ifndef	MGL_LITE
//	dc->r.stippleLine			= WIN_stippleLine;
//	_MGL_setTrapVecs(dc,	  	  WIN_trap);
//	dc->r.putSrcTransparent		= WIN_putSrcTransparent;
//	dc->r.putDstTransparent		= WIN_putDstTransparent;
	dc->r.patt.line				= __EMU__fatPenLine;
	dc->r.colorPatt.line		= __EMU__fatPenLine;
	dc->r.fatSolid.line			= __EMU__fatPenLine;
	dc->r.fatRopSolid.line		= __EMU__fatPenLine;
	dc->r.fatPatt.line			= __EMU__fatPenLine;
	dc->r.fatColorPatt.line		= __EMU__fatPenLine;
//	dc->r.patt.scanLine 		= WIN_scanLine;
//	dc->r.colorPatt.scanLine 	= WIN_scanLine;
//	dc->r.fatSolid.scanLine 	= WIN_scanLine;
//	dc->r.fatRopSolid.scanLine 	= WIN_scanLine;
//	dc->r.fatPatt.scanLine 		= WIN_scanLine;
//	dc->r.fatColorPatt.scanLine = WIN_scanLine;
//	dc->r.dither.scanLine 		= WIN_scanLine;
	_MGL_setDrawScanListVecs(dc,  __EMU__drawScanList);
	_MGL_setDrawRegionVecs(dc,	  __EMU__drawRegion);
	_MGL_setEllipseVecs(dc,	  	  __EMU__ellipse);
//	_MGL_setEllipseVecs(dc,	  	  WIN_ellipse);
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
	dc->r.translateImage		= WIN_translateImage;
//	dc->r.bitBlt				= WIN_bitBlt;
//	dc->r.srcTransBlt			= WIN_srcTransBlt;
//	dc->r.dstTransBlt			= WIN_dstTransBlt;
//	dc->r.scanRightForColor		= WIN_scanRightForColor;
//	dc->r.scanLeftForColor		= WIN_scanLeftForColor;
//	dc->r.scanRightWhileColor	= WIN_scanRightWhileColor;
//	dc->r.scanLeftWhileColor	= WIN_scanLeftWhileColor;
#endif
}

ibool _ASMAPI WINDC_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		WINDC_initDriver
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
	RECT	r;

	/* Allocate a single buffer for the driver specific state buffer */
	if (g_state == NULL) {
		if ((g_state = MGL_calloc(1,sizeof(WINDC_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	dc->v = g_state;
	dc->v->m.refCount++;
	dc->wm.windc.hwnd = (HWND)hwnd;
	dc->wm.windc.hpal = NULL;

	/* Initialise the driver */
	dc->wm.windc.hdc = GetDC(dc->wm.windc.hwnd);
	GetClientRect(dc->wm.windc.hwnd,&r);
	dc->mi.xRes = r.right - r.left - 1;
	dc->mi.yRes = r.bottom - r.top - 1;
	dc->mi.maxPage = 0;
	dc->mi.bytesPerLine = 0;
	dc->mi.pageSize = 0;
	dc->surface = dc->surfaceStart = NULL;
	WIN_findPixelFormat(dc,dc->wm.windc.hdc);
	switch (dc->mi.bitsPerPixel) {
		case 4:		dc->mi.maxColor = 0xFUL;		break;
		case 8:		dc->mi.maxColor = 0xFFUL;		break;
		case 15:	dc->mi.maxColor = 0x7FFFUL;		break;
		case 16:	dc->mi.maxColor = 0xFFFFUL;		break;
		case 24:	dc->mi.maxColor = 0xFFFFFFUL;	break;
		case 32:	dc->mi.maxColor = 0xFFFFFFFFUL;	break;
		}
	dc->mi.aspectRatio = GetDeviceCaps(dc->wm.windc.hdc,ASPECTY) * 100;
	if (_MGL_haveWin31)
		ReleaseDC(dc->wm.windc.hwnd,dc->wm.windc.hdc);

	/* Load the device context with the device information for the
	 * selected video mode
	 */
	dc->deviceType = MGL_WINDOWED_DEVICE;
	dc->xInch = 0;
	dc->yInch = 0;
	dc->ellipseFixup = false;
	dc->v->w.destroy		= destroyDC;

	/* Fill in required device context vectors */
	dc->r.getWinDC				= WIN_getWinDC;
	dc->r.getDefaultPalette 	= WIN_getDefaultPalette;
	dc->r.realizePalette 		= WIN_realizePalette;
	dc->r.setClipRect 			= DRV_setClipRect;
    WINDC_initInternal(dc);

	/* Initialize event handling functions */
	_EVT_init((HWND)hwnd);
	return true;
}

