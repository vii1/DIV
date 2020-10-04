/****************************************************************************
*
*				 VESA BIOS Extensions/Accelerator Functions
*
*                   Copyright (C) 1996 SciTech Software.
*                           All rights reserved.
*
* Filename:     $Workfile:   aftests.c  $
* Version:      $Revision:   1.2  $
*
* Language:     ANSI C
* Environment:  IBM PC 32 bit Protected Mode.
*
* Description:  Simple test code to test out each of the accelerator
*				functions that are available.
*
*               MUST be compiled in the FLAT model.
*
* $Date:   19 Apr 1996 19:46:02  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "pmode.h"
#include "svga.h"

#ifdef	__WINDOWS__
#undef	TRUE
#undef	FALSE
#include <windows.h>
#endif

#ifndef	__16BIT__

/*---------------------------- Global Variables ---------------------------*/

extern	SV_devCtx	*DC;

/* Pre-defined colors */

PRIVATE AF_int32	white;
PRIVATE AF_int32	black;

/* Simple test pattern */

PRIVATE AF_pattern	testPat[] =
	{0xF8, 0x74, 0x22, 0x47, 0x8F, 0x17, 0x22, 0x71};

/* Standard arrow cursor */

PRIVATE AF_cursor arrowCursor = {
	0x00000000, 0x00000040, 0x00000060, 0x00000070,
	0x00000078, 0x0000007C, 0x0000007E, 0x0000007F,
	0x0000807F, 0x0000C07F, 0x0000007E, 0x00000076,
	0x00000066, 0x00000043, 0x00000003, 0x00008001,
	0x00008001, 0x0000C000, 0x0000C000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x000000C0, 0x000000E0, 0x000000F0, 0x000000F8,
	0x000000FC, 0x000000FE, 0x000000FF, 0x000080FF,
	0x0000C0FF, 0x0000E0FF, 0x0000F0FF, 0x000000FF,
	0x000000FF, 0x000080EF, 0x000080C7, 0x0000C083,
	0x0000C003, 0x0000E001, 0x0000E001, 0x0000C000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	8,8
	};

/*----------------------------- Implementation ----------------------------*/

/* Keyboard handlers provided by main application	*/
int KeyHit(void);
int GetChar(void);

PRIVATE int realColor(int index)
/****************************************************************************
*
* Function:     realColor
* Parameters:	index	- Color index to get the proper packed color for
*
* Description:  Returns the real color value for the specified color. In
*				RGB modes we look up the color value from the palette and
*				pack into the proper format.
*
****************************************************************************/
{
	SV_palette *pal = SV_getDefPalette();

	if (DC->bitsperpixel <= 8)
		return index;
	return SV_rgbColor(pal[index].red,
					   pal[index].green,
					   pal[index].blue);
}

PRIVATE int accelLineTest(void)
/****************************************************************************
*
* Function:     accelLineTest
*
* Description:  Draws a simple Moire pattern on the display screen using
*               accelerated lines, and waits for a key press.
*
****************************************************************************/
{
	int     i;

	if (!DC->AFDC->DrawLine)
		return 0;
	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);

	for (i = 0; i < DC->maxx; i += 4) {
		AF_drawLine(DC->AFDC,realColor(i % 255),
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),AF_TOFIX(i),0);
		AF_drawLine(DC->AFDC,realColor((i+1) % 255),
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),AF_TOFIX(i),AF_TOFIX(DC->maxy));
		}
	for (i = 0; i < DC->maxy; i += 4) {
		AF_drawLine(DC->AFDC,realColor((i+2) % 255),
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),0,AF_TOFIX(i));
		AF_drawLine(DC->AFDC,realColor((i+3) % 255),
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),AF_TOFIX(DC->maxx),AF_TOFIX(i));
		}
	AF_drawLine(DC->AFDC,white,0,0,AF_TOFIX(DC->maxx),0);
	AF_drawLine(DC->AFDC,white,0,0,0,AF_TOFIX(DC->maxy));
	AF_drawLine(DC->AFDC,white,AF_TOFIX(DC->maxx),0,AF_TOFIX(DC->maxx),AF_TOFIX(DC->maxy));
	AF_drawLine(DC->AFDC,white,0,AF_TOFIX(DC->maxy),AF_TOFIX(DC->maxx),AF_TOFIX(DC->maxy));
	return GetChar();
}

