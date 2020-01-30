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
* Description:	Header file for the MVRadioButton class.
*
*
****************************************************************************/

#ifndef	__MVIS_MRADIO_HPP
#define	__MVIS_MRADIO_HPP

#ifndef	__MVIS_MBITBTN_HPP
#include "mvis/mbitbtn.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following is a class for representing standard radio buttons.
//---------------------------------------------------------------------------

class MVRadioButton : public MVSysBitmapButton {
protected:
			// Internal routine to send clicked message to owner
	virtual	void buttonClicked();

public:
			// Constructor
			MVRadioButton(MGLDevCtx& dc,const MVRect& bounds,
				ushort command,ushort flags = bfNormal,
				ibool selected = false);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase = phFocused);
	};

#endif	// __MVIS_MRADIO_HPP
