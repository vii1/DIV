/****************************************************************************
*
*			 Quick 2D - A 2D C++ tranformation class for the MGL
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
* Description:  2d circle drawing routines.
*
*
****************************************************************************/

#include "quick2d/quick2d.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick2d::arc(const FXPoint2d& centre,real radius,real startAngle,
	real endAngle)
/****************************************************************************
*
* Function:		Quick2d::ellipseArc
* Parameters:	centre		- Centre point for the arc
*				radius		- radius for the circular arc
*				startAngle	- Starting angle (in degrees)
*				endAngle	- Ending angle (in degrees)
*
* Description:	Draws a circular arc that is approximated by a number
*				of straight line segments. The starting and ending angles
*				are given in degrees.
*
****************************************************************************/
{
	int			numsegs;
	real		deltaAngle,cosine,sine;
	FXPoint2d	p,d;

	// Calculate the initial point on the arc, and move pen to this location.
	FXsincos(startAngle,&sine,&cosine);
	p.x = centre.x + FXmul(radius,cosine);
	p.y = centre.y + FXmul(radius,sine);
	start = p;
	moveTo(start);

	// Fix the ending angle so that it is >= starting angle
	while (endAngle < startAngle)
		endAngle += REAL(360);

	// Calculate the approximate number of line segments to use when drawing
	// the arc, the angular increment, and the sine and cosine of this
	// angular increment.
	numsegs = FXrealToInt(FXmul(FXintToReal(csegs),
		FXdiv(endAngle - startAngle,REAL(360)) + REAL(0.5)));
	deltaAngle = FXdiv(endAngle - startAngle,FXintToReal(numsegs));
	FXsincos(deltaAngle,&sine,&cosine);

	// Draw each of the line segments that make up the arc
	while (numsegs--) {
		d = p - centre;
		p.x = centre.x + FXmul(d.x,cosine) - FXmul(d.y,sine);
		p.y = centre.y + FXmul(d.x,sine) + FXmul(d.y,cosine);
		lineTo(p);
		}
	end = p;
}

void Quick2d::circle(const FXPoint2d& centre,real radius)
/****************************************************************************
*
* Function:		Quick2d::circle
* Parameters:	centre	- Centre point for the circle
*				radius	- radius for the circle
*
* Description:	Draws a circle that is approximated by a number of straight
*				line segments.
*
****************************************************************************/
{
	int			i;
	real		deltaAngle,cosine,sine;
	FXPoint2d	p,d;

	// Calculate the initial point on the circle, and move pen to
	// this location.
	p = centre;	p.x += radius;
	start = p;
	moveTo(p);

	// Calculate the the angular increment, and the sine and cosine of this
	// angular increment.
	deltaAngle = FXdiv(REAL(360),FXintToReal(csegs));
	FXsincos(deltaAngle,&sine,&cosine);

	// Draw each of the line segments that make up the arc
	i = csegs;
	while (i--) {
		d = p - centre;
		p.x = centre.x + FXmul(d.x,cosine) - FXmul(d.y,sine);
		p.y = centre.y + FXmul(d.x,sine) + FXmul(d.y,cosine);
		lineTo(p);
		}
	lineTo(start);		// Draw last line to ensure it is closed
}

void Quick2d::fillArc(const FXPoint2d& centre,real radius,real startAngle,
	real endAngle)
