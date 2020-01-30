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
* Language:		ANSI C
* Environment:	IBM PC (MSDOS and Windows)
*
* Description:	Module to interface to the BIOS Timer Tick for timing
*				code that takes up to 24 hours (ray tracing etc). There
*				is a small overhead in calculating the time, this
*				will be negligible for such long periods of time.
*
****************************************************************************/

#include "ztimer.h"
#include <math.h>
#ifdef	__WINDOWS32__
#define	WIN32_LEAN_AND_MEAN
#define	STRICT
#include <windows.h>
#include <mmsystem.h>
#endif

#ifdef	NO_ASSEMBLER
#undef	__INTEL__
#define	__ALPHA__
#endif

#ifdef	DEBUG
#define	DB(p)	p
#else
#define	DB(p)
#endif

/*----------------------------- Implementation ----------------------------*/

/* External Intel assembler functions */
#ifdef	__INTEL__
/* {secret} */
ibool	_ASMAPI _CPU_haveCPUID(void);
/* {secret} */
ibool	_ASMAPI _CPU_check80386(void);
/* {secret} */
ibool	_ASMAPI _CPU_check80486(void);
/* {secret} */
uint	_ASMAPI _CPU_checkCPUID(void);
/* {secret} */
uint	_ASMAPI _CPU_getCPUIDModel(void);
/* {secret} */
uint	_ASMAPI _CPU_getCPUIDFeatures(void);
/* {secret} */
ibool	_ASMAPI _CPU_checkClone(void);
/* {secret} */
ulong	_ASMAPI _CPU_quickRDTSC(void);
/* {secret} */
void	_ASMAPI	_CPU_runBSFLoop(int iterations);
/* {secret} */
void ZTimerQuickInit(void);
#define	CPU_HaveMMX		0x00800000
#define	CPU_HaveRDTSC	0x00000010
#endif

/****************************************************************************
DESCRIPTION:
Returns the type of processor in the system.

HEADER:
ztimer.h

RETURNS:
Numerical identifier for the installed processor

REMARKS:
Returns the type of processor in the system. Note that if the CPU is an
unknown Pentium family processor that we don't have an enumeration for,
the return value will be greater than or equal to the value of CPU_UnkPentium
(depending on the value returned by the CPUID instruction).

SEE ALSO:
CPU_getProcessorSpeed, CPU_haveMMX
****************************************************************************/
uint CPU_getProcessorType(void)
{
#if		defined(__INTEL__)
	uint	cpu,clone,model;

	if (_CPU_haveCPUID()) {
		cpu = _CPU_checkCPUID();
		clone = cpu & CPU_IntelClone;
		switch (cpu & CPU_mask) {
			case 4:	cpu = CPU_i486;			break;
			case 5:	cpu = CPU_Pentium;		break;
			case 6:
				model = _CPU_getCPUIDModel();
				if (model == 1)
					cpu = CPU_PentiumPro;
				else
					cpu = CPU_PentiumII;
				break;
			default:
				cpu = CPU_UnkPentium;
			}
		}
	else {
		clone = _CPU_checkClone();
		if (_CPU_check80386())
			cpu = CPU_i386;
		else  if (_CPU_check80486())
			cpu = CPU_i486;
		else
			cpu = CPU_Pentium;
		}
	return cpu | clone;
#elif	defined(__ALPHA__)
	return CPU_Alpha;
#elif	defined(__MIPS__)
	return CPU_Mips;
#elif	defined(__PPC__)
	return CPU_PowerPC;
#endif
}

/****************************************************************************
DESCRIPTION:
Returns true if the processor supports Intel MMX extensions.

HEADER:
ztimer.h

RETURNS:
True if MMX is available, false if not.

REMARKS:
This function determines if the processor supports the Intel MMX extended
instruction set. If the processor is not an Intel or Intel clone CPU, this
function will always return false.

SEE ALSO:
CPU_getProcessorType, CPU_getProcessorSpeed
****************************************************************************/
ibool CPU_haveMMX(void)
{
#ifdef	__INTEL__
	if (_CPU_haveCPUID())
		return (_CPU_getCPUIDFeatures() & CPU_HaveMMX) != 0;
	return false;
#else
	return false;
#endif
}

/****************************************************************************
DESCRIPTION:
Returns true if the processor supports the RDTSC instruction

HEADER:
ztimer.h

RETURNS:
True if the RTSC instruction is available, false if not.

REMARKS:
This function determines if the processor supports the Intel RDTSC
instruction, for high precision timing. If the processor is not an Intel or
Intel clone CPU, this function will always return false.

SEE ALSO:
CPU_getProcessorType, CPU_isMMXAvailable
****************************************************************************/
/* {secret} */
ibool _CPU_haveRDTSC(void)
{
#ifdef	__INTEL__
	if (_CPU_haveCPUID())
		return (_CPU_getCPUIDFeatures() & CPU_HaveRDTSC) != 0;
	return false;
#else
	return false;
#endif
}

