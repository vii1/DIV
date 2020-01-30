/****************************************************************************
*
*           The SuperVGA Kit - UniVBE Software Development Kit
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
* Environment:  IBM PC (MSDOS) Real Mode and 16/32 bit Protected Mode.
*
* Description:  Simple module to test the operation of the SuperVGA
*               bank switching code and page flipping code for the
*               all supported video modes.
*
*               MUST be compiled in the large or flat models.
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
#include "svga.h"
#include "ztimer.h"
#include "pmode.h"
#include "svgakit/tests.h"

/*---------------------------- Global Variables ---------------------------*/

PUBLIC  int         cntMode,cntBuffers;
PRIVATE int         x,y;
extern  SV_devCtx   *DC;

/*----------------------------- Implementation ----------------------------*/

void us_delay(long us)
/****************************************************************************
*
* Function:     us_delay
* Parameters:   us  - Number of microseconds to delay for
*
* Description:  Delays for the specified number of microseconds. We simply
*               use the Zen Timer routines to do this for us, since the
*               delay() function is not normally supported across all
*               compilers.
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

void gmoveto(int _x,int _y)
/****************************************************************************
*
* Function:     gmoveto
* Parameters:   x,y	- Location to move text cursor to
*
* Description:  Moves the current text location to the specified position.
*
****************************************************************************/
{
	x = _x;
	y = _y;
}

void gnewline(void)
{ y += 16; }

int ggetx(void)
{ return x; }

int ggety(void)
{ return y; }

int gprintf(char *fmt, ... )
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
	y += 16;
	va_end(argptr);

    return cnt;                     /* Return the conversion count      */
}

void displaymodeInfo(ushort mode)
/****************************************************************************
*
* Function:     displaymodeInfo
*
* Description:  Display the information about the video mode.
*
****************************************************************************/
{
	gprintf("Video mode: %d x %d %d bit %s (%04hX)",DC->maxx+1,DC->maxy+1,
		DC->bitsperpixel, (cntMode & vbeLinearBuffer) ? "Linear" : "Banked",
		mode);
	if (DC->virtualBuffer) {
		gprintf("Using *virtual* linear frame buffer");
		}
}

int moireTest(ushort mode)
/****************************************************************************
*
* Function:     moireTest
*
* Description:  Draws a simple Moire pattern on the display screen using
*               lines, and waits for a key press.
*
****************************************************************************/
{
	moire(DC->defcolor);
	if (DC->maxx > 512) {
		gmoveto(80,80);
		gprintf("Bank switching test");  y += 16;
		displaymodeInfo(mode);
		gprintf("Maximum x: %d, Maximum y: %d, BytesPerLine %ld, Pages: %d",
            DC->maxx,DC->maxy,DC->bytesperline,DC->maxpage+1);
		y += 16;
		gprintf("You should see a colorful Moire pattern on the screen");
		}
	else {
		gmoveto(8,40);
		displaymodeInfo(mode);
        }
	gprintf("Press any key to continue");
	y += 16;
	return GetChar();
}

int pageFlipTest(int waitVRT)
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
	ibool    done = false;
	ulong   color,lastCount = 0,newCount;
	char    buf[80],buf2[80];

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
		while (!done) {
			SV_setActivePage(apage);
			SV_clear(0);
			gmoveto(4,4);
			gprintf("%3d.%d fps", fpsRate / 10, fpsRate % 10);
			sprintf(buf,"%d x %d %d bit %s",DC->maxx+1,DC->maxy+1,
				DC->bitsperpixel,
				(cntMode & vbeLinearBuffer) ? "Linear" : "Banked");
			switch (waitVRT) {
				case svTripleBuffer:
					strcpy(buf2,"Triple buffering - should be no flicker");
					break;
				case svWaitVRT:
					strcpy(buf2,"Double buffering - should be no flicker");
					break;
				default:
					strcpy(buf2,"Page flipping (no wait) - may flicker");
					break;
				}

			if (DC->maxx <= 360) {
				SV_writeText(8,60,buf,DC->defcolor);
				SV_writeText(8,80,buf2,DC->defcolor);
				sprintf(buf,"Page %d of %d", apage+1, DC->maxpage+1);
				SV_writeText(8,100,buf,DC->defcolor);
				}
			else {
				SV_writeText(80,60,buf,DC->defcolor);
				SV_writeText(80,80,buf2,DC->defcolor);
				sprintf(buf,"Page %d of %d", apage+1, DC->maxpage+1);
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
			if (KeyHit()) {
				key = GetChar();                /* Swallow keypress */
				if (key == 'v' || key == 'V') {
					waitVRT -= 1;
					if (DC->haveTripleBuffer) {
						if (waitVRT < svTripleBuffer)
							waitVRT = svDontWait;
						}
					else {
						if (waitVRT < svWaitVRT)
							waitVRT = svDontWait;
						}
					}
				else
					break;
				}
			}
		LZTimerOff();
		}
	SV_setActivePage(0);
	SV_setVisualPage(0,svDontWait);
	return key;
}

