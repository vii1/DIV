/****************************************************************************
*
*			The SuperVGA Kit - UniVBE Software Development Kit
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Filename:     $Workfile:   tests.c  $
* Version:      $Revision:   1.7  $
*
* Language:     ANSI C
* Environment:  IBM PC (MSDOS) Real Mode and 16/32 bit Protected Mode.
*
* Description:  Simple module to test the operation of the SuperVGA
*               bank switching code and page flipping code for the
*               all supported video modes.
*
*               MUST be compiled in the large or flat models.
*
* $Date:   19 Apr 1996 19:18:32  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "svga.h"
#include "ztimer.h"

/*---------------------------- Global Variables ---------------------------*/

PRIVATE	int     	cntMode,x,y;
extern	SV_devCtx	*DC;

/*----------------------------- Implementation ----------------------------*/

/* Keyboard handlers provided by main application	*/
int KeyHit(void);
int GetChar(void);

void us_delay(long us)
/****************************************************************************
*
* Function:		us_delay
* Parameters:	us	- Number of microseconds to delay for
*
* Description:	Delays for the specified number of microseconds. We simply
*				use the Zen Timer routines to do this for us, since the
*				delay() function is not normally supported across all
*				compilers.
*
****************************************************************************/
{
	ZTimerInit();
	LZTimerOn();
	while (LZTimerLap() < us)
		;
	LZTimerOff();
}

void moire(ulong defcolor)
/****************************************************************************
*
* Function:     moire
*
* Description:  Draws a simple Moire pattern on the display screen using
*               lines.
*
****************************************************************************/
{
	int     i,value;

	SV_beginLine();
	if (DC->maxcolor >= 0x7FFFL) {
		for (i = 0; i < DC->maxx; i++) {
			SV_lineFast(DC->maxx/2,DC->maxy/2,i,0,SV_rgbColor((uchar)((i*255L)/DC->maxx),0,0));
			SV_lineFast(DC->maxx/2,DC->maxy/2,i,DC->maxy,SV_rgbColor(0,(uchar)((i*255L)/DC->maxx),0));
			}
		for (i = 0; i < DC->maxy; i++) {
			value = (int)((i*255L)/DC->maxy);
			SV_lineFast(DC->maxx/2,DC->maxy/2,0,i,SV_rgbColor((uchar)value,0,(uchar)(255 - value)));
			SV_lineFast(DC->maxx/2,DC->maxy/2,DC->maxx,i,SV_rgbColor(0,(uchar)(255 - value),(uchar)value));
			}
		}
	else {
		for (i = 0; i < DC->maxx; i += 5) {
			SV_lineFast(DC->maxx/2,DC->maxy/2,i,0,i % DC->maxcolor);
			SV_lineFast(DC->maxx/2,DC->maxy/2,i,DC->maxy,(i+1) % DC->maxcolor);
			}
		for (i = 0; i < DC->maxy; i += 5) {
			SV_lineFast(DC->maxx/2,DC->maxy/2,0,i,(i+2) % DC->maxcolor);
			SV_lineFast(DC->maxx/2,DC->maxy/2,DC->maxx,i,(i+3) % DC->maxcolor);
			}
		}
	SV_lineFast(0,0,DC->maxx,0,defcolor);
	SV_lineFast(0,0,0,DC->maxy,defcolor);
	SV_lineFast(DC->maxx,0,DC->maxx,DC->maxy,defcolor);
	SV_lineFast(0,DC->maxy,DC->maxx,DC->maxy,defcolor);
	SV_endLine();
}

void displayModeInfo(void)
/****************************************************************************
*
* Function:     displayModeInfo
*
* Description:  Display the information about the video mode.
*
****************************************************************************/
{
	char    buf[80];

	sprintf(buf,"Video mode: %d x %d %d bit",DC->maxx+1,DC->maxy+1,DC->bitsperpixel);
	SV_writeText(x,y,buf,DC->defcolor);    y += 16;
	if (cntMode & vbeLinearBuffer) {
		sprintf(buf,"Using linear frame buffer");
		SV_writeText(x,y,buf,DC->defcolor);    y += 16;
		}
	else if (DC->virtualBuffer) {
		sprintf(buf,"Using *virtual* linear frame buffer");
		SV_writeText(x,y,buf,DC->defcolor);    y += 16;
		}
}

