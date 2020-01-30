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
* Description:	Member functions for the MVRadioButton class
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/msgbox.hpp"
#include "mvis/mgroup.hpp"
#include "mvis/mradio.hpp"

/*----------------------------- Implementation ----------------------------*/

PRIVATE	int	radioIndexes[32] = {
	sbRadio,						// normal
	sbRadioSel,						// selected
	sbRadioDown,					// pressed
	sbRadioDownSel,					// pressed selected
	sbRadioDisable,					// disabled
	sbRadioDisableSel,				// disabled selected
	sbRadioDisable,					// disabled pressed
	sbRadioDisableSel,				// disabled pressed selected

	sbRadio,						// focused normal
	sbRadioSel,						// focused selected
	sbRadioDown,					// focused pressed
	sbRadioDownSel,					// focused pressed selected
	sbRadioDisable,					// focused disabled
	sbRadioDisableSel,				// focused disabled selected
	sbRadioDisable,					// focused disabled pressed
	sbRadioDisableSel,				// focused disabled pressed selected

	sbRadio,						// default normal
	sbRadioSel,						// default selected
	sbRadioDown,					// default pressed
	sbRadioDownSel,					// default pressed selected
	sbRadioDisable,					// default disabled
	sbRadioDisableSel,				// default disabled selected
	sbRadioDisable,					// default disabled pressed
	sbRadioDisableSel,				// default disabled pressed selected

	sbRadio,						// default focused normal
	sbRadioSel,						// default focused selected
	sbRadioDown,					// default focused pressed
	sbRadioDownSel,					// default focused pressed selected
	sbRadioDisable,					// default focused disabled
	sbRadioDisableSel,				// default focused disabled selected
	sbRadioDisable,					// default focused disabled pressed
	sbRadioDisableSel,				// default focused disabled pressed selected
	};

MVRadioButton::MVRadioButton(MGLDevCtx& dc,const MVRect& bounds,
	ushort command,ushort flags,ibool selected)
	: MVSysBitmapButton(dc,bounds,command,radioIndexes,flags | bfSelectable,
	  selected)
/****************************************************************************
*
* Function:		MVRadioButton::MVRadioButton
* Parameters:	bounds		- Bounding rectangle for the button
*				command		- Command code to send when activated
*				flags		- Flags for the button
*
****************************************************************************/
{
}

void MVRadioButton::buttonClicked()
/****************************************************************************
*
* Function:		MVRadioButton::buttonClicked
*
* Description:	Send both button clocked and radio selected messages to
*				owner window.
*
****************************************************************************/
{
	MV_message(owner,evBroadcast,cmButtonClicked,this);
	MV_message(owner,evBroadcast,cmRadioSelected,this);
}

void MVRadioButton::handleEvent(MVEvent& event,phaseType phase)
/****************************************************************************
*
* Function:		MVRadioButton::handleEvent
* Parameters:	event	- Event to handle
*				phase	- Current phase for the event (pre,focus,post)
*
* Description:	Event handling routine for radio buttons.
*
****************************************************************************/
{
	MVSysBitmapButton::handleEvent(event,phase);

	/* Handle auto-matic deselection of radio buttons when a new one is
	 * selected in the group.
	 */
	if (event.what == evBroadcast && event.message.command == cmRadioSelected) {
		if ((MVView*)event.message.infoPtr != this) {
			if (selected) {
				selected = false;
				draw(bounds);
				}
			}
		}
}
