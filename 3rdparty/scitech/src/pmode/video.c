/****************************************************************************
*
*						  Protected Mode Library
*
*                   Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   video.c  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	any
*
* Description:  Test program to check the ability to generate real mode
*               interrupts and to be able to obtain direct access to the
*				video memory from protected mode. Compile and link with
*				the appropriate command line for your DOS extender.
*
*				Functions tested:	PM_getBIOSSelector()
*									PM_mapPhysicalAddr()
*									PM_int86()
*
* $Date:   05 Feb 1996 21:41:32  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "pmode.h"

unsigned		bios;			/* Selector for BIOS data are		*/
unsigned char	*videoPtr;		/* Pointer to VGA framebuffer		*/

/* Routine to return the current video mode number */

int getVideoMode(void)
{
	return PM_getByte(bios, 0x49);
}

/* Routine to set a specified video mode */

void setVideoMode(int mode)
{
	RMREGS r;

	r.x.ax = mode;
	PM_int86(0x10, &r, &r);
}

/* Routine to clear a rectangular region on the display by calling the
 * video BIOS.
 */

void clear(int startx, int starty, int endx, int endy, unsigned char attr)
{
	RMREGS r;

	r.x.ax = 0x0600;
	r.h.bh = attr;
	r.h.cl = startx;
	r.h.ch = starty;
	r.h.dl = endx;
	r.h.dh = endy;
	PM_int86(0x10, &r, &r);
}

/* Routine to fill a rectangular region on the display using direct
 * video writes.
 */

#define SCREEN(x,y)	(videoPtr + ((y) * 160) + ((x) << 1))

void fill(int startx, int starty, int endx, int endy, unsigned char c,
	unsigned char attr)
{
	unsigned char	*v;
	int				x,y;

	for (y = starty; y <= endy; y++) {
		v = SCREEN(startx,y);
		for (x = startx; x <= endx; x++) {
			*v++ = c;
			*v++ = attr;
			}
		}
}

/* Routine to display a single character using direct video writes */

void writeChar(int x, int y, unsigned char c, unsigned char attr)
{
	unsigned char *v = SCREEN(x,y);
	*v++ = c;
	*v = attr;
}

/* Routine to draw a border around a rectangular area using direct video
 * writes.
 */

static unsigned char border_chars[] = {
	186, 205, 201, 187, 200, 188        /* double box chars */
	};

void border(int startx, int starty, int endx, int endy, unsigned char attr)
{
	unsigned char	*v;
	unsigned char 	*b;
	int				i;

	b = border_chars;

	for (i = starty+1; i < endy; i++) {
		writeChar(startx, i, *b, attr);
		writeChar(endx, i, *b, attr);
		}
	b++;
	for (i = startx+1, v = SCREEN(startx+1, starty); i < endx; i++) {
		*v++ = *b;
		*v++ = attr;
		}
	for (i = startx+1, v = SCREEN(startx+1, endy); i < endx; i++) {
		*v++ = *b;
		*v++ = attr;
		}
	b++;
	writeChar(startx, starty, *b++, attr);
	writeChar(endx, starty, *b++, attr);
	writeChar(startx, endy, *b++, attr);
	writeChar(endx, endy, *b++, attr);
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

	printf("Hit any key to start 80x25 text mode and perform some direct video output.\n");
	getch();

	setVideoMode(0x3);
	bios = PM_getBIOSSelector();
	if ((videoPtr = PM_mapPhysicalAddr(0xB8000,0xFFFF)) == NULL) {
		printf("Unable to obtain pointer to framebuffer!\n");
		exit(1);
		}

	fill(0, 0, 79, 24, 176, 0x1E);
	border(0, 0, 79, 24, 0x1F);
	getch();
	clear(0, 0, 79, 24, 0x7);
	return 0;
}
