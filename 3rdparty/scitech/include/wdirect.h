/****************************************************************************
*
*				   WinDirect - Windows Full Screen Interface
*
*                     Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   wdirect.h  $
* Version:		$Revision:   1.4  $
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for Windows full screen interface routines for
*				Windows 3.1 and Windows '95. Also provides a full event
*				handling mechanism for handling keyboard, mouse and
*				timer events while in full screen modes. The event handling
*				API is identical to that provided by the MGL under Windows
*				and DOS.
*
*				Note that WinDirect scales all mouse coordinates to lie
*				within the range from (0,xRes) and (0,yRes) after a call to
*				WD_startFullScreen is made. The values that you pass for
*				xRes and yRes should reflect the resoltion of the video
*				mode that you intend to initialise after the call to
*				WD_startFullScreen(). Note also that although you can get
*				and set the mouse cursor position, the mouse cursor is not
*				drawn for you. It is up to your application to draw the mouse
*				cursor at the appropriate location on the screen. We do
*				however allow you to register a callback that will be called
*				when the mouse cursor position is changed, so that you can
*				move your own mouse cursor around on the display.
*
*				When the GDI is shut down, WinDirect provides a full screen
*				Window that is used for all event handling, and will
*				return a handle to this window when full screen mode is
*				started. If you dont wish to use the provided WinDirect
*				event handling API, you can subclass the WindowProc for
*				the fullscreen window and provide your own window procedure.
*
*				The WinDirect DOS compatability library provides the entire
*				WinDirect API under DOS included full event handling, so
*				that you can write a single source WinDirect application
*               that will compile and run for real DOS or WinDirect under
*				Windows.
*
* $Date:   09 Apr 1996 12:27:46  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef	__WDIRECT_H
#define	__WDIRECT_H

#ifndef	__DEBUG_H
#include "debug.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)				/* Pack structures to byte granularity		*/

/* Event message masks for keyDown events */

#define	EVT_ASCIIMASK	0x00FF	/* Ascii code of key pressed			*/
#define	EVT_SCANMASK	0xFF00	/* Scan code of key pressed				*/
#define	EVT_COUNTMASK	0x7FFF0000L		/* Count for KEYREPEAT's		*/

#define	WD_asciiCode(m)		( (uchar) (m & EVT_ASCIIMASK) )
#define	WD_scanCode(m)		( (uchar) ( (m & EVT_SCANMASK) >> 8 ) )
#define	WD_repeatCount(m)	( (short) ( (m & EVT_COUNTMASK) >> 16 ) )

/* Event message masks for mouse events */

#define	EVT_LEFTBMASK	0x0001	/* Left button is bit 0					*/
#define	EVT_RIGHTBMASK	0x0004	/* Right button is bit 1				*/
#define	EVT_BOTHBMASK	0x0005	/* Both left and right together			*/
#define	EVT_ALLBMASK	0x0005	/* All buttons pressed					*/

/* Modifier masks */

#define	EVT_LEFTBUT		0x0001	/* Set if left button was down			*/
#define	EVT_RIGHTBUT	0x0002	/* Set if right button was down			*/
#define	EVT_CTRLSTATE	0x0020	/* Set if ctrl key down					*/
#define	EVT_ALTSTATE	0x0040	/* Set if alt key down					*/
#define	EVT_SHIFTKEY	0x0018	/* Any shift key						*/
#define	EVT_DBLCLK		0x8000	/* Mouse down was a double click		*/

/* Event codes */

#define	EVT_NULLEVT		0x0000	/* A null event							*/
#define	EVT_KEYDOWN		0x0001	/* Key down event						*/
#define	EVT_KEYREPEAT	0x0002	/* Key repeat event						*/
#define	EVT_KEYUP		0x0004	/* Key up event							*/
#define	EVT_MOUSEDOWN	0x0008	/* Mouse down event						*/
#define	EVT_MOUSEUP		0x0010	/* Mouse up event						*/
#define	EVT_MOUSEMOVE	0x0020	/* Mouse movement event					*/
#define	EVT_TIMERTICK	0x0040	/* Timer tick event						*/
#define	EVT_USEREVT		0x0080	/* First user event						*/

