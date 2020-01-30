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
* Description:  3d line drawing routines.
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick3d::moveTo(const FXPoint3d& p)
/****************************************************************************
*
* Function:		Quick3d::moveTo
* Parameters:	p	- Point to move to
*
* Description:	Moves the CP to the new position. We transform the point
*				into NDC coords and cache this value.
*
****************************************************************************/
{
	FXPoint4d	r;
	validateTotalMapping();
	map(CPfx,r,p);
	CP = p;
}

void Quick3d::moveRel(const FXVec3d& p)
{
	CP += p;
	moveTo(CP);
}

void Quick3d::lineTo(const FXPoint3d& p)
/****************************************************************************
*
* Function:		Quick3d::lineTo
* Parameters:	p	- Point to draw line to
*
* Description:	Draws a line from the CP, to the new point p, moving the
*				CP to the position that p was in.
*
****************************************************************************/
{
	FXPoint4d	r;
	fxpointz_t	f;

	validateTotalMapping();
	map(f,r,p);
	if (hsrMode == ZBufferHSR)
		MGLDevCtx::zLine(CPfx,f);
	else
		MGLDevCtx::line(CPfx.p,f.p);
	CP = p;
	CPfx = f;
}

void Quick3d::lineRel(const FXVec3d& p)
{
	CP += p;
	lineTo(CP);
}

void Quick3d::lineFast(const FXPoint3d& p1,const FXPoint3d& p2)
/****************************************************************************
*
* Function:		Quick3d::lineFast
* Parameters:	p1	- First point in line
*				p2	- Second point in line
*
* Description:	Draws a line from one point to another point.
*
****************************************************************************/
{
	FXPoint4d	r;
	fxpointz_t	f1,f2;

	validateTotalMapping();
	map(f1,r,p1);
	map(f2,r,p2);
	if (hsrMode == ZBufferHSR)
		MGLDevCtx::zLine(f1,f2);
	else
		MGLDevCtx::line(f1.p,f2.p);
}

void Quick3d::line(const FXPoint3d& p1,const FXPoint3d& p2)
{
	lineFast(p1,p2);
}
