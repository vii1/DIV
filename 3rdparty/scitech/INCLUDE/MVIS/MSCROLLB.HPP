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
* Description:	Header file for the MVScrollBar classes. These classes
*				are used to represent the horizontal and vertical
*				scroll bar objects.
*
*				You may make the scroll bar's as wide as you wish, but the
*				standard scroll bar width is 14 pixels.
*
*
****************************************************************************/

#ifndef	__MVIS_MSCROLLB_HPP
#define	__MVIS_MSCROLLB_HPP

#ifndef	__MVIS_MVIEW_HPP
#include "mvis/mview.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

ushort const

// Scroll bar part codes

	sbLeftArrow		= 0,
	sbRightArrow	= 1,
	sbPageLeft		= 2,
	sbPageRight		= 3,
	sbThumb			= 4,

// Scroll bar flags

	sbInteracting	= 0x0001,		// The scroll bar is interacting
	sbDirtyLeft		= 0x0002,		// The left arrow is dirty
	sbDirtyRight	= 0x0004,		// The right arrow is dirty
	sbDirtyThumb	= 0x0008,		// The thumb is dirty
	sbDirty			= sbDirtyLeft | sbDirtyRight | sbDirtyThumb;

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following is the class used to represent both horiztontal and
// vertical scroll bars.
//
// MVScrollBar's are intended to be inserted into MVWindow and descendant
// classes.
//---------------------------------------------------------------------------

class MVScrollBar : public MVView {
protected:
	short	value;				// Current value for scroll bar
	short	minVal,maxVal;		// Minimum/maximum scroll bar values
	short	pageStep;			// Amount to move if page area activated
	short	arrowStep;			// Amount to move if arrow area activated
	MVRect	leftArrow;			// Bounds for left arrow
	MVRect	rightArrow;			// Bounds for right arrow
	MVRect	thumb;				// Bounds for thumb
	ibool	vertical;			// True if scroll bar is vertical
	ushort	flags;				// Flags for the scroll bar

			// Methods to draw the scroll bar parts
			void drawLeftArrow(ibool down);
			void drawRightArrow(ibool down);
			void drawThumb(int which);

			// Method to determine which part of scroll bar was clicked
			int getPartHit(const MVPoint& p);

			// Method to draw the specified part
			void drawPart(int part,ibool down);

			// Method to compute the location of the thumb
			void moveThumb();

			// Methods to change the scroll bar value, and redraw it
			void changeValue(int part);
			void changeValue(const MVPoint& where);
public:
			// Constructor
			MVScrollBar(MGLDevCtx& dc,const MVRect& bounds);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase);

			// Method to move the group to another location
	virtual	void moveTo(int x,int y);

			// Method to set the bounds for the scroll bar
	virtual	void setBounds(const MVRect& bounds);

			// Method to draw the scrollbar in current state
	virtual	void draw(const MVRect& dirty);

			// Method to update the state of the scroll bar (thumb posn)
			void update();

			// Methods to get/set the values for the scroll bar
			void setValue(short v);
			void setMinVal(short v);
			void setMaxVal(short v);
			void setPageStep(short s)	{ pageStep = s; };
			void setArrowStep(short s)	{ arrowStep = s; };
			short getValue() const		{ return value; };
			short getMinVal() const		{ return minVal; };
			short getMaxVal() const		{ return maxVal; };
			short getPageStep() const	{ return pageStep; };
			short getArrowStep() const	{ return arrowStep; };
			void setRange(short min,short max);
	};

#endif	// __MVIS_MSCROLLB_HPP
