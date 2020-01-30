/****************************************************************************
*
*			  Quick3D - A 3D C++ rendering pipeline for the MGL
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
* Language:		C++ 3.0
* Environment:	any
*
* Description:  3d circle drawing routines.
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick3d::arc(const FXPoint3d& centre,real radius,real startAngle,
	real endAngle)
/****************************************************************************
*
* Function:		Quick3d::ellipseArc
* Parameters:	centre		- Centre point for the arc
*				radius		- radius for the circular arc
*				startAngle	- Starting angle (in 1/10 of a degree)
*				endAngle	- Ending angle (in 1/10 of a degree)
*
* Description:	Draws a circular arc that is approximated by a number
*				of straight line segments. The starting and ending angles
*				are given in increments of 1/10th of a degree.
*
****************************************************************************/
{
	int		numsegs;
	real	deltaAngle,cosine,sine,dx,dy;
	FXPoint3d	p;

	beginLine();

	// Calculate the initial point on the arc, and move pen to this location.
	FXsincos(startAngle,&sine,&cosine);
	p.x = centre.x + FXmul(radius,cosine);
	p.y = centre.y + FXmul(radius,sine);
	p.z = centre.z;
	moveTo(start = p);

	// Fix the ending angle so that it is >= starting angle
	while (endAngle < startAngle)
		endAngle += REAL(360);

	// Calculate the approximate number of line segments to use when drawing
	// the arc, the angular increment, and the sine and cosine of this
	// angular increment.
	numsegs = FXrealToInt(FXmul(FXintToReal(csegs),
		FXdiv(endAngle - startAngle,REAL(360))) + REAL(0.5));
	deltaAngle = FXdiv(endAngle - startAngle,FXintToReal(numsegs));
	FXsincos(deltaAngle,&sine,&cosine);

	// Draw each of the line segments that make up the arc
	while (numsegs--) {
		dx = p.x - centre.x;	dy = p.y - centre.y;
		p.x = centre.x + FXmul(dx,cosine) - FXmul(dy,sine);
		p.y = centre.y + FXmul(dx,sine) + FXmul(dy,cosine);
		lineTo(p);
		}
	end = p;

	endLine();
}

void Quick3d::circle(const FXPoint3d& centre,real radius)
/****************************************************************************
*
* Function:		Quick3d::circle
* Parameters:	centre	- Centre point for the circle
*				radius	- radius for the circle
*
* Description:	Draws a circle that is approximated by a number of straight
*				line segments.
*
****************************************************************************/
{
	int		i;
	real	deltaAngle,cosine,sine,dx,dy;
	FXPoint3d	p;

	beginLine();

	// Calculate the initial point on the circle, and move pen to
	// this location.

	p = centre;	p.x += radius;
	moveTo(start = p);

	// Calculate the the angular increment, and the sine and cosine of this
	// angular increment.

	deltaAngle = FXdiv(REAL(360),FXintToReal(csegs));
	FXsincos(deltaAngle,&sine,&cosine);

	// Draw each of the line segments that make up the arc

	i = csegs;
	while (i--) {
		dx = p.x - centre.x;	dy = p.y - centre.y;
		p.x = centre.x + FXmul(dx,cosine) - FXmul(dy,sine);
		p.y = centre.y + FXmul(dx,sine) + FXmul(dy,cosine);
		lineTo(p);
		}
	lineTo(start);		// Draw last line to ensure it is closed

	endLine();
}

void Quick3d::fillArc(const FXPoint3d& centre,real radius,real startAngle,
	real endAngle)
