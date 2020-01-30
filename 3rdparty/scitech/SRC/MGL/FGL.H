/****************************************************************************
*
*						MegaGraph Graphics Library
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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MGL fullscreen OpenGL driver interface.
*
*
****************************************************************************/

#ifndef	__FGL_H
#define	__FGL_H

#ifndef __SCITECH_H
#include "scitech.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)

#ifndef	__MGRAPH_H

/* Palette entry structure	*/

typedef struct {
	uchar	blue;				/* Blue component of color				*/
	uchar	green;				/* Green component of color				*/
	uchar	red;				/* Blue component of color				*/
	uchar	alpha;				/* Alpha or alignment byte				*/
	} palette_t;

#endif

typedef struct {
	int    	redSize;            /* Size of direct color red mask        */
	int    	redPos;             /* Bit posn of lsb of red mask          */
	int    	greenSize;          /* Size of direct color green mask      */
	int    	greenPos;           /* Bit posn of lsb of green mask        */
	int    	blueSize;           /* Size of direct color blue mask       */
	int    	bluePos;            /* Bit posn of lsb of blue mask         */
	int    	rsvdSize;           /* Size of direct color res mask        */
	int   	rsvdPos;            /* Bit posn of lsb of res mask          */
	} pixelFormat;

/* The following defines the hardware information returned by fglDetect */

typedef struct _FGL_hwInfo {
	char	driverName[13];			/* Name of driver (ie: 3DFX.DRV)	*/
	int		videoMemSize;			/* Amount of video memory			*/
	int		zbufferSize;			/* Amount of z-buffer memory		*/
	int		textureSize;			/* Amount of texture memory			*/
	} FGL_hwInfo;

/* The following defines the mode information returned by fglDetect */

typedef struct _FGL_modeInfo {
	int				xRes;			/* Device x resolution - 1			*/
	int				yRes;			/* Device y resolution - 1		  	*/
	int				bitsPerPixel;	/* Number of bits per pixel			*/
	int				bytesPerLine;	/* Number of bytes in a line		*/
    int				maxBuffers;		/* Number of buffers				*/
	pixelFormat		pf;				/* Pixel format information 		*/
	} FGL_modeInfo;

/* Values for passing to fglBeginDirectAccess */

typedef enum {
	FGL_FRONT_BUFFER,
	FGL_BACK_BUFFER,
	FGL_Z_BUFFER,
	FGL_TEXTURE_BUFFER,
    } FGL_directAccessType;

typedef enum {
	FGL_PRIMARY_DRIVER		= 0x1,
	FGL_SECONDARY_DRIVER	= 0x2,
	FGL_COMPLIANT_DRIVER	= 0x4,
	} FGL_hwFlagsType;

#pragma pack()

/*------------------------- Function Prototypes ---------------------------*/

#ifndef	BUILD_MGL

/* Fullscreen OpenGL driver bindings for secondary drivers with a
 * separate CRTC controller. This includes boards like the 3Dfx Voodoo.
 * If your driver is a secondary controller with an external CRTC, you
 * need to add the following Registry entries to the Windows registry to
 * register your driver with the MGL:
 *
 *	HKEY_LOCAL_MACHINE\SOFTWARE\SciTech Software\MGL\OpenGL Drivers\Secondary
 *		DriverName	 = "mygl.dll"	- String value
 *		FullDriver	 = 0 or 1		- DWORD value
 *		ExternalCRTC = 1			- DWORD value
 *
 * The DriverName value contains the name of your DLL, and the DLL should be
 * located somewhere on the path (ie: probably in the Windows system
 * directory).
 *
 * The FullDriver value is a boolean value that lets the MGL know
 * if your driver is a full OpenGL implementation or a partial implementation.
 * If your driver does not implement all of the OpenGL API functions, you
 * should pass back a value of 0 for this, so the MGL and developers know
 * that only a subset of OpenGL is available via your driver.
 *
 * The ExternalCRTC value is boolean value that lets the MGL know if your
 * driver has an external CRTC controller, and should be set to 1 for
 * boards like the 3Dfx Voodoo.
 */

ibool	APIENTRY fglDetect(int *numModes,FGL_hwInfo *hwInfo,FGL_modeInfo *modes);
ibool	APIENTRY fglSetVideoMode(int mode);
void	APIENTRY fglSetFocus(ibool active);
void	APIENTRY fglRestoreMode(void);
void * 	APIENTRY fglBeginDirectAccess(int buffer);
void	APIENTRY fglEndDirectAccess(void);
ibool  	APIENTRY fglSwapBuffers(ibool waitVRT);
void 	APIENTRY fglRealizePalette(palette_t *pal,int numColors,int startIndex,int waitVRT);

