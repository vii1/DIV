/****************************************************************************
*
*          			WinDirect DOS Compatibility Library
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
* Description:  Routines to provide a DOS event queue, which automatically
*               handles keyboard and mouse events for the WinDirect DOS
*				compatability libraries. Based on the event handling code
*				in the MGL.
*
*               NOTE: It seems that there is a bug in the BC++ IDE debugger,
*                     that prevents you from stepping through code when
*                     the keyboard event handler is installed. Everything
*                     works fine in the standalone debugger however.
*
*                     For this reason you can define NO_IDE_BUG, which will
*                     not install the keyboard event handler, but will
*                     call getch() and kbhit() from the standard library
*                     to get keyboard events.
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <signal.h>
#include "wdirect.h"
#include "pmpro.h"

/*--------------------------- Global variables ----------------------------*/

static int  _WD_cDataStart;

#define EVENTQSIZE  100        		/* Number of events in event queue  */

PRIVATE int     head = -1;      	/* Head of event queue              */
PRIVATE int     tail = -1;      	/* Tail of event queue              */
PRIVATE int     freeHead = -1;  	/* Head of free list                */
PRIVATE int     count = 0;      	/* No. of items currently in queue  */
PRIVATE WD_event evtq[EVENTQSIZE];	/* The queue structure itself      */
PRIVATE int     timerReset;     	/* Value to reset timer with        */
PRIVATE int     timerCount = 0; 	/* Current timer countdown          */
PRIVATE	int		oldMove = -1;		/* Previous movement event			*/
PRIVATE	int		oldKey = -1;		/* Previous key repeat event		*/
PRIVATE ibool    installed = 0;  	/* Event handers installed?         */
PRIVATE	ulong	multiplier = 1193*2L;/* Timer 0 multiplier factor		*/
PRIVATE	int		mx,my;				/* Current mouse position			*/
PRIVATE	int		xRes,yRes;			/* Screen resolution coordinates	*/
PUBLIC	uint	_WD_biosSel = 0;	/* Selector for BIOS data area		*/
PRIVATE int		buttons = 0;
PRIVATE	void 	(_ASMAPI *moveCursor)(int x,int y) = NULL;

extern int	_WD_dataStart;
extern int	_WD_dataEnd;

static int  _WD_cDataEnd;

/*---------------------------- Implementation -----------------------------*/

void 	_ASMAPI _WD_keyISR(void);
uint 	_ASMAPI _WD_disableInt(void);
uint 	_ASMAPI _WD_restoreInt(uint flags);
void 	_ASMAPI _WD_enableInt(void);
void 	_ASMAPI _WD_codeStart(void);
void 	_ASMAPI _WD_codeEnd(void);
void 	_ASMAPI _WD_cCodeStart(void);
void 	_ASMAPI _WD_cCodeEnd(void);
void 	_ASMAPI _WD_sound(int freq);
void 	_ASMAPI _WD_nosound(void);
void 	_ASMAPI _WD_delay(int milliseconds);
uint 	_ASMAPI _WD_readtimer(void);
int 	_ASMAPI _WD_getKeyCode(void);
void 			_WD_maskKeyCode(WD_event *evt);

void 	_ASMAPI _WD_cCodeStart(void) {}

#ifdef	DEBUG_EVENTS
void checkIntegrity(void)
{
	int i,j,k,fail = 0,*zero = NULL;

	if (count > EVENTQSIZE)
		fail = *zero;			/* Cause GPF under protected mode	*/

	/* Check the freeList for integrity */
	for (i = freeHead,j = 0; i != -1; i = evtq[i].next,j++) {
		if (j > EVENTQSIZE || i == head || i == tail)
			fail = *zero;		/* Cause GPF under protected mode	*/
		}

	/* Check the list of events in queue from head forwards */
	for (i = head,j = 0; i != -1; i = evtq[i].next, j++) {
		if (j > EVENTQSIZE)
			fail = *zero;		/* Cause GPF under protected mode	*/
		for (k = freeHead; k != -1; k = evtq[k].next)
			if (k == i)
				fail = *zero;	/* Cause GPF under protected mode	*/
		}

	/* Check the list of events in queue from tail backwards */
	for (i = tail,j = 0; i != -1; i = evtq[i].prev, j++) {
		if (j > EVENTQSIZE)
			fail = *zero;		/* Cause GPF under protected mode	*/
		for (k = freeHead; k != -1; k = evtq[k].next)
			if (k == i)
				fail = *zero;	/* Cause GPF under protected mode	*/
		}
}
#else
#define	checkIntegrity()
#endif