/****************************************************************************
*
* Function:		Quick2d::fillArc
* Parameters:	centre		- Centre point for the arc
*				radius		- radius for the circular arc
*				startAngle	- Starting angle (in degrees)
*				endAngle	- Ending angle (in degrees)
* Returns:		True if the arc was drawn, false if not.
*
* Description:	Fills a circular arc that is approximated by a number
*				of straight line segments. The starting and ending angles
*				are given in degrees.
*
****************************************************************************/
{
	int			i,numsegs;
	real		deltaAngle,cosine,sine;
	fxpoint_t	*NDC_array;
	FXPoint2d	p,d,r;

	// Get the memory buffer required to convert to arc's vertices
	if ((csegs+2) * sizeof(fxpoint_t) > bufSize)
		MGL_fatalError("Too many segments in arc");
	NDC_array = (fxpoint_t*)buf;

	// Calculate the initial point on the arc, and move pen to this location.
	map(*NDC_array++,r,centre);
	FXsincos(startAngle,&sine,&cosine);
	p.x = centre.x + FXmul(radius,cosine);
	p.y = centre.y + FXmul(radius,sine);
	start = p;
	map(*NDC_array++,r,start);

	// Fix the ending angle so that it is >= starting angle
	while (endAngle < startAngle)
		endAngle += REAL(360);

	// Calculate the approximate number of line segments to use when drawing
	// the arc, the angular increment, and the sine and cosine of this
	// angular increment.
	i = numsegs = FXrealToInt(FXmul(FXintToReal(csegs),
		FXdiv(endAngle - startAngle,REAL(360)) + REAL(0.5)));
	deltaAngle = FXdiv(endAngle - startAngle,FXintToReal(numsegs));
	FXsincos(deltaAngle,&sine,&cosine);

	// Transform each point in the arc, storing coordinates in the polygon
	// array.
	validateTotalMapping();
	while (i--) {
		d = p - centre;
		p.x = centre.x + FXmul(d.x,cosine) - FXmul(d.y,sine);
		p.y = centre.y + FXmul(d.x,sine) + FXmul(d.y,cosine);
		map(*NDC_array++,r,p);
		}
	end = p;

	MGLDevCtx::fillPolygon(NDC_count = numsegs+2,(fxpoint_t*)buf,
		sizeof(fxpoint_t),0,0);
}

void Quick2d::fillCircle(const FXPoint2d& centre,real radius)
/****************************************************************************
*
* Function:		Quick2d::fillCircle
* Parameters:	centre		- Centre point for the arc
*				radius		- radius for the circular arc
* Returns:		True if the circle was drawn, false if not.
*
* Description:	Fills a circle that is approximated by a number
*				of straight line segments.
*
****************************************************************************/
{
	int			i;
	real		deltaAngle,cosine,sine;
	fxpoint_t	*NDC_array;
	FXPoint2d	p,d,r;

	// Get the memory buffer required to convert to arc's vertices
	if ((NDC_count = csegs+1) * sizeof(fxpoint_t) > bufSize)
		MGL_fatalError("Too many segments in circle");
	NDC_array = (fxpoint_t*)buf;

	// Calculate the initial point on the circle
	p = centre;	p.x += radius;
	map(*NDC_array++,r,p);

	// Calculate the the angular increment, and the sine and cosine of this
	// angular increment.
	deltaAngle = FXdiv(REAL(360),FXintToReal(csegs));
	FXsincos(deltaAngle,&sine,&cosine);

	// Transform each point in the circle, storing coordinates in the polygon
	// array.
	validateTotalMapping();
	i = csegs;
	while (i--) {
		d = p - centre;
		p.x = centre.x + FXmul(d.x,cosine) - FXmul(d.y,sine);
		p.y = centre.y + FXmul(d.x,sine) + FXmul(d.y,cosine);
		map(*NDC_array++,r,p);
		}

	MGLDevCtx::fillPolygon(NDC_count,(fxpoint_t*)buf,
		sizeof(fxpoint_t),0,0);
}

void Quick2d::circleOutline(void)
/****************************************************************************
*
* Function:		Quick2d::circleOutline
*
* Description:	Draws the outline of a previously drawn circle or arc. This
*				routine MUST be called directly after drawing the circle
*				or arc, but you can change the attributes used to draw the
*				outline before calling it.
*
****************************************************************************/
{
	fxpoint_t	*NDC_array = (fxpoint_t*)buf;

	// Draw the closing line first
	MGLDevCtx::line(NDC_array[0].x,NDC_array[0].y,NDC_array[NDC_count-1].x,
		NDC_array[NDC_count-1].y);

	// Draw the remaining lines in the outline
	while (--NDC_count) {
		MGLDevCtx::line(NDC_array[0].x,NDC_array[0].y,NDC_array[1].x,
			NDC_array[1].y);
		NDC_array++;
		}
}

int Quick2d::setCirclePrecision(int nsegs)
/****************************************************************************
*
* Function:		Quick2d::setCirclePrecision
* Parameters:	nsegs	- New ellipse/ellipse arc precision
* Returns:		Current ellipse precision.
*
****************************************************************************/
{
	int	retval = csegs;
	csegs = nsegs;
	return retval;
}

void Quick2d::getArcCoords(FXPoint2d& _start,FXPoint2d& _end)
/****************************************************************************
*
* Function:		Quick2d::getArcCoords
* Parameters:	_start	- Starting point on ellipse
*				_end	- Ending point on ellipse
*
****************************************************************************/
{
	_start = start;
	_end = end;
}
