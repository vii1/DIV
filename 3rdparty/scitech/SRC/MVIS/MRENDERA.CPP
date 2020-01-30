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
* Description:	Member functions for the MVRenderArea class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mrendera.hpp"
#include "mvis/mprogram.hpp"
#include "tcl/techlib.hpp"

/*----------------------------- Implementation ----------------------------*/

MVRenderArea::MVRenderArea(MGLDevCtx& dc,const MVRect& bounds)
	: MVView(dc,bounds)
/****************************************************************************
*
* Function:		MVRenderArea::MVRenderArea
* Parameters:	bounds	- Bounding box for the render area
*
* Description:	Constructor for the MVRenderArea class.
*
****************************************************************************/
{
	PRECONDITION(MGL_getActivePage(dc.getDC()) == 0 && MGL_getVisualPage(dc.getDC()) == 0);
	doubleBuffered = false;
}

MVRenderArea *MVRenderArea::clipAndRenderView;

void _ASMAPI MVRenderArea::clipAndRender(const rect_t *r)
{
	rect_t	offsetR = *r;
	MGL_offsetRect(offsetR,
		-clipAndRenderView->bounds.left(),
		-clipAndRenderView->bounds.top());
	clipAndRenderView->dc.setClipRect(offsetR);
	clipAndRenderView->render();
}

void MVRenderArea::paint()
/****************************************************************************
*
* Function:		MVRenderArea::paint
*
* Description:	This function paints the view with the current clip
*				region. We set the viewport to the coordinate space for
*				the view and then call the render function clipping to
*				each of the clip rectangles in the clip region.
*
****************************************************************************/
{
	MS_obscure();
	setupViewport();
	clipAndRenderView = this;
	clipRgn.traverse(clipAndRender);
	resetViewport();
	MS_show();
}

void MVRenderArea::doubleBufferOn()
/****************************************************************************
*
* Function:		MVRenderArea::doubleBufferOn
*
* Description:	Turns on double buffering for the render area. This member
*				function should be called just before peforming any
*				animation in the render area.
*
****************************************************************************/
{
	if (dc.doubleBuffer()) {
		MVEvent  evt;

		// Force a quick repaint onto the hidden display page
		doubleBuffered = true;
		MVProgram::application->quickRepaint();
		getEvent(evt,evRepaint);
		}
}

void MVRenderArea::doubleBufferOff()
/****************************************************************************
*
* Function:		MVRenderArea::doubleBufferOff
*
* Description:	Turns off double buffering. The visual display page will
*				not necessarily be zero after this.
*
****************************************************************************/
{
	dc.singleBuffer();
}
