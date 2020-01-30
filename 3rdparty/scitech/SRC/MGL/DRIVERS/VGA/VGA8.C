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
* Description:	Main VGA 256 color device driver file. This file will be
*				linked in when a call to MGL_registerDriver() is made
*				to register this device driver for use. Simply uses the
*				SuperVGA linear frame buffer code.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\vga\vga8.h"

#if	!defined(__16BIT__) || defined(TESTING)

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI VGA8_driver = {
	IDENT_FLAG,
	MGL_VGA8NAME,
	MGL_VGA8NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grVGA,
	VGA8_detect,
	VGA8_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t VGA8_modes[] = {
	{319,199,8,1,0xFF,0,320,1200,0x10000L,0x13},
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI VGA8_detect(int id,int force,int *driver,int *mode,modetab modes)
/****************************************************************************
*
* Function:		VGA8_detect
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
#ifndef	__WIN386__
	if (force >= grVGA && VGA_detect(driver)) {
		VGA_useMode(modes,grVGA_320x200x256,id,&VGA8_modes[0]);
		if (*mode == grDETECT)
			*mode = grVGA_320x200x256;
		return true;
		}
#endif
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

ibool _ASMAPI VGA8_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		VGA8_initDriver
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
	VGA8_state	*state;
	display_vec	*d;

	/* This driver does not support scrolling surfaces */
	if (virtualX != -1 || virtualY != -1)
		return false;
	if (stereo)
    	return false;
	numBuffers = numBuffers;

	/* Allocate a single buffer for the driver specific state buffer */
	dc->mi = VGA8_modes[0];
	if (g_state == NULL) {
		if ((g_state = MGL_calloc(1,sizeof(VGA8_state))) == NULL) {
			_MGL_result = grNoMem;
			return false;
			}
		}
	state = (VGA8_state*)(dc->v = g_state);
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
	d->restoreTextMode 		= VGA8_restoreTextMode;
    d->restoreGraphMode		= VGA8_restoreGraphMode;
	d->destroy				= destroyDC;
	d->makeOffscreenDC 		= NULL;
	d->makeLinearOffscreenDC = NULL;

	LINEAR8_initInternal(dc);

	/* Set the video mode */
	return VGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,
		&state->oldBIOSMode,&state->old50Lines,NULL);
}

ibool _ASMAPI VGA8_restoreGraphMode(MGLDC *dc)
{
	short t1; ibool t2;
	return VGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,&t1,&t2,NULL);
}

void _ASMAPI VGA8_restoreTextMode(MGLDC *dc)
{
	VGA8_state *state = (VGA8_state*)dc->v;
	VGA_restoreMode(state->oldBIOSMode,state->old50Lines,NULL);
}

void * _ASMAPI VGA8_getDefaultPalette(MGLDC *dc)
{ dc = dc; return _VGA8_defPal; }

#endif
