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
* Description:	C language stubs to pass control to the	device context
*				routines etc. We also have an internal set of macro or
*				inline versions of these for efficiency internally in the
*				MGL.
*
*
****************************************************************************/

#include "mgl.h"

/* Undefine the macros for internal functions */

#undef MGL_setColor
#undef MGL_setBackColor
#undef MGL_beginPixel
#undef MGL_endPixel
#undef MGL_getArcCoords
#undef MGL_scanRightForColor
#undef MGL_scanLeftForColor
#undef MGL_scanRightWhileColor
#undef MGL_scanLeftWhileColor

#if defined(NO_ASSEMBLER) || !defined(__INTEL__)

/****************************************************************************
DESCRIPTION:
Sets the current foreground color.


HEADER:
mgraph.h

PARAMETERS:
color	- New foreground color value

REMARKS:
Sets the current foreground color values. The foreground color value is used to draw
all primitives.

Note that the value passed to this routine is either a color index or a color value in
the correct packed pixel format for the current video mode. Use the
MGL_packColor routine to pack 24 bit RGB values for direct color video modes.

SEE ALSO:
MGL_getColor, MGL_setBackColor, MGL_getBackColor, MGL_packColor
****************************************************************************/
void ASMAPI MGL_setColor(
	color_t color)
{ DC.r.setColor(color); }

/****************************************************************************
Description:
Sets the currently active background color.

HEADER:
mgraph.h

PARAMETERS:
color	- New background color value

REMARKS:
Sets the current background color value. The background color value is used to
clear the display and viewport with the MGL_clearDevice and MGL_clearViewport
routines, and is also used for filling solid primitives in the
MGL_BITMAP_OPAQUE fill mode.

Note that the value passed to this routine is either a color index or a color value in
the correct packed pixel format for the device context. Use the MGL_packColor
routine to pack 24 bit RGB values for RGB device contexts.

SEE ALSO:
MGL_getBackColor, MGL_setColor, MGL_getColor, MGL_packColor
****************************************************************************/
void ASMAPI MGL_setBackColor(
	color_t color)
{ DC.r.setBackColor(color); }


/****************************************************************************
DESCRIPTION:
Enables direct framebuffer access.

HEADER:
mgraph.h

REMARKS:
Enables direct framebuffer access so that you can directly rasterize to the linear
framebuffer memory using your own custom routines. Note that calling this
function is absolutely necessary when using hardware acceleration, as this function
correctly arbitrates
between the hardware accelerator graphics engine and your direct framebuffer
rasterizing code.

SEE ALSO:
SV_endDirectAccess

****************************************************************************/
void ASMAPI MGL_beginDirectAccess(void)
{ DC.r.beginDirectAccess(); }


/****************************************************************************
DESCRIPTION:
Disables direct framebuffer access.

HEADER:
mgraph.h

REMARKS:
Disables direct framebuffer access so that you can use the accelerator functions to
draw to the framebuffer memory. Note that calling this function is absolutely
necessary when using hardware acceleration, as this function and the corresponding
MGL_beginDirectAccess correctly arbitrate between the hardware accelerator
graphics engine and your direct framebuffer writes.

SEE ALSO:
MGL_beginDirectAccess

****************************************************************************/
void ASMAPI MGL_endDirectAccess(void)
{ DC.r.endDirectAccess(); }

#ifndef	MGL_LITE
/****************************************************************************
DESCRIPTION:
Sets the current write mode operation.

HEADER:
mgraph.h

PARAMETERS:
mode	- New write mode operation to use

REMARKS:
Sets the currently active write mode. Write mode operations supported by the
SciTech MGL for all output primitives are enumerated in MGL_writeModeType.

SEE ALSO:
MGL_getWriteMode
****************************************************************************/
void ASMAPI MGL_setWriteMode(
	int mode)
{ DC.r.setWriteMode(mode); }

/****************************************************************************
DESCRIPTION:
Sets the current pen style.

HEADER:
mgraph.h

PARAMETERS:
style	- New pen style to use

REMARKS:
Returns the currently active pen style.  Pen styles supported by the SciTech MGL
are enumerated in MGL_penStyleType.

When filling in the MGL_BITMAP_ TRANSPARENT mode, the foreground color
is used to fill in all pixels in the bitmap pattern that are a 1. Where the pixels in the
bitmap pattern are a 0, the original background color is retained. In the
MGL_BITMAP_ OPAQUE mode, the background color is used to fill in the pixels
in the bitmap that are set to a 0. When filling in MGL_PIXMAP mode, the
foreground and background color values are not used, and the  pixel colors are
obtained directly from the pixmap pattern colors.

SEE ALSO:
MGL_getPenStyle, MGL_setPenBitmapPattern
****************************************************************************/
void ASMAPI MGL_setPenStyle(
	int style)
{ DC.r.setPenStyle(style); }

