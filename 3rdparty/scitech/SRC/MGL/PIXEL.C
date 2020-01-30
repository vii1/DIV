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
* Description:	Pixel manipulation routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

#undef	MGL_pixelFast
#undef	MGL_pixel
#undef	MGL_getPixelFast
#undef	MGL_getPixel

/****************************************************************************
DESCRIPTION:
Draws a pixel at the specified location as fast as possible.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to plot pixel at
y	- y coordinate to plot pixel at

REMARKS:
Plots a single pixel at the specified location in the current foreground color. This
routine is designed to allow plotting of multiple pixels as fast as possible.

Note that you must call MGL_beginPixel before calling this function, and you must
call MGL_endPixel after you have finished plotting a number of pixels.

SEE ALSO:
MGL_pixelFast, MGL_beginPixel, MGL_endPixel.
****************************************************************************/
void MGLAPI MGL_pixelCoordFast(
	int x,
	int y)
{
	if (DC.a.clip && !MGL_ptInRectCoord(x,y,DC.a.clipRect))
		return;
#ifndef	MGL_LITE
	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		DC.r.ditherPixel(x + DC.a.viewPort.left,y + DC.a.viewPort.top);
	else
#endif
		DC.r.putPixel(x + DC.a.viewPort.left,y + DC.a.viewPort.top);
}

/****************************************************************************
DESCRIPTION:
Draws a pixel at the specified location as fast as possible.

HEADER:
mgraph.h

PARAMETERS:
p	- Point to plot pixel at

REMARKS:
This function is the same as MGL_pixelCoordFast, however it takes the coordinate
of the pixel to plot as a point.

SEE ALSO:
MGL_pixelCoordFast, MGL_beginPixel, MGL_endPixel.
****************************************************************************/
void MGL_pixelFast(
	point_t p);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Returns the color of a specified pixel.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate of the pixel to read
y	- y coordinate of the pixel to read

RETURNS:
Color of the specified pixel.

REMARKS:
This function returns the color of the pixel at the specified coordinate. Note that you
must ensure that you call the routine MGL_beginPixel before reading any pixel
values and the routine MGL_endPixel after reading a bunch of pixels with these fast
pixel routines.

SEE ALSO:
MGL_getPixelFast, MGL_beginPixel, MGL_endPixel
****************************************************************************/
color_t MGLAPI MGL_getPixelCoordFast(
	int x,
	int y)
{
	if (DC.a.clip && !MGL_ptInRectCoord(x,y,DC.a.clipRect))
		return (color_t)-1;
	return DC.r.getPixel(x + DC.a.viewPort.left,y + DC.a.viewPort.top);
}

/****************************************************************************
DESCRIPTION:
Returns the color of a specified pixel.

HEADER:
mgraph.h

PARAMETERS:
p	- Coordinate of the pixel to read

RETURNS:
Color of the specified pixel.

REMARKS:
This function is the same as MGL_getPixelCoordFast, however it takes the
coordinate of the pixel to read as a point.

SEE ALSO:
MGL_getPixelCoordFast, MGL_beginPixel, MGL_endPixel
****************************************************************************/
color_t MGL_getPixelFast(
	point_t p);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a pixel at the specified location.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to plot pixel at
y	- y coordinate to plot pixel at

REMARKS:
Plots a single pixel at the specified location in the current foreground color.

SEE ALSO:
MGL_pixel
****************************************************************************/
void MGLAPI MGL_pixelCoord(
	int x,
	int y)
{
	MGL_beginPixel();
	MGL_pixelCoordFast(x,y);
	MGL_endPixel();
}

/****************************************************************************
DESCRIPTION:
Draws a pixel at the specified location.

HEADER:
mgraph.h

PARAMETERS:
p	- Point to plot pixel at

REMARKS:
This function is the same as MGL_pixelFast, however it takes the
coordinate of the pixel to plot as a point.

SEE ALSO:
MGL_pixelCoord
****************************************************************************/
void MGL_pixel(
	point_t p);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Returns the color of a specified pixel.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate of the pixel to read 
y	- y coordinate of the pixel to read

RETURNS:
Color of the specified pixel.

REMARKS:
This function returns the color of the pixel at the specified coordinate.

SEE ALSO:
MGL_getPixel, MGL_beginPixel, MGL_endPixel
****************************************************************************/
color_t MGLAPI MGL_getPixelCoord(
	int x,
	int y)
{
	int v;
	MGL_beginPixel();
	v = MGL_getPixelCoordFast(x,y);
	MGL_endPixel();
	return v;
}

/****************************************************************************
DESCRIPTION:
Returns the color of a specified pixel.

HEADER:
mgraph.h

PARAMETERS:
p	- Coordinate of the pixel to read

RETURNS:
Color of the specified pixel.

REMARKS:
This function is the same as MGL_getPixelCoord, however it takes the
coordinate of the pixel to read as a point.

SEE ALSO:
MGL_getPixelCoord, MGL_beginPixel, MGL_endPixel
****************************************************************************/
color_t MGL_getPixel(
	point_t p);
/* Implemented as a macro */

/* {secret} */
void _ASMAPI _MGL_clipPixel(int x,int y)
/****************************************************************************
*
* Function:		clipPixel
* Parameters:	x,y	- Coordinate of pixel to draw
*
* Description:	Draws a pixel at coordinate (x,y) in the current foreground
*				color attribute, clipping it to the current clipping
*				rectangle.
*
****************************************************************************/
{
	if (!MGL_ptInRectCoord(x,y,DC.intClipRect))
		return;
	DC.r.putPixel(x,y);
}

#ifndef	MGL_LITE
/* {secret} */
void _ASMAPI _MGL_clipDitherPixel(int x,int y)
/****************************************************************************
*
* Function:		clipDitherPixel
* Parameters:	x,y	- Coordinate of pixel to draw
*
* Description:	Draws a pixel at coordinate (x,y) in the current foreground
*				color attribute, clipping it to the current clipping
*				rectangle.
*
****************************************************************************/
{
	if (!MGL_ptInRectCoord(x,y,DC.intClipRect))
		return;
	DC.r.ditherPixel(x,y);
}
#endif

