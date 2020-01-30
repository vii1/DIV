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
* Description:	Main platform independant interface and setup code.
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/
#ifdef	__WINDOWS32__
#define	STRICT
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/*--------------------------- Global Variables ----------------------------*/

MGLDC	_VARAPI _MGL_dc;				/* Global device context 				*/
MGLDC	* _VARAPI _MGL_dcPtr = NULL;	/* Pointer to currently active context	*/
int		_VARAPI _MGL_bufSize = DEF_MGL_BUFSIZE;
void	* _VARAPI _MGL_buf;				/* Internal MGL scratch buffer			*/
int		_VARAPI _MGL_result;			/* Result of last operation				*/
char	_VARAPI _MGL_path[_MAX_PATH];	/* Root directory for all MGL files		*/
uchar   _VARAPI _MGL_modeTable[grMAXMODE] = {0xFF};
modetab	_VARAPI _MGL_availableModes;
int		_VARAPI _MGL_stereoSyncType = MGL_STEREO_BLUE_CODE;
int		_VARAPI _MGL_blueCodeIndex = 0xFF;
AF_devCtx * _VARAPI _MGL_wdDriver = NULL;
int     _VARAPI _MGL_surfLock = -1;
FILE *	(*_MGL_fopen)(const char *filename,const char *mode)	= fopen;
int 	(*_MGL_fclose)(FILE *f)									= fclose;
int 	(*_MGL_fseek)(FILE *f,long offset,int whence)			= fseek;
long 	(*_MGL_ftell)(FILE *f)									= ftell;
size_t	(*_MGL_fread)(void *ptr,size_t size,size_t n,FILE *f)	= fread;
size_t	(*_MGL_fwrite)(const void *ptr,size_t size,size_t n,FILE *f) = fwrite;
vecs	_PACKED_vecs;					/* Copy of packed pixel vectors	*/

/* Pre-defined pixel formats table */

pixel_format_t _VARAPI _MGL_pixelFormats[] = {
	{0x1F,0x1F,0x1F,0x01,0x0A,3,0x5,3,0x0,3,0x0F,7},	/* pfRGB555		*/
	{0x1F,0x3F,0x1F,0x00,0x0B,3,0x5,2,0x0,3,0x00,0},	/* pfRGB565		*/
	{0xFF,0xFF,0xFF,0x00,0x10,0,0x8,0,0x0,0,0x00,0},	/* pfRGB24		*/
	{0xFF,0xFF,0xFF,0x00,0x0,0,0x8,0,0x10,0,0x00,0},	/* pfBGR24		*/
	{0xFF,0xFF,0xFF,0xFF,0x10,0,0x8,0,0x0,0,0x18,0},	/* pfARGB32		*/
	{0xFF,0xFF,0xFF,0xFF,0x0,0,0x8,0,0x10,0,0x18,0},	/* pfABGR32		*/
	{0xFF,0xFF,0xFF,0xFF,0x18,0,0x10,0,0x8,0,0x0,0},	/* pfRGBA32		*/
	{0xFF,0xFF,0xFF,0xFF,0x8,0,0x10,0,0x18,0,0x0,0},	/* pfBGRA32		*/
	};

/* Graphics device driver table */

driverent	_VARAPI _MGL_driverTable[] = {
	{MGL_PACKED1NAME,false,NULL},		/* Memory device drivers first	*/
	{MGL_PACKED4NAME,false,NULL},
	{MGL_PACKED8NAME,false,NULL},
	{MGL_PACKED16NAME,false,NULL},
	{MGL_PACKED24NAME,false,NULL},
	{MGL_PACKED32NAME,false,NULL},
	{MGL_VGA4NAME,false,NULL},
	{MGL_VGAXNAME,false,NULL},
	{MGL_VGA8NAME,false,NULL},
	{MGL_SVGA4NAME,false,NULL},
	{MGL_SVGA8NAME,false,NULL},
	{MGL_SVGA16NAME,false,NULL},
	{MGL_SVGA24NAME,false,NULL},
	{MGL_SVGA32NAME,false,NULL},
	{MGL_FULLDIB8NAME,false,NULL},
	{MGL_FULLDIB16NAME,false,NULL},
	{MGL_FULLDIB24NAME,false,NULL},
	{MGL_FULLDIB32NAME,false,NULL},
	{MGL_LINEAR8NAME,false,NULL},
	{MGL_LINEAR16NAME,false,NULL},
	{MGL_LINEAR24NAME,false,NULL},
	{MGL_LINEAR32NAME,false,NULL},
	{MGL_ACCEL8NAME,false,NULL},
	{MGL_ACCEL16NAME,false,NULL},
	{MGL_ACCEL24NAME,false,NULL},
	{MGL_ACCEL32NAME,false,NULL},
	{MGL_DDRAW8NAME,false,NULL},
	{MGL_DDRAW16NAME,false,NULL},
	{MGL_DDRAW24NAME,false,NULL},
	{MGL_DDRAW32NAME,false,NULL},
	{MGL_OPENGLNAME,false,NULL},
	{MGL_FSOGL8NAME,false,NULL},
	{MGL_FSOGL16NAME,false,NULL},
	{MGL_FSOGL24NAME,false,NULL},
	{MGL_FSOGL32NAME,false,NULL},
	};

#define	NUMDRIVERS	(sizeof(_MGL_driverTable)/sizeof(driverent))

/*------------------------- Implementation --------------------------------*/

/* {secret} */
void _MGL_scratchTooSmall(void)
{ MGL_fatalError("Scratch buffer too small for rendering operation!\n"); }

/****************************************************************************
DESCRIPTION:
Returns result code of the last graphics operation.

HEADER:
mgraph.h

RETURNS:
Result code of the last graphics operation

REMARKS:
This function returns the result code of the last graphics operation. The internal
result code is reset back to grOK on return from this routine, so you should only call
the routine once after the graphics operation. Error codes returned by this function
are enumerated in MGL_errorType.


SEE ALSO:
MGL_setResult
****************************************************************************/
int MGLAPI MGL_result(void)
{
	int	result;

	result = _MGL_result;
	_MGL_result = grOK;
	return result;
}

/****************************************************************************
DESCRIPTION:
Sets the internal MGL result flag.

HEADER:
mgraph.h

PARAMETERS:
result	- New internal result flag

REMARKS:
Sets the internal MGL result flag to the specified value. This routine is primarily for
extension libraries, but you can use it to add your own extension functions to MGL
that will return result codes in the same manner as MGL.

SEE ALSO:
MGL_result, MGL_errorType
****************************************************************************/
void MGLAPI MGL_setResult(
	int result)
{ _MGL_result = result; }

/****************************************************************************
DESCRIPTION:
Returns a string describing an error condition code.

HEADER:
mgraph.h

PARAMETERS:
err	- Error code to obtain string for

RETURNS:
Pointer to string describing the error condition.

REMARKS:
Returns a pointer to a string describing a specified error code. You can use this to
convert the error codes from a numerical id return by MGL_result to a string which
you can display for the users of your programs.

SEE ALSO:
MGL_result.

****************************************************************************/
const char * MGLAPI MGL_errorMsg(
	int err)
{
	static char	*msgs[] = {
		"No error",
		"Graphics driver has not been installed",
		"Graphics hardware not detected",
		"Couldn't find graphics driver",
		"Graphics driver file corrupted",
		"Not enough memory to load graphics driver",
		"Invalid mode for selected driver",
		"Invalid driver number",
		"General graphics error",
		"Invalid graphics driver name",
		"Not enough memory to perform operation",
		"Selected graphics mode not supported",
		"Invalid font data",
		"Font file corrupted",
		"Couldn't find font file",
		"Incorrect version for driver file",
		"Invalid device type for operation",
		"Invalid device context",
		"Invalid cursor file",
		"Cursor file was not found",
		"Invalid icon file",
		"Icon file was not found",
		"Invalid bitmap file",
		"Bitmap file was not found",
		"Invalid error number",
		"Zbuffer allocation is too large",
		"Only Windows 2.x font files supported",
		"MGL_endRegion() was not called",
		"Double buffering is not available",
		"OffscreenDC's require hardware bitBlt support",
		"No available Offscreen display memory",
		"Invalid pixel format for memory DC",
		};

	if (err > 0 || err < grLastError)
		err = grLastError-1;
	return msgs[-err];
}

/****************************************************************************
DESCRIPTION:
Make a device context the current device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- New device context to make the current context

RETURNS:
Previous current device context (possibly NULL).

REMARKS:
This function makes the specified device context the current device context. The
current device context is the one that is used for all the output rasterizing routines,
and when a device context is made current, a copy of the device context is cached
internally for maximum speed in the rasterizing code. While a device context is
current, changes made to the global state of the current device context are changed
in the cached copy only and are not updated in the original device context. When
the device context is changed however, the values in the cached device context are
flushed back to the original device context to keep it up to date. You can flush the
current device context explicitly by passing a NULL for the new current device
context.

Because of this caching mechanism, changing the current device context is an
expensive operation, so you should try to minimize the need to change the current
device context. Normally you will maintain a single device context that will be used
for all rasterizing operations, and leave this as your current device context. If the
device context specified is already the current device context, this function simply
does nothing.

SEE ALSO:
MGL_isCurrentDC
****************************************************************************/
MGLDC * MGLAPI MGL_makeCurrentDC(
	MGLDC *dc)
{
	MGLDC	*oldDC = _MGL_dcPtr;

	if (dc == _MGL_dcPtr || dc == &DC)
		return oldDC;
	if (_MGL_dcPtr) {
		*_MGL_dcPtr = _MGL_dc;			/* 'Write back' the old DC		*/
		_MGL_dcPtr = NULL;				/* This DC is no longer cached	*/
		}
	if (dc) {
		_MGL_dcPtr = dc;				/* Cache DC in global structure	*/
		_MGL_dc = *dc;					/* Save pointer to original DC	*/

		/* Now update those attributes that can be cached by the
		 * hardware accelerated drivers
		 */
#ifndef	MGL_LITE
		MGL_setWriteMode(DC.a.writeMode);
		MGL_setPenStyle(DC.a.penStyle);
		MGL_setPenBitmapPattern(&DC.a.penPat);
		MGL_setPenPixmapPattern(&DC.a.penPixPat);
#endif
		MGL_setClipRect(DC.a.clipRect);
		}
	return oldDC;
}

/****************************************************************************
DESCRIPTION:
Determines if the specified device context is the currently active context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to check

RETURNS:
True if DC is the currently active context, false otherwise.

REMARKS:
This function determines if the passed in device context is the currently
active device context. The currently active device context is where all
the output from all the MGL rendering functions will be displayed.

SEE ALSO:
MGL_makeCurrentDC
****************************************************************************/
ibool MGLAPI MGL_isCurrentDC(
	MGLDC *dc)
{
	return (dc == _MGL_dcPtr);
}

/* {secret} */
void _MGL_updateCurrentDC(MGLDC *dc)
/****************************************************************************
*
* Function:		_MGL_updateCurrentDC
* Parameters:	dc	- DC that has been updated
*
* Description:	This is an internal routine that gets called when major
*				changes are made to a DC through a pointer, not the current
*				DC. If the DC being modified is the current one, we flush
*				all changes to the newly updated DC to the current DC.
*
****************************************************************************/
{
	if (dc == _MGL_dcPtr)
		_MGL_dc = *_MGL_dcPtr;		/* Update the cached copy			*/
}