#ifdef	__INTEL__
#ifdef	__WINDOWS32__
static ibool havePerformanceCounter;
static void GetCounterFrequency(CPU_largeInteger *freq)
{
	if (!QueryPerformanceFrequency((LARGE_INTEGER*)freq)) {
		havePerformanceCounter = false;
		freq->low = 100000;
		freq->high = 0;
		}
	else
		havePerformanceCounter = true;
}
#define	GetCounter(t)										\
	if (havePerformanceCounter)                             \
		QueryPerformanceCounter((LARGE_INTEGER*)t);			\
	else {                                                  \
		(t)->low = timeGetTime() * 100L;                    \
		(t)->high = 0;                                      \
		}
#elif	defined(__WINDOWS16__)
static void GetCounterFrequency(CPU_largeInteger *freq)
{
	ZTimerQuickInit();
	freq->low = 100000;
	freq->high = 0;
}

static void GetCounter(CPU_largeInteger *t)
{
	t->low = ULZReadTime() * 100L;
	t->high = 0;
}
#else	/* MSDOS */
static void GetCounterFrequency(CPU_largeInteger *freq)
{
	ZTimerQuickInit();
	freq->low = (ulong)(5500.0 / ULZTimerResolution());
	freq->high = 0;
}

static void GetCounter(CPU_largeInteger *t)
{
	t->low = ULZReadTime() * 5500L;
	t->high = 0;
}
#endif

#define MAXCLOCKS		150
#define ROUND_THRESHOLD	6
#define ITERATIONS		16000
#define MAX_TRIES		20
#define TOLERANCE		1
#define	SAMPLINGS		10

/****************************************************************************
REMARKS:
If processor does not support time stamp reading, but is at least a 386 or
above, utilize method of timing a loop of BSF instructions which take a
known number of cycles to run on i386(tm), i486(tm), and Pentium(R)
processors.
****************************************************************************/
static ulong GetBSFCpuSpeed(ulong cycles)
{
	CPU_largeInteger t0,t1,count_freq;
	ulong  	ticks;				// Microseconds elapsed during test
	ulong 	current;      		// Variable to store time elapsed during loop of of BSF instructions
	int 	i;
	ulong 	lowest  = (ulong)-1;
#ifdef	__WINDOWS32__
	int 	iPriority;
	HANDLE 	hThread = GetCurrentThread();

	/* Increase thread priority */
	iPriority = GetThreadPriority(hThread);
	if (iPriority != THREAD_PRIORITY_ERROR_RETURN)
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
#endif

#ifdef	__REALDOS__
	count_freq.low = 100000L;
	count_freq.high = 0;
	ZTimerQuickInit();
	LZTimerOn();
#else
	GetCounterFrequency(&count_freq);
#endif
	for (i = 0; i < SAMPLINGS; i++) {
#ifdef	__REALDOS__
		t0.low = LZTimerLap()/10;
#else
		GetCounter(&t0);
#endif
		_CPU_runBSFLoop(ITERATIONS);
#ifdef	__REALDOS__
		t1.low = LZTimerLap()/10;
#else
		GetCounter(&t1);
#endif
		current = t1.low - t0.low;
		if (current < lowest)
			lowest = current;
		}
#ifdef	__REALDOS__
	LZTimerOff();
#endif

#ifdef	__WINDOWS32__
	/* Restore thread priority */
	if (iPriority != THREAD_PRIORITY_ERROR_RETURN)
		SetThreadPriority(hThread, iPriority);
#endif

	/* Compute frequency */
	ticks = lowest;
	ticks = ticks * 100000;
	ticks = ticks / (count_freq.low/10);
	if ((ticks % count_freq.low) > (count_freq.low/2))
		ticks++;			/* Round up if necessary 					*/
	if (ticks == 0)
		return 0;
	return (cycles/ticks);
}