int moireTest(void)
/****************************************************************************
*
* Function:     moireTest
*
* Description:  Draws a simple Moire pattern on the display screen using
*               lines, and waits for a key press.
*
****************************************************************************/
{
	char    buf[80];

	moire(DC->defcolor);
	if (DC->maxx > 512) {
		x = 80;
		y = 80;
		SV_writeText(x,y,"Bank switching test",DC->defcolor);  y += 32;
		displayModeInfo();
		sprintf(buf,"Maximum x: %d, Maximum y: %d, BytesPerLine %d, Pages: %d",
			DC->maxx,DC->maxy,DC->bytesperline,DC->maxpage+1);
		SV_writeText(x,y,buf,DC->defcolor);    y += 32;
		SV_writeText(x,y,"You should see a colorful Moire pattern on the screen",DC->defcolor);
		y += 16;
		}
	else {
		x = 40;
		y = 40;
		displayModeInfo();
		}
	SV_writeText(x,y,"Press any key to continue",DC->defcolor);
	y += 32;
	return GetChar();
}

int pageFlipTest(bool waitVRT)
/****************************************************************************
*
* Function:     pageFlipTest
*
* Description:  Animates a line on the display using page flipping if
*               page flipping is active.
*
****************************************************************************/
{
	int     i,j,istep,jstep,apage,vpage,fpsRate = 0,key = 0;
	ulong	color,lastCount = 0,newCount;
    char    buf[80];

	if (DC->maxpage != 0) {
		vpage = 0;
		apage = 1;
		SV_setActivePage(apage);
		SV_setVisualPage(vpage,waitVRT);
		i = 0;
		j = DC->maxy;
		istep = 2;
		jstep = -2;
		color = 15;
		if (DC->maxcolor > 255)
			color = DC->defcolor;
		ZTimerInit();
		LZTimerOn();
		while (!KeyHit()) {
			SV_setActivePage(apage);
			SV_clear(0);
			sprintf(buf,"%3d.%d fps", fpsRate / 10, fpsRate % 10);
			SV_writeText(4,4,buf,DC->defcolor);
			sprintf(buf,"Page %d of %d", apage+1, DC->maxpage+1);
			if (DC->maxx <= 360) {
				SV_writeText(4,80,"Page flipping - should be no flicker",DC->defcolor);
				SV_writeText(4,100,buf,DC->defcolor);
                }
			else {
				SV_writeText(80,80,"Page flipping - should be no flicker",DC->defcolor);
				SV_writeText(80,100,buf,DC->defcolor);
				}
			SV_beginLine();
			SV_lineFast(i,0,DC->maxx-i,DC->maxy,color);
			SV_lineFast(0,DC->maxy-j,DC->maxx,j,color);
			SV_lineFast(0,0,DC->maxx,0,DC->defcolor);
			SV_lineFast(0,0,0,DC->maxy,DC->defcolor);
			SV_lineFast(DC->maxx,0,DC->maxx,DC->maxy,DC->defcolor);
			SV_lineFast(0,DC->maxy,DC->maxx,DC->maxy,DC->defcolor);
			SV_endLine();
			vpage = ++vpage % (DC->maxpage+1);
			SV_setVisualPage(vpage,waitVRT);
			apage = ++apage % (DC->maxpage+1);
			i += istep;
			if (i > DC->maxx) {
				i = DC->maxx-2;
				istep = -2;
				}
			if (i < 0)  i = istep = 2;
			j += jstep;
			if (j > DC->maxy) {
				j = DC->maxy-2;
				jstep = -2;
				}
			if (j < 0)  j = jstep = 2;

			/* Compute the frames per second rate after going through an entire
			 * set of display pages.
			 */
			if (apage == 0) {
				newCount = LZTimerLap();
				fpsRate = (int)(10000000L / (newCount - lastCount)) * (DC->maxpage+1);
				lastCount = newCount;
                }
			}
		LZTimerOff();
		key = GetChar();				/* Swallow keypress */
		}
	SV_setActivePage(0);
	SV_setVisualPage(0,false);
	return key;
}

