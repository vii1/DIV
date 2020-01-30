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
*
* Language:		ANSI C
* Environment:	IBM PC (Win32 Console)
*
* Description:	Win32 console test program for the Zen Timer library.
*
*
****************************************************************************/

#include "ztimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

/*----------------------------- Implementation ----------------------------*/

void main(void)
{
	int		cpu;
	ulong	speed,count;

	printf("Detecting processor information ...");
	cpu = CPU_getProcessorType();
	speed = CPU_getProcessorSpeed();
	printf("\n\n%d Mhz ", speed);
	switch (cpu & CPU_mask) {
		case CPU_i386:
			printf("Intel i386 processor");
			break;
		case CPU_i486:
			printf("Intel i486 processor");
			break;
		case CPU_Pentium:
			printf("Intel Pentium processor");
			break;
		case CPU_PentiumPro:
			printf("Intel PentiumPro processor");
			break;
		case CPU_PentiumII:
			printf("Intel PentiumII processor");
			break;
		default:
			printf("Unknown Intel processor");
		}
	if (cpu & CPU_IntelClone)
    	printf(" clone");
	if (CPU_haveMMX())
		printf(" with MMX(R) technology");
	printf("\n");
	getch();

#if 0
	printf("Press a key to start timing\n");
	getch();
	printf("Press a key to stop timing\n");
	ZTimerInit();
	LZTimerOn();
	getch();
	LZTimerOff();
	count = LZTimerCount();
	printf("Elapsed time = %dus (%.2f seconds)\n",
		count,count / 1000000.0);
#endif
}