PRIVATE int accelStippleLineTest(void)
/****************************************************************************
*
* Function:     accelStippleLineTest
*
* Description:  Draws a simple Moire pattern on the display screen using
*               accelerated patterned lines, and waits for a key press.
*
****************************************************************************/
{
	int     i;

	if (!DC->AFDC->DrawStippleLine)
		return 0;
	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);
	AF_setLineStipple(DC->AFDC,0xCCCC);

	for (i = 0; i < DC->maxx; i += 20) {
		AF_drawStippleLine(DC->AFDC,realColor(i % 255),black,
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),AF_TOFIX(i),0);
		AF_drawStippleLine(DC->AFDC,(i+1) % 255,black,
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),AF_TOFIX(i),AF_TOFIX(DC->maxy));
		}
	for (i = 0; i < DC->maxy; i += 20) {
		AF_drawStippleLine(DC->AFDC,realColor((i+2) % 255),black,
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),0,AF_TOFIX(i));
		AF_drawStippleLine(DC->AFDC,realColor((i+3) % 255),black,
			AF_TOFIX(DC->maxx/2),AF_TOFIX(DC->maxy/2),AF_TOFIX(DC->maxx),AF_TOFIX(i));
		}
	AF_setLineStipple(DC->AFDC,0x0C3F);
	AF_drawStippleLine(DC->AFDC,white,black,0,0,AF_TOFIX(DC->maxx),0);
	AF_drawStippleLine(DC->AFDC,white,black,0,0,0,AF_TOFIX(DC->maxy));
	AF_drawStippleLine(DC->AFDC,white,black,AF_TOFIX(DC->maxx),0,AF_TOFIX(DC->maxx),AF_TOFIX(DC->maxy));
	AF_drawStippleLine(DC->AFDC,white,black,0,AF_TOFIX(DC->maxy),AF_TOFIX(DC->maxx),AF_TOFIX(DC->maxy));
	return GetChar();
}

PRIVATE int rectTest(int waitKey)
/****************************************************************************
*
* Function:     rectTest
*
* Description:  Draws a bunch of colored accelerated rectangles on the
*				display.
*
****************************************************************************/
{
	int	x,y,i,j,width,height;
	int	color;

	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);

	width = 2 * ((DC->maxx+1) / 46);
	height = 2 * ((DC->maxy-10)  / 47);
	x = width / 2;
	y = height / 2;
	color = 1;
	for (j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			AF_drawRect(DC->AFDC,realColor(color++),x,y,width,height);
			x += (width/2) * 3;
			}
		y += (height / 2) * 3;
		x = width / 2;
		}

	if (waitKey)
		return GetChar();
	return 0;
}

PRIVATE int pattRectTest(void)
/****************************************************************************
*
* Function:     pattRectTest
*
* Description:  Draws a bunch of patterned accelerated rectangles on the
*				display.
*
****************************************************************************/
{
	int	x,y,i,j,width,height;
	int	color;

	if (!DC->AFDC->DrawPattRect)
		return 0;

	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,realColor(1),0,0,DC->maxx+1,DC->maxy+1);
	AF_set8x8MonoPattern(DC->AFDC,testPat);

	width = 2 * ((DC->maxx+1) / 46);
	height = 2 * ((DC->maxy-10)  / 47);
	x = width / 2;
	y = height / 2;
	color = 1;
	for (j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			if (i % 2)
				AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
			else AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_NOP_MIX);
			AF_drawPattRect(DC->AFDC,realColor(color++),realColor(2),x,y,width,height);
			x += (width/2) * 3;
			}
		y += (height / 2) * 3;
		x = width / 2;
		}
	return GetChar();
}

