/****************************************************************************
*
*                               Hello VBE!
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Language:     ANSI C
* Environment:  IBM PC (MSDOS) 16 bit Real Mode
* Author:       Kendall Bennett, SciTech Software
*
* Description:  Simple 'Hello World' program to initialise a user
*               specified 256 color graphics mode, and display a simple
*               moire pattern. Tested with VBE 1.2 and above.
*
*               This code does not have any hard-coded VBE mode numbers,
*               but will use the VBE 2.0 aware method of searching for
*               available video modes, so will work with any new extended
*               video modes defined by a particular OEM VBE 2.0 version.
*
*               For brevity we dont check for failure conditions returned
*               by the VBE (but we shouldn't get any).
*
* $Date:   05 Feb 1996 18:35:20  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>

/* Uncomment the following to enable direct bank switching, rather than
 * using the Int 10h software interrupt (requires Borland C++ inline
 * assembler).
 */

/* #define  DIRECT_BANKING */

/*---------------------- Macro and type definitions -----------------------*/

/* SuperVGA information block */

typedef struct {
    char    VESASignature[4];       /* 'VESA' 4 byte signature          */
    short   VESAVersion;            /* VBE version number               */
    char    _far *OEMStringPtr;     /* Pointer to OEM string            */
    long    Capabilities;           /* Capabilities of video card       */
    short   _far *VideoModePtr;     /* Pointer to supported modes       */
    short   TotalMemory;            /* Number of 64kb memory blocks     */
    char    reserved[236];          /* Pad to 256 byte block size       */
    } VgaInfoBlock;

/* SuperVGA mode information block */

typedef struct {
    short   ModeAttributes;         /* Mode attributes                  */
    char    WinAAttributes;         /* Window A attributes              */
    char    WinBAttributes;         /* Window B attributes              */
    short   WinGranularity;         /* Window granularity in k          */
    short   WinSize;                /* Window size in k                 */
    short   WinASegment;            /* Window A segment                 */
    short   WinBSegment;            /* Window B segment                 */
    void    _far *WinFuncPtr;       /* Pointer to window function       */
    short   BytesPerScanLine;       /* Bytes per scanline               */
    short   XResolution;            /* Horizontal resolution            */
    short   YResolution;            /* Vertical resolution              */
    char    XCharSize;              /* Character cell width             */
    char    YCharSize;              /* Character cell height            */
    char    NumberOfPlanes;         /* Number of memory planes          */
    char    BitsPerPixel;           /* Bits per pixel                   */
    char    NumberOfBanks;          /* Number of CGA style banks        */
    char    MemoryModel;            /* Memory model type                */
    char    BankSize;               /* Size of CGA style banks          */
    char    NumberOfImagePages;     /* Number of images pages           */
    char    res1;                   /* Reserved                         */
    char    RedMaskSize;            /* Size of direct color red mask    */
    char    RedFieldPosition;       /* Bit posn of lsb of red mask      */
    char    GreenMaskSize;          /* Size of direct color green mask  */
    char    GreenFieldPosition;     /* Bit posn of lsb of green mask    */
    char    BlueMaskSize;           /* Size of direct color blue mask   */
    char    BlueFieldPosition;      /* Bit posn of lsb of blue mask     */
    char    RsvdMaskSize;           /* Size of direct color res mask    */
    char    RsvdFieldPosition;      /* Bit posn of lsb of res mask      */
    char    DirectColorModeInfo;    /* Direct color mode attributes     */
    char    res2[216];              /* Pad to 256 byte block size       */
    } ModeInfoBlock;

typedef enum {
    memPL       = 3,                /* Planar memory model              */
    memPK       = 4,                /* Packed pixel memory model        */
    memRGB      = 6,                /* Direct color RGB memory model    */
    memYUV      = 7,                /* Direct color YUV memory model    */
    } memModels;

/*--------------------------- Global Variables ----------------------------*/

int     xres,yres;                  /* Resolution of video mode used    */
int     bytesperline;               /* Logical CRT scanline length      */
int     curBank;                    /* Current read/write bank          */
int     bankShift;                  /* Bank granularity adjust factor   */
int     oldMode;                    /* Old video mode number            */
void    _far (*bankSwitch)(void);   /* Direct bank switching function   */
char    _far *screenPtr;            /* Pointer to start of video memory */

/*------------------------ VBE Interface Functions ------------------------*/

/* Get SuperVGA information, returning true if VBE found */

int getVgaInfo(VgaInfoBlock *vgaInfo)
{
    union REGS regs;
    struct SREGS sregs;
    regs.x.ax = 0x4F00;
    regs.x.di = FP_OFF(vgaInfo);
    sregs.es = FP_SEG(vgaInfo);
    int86x(0x10, &regs, &regs, &sregs);
    return (regs.x.ax == 0x4F);
}

/* Get video mode information given a VBE mode number. We return 0 if
 * if the mode is not available, or if it is not a 256 color packed
 * pixel mode.
 */

