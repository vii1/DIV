/****************************************************************************
*
*					 MegaVision Application Framework
*
*			A C++ GUI Toolkit for the MegaGraph Graphics Library
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
* Environment:	IBM PC (MS DOS)
*
* Description:	Member functions for the TProgram class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mbackgnd.hpp"
#include <string.h>

/*----------------------------- Implementation ----------------------------*/

MVBackGround::MVBackGround(MGLDevCtx& dc,const MVRect& bounds,int fillStyle,
	pattern_t *pat)
	: MVView(dc,bounds), fillStyle(fillStyle)
/****************************************************************************
*
* Function:		MVBackGround::MVBackGround
* Parameters:	bounds		- Bounding box for the background
*				fillStyle	- Style to fill the background with
*				pat			- Pattern to fill the background with
*
* Description:	Constructor for the background class.
*
****************************************************************************/
{
	memcpy(MVBackGround::pat,pat,sizeof(pattern_t));
}

MVBackGround::~MVBackGround()
{
}

void MVBackGround::draw(const MVRect& dirty)
/****************************************************************************
*
* Function:		MVBackGround::draw
* Parameters:	clip	- Clip rectangle for the view
*
* Description:	Method to draw the background view.
*
****************************************************************************/
{
	int oldPenStyle = dc.getPenStyle();
	dc.setBackColor(getColor(scDesktopBackground));
	dc.setColor(getColor(scDesktopForeground));
	dc.setPenStyle(fillStyle);
	dc.setPenBitmapPattern((const pattern_t *)&pat);
	dc.fillRect(dirty);
	dc.setPenStyle(oldPenStyle);
}
