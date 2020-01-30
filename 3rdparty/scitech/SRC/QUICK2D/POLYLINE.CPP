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
* Description:  2d polyline drawing routines.
*
*
****************************************************************************/

#include "quick2d/quick2d.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick2d::marker(const FXPoint2d& p)
/****************************************************************************
*
* Function:		Quick2d::marker
* Parameters:	p	- Point to draw marker at
*
* Description:	Draws a marker in the current marker color and style at the
*				specified point.
*
****************************************************************************/
{
	FXPoint2d	r;
	MGLPoint	NDCp;

	validateTotalMapping();
	map(NDCp,r,p);
	MGLDevCtx::marker(NDCp);
}

void Quick2d::polyMarker(int count,const FXPoint2d *vArray)
/****************************************************************************
*
* Function:		Quick2d::polyMarker
* Parameters:	count	- Number of points in array
*				vArray	- Vertex array of points
*
* Description:	Draws a marker at every position specified in the vertex
*				array.
*
****************************************************************************/
{
	FXPoint2d	r;
	MGLPoint	NDCp;

	validateTotalMapping();
	while (count--) {
		map(NDCp,r,*vArray++);
		MGLDevCtx::marker(NDCp);
		}
}

void Quick2d::polyLine(int count,const FXPoint2d *vArray)
/****************************************************************************
*
* Function:		Quick2d::polyLine
* Parameters:	count	- Number of points in array
*				vArray	- Vertex array of points
*
* Description:	Draws a polyline in the current drawing attributes.
*
****************************************************************************/
{
	FXPoint2d	r;
	fxpoint_t   prev,next;

	validateTotalMapping();
	map(prev,r,*vArray++);
	while (--count) {
		map(next,r,*vArray++);
		MGLDevCtx::line(prev.x,prev.y,next.x,next.y);
		prev = next;
		}
}

void Quick2d::polyLineMarker(int count,const FXPoint2d *vArray)
/****************************************************************************
*
* Function:		Quick2d::polyLineMarker
* Parameters:	count	- Number of points in array
*				vArray	- Vertex array of points
*
* Description:	Draws a polyline with markers superimposed over the top.
*
****************************************************************************/
{
	FXPoint2d	r;
	fxpoint_t   prev,next;

	validateTotalMapping();
	map(prev,r,*vArray++);
	while (--count) {
		map(next,r,*vArray++);
		MGLDevCtx::line(prev.x,prev.y,next.x,next.y);
		MGLDevCtx::marker(MGLPoint(MGL_FIXROUND(prev.x),MGL_FIXROUND(prev.y)));
		prev = next;
		}
	MGLDevCtx::marker(MGLPoint(MGL_FIXROUND(prev.x),MGL_FIXROUND(prev.y)));
}
