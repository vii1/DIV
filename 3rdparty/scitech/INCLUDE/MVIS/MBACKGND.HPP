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
* Description:	Header file for the TBackGround class. This class is used
*				to draw a patterned background in the system background
*				colors.
*
*
****************************************************************************/

#ifndef	__MVIS_MBACKGND_HPP
#define	__MVIS_MBACKGND_HPP

#ifndef	__MVIS_MVIEW_HPP
#include "mvis/mview.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The TBackGround class is a view that draws a patterned backGround in
// the system background colors.
//---------------------------------------------------------------------------

class MVBackGround : public MVView {
protected:
	int			fillStyle;		// Style for filling background
	pattern_t	pat;			// Pattern to draw with

public:
			// Constructor
			MVBackGround(MGLDevCtx& dc,const MVRect& bounds,
				int fillStyle = MGL_BITMAP_SOLID,pattern_t *pat = MGL_SOLID_FILL);

			// Virtual destructor
	virtual	~MVBackGround();

			// Method to draw the background
	virtual	void draw(const MVRect& dirty);
	};

#endif	// __MVIS_MBACKGND_HPP