// TODO: Add support for optional functions like rectangle filling and blitting

/* Fullscreen OpenGL driver bindings for secondary drivers without a
 * separate CRTC controller. This includes boards like the NEC PowerVR
 * and SMOS PIX that do all drawing into the primary controllers hardware
 * linear framebuffer over the PCI or AGP bus. If your driver is a secondary
 * controller without an external CRTC, you need to add the following Registry
 * entries to the Windows registry to register your driver with the MGL:
 *
 *	HKEY_LOCAL_MACHINE\SOFTWARE\SciTech Software\MGL\OpenGL Drivers\Secondary
 *		DriverName	 = "mygl.dll"	- String value
 *		FullDriver	 = 0 or 1		- DWORD value
 *		ExternalCRTC = 0			- DWORD value
 *
 * The DriverName value contains the name of your DLL, and the DLL should be
 * located somewhere on the path (ie: probably in the Windows system
 * directory).
 *
 * The FullDriver value is a boolean value that lets the MGL know
 * if your driver is a full OpenGL implementation or a partial implementation.
 * If your driver does not implement all of the OpenGL API functions, you
 * should pass back a value of 0 for this, so the MGL and developers know
 * that only a subset of OpenGL is available via your driver.
 *
 * The ExternalCRTC value is boolean value that lets the MGL know if your
 * driver has an external CRTC controller, and should be set to 0 for
 * boards like the NEC PowerVR and SMOS PIX.
 */

ibool	APIENTRY fglDetectNoCRTC(void);
void	APIENTRY fglInformModeSet(FGL_modeInfo *mode);
void	APIENTRY fglSetActivePage(ulong physicalAddr);
void	APIENTRY fglRestoreMode(void);

/* The following 'wgl' functions are *required* by the fullscreen OpenGL
 * drivers.
 *
 * In the case of secondary drivers without an external CRTC controller
 * (like the NEC PowerVR), the MGL will use DirectDraw or WinDirect to
 * create the fullscreen graphics mode, and will then call the
 * wglChoose/Get/SetPixelFormat functions for an HDC created for the main
 * MGL window. The HDC is likely to return incorrect information about the
 * pixel format for the mode (especially in WinDirect modes), so it is
 * vital that your driver gets the mode information for the current graphic
 * modes via the fglInformModeSet function.
 *
 * In the case of secondary drivers with an extern CRTC controller (like
 * the 3Dfx Voodoo), the MGL will use DirectDraw the set a fullscreen mode
 * close (but identical if possible) to the desired resolution for the mode,
 * and will call the wglChoose/Get/SetPixelFormat functions for an HDC created
 * for the main MGL window. Once again the pixel format information for this
 * fullscreen window is likely to be incorrect since we may not have the
 * ability to run in the same reslution and color depth on the primary
 * controller, so the driver should use the information from the last
 * fglSetVideoMode call to determine the pixel format info. 
 */

HGLRC 	APIENTRY wglCreateContext(HDC);
BOOL  	APIENTRY wglDeleteContext(HGLRC);
PROC  	APIENTRY wglGetProcAddress(LPCSTR);
BOOL  	APIENTRY wglMakeCurrent(HDC, HGLRC);
BOOL  	APIENTRY wglUseFontBitmapsA(HDC, DWORD, DWORD, DWORD);
BOOL  	APIENTRY wglUseFontBitmapsW(HDC, DWORD, DWORD, DWORD);
BOOL  	APIENTRY wglUseFontOutlinesA(HDC, DWORD, DWORD, DWORD, FLOAT,FLOAT, int, LPGLYPHMETRICSFLOAT);
BOOL  	APIENTRY wglUseFontOutlinesW(HDC, DWORD, DWORD, DWORD, FLOAT,FLOAT, int, LPGLYPHMETRICSFLOAT);
int   	APIENTRY wglDescribePixelFormat(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
int   	APIENTRY wglChoosePixelFormat(HDC, CONST PIXELFORMATDESCRIPTOR *);
int   	APIENTRY wglGetPixelFormat(HDC);
BOOL  	APIENTRY wglSetPixelFormat(HDC, int, CONST PIXELFORMATDESCRIPTOR *);

/* The following 'wgl' functions are not used and not required in the driver */

HGLRC 	APIENTRY wglGetCurrentContext(VOID);
HDC   	APIENTRY wglGetCurrentDC(VOID);
BOOL  	APIENTRY wglShareLists(HGLRC, HGLRC);
BOOL  	APIENTRY wglSwapBuffers(HDC);

#endif

#endif	/* __FGL_H */