PRIVATE int scanTest(void)
/****************************************************************************
*
* Function:     scanTest
*
* Description:  Draws a bunch of colored accelerated scanlines on the
*				display.
*
****************************************************************************/
{
	int	x,y,i,j,k,width,height;
	int	color;

	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);

	width = 2 * ((DC->maxx+1) / 46);
	height = 2 * ((DC->maxy-10)  / 47);
	x = width / 2;
	y = height / 2;
	color = 1;
	for (j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			for (k = 0; k < height; k += 2)
				AF_drawScan(DC->AFDC,realColor(color),y+k,x,x+width);
			color++;
			x += (width/2) * 3;
			}
		y += (height / 2) * 3;
		x = width / 2;
		}
	return GetChar();
}

PRIVATE int pattScanTest(void)
/****************************************************************************
*
* Function:     pattScanTest
*
* Description:  Draws a bunch of colored accelerated scanlines on the
*				display.
*
****************************************************************************/
{
	int	x,y,i,j,k,width,height;
	int	color;

	if (!DC->AFDC->DrawPattScan)
		return 0;

	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,realColor(1),0,0,DC->maxx+1,DC->maxy+1);
	AF_set8x8MonoPattern(DC->AFDC,testPat);

	width = 2 * ((DC->maxx+1) / 46);
	height = 2 * ((DC->maxy-10)  / 47);
	x = width / 2;
	y = height / 2;
	color = 1;
	for (j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			if (i % 2)
				AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
			else AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_NOP_MIX);
            for (k = 0; k < height; k += 2)
				AF_drawPattScan(DC->AFDC,realColor(color),realColor(2),y+k,x,x+width);
			color++;
			x += (width/2) * 3;
			}
		y += (height / 2) * 3;
		x = width / 2;
		}
	return GetChar();
}

PRIVATE int scanListTest(void)
/****************************************************************************
*
* Function:     scanListTest
*
* Description:  Draws an accelerated scanline list
*
****************************************************************************/
{
	AF_int16 scans[20] = {
		100,200,	95,215,		90,220,		80,230,
		85,230,		85,230,		90,220,		95,200,
		100,150,	120,130,
		};

	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);
	AF_drawScanList(DC->AFDC,realColor(1),10,10,scans);
	return GetChar();
}

PRIVATE int trapTest(void)
/****************************************************************************
*
* Function:     trapTest
*
* Description:  Draws an accelerated trapezoid
*
****************************************************************************/
{
	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawTrap(DC->AFDC,realColor(2),30,100,AF_TOFIX(100),AF_TOFIX(150),
		-0x8000,0x4000);
	return GetChar();
}

PRIVATE uchar image[] = {
	0xFF, 0xDE, 0x7B, 0xFE, 0,	/* 11111111 11011110 01111011 11111110 00000000 */
	0xFF, 0xCC, 0x31, 0xFE, 0,	/* 11111111 11001100 00110001 11111110 00000000 */
	0x8C, 0x4C, 0x31, 0x82, 0,	/* 10001100 01001100 00110001 10000010 00000000 */
	0x0C, 0x0C, 0x31, 0x80, 0,	/* 00001100 00001100 00110001 10000000 00000000 */
	0x0C, 0x0C, 0x31, 0x90, 0,	/* 00001100 00001100 00110001 10010000 00000000 */
	0x0C, 0x0F, 0xF1, 0xF0, 0,	/* 00001100 00001111 11110001 11110000 00000000 */
	0x0C, 0x0F, 0xF1, 0xF0, 0,	/* 00001100 00001111 11110001 11110000 00000000 */
	0x0C, 0x0C, 0x31, 0x90, 0,	/* 00001100 00001100 00110001 10010000 00000000 */
	0x0C, 0x0C, 0x31, 0x80, 0,	/* 00001100 00001100 00110001 10000000 00000000 */
	0x0C, 0x0C, 0x31, 0x82, 0,	/* 00001100 00001100 00110001 10000010 00000000 */
	0x0C, 0x0C, 0x31, 0xFE, 0,	/* 00001100 00001100 00110001 11111110 00000000 */
	0x1E, 0x1E, 0x7B, 0xFE, 0,	/* 00011110 00011110 01111011 11111110 00000000 */
	};

