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
* Description:	Header file for the MVDialog class. This class is used
*				to group together a number of views into a pop-up box
*				used for displaying messages etc.
*
*
****************************************************************************/

#ifndef	__MVIS_MDIALOG_HPP
#define	__MVIS_MDIALOG_HPP

#ifndef	__MVIS_MWINDOW_HPP
#include "mvis/mwindow.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

const ushort

// Default MVDialog flags

	wfNormalDialog	= wfMoveable | wfSystemMenu | wfClose | wfFramed
						| wfLeftJust;

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVDialog class is a type of window that is designed to contain
// buttons and other interactive views. By default MVDialogs are moveable
// but not-resizeable and modal.
//---------------------------------------------------------------------------

class MVDialog : public MVWindow {
public:
			// Constructor
			MVDialog(MGLDevCtx& dc,const MVRect& bounds,const char *title = NULL,
				ushort flags = wfNormalDialog);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase = phFocused);

			// Virtual to determine if the dialog has finished
	virtual	ibool valid(ushort command);
	};

#endif	// __MVIS_MDIALOG_HPP
