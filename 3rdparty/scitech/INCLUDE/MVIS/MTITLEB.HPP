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
* Description:	Header file for the MVTitleBar class. This class is used
*				to represent the title bar for the window class.
*
*
****************************************************************************/

#ifndef	__MVIS_MTITLEB_HPP
#define	__MVIS_MTITLEB_HPP

#ifndef	__MVIS_MTBBTN_HPP
#include "mvis/mtbbtn.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVTitleBar class is used to represent the title bar drawn at the top
// of the window.
//---------------------------------------------------------------------------

class MVWindow;

class MVTitleBar : public MVGroup {
	char			*title;				// Title text
	MVWindow 		*owner;				// Window that owns this titlebar
	MVTBButton		*minimiseBox;		// Minimise box button
	MVTBButton		*maximiseBox;		// Maximise box button
	MVTBButton		*closeBox;			// Close box button
	MVPoint			start;				// Place to start drawing title
	MVTextJust		tjust;				// Text justification

public:
			// Constructor
			MVTitleBar(MGLDevCtx& dc,const MVRect& bounds,const char *title,
				MVWindow *owner,ushort flags);

			// Destructor
			~MVTitleBar()	{ delete [] title; };

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase);

			// Method to set the bounds of the view
	virtual	void setBounds(const MVRect& bounds);

			// Method to draw the title bar
	virtual	void drawBackground(const MVRect& dirty);
	};

#endif	// __MVIS_MTITLEBAR_HPP
