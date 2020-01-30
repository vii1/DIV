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
* Environment:  MSDOS/Win32 full screen
*
* Description:  Game Framework for SciTech MGL. This library sprovides the
*				common functions that every game written with the MGL
*				will require, and automatically handles all of the 'polish'
*				functionality required of commercial games for the DOS
*				and Windows environments. This library provides support for
*				issues such as the following:
*
*				. Handling alt-tab'ing to GDI mode and back to fullscreen
*				. Static system color management in 8bpp modes
*				. Palette management for 8bpp modes windowed and fullscreen
*				. Switching between fullscreen and windowed modes on the fly
*				. Handling of system menu, maximise and minimise buttons 
*				. System buffering in a window or with one hardware page
*				. Hardware multi-buffering for smooth animation fullscreen
*				. 1x2 and 2x2 stretching for both fullscreen and a window
*				. Event handling for mouse and keyboard
*				. Application activation notification
*				. Dirty rectangle management for system buffering
*				. OpenGL rendering support both windowed and fullscreen 
*
*
****************************************************************************/

#ifndef	__GM_H
#define	__GM_H

#ifndef __MGRAPH_H
#include "mgraph.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(1)

/****************************************************************************
REMARKS:
Stretch flags to pass to GM_setMode. The 1x2 stretch is
useful for rendering in 320x200/240 modes on hardware that can't do
double scanning but does support 320x400/480 modes natively. 1x2
stretching in the MGL is very efficient so this is better than
rendering at 320x400 natively if each pixel takes a lot of computation
time.

HEADER:
gm\gm.h

MEMBERS:
GM_STRETCH_1x1	- No stretching
GM_STRETCH_1x2	- 1x2 stretch (2x vertical stretch)
GM_STRETCH_2x2	- 2x2 stretch (2x stretch in both X and Y)
****************************************************************************/
typedef enum {
	GM_STRETCH_1x1,
	GM_STRETCH_1x2,
	GM_STRETCH_2x2,
	} GM_stretchType;

/****************************************************************************
REMARKS:
Mode flags to pass to GM_init. These flags inform the Game Framework which
color depths you want to support in your game, and can be a logical OR
combination of all the available flags. Hence if you game only supports 8bpp
modes then you would pass in GM_MODE_8BPP. If you game only supports 8bpp
and 16bpp, then you would pass in GM_MODE_8BPP | GM_MODE_16BPP.

Note: GM_MODE_16BPP includes support for both 15bpp (5:5:5) and 16bpp (5:6:5)
	  MGL modes and if you support GM_MODE_16BPP then you will have to be
	  able to support either format on the end users system.

HEADER:
gm\gm.h

MEMBERS:
GM_MODE_8BPP		- Include support for 8bpp modes
GM_MODE_16BPP		- Include support for 15bpp and 16bpp modes
GM_MODE_24BPP		- Include support for 24bpp modes (3 bytes per pixel)
GM_MODE_32BPP		- Include support for 32bpp modes (4 bytes per pixel)
GM_ONLY_2D_ACCEL	- Only include modes with 2D hardware support
GM_ONLY_3D_ACCEL	- Only include modes with 3D hardware support
GM_ALLOW_WINDOWED	- Allow for windowed only modes in mode list
GM_MODE_ALLBPP		- Include support for all color depths
****************************************************************************/
typedef enum {
	GM_MODE_8BPP		= 0x01,
	GM_MODE_16BPP		= 0x02,
	GM_MODE_24BPP		= 0x04,
	GM_MODE_32BPP		= 0x08,
	GM_ONLY_2D_ACCEL	= 0x10,
	GM_ONLY_3D_ACCEL	= 0x20,
	GM_ALLOW_WINDOWED	= 0x40,
	GM_MODE_ALLBPP		= 0x0F,
	} GM_modeFlagsType;