/****************************************************************************
DESCRIPTION:
Registers a device driver to be linked in.

HEADER:
mgraph.h

PARAMETERS:
name	- Name of the driver to register
driver	- Pointer to the start of the driver in memory

RETURNS:
grOK on success, grBadDriver if driver was invalid.

REMARKS:
This function registers a specific device driver to be linked into the
application code, and to be used in the automatic device detection process.
This function is used to link in both display device drivers and packed pixel
memory device drivers. If you do not have the proper device driver loaded you
will not be able to create a device context that requires that driver. For
instance if you do not register the 8 bit packed pixel device, and attempt to
create an 8 bit memory device context, the function will fail.

The names of the standard fullscreen display device drivers currently supported
are:

Driver				- Description
MGL_VGA4NAME		- Standard VGA 4 bit display driver
MGL_VGA8NAME		- Standard VGA 8 bit display driver (32 bit only)
MGL_VGAXNAME		- Standard VGA ModeX 8 bit display driver
MGL_SVGA4NAME		- VBE 1.x 4 bit display driver
MGL_SVGA8NAME		- VBE 1.x 8 bit display driver
MGL_SVGA16NAME		- VBE 1.x 15/16 bit display driver
MGL_SVGA24NAME		- VBE 1.x 24 bit display driver
MGL_SVGA32NAME		- VBE 1.x 32 bit display driver
MGL_LINEAR8NAME		- VBE 2.0 8 bit linear framebuffer display driver
MGL_LINEAR16NAME	- VBE 2.0 15/16 bit linear framebuffer display driver
MGL_LINEAR24NAME	- VBE 2.0 24 bit linear framebuffer display driver
MGL_LINEAR32NAME	- VBE 2.0 32 bit linear framebuffer display driver
MGL_ACCEL8NAME		- VBE/AF 8 bit accelerated display driver
MGL_ACCEL16NAME		- VBE/AF 15/16 bit accelerated display driver
MGL_ACCEL24NAME		- VBE/AF 24 bit accelerated display driver
MGL_ACCEL32NAME		- VBE/AF 32 bit accelerated display driver
MGL_DDRAW8NAME		- DirectDraw 8 bit accelerated display driver
MGL_DDRAW16NAME		- DirectDraw 15/16 bit accelerated display driver
MGL_DDRAW24NAME		- DirectDraw 24 bit accelerated display driver
MGL_DDRAW32NAME		- DirectDraw 32 bit accelerated display driver
MGL_FULLDIB8NAME	- Fullscreen DIB 8 bit display driver
MGL_FULLDIB16NAME	- Fullscreen DIB 15/16 bit display driver
MGL_FULLDIB24NAME	- Fullscreen DIB 24 bit display driver
MGL_FULLDIB32NAME	- Fullscreen DIB 32 bit display driver
MGL_OPENGLNAME		- Hardware Microsoft OpenGL display driver
MGL_FSOGL8NAME		- Hardware 8 bit fullscreen OpenGL display driver
MGL_FSOGL16NAME		- Hardware 15/16 bit fullscreen OpenGL display driver
MGL_FSOGL24NAME		- Hardware 24 bit fullscreen OpenGL display driver
MGL_FSOGL32NAME		- Hardware 32 bit fullscreen OpenGL display driver

The names of the standard packed pixel memory device drivers currently
supported are:

Driver				- Description
MGL_PACKED1NAME		- Packed pixel 1 bit memory driver
MGL_PACKED4NAME		- Packed pixel 4 bit memory driver
MGL_PACKED8NAME		- Packed pixel 8 bit memory driver
MGL_PACKED16NAME	- Packed pixel 15/16 bit memory driver
MGL_PACKED24NAME	- Packed pixel 24 bit memory driver
MGL_PACKED32NAME	- Packed pixel 32 bit memory driver

You must also pass the address of the driver, which is the type and color depth
of the driver but with '_driver' added to the end. For instance to register
just the 8 bit VBE SuperVGA driver, you would use the following call:

	MGL_registerDriver(MGL_SVGA8NAME, SVGA8_driver);

SEE ALSO:
MGL_init, MGL_detectGraph, MGL_unregisterAllDrivers
****************************************************************************/
int MGLAPI MGL_registerDriver(
	const char *name,
	void *driver)
{
	int			i;

	_MGL_result = grInvalidName;				/* Assume name is invalid	*/
	for (i = 0; i < NUMDRIVERS; i++)
		if (!strcmp(_MGL_driverTable[i].name,name)) {
			_MGL_result = grOK;
			if (!strcmp(((drivertype *)driver)->ident,IDENT_FLAG) &&
				!strcmp(((drivertype *)driver)->name,_MGL_driverTable[i].name)) {
				if (((drivertype*)driver)->majorversion != DRIVER_MAJOR_VERSION
					|| ((drivertype*)driver)->minorversion != DRIVER_MINOR_VERSION) {
					return _MGL_result = grOldDriver;
					}
				_MGL_driverTable[i].registered = true;
				_MGL_driverTable[i].driver = (drivertype*)driver;
				break;
				}
			else {
				_MGL_result = grBadDriver;
				break;
				}
			}
	return _MGL_result;
}

/****************************************************************************
DESCRIPTION:
Unregisters all currently registered display and memory drivers.

HEADER:
mgraph.h

REMARKS:
This function unregisters all currently registered display drivers and allows you to
dynamically change the set of registered drivers at runtime. This function is most
useful for allowing the user to select at runtime whether to use WinDirect VBE 2.0
and VBE/AF drivers or to use the Microsoft DirectDraw drivers. To switch between
WinDirect and DirectDraw you can use this function to unregister all the drivers,
then re-register only the DirectDraw drivers or only the WinDirect drivers.

Note that this function also unregisters all the registered memory device drivers, so
you will need to re-register the packed pixel memory drivers that you require when
you re-register the display drivers.
****************************************************************************/
void MGLAPI MGL_unregisterAllDrivers(void)
{
	int			i;

	for (i = 0; i < NUMDRIVERS; i++) {
		_MGL_driverTable[i].registered = false;
		_MGL_driverTable[i].driver = NULL;
		}
}

/****************************************************************************
DESCRIPTION:
Detects the currently installed video hardware.

HEADER:
mgraph.h

PARAMETERS:
driver	- Pointer to graphics device driver id
mode	- Pointer to default graphics mode for device

REMARKS:
This function autodetects the presence of all the standard graphics adapters
supported by MGL. If no suitable hardware is detected, it returns grNONE in the
graphdriver parameter. If suitable hardware is detected, it will return an appropriate
device driver id number in the driver parameter and a suggested default graphics mode
in the mode parameter.

The fullscreen device drivers currently supported by MGL are enumerated in
MGL_driverType.

If you pass a value of grDETECT to the this routine, it will attempt to find the
highest performance driver that has currently been registered by the
MGL_registerDriver function. However you can also pass explicit values in the
driver parameter, and this will automatically set the desired maximum performance
driver. For instance you may pass the value of grVGA for the driver id, in which
case the detection code will ignore all the VESA and SuperVGA device drivers, so
the only device driver that will be used will be the standard VGA ones. This can be
used to allow the user to disable certain drivers (like accelerated drivers) to get
around problems in the field relating to incompatible video hardware.

If you have registered all available drivers, the priority ordering of which will be
used depends on the capabilities of the underlying display hardware, and the drivers
will be chosen in the following order (first one in table is selected in
preference to ones below it):

Driver			- Highest performance driver selected
VBEAF			- ACCEL8/16/24/32
VBE2			- LINEAR8/16/24/32
DirectDraw		- DDRAW8/16/24/32
VBE1			- SVGA4/8/16/24/32
StandardVGA		- VGA4, VGAX, VGA8

For instance if you had DirectDraw installed on your system and MGL found either
a VBE 2.0 or VBE/AF driver, those modes supported by the VBE drivers will be
used in preference to the DirectDraw modes. If however the DirectDraw driver has
additional modes not supported by the VBE 2.0 or VBE/AF (for instance
320x240x256), the DirectDraw drivers would be used for those modes. If however
you had DirectDraw installed and only had a VBE 1.2 driver available, the
DirectDraw drivers will be used for all modes whenever possible.

To change this default behavior you can selectively register only those drivers you
wish to use before calling this function or MGL_init. A typical sequence of code to
register drivers and allows the program to force WinDirect or DirectDraw using the
settings of useDirectDraw or useWinDirect might be as follows:

	MGL_unregisterAllDrivers();
	if (useWinDirect) {
	  MGL_registerDriver(MGL_VGA4NAME,VGA4_driver);
	  MGL_registerDriver(MGL_VGAXNAME,VGAX_driver);
	  MGL_registerDriver(MGL_SVGA4NAME,SVGA4_driver);
	  MGL_registerDriver(MGL_SVGA8NAME,SVGA8_driver);
	  MGL_registerDriver(MGL_SVGA16NAME,SVGA16_driver);
	  MGL_registerDriver(MGL_SVGA24NAME,SVGA24_driver);
	  MGL_registerDriver(MGL_SVGA32NAME,SVGA32_driver);
	  MGL_registerDriver(MGL_VGA8NAME,VGA8_driver);
	  MGL_registerDriver(MGL_LINEAR8NAME,LINEAR8_driver);
	  MGL_registerDriver(MGL_LINEAR16NAME,LINEAR16_driver);
	  MGL_registerDriver(MGL_LINEAR24NAME,LINEAR24_driver);
	  MGL_registerDriver(MGL_LINEAR32NAME,LINEAR32_driver);
	  MGL_registerDriver(MGL_ACCEL8NAME,ACCEL8_driver);
	  MGL_registerDriver(MGL_ACCEL16NAME,ACCEL16_driver);
	  MGL_registerDriver(MGL_ACCEL24NAME,ACCEL24_driver);
	  MGL_registerDriver(MGL_ACCEL32NAME,ACCEL32_driver);
	  }
	if (useDirectDraw) {
	  MGL_registerDriver(MGL_DDRAW8NAME,DDRAW8_driver);
	  MGL_registerDriver(MGL_DDRAW16NAME,DDRAW16_driver);
	  MGL_registerDriver(MGL_DDRAW24NAME,DDRAW24_driver);
	  MGL_registerDriver(MGL_DDRAW32NAME,DDRAW32_driver);
	  }
	driver = grDETECT;
	mode = grDETECT;
	MGL_detectGraph(&driver,&mode);
	...

MGL supports a number of different video mode resolutions, ranging from 320x200
up to 1600x1200 with color ranges from 16 colors up to 16.7 million colors. You
can construct the mode identifiers easily for every desired resolution, given the
name of the mode and the resolution. For instance the 320x200x256 mode is named
grVGA_320x200x256 while the 1280x1024x256 mode is named
grSVGA_1280x1024x256. Please consult the MGL header files for a complete list
of all the available video modes.

Once you have called MGL_detectGraph, you can then call the
MGL_availableModes and MGL_availablePages to determine all the available
graphics modes for the currently installed driver and the number of available video
pages for each graphics mode. You can also call the MGL_modeResolution function to
get numeric resolution information about all the available graphics modes, so you can
search for specific graphics modes according to the desired resolution and/or color
depth.

SEE ALSO:
MGL_init, MGL_availableModes, MGL_availablePages, MGL_modeResolution
****************************************************************************/
void MGLAPI MGL_detectGraph(
	int *driver,
	int *mode)
{
	int	i,j,force,defmode = grDETECT,found = FALSE;

	/* Initialise the mode table to empty */

	for (i = j = 0; i < grMAXMODE; i++) {
		_MGL_availableModes[i].driver = 0xFF;
		_MGL_availableModes[i].pages = 0;
        _MGL_availableModes[i].flags = 0;
		}
	_MGL_modeTable[0] = 0xFF;

	force = (*driver == grDETECT) ? 0xFF : *driver;
	for (i = 0; i < NUMDRIVERS; i++) {
		if (_MGL_driverTable[i].driver && _MGL_driverTable[i].driver->detect)
			if (_MGL_driverTable[i].driver->detect(i,force,driver,&defmode,_MGL_availableModes))
				found = TRUE;
		}
	if (found) {
		/* A valid video device was found, so now build up the list of
		 * available graphics modes from the table filled in by the detection
		 * routines.
		 */
		for (i = j = 0; i < grMAXMODE; i++) {
			if (_MGL_availableModes[i].driver != 0xFF)
				_MGL_modeTable[j++] = i;
			}
		_MGL_modeTable[j] = 0xFF;
		if (*mode == grDETECT)
			*mode = defmode;
		}
	else *driver = grNONE;
}

/* {secret} */
ibool _MGL_initDC(MGLDC *dc,drivertype *driver,MGL_HWND hwnd,int virtualX,
	int virtualY,int numBuffers,ibool stereo,int refreshRate)
/****************************************************************************
*
* Function:		_MGL_initDC
* Parameters:	dc			- Device context to fill in
*				driver		- Pointer to device driver to use
*				hwnd		- Handle to OS fullscreen window
*				virtualX	- Virtual X coordinate for display DC
*				virutalY	- Virtual Y coordinate for display DC
*				numBufffers	- Number of display buffers requested
* Returns:		True on success, false on failure.
*
* Description:  Attempts to initialize the low level device driver. If this
*				is a display device driver, it will start the specified
*				graphics mode. If this is a memory device driver, it will
*				allocate the memory for the display surface. First we
*				allocate all resources for the device driver, and then
*				intialise it to fill in the device context vector tables.
*
****************************************************************************/
{
	/* Check to ensure that the driver was actually found. If the driver
	 * was not registered, then we need to catch this.
     */
	_MGL_result = grOK;
	if (!driver) {
		_MGL_result = grDriverNotFound;
		return false;
		}

	/* Clear out some important variables */
	dc->zbuffer = NULL;
	dc->flags = 0;

	/* Initialise the driver and set the graphics mode */

	_MGL_setCommonVectors(dc);
	if (driver->initDriver(dc,_MGL_driverId,_MGL_modeId,(ulong)hwnd,virtualX,virtualY,
			numBuffers,stereo,refreshRate)) {
		/* Initialise internal dimensions */
		dc->size.left = 0;
		dc->size.top = 0;
		dc->size.right = dc->mi.xRes+1;
		dc->size.bottom = dc->mi.yRes+1;

		/* Allocate color lookup table cache - make sure it is cleared */
		dc->colorTab = MGL_calloc(1,sizeof(color_t) * 256);

		if (dc->deviceType != MGL_MEMORY_DEVICE) {
			MGL_setDefaultPalette(dc);
			MGL_realizePalette(dc,MGL_getPaletteSize(dc),0,false);
			}
		MGL_defaultAttributes(dc);

		/* Set default rendering vectors */
		dc->r.cur = dc->r.solid;
		}
	else
		_MGL_result = (_MGL_isFullscreenDevice(dc) ? grNoModeSupport : grNoMem);

	return _MGL_result == grOK;
}

