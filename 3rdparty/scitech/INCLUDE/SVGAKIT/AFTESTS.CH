/****************************************************************************
*
*                VESA BIOS Extensions/Accelerator Functions
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
* Language:     ANSI C
* Environment:  IBM PC 32 bit Protected Mode.
*
* Description:  Simple test code to test out each of the accelerator
*               functions that are available.
*
*               MUST be compiled in the FLAT model.
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <stdarg.h>
#include "scitech.h"
#ifdef	__WINDOWS__
#define	STRICT
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "pmode.h"
#include "svga.h"

#ifndef __16BIT__

/*---------------------------- Global Variables ---------------------------*/

#if	(!defined(LITE) && !defined(PRO) && !defined(__WINDOWS__)) || defined(TEST_VBEAF)
#define	TEST_AF
#endif

#ifdef TEST_AF
extern  SV_devCtx   *DC;

/* Pre-defined colors */

static AF_int32    white;
static AF_int32    black;

/* Simple test pattern */

AF_pattern	testPat[] =
	{0xF8, 0x74, 0x22, 0x47, 0x8F, 0x17, 0x22, 0x71};

AF_pattern	testPat2[] =
	{0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

AF_color	testColorPat[8][8];

/* Simple test scanlist */

AF_int16 scans[20] = {
	100,200,    95,215,     90,220,     80,230,
	85,230,     85,230,     90,220,     95,200,
	100,150,    120,130,
	};

/* Standard arrow cursor */

static AF_cursor arrowCursor = {
  { 0x00000000, 0x00000040, 0x00000060, 0x00000070,
	0x00000078, 0x0000007C, 0x0000007E, 0x0000007F,
	0x0000807F, 0x0000C07F, 0x0000007E, 0x00000076,
	0x00000066, 0x00000043, 0x00000003, 0x00008001,
	0x00008001, 0x0000C000, 0x0000C000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, },
  { 0x000000C0, 0x000000E0, 0x000000F0, 0x000000F8,
	0x000000FC, 0x000000FE, 0x000000FF, 0x000080FF,
	0x0000C0FF, 0x0000E0FF, 0x0000F0FF, 0x000000FF,
	0x000000FF, 0x000080EF, 0x000080C7, 0x0000C083,
	0x0000C003, 0x0000E001, 0x0000E001, 0x0000C000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, },
    8,8
    };

/*----------------------------- Implementation ----------------------------*/

/* Keyboard handlers provided by main application   */
int KeyHit(void);
int GetChar(void);

static int realColor(int index)
/****************************************************************************
*
* Function:     realColor
* Parameters:   index   - Color index to get the proper packed color for
*
* Description:  Returns the real color value for the specified color. In
*               RGB modes we look up the color value from the palette and
*               pack into the proper format.
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

static int gprintf(int x,int y,char *fmt, ... )
/****************************************************************************
*
* Function:     gprintf
* Parameters:   fmt     - Format string
*               ...     - Standard printf style parameters
* Returns:      Number of items converted successfully.
*
* Description:  Simple printf style output routine for sending text to
*               the current graphics modes. It begins drawing the string at
*               the current location, and moves to the start of the next
*				logical line.
*
****************************************************************************/
{
    va_list argptr;                 /* Argument list pointer            */
    char    buf[255];               /* Buffer to build sting into       */
    int     cnt;                    /* Result of SPRINTF for return     */

	va_start(argptr,fmt);
	cnt = vsprintf(buf,fmt,argptr);
	SV_writeText(x,y,buf,DC->defcolor);
	va_end(argptr);
	return cnt;                     /* Return the conversion count      */
}

static void clearScreen(ulong color,int mix)
{
	AF_setMix(DC->AFDC,mix,mix);
	AF_drawRect(DC->AFDC,color,0,0,DC->maxx+1,DC->maxy+1);
}

static void testName(const char *msg)
{
	AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
	AF_drawRect(DC->AFDC,black,1,DC->maxy-16,320,DC->maxy+1);
	gprintf(1,DC->maxy-16,msg);
}

static int accelLineTest(int mix)
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

	if (!DC->AFDC->cFuncs.DrawLine)
		return -1;
	AF_setMix(DC->AFDC,mix,mix);
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

static int accelStippleLineTest(int mix)
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

	if (!DC->AFDC->cFuncs.DrawStippleLine)
        return 0;
	AF_setMix(DC->AFDC,mix,mix);
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

static int rectTest(int waitKey,int mix)
/****************************************************************************
*
* Function:     rectTest
*
* Description:  Draws a bunch of colored accelerated rectangles on the
*               display.
*
****************************************************************************/
{
    int x,y,i,j,width,height;
    int color;

	AF_setMix(DC->AFDC,mix,mix);
	width = 2 * ((DC->maxx+1) / 46)-1;
	height = 2 * ((DC->maxy-10)  / 47)-1;
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

static int pattRectTest(int mix)
/****************************************************************************
*
* Function:     pattRectTest
*
* Description:  Draws a bunch of patterned accelerated rectangles on the
*               display.
*
****************************************************************************/
{
    int x,y,i,j,width,height;
    int color;

	if (!DC->AFDC->cFuncs.DrawPattRect)
        return 0;

	AF_setMix(DC->AFDC,mix,mix);
	AF_set8x8MonoPattern(DC->AFDC,testPat);
	width = 2 * ((DC->maxx+1) / 46)-1;
	height = 2 * ((DC->maxy-10)  / 47)-1;
    x = width / 2;
    y = height / 2;
    color = 1;
    for (j = 0; j < 16; j++) {
        for (i = 0; i < 16; i++) {
            if (i % 2)
				AF_setMix(DC->AFDC,mix,mix);
			else
				AF_setMix(DC->AFDC,mix,AF_NOP_MIX);
            AF_drawPattRect(DC->AFDC,realColor(color++),realColor(2),x,y,width,height);
            x += (width/2) * 3;
            }
        y += (height / 2) * 3;
        x = width / 2;
        }
    return GetChar();
}

static void setupColorPat(int vertical)
{
	int i,j;

	if (vertical) {
		for (i = 0; i < 8; i++)
			for (j = 0; j < 8; j++)
				testColorPat[j][i] = realColor(i);
		}
	else {
		for (i = 0; i < 8; i++)
			for (j = 0; j < 8; j++)
				testColorPat[j][i] = realColor(j);
		}
}

static int colorPattRectTest(int mix)
/****************************************************************************
*
* Function:     colorPattRectTest
*
* Description:  Draws a bunch of color patterned accelerated rectangles on
*				the display.
*
****************************************************************************/
{
    int x,y,i,j,width,height;
    int color;

	if (!DC->AFDC->cFuncs.DrawColorPattRect)
		return 0;

	AF_setMix(DC->AFDC,mix,mix);
	width = 2 * ((DC->maxx+1) / 46)-1;
	height = 2 * ((DC->maxy-10)  / 47)-1;
    x = width / 2;
    y = height / 2;
    color = 1;
    for (j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			setupColorPat(i & 1);
			AF_set8x8ColorPattern(DC->AFDC,0,testColorPat);
			AF_use8x8ColorPattern(DC->AFDC,0);
			AF_drawColorPattRect(DC->AFDC,x,y,width,height);
            x += (width/2) * 3;
            }
        y += (height / 2) * 3;
        x = width / 2;
        }
    return GetChar();
}

static int scanTest(int mix)
/****************************************************************************
*
* Function:     scanTest
*
* Description:  Draws a bunch of colored accelerated scanlines on the
*               display.
*
****************************************************************************/
{
    int x,y,i,j,k,width,height;
    int color;

	if (!DC->AFDC->cFuncs.DrawScan)
		return 0;

	AF_setMix(DC->AFDC,mix,mix);
	width = 2 * ((DC->maxx+1) / 46)-1;
	height = 2 * ((DC->maxy-10)  / 47)-1;
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

static int pattScanTest(int mix)
/****************************************************************************
*
* Function:     pattScanTest
*
* Description:  Draws a bunch of colored accelerated scanlines on the
*               display.
*
****************************************************************************/
{
    int x,y,i,j,k,width,height;
    int color;

	if (!DC->AFDC->cFuncs.DrawPattScan)
        return 0;

	AF_setMix(DC->AFDC,mix,mix);
	AF_set8x8MonoPattern(DC->AFDC,testPat);
	width = 2 * ((DC->maxx+1) / 46)-1;
	height = 2 * ((DC->maxy-10)  / 47)-1;
    x = width / 2;
    y = height / 2;
    color = 1;
    for (j = 0; j < 16; j++) {
        for (i = 0; i < 16; i++) {
            if (i % 2)
				AF_setMix(DC->AFDC,mix,mix);
			else AF_setMix(DC->AFDC,mix,AF_NOP_MIX);
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

static int colorPattScanTest(int mix)
/****************************************************************************
*
* Function:     colorPattScanTest
*
* Description:  Draws a bunch of color patterned accelerated scanlines on the
*               display.
*
****************************************************************************/
{
    int x,y,i,j,k,width,height;
    int color;

	if (!DC->AFDC->cFuncs.DrawColorPattScan)
        return 0;

	AF_setMix(DC->AFDC,mix,mix);
	AF_set8x8ColorPattern(DC->AFDC,0,testColorPat);
	AF_use8x8ColorPattern(DC->AFDC,0);
	width = 2 * ((DC->maxx+1) / 46)-1;
	height = 2 * ((DC->maxy-10)  / 47)-1;
    x = width / 2;
    y = height / 2;
    color = 1;
    for (j = 0; j < 16; j++) {
        for (i = 0; i < 16; i++) {
			setupColorPat(i & 1);
			AF_set8x8ColorPattern(DC->AFDC,0,testColorPat);
			AF_use8x8ColorPattern(DC->AFDC,0);
			for (k = 0; k < height; k += 2)
				AF_drawColorPattScan(DC->AFDC,y+k,x,x+width);
            color++;
            x += (width/2) * 3;
            }
        y += (height / 2) * 3;
		x = width / 2;
		}
	return GetChar();
}

static int scanListTest(int mix)
/****************************************************************************
*
* Function:     scanListTest
*
* Description:  Draws an accelerated scanline list
*
****************************************************************************/
{
	if (!DC->AFDC->cFuncs.DrawScanList)
		return 0;

	AF_setMix(DC->AFDC,mix,mix);
	AF_drawScanList(DC->AFDC,realColor(1),10,10,scans);
	return GetChar();
}

static int pattScanListTest(int mix)
/****************************************************************************
*
* Function:     pattScanListTest
*
* Description:  Draws an accelerated scanline list
*
****************************************************************************/
{
	if (!DC->AFDC->cFuncs.DrawPattScanList)
		return 0;

	AF_setMix(DC->AFDC,mix,mix);
	AF_set8x8MonoPattern(DC->AFDC,testPat);
	AF_drawPattScanList(DC->AFDC,realColor(1),realColor(2),30,10,scans);
	return GetChar();
}

static int colorPattScanListTest(int mix)
/****************************************************************************
*
* Function:     colorPattScanListTest
*
* Description:  Draws an accelerated scanline list
*
****************************************************************************/
{
	if (!DC->AFDC->cFuncs.DrawColorPattScanList)
		return 0;

	AF_setMix(DC->AFDC,mix,mix);
	AF_set8x8ColorPattern(DC->AFDC,0,testColorPat);
	AF_use8x8ColorPattern(DC->AFDC,0);
	AF_drawColorPattScanList(DC->AFDC,50,10,scans);
    return GetChar();
}

static int trapTest(int mix)
/****************************************************************************
*
* Function:     trapTest
*
* Description:  Draws an accelerated trapezoid
*
****************************************************************************/
{
	AF_trap	trap;

	if (!DC->AFDC->cFuncs.DrawTrap)
		return 0;

	AF_setMix(DC->AFDC,mix,mix);
	trap.y = 30;
	trap.count = 100;
	trap.x1 = AF_TOFIX(100);
	trap.x2 = AF_TOFIX(100);
	trap.slope1 = -0x8000;
	trap.slope2 = 0x4000;
	AF_drawTrap(DC->AFDC,realColor(2),&trap);
	trap.count = 100;
	trap.slope1 = 0x8000;
	trap.slope2 = -0x4000;
	AF_drawTrap(DC->AFDC,realColor(3),&trap);
	return GetChar();
}

static uchar image[] = {
	0xFF, 0xDE, 0x7B, 0xFE, 0,  /* 11111111 11011110 01111011 11111110 00000000 */
	0xFF, 0xCC, 0x31, 0xFE, 0,  /* 11111111 11001100 00110001 11111110 00000000 */
	0x8C, 0x4C, 0x31, 0x82, 0,  /* 10001100 01001100 00110001 10000010 00000000 */
	0x0C, 0x0C, 0x31, 0x80, 0,  /* 00001100 00001100 00110001 10000000 00000000 */
	0x0C, 0x0C, 0x31, 0x90, 0,  /* 00001100 00001100 00110001 10010000 00000000 */
	0x0C, 0x0F, 0xF1, 0xF0, 0,  /* 00001100 00001111 11110001 11110000 00000000 */
	0x0C, 0x0F, 0xF1, 0xF0, 0,  /* 00001100 00001111 11110001 11110000 00000000 */
	0x0C, 0x0C, 0x31, 0x90, 0,  /* 00001100 00001100 00110001 10010000 00000000 */
	0x0C, 0x0C, 0x31, 0x80, 0,  /* 00001100 00001100 00110001 10000000 00000000 */
	0x0C, 0x0C, 0x31, 0x82, 0,  /* 00001100 00001100 00110001 10000010 00000000 */
	0x0C, 0x0C, 0x31, 0xFE, 0,  /* 00001100 00001100 00110001 11111110 00000000 */
	0x1E, 0x1E, 0x7B, 0xFE, 0,  /* 00011110 00011110 01111011 11111110 00000000 */
	};

static int putMonoImageTest(int mix)
/****************************************************************************
*
* Function:     putMonoImageTest
*
* Description:  Display a bunch of monochrome images
*
****************************************************************************/
{
	int x,y,byteWidth,height;

	if (!DC->AFDC->cFuncs.PutMonoImage)
		return 0;

	AF_setMix(DC->AFDC,mix,mix);
	byteWidth = 5;
	height = sizeof(image)/byteWidth;
	for (x = 0; x < DC->maxx; x += 40)
		for (y = 0; y < DC->maxy; y += sizeof(image)/byteWidth + 2) {
			if (x % 80)
				AF_setMix(DC->AFDC,mix,mix);
			else
				AF_setMix(DC->AFDC,mix,AF_NOP_MIX);
			AF_putMonoImage(DC->AFDC,realColor((x + y) % 16),realColor(2),x,y,byteWidth,
				0,0,byteWidth<<3,height,image);
			}
	return GetChar();
}

static int bitBltTest(int mix)
/****************************************************************************
*
* Function:     bitBltTest
*
* Description:  Scroll the display using a hardware bitBlt
*
****************************************************************************/
{
	int	i,left,top,width,height;

	if (!DC->AFDC->cFuncs.BitBlt)
		return 0;

	/* Scroll upwards */
	left = 0;				top = 5;
	width = DC->maxx+1-left;	height = DC->maxy+1-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"upwards\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,0,mix);
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Scroll downwards */
	left = 0;				top = 0;
	width = DC->maxx+1-left;	height = DC->maxy-4-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"downwards\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,5,mix);
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Scroll right */
	left = 0;				top = 0;
	width = DC->maxx-3-left;	height = DC->maxy+1-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"right\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,4,0,mix);
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Scroll left */
	left = 4;				top = 0;
	width = DC->maxx+1-left;	height = DC->maxy+1-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"left\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,0,mix);
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Scroll right and down */
	left = 0;				top = 0;
	width = DC->maxx-4-left;	height = DC->maxy-4-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"right,down\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,5,5,mix);
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Scroll left and up */
	left = 4;				top = 5;
	width = DC->maxx+1-left;	height = DC->maxy+1-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"left,up\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,0,mix);
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Scroll right and up */
	left = 0;				top = 5;
	width = DC->maxx-4-left;	height = DC->maxy-4-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"right,up\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,5,0,mix);
	if (GetChar() == 0x1B)
		return 0x1B;

	/* Scroll left and down */
	left = 4;				top = 0;
	width = DC->maxx+1-left;	height = DC->maxy+1-top;
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltTest(\"left,down\")");
	rectTest(1,AF_REPLACE_MIX);
	for (i = 0; i < DC->maxy; i += 8)
		AF_bitBlt(DC->AFDC,left,top,width,height,0,5,mix);
	return GetChar();
}

static int bitBltLinTest(void)
/****************************************************************************
*
* Function:     bitBltLinTest
*
* Description:  Perform a linear source bitBlt on the display
*
****************************************************************************/
{
    int i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->cFuncs.BitBltLin)
        return 0;

	/* Draw the background and the source to be blited */
	clearScreen(black,AF_REPLACE_MIX);
	testName("bitBltLinTest");
	rectTest(0,AF_REPLACE_MIX);
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
		AF_bitBltLin(DC->AFDC,xres*100,0,0,0,
			(xres/8) + (i * (xres/8)),i * (xres/8),
			xres/8,yres/6,AF_REPLACE_MIX);
		}
    return GetChar();
}

static int srcTransBltTest(void)
/****************************************************************************
*
* Function:     srcTransBltTest
*
* Description:  Perform a transparent bitBlt on the display
*
****************************************************************************/
{
	int i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->cFuncs.SrcTransBlt)
		return 0;

	/* Draw the background and the source to be blited */
	clearScreen(black,AF_REPLACE_MIX);
	testName("srcTransBltTest");
	rectTest(0,AF_REPLACE_MIX);
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

static int srcTransBltLinTest(void)
/****************************************************************************
*
* Function:     srcTransBltLinTest
*
* Description:  Perform a linear source transparent bitBlt on the display
*
****************************************************************************/
{
    int i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->cFuncs.SrcTransBltLin)
        return 0;

    /* Draw the background and the source to be blited */
	clearScreen(black,AF_REPLACE_MIX);
	testName("srcTransBltLinTest");
	rectTest(0,AF_REPLACE_MIX);
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
		AF_srcTransBltLin(DC->AFDC,xres*100,0,0,0,
            (xres/8) + (i * (xres/8)),i * (xres/8),
            xres/8,yres/6,
            AF_REPLACE_MIX,realColor(9));
        }
    return GetChar();
}

static int dstTransBltTest(void)
/****************************************************************************
*
* Function:     dstTransBltTest
*
* Description:  Perform a transparent bitBlt on the display
*
3****************************************************************************/
{
    int i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->cFuncs.DstTransBlt)
        return 0;

    /* Draw the background and the source to be blited */
	clearScreen(black,AF_REPLACE_MIX);
	testName("dstTransBltTest");
	rectTest(0,AF_REPLACE_MIX);
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

static int dstTransBltLinTest(void)
/****************************************************************************
*
* Function:     dstTransBltLinTest
*
* Description:  Perform a linear destination transparent bitBlt on the
*               display
*
****************************************************************************/
{
    int i,xres = DC->maxx+1,yres = DC->maxy+1;

	if (!DC->AFDC->cFuncs.DstTransBltLin)
        return 0;

    /* Draw the background and the source to be blited */
	clearScreen(black,AF_REPLACE_MIX);
	testName("dstTransBltLinTest");
	rectTest(0,AF_REPLACE_MIX);
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
		AF_dstTransBltLin(DC->AFDC,xres*100,0,0,0,
            (xres/8) + (i * (xres/8)),i * (xres/8),
            xres/8,yres/6,
            AF_REPLACE_MIX,0);
        }
    return GetChar();
}

