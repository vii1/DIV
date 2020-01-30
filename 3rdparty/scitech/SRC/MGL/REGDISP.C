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
* Description:	Routines to register all the display device drivers. This
*				routine is separate so that the unused display drivers
*				will not be linked in.
*
*
****************************************************************************/

#define	BUILD_MGLDLL
#include "mgraph.h"

/*--------------------------- Global Variables ----------------------------*/

/****************************************************************************
DESCRIPTION:
Registers all available display drivers to be linked in and used for detection.

HEADER:
mgraph.h

PARAMETERS:
useWinDirect		- True if WinDirect drivers should be registered
useDirectDraw		- True if DirectDraw drivers should be registered
useVGA				- True if VGA drivers should be registered
useVGAX				- True if VGAX drivers should be registered
useVBE				- True if VBE drivers should be registered
useLinear			- True if linear drivers should be registered
useVBEAF			- True if VBEAF drivers should be registered
useFullscreenDIB	- True if fullscreen DIB drivers should be registered

REMARKS:
This function registers all available display drivers to be linked in, and also
registers those drivers in the device detection chain. If you pass a value of
true for useDirectDraw, all the DirectDraw drivers will be registered for
Windows (this parameter is ignored under DOS). If you pass a value of true
for useWinDirect, all the WinDirect drivers will be registered. These
parameters are useful for allowing the user to interactively force the use of
DirectDraw or WinDirect dynamically from within your application (see the
sample programs which all provide this support under Windows and the
MGL_detectGraph function for more information).

The useVGA, useVGAX, useVBE, useLinear and useVBEAF parameters all control
the use of WinDirect device drivers, so if you pass a value of false to
useWinDirect, it is the same as passing a value of false in all of the above
parameters.

This is a useful function to get code going quickly, but for release code you
should only link in those display drivers for the video modes that your
application will be using to reduce the size of the resulting executable.

SEE ALSO:
MGL_registerAllDispDrivers, MGL_registerDriver, MGL_detectGraph
****************************************************************************/
void MGLAPI MGL_registerAllDispDriversExt(
	ibool useWinDirect,
	ibool useDirectDraw,
	ibool useVGA,
	ibool useVGAX,
	ibool useVBE,
	ibool useLinear,
	ibool useVBEAF,
	ibool useFullscreenDIB)
{
	if (useWinDirect) {
		if (useVGA) {
			MGL_registerDriver(MGL_VGA4NAME,VGA4_driver);
			MGL_registerDriver(MGL_VGA8NAME,VGA8_driver);
			if (useVGAX)
				MGL_registerDriver(MGL_VGAXNAME,VGAX_driver);
			}
		if (useVBE) {
#ifndef	__WINDOWS__
			MGL_registerDriver(MGL_SVGA4NAME,SVGA4_driver);
			MGL_registerDriver(MGL_SVGA8NAME,SVGA8_driver);
			MGL_registerDriver(MGL_SVGA16NAME,SVGA16_driver);
			MGL_registerDriver(MGL_SVGA24NAME,SVGA24_driver);
			MGL_registerDriver(MGL_SVGA32NAME,SVGA32_driver);
#endif
			if (useLinear) {
				MGL_registerDriver(MGL_LINEAR8NAME,LINEAR8_driver);
				MGL_registerDriver(MGL_LINEAR16NAME,LINEAR16_driver);
				MGL_registerDriver(MGL_LINEAR24NAME,LINEAR24_driver);
				MGL_registerDriver(MGL_LINEAR32NAME,LINEAR32_driver);
				if (useVBEAF) {
					MGL_registerDriver(MGL_ACCEL8NAME,ACCEL8_driver);
					MGL_registerDriver(MGL_ACCEL16NAME,ACCEL16_driver);
					MGL_registerDriver(MGL_ACCEL24NAME,ACCEL24_driver);
					MGL_registerDriver(MGL_ACCEL32NAME,ACCEL32_driver);
					}
				}
			}
		}
#if defined(__WINDOWS__)
	if (useFullscreenDIB) {
		MGL_registerDriver(MGL_FULLDIB8NAME,FULLDIB8_driver);
		MGL_registerDriver(MGL_FULLDIB16NAME,FULLDIB16_driver);
		MGL_registerDriver(MGL_FULLDIB24NAME,FULLDIB24_driver);
		MGL_registerDriver(MGL_FULLDIB32NAME,FULLDIB32_driver);
        }
	if (useDirectDraw) {
		MGL_registerDriver(MGL_DDRAW8NAME,DDRAW8_driver);
		MGL_registerDriver(MGL_DDRAW16NAME,DDRAW16_driver);
		MGL_registerDriver(MGL_DDRAW24NAME,DDRAW24_driver);
		MGL_registerDriver(MGL_DDRAW32NAME,DDRAW32_driver);
	   }
#endif
}

/****************************************************************************
DESCRIPTION:
Registers all available display drivers to be linked in and used for detection.

HEADER:
mgraph.h

PARAMETERS:
useLinear			- True if linear drivers should be registered
useWinDirect		- True if WinDirect drivers should be registered
useDirectDraw		- True if DirectDraw drivers should be registered

REMARKS:
This function is a backwards compatibility function for older versions of
the library, and simply calls MGL_registerAllDispDriversExt to achieve the
same functionality. Please see the new function for more information.

SEE ALSO:
MGL_registerAllDispDriversExt
****************************************************************************/
void MGLAPI MGL_registerAllDispDrivers(
	ibool useLinear,
	ibool useDirectDraw,
	ibool useWinDirect)
{
	MGL_registerAllDispDriversExt(useWinDirect,useDirectDraw,true,true,true,
		useLinear,true,true);
}

