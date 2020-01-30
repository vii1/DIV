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
* Description:	Header file for the MVInputLine class. This class is used
*				to obtain a single line of text from the user.
*
*
****************************************************************************/

#ifndef	__MVIS_MINPUTL_HPP
#define	__MVIS_MINPUTL_HPP

#ifndef	__MVIS_MVIEW_HPP
#include "mvis/mview.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

ushort const

	tlCursorVisible	= 0x0001,		// Cursor is visible
	tlOverwriteMode	= 0x0002,		// Key's overwrite the text
	tlDirty			= 0x0004;		// The input line is dirty (needs drawing)

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVInputLine class is used to obtain a single line of text from the
// user, up to a maximum specified length. Scrolling is supported if the
// line is too long to fit within the view. Note that the buffer passed
// to the input line routine will need to be large enough to store the
// edited text.
//
// MVInputLine's are intended to be inserted into TDialog's.
//---------------------------------------------------------------------------

class MVInputLine : public MVView {
protected:
	char 	*text;				// Text being edited
	short	maxLen;				// Maximum length for input line
	short	len;				// Current text length
	short	curPos;				// Current insertion position (index)
	short	curLoc;				// Current cursor location (coordinate)
	short	firstPos;			// Index of first displayed character
	short	selStart;			// Index of first selected character
	short	selEnd;				// Index of last selected character
	short	starty;				// Starting y coordinate for text
	short	scrollChars;		// Number of characters to scroll
	ushort	flags;				// Flags for the input line
	long	oldTicks;			// Old tick count
	long	cursorRate;			// Rate of cursor flashing

			// Method to reposition to text within the window
			void repositionText();

			// Method to clear the highlight
			void clearHighlight();

			// Method to delete a number of characters from the text
			void deleteChars(short start,short count);

			// Method to insert a character into the text
			void insertChar(char ch);

			// Delete the selected characters
			ibool deleteHighlight();

			// Method to find a character position given a coordinate
			short findPos(const MVPoint& p);

			// Method to select the next left character
			void selectLeft(short count,ushort modifiers);

			// Method to select the next right character
			void selectRight(short count,ushort modifiers);

			// Method to repaint the display
			void repaint(const MVRect& dirty);

			// Method to refresh the display (calls repaint if necessary)
			void refresh();

			// Method to draw the cursor
			void drawCursor(ibool visible);

public:
			// Constructor
			MVInputLine(MGLDevCtx& dc,const MVRect& bounds,int maxLen,
				const char *defaultText = NULL);

			// Virtual destructor
	virtual	~MVInputLine();

			// Method to set the bounds for the input line
	virtual	void setBounds(const MVRect& bounds);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase);

			// Method to draw the input line
	virtual	void draw(const MVRect& dirty);

			// Method called when idle
	virtual	void idle();

			// Method to change the cursor
	virtual	MGLCursor *getCursor(const MVPoint& p);

			// Method to set a state flag
	virtual	void setState(ushort state,ibool set);

			// Method to get the text from the input line
			const char *getText() const { return text; };

			// Method to set the text for the input line
			void setText(const char *text);

			// Method to set the cursor flash rate
			void setCursorRate(ushort rate)	{ cursorRate = rate; };
			ushort getCursorRate()			{ return cursorRate; };
	};

#endif	// __MVIS_MINPUTL_HPP
