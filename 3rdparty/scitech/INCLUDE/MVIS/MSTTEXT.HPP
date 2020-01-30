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
* Description:	Header file for the MVStaticText class. This class is used
*				to represent text items in a view that are static and
*				do not respond to events. Test items clear the visRgn
*				as they dont clear the background that they are drawn over.
*				Hence you can overlay static text items on top of other
*				background items.
*
*
****************************************************************************/

#ifndef	__MVIS_MSTTEXT_HPP
#define	__MVIS_MSTTEXT_HPP

#ifndef	__MVIS_MVIEW_HPP
#include "mvis/mview.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

ushort const

// Static text creation flags

	sfLeftJust		= 0x0001,		// Text is left justified
	sfRightJust		= 0x0002,		// Text is right justified
	sfTopJust		= 0x0004,		// Text is top justified
	sfBottomJust	= 0x0008,		// Text is bottom justified
	sfWrap			= 0x0010,		// Text wraps at right edge
	sfNormal		= sfLeftJust | sfTopJust | sfWrap;

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVStaticText class is used to represent text items in a view that
// are static and do not respond to any events. MVStaticText views cannot
// be focused.
//
// Note that the text will be clipped to the bounds of the text item.
//---------------------------------------------------------------------------

class MVStaticText : public MVView {
protected:
	const char 		*text;		// Text to display
	MVPoint			start;		// Position to begin drawing title at
	MVTextJust		tjust;		// Text justification
	short			textColor;	// Color value to display text
	ushort			flags;		// Creation flags

public:
			// Constructor
			MVStaticText(MGLDevCtx& dc,const MVRect& bounds,
				const char *text,ushort flags = sfNormal,
				short color = scStaticText);

			// Method to set the bounds for the static text item
	virtual	void setBounds(const MVRect& bounds);

			// Method to draw the static text item
	virtual	void draw(const MVRect& dirty);

			// Method to set the text for the text item - returns old value
	virtual	const char *setText(const char *t);
	};

//---------------------------------------------------------------------------
// The MVLabel class is a special static text class that can be used to
// automatically select another view when it is clicked.
//---------------------------------------------------------------------------

class MVLabel : public MVStaticText {
protected:
	MVView	*attached;			// View attached to the label

public:
			// Constructor
			MVLabel(MGLDevCtx& dc,const MVRect& bounds,const char *text,
				MVView *attached,ushort flags = sfNormal,
				short color = scStaticText)
				: MVStaticText(dc,bounds,text,flags,color),
				  attached(attached) {};

			// Method to set/get the attached view
			void setAttached(MVView *a)	{ attached = a; };
			MVView *getAttached()		{ return attached; };

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase = phFocused);
	};

//---------------------------------------------------------------------------
// The TMultiLineText class is used to represent static text items in a
// view that contain multiple lines of text.
//---------------------------------------------------------------------------

class MVMultiLineText : public MVStaticText {
protected:
	short	lines;				// Number of lines of text

public:
			// Constructor
			MVMultiLineText(MGLDevCtx& dc,const MVRect& bounds,
				const char *text,ushort flags = sfNormal,
				short color = scStaticText);

			// Method to draw the static text item
	virtual	void draw(const MVRect& dirty);

			// Method to set the text for the text item - returns old value
	virtual	const char *setText(const char *t);

			// Method to obtain the number of lines
			short numberOfLines()	{ return lines; };
	};

#endif	// __MVIS_MSTTEXT_HPP