void _ASMAPI WD_delay(int msecs)
/****************************************************************************
*
* Function:		WD_delay
* Parameters:	msecs	- Number of milliseconds to delay
*
****************************************************************************/
{
	ulong 	stop;
	uint	cur,prev;

	stop = (prev = _WD_readtimer()) + (msecs * multiplier);

	while ((cur = _WD_readtimer()) < stop) {
		if (cur < prev) {		/* Check for timer wraparound	*/
			if (stop < 0x10000L)
				break;
			stop -= 0x10000L;
			}
		prev = cur;
		}
}

void _ASMAPI WD_beep(int freq,int msecs)
/****************************************************************************
*
* Function:		WD_beep
* Parameters:	freq	- Frequency to beep at
*				msecs	- Number of millseconds duration for time
*
* Description:	Beeps the PC' speaker at the specified frequency.
*
****************************************************************************/
{
	_WD_sound(freq);
	WD_delay(msecs);
	_WD_nosound();
}

PRIVATE void addEvent(WD_event *evt)
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
*               NOTE: Interrupts MUST be OFF while this routine is called
*                     to ensure we have mutually exclusive access to our
*                     internal data structures.
*
****************************************************************************/
{
    int         evtID;

	checkIntegrity();

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

	checkIntegrity();
}

PRIVATE void addMouseEvent(uint what,uint message,int x,int y,uint but_stat)
/****************************************************************************
*
* Function:     addMouseEvent
* Parameters:   what        - Event code
*               message     - Event message
*               x,y         - Mouse position at time of event
*               but_stat    - Mouse button status at time of event
*
* Description:  Adds a new mouse event to the event queue. This routine is
*               called from within the mouse interrupt subroutine, so it
*               must be efficient.
*
*               NOTE: Interrupts are OFF when this routine is called by
*                     the mouse ISR, and we leave them off
*
****************************************************************************/
{
	WD_event	evt;
	uint    mods;

	if (count < EVENTQSIZE) {
		/* Save information in event record. */
		evt.when = PM_getLong(_WD_biosSel,0x6C);
		evt.what = what;
		evt.message = message;
		evt.modifiers = but_stat;
		evt.where_x = x;                /* Save mouse event position    */
		evt.where_y = y;

		/* Determine and store keyboard shift modifiers */
		mods = PM_getWord(_WD_biosSel,0x17);
		evt.modifiers |= ((mods & 0x000F) << 3) | ((mods & 0x0300) >> 1);

		addEvent(&evt);            		/* Add to tail of event queue   */
		}
	else {
		_WD_enableInt();	/* Unconditionally enable interrupts		*/
		WD_beep(1700,50);	/* Beep the speaker to indicate full queue	*/
		}
}

PRIVATE void _ASMAPI mouseISR(unsigned mask, unsigned butstate,unsigned x,
	unsigned y,unsigned mickeyX,unsigned mickeyY)
