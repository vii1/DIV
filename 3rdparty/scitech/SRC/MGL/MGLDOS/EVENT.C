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
*               handles keyboard and mouse events for the MGL.
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

#include "mgl.h"
#pragma hdrstop
#include "mgldos\internal.h"
#include <conio.h>
#include <signal.h>

/*--------------------------- Global variables ----------------------------*/

static int  _EVT_cDataStart;

#define EVENTQSIZE  100             /* Number of events in event queue  */

static int         	head = -1;      /* Head of event queue              */
static int         	tail = -1;      /* Tail of event queue              */
static int         	freeHead = -1;  /* Head of free list                */
static int         	count = 0;      /* No. of items currently in queue  */
static event_t   	evtq[EVENTQSIZE];/* The queue structure itself      */
static int         	timerReset;     /* Value to reset timer with        */
static int         	timerCount = 0; /* Current timer countdown          */
static int			oldMove = -1;	/* Previous movement event			*/
static int			oldKey = -1;	/* Previous key repeat event		*/
static int			mx,my;			/* Current mouse position			*/
static ibool			useEvents = true;/* True to use event handling		*/
static int			haveWin95 = -1;	/* True if Win95 is present			*/
static ibool         (*userEventCallback)(event_t *evt) = NULL;
ibool _VARAPI 		_EVT_installed = 0;/* Event handers installed?  	*/

extern int	_ASMAPI _EVT_dataStart;
extern int	_ASMAPI _EVT_dataEnd;

static int  _EVT_cDataEnd;

/*---------------------------- Implementation -----------------------------*/

/* {secret} */
void _ASMAPI _EVT_keyISR(void);
/* {secret} */
uint _ASMAPI _EVT_disableInt(void);
/* {secret} */
uint _ASMAPI _EVT_restoreInt(uint flags);
/* {secret} */
void _ASMAPI _EVT_enableInt(void);
/* {secret} */
void _ASMAPI _EVT_codeStart(void);
/* {secret} */
void _ASMAPI _EVT_codeEnd(void);
/* {secret} */
void _ASMAPI _EVT_cCodeStart(void);
/* {secret} */
void _ASMAPI _EVT_cCodeEnd(void);
/* {secret} */
void _ASMAPI _EVT_cCodeStart(void) {}
/* {secret} */
ibool ASMAPI _EVT_isKeyDown(uchar scanCode);

#ifdef	DEBUG_EVENTS
static void checkIntegrity(void)
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

static void addEvent(event_t *evt)
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

	/* Call user supplied callback to modify the event if desired */
	if (userEventCallback) {
		if (!userEventCallback(evt))
			return;
		}

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

static void addMouseEvent(uint what,uint message,int x,int y,uint but_stat)
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
	event_t	evt;
	uint    mods;

	if (count < EVENTQSIZE) {
		/* Save information in event record. */
		evt.when = PM_getLong(_MGL_biosSel,0x6C);
		evt.what = what;
		evt.message = message;
		evt.modifiers = but_stat;
		evt.where_x = x;                /* Save mouse event position    */
		evt.where_y = y;

		/* Determine and store keyboard shift modifiers */
		mods = PM_getWord(_MGL_biosSel,0x17);
		evt.modifiers |= ((mods & 0x000F) << 3) | ((mods & 0x0300) >> 1);

		addEvent(&evt);            		/* Add to tail of event queue   */
		}
}

static void _ASMAPI mouseISR(unsigned mask, unsigned butstate,unsigned x,
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
	uchar 	mode = PM_getByte(_MGL_biosSel,0x49);

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
		ps = _EVT_disableInt();
		if (oldMove != -1) {
			evtq[oldMove].where_x = x;  		/* Modify existing one  */
			evtq[oldMove].where_y = y;
			}
		else {
			oldMove = freeHead;			/* Save id of this move event	*/
			addMouseEvent(EVT_MOUSEMOVE,0,x,y,butstate);
			}
		_EVT_restoreInt(ps);
		}
	if (mask & 0x2A) {
		ps = _EVT_disableInt();
		addMouseEvent(EVT_MOUSEDOWN,mask >> 1,x,y,butstate);
		oldMove = -1;
		_EVT_restoreInt(ps);
		}
	if (mask & 0x54) {
		ps = _EVT_disableInt();
		addMouseEvent(EVT_MOUSEUP,mask >> 2,x,y,butstate);
		oldMove = -1;
		_EVT_restoreInt(ps);
		}
	oldKey = -1;
	mickeyX = mickeyX;
	mickeyY = mickeyY;
}

