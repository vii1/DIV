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
* Description:	Header file for the MVWindow class. This class is used
*				to represent pop-up windows used by TDialogs etc.
*
*
****************************************************************************/

#ifndef	__MVIS_MWINDOW_HPP
#define	__MVIS_MWINDOW_HPP

#ifndef	__MVIS_MGROUP_HPP
#include "mvis/mgroup.hpp"
#endif

#ifndef	__MVIS_MFRAME_HPP
#include "mvis/mframe.hpp"
#endif

#ifndef	__MVIS_MTITLEB_HPP
#include "mvis/mtitleb.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

const ushort

// MVWindow creation flag masks

	wfMoveable		= 0x0001,		// Window can be moved
	wfResizeable	= 0x0002,		// Window can be resized
	wfSystemMenu	= 0x0004,		// Window has a system menu
	wfClose			= 0x0008,		// Window has a close icon
	wfMinimise		= 0x0010,		// Window has a minimise icon
	wfMaximise		= 0x0020,		// Window has a maximise icon
	wfFramed		= 0x0040,		// Window has a 3d frame
	wfLeftJust		= 0x0080,		// Window title is left justified
	wfRightJust		= 0x0100,		// Window title is right justified
	wfTopJust		= 0x0200,		// Window title is top justified
	wfBottomJust	= 0x0400,		// Window title is bottom justified
	wfFixedWin		= wfSystemMenu | wfClose | wfFramed | wfLeftJust,
	wfMoveableWin	= wfMoveable | wfSystemMenu | wfClose | wfMinimise
						| wfFramed | wfLeftJust,
	wfNormal		= wfMoveable | wfResizeable | wfSystemMenu | wfClose
						| wfMinimise | wfMaximise | wfFramed | wfLeftJust;

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVWindowInit class is used to create a class to hold a pointer to a
// frame creation routine. We need this to solve the problem with virtual
// function calls from within a superclass's constructor.
//---------------------------------------------------------------------------

class MVWindowInit {
protected:
	MVFrame		*(*createFrame)(MGLDevCtx& dc,const MVRect& bounds,int,ushort);
	MVTitleBar	*(*createTitleBar)(MGLDevCtx& dc,const MVRect& bounds,
					const char *title,MVWindow *,ushort flags);

public:
			MVWindowInit(
				MVFrame *(*createFrame)(MGLDevCtx& dc,const MVRect&,int,ushort),
				MVTitleBar *(*createTitleBar)(MGLDevCtx& dc,const MVRect&,
					const char*,MVWindow *,ushort))
				: createFrame(createFrame),
				  createTitleBar(createTitleBar) {};
	};

//---------------------------------------------------------------------------
// The MVWindow class is special group that maintains a collection of views
// in a moveable and re-sizeable window. MVWindow's are modeless by default.
//
// The frame and title for the window are within the main window bounds.
//---------------------------------------------------------------------------

class MVWindow : public MVGroup, public virtual MVWindowInit {
protected:
	MVFrame		*frame;				// Frame for the window
	MVTitleBar	*titleBar;			// Title bar for the window
	MVRect		normalBounds;		// Normal bounds for window
	ibool		maximised;			// True if window is maximised
	ibool		minimised;			// True if window is minimised
	short		number;				// Number of this window
	ushort		flags;				// Flags the type of window

	// TODO:	icon;				// Icon to display when minimised

			// Method to draw the window's background
	virtual	void drawBackground(const MVRect& dirty);

			// Track the movement of the window bounds
			void trackMovement(MVEvent& event,MVRect& bounds);

			// Track resizing of window bounds
			void trackResize(MVEvent& event,MVRect& bounds);
public:
			// Constructor
			MVWindow(MGLDevCtx& dc,const MVRect& bounds,
				const char *title = NULL,ushort flags = wfNormal,
				int frameWidth = 4);

			// Virtual to handle an event
	virtual	void handleEvent(MVEvent& event,phaseType phase = phFocused);

			// Method to set the bounds of the view
	virtual	void setBounds(const MVRect& bounds);

			// Method to move the window to another location
	virtual	void moveTo(int x,int y);

			// Method to move the window
			void moveWindow(MVEvent& event);

			// Method to resize the window
			void resizeWindow(MVEvent& event);

			// Method to minimise/maximise/restore the window size
	virtual	void minimiseWindow();
	virtual	void maximiseWindow();
	virtual	void restoreWindow();

			// Determine if the window is minimised/maximised
			ibool isMinimised()	{ return minimised; };
			ibool isMaximised()	{ return maximised; };

			// Default creation routines for windows
	static	MVFrame* initFrame(MGLDevCtx& dc,const MVRect& bounds,
				int width,ushort flags);
	static	MVTitleBar *initTitleBar(MGLDevCtx& dc,const MVRect& bounds,
				const char *title,MVWindow *owner,ushort flags);
	};

	// TODO: Overload the draw() or paint() function to handle windows
	// minimised to the owner drawn as an icon in owner client region

#endif	// __MVIS_MWINDOW_HPP
