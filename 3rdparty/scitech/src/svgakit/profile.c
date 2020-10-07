/****************************************************************************
*
*			The SuperVGA Kit - UniVBE Software Development Kit
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Filename:     $Workfile:   profile.c  $
* Version:      $Revision:   1.4  $
*
* Language:     ANSI C
* Environment:  IBM PC (MSDOS) Real Mode and 16/32 bit Protected Mode.
*
* Description:  Simple program to profile the speed of certain operations
*				for the UVBELib library. This is great way to test the
*				performance of different SuperVGA card and different
*				compiler configurations.
*
*				Note, this library uses the Zen Timer Library for
*				microsecond accuracy timing of the routines.
*
* $Date:   04 May 1996 17:29:52  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include "getopt.h"
#include "svga.h"
#include "pmode.h"
#include "ztimer.h"

/*----------------------------- Implementation ----------------------------*/

SV_devCtx	*DC;
bool		linearBuffer;
float		lineTime;
long		numLines;
float		linesPerSec;
float		clearTime;
long		numClears;
float		clearsPerSec;
float		clearsMbPerSec;
float		clearSysMbPerSec;
float		bitBltTime;
long		numBitBlts;
float		bitBltsPerSec;
float		bitBltsMbPerSec;
float		copySysMbPerSec;
bool		thrashCache = false;
bool		useVBEAF = false;
bool		useVirtualBuffer = false;
char		logfilename[80] = "nothrash.log";

#include "version.c"

/* External assembly language routine to perform a full screen Blt from
 * system memory, using fast REP MOVSD strings instructions for entire
 * 64k banks - about as optimal as you will ever get for a full screen
 * Blt.
 */

void _cdecl bltImage(void *videoMem,char *image,int numBanks,int lastBytes);
void _cdecl bltImageLin(void *videoMem,char *image,int numBytes);
void _cdecl clearSysBuf(void *buffer,long value,uint size);
void _cdecl copySysBuf(void *buffer,char *image,uint size);

void _cdecl VBE_fatalError(char *msg)
{
	fprintf(stderr,"%s\n", msg);
	exit(1);
}

int _random(int max)
{
	return (rand() % (max+1));
}

#define	MAXLINES	1000

int x[MAXLINES];
int y[MAXLINES];

void profileLines(void)
/****************************************************************************
*
* Function:		profileLines
*
* Description:	Test the speed of line drawing in the specified video
*				mode. We blast out a whole bunch of random lines as fast
*				as possible.
*
****************************************************************************/
{
	int		i,j,max;
	long	step,color;

	switch (DC->maxcolor) {
		case 0xF:		max = 20;	break;
		case 0xFF:
		case 0x7FFF:
		case 0xFFFF:	max = 30;	break;
		default:		max = 20;
		}

#ifdef  QUICK_PROFILE
    max = 1;
#endif

	srand(1000);
    for (i = 0; i < MAXLINES; i++) {
		x[i] = _random(DC->maxx);
		y[i] = _random(DC->maxy);
		}

	SV_clear(0);
	step = (DC->maxcolor <= 0xFF) ? 1 : DC->maxcolor / max;
	SV_beginLine();
	LZTimerOn();
	for (j = 0, color = 1; j < max; j++, color += step) {
		for (i = 0; i < MAXLINES-1; i++)
			SV_lineFast(x[i],y[i],x[i+1],y[i+1],color);
		}
	LZTimerOff();
	SV_endLine();
	lineTime = LZTimerCount() * LZTIMER_RES;
	numLines = (long)MAXLINES * max;
	linesPerSec = numLines / lineTime;
}