PRIVATE int putMonoImageTest(void)
/****************************************************************************
*
* Function:     putMonoImageTest
*
* Description:  Display a bunch of monochrome images
*
****************************************************************************/
{
	int	x,y,byteWidth,height;

	if (!DC->AFDC->PutMonoImage)
		return 0;

	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);
	byteWidth = 5;
	height = sizeof(image)/byteWidth;
	for (x = 0; x < DC->maxx; x += 40)
		for (y = 0; y < DC->maxy; y += sizeof(image)/4 + 2)
			AF_putMonoImage(DC->AFDC,realColor((x + y) % 16),black,x,y,byteWidth,height,image);
	return GetChar();
}

PRIVATE int bitBltTest(void)
/****************************************************************************
*
* Function:     bitBltTest
*
* Description:  Scroll the display using a hardware bitBlt
*
****************************************************************************/
{
	int	i,left,top,width,height;

	if (!DC->AFDC->BitBlt)
		return 0;

	/* Scroll upwards */
	left = 0;				top = 5;
	width = DC->maxx+1-left;	height = DC->maxy+1-top;
	rectTest(1);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,0,AF_REPLACE_MIX);
	getch();

	/* Scroll downwards */
	left = 0;				top = 0;
	width = DC->maxx+1-left;	height = DC->maxy-4-top;
	rectTest(1);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,5,AF_REPLACE_MIX);
	getch();

	/* Scroll right */
	left = 0;				top = 0;
	width = DC->maxx-3-left;	height = DC->maxy+1-top;
	rectTest(1);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,4,0,AF_REPLACE_MIX);
	getch();

	/* Scroll left */
	left = 4;				top = 0;
	width = DC->maxx+1-left;	height = DC->maxy+1-top;
	rectTest(1);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,0,AF_REPLACE_MIX);
	getch();

	/* Scroll right and down */
	left = 0;				top = 0;
	width = DC->maxx-4-left;	height = DC->maxy-4-top;
	rectTest(1);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,5,5,AF_REPLACE_MIX);
	return GetChar();
}

PRIVATE int bitBltLinTest(void)
/****************************************************************************
*
* Function:     bitBltLinTest
*
* Description:  Perform a linear source bitBlt on the display
*
****************************************************************************/
{
	int	i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->BitBltLin)
		return 0;

	/* Draw the background and the source to be blited */
	rectTest(0);
	AF_drawRect(DC->AFDC,realColor(15),0,0,xres/8,yres/6);
	AF_drawRect(DC->AFDC,realColor(12),xres / 64,yres / 48,
		(xres / 8) - (2 * (xres / 64)),
		(yres / 6) - (2 * (yres / 48)));
	AF_drawRect(DC->AFDC,realColor(9),xres / 32,yres / 24,
		(xres / 8) - (2 * (xres / 32)),
		(yres / 6) - (2 * (yres / 24)));
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Do several blits at different locations */
	for (i = 0; i < 6; i++) {
		AF_bitBltLin(DC->AFDC,xres*100,
			(xres/8) + (i * (xres/8)),i * (xres/8),
			xres/8,yres/6,AF_REPLACE_MIX);
		}
	return GetChar();
}

