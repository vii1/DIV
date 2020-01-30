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
* Description:	Set of routines for drawing 3D style borders (ala windows
*				and OS/2 PM).
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

#undef	MGL_drawBorder

/****************************************************************************
DESCRIPTION:
Draws a 3d border around a specified rectangle.

HEADER:
mgraph.h

PARAMETERS:
left	- Left edge of rectangle
top	- Top edge of rectangle
right	- Right edge of rectangle
bottom	- Bottom edge of rectangle
style	- Style of border to draw (INSET, OUTSET etc)
thickness	- Thickness of the border in pixels

REMARKS:
If the style is set to BDR_INSET, the border is colored so that it look
like the interior of the rectangle is inset into the display. If style is set
to BDR_OUTSET, the border is colored so that the interior looks like it is
raised. If style is set to OUTLINE, the border is drawn as a 3d 2 pixels thick
outline.

SEE ALSO:
MGL_drawBorder
****************************************************************************/
void MGLAPI MGL_drawBorderCoord(
	int left,
	int top,
	int right,
	int bottom,
	int style,
	int thickness)
{
	int		i;
	color_t	color = DC.a.color;
	color_t	bright,dark;

	if (bottom <= top+1 || right <= left+1)
		return;

	switch (style) {
		case MGL_BDR_INSET:
			bright = DC.a.bdrDark;
			dark = DC.a.bdrBright;
			break;
		case MGL_BDR_OUTSET:
			bright = DC.a.bdrBright;
			dark = DC.a.bdrDark;
			break;
		default:	/* MGL_BDR_OUTLINE: */
			MGL_setColor(DC.a.bdrBright);
			MGL_rectCoord(left+1,top+1,right,bottom);
			MGL_setColor(DC.a.bdrDark);
			MGL_rectCoord(left,top,right-1,bottom-1);
			MGL_setColor(color);
			return;
		}

	MGL_setColor(bright);
	for (i = 0; i < thickness; i++) {
		MGL_lineCoord(left, top+i, right-i-1, top+i);
		MGL_lineCoord(left+i, top, left+i, bottom-i-1);
		}

	MGL_setColor(dark);
	for (i = 0; i < thickness; i++) {
		MGL_lineCoord(right-i-1, top+i+1, right-i-1, bottom-1);
		MGL_lineCoord(left+i+1, bottom-i-1, right-1, bottom-i-1);
		}
	MGL_setColor(color);
}

/****************************************************************************
DESCRIPTION:
Draws a 3d border around a specified rectangle.

HEADER:
mgraph.h

PARAMETERS:
r	- Rectangle to draw border around (Syntax 2)
style	- Style of border to draw (INSET, OUTSET etc)
thickness	- Thickness of the border in pixels

REMARKS:
This function is the same as MGL_drawBorderCoord, however it takes entire
rectangles as arguments instead of coordinates.

SEE ALSO:
MGL_drawBorderCoord
****************************************************************************/
void MGLAPI MGL_drawBorder(
	rect_t r,
	int style,
	int thickness);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a 3d horizontal dividing line.

HEADER:
mgraph.h

PARAMETERS:
y	- Y coordinate to draw line at
x1	- Starting x coordinate
x2	- Ending x coordinate

REMARKS:
The line is drawn is two pixels thick, ending at y+1.
****************************************************************************/
void MGLAPI MGL_drawHDivider(
	int y,
	int x1,
	int x2)
{
	color_t	color = DC.a.color;

	MGL_setColor(DC.a.bdrDark);
	MGL_lineCoord(x1,y,x2,y);
	MGL_setColor(DC.a.bdrBright);
	MGL_lineCoord(x1,y+1,x2,y+1);
	MGL_setColor(color);
}

/****************************************************************************
DESCRIPTION:
Draws a 3d vertical dividing line.

HEADER:
mgraph.h

PARAMETERS:
x	- X coordinate to draw line at
y1	- Starting y coordinate
y2	- Ending y coordinate

RETURNS:
void

REMARKS:
The line is drawn is two pixels wide, ending at x+1.
****************************************************************************/
void MGLAPI MGL_drawVDivider(
	int x,
	int y1,
	int y2)
{
	color_t	color = DC.a.color;

	MGL_setColor(DC.a.bdrDark);
	MGL_lineCoord(x,y1,x,y2);
	MGL_setColor(DC.a.bdrBright);
	MGL_lineCoord(x+1,y1,x+1,y2);
	MGL_setColor(color);
}

#endif	/* !MGL_LITE */