/****************************************************************************
REMARKS:
Does the hard work to create the actual display device context.

SEE ALSO:
MGL_createMemoryDC, MGL_createDisplayDC, MGL_destroyDC,
MGL_setWinDC, MGL_activatePalette, MGL_initWindowed, MGL_changeDisplayMode
****************************************************************************/
static MGLDC * _MGL_createDisplayDC(
	int virtualX,
	int virtualY,
	int numBuffers,
    ibool stereo,
	int refreshRate)
{
	MGLDC		*dc;
	drivertype	*driver;

	if ((dc = _LST_newNode(sizeof(MGLDC))) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}

	if (_MGL_dispDCList == NULL) {
		/* The display device list is currently empty, so create the new
		 * list and start the specified graphics mode
		 */
		if ((_MGL_dispDCList = _LST_create()) == NULL) {
			_MGL_result = grNoMem;
			goto Error;
			}
		driver = _MGL_driverTable[_MGL_availableModes[_MGL_modeId].driver].driver;
		_MGL_driverId = driver->driverId;
		if (!_MGL_initDC(dc,driver,0,virtualX,virtualY,numBuffers,stereo,refreshRate))
			goto Error;

		/* Set up the mouse cursor */
		_MS_setDisplayDC(dc);
		MS_setCursor(MGL_DEF_CURSOR);
		_MS_sizeScreen(dc->mi.xRes,dc->mi.yRes);
		MS_moveTo(dc->mi.xRes/2,dc->mi.yRes/2);

		/* Now clear the device page */
		MGL_makeCurrentDC(dc);
		MGL_clearDevice();

		/* And clear the event queue of any extraneous events */
		EVT_flush(EVT_EVERYEVT);
		_MGL_disableAutoPlay();
		}
	else {
		/* A valid display device context already exists, so simply clone
		 * the current one, and set the attributes to the default.
		 */
		*dc = *((MGLDC*)_LST_first(_MGL_dispDCList));
		dc->v->d.refCount++;

		/* Initialise internal dimensions */
		dc->size.left = 0;
		dc->size.top = 0;
		dc->size.right = dc->mi.xRes+1;
		dc->size.bottom = dc->mi.yRes+1;

		/* Allocate the color lookup table cache */
		dc->colorTab = MGL_malloc(sizeof(color_t) * 256);

		MGL_defaultAttributes(dc);
		MGL_setDefaultPalette(dc);
		}

	/* Add the new DC to the start of the DC chain */

	RESET_DEFAULT_CW();
	_LST_addAfter(_MGL_dispDCList,dc,LST_HEAD(_MGL_dispDCList));
	return dc;

Error:
	_MGL_fullScreen = false;
	if (_MGL_dispDCList && _MGL_dispDCList->count == 0) {
		_LST_destroy(_MGL_dispDCList,_LST_freeNode);
		_MGL_dispDCList = NULL;
		}
	_LST_freeNode(dc);
	return NULL;
}

/****************************************************************************
DESCRIPTION:
Create a new hardware scrolling display device context.

HEADER:
mgraph.h

PARAMETERS:
virtualX	- Virtual width of desired mode
virtualY	- Virtual height of desired mode
numBuffers	- Number of buffers for multibuffering

RETURNS:
Pointer to the newly created hardware scrolling display device context, or NULL if
not enough memory.

REMARKS:
Creates a new display device context for drawing information directly to the
hardware display device in fullscreen graphics modes. Essentially this function is
identical to MGL_createDisplayDC, however hardware scrolling (or panning) is
supported. Some hardware devices may not support hardware scrolling, in which
case this function will fail and return a NULL. In these cases you should provide an
alternative method of scrolling the display, such as drawing to a memory device
context and copying the appropriate portion of the image to the display.

If there are no currently active display device contexts, MGL will start the video
mode specified in the MGL_init call or the previous call to MGL_changeDisplayMode
(which is used to override the setting from MGL_init), and will initialize the
specific device driver. If however a display device context already exists, we
simply create a new device context to talk to the currently active display driver,
so that you can actually create two independent device contexts that address the
same display device (with totally independent attributes).

Once you have created a hardware scrolling device context, the display starting
coordinate will be set to (0,0) within the virtual image. To hardware pan around
within the virtual image, you can use the MGL_setDisplayStart function to change
the display starting x and y coordinates.

SEE ALSO:
MGL_createMemoryDC, MGL_createDisplayDC, MGL_destroyDC,
MGL_setWinDC, MGL_activatePalette, MGL_initWindowed, MGL_changeDisplayMode
****************************************************************************/
MGLDC * MGLAPI MGL_createScrollingDC(
	int virtualX,
	int virtualY,
	int numBuffers)
{
	return _MGL_createDisplayDC(virtualX,virtualY,numBuffers,false,MGL_DEFAULT_REFRESH);
}

/****************************************************************************
DESCRIPTION:
Create a new display device context.

HEADER:
mgraph.h

PARAMETERS:
numBuffers	- Number of buffers to allocate for double/multi-buffering.

RETURNS:
Pointer to the newly created display device context, NULL on failure

REMARKS:
Creates a new display device context for drawing information directly to the
hardware display device in fullscreen graphics modes. If there are no currently
active display device contexts, MGL will start the graphics mode specified in
the MGL_init call or the previous call to MGL_changeDisplayMode (which is
used to override the setting from MGL_init), and will initialize the specific
device driver. If however a display device context already exists, we simply
create a new device context to interface to the currently active display driver,
so that you can create two independent device contexts that address the same
display device (with totally independent attributes).

Note that when running under Windows, as soon as you have created the first
display device context, the system will have been switched into the fullscreen
graphics mode, and the normal Windows GDI functions will not be available for
drawing to the screen (under Windows95 you can use the MGL_getWinDC function
to get a Windows compatible HDC for drawing to the MGL device context however).
When you wish to return to GDI mode, a call to MGL_exit will destroy all
allocated display device contexts and return the system to GDI mode. Note
however that the MGL /late binds/ the return to GDI mode if you simply
destroy all active display device contexts. Hence the MGL will remain in
fullscreen mode after destroying the last active display device context in order
to provide a smooth transition between fullscreen graphics modes. Hence if you
wish to switch from 640x480x256 to 800x600x256, the MGL will remain in fullscreen
mode after destroying the 640x480x256 device context and will simply switch
directly to the 800x600x256 graphics mode when you create the new display
device context for this mode (after calling MGL_changeDisplayMode to make
800x600x256 the new active display mode). This way switches between fullscreen
modes are clean and do not result in an intermediate switch back to GDI mode.

However if you wish to return to GDI mode, such as when switching from
fullscreen mode to windowed mode, you have to call MGL_changeDisplayMode(grWINDOWED)
to inform the MGL that you wish to switch to windowed mode and the MGL will force
the switch back to GDI mode at this time.

If you intend to use double or multi-buffered graphics using the display device, you
should set the numBuffers to the number of buffers that you require, so that the
device will be properly configured for multi-buffered operation. Note that if you
request more buffers than is currently available, this function will fail. Hence you
should first call MGL_availablePages to determine how many buffers can be used
in the desired graphics mode.

Once the display device context has been allocated, if the surface pointer of the
MGLDC structure is not NULL, you can directly access the surface of the device
context as a flat linear framebuffer (only available in 32 bit versions) or as a banked
framebuffer. However for some display devices this surface is actually a virtual
linear framebuffer, and you must ensure that if you rasterize directly to the surface
that you only access it with BYTE, WORD or DWORD aligned accesses. If you
access it with a non-aligned location that spans a SuperVGA bank boundary, you
will hang the system by causing an infinite page fault loop. You can check the
surface access flags with the MGL_surfaceAccessType function to determine if the
surface is a banked, virtualized or hardware linear framebuffer. If the
MGL_surfaceAccessType function return MGL_NOACCESS the framebuffer will
be banked and if you wish to rasterize directly to it you will need to use the
SVGA_setBank functions to change banks. In banked modes the surface pointer
points to the start of the banked framebuffer window (ie: 0xA0000).

If the surface is not a hardware linear framebuffer, you may wish to do all your
rasterizing to a memory device context (always accessible as a linear buffer) and
then use MGL_bitBlt get it to the screen, letting MGL take care of the virtualization
issues or bank switching issues. In most cases we have tested, this usually leads
to higher performance than rasterizing directly to a banked or virtual linear
framebuffer directly. If the surface is a real hardware linear framebuffer you
should try to always rasterize directly to the surface, as this usually provides
the maximum possible performance for MGL and your own custom rasterizing code.

The dimensions and pixel format of the device context surface are stored in the
gmode_t field of the MGLDC structure, and you should use these values to write
your own direct rasterizing code.

Note that all device contexts have an associated color palette, even RGB device
contexts. In RGB modes the color palette is used for converting color index pixel
values to RGB values during BitBlt operations and with the MGL_realColor and
MGL_setColorCI function.

Note: 	Due to the dynamic nature of the speed of both graphics memory and system
		memory, and due to the fact that each tends to increase in speed at
		different rates we have found that on some systems rendering directly
		to the hardware linear framebuffer provides a larger performance
		improvement. However with new Pentium II systems coming out with
		faster memory subsystems, this may change and it may be faster to draw
		to system memory and then MGL_bitBlt it to the screen. But then again
		with the onset of AGP for graphic adapters, this may change again and
		make it much faster to render directly to video memory. Hence we /always/
		recommend that you provide the option to do both in your product, and
		if necessary profile each method on the end users system and select the
		one that provides the highest performance (dont /ever assume/ you know
		which is going to be the fastest!).

SEE ALSO:
MGL_init, MGL_destroyDC, MGL_createOffscreenDC, MGL_createMemoryDC,
MGL_createWindowedDC, MGL_createStereoDisplayDC, MGL_changeDisplayMode,
MGL_modeFlags
****************************************************************************/
MGLDC * MGLAPI MGL_createDisplayDC(
	int numBuffers)
{
	return _MGL_createDisplayDC(-1,-1,numBuffers,false,MGL_DEFAULT_REFRESH);
}