int getModeInfo(int mode, ModeInfoBlock *modeInfo)
{
    union REGS regs;
    struct SREGS sregs;
    if (mode < 0x100) return 0;     /* Ignore non-VBE modes             */
    regs.x.ax = 0x4F01;
    regs.x.cx = mode;
    regs.x.di = FP_OFF(modeInfo);
    sregs.es = FP_SEG(modeInfo);
    int86x(0x10, &regs, &regs, &sregs);
    if (regs.x.ax != 0x4F) return 0;
    if ((modeInfo->ModeAttributes & 0x1) && modeInfo->MemoryModel == memPK
            && modeInfo->BitsPerPixel == 8 && modeInfo->NumberOfPlanes == 1)
        return 1;
    return 0;
}

/* Set a VBE video mode */

void setVBEMode(int mode)
{
    union REGS regs;
    regs.x.ax = 0x4F02; regs.x.bx = mode;
    int86(0x10,&regs,&regs);
}

/* Return the current VBE video mode */

int getVBEMode(void)
{
    union REGS regs;
    regs.x.ax = 0x4F03;
    int86(0x10,&regs,&regs);
    return regs.x.bx;
}

/* Set new read/write bank. We must set both Window A and Window B, as
 * many VBE's have these set as separately available read and write
 * windows. We also use a simple (but very effective) optimisation of
 * checking if the requested bank is currently active.
 */

void setBank(int bank)
{
    union REGS  regs;
    if (bank == curBank) return;    /* Bank is already active           */
    curBank = bank;                 /* Save current bank number         */
    bank <<= bankShift;             /* Adjust to window granularity     */
#ifdef  DIRECT_BANKING
asm {   mov bx,0;
        mov dx,bank; }
    bankSwitch();
asm {   mov bx,1;
        mov dx,bank; }
    bankSwitch();
#else
    regs.x.ax = 0x4F05; regs.x.bx = 0;  regs.x.dx = bank;
    int86(0x10, &regs, &regs);
    regs.x.ax = 0x4F05; regs.x.bx = 1;  regs.x.dx = bank;
    int86(0x10, &regs, &regs);
#endif
}

/*-------------------------- Application Functions ------------------------*/

/* Plot a pixel at location (x,y) in specified color (8 bit modes only) */

void putPixel(int x,int y,int color)
{
    long addr = (long)y * bytesperline + x;
    setBank(addr >> 16);
    *(screenPtr + (addr & 0xFFFF)) = color;
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

/* Draw a simple moire pattern of lines on the display */

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

/* Display a list of available resolutions. Be careful with calls to
 * function 00h to get SuperVGA mode information. Many VBE's build the
 * list of video modes directly in this information block, so if you
 * are using a common buffer (which we aren't here, but in protected
 * mode you will), then you will need to make a local copy of this list
 * of available modes.
 */

void availableModes(void)
{
    short           *p;
    VgaInfoBlock    vgaInfo;
    ModeInfoBlock   modeInfo;

    if (!getVgaInfo(&vgaInfo)) {
        printf("No VESA VBE detected\n");
        exit(1);
        }
    printf("VESA VBE Version %d.%d detected (%s)\n\n",
        vgaInfo.VESAVersion >> 8, vgaInfo.VESAVersion & 0xF,
        vgaInfo.OEMStringPtr);
    printf("Available 256 color video modes:\n");
    for (p = vgaInfo.VideoModePtr; *p != -1; p++) {
        if (getModeInfo(*p, &modeInfo)) {
            printf("    %4d x %4d %d bits per pixel\n",
                modeInfo.XResolution, modeInfo.YResolution,
                modeInfo.BitsPerPixel);
            }
        }
    printf("\nUsage: hellovbe <xres> <yres>\n");
    exit(1);
}

/* Initialise the specified video mode. Notice how we determine a shift
 * factor for adjusting the Window granularity for bank switching. This
 * is much faster than doing it with a multiply (especially with direct
 * banking enabled).
 */

void initGraphics(int x,int y)
{
    short           *p;
    VgaInfoBlock    vgaInfo;
    ModeInfoBlock   modeInfo;

    if (!getVgaInfo(&vgaInfo)) {
        printf("No VESA VBE detected\n");
        exit(1);
        }
    for (p = vgaInfo.VideoModePtr; *p != -1; p++) {
        if (getModeInfo(*p, &modeInfo) && modeInfo.XResolution == x
                && modeInfo.YResolution == y) {
            xres = x;   yres = y;
            bytesperline = modeInfo.BytesPerScanLine;
            bankShift = 0;
            while ((64 >> bankShift) != modeInfo.WinGranularity)
                bankShift++;
            bankSwitch = modeInfo.WinFuncPtr;
            curBank = -1;
            screenPtr = MK_FP(0xA000,0);
            oldMode = getVBEMode();
            setVBEMode(*p);
            return;
            }
        }
    printf("Valid video mode not found\n");
    exit(1);
}

/* Main routine. Expects the x & y resolution of the desired video mode
 * to be passed on the command line. Will print out a list of available
 * video modes if no command line is present.
 */

void main(int argc,char *argv[])
{
    int x,y;

    if (argc != 3)
        availableModes();       /* Display list of available modes      */

    x = atoi(argv[1]);          /* Get requested resolution             */
    y = atoi(argv[2]);
    initGraphics(x,y);          /* Start requested video mode           */
    drawMoire();                /* Draw a moire pattern                 */
    getch();                    /* Wait for keypress                    */
    setVBEMode(oldMode);        /* Restore previous mode                */
}
