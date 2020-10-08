/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   altbrk.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to install a C based
*				control C/break interrupt handler. Note that this
*				alternate version does not work with all extenders.
*
*				Functions tested:	PM_installAltBreakHandler()
*									PM_restoreBreakHandler()
*
* $Date:   05 Feb 1996 21:41:26  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmpro.h"

volatile int breakHit = false;
volatile int ctrlCHit = false;

#pragma off (check_stack)			/* No stack checking under Watcom	*/

void PMAPI breakHandler(uint bHit)
{
	if (bHit)
		breakHit = true;
	else
		ctrlCHit = true;
}

int main(void)
{
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

	PM_installAltBreakHandler(breakHandler);
	printf("Control C/Break interrupt handler installed\n");
	while (1) {
		if (ctrlCHit) {
			printf("Code termimated with Ctrl-C.\n");
			break;
			}
		if (breakHit) {
			printf("Code termimated with Ctrl-Break.\n");
			break;
			}
		if (kbhit() && getch() == 0x1B) {
			printf("No break code detected!\n");
			break;
			}
		printf("Hit Ctrl-C or Ctrl-Break to exit!\n");
		}

	PM_restoreBreakHandler();
	return 0;
}
