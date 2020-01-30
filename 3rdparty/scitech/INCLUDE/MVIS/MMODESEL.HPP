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
* Description:	Header file for the MVModeSelector class. This class is used
*				to represent a dialog box for selecting a valid new
*				video mode to put the system into.
*
*
****************************************************************************/

#ifndef	__MVIS_MMODESEL_HPP
#define	__MVIS_MMODESEL_HPP

#ifndef	__MVIS_MDIALOG_HPP
#include "mvis/mdialog.hpp"
#endif

#ifndef	__MVIS_MINPUTL_HPP
#include "mvis/minputl.hpp"
#endif

#ifndef	__MVIS_MLIST_HPP
#include "mvis/mlist.hpp"
#endif

#ifndef	__TCL_ARRAY_HPP
#include "tcl/array.hpp"
#endif

#ifndef	__TCL_STR_HPP
#include "tcl/str.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVModeSelector class is designed to provide a standard mechanism for
// allowing users to select a new video mode.
//
// Returns cmOK if a valid selection was made.
//---------------------------------------------------------------------------

class MVModeSelector : public MVDialog {
protected:
	int					driver;			// Video drive number
	int					mode;			// Current video mode number
	MVList				*modeList;		// List of video modes
	TCISArray<TCDynStr>	modenames;		// Array of video mode names

	static const char *setText;		// Text for standard buttons
	static const char *helpText;
	static const char *cancelText;
	static const char *modeListText;

public:
			// Constructor
			MVModeSelector(MGLDevCtx &dc,const char *title,int driver,
				int mode,uchar *validModes);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase);

			// Method to obtain the selected video mode
			int getMode();
	};

#endif	// __MVIS_MMODESEL_HPP