/****************************************************************************
DESCRIPTION:
Create a new display device context for stereo LC shutter glasses

HEADER:
mgraph.h

PARAMETERS:
numBuffers	- Number of buffers to allocate for double/multi-buffering.
refreshRate	- Requested refresh rate for the graphics mode

RETURNS:
Pointer to the newly created display device context, NULL on failure.

REMARKS:
Creates a new display device context for drawing information directly to the
hardware display device in fullscreen graphics modes. Essentially this function is
identical to MGL_createDisplayDC, however support for LC shutter glasses is
provided and the MGL will take care of automatically flipping between the
left and right images to create the stereo display. In some cases we may not
be able to initialise support for LC shutter glasses, and in this case this
function will return NULL.

If there are no currently active display device contexts, MGL will start the video
mode specified in the MGL_init call or the previous call to MGL_changeDisplayMode
(which is used to override the setting from MGL_init), and will initialize the
specific device driver. If however a display device context already exists, we
simply create a new device context to talk to the currently active display driver,
so that you can actually create two independent device contexts that address the
same display device (with totally independent attributes).

When running in stereo mode, the MGL actually allocates twice the number of
buffers that you request for drawing images, since we need one buffer for the
left eye image and another buffer for the right eye image (ie: if you request
two stereo buffers for double buffering, the MGL will actually allocate room
for four). The reason for this is that when displaying one of the stereo buffers,
the MGL will automatically /swap/ between the left and right eye images at every
vertical retrace. It also sends a signal to the LC shutter glasses to tell them
to block out the image for the eye that should not be seeing the image on the
screen (ie: when the left image is being displayed, the shutter over the right
eye will be blacked out). Hence by drawing images with slightly different
viewing parameters (ie: as viewed from the left or right eye when doing 3D
rendering), the user sees a single image with complete with visual depth cues!

When running in stereo mode, you have to tell the MGL which buffer you want to
draw to when drawing the left or right eye images. Just like you normally
do in double and multi-buffering, you use the MGL_setActivePage function to
tell the MGL the active display page you wish to draw to. However in stereo
modes you must also pass in the MGL_LEFT_BUFFER or MGL_RIGHT_BUFFER values
to tell the MGL which eye you are drawing for. For instance to draw to
stereo page 1, left eye you would use MGL_setActivePage(1 | MGL_LEFT_BUFFER),
and for the right eye you would use MGL_setActivePage(1 | MGL_RIGHT_BUFFER).

Note:	In OpenGL rendering modes, changing the draw buffer is done with the
		OpenGL glDrawBuffer(GL_BACK_LEFT) and glDrawBuffer(GL_BACK_RIGHT)
		functions instead of using MGL_setActivePage.

One of the biggest drawbacks to viewing stereo images using LC shutter glasses
is that the refresh rate viewed in each eye is exactly half that of the refresh
rate of the display mode. Hence if running in a display mode with a 60Hz refresh
rate, the user will experience an overall refresh rate of 30Hz per eye! As you
can image this can be extremely tiresome for extended viewing periods, so to
get around this the MGL allows you to pass in a value to request a higher
refresh rate for the mode. Ideally you want to try and use a refresh rate
that is twice the desired refresh rate per eye, such as 120Hz for viewing
images at 60Hz, however you /must/ allow the user to override or suggest a
desired refresh rate as many older monitors may not be capable of displaying an
image at a high refresh rate like 120Hz.

The refresh rate value that you pass in is a /suggested/ value in that
the MGL will attempt to set the refresh rate to this value, however if the
hardware does not support that refresh rate the next lowest available refresh
rate will be used instead. In some situations where no refresh rate control
is available, the value will be ignored and the adapter default refresh rate
will be used. If you dont care about the refresh rate and want to use the
adapter default setting, pass in a value of MGL_DEFAULT_REFRESH.

Note:	In the USA and Canada, the mains frequency runs at 60Hz, and all
		fluorescent lights will be illuminating your room at frequency of
		60Hz. If you use a refresh rate that is not a multiple of the mains
		frequency and you are viewing the image in a room with fluorescent
		lights, you may experience severe /beating/ at a frequency that is
		the difference between the monitor refresh rate and the fluorescent
		light frequency (ie: at 100Hz you will experience a 20Hz annoying
		beat frequency). In order to get around this problem, always try to
		use a frequency that is double the mains frequency such as 120Hz
		to avoid the beating, or have the user turn off their fluorescent
		lights!

When you create a stereo display device context, the MGL does not automatically
start stereo page flipping, and you must start this with a call to
MGL_startStereo. You can also turn stereo mode on an off at any time (ie: you
can turn it off when you go to your menu system) using the MGL_stopStereo and
MGL_startStereo functions. Note that when stereo mode is disabled, the MGL
always displays from the left eye buffer.

When viewing an image with LC shutter glasses, the MGL also needs to know how
to communicate with the LC shutter glasses and let them know when the left and
right eye images are being displayed on the screen. This communication
mechanism is called the /stereo sync/, and the MGL supports a number of
different types of stereo sync mechanisms (blue codes, serial port, parallel
port and hardware stereo sync). You can use the MGL_setStereoSyncType function
to change the stereo sync method used to communicate with the LC shutter
glasses. 

SEE ALSO:
MGL_createDisplayDC, MGL_destroyDC, MGL_changeDisplayMode, MGL_startStereo,
MGL_stopStereo, MGL_setStereoSyncType, MGL_setBlueCodeIndex
****************************************************************************/
MGLDC * MGLAPI MGL_createStereoDisplayDC(
	int numBuffers,
	int refreshRate)
{
	return _MGL_createDisplayDC(-1,-1,numBuffers,true,refreshRate);
}

/****************************************************************************
DESCRIPTION:
Enables free running stereo display mode

HEADER:
mgraph.h

REMARKS:
This function enables the free running stereo display mode for LC shutter
glasses. This function only works if the display device context you created
was a stereo display device context created with MGL_createStereoDisplayDC.
By default free running stereo mode is on when you create the stereo
display device context.

SEE ALSO:
MGL_stopStereo, MGL_createStereoDisplayDC
****************************************************************************/
void MGLAPI MGL_startStereo(MGLDC *dc)
{
	if (_MGL_isFullscreenDevice(dc) && dc->v->d.isStereo != _MGL_NO_STEREO)
    	dc->v->d.startStereo(dc);
}

/****************************************************************************
DESCRIPTION:
Disables free running stereo display mode

HEADER:
mgraph.h

REMARKS:
This function disables the free running stereo display mode for LC shutter
glasses. This function only works if the display device context you created
was a stereo display device context created with MGL_createStereoDisplayDC.
By default free running stereo mode is on when you create the stereo
display device context, and you can use this function to disable it in
parts of your application that dont require stereo (such as when navigating
the menu system etc). Note that when stereo mode is disabled, the MGL always
displays from the left eye buffer.

SEE ALSO:
MGL_startStereo, MGL_createStereoDisplayDC
****************************************************************************/
void MGLAPI MGL_stopStereo(MGLDC *dc)
{
	if (_MGL_isFullscreenDevice(dc) && dc->v->d.isStereo != _MGL_NO_STEREO)
		dc->v->d.stopStereo(dc);
}

/****************************************************************************
DESCRIPTION:
Sets the stereo sync type for LC shutter glasses

HEADER:
mgraph.h

REMARKS:
This function sets the stereo sync type that the MGL uses to communicate
with the LC shutter glasses, letting them know when the left and right
eye images are being displayed on the screen. By default the MGL starts
in MGL_STEREO_AUTO mode and if there is no hardware stereo sync available
it will enable all supports sync mechanisms at the same time. This provides
the simplest and most powerful support, since the user can plug in any
of the supported LC glasses and correctly view and image. However you
may wish to force the value to one of the supported mechanisms listed
in the MGL_stereoSyncType enumeration.

Hardware stereo sync is supported by many new graphics controllers, and
requires a physical connector from the back of the graphics adapter (usually
a VESA MINI DIN-3 connector or VESA EVC connector) that passes the sync
information directly from the graphics chip to the LC shutter glasses. Many
older graphics adapters dont support this and the MGL will use a software
sync mechanism on these adapters. If the user has problems with the hardware
stereo sync (such as if they have older glasses that dont support this),
you can disable this and force software syncing with the
MGL_STEREO_IGNORE_HARDWARE flag.

Note:	 You may call this function while a stereo image is being displayed
		 to change the stereo sync mechanism on the fly.

SEE ALSO:
MGL_startStereo, MGL_stopStereo, MGL_createStereoDisplayDC,
MGL_setBlueCodeIndex
****************************************************************************/
void MGLAPI MGL_setStereoSyncType(
	int type)
{ _MGL_stereoSyncType = type; }

/****************************************************************************
DESCRIPTION:
Sets the color index used for the blue code stereo sync mechanism

HEADER:
mgraph.h

REMARKS:
This function sets the color index used in the blue code stereo sync
mechanism. The blue code system as pioneered by StereoGraphics, requires
the MGL to draw pure blue sync lines at the bottom of the screen of differing
lengths to signal to the LC glasses which is the left and right eye
images. In order to do this in 8bpp color index modes, the MGL must take
up a single palette entry for drawing the blue codes, which by default
is set to index 255. You can use this function to set the blue code index
to another value other than 255 to suit your applications palette arrangement.

Note: 	You must call this function /before/ you create a stereo display
		device context if you wish to change the blue code index.

SEE ALSO:
MGL_startStereo, MGL_stopStereo, MGL_createStereoDisplayDC,
MGL_setStereoSyncType
****************************************************************************/
void MGLAPI MGL_setBlueCodeIndex(
	int index)
{ _MGL_blueCodeIndex = index; }

/* {secret} */
ibool _MGL_destroyDisplayDC(MGLDC *dc)
/****************************************************************************
*
* Function:		_MGL_destroyDisplayDC
* Parameters:	dc	- Device context to destroy
* Returns:		True if the context was destroyed, false on error.
*
* Description:	Searches for the display device context in the list of
*				currently allocated contexts, and destroys it if it
*				exists. If this is the last display device context, then
*				we also shut down the graphics mode and restore the
*				original graphics mode.
*
****************************************************************************/
{
	MGLDC	*prev;

	if (_MGL_dispDCList->count == 1) {
		/* There is only one active display context left, so restore
		 * the original display mode and destroy the entire display
		 * device context list
		 */
		if (dc != _LST_first(_MGL_dispDCList)) {
			_MGL_result = grInvalidDC;
			return false;
			}
		dc->v->d.restoreTextMode(dc);
		MGL_free(dc->colorTab);
		dc->v->d.destroy(dc);
		_LST_destroy(_MGL_dispDCList,_LST_freeNode);
		_MGL_dispDCList = NULL;

		/* We are no longer in Windows full screen mode but back in GDI */
#ifdef	__WINDOWS__
		_MS_setDisplayDC(NULL);
#endif
		}
	else {
		/* There is more than one active display context, so simply
		 * remove the current one from the list and destroy it.
		 */
		if ((prev = _LST_findNode(_MGL_dispDCList,dc)) == NULL) {
			_MGL_result = grInvalidDC;
			return false;
			}
		MGL_free(dc->colorTab);
		dc->v->d.destroy(dc);
		_LST_removeNext(_MGL_dispDCList,prev);
		_LST_freeNode(dc);
		}
	return true;
}

#ifndef	MGL_LITE
/* {secret} */
MGLDC * _MGL_createOffscreenDC(ibool linear)
/****************************************************************************
*
* Function:		_MGL_createOffscreenDC
* Parameters:	linear	- True if we should build a linear access device
* Returns:		Pointer to the newly created offscreen device context.
*
* Description:	Creates an MGL offscreen display device context. You can
*				only create an offscreen DC after you have created a full
*				screen MGLDisplayDC. The OffscreenDC then addresses the
*				remainder of accessible hardware video memory and can be
*				used for offscreen rendering and caching of bitmap data for
*				fast hardware transfers to the display memory with hardware
*				bitBlt operations.
*
*				Note that the scanline width of the offscreen DC will be
*				idential to the scanline width of the normal display DC (but
*				the horizontal resolution may differ slightly).
*
*				For linear display DC's, the offscreen memory can be
*				accessed directly as a linear cache for bitmaps, and you
*				can perform MGL_bitBltLin and MGL_transBltLin operations
*               between the offscreen DC and the display DC. For a normal
*				offscreen DC you can only perform rectangular blt's so you
*				need to divide up the offscreen device context into
*				rectangular areas to hold your bitmaps. Linear offscreen
*				DC's are only available on some hardware accelerated
*				devices.
*
****************************************************************************/
{
	MGLDC	*dc,*dispDC;

	if (!_MGL_dispDCList)
		MGL_fatalError("You must create a Display DC first!\n");
	dispDC = (MGLDC*)_LST_first(_MGL_dispDCList);
	if (linear) {
		if (!dispDC->v->d.makeLinearOffscreenDC) {
			_MGL_result = grNoModeSupport;
			return NULL;
			}
		}
	else {
		if (!dispDC->v->d.makeOffscreenDC) {
			_MGL_result = grNoModeSupport;
			return NULL;
			}
		}

	if ((dc = _LST_newNode(sizeof(MGLDC))) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}

	if (_MGL_offDCList == NULL) {
		if ((_MGL_offDCList = _LST_create()) == NULL) {
			_MGL_result = grNoMem;
			goto Error;
			}
		}

	/* A valid display device context already exists, so simply clone
	 * the current one, and set the attributes to the default.
	 */
	*dc = *((MGLDC*)_LST_first(_MGL_dispDCList));
	dc->v->d.refCount++;
	if (linear) {
		if (!dc->v->d.makeLinearOffscreenDC(dc))
			goto Error;
		}
	else {
		if (!dc->v->d.makeOffscreenDC(dc))
			goto Error;
		}

	/* Allocate the color lookup table cache */
	dc->colorTab = MGL_malloc(sizeof(color_t) * 256);
	MGL_defaultAttributes(dc);
	MGL_setDefaultPalette(dc);

	/* Add the new DC to the start of the DC chain */
	RESET_DEFAULT_CW();
	_LST_addAfter(_MGL_offDCList,dc,LST_HEAD(_MGL_offDCList));
	return dc;

Error:
	if (_MGL_offDCList && _MGL_offDCList->count == 0) {
		_LST_destroy(_MGL_offDCList,_LST_freeNode);
		_MGL_offDCList = NULL;
		}
	_LST_freeNode(dc);
	return NULL;
}