/****************************************************************************
*
* Function:		Quick3d::fillArc
* Parameters:	centre		- Centre point for the arc
*				radius		- radius for the circular arc
*				startAngle	- Starting angle (in 1/10 of a degree)
*				endAngle	- Ending angle (in 1/10 of a degree)
*
* Description:	Fills a circular arc that is approximated by a number
*				of straight line segments. The starting and ending angles
*				are given in increments of 1/10th of a degree.
*
****************************************************************************/
{
	int		i,numsegs;
	real	deltaAngle,cosine,sine,dx,dy;
	FXPoint3d	p;

	// Calculate the initial point on the arc, and move pen to this location.
	FXsincos(startAngle,&sine,&cosine);
	p.x = centre.x + FXmul(radius,cosine);
	p.y = centre.y + FXmul(radius,sine);
	p.z = centre.z;

	// Fix the ending angle so that it is >= starting angle
	while (endAngle < startAngle)
		endAngle += REAL(360);

	// Calculate the number of line segments to use when drawing
	// the arc, the angular increment, and the sine and cosine of this
	// angular increment.
	i = numsegs = FXrealToInt(FXmul(FXintToReal(csegs),
		FXdiv(endAngle - startAngle,REAL(360))) + REAL(0.5));
	NDC_count = numsegs+2;
	deltaAngle = FXdiv(endAngle - startAngle,FXintToReal(numsegs));
	FXsincos(deltaAngle,&sine,&cosine);

	beginPoly();
	vertex(centre);
	vertex(start = p);

	// Transform each point in the arc, storing coordinates in the polygon
	// array.
	while (i--) {
		dx = p.x - centre.x;	dy = p.y - centre.y;
		p.x = centre.x + FXmul(dx,cosine) - FXmul(dy,sine);
		p.y = centre.y + FXmul(dx,sine) + FXmul(dy,cosine);
		vertex(p);
		}
	end = p;
	endPoly();
}

void Quick3d::fillCircle(const FXPoint3d& centre,real radius)
/****************************************************************************
*
* Function:		Quick3d::fillCircle
* Parameters:	centre		- Centre point for the arc
*				radius		- radius for the circular arc
*
* Description:	Fills a circle that is approximated by a number
*				of straight line segments.
*
****************************************************************************/
{
	int		i;
	real	deltaAngle,cosine,sine,dx,dy;
	FXPoint3d	p;

	// Calculate the initial point on the circle

	p = centre;	p.x += radius;

	// Calculate the the angular increment, and the sine and cosine of this
	// angular increment.

	deltaAngle = FXdiv(REAL(360),FXintToReal(csegs));
	FXsincos(deltaAngle,&sine,&cosine);

	// Transform each point in the circle, storing coordinates in the polygon
	// array.

	NDC_count = (i = csegs)+1;
	beginPoly();
	vertex(p);
	while (i--) {
		dx = p.x - centre.x;	dy = p.y - centre.y;
		p.x = centre.x + FXmul(dx,cosine) - FXmul(dy,sine);
		p.y = centre.y + FXmul(dx,sine) + FXmul(dy,cosine);
		vertex(p);
		}
	endPoly();
}

void Quick3d::circleOutline(void)
/****************************************************************************
*
* Function:		Quick3d::circleOutline
*
* Description:	Draws the outline of a previously drawn circle or arc. This
*				routine MUST be called directly after drawing the circle
*				or arc, but you can change the attributes used to draw the
*				outline before calling it.
*
****************************************************************************/
{
	pipeEntry	*p = pipe;
	int			count = NDC_count;

	beginDrawing();

	// Draw the closing line first
	MGLDevCtx::lineFast(pipe->win.x,pipe->win.y,
		pipe[NDC_count-1].win.x,pipe[NDC_count-1].win.y);

	// Draw the remaining lines in the outline
	while (--count) {
		MGLDevCtx::lineFast(pipe->win.x,pipe->win.y,pipe[1].win.x,pipe[1].win.y);
		p++;
		}

	endDrawing();
}

int Quick3d::setCirclePrecision(int nsegs)
/****************************************************************************
*
* Function:		Quick3d::setCirclePrecision
* Parameters:	nsegs	- New ellipse/ellipse arc precision
* Returns:		Current ellipse precision.
*
****************************************************************************/
{
	int	retval = csegs;
	csegs = nsegs;
	return retval;
}

void Quick3d::getArcCoords(FXPoint3d& _start,FXPoint3d& _end)
/****************************************************************************
*
* Function:		Quick3d::getArcCoords
* Parameters:	_start	- Starting point on ellipse
*				_end	- Ending point on ellipse
*
****************************************************************************/
{
	_start = start;
	_end = end;
}
