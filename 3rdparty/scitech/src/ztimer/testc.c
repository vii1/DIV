/****************************************************************************
*
*								 Zen Timer
*
*							   From the book
*						 "Zen of Assembly Language"
*							Volume 1, Knowledge
*
*							 by Michael Abrash
*
*					Simple Test program by Kendall Bennett
*                    Copyright (C) 1996 SciTech Software
*
* Filename:		$Workfile:   testc.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	MS DOS (IBM PC)
*
* Description:	Test program for the Zen Timer Library.
*
* $Date:   05 Feb 1996 14:50:20  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <dos.h>
#include "pmode.h"
#include "debug.h"
#include "ztimer.h"

#define	DELAY_SECS	10

/*-------------------------- Implementation -------------------------------*/

/* The following routine takes a long count in microseconds and outputs
 * a string representing the count in seconds. It could be modified to
 * return a pointer to a static string representing the count rather
 * than printing it out.
 */

void ReportTime(ulong count)
{
	ulong	secs;

	secs = count / 1000000L;
	count = count - secs * 1000000L;
	printf("Time taken: %lu.%06lu seconds\n",secs,count);
}

int		i,j;								/* NON register variables! */

int main(void)
{
#ifdef	LONG_TEST
	ulong	start,finish;
#endif

    ZTimerInit();

	/* Test the long period Zen Timer (we don't check for overflow coz
	 * it would take tooooo long!)
	 */

	LZTimerOn();
	for (j = 0; j < 10; j++)
		for (i = 0; i < 20000; i++)
			i = i;
	LZTimerOff();
	ReportTime(LZTimerCount());

	/* Test the ultra long period Zen Timer */
#ifdef LONG_TEST
	start = ULZReadTime();
	delay(DELAY_SECS * 1000);
	finish = ULZReadTime();
	printf("Delay of %d secs took %d 1/10ths of a second\n",
		DELAY_SECS,ULZElapsedTime(start,finish));
#endif

	return 0;
}
