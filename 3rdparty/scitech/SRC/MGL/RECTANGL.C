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
* Description:	Rectangle drawing routines
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

#undef	MGL_rect
#undef	MGL_rectPt
#undef	MGL_fillRect
#undef	MGL_fillRectPt

/****************************************************************************
DESCRIPTION:
Draws a rectangle outline.

HEADER:
mgraph.h

PARAMETERS:
left	- Left coordinate of the rectangle
top	- Top coordinate of the rectangle
right	- Right coordinate of the rectangle
bottom	- Bottom coordinate of the rectangle
r	- Rectangle to draw
leftTop	- Point for upper left corner of rectangle
rightBottom	- Point containing lower right corner of rectangle 

REMARKS:
This function draws a rectangle outline in the current drawing attributes at the
specified location.

SEE ALSO:
MGL_rect, MGL_rectPt
****************************************************************************/
void MGLAPI MGL_rectCoord(
	int left,
	int top,
	int right,
	int bottom)
{
	int	h = DC.a.penHeight+1;

	if (bottom <= top+1 || right <= left+1)
		return;
	MGL_lineCoord(left,top,right-1,top);
	MGL_lineCoord(left,bottom-1,right-1,bottom-1);
	MGL_lineCoord(left,top+h,left,bottom-h-1);
	MGL_lineCoord(right-1,top+h,right-1,bottom-h-1);
}

/****************************************************************************
DESCRIPTION:
Draws a rectangle outline.

HEADER:
mgraph.h

PARAMETERS:
r	- Rectangle to draw 

REMARKS:
This function is the same as MGL_rectCoord, however it takes an entire
rectangle as the parameter instead of coordinates.

SEE ALSO:
MGL_rectCoord, MGL_rectPt
****************************************************************************/
void MGL_rect(
	rect_t r);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a rectangle outline.

HEADER:
mgraph.h

PARAMETERS:
r	- Rectangle to draw 

REMARKS:
This function is the same as MGL_rectCoord, however it takes the top left
and bottom right coordinates of the rectangle as two points instead of
four coordinates.

SEE ALSO:
MGL_rectCoord, MGL_rect
****************************************************************************/
void MGL_rectPt(
	point_t leftTop,
	point_t rightBottom);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a filled rectangle.

HEADER:
mgraph.h

PARAMETERS:
left	- Left coordinate of rectangle (syntax 1)
top	- Top coordinate of rectangle (syntax 1)
right	- Right coordinate of rectangle (syntax 1)
bottom	- Bottom coordinate of rectangle (syntax 1)

REMARKS:
Fills a rectangle in the current drawing attributes. The mathematical definition of a
rectangle does not include the right and bottom edges, so effectively the right and
bottom edges are not rasterized (solving problems with shared edges).

SEE ALSO:
MGL_fillRect, MGL_fillRectPt, MGL_rect
****************************************************************************/
void MGLAPI MGL_fillRectCoord(
	int left,
	int top,
	int right,
	int bottom)
{
	rect_t	d;

	d.left = left;		d.top = top;
	d.right = right;	d.bottom = bottom;

	if (DC.a.clip && !MGL_sectRect(DC.a.clipRect,d,&d))
		return;
	MGL_offsetRect(d,DC.a.viewPort.left,DC.a.viewPort.top);
	DC.r.cur.fillRect(d.left,d.top,d.right,d.bottom);
}

/****************************************************************************
DESCRIPTION:
Draws a filled rectangle.

HEADER:
mgraph.h

PARAMETERS:
r	- Rectangle to be filled (syntax 2)

REMARKS:
This function is the same as MGL_fillRectCoord, however it takes an entire
rectangle as the parameter instead of coordinates.

SEE ALSO:
MGL_fillRectCoord, MGL_fillRectPt, MGL_rect
****************************************************************************/
void MGL_fillRect(
	rect_t r);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a filled rectangle.

HEADER:
mgraph.h

PARAMETERS:
leftTop	- Top left coordinate of rectangle (syntax 3)
rightBottom	- Bottom right coordinate of rectangle (syntax 3)

REMARKS:
This function is the same as MGL_fillRectCoord, however it takes the top left
and bottom right coordinates of the rectangle as two points instead of
four coordinates.

SEE ALSO:
MGL_fillRect, MGL_fillRectPt, MGL_rect
****************************************************************************/
void MGL_fillRectPt(
	point_t leftTop,
	point_t rightBottom);
/* Implemented as a macro */

#endif	/* !MGL_LITE */