/****************************************************************************
REMARKS:
On processors supporting the Read Time Stamp opcode, compare elapsed
time on the High-Resolution Counter with elapsed cycles on the Time
Stamp Register.

The inner loop runs up to 20 times oruntil the average of the previous
three calculated frequencies is within 1 MHz of each of the individual
calculated frequencies. This resampling increases the accuracy of the
results since outside factors could affect this calculation.
****************************************************************************/
static ulong GetRDTSCCpuSpeed(void)
{
	CPU_largeInteger	t0,t1,count_freq;
	ulong 	freq  =0;				/* Most current frequ. calculation                      */
	ulong 	freq2 =0;				/* 2nd most current frequ. calc.                        */
	ulong	freq3 =0;				/* 3rd most current frequ. calc.                        */
	ulong 	total;					/* Sum of previous three frequency calculations         */
	int    	tries=0;				/* Number of times a calculation has been made          */
	ulong  	total_cycles=0, cycles;	/* Clock cycles elapsed during test                     */
	ulong  	stamp0, stamp1;			/* Time Stamp Variable for beginning and end of test    */
	ulong  	total_ticks=0, ticks;	/* Microseconds elapsed during test                     */
#ifdef	__WINDOWS32__
	int 	iPriority;
	HANDLE 	hThread = GetCurrentThread();

	/* Increase thread priority */
	iPriority = GetThreadPriority(hThread);
	if (iPriority != THREAD_PRIORITY_ERROR_RETURN)
		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
#endif

	GetCounterFrequency(&count_freq);
	DB(printf("Counter Frequency is (%u, %u)\n", count_freq.high, count_freq.low));
	do {
		tries++;		/* Increment number of times sampled */
		freq3 = freq2;	/* Shift frequencies back */
		freq2 = freq;

		/* Loop until 50 ticks have  passed	since last read of hi-res counter.
		 * This accounts for overhead later.
		 */
		GetCounter(&t0);
		t1.low = t0.low;
		t1.high = t0.high;
		while ((t1.low - t0.low) < 50) {
			GetCounter(&t1);
			stamp0 = _CPU_quickRDTSC();
			}

		/* Loop until 1000 ticks have passed since last read of hi-res counter.
		 * This allows for elapsed time for sampling.
		 */
		t0.low = t1.low;
		t0.high = t1.high;
		while ((t1.low - t0.low) < 1000) {
			GetCounter(&t1);
			stamp1 = _CPU_quickRDTSC();
			}

		/* Find the difference during the timing loop */
		cycles = stamp1 - stamp0;
		ticks = t1.low - t0.low;

		DB(printf("Counter 0 is %u\n", t0.low));
		DB(printf("Counter 1 is %u\n", t1.low));
		DB(printf("Counter diff is %u\n", ticks));
		DB(printf("Stamp 0 is %u\n", stamp0));
		DB(printf("Stamp 1 is %u\n", stamp1));
		DB(printf("Stamp diff is %u\n", cycles));

		/* Note that some seemingly arbitrary mulitplies and  divides are done
		 * below. This is to maintain a high level of precision without truncating
		 * the most significant data. According to what value ITERATIIONS is set
		 * to, these multiplies and divides might need to be shifted for optimal
		 * precision.
         */
		ticks = ticks * 100000;
		ticks = ticks / (count_freq.low/10);		
		total_ticks += ticks;
		total_cycles += cycles;
		if ((ticks % count_freq.low) > (count_freq.low/2))
			ticks++;			/* Round up if necessary 					*/
		freq = cycles/ticks;	/* Cycles / us  = MHz 						*/
		if ((cycles % ticks) > (ticks/2))
			freq++;				/* Round up if necessary					*/
		total = (freq + freq2 + freq3);
								/* Total last three frequency calculations	*/
		} while ( (tries < 3 ) ||
				  (tries < 20) &&
				  ((abs(3 * freq -total) > (3*TOLERANCE)) ||
				   (abs(3 * freq2-total) > (3*TOLERANCE)) ||
				   (abs(3 * freq3-total) > (3*TOLERANCE))));

#ifdef	__WINDOWS32__
	/* Restore thread priority */
	if (iPriority != THREAD_PRIORITY_ERROR_RETURN)
		SetThreadPriority(hThread, iPriority);
#endif
	return (total_cycles / total_ticks);
}
#endif	/* __INTEL__ */

/****************************************************************************
DESCRIPTION:
Returns the speed of the processor in Mhz.

HEADER:
ztimer.h

RETURNS:
Processor speed in Mhz.

REMARKS:
This function returns the speed of the CPU in Mhz. Note that if the speed
cannot be determined, this function will return 0.

SEE ALSO:
CPU_getProcessorType, CPU_haveMMX
****************************************************************************/
ulong CPU_getProcessorSpeed(void)
{
#if		defined(__INTEL__)
	/* Number of cycles needed to execute a single BSF instruction on i386+
	 * processors.
	 */
	ulong	cpuSpeed;
	uint	i,processor = CPU_getProcessorType() & CPU_mask;
	static 	ulong processor_cycles[] = {
		00,  115, 47, 43, 38, 38, 38
		};
	static 	ulong known_speeds[] = {
		450,400,350,333,300,266,233,200,166,150,133,120,100,90,75,66,60,50,33,20,0
		};
	if (_CPU_haveRDTSC()) {
		DB(printf("\nMeasuring speed using RDTSC\n"));
		cpuSpeed = GetRDTSCCpuSpeed();
		}
	else {
		DB(printf("\nMeasuring speed using BSF\n"));
		cpuSpeed = GetBSFCpuSpeed(ITERATIONS * processor_cycles[processor]);
		}
	DB(printf("Measured CPU speed is %d Mhz", cpuSpeed));

	/* Now normalise the results given known processors speeds, if the
	 * speed we measure is within 2Mhz of the expected values
	 */
	for (i = 0; known_speeds[i] != 0; i++) {
		if (cpuSpeed >= (known_speeds[i]-2) && cpuSpeed <= (known_speeds[i]+2)) {
			return known_speeds[i];
			}
		}
	return cpuSpeed;
#else
	return 0;
#endif
}
