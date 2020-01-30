/****************************************************************************
*
*                       MegaGraph Graphics Library
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
* Language:     ANSI C
* Environment:  IBM PC (MS DOS)
*
* Description:  Routines to provide an event queue, which automatically
*               handles keyboard and mouse events for the MGL. This module
*				basically provides a common interface for DOS and Windows
*               applications for managing events, and uses the WinDirect
*				event handling API.
*
*
****************************************************************************/

#include "mglwin\internal.h"

/*---------------------------- Global Variables ---------------------------*/

#define EVENTQSIZE  		100		/* Number of events in event queue  */

static	int         head = -1;      /* Head of event queue              */
static	int         tail = -1;      /* Tail of event queue              */
static	int         freeHead = 0;  	/* Head of free list                */
static	int         count = 0;      /* No. of items currently in queue  */
static	int			oldMove = -1;	/* Previous movement event			*/
static	event_t		evtq[EVENTQSIZE];/* The queue structure itself		*/
static	ushort		keyUpMsg[256] = {0};/* Table of key up messages		*/
static 	HWND		hwndFullScreen;	/* Fullscreen window handle			*/

/*---------------------------- Implementation -----------------------------*/

static void AddEvent(event_t *evt)
/****************************************************************************
*
* Function:     addEvent
* Parameters:   evt - Event to place onto event queue
*
* Description:  Adds an event to the event queue by tacking it onto the
*               tail of the event queue. This routine assumes that at least
*               one spot is available on the freeList for the event to
*               be inserted.
*
****************************************************************************/
{
    int         evtID;

	/* Get spot to place the event from the free list */
	evtID = freeHead;
	freeHead = evtq[freeHead].next;

    /* Add to the tail of the event queue   */
    evt->next = -1;
	evt->prev = tail;
    if (tail != -1)
        evtq[tail].next = evtID;
    else
        head = evtID;
    tail = evtID;
	evtq[evtID] = *evt;
	count++;
}

static void InitEvent(void)
/****************************************************************************
*
* Function:     InitEvent
*
* Description:  Initialises the event queue data structures.
*
****************************************************************************/
{
	int     	i;

	/* Build free list, and initialise global data structures */
	head = tail = -1;
	count = freeHead = 0;
	oldMove = -1;
	for (i = 0; i < EVENTQSIZE; i++)
		evtq[i].next = i+1;
	evtq[EVENTQSIZE-1].next = -1;       /* Terminate list           */
	for (i = 0; i < 256; i++)
		keyUpMsg[i] = 0;
}

