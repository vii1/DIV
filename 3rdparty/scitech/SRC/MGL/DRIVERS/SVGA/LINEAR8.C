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
* Description:	Main Linear Framebuffer SuperVGA 256 color device driver.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\svga\linear8.h"
#include "drivers\packed\packed8.h"

#if	!defined(__16BIT__) || defined(TESTING)

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI LINEAR8_driver = {
	IDENT_FLAG,
	MGL_LINEAR8NAME,
	MGL_LINEAR8NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grVESA,
	LINEAR8_detect,
	LINEAR8_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t LINEAR8_modes[] = {
	{319,199,8,1,0xFF,0,0,1200,0,0x4F02},
	{319,239,8,1,0xFF,0,0,1000,0,0x4F02},
	{319,399,8,1,0xFF,0,0,600,0,0x4F02},
	{319,479,8,1,0xFF,0,0,500,0,0x4F02},
	{399,299,8,1,0xFF,0,0,1000,0,0x4F02},
	{511,383,8,1,0xFF,0,0,1000,0,0x4F02},
	{639,349,8,1,0xFF,0,0,1371,0,0x4F02},
	{639,399,8,1,0xFF,0,0,1200,0,0x4F02},
	{639,479,8,1,0xFF,0,0,1000,0,0x4F02},
	{799,599,8,1,0xFF,0,0,1000,0,0x4F02},
	{1023,767,8,1,0xFF,0,0,1000,0,0x4F02},
	{1151,863,8,1,0xFF,0,0,1000,0,0x4F02},
	{1279,959,8,1,0xFF,0,0,1000,0,0x4F02},
	{1279,1023,8,1,0xFF,0,0,938,0,0x4F02},
	{1599,1199,8,1,0xFF,0,0,1000,0,0x4F02},
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI LINEAR8_detect(int id,int force,int *driver,int *mode,
	modetab modes)
/****************************************************************************
*
* Function:		LINEAR8_detect
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
		SVGALIN_useMode(modes,grSVGA_320x200x256,id,&LINEAR8_modes[0],true);
		SVGALIN_useMode(modes,grSVGA_320x240x256,id,&LINEAR8_modes[1],true);
		SVGALIN_useMode(modes,grSVGA_320x400x256,id,&LINEAR8_modes[2],true);
		SVGALIN_useMode(modes,grSVGA_320x480x256,id,&LINEAR8_modes[3],true);
		SVGALIN_useMode(modes,grSVGA_400x300x256,id,&LINEAR8_modes[4],true);
		SVGALIN_useMode(modes,grSVGA_512x384x256,id,&LINEAR8_modes[5],true);
		SVGALIN_useMode(modes,grSVGA_640x350x256,id,&LINEAR8_modes[6],true);
		SVGALIN_useMode(modes,grSVGA_640x400x256,id,&LINEAR8_modes[7],true);
		SVGALIN_useMode(modes,grSVGA_640x480x256,id,&LINEAR8_modes[8],true);
		SVGALIN_useMode(modes,grSVGA_800x600x256,id,&LINEAR8_modes[9],true);
		SVGALIN_useMode(modes,grSVGA_1024x768x256,id,&LINEAR8_modes[10],true);
		SVGALIN_useMode(modes,grSVGA_1152x864x256,id,&LINEAR8_modes[11],true);
		SVGALIN_useMode(modes,grSVGA_1280x960x256,id,&LINEAR8_modes[12],true);
		SVGALIN_useMode(modes,grSVGA_1280x1024x256,id,&LINEAR8_modes[13],true);
		SVGALIN_useMode(modes,grSVGA_1600x1200x256,id,&LINEAR8_modes[14],true);
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

PUBLIC void LINEAR8_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		LINEAR8_initInternal
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver tables.
*
****************************************************************************/
{
	display_vec	*d = &dc->v->d;

	/* Fill in required device context vectors */
	d->maxProgram				= 256;
	dc->r.getWinDC				= FULLSCREEN_getWinDC;
	dc->r.getDefaultPalette 	= SVGA_getDefaultPalette;
	_MGL_computePF(dc);
	PACKED8_initInternal(dc);
}

ibool _ASMAPI LINEAR8_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		LINEAR8_initDriver
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
	dc->mi = LINEAR8_modes[modeId - grSVGA_320x200x256];
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

	LINEAR8_initInternal(dc);

	/* Set the video mode */
	return SVGA_setMode(dc,dc->mi.scratch1,
		dc->mi.scratch2 & ~VBE_VIRTUAL_LINEAR,
		&state->oldBIOSMode,&state->old50Lines,virtualX,virtualY,numBuffers,
		stereo,refreshRate);
}

#endif
