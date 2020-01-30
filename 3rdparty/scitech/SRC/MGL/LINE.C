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
* Description:  Line drawing routines (including code for the
*				Cohen-Sutherland line clipping algorithm).
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

#undef	MGL_moveTo
#undef	MGL_moveRel
#undef	MGL_line
#undef	MGL_lineFX
#undef	MGL_lineTo
#undef	MGL_lineRel

/*******************************************************************************
DESCRIPTION:
Moves the CP to a new location.

HEADER:
mgraph.h

PARAMETERS:
x	- New x coordinate for CP
y	- New y coordinate for CP

REMARKS:
Moves the current position (CP) to the new point (x,y).

SEE ALSO:
MGL_moveTo
*******************************************************************************/
void MGLAPI MGL_moveToCoord(
	int x,
	int y)
{ DC.a.CP.x = x; DC.a.CP.y = y; }

/*******************************************************************************
DESCRIPTION:
Moves the CP to a new location.

HEADER:
mgraph.h

PARAMETERS:
p	- New Point for CP

REMARKS:
This function is the same as MGL_moveToCoord, however it takes the
coordinate to move to as a point.

SEE ALSO:
MGL_moveToCoord
*******************************************************************************/
void MGL_moveTo(
	point_t p);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Moves the CP to a new relative location.

HEADER:
mgraph.h

PARAMETERS:
dx	- Amount to move x coordinate
dy	- Amount to move y coordinate

REMARKS:
Moves the current position (CP) to the relative location that is a distance of (dx,dy)
away from the CP. Thus the location the CP is moved to is (CP.x + dx, CP.y + dy).

SEE ALSO:
MGL_moveRel
****************************************************************************/
void MGLAPI MGL_moveRelCoord(
	int dx,
	int dy)
{ DC.a.CP.x += dx; DC.a.CP.y += dy; }

/****************************************************************************
DESCRIPTION:
Moves the CP to a new relative location.

HEADER:
mgraph.h

PARAMETERS:
p	- Use coordinates of this point as offsets

REMARKS:
This function is the same as MGL_moveRelCoord, however it takes the
amount to move as a point.

SEE ALSO:
MGL_moveRelCoord
****************************************************************************/
void MGL_moveRel(
	point_t p);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Returns the x coordinate of the current position.

HEADER:
mgraph.h

RETURNS:
x coordinate of current position

REMARKS:
Returns the x coordinate of the current position (CP). The CP is the current graphics
cursor position, and is used by a number of output routines to determine where to
being drawing.

SEE ALSO:
MGL_getY, MGL_getCP,
****************************************************************************/
int MGLAPI MGL_getX(void)
{ return DC.a.CP.x; }

/****************************************************************************
DESCRIPTION:
Returns the y coordinate of the current position.

HEADER:
mgraph.h

RETURNS:
y coordinate of current position

REMARKS:
Returns the y coordinate of the current position (CP). The CP is the current graphics
cursor position, and is used by a number of output routines to determine where to
being drawing.

SEE ALSO:
MGL_getX, MGL_getCP

****************************************************************************/
int MGLAPI MGL_getY(void)
{ return DC.a.CP.y; }

/****************************************************************************
DESCRIPTION:
Returns the current position value.

HEADER:
mgraph.h

PARAMETERS:
CP	- Place to store the current position

REMARKS:
Returns the current position (CP). The CP is the current logical graphics cursor
position, and is used by a number of routines to determine where to being drawing
output. You can use the MGL_moveTo routine to directly
move the CP to a new position.

SEE ALSO:
MGL_moveTo, MGL_moveRel, MGL_lineTo, MGL_lineRel, MGL_drawStr
****************************************************************************/
void MGLAPI MGL_getCP(
	point_t *CP)
{ *CP = DC.a.CP; }