/****************************************************************************
DESCRIPTION:
Creates a new offscreen display device context.

HEADER:
mgraph.h

RETURNS:
Pointer to the newly created offscreen device context, NULL if not valid.

REMARKS:
Creates a new offscreen display device context for rasterizing to offscreen
video memory when running in hardware accelerated video modes. You must already
have created a valid display device context before this function is called,
otherwise this function will return NULL. Also if the display device does not
support hardware accelerated offscreen display memory, this function will also
return NULL indicating that the device context could not be created. See the
MGL_result error code for more information on why the function failed.

Once the display device context has been allocated, if the surface pointer of the
MGLDC structure is not NULL, you can directly access the surface of the device
context as a flat linear framebuffer (only available in 32 bit versions) or as a banked
framebuffer. However for some display devices this surface is actually a virtual
linear framebuffer, and you must ensure that if you rasterize directly to the surface
that you only access it with BYTE, WORD or DWORD aligned accesses. If you
access it with a non-aligned location that spans a SuperVGA bank boundary, you
will hang the system by causing an infinite page fault loop. You can check the
surface access flags with the MGL_surfaceAccessType function to determine if the
surface is a banked, virtualized or hardware linear framebuffer. If the
MGL_surfaceAccessType function return MGL_NOACCESS the framebuffer will
be banked and if you wish to rasterize directly to it you will need to use the
SVGA_setBank functions to change banks. In banked modes the surface pointer
points to the start of the banked framebuffer window (ie: 0xA0000).

The dimensions and pixel format of the device context surface are stored in
the gmode_t field of the MGLDC structure, and you should use these values to
write your own direct rasterizing code. Note also that for offscreen device
contexts, you can also rasterize directly to any part for the offscreen
buffering using MGL rasterizing routines, which will use the hardware
accelerator where possible.

SEE ALSO:
MGL_init, MGL_destroyDC, MGL_createDisplayDC, MGL_createLinearOffscreenDC,
MGLDC
****************************************************************************/
MGLDC * MGLAPI MGL_createOffscreenDC(void)
{ return _MGL_createOffscreenDC(false); }


/*******************************************************************************
DESCRIPTION:
Creates a new linear offscreen display device context.

HEADER:
mgraph.h

RETURNS:
Pointer to the newly created linear offscreen device context, NULL if not valid.

REMARKS:
Creates a new linear offscreen display device context for storing bitmaps linearly in
offscreen video memory when running in hardware accelerated graphics modes. You
must already have created a valid display device context before this function is 
called, otherwise this function will return NULL. If the display device does not 
support hardware accelerated linear offscreen display memory (many do not), this 
function will also return NULL. See the MGL_result error code for more
information on why the function failed.

A linear offscreen DC is almost identical to a normal offscreen device context, and 
you can rasterize directly to it as a rectangular region. However it is primarily 
intended for storing bitmaps and sprites in offscreen memory linearly (i.e. one after 
each other in memory, not as rectangular bitmaps) for better memory efficiency on 
devices that can support this. All devices that support a linear offscreen device 
context will also support a normal offscreen device context.

SEE ALSO:
MGL_init, MGL_destroyDC, MGL_createDisplayDC, MGL_createOffscreenDC, 
MGLDC
*******************************************************************************/
MGLDC * MGLAPI MGL_createLinearOffscreenDC(void)
{ return _MGL_createOffscreenDC(true); }

/* {secret} */
ibool _MGL_destroyOffscreenDC(MGLDC *dc)
/****************************************************************************
*
* Function:		_MGL_destroyOffscreenDC
* Parameters:	dc	- Device context to destroy
* Returns:		True if the context was destroyed, false on error.
*
* Description:	Kills the offscreen display DC.
*
****************************************************************************/
{
	MGLDC	*prev;

	/* Remove the DC from the list of allocated memory DC's				*/

	if ((prev = _LST_findNode(_MGL_offDCList,dc)) == NULL) {
		_MGL_result = grInvalidDC;
		return false;
		}
	_LST_removeNext(_MGL_offDCList,prev);

	/* Free up all memory occupied by the DC */
	MGL_free(dc->colorTab);
	dc->v->d.destroy(dc);
	_LST_freeNode(dc);

	if (_MGL_offDCList->count == 0) {
		/* This is the last offscreen device context, so destroy the list */
		_LST_destroy(_MGL_offDCList,_LST_freeNode);
		_MGL_offDCList = NULL;
		}

	return true;
}
#endif

/****************************************************************************
DESCRIPTION:
Creates a new custom memory device context.

HEADER:
mgraph.h

PARAMETERS:
xSize	- X resolution for the memory context
ySize	- Y resolution for the memory context
bitsPerPixel	- Pixel depth for the memory context
pf	- Pixel format for memory context
bytesPerLine	- Buffer pitch for memory context
surface	- Pointer to surface memory for context
hbm	- Handle to HBITMAP for DIB section

RETURNS:
Pointer to the allocated memory device context.

REMARKS:
This function is useful for creating a display context in memory provided by
an application, allowing the SciTech MGL to render to memory it does not own.
(e.g. custom hardware with LFB's).

For Windows if the hbm parameter is not NULL, it is assumed that the
original memory was created by a call to CreateDIBSection and the hbm
parameter is a handle to the bitmap object for the DIB section. This parameter
can be used to blit the memory DC image to a windowed DC using the standard
Windows GDI blit functions. The hbm parameter is not necessary for
fullscreen modes.

****************************************************************************/
MGLDC * MGLAPI MGL_createCustomDC(
	int xSize,
	int ySize,
	int bitsPerPixel,
	pixel_format_t *pf,
	int bytesPerLine,
	void *surface,
	MGL_HBITMAP hbm)
{
	MGLDC	*dc;
	int		id;

	if ((dc = _LST_newNode(sizeof(MGLDC))) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}

	if (_MGL_memDCList == NULL) {
		if ((_MGL_memDCList = _LST_create()) == NULL) {
			_MGL_result = grNoMem;
			goto Error;
			}
		}

	/* Find the id of the device driver to use */
	switch (bitsPerPixel) {
		case 1:		id = PACKED1INDEX;	break;
		case 4:	 	id = PACKED4INDEX;	break;
		case 8:	 	id = PACKED8INDEX;	break;
		case 15:
		case 16: 	id = PACKED16INDEX;	break;
		case 24: 	id = PACKED24INDEX;	break;
		case 32: 	id = PACKED32INDEX;	break;
		default:
			_MGL_result = grInvalidMode;
			goto Error;
		}

	/* Store the mode information and initalise the device context	*/
	dc->size.left = 0;
	dc->size.top = 0;
	dc->size.right = xSize;
	dc->size.bottom = ySize;
	dc->mi.xRes = xSize-1;
	dc->mi.yRes = ySize-1;
	dc->mi.bitsPerPixel = bitsPerPixel;
	dc->mi.bytesPerLine = bytesPerLine;
	dc->surface = surface;
	if (pf)
		dc->pf = *pf;
	dc->ownMemory = false;
	if (!_MGL_initDC(dc,_MGL_driverTable[id].driver,NULL,-1,-1,1,false,MGL_DEFAULT_REFRESH))
		goto Error;

#ifdef	__WINDOWS32__
	/* If the user passed in a valid Windows HBITMAP, then use this to
	 * set up our device context information so we can blit the image to
     * a Windowed DC.
	 */ 
	if (hbm) {
		HDC	hdc = GetDC(NULL);
		dc->wm.memdc.hdc = CreateCompatibleDC(hdc);
		SelectObject(dc->wm.memdc.hdc,hbm);
		dc->wm.memdc.hbm = hbm;
		ReleaseDC(NULL,hdc);
		}
#endif

	/* Add the new DC to the start of the DC chain */
	RESET_DEFAULT_CW();
	_LST_addAfter(_MGL_memDCList,dc,LST_HEAD(_MGL_memDCList));
	return dc;

Error:
	if (_MGL_memDCList && _MGL_memDCList->count == 0) {
		_LST_destroy(_MGL_memDCList,_LST_freeNode);
		_MGL_memDCList = NULL;
		}
	_LST_freeNode(dc);
	return NULL;
}



/****************************************************************************
DESCRIPTION:
Create a new memory device context.

HEADER:
mgraph.h

PARAMETERS:
xSize	- x resolution for the memory context
ySize	- y resolution for the memory context
bitsPerPixel	- Pixel depth for the memory context
pf	- Pixel format for memory context (NULL for 8 bits and below)

RETURNS:
Pointer to the allocated memory device context, NULL if not enough memory.

REMARKS:
Creates a new memory device context, allocating the necessary memory resources
to hold the surface of the memory device given the specified resolution and pixel
depth. The surface of a memory device context is always allocated using the
appropriate operating system specific functions, and you can always directly access
the surface of the device context via the surface pointer of the MGLDC structure. If
you do directly access the surface, the dimensions and pixel format of the device
context surface are stored in the gmode_t field of the MGLDC structure, and you
should use these values to write your own direct rasterizing code.

For memory device contexts with pixel depths greater than 8 bits per pixel, you
must also pass a valid pixel_format_t structure which defines the pixel format to be
used for the device context. If you wish to create a memory device context for your
main rasterizing context which you then wish to Blt to the screen, you must ensure
that you use the same pixel format for the memory device as the display device for
the current graphics mode, otherwise the pixel formats will be translated on the
fly by the MGL_bitBlt function resulting in very low
performance. You can use the MGL_getPixelFormat function to obtain the pixel
format information for the display device context you are using.

Note that before you can call this routine, you must ensure that you have
registered the appropriate packed pixel device drivers that you are interested
in using in your code with the MGL_registerDriver function. The process of
registering the device drivers ensure that the code will be linked in when you
link your application (by default the code will not be linked, to save space
in the resulting executable). You can simply call MGL_RegisterAllDispDrivers
to register packed pixel drivers for all possible pixel depths, however this
will end up linking in a lot of code that may not be necessary for your
application.

Note that all device contexts have an associated color palette, even RGB device
contexts. In RGB modes the color palette is used for converting color index pixel
values to RGB values during BitBlt operations and with the MGL_realColor and
MGL_setColorCI function.

SEE ALSO:
MGL_createDisplayDC, MGL_destroyDC, MGL_createWindowedDC, MGLDC
****************************************************************************/
MGLDC * MGLAPI MGL_createMemoryDC(
	int xSize,
	int ySize,
	int bitsPerPixel,
	pixel_format_t *pf)
{ return MGL_createCustomDC(xSize,ySize,bitsPerPixel,pf,0,0,NULL); }

/* {secret} */
ibool _MGL_destroyMemoryDC(MGLDC *dc)
/****************************************************************************
*
* Function:		_MGL_destroyMemoryDC
* Parameters:	dc	- Device context to destroy
* Returns:		True if the context was destroyed, false on error.
*
* Description:	Searches for the memory device context in the list of
*				currently allocated contexts, and destroys it if it
*				exists.
*
****************************************************************************/
{
	MGLDC	*prev;

	/* Remove the DC from the list of allocated memory DC's				*/

	if ((prev = _LST_findNode(_MGL_memDCList,dc)) == NULL) {
		_MGL_result = grInvalidDC;
		return false;
		}
	_LST_removeNext(_MGL_memDCList,prev);

	/* Free up all memory occupied by the DC */
	MGL_free(dc->colorTab);
	dc->v->m.destroy(dc);
	_LST_freeNode(dc);

	if (_MGL_memDCList->count == 0) {
		/* This is the last memory device context, so destroy the list	*/

		_LST_destroy(_MGL_memDCList,_LST_freeNode);
		_MGL_memDCList = NULL;
		}

	return true;
}

/****************************************************************************
DESCRIPTION:
Returns a list of all available graphics modes.

HEADER:
mgraph.h

RETURNS:
Pointer to list of available graphics modes

REMARKS:
Returns a list of all the currently available graphics modes.  You may call this routine
before MGL_init is called to determine what graphics modes are available before
actually initializing a particular graphics mode. You must ensure however that the
MGL_detectGraph routine is called before you call this routine.

The list of available graphics modes is returned as a table of 8-bit integer values. The
table is terminated with a -1(0xFF). MGL supports a number of different video
mode resolutions, ranging from 320x200 up to 1600x1200 with color ranges from
16 colors up to 16.7 million colors. See the MGL_modeType type for a list of
all the valid MGL graphics modes.

SEE ALSO:
MGL_detectGraph, MGL_init, MGL_availablePages

****************************************************************************/
uchar * MGLAPI MGL_availableModes(void)
{
	return _MGL_modeTable;
}

/****************************************************************************
DESCRIPTION:
Determine the number of available video pages for a specific graphics mode.

PARAMETERS:
mode	- MGL mode number to query

HEADER:
mgraph.h

RETURNS:
Number of available video pages for mode, -1 for invalid mode number.

REMARKS:
Returns the number of pages of physical video memory available for a specific
MGL graphics mode. You may call this routine before the MGL_init routine is used to
initialize a particular graphics modes. You must ensure however that the
MGL_detectGraph routine is called before you call this routine. Thus you can filter
out support for modes that do not have the required number of hardware video
pages that your application requires.

SEE ALSO:
MGL_detectGraph, MGL_init, MGL_availableModes

****************************************************************************/
int MGLAPI MGL_availablePages(
	int mode)
{
	if (mode < 0 || mode >= grMAXMODE)
		return -1;
	return _MGL_availableModes[mode].pages;
}