static void PumpMessages(void)
/****************************************************************************
*
* Function:     PumpMessages
*
* Description:  Pumps all messages in the message queue through the
*				Window procedure, which in turns adds them in order to
*				our event queue.
*
****************************************************************************/
{
	MSG			msg;
	MSG			charMsg;
	event_t	evt;

	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		memset(&evt,0,sizeof(evt));
		switch (msg.message) {
			case WM_MOUSEMOVE:
				evt.what = EVT_MOUSEMOVE;
				break;
			case WM_LBUTTONDBLCLK:
				evt.modifiers = EVT_DBLCLK;
			case WM_LBUTTONDOWN:
				evt.what = EVT_MOUSEDOWN;
				evt.message = EVT_LEFTBMASK;
				break;
			case WM_LBUTTONUP:
				evt.what = EVT_MOUSEUP;
				evt.message = EVT_LEFTBMASK;
				break;
			case WM_RBUTTONDBLCLK:
				evt.modifiers = EVT_DBLCLK;
			case WM_RBUTTONDOWN:
				evt.what = EVT_MOUSEDOWN;
				evt.message = EVT_RIGHTBMASK;
				break;
			case WM_RBUTTONUP:
				evt.what = EVT_MOUSEUP;
				evt.message = EVT_RIGHTBMASK;
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (HIWORD(msg.lParam) & KF_REPEAT) {
					evt.what = EVT_KEYREPEAT;
					}
				else {
					evt.what = EVT_KEYDOWN;
					}
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				evt.what = EVT_KEYUP;
				break;
			case WM_TIMER:
				evt.what = EVT_TIMERTICK;
				break;
			}

		/* Convert mouse event modifier flags */
		if (evt.what & EVT_MOUSEEVT) {
        	if (_MGL_deskX) { 
				evt.where_x = ((long)msg.pt.x * _MGL_xRes) / _MGL_deskX;
				evt.where_y = ((long)msg.pt.y * _MGL_yRes) / _MGL_deskY;
				}
			else {
				ScreenToClient(_MGL_hwndUser, &msg.pt);
				evt.where_x = msg.pt.x;
				evt.where_y = msg.pt.y;
				}
			if (evt.what == EVT_MOUSEMOVE) {
				if (oldMove != -1) {
					evtq[oldMove].where_x = evt.where_x;/* Modify existing one  */
					evtq[oldMove].where_y = evt.where_y;
                    evt.what = 0;
					}
				else {
					oldMove = freeHead;	/* Save id of this move event	*/
					}
				}
			else
				oldMove = -1;
			if (msg.wParam & MK_LBUTTON)
				evt.modifiers |= EVT_LEFTBUT;
			if (msg.wParam & MK_RBUTTON)
				evt.modifiers |= EVT_RIGHTBUT;
			if (msg.wParam & MK_SHIFT)
				evt.modifiers |= EVT_SHIFTKEY;
			if (msg.wParam & MK_CONTROL)
				evt.modifiers |= EVT_CTRLSTATE;
			}

		/* Convert keyboard codes */
		TranslateMessage(&msg);
		if (evt.what & EVT_KEYEVT) {
			int scanCode = (msg.lParam >> 16) & 0xFF;
			if (evt.what == EVT_KEYUP) {
            	/* Get message for keyup code from table of cached down values */
				evt.message = keyUpMsg[scanCode];
                keyUpMsg[scanCode] = 0;
				}
			else {
				if (PeekMessage(&charMsg,NULL,WM_CHAR,WM_CHAR,PM_REMOVE))
					evt.message = charMsg.wParam;
				if (PeekMessage(&charMsg,NULL,WM_SYSCHAR,WM_SYSCHAR,PM_REMOVE))
					evt.message = charMsg.wParam;
				evt.message |= ((msg.lParam >> 8) & 0xFF00);
                keyUpMsg[scanCode] = (ushort)evt.message;
				}
			if (evt.what == EVT_KEYREPEAT)
            	evt.message |= (msg.lParam << 16);
			if (HIWORD(msg.lParam) & KF_ALTDOWN)
				evt.modifiers |= EVT_ALTSTATE;
			if (GetKeyState(VK_SHIFT) & 0x8000U)
				evt.modifiers |= EVT_SHIFTKEY;
			if (GetKeyState(VK_CONTROL) & 0x8000U)
				evt.modifiers |= EVT_CTRLSTATE;
			oldMove = -1;
			}

		if (evt.what != 0) {
			/* Add time stamp and add the event to the queue */
			evt.when = msg.time;
			if (count < EVENTQSIZE) {
				AddEvent(&evt);
				}
			}
		DispatchMessage(&msg);
		}
}

ibool MGLAPI EVT_getNext(event_t *evt,uint mask)
/****************************************************************************
*
* Function:     EVT_getNext
* Parameters:   evt     - Place to store retrieved event
*               mask    - Mask of events to retrieve.
* Returns:      True if an event was pending.
*
* Description:  Retrieves the next pending event defined in 'mask' from the
*               event queue. The event queue is adjusted to reflect the new
*               state after the event has been removed.
*
****************************************************************************/
{
	int     evtID,next,prev;

	PumpMessages();
	evt->what = EVT_NULLEVT;                
    if (count) {
		/* It is possible that an event be posted while we are trying
         * to access the event queue. This would create problems since
         * we may end up with invalid data for our event queue pointers. To
         * alleviate this, all interrupts are suspended while we manipulate
		 * our pointers.
		 */

		/* Search for the specified event type in the event queue */
		for (evtID = head; evtID != -1; evtID = evtq[evtID].next) {
			if (evtq[evtID].what & mask)
				break;                      /* Found an event           */
			}
		if (evtID == -1) {
			return false;                   /* Event was not found      */
			}

		next = evtq[evtID].next;
        prev = evtq[evtID].prev;

        if (prev != -1)
            evtq[prev].next = next;
		else
			head = next;

		if (next != -1)
			evtq[next].prev = prev;
		else
			tail = prev;

		*evt = evtq[evtID];                 /* Return the event         */
		evtq[evtID].next = freeHead;        /* and return to free list  */
		freeHead = evtID;
		count--;
		if (evt->what == EVT_MOUSEMOVE)
			oldMove = -1;
		}
	return evt->what != EVT_NULLEVT;
}

