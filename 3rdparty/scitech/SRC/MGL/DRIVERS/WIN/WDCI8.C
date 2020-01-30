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
* Description:	Main 8 bit windowed device driver file. This file
*				will be linked in when a call to MGL_registerDriver() is
*				made to register this device driver for use. This driver
*				requires either DCI or DirectDraw to be available so that
*				we can use these services to directly access the video
*				memory for the window surface.
*
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\win\wdci8.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI WDCI8_driver = {
	IDENT_FLAG,
	MGL_WDCI8NAME,
	MGL_WDCI8NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
	grNONE,
	NULL,
	WDCI8_initDriver,
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
	if (dc->wm.windc.hdc)
		ReleaseDC(dc->wm.windc.hwnd,dc->wm.windc.hdc);
}

ibool _ASMAPI WDCI_init(MGLDC *dc,ulong hwnd)
/****************************************************************************
*
* Function:		WDCI_init
* Parameters:	dc		- Device context.
*				hwnd	- Handle to window
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the basic functions required for a DCI windowed
*				device.
*
****************************************************************************/
{
	RECT	r;

	/* Initialise the driver */
	dc->wm.windc.hwnd = (HWND)hwnd;
	dc->wm.windc.hpal = NULL;
	dc->wm.windc.hdc = GetDC(dc->wm.windc.hwnd);
	GetClientRect(dc->wm.windc.hwnd,&r);
	dc->mi.xRes = r.right - r.left - 1;
	dc->mi.yRes = r.bottom - r.top - 1;
	dc->mi.maxPage = 0;
	dc->mi.pageSize = 0;
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

	// TODO: Get the surface info for the window!!
	dc->mi.bytesPerLine = 0;
	dc->surface = dc->surfaceStart = NULL;

	/* Load the device context with the device information for the
	 * selected video mode
	 */
	dc->deviceType = MGL_WINDOWED_DEVICE;
	dc->xInch = 0;
	dc->yInch = 0;
	dc->ellipseFixup = false;

	/* Fill in required device context vectors */
	dc->r.getWinDC				= WIN_getWinDC;
	dc->r.getDefaultPalette 	= WIN_getDefaultPalette;
	dc->r.realizePalette 		= WIN_realizePalette;

	/* Initialize event handling functions */
	_EVT_init((HWND)hwnd);
	return true;
}