/* {secret} */
void _EVT_moveMouse(int x,int y)
/****************************************************************************
*
* Function:		_EVT_moveMouse
*
* Description:	Updates the internal mouse coordinate information in this
*				module.
*
****************************************************************************/
{
	mx = x;
	my = y;
}

/* {secret} */
void _ASMAPI _EVT_addKeyEvent(uint what,uint message)
/****************************************************************************
*
* Function:     _EVT_addKeyEvent
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
	event_t	evt;
	uint	mods;

	if (count < EVENTQSIZE) {
		/* Save information in event record */
		evt.when = PM_getLong(_MGL_biosSel,0x6C);
		evt.what = what;
		evt.message = message | 0x10000UL;
		evt.where_x = evt.where_y = 0;

		/* Determine and store keyboard shift modifiers */
		mods = PM_getWord(_MGL_biosSel,0x17);
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
}

static void _ASMAPI timerISR(void)
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
	event_t	evt;
	uint	mods;

    if (timerCount >= 0 && (--timerCount == 0)) {
        if (count < EVENTQSIZE) {
            /* Save information in event record */
			evt.when = PM_getLong(_MGL_biosSel,0x6C);
			evt.what = EVT_TIMERTICK;
			evt.message = 0;
			evt.where_x = evt.where_y = 0;

			/* Determine and store keyboard shift modifiers */
			mods = PM_getWord(_MGL_biosSel,0x17);
			evt.modifiers = ((mods & 0x000F) << 3) | ((mods & 0x0300) >> 1);

			addEvent(&evt);         /* Add to tail of event queue   */
			oldMove = -1;
			oldKey = -1;
            }
        timerCount = timerReset;
        }
    PM_chainPrevTimer();
}

/*------------------------ Public interface routines ----------------------*/

/****************************************************************************
DESCRIPTION:
Posts a user defined event to the event queue

HEADER:
mgraph.h

RETURNS:
True if event was posted, false if event queue is full.

PARAMETERS:
what		- Type code for message to post
message		- Event specific message to post
modifiers	- Event specific modifier flags to post

REMARKS:
This routine is used to post user defined events to the event queue.

SEE ALSO:
EVT_flush, EVT_getNext, EVT_peekNext, EVT_halt
****************************************************************************/
ibool MGLAPI EVT_post(
	ulong which,
	uint what,
	ulong message,
	ulong modifiers)
{
	event_t	evt;
	uint	ps;

	if (count < EVENTQSIZE) {
		/* Save information in event record */

		ps = _EVT_disableInt();
		evt.which = which;
		evt.when = PM_getLong(_MGL_biosSel,0x6C);
		evt.what = what;
		evt.message = message;
		evt.modifiers = modifiers;
		addEvent(&evt);             /* Add to tail of event queue   */
		_EVT_restoreInt(ps);
		return true;
		}
	else
		return false;
}

/****************************************************************************
DESCRIPTION:
Flushes all events of a specified type from the event queue.

PARAMETERS:
mask	- Mask specifying the types of events that should be removed

HEADER:
mgraph.h

REMARKS:
Flushes (removes) all pending events of the specified type from the event queue.
You may combine the masks for different event types with a simple logical OR.

SEE ALSO:
EVT_getNext, EVT_halt, EVT_peekNext
****************************************************************************/
void MGLAPI EVT_flush(
	uint mask)
{
	event_t	evt;

	do {                            /* Flush all events */
		EVT_getNext(&evt,mask);
		} while (evt.what != EVT_NULLEVT);
}

