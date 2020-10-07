/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   callreal.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to call a real mode
*				procedure. We simply copy a terribly simple assembly
*				language routine into a real mode block that we allocate,
*				and then attempt to call the routine and verify that it
*				was successful.
*
*				Functions tested:	PM_allocRealSeg()
*									PM_freeRealSeg()
*									PM_callRealMode()
*
* $Date:   05 Feb 1996 21:41:28  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pmode.h"

/* Block of real mode code we will eventually call */

static unsigned char realModeCode[] = {
	0x93,			/*	xchg	ax,bx	*/
	0x87, 0xCA,		/*	xchg	cx,dx	*/
	0xCB			/*	retf			*/
	};

int main(void)
{
	RMREGS			regs;
	RMSREGS			sregs;
	unsigned 		sel,off,r_seg,r_off;

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

	/* Allocate a the block of real mode memory */
	if (!PM_allocRealSeg(sizeof(realModeCode), &sel, &off, &r_seg, &r_off)) {
		printf("Unable to allocate real mode memory!\n");
		exit(1);
		}

	/* Copy the real mode code */
	PM_memcpyfn(sel,off,realModeCode,sizeof(realModeCode));

	/* Now call the real mode code */
	regs.x.ax = 1;
	regs.x.bx = 2;
	regs.x.cx = 3;
	regs.x.dx = 4;
	regs.x.si = 5;
	regs.x.di = 6;
	sregs.es = 7;
	sregs.ds = 8;
	PM_callRealMode(r_seg,r_off,&regs,&sregs);
	if (regs.x.ax != 2 || regs.x.bx != 1 || regs.x.cx != 4 || regs.x.dx != 3
			|| regs.x.si != 5 || regs.x.di != 6 || sregs.es != 7
			|| sregs.ds != 8) {
		printf("Real mode call failed!\n");
		printf("\n");
		printf("ax = %04d, bx = %04d, cx = %04d, dx = %04d\n",
			regs.x.ax,regs.x.bx,regs.x.cx,regs.x.dx);
		printf("si = %04d, di = %04d, es = %04d, ds = %04d\n",
			regs.x.si,regs.x.di,sregs.es,sregs.ds);
		}
	else printf("Real mode call succeeded!\n");

	/* Free the memory we allocated */
	PM_freeRealSeg(sel,off);
	return 0;
}