PRIVATE int srcTransBltTest(void)
/****************************************************************************
*
* Function:     srcTransBltTest
*
* Description:  Perform a transparent bitBlt on the display
*
****************************************************************************/
{
	int	i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->SrcTransBlt)
		return 0;

	/* Draw the background and the source to be blited */
	rectTest(0);
	AF_drawRect(DC->AFDC,realColor(15),0,0,xres/8,yres/6);
	AF_drawRect(DC->AFDC,realColor(12),xres / 64,yres / 48,
		(xres / 8) - (2 * (xres / 64)),
		(yres / 6) - (2 * (yres / 48)));
	AF_drawRect(DC->AFDC,realColor(9),xres / 32,yres / 24,
		(xres / 8) - (2 * (xres / 32)),
		(yres / 6) - (2 * (yres / 24)));
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Do several transparent blits at different locations */
	for (i = 0; i < 6; i++) {
		AF_srcTransBlt(DC->AFDC,0,0,xres/8,yres/6,
			(xres/8) + (i * (xres/8)),i * (xres/8),AF_REPLACE_MIX,realColor(9));
		}
	return GetChar();
}

PRIVATE int srcTransBltLinTest(void)
/****************************************************************************
*
* Function:     srcTransBltLinTest
*
* Description:  Perform a linear source transparent bitBlt on the display
*
****************************************************************************/
{
	int	i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->SrcTransBltLin)
		return 0;

	/* Draw the background and the source to be blited */
	rectTest(0);
	AF_drawRect(DC->AFDC,realColor(15),0,0,xres/8,yres/6);
	AF_drawRect(DC->AFDC,realColor(12),xres / 64,yres / 48,
		(xres / 8) - (2 * (xres / 64)),
		(yres / 6) - (2 * (yres / 48)));
	AF_drawRect(DC->AFDC,realColor(9),xres / 32,yres / 24,
		(xres / 8) - (2 * (xres / 32)),
		(yres / 6) - (2 * (yres / 24)));
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Do several transparent blits at different locations */
	for (i = 0; i < 6; i++) {
		AF_srcTransBltLin(DC->AFDC,xres*100,
			(xres/8) + (i * (xres/8)),i * (xres/8),
			xres/8,yres/6,
			AF_REPLACE_MIX,realColor(9));
		}
	return GetChar();
}

PRIVATE int dstTransBltTest(void)
/****************************************************************************
*
* Function:     dstTransBltTest
*
* Description:  Perform a transparent bitBlt on the display
*
****************************************************************************/
{
	int	i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->DstTransBlt)
		return 0;

	/* Draw the background and the source to be blited */
	rectTest(0);
	AF_drawRect(DC->AFDC,realColor(15),0,0,xres/8,yres/6);
	AF_drawRect(DC->AFDC,realColor(12),xres / 64,yres / 48,
		(xres / 8) - (2 * (xres / 64)),
		(yres / 6) - (2 * (yres / 48)));
	AF_drawRect(DC->AFDC,realColor(9),xres / 32,yres / 24,
		(xres / 8) - (2 * (xres / 32)),
		(yres / 6) - (2 * (yres / 24)));
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Do several transparent blits at different locations */
	for (i = 0; i < 6; i++) {
		AF_dstTransBlt(DC->AFDC,0,0,xres/8,yres/6,
			(xres/8) + (i * (xres/8)),i * (xres/8),AF_REPLACE_MIX,0);
		}
	return GetChar();
}

PRIVATE int dstTransBltLinTest(void)
/****************************************************************************
*
* Function:     dstTransBltLinTest
*
* Description:  Perform a linear destination transparent bitBlt on the
*				display
*
****************************************************************************/
{
	int	i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->DstTransBltLin)
		return 0;

	/* Draw the background and the source to be blited */
	rectTest(0);
	AF_drawRect(DC->AFDC,realColor(15),0,0,xres/8,yres/6);
	AF_drawRect(DC->AFDC,realColor(12),xres / 64,yres / 48,
		(xres / 8) - (2 * (xres / 64)),
		(yres / 6) - (2 * (yres / 48)));
	AF_drawRect(DC->AFDC,realColor(9),xres / 32,yres / 24,
		(xres / 8) - (2 * (xres / 32)),
		(yres / 6) - (2 * (yres / 24)));
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Do several transparent blits at different locations */
	for (i = 0; i < 6; i++) {
		AF_dstTransBltLin(DC->AFDC,xres*100,
			(xres/8) + (i * (xres/8)),i * (xres/8),
			xres/8,yres/6,
			AF_REPLACE_MIX,0);
		}
	return GetChar();
}

