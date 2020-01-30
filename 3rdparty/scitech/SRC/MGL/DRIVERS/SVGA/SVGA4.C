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
* Description:	Main SVGA4 device driver file. This file will be linked in
*				when a call to MGL_registerDriver() is made to register
*				this device driver for use. This provides support for
*				the SuperVGA style versions of the standard VGA modes
*				along with support for 800x600x16.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop
#include "drivers\svga\svga4.h"

/*---------------------------- Driver Tables -----------------------------*/

drivertype	_VARAPI SVGA4_driver = {
	IDENT_FLAG,
	MGL_SVGA4NAME,
	MGL_SVGA4NAME,
	"\r\n\r\nVersion " DRIVER_VERSION_STR " - " __DATE__ "\r\n"
	"Copyright (C) 1996 SciTech Software\r\n",
	DRIVER_MAJOR_VERSION,
	DRIVER_MINOR_VERSION,
    grVESA,
	SVGA4_detect,
	SVGA4_initDriver,
	};

/* List of mode specific device context information */

PRIVATE gmode_t SVGA4_modes[] = {
	{639,349,4,4,15,0,80,1371,0x8000,0x4F02},
	{639,399,4,4,15,0,80,1200,0x8000,0x4F02},
	{639,479,4,4,15,0,80,1000,0x10000L,0x4F02},
	{799,599,4,4,15,0,100,1000,0x10000L,0x4F02},
	};

PRIVATE drv_vec	*g_state = NULL;	/* Global state buffer pointer	*/

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI SVGA4_detect(int id,int force,int *driver,int *mode,modetab modes)
/****************************************************************************
*
* Function:		SVGA4_detect
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
		SVGA_useMode(modes,grVGA_640x350x16,id,&SVGA4_modes[0]);
		SVGA_useMode(modes,grVGA_640x400x16,id,&SVGA4_modes[1]);
		SVGA_useMode(modes,grVGA_640x480x16,id,&SVGA4_modes[2]);
		SVGA_useMode(modes,grSVGA_800x600x16,id,&SVGA4_modes[3]);
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

ibool _ASMAPI SVGA4_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,
	int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		SVGA4_initDriver
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

	/* Allocate a single buffer for the driver specific state buffer */
	dc->mi = SVGA4_modes[modeId - grVGA_640x350x16];
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
	d->restoreTextMode 		= SVGA_restoreTextMode;
	d->restoreGraphMode		= SVGA4_restoreGraphMode;

	/* Set the video mode */
	if (!SVGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,&state->oldBIOSMode,
			&state->old43Lines,virtualX,virtualY,numBuffers,stereo,refreshRate))
		return false;

	/* Do extra mode initilisation */
	VGA4_initMode(0);
	return true;
}

ibool _ASMAPI SVGA4_restoreGraphMode(MGLDC *dc)
{
	short t1; ibool t2;
	if (!VGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,&t1,&t2,SVGA_saveVideoMemory))
		return false;
	VGA4_initMode(0);
	return true;
}

