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
* Description:	Implementation for the validView function.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mprogram.hpp"

/*----------------------------- Implementation ----------------------------*/

MVView *MV_validView(MVView *view)
/****************************************************************************
*
* Function:		MV_validView
* Parameters:	view	- View to check for validity
* Returns:		Pointer to the view if valid, NULL if not.
*
* Description:	Checks to see if the safety pool has been hit during
*				memory allocation, or the view was not validly constructed.
*				If either is the case, the view is destroyed and the
*				appropriate action is taken (returning NULL for the
*				view pointer).
*
****************************************************************************/
{
	if (view) {
		if (MV_lowMemory()) {
			delete view;

			// Only call the outOfMemory() routine _iff_ the application
			// instance has been successfully initialised. This will
			// be true only if the application pointer is not NULL.
			if (MVProgram::application)
				MVProgram::application->outOfMemory();
			return NULL;
			}
		if (!view->valid(cmValid)) {
			delete view;
			return NULL;
			}
		}
	return view;
}