/****************************************************************************
DESCRIPTION:
Returns the resolution and pixel depth of a specific graphics mode.

HEADER:
mgraph.h

PARAMETERS:
mode			- graphics mode number to get resolution for
xRes			- Place to store the x resolution
yRes			- Place to store the y resolution
bitsPerPixel	- Place to store the pixel depth

RETURNS:
True on success, false for an invalid graphics mode number.

REMARKS:
This function returns the pixel resolution and depth of the specified MGL mode
number, and can be used to display this information to the end user, or to search for
specific graphics modes for use by an application program depending on the desired
resolution or color depth.

SEE ALSO:
MGL_modeName, MGL_driverName
****************************************************************************/
int MGLAPI MGL_modeResolution(
	int mode,
	int *xRes,
	int *yRes,
	int *bitsPerPixel)
{
	struct { int xRes,yRes; uchar bits;	} modeRes[] = {
		{320,200,4},          /* grVGA_320x200x16     */
		{640,200,4},          /* grVGA_640x200x16     */
		{640,350,4},          /* grVGA_640x350x16     */
		{640,400,4},          /* grVGA_640x400x16     */
		{640,480,4},          /* grVGA_640x480x16     */
		{800,600,4},          /* grSVGA_800x600x16    */

        {320,200,8},          /* grVGAX_320x200x256   */
        {320,240,8},          /* grVGAX_320x240x256   */
        {320,400,8},          /* grVGAX_320x400x256   */
        {320,480,8},          /* grVGAX_320x480x256   */

        {320,200,8},          /* grVGA_320x200x256    */

		{320,200,8},          /* grVGAS_320x200x256	  */
		{320,240,8},          /* grSVGA_320x240x256   */
        {320,400,8},          /* grSVGA_320x400x256   */
        {320,480,8},          /* grSVGA_320x480x256   */
        {400,300,8},          /* grSVGA_400x300x256   */
        {512,384,8},          /* grSVGA_512x384x256   */
        {640,350,8},          /* grSVGA_640x350x256   */
        {640,400,8},          /* grSVGA_640x400x256   */
        {640,480,8},          /* grSVGA_640x480x256   */
        {800,600,8},          /* grSVGA_800x600x256   */
        {1024,768,8},         /* grSVGA_1024x768x256  */
        {1152,864,8},         /* grSVGA_1152x864x256  */
        {1280,960,8},         /* grSVGA_1280x960x256  */
        {1280,1024,8},        /* grSVGA_1280x1024x256 */
        {1600,1200,8},        /* grSVGA_1600x1200x256 */

        {320,200,15},         /* grSVGA_320x200x32k   */
        {320,240,15},         /* grSVGA_320x240x32k   */
        {320,400,15},         /* grSVGA_320x400x32k   */
        {320,480,15},         /* grSVGA_320x480x32k   */
        {400,300,15},         /* grSVGA_400x300x32k   */
        {512,384,15},         /* grSVGA_512x384x32k   */
        {640,350,15},         /* grSVGA_640x350x32k   */
        {640,400,15},         /* grSVGA_640x400x32k   */
        {640,480,15},         /* grSVGA_640x480x32k   */
        {800,600,15},         /* grSVGA_800x600x32k   */
        {1024,768,15},        /* grSVGA_1024x768x32k  */
        {1152,864,15},        /* grSVGA_1152x864x32k  */
        {1280,960,15},        /* grSVGA_1280x960x32k  */
        {1280,1024,15},       /* grSVGA_1280x1024x32k */
        {1600,1200,15},       /* grSVGA_1600x1200x32k */

        {320,200,16},         /* grSVGA_320x200x64k   */
        {320,240,16},         /* grSVGA_320x240x64k   */
        {320,400,16},         /* grSVGA_320x400x64k   */
        {320,480,16},         /* grSVGA_320x480x64k   */
        {400,300,16},         /* grSVGA_400x300x64k   */
        {512,384,16},         /* grSVGA_512x384x64k   */
        {640,350,16},         /* grSVGA_640x350x64k   */
        {640,400,16},         /* grSVGA_640x400x64k   */
        {640,480,16},         /* grSVGA_640x480x64k   */
        {800,600,16},         /* grSVGA_800x600x64k   */
        {1024,768,16},        /* grSVGA_1024x768x64k  */
        {1152,864,16},        /* grSVGA_1152x864x64k  */
        {1280,960,16},        /* grSVGA_1280x960x64k  */
        {1280,1024,16},       /* grSVGA_1280x1024x64k */
        {1600,1200,16},       /* grSVGA_1600x1200x64k */

        {320,200,24},         /* grSVGA_320x200x16m   */
        {320,240,24},         /* grSVGA_320x240x16m   */
        {320,400,24},         /* grSVGA_320x400x16m   */
        {320,480,24},         /* grSVGA_320x480x16m   */
        {400,300,24},         /* grSVGA_400x300x16m   */
        {512,384,24},         /* grSVGA_512x384x16m   */
        {640,350,24},         /* grSVGA_640x350x16m   */
        {640,400,24},         /* grSVGA_640x400x16m   */
        {640,480,24},         /* grSVGA_640x480x16m   */
        {800,600,24},         /* grSVGA_800x600x16m   */
        {1024,768,24},        /* grSVGA_1024x768x16m  */
        {1152,864,24},        /* grSVGA_1152x864x16m  */
        {1280,960,24},        /* grSVGA_1280x960x16m  */
        {1280,1024,24},       /* grSVGA_1280x1024x16m */
        {1600,1200,24},       /* grSVGA_1600x1200x16m */

        {320,200,32},         /* grSVGA_320x200x4G    */
        {320,240,32},         /* grSVGA_320x240x4G    */
        {320,400,32},         /* grSVGA_320x400x4G    */
        {320,480,32},         /* grSVGA_320x480x4G    */
        {400,300,32},         /* grSVGA_400x300x4G    */
        {512,384,32},         /* grSVGA_512x384x4G    */
        {640,350,32},         /* grSVGA_640x350x4G    */
        {640,400,32},         /* grSVGA_640x400x4G    */
        {640,480,32},         /* grSVGA_640x480x4G    */
        {800,600,32},         /* grSVGA_800x600x4G    */
        {1024,768,32},        /* grSVGA_1024x768x4G   */
        {1152,864,32},        /* grSVGA_1152x864x4G   */
        {1280,960,32},        /* grSVGA_1280x960x4G   */
        {1280,1024,32},       /* grSVGA_1280x1024x4G  */
        {1600,1200,32},       /* grSVGA_1600x1200x4G  */
		};

	if (mode < 0 || mode >= grMAXMODE)
		return false;
	*xRes = modeRes[mode].xRes;
	*yRes = modeRes[mode].yRes;
	*bitsPerPixel = modeRes[mode].bits;
    return true;
}

/****************************************************************************
DESCRIPTION:
Returns the mode flags for the specific graphics mode.

HEADER:
mgraph.h

PARAMETERS:
mode	- graphics mode number to get flags for

RETURNS:
Flags for the specific graphics mode, or 0 for invalid mode.

REMARKS:
This function returns mode flags for the specified mode. The mode flags
are available after calling MGL_detectGraph, and provides information
about the hardware capabilities of the graphics mode, such as whether
it supports 2D or 3D acceleration, video acceleration, refresh rate control
and hardware stereo support. You can use these flags to make choices about
the graphics mode to use for your application prior to initialising a
specific graphics mode.

Specific mode flags are enumerated in MGL_modeFlagsType.

SEE ALSO:
MGL_modeName, MGL_driverName, MGL_modeResolution
****************************************************************************/
ulong MGLAPI MGL_modeFlags(
	int mode)
{
	if (mode < 0 || mode >= grMAXMODE)
		return 0;
	return _MGL_availableModes[mode].flags;
}

/* {secret} */
long _MGL_fileSize(FILE *f)
/****************************************************************************
*
* Function:		_MGL_fileSize
* Parameters:	f	- Open file to determine the size of
* Returns:		Length of the file in bytes.
*
* Description:	Determines the length of the file, without altering the
*				current position in the file.
*
****************************************************************************/
{
	long	size,oldpos = _MGL_ftell(f);

	_MGL_fseek(f,0,SEEK_END);		/* Seek to end of file				*/
	size = _MGL_ftell(f);			/* Determine the size of the file	*/
	_MGL_fseek(f,oldpos,SEEK_SET);	/* Seek to old position in file		*/
	return size;					/* Return the size of the file		*/
}

/****************************************************************************
DESCRIPTION:
Restricts the output from the display device context to a specified output
region.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to convert
left	- Minimum left coordinate for all subsequent output
top	- Minimum top coordinate for all subsequent output
right	- Maximum right coordinate for all subsequent output
bottom	- Maximum bottom coordinate for all subsequent output

REMARKS:
This is similar to setting a
viewport for the device, however this routine fully restricts
all output to this specified rectangle, effectively making
a DC that is a subsize of the original DC size. Once this
call is made, the size of the zbuffer and system memory
buffer need for double buffering with this DC are reduced
to the specified size. Normally the dimensions for the
entire display device would be used.
****************************************************************************/
void MGLAPI MGL_makeSubDC(
	MGLDC *dc,
	int left,
	int top,
	int right,
	int bottom)
{
	rect_t 	r;
	MGLDC	*oldDC = _MGL_dcPtr;

	/* Compute new internal dimensions for the display DC restricted to
	 * the physical display dimension size
	 */
	r.left = left;		r.top = top;
	r.right = right;	r.bottom = bottom;
	dc->size.left = dc->size.top = 0;
	dc->size.right = dc->mi.xRes+1;
	dc->size.bottom = dc->mi.yRes+1;
	if (MGL_sectRect(r,dc->size,&r)) {
		_MGL_updateCurrentDC(dc);
		MGL_setViewport(r);
		MGL_makeCurrentDC(NULL);		/* Flush changes to DC 			*/
		MGL_makeCurrentDC(oldDC);		/* Restore previous active DC	*/
		dc->size = r;
		}
}

/****************************************************************************
DESCRIPTION:
Restricts the output from the display device context to a specified output
region.

HEADER:
mgraph.h

PARAMETERS:
dir		- MGL directory to find the find under
name	- Name of the file to open

REMARKS:
Attempts to open the specified MGL file in binary mode. This routine will
use the standard MGL directory searching algorithm to find the specified
file. First an attempt is made to locate the file relative to the _MGL_path
variable (initialized by the application program via the MGL_init call).
If this fails, an attempt is made to search for the file relative to the
MGL_ROOT environment variable if this is present. Lastly we search the
current directory for the file (without adding the dir extension).
Otherwise we return NULL.
****************************************************************************/
/* {secret} */
ibool MGLAPI _MGL_findFile(
	char *validpath,
	const char * dir,
	const char *name,
	const char *mode)
{
	FILE	*f;

	/* First try opening the file with just the file name (checking for
	 * local directories and an explicit file path).
	 */
	strcpy(validpath,name);
	if ((f = _MGL_fopen(validpath,mode)) == NULL) {
		validpath[0] = '\0';
		if (_MGL_path[0] != '\0') {
			strcpy(validpath, _MGL_path);
			if (validpath[strlen(validpath)-1] != '\\')
				strcat(validpath, "\\");
			}
		strcat(validpath, dir);
		strcat(validpath, name);

		if ((f = _MGL_fopen(validpath,mode)) == NULL) {
			/* Search the environment variable */

			if (getenv(MGL_ROOT)) {
				strcpy(validpath, getenv(MGL_ROOT));
				if (validpath[strlen(validpath)-1] != '\\')
					strcat(validpath,"\\");
				strcat(validpath, dir);
				strcat(validpath, name);
				if ((f = _MGL_fopen(validpath, mode)) == NULL) {
					/* Search the current directory for the file */
					strcpy(validpath, name);
					f = _MGL_fopen(validpath, mode);
					}
				}
			}
		}
	if (f)
		_MGL_fclose(f);
	return (f != NULL);
}

/* {secret} */
FILE * _MGL_openFile(const char *dir, const char *name, const char *mode)
/****************************************************************************
*
* Function:		_MGL_openFile
* Parameters:	dir		- MGL directory to find the find under
*				name	- Name of the file to open
* Returns:		Pointer to the opened file, or NULL if not found.
*
****************************************************************************/
{
	char    validpath[_MAX_PATH];

	if (_MGL_findFile(validpath,dir,name,mode))
		return _MGL_fopen(validpath,mode);
	return NULL;
}

