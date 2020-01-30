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
* Description:	Main Linear Framebuffer SuperVGA 32 bit device driver.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\svga\linear32.h"
#include "drivers\packed\packed32.h"

#ifndef __16BIT__

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI LINEAR32_driver = {
	IDENT_FLAG,
	MGL_LINEAR32NAME,
	MGL_LINEAR32NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grVESA,
	LINEAR32_detect,
	LINEAR32_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t LINEAR32_modes[] = {
	{319,199,32,1,0xFFFFFFFFL,0,0,1200,0,0x4F02},
	{319,239,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{319,399,32,1,0xFFFFFFFFL,0,0,600,0,0x4F02},
	{319,479,32,1,0xFFFFFFFFL,0,0,500,0,0x4F02},
	{399,299,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{511,383,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{639,349,32,1,0xFFFFFFFFL,0,0,1371,0,0x4F02},
	{639,399,32,1,0xFFFFFFFFL,0,0,1200,0,0x4F02},
	{639,479,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{799,599,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{1023,767,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{1151,863,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{1279,959,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	{1279,1023,32,1,0xFFFFFFFFL,0,0,938,0,0x4F02},
	{1599,1199,32,1,0xFFFFFFFFL,0,0,1000,0,0x4F02},
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI LINEAR32_detect(int id,int force,int *driver,int *mode,modetab modes)
/****************************************************************************
*
* Function:		LINEAR32_detect
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
		SVGALIN_useMode(modes,grSVGA_320x200x4G,id,&LINEAR32_modes[0],true);
		SVGALIN_useMode(modes,grSVGA_320x240x4G,id,&LINEAR32_modes[1],true);
		SVGALIN_useMode(modes,grSVGA_320x400x4G,id,&LINEAR32_modes[2],true);
		SVGALIN_useMode(modes,grSVGA_320x480x4G,id,&LINEAR32_modes[3],true);
		SVGALIN_useMode(modes,grSVGA_400x300x4G,id,&LINEAR32_modes[4],true);
		SVGALIN_useMode(modes,grSVGA_512x384x4G,id,&LINEAR32_modes[5],true);
		SVGALIN_useMode(modes,grSVGA_640x350x4G,id,&LINEAR32_modes[6],true);
		SVGALIN_useMode(modes,grSVGA_640x400x4G,id,&LINEAR32_modes[7],true);
		SVGALIN_useMode(modes,grSVGA_640x480x4G,id,&LINEAR32_modes[8],true);
		SVGALIN_useMode(modes,grSVGA_800x600x4G,id,&LINEAR32_modes[9],true);
		SVGALIN_useMode(modes,grSVGA_1024x768x4G,id,&LINEAR32_modes[10],true);
		SVGALIN_useMode(modes,grSVGA_1152x864x4G,id,&LINEAR32_modes[11],true);
		SVGALIN_useMode(modes,grSVGA_1280x960x4G,id,&LINEAR32_modes[12],true);
		SVGALIN_useMode(modes,grSVGA_1280x1024x4G,id,&LINEAR32_modes[13],true);
		SVGALIN_useMode(modes,grSVGA_1600x1200x4G,id,&LINEAR32_modes[14],true);
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
	FULLSCREEN_destroyHDC(dc);
	if (--dc->v->d.refCount == 0) {
		if (dc->v != g_state)
			MGL_fatalError("Internal error: dc->v != g_state!\n");
		MGL_free(dc->v);
		g_state = NULL;
		}
}

void LINEAR32_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		LINEAR32_initInternal
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver tables.
*
****************************************************************************/
{
	/* Fill in required device context vectors */
	dc->r.getWinDC				= FULLSCREEN_getWinDC;
	dc->r.getDefaultPalette 	= SVGA_getDefaultPalette;
	_MGL_computePF(dc);
	PACKED32_initInternal(dc);
}

ibool _ASMAPI LINEAR32_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		LINEAR32_initDriver
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
	dc->mi = LINEAR32_modes[modeId - grSVGA_320x200x4G];
	if (g_state == NULL) {
		if ((g_state = MGL_calloc(1,sizeof(SVGA_state))) == NULL) {
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

	LINEAR32_initInternal(dc);

	/* Set the video mode */
	return SVGA_setMode(dc,dc->mi.scratch1,
		dc->mi.scratch2 & ~VBE_VIRTUAL_LINEAR,
		&state->oldBIOSMode,&state->old50Lines,virtualX,virtualY,numBuffers,
		stereo,refreshRate);
}

#endif

