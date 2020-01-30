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
* Description:  2d rectangle drawing routines.
*
*
****************************************************************************/

#include "quick2d/quick2d.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick2d::rect(const FXPoint2d& ul,const FXPoint2d& lr)
/****************************************************************************
*
* Function:		Quick2d::rect
* Parameters:	ul	- Upper left point for rectangle
*				lr	- Lower right point for rectangle
*
* Description:	Draws a rectangle.
*
****************************************************************************/
{
	FXPoint2d	result;
	fxpoint_t	p[4];
	MGLRect		r;

	validateTotalMapping();
	if (m.isSpecial()) {
		map(r.topLeft,result,ul);
		map(r.botRight,result,lr);
		r.botRight.x += 1;	r.botRight.y += 1;
		MGLDevCtx::rect(r);
		}
	else {
		map(p[0],result,ul);
		map(p[1],result,FXPoint2d(lr.x,ul.y));
		map(p[2],result,lr);
		map(p[3],result,FXPoint2d(ul.x,lr.y));
		MGLDevCtx::line(p[0].x,p[0].y,p[1].x,p[1].y);
		MGLDevCtx::line(p[1].x,p[1].y,p[2].x,p[2].y);
		MGLDevCtx::line(p[2].x,p[2].y,p[3].x,p[3].y);
		MGLDevCtx::line(p[3].x,p[3].y,p[0].x,p[0].y);
		}
}

void Quick2d::rect(real left,real top,real right,real bottom)
{ rect(FXPoint2d(left,top),FXPoint2d(right,bottom)); }

void Quick2d::fillRect(const FXPoint2d& ul,const FXPoint2d& lr)
/****************************************************************************
*
* Function:		Quick2d::fillRectFast
* Parameters:	ul	- Upper left point for rectangle
*				lr	- Lower right point for rectangle
*
* Description:	Fills a rectangle. Shared edges are not drawn.
*
****************************************************************************/
{
	FXPoint2d	result;
	fxpoint_t	p[4];
	MGLRect		r;

	validateTotalMapping();
	if (m.isSpecial()) {
		map(r.topLeft,result,ul);
		map(r.botRight,result,lr);
		r.botRight.x += 1;	r.botRight.y += 1;
		MGLDevCtx::fillRect(r);
		}
	else {
		map(p[0],result,ul);
		map(p[1],result,FXPoint2d(lr.x,ul.y));
		map(p[2],result,lr);
		map(p[3],result,FXPoint2d(ul.x,lr.y));
		MGLDevCtx::fillPolygon(4,p,sizeof(fxpoint_t),0,0);
		}
}

void Quick2d::fillRect(real left,real top,real right,real bottom)
{ fillRect(FXPoint2d(left,top),FXPoint2d(right,bottom)); }