void WDCI_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		WDCI_initInternal
* Parameters:	dc		- Device context.
*
* Description:	Initializes the default vectors for the DCI device context,
*				by overriding the currently initialized packed pixel
*				rendering vectors with vectors to lock and unlock the
*				framebuffer via DCI services before drawing directly on it.  
*
****************************************************************************/
{
	/* First vector all rendering routines to our stub functions that
	 * correctly arbitrate between direct memory access and GDI
	 * access. Note that we save a complete copy of the packed pixel
	 * render vectors active before we override them with our stub
	 * functions (so we can call the original routines directly).
	 */
	_PACKED_vecs = dc->r;
	dc->r.beginDirectAccess 	= WDCI_beginDirectAccess;
	dc->r.endDirectAccess 		= WDCI_endDirectAccess;
	dc->r.beginPixel 			= WDCI_beginPixel;
	dc->r.endPixel 				= WDCI_endPixel;

	dc->r.getPixel				= WDCISTUB_getPixel;
	dc->r.putPixel				= WDCISTUB_putPixel;
	dc->r.putMonoImage			= WDCISTUB_putMonoImage;
	dc->r.putMouseImage			= WDCISTUB_putMouseImage;
	dc->r.getImage				= WDCISTUB_getImage;
	dc->r.putImage				= WDCISTUB_putImage;
	dc->r.getDivot 				= WDCISTUB_getDivot;
	dc->r.putDivot 				= WDCISTUB_putDivot;
	dc->r.stretchBlt1x2			= WDCISTUB_stretchBlt1x2;
	dc->r.stretchBlt2x2			= WDCISTUB_stretchBlt2x2;
	dc->r.stretchBlt			= WDCISTUB_stretchBlt;

	dc->r.solid.line			= WDCISTUB_solidLine;
	dc->r.ropSolid.line			= WDCISTUB_ropSolidLine;
	dc->r.patt.line				= WDCISTUB_pattLine;
	dc->r.colorPatt.line		= WDCISTUB_colorPattLine;
	dc->r.fatSolid.line			= WDCISTUB_fatSolidLine;
	dc->r.fatRopSolid.line		= WDCISTUB_fatRopSolidLine;
	dc->r.fatPatt.line			= WDCISTUB_fatPattLine;
	dc->r.fatColorPatt.line		= WDCISTUB_fatColorPattLine;
	dc->r.dither.line			= WDCISTUB_ditherLine;

	dc->r.solid.scanLine		= WDCISTUB_solidScanLine;
	dc->r.ropSolid.scanLine		= WDCISTUB_ropSolidScanLine;
	dc->r.patt.scanLine			= WDCISTUB_pattScanLine;
	dc->r.colorPatt.scanLine	= WDCISTUB_colorPattScanLine;
	dc->r.fatSolid.scanLine		= WDCISTUB_fatSolidScanLine;
	dc->r.fatRopSolid.scanLine	= WDCISTUB_fatRopSolidScanLine;
	dc->r.fatPatt.scanLine		= WDCISTUB_fatPattScanLine;
	dc->r.fatColorPatt.scanLine	= WDCISTUB_fatColorPattScanLine;
	dc->r.dither.scanLine		= WDCISTUB_ditherScanLine;

	dc->r.solid.fillRect		= WDCISTUB_solidFillRect;
	dc->r.ropSolid.fillRect		= WDCISTUB_ropSolidFillRect;
	dc->r.patt.fillRect			= WDCISTUB_pattFillRect;
	dc->r.colorPatt.fillRect	= WDCISTUB_colorPattFillRect;
	dc->r.fatSolid.fillRect		= WDCISTUB_fatSolidFillRect;
	dc->r.fatRopSolid.fillRect	= WDCISTUB_fatRopSolidFillRect;
	dc->r.fatPatt.fillRect		= WDCISTUB_fatPattFillRect;
	dc->r.fatColorPatt.fillRect	= WDCISTUB_fatColorPattFillRect;
	dc->r.dither.fillRect		= WDCISTUB_ditherFillRect;

	dc->r.solid.drawScanList		= WDCISTUB_solidDrawScanList;
	dc->r.ropSolid.drawScanList		= WDCISTUB_ropSolidDrawScanList;
	dc->r.patt.drawScanList			= WDCISTUB_pattDrawScanList;
	dc->r.colorPatt.drawScanList	= WDCISTUB_colorPattDrawScanList;
	dc->r.fatSolid.drawScanList		= WDCISTUB_fatSolidDrawScanList;
	dc->r.fatRopSolid.drawScanList	= WDCISTUB_fatRopSolidDrawScanList;
	dc->r.fatPatt.drawScanList		= WDCISTUB_fatPattDrawScanList;
	dc->r.fatColorPatt.drawScanList	= WDCISTUB_fatColorPattDrawScanList;
	dc->r.dither.drawScanList		= WDCISTUB_ditherDrawScanList;

	dc->r.solid.trap			= WDCISTUB_solidTrap;
	dc->r.ropSolid.trap			= WDCISTUB_ropSolidTrap;
	dc->r.patt.trap				= WDCISTUB_pattTrap;
	dc->r.colorPatt.trap		= WDCISTUB_colorPattTrap;
	dc->r.fatSolid.trap			= WDCISTUB_fatSolidTrap;
	dc->r.fatRopSolid.trap		= WDCISTUB_fatRopSolidTrap;
	dc->r.fatPatt.trap			= WDCISTUB_fatPattTrap;
	dc->r.fatColorPatt.trap		= WDCISTUB_fatColorPattTrap;
	dc->r.dither.trap			= WDCISTUB_ditherTrap;

	dc->r.solid.drawRegion			= WDCISTUB_solidDrawRegion;
	dc->r.ropSolid.drawRegion		= WDCISTUB_ropSolidDrawRegion;
	dc->r.patt.drawRegion			= WDCISTUB_pattDrawRegion;
	dc->r.colorPatt.drawRegion		= WDCISTUB_colorPattDrawRegion;
	dc->r.fatSolid.drawRegion		= WDCISTUB_fatSolidDrawRegion;
	dc->r.fatRopSolid.drawRegion	= WDCISTUB_fatRopSolidDrawRegion;
	dc->r.fatPatt.drawRegion		= WDCISTUB_fatPattDrawRegion;
	dc->r.fatColorPatt.drawRegion	= WDCISTUB_fatColorPattDrawRegion;
	dc->r.dither.drawRegion			= WDCISTUB_ditherDrawRegion;

	dc->r.solid.ellipse			= WDCISTUB_solidEllipse;
	dc->r.ropSolid.ellipse		= WDCISTUB_ropSolidEllipse;
	dc->r.patt.ellipse			= WDCISTUB_pattEllipse;
	dc->r.colorPatt.ellipse		= WDCISTUB_colorPattEllipse;
	dc->r.fatSolid.ellipse		= WDCISTUB_fatSolidEllipse;
	dc->r.fatRopSolid.ellipse	= WDCISTUB_fatRopSolidEllipse;
	dc->r.fatPatt.ellipse		= WDCISTUB_fatPattEllipse;
	dc->r.fatColorPatt.ellipse	= WDCISTUB_fatColorPattEllipse;
	dc->r.dither.ellipse		= WDCISTUB_ditherEllipse;

	dc->r.solid.fillEllipse			= WDCISTUB_solidFillEllipse;
	dc->r.ropSolid.fillEllipse		= WDCISTUB_ropSolidFillEllipse;
	dc->r.patt.fillEllipse			= WDCISTUB_pattFillEllipse;
	dc->r.colorPatt.fillEllipse		= WDCISTUB_colorPattFillEllipse;
	dc->r.fatSolid.fillEllipse		= WDCISTUB_fatSolidFillEllipse;
	dc->r.fatRopSolid.fillEllipse	= WDCISTUB_fatRopSolidFillEllipse;
	dc->r.fatPatt.fillEllipse		= WDCISTUB_fatPattFillEllipse;
	dc->r.fatColorPatt.fillEllipse	= WDCISTUB_fatColorPattFillEllipse;
	dc->r.dither.fillEllipse		= WDCISTUB_ditherFillEllipse;

	dc->r.solid.ellipseArc			= WDCISTUB_solidEllipseArc;
	dc->r.ropSolid.ellipseArc		= WDCISTUB_ropSolidEllipseArc;
	dc->r.patt.ellipseArc			= WDCISTUB_pattEllipseArc;
	dc->r.colorPatt.ellipseArc		= WDCISTUB_colorPattEllipseArc;
	dc->r.fatSolid.ellipseArc		= WDCISTUB_fatSolidEllipseArc;
	dc->r.fatRopSolid.ellipseArc	= WDCISTUB_fatRopSolidEllipseArc;
	dc->r.fatPatt.ellipseArc		= WDCISTUB_fatPattEllipseArc;
	dc->r.fatColorPatt.ellipseArc	= WDCISTUB_fatColorPattEllipseArc;
	dc->r.dither.ellipseArc			= WDCISTUB_ditherEllipseArc;

	dc->r.solid.fillEllipseArc			= WDCISTUB_solidFillEllipseArc;
	dc->r.ropSolid.fillEllipseArc		= WDCISTUB_ropSolidFillEllipseArc;
	dc->r.patt.fillEllipseArc			= WDCISTUB_pattFillEllipseArc;
	dc->r.colorPatt.fillEllipseArc		= WDCISTUB_colorPattFillEllipseArc;
	dc->r.fatSolid.fillEllipseArc		= WDCISTUB_fatSolidFillEllipseArc;
	dc->r.fatRopSolid.fillEllipseArc	= WDCISTUB_fatRopSolidFillEllipseArc;
	dc->r.fatPatt.fillEllipseArc		= WDCISTUB_fatPattFillEllipseArc;
	dc->r.fatColorPatt.fillEllipseArc	= WDCISTUB_fatColorPattFillEllipseArc;
	dc->r.dither.fillEllipseArc			= WDCISTUB_ditherFillEllipseArc;

	dc->r.stippleLine			= WDCISTUB_stippleLine;
	dc->r.drawStrBitmap			= WDCISTUB_drawStrBitmap;
	dc->r.drawCharVec			= WDCISTUB_drawCharVec;
	dc->r.complexPolygon		= WDCISTUB_complexPolygon;
	dc->r.polygon				= WDCISTUB_polygon;
	dc->r.ditherPolygon			= WDCISTUB_ditherPolygon;
	dc->r.translateImage		= WDCISTUB_translateImage;
	dc->r.bitBlt				= WDCISTUB_bitBlt;
	dc->r.srcTransBlt			= WDCISTUB_srcTransBlt;
	dc->r.dstTransBlt			= WDCISTUB_dstTransBlt;
	dc->r.scanRightForColor		= WDCISTUB_scanRightForColor;
	dc->r.scanLeftForColor		= WDCISTUB_scanLeftForColor;
	dc->r.scanRightWhileColor	= WDCISTUB_scanRightWhileColor;
	dc->r.scanLeftWhileColor	= WDCISTUB_scanLeftWhileColor;

	dc->r.cLine					= WDCISTUB_cLine;
	dc->r.rgbLine               = WDCISTUB_rgbLine;
	dc->r.tri                   = WDCISTUB_tri;
	dc->r.ditherTri				= WDCISTUB_ditherTri;
	dc->r.cTri                  = WDCISTUB_cTri;
	dc->r.rgbTri                = WDCISTUB_rgbTri;
	dc->r.quad                  = WDCISTUB_quad;
	dc->r.ditherQuad			= WDCISTUB_ditherQuad;
	dc->r.cQuad                 = WDCISTUB_cQuad;
	dc->r.rgbQuad               = WDCISTUB_rgbQuad;
	dc->r.cTrap                 = WDCISTUB_cTrap;
	dc->r.rgbTrap               = WDCISTUB_rgbTrap;

	dc->r.z16.zLine				= WDCISTUB_z16_zLine;
	dc->r.z16.zDitherLine		= WDCISTUB_z16_zDitherLine;
	dc->r.z16.czLine            = WDCISTUB_z16_czLine;
	dc->r.z16.rgbzLine          = WDCISTUB_z16_rgbzLine;
	dc->r.z16.zTri              = WDCISTUB_z16_zTri;
	dc->r.z16.zDitherTri        = WDCISTUB_z16_zDitherTri;
	dc->r.z16.czTri             = WDCISTUB_z16_czTri;
	dc->r.z16.rgbzTri           = WDCISTUB_z16_rgbzTri;
	dc->r.z16.zQuad             = WDCISTUB_z16_zQuad;
	dc->r.z16.zDitherQuad       = WDCISTUB_z16_zDitherQuad;
	dc->r.z16.czQuad            = WDCISTUB_z16_czQuad;
	dc->r.z16.rgbzQuad          = WDCISTUB_z16_rgbzQuad;
	dc->r.z16.zTrap             = WDCISTUB_z16_zTrap;
	dc->r.z16.zDitherTrap       = WDCISTUB_z16_zDitherTrap;
	dc->r.z16.czTrap            = WDCISTUB_z16_czTrap;
	dc->r.z16.rgbzTrap          = WDCISTUB_z16_rgbzTrap;

	dc->r.z32.zLine				= WDCISTUB_z32_zLine;
	dc->r.z32.zDitherLine		= WDCISTUB_z32_zDitherLine;
	dc->r.z32.czLine            = WDCISTUB_z32_czLine;
	dc->r.z32.rgbzLine          = WDCISTUB_z32_rgbzLine;
	dc->r.z32.zTri              = WDCISTUB_z32_zTri;
	dc->r.z32.zDitherTri        = WDCISTUB_z32_zDitherTri;
	dc->r.z32.czTri             = WDCISTUB_z32_czTri;
	dc->r.z32.rgbzTri           = WDCISTUB_z32_rgbzTri;
	dc->r.z32.zQuad             = WDCISTUB_z32_zQuad;
	dc->r.z32.zDitherQuad       = WDCISTUB_z32_zDitherQuad;
	dc->r.z32.czQuad            = WDCISTUB_z32_czQuad;
	dc->r.z32.rgbzQuad          = WDCISTUB_z32_rgbzQuad;
	dc->r.z32.zTrap             = WDCISTUB_z32_zTrap;
	dc->r.z32.zDitherTrap       = WDCISTUB_z32_zDitherTrap;
	dc->r.z32.czTrap            = WDCISTUB_z32_czTrap;
	dc->r.z32.rgbzTrap          = WDCISTUB_z32_rgbzTrap;
}

ibool _ASMAPI WDCI8_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers)
/****************************************************************************
*
* Function:		WDCI8_initDriver
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
	dc->v->w.destroy			= destroyDC;
	if (!WDCI_init(dc,hwnd))
    	return false;
	PACKED8_initInternal(dc);
	WDCI_initInternal(dc);
	return true;
}