PRIVATE bool SV_setBytesPerLine(int bytes)
/****************************************************************************
*
* Function:		SV_setBytesPerLine
* Parameters:	bytes	- New bytes per line value
* Returns:		True on success, false on failure.
*
* Description:	Sets the scanline length to a specified bytes per line
*				value. This function only works with VBE 2.0.
*
****************************************************************************/
{
	int	newbytes,xres,yres;

	if (!VBE_setBytesPerLine(bytes,&newbytes,&xres,&yres))
		return false;
	DC->bytesperline = newbytes;
	DC->maxx = xres-1;
	DC->maxy = yres-1;
	return true;
}

PRIVATE bool SV_setPixelsPerLine(int xMax)
/****************************************************************************
*
* Function:		SV_setPixelsPerLine
* Parameters:	xMax	- New pixels per line value
* Returns:		True on success, false on failure.
*
* Description:	Sets the scanline length to a specified pixels per line
*				value. This function only works with VBE 1.2 and above.
*
****************************************************************************/
{
	int	newbytes,xres,yres;

	if (!VBE_setPixelsPerLine(xMax,&newbytes,&xres,&yres))
		return false;
	DC->bytesperline = newbytes;
	DC->maxx = xres-1;
	DC->maxy = yres-1;
	return true;
}

int virtualTestVBE(bool useVirtualBuffer)
/****************************************************************************
*
* Function:		virtualTestVBE
*
* Description:	Checks the CRT logical scanline length routines, setting
*				up a virtual display buffer and scrolling around within
*				this buffer.
*
****************************************************************************/
{
	int		i,x,y,scrollx,scrolly,oldmaxx,oldmaxy,oldbytesperline,max,key;
	char	buf[80];

	if (!VBE_setDisplayStart(10,10,false))
		return 0;
	if (DC->maxx == 319 && DC->maxy == 199 && DC->maxpage == 0)
		return 0;
	SV_setMode(VBE_getVideoMode(),false,useVirtualBuffer,0);

	/* Set up for the widest possible virtual display buffer */

	oldmaxx = DC->maxx;
	oldmaxy = DC->maxy;
	oldbytesperline = DC->bytesperline;

	/* Find the largest value that we can set the virtual buffer width
	 * to that the VBE supports
	 */
	switch (DC->bitsperpixel) {
		case 4:		max = (int)((DC->memory*2048L) / (DC->maxy+1));	break;
		case 8:		max = (int)((DC->memory*1024L) / (DC->maxy+1));	break;
		case 15:
		case 16:	max = (int)((DC->memory*512L) / (DC->maxy+1));	break;
		case 24:	max = (int)((DC->memory*341L) / (DC->maxy+1));	break;
		case 32:	max = (int)((DC->memory*256L) / (DC->maxy+1));	break;
		}

	for (i = max; i > oldmaxx+1; i--) {
		if (!SV_setPixelsPerLine(i))
			continue;
		if (DC->maxx > oldmaxx+1 && DC->maxx < max)
			break;				/* Large value has been set			*/
		}

	/* Perform huge horizontal scroll */

	VBE_setDisplayStart(0,0,false);
	SV_clear(0);
	moire(DC->defcolor);
	if (DC->maxx == oldmaxx) {
		sprintf(buf,"Virtual buffer not resizeable in this mode (still %d x %d pixels)",DC->maxx+1,DC->maxy+1);
		SV_writeText(20,40,buf,DC->defcolor);
		SV_writeText(20,60,"Press any key to begin vertical scrolling",DC->defcolor);
		goto StartVerticalScroll;
		}
	else
		sprintf(buf,"Virtual buffer now set to %d x %d pixels",DC->maxx+1,DC->maxy+1);
	SV_writeText(20,40,buf,DC->defcolor);
	SV_writeText(20,60,"Press any key to begin virtual scrolling",DC->defcolor);
	GetChar();
	scrollx = DC->maxx-oldmaxx;
	scrolly = DC->maxy-oldmaxy;
	for (x = y = 0; x <= scrollx; x++) {
		VBE_setDisplayStart(x,y,false);
		us_delay(1000);
		if (KeyHit())
			goto DoneHorizontal;
		}
	for (x = scrollx,y = 0; y <= scrolly; y++) {
		VBE_setDisplayStart(x,y,false);
		us_delay(1000);
		if (KeyHit())
			goto DoneHorizontal;
		}
	for (x = scrollx,y = scrolly; x >= 0; x--) {
		VBE_setDisplayStart(x,y,false);
		us_delay(1000);
		if (KeyHit())
			goto DoneHorizontal;
		}
	for (x = 0,y = scrolly; y >= 0; y--) {
		VBE_setDisplayStart(x,y,false);
		us_delay(1000);
		if (KeyHit())
			goto DoneHorizontal;
		}

DoneHorizontal:
	if ((key = GetChar()) == 0x1B)
		goto ResetMode;
	if (DC->maxx == oldmaxx) goto ResetMode;

	/* Now perform huge vertical scroll */

	VBE_setDisplayStart(0,0,false);
	if (DC->VBEVersion < 0x200)
		SV_setPixelsPerLine(oldmaxx+1);
	else
		SV_setBytesPerLine(oldbytesperline);
	DC->maxx = oldmaxx;
	SV_clear(0);
	moire(DC->defcolor);
	sprintf(buf,"Virtual buffer now set to %d x %d pixels",DC->maxx+1,DC->maxy+1);
	SV_writeText(20,40,buf,DC->defcolor);
	SV_writeText(20,60,"Press any key to begin virtual scrolling",DC->defcolor);
StartVerticalScroll:
	if ((key = GetChar()) == 0x1B)
		goto ResetMode;
	scrolly = DC->maxy-oldmaxy;
	for (y = 0; y <= scrolly; y++) {
		VBE_setDisplayStart(0,y,false);
		us_delay(1000);
		if (KeyHit())
			goto DoneVertical;
		}
	for (y = scrolly; y >= 0; y--) {
		VBE_setDisplayStart(0,y,false);
		us_delay(1000);
		if (KeyHit())
			goto DoneVertical;
		}
DoneVertical:
	key = GetChar();
ResetMode:
	SV_setMode(VBE_getVideoMode(),false,useVirtualBuffer,0);
	return key;
}