/****************************************************************************
DESCRIPTION:
Returns a string describing the name of the device driver.

HEADER:
mgraph.h

PARAMETERS:
Driver	- Device driver id number

RETURNS:
Pointer to device driver name string.

SEE ALSO:
MGL_modeName, MGL_modeDriverName

****************************************************************************/
const char * MGLAPI MGL_driverName(
	int driver)
{
	static char *displayDriverNames[] = {
		"NONE",
		"VGA",
		"Fullscreen DIBSection",
		"VESA VBE SuperVGA",
		"Unaccelerated SuperVGA",
		"Accelerated SuperVGA",
		"Unaccelerated DirectDraw",
		"Accelerated DirectDraw",
		"3D Accelerated DirectDraw",
		"Hardware OpenGL (Microsoft ICD/MCD)",
		"MGL hardware OpenGL mini-driver",
		"MGL hardware OpenGL driver",
		};

	if (grDETECT < driver && driver < grMAXDRIVER)
		return displayDriverNames[driver];
	else
		return "User graphics driver";
}

/****************************************************************************
DESCRIPTION:
Returns the name of the specified graphics mode.

HEADER:
mgraph.h

PARAMETERS:
mode	- graphics mode number to get name for

RETURNS:
Pointer to a string representing the name of the mode.

SEE ALSO:
MGL_driverName
****************************************************************************/
const char * MGLAPI MGL_modeName(
	int mode)
{
	static char *displayModeNames[] = {
		"320 x 200 VGA 16 color",
		"640 x 200 VGA 16 color",
		"640 x 350 VGA 16 color",
		"640 x 400 VGA 16 color",
		"640 x 480 VGA 16 color",
		"800 x 600 SVGA 16 color",
		"320 x 200 VGAX 256 color",
		"320 x 240 VGAX 256 color",
		"320 x 400 VGAX 256 color",
		"320 x 480 VGAX 256 color",
		"320 x 200 VGA 256 color",
		"320 x 200 SVGA 256 color",
		"320 x 240 SVGA 256 color",
		"320 x 400 SVGA 256 color",
		"320 x 480 SVGA 256 color",
		"400 x 300 SVGA 256 color",
		"512 x 384 SVGA 256 color",
		"640 x 350 SVGA 256 color",
		"640 x 400 SVGA 256 color",
		"640 x 480 SVGA 256 color",
		"800 x 600 SVGA 256 color",
		"1024 x 768 SVGA 256 color",
		"1152 x 864 SVGA 256 color",
		"1280 x 960 SVGA 256 color",
		"1280 x 1024 SVGA 256 color",
		"1600 x 1200 SVGA 256 color",
		"320 x 200 SVGA 32k color",
		"320 x 240 SVGA 32k color",
		"320 x 400 SVGA 32k color",
		"320 x 480 SVGA 32k color",
		"400 x 300 SVGA 32k color",
		"512 x 384 SVGA 32k color",
		"640 x 350 SVGA 32k color",
		"640 x 400 SVGA 32k color",
		"640 x 480 SVGA 32k color",
		"800 x 600 SVGA 32k color",
		"1024 x 768 SVGA 32k color",
		"1152 x 864 SVGA 32k color",
		"1280 x 960 SVGA 32k color",
		"1280 x 1024 SVGA 32k color",
		"1600 x 1200 SVGA 32k color",
		"320 x 200 SVGA 64k color",
		"320 x 240 SVGA 64k color",
		"320 x 400 SVGA 64k color",
		"320 x 480 SVGA 64k color",
		"400 x 300 SVGA 64k color",
		"512 x 384 SVGA 64k color",
		"640 x 350 SVGA 64k color",
		"640 x 400 SVGA 64k color",
		"640 x 480 SVGA 64k color",
		"800 x 600 SVGA 64k color",
		"1024 x 768 SVGA 64k color",
		"1152 x 864 SVGA 64k color",
		"1280 x 960 SVGA 64k color",
		"1280 x 1024 SVGA 64k color",
		"1600 x 1200 SVGA 64k color",
		"320 x 200 SVGA 16m color",
		"320 x 240 SVGA 16m color",
		"320 x 400 SVGA 16m color",
		"320 x 480 SVGA 16m color",
		"400 x 300 SVGA 16m color",
		"512 x 384 SVGA 16m color",
		"640 x 350 SVGA 16m color",
		"640 x 400 SVGA 16m color",
		"640 x 480 SVGA 16m color",
		"800 x 600 SVGA 16m color",
		"1024 x 768 SVGA 16m color",
		"1152 x 864 SVGA 16m color",
		"1280 x 960 SVGA 16m color",
		"1280 x 1024 SVGA 16m color",
		"1600 x 1200 SVGA 16m color",
		"320 x 200 SVGA 4G color",
		"320 x 240 SVGA 4G color",
		"320 x 400 SVGA 4G color",
		"320 x 480 SVGA 4G color",
		"400 x 300 SVGA 4G color",
		"512 x 384 SVGA 4G color",
		"640 x 350 SVGA 4G color",
		"640 x 400 SVGA 4G color",
		"640 x 480 SVGA 4G color",
		"800 x 600 SVGA 4G color",
		"1024 x 768 SVGA 4G color",
		"1152 x 864 SVGA 4G color",
		"1280 x 960 SVGA 4G color",
		"1280 x 1024 SVGA 4G color",
		"1600 x 1200 SVGA 4G color",
		};

	if (0 <= mode && mode < grMAXMODE)
		return displayModeNames[mode];
	else
		return "Invalid graphics mode";
}

/****************************************************************************
DESCRIPTION:
Get a the name of the device driver used for particular graphics mode.

HEADER:
mgraph.h

PARAMETERS:
mode	- graphics mode number

RETURNS:
Pointer to the name of device driver serving this mode

REMARKS:
This function returns the name of the device driver that is currently being used to
support the specified graphics mode. MGL provides a number of device drivers for
supporting the fullscreen graphics mode resolutions depending on the capabilities of
the underlying hardware. This function allows you to determine which driver is
currently being used to support each mode.

SEE ALSO:
MGL_driverName, MGL_modeName
****************************************************************************/
const char * MGLAPI MGL_modeDriverName(
	int mode)
{
	if (0 <= mode && mode < grMAXMODE) {
		int driver = _MGL_availableModes[mode].driver;
		if (driver != 0xFF && _MGL_driverTable[driver].driver)
			return _MGL_driverTable[driver].driver->realname;
		else
			return "Invalid graphics mode";
		}
	else
		return "Invalid graphics mode";
}

/****************************************************************************
DESCRIPTION:
Sets the size of the internal MGL buffer.


HEADER:
mgraph.h

PARAMETERS:
size	- New size of the internal MGL buffer

REMARKS:
This function sets the size of the internal MGL scratch buffer, which MGL uses for
local scratch space while rasterizing the primitives. The default size of this buffer is
6Kb for 16 bit code and 12Kb for 32 bit code, which is adequate for most needs. If
however you attempt to rasterize some primitives and MGL runs out of local
storage space you will need to increase the size of this internal buffer.

Note that this routine must be called before MGL_init or MGL_initWindowed is
called for the first time.

SEE ALSO:
MGL_init, MGL_initWindowed
****************************************************************************/
void MGLAPI MGL_setBufSize(
	unsigned size)
{
	_MGL_bufSize = size;
}

#ifdef	MGL_3D
/****************************************************************************
DESCRIPTION:
Enables shared Z-buffered hidden surface removal for a device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to enable Z-buffering for
dcShared	- Device context to share the Z-buffer with
zbits	- Requested Z-buffer depth

RETURNS:
True if Z-buffering enabled, false on error.

REMARKS:
This function attempts to start Z-buffered rasterizing mode for the device context,
but can share a pre-existing software Z-buffer with another device context. This is
used so that you can create a display device context as the front buffer, and a
memory device context as the back buffer that share the same software Z-buffer.
The two device contexts must be of exactly the same dimensions and Z-buffer
depth, otherwise this function will return an error code. The device context that
shares the Z-buffer memory will not destroy it, but the original device context will.
Hence if you destroy the original device context before the one sharing the Z-buffer,
the Z-buffer memory will become invalid for the device context that was sharing it!

If the device context to be shared with does not have a software Z-buffer allocated
for it (if the device has hardware Z-buffering for instance), then the Z-buffer will be
allocated using the normal MGL_zBegin function.

MGL supports software Z-buffering with Z-buffer depths of either 16, 24 or 32 bits
per z element. A 32 bit Z-buffer provides the maximum precision but requires twice
the memory of a 16 bit Z-buffer and hence rasterizing will be slightly slower. Note
that currently 24 bit Z-buffer depths are only intended for supporting hardware Z-
buffers that are 24 bits deep and will be extremely slow in software. In fact at the
moment MGL will usually draw nothing if you select a 24 bit deep software Z-
buffer.

SEE ALSO:
MGL_zBegin
****************************************************************************/
/* {secret} */
ibool MGLAPI MGL_zShareZBuffer(
	MGLDC *dc,
	MGLDC *dcShared,
	int zbits)
{
	if (dc->deviceType != MGL_MEMORY_DEVICE)
		MGL_fatalError("MGL_zBeginShared can only be called for MEMORY DC's!\n");

	if ((MGL_sizex(dc) != MGL_sizex(dcShared))
			|| MGL_sizey(dc) != MGL_sizey(dcShared))
		MGL_fatalError("MGL_zBeginShared called for DC's of different dimensions!\n");

	if (dcShared->zbuffer && zbits == dcShared->zbits) {
		dc->zbuffer = dcShared->zbuffer;
		dc->zbits = zbits;
		dc->zwidth = dcShared->zwidth;
		dc->zOffset = dcShared->zOffset;
		dc->flags |= MGL_SHARED_ZBUFFER;
		}
	else if (!MGL_zBegin(dc,zbits))
		return false;
	_MGL_updateCurrentDC(dc);
	return true;
}

/****************************************************************************
DESCRIPTION:
Sets the current RGB shade table.

HEADER:
mgraph.h

PARAMETERS:
shadeTab	- New shade table to use for rasterizing

REMARKS:
This function sets the current RGB shade table for fast color index shading of
primitives when in RGB modes. This shade table is effectively a 256 entry lookup
table that is used to compute the final RGB color values for an interpolated 8 bit
color index value. Using a shade table for RGB smooth shaded primitives provides
for faster rasterizing (because only a single color index is interpolated, rather than
interpolating all three RGB color channels), however you cannot properly blend
color values between vertices.
****************************************************************************/
/* {secret} */
void MGLAPI MGL_setShadeTable(
	color_t *shadeTab)
{
	DC.shadeTab = shadeTab;
}
#endif

/****************************************************************************
DESCRIPTION:
Determines if the specified device context is a display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to check

RETURNS:
True if the device context is a display DC, false if not.

REMARKS:
This function determines if the passed in device context is a fullscreen
display device context, or some other type of device context.

SEE ALSO:
MGL_isMemoryDC, MGL_isWindowedDC
****************************************************************************/
ibool MGLAPI MGL_isDisplayDC(
	MGLDC *dc)
{ return _MGL_isFullscreenDevice(dc); }

/****************************************************************************
DESCRIPTION:
Determines if the specified device context is a display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to check

RETURNS:
True if the device context is a windowed DC, false if not.

REMARKS:
This function determines if the passed in device context is a windowed
device context, or some other type of device context.

SEE ALSO:
MGL_isDisplayDC, MGL_isMemoryDC
****************************************************************************/
ibool MGLAPI MGL_isWindowedDC(
	MGLDC *dc)
{ return dc->deviceType == MGL_WINDOWED_DEVICE || dc->deviceType == MGL_FULLDIB_DEVICE; }

/****************************************************************************
DESCRIPTION:
Determines if the specified device context is a memory device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to check

RETURNS:
True if the device context is a memory DC, false if not.

REMARKS:
This function determines if the passed in device context is a memory
device context, or some other type of device context.

SEE ALSO:
MGL_isDisplayDC, MGL_isWindowedDC
****************************************************************************/
ibool MGLAPI MGL_isMemoryDC(
	MGLDC *dc)
{ return dc->deviceType == MGL_MEMORY_DEVICE; }

/*******************************************************************************
DESCRIPTION:
Sets the current palette snow level for a display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context to set snow level for
level	- New snow level to set

REMARKS:
This function sets the number of palette entries that can be programmed during a
single vertical retrace before the onset of snow. By default MGL programs all 256
entries per retrace, but you may need to slow this down on systems with slower
hardware that causes snow during multiple palette realization commands.

SEE ALSO:
MGL_getPaletteSnowLevel, MGL_setPalette
*******************************************************************************/
void MGLAPI MGL_setPaletteSnowLevel(
	MGLDC *dc,
	int level)
{
	if (_MGL_isFullscreenDevice(dc)) {
		if (level > 0 && level <= 256)
			dc->v->d.maxProgram = level;
		}
}