/****************************************************************************
DESCRIPTION:
Generates the set of integer points on a line, given fixed point coordinates.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate for first endpoint
y1	- y coordinate for first endpoint
x2	- x coordinate for second endpoint
y2	- y coordinate for second endpoint
plotPoint	- User supplied pixel plotting routine

REMARKS:
This function generates the set of points on a line, and calls a user supplied
plotPoint routine for every point generated. The set of points generated will always
be in the same order for any two endpoints, no matter which order the endpoints are
given.

Note:	Lines must always be passed in with the X1 value less than the X2 value!

Note:	This routine expects the endpoints to be in 32 bit fixed point 16.16 format,
		and can scan convert lines with non-integer endpoints.

SEE ALSO:
MGL_ellipseEngine, MGL_ellipseArcEngine
****************************************************************************/
void MGLAPI MGL_lineEngine(
	fix32_t x1,
	fix32_t y1,
	fix32_t x2,
	fix32_t y2,
	void (ASMAPI *plotPoint)(
		int x,
		int y))
{
	fix32_t	d;						/* Decision variable				*/
	fix32_t	dx,dy;					/* Dx and Dy values for the line	*/
	fix32_t	Eincr,NEincr;			/* Decision variable increments		*/
	int		x,y,count;				/* Current (x,y) and count value	*/
	int		yincr;					/* Increment for values				*/
	ibool	lowslope = true;		/* True if slope <= 1				*/

	if ((x2 - x1) < 0) {
		SWAP(x2,x1);
		SWAP(y2,y1);
		}

	dx = x2 - x1;
	yincr = 1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		}

	if (dy > dx) {
		SWAP(dx,dy);
		lowslope = false;
		}

// TODO: It appears that the last pixel is missing from some hi slope
// lines, and this is probably related to finding the count of the number
// pixels. We should truncate the smallest pixel and round up the largest
// pixel to get the final count rather than rounding the difference between
// the two coordinates.
//
// 320x200 mode with lines clipped has this in the lower right hand
// corner, which we can check for.
//
// Count should not be MGL_FIXTOINT(dx) but should be:
//
//		count = MGL_FIXROUND(x2) - MGL_FIXROUND(x1)
//
// Need to change the code here, test the results and if correct modify
// all the assembler line drawing code to do this also.

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		while (count--) {
			plotPoint(x,y);
			x++;					/* Increment x coordinate			*/

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		while (count--) {
			plotPoint(x,y);
			y += yincr;				/* Increment y coordinate			*/

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				x++;
				}
			}
		}
}

/****************************************************************************
DESCRIPTION:
Draws a line with fixed point coordinates.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate for first endpoint
y1	- y coordinate for first endpoint
x2	- x coordinate for second endpoint
y2	- y coordinate for second endpoint

REMARKS:
Draws a line starting at the point (x1,y1) and ending at the point (x2,y2) in the
current pen style, color and dimensions. The CP is not updated, and the line is
clipped to the current clipping rectangle if clipping is on. Note that the coordinates
are passed in 16.16 fixed point format, which provides for maximum precision
when the lines are drawn.

SEE ALSO:
MGL_lineFX
****************************************************************************/
void MGLAPI MGL_lineCoordFX(
	fix32_t x1,
	fix32_t y1,
	fix32_t x2,
	fix32_t y2)
{
	/* Ensure that we are always scan converting the line from right to
	 * left.
	 */
	if ((x2 - x1) < 0) {
		fix32_t	t;
		SWAPT(x2,x1,t);
		SWAPT(y2,y1,t);
		}
	if (y1 == y2 && (DC.a.lineStyle == MGL_LINE_PENSTYLE)) {
		int iy,iymax,ix1,ix2;

		iymax = (iy = MGL_FIXROUND(y1)) + DC.a.penHeight;
		ix1 = MGL_FIXROUND(x1);
		ix2 = MGL_FIXROUND(x2) + DC.a.penWidth+1;
		while (iy <= iymax)
			_MGL_clipScanLineV(iy++,ix1,ix2);
		}
	else {
		if (DC.a.clip && !MGL_clipLineFX(&x1,&y1,&x2,&y2,
				DC.clipRectFX.left,DC.clipRectFX.top,
				DC.clipRectFX.right,DC.clipRectFX.bottom))
			return;
#ifndef	MGL_LITE
		if (DC.a.lineStyle == MGL_LINE_STIPPLE) {
			DC.r.stippleLine(
				x1 + DC.viewPortFX.left,y1 + DC.viewPortFX.top,
				x2 + DC.viewPortFX.left,y2 + DC.viewPortFX.top);
			}
		else
#endif
			DC.r.cur.line(
				x1 + DC.viewPortFX.left,y1 + DC.viewPortFX.top,
				x2 + DC.viewPortFX.left,y2 + DC.viewPortFX.top);
		}
}

