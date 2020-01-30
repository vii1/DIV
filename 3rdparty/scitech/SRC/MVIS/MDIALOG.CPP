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
* Description:	Member functions for the MVDialog class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mdialog.hpp"
#include "mvis/mkeys.hpp"

/*----------------------------- Implementation ----------------------------*/

MVDialog::MVDialog(MGLDevCtx& dc,const MVRect& bounds,const char *title,
	ushort flags)
	: MVWindow(dc,bounds,title,flags),
	  MVWindowInit(MVWindow::initFrame,MVWindow::initTitleBar)
/****************************************************************************
*
* Function:		MVDialog::MVDialog
* Parameters:	bounds	- Bounding box for the dialog
*				title	- Title text for the dialog (NULL for no title bar)
*				tjust	- Text justification for title bar
*				flags	- Window creation flags
*
* Description:	Constructor for the MVDialog class.
*
****************************************************************************/
{
}

void MVDialog::handleEvent(MVEvent& event,phaseType phase)
/****************************************************************************
*
* Function:		MVDialog::handleEvent
* Parameters:	event	- Event to handle
*				phase	- Current phase for the event (pre,focus,post)
*
* Description:	Event handling routine for the MVDialog class. Here we
*				handle events normally for the group, and also check to
*				see if the dialog box should be ended.
*
***************************************************************************/
{
	MVWindow::handleEvent(event,phase);

	switch (event.what) {
		case evKeyDown:
			switch (event.key.keyCode) {
				case kbEsc:
					// The escape key was pressed, so post a cancel message
					// to kill the dialog box.
					event.what = evCommand;
					event.message.command = cmCancel;
					event.message.infoPtr = NULL;
					putEvent(event);
					clearEvent(event);
					break;
				case kbEnter:
				case kbGrayEnter:
					// The enter key was pressed, so post a cmDefault
					// message to be processed by the default view (must
					// be a broadcast as we do not know who is the
					// default view.
					event.what = evBroadcast;
					event.message.command = cmDefault;
					event.message.infoPtr = NULL;
					putEvent(event);
					clearEvent(event);
					break;
				}
			break;
		case evCommand:
			if (event.message.command == cmClose && event.which == this)
				event.message.command = cmCancel;
			switch (event.message.command) {
				case cmYes:
				case cmNo:
				case cmOk:
				case cmCancel:
					// One of the standard dialog commands was posted,
					// so attempt to end the modal view with the command.
					if (state & sfModal) {
						endModal(event.message.command);
						clearEvent(event);
						}
					break;
				}
			break;
		}
}

ibool MVDialog::valid(ushort command)
/****************************************************************************
*
* Function:		MVDialog::valid
* Parameters:	command	- Command ending the modal operation
* Returns:		True if the dialog is valid
*
* Description:	Checks to see if the dialog is valid, or the modal view
*				should end.
*
****************************************************************************/
{
	if (command == cmCancel)
		return true;					// Cancel always works
	else
		return MVGroup::valid(command);
}