/****************************************************************************
*
* Function:     mouseISR
* Parameters:   mask        - Event mask
*               butstate    - Button state
*               x           - Mouse x coordinate
*               y           - Mouse y coordinate
*
* Description:  Mouse event handling routine. This gets called when a mouse
*               event occurs, and we call the addMouseEvent() routine to
*               add the appropriate mouse event to the event queue.
*
*				Note: Interrupts are ON when this routine is called by the
*					  mouse driver code.
*
****************************************************************************/
{
	uint	ps;
	uchar 	mode = PM_getByte(_WD_biosSel,0x49);

	if (mode == 0x0D || mode == 0x13)
		x /= 2;
	if (mask & 1) {
		/* Save the current mouse coordinates */
		mx = x; my = y;

		/* If the last event was a movement event, then modify the last
		 * event rather than post a new one, so that the queue will not
		 * become saturated. Before we modify the data structures, we
		 * MUST ensure that interrupts are off.
		 */
		ps = _WD_disableInt();
		if (oldMove != -1) {
			evtq[oldMove].where_x = x;  		/* Modify existing one  */
			evtq[oldMove].where_y = y;
			}
		else {
			oldMove = freeHead;			/* Save id of this move event	*/
			addMouseEvent(EVT_MOUSEMOVE,0,x,y,butstate);
			}
		_WD_restoreInt(ps);
		}
	if (mask & 0x2A) {
		ps = _WD_disableInt();
		addMouseEvent(EVT_MOUSEDOWN,mask >> 1,x,y,butstate);
		oldMove = -1;
		_WD_restoreInt(ps);
		}
	if (mask & 0x54) {
		ps = _WD_disableInt();
		addMouseEvent(EVT_MOUSEUP,mask >> 2,x,y,butstate);
		oldMove = -1;
		_WD_restoreInt(ps);
		}
	oldKey = -1;
	mickeyX = mickeyX;
	mickeyY = mickeyY;
}

void _WD_moveMouse(int x,int y)
/****************************************************************************
*
* Function:		_WD_moveMouse
*
* Description:	Updates the internal mouse coordinate information in this
*				module.
*
****************************************************************************/
{
	mx = x;
	my = y;
	if (moveCursor)
		moveCursor(x,y);
}

void _ASMAPI _WD_addKeyEvent(uint what,uint message)
/****************************************************************************
*
* Function:     _WD_addKeyEvent
* Parameters:   what        - Event code
*               message     - Event message (ASCII code and scan code)
*
* Description:  Adds a new keyboard event to the event queue. This routine is
*               called from within the keyboard interrupt subroutine, so it
*               must be efficient.
*
*               NOTE: Interrupts are OFF when this routine is called by
*                     the keyboard ISR, and we leave them OFF the entire
*					  time.
*
****************************************************************************/
{
	WD_event	evt;
	uint	mods;

	if (count < EVENTQSIZE) {
		/* Save information in event record */
		evt.when = PM_getLong(_WD_biosSel,0x6C);
		evt.what = what;
		evt.message = message | 0x10000UL;
		evt.where_x = evt.where_y = 0;

		/* Determine and store keyboard shift modifiers */
		mods = PM_getWord(_WD_biosSel,0x17);
		evt.modifiers = ((mods & 0x000F) << 3) | ((mods & 0x0300) >> 1);

		if (evt.what == EVT_KEYREPEAT) {
			if (oldKey != -1)
				evtq[oldKey].message += 0x10000UL;
			else {
				oldKey = freeHead;
				addEvent(&evt);			/* Add to tail of event queue   */
				}
			}
		else addEvent(&evt);			/* Add to tail of event queue   */
		oldMove = -1;
		}
	else {
		_WD_enableInt();	/* Unconditionally enable interrupts		*/
		WD_beep(1700,50);	/* Beep the speaker to indicate full queue	*/
		}
}

PRIVATE void _ASMAPI timerISR(void)
/****************************************************************************
*
* Function:     timerISR
*
* Description:  Timer tick posting subroutine. This routine takes care of
*               periodically posting timer events to the event queue.
*
*				Note: Interrupts are OFF when this routine is entered
*					  during the timer interrupt. We leave them off during
*					  the entire routine.
*
****************************************************************************/
{
	WD_event	evt;
	uint	mods;

    if (timerCount >= 0 && (--timerCount == 0)) {
        if (count < EVENTQSIZE) {
            /* Save information in event record */
			evt.when = PM_getLong(_WD_biosSel,0x6C);
			evt.what = EVT_TIMERTICK;
			evt.message = 0;
			evt.where_x = evt.where_y = 0;

			/* Determine and store keyboard shift modifiers */
			mods = PM_getWord(_WD_biosSel,0x17);
			evt.modifiers = ((mods & 0x000F) << 3) | ((mods & 0x0300) >> 1);

			addEvent(&evt);         /* Add to tail of event queue   */
			oldMove = -1;
			oldKey = -1;
			}
		timerCount = timerReset;
		}
	PM_chainPrevTimer();
}