/****************************************************************************
DESCRIPTION:
Set the current line stipple pattern.

PARAMETERS:
stipple	- New 16 - bit stipple pattern to set.

HEADER:
mgraph.h

REMARKS:
Sets the current line stipple pattern. The line stipple pattern is used to determine
which pixels in the line get drawn depending on which bits in the pattern are set.
The stipple pattern is a 16-bit value, and everywhere that a bit is set to a 1 in the
pattern, a pixel will be drawn in the line. Everywhere that a bit is a 0, the pixel will
be skipped in the line. Note that bit 0 in the stipple pattern corresponds to pixel
0,16,32,... in the line, bit 1 is pixel 1,17,33 etc. To create a line that is drawn as a
'dot dot dash dash' you would use the following value:

	0011100111001001b or 0x39C9

Note that to enable stippled line mode you must call MGL_setLineStyle, with the
MGL_LINE_STIPPLE parameter. Also note that stippled lines can only be 1 pixel
wide, and the pen size will be ignored when drawing a stippled line.

SEE ALSO:
MGL_setLineStyle, MGL_setLineStippleCount, MGL_getLineStipple
****************************************************************************/
void ASMAPI MGL_setLineStipple(
	ushort stipple)
{ DC.r.setLineStipple(stipple); }

/****************************************************************************
DESCRIPTION:

Sets the current line stipple counter.

HEADER:
mgraph.h

PARAMETERS:
stippleCount	- New line stipple counter to use

REMARKS:
Sets the current line stipple counter to a specific value.  The line stipple counter is
used to count the number of pixels that have been drawn in the line, and is updated
after the line has been drawn. The purpose of this counter is to allow you to draw
connected lines using a stipple pattern and the stippling will be continuous across
the break in the lines. You can use this function to reset the stipple counter to a
known value before drawing lines to force the stipple pattern to start at a specific bit
position (usually resetting it to 0 before drawing a group of lines is sufficient).

Note that VBE/AF 1.0 accelerated devices do not support the line stipple counter, so
this counter is essentially reset to 0 every time that a line is drawn using the
hardware. VBE/AF 2.0 will rectify this problem in the future.

SEE ALSO:
MGL_setLineStyle, MGL_setLineStipple, MGL_getLineStippleCount

****************************************************************************/
void ASMAPI MGL_setLineStippleCount(
	uint stippleCount)
{ DC.r.setLineStippleCount(stippleCount); }

/****************************************************************************
DESCRIPTION:
Sets the currently active bitmap pattern.

HEADER:
mgraph.h

PARAMETERS:
pat	- New bitmap pattern to use

REMARKS:
This function sets the currently active bitmap pattern used when rasterizing
patterned primitive in the MGL_BITMAP_ TRANSPARENT and MGL_BITMAP_
OPQAUE pen styles. A bitmap pattern is defined as an 8 x 8 pixel monochrome
pattern stored as an array of 8 bytes.

When filling in the MGL_BITMAP_ TRANSPARENT mode, the foreground color
is used to fill in all pixels in the bitmap pattern that are a 1. Where the pixels in the
bitmap pattern are a 0, the original background color is retained. In the
MGL_BITMAP_ OPAQUE mode, the background color is used to fill in the pixels
in the bitmap that are set to a 0.

SEE ALSO:
MGL_getPenBitmapPattern, MGL_setPenPixmapPattern, MGL_setPenStyle
****************************************************************************/
void ASMAPI MGL_setPenBitmapPattern(
	const pattern_t *pat)
{ DC.r.setPenBitmapPattern(pat); }

/****************************************************************************
DESCRIPTION:
Sets the currently active pixmap pattern.

HEADER:
mgraph.h

PARAMETERS:
pat	- New pixmap pattern to use

REMARKS:
This function sets the currently active pixmap pattern used when rasterizing
patterned primitive in the MGL_PIXMAP pen style. A pixmap pattern is defined as
an 8 x 8 pixel color pattern stored as an 8 x 8 array of MGL color values. When
filling in MGL_PIXMAP mode, the foreground and background color values are
not used, and the  pixel colors are obtained directly from the pixmap pattern colors.

SEE ALSO:
MGL_getPenPixmapPattern, MGL_setPenBitmapPattern, MGL_setPenStyle
****************************************************************************/
void ASMAPI MGL_setPenPixmapPattern(
	const pixpattern_t *pat)
{ DC.r.setPenPixmapPattern(pat); }

