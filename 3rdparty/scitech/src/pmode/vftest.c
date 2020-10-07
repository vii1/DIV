/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile$
* Version:		$Revision$
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to test the VFlat virtual framebuffer functions.
*
*				Functions tested:	VF_available()
*									VF_init()
*									VF_exit()
*
* $Date$ $Author$
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmpro.h"

uchar code[] = {
	0xC3,					/* ret			*/
	};

int main(void)
{
	void	*vfBuffer;

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

	if (!VF_available()) {
		printf("Virtual Linear Framebuffer not available.\n");
		exit(1);
		}

	vfBuffer = VF_init(0xA0000,64,sizeof(code),code);
	if (!vfBuffer) {
		printf("Failure to initialise Virtual Linear Framebuffer!\n");
		exit(1);
		}
	VF_exit();
	printf("Virtual Linear Framebuffer set up successfully!\n");
	return 0;
}
