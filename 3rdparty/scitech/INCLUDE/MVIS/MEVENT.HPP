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
* Description:	Header file for the event collection subsystem. We rely on
*				the MegaToobox event module to collect the events for us,
*				which when then stick into a more suitable structure for
*				C++.
*
*				Note that all ticks are measured in the timer tick
*				resolution for the current platform. Use
*				MGL_getTickResolution() if you need to convert between real
*				values and a tick count.
*
*
****************************************************************************/

#ifndef	__MVIS_MEVENT_HPP
#define	__MVIS_MEVENT_HPP

#ifndef	__MVIS_MPOINT_HPP
#include "mvis/mpoint.hpp"
#endif

#ifndef	__MGRAPH_H
#include "mgraph.h"
#endif

/*------------------------------ Constants --------------------------------*/

const ushort

// Event codes

	evKeyDown		= EVT_KEYDOWN,		// Key was depressed
	evKeyAuto		= EVT_KEYREPEAT,	// Key auto repeat
	evKeyUp			= EVT_KEYUP,		// Key was released
	evMouseDown		= EVT_MOUSEDOWN,	// Mouse button was depressed
	evMouseAuto		= 0x0080,			// Mouse button auto repeat
	evMouseUp		= EVT_MOUSEUP,		// Mouse button was released
	evMouseMove		= EVT_MOUSEMOVE,	// Mouse cursor was moved
	evTimerTick		= EVT_TIMERTICK,	// A scheduled timer tick event occurred
	evCommand		= 0x0100,			// Event is a command message
	evBroadcast		= 0x0200,			// Event is a broadcast message
	evRepaint		= 0x0400,			// Event is a repaint message

// Event code masks

	evNothing		= EVT_NULLEVT,
	evMouse			= evMouseDown | evMouseUp | evMouseMove | evMouseAuto,
	evKeyboard		= evKeyDown | evKeyAuto | evKeyUp,
	evMessage		= 0xFF00,			// Event was a message of some type
	evAll			= 0xFFFF,			// All events mask

// Mouse button masks

	mbLeftButton	= EVT_LEFTBMASK,
	mbRightButton	= EVT_RIGHTBMASK,
	mbBothButtons	= mbLeftButton | mbRightButton,

// Event modifier masks

	mdLeftButton	= EVT_LEFTBUT,
	mdRightButton	= EVT_RIGHTBUT,
	mdLeftShift		= EVT_LEFTSHIFT,
	mdRightShift	= EVT_RIGHTSHIFT,
	mdLeftCtrl		= EVT_LEFTCTRL,
	mdLeftAlt		= EVT_LEFTALT,
	mdShift			= mdLeftShift | mdRightShift,
	mdCtrl			= EVT_CTRLSTATE,
	mdAlt			= EVT_ALTSTATE,
	mdMouse			= mdLeftButton | mdRightButton,
	mdKey			= mdShift | mdCtrl | mdAlt,

//  Event masks

	positionalEvents= evMouse,
	focusedEvents	= evKeyboard | evCommand;

/*-------------------------- Class definitions ----------------------------*/

#pragma pack(1)

struct MVMouseEvent {
	ushort	buttons;				// Button code mask
	short	doubleClick;			// Event was a double click
	ulong	modifiers;				// Modifier flags
	};

struct MVKeyEvent {
	union {
		ushort		keyCode;		// Combined ASCII and scan codes
		struct {
			uchar	charCode;		// ASCII code
			uchar	scanCode;		// Scan code
			} charScan;
		};
	short	count;					// Repeat count
	ulong	modifiers;				// Modifier flags
	};

struct MVMessageEvent {
	ulong	command;				// Command for this message
	union {
		void*	infoPtr;			// Various message data types
		long	infoLong;
		ushort	infoWord;
		short	infoInt;
		uchar	infoByte;
		char	infoChar;
		};
	};

class MVView;

struct MVEvent {
	MVView	*which;					// Which view to send event to
	uint	what;					// Event code
	ulong	when;					// Clock ticks when message occurred
	MVPoint	where;					// Where the event occurred
	union {
		MVMouseEvent	mouse;		// Mouse event information
		MVKeyEvent		key;		// Key event information
		MVMessageEvent	message;	// Message event information
		};
	int     res[2];					// Reserved
	};

#pragma pack()

//---------------------------------------------------------------------------
// The following class implements the event queue mechanism for the
// class library. It is basically C++ wrapper to the stuff in the Event
// module of the MegaToolbox, with a few extras. Note that there is no
// destructor, since the event module already ensures that the event
// system will be shut down upon proper program termination.
//
// It automatically handles generating mouse double click and auto repeat
// events, which are not handled directly by the Event module.
//---------------------------------------------------------------------------

class MVEventQueue {
private:
	ushort	doubleClick;		// Ticks between double clicks
	ushort	doubleClickThresh;	// Movement allowed for clicks (in pixels)
	ushort	autoRepeat;			// Ticks betwen mouse auto repeats
	ibool	firstAuto;			// True for first auto mouse repeat
	long	autoDelay;			// Ticks before mouse auto repeat begins
	long	autoTicks;			// Current auto tick value
    long	tickConvert;		// Timer tick conversion factor
	MVEvent	downMouse;			// Last mouse down event
	MVPoint	autoMousePos;		// Auto mouse down position

public:
			MVEventQueue();

			// Obtain or look at the next event
			ibool getNext(MVEvent& event,ushort mask = evAll);
			ibool peekNext(MVEvent& event,ushort mask = evAll)
			{ return EVT_peekNext((event_t*)&event,mask); };

			// Post an event to the event queue
			ibool post(const MVEvent& event);

			// Flush all events specified by the mask
			void flush(ushort mask = evAll)	{ EVT_flush(mask); };

			// Turn collection of timer tick events on or off
			void timerTickOn(ushort ticks)	{ EVT_setTimerTick(ticks); };
			void timerTickOff()				{ EVT_setTimerTick(0); };

			// Set/get the double click speed
			void setDoubleClick(ushort ticks)	{ doubleClick = ticks; };
			ushort getDoubleClick()				{ return doubleClick; };

			// Set/get the double click threshold
			void setDoubleClickThreshold(ushort thresh)
			{ doubleClickThresh = thresh; };
			ushort getDoubleClickThreshold()
			{ return doubleClickThresh; };

			// Set/get the mouse auto repeat speed
			void setAutoRepeat(ushort ticks)	{ autoRepeat = ticks; };
			ushort getAutoRepeat()				{ return autoRepeat; };

			// Set/get the mouse auto repeat delay
			void setAutoRepeatDelay(ushort ticks)	{ autoDelay = ticks; };
			ushort getAutoRepeatDelay()				{ return autoDelay; };

			// Get the conversion factor to convert from 18.2 ticks per
			// second to the current timer tick resolution.
			int getTickConvert()					{ return tickConvert; };

			// Compatability routine to set tick values in 18.2Hz units
			void setDoubleClick18Hz(ushort ticks)
			{ setDoubleClick(ticks * tickConvert); };
			void setDoubleClickThreshold18Hz(ushort thresh)
			{ setDoubleClickThreshold(thresh * tickConvert); };
			void setAutoRepeat18Hz(ushort ticks)
			{ setAutoRepeat(ticks * tickConvert); };
			void setAutoRepeatDelay18Hz(ushort ticks)
			{ setAutoRepeatDelay(ticks * tickConvert); };
	};

/*--------------------------- Global Variables ----------------------------*/

extern MVEventQueue	MV_eventQueue;

#endif	// __MVIS_MEVENT_HPP