/****************************************************************************
DESCRIPTION:
Flushes all events of a specified type from the event queue.

HEADER:
mgraph.h

PARAMETERS:
evt		- Pointer to
mask	- Mask specifying the types of events that should be removed

REMARKS:
Flushes (removes) all pending events of the specified type from the event queue.
You may combine the masks for different event types with a simple logical OR.

SEE ALSO:
EVT_getNext, EVT_halt, EVT_peekNext
****************************************************************************/
void MGLAPI EVT_halt(
	event_t *evt,
	uint mask)
{
	do {                            /* Wait for an event    */
		EVT_getNext(evt,EVT_EVERYEVT);
		} while (!(evt->what & mask));
}

#ifdef NO_IDE_BUG
/* {secret} */
void NoIDEBug(void)
{
	uint key,ps;

	/* Since the keyboard ISR has not been installed if NO_IDE_BUG has
	 * been defined, we first check for any pending keyboard events
	 * here, and if there are some insert them into the event queue to
	 * be picked up later - what a kludge.
	 */
	while ((key = _EVT_getKeyCode()) != 0) {
		ps = _EVT_disableInt();
		_EVT_addKeyEvent(EVT_KEYDOWN, key);
		_EVT_restoreInt(ps);
		}
}
#else
/* This is a real hack to get around bugs under the Windows 3.1 and OS/2
 * 2.1 DOS shell emulation routines. Unless we continuously call the
 * Int 16h BIOS keyboard handler code, our own keyboard interrupt
 * routines end up hanging the system. At this stage we have no idea
 * what causes this, but this a workaround that solves the problem.
 */
/* {secret} */
void _ASMAPI _EVT_winBugFix(void);
#define	NoIDEBug()	if (!haveWin95) _EVT_winBugFix()
#endif

