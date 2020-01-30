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
* Description:	Member functions for the MVLabel class. The MVLabel class
*				is a special type of static text label that when clicked
*				can be used to selected another view (such as a list box
*				or input line).
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/msttext.hpp"

/*----------------------------- Implementation ----------------------------*/

void MVLabel::handleEvent(MVEvent& event,phaseType)
/****************************************************************************
*
* Function:		MVLabel::handleEvent
* Parameters:	event	- Event to handle
*				phase	- Current phase for the event (pre,focus,post)
*
* Description:	Event handling routine for the MVLabel class. We simply
*				determine if the static text item has been clicked,
*				selecting the attached view if it was.
*
****************************************************************************/
{
	if (event.what == evMouseDown && (event.mouse.buttons & mbLeftButton)) {
		if (attached && !(attached->getState() & sfDisabled))
			attached->select();
		clearEvent(event);
		}
}
