/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   tick.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to install a C based
*				timer interrupt handler.
*
*				Functions tested:	PM_setTimerHandler()
*									PM_chainPrevTimer();
*									PM_restoreTimerHandler()
*
* $Date:   05 Feb 1996 21:41:32  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmpro.h"

volatile long count = 0;

#pragma off (check_stack)			/* No stack checking under Watcom	*/

void PMAPI timerHandler(void)
{
	PM_chainPrevTimer();        /* Chain to previous handler */
	count++;
}

int main(void)
{
	long		oldCount;

	printf("Program running in ");
	switch (PM_getModeType()) {
		case PM_realMode:
			printf("real mode.\n\n");
			break;
		case PM_286:
			printf("16 bit protected mode.\n\n");
			break;
		case PM_386:
			printf("32 bit protected mode.\n\n");
			break;
		}

	/* Install our timer handler and lock handler pages in memory. It is
	 * difficult to get the size of a function in C, but we know our
	 * function is well less than 100 bytes (and an entire 4k page will
	 * need to be locked by the server anyway).
	 */
    PM_lockCodePages((__codePtr)timerHandler,100);
	PM_lockDataPages((void*)&count,sizeof(count));
	PM_installBreakHandler();			/* We *DONT* want Ctrl-Breaks! */
	PM_setTimerHandler(timerHandler);
	printf("Timer interrupt handler installed - Hit ESC to exit\n");
	oldCount = count;
	while (1) {
		if (kbhit() && (getch() == 0x1B))
			break;
		if (count != oldCount) {
			printf("Tick, Tock: %d\n", count);
			oldCount = count;
			}
		}

	PM_restoreTimerHandler();
	PM_restoreBreakHandler();
	PM_unlockDataPages((void*)&count,sizeof(count));
    PM_unlockCodePages((__codePtr)timerHandler,100);
	printf("Timer handler was called %ld times\n", count);
	return 0;
}