PRIVATE void getMousePos(int *x,int *y)
/****************************************************************************
*
* Function:     getMousePos
*
* Description:  Call the mouse driver to poll the mouse cursor location.
*				We dont bother with tricking the mouse driver so it knows
*				we are in a graphics mode, so the cursor will most likely
*				jump in 8 pixel steps, but this is suitable for our
*				purposes.
*
****************************************************************************/
{
#ifdef	__MSDOS__
	RMREGS	regs;
	regs.x.ax = 3;
	PM_int86(0x33,&regs,&regs);
	*x = regs.x.cx;
	*y = regs.x.dx;
#elif	defined(__WINDOWS__)
	POINT p;
	GetCursorPos(&p);
	*x = p.x;
	*y = p.y;
#endif
}

PRIVATE int cursorTest(bool haveAccel)
/****************************************************************************
*
* Function:     cursorTest
*
* Description:  Test the hardware mouse cursor routines.
*
****************************************************************************/
{
	int	x,y,oldx = -1,oldy = -1;

	if (!DC->AFDC->SetCursor)
		return 0;

	if (haveAccel)
		rectTest(1);
	AF_setCursor(DC->AFDC,&arrowCursor);
	AF_setCursorColor(DC->AFDC,0xFF,0xFF,0xFF);
	AF_showCursor(DC->AFDC,true);
	while (!KeyHit()) {
		getMousePos(&x,&y);
		if (x != oldx || y != oldy)
			AF_setCursorPos(DC->AFDC,oldx = x,oldy = y);
		}
	GetChar();
	AF_showCursor(DC->AFDC,false);
	return GetChar();
}

PRIVATE int doAccelTests(void)
{
	black = realColor(0);
	white = realColor(15);
	rectTest(1);
	AF_setMix(DC->AFDC,AF_XOR_MIX,AF_XOR_MIX);
	AF_drawRect(DC->AFDC,realColor(0xFF),0,0,DC->maxx+1,DC->maxy+1);
	if (GetChar() == 0x1B)
		return 0x1B;
	if (pattRectTest() == 0x1B)
		return 0x1B;
	if (scanTest() == 0x1B)
		return 0x1B;
	if (pattScanTest() == 0x1B)
		return 0x1B;
	if (scanListTest() == 0x1B)
		return 0x1B;
	if (trapTest() == 0x1B)
		return 0x1B;
	if (accelLineTest() == 0x1B)
		return 0x1B;
	if (accelStippleLineTest() == 0x1B)
		return 0x1B;
	if (putMonoImageTest() == 0x1B)
		return 0x1B;
	if (bitBltTest() == 0x1B)
		return 0x1B;
	if (bitBltLinTest() == 0x1B)
		return 0x1B;
	if (srcTransBltTest() == 0x1B)
		return 0x1B;
	if (srcTransBltLinTest() == 0x1B)
		return 0x1B;
	if (dstTransBltTest() == 0x1B)
		return 0x1B;
	if (dstTransBltLinTest() == 0x1B)
		return 0x1B;
	return 0;
}

void doAFTests(SV_devCtx *DC)
{
	/* Test accelerated drawing */
	if (DC->AFDC->SetClipRect)
		AF_setClipRect(DC->AFDC,0,0,DC->maxx,DC->maxy);
	if (doAccelTests() == 0x1B)
		return;
	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);
	if (DC->AFDC->SetClipRect) {
		AF_setClipRect(DC->AFDC,50,50,DC->maxx-50,DC->maxy-50);
		if (doAccelTests() == 0x1B)
			return;
		}

	/* Test hardware cursor */
	cursorTest(DC->AFDC->DrawRect != NULL);
}

#endif