void profileClears(void)
/****************************************************************************
*
* Function:		profileClears
*
* Description:	Test the speed of screen clearing to a specific color.
*
****************************************************************************/
{
	int		i,max;
	long	step,color;

	switch (DC->maxcolor) {
		case 0xF:		max = 1000;	break;
		case 0xFF:		max = 300;	break;
		case 0x7FFF:
		case 0xFFFF:	max = 150;	break;
		case 0xFFFFFF:	max = 30;	break;
		default:		max = 50;	break;
		}

	step = (DC->maxcolor <= 0xFF) ? 1 : DC->maxcolor / max;
	LZTimerOn();
	for (i = 0, color = 0; i < max; i++, color += step)
		SV_clear(color);
	LZTimerOff();
	clearTime = LZTimerCount() * LZTIMER_RES;
	numClears = max;
	clearsPerSec = numClears / clearTime;
	clearsMbPerSec = (clearsPerSec * DC->bytesperline * (DC->maxy+1)) / 1048576.0;
	if (DC->maxcolor == 0xF)
		clearsMbPerSec /= 2;
}

void profileBitBlt(void)
/****************************************************************************
*
* Function:		profileBitBlt
*
* Description:	Test the speed of blitting full size image from system RAM
*				to video RAM.
*
*				NOTE: The bitBlt'ing routine used blt's and entire display
*					  memory frame at a time, which is as optimal as you
*					  can get. Thus the results of this profiling test will
*					  give you a good idea of what you can expect as the
*					  absolute best case in real world performance.
*
*				NOTE: In order to thrash the system RAM cache, so that we
*					  can determine the RAW blitting performance we
*					  allocate a number of system memory buffers and cycle
*					  through each one (only in 32 bit PM version however)
*
****************************************************************************/
{
	int		i,numBanks,max,maxImages;
	uint	lastBytes;
	ulong	imageSize;
	char	*image[10],*dst;

	switch (DC->maxcolor) {
		case 0xFF:      max = 150;  break;
		case 0x7FFF:
		case 0xFFFF:    max = 75;  break;
		default:        max = 30;
		}

	imageSize = (long)DC->bytesperline * (DC->maxy+1);
#ifndef	PM386
	if (imageSize > 0x10000) {
		bitBltTime = -1;
		return;
		}
	maxImages = 1;
#else
	if (thrashCache)
		maxImages = ((512 * 1024U) / imageSize) + 1;
	else
		maxImages = 1;
#endif

	numBanks = imageSize / 0x10000;
	lastBytes = imageSize % 0x10000;
	for (i = 0; i < maxImages; i++) {
		image[i] = malloc(imageSize);
		if (image[i] == NULL) {
			bitBltTime = -1;
			return;
			}
		}

	/* Copy the current image from the frame buffer into our system memory
	 * buffer (which will still contain an image from the profileLines()
	 * routine).
	 */
	dst = image[0];
	if (linearBuffer) {
		memcpy(dst,DC->videoMem,imageSize);		/* Blt all at once */
		}
	else {
		for (i = 0; i < numBanks; i++) {		/* Blt all full memory banks */
			SV_setBank(i);
			memcpy(dst,DC->videoMem,0x10000);
			dst += 0x10000;
			}
		if (lastBytes) {
			SV_setBank(i);
			memcpy(dst,DC->videoMem,lastBytes);	/* Blt the last partial bank */
			}
		}

	for (i = 1; i < maxImages; i++)
		memcpy(image[i], image[0], imageSize);

	/* Now blt the image from system RAM back to the video frame buffer */
	SV_clear(0);
	LZTimerOn();
	if (linearBuffer) {
		for (i = 0; i < max; i++)
			bltImageLin(DC->videoMem,image[i % maxImages],imageSize);
		}
	else {
		for (i = 0; i < max; i++)
			bltImage(DC->videoMem,image[i % maxImages],numBanks,lastBytes);
		}
	LZTimerOff();
	bitBltTime = LZTimerCount() * LZTIMER_RES;
	numBitBlts = max;
	bitBltsPerSec = numBitBlts / bitBltTime;
	bitBltsMbPerSec = (bitBltsPerSec * DC->bytesperline * (DC->maxy+1)) / 1048576.0;
	if (DC->maxcolor == 0xF)
		bitBltsMbPerSec /= 2;

	for (i = 0; i < maxImages; i++)
		free(image[i]);
}

