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
* Description:	Header file for the MVFrame class. This class is used
*				to represent the frame around the outside of dialog
*				boxes and windows.
*
*
****************************************************************************/

#ifndef	__MVIS_MFRAME_HPP
#define	__MVIS_MFRAME_HPP

#ifndef	__MVIS_MVIEW_HPP
#include "mvis/mview.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVFrame class is used to represent the frames drawn around the
// outside of dialog boxes and windows.
//
// If the width of the frame is less than 3, then only a flat black frame
// will be used.
//---------------------------------------------------------------------------

class MVFrame : public MVView {
	int		width;					// Width of the frame
	ushort	flags;					// Window creation flags
	MVRect	inside;					// Inner rectangle for frame
	int		where;					// Classification for last mouse click

	// Cursor classification codes
	enum {
		Inside,
		Left,
		Top,
		Right,
		Bottom,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight,
		};

			// Classify where a point is in relation to frame
			int classifyPoint(const MVPoint& p);

public:
			// Constructor
			MVFrame(MGLDevCtx& dc,const MVRect& bounds,int width,
				ushort flags);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase = phFocused);

			// Method to draw the frame
	virtual	void draw(const MVRect& dirty);

			// Method to set the bounds of the view
	virtual	void setBounds(const MVRect& bounds);

			// Method to change the cursor
	virtual	MGLCursor *getCursor(const MVPoint& p);

			// Method to return the inner rectangle()
			MVRect inner();

			// Method to return the frame width
			int getWidth()	{ return width; };

			// Method to adjust the re-size rectangle given a delta value
			void resizeDragRect(int deltaX,int deltaY,MVRect &dragRect);
	};

#endif	// __MVIS_MFRAME_HPP