static void getMousePos(int *x,int *y)
/****************************************************************************
*
* Function:     getMousePos
*
* Description:  Call the mouse driver to poll the mouse cursor location.
*               We dont bother with tricking the mouse driver so it knows
*               we are in a graphics mode, so the cursor will most likely
*               jump in 8 pixel steps, but this is suitable for our
*               purposes.
*
****************************************************************************/
{
#ifdef  __MSDOS__
    RMREGS  regs;
    regs.x.ax = 3;
    PM_int86(0x33,&regs,&regs);
    *x = regs.x.cx;
    *y = regs.x.dx;
#elif   defined(__WINDOWS__)
    POINT p;
    GetCursorPos(&p);
    *x = p.x;
    *y = p.y;
#endif
}

static int cursorTest(ibool haveAccel)
/****************************************************************************
*
* Function:     cursorTest
*
* Description:  Test the hardware mouse cursor routines.
*
****************************************************************************/
{
    int x,y,oldx = -1,oldy = -1;

	if (!DC->AFDC->cFuncs.SetCursor)
        return 0;

    if (haveAccel)
		rectTest(1,AF_REPLACE_MIX);
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

static int doAccelTests(void)
{
	black = realColor(0);
	white = realColor(15);

	clearScreen(black,AF_REPLACE_MIX);
	testName("rectTest(AF_REPLACE_MIX)");
	if (rectTest(1,AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("rectTest(AF_AND_MIX)");
	if (rectTest(1,AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("rectTest(AF_XOR_MIX)");
	if (rectTest(1,AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("rectTest(AF_OR_MIX)");
	if (rectTest(1,AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("pattRectTest(AF_REPLACE_MIX)");
	if (pattRectTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("pattRectTest(AF_AND_MIX)");
	if (pattRectTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("pattRectTest(AF_XOR_MIX)");
	if (pattRectTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("pattRectTest(AF_AND_MIX)");
	if (pattRectTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("colorPattRectTest(AF_REPLACE_MIX)");
	if (colorPattRectTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattRectTest(AF_AND_MIX)");
	if (colorPattRectTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattRectTest(AF_XOR_MIX)");
	if (colorPattRectTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattRectTest(AF_OR_MIX)");
	if (colorPattRectTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("scanTest(AF_REPLACE_MIX)");
	if (scanTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("scanTest(AF_AND_MIX)");
	if (scanTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("scanTest(AF_XOR_MIX)");
	if (scanTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("scanTest(AF_OR_MIX)");
	if (scanTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("pattScanTest(AF_REPLACE_MIX)");
	if (pattScanTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("pattScanTest(AF_AND_MIX)");
	if (pattScanTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("pattScanTest(AF_XOR_MIX)");
	if (pattScanTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("pattScanTest(AF_OR_MIX)");
	if (pattScanTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("colorPattScanTest(AF_REPLACE_MIX)");
	if (colorPattScanTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattScanTest(AF_AND_MIX)");
	if (colorPattScanTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattScanTest(AF_XOR_MIX)");
	if (colorPattScanTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattScanTest(AF_OR_MIX)");
	if (colorPattScanTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("scanListTest(AF_REPLACE_MIX)");
	if (scanListTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("scanListTest(AF_AND_MIX)");
	if (scanListTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("scanListTest(AF_XOR_MIX)");
	if (scanListTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("scanListTest(AF_AND_MIX)");
	if (scanListTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("pattScanListTest(AF_REPLACE_MIX)");
	if (pattScanListTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("pattScanListTest(AF_AND_MIX)");
	if (pattScanListTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("pattScanListTest(AF_OR_MIX)");
	if (pattScanListTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("pattScanListTest(AF_XOR_MIX)");
	if (pattScanListTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("colorPattScanListTest(AF_REPLACE_MIX)");
	if (colorPattScanListTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattScanListTest(AF_AND_MIX)");
	if (colorPattScanListTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattScanListTest(AF_XOR_MIX)");
	if (colorPattScanListTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("colorPattScanListTest(AF_OR_MIX)");
	if (colorPattScanListTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("trapTest(AF_REPLACE_MIX)");
	if (trapTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("trapTest(AF_AND_MIX)");
	if (trapTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("trapTest(AF_XOR_MIX)");
	if (trapTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("trapTest(AF_OR_MIX)");
	if (trapTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("accelLineTest(AF_REPLACE_MIX)");
	if (accelLineTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("accelLineTest(AF_AND_MIX)");
	if (accelLineTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("accelLineTest(AF_XOR_MIX)");
	if (accelLineTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("accelLineTest(AF_OR_MIX)");
	if (accelLineTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("accelStippleLineTest(AF_REPLACE_MIX)");
	if (accelStippleLineTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("accelStippleLineTest(AF_AND_MIX)");
	if (accelStippleLineTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("accelStippleLineTest(AF_XOR_MIX)");
	if (accelStippleLineTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("accelStippleLineTest(AF_OR_MIX)");
	if (accelStippleLineTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	clearScreen(black,AF_REPLACE_MIX);
	testName("putMonoImageTest(AF_REPLACE_MIX)");
	if (putMonoImageTest(AF_REPLACE_MIX) == 0x1B)
		return 0x1B;
	testName("putMonoImageTest(AF_AND_MIX)");
	if (putMonoImageTest(AF_AND_MIX) == 0x1B)
		return 0x1B;
	testName("putMonoImageTest(AF_XOR_MIX)");
	if (putMonoImageTest(AF_XOR_MIX) == 0x1B)
		return 0x1B;
	testName("putMonoImageTest(AF_OR_MIX)");
	if (putMonoImageTest(AF_OR_MIX) == 0x1B)
		return 0x1B;

	if (bitBltTest(AF_REPLACE_MIX) == 0x1B)
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
#endif

void doAFTests(SV_devCtx *DC)
{
#ifdef TEST_AF
	/* Test accelerated drawing */
	if (DC->AFDC->cFuncs.SetClipRect)
		AF_setClipRect(DC->AFDC,0,0,DC->maxx,DC->maxy);
    if (doAccelTests() == 0x1B)
        return;
    AF_setMix(DC->AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
    AF_drawRect(DC->AFDC,black,0,0,DC->maxx+1,DC->maxy+1);
	if (DC->AFDC->cFuncs.SetClipRect) {
        AF_setClipRect(DC->AFDC,50,50,DC->maxx-50,DC->maxy-50);
        if (doAccelTests() == 0x1B)
            return;
        }

    /* Test hardware cursor */
	cursorTest(DC->AFDC->cFuncs.DrawRect != NULL);
#endif
}

#endif