int virtualTestAF(void)
/****************************************************************************
*
* Function:		virtualTestAF
*
* Description:	Virtual scrolling test for VBE/AF modes.
*
****************************************************************************/
{
	return 0;
}

int wideDACTest(void)
/****************************************************************************
*
* Function:		wideDACTest
*
* Description:  Displays a set of color values using the wide DAC support
*				if available.
*
****************************************************************************/
{
	int			i,key;
	SV_palette	pal[256];

	if (DC->AFDC || !VBE_setDACWidth(8))
		return 0;

	memset(pal,0,256*sizeof(SV_palette));
	for (i = 0; i < 256; i += 4) {
		pal[64 + (i >> 2)].red = i;
		pal[128 + (i >> 2)].green = i;
		pal[192 + (i >> 2)].blue = i;
		}
	pal[(int)DC->defcolor].red = 255;
	pal[(int)DC->defcolor].green = 255;
	pal[(int)DC->defcolor].blue = 255;

	VBE_setPalette(0,256,(VBE_palette*)pal,false);
	SV_clear(0);
	SV_beginLine();
	SV_lineFast(0,0,DC->maxx,0,DC->defcolor);
	SV_lineFast(0,0,0,DC->maxy,DC->defcolor);
	SV_lineFast(DC->maxx,0,DC->maxx,DC->maxy,DC->defcolor);
	SV_lineFast(0,DC->maxy,DC->maxx,DC->maxy,DC->defcolor);
	SV_endLine();

	if (DC->maxx > 360) {
		x = 80;
		y = 80;
		}
	else {
		x = 40;
		y = 40;
		}

	SV_writeText(x,y,"Wide DAC test",DC->defcolor);
	y += 32;
	if (DC->maxx > 360) {
		SV_writeText(x,y,"You should see a smooth transition of colors",DC->defcolor);
		y += 16;
		SV_writeText(x,y,"If the colors are broken into 4 lots, the wide DAC is not working",DC->defcolor);
		y += 32;
		}

	SV_beginLine();
	for (i = 0; i < 192; i++) {
		SV_lineFast(x+i, y,    x+i, y+32,  64+i/3);
		SV_lineFast(x+i, y+32, x+i, y+64,  128+i/3);
		SV_lineFast(x+i, y+64, x+i, y+96,  192+i/3);
		}
	SV_endLine();

	key = GetChar();
	VBE_setDACWidth(6);
	return key;
}

