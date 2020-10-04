/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:     $Workfile:   brk.c  $
* Version:      $Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to install a C based
*				control C/break interrupt handler.
*
*				Functions tested:	PM_installBreakHandler()
*									PM_ctrlCHit()
*									PM_ctrlBreakHit()
*									PM_restoreBreakHandler()
*
* $Date:   05 Feb 1996 21:41:28  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmpro.h"

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

	PM_installBreakHandler();
	printf("Control C/Break interrupt handler installed\n");
	while (1) {
		if (PM_ctrlCHit(1)) {
			printf("Code termimated with Ctrl-C.\n");
			break;
			}
		if (PM_ctrlBreakHit(1)) {
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
