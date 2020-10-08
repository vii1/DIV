/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   mouse.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to install an assembly
*				language mouse interrupt handler. We use assembly language
*				as it must be a far function and should swap to a local
*				32 bit stack if it is going to call any C based code (which
*				we do in this example).
*
*				Functions tested:	PM_installMouseHandler()
*									PM_int86()
*
* $Date:   05 Feb 1996 21:41:30  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmpro.h"

volatile long count = 0;

#pragma off (check_stack)			/* No stack checking under Watcom	*/

void PMAPI mouseHandler(unsigned mask,unsigned butstate,unsigned x,
	unsigned y,unsigned mickeyX,unsigned mickeyY)
{
	mask = mask;				/* We dont use any of the parameters	*/
	butstate = butstate;
	x = x;
	y = y;
	mickeyX = mickeyX;
	mickeyY = mickeyY;
	count++;
}

int main(void)
{
	RMREGS      regs;

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

	regs.x.ax = 33;		/* Mouse function 33 - Software reset		*/
	PM_int86(0x33,&regs,&regs);
	if (regs.x.bx == 0) {
		printf("No mouse installed.\n");
		exit(1);
		}

	/* Install our mouse handler and lock handler pages in memory. It is
	 * difficult to get the size of a function in C, but we know our
	 * function is well less than 100 bytes (and an entire 4k page will
	 * need to be locked by the server anyway).
	 */
	PM_lockCodePages((__codePtr)mouseHandler,100);
	PM_lockDataPages((void*)&count,sizeof(count));
	if (!PM_setMouseHandler(0xFFFF, mouseHandler)) {
		printf("Unable to install mouse handler!\n");
		exit(1);
		}
	printf("Mouse handler installed - Hit any key to exit\n");
	getch();

	PM_restoreMouseHandler();
	PM_unlockDataPages((void*)&count,sizeof(count));
    PM_unlockCodePages((__codePtr)mouseHandler,100);
    printf("Mouse handler was called %ld times\n", count);
	return 0;
}
