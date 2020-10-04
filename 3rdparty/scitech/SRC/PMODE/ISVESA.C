/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   isvesa.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to allocate real mode
*				memory and to call real mode interrupt handlers such as
*				the VESA VBE BIOS from protected mode. Compile and link
*				with the appropriate command line for your DOS extender.
*
*				Functions tested:	PM_allocRealSeg()
*									PM_freeRealSeg()
*									PM_mapRealPointer()
*									PM_int86x()
*
* $Date:   05 Feb 1996 21:41:28  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pmode.h"

/* SuperVGA information block */

#pragma pack(1)

typedef struct {
	char    VESASignature[4];       /* 'VESA' 4 byte signature          */
	short   VESAVersion;            /* VBE version number               */
	ulong	OEMStringPtr;           /* Far pointer to OEM string        */
	ulong	Capabilities;           /* Capabilities of video card       */
	ulong	VideoModePtr;           /* Far pointer to supported modes   */
	short   TotalMemory;            /* Number of 64kb memory blocks     */
	char    reserved[236];          /* Pad to 256 byte block size       */
	} VgaInfoBlock;

#pragma pack()

int main(void)
{
	RMREGS			regs;
	RMSREGS			sregs;
	VgaInfoBlock	vgaInfo;
	unsigned 		vgsel,vgoff,r_vgseg,r_vgoff;
	unsigned        sel,off;

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

	/* Allocate a 256 byte block of real memory for communicating with
	 * the VESA BIOS.
	 */
	if (!PM_allocRealSeg(256, &vgsel, &vgoff, &r_vgseg, &r_vgoff)) {
		printf("Unable to allocate real mode memory!\n");
		exit(1);
		}

	/* Call the VESA VBE to see if it is out there */
	regs.x.ax = 0x4F00;
	regs.x.di = r_vgoff;
	sregs.es = r_vgseg;
	PM_int86x(0x10, &regs, &regs, &sregs);
	PM_memcpynf(&vgaInfo,vgsel,vgoff,sizeof(VgaInfoBlock));
	if (regs.x.ax == 0x4F && strncmp(vgaInfo.VESASignature,"VESA",4) == 0) {
		printf("VESA VBE version %d.%d BIOS detected\n\n",
			vgaInfo.VESAVersion >> 8, vgaInfo.VESAVersion & 0xF);
		printf("Available video modes:\n");
		PM_mapRealPointer(&sel,&off,vgaInfo.VideoModePtr >> 16,
			vgaInfo.VideoModePtr & 0xFFFF);
		while (PM_getWord(sel,off) != 0xFFFF) {
			printf("    %04hXh\n", PM_getWord(sel,off));
			off += 2;
			}
		}
	else printf("VESA VBE not found\n");

	PM_freeRealSeg(vgsel,vgoff);
	return 0;
}