int stereoTest(int mode,ibool useVirtualBuffer,int waitVRT,SV_CRTCInfo *crtc)
/****************************************************************************
*
* Function:     stereoTest
*
* Description:  Animates a line on the display using page flipping if
*               page flipping is active.
*
****************************************************************************/
{
	int     	i,apage,vpage,buffer,key = 0,numBuffers;
	ibool    	done = false;
	char    	buf[80],buf2[80];
	SV_modeInfo	mi;

	SV_getModeInfo(mode,&mi);
	if (DC->maxpage != 0 && mi.Attributes & svHaveStereo) {
		/* If we have at least 4 pages, then we can do stereo with double
		 * buffering. If we have at least two pages then we can do just
		 * plain stereo without double buffering, otherwise we cant do
		 * stereo so we skip this test.
		 */
		if (mi.NumberOfPages >= 4)
			numBuffers = 2;
		else if (mi.NumberOfPages >= 2)
			numBuffers = 1;
		else
			return 0;
		if (crtc->RefreshRate != 0) {
			if (!SV_setModeExt(mode | svMultiBuffer | svRefreshCtrl | svStereo,false,useVirtualBuffer,numBuffers,crtc))
				return 0;
			}
		else {
			if (!SV_setMode(mode | svMultiBuffer | svStereo,false,useVirtualBuffer,numBuffers))
				return 0;
			}
		numBuffers = DC->maxpage+1;
		if (numBuffers > 1) {
			/* Setup for double buffering */
			vpage = 0;
			apage = 1;
			}
		else {
			/* Setup for single buffered stereo */
			vpage = 0;
			apage = 0;
			}
		SV_setActivePage(apage | svLeftBuffer);
		SV_setVisualPage(vpage,waitVRT);
		while (!done) {
			buffer = svLeftBuffer;
			for (i = 0; i < 2; i++) {
				SV_setActivePage(apage | buffer);
				SV_clear(0);
				moire(DC->defcolor);
				gmoveto(4,4);
				sprintf(buf,"%d x %d %d bit Hardware Stereo",DC->maxx+1,DC->maxy+1,
					DC->bitsperpixel);
				switch (waitVRT) {
					case svTripleBuffer:
						strcpy(buf2,"Triple buffering - should be no flicker");
						break;
					case svWaitVRT:
						strcpy(buf2,"Double buffering - should be no flicker");
						break;
					default:
						strcpy(buf2,"Page flipping (no wait) - may flicker");
						break;
					}

				if (DC->maxx <= 360) {
					SV_writeText(8,60,buf,DC->defcolor);
					SV_writeText(8,80,buf2,DC->defcolor);
					sprintf(buf,"Page %d of %d", apage+1,numBuffers);
					SV_writeText(8,100,buf,DC->defcolor);
					if (buffer == svLeftBuffer)
						SV_writeText(8,120,"LEFT",DC->defcolor);
					else
						SV_writeText(68,120,"RIGHT",DC->defcolor);
					}
				else {
					SV_writeText(80,60,buf,DC->defcolor);
					SV_writeText(80,80,buf2,DC->defcolor);
					sprintf(buf,"Page %d of %d", apage+1,numBuffers);
					SV_writeText(80,100,buf,DC->defcolor);
					if (buffer == svLeftBuffer)
						SV_writeText(80,120,"LEFT",DC->defcolor);
					else
						SV_writeText(120,120,"RIGHT",DC->defcolor);
					}
				buffer = svRightBuffer;
				}
			if (numBuffers > 1) {
				/* Flip the display page */
				vpage ^= 1;
				SV_setVisualPage(vpage,waitVRT);
				apage ^= 1;
				}
			else {
				/* We only have one page, however we have to call
				 * SV_setVisualPage in order to display the blue code
				 * sync signal over the top for software stereo sync.
				 */
				SV_setVisualPage(vpage,svDontWait);
				while (!KeyHit())
					;
				}

			/* Check if a key has been hit */
			if (KeyHit()) {
				key = GetChar();                /* Swallow keypress */
				if (key == 'v' || key == 'V') {
					waitVRT -= 1;
					if (DC->haveTripleBuffer) {
						if (waitVRT < svTripleBuffer)
							waitVRT = svDontWait;
						}
					else {
						if (waitVRT < svWaitVRT)
							waitVRT = svDontWait;
						}
					}
				else
					break;
				}
			}
		SV_stopStereo();
		}
	SV_setActivePage(0);
	SV_setVisualPage(0,svDontWait);
	return key;
}