/****************************************************************************
DESCRIPTION:
Draws a line with fixed point coordinates.

HEADER:
mgraph.h

PARAMETERS:
p1	- First endpoint of line
p2	- Second endpoint of line

REMARKS:
This function is the same as MGL_lineCoordFX, however it takes the
coordinates of the line as two points.

SEE ALSO:
MGL_lineCoordFX
****************************************************************************/
void MGL_lineFX(
	fxpoint_t p1,
	fxpoint_t p2);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a line with integer coordinates.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate for first endpoint
y1	- y coordinate for first endpoint
x2	- x coordinate for second endpoint
y2	- y coordinate for second endpoint

REMARKS:
Draws a line starting at the point (x1,y1) and ending at the point (x2,y2) in the
current pen style, color and dimensions. The CP is not updated, and the line is
clipped to the current clipping rectangle if clipping in on. Note that this function
takes the coordinates of the lines in integer format. MGL draws all lines internally
in 16.16 fixed point format, so this routine always converts the coordinates and then
calls the MGL_lineFX routines. If you need maximum
performance, you should call the fixed point line drawing functions instead.

SEE ALSO:
MGL_line
****************************************************************************/
void MGLAPI MGL_lineCoord(
	int x1,
	int y1,
	int x2,
	int y2)
{
	MGL_lineCoordFX(MGL_TOFIX(x1),MGL_TOFIX(y1),MGL_TOFIX(x2),MGL_TOFIX(y2));
}

/****************************************************************************
DESCRIPTION:
Draws a line with integer coordinates.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate for first endpoint
y1	- y coordinate for first endpoint
x2	- x coordinate for second endpoint
y2	- y coordinate for second endpoint

REMARKS:
This function is the same as MGL_lineCoord, however it takes the
coordinates of the line as two points.

SEE ALSO:
MGL_lineCoord
****************************************************************************/
void MGL_line(
	point_t p1,
	point_t p2);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a line from the CP to the specified point.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to draw to
y	- y coordinate to draw to
p	- Point to draw to

REMARKS:
Draws a line from the current position (CP) to the new point (x,y). The CP is set to
the point (x,y) on return from this routine.

SEE ALSO:
MGL_lineTo
****************************************************************************/
void MGLAPI MGL_lineToCoord(
	int x,
	int y)
{
	point_t	p;

	p.x = x;	p.y = y;
	MGL_lineCoord(DC.a.CP.x,DC.a.CP.y,p.x,p.y);
	DC.a.CP = p;
}

/****************************************************************************
DESCRIPTION:
Draws a line from the CP to the specified point.

HEADER:
mgraph.h

PARAMETERS:
p	- Point to draw to

REMARKS:
This function is the same as MGL_lineToCoord, however the point to draw to
is passed as a point.

SEE ALSO:
MGL_lineTo
****************************************************************************/
void MGL_lineTo(
	point_t p);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Draws a relative line.

HEADER:
mgraph.h

PARAMETERS:
dx	- Amount to offset in x coordinate
dy	- Amount to offset in y coordinate

REMARKS:
Draws a line from the current position (CP) to the relative location that is a distance
of (dx,dy) away from the CP. Thus the location of the next point on the line is:

	(CP.x + dx, CP.y + dy)

 The CP is updated to this value.

SEE ALSO:
MGL_lineRel, MGL_moveRelCoord
****************************************************************************/
void MGLAPI MGL_lineRelCoord(
	int dx,
	int dy)
{
	point_t	p1,p2;

	p1 = p2 = DC.a.CP;
	p2.x += dx;	p2.y += dy;
	MGL_lineCoord(p1.x,p1.y,p2.x,p2.y);
	DC.a.CP = p2;
}

/****************************************************************************
DESCRIPTION:
Draws a relative line.

HEADER:
mgraph.h

PARAMETERS:
p	- Amount to offset in (x,y) coordinates

REMARKS:
This function is the same as MGL_lineRelCoord, however the amount to move
the CP by is passed as a point.

SEE ALSO:
MGL_lineRelCoord, MGL_moveRelCoord
****************************************************************************/
void MGL_lineRel(
	point_t p);
/* Implemented as a macro */