/****************************************************************************
REMARKS:
The following is a set of useful constants for the keyboard message
passed in the event_t message field by the MGL's event handling functions.
Use EVT_asciiCode to extract the ASCII codes for regular keys.
****************************************************************************/
typedef enum {
	kbGrayEnter     = 0xE00D,
	kbGrayMinus     = 0x4A2D,
	kbGrayPlus      = 0x4E2B,
	kbGrayTimes     = 0x372A,
	kbGrayDivide    = 0x36E0,
	kbF1            = 0x3B00,
	kbF2            = 0x3C00,
	kbF3            = 0x3D00,
	kbF4            = 0x3E00,
	kbF5            = 0x3F00,
	kbF6            = 0x4000,
	kbF7            = 0x4100,
	kbF8            = 0x4200,
	kbF9            = 0x4300,
	kbF10           = 0x4400,
	kbF11           = 0x8500,
	kbF12           = 0x8600,
	kbLeft          = 0x4B00,
	kbRight         = 0x4D00,
	kbUp            = 0x4800,
	kbDown          = 0x5000,
	kbIns           = 0x5200,
	kbDel           = 0x5300,
	kbHome          = 0x4700,
	kbEnd           = 0x4F00,
	kbPgUp          = 0x4900,
	kbPgDn          = 0x5100,
	} GM_keyCodes;

/****************************************************************************
REMARKS:
This structure contains the flags for letting the Game Framework know which
drivers should be registered with the MGL to enable support for different
device driver technologies. By default all these drivers
are enabled, and you can change the values of these flags by calling
GM_setDriverOptions before calling GM_init.

HEADER:
gm\gm.h

MEMBERS:
useWinDirect		- True to enable WinDirect support
useDirectDraw		- True to enable DirectDraw support
useVGA				- True to enable Standard VGA support
useVGAX				- True to enable VGA ModeX support
useVBE				- True to enable VBE 2.0 support
useLinear			- True to enable linear framebuffer support
useVBEAF			- True to enable VBE/AF support
useFullscreenDIB	- True to enable fullscreen DIB support
useHWOpenGL			- True to enable OpenGL hardware support
openGLType			- OpenGL type to be used (defaults to MGL_GL_AUTO)
modeFlags			- Mode flags for supported color depths
****************************************************************************/
typedef struct {
	ibool				useWinDirect;
	ibool				useDirectDraw;
	ibool				useVGA;
    ibool				useVGAX;
	ibool				useVBE;
    ibool				useLinear;
	ibool				useVBEAF;
    ibool				useFullscreenDIB;
	ibool 				useHWOpenGL;
	MGL_glOpenGLType	openGLType;
	GM_modeFlagsType	modeFlags;
	} GM_driverOptions;

/****************************************************************************
REMARKS:
The structure maintains information about the graphics modes that are supported
by the game framework and is passed to GM_setMode to specify the mode to be initialized.
Note that the xRes and yRes values are the logical resolution for the mode which may be
different to the physical resolution, since the Game Framework also enumerates
/pseudo/ modes that use stretching. Hence even if the hardware does not have
native support for a 320x240 mode, it may appear in the list using 320x480
as the real mode and a stretch factor of 1x2 or using 640x480 as the real
mode and a stretch factor of 2x2.

If you wish to set a windowed mode directly set the mode parameter to
grWINDOWED and the mode will start as a windowed mode.

HEADER:
gm\gm.h

MEMBERS:
mode			- Fullscreen MGL mode number (-1 means windowed mode)
xRes			- Logical X resolution for mode (not physical!)
yRes			- Logical Y resolution for mode (not physical!) 
bitsPerPixel	- Color depth for mode. Note 16bpp includes 15bpp (5:5:5)
pages			- Number of hardware display pages for mode
flags			- Mode flags for the mode 
driverName		- Name of driver that will be used in fullscreen modes
stretch			- Stretch factor for the mode
windowedStretch	- Stretch factor to use in windowed modes
****************************************************************************/
typedef struct {
	int				mode;
	int				xRes;
	int				yRes;
	int				bitsPerPixel;
	int				pages;
	ulong			flags;
	char			driverName[13];
	GM_stretchType	stretch;
	GM_stretchType	windowedStretch;
	} GM_modeInfo;

