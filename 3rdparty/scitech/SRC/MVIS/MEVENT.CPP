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
* Description:	Module to implement the mouse and keyboard event collection
*				queue routines for the toolkit.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mevent.hpp"
#include <string.h>

/*--------------------------- Global Variables ----------------------------*/

MVEventQueue	MV_eventQueue;

/*----------------------------- Implementation ----------------------------*/

MVEventQueue::MVEventQueue()
/****************************************************************************
*
* Function:		MVEventQueue::MVEventQueue
*
* Description:	Constructor for the event queue. We simply initialise the
*				event gathering subsystem, and set a few internal flags
*				to zero.
*
****************************************************************************/
{
	// Default values for double click speed and mouse auto repeat rate
	tickConvert = (long)(1.0 / (18.2 * (MGL_getTickResolution() / 1000000.0)));
	doubleClick = 8 * tickConvert;
	autoRepeat = 0;
	autoDelay = 6 * tickConvert;
	doubleClickThresh = 5;
	firstAuto = true;

	memset(&downMouse,0,sizeof(downMouse));
	memset(&autoMousePos,0,sizeof(autoMousePos));
}

ibool MVEventQueue::getNext(MVEvent& evt,ushort mask)
/****************************************************************************
*
* Function:		MVEventQueue::getNext
* Parameters:	evt		- Place to store the event
*				mask	- Event mask
* Returns:		True if an event was found, false if no events pending.
*
* Description:	Returns the next instance of the event from the event
*				queue, removing it from the queue. In here we handle the
*				double click and mouse auto repeat events.
*
****************************************************************************/
{
	if (EVT_getNext((event_t*)&evt,mask)) {
		if (evt.what & evMouse) {
			if (evt.what & evMouseMove) {
				autoMousePos = evt.where;
				return true;
				}
			if ((evt.what & evMouseDown) == 0) {
				downMouse.what = evNothing;
				firstAuto = true;
				return true;
				}

			// Determine if the last mouse event was a double click event
			MVPoint diff(evt.where - downMouse.where);
			diff.x = ABS(diff.x);	diff.y = ABS(diff.y);

			if (!downMouse.mouse.doubleClick
				&& (evt.mouse.buttons == downMouse.mouse.buttons)
				&& (evt.when - downMouse.when) <= doubleClick
				&& (diff.x <= doubleClickThresh)
				&& (diff.y <= doubleClickThresh)) {
				evt.mouse.doubleClick = true;
				}
			downMouse = evt;
			autoMousePos = evt.where;
			autoTicks = MGL_getTicks();
			}
		return true;
		}

	if (autoRepeat && (mask & evMouseAuto) && (downMouse.what & evMouseDown)) {
		// No events are pending and the mouse is currently down, so
		// check to see if we should generate an auto mouse down event
		long ticks = MGL_getTicks();
		if ((ticks - autoTicks) >= (autoRepeat + (firstAuto ? autoDelay : 0))) {
			evt = downMouse;
			evt.what = evMouseAuto;
			evt.where = autoMousePos;
			autoTicks = evt.when = ticks;
			firstAuto = false;
			return true;
			}
		}

	evt.what = evNothing;
	return false;
}

ibool MVEventQueue::post(const MVEvent& evt)
/****************************************************************************
*
* Function:		MVEventQueue::post
* Parameters:	evt	- Event to post to the event queue
* Returns:		True if the event was posted, false if queue is full.
*
* Description:	Posts an event to the event queue. If the event is a double
*				click event, we simulate this by duplicating a mouse down
*				event.
*
****************************************************************************/
{
	if (evt.what & evKeyboard)
		return EVT_post(0,evt.what,evt.key.keyCode,evt.key.modifiers);

	if (evt.what & evMouse) {
		if (!EVT_post(0,evt.what,evt.mouse.buttons,evt.mouse.modifiers))
			return false;
		if (evt.mouse.doubleClick)
			return EVT_post(0,evt.what,evt.mouse.buttons,evt.mouse.modifiers);
		}
	return EVT_post((ulong)evt.which,evt.what,evt.message.command,evt.message.infoLong);
}
