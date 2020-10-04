/****************************************************************************
*
*			  AVIDirect - WinDirect SuperVGA video playback module
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Filename:     $Workfile:   avidirec.h  $
* Version:      $Revision:   1.0  $
*
* Language:     ANSI C
* Environment:  Win16/Win32 running on Windows 3.1 or Windows '95
*
* Description:  This is a simple, high performance module for playing back
*				8,15,16 and 24 bit DIB bitmaps from system memory directly
*				to fullscreen modes using WinDirect. This module supports
*				playback in any 8,15,16 and 24 bits per pixel video modes,
*				and will optionally 1x2 zoomed for playing back 320x200 or
*				320x240 images into a 320x400 or 320x480 video mode, or 2x2
*				zoomed for playing back 320x200 or 320x240 images into a
*				640x400 or 640x480 video mode (for performance 320x200 or
*				320x240 modes should be used if available). Note that 24 bit
*				DIB's can be played back in any 15,16 or 24 bit video mode,
*				while 8,15 and 16 bit native DIB's can only be played back
*				in the corresponding native mode. Hence if your video playback
*				modules can only decompress to 24 bit then you can still play
*				back fast in 15 or 16 bit video modes. However if you can
*				decompress to real 8,15 or 16 bit DIB's you can blt these
*				directly to the display for maximum performance.  
*
*				This module also includes a custom AVI draw handler for
*				rendering directly to WinDirect video modes, which can be
*				use directly with normal MCI AVI video playback routines.
*				The custom draw handler will try and match up the best
*				DIB format for fastest playback on the system. If the
*				compression codec can decompress directly to 15 or 16 bit
*				DIB's then the draw handler will use this, otherwise it
*				will blt from decompressed 24 bit DIB's to the display
*				mode.
*
*				NOTE: The bitmaps to be displayed must be in 24 bit in the
*					  bottom up format (ie: standard Windows DIB format).
*
*				NOTE: Because this library uses hardware linear and virtual
*					  linear framebuffer code, it *requires* that VBE 2.0 or
*					  higher be installed.
*
* $Date:   20 Feb 1996 19:32:52  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef __AVIDIREC_H
#define __AVIDIREC_H

#ifndef __WINDOWS_H
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifndef	__DEBUG_H
#include "debug.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

/* Flags to pass to AVIDirectRecommendMode(). Note that the DIB size is
 * really only specifying the aspect ratio of the DIB. AVIDirect will
 * properly handle DIB sized less than this in size but the output will
 * be centered in the middle of the display.
 */

#define	AVIDIRECT_DIB_320x200		0x0001
#define	AVIDIRECT_DIB_320x240		0x0002
#define	AVIDIRECT_DIB				0x000F
#define	AVIDIRECT_IGNORE_24BIT		0x0100
#define	AVIDIRECT_FORCE_MEDRES		0x0200
#define	AVIDIRECT_FORCE_HIRES		0x0400
#define	AVIDIRECT_IGNORE			0x0F00

/* Supported 8 bit SuperVGA modes */

#define AVIDIRECT_MODE_320x200x8 	0x0000
#define AVIDIRECT_MODE_320x240x8	0x0001
#define AVIDIRECT_MODE_320x400x8 	0x0002
#define AVIDIRECT_MODE_320x480x8	0x0003
#define	AVIDIRECT_MODE_512x384x8	0x0004
#define AVIDIRECT_MODE_640x400x8	0x0005
#define AVIDIRECT_MODE_640x480x8	0x0006

/* Supported 15 bit SuperVGA modes */

#define AVIDIRECT_MODE_320x200x15 	0x0007
#define AVIDIRECT_MODE_320x240x15	0x0008
#define AVIDIRECT_MODE_320x400x15 	0x0009
#define AVIDIRECT_MODE_320x480x15	0x000A
#define	AVIDIRECT_MODE_512x384x15	0x000B
#define AVIDIRECT_MODE_640x400x15	0x000C
#define AVIDIRECT_MODE_640x480x15	0x000D

/* Supported 16 bit SuperVGA modes */

#define AVIDIRECT_MODE_320x200x16 	0x000E
#define AVIDIRECT_MODE_320x240x16	0x000F
#define AVIDIRECT_MODE_320x400x16 	0x0010
#define AVIDIRECT_MODE_320x480x16	0x0011
#define	AVIDIRECT_MODE_512x384x16	0x0012
#define AVIDIRECT_MODE_640x400x16	0x0013
#define AVIDIRECT_MODE_640x480x16	0x0014

/* Supported 24 bit SuperVGA modes */

#define AVIDIRECT_MODE_320x200x24 	0x0015
#define AVIDIRECT_MODE_320x240x24	0x0016
#define AVIDIRECT_MODE_320x400x24 	0x0017
#define AVIDIRECT_MODE_320x480x24	0x0018
#define	AVIDIRECT_MODE_512x384x24	0x0019
#define AVIDIRECT_MODE_640x400x24	0x001A
#define AVIDIRECT_MODE_640x480x24	0x001B
#define	AVIDIRECT_MODE_MAX			0x001C

/* Flags returned when recommended mode will be stretched */

#define	AVIDIRECT_MODE_NOZOOM		0x0000	
#define	AVIDIRECT_MODE_ZOOM12		0x1000	/* 1x2 stretch			*/
#define	AVIDIRECT_MODE_ZOOM22		0x2000	/* 2x2 stretch			*/
#define	AVIDIRECT_ZOOM_FLAGS		0x7000	/* Mask for zoom flags	*/

/* Type definition for HAVIBLK, which is an internal structure */

typedef	struct AVIBLK	*HAVIBLK;				

/*------------------------- Function Prototypes ---------------------------*/

#ifdef  __cplusplus
extern "C" {            /* Use "C" linkage when in C++ mode */
#endif

HAVIBLK	_DLLAPI AVIDirectInit(void);
void	_DLLAPI AVIDirectExit(HAVIBLK aviBlk);
short 	_DLLAPI AVIDirectRecommendMode(HAVIBLK aviBlk,short dibSize,short dibBits);
bool	_DLLAPI AVIDirectGetModeInfo(HAVIBLK aviBlk,short mode,int *xRes,int *yRes,int *bits);
bool	_DLLAPI AVIDirectBegin(HAVIBLK aviBlk,short mode,short dibBits,bool hwPageFlip);
void *	_DLLAPI AVIDirectGetSurfacePtr(HAVIBLK aviBlk);
bool	_DLLAPI AVIDirectBlt(HAVIBLK aviBlk,LPBITMAPINFOHEADER lpbi,LPSTR lpbits,int zoomFlag);
void	_DLLAPI AVIDirectChangePalette(HAVIBLK aviBlk,int start,int count,LPRGBQUAD rgb);
void 	_DLLAPI AVIDirectSwapBuffers(HAVIBLK aviBlk,bool waitVRT);
void 	_DLLAPI AVIDirectEnd(HAVIBLK aviBlk);

/* Video for Windows custom draw handler for fullscreen AVI file playback */

LONG 	_DLLAPI AVIDirectDrawProc(DWORD id,HDRVR hDriver,UINT uiMessage,LPARAM lParam1,LPARAM lParam2);

/* Pop up the configuration dialog box */

void	_DLLAPI AVIDirectConfigure(HWND hWndParent);

#ifdef  __cplusplus
}                       /* End of "C" linkage for C++   */
#endif

#endif	/* __AVIDIREC_H */

