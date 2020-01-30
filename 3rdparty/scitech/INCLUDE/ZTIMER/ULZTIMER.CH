/****************************************************************************
*
*						  Ultra Long Period Timer
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
* Environment:	IBM PC (MSDOS and Windows)
*
* Description:	Module to interface to the BIOS Timer Tick for timing
*				code that takes up to 24 hours (ray tracing etc). There
*				is a small overhead in calculating the time, this
*				will be negligible for such long periods of time.
*
*
****************************************************************************/

#include "ztimer.h"
#include "pmode.h"
#ifdef  __WINDOWS__
#define	WIN32_LEAN_AND_MEAN
#define	STRICT
#include <windows.h>
#include <mmsystem.h>
#ifdef	__WINDOWS16__
#include <toolhelp.h>
#endif
#endif

#ifdef	NO_ASSEMBLER
#undef	__INTEL__
#define	__ALPHA__
#endif

/*---------------------------- Global variables ---------------------------*/

static long				cpuSpeed = -1;
static CPU_largeInteger	tmStart,tmEnd;
static ibool				haveRDTSC;
#ifdef	__WINDOWS32__
static CPU_largeInteger	countFreq;
static ibool				havePerformanceCounter;
static ulong			start,finish;
#endif
#ifdef	__WINDOWS16__
static	TIMERINFO		tiStart,tiEnd,tiLap,tiTemp;
static	TIMERINFO		start,finish;
#endif
#ifdef	__REALDOS__
/* {secret} */
ushort _ASMAPI 			_ZTimerBIOS;
static ulong			start,finish;
#endif

/*----------------------------- Implementation ----------------------------*/

#ifdef	__INTEL__
/* {secret} */
ibool  _CPU_haveRDTSC(void);
/* {secret} */
void  _ASMAPI _CPU_readTimeStamp(CPU_largeInteger *time);
/* {secret} */
ulong _ASMAPI _CPU_diffTime64(CPU_largeInteger *t1,CPU_largeInteger *t2,CPU_largeInteger *t);
/* {secret} */
ulong _ASMAPI _CPU_calcMicroSec(CPU_largeInteger *count,ulong freq);
/* {secret} */
void _ASMAPI LZ_timerOn(void);
/* {secret} */
ulong _ASMAPI LZ_timerLap(void);
/* {secret} */
void _ASMAPI LZ_timerOff(void);
/* {secret} */
ulong _ASMAPI LZ_timerCount(void);
/* {secret} */
void _ASMAPI LZ_disable(void);
/* {secret} */
void _ASMAPI LZ_enable(void);
#endif

/* {secret} */
void ZTimerQuickInit(void)
{
#ifdef	__WINDOWS16__
	tiStart.dwSize = tiEnd.dwSize = tiLap.dwSize = sizeof(TIMERINFO);
	tiTemp.dwSize = sizeof(TIMERINFO);
	start.dwSize = finish.dwSize = sizeof(TIMERINFO);
#endif
#ifdef	__REALDOS__
	_ZTimerBIOS = PM_getBIOSSelector();
#endif
}

/****************************************************************************
DESCRIPTION:
Initializes the Zen Timer library.

HEADER:
ztimer.h

REMARKS:
This function initializes the Zen Timer library, and /must/ be called before
any of the remaining Zen Timer library functions are called.
****************************************************************************/
void ZTimerInit(void)
{
	if (cpuSpeed == -1) {
		ZTimerQuickInit();
#ifdef	__INTEL__
		cpuSpeed = CPU_getProcessorSpeed() * 1000000;
		haveRDTSC = _CPU_haveRDTSC() && (cpuSpeed > 0);
#else
		cpuSpeed = 0;
		haveRDTSC = false;
#endif
#ifdef	NO_ASSEMBLER
		havePerformanceCounter = false;
#else
#ifdef	__WINDOWS32__
		havePerformanceCounter = QueryPerformanceFrequency((LARGE_INTEGER*)&countFreq);
#endif
#endif
		}
}