void profileBaseLine(void)
/****************************************************************************
*
* Function:		profileBaseLine
*
* Description:	Finds the baseline values for clearing and moving system
*				memory buffers for comparison purposes.
*
****************************************************************************/
{
	int		i;
	float   clearSysTime,copySysTime;
	void	*buffer;
	char	*image;
	uint	size;
	int		max;

#ifdef  PM386
	if (thrashCache) {
		size = 512 * 1024U;		/* Large memory buffer to thrash cache */
		max = 10;
		}
	else {
		size = 64000U;
		max = 80;
		}
#else
	size = 64000U;
	max = 80;
#endif
	buffer = malloc(size);
	image = malloc(size);

	if (!buffer || !image) {
		clearSysMbPerSec = 0;
		copySysMbPerSec = 0;
		goto QuickExit;
		}

	LZTimerOn();
	for (i = 0; i < max; i++)
		clearSysBuf(buffer,i,size);
	LZTimerOff();
	clearSysTime = LZTimerCount() * LZTIMER_RES;
	clearSysMbPerSec = ((float)max * size) / (1048576.0 * clearSysTime);

	LZTimerOn();
	for (i = 0; i < max; i++)
		copySysBuf(buffer,image,size);
	LZTimerOff();
	copySysTime = LZTimerCount() * LZTIMER_RES;
	copySysMbPerSec = ((float)max * size) / (1048576.0 * copySysTime);

QuickExit:
	free(buffer);
	free(image);
}

void help(void)
{
	VBE_modeInfo	mi;
	ushort			*p;

	printf("Profile - UniVBE Performance Profiling Program\n");
	printf("          Release %s.%s (%s)\n\n",
		release_major,release_minor,release_date);
	printf("%s\n", copyright_str);
	printf("\n");
	printf("Options are:\n");
	printf("    -b       - Use virtual linear framebuffer\n");
	printf("    -t       - Thrash the system memory cache during BitBlt's (32 bit only)\n\n");
	printf("Usage: profile [-bt] <mode> [video card name [logfile]]\n\n");
	printf("Press a key for list of video modes.");
	getch();
	printf("\n\nAvailable modes are (add 4000 for Linear Framebuffer version):\n");
	for (p = DC->modeList; *p != 0xFFFF; p++) {
		VBE_getModeInfo(*p, &mi);
		if (mi.XResolution == 0)
			continue;
		printf("    %03X - %4d x %4d %2d bits per pixel\n",
			*p, mi.XResolution, mi.YResolution, mi.BitsPerPixel);
		}
	exit(1);
}

void parseArguments(int argc,char *argv[])
/****************************************************************************
*
* Function:     parseArguments
* Parameters:   argc    - Number of command line arguments
*               argv    - Array of command line arguments
*
* Description:  Parses the command line and forces detection of specific
*               SuperVGA's if specified.
*
****************************************************************************/
{
	int     option;
	char    *argument;

	/* Parse command line options */

	do {
		option = getopt(argc,argv,"bt",&argument);
		switch (option) {
			case 'b':
				useVirtualBuffer = true;
				break;
			case 't':
#ifdef	PM386
				thrashCache = true;
				strcpy(logfilename, "thrash.log");
#endif
				break;
			case ALLDONE:
				break;
			case PARAMETER:
				break;
			case 'h':
			case INVALID:
			default:
				help();
			}
		} while (option != ALLDONE && option != PARAMETER);
}