/****************************************************************************
DESCRIPTION:
Retrieves the next pending event from the event queue.

PARAMETERS:
evt		- Pointer to structure to return the event info in
mask	- Mask specifying the types of events that should be removed

HEADER:
mgraph.h

RETURNS:
True if an event was pending, false if not.

REMARKS:
Retrieves the next pending event from the event queue, and stores it in a event_t
structure. The mask parameter is used to specify the type of events to be removed,
and can be any logical combination of any of the flags defined by the
MGL_eventType enumeration.

The what field of the event contains the event code of the event that was extracted.
All application specific events should begin with the EVT_USEREVT code and
build from there. Since the event code is stored in an integer, there is a maximum of
16 different event codes that can be distinguished (32 for the 32 bit version). You
can store extra information about the event in the message field to distinguish
between events of the same class (for instance the button used in a
EVT_MOUSEDOWN event).

If an event of the specified type was not in the event queue, the what field of the
event will be set to NULLEVT, and the return value will return false.

The EVT_TIMERTICK event is used to report that a specified time interval has
elapsed since the last EVT_TIMERTICK event occurred. See EVT_setTimerTick()
for information on how to enable timer tick events and to set the timer interval.

SEE ALSO:
EVT_flush, EVT_halt, EVT_peekNext, EVT_setTimerTick
****************************************************************************/
ibool MGLAPI EVT_getNext(
	event_t *evt,
	uint mask)
{
	int     evtID,next,prev;
	uint	ps;

	_MS_moveCursor(mx,my);					/* Move the mouse cursor	*/
	NoIDEBug();
	evt->what = EVT_NULLEVT;                /* Default to null event    */

	if (count) {
		/* It is possible that an event be posted while we are trying
		 * to access the event queue. This would create problems since
		 * we may end up with invalid data for our event queue pointers. To
		 * alleviate this, all interrupts are suspended while we manipulate
		 * our pointers.
		 */
		ps = _EVT_disableInt();				/* disable interrupts       */
		for (evtID = head; evtID != -1; evtID = evtq[evtID].next) {
			if (evtq[evtID].what & mask)
				break;                      /* Found an event           */
			}
		if (evtID == -1) {
			_EVT_restoreInt(ps);
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
		_EVT_restoreInt(ps);				/* enable interrupts        */

		if (evt->what & EVT_KEYEVT)
			_EVT_maskKeyCode(evt);
		}

	return evt->what != EVT_NULLEVT;
}

/****************************************************************************
DESCRIPTION:
Peeks at the next pending event in the event queue.

HEADER:
mgraph.h

RETURNS:
True if an event is pending, false if not.

PARAMETERS:
evt		- Pointer to structure to return the event info in
mask	- Mask specifying the types of events that should be removed

REMARKS:
Peeks at the next pending event of the specified type in the event queue. The mask
parameter is used to specify the type of events to be peeked at, and can be any
logical combination of any of the flags defined by the MGL_eventType enumeration.

In contrast to EVT_getNext, the event is not removed from the event queue. You
may combine the masks for different event types with a simple logical OR.

SEE ALSO:
EVT_flush, EVT_getNext, EVT_halt
****************************************************************************/
ibool MGLAPI EVT_peekNext(
	event_t *evt,
	uint mask)
{
	int     evtID;
	uint	ps;

	_MS_moveCursor(mx,my);					/* Move the mouse cursor	*/
	NoIDEBug();
	evt->what = EVT_NULLEVT;             	/* Default to null event    */

	if (count) {
		/* It is possible that an event be posted while we are trying
		 * to access the event queue. This would create problems since
		 * we may end up with invalid data for our event queue pointers. To
		 * alleviate this, all interrupts are suspended while we manipulate
		 * our pointers.
		 */
		ps = _EVT_disableInt();				/* disable interrupts       */
		for (evtID = head; evtID != -1; evtID = evtq[evtID].next) {
			if (evtq[evtID].what & mask)
				break;                      /* Found an event           */
			}
		if (evtID == -1) {
			_EVT_restoreInt(ps);
			return false;                   /* Event was not found      */
			}

		*evt = evtq[evtID];                 /* Return the event         */
		_EVT_restoreInt(ps);

		if (evt->what & EVT_KEYEVT)
			_EVT_maskKeyCode(evt);
		}

	return evt->what != EVT_NULLEVT;
}

static int detectWin95(void)
/****************************************************************************
*
* Function:		detectWin95
*
****************************************************************************/
{
	RMREGS	regs;

	regs.x.ax = 0x160A;
	PM_int86(0x2F,&regs,&regs);
	if (regs.x.ax == 0) {
		if (regs.h.bh != 3)
			return true;
		}
	return false;
}

/* {secret} */
void _EVT_init(void)
/****************************************************************************
*
* Function:     _EVT_init
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
	int	i;

	MGLPC_init();						/* Init _MGL_biosSel		*/
	_EVT_resume();
	haveWin95 = detectWin95();

	/* Grab all characters pending in the keyboard buffer and stuff
	 * them into our event buffer
	 */
	while ((i = _EVT_getKeyCode()) != 0)
		EVT_post(0,EVT_KEYDOWN,i,0);
}

/* {secret} */
void _EVT_abort()
{
	MGL_fatalError("Unhandled exception!");
}