/* Event code masks */

#define	EVT_KEYEVT		(EVT_KEYDOWN | EVT_KEYREPEAT | EVT_KEYUP)
#define	EVT_MOUSEEVT	(EVT_MOUSEDOWN | EVT_MOUSEUP | EVT_MOUSEMOVE)
#define	EVT_MOUSECLICK	(EVT_MOUSEDOWN | EVT_MOUSEUP)
#define	EVT_EVERYEVT	0xFFFF

typedef	struct {
	uint		what;			/* Event code							*/
	ulong		when;			/* Time stamp (in ms)					*/
	int			where_x;		/* Mouse location						*/
	int			where_y;
	ulong		message;		/* Event specific message				*/
	ulong		modifiers;		/* Modifier flags						*/
	int			next;   		/* Internal use							*/
	int			prev;			/* Internal use							*/
	} WD_event;

/* Suspend Application callback type codes. This callback is called
 * when the user presses one of the corresponding keys indicating that
 * they wish to change the active application. WinDirect will catch these
 * events and if you have registered a callback, will call the callback to
 * save the state of the application (ie: video mode, framebuffer contents,
 * etc etc) so that it can be properly restored when the user switches back
 * to your application. WinDirect will take care of shutting down and
 * restoring GDI, and you can elect to ignore these requests by returning
 * a failure code (you may wish to allow this only when at the options screen
 * in a game for instance).
 */

#define	WD_DEACTIVATE	0x0001	/* Application losing active focus		*/
#define	WD_REACTIVATE	0x0002	/* Application regaining active focus	*/

#ifdef	__WINDOWS32__
#define	_WDAPI	_DLLASM			/* 'C' calling conventions for Win32	*/
#elif	defined(__WINDOWS16__)
#define	_WDAPI	_DLLAPI			/* Pascal calling conventions for Win16	*/
#else
#define	_WDAPI  _PUBAPI			/* Default calling conventions for DOS	*/
#endif

#ifdef	__WINDOWS__
typedef	HWND	WD_HWND;		/* Real window handle under Windows		*/
#else
typedef	void	*WD_HWND;		/* Fake window handle under DOS			*/
#endif

/*------------------------- Function Prototypes ---------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Shut down GDI and start full screen video mode. Under DOS this enables
 * the fullscreen event handling interface, and shuts it down again when
 * we go back to normal text mode.
 */

WD_HWND	_WDAPI WD_startFullScreen(WD_HWND hwndMain,int xRes,int yRes);
void	_WDAPI WD_inFullScreen(void);
void	_WDAPI WD_restoreGDI(void);
void	_WDAPI WD_setSuspendAppCallback(int (_ASMAPI *saveState)(int flags));

/* Error message reporting (Windows version only) */

#ifdef	__WINDOWS__
int		_WDAPI WD_messageBox(LPCSTR lpszText,LPCSTR lpszTitle,UINT fuStyle);
#endif

/* Event handling support */

bool 	_WDAPI WD_getEvent(WD_event *evt,uint mask);
bool	_WDAPI WD_peekEvent(WD_event *evt,uint mask);
bool	_WDAPI WD_postEvent(uint what,ulong message,ulong modifiers);
void	_WDAPI WD_flushEvent(uint mask);
void 	_WDAPI WD_haltEvent(WD_event *evt,uint mask);
int		_WDAPI WD_setTimerTick(int ticks);

/* Mouse handling code */

void	_WDAPI WD_getMousePos(int *x,int *y);
void	_WDAPI WD_setMousePos(int x,int y);
void	_WDAPI WD_setMouseCallback(void (_ASMAPI *moveCursor)(int x,int y));

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#pragma pack()

#endif	/* __WDIRECT_H */
