/****************************************************************************
*
*						   The Zen Timer Library
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
* Language:		ANSI C, C++ 2.1
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the Zen Timer library. Provides a number
*				of routines to accurately time segments of code. A long
*				period timer is provided to time code that takes up to
*				one hour to execute, with microsecond precision, and an
*				ultra long period timer for timing code that takes up to
*				24 hours to execute (raytracing etc).
*
*				We also provide a set of C++ classes to manipulate
*				the Zen Timers. Note that you can only have one LZTimer
*				running at a time (you can have multiple ULZTimers however),
*				and that the total aggregate time of thr ULZTimer is about
*				65,000 hours, which should suit most timing purposes.
*
*				This library also includes routines for querying the CPU
*				type, CPU speed and CPU features, and includes support for
*				high precision timing on Pentium based systems using the
*				Read Time Stamp Counter. Based on Intel sample code.
*
*
****************************************************************************/

#ifndef	__ZTIMER_H
#define	__ZTIMER_H

#ifndef __SCITECH_H
#include "scitech.h"
#endif

/*--------------------- Macros and type definitions -----------------------*/

/****************************************************************************
REMARKS:
Defines the types of processors returned by CPU_getType.

HEADER:
ztimer.h

MEMBERS:
CPU_i386		- Intel 80386 processor
CPU_i486		- Intel 80486 processor
CPU_Pentium		- Intel Pentium(R) processor
CPU_PentiumPro	- Intel PentiumPro(R) processor
CPU_PentiumII	- Intel PentiumII(R) processor
CPU_UnkPentium	- Unknown Intel Pentium family processor
CPU_Alpha		- DEC Alpha processor
CPU_Mips		- MIPS processor
CPU_PowerPC		- PowerPC processor
CPU_mask		- Mask to remove flags and get CPU type
CPU_IntelClone	- This bit is set if the processor is an Intel clone
****************************************************************************/
typedef enum {
	CPU_unknown		= 0,
	CPU_i386		= 1,
	CPU_i486		= 2,
	CPU_Pentium		= 3,
	CPU_PentiumPro	= 4,
	CPU_PentiumII	= 5,
	CPU_UnkPentium	= 6,
	CPU_Alpha		= 100,
	CPU_Mips		= 200,
	CPU_PowerPC		= 300,
    CPU_mask		= 0x7FFF,
    CPU_IntelClone	= 0x8000,
	} CPU_processorType;

/****************************************************************************
REMARKS:
Defines the structure for holding 64-bit integers used for storing the values
returned by the Intel RDTSC instruction.

HEADER:
ztimer.h

MEMBERS:
low		- Low 32-bits of the 64-bit integer
high	- High 32-bits of the 64-bit integer
****************************************************************************/
typedef struct {
	ulong	low;
	ulong	high;
	} CPU_largeInteger;

/*-------------------------- Function Prototypes --------------------------*/

#define	LZTIMER_RES		1e-6			/* Seconds in a tick			*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Routines to obtain CPU information */

uint	CPU_getProcessorType(void);
ibool	CPU_haveMMX(void);
ulong	CPU_getProcessorSpeed(void);

/* Routine to initialise the library - you MUST call this first! */

void	ZTimerInit(void);

/* Long period timing routines (times up to 1 hour) */

void 	LZTimerOn(void);
ulong	LZTimerLap(void);
void 	LZTimerOff(void);
ulong 	LZTimerCount(void);

/* Ultra long period timing routines (times up to 65,000 hours) */

void	ULZTimerOn(void);
ulong	ULZTimerLap(void);
void	ULZTimerOff(void);
ulong	ULZTimerCount(void);
ulong	ULZReadTime(void);
ulong	ULZElapsedTime(ulong start,ulong finish);
float	ULZTimerResolution(void);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

/*--------------------------- Class Definitions ---------------------------*/

#ifdef	__cplusplus

#ifndef	__IOSTREAM_H
#include <iostream.h>
#endif

//---------------------------------------------------------------------------
// Long Period Zen Timer class. This can be used to time code that takes up
// to 1 hour to execute between calls to start() and stop() or lap(). The
// aggregate count can be up to 2^32 - 1 microseconds (about 1 hour
// and 10 mins).
//---------------------------------------------------------------------------

class LZTimer {
protected:
	ulong	_count;
	short	_overflow;

			// Compute the current count
			void computeTime();

public:
			// Constructor
	inline	LZTimer()		{ ZTimerInit(); _count = 0; _overflow = false; };

			// Method to start the timer
	inline	void start()	{ LZTimerOn(); };

			// Method to restart the timer
	inline	void restart()	{ reset(); start(); };

			// Method to return the current count without stop timer
	inline	ulong lap()		{ return _count + LZTimerLap(); };

			// Method to stop the timer
	inline	void stop()		{ LZTimerOff(); computeTime(); };

			// Method to return the current count
	inline	ulong count()	{ return _count; };

			// Method to reset the timer to a zero count
	inline	void reset()	{ _count = 0; _overflow = false; };

			// Method to determine if overflow occurred
	inline	ibool overflow()	{ return _overflow; };

			// Method to return timer resolution (seconds in a count).
	inline	float resolution()	{ return LZTIMER_RES; };

			// Method to display the timed count in seconds
	friend	ostream& operator << (ostream& o,LZTimer& timer);
	};

//---------------------------------------------------------------------------
// Ultra Long Period Zen Timer class. This can be used to time code that
// takes up 24 hours total to execute between calls to start() and stop().
// The aggregate count can be up to 2^32 - 1 1/18ths of a second, which
// is about 65,000 hours! Should be enough for most applications.
//---------------------------------------------------------------------------

class ULZTimer {
protected:
	ulong	_count,_start,_finish;

public:
			// Constructor
	inline	ULZTimer()		{ ZTimerInit(); _count = 0; };

			// Method to start the timer
	inline	void start()	{ _start = ULZReadTime(); };

			// Method to restart the timer
	inline	void restart()	{ reset(); start(); };

			// Method to return the current count without stoping timer
	inline	ulong lap()		{ return ULZElapsedTime(_start,ULZReadTime()); };

			// Method to stop the timer
			void stop();

			// Method to return the current count
	inline	ulong count()	{ return _count; };

			// Method to reset the timer to a zero count
	inline	void reset()	{ _count = 0; };

			// Method to return timer resolution (seconds in a count).
	inline	float resolution()	{ return ULZTimerResolution(); };

			// Method to display the timed count in seconds
	friend	ostream& operator << (ostream& o,ULZTimer& timer);
	};

#endif

#endif
