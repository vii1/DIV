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
* Description:	Implementation for the message function.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mview.hpp"

/*----------------------------- Implementation ----------------------------*/

MVView *MV_message(MVView *view,ushort what,ushort command,void *info)
/****************************************************************************
*
* Function:		MV_message
* Parameters:	view	- View to send the message to
*				what	- Event code for the message
*				command	- Command code for the message
*				info	- Pointer to optional information for the message
* Returns:		Pointer to the view that handled the message, NULL if none
*
* Description:	Sends a message to the specified view to be handled
*				immediately. This function does not return until the view
*				has finished processing the message.
*
****************************************************************************/
{
	if (view == NULL)
		return NULL;

	MVEvent event;
	event.which = view;
	event.what = what;
	event.message.command = command;
	event.message.infoPtr = info;

	view->setupOwnerViewport();
	view->handleEvent(event);
	view->resetViewport();

	// If the event was handled, the what field will be set to evNothing,
	// so return a pointer to the view that handled the message.
	if (event.what == evNothing)
		return (MVView*)event.message.infoPtr;
	return NULL;
}

void MV_postMessage(MVView *view,ushort what,ushort command,void *info)
/****************************************************************************
*
* Function:		MV_postMessage
* Parameters:	view	- View to send the message to
*				what	- Event code for the message
*				command	- Command code for the message
*				info	- Pointer to optional information for the message
*
* Description:	Post a message to the application event queue to be handled
*				at a later date. This should be used for events that do
*				not need to be handled immediately, and may change the global
*				state of windows on the desktop. It will return immediately
*				and the message will be handled later.
*
*				If the view parameter is NULL, the message will be broadcast
*				to all views, otherwise it will be directed to a specific
*				view.
*
****************************************************************************/
{
	MVEvent event;
	event.which = view;
	event.what = what;
	event.message.command = command;
	event.message.infoPtr = info;
	MV_eventQueue.post(event);
}
