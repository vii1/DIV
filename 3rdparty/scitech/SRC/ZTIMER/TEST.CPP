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
*				   Simple Test program by Kendall Bennett
*                   Copyright (C) 1993-4 SciTech Software
*
* Filename:		$Workfile:   test.cpp  $
* Version:		$Revision:   1.0  $
*
* Language:		C++ 2.1
* Environment:	MS DOS (IBM PC)
*
* Description:	Test program for the Zen Timer Library C++ interface.
*
* $Date:   05 Feb 1996 14:50:20  $ $Author:   KendallB  $
*
****************************************************************************/

#include <iostream.h>
#include <dos.h>
#include "pmode.h"
#include "ztimer.h"

/*-------------------------- Implementation -------------------------------*/

int		i,j,k;								/* NON register variables! */

void dummy() {}

int main(void)
{
	LZTimer		ltimer;
	ULZTimer	ultimer;

	ZTimerInit();

	/* Test the long period Zen Timer (we don't check for overflow coz
	 * it would take tooooo long!)
	 */

	cout << endl;
	ultimer.restart();
	ltimer.start();
	for (j = 0; j < 10; j++)
		for (i = 0; i < 20000; i++)
			dummy();
	ltimer.stop();
	ultimer.stop();
	cout << "LCount:  " << ltimer.count() << endl;
	cout << "Time:    " << ltimer << " secs\n";
	cout << "ULCount: " << ultimer.count() << endl;
	cout << "ULTime:  " << ultimer << " secs\n";

	cout << endl << "Timing ... \n";
	ultimer.restart();
	ltimer.restart();
    for (j = 0; j < 200; j++)
		for (i = 0; i < 20000; i++)
			dummy();
	ltimer.stop();
	ultimer.stop();
	cout << "LCount:  " << ltimer.count() << endl;
	cout << "Time:    " << ltimer << " secs\n";
	cout << "ULCount: " << ultimer.count() << endl;
	cout << "ULTime:  " << ultimer << " secs\n";

	/* Test the lap function of the long period Zen Timer */

	cout << endl << "Timing ... \n";
	ultimer.restart();
	ltimer.restart();
	for (j = 0; j < 20; j++) {
		for (k = 0; k < 10; k++)
			for (i = 0; i < 20000; i++)
				dummy();
		cout << "lap: " << ltimer.lap() << endl;
		}
	ltimer.stop();
	ultimer.stop();
	cout << "LCount:  " << ltimer.count() << endl;
	cout << "Time:    " << ltimer << " secs\n";
	cout << "ULCount: " << ultimer.count() << endl;
	cout << "ULTime:  " << ultimer << " secs\n";

#ifdef	LONG_TEST
	/* Test the ultra long period Zen Timer */

	ultimer.start();
	delay(DELAY_SECS * 1000);
	ultimer.stop();
	cout << "Delay of " << DELAY_SECS << " secs took " << ultimer.count()
		 << " 1/10ths of a second\n";
	cout << "Time: " << ultimer << " secs\n";
#endif
	return 0;
}
