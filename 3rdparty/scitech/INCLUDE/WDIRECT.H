/****************************************************************************
*
*				   WinDirect - Windows Full Screen Interface
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
*
****************************************************************************/

#ifndef	__WDIRECT_H
#define	__WDIRECT_H

#ifndef	__SCITECH_H
#include "scitech.h"
#endif

#ifdef	__WINDOWS__
#define	WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#endif

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)

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
	ulong		which;			/* Which window for window manager code	*/
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
 * in a game for instance; see below).
 */

#define	WD_DEACTIVATE	0x0001	/* Application losing active focus		*/
#define	WD_REACTIVATE	0x0002	/* Application regaining active focus	*/

/* Return codes from the suspend application callback. The normal value
 * to be returned is WD_SUSPEND_APP and this will cause the app to be
 * suspended while back in GDI mode until the app is re-activated again
 * by the user.
 *
 * WD_NO_DEACTIVATE signals to WinDirect that the application does not want
 * to allow switching to occur, and the switch request will be ignored and
 * the app will remain in fullscreen mode.
 *
 * WD_NO_SUSPEND_APP can be used to tell WinDirect to switch back to the
 * desktop, but not to suspend the application. This must be used with
 * care as the suspend application callback is then responsible for setting
 * a flag in the application that will stop the application from doing any
 * rendering to the framebuffer while the application is in GDI mode. This
 * return value is useful for games that need to maintain network
 * connectivity while the user has temporarily switched back to GDI mode.
 */

#define	WD_NO_DEACTIVATE	0	/* Dont allow app to be deactivated 	*/
#define	WD_SUSPEND_APP		1	/* Suspend application until restored	*/
#define	WD_NO_SUSPEND_APP	2	/* Dont suspend, but allow switch		*/

#ifdef	__WINDOWS32__
#define	_WDAPI	_DLLASM			/* 'C' calling conventions for Win32	*/
#define	WDHKEY	HKEY			/* Handle to registry key for Win32		*/
#elif	defined(__WINDOWS16__)
#define	_WDAPI	_DLLAPI			/* Pascal calling conventions for Win16	*/
#define	WDHKEY	ulong			/* Handle to registry key for Win16		*/
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

/* Initialise WinDirect for Watcom Win386 support */

#ifdef __WIN386__
ibool	_WDAPI WD_initWin386(HINSTANCE hInstance);
#endif

/* Shut down GDI and start full screen video mode. Under DOS this enables
 * the fullscreen event handling interface, and shuts it down again when
 * we go back to normal text mode.
 */

WD_HWND	_WDAPI WD_startFullScreen(WD_HWND hwndMain,int xRes,int yRes);
void	_WDAPI WD_changeResolution(int xRes,int yRes);
void	_WDAPI WD_inFullScreen(void);
void	_WDAPI WD_restoreGDI(void);
void	_WDAPI WD_setSuspendAppCallback(int (_ASMAPI *saveState)(int flags));

/* Function to register a fullscreen window with WinDirect. If you wish
 * for WinDirect to use your own window for fullscreen modes, you can
 * register it with this function. Note that when WinDirect goes into
 * fullscreen modes, the attributes, size and position of the window are
 * modified to make it into a fullscreen Window necessary to cover the
 * entire desktop, and the state of the window will be restore to the original
 * format on return to normal GDI mode.
 *
 * Note that if you are using a common window for Windowed mode and fullscreen
 * modes of your application, you will need to ensure that certain messages
 * that you window normally handles in windowed modes are ignored when in
 * fullscreen modes.
 */

#ifdef	__WINDOWS__
void 	_WDAPI WD_registerFullScreenWindow(HWND hwndFullScreen);
#endif

/* Function to register a fullscreen event handling window procedure.
 * If you wish to do your own event handling, you can register your window
 * procedure with WinDirect using this function and it will be called
 * when there are messages to be handled. You can still call the WD_event()
 * functions even if you have registered an event handling procedure.
 */

#ifdef	__WINDOWS__
void 	_WDAPI WD_registerEventProc(WNDPROC userWndProc);
#endif

/* Error message reporting (Windows version only) */

#ifdef	__WINDOWS__
int		_WDAPI WD_messageBox(LPCSTR lpszText,LPCSTR lpszTitle,UINT fuStyle);
#endif

/* Check if running in safe mode (Windows version only) */

#ifdef	__WINDOWS__
int		_WDAPI WD_isSafeMode(void);
#endif

/* Function to set the application instance handle for WinDirect */

#ifdef	__WINDOWS__
void	_WDAPI WD_setAppInstance(HINSTANCE hInstApp);
#endif

/* Event handling support */

ibool 	_WDAPI WD_getEvent(WD_event *evt,uint mask);
ibool	_WDAPI WD_peekEvent(WD_event *evt,uint mask);
ibool	_WDAPI WD_postEvent(ulong which,uint what,ulong message,ulong modifiers);
void	_WDAPI WD_flushEvent(uint mask);
void 	_WDAPI WD_haltEvent(WD_event *evt,uint mask);
int		_WDAPI WD_setTimerTick(int ticks);
ibool 	_WDAPI WD_isKeyDown(uchar scanCode);

/* Mouse handling code */

void	_WDAPI WD_getMousePos(int *x,int *y);
void	_WDAPI WD_setMousePos(int x,int y);
void	_WDAPI WD_setMouseCallback(void (_ASMAPI *moveCursor)(int x,int y));

/* Sound functions in WinDirect DOS libraries */

#ifndef	__WINDOWS__
void 	_ASMAPI _WD_sound(int freq);
void 	_ASMAPI _WD_nosound(void);
void 	_ASMAPI WD_delay(int msecs);
void 	_ASMAPI WD_beep(int freq,int msecs);
#endif

#ifdef	__WINDOWS__
#define	DISPLAY_CLASS	0
#define	MONITOR_CLASS	1
#define	DEV_NODE		0
#define	DRIVER_NODE		1
uint	_WDAPI SDD9X_getVersion(void);
void	_WDAPI SDD9X_physicalEnable(void);
ibool	_WDAPI SDD9X_openDevRegKey(uint deviceType,WDHKEY *hKeyDev,WDHKEY *hKeyDrv);
ibool	_WDAPI SDD9X_openNextDevRegKey(WDHKEY *hKeyDev,WDHKEY *hKeyDrv);
ibool	_WDAPI SDD9X_openActiveDevRegKey(uint deviceType,ulong devNode,WDHKEY *hKeyDev,WDHKEY *hKeyDrv);
#endif

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#pragma pack()

#endif	/* __WDIRECT_H */

