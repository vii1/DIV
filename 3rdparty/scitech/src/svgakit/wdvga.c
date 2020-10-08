/****************************************************************************
*
*                          WinDirect Test Program
*
*                   Copyright (C) 1996 SciTech Software.
*                           All rights reserved.
*
* Filename:     $Workfile:   wdvga.c  $
* Version:      $Revision:   1.1  $
* Author:       Kendall Bennett
*
* Language:     ANSI C
* Environment:  Win16/32, running on Windows 3.1 or Windows '95 or DOS
*
* Description:  Simple WinDirect program to start Standard VGA mode 13h
*               and draw some lines on the display, and to wait until a
*               key is pressed.
*
* $Date:   17 Feb 1996 19:34:52  $ $Author:   KendallB  $
*
****************************************************************************/

#include "wdirect.h"
#include "pmode.h"

/*--------------------------- Global Variables ----------------------------*/

int     xres,yres;                  /* Resolution of video mode used    */
int     bytesperline;               /* Logical CRT scanline length      */
char    *screenPtr;                 /* Pointer to start of video memory */

/*------------------------------ Implementation ---------------------------*/

/* Plot a pixel at location (x,y) in specified color */

void putPixel(int x,int y,int color)
{
    *(screenPtr + y * bytesperline + x) = color;
}

/* Draw a line from (x1,y1) to (x2,y2) in specified color */

void line(int x1,int y1,int x2,int y2,int color)
{
    int     d;                      /* Decision variable                */
    int     dx,dy;                  /* Dx and Dy values for the line    */
    int     Eincr,NEincr;           /* Decision variable increments     */
    int     yincr;                  /* Increment for y values           */
    int     t;                      /* Counters etc.                    */

#define ABS(a)   ((a) >= 0 ? (a) : -(a))

    dx = ABS(x2 - x1);
    dy = ABS(y2 - y1);
    if (dy <= dx) {
        /* We have a line with a slope between -1 and 1
         *
         * Ensure that we are always scan converting the line from left to
         * right to ensure that we produce the same line from P1 to P0 as the
         * line from P0 to P1.
         */
        if (x2 < x1) {
            t = x2; x2 = x1; x1 = t;    /* Swap X coordinates           */
            t = y2; y2 = y1; y1 = t;    /* Swap Y coordinates           */
            }
        if (y2 > y1)
            yincr = 1;
        else
            yincr = -1;
        d = 2*dy - dx;              /* Initial decision variable value  */
        Eincr = 2*dy;               /* Increment to move to E pixel     */
        NEincr = 2*(dy - dx);       /* Increment to move to NE pixel    */
        putPixel(x1,y1,color);      /* Draw the first point at (x1,y1)  */

        /* Incrementally determine the positions of the remaining pixels
         */
        for (x1++; x1 <= x2; x1++) {
            if (d < 0)
                d += Eincr;         /* Choose the Eastern Pixel         */
            else {
                d += NEincr;        /* Choose the North Eastern Pixel   */
                y1 += yincr;        /* (or SE pixel for dx/dy < 0!)     */
                }
            putPixel(x1,y1,color);  /* Draw the point                   */
            }
        }
    else {
        /* We have a line with a slope between -1 and 1 (ie: includes
         * vertical lines). We must swap our x and y coordinates for this.
         *
         * Ensure that we are always scan converting the line from left to
         * right to ensure that we produce the same line from P1 to P0 as the
         * line from P0 to P1.
         */
        if (y2 < y1) {
            t = x2; x2 = x1; x1 = t;    /* Swap X coordinates           */
            t = y2; y2 = y1; y1 = t;    /* Swap Y coordinates           */
            }
        if (x2 > x1)
            yincr = 1;
        else
            yincr = -1;
        d = 2*dx - dy;              /* Initial decision variable value  */
        Eincr = 2*dx;               /* Increment to move to E pixel     */
        NEincr = 2*(dx - dy);       /* Increment to move to NE pixel    */
        putPixel(x1,y1,color);      /* Draw the first point at (x1,y1)  */

        /* Incrementally determine the positions of the remaining pixels
         */
        for (y1++; y1 <= y2; y1++) {
            if (d < 0)
                d += Eincr;         /* Choose the Eastern Pixel         */
            else {
                d += NEincr;        /* Choose the North Eastern Pixel   */
                x1 += yincr;        /* (or SE pixel for dx/dy < 0!)     */
                }
            putPixel(x1,y1,color);  /* Draw the point                   */
            }
        }
}

/* Draw a simple pattern of lines on the display */

void drawMoire(void)
{
    int     i;

    for (i = 0; i < xres; i += 5) {
        line(xres/2,yres/2,i,0,i % 0xFF);
        line(xres/2,yres/2,i,yres,(i+1) % 0xFF);
        }
    for (i = 0; i < yres; i += 5) {
        line(xres/2,yres/2,0,i,(i+2) % 0xFF);
        line(xres/2,yres/2,xres,i,(i+3) % 0xFF);
        }
	line(0,0,xres-1,0,15);
    line(0,0,0,yres-1,15);
    line(xres-1,0,xres-1,yres-1,15);
    line(0,yres-1,xres-1,yres-1,15);
}

#ifdef	__WINDOWS__
int PASCAL WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR szCmdLine,int sw)
#else
int main(void)
#endif
{
    RMREGS      regs;
    WD_event    evt;

    /* Shutdown GDI and start VGA mode 13h */
    WD_startFullScreen(NULL,320,200);
    regs.x.ax = 0x13;
    PM_int86(0x10,&regs,&regs);
    WD_inFullScreen();
    xres = 320;
    yres = 200;
    bytesperline = 320;
    screenPtr = PM_mapPhysicalAddr(0xA0000,0xFFFF);

    /* Draw a pattern of lines */
    drawMoire();

    /* Wait for a key or mouse click */
    WD_haltEvent(&evt,EVT_KEYDOWN | EVT_MOUSEDOWN);

    /* Restore text mode */
    regs.x.ax = 0x3;
    PM_int86(0x10,&regs,&regs);

    /* Restore GDI and exit */
    WD_restoreGDI();
    return 0;
}

