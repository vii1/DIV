/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   biosptr.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to manipulate the
*				BIOS data area from protected mode using the PM
*				library. Compile and link with the appropriate command
*				line for your DOS extender.
*
*				Functions tested:	PM_getBIOSSelector()
*									PM_getLong()
*									PM_getByte()
*									PM_getWord()
*
* $Date:   05 Feb 1996 21:41:26  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmode.h"

/* Macros to obtain values from the BIOS data area */

#define TICKS()		PM_getLong(bios, 0x6C)
#define KB_STAT		PM_getByte(bios, 0x17)
#define KB_HEAD		PM_getWord(bios, 0x1A)
#define KB_TAIL		PM_getWord(bios, 0x1C)

/* Macros for working with the keyboard buffer */

#define KB_HIT()	(KB_HEAD != KB_TAIL)
#define CTRL()		(KB_STAT & 4)
#define SHIFT()		(KB_STAT & 2)
#define ESC			0x1B

/* Selector for BIOS data area */

unsigned bios;

void _cdecl _PM_saveDGROUP(void);

int main(void)
{
	int	c,done = 0;

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

	bios = PM_getBIOSSelector();
	printf("Hit any key to test, Ctrl-Shift-Esc to quit\n");
	while (!done) {
		if (KB_HIT()) {
			c = getch();
			if (c == 0) getch();
			printf("TIME=%-8lX ST=%02X CHAR=%02X ", TICKS(), KB_STAT, c);
			printf("\n");
			if ((c == ESC) && SHIFT() && CTRL())/* Ctrl-Shift-Esc */
                break;
			}
		}

	return 0;
}
