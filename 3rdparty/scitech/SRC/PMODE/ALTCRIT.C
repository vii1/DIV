/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   altcrit.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to install a C based
*				critical error handler.
*
*				Functions tested:	PM_installCriticalHandler()
*									PM_criticalError()
*									PM_restoreCriticalHandler()
*
* $Date:   05 Feb 1996 21:41:26  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmpro.h"

volatile uint criticalError = false;
volatile uint axValue;
volatile uint diValue;

#pragma off (check_stack)			/* No stack checking under Watcom	*/

uint PMAPI criticalHandler(uint axVal,uint diVal)
{
	criticalError = true;
	axValue = axVal;
	diValue = diVal;
	return 3;		/* Tell MS-DOS to fail the operation */
}

int main(void)
{
	FILE	*f;

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

	PM_installAltCriticalHandler(criticalHandler);
	printf("Critical Error handler installed - trying to read from A: drive...\n");
	f = fopen("a:\bog.bog","rb");
	if (f) fclose(f);
	if (criticalError) {
		printf("Critical error occured on INT 21h function %02X!\n",
			axValue >> 8);
		}
	else
		printf("Critical error was not caught!\n");
	PM_restoreCriticalHandler();
	return 0;
}