/****************************************************************************
DESCRIPTION:
Starts the Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Starts the Long Period Zen Timer counting. Once you have started the timer,
you can stop it with LZTimerOff or you can latch the current count with
LZTimerLap.

The Long Period Zen Timer uses a number of different high precision timing
mechanisms to obtain microsecond accurate timings results whenever possible.
The following different techniques are used depending on the operating
system, runtime environment and CPU on the target machine. If the target
system has a Pentium CPU installed which supports the Read Time Stamp
Counter instruction (RDTSC), the Zen Timer library will use this to
obtain the maximum timing precision available.

Under 32-bit Windows, if the Pentium RDTSC instruction is not available, we
first try to use the Win32 QueryPerformanceCounter API, and if that is not
available we fall back on the timeGetTime API which is always supported.

Under 32-bit DOS, if the Pentium RDTSC instruction is not available, we
then do all timing using the old style 8253 timer chip. The 8253 timer
routines provide highly accurate timings results in pure DOS mode, however
in a DOS box under Windows or other Operating Systems the virtualization
of the timer can produce inaccurate results.

Note: Because the Long Period Zen Timer stores the results in a 32-bit
	  unsigned integer, you can only time periods of up to 2^32 microseconds,
	  or about 1hr 20mins. For timing longer periods use the Ultra Long
	  Period Zen Timer.

SEE ALSO:
LZTimerOff, LZTimerLap, LZTimerCount
****************************************************************************/
void LZTimerOn(void)
{
	if (haveRDTSC) {
#ifdef	__INTEL__
		_CPU_readTimeStamp(&tmStart);
#endif
		}
#if		defined(__WINDOWS32__)
	else if (havePerformanceCounter)
		QueryPerformanceCounter((LARGE_INTEGER*)&tmStart);
	else
		tmStart.low = timeGetTime();
#elif	defined(__WINDOWS16__)
	else
		TimerCount(&tiStart);
#else
	else
		LZ_timerOn();
#endif
}

/****************************************************************************
DESCRIPTION:
Returns the current count for the Long Period Zen Timer and keeps it
running.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in microseconds.

REMARKS:
Returns the current count that has elapsed since the last call to
LZTimerOn in microseconds. The time continues to run after this function is
called so you can call this function repeatedly.

SEE ALSO:
LZTimerOn, LZTimerOff, LZTimerCount
****************************************************************************/
ulong LZTimerLap(void)
{
	CPU_largeInteger	tmLap,tmCount;

	if (haveRDTSC) {
#ifdef	__INTEL__
		_CPU_readTimeStamp(&tmLap);
		_CPU_diffTime64(&tmStart,&tmLap,&tmCount);
		return _CPU_calcMicroSec(&tmCount,cpuSpeed);
#else
		return 0;
#endif
		}
#if		defined(__WINDOWS32__)
#ifndef	NO_ASSEMBLER
	else if (havePerformanceCounter) {
		QueryPerformanceCounter((LARGE_INTEGER*)&tmLap);
		_CPU_diffTime64(&tmStart,&tmLap,&tmCount);
		return _CPU_calcMicroSec(&tmCount,countFreq.low);
		}
#endif
	else {
		tmLap.low = timeGetTime();
		return (tmLap.low - tmStart.low) * 1000L;
		}
#elif	defined(__WINDOWS16__)
	else {
		TimerCount(&tiLap);
		return (tiLap.dwmsSinceStart - tiStart.dwmsSinceStart) * 1000L;
		}
#else
	else
		return LZ_timerLap();
#endif
}

/****************************************************************************
DESCRIPTION:
Stops the Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Stops the Long Period Zen Timer counting and latches the count. Once you
have stopped the timer you can read the count with LZTimerCount. If you need
highly accurate timing, you should use the on and off functions rather than
the lap function since the lap function does not subtract the overhead of
the function calls from the timed count.

SEE ALSO:
LZTimerOn, LZTimerLap, LZTimerCount
****************************************************************************/
void LZTimerOff(void)
{
	if (haveRDTSC) {
#ifdef	__INTEL__
		_CPU_readTimeStamp(&tmEnd);
#endif
		}
#if		defined(__WINDOWS32__)
	else if (havePerformanceCounter)
		QueryPerformanceCounter((LARGE_INTEGER*)&tmEnd);
	else
		tmEnd.low = timeGetTime();
#elif	defined(__WINDOWS16__)
	else
		TimerCount(&tiEnd);
#else
	else
		LZ_timerOff();
#endif
}

