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
* Description:	Device context manipulation routines. Many of these are
*				implemented as macros in MGRAPH.H for speed.
*
*
****************************************************************************/

#include "mgl.h"				/* static declarations					*/

/*--------------------------- Global Variables ----------------------------*/

/* The standard bitmap patterns */

#ifndef	MGL_LITE
pattern_t	_VARAPI _MGL_empty_pat	= {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
pattern_t	_VARAPI _MGL_gray_pat 	= {0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
pattern_t	_VARAPI _MGL_solid_pat	= {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
ibool		_VARAPI _MGL_checkIdentityPal = true;
#endif

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
DESCRIPTION:
Returns the pixel depth for the device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Pointer to device context of interest

RETURNS:
Pixel depth for the device context.

SEE ALSO:
MGL_getPixelFormat
****************************************************************************/
int	MGLAPI MGL_getBitsPerPixel(
	MGLDC *dc)
{ return dc->mi.bitsPerPixel; }

/****************************************************************************
DESCRIPTION:
Returns the maximum available color value.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to check

RETURNS:
Maximum color value for current video mode.

REMARKS:
Returns the value of the largest available color value for the current video mode.
This value will always be one less than the number of available colors in that
particular video mode.
****************************************************************************/
color_t	MGLAPI MGL_maxColor(
	MGLDC *dc)
{ return dc->mi.maxColor; }

/****************************************************************************
DESCRIPTION:
Returns the maximum available hardware video page index.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to check.

RETURNS:
Index of maximum available hardware video page.

REMARKS:
Returns the index of the highest hardware video page that is available. This value
will always be one less than the number of hardware video pages available. Some
video modes only have one hardware video page available, so this value will be 0.
****************************************************************************/
int	MGLAPI MGL_maxPage(
	MGLDC *dc)
{ return dc->mi.maxPage; }

/****************************************************************************
DESCRIPTION:
Returns the current foreground color.

HEADER:
mgraph.h

RETURNS:
Current foreground color.

SEE ALSO:
MGL_setColor, MGL_getBackColor, MGL_setBackColor
****************************************************************************/
color_t MGLAPI MGL_getColor(void)
{ return DC.a.color; }

/****************************************************************************
DESCRIPTION:
Returns the current background color value.

HEADER:
mgraph.h

RETURNS:
Current background color value.

REMARKS:
Returns the current background color value. The background color value is used to
clear the display and viewport with the MGL_clearDevice and MGL_clearViewport
routines, and is also used for filling solid primitives in the
MGL_BITMAP_OPAQUE fill mode.

SEE ALSO:
MGL_setBackColor, MGL_getColor, MGL_setColor

****************************************************************************/
color_t MGLAPI MGL_getBackColor(void)
{ return DC.a.backColor; }

/****************************************************************************

MGL_setAspectRatio
<<BMP>>

DESCRIPTION:

Sets the current video mode's aspect ratio.


HEADER:
mgraph.h

PARAMETERS:
aspectRatio	- New value for the aspect ratio

REMARKS:
This function sets the aspect ratio for the device context to a new value. This ratio is
equal to:

	pixel x size
	------------  * 1000
	pixel y size

The device context aspect ratio can be used to display circles and squares on the
device by approximating them with ellipses and rectangles of the appropriate
dimensions. Thus in order to determine the number of pixels in the y direction for a
square with 100 pixels in the x direction, we can simply use the code:

	y_pixels = ((long)x_pixels * 1000) / aspectratio

Note the cast to a long to avoid arithmetic overflow, as the aspect ratio is returned
as an integer value with 1000 being a 1:1 aspect ratio.

SEE ALSO:
MGL_getAspectRatio
****************************************************************************/
void MGLAPI MGL_setAspectRatio(
	int aspectRatio)
{ DC.mi.aspectRatio = aspectRatio; }

/****************************************************************************
DESCRIPTION:
Returns the current device context aspect ratio.

HEADER:
mgraph.h

REMARKS:
This function returns the aspect ratio of the currently active output device's physical
pixels. This ratio is equal to:

	pixel x size
	------------  x 1000
	pixel y size

The device context aspect ratio can be used to display circles and squares on the
device by approximating them with ellipses and rectangles of the appropriate
dimensions. Thus in order to determine the number of pixels in the y direction for a
square with 100 pixels in the x direction, we can simply use the code:

	y_pixels = ((long)x_pixels * 1000) / aspectratio

Note the cast to a long to avoid arithmetic overflow, as the aspect ratio is returned
as an integer value with 1000 being a 1:1 aspect ratio.

RETURNS:
Current video mode aspect ratio * 1000.

SEE ALSO:
MGL_setAspectRatio

****************************************************************************/
int MGLAPI MGL_getAspectRatio(void)
{ return DC.mi.aspectRatio; }

/****************************************************************************
DESCRIPTION:
Returns the value for current default color (always white but value may vary).

HEADER:
mgraph.h

RETURNS:
Default color value for current video mode (always white).

REMARKS:
Returns the default color value for the current video mode. This color value is
white if the palette has not been changed, and will always be white in direct
color modes. However, the numerical value for white will vary depending on the
color depth.

SEE ALSO:
MGL_setColor, MGL_getColor
****************************************************************************/
color_t	MGLAPI MGL_defaultColor(void)
{ return MGL_realColor(&DC, MGL_WHITE); }

#ifndef	MGL_LITE
/****************************************************************************
DESCRIPTION:
Sets the current color map mode.

HEADER:
mgraph.h

PARAMETERS:
mode	- New color map mode to set

REMARKS:
This function sets the current color map mode for the device context.  Supported
modes are enumerated in MGL_colorModes.

The current color map mode only affects 8 bit display modes.

SEE ALSO:
MGL_getColorMapMode
****************************************************************************/
void MGLAPI MGL_setColorMapMode(
	int mode)
{
	DC.a.colorMode = mode;
	_MGL_setRenderingVectors();
}
/****************************************************************************
DESCRIPTION:
Returns the current color map mode.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Current color map mode.

REMARKS:
This function returns the current color map mode for the device context.  Defined
modes are enumerated in MGL_colorModes.

The current color map mode only affects 8 bit display modes.

SEE ALSO:
MGL_setColorMapMode
****************************************************************************/
int	MGLAPI MGL_getColorMapMode(void)
{ return DC.a.colorMode; }

/****************************************************************************
DESCRIPTION:
Sets the current marker size value.

HEADER:
mgraph.h

PARAMETERS:
size	- New marker size

REMARKS:
Sets the current marker size. The marker size is used to determine how big to draw
the markers with the MGL_marker routine. The size is defined as dimension from
the middle of the marker to the edges, so the actual dimensions of the marker will
be approximately twice the maker size. A marker size of 1 will define a marker that
is contained within a rectangle 3 pixels wide.

SEE ALSO:
MGL_getMarkerSize, MGL_marker, MGL_polyMarker
****************************************************************************/
void MGLAPI MGL_setMarkerSize(
	int size)
{ DC.a.markerSize = size; }

/****************************************************************************
DESCRIPTION:
Returns the current marker size value.

HEADER:
mgraph.h

RETURNS:
Current marker size

REMARKS:
Returns the current marker size. The marker size is used to determine how big to
draw the markers that are drawn with the MGL_marker routine. The size is defined
as the dimension from the middle of the marker to the edges, so the actual
dimensions of the marker will be twice the maker size plus 1. A marker size of 1
will define a marker that is contained within a rectangle 3 pixels wide.

SEE ALSO:
MGL_setMarkerSize, MGL_setMarkerStyle, MGL_getMarkerStyle, MGL_marker,
MGL_polyMarker
****************************************************************************/
int MGLAPI MGL_getMarkerSize(void)
{ return DC.a.markerSize; }

/****************************************************************************
DESCRIPTION:
Sets the current marker style.

HEADER:
mgraph.h

PARAMETERS:
style	- New marker style value       

REMARKS:
Sets the current marker style value. The marker style defines the type of marker to
be rasterized. MGL Marker styles are defined in MGL_markerStyleType.

SEE ALSO:
MGL_getMarkerStyle, MGL_marker, MGL_polyMarker
****************************************************************************/
void MGLAPI MGL_setMarkerStyle(
	int style)
{ DC.a.markerStyle = style; }

/****************************************************************************
DESCRIPTION:
Returns the current marker style.

HEADER:
mgraph.h

RETURNS:
Current marker style value.

REMARKS:
Returns the current marker style value. The marker style defines the type of marker
to be rasterized. Marker styles defined in the SciTech MGL are enumerated in
MGL_markerStyleType.

SEE ALSO:
MGL_setMarkerSize, MGL_getMarkerSize, MGL_setMarkerStyle, MGL_marker,
MGL_polyMarker
****************************************************************************/
int MGLAPI MGL_getMarkerStyle(void)
{ return DC.a.markerStyle; }

/****************************************************************************
DESCRIPTION:
Sets the current marker color value.

HEADER:
mgraph.h

PARAMETERS:
color	- New marker color to set

REMARKS:
Sets the current marker color value. The marker color is used when drawing
markers with the MGL_marker routine.

Note that the value passed to this routine is either a color index or a color value in
the correct packed pixel format for the current video mode. Use the
MGL_packColor routine to pack 24 bit RGB values for direct color video modes.

SEE ALSO:
MGL_getMarkerColor, MGL_marker, MGL_polyMarker, MGL_packColor
****************************************************************************/
void MGLAPI MGL_setMarkerColor(
	color_t color)
{ DC.a.markerColor = color; }

/****************************************************************************
DESCRIPTION:
Returns the current marker color value.

HEADER:
mgraph.h

RETURNS:
Current marker color value.

REMARKS:
Returns the current marker color value. The marker color is used when drawing
markers with the MGL_marker routine.

SEE ALSO:
MGL_setMarkerColor, MGL_marker, MGL_polyMarker
****************************************************************************/
color_t	MGLAPI MGL_getMarkerColor(void)
{ return DC.a.markerColor; }

/****************************************************************************
DESCRIPTION:
Sets the border colors for the current device context.

HEADER:
mgraph.h

PARAMETERS:
bright	- Color for the bright component of the border color
dark	- Color for the dark component of the border color

REMARKS:
The border colors are the two colors used to draw horizontal, vertical
and rectangular borders with the MGL_drawBorder function.
****************************************************************************/
void MGLAPI MGL_setBorderColors(
	color_t bright,
	color_t dark)
{
	DC.a.bdrBright = bright;
	DC.a.bdrDark = dark;
}

/****************************************************************************
DESCRIPTION:
Returns the border colors of the current device context.

HEADER:
mgraph.h

PARAMETERS:
bright	- Place to store the value for the bright component of the border color
dark	- Place to store the value for the dark component of the border color

REMARKS:
The border colors are the two colors used to draw horizontal, vertical
and rectangular borders with the MGL_drawBorder function.
****************************************************************************/
void MGLAPI MGL_getBorderColors(
	color_t *bright,
	color_t *dark)
{
	*bright = DC.a.bdrBright;
	*dark = DC.a.bdrDark;
}

/****************************************************************************
DESCRIPTION:
Returns the current write mode operation.

HEADER:
mgraph.h

RETURNS:
Current write mode operation.

REMARKS:
Returns the currently active write mode. Write modes supported by the SciTech MGL
for all output primitives are enumerated in MGL_writeModeType.

SEE ALSO:
MGL_setWriteMode

****************************************************************************/
int MGLAPI MGL_getWriteMode(void)
{ return DC.a.writeMode; }

/****************************************************************************
DESCRIPTION:
Returns the current pen style.

HEADER:
mgraph.h

RETURNS:
Current pen style.

REMARKS:
This function returns the currently active pen style.  Pen styles supported by
the SciTech MGL are enumerated in MGL_penStyleType.

When filling in the MGL_BITMAP_ TRANSPARENT mode, the foreground color
is used to fill in all pixels in the bitmap pattern that are a 1. Where the pixels in the
bitmap pattern are a 0, the original background color is retained. In the
MGL_BITMAP_ OPAQUE mode, the background color is used to fill in the pixels
in the bitmap that are set to a 0. When filling in MGL_PIXMAP mode, the
foreground and background color values are not used, and the  pixel colors are
obtained directly from the pixmap pattern colors.

SEE ALSO:
MGL_setPenStyle, MGL_setPenBitmapPattern, MGL_setPenPixmapPattern
****************************************************************************/
int MGLAPI MGL_getPenStyle(void)
{ return DC.a.penStyle; }

/****************************************************************************
DESCRIPTION:
Sets the current line style.

HEADER:
mgraph.h

PARAMETERS:
style	- New line style to use

REMARKS:
Sets the current line style.  MGL supports two different line styles, either pen style
patterned lines (MGL_LINE_PENSTYLE) or stippled lines
(MGL_LINE_STIPPLE). Pen style patterned lines are similar to those provided by
QuickDraw for the Macintosh where lines are drawing using a rectangular pen that
can have an arbitrary size and can be filled with an arbitrary pattern. Pen style
patterned lines are the default. Stippled lines are similar to those used by CAD
programs on the PC, and are 1-pixel wide lines that can be drawn using a 16-bit
stipple mask. Stippled lines can be drawn very fast in hardware using the VBE/AF
accelerator drivers.

In stippled line mode the line stipple pattern is used to determine which pixels in the
line get drawn depending on which bits in the pattern are set. The stipple pattern is a
16-bit value, and everywhere that a bit is set to a 1 in the pattern, a pixel will be
drawn in the line. Everywhere that a bit is a 0, the pixel will be skipped in the line.
Note that bit 0 in the stipple pattern corresponds to pixel 0,16,32,... in the line, bit 1
is pixel 1,17,33 etc. To create a line that is drawn as a 'dot dot dash dash' you
would use the following value:

	0011100111001001b or 0x39C9


In stippled line mode the line stipple counter is used to count the number of pixels
that have been drawn in the line, and is updated after the line has been drawn. The
purpose of this counter is to allow you to draw connected lines using a stipple
pattern and the stippling will be continuous across the break in the lines. You can
use this function to reset the stipple counter to a known value before drawing lines
to force the stipple pattern to start at a specific bit position (usually resetting it to 0
before drawing a group of lines is sufficient).

Note that VBE/AF 1.0 accelerated devices do not support the line stipple counter, so
this counter is essentially reset to 0 every time that a line is drawn using the
hardware. VBE/AF 2.0 will rectify this problem in the future.

SEE ALSO:
MGL_setLineStipple, MGL_setLineStippleCount, MGL_getLineStyle
****************************************************************************/
void MGLAPI MGL_setLineStyle(
	int style)
{ DC.a.lineStyle = style; }

/****************************************************************************
DESCRIPTION:
Returns the current line style.

HEADER:
mgraph.h

REMARKS:
Returns the current line style. Refer to MGL_setLineStipple for more information
on stippled lines.

SEE ALSO:
MGL_setLineStyle, MGL_setLineStipple, MGL_setLineStippleCount
****************************************************************************/
int MGLAPI MGL_getLineStyle(void)
{ return DC.a.lineStyle; }

/****************************************************************************
DESCRIPTION:
Returns the current line stipple pattern.

HEADER:
mgraph.h

REMARKS:
Return the current line stipple pattern. Refer to MGL_setLineStipple for more
information on stippled lines.

SEE ALSO:
MGL_setLineStyle, MGL_setLineStipple, MGL_setLineStippleCount
****************************************************************************/
ushort MGLAPI MGL_getLineStipple(void)
{ return DC.a.lineStipple; }

/****************************************************************************
DESCRIPTION:
Returns the current line stipple counter.

HEADER:
mgraph.h

REMARKS:
Return the current line stipple counter. Refer to MGL_setLineStipple for more
information on stippled lines.

SEE ALSO:
MGL_setLineStyle, MGL_setLineStipple, MGL_setLineStippleCount
****************************************************************************/
uint MGLAPI MGL_getLineStippleCount(void)
{ return DC.a.stippleCount; }

/****************************************************************************
DESCRIPTION:
Sets the current polygon type.

HEADER:
mgraph.h

PARAMETERS:
type	- New polygon type

REMARKS:
Sets the current polygon type. You can change this value to force MGL to work
with a specific polygon type (and to avoid the default automatic polygon type
checking). Polygon types supported by the SciTech MGL are enumerated in MGL_polygonType.

If you expect to be drawing lots of complex or convex polygons, setting the polygon
type can result in faster polygon rasterizing. Note that this setting does not affect the
specialized triangle and quadrilateral rasterizing routines.

SEE ALSO:
MGL_getPolygonType, MGL_fillPolygon
****************************************************************************/
void MGLAPI MGL_setPolygonType(
	int type)
{ DC.a.polyType = type; }

/****************************************************************************
DESCRIPTION:
Returns the current polygon type.

HEADER:
mgraph.h

RETURNS:
Current polygon type code.

REMARKS:
Returns the current polygon type. You can change this value with the
MGL_setPolygonType to force MGL to work with a specific polygon type (and to
avoid the default automatic polygon type checking). Polygon types supported by
the SciTech MGL are enumerated in MGL_polygonType.

If you expect to be drawing lots of complex or convex polygons, setting the polygon
type can result in faster polygon rasterizing. Note that this setting does not affect the
specialized triangle and quadrilateral rasterizing routines.

SEE ALSO:
MGL_setPolygonType, MGL_fillPolygon
****************************************************************************/
int	MGLAPI MGL_getPolygonType(void)
{ return DC.a.polyType; }

/****************************************************************************
DESCRIPTION:

Sets the current pen size.


HEADER:
mgraph.h

PARAMETERS:
h	- Height of the pen in pixels
w	- Width of the pen in pixels

REMARKS:
Sets the size of the current pen size in pixels. The default pen is 1 pixel by 1 pixel in
dimensions, however you can change this to whatever value you like. When
primitives are rasterized with a pen other than the default, the pixels in the pen
always lie to the right and below the current pen position.

SEE ALSO:
MGL_getPenSize

****************************************************************************/
void MGLAPI MGL_setPenSize(
	int h,
	int w)
{
	DC.a.penHeight = h-1;
	DC.a.penWidth = w-1;
	_MGL_setRenderingVectors();
}

/****************************************************************************
DESCRIPTION:
Returns the current pen size.

HEADER:
mgraph.h

PARAMETERS:
height	- Place to store the current pen height
width	- Place to store the current pen width

REMARKS:
Return the size of the current pen in pixels. The default pen is 1 pixel by 1 pixel in 
dimensions, however you can change this to whatever value you like. When
primitives are rasterized with a pen other than the default, the pixels in the pen
always lie to the right and below the current pen position.

SEE ALSO:
MGL_setPenSize

****************************************************************************/
void MGLAPI MGL_getPenSize(
	int *height,
	int *width)
{
	*height = DC.a.penHeight+1;
	*width = DC.a.penWidth+1;
}

/****************************************************************************
DESCRIPTION:
Returns the currently active bitmap pattern.

HEADER:
mgraph.h

PARAMETERS:
pat	- Place to store the bitmap pattern

REMARKS:
This function copies the currently active bitmap pattern used when rasterizing
patterned primitive in the MGL_BITMAP_ TRANSPARENT and MGL_BITMAP_
OPQAUE pen styles. A bitmap pattern is defined as an 8 x 8 pixel monochrome
pattern stored as an array of 8 bytes.

When filling in the MGL_BITMAP_ TRANSPARENT mode, the foreground color
is used to fill in all pixels in the bitmap pattern that are a 1. Where the pixels in the
bitmap pattern are a 0, the original background color is retained. In the
MGL_BITMAP_ OPAQUE mode, the background color is used to fill in the pixels
in the bitmap that are set to a 0.

SEE ALSO:
MGL_setPenBitmapPattern, MGL_setPenPixmapPattern, MGL_setPenStyle, MGL_getPenStyle
****************************************************************************/
void MGLAPI MGL_getPenBitmapPattern(
	pattern_t *pat)
{ memcpy(pat,&(DC.a.penPat),sizeof(pattern_t)); }

/****************************************************************************
DESCRIPTION:
Returns the currently active pixmap pattern.

HEADER:
mgraph.h

PARAMETERS:
pat	- Place to store the pixmap pattern

REMARKS:
This function copies the currently active pixmap pattern used when rasterizing
patterned primitive in the MGL_PIXMAP pen style. A pixmap pattern is defined as
an 8 x 8 pixel color pattern stored as an 8 x 8 array of MGL color values. When
filling in MGL_PIXMAP mode, the foreground and background color values are
not used, and the  pixel colors are obtained directly from the pixmap pattern colors.

SEE ALSO:
MGL_setPenPixmapPattern, MGL_setPenBitmapPattern, MGL_setPenStyle,
MGL_getPenStyle
****************************************************************************/
void MGLAPI MGL_getPenPixmapPattern(
	pixpattern_t *pat)
{ memcpy(pat,&DC.a.penPixPat,sizeof(pixpattern_t)); }

/****************************************************************************
DESCRIPTION:
Sets the current text horizontal and vertical justification.


HEADER:
mgraph.h

PARAMETERS:
horiz	- New horizontal text justification value
vert	- New vertical text justification value

REMARKS:
Sets the current text justification values. Horizontal and vertical justification
type supported by the SciTech MGL are enumerated in MGL_textJustType.

SEE ALSO:
MGL_getTextJustify

****************************************************************************/
void MGLAPI MGL_setTextJustify(
	int horiz,
	int vert)
{
	DC.a.ts.horizJust = horiz;
	DC.a.ts.vertJust = vert;
}

/****************************************************************************
DESCRIPTION:
Returns the current text justification.

HEADER:
mgraph.h

PARAMETERS:
horiz	- Place to store horizontal justification
vert	- Place to store vertical justification

REMARKS:
Returns the current text justification values. Justification types supported by
the SciTech MGL are enumerated in MGL_textJustType.

SEE ALSO:
MGL_setTextJustify
****************************************************************************/
void MGLAPI MGL_getTextJustify(
	int *horiz,
	int *vert)
{
	*horiz = DC.a.ts.horizJust;
	*vert = DC.a.ts.vertJust;
}

/****************************************************************************
DESCRIPTION:
Sets the current text direction.

HEADER:
mgraph.h

PARAMETERS:
direction	- New text direction value

REMARKS:
Sets the current text direction.  Directions supported by the SciTech MGL are
enumerated in MGL_textJustType.


SEE ALSO:
MGL_getTextDirection, MGL_drawStr

****************************************************************************/
void MGLAPI MGL_setTextDirection(
	int direction)
{ DC.a.ts.dir = direction; }

/****************************************************************************
DESCRIPTION:
Returns the current text direction.

HEADER:
mgraph.h

RETURNS:
Current text direction.

REMARKS:
Returns the current text direction.  Directions supported by the SciTech MGL are
enumerated in MGL_textJustType.

SEE ALSO:
MGL_setTextDirection, MGL_drawStr
****************************************************************************/
int MGLAPI MGL_getTextDirection(void)
{ return DC.a.ts.dir; }

/****************************************************************************
DESCRIPTION:
Sets the current text scaling factors

HEADER:
mgraph.h

PARAMETERS:
numerx	- x scaling numerator value
denomx	- x scaling denominator value
numery	- y scaling numerator value
denomy	- y scaling denominator value

REMARKS:
Sets the current text scaling factors used by MGL. The text size values define an
integer scaling factor to be used, where the actual values will be computed using the
following formula:

			 unscaled * numer
	scaled = ----------------
				  denom

Note:MGL can only scale vectored fonts.

SEE ALSO:
MGL_getTextSize

****************************************************************************/
void MGLAPI MGL_setTextSize(
	int numerx,
	int denomx,
	int numery,
	int denomy)
{
	DC.a.ts.szNumerx = numerx;
	DC.a.ts.szNumery = numery;
	DC.a.ts.szDenomx = denomx;
	DC.a.ts.szDenomy = denomy;
}

/****************************************************************************
DESCRIPTION:
Returns the current text scaling factors.

HEADER:
mgraph.h

PARAMETERS:
numerx	- Place to store the x numerator value
denomx	- Place to store the x denominator value
numery	- Place to store the y numerator value
denomy	- Place to store the y denominator value

REMARKS:
Returns the current text scaling factors used by MGL. The text size values define an
integer scaling factor to be used, where the actual values will be computed using the
following formula:

Note:MGL can only scale vector fonts. Bitmap fonts cannot be scaled.

SEE ALSO:
MGL_setTextSize

****************************************************************************/
void MGLAPI MGL_getTextSize(
	int *numerx,
	int *denomx,
	int *numery,
	int *denomy)
{
	*numerx = DC.a.ts.szNumerx;
	*numery = DC.a.ts.szNumery;
	*denomx = DC.a.ts.szDenomx;
	*denomy = DC.a.ts.szDenomy;
}

/****************************************************************************
DESCRIPTION:
Sets the current space extra value.

HEADER:
mgraph.h

PARAMETERS:
extra	- New space extra value

REMARKS:
Sets the current space extra value used when drawing text in the current font. The
space extra value is normally zero, but can be a positive or negative value. When
this value is positive, it will insert extra space between the characters in a font and
making this value negative will make the characters run on top of each other.

SEE ALSO:
MGL_getSpaceExtra, MGL_drawStr
****************************************************************************/
void MGLAPI MGL_setSpaceExtra(
	int extra)
{ DC.a.ts.spaceExtra = extra; }

/****************************************************************************
DESCRIPTION:
Returns the current space extra value.

HEADER:
mgraph.h

RETURNS:
Current space extra value.

REMARKS:
Returns the current space extra value used when drawing text in the current font.
The space extra value is normally zero, but can be a positive or negative value. This
value can be used to insert extra space between the characters in a font (making this
value a large negative value will make the characters run on top of each other).

SEE ALSO:
MGL_setSpaceExtra, MGL_drawStr
****************************************************************************/
int	MGLAPI MGL_getSpaceExtra(void)
{ return DC.a.ts.spaceExtra; }

/****************************************************************************

MGL_getFont
<<BMP>>

DESCRIPTION:

Returns the currently active font.

HEADER:
mgraph.h

RETURNS:
Pointer to currently active font.

REMARKS:
Returns a pointer to the currently active font. The currently active font is used to
perform all text output by MGL.

SEE ALSO:
MGL_useFont, MGL_loadFont, MGL_unloadFont
****************************************************************************/
font_t * MGLAPI MGL_getFont(void)
{ return DC.a.ts.font; }
#endif

/****************************************************************************
DESCRIPTION:
Returns the current device context driver ID.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Current device context driver id.

REMARKS:
This function returns the numerical id of the currently active display device driver
for the device context. This value is only meaningful for display device drivers
where the actual hardware device driver can be different. This value can be
converted to a printable representation with the MGL_driverName routine.

SEE ALSO:
MGL_driverName, MGL_getMode

****************************************************************************/
int MGLAPI MGL_getDriver(
	MGLDC *dc)
{ return _MGL_isFullscreenDevice(dc) ? dc->v->d.graphDriver : grNONE; }

/****************************************************************************
DESCRIPTION:
Returns the current video mode number for the display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Current video mode number.

REMARKS:
Returns the currently active video mode number. This number can be converted to a
printable form using the MGL_modeName routine.

SEE ALSO:
MGL_init, MGL_modeName
****************************************************************************/
int MGLAPI MGL_getMode(
	MGLDC *dc)
{ return _MGL_isFullscreenDevice(dc) ? dc->v->d.graphMode : 0; }


/****************************************************************************
DESCRIPTION:
Returns the currently active hardware display page.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Currently active hardware display page.

REMARKS:
This function returns the currently active hardware display page number. The first
hardware display page is 0, the second is 1 and so on. The number of available
hardware pages depends on the type of underlying hardware, the video mode
resolution and amount of video memory installed. Thus not all video modes support
multiple hardware display pages.

All MGL output is always sent to the currently active hardware display page, and
changing the active and visual display pages is used to implement double buffering.

SEE ALSO:
MGL_setActivePage, MGL_getVisualPage, MGL_setVisualPage.

****************************************************************************/
int MGLAPI MGL_getActivePage(
	MGLDC *dc)
{
	return _MGL_isFullscreenDevice(dc) ? dc->v->d.activePage : 0;
}

/****************************************************************************
DESCRIPTION:
Returns the currently visible hardware video page.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Currently visible hardware video page.

REMARKS:
This function returns the currently visible hardware video page number for the
given device context. The first hardware video page is number 0, the second is 1
and so on. The number of available hardware video pages depends on the type of
underlying hardware, the video mode resolution and amount of video memory
installed. Thus not all video modes support multiple hardware video pages.

SEE ALSO:
MGL_setVisualPage, MGL_getActivePage, MGL_setActivePage
****************************************************************************/
int MGLAPI MGL_getVisualPage(
	MGLDC *dc)
{
	return _MGL_isFullscreenDevice(dc) ? dc->v->d.visualPage : 0;
}

/****************************************************************************
DESCRIPTION:
Sets the currently active hardware display page for a display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context of interest
page	- Number of active hardware display page to use

REMARKS:
This function sets the currently active hardware video page to which all output from
MGL is sent to. The first hardware video page is number 0, the second is 1 and so
on. The number of available hardware video pages depends on the type of
underlying hardware, the video mode resolution and amount of video memory
installed. Thus not all video modes support multiple hardware video pages.

SEE ALSO:
MGL_getActivePage, MGL_setVisualPage, MGL_getVisualPage,
MGL_swapBuffers

****************************************************************************/
void MGLAPI MGL_setActivePage(
	MGLDC *dc,
	int page)
{
	if (_MGL_isFullscreenDevice(dc)) {
		dc->v->d.setActivePage(dc,page);
		if (dc == _MGL_dcPtr) {
			/* Make sure we update the globals cached in the current device
			 * context also
			 */
			DC.surface = dc->surface;
			DC.bankOffset = dc->bankOffset;
			}
		}
}

/****************************************************************************
DESCRIPTION:
Sets the currently visible hardware video page for a display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context
page	- New hardware display page
waitVRT	- True if we should sync to the vertical retrace

REMARKS:
This function sets the currently visible hardware video page for a display device
context. The first hardware video page is number 0, the second is 1 and so on. The
number of available hardware video pages depends on the type of underlying
hardware, the video mode resolution and amount of video memory installed. Thus
not all video modes support multiple hardware video pages.

When the visible display page is changed, you should normally allow MGL to sync
to the vertical retrace to ensure that the change occurs in the correct place, and that
you don't get flicker effects on the display. You may however turn off the vertical
retrace synching if you are synching up with the retrace period using some other
means.

Note that if you wish to implement both double buffering and hardware scrolling or
panning, you should call the MGL_setDisplayStart function first with waitVRT set
to -1, and then call this function with waitVRT set to true to actually update the
hardware. The first call to MGL_setDisplayStart simply updates the internal display
start variables but does not program the hardware. For more information please see
the MGL_setDisplayStart function.

SEE ALSO:
MGL_getVisualPage, MGL_getActivePage, MGL_setActivePage,
MGL_swapBuffers, MGL_setDisplayStart
****************************************************************************/
void MGLAPI MGL_setVisualPage(
	MGLDC *dc,
	int page,
	int waitVRT)
{
	if (_MGL_isFullscreenDevice(dc)) {
		int oldPage = dc->v->d.visualPage;
		_MS_preChangeVisualPage(page);
		dc->v->d.setVisualPage(dc,page,waitVRT);
		_MS_postChangeVisualPage(oldPage);
		}
}
/****************************************************************************

DESCRIPTION:
Changes the display start address for virtual scrolling.

HEADER:
mgraph.h

PARAMETERS:
dc	- Scrolling display device context to change
x	- New display start x coordinate
y	- New display start y coordinate
waitFlag	- True if we should wait for the vertical retrace

REMARKS:
This function sets the CRTC display starting address for the hardware scrolling
device context to the specified (x,y) coordinate. You can use this routine to
implement hardware scrolling or panning by moving the display start address
coordinates.

The waitVRT flag is used for synchronizing with the vertical retrace and can be one
of the following values:

waitVRT		- Meaning
0			- Set coordinates and update hardware, but do not wait for a vertical
			  retrace when changing the hardware start address.
1			- Set coordinates and update hardware, waiting for a vertical retrace
			  during the update for flicker free panning.
minus1		- Set coordinates but don't update hardware display start.

Passing a waitVRT flag of -1 can be used to implement double buffering and
hardware scrolling at the same time. To do this you would call this function first to
set the display start x and y coordinates without updating the hardware, and then
call MGL_setVisualPage to swap display pages and the new hardware start address
will then be programmed.

SEE ALSO:
MGL_getDisplayStart, MGL_createScrollingDC
****************************************************************************/
void MGLAPI MGL_setDisplayStart(
	MGLDC *dc,
	int x,
	int y,
	int waitVRT)
{
	if (_MGL_isFullscreenDevice(dc)) {
		dc->v->d.setDisplayStart(dc,x,y,waitVRT);
		_MS_displayStartChanged();
		}
}

/****************************************************************************
DESCRIPTION:
Returns the current hardware display starting coordinates.

HEADER:
mgraph.h

PARAMETERS:
dc	- Hardware scrolling device context
x	- Place to store the display start x coordinate
y	- Place to store the display start y coordinate

REMARKS:
This function returns the current hardware display start coordinates for the hardware
scrolling display device context. You can change the display start address with the
MGL_setDisplayStart function.

SEE ALSO:
MGL_setDisplayStart
****************************************************************************/
void MGLAPI MGL_getDisplayStart(
	MGLDC *dc,
	int *x,
	int *y)
{
	if (_MGL_isFullscreenDevice(dc)) {
		*x = dc->startX;
		*y = dc->startY;
		}
}

/****************************************************************************
DESCRIPTION:
Waits for the vertical sync for a display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context of interest

REMARKS:
This function will wait until the next vertical sync comes along for the
current fullscreen graphics mode, before returning.
****************************************************************************/
void MGLAPI MGL_vSync(
	MGLDC *dc)
{
	if (_MGL_isFullscreenDevice(dc)) dc->v->d.vSync(dc);
}

/****************************************************************************
DESCRIPTION:
Tests whether the palette for a device context uses an 8-bit wide palette.

HEADER:
mgraph.h

RETURNS:
True if palette is 8 bits wide, false if 6 bits wide.

PARAMETERS:
dc	- Display device context of interest.

REMARKS:
This function will return true if the display device context is using an
8-bit per primary hardware palette, or false if it is using a 6-bit per
primary hardware palette. The original VGA standard only supports 6-bits
per primary for the color palette, giving you the selection of 256 colors
out of 256,000 for 8bpp modes. However more modern controllers provide
support for 8-bits per primary, giving you a selection of 256 colors out of
a total 16.7 million for 8bpp modes.
****************************************************************************/
ibool MGLAPI MGL_haveWidePalette(
	MGLDC *dc)
{
	if (_MGL_isFullscreenDevice(dc))
		return dc->v->d.widePalette;
	return false;
}

/****************************************************************************
DESCRIPTION:
Enables double buffering for the specified display device context.

HEADER:
mgraph.h

dc	- Device context to enable double buffering for

RETURNS:
True if double buffering is now enabled, false if not.

REMARKS:
Enables double buffered graphics mode for the specified device context if possible.
When the device context is in double buffered mode, all active output is sent to the
hidden backbuffer, while the current front buffer is being displayed. You then make
calls to MGL_swapBuffers to swap the front and back buffers so that the previously
hidden backbuffer is instantly displayed.

If you intend to start double buffered graphics, you should make sure you call the
MGL_createDisplayDC function with the double buffer flag set to true, so that
some of offscreen video memory will be allocated for the backbuffer. If the device
context only has one video page available, double buffering cannot be started and
this function will fail.

SEE ALSO:
MGL_singleBuffer, MGL_swapBuffers
****************************************************************************/
ibool MGLAPI MGL_doubleBuffer(
	MGLDC *dc)
{
	if (dc->mi.maxPage > 0) {
		MGL_setActivePage(dc,(dc->v->d.visualPage+1)%2);
		return true;
		}
	else
		return false;
}

/****************************************************************************
DESCRIPTION:
Returns the display device context single buffered mode.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context

REMARKS:
This function puts the display device context into single buffer mode. The active
display page is made to be the same as the current visual display page for hardware
double buffering. This may or may not be the first hardware video page.

SEE ALSO:
MGL_doubleBuffer, MGL_swapBuffers
****************************************************************************/
void MGLAPI MGL_singleBuffer(
	MGLDC *dc)
{
	MGL_setActivePage(dc,dc->v->d.visualPage);
}

/****************************************************************************
DESCRIPTION:
Swaps the currently active front and back buffers for a display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context
waitVRT	- Should we wait for the vertical retrace?

REMARKS:
This function swaps the currently active front and back buffers. This routine should
only be called after the MGL_doubleBuffer has been called to initialize the double
buffering for the device context. Once double buffering has been set up, all output
from MGL will go to the current offscreen buffer, and the output can be made
visible by calling this routine. This routine is the standard technique used to achieve
smooth animation.

When the visible display buffer is changed, you should normally allow MGL to
sync to the vertical retrace to ensure that the change occurs in the correct place, and
that you don't get flicker effects on the display. You may however turn off the
vertical retrace synching if you are synching up with the retrace period using some
other means.

SEE ALSO:
MGL_doubleBuffer, MGL_singleBuffer
****************************************************************************/
void MGLAPI MGL_swapBuffers(
	MGLDC *dc,
	int waitVRT)
{
	MGL_setActivePage(dc,(dc->v->d.activePage+1)%2);
	MGL_setVisualPage(dc,(dc->v->d.visualPage+1)%2,waitVRT);
}

#ifndef	MGL_LITE
/****************************************************************************
DESCRIPTION:
Restores the current text settings.

HEADER:
mgraph.h

PARAMETERS:
settings	- Text settings to restore

REMARKS:
Restores a set of previously saved text settings. This routine provides a way to save
and restore all the values relating to the rasterizing of text in MGL with a single
function call.

SEE ALSO:
MGL_getTextSettings
****************************************************************************/
void MGLAPI MGL_setTextSettings(
	text_settings_t *settings)
{
	DC.a.ts = *settings;
	MGL_useFont(settings->font);
	MGL_setTextDirection(settings->dir);
}

/****************************************************************************
DESCRIPTION:

Returns the current text settings.

HEADER:
mgraph.h

PARAMETERS:
settings	- Place to store the current text settings

REMARKS:
Returns a copy of the currently active text settings. This routine provides a way to
save and restore all the values relating to the rasterizing of text in MGL with a
single function call.

SEE ALSO:
MGL_setTextSettings

****************************************************************************/
void MGLAPI MGL_getTextSettings(
	text_settings_t *settings)
{ *settings = DC.a.ts; }


/*******************************************************************************
DESCRIPTION:
Sets the currently active font.

HEADER:
mgraph.h

PARAMETERS:
font	- New font to use

RETURNS:
True if the font was valid and selected, false if not.

REMARKS:
Selects the specified font as the currently active font for the active device context. If
the font data is invalid, the MGL result flag is set and the routine will return false.

Do not unload a font file if it is currently in use by MGL!

SEE ALSO:
MGL_drawStr,MGL_loadFont, MGL_unloadFont

*******************************************************************************/
ibool MGLAPI MGL_useFont(
	font_t *font)
{
	_MGL_result = grOK;
	if (font == NULL)
		return true;
	if (font->fontType < 0 || font->fontType > MGL_PROPFONT) {
		_MGL_result = grInvalidFont;
		return false;
		}

	/* Ensure that the font is rotated back the default direction before
	 * swapping to the new font.
	 */

	MGL_setTextDirection(MGL_RIGHT_TEXT);
	DC.a.ts.font = font;
	return true;
}
#endif

/* {secret} */
void ASMAPI _MGL_setRenderingVectors(void)
/****************************************************************************
*
* Function:		_MGL_setRenderingVectors
*
* Description:	Sets up all the rendering vectors depending on the current
*				rendering options so that we can eliminate all switching
*				code in the high level MGL functions.
*
****************************************************************************/
{
	if (DC.a.colorMode == MGL_DITHER_RGB_MODE) {
		DC.r.cur = DC.r.dither;
		}
	else {
		switch (DC.a.penStyle) {
			case MGL_BITMAP_SOLID:
				if (DC.a.penWidth != 0 || DC.a.penHeight != 0) {
					if (DC.a.writeMode != MGL_REPLACE_MODE)
						DC.r.cur = DC.r.fatRopSolid;
					else
						DC.r.cur = DC.r.fatSolid;
					}
				else if (DC.a.writeMode != MGL_REPLACE_MODE)
					DC.r.cur = DC.r.ropSolid;
				else
					DC.r.cur = DC.r.solid;
				break;
			case MGL_BITMAP_OPAQUE:
			case MGL_BITMAP_TRANSPARENT:
				if (DC.a.penWidth != 0 || DC.a.penHeight != 0)
					DC.r.cur = DC.r.fatPatt;
				else
					DC.r.cur = DC.r.patt;
				break;
			case MGL_PIXMAP:
				if (DC.a.penWidth != 0 || DC.a.penHeight != 0)
					DC.r.cur = DC.r.fatColorPatt;
				else
					DC.r.cur = DC.r.colorPatt;
				break;
			}
		}
}

/****************************************************************************
DESCRIPTION:
Returns a copy of the current rasterizing attributes.

HEADER:
mgraph.h

PARAMETERS:
attr	- Pointer to structure to store attribute values in

REMARKS:
This function returns a copy of the currently active attributes. You can use this
routine to save the state of MGL and later restore this state with the
MGL_restoreAttributes routine.

SEE ALSO:
MGL_restoreAttributes
****************************************************************************/
void MGLAPI MGL_getAttributes(
	attributes_t *attr)
{
	*attr = DC.a;
	attr->penHeight++;
	attr->penWidth++;
}

/****************************************************************************
DESCRIPTION:
Restores a previously saved set of rasterizing attributes.

HEADER:
mgraph.h

PARAMETERS:
attr	- Pointer to the attribute list to restore

REMARKS:
This function restores a set of attributes that were saved with the
MGL_getAttributes routine. The attributes list represents the current state of MGL.
The value of the color palette is not changed by this routine.

SEE ALSO:
MGL_getAttributes
****************************************************************************/
void MGLAPI MGL_restoreAttributes(
	attributes_t *attr)
{
	DC.a = *attr;
	DC.a.penHeight--;
	DC.a.penWidth--;
	MGL_setColor(attr->color);
	MGL_setBackColor(attr->backColor);
#ifndef	MGL_LITE
	MGL_setWriteMode(attr->writeMode);
	MGL_setPenStyle(attr->penStyle);
	MGL_setLineStipple((ushort)attr->lineStipple);
	MGL_setLineStippleCount(attr->stippleCount);
	MGL_setPenBitmapPattern(&attr->penPat);
	MGL_setPenPixmapPattern(&attr->penPixPat);
	MGL_setTextSettings(&attr->ts);
#endif
	MGL_setViewport(attr->viewPort);
	MGL_setClipRect(attr->clipRect);
}

/****************************************************************************
DESCRIPTION:

Reset all rasterizing attributes to their default values.

HEADER:
mgraph.h

PARAMETERS:
dc	- device context to be reset

REMARKS:
This function resets all of the device context attributes to their default
values.

SEE ALSO:
MGL_getAttributes, MGL_restoreAttributes, MGL_getDefaultPalette

****************************************************************************/
void MGLAPI MGL_defaultAttributes(
	MGLDC *dc)
{
	MGLDC	*oldDC = _MGL_dcPtr;

	dc->a.color				= MGL_realColor(dc, MGL_WHITE);
	dc->a.backColor			= MGL_BLACK;
	dc->a.colorMode			= MGL_CMAP_MODE;
	dc->a.markerSize		= 3;
	dc->a.markerStyle		= MGL_MARKER_SQUARE;
	dc->a.markerColor 		= dc->a.color;
	dc->a.bdrBright 		= MGL_realColor(dc,MGL_WHITE);
	dc->a.bdrDark 			= MGL_realColor(dc,MGL_DARKGRAY);
	dc->a.CP.x 				= 0;
	dc->a.CP.y 				= 0;
	dc->a.writeMode			= MGL_REPLACE_MODE;
	dc->a.penStyle			= MGL_BITMAP_SOLID;
	dc->a.penHeight 		= 1;
	dc->a.penWidth			= 1;
	dc->a.lineStyle			= MGL_LINE_PENSTYLE;
	dc->a.lineStipple		= 0xFFFF;
	dc->a.stippleCount		= 0;
	dc->a.viewPort			= dc->size;
	dc->a.clipRect			= dc->size;
	dc->a.clip 				= MGL_CLIPON;
	dc->a.polyType			= MGL_AUTO_POLYGON;
	dc->a.ts.horizJust		= MGL_LEFT_TEXT;
	dc->a.ts.vertJust		= MGL_TOP_TEXT;
	dc->a.ts.dir			= MGL_RIGHT_TEXT;
	dc->a.ts.szNumerx		= 1;
	dc->a.ts.szNumery		= 1;
	dc->a.ts.szDenomx		= 1;
	dc->a.ts.szDenomy		= 1;
	dc->a.ts.spaceExtra 	= 0;
	dc->a.ts.font			= NULL;
	MGL_makeCurrentDC(dc);
	_MGL_updateCurrentDC(dc);
	MGL_restoreAttributes(&DC.a);
	MGL_makeCurrentDC(oldDC);
}

/****************************************************************************
DESCRIPTION:
Pack an RGB tuple into a packed 24 bit color.

HEADER:
mgraph.h

PARAMETERS:
R	- Red color component to pack (0 - 255)
G	- Green color component to pack (0 - 255)
B	- Blue color component to pack (0 - 255)
c	- Color into which to pack values (syntax 3)

RETURNS:
MGL packed color value appropriate for the specified pixel format information.

REMARKS:
This function takes an RGB tuple of 8 bit color components and packs them into a
24 bit packed color value. This function is useful for packing 24 bit color values to
be passed to MGL when running in dithered 8 bit modes.

SEE ALSO:
MGL_packColorRGBFast
****************************************************************************/
color_t ASMAPI MGL_packColorRGB(
	uchar R,
	uchar G,
	uchar B)
{
	return MGL_packColorRGBFast(R,G,B);
}

/****************************************************************************
DESCRIPTION:
Unpacks a packed MGL color value into RGB components.


HEADER:
mgraph.h

PARAMETERS:
pf	- Pixel format to use for unpacking
color	- Color to unpack
R	- Place to store red extracted component
G	- Place to store green extracted component
B	- Place to store blue extracted component

REMARKS:
This function takes a packed color value in the correct format for the specified pixel
format and extracts the red, green and blue components. Note that the color values
may not be the same as when you packed them with MGL_packColor if the pixel
format is a 15 or 16 bit format because of loss of precision. The values are scaled
back into the normal 24 bit RGB space.

SEE ALSO:
MGL_unpackColorFast, MGL_packColor, MGL_getPixelFormat
****************************************************************************/
void MGLAPI MGL_unpackColor(
	pixel_format_t *pf,
	color_t color,
	uchar *R,
	uchar *G,
	uchar *B)
{
	MGL_unpackColorFast(pf,color,*R,*G,*B);
}

/****************************************************************************
DESCRIPTION:
Unpacks a packed 24 bit color value into RGB components.

HEADER:
mgraph.h

PARAMETERS:
color	- Color to unpack
R	- Place to store red extracted component
G	- Place to store green extracted component
B	- Place to store blue extracted component

REMARKS:
This function takes a packed 24 bit color value and extracts the red, green and blue
components.

SEE ALSO:
MGL_unpackColorRGBFast, MGL_packColorRGB, MGL_packColor
****************************************************************************/
void MGLAPI MGL_unpackColorRGB(
	color_t color,
	uchar *R,
	uchar *G,
	uchar *B)
{
	MGL_unpackColorRGBFast(color,*R,*G,*B);
}

/****************************************************************************
DESCRIPTION:
Returns the current packed pixel format information.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest
format	- Place to store the pixel format information

REMARKS:
This function returns the current pixel format information for the specified device
context. This information is used by MGL to encode the packed pixel information,
and can be used by your application to work out how to pack values correctly for
the RGB device contexts.

SEE ALSO:
MGL_packColor, MGL_unpackColor
****************************************************************************/
void MGLAPI MGL_getPixelFormat(
	MGLDC *dc,
	pixel_format_t *pf)
{ *pf = dc->pf; }


/****************************************************************************
DESCRIPTION:
Computes the address of a pixel in the device context surface.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to compute the pixel address for
x	- X coordinate of pixel to address
y	- Y coordinate of pixel to address

RETURNS:
Pointer to the start of the pixel in the surface of the device context.

REMARKS:
This function computes the address of a pixel in the surface of a specific
device context. This function is most useful for developing custom rendering
routines that draw directly to the surface of a device context, and will
compute the address of the pixel correctly regardless of the color depth
of the device context. Essentially this function computes the following:

	addr = dc->surface + (y * bytesPerLine) + (x * bytesPerpixel)

If you are going to be doing a lot of address calculations, it will be
faster to optimise your code to do the calculations directly in place (such
as with a macro) and specifically to eliminate the last multiply if you
know in advance what color depth you are working with (ie: change it to be
x, x*2, x*3 or x*4 depending on the color depth).

Note: You cannot use this function to address the device context surface
	  if the device surface access type returned by MGL_getSurfaceAccessType
	  is set to MGL_NO_ACCESS.
****************************************************************************/
void * MGLAPI MGL_computePixelAddr(
	MGLDC *dc,
	int x,
	int y)
{
	return _MGL_pixelAddr2(dc->surface,x,y,dc->mi.bytesPerLine,
		dc->mi.bitsPerPixel);
}

#ifndef	MGL_LITE

/****************************************************************************
DESCRIPTION:
Turns on or off identity palette checking.

HEADER:
mgraph.h

PARAMETERS:
enable	- True to enable identity palette checking, false to disable

RETURNS:
Old value of the identity palette check flag.

REMARKS:
Turns on or off the checking of identity palette mappings for MGL. This is a global
flag, and by default, identity palette checking is turned on. When MGL_bitBlt
is called for 4 and 8 bit source bitmaps MGL first checks if the
color palettes for the source and destination bitmaps are the same. If they are not,
MGL translates the pixel values from the source bitmap to the destination color
palette, looking for the closest match color if an exact match is not found. You can
use this function to tell MGL that all bitmaps will have identity palettes and that
you will handle all pixel translations yourself, which allows the BitBlt code to run
with higher performance.

SEE ALSO:
MGL_bitBlt

****************************************************************************/
ibool MGLAPI MGL_checkIdentityPalette(
	ibool enable)
{
	ibool old = _MGL_checkIdentityPal;
	_MGL_checkIdentityPal = enable;
	return old;
}

pattern_t * MGLAPI _MGL_getEmptyPat(void)  { return &_MGL_empty_pat; }
pattern_t * MGLAPI _MGL_getGrayPat(void)   { return &_MGL_gray_pat; }
pattern_t * MGLAPI _MGL_getSolidPat(void)  { return &_MGL_solid_pat; }
#endif

#if defined(NO_ASSEMBLER) || !defined(__INTEL__)

/****************************************************************************
DESCRIPTION:
Pack an RGB tuple into an MGL color.

HEADER:
mgraph.h

PARAMETERS:
pf	- Pixel format to get packing information from
R	- Red color component to pack (0	- 255)
G	- Green color component to pack (0	- 255)
B	- Blue color component to pack (0	- 255)

RETURNS:
MGL packed color value appropriate for the specified pixel format information.

REMARKS:
This function takes an RGB tuple of 8 bit color components and packs them into an
MGL packed color value. The color components are packed according to the
specified pixel format information, which can be obtained directly from the mode
information for a bitmap or an MGL device context. When running in dithered 8 bit
modes you may wish to use the MGL_packColorRGB functions which will be
faster.

SEE ALSO:
MGL_packColorFast, MGL_packColorRGB, MGL_unpackColor
****************************************************************************/
color_t ASMAPI MGL_packColor(
	pixel_format_t *pf,
	uchar R,
	uchar G,
	uchar B)
{
	return MGL_packColorFast(pf,R,G,B);
}

#undef MGL_unpackColorFast
#undef MGL_unpackColorRGBFast
#undef MGL_packColorFast
#undef MGL_packColorRGBFast
#undef MGL_packColorRGBFast2
#undef MGL_rgbBlue
#undef MGL_rgbGreen
#undef MGL_rgbRed

/****************************************************************************
DESCRIPTION:
Unpacks a packed MGL color value into RGB components.

HEADER:
mgraph.h

PARAMETERS:
pf	- Pixel format to use for unpacking
color	- Color to unpack
R	- Place to store red extracted component
G	- Place to store green extracted component
B	- Place to store blue extracted component

REMARKS:
This function is the same as MGL_unpackColor, however it is implemented as
a macro and hence is more efficient.

SEE ALSO:
MGL_unpackColor, MGL_packColor, MGL_getPixelFormat
****************************************************************************/
void MGL_unpackColorFast(
	pixel_format_t *pf,
	color_t color,
	uchar R,
	uchar G,
	uchar B);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Unpacks a packed 24 bit color value into RGB components.

HEADER:
mgraph.h

PARAMETERS:
color	- Color to unpack
R		- Place to store red extracted component
G		- Place to store green extracted component
B		- Place to store blue extracted component

REMARKS:
This function is the same as MGL_unpackColorRGB, however it is implemented as
a macro and hence is more efficient.

SEE ALSO:
MGL_unpackColorRGB, MGL_packColorRGB, MGL_packColor
****************************************************************************/
void MGL_unpackColorRGBFast(
	color_t color,
	uchar R,
	uchar G,
	uchar B);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Pack an RGB tuple into an MGL color.

HEADER:
mgraph.h

PARAMETERS:
pf	- Pixel format to get packing information from
R	- Red color component to pack (0	- 255)
G	- Green color component to pack (0	- 255)
B	- Blue color component to pack (0	- 255)

RETURNS:
MGL packed color value appropriate for the specified pixel format information.

REMARKS:
This function is the same as MGL_packColor, however it is implemented as a
macro and hence is more efficient.

SEE ALSO:
MGL_packColor, MGL_packColorRGB, MGL_unpackColor
****************************************************************************/
color_t MGL_packColorFast(
	pixel_format_t *pf,
	uchar r,
	uchar g,
	uchar b);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Pack an RGB tuple into a packed 24 bit color.

HEADER:
mgraph.h

PARAMETERS:
R	- Red color component to pack (0 - 255)
G	- Green color component to pack (0 - 255)
B	- Blue color component to pack (0 - 255)
c	- Color into which to pack values (syntax 3)

RETURNS:
MGL packed color value appropriate for the specified pixel format information.

REMARKS:
This function is the same as MGL_packColorRGB, however it is implemented as a
macro and hence is more efficient.

SEE ALSO:
MGL_packColorRGB
****************************************************************************/
color_t MGL_packColorRGBFast(
	uchar r,
	uchar b,
	uchar g);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Pack an RGB tuple into a packed 24 bit color.

HEADER:
mgraph.h

PARAMETERS:
c	- Color into which to pack values (syntax 3)
R	- Red color component to pack (0 - 255)
G	- Green color component to pack (0 - 255)
B	- Blue color component to pack (0 - 255)

REMARKS:
This function is the same as MGL_packColorRGBFast, however it is implemented as a
macro and hence is more efficient. It also packs the color value directly
into the c parameter without requiring you to pass a pointer (because it is
a macro).

SEE ALSO:
MGL_packColorRGBFast, MGL_packColorRGB
****************************************************************************/
void MGL_packColorRGBFast2(
	color_t c,
	uchar r,
	uchar g,
	uchar b);
/* Implemented as a macro */

/***************************************************************************
DESCRIPTION:
Extract the blue component from a packed 24 bit RGB tuple.

HEADER:
mgraph.h

PARAMETERS:
c	- Packed 24 bit color value to extract value from

RETURNS:
Blue component from the packed 24 bit color value.

SEE ALSO:
MGL_rgbRed, MGL_rgbGreen
***************************************************************************/
uchar  MGL_rgbBlue(
	color_t c);
/* Implemented as a macro */

/***************************************************************************
DESCRIPTION:
Extract the green component from a packed 24 bit RGB tuple.

HEADER:
mgraph.h

PARAMETERS:
c	- Packed 24 bit color value to extract value from

RETURNS:
Green component from the packed 24 bit color value.

SEE ALSO:
MGL_rgbRed, MGL_rgbBlue
***************************************************************************/
uchar  MGL_rgbGreen(
	color_t c);
/* Implemented as a macro */

/***************************************************************************
DESCRIPTION:
Extract the red component from a packed 24 bit RGB tuple.

HEADER:
mgraph.h

PARAMETERS:
c	- Packed 24 bit color value to extract value from

RETURNS:
Red component from the packed 24 bit color value.

SEE ALSO:
MGL_rgbGreen, MGL_rgbBlue
***************************************************************************/
uchar  MGL_rgbRed(
	color_t c);
/* Implemented as a macro */

#endif

