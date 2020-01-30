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
* Description:	Main 16 bit accelerated device driver file. This file
*				will be linked in when a call to MGL_registerDriver() is
*				made to register this device driver for use. This driver
*				requires linear framebuffer access for non-accelerated
*				code, so will only work with if a virtual or real linear
*				framebuffer is present.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\accel\accel16.h"

#if	!defined(__16BIT__) || defined(TESTING)

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI ACCEL16_driver = {
	IDENT_FLAG,
	MGL_ACCEL16NAME,
	MGL_ACCEL16NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grSVGA,
	ACCEL16_detect,
	ACCEL16_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t ACCEL15_modes[] = {
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

PRIVATE gmode_t ACCEL16_modes[] = {
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

ibool _ASMAPI ACCEL16_detect(int id,int force,int *driver,int *mode,
	modetab modes)
/****************************************************************************
*
* Function:		ACCEL16_detect
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
	if (force >= grSVGA && ACCEL_detect(driver,&ACCEL16_driver.driverId)) {
		/* Fill in information here for the available video modes by
		 * querying the ACCEL.DRV for the information.
		 */
		ACCEL_useMode(modes,grSVGA_320x200x32k,id,&ACCEL15_modes[0]);
		ACCEL_useMode(modes,grSVGA_320x240x32k,id,&ACCEL15_modes[1]);
		ACCEL_useMode(modes,grSVGA_320x400x32k,id,&ACCEL15_modes[2]);
		ACCEL_useMode(modes,grSVGA_320x480x32k,id,&ACCEL15_modes[3]);
		ACCEL_useMode(modes,grSVGA_400x300x32k,id,&ACCEL15_modes[4]);
		ACCEL_useMode(modes,grSVGA_512x384x32k,id,&ACCEL15_modes[5]);
		ACCEL_useMode(modes,grSVGA_640x350x32k,id,&ACCEL15_modes[6]);
		ACCEL_useMode(modes,grSVGA_640x400x32k,id,&ACCEL15_modes[7]);
		ACCEL_useMode(modes,grSVGA_640x480x32k,id,&ACCEL15_modes[8]);
		ACCEL_useMode(modes,grSVGA_800x600x32k,id,&ACCEL15_modes[9]);
		ACCEL_useMode(modes,grSVGA_1024x768x32k,id,&ACCEL15_modes[10]);
		ACCEL_useMode(modes,grSVGA_1152x864x32k,id,&ACCEL15_modes[11]);
		ACCEL_useMode(modes,grSVGA_1280x960x32k,id,&ACCEL15_modes[12]);
		ACCEL_useMode(modes,grSVGA_1280x1024x32k,id,&ACCEL15_modes[13]);
		ACCEL_useMode(modes,grSVGA_1600x1200x32k,id,&ACCEL15_modes[14]);

		ACCEL_useMode(modes,grSVGA_320x200x64k,id,&ACCEL16_modes[0]);
		ACCEL_useMode(modes,grSVGA_320x240x64k,id,&ACCEL16_modes[1]);
		ACCEL_useMode(modes,grSVGA_320x400x64k,id,&ACCEL16_modes[2]);
		ACCEL_useMode(modes,grSVGA_320x480x64k,id,&ACCEL16_modes[3]);
		ACCEL_useMode(modes,grSVGA_400x300x64k,id,&ACCEL16_modes[4]);
		ACCEL_useMode(modes,grSVGA_512x384x64k,id,&ACCEL16_modes[5]);
		ACCEL_useMode(modes,grSVGA_640x350x64k,id,&ACCEL16_modes[6]);
		ACCEL_useMode(modes,grSVGA_640x400x64k,id,&ACCEL16_modes[7]);
		ACCEL_useMode(modes,grSVGA_640x480x64k,id,&ACCEL16_modes[8]);
		ACCEL_useMode(modes,grSVGA_800x600x64k,id,&ACCEL16_modes[9]);
		ACCEL_useMode(modes,grSVGA_1024x768x64k,id,&ACCEL16_modes[10]);
		ACCEL_useMode(modes,grSVGA_1152x864x64k,id,&ACCEL16_modes[11]);
		ACCEL_useMode(modes,grSVGA_1280x960x64k,id,&ACCEL16_modes[12]);
		ACCEL_useMode(modes,grSVGA_1280x1024x64k,id,&ACCEL16_modes[13]);
		ACCEL_useMode(modes,grSVGA_1600x1200x64k,id,&ACCEL16_modes[14]);
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

PUBLIC void ACCEL16_initInternal(MGLDC *dc)
/****************************************************************************
*
* Function:		ACCEL16_initInternal
* Parameters:	dc	- Device context.
* Returns:		True if the device was correctly initialised.
*
* Description:	Initialises the device driver internal tables for 8 bit
*				unaccelerated driver routines.
*
****************************************************************************/
{
	/* Fill in required device context vectors */
	dc->r.getWinDC				= FULLSCREEN_getWinDC;
	dc->r.getDefaultPalette 	= SVGA_getDefaultPalette;
	_MGL_computePF(dc);
	PACKED16_initInternal(dc);
}

ibool _ASMAPI ACCEL16_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		ACCEL16_initDriver
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
	ACCEL_state	*state;
	display_vec	*d;

	/* Allocate a single buffer for the driver specific state buffer */
	if (modeId >= grSVGA_320x200x64k)
		dc->mi = ACCEL16_modes[modeId - grSVGA_320x200x64k];
	else
		dc->mi = ACCEL15_modes[modeId - grSVGA_320x200x32k];
	if (g_state == NULL) {
		if ((g_state = MGL_calloc(1,sizeof(ACCEL_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	state = (ACCEL_state*)(dc->v = g_state);
	d = &dc->v->d;
	d->refCount++;
	d->hwnd = hwnd;
	d->graphDriver = driverId;
	d->graphMode = modeId;

	/* Load the device context with the device information for the
	 * selected video mode. By default we set up the device to use the
	 * software rendering routines, and the ACCEL_setMode() function
	 * will re-vector the appropriate routines to the hardware accelerated
	 * versions depending on what the hardware supports.
	 */
	dc->deviceType 			= MGL_DISPLAY_DEVICE;
	dc->xInch 				= 9000;
	dc->yInch 				= 7000;
	d->hardwareCursor 		= FALSE;
	d->restoreTextMode 		= ACCEL_restoreTextMode;
	d->restoreGraphMode		= ACCEL_restoreGraphMode;
	d->destroy				= destroyDC;
	d->makeOffscreenDC 		= NULL;
	d->makeLinearOffscreenDC = NULL;

	ACCEL16_initInternal(dc);

	/* Set the video mode */
	return ACCEL_setMode(dc,dc->mi.scratch1,&state->oldBIOSMode,
		&state->old50Lines,virtualX,virtualY,numBuffers,stereo,refreshRate);
}

#endif