int main(int argc, char *argv[])
{
	int		mode;
	ushort	*p;
	char	systemName[80];

	if (SV_queryCpu() < SV_cpu386) {
		printf("This program contains '386 specific instructions, and will not work on\n");
        printf("this machine - sorry\n");
        }

	DC = SV_init(useVBEAF);
	if (!DC || DC->VBEVersion < 0x102) {
		printf("This program requires a VESA VBE 1.2 or higher compatible SuperVGA. Try\n");
		printf("installing the Universal VESA VBE for your video card, or contact your\n");
		printf("video card vendor and ask for a suitable TSR\n");
		exit(1);
		}

	parseArguments(argc,argv);
	argc -= (nextargv-1);
	if (argc < 2 || argc > 4)
		help();
	sscanf(argv[nextargv], "%X", &mode);
	if (argc >= 3)
		strcpy(systemName,argv[nextargv+1]);
	if (argc == 4)
		strcpy(logfilename,argv[nextargv+2]);
	ZTimerInit();

    if (mode != 0x13) {
		for (p = DC->modeList; *p != 0xFFFF; p++) {
            if (*p == (mode & ~vbeLinearBuffer))
                break;
            }
        if (*p == 0xFFFF) {
            printf("Mode not available - ignoring\n");
			exit(1);
            }
        }

	if (SV_setMode(mode,false,useVirtualBuffer,0)) {
		linearBuffer = (mode & vbeLinearBuffer) || DC->virtualBuffer;
		if (stricmp(systemName,"baseline") == 0)
			profileBaseLine();
		else {
			profileBaseLine();
			profileLines();
			if (DC->maxcolor > 0xF)
				profileBitBlt();
			profileClears();
			}
		SV_restoreMode();

		if (argc == 2) {
			printf("Profiling results for mode %04Xh, %dx%d %ld color.\n",
				mode,DC->maxx+1,DC->maxy+1,DC->maxcolor+1);
			printf("Running in %s with %s framebuffer\n",
#ifdef	PM386
                "32 bit protected mode",
#elif	defined(PM286)
                "16 bit protected mode",
#else
                "16 bit real mode",
#endif
				DC->virtualBuffer ? "*virtual* linear" : linearBuffer ? "linear" : "banked");

            printf("\n");
			printf("%7.4fs for %8ld lines   => %10.2f lines/s\n", lineTime, numLines, linesPerSec);
			printf("%7.4fs for %8ld clears  => %10.2f clears/s, %7.2f Mb/s\n", clearTime, numClears, clearsPerSec, clearsMbPerSec);
			if (DC->maxcolor > 0xF && bitBltTime != -1)
				printf("%7.4fs for %8ld bitBlts => %10.2f bitBlt/s, %7.2f Mb/s\n", bitBltTime, numBitBlts, bitBltsPerSec, bitBltsMbPerSec);
			if (clearSysMbPerSec != 0.0) {
				printf("\nBaseline values:\n\n");
				printf("REP STOSD in system memory: %7.2f Mb/s\n", clearSysMbPerSec);
				printf("REP MOVSD in system memory: %7.2f Mb/s\n", copySysMbPerSec);
				}
#ifdef	PM386
			if (thrashCache)
				printf("\nCache thrashing active.\n");
			else
#endif
				printf("\nNo cache thrashing.\n");
			}
		else {
			if (stricmp(systemName,"baseline") == 0) {
				FILE *log = fopen(logfilename, "wt");
                fprintf(log,"+---------------------------------+-------+-----------------+-----------------+\n");
                fprintf(log,"| Video card name                 |  mode | clears/s (Mb/s) | bitBlt/s (Mb/s) |\n");
                fprintf(log,"+---------------------------------+-------+-----------------+-----------------+\n");
                fprintf(log,"| Baseline/system RAM             |   N/A |     N/A (%5.2f) |     N/A (%5.2f) |\n",
                    clearSysMbPerSec, copySysMbPerSec);
				fclose(log);
				}
			else {
				FILE *log = fopen(logfilename, "at");
                fprintf(log,"| %-31s | %4Xh | %7.2f (%5.2f) | %7.2f (%5.2f) |\n",
					systemName, mode, clearsPerSec, clearsMbPerSec, bitBltsPerSec,
					bitBltsMbPerSec);
				fclose(log);
				}
			}
		}
	else printf("Could not set specified video mode\n");
	return 0;
}