#define	GM_WINDOWED_ONLY	-1

/****************************************************************************
REMARKS:
Main structure for maintaining the state information for the Game
Framework. The application program always does all drawing and rendering
to the GMDC dc member, which will draw directly to the framebuffer or
to a system memory buffer depending on the hardware and the
initialization information. The modeFlags field contains the original mode
flags information passed to GM_init, which defines which color depths your
game will support. The modeList contains a complete list of all the
available graphics modes supported by the Game Framework, including /psuedo/
modes that are modes that include auto-stretching.

The /dispdc/ and /memdc/ field are primarily for internal use by the Game
Framework, and you should not use those fields unless you are clear what
they are used for.

HEADER:
gm\gm.h

MEMBERS:
dc					- DC for drawing
dispdc				- Main display DC
memdc				- Memory back buffer if necessary
driver				- Driver ID detected by the MGL
numModes			- Number of modes in the mode list
numFullscreenModes	- Number of fullscreen capable modes in the mode list
modeFlags			- Mode flags for current graphics mode
modeList			- List of all available modes supported by the Game Framework
mainWindow			- Handle to main window (Windows only)
****************************************************************************/
typedef struct {
	MGLDC   			*dc;
	MGLDC   			*dispdc;
	MGLDC   			*memdc;
	int					driver;
	int					numModes;
	int					numFullscreenModes;
	ulong				modeFlags;
	GM_modeInfo			modeList[grMAXMODE+1];
#ifdef __WINDOWS__
	MGL_HWND			mainWindow;
#endif
	} GMDC;

/* Type definitions for Game Framework callback functions */

// {partOf:GM_setDrawFunc}
typedef void (*GM_drawFunc)(void);
// {partOf:GM_setKeyDownFunc}
typedef void (*GM_keyDownFunc)(
	event_t *evt);
// {partOf:GM_setKeyRepeatFunc}
typedef void (*GM_keyRepeatFunc)(
	event_t *evt);
// {partOf:GM_setKeyUpFunc}
typedef void (*GM_keyUpFunc)(
	event_t *evt);
// {partOf:GM_setMouseDownFunc}
typedef void (*GM_mouseDownFunc)(
	event_t *evt);
// {partOf:GM_setMouseUpFunc}
typedef void (*GM_mouseUpFunc)(
	event_t *evt);
// {partOf:GM_setMouseMoveFunc}
typedef void (*GM_mouseMoveFunc)(
	event_t *evt);
// {partOf:GM_setEventFunc}
typedef void (*GM_eventFunc)(
	event_t *evt);
// {partOf:GM_setModeSwitchFunc}
typedef void (*GM_modeSwitchFunc)(
	GM_modeInfo *mode,
    ibool windowed);
// {partOf:GM_setPreModeSwitchFunc}
typedef ibool (*GM_preModeSwitchFunc)(
	GM_modeInfo *mode,
    ibool windowed);
// {partOf:GM_setGameLogicFunc}
typedef void (*GM_gameFunc)(void);
// {partOf:GM_setExitFunc}
typedef void (*GM_exitFunc)(void);
// {partOf:GM_setAppActivate}
typedef void (*GM_activateFunc)(
	ibool active);
// {partOf:GM_setModeFilterFunc}
typedef ibool (*GM_modeFilterFunc)(
	int xRes,
    int yRes,
    int bits,
    ulong flags);

#pragma pack()

/*----------------------------- Global Variables --------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

extern ibool	GM_haveWin31;	/* True if we have Windows 3.1 				*/
extern ibool	GM_haveWin95;	/* True if we have Windows 95				*/
extern ibool	GM_haveWinNT;	/* True if we have Windows NT				*/
extern ibool	GM_doDraw;		/* True if we should be drawing frames		*/
extern ibool	GM_exitMainLoop;/* True if the main loop should be exited	*/

/*------------------------- Function Prototypes ---------------------------*/

/*---------------------------------------------------------------------------
 * Initialisation functions. 
 *-------------------------------------------------------------------------*/