/* {secret} */
void _EVT_resume(void)
/****************************************************************************
*
* Function:     _EVT_resume
*
* Description:  Installs the event handling code for the MGL.
*
****************************************************************************/
{
	static int	locked = 0;
	int			i,stat;

	if (useEvents) {
		/* Build free list, and initialize global data structures */
		for (i = 0; i < EVENTQSIZE; i++)
			evtq[i].next = i+1;
		evtq[EVENTQSIZE-1].next = -1;       /* Terminate list           */
		count = freeHead = 0;
		head = tail = -1;
		timerReset = timerCount = 0;
		oldMove = -1;
		oldKey = -1;

#ifndef NO_IDE_BUG
		PM_setKeyHandler(_EVT_keyISR);
#endif
		PM_setTimerHandler(timerISR);
		_MS_setMouseHandler(0xFFFF,mouseISR);

		/* Lock all of the code and data used by our protected mode interrupt
		 * handling routines, so that it will continue to work correctly
		 * under real mode.
		 */
		if (!locked) {
			stat  = !PM_lockDataPages(&_EVT_cDataStart,(int)&_EVT_cDataEnd-(int)&_EVT_cDataStart);
			stat |= !PM_lockDataPages(&_EVT_dataStart,(int)&_EVT_dataEnd-(int)&_EVT_dataStart);
			stat |= !PM_lockCodePages((__codePtr)_EVT_cCodeStart,(int)_EVT_cCodeEnd-(int)_EVT_cCodeStart);
			stat |= !PM_lockCodePages((__codePtr)_EVT_codeStart,(int)_EVT_codeEnd-(int)_EVT_codeStart);
			if (stat) {
				printf("Page locking services failed - interrupt handling not safe!\n");
				exit(1);
				}
			locked = 1;
			}

		/* Catch program termination signals so we can clean up properly */
		signal(SIGABRT, _EVT_abort);
		signal(SIGFPE, _EVT_abort);
		signal(SIGINT, _EVT_abort);
		_EVT_installed = true;
		}
}