/* The following two routines fool the mouse driver into working correctly
 * in SuperVGA video modes, by making it think it is always in EGA mode 10h,
 * which means we always get coordinates returned in virtual coordinates.
 */

PRIVATE int _MS_foolDriver(void)
{
	int oldmode = PM_getByte(_WD_biosSel,0x49);
	if (oldmode > 0x13)
		PM_setByte(_WD_biosSel,0x49,0x10);
	return oldmode;
}

PRIVATE void _MS_unfoolDriver(int oldmode)
{
	PM_setByte(_WD_biosSel,0x49,oldmode);
}

PRIVATE void callMouse(RMREGS *r)
/****************************************************************************
*
* Function:		callMouse
* Parameters:	r	- Pointer to register values to load
*
* Description:	Calls the mouse driver with the appropriate values.
*
****************************************************************************/
{
	int oldval = _MS_foolDriver();
	PM_int86(0x33,r,r);
	_MS_unfoolDriver(oldval);
}

PRIVATE ibool MS_installed(void)
/****************************************************************************
*
* Function:		MS_installed
* Returns:		True if a mouse driver is currently installed in the system
*
* Description:	Determines if a software mouse driver is currently installed
*				in the system.
*
****************************************************************************/
{
	RMREGS	regs;
	RMSREGS	sregs;
	uint    sel,off;
	ibool	retval;

	regs.x.ax = 0x3533;					/* Get interrupt vector 0x33	*/
	PM_int86x(0x21,&regs,&regs,&sregs);

	/* Check that interrupt vector 0x33 is not a zero, and that the first
	 * instruction in the interrupt vector is not an IRET instruction
	 */

	retval = true;
	PM_mapRealPointer(&sel,&off,sregs.es, regs.x.bx);
	retval = (sregs.es != 0) && (regs.x.bx != 0) && (PM_getByte(sel,off) != 207);
	return retval;
}

PRIVATE void _MS_init(void)
/****************************************************************************
*
* Function:		_MS_init
*
* Description:	Initialises the mouse module by doing a software reset on
*				the mouse driver (if it is installed).
*
****************************************************************************/
{
	RMREGS	regs;

	if (MS_installed()) {
		regs.x.ax = 33;		/* Mouse function 33 - Software reset		*/
		callMouse(&regs);
		if (regs.x.bx != 0)
			buttons = regs.x.bx;	/* Buttons returned in BX			*/
		}
}

PRIVATE void _MS_setMouseHandler(int mask,PM_mouseHandler mh)
/****************************************************************************
*
* Function:		_MS_setMouseHandler
* Parameters:	mask	- Mouse handler mask
*				mh		- Mouse handler routine to install
*
* Description:	Simply calls the PMODE library to install the mouse
*				handler, but first we must fool the mouse driver to ensure
*				it will work correctly in SuperVGA video modes.
*
****************************************************************************/
{
	if (buttons) {
		int oldmode = _MS_foolDriver();
		PM_setMouseHandler(mask,mh);
		_MS_unfoolDriver(oldmode);
		}
}

PRIVATE void _MS_restoreMouseHandler(void)
{
	if (buttons) {
		int oldmode = _MS_foolDriver();
		PM_restoreMouseHandler();
		_MS_unfoolDriver(oldmode);
		}
}

/*------------------------ Public interface routines ----------------------*/

