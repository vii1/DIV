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

#include "mvis/mdesktop.hpp"

/*----------------------------- Implementation ----------------------------*/

MVDeskTop::MVDeskTop(MGLDevCtx& dc,const MVRect& bounds,
	MVBackGround *backGround)
	: MVGroup(dc,bounds), backGround(backGround)
/****************************************************************************
*
* Function:		MVDeskTop::MVDeskTop
* Parameters:	bounds		- Bounding rectangle for the desktop
*				backGround	- Pointer to background view to use
*
* Description:
*
****************************************************************************/
{
	if (backGround)
		insert(backGround);
}

void MVDeskTop::handleEvent(MVEvent& event,phaseType phase)
/****************************************************************************
*
* Function:		MVDeskTop::handleEvent
* Parameters:	event	- Event to handle
*				phase	- Current phase for the event (pre,focus,post)
*
* Description:	Event handling routine for the TDesktop class. Here we
*				handle the events to move the active window.
*
****************************************************************************/
{
	MVGroup::handleEvent(event,phase);

	if (event.what == evCommand) {
		switch (event.message.command) {
			case cmMoveToFront:
				MVView *view = (MVView*)event.message.infoPtr;
				if (firstView() != view) {
					remove(view);
					insert(view);
					}
				select(view);
				break;

			// Here we handle the cmNext, cmPrev messages.

			}
		}
}

void MVDeskTop::drawBackground(const MVRect& clip)
/****************************************************************************
*
* Function:		MVDeskTop::drawBackground
* Parameters:	clip	- Clipping rectangle for the view.
*
* Description:	Draws the desktop's background if present.
*
****************************************************************************/
{
	if (backGround)
		backGround->draw(clip);
}