/* {secret} */
void _EVT_suspend(void)
/****************************************************************************
*
* Function:     _EVT_suspend
*
* Description:  Suspends all of our event handling operations. This is
*               also used to de-install the event handling code.
*
****************************************************************************/
{
	if (_EVT_installed) {
#ifndef NO_IDE_BUG
		PM_restoreKeyHandler();
#endif
		PM_restoreTimerHandler();
		_MS_restoreMouseHandler();
		signal(SIGABRT, SIG_DFL);
		signal(SIGFPE, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		_EVT_installed = false;
		}
}

/****************************************************************************
DESCRIPTION:
Enables or disables event handling functions.

HEADER:
mgldos.h

PARAMETERS:
use	- True to enable events, false to disable.

REMARKS:
This function allows the application programmer to enable or disable the use of the
MGL event handling functions for DOS applications. If you wish to use MGL for
graphics output, but you already have your own functions for handling keyboard
and mouse input, you can call this function before you call MGL_init for the first
time and it will disable the built in MGL event handling code.

SEE ALSO:
MGL_suspend, MGL_resume
****************************************************************************/
void MGLAPI MGL_useEvents(
	ibool use)
{ useEvents = use; }

/****************************************************************************
DESCRIPTION:
Installs a user supplied event filter callback for DOS event handling.

HEADER:
mgldos.h

PARAMETERS:
userEventFilter	- Address of user supplied event filter callback

REMARKS:
This function allows the application programmer to install an event filter
callback for DOS event handling. Once you install your callback, the MGL
event handling routines will call your callback with a pointer to the
new event that will be placed into the event queue. Your callback can the
modify the contents of the event before it is placed into the queue (for
instance adding custom information or perhaps high precision timing
information).

If your callback returns FALSE, the event will be ignore and will not be
posted to the event queue. You should always return true from your event
callback unless you plan to use the events immediately that they are
recieved.

Note:	Your event callback will be called in response to a hardware
		interrupt and will be executing in the context of the hardware
		interrupt handler (ie: keyboard interrupt, mouse interrupt or
		timer interrupt). For this reason the MGL automatically locks down
		the code pages for the callback that you register with the
		PM_lockCodePages function. If your filter callback calls any other
		functions, either place those function immediately *after* your
		filter callback, or explicitly call PM_lockCodePages for all the
		functions that might be called from your handler. You should also
		ensure that your filter callback runs as fast as possible to ensure
		that the system is ready to recieve the next event function.

Note:   You can also use this filter callback to process events at the
		time they are activated by the user (ie: when the user hits the
		key or moves the mouse), but make sure your code runs as fast as
		possible as it will be executing inside the context of an interrupt
		handler.

SEE ALSO:
EVT_getNext, EVT_peekNext
****************************************************************************/
void MGLAPI MGL_setUserEventFilter(
	ibool (*userEventFilter)(event_t *evt))
{
	if ((userEventCallback = userEventFilter) != NULL)
		PM_lockCodePages((__codePtr)userEventCallback,4096);
}

/****************************************************************************
DESCRIPTION:
Set the interval between EVT_TIMERTICK events.

HEADER:
mgraph.h

RETURNS:
Old value of timer tick interval

PARAMETERS:
ticks	- New value for timer tick interval (in milliseconds)

REMARKS:
This routine sets the number of ticks between each posting of the
EVT_TIMERTICK event to the event queue. The EVT_TIMERTICK event is off
by default. You can turn off the posting of EVT_TIMERTICK events by setting the
tick interval to 0.

Under Windows, one tick is approximately equal to 1/1000 of a second
(millisecond).

Under MSDOS, one tick is approximately equal to 1/18.2 of a second. To work out
a precise interval given in seconds, use the following expression:

	ticks = secs * (1193180.0 / 65536.0);

****************************************************************************/
int MGLAPI EVT_setTimerTick(
	int ticks)
{
	int     oldticks;
	uint	ps;

	ps = _EVT_disableInt();
	timerCount = ticks;
	oldticks = timerReset;
	timerReset = ticks;
	_EVT_restoreInt(ps);

	return oldticks;
}

/* {secret} */
void _ASMAPI _EVT_cCodeEnd(void) {}

/* The following contains fake C prototypes and documentation for the
 * macro functions in the MGRAPH.H header file. These exist soley so
 * that DocJet will correctly pull in the documentation for these functions.
 */
#if 0

/****************************************************************************
DESCRIPTION:
Macro to extract the ASCII code from a message.

PARAMETERS:
message	- Message to extract ASCII code from

RETURNS:
ASCII code extracted from the message.

HEADER:
mgraph.h

REMARKS:
Macro to extract the ASCII code from the message field of the event_t
structure. You pass the message field to the macro as the parameter and
the ASCII code is the result, for example:

	event_t myEvent;
	uchar   code;
	code = MGL_asciiCode(myEvent.message);

SEE ALSO:
EVT_scanCode, EVT_repeatCount
****************************************************************************/
uchar EVT_asciiCode(
	ulong message);

/****************************************************************************
DESCRIPTION:
Macro to extract the keyboard scan code from a message.

HEADER:
mgraph.h

PARAMETERS:
message	- Message to extract scan code from

RETURNS:
Keyboard scan code extracted from the message.

REMARKS:
Macro to extract the keyboard scan code from the message field of the event
structure. You pass the message field to the macro as the parameter and
the scan code is the result, for example:

	event_t myEvent;
	uchar   code;
	code = MGL_scanCode(myEvent.message);

SEE ALSO:
EVT_asciiCode, EVT_repeatCount
****************************************************************************/
uchar EVT_scanCode(
	ulong message);

/****************************************************************************
DESCRIPTION:
Macro to extract the repeat count from a message.

HEADER:
mgraph.h

PARAMETERS:
message	- Message to extract repeat count from

RETURNS:
Repeat count extracted from the message.

REMARKS:
Macro to extract the repeat count from the message field of the event
structure. The repeat count is the number of times that the key repeated
before there was another keyboard event to be place in the queue, and
allows the event handling code to avoid keyboard buffer overflow
conditions when a single key is held down by the user. If you are processing
a key repeat code, you will probably want to check this field to see how
many key repeats you should process for this message.

SEE ALSO:
EVT_asciiCode, EVT_repeatCount
****************************************************************************/
short EVT_repeatCount(
	ulong message);

/****************************************************************************
DESCRIPTION:
Determines if a specified key is currently down.

PARAMETERS:
scanCode	- Scan code to test

RETURNS:
True of the specified key is currently held down.

HEADER:
mgraph.h

REMARKS:
This function determines if a specified key is currently down at the
time that the call is made. You simply need to pass in the scan code of
the key that you wish to test, and the MGL will tell you if it is currently
down or not. The MGL does this by keeping track of the up and down state
of all the keys.
****************************************************************************/
ibool MGLAPI EVT_isKeyDown(
	uchar scanCode)
{
	return _EVT_isKeyDown(scanCode);
}

#endif