ibool _WDAPI WD_postEvent(ulong which,uint what,ulong message,ulong modifiers)
/****************************************************************************
*
* Function:     WD_postEvent
* Parameters:	dc			- Device context
*				what        - Event code
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
	WD_event	evt;
	uint	ps;

	if (count < EVENTQSIZE) {
		/* Save information in event record */

		ps = _WD_disableInt();
		evt.which = which;
		evt.when = PM_getLong(_WD_biosSel,0x6C);
		evt.what = what;
		evt.message = message;
		evt.modifiers = modifiers;
		addEvent(&evt);             /* Add to tail of event queue   */
		_WD_restoreInt(ps);
		return true;
		}
	else
		return false;
}

void _WDAPI WD_flushEvent(uint mask)
/****************************************************************************
*
* Function:     WD_flushEvent
* Parameters:   mask    - Event mask to use
*
* Description:  Flushes all the event specified in 'mask' from the event
*               queue.
*
****************************************************************************/
{
	WD_event	evt;

	do {                            /* Flush all events */
		WD_getEvent(&evt,mask);
		} while (evt.what != EVT_NULLEVT);
}

void _WDAPI WD_haltEvent(WD_event *evt,uint mask)
/****************************************************************************
*
* Function:     WD_haltEvent
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
		WD_getEvent(evt,EVT_EVERYEVT);
		} while (!(evt->what & mask));
}

#ifdef NO_IDE_BUG
void NoIDEBug(void)
{
	uint key,ps;

	/* Since the keyboard ISR has not been installed if NO_IDE_BUG has
	 * been defined, we first check for any pending keyboard events
	 * here, and if there are some insert them into the event queue to
	 * be picked up later - what a kludge.
	 */
	while ((key = _WD_getKeyCode()) != 0) {
		ps = _WD_disableInt();
		_WD_addKeyEvent(EVT_KEYDOWN, key);
		_WD_restoreInt(ps);
		}
}
#else
void _ASMAPI _WD_winBugFix(void);
void NoIDEBug(void)
{
	/* This is a real hack to get around bugs under the Windows 3.1 and OS/2
	 * 2.1 DOS shell emulation routines. Unless we continuously call the
	 * Int 16h BIOS keyboard handler code, our own keyboard interrupt
	 * routines end up hanging the system. At this stage we have no idea
	 * what causes this, but this a workaround that solves the problem.
	 */
	_WD_winBugFix();
}
#endif

ibool _WDAPI WD_getEvent(WD_event *evt,uint mask)
/****************************************************************************
*
* Function:     WD_getEvent
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
	uint	ps;

	if (moveCursor)
		moveCursor(mx,my);					/* Move the mouse cursor	*/
	NoIDEBug();
	evt->what = EVT_NULLEVT;                /* Default to null event    */

    if (count) {
		/* It is possible that an event be posted while we are trying
         * to access the event queue. This would create problems since
         * we may end up with invalid data for our event queue pointers. To
         * alleviate this, all interrupts are suspended while we manipulate
		 * our pointers.
		 */
		ps = _WD_disableInt();				/* disable interrupts       */
		for (evtID = head; evtID != -1; evtID = evtq[evtID].next) {
			if (evtq[evtID].what & mask)
				break;                      /* Found an event           */
			}
		if (evtID == -1) {
			_WD_restoreInt(ps);
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
		if (evt->what == EVT_KEYREPEAT)
			oldKey = -1;
		_WD_restoreInt(ps);				/* enable interrupts        */

		if (evt->what & EVT_KEYEVT)
			_WD_maskKeyCode(evt);
		}

	return evt->what != EVT_NULLEVT;
}

ibool _WDAPI WD_peekEvent(WD_event *evt,uint mask)
/****************************************************************************
*
* Function:     WD_peekEvent
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
	uint	ps;

	if (moveCursor)
		moveCursor(mx,my);					/* Move the mouse cursor	*/
	NoIDEBug();
	evt->what = EVT_NULLEVT;             	/* Default to null event    */

    if (count) {
        /* It is possible that an event be posted while we are trying
         * to access the event queue. This would create problems since
         * we may end up with invalid data for our event queue pointers. To
         * alleviate this, all interrupts are suspended while we manipulate
         * our pointers.
         */
		ps = _WD_disableInt();				/* disable interrupts       */
		for (evtID = head; evtID != -1; evtID = evtq[evtID].next) {
			if (evtq[evtID].what & mask)
				break;                      /* Found an event           */
			}
		if (evtID == -1) {
			_WD_restoreInt(ps);
			return false;                   /* Event was not found      */
			}

		*evt = evtq[evtID];                 /* Return the event         */
		_WD_restoreInt(ps);

		if (evt->what & EVT_KEYEVT)
			_WD_maskKeyCode(evt);
		}

	return evt->what != EVT_NULLEVT;
}

