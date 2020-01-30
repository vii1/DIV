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
* Description:	Header file for the TDeskTop class. This class is a
*				special MVGroup that maintains all of the views used by
*				the program, along with a simple background view if
*				specified.
*
*
****************************************************************************/

#ifndef	__MVIS_MDESKTOP_HPP
#define	__MVIS_MDESKTOP_HPP

#ifndef	__MVIS_MGROUP_HPP
#include "mvis/mgroup.hpp"
#endif

#ifndef	__MVIS_MBACKGND_HPP
#include "mvis/mbackgnd.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVDeskTop class is a special MVGroup to group together all views used
// by the main program.
//---------------------------------------------------------------------------

class MVDeskTop : public MVGroup {
protected:
	MVBackGround	*backGround;

			// Method to draw the background
	virtual	void drawBackground(const MVRect& dirty);

public:
			// Constructor
			MVDeskTop(MGLDevCtx& dc,const MVRect& bounds,
				MVBackGround *backGround = NULL);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase = phFocused);
	};

#endif	// __MVIS_MESKTOP_HPP