void fadePalette(SV_palette *pal,SV_palette *fullIntensity,int numColors,
    int startIndex,uchar intensity)
/****************************************************************************
*
* Function:     fadePalette
* Parameters:   pal             - Palette to fade
*               fullIntensity   - Palette of full intensity values
*               numColors       - Number of colors to fade
*               startIndex      - Starting index in palette
*               intensity       - Intensity value for entries (0-255)
*
* Description:  Fades each of the palette values in the palette by the
*               specified intensity value. The values to fade from are
*               contained in the 'fullItensity' array, which should be at
*               least numColors in size.
*
****************************************************************************/
{
    uchar   *p,*fi;
    int     i;

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
* Function:     paletteTest
*
* Description:  Performs a palette programming test by displaying all the
*               colors in the palette and then quickly fading the values
*               out then in again.
*
****************************************************************************/
{
	int         i,key;
	SV_palette  pal[256],tmp[256];
	char    	buf[80];

	SV_clear(0);
	moire(63);
	if (DC->maxx > 360) {
		x = 80; y = 80;
		}
	else {
		x = 40; y = 40;
		}

	sprintf(buf,"%d x %d %d bit %s",DC->maxx+1,DC->maxy+1,
		DC->bitsperpixel,
		(cntMode & vbeLinearBuffer) ? "Linear" : "Banked");
	SV_writeText(x,y,buf,63);
	y += 16;
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
	key = GetChar();
    if (key == 0x1B || key == 'N' || key == 'n' || key == 'F' || key == 'f')
        goto ResetPalette;

    /* Palette fade out */
    for (i = 63; i >= 0; i--) {
		fadePalette(tmp,pal,256,0,i*4);
        SV_setPalette(0,256,tmp,maxProgram);
        if (KeyHit())
			goto DoneFade;
        }

    /* Palette fade in */
    for (i = 0; i <= 63; i++) {
        fadePalette(tmp,pal,256,0,i*4);
		SV_setPalette(0,256,tmp,maxProgram);
        if (KeyHit())
            goto DoneFade;
        }

DoneFade:
	key = GetChar();
ResetPalette:
	SV_setPalette(0,256,SV_getDefPalette(),-1);
	return key;
}

/* Internal VGA palette setting function in SuperVGA Kit source code */

void _ASMAPI _VGA_setPalette(int start,int num,SV_palette *pal,ibool waitVRT);

int wideDACTest(void)
/****************************************************************************
*
* Function:     wideDACTest
*
* Description:  Displays a set of color values using the wide DAC support
*               if available.
*
****************************************************************************/
{
	int         i,key;
	SV_palette  pal[256];
	char    	buf[80];

	if (!VBE_setDACWidth(8))
		return -1;

	memset(pal,0,256*sizeof(SV_palette));
	for (i = 0; i < 256; i += 4) {
		pal[64 + (i >> 2)].red = i;
		pal[128 + (i >> 2)].green = i;
		pal[192 + (i >> 2)].blue = i;
		}
	pal[(int)DC->defcolor].red = 255;
	pal[(int)DC->defcolor].green = 255;
	pal[(int)DC->defcolor].blue = 255;

	if (DC->isNonVGA)
		VBE_setPalette(0,256,(VBE_palette*)pal,false);
	else
		_VGA_setPalette(0,256,pal,false);

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

	sprintf(buf,"%d x %d %d bit %s",DC->maxx+1,DC->maxy+1,
		DC->bitsperpixel,
		(cntMode & vbeLinearBuffer) ? "Linear" : "Banked");
	SV_writeText(x,y,buf,DC->defcolor);
	y += 16;
	SV_writeText(x,y,"Wide DAC test",DC->defcolor);
	y += 32;
	if (DC->maxx >= 640) {
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
	SV_setPalette(0,256,SV_getDefPalette(),-1);
	VBE_setDACWidth(6);
	return key;
}

PRIVATE ibool SV_setBytesPerLine(int bytes)
/****************************************************************************
*
* Function:     SV_setBytesPerLine
* Parameters:   bytes   - New bytes per line value
* Returns:      True on success, false on failure.
*
* Description:  Sets the scanline length to a specified bytes per line
*               value. This function only works with VBE 2.0.
*
****************************************************************************/
{
	int newbytes,xres,yres;

    if (!VBE_setBytesPerLine(bytes,&newbytes,&xres,&yres))
        return false;
    DC->bytesperline = newbytes;
	DC->maxx = xres-1;
    DC->maxy = yres-1;
    return true;
}

PRIVATE ibool SV_setPixelsPerLine(int xMax)
/****************************************************************************
*
* Function:     SV_setPixelsPerLine
* Parameters:   xMax    - New pixels per line value
* Returns:      True on success, false on failure.
*
* Description:  Sets the scanline length to a specified pixels per line
*               value. This function only works with VBE 1.2 and above.
*
****************************************************************************/
{
    int newbytes,xres,yres;

	if (!VBE_setPixelsPerLine(xMax,&newbytes,&xres,&yres))
		return false;
	DC->bytesperline = newbytes;
	DC->maxx = xres-1;
	DC->maxy = yres-1;
	return true;
}

ibool CheckWaitVRT(int *key,ibool *waitVRT)
/****************************************************************************
*
* Function:     CheckWaitVRT
* Parameters:   key     - Place to return the key hit
*               waitVRT - Wait for retrace flag.
* Returns:      True if retrace flag toggled, false for other key.
*
****************************************************************************/
{
	*key = GetChar();
	if (*key == 'v' || *key == 'V') {
		*waitVRT = !*waitVRT;
		return true;
		}
	return false;
}

PRIVATE ibool setDisplayStart(int x,int y, ibool waitVRT)
{ return VBE_setDisplayStart(x,y,waitVRT); }

int virtualTestVBE(int mode,ibool useVirtualBuffer,SV_CRTCInfo *crtc,
	ibool (* setDisplayStart)(int x,int y, ibool waitVRT))
/****************************************************************************
*
* Function:     virtualTestVBE
* Parameters:   useVirtualBuffer    - True if virual linear buffer is used
*               setDisplayStart     - Function to change display start
*
* Description:  Checks the CRT logical scanline length routines, setting
*               up a virtual display buffer and scrolling around within
*               this buffer.
*
****************************************************************************/
{
	int     i,x,y,scrollx,scrolly,oldmaxx,oldmaxy,oldbytesperline,max,key;
	int		horzLimit,vertLimit;
	ibool    waitVRT = false;
	char    buf[80];

	if (!setDisplayStart(10,10,false))
		return -1;
	if (DC->maxx == 319 && DC->maxy == 199 && DC->maxpage == 0)
		return -1;
	if (crtc->RefreshRate != 0) {
		if (!SV_setModeExt(mode | svRefreshCtrl,false,useVirtualBuffer,1,
				crtc))
			return false;
		}
	else {
		if (!SV_setMode(mode,false,useVirtualBuffer,1))
			return false;
		}

	/* Set up for the widest possible virtual display buffer */
	oldmaxx = DC->maxx;
	oldmaxy = DC->maxy;
	oldbytesperline = DC->bytesperline;

	/* Find the largest value that we can set the virtual buffer width
	 * to that the VBE supports
	 */
	switch (DC->bitsperpixel) {
		case 4:     max = (int)((DC->memory*2048L) / (DC->maxy+1)); break;
		case 8:     max = (int)((DC->memory*1024L) / (DC->maxy+1)); break;
		case 15:
		case 16:    max = (int)((DC->memory*512L) / (DC->maxy+1));  break;
		case 24:    max = (int)((DC->memory*341L) / (DC->maxy+1));  break;
		case 32:    max = (int)((DC->memory*256L) / (DC->maxy+1));  break;
		}

	for (i = max; i > oldmaxx+1; i--) {
		if (!SV_setPixelsPerLine(i))
			continue;
		if (DC->maxx > oldmaxx+1 && DC->maxx < max)
			break;              /* Large value has been set         */
		}

	/* Perform huge horizontal scroll */

	setDisplayStart(0,0,false);
	SV_clear(0);
	moire(DC->defcolor);
	sprintf(buf,"%d x %d %d bit %s",oldmaxx+1,oldmaxy+1,
		DC->bitsperpixel,
		(cntMode & vbeLinearBuffer) ? "Linear" : "Banked");
	SV_writeText(20,20,buf,DC->defcolor);
	if (DC->maxx == oldmaxx) {
		sprintf(buf,"Buffer not resizeable (still %d x %d)",DC->maxx+1,DC->maxy+1);
		SV_writeText(20,40,buf,DC->defcolor);
		SV_writeText(20,60,"Press any key to begin scrolling",DC->defcolor);
		goto StartVerticalScroll;
		}
	else {
		sprintf(buf,"Buffer now set to %d x %d pixels",DC->maxx+1,DC->maxy+1);
		SV_writeText(20,40,buf,DC->defcolor);
		SV_writeText(20,60,"Press any key to begin scrolling",DC->defcolor);
		if ((key = GetChar()) != ' ' && (key != '\xD'))
			goto ResetMode;
		}

	scrollx = DC->maxx-oldmaxx;
	scrolly = DC->maxy-oldmaxy;
	horzLimit = (oldmaxx * 3) / 2;
	vertLimit = (oldmaxy * 3) / 2;
	for (x = y = 0; x <= scrollx; x++) {
		setDisplayStart(x,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (x < (scrollx - horzLimit))
				x = scrollx - horzLimit;
			else if (key == ' ' || key == '\xD') {
				setDisplayStart(scrollx,y,waitVRT);
				break;
				}
		}
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;
	for (x = scrollx,y = scrolly; x >= 0; x--) {
		setDisplayStart(x,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (x > horzLimit)
				x = horzLimit;
			else if (key == ' ' || key == '\xD') {
				setDisplayStart(0,y,waitVRT);
				break;
				}
		}
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;

	/* Now perform huge vertical scroll */

	if (DC->maxx == oldmaxx) goto ResetMode;
	setDisplayStart(0,0,false);
	if (DC->VBEVersion < 0x200)
		SV_setPixelsPerLine(oldmaxx+1);
	else
		SV_setBytesPerLine(oldbytesperline);
	DC->maxx = oldmaxx;
	SV_clear(0);
	moire(DC->defcolor);
	sprintf(buf,"%d x %d %d bit %s",oldmaxx+1,oldmaxy+1,
		DC->bitsperpixel,
		(cntMode & vbeLinearBuffer) ? "Linear" : "Banked");
	SV_writeText(20,20,buf,DC->defcolor);
	sprintf(buf,"Buffer now set to %d x %d pixels",DC->maxx+1,DC->maxy+1);
	SV_writeText(20,40,buf,DC->defcolor);
	SV_writeText(20,60,"Press any key to begin scrolling",DC->defcolor);
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;
StartVerticalScroll:
	scrolly = DC->maxy-oldmaxy;
	for (y = 0; y <= scrolly; y++) {
		setDisplayStart(0,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (y < (scrolly - vertLimit))
				y = scrolly - vertLimit;
			else if (key == ' ' || key == '\xD') {
				setDisplayStart(0,scrolly,waitVRT);
				break;
				}
		}
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;
	for (y = scrolly; y >= 0; y--) {
		setDisplayStart(0,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (y > vertLimit)
				y = vertLimit;
			else if (key == ' ' || key == '\xD') {
				setDisplayStart(0,0,waitVRT);
				break;
				}
		}
	key = GetChar();
ResetMode:
	SV_setMode(mode,false,useVirtualBuffer,1);
	return key;
}

#ifndef __16BIT__

PUBLIC int wideDACTestAF(AF_devCtx *dc)
/****************************************************************************
*
* Function:		wideDACTestAF
*
* Description:  Displays a set of color values using the wide DAC support
*				if available.
*
****************************************************************************/
{
	int					i,x,y,key;
	AF_palette			pal[256];
	char	buf[80];


	if (!(DC->AFDC->Attributes & afHave8BitDAC))
		return -1;

	memset(pal,0,256*sizeof(AF_palette));
	for (i = 0; i < 256; i += 4) {
		pal[64 + (i >> 2)].red = i;
		pal[128 + (i >> 2)].green = i;
		pal[192 + (i >> 2)].blue = i;
		}
	pal[(int)DC->defcolor].red = 255;
	pal[(int)DC->defcolor].green = 255;
	pal[(int)DC->defcolor].blue = 255;

	AF_setPaletteData(dc,pal,256,0,0);
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


	sprintf(buf,"%d x %d %d bit %s",DC->maxx+1,DC->maxy+1,
		DC->bitsperpixel,
		(cntMode & vbeLinearBuffer) ? "Linear" : "Banked");

	SV_writeText(x,y,buf,DC->defcolor);
	y += 16;
	SV_writeText(x,y,"Wide DAC test",DC->defcolor);
	y += 32;
	if (DC->maxx >= 640) {
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

	key = (char)GetChar();
	SV_setPalette(0,256,SV_getDefPalette(),-1);
	return key;
}

PUBLIC int virtualTestAF(ibool useVirtualBuffer,int mode)
/****************************************************************************
*
* Function:     virtualTestAF
*
* Description:  Virtual scrolling test for VBE/AF modes.                                     *
*
****************************************************************************/
{
	int     		x,y,scrollx,scrolly,oldmaxx,oldmaxy,oldbytesperline,key;
	int				i,max,horzLimit,vertLimit;
	AF_int32		virtualX,virtualY;
	ibool    		waitVRT = false;
	char    		buf[80];
	AF_modeInfo 	modeInfo;

	if (!(DC->AFDC->Attributes & afHaveVirtualScroll) )
		return -1;

	oldmaxx = DC->maxx;
	oldmaxy = DC->maxy;
	oldbytesperline = DC->bytesperline;

	if (AF_getVideoModeInfo(DC->AFDC, mode, &modeInfo) == -1)
		return -1;
	if ((modeInfo.MaxBytesPerScanLine == 0) || (modeInfo.MaxScanLineWidth == 0))
		return -1;

	/* Find the largest value that we can set the virtual buffer width
	 * to that the VBE/AF driver supports
	 */
	max = modeInfo.MaxBytesPerScanLine;
	for (i = max; i > oldmaxx+1; i--) {
		virtualY = ((DC->AFDC->TotalMemory*1024) / i);
		if (virtualY >= (oldmaxy+1))
			break;
		}
	virtualX = i/DC->bytesperpixel;
	if (SV_setVirtualMode(mode|afVirtualScroll, virtualX, virtualY, true,useVirtualBuffer, 1)==-1)
		return -1;

	if (virtualX <= DC->AFDC->BufferEndX)
		goto StartVerticalScroll;

	/* Perform huge horizontal scroll */

	AF_setDisplayStart(DC->AFDC,0,0,false);
	SV_clear(0);
	moire(DC->defcolor);
	sprintf(buf,"%d x %d %d bit %s",modeInfo.XResolution,modeInfo.YResolution,
		modeInfo.BitsPerPixel,
		(DC->AFDC->LinearSize) ? "Linear" : "Banked");
	SV_writeText(20,20,buf,DC->defcolor);
	sprintf(buf,"Buffer now set to %d x %d pixels",virtualX, virtualY);
	SV_writeText(20,40,buf,DC->defcolor);
	SV_writeText(20,60,"Press any key to begin scrolling",DC->defcolor);
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		 goto ResetMode;

	scrollx = virtualX-modeInfo.XResolution;
	scrolly = virtualY-modeInfo.YResolution;
	horzLimit = (modeInfo.XResolution * 3) / 2;
	vertLimit = (modeInfo.YResolution * 3) / 2;

	for (x = y = 0; x <= scrollx; x++) {
		AF_setDisplayStart(DC->AFDC,x,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (x < (scrollx - horzLimit))
				x = scrollx - horzLimit;
			else if (key == ' ' || key == '\xD') {
				AF_setDisplayStart(DC->AFDC,scrollx,y,waitVRT);
				break;
				}
		}
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;

	/* scroll down if y is greater than screen size */
	if (scrolly > 1) {
		for (y=0,x=scrollx; y <= scrolly; y++) {
			AF_setDisplayStart(DC->AFDC,x,y,waitVRT);
			us_delay(700);
			if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
				if (y < (scrolly - vertLimit))
					y = scrolly - vertLimit;
				else if (key == ' ' || key == '\xD') {
					AF_setDisplayStart(DC->AFDC,scrollx,scrolly,waitVRT);
					break;
			}
		}
	}

	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;

	for (x = scrollx,y = scrolly; x >= 0; x--) {
		AF_setDisplayStart(DC->AFDC,x,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (x > horzLimit)
				x = horzLimit;
			else if (key == ' ' || key == '\xD') {
				AF_setDisplayStart(DC->AFDC,0,y,waitVRT);
				break;
			}

	}

	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;

	/* scroll up if y is greater than screen size */
	if (scrolly > 1) {
		for (y = scrolly; y >= 0; y--) {
		AF_setDisplayStart(DC->AFDC,0,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (y > vertLimit)
				y = vertLimit;
			else if (key == ' ' || key == '\xD') {
				AF_setDisplayStart(DC->AFDC,0,0,waitVRT);
				break;
				}
		}
	}

	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;

	/* Now perform huge vertical scroll */

	virtualX = oldmaxx+1;
	virtualY = ( (DC->AFDC->TotalMemory*1024)/oldbytesperline);

	if (SV_setVirtualMode(mode|afVirtualScroll, virtualX, virtualY, true,useVirtualBuffer, 1)==-1)
		return -1;

	SV_clear(0);
	moire(DC->defcolor);
	sprintf(buf,"%d x %d %d bit %s",modeInfo.XResolution,modeInfo.YResolution,
		modeInfo.BitsPerPixel,
		(DC->AFDC->LinearSize) ? "Linear" : "Banked");
	SV_writeText(20,20,buf,DC->defcolor);
	sprintf(buf,"Buffer now set to %d x %d pixels",virtualX,virtualY);
	SV_writeText(20,40,buf,DC->defcolor);
	SV_writeText(20,60,"Press any key to begin scrolling",DC->defcolor);
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;
StartVerticalScroll:
	scrolly = virtualY-oldmaxy;
	for (y = 0; y <= scrolly; y++) {
		AF_setDisplayStart(DC->AFDC,0,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (y < (scrolly - vertLimit))
				y = scrolly - vertLimit;
			else if (key == ' ' || key == '\xD') {
				AF_setDisplayStart(DC->AFDC,0,scrolly,waitVRT);
				break;
				}
		}
	if ((key = GetChar()) != ' ' && (key != '\xD'))
		goto ResetMode;
	for (y = scrolly; y >= 0; y--) {
		AF_setDisplayStart(DC->AFDC,0,y,waitVRT);
		us_delay(700);
		if (KeyHit() && !CheckWaitVRT(&key,&waitVRT))
			if (y > vertLimit)
				y = vertLimit;
			else if (key == ' ' || key == '\xD') {
				AF_setDisplayStart(DC->AFDC,0,0,waitVRT);
				break;
				}
		}
	key = GetChar();
ResetMode:
	SV_setMode(mode,false,useVirtualBuffer,0);
	return key;
}

#endif

void    doAFTests(SV_devCtx *DC);

ibool doTest(ushort mode,ibool doPalette,ibool doVirtual,
	ibool doRetrace,int maxProgram,ibool use8BitDAC,ibool useVirtualBuffer,
	SV_CRTCInfo *crtc)
{
	SV_modeInfo mi;
	int			maxPages;
#ifdef GET_VGA_MODES
	RMREGS		regs;
#endif

	if (!SV_getModeInfo(mode,&mi))
		return false;

	/* Find the number of available image pages from the mode information
	 * block. This is set up to allow for different numbers of pages
	 * for both banked and linear modes which is required on some chips.
	 */
	if (DC->VBEVersion >= 0x300 || (DC->AFDC && DC->AFDC->Version >= 0x200)) {
		if (mode & svLinearBuffer)
			maxPages = mi.LinNumberOfPages;
		else
			maxPages = mi.BnkNumberOfPages;
		}
	else
		maxPages = mi.NumberOfPages;

	/* Do refresh rate controlled mode set if requested */
	if (crtc->RefreshRate != 0) {
		if (!SV_setModeExt(mode | svMultiBuffer | svRefreshCtrl,false,useVirtualBuffer,maxPages,
				crtc))
			return false;
		}
	else {
		if (!SV_setMode(mode | svMultiBuffer,false,useVirtualBuffer,maxPages))
			return false;
		}
#ifdef GET_VGA_MODES
	regs.h.ah = 0x0F;
	PM_int86(0x10,&regs,&regs);
#endif
	cntMode = mode;
	cntBuffers = mi.NumberOfPages;
	if (moireTest(mode) == 0x1B)
		goto DoneTests;
	if (pageFlipTest(doRetrace) == 0x1B)
		goto DoneTests;
	if (stereoTest(mode,useVirtualBuffer,doRetrace,crtc) == 0x1B)
		goto DoneTests;
	if (doPalette && DC->maxcolor == 255) {
		if (paletteTest(maxProgram) == 0x1B)
			goto DoneTests;
		if (use8BitDAC) {
#ifndef	__16BIT__
			if (DC->AFDC) {
				if (wideDACTestAF(DC->AFDC) == 0x1B)
					goto DoneTests;
				}
			else
#endif
				{
				if (wideDACTest() == 0x1B)
					goto DoneTests;
				}
			}
		}
	if (doVirtual) {
#ifndef	__16BIT__
		if (DC->AFDC) {
#if 0
// TODO: This is mapped out for the moment as it causes problems
			if (virtualTestAF(useVirtualBuffer,mode) == 0x1B)
				goto DoneTests;
#endif
			}
		else
#endif
			{
			if (virtualTestVBE(mode,useVirtualBuffer,crtc,setDisplayStart) == 0x1B)
				goto DoneTests;
			}
		}
#ifndef __16BIT__
	if (mi.Attributes & svHaveAccel2D)
		doAFTests(DC);
#endif
DoneTests:
	SV_restoreMode();
#ifdef  GET_VGA_MODES
	printf("VGA mode = 0x%02X\n", regs.h.al & 0x7F);
	getch();
#endif
	return true;
}