void _WD_abort()
{
	printf("Unhandled exception!\n");
	exit(1);
}

WD_HWND	_WDAPI WD_startFullScreen(WD_HWND hwndMain,int _xRes,int _yRes)
/****************************************************************************
*
* Function:     WD_startFullScreen
*
* Description:  Initiliase the event handling module. Here we install our
*               mouse handling ISR to be called whenever any button's are
*               pressed or released. We also build the free list of events
*               in the event queue.
*
*               We use handler number 2 of the mouse libraries interrupt
*               handlers for our event handling routines.
*
****************************************************************************/
{
	static int	locked = 0;
	int			i,stat;

	if (!_WD_biosSel) {
		_WD_biosSel = PM_getBIOSSelector();

		/* Now initialise the timer multiplier factor that we use for
		 * timing in the _EVT_delay routine. This code determines if the
		 * timer is in mode 2 or mode 3. In mode 2 the timer can have both
		 * odd and even values, while in mode 3 only even values.
		 */
		for (i = 0; i < 100; i++)
			if ((_WD_readtimer() & 1) == 0) {
				multiplier = 1193L;
				break;
				}
		}

	/* Ensure we are suspended first */
	WD_restoreGDI();

	/* Initialise mouse module */
	_MS_init();

	/* Build free list, and initialise global data structures */
	for (i = 0; i < EVENTQSIZE; i++)
		evtq[i].next = i+1;
	evtq[EVENTQSIZE-1].next = -1;       /* Terminate list           */
	count = freeHead = 0;
	head = tail = -1;
	timerReset = timerCount = 0;
	oldMove = -1;
	oldKey = -1;

#ifndef NO_IDE_BUG
	PM_setKeyHandler(_WD_keyISR);
#endif
	PM_setTimerHandler(timerISR);
	_MS_setMouseHandler(0xFFFF,mouseISR);

	/* Lock all of the code and data used by our protected mode interrupt
	 * handling routines, so that it will continue to work correctly
	 * under real mode.
	 */
	if (!locked) {
		stat  = !PM_lockDataPages(&_WD_cDataStart,(int)&_WD_cDataEnd-(int)&_WD_cDataStart);
		stat |= !PM_lockDataPages(&_WD_dataStart,(int)&_WD_dataEnd-(int)&_WD_dataStart);
		stat |= !PM_lockCodePages((__codePtr)_WD_cCodeStart,(int)_WD_cCodeEnd-(int)_WD_cCodeStart);
		stat |= !PM_lockCodePages((__codePtr)_WD_codeStart,(int)_WD_codeEnd-(int)_WD_codeStart);
		if (stat) {
			printf("Page locking services failed - interrupt handling not safe!\n");
			exit(1);
			}
		locked = 1;
		}

	/* Catch program termination signals so we can clean up properly */
	signal(SIGABRT, _WD_abort);
	signal(SIGFPE, _WD_abort);
	signal(SIGINT, _WD_abort);
	installed = true;
	/* Grab all characters pending in the keyboard buffer and stuff
	 * them into our event buffer
	 */
	while ((i = _WD_getKeyCode()) != 0)
		WD_postEvent(0,EVT_KEYDOWN,i,0);

	xRes = _xRes;
	yRes = _yRes;
	return hwndMain;
}

