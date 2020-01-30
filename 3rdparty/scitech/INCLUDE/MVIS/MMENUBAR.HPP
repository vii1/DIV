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
* Description:	Header file for the MVMenuBar class. This class is used
*				to implement the standard menu bar mechanism for the
*				library.
*
*
****************************************************************************/

#ifndef	__MVIS_MMENUBAR_HPP
#define	__MVIS_MMENUBAR_HPP

#ifndef	__MVIS_MMENU_HPP
#include "mvis/mmenu.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVMenuBar class is the class used to implement the standard horizontal
// menu bar mechanism for the library. The palette is the same as the
// standard MVMenu palette.
//---------------------------------------------------------------------------

class MVMenuBar : public MVMenu {
protected:
			// Method to draw a menu item
	virtual	void drawItem(int index);

			// Method to compute the bounding rectangle for an item
	virtual	MVRect& setItemBounds(int index,const MVPoint& start);

			// Method to find which menu item was hit
			int findSelected(MVPoint& where);

public:
			// Constructor
			MVMenuBar(MGLDevCtx& dc,const MVRect& bounds)
				: MVMenu(dc,bounds) {};

			// Virtual to execute the menu tracking the mouse etc
	virtual	ushort execute();

			// Method to complete the menu bar definition
	virtual	void doneDefinition();
	};

#endif	// __MVIS_MMENUBAR_HPP