/****************************************************************************
DESCRIPTION:
Returns the current count for the Long Period Zen Timer.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in microseconds.

REMARKS:
Returns the current count that has elapsed between calls to
LZTimerOn and LZTimerOff in microseconds.

SEE ALSO:
LZTimerOn, LZTimerOff, LZTimerLap
****************************************************************************/
ulong LZTimerCount(void)
{
	CPU_largeInteger	tmCount;

	if (haveRDTSC) {
#ifdef	__INTEL__
		_CPU_diffTime64(&tmStart,&tmEnd,&tmCount);
		return _CPU_calcMicroSec(&tmCount,cpuSpeed);
#else
		return 0;
#endif
		}
#if		defined(__WINDOWS32__)
#ifndef	NO_ASSEMBLER
	else if (havePerformanceCounter) {
		_CPU_diffTime64(&tmStart,&tmEnd,&tmCount);
		return _CPU_calcMicroSec(&tmCount,countFreq.low);
		}
#endif
	else
		return (tmEnd.low - tmStart.low) * 1000L;
#elif	defined(__WINDOWS16__)
	else
		return (tiEnd.dwmsSinceStart - tiStart.dwmsSinceStart) * 1000L;
#else
	else
		return LZ_timerCount();
#endif
}

#if		defined(__WINDOWS32__)

/****************************************************************************
DESCRIPTION:
Starts the Ultra Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Starts the Ultra Long Period Zen Timer counting. Once you have started the
timer, you can stop it with ULZTimerOff or you can latch the current count
with ULZTimerLap.

The Ultra Long Period Zen Timer uses the available operating system services
to obtain accurate timings results with as much precision as the operating
system provides, but with enough granularity to time longer periods of
time than the Long Period Zen Timer. Note that the resolution of the timer
ticks is not constant between different platforms, and you should use the
ULZTimerResolution function to determine the number of seconds in a single
tick of the timer, and use this to convert the timer counts to seconds.

Under 32-bit Windows, we use the timeGetTime function which provides a
resolution of 1 millisecond (0.001 of a second). Given that the timer
count is returned as an unsigned 32-bit integer, this we can time intervals
that are a maximum of 2^32 milliseconds in length (or about 1,200 hours or
50 days!).

Under 32-bit DOS, we use the system timer tick which runs at 18.2 times per
second. Given that the timer count is returned as an unsigned 32-bit integer,
this we can time intervals that are a maximum of 2^32 * (1/18.2) in length
(or about 65,550 hours or 2731 days!).

SEE ALSO:
ULZTimerOff, ULZTimerLap, ULZTimerCount, ULZElapsedTime, ULZReadTime
****************************************************************************/
void ULZTimerOn(void)
{ start = timeGetTime(); }

/****************************************************************************
DESCRIPTION:
Returns the current count for the Ultra Long Period Zen Timer and keeps it
running.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in resolution counts.

REMARKS:
Returns the current count that has elapsed since the last call to
ULZTimerOn in microseconds. The time continues to run after this function is
called so you can call this function repeatedly.

SEE ALSO:
ULZTimerOn, ULZTimerOff, ULZTimerCount
****************************************************************************/
ulong ULZTimerLap(void)
{ return (timeGetTime() - start); }

/****************************************************************************
DESCRIPTION:
Stops the Long Period Zen Timer counting.

HEADER:
ztimer.h

REMARKS:
Stops the Ultra Long Period Zen Timer counting and latches the count. Once
you have stopped the timer you can read the count with ULZTimerCount.

SEE ALSO:
ULZTimerOn, ULZTimerLap, ULZTimerCount
****************************************************************************/
void ULZTimerOff(void)
{ finish = timeGetTime(); }