void fadePalette(SV_palette *pal,SV_palette *fullIntensity,int numColors,
	int startIndex,uchar intensity)
/****************************************************************************
*
* Function:		fadePalette
* Parameters:	pal				- Palette to fade
*				fullIntensity	- Palette of full intensity values
*               numColors		- Number of colors to fade
*               startIndex		- Starting index in palette
*				intensity		- Intensity value for entries (0-255)
*
* Description:  Fades each of the palette values in the palette by the
*				specified intensity value. The values to fade from are
*				contained in the 'fullItensity' array, which should be at
*				least numColors in size.
*
****************************************************************************/
{
	uchar	*p,*fi;
	int		i;

	p = (uchar*)&pal[startIndex];
	fi = (uchar*)fullIntensity;
	for (i = 0; i < numColors; i++) {
		*p++ = (*fi++ * intensity) / (uchar)255;
		*p++ = (*fi++ * intensity) / (uchar)255;
		*p++ = (*fi++ * intensity) / (uchar)255;
		p++; fi++;
		}
}

int paletteTest(int maxProgram)
/****************************************************************************
*
* Function:		paletteTest
*
* Description:	Performs a palette programming test by displaying all the
*				colors in the palette and then quickly fading the values
*				out then in again.
*
****************************************************************************/
{
	int			i,key;
	SV_palette	pal[256],tmp[256];

	SV_clear(0);
	moire(63);
	if (DC->maxx > 360) {
		x = 80;	y = 80;
		}
	else {
		x = 40;	y = 40;
		}

	SV_writeText(x,y,"Palette programming test",63);
	y += 32;
	SV_writeText(x,y,"Hit a key to fade palette",63);

	memset(pal,0,256*sizeof(SV_palette));
	for (i = 0; i < 64; i++) {
		pal[i].red = pal[i].green = pal[i].blue = i*4;
		pal[64 + i].red = i*4;
		pal[128 + i].green = i*4;
		pal[192 + i].blue = i*4;
		}

	SV_setPalette(0,256,pal,-1);
	GetChar();

	/* Palette fade out */
	for (i = 63; i >= 0; i--) {
		fadePalette(tmp,pal,256,0,i*4);
		SV_setPalette(0,256,tmp,maxProgram);
		}

	/* Palette fade in */
	for (i = 0; i <= 63; i++) {
		fadePalette(tmp,pal,256,0,i*4);
		SV_setPalette(0,256,tmp,maxProgram);
		}
	key = GetChar();
	SV_setPalette(0,256,SV_getDefPalette(),-1);
	return key;
}

#ifndef	__16BIT__
void	doAFTests(SV_devCtx *DC);
#endif

bool doTest(ushort mode,bool doPalette,bool doVirtual,
	bool doRetrace,int maxProgram,bool use8BitDAC,bool useVirtualBuffer)
{
	SV_modeInfo	mi;

	if (!SV_getModeInfo(mode,&mi))
		return false;
	if (!SV_setMode(mode | svMultiBuffer,false,useVirtualBuffer,mi.NumberOfPages))
		return false;
	else {
		cntMode = mode;
		if (moireTest() == 0x1B)
			goto DoneTests;
		if (pageFlipTest(doRetrace) == 0x1B)
			goto DoneTests;
		if (doPalette && DC->maxcolor == 255) {
			if (paletteTest(maxProgram) == 0x1B)
				goto DoneTests;
			if (use8BitDAC) {
				if (wideDACTest() == 0x1B)
					goto DoneTests;
				}
			}
		if (doVirtual) {
			if (DC->AFDC) {
				if (virtualTestAF() == 0x1B)
					goto DoneTests;
				}
			else {
				virtualTestVBE(useVirtualBuffer);
				}
			}
#ifndef	__16BIT__
		if (mi.Attributes & svHaveAccel2D)
			doAFTests(DC);
#endif
DoneTests:
		SV_restoreMode();
		}
	return true;
}
