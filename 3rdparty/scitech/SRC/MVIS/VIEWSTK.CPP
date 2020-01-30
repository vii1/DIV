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
* Description:	Implementation of the viewport stack. This code used to be
*				in the MGL, but was removed for the MGL 2.0 so we implement
*				some compatability routines in here using a the C++ stack
*				class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "tcl/cstack.hpp"

/*----------------------------- Implementation ----------------------------*/

struct ClipAndView {
	MVRect	view;			// Original viewport before save
	MVRect	clip;			// Original clip rectangle before save
	};

PRIVATE	TCStaticCStack<ClipAndView,20>	viewStack;
PRIVATE	TCStaticCStack<MVRect,20>		clipStack;

void MV_pushViewport(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		MV_pushViewport
*
* Description:	Pushes the currently active viewport onto the stack. If the
*				viewport stack overflows, we bomb out with an error message.
*
****************************************************************************/
{
	ClipAndView	v;

	if (viewStack.isFull())
		MGL_fatalError("Viewport stack overflow!");
	dc.getViewport(v.view);
	dc.getClipRect(v.clip);
	viewStack.pushq(v);
}

void MV_popViewport(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		MV_popViewport
*
* Description:	Pops the viewport off the top of the stack and makes it
*				the new active viewport. If the viewport stack underflows,
*				we bomb out with an error message. Note that we also
*				restore the original clipping rectangle as well.
*
****************************************************************************/
{
	if (viewStack.isEmpty())
		MGL_fatalError("Viewport stack underflow!");
	ClipAndView v(viewStack.popq());
	dc.setViewport(v.view);
	dc.setClipRect(v.clip);
}

void MV_pushClipRect(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		MV_pushClipRect
*
* Description:	Pushes the currently active clip rectangle onto the stack
*				individually to the viewport. If the ClipRect stack
*				overflows, we bomb out with an error message.
*
****************************************************************************/
{
	MVRect	clip;

	if (clipStack.isFull())
		MGL_fatalError("ClipRect stack overflow!");
	dc.getClipRect(clip);
	clipStack.pushq(clip);
}

void MV_popClipRect(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		MV_popClipRect
*
* Description:	Pops the ClipRect off the top of the stack and makes it
*				the new active ClipRect. If the ClipRect stack underflows,
*				we bomb out with an error message. Note that we also
*				restore the original clipping rectangle as well.
*
****************************************************************************/
{
	if (clipStack.isEmpty())
		MGL_fatalError("ClipRect stack underflow!");
	dc.setClipRect(clipStack.popq());
}