/****************************************************************************
DESCRIPTION:
Returns the current count for the Ultra Long Period Zen Timer.

HEADER:
ztimer.h

RETURNS:
Count that has elapsed in resolution counts.

REMARKS:
Returns the current count that has elapsed between calls to
ULZTimerOn and ULZTimerOff in resolution counts.

SEE ALSO:
ULZTimerOn, ULZTimerOff, ULZTimerLap, ULZTimerResolution
****************************************************************************/
ulong ULZTimerCount(void)
{ return (finish - start); }

/****************************************************************************
DESCRIPTION:
Reads the current time from the Ultra Long Period Zen Timer.

HEADER:
ztimer.h

RETURNS:
Current timer value in resolution counts.

REMARKS:
Reads the current Ultra Long Period Zen Timer and returns it’s current
count. You can use the ULZElapsedTime function to find the elapsed time
between two timer count readings.

SEE ALSO:
ULZElapsedTime, ULZTimerResolution
****************************************************************************/
ulong ULZReadTime(void)
{ return timeGetTime(); }

/****************************************************************************
DESCRIPTION:
Compute the elapsed time between two timer counts.

HEADER:
ztimer.h

PARAMETERS:
start	- Starting time for elapsed count
finish	- Ending time for elapsed count

RETURNS:
Elapsed timer in resolution counts.

REMARKS:
Returns the elapsed time for the Ultra Long Period Zen Timer in units of the
timers resolution (1/18th of a second under DOS). This function correctly
computes the difference even if a midnight boundary has been crossed
during the timing period.

SEE ALSO:
ULZReadTime, ULZTimerResolution
****************************************************************************/
ulong ULZElapsedTime(ulong start,ulong finish)
{ return finish - start; }

/****************************************************************************
DESCRIPTION:
Returns the resolution of the Ultra Long Period Zen Timer.

HEADER:
ztimer.h

RETURNS:
Resolution of the timer in seconds per timer count.

REMARKS:
Returns the resolution of the Ultra Long Period Zen Timer as a floating
point value measured in seconds per timer count.

SEE ALSO:
ULZReadTime, ULZElapsedTime, ULZTimerCount
****************************************************************************/
float ULZTimerResolution(void)
{ return 0.001F; }

#elif	defined(__WINDOWS16__)

void ULZTimerOn(void)
{ TimerCount(&start); }

void ULZTimerOff(void)
{ TimerCount(&finish); }

ulong ULZTimerLap(void)
{
	TimerCount(&tiLap);
	return (tiLap.dwmsSinceStart - start.dwmsSinceStart);
}

ulong ULZTimerCount(void)
{ return (finish.dwmsSinceStart - start.dwmsSinceStart); }

ulong ULZReadTime(void)
{
	TimerCount(&tiTemp);
	return (tiTemp.dwmsSinceStart);
}

ulong ULZElapsedTime(ulong start,ulong finish)
{ return finish - start; }

float ULZTimerResolution(void)
{ return 0.001; }

#else	/* DOS */

void ULZTimerOn(void)
{ start = ULZReadTime(); }

void ULZTimerOff(void)
{ finish = ULZReadTime(); }

ulong ULZTimerLap(void)
{ return ULZElapsedTime(start,ULZReadTime()); }

ulong ULZTimerCount(void)
{ return ULZElapsedTime(start,finish); }

ulong ULZReadTime(void)
{
	ulong	ticks;
	LZ_disable();            /* Turn of interrupts               */
	ticks = PM_getLong(_ZTimerBIOS,0x6C);
	LZ_enable();             /* Turn on interrupts again         */
	return ticks;
}

ulong ULZElapsedTime(ulong start,ulong finish)
{
	/* Check to see whether a midnight boundary has passed, and if so
	 * adjust the finish time to account for this. We cannot detect if
	 * more that one midnight boundary has passed, so if this happens
	 * we will be generating erronous results.
	 */
	if (finish < start)
		finish += 1573040L;			/* Number of ticks in 24 hours		*/
	return finish - start;
}

float ULZTimerResolution(void)
{ return 0.054925; }

#endif	/* DOS */