ibool MGLAPI EVT_peekNext(event_t *evt,uint mask)
/****************************************************************************
*
* Function:     EVT_peekNext
* Parameters:   evt     - Place to store retrieved event
*               mask    - Mask of events to retrieve.
* Returns:      True if an event is pending.
*
* Description:  Peeks at the next pending event defined in 'mask' in the
*               event queue. The event is not removed from the event queue.
*
****************************************************************************/
{
	int     evtID;

    PumpMessages();
	evt->what = EVT_NULLEVT;
	if (count) {
        /* Search for the specified event type in the event queue */
		for (evtID = head; evtID != -1; evtID = evtq[evtID].next) {
            if (evtq[evtID].what & mask)
                break;                      /* Found an event           */
            }
        if (evtID == -1) 
			return false;                   /* Event was not found      */

		*evt = evtq[evtID];                 /* Return the event         */
		}
	return evt->what != EVT_NULLEVT;
}

ibool MGLAPI EVT_post(ulong which,uint what,ulong message,ulong modifiers)
/****************************************************************************
*
* Function:     EVT_post
* Parameters:	what        - Event code
*               message     - Event message
*               modifiers   - Shift key/mouse button modifiers
* Returns:      True if the event was posted, false if queue is full.
*
* Description:  Posts an event to the event queue. This routine can be used
*               to post any type of event into the queue. If the 'stat'
*               pointer is NULL, the mouse is polled for it's current status.
*
****************************************************************************/
{
	event_t	evt;

	if (count < EVENTQSIZE) {
		/* Save information in event record */

		evt.which = which;
		evt.when = MGL_getTicks();
		evt.what = what;
		evt.message = message;
		evt.modifiers = modifiers;
		AddEvent(&evt);             /* Add to tail of event queue   */
		return true;
		}
    return false;
}

void MGLAPI EVT_flush(uint mask)
/****************************************************************************
*
* Function:     EVT_flush
* Parameters:   mask    - Event mask to use
*
* Description:  Flushes all the event specified in 'mask' from the event
*               queue.
*
****************************************************************************/
{
	event_t	evt;
	do {                            /* Flush all events */
		EVT_getNext(&evt,mask);
		} while (evt.what != EVT_NULLEVT);
}

void MGLAPI EVT_halt(event_t *evt,uint mask)
/****************************************************************************
*
* Function:     EVT_halt
* Parameters:   evt     - Place to store event
*               mask    - Event mask to use
*
* Description:  Halts program execution until a specified event occurs.
*               The event is returned. All pending events not in the
*               specified mask will be ignored and removed from the queue.
*
****************************************************************************/
{
	do {                            /* Wait for an event    */
		EVT_getNext(evt,EVT_EVERYEVT);
		} while (!(evt->what & mask));
}

int	MGLAPI EVT_setTimerTick(int ticks)
/****************************************************************************
*
* Function:     EVT_setTimerTick
* Parameters:   ticks   - Number of ticks between timer tick messages
* Returns:      Previous value for the timer tick event spacing.
*
* Description:  The event module will automatically generate periodic
*               timer tick events for you, with 'ticks' between each event
*               posting. If you set the value of 'ticks' to 0, the timer
*               tick events are turned off.
*
****************************************************************************/
{
	static int	oldticks = 0;
	int			ret = oldticks;

	KillTimer(hwndFullScreen,1);
	if ((oldticks = ticks) != 0)
		SetTimer(hwndFullScreen,1,(ulong)(ticks * 54.925),NULL);
	return ret;
}

ibool MGLAPI EVT_isKeyDown(uchar scanCode)
{
	return (keyUpMsg[scanCode] != 0);
}

void _EVT_init(MGL_HWND hwnd)
{
	InitEvent();
	hwndFullScreen = hwnd;
}