/****************************************************************************
DESCRIPTION:
Returns the starting and ending arc coordinates.

HEADER:
mgraph.h

PARAMETERS:
coords	- Pointer to structure to store coordinates

REMARKS:
This function returns the center coordinate, and starting and ending points on the
ellipse that define the last elliptical arc that was rasterized. You can then use these
coordinates to draw a line from the center of the ellipse to the starting and ending
points to complete the outline of an elliptical wedge.

Note that you must call this routine immediately after calling the MGL_ellipseArc
family of routines.

SEE ALSO:
MGL_ellipseArc, MGL_fillEllipseArc
****************************************************************************/
void ASMAPI MGL_getArcCoords(
	arc_coords_t *coords)
{ DC.r.getArcCoords(coords); }

/****************************************************************************
DESCRIPTION:
Scans right in viewport for a specified color.

HEADER:
mgraph.h

PARAMETERS:
x	- Starting x coordinate to scan from
y	- Starting y coordinate to scan from
color	- Color value to scan for

RETURNS:
x coordinate of pixel if found, or maxx+1 if search hit right edge of viewport

REMARKS:
This function begins scanning in the viewport at the specified location for the
specified color. The search begins at the location (x,y) and searches right along the
scanline from this point and returns the x coordinate of the pixel if one is found, or
one more than the maximum x coordinate if the search went beyond the right edge
of device context.

No clipping or viewport mapping is performed by this routine, but can be performed
after calling this routine. This routine can be used as the basis of a high performance
floodfill operation. Have a look in the file FFILL.C in the EXAMPLES directory
which uses this routine to implement a fast floodfill operation.

SEE ALSO:
MGL_scanLeftForColor, MGL_scanLeftWhileColor, MGL_scanRightWhileColor
****************************************************************************/
int	ASMAPI MGL_scanRightForColor(
	int x,
	int y,
	color_t color)
{ return DC.r.scanRightForColor(x,y,color); }

/****************************************************************************
DESCRIPTION:
Scans left in viewport surface for a specified color.


HEADER:
mgraph.h

PARAMETERS:
x	- Starting x coordinate to scan from
y	- Starting y coordinate to scan from
color	- Color value to scan for

RETURNS:
x coordinate of pixel if found, or -1 if search hit left edge of viewport.

REMARKS:
This function begins scanning in the viewport at the specified location for the
specified color. The search begins at the location (x,y) and searches left along the
scanline from this point. If a pixel is found that matches the color, the x coordinate
of the pixel is returned. If the search went beyond the left edge of the viewport -1 is
returned.

No clipping or viewport mapping is performed by this routine, but can be performed
after calling this routine. This routine can be used as the basis of a high performance
floodfill operation. Have a look in the file FFILL.C in the EXAMPLES directory
which uses this routine to implement a fast floodfill operation.

SEE ALSO:
MGL_scanRightForColor, MGL_scanLeftWhileColor, MGL_scanRightWhileColor
****************************************************************************/
int	ASMAPI MGL_scanLeftForColor(
	int x,
	int y,
	color_t color)
{ return DC.r.scanLeftForColor(x,y,color); }

/****************************************************************************
DESCRIPTION:
Scans right in viewport for any color but the specified color.


HEADER:
mgraph.h

PARAMETERS:
x	- Starting x coordinate to scan from
y	- Starting y coordinate to scan from
color	- Color value to scan on

RETURNS:
x coordinate of pixel if found, maxx+1 if search hit right edge of viewport

REMARKS:
This function begins scanning in the viewport at the specified location and
continues to scan while the pixels are the same as the specified seed color. The
search begins at the location (x,y) and searches right along the scanline from this
point and returns the x coordinate of the first pixel found that is not of the specified
color, or one more than the maximum x coordinate if the search went beyond the
right edge of the viewport.

No clipping or viewport mapping is performed by this routine, but can be performed
after calling this routine. This routine can be used as the basis of a high performance
floodfill operation. Have a look in the file FFILL.C in the EXAMPLES directory
which uses this routine to implement a fast floodfill operation.

SEE ALSO:
MGL_scanLeftForColor, MGL_scanRightForColor, MGL_scanLeftWhileColor
****************************************************************************/
int ASMAPI MGL_scanRightWhileColor(
	int x,
	int y,
	color_t color)
{ return DC.r.scanRightWhileColor(x,y,color); }

