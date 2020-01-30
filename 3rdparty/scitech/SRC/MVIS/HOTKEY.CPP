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
* Description:	Implementation for the MVHotKey class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mkeys.hpp"

ibool MVHotKey::operator == (const MVKeyEvent& event) const
/****************************************************************************
*
* Function:		MVHotKey::operator ==
* Parameters:	event	- Keyboard event to check
* Returns:		True if the hotkey value is present
*
****************************************************************************/
{
	if (event.keyCode != keyCode)
		return false;

	if (modifiers & mdShift) {
		// If no shift key is down then this is not out hot key (shift is
		// present in the hot key)
		if (!(event.modifiers & mdShift))
			return false;
		}

	// If we get here, just return true if the Ctrl and Alt flags are
	// equal to the ones in the modifiers mask for this hot key
	return ((ushort)event.modifiers & (mdCtrl | mdAlt)) ==
		(modifiers & (mdCtrl | mdAlt));
}