void 	MGLAPI GM_initSysPalNoStatic(ibool flag);
void 	MGLAPI GM_initPath(const char *MGLPath);
void 	MGLAPI GM_initWindowPos(int x,int y);
void	MGLAPI GM_setModeFilterFunc(GM_modeFilterFunc filter);
void	MGLAPI GM_setDriverOptions(GM_driverOptions *opt);
GMDC * 	MGLAPI GM_init(const char *windowTitle);

/*---------------------------------------------------------------------------
 * Functions to register application callbacks with the game framework.
 *-------------------------------------------------------------------------*/

void 	MGLAPI GM_setDrawFunc(GM_drawFunc func);
void 	MGLAPI GM_setKeyDownFunc(GM_keyDownFunc func);
void 	MGLAPI GM_setKeyRepeatFunc(GM_keyRepeatFunc func);
void 	MGLAPI GM_setKeyUpFunc(GM_keyUpFunc func);
void 	MGLAPI GM_setMouseDownFunc(GM_mouseDownFunc func);
void 	MGLAPI GM_setMouseUpFunc(GM_mouseUpFunc func);
void 	MGLAPI GM_setMouseMoveFunc(GM_mouseMoveFunc func);
void 	MGLAPI GM_setEventFunc(GM_eventFunc func);
void 	MGLAPI GM_setGameLogicFunc(GM_gameFunc func);
void 	MGLAPI GM_setExitFunc(GM_exitFunc func);
void 	MGLAPI GM_setAppActivate(GM_activateFunc func);
void 	MGLAPI GM_setPreModeSwitchFunc(GM_preModeSwitchFunc func);
void 	MGLAPI GM_setModeSwitchFunc(GM_modeSwitchFunc func);

/*---------------------------------------------------------------------------
 * Set a fullscreen suspend application callback function. This is used in
 * fullscreen video modes to allow switching back to the normal operating
 * system graphical shell (such as Windows GDI, OS/2 PM etc). Refer to the
 * MGL header files/documentation for the definitions of the flags passed
 * to this function and the values it should return.
 *-------------------------------------------------------------------------*/

void 	MGLAPI GM_setSuspendAppCallback(MGL_suspend_cb_t saveState);

/*---------------------------------------------------------------------------
 * The following allows the application programmer to provide their own
 * Win32 window procedure for the Windows environment. In most cases this
 * will not be necessary, but if there is something you need to do that we
 * don't provide you can hook this to handle window messages directly.
 *-------------------------------------------------------------------------*/

#ifdef __WINDOWS__
void	MGLAPI GM_registerEventProc(MGL_WNDPROC winproc);
#endif

/*---------------------------------------------------------------------------
 * The following allows the application programmer to do the creation of
 * the main window themselves, and then register this with the Game
 * Framework to be used for both the window for windowed and fullscreen
 * modes (only one window is ever created). This is mostly to support
 * integrating the Game Framework libraries with existing code that already
 * does window creation and message handling with the Game Framework code.
 *-------------------------------------------------------------------------*/

#ifdef	__WINDOWS__
void	MGLAPI GM_registerMainWindow(MGL_HWND hwndMain);
#endif

/*---------------------------------------------------------------------------
 * The following will pop up a dialog box allowing the user to interactively
 * choose a graphics mode, and change the support device driver technologies
 * interactively. Only those color depths that were requested to be
 * supported in the flags passed to GM_init will be available. This
 * function can only be called while in windowed modes!
 *-------------------------------------------------------------------------*/

#ifdef	__WINDOWS__
ibool	MGLAPI GM_chooseMode(GM_modeInfo *mode,ibool *startWindowed);
#endif

/*---------------------------------------------------------------------------
 * The following will search the list of available graphics modes for
 * one that has the desired resolution and color depth. 
 *-------------------------------------------------------------------------*/

ibool 	MGLAPI GM_findMode(GM_modeInfo *mode,int xRes,int yRes,int bits);