/*******************************************************************************
DESCRIPTION:
Returns the current palette snow level.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Current palette snow level.

REMARKS:
This function returns the number of palette entries that can be programmed during a
single vertical retrace before the onset of snow. By default MGL programs use all 256
entries per retrace, but you may need to slow this down on systems with slower
hardware that causes snow during multiple palette realization commands.

SEE ALSO:
MGL_setPaletteSnowLevel
*******************************************************************************/
int MGLAPI MGL_getPaletteSnowLevel(
	MGLDC *dc)
{ return _MGL_isFullscreenDevice(dc) ? dc->v->d.maxProgram : 0; }

/****************************************************************************
DESCRIPTION:
Return the direct surface access flags.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Direct surface access flags for the device context.

REMARKS:
This function returns the direct surface access flags for the specified device context,
which can be used to determine if the surface for the device context is directly
accessible, and if the surface has been virtualized in software. The access flags
returned are enumerated in MGL_surfaceAccessFlagsType.

If the surface access flags is MGL_VIRTUAL_ACCESS, this means that the
surface for the device can be directly accessed, however the surface is actually
virtualized in software using a page fault handler for SuperVGA devices that do not
have a real hardware linear framebuffer. If the surface is virtualized, you must
ensure that when you directly access the surface you do so on BYTE, WORD and
DWORD aligned boundaries. If you access it on a non-aligned boundary across a
page fault, you will cause an infinite page fault loop to occur. If the surface access
flags is MGL_NO_ACCESS the framebuffer will be banked and if you wish to
rasterize directly to it you will need to use the SVGA_setBank functions to change
banks. In banked modes the surface pointer points to the start of the banked
framebuffer window (ie: 0xA0000).
****************************************************************************/
int	MGLAPI MGL_surfaceAccessType(
	MGLDC *dc)
{ return (dc->flags & MGL_SURFACE_FLAGS); }

/****************************************************************************
DESCRIPTION:
Gets the Z-buffer surface direct access flags for a device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to get Z-buffer access flags for

RETURNS:
Direct surface access flag for the device context.

REMARKS:
This function returns the direct Z-buffer surface access flags for the specified
device context, which can be used to determine if the Z-buffer surface for the
device context is directly accessible, and if the surface has been virtualized in
software. The access flags returned are enumerated in MGL_surfaceAccessFlagsType.

If the surface access flag is MGL_VIRTUAL_ZACCESS, this means that the
surface for the device can be directly accessed, however the surface is actually
virtualized in software using a page fault handler for SuperVGA devices that do not
have real hardware linear Z-buffer access. If the surface is virtualized, you must
ensure that when you directly access the surface you do so on BYTE, WORD and
DWORD aligned boundaries. If you access it on a non-aligned boundary across a
page fault, you will cause an infinite page fault loop to occur.
****************************************************************************/
int	MGLAPI MGL_zbufferAccessType(
	MGLDC *dc)
{ return (dc->flags & MGL_ZBUFFER_FLAGS); }

/****************************************************************************
DESCRIPTION:
Returns the current hardware acceleration flags for the display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Current hardware acceleration flags for the device context.

REMARKS:
This function returns the current hardware acceleration flags for the display device
context (this function returns 0 for non-display device contexts). The set of
hardware acceleration flags inform the application of what specific hardware
acceleration features the underlying video hardware has, so that the application can
tailor it's use of specific MGL functions. For instance an application may check if
the hardware has transparent BitBlt capabilities for sprite animation, and if not will
uses it's own application specific set of routines that rasterize directly to the display
surface rather than using the MGL specific functions.


The set of hardware acceleration flags that are returned will be a logical
combination of one or more of the values enumerated in MGL_hardwareFlagsType.
****************************************************************************/
long MGLAPI MGL_getHardwareFlags(
	MGLDC *dc)
{ return (dc->flags & MGL_HW_FLAGS); }

/* {secret} */
void _MGL_computePF(MGLDC *dc)
/****************************************************************************
*
* Function:		_MGL_computePF
* Parameters:	dc	- Device context
*
* Description:	Computes the pixel format information from the mode info
*				block for the graphics mode.
*
****************************************************************************/
{
	dc->pf.redAdjust = 8 - dc->mi.redMaskSize;
	dc->pf.redMask = 0xFF >> dc->pf.redAdjust;
	dc->pf.redPos = dc->mi.redFieldPosition;
	dc->pf.greenAdjust = 8 - dc->mi.greenMaskSize;
	dc->pf.greenMask = 0xFF >> dc->pf.greenAdjust;
	dc->pf.greenPos = dc->mi.greenFieldPosition;
	dc->pf.blueAdjust = 8 - dc->mi.blueMaskSize;
	dc->pf.blueMask = 0xFF >> dc->pf.blueAdjust;
	dc->pf.bluePos = dc->mi.blueFieldPosition;
	if (dc->mi.rsvdMaskSize == 0) {
		dc->pf.rsvdAdjust = 0;
		dc->pf.rsvdMask = 0;
		dc->pf.rsvdPos = 0;
		}
	else {
		dc->pf.rsvdAdjust = 8 - dc->mi.rsvdMaskSize;
		dc->pf.rsvdMask = 0xFF >> dc->pf.rsvdAdjust;
		dc->pf.rsvdPos = dc->mi.rsvdFieldPosition;
		}
}

/****************************************************************************
DESCRIPTION:
Sets the internal VBE/AF accelerator functions driver used by the MGL.

HEADER:
mgraph.h

PARAMETERS:
driver	- Pointer to the VBE/AF driver for the MGL to use

REMARKS:
This function allows the application developer to load a VESA VBE/AF
Accelerator Functions driver from disk, and let the MGL know about it so
that it will use the pre-loaded version instead of attempting to load
the version itself. The main purpose of this function is to allow customers
using the SciTech UVBELib/Accel libraries to let the MGL know about the
driver generated by these device support libraries and use it instead of
the one on the end users system.
****************************************************************************/
void MGLAPI MGL_setACCELDriver(
	void *driver)
{ _MGL_wdDriver = driver; }

/****************************************************************************
DESCRIPTION:
Overrides the default file I/O functions used by MGL.

HEADER:
mgraph.h

PARAMETERS:
fio	- Structure containing new file I/O functions

REMARKS:
This function allows the programmer to override the default file I/O functions used
by all the MGL functions that access files (bitmap, font, icon and cursor loading).
By default the standard C I/O functions are used and you can reset back to the
standard C I/O functions by calling this function with the fio parameter set to
NULL.

This function is useful for creating your own file system, such as storing all the
bitmaps, fonts and icons that your application requires in a large file of your own
format. This way end users browsing your program's data files will not be able to
view any of the data (game developers may wish to keep the bitmaps used for levels
in the game secret to make it harder for the user to cheat when playing the game).

This function allows you to overload the fopen, fclose, fseek, ftell, fread and fwrite
functions used by MGL. See the fileio_t structure for more information.

****************************************************************************/
void MGLAPI MGL_setFileIO(
	fileio_t *fio)
{
	if (fio) {
		_MGL_fopen 	= fio->fopen;
		_MGL_fclose = fio->fclose;
		_MGL_fseek 	= fio->fseek;
		_MGL_ftell 	= fio->ftell;
		_MGL_fread 	= fio->fread;
		_MGL_fwrite = fio->fwrite;
		}
	else {
		_MGL_fopen 	= fopen;
		_MGL_fclose = fclose;
		_MGL_fseek 	= fseek;
		_MGL_ftell 	= ftell;
		_MGL_fread 	= fread;
		_MGL_fwrite = fwrite;
		}
}


/****************************************************************************
DESCRIPTION:
Opens a stream.

HEADER:
mgraph.h

PARAMETERS:
f	- Filename
mode - Mode to open file in.

RETURNS:
Pointer to newly opened stream, or NULL in the event of an error.

REMARKS:
This function is identical to the C library fopen function, but goes via
the MGL's internal file handling function pointers, which by default simply
points to the standard C library functions. These functions are intended
to allow the application programmer to override all the MGL file I/O
functions with MGL_setFileIO, for custom I/O handling.

SEE ALSO:
MGL_fclose, MGL_fseek, MGL_ftell, MGL_fread, MGL_fwrite, MGL_setFileIO
****************************************************************************/
FILE * MGLAPI MGL_fopen(
	const char *filename,
	const char *mode)
{ return _MGL_fopen(filename,mode); }

/****************************************************************************
DESCRIPTION:
Closes an open disk file.

HEADER:
mgraph.h

PARAMETERS:
f	- Pointer to file to close

RETURNS:
0 on success, EOF on an error.

REMARKS:
This function is identical to the C library fclose function, but goes via
MGL's internal file handling function pointers, which by default simply
point to the standard C library functions. These functions are intended
to allow the application programmer to override all the MGL file I/O
functions with MGL_setFileIO, for custom I/O handling.

SEE ALSO:
MGL_fopen, MGL_fseek, MGL_ftell, MGL_fread, MGL_fwrite, MGL_setFileIO
****************************************************************************/
int MGLAPI MGL_fclose(
	FILE *f)
{ return _MGL_fclose(f); }

/****************************************************************************
DESCRIPTION:
Repositions the file pointer on a stream.

HEADER:
mgraph.h

PARAMETERS:
f	- Stream of interest
Offset	- Offset of location from whence
whence	- New location of file pointer

RETURNS:
0 if move was successful, otherwise non-zero.

REMARKS:
This function is identical to the C library fseek function, but goes via
MGL's internal file handling function pointers, which by default simply
point to the standard C library functions. These functions are intended
to allow the application programmer to override all the MGL file I/O
functions with MGL_setFileIO, for custom I/O handling.

SEE ALSO:
MGL_fopen, MGL_fclose, MGL_ftell, MGL_fread, MGL_fwrite, MGL_setFileIO
****************************************************************************/
int MGLAPI MGL_fseek(
	FILE *f,
	long offset,
	int whence)
{ return _MGL_fseek(f,offset,whence); }


/****************************************************************************
DESCRIPTION:
Returns the current file pointer.

HEADER:
mgraph.h

PARAMETERS:
f	- Pointer to file of interest

RETURNS:
Current file pointer on success, -1L on error.

REMARKS:
This function is identical to the C library fopen function, but goes via
the MGL's internal file handling function pointers, which by default simply
points to the standard C library functions. These functions are intended
to allow the application programmer to override all the MGL file I/O
functions with MGL_setFileIO, for custom I/O handling.

SEE ALSO:
MGL_fopen, MGL_fclose, MGL_fseek, MGL_fread, MGL_fwrite, MGL_setFileIO
****************************************************************************/
long MGLAPI MGL_ftell(
	FILE *f)
{ return _MGL_ftell(f); }

/****************************************************************************
DESCRIPTION:
Reads data from a stream.

HEADER:
mgraph.h

PARAMETERS:
ptr	- Pointer to block in stream at which to begin read
size	- Size of items to be read from stream
n	- Number of items to be read from stream
f	- Stream to be read

RETURNS:
Number of items read in, or a short count (possibly 0).

REMARKS:
This function is identical to the C library fread function, but goes via
the MGL's internal file handling function pointers, which by default simply
points to the standard C library functions. These functions are intended
to allow the application programmer to override all the MGL file I/O
functions with MGL_setFileIO, for custom I/O handling.

SEE ALSO:
MGL_fopen, MGL_fclose, MGL_fseek, MGL_ftell, MGL_fwrite, MGL_setFileIO
****************************************************************************/
size_t MGLAPI MGL_fread(
	void *ptr,
	size_t size,
	size_t n,
	FILE *f)
{ return _MGL_fread(ptr,size,n,f); }

/****************************************************************************
DESCRIPTION:
Writes to a stream.

HEADER:
mgraph.h

PARAMETERS:
ptr	- Pointer to the starting location of data to be written
size	- Size of items to be written to file
n	- Number of items to be written to file
f	- Pointer to the file stream to write the data to

RETURNS:
The number of items written.

REMARKS:
This function is identical to the C library fopen function, but goes via
the MGL's internal file handling function pointers, which by default simply
points to the standard C library functions. These functions are intended
to allow the application programmer to override all the MGL file I/O
functions with MGL_setFileIO, for custom I/O handling.

SEE ALSO:
MGL_fopen, MGL_fclose, MGL_fseek, MGL_ftell, MGL_fread, MGL_setFileIO
****************************************************************************/
size_t MGLAPI MGL_fwrite(
	const void *ptr,
	size_t size,
	size_t n,
	FILE *f)
{ return _MGL_fwrite(ptr,size,n,f); }
