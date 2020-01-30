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
* Description:  2d line drawing methods. All transformations are done to
*               32 bit fixed point values, and we draw stuff using the
*				MGL's 32 bit fixed point line drawing routines for higher
*				precision.
*
*
****************************************************************************/

#include "quick2d/quick2d.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick2d::moveTo(const FXPoint2d& p)
/****************************************************************************
*
* Function:		Quick2d::moveTo
* Parameters:	p	- Point to move to
*
* Description:	Moves the CP to the new position. We transform the point
*				into NDC coords and cache this value.
*
****************************************************************************/
{
	FXPoint2d	r;
	validateTotalMapping();
	map(CPfx,r,p);
	CP = p;
}

void Quick2d::moveRel(const FXVec2d& p)
{
	CP += p;
	moveTo(CP);
}

void Quick2d::lineTo(const FXPoint2d& p)
/****************************************************************************
*
* Function:		Quick2d::lineTo
* Parameters:	p	- Point to draw line to
*
* Description:	Draws a line from the CP, to the new point p, moving the
*				CP to the position that p was in.
*
****************************************************************************/
{
	FXPoint2d	r;
	fxpoint_t   newCPfx;

	validateTotalMapping();
	map(newCPfx,r,p);
	MGLDevCtx::line(CPfx.x,CPfx.y,newCPfx.x,newCPfx.y);
	CP = p;
	CPfx = newCPfx;
}

void Quick2d::lineRel(const FXVec2d& p)
{
	CP += p;
	lineTo(CP);
}

void Quick2d::line(const FXPoint2d& p1,const FXPoint2d& p2)
{
	line(p1,p2);
}