/*---------------------------------------------------------------------------
 * The following sets a graphics mode for use with the Game Framework, and
 * is passed a GM_modeInfo structure that represents the mode to start.
 * Note that the modes can be both windowed and fullscreen modes, and can
 * this function can be called while already in a windowed or fullscreen
 * mode to change modes dynamically on the fly.
 *-------------------------------------------------------------------------*/

ibool	MGLAPI GM_setMode(GM_modeInfo *info,ibool windowed,int pages,ibool forceSysMem);

/*---------------------------------------------------------------------------
 * Function to start OpenGL rendering support for the device context. After
 * this function has been called, you must perform all rendering operations
 * via OpenGL functions as the regular MGL functions may no longer have
 * direct access to the device context surface. You can call
 * MGL_surfaceAccessType() after calling this function to determine if the
 * MGL functions can draw on the OpenGL surface (MGL_NO_ACCESS means we dont
 * have direct access). In the future this will be more fully integrated and
 * the MGL functions will always be accessible.
 *
 * The flags passed to this function as the same flags passed to
 * MGL_glCreateContext. If you pass a value of MGL_GL_VISUAL you should
 * first call MGL_glChooseVisual/MGL_glSetVisual for the GMDC->dispdc
 * device context to set a custom visual format.
 *
 * Note also that this function must be called after any call to
 * MGL_setWindowedMode or MGL_setFullScreenMode. 
 *-------------------------------------------------------------------------*/

ibool 	MGLAPI GM_startOpenGL(MGL_glContextFlagsType flags);

/*---------------------------------------------------------------------------
 * Functions to swap display buffers. The second version can be used in
 * windowing modes or with a system memory back buffer to collect a region
 * of dirty rectangle that need to be updated and only those rectangles
 * will be copied. It is recommended that the application call
 * MGL_optimizeRegion on the dirty region before calling this function
 * to minimize the number of blit operations required for the swap.
 *-------------------------------------------------------------------------*/

void	MGLAPI GM_swapBuffers(MGL_waitVRTFlagType waitVRT);
void 	MGLAPI GM_swapDirtyBuffers(region_t *dirty,MGL_waitVRTFlagType waitVRT);

/*---------------------------------------------------------------------------
 * Functions to set the palette entries and to realize the current palette
 * in the hardware. Note that you should always uses these functions rather
 * than the MGL functions as these ensure that any system memory back
 * buffers have the palette properly updated to ensure identiy palette
 * mappings for maximum blitting performance in a window.
 *-------------------------------------------------------------------------*/

void 	MGLAPI GM_setPalette(palette_t *pal,int numColors,int startIndex);
void 	MGLAPI GM_realizePalette(int numColors,int startIndex,int waitVRT);

/*---------------------------------------------------------------------------
 * Function to execute the Game Framework main loop. This is the last
 * function in the Game Framework that your application should call and it
 * will continue until you request to exit. This function will call the
 * installed exit function if one is registered before destroying the
 * graphics mode.
 *-------------------------------------------------------------------------*/

void 	MGLAPI GM_mainLoop(void);

/*---------------------------------------------------------------------------
 * Functions used to implement the GM_mainLoop function, allowing you to
 * write your own replacement for GM_mainLoop.
 *-------------------------------------------------------------------------*/

void 	MGLAPI GM_processEvents(void);
#ifdef __WINDOWS__
void 	MGLAPI GM_processEventsWin(void);
#endif
void 	MGLAPI GM_cleanup(void);

/*---------------------------------------------------------------------------
 * Function to inform that game framework main loop that you wish to exit
 * the application. This simply causes the main loop function to return
 * after all pending messages have been processed.
 *-------------------------------------------------------------------------*/

void 	MGLAPI GM_exit(void);

/*---------------------------------------------------------------------------
 * Delphi DLL entry points to read global variables
 *-------------------------------------------------------------------------*/

ibool	MGLAPI GM_getHaveWin31(void);	
ibool	MGLAPI GM_getHaveWin95(void);
ibool	MGLAPI GM_getHaveWinNT(void);
ibool	MGLAPI GM_getDoDraw(void);
ibool	MGLAPI GM_getExitMainLoop(void);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif	/* __cplusplus */

#endif	/* __GM_H */

