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
* Description:	Member functions for the MVCheckBox class
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mcheckbx.hpp"

/*----------------------------- Implementation ----------------------------*/

PRIVATE	int	checkIndexes[32] = {
	sbCheck,						// normal
	sbCheckSel,						// selected
	sbCheckDown,					// pressed
	sbCheckDownSel,					// pressed selected
	sbCheckDisable,					// disabled
	sbCheckDisableSel,				// disabled selected
	sbCheckDisable,					// disabled pressed
	sbCheckDisableSel,				// disabled pressed selected

	sbCheck,						// focused normal
	sbCheckSel,						// focused selected
	sbCheckDown,					// focused pressed
	sbCheckDownSel,					// focused pressed selected
	sbCheckDisable,					// focused disabled
	sbCheckDisableSel,				// focused disabled selected
	sbCheckDisable,					// focused disabled pressed
	sbCheckDisableSel,				// focused disabled pressed selected

	sbCheck,						// default normal
	sbCheckSel,						// default selected
	sbCheckDown,					// default pressed
	sbCheckDownSel,					// default pressed selected
	sbCheckDisable,					// default disabled
	sbCheckDisableSel,				// default disabled selected
	sbCheckDisable,					// default disabled pressed
	sbCheckDisableSel,				// default disabled pressed selected

	sbCheck,						// default focused normal
	sbCheckSel,						// default focused selected
	sbCheckDown,					// default focused pressed
	sbCheckDownSel,					// default focused pressed selected
	sbCheckDisable,					// default focused disabled
	sbCheckDisableSel,				// default focused disabled selected
	sbCheckDisable,					// default focused disabled pressed
	sbCheckDisableSel,				// default focused disabled pressed selected
	};

MVCheckBox::MVCheckBox(MGLDevCtx& dc,const MVRect& bounds,
	ushort command,ushort flags,ibool selected)
	: MVSysBitmapButton(dc,bounds,command,checkIndexes,flags | bfTwoState,
	  selected)
/****************************************************************************
*
* Function:		MVCheckBox::MVCheckBox
* Parameters:	bounds		- Bounding rectangle for the button
*				command		- Command code to send when activated
*				flags		- Flags for the button
*
****************************************************************************/
{
}
