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
* Description:	Header file for the MVBitmapButton classes for buttons
*				that are defined by a set of bitmaps.
*
*
****************************************************************************/

#ifndef	__MVIS_MBITBTN_HPP
#define	__MVIS_MBITBTN_HPP

#ifndef	__MVIS_MBUTTON_HPP
#include "mvis/mbutton.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following is a class for creating standard buttons that are defined
// but a set of system bitmap indices. You must provide a set of 32 possible
// indices to represent all the different states that a button may be in
// from the button body state flags. In most cases you will simply use the
// same index for multiple entries.
//---------------------------------------------------------------------------

class MVSysBitmapButton : public MVButtonBase {
protected:
	char	index[32];			// Array of 16 possible bitmap indexes

			// Internal method to draw the button body in specified state
	virtual	void drawBody(const MVRect& clip,ushort bodyState);

public:
			// Constructor
			MVSysBitmapButton(MGLDevCtx& dc,const MVRect& bounds,
				ushort command,int *indexes,ushort flags = bfNormal,
				ibool selected = false);
	};

//---------------------------------------------------------------------------
// The following is a class for creating standard buttons that are defined
// but a set of user defined bitmaps. You must provide a set of 16 possible
// bitmaps to represent all the different states that a button may be in
// from the button body state flags. In most cases you will simply use the
// same bitmap for multiple entries. Note also that this class does not
// own the bitmaps, so they will not be destroyed when the button is
// destroyed (allowing you to use them for multiple buttons).
//---------------------------------------------------------------------------

class MVBitmapButton : public MVButtonBase {
protected:
	bitmap_t	*bmp[32];		// Array of 32 possible bitmaps to display

			// Internal method to draw the button body in specified state
	virtual	void drawBody(const MVRect& clip,ushort bodyState);

public:
			// Constructor
			MVBitmapButton(MGLDevCtx& dc,const MVRect& bounds,ushort command,
				bitmap_t *bmps[],ushort flags = bfNormal,ibool selected = false);
	};

#endif	// __MVIS_MBITBTN_HPP