/****************************************************************************
DESCRIPTION:
Scans left in viewport surface for any color but the specified color.

HEADER:
mgraph.h

PARAMETERS:
x	- Starting x coordinate to scan from
y	- Starting y coordinate to scan from
color	- Color value to scan on

RETURNS:
x coordinate of pixel if found, -1 if search hit left edge of viewport.

REMARKS:
This function begins scanning in the viewport at the specified location and
continues to scan while the pixels are the same as the specified seed color. The
search begins at the location (x,y) and searches left along the scanline from this
point and returns the x coordinate of the first pixel found that is not of the specified
color, or -1 if the search went beyond the left edge of the viewport.

No clipping or viewport mapping is performed by this routine, but can be performed
after calling this routine. This routine can be used as the basis of a high performance
floodfill operation. Have a look in the file FFILL.C in the EXAMPLES directory
which uses this routine to implement a fast floodfill operation.

SEE ALSO:
MGL_scanLeftForColor, MGL_scanRightForColor, MGL_scanRightWhileColor
****************************************************************************/
int	ASMAPI MGL_scanLeftWhileColor(
	int x,
	int y,
	color_t color)
{ return DC.r.scanLeftWhileColor(x,y,color); }

/****************************************************************************
DESCRIPTION:
Enables Z-buffered hidden surface removal for a device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to enable Z-buffering for
bits	- Requested Z-buffer depth (16, 24 or 32 bits)

RETURNS:
True if Z-buffering enabled, false on error.

REMARKS:
This function attempts to start Z-buffered rasterizing mode for the device context. If
the device does not support a hardware Z-buffer, MGL will attempt to allocate a
software Z-buffer for the device context with the specified Z-buffer depth. The size
of the Z-buffer allocated will be equal to the dimensions of the entire device
context, which can be extremely large for very high resolution displays using a
software Z-buffer. If the Z-buffer memory cannot be allocated for the device
context, this routine will return false and will set the MGL_result error code.

MGL supports software Z-buffering with Z-buffer depths of either 16, 24 or 32 bits
per z element. A 32 bit Z-buffer provides the maximum precision but requires twice
the memory of a 16 bit Z-buffer and hence rasterizing will be slightly slower. Note
that currently 24 bit Z-buffer depths are only intended for supporting hardware Z-
buffers that are 24 bits deep and will be extremely slow in software. In fact at the
moment MGL will usually draw nothing if you select a 24 bit deep software Z-
buffer.

SEE ALSO:
MGL_zShareZBuffer
****************************************************************************/
/* {secret} */
ibool ASMAPI MGL_zBegin(
	MGLDC *dc,
	int zbits)
{ return DC.r.zBegin(dc,zbits); }

/****************************************************************************
DESCRIPTION:
Fills a specified scanline.

HEADER:
mgraph.h

PARAMETERS:
y	- y coordinate of scanline to fill
x1	- Starting x coordinate of scanline to fill
x2	- Ending x coordinate of scanline to fill

REMARKS:
MGL_scanLine fills the specified portion of a scanline in the current attributes and
fill pattern. This can be used to implement higher level complex fills, such as region
fills, floodfills etc.

SEE ALSO:
MGL_penStyleType, MGL_setPenBitmapPattern, MGL_setPenPixmapPattern
****************************************************************************/
void ASMAPI MGL_scanLine(
	int y,
	int x1,
	int x2)
{ DC.r.cur.scanLine(y,x1,x2); }
#endif	/* !MGL_LITE */

#endif

/****************************************************************************
DESCRIPTION:
Setup for high speed pixel drawing.

HEADER:
mgraph.h

REMARKS:
Sets up the video hardware for plotting single pixels as fast a possible. You must
call this routine before calling any of the MGL_pixel and
MGL_getPixel routines to ensure correct operation, and you
must call the MGL_endPixel routine after you have finished.

This routine is intended primarily to ensure fast operation if you need to plot more
than a single pixel at a time.

SEE ALSO:
MGL_endPixel, MGL_pixel, MGL_getPixel.
****************************************************************************/
void ASMAPI MGL_beginPixel(void)
{ DC.r.beginPixel(&DC); }

/****************************************************************************
DESCRIPTION:
Ends high speed pixel drawing operation.

HEADER:
mgraph.h

REMARKS:
This function ends a set of high speed pixel drawing operations, started with a call
to MGL_beginPixel. This routine is intended primarily to ensure fast operation if
you intend to plot more than a single pixel at a time.

SEE ALSO:
MGL_beginPixel

****************************************************************************/
void ASMAPI MGL_endPixel(void)
{ DC.r.endPixel(&DC); }