void _WDAPI WD_inFullScreen(void)
/****************************************************************************
*
* Function:		WD_inFullScreen
*
* Description:	Lets the library know when fullscreen graphics mode has
*				been initialized so that we can properly scale the mouse
*				driver coordinates.
*
****************************************************************************/
{
	RMREGS	regs;
	int		oldmode;
	uchar	mode;

	/* Do a hardware reset on the mouse driver, to ensure that our
	 * fooling code works correctly, and the mouse cursor moves correctly
	 * in graphics mode. We basically do a mouse reset and then scale
	 * the mouse cursor coordinates to the resolution specified when
	 * fullscreen mode was started.
	 */
	if (buttons) {
		oldmode = _MS_foolDriver();
		PM_resetMouseDriver(1);
		_MS_unfoolDriver(oldmode);
		mode = PM_getByte(_WD_biosSel,0x49);
		if (mode == 0x0D || mode == 0x13)
			xRes *= 2;
		regs.x.ax = 7;	/* Mouse function 7 - Set horizontal min and max */
		regs.x.cx = 0;
		regs.x.dx = xRes;
		callMouse(&regs);
		regs.x.ax = 8;	/* Mouse function 8 - Set vertical min and max	 */
		regs.x.cx = 0;
		regs.x.dx = yRes;
		callMouse(&regs);
		}
}

void _WDAPI WD_restoreGDI(void)
/****************************************************************************
*
* Function:     WD_restoreGDI
*
* Description:  Suspends all of our event handling operations. This is
*               also used to de-install the event handling code.
*
****************************************************************************/
{
	if (installed) {
#ifndef NO_IDE_BUG
		PM_restoreKeyHandler();
#endif
		PM_restoreTimerHandler();
		_MS_restoreMouseHandler();
		PM_unlockCodePages(addEvent,4096);
		PM_unlockDataPages(&head,4096);

		signal(SIGABRT, SIG_DFL);
		signal(SIGFPE, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		installed = false;
		}
}

int _WDAPI WD_setTimerTick(int ticks)
/****************************************************************************
*
* Function:     WD_setTimerTick
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
	int     oldticks;
	uint	ps;

	ps = _WD_disableInt();
	timerCount = ticks;
	oldticks = timerReset;
	timerReset = ticks;
	_WD_restoreInt(ps);

    return oldticks;
}

void _WDAPI WD_setSuspendAppCallback(int (_ASMAPI *saveState)(int flags))
{
	saveState = saveState;
}

void _WDAPI WD_setMousePos(int x,int y)
/****************************************************************************
*
* Function:		WD_setMousePos
* Parameters:	x,y	- Coordinate to move to
*
* Description:	Moves to mouse cursor to the specified coordinate.
*
****************************************************************************/
{
	RMREGS	regs;

	if (buttons) {
		uchar mode = PM_getByte(_WD_biosSel,0x49);
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		regs.x.ax = 4;		/* Mouse function 4 - Set mouse position	*/
		regs.x.cx = x;		/* New horizontal coordinate				*/
		regs.x.dx = y;		/* New vertical coordinate					*/
		if (mode == 0x0D || mode == 0x13)
			regs.x.cx *= 2;
		callMouse(&regs);
		_WD_moveMouse(mx = x,my = y);
		}
}

void _WDAPI WD_getMousePos(int *x,int *y)
/****************************************************************************
*
* Function:		WD_getMousePos
* Parameters:	x,y	- Place to store the mouse coordinate
*
* Description:	Reads the current mouse cursor location int *screen*
*				coordinates.
*
****************************************************************************/
{
	RMREGS	regs;

	if (buttons) {
		uchar mode = PM_getByte(_WD_biosSel,0x49);
		regs.x.ax = 3;		/* Mouse function 3 - Get mouse position	*/
		callMouse(&regs);
		*x = regs.x.cx;
		*y = regs.x.dx;
		if (mode == 0x0D || mode == 0x13)
			*x /= 2;
		}
}

void _WDAPI WD_setMouseCallback(void (_ASMAPI *mcb)(int x,int y))
{ moveCursor = mcb; }

void _ASMAPI _WD_cCodeEnd(void) {}
