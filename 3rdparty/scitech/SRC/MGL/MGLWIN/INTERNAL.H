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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Internal header file for the MGLWIN bindings for the MGL.
*				All WinDirect full screen Windows display drivers require
*				the WinDirect VxD to be loaded in order to provide access
*				to the VESA VBE services from the Win32 application. 
*
****************************************************************************/

#ifndef	__MGLWIN_INTERNAL_H
#define	__MGLWIN_INTERNAL_H

#define	STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "mgl.h"
#include "wdirect.h"
#include "vesavbe.h"
#include "mgldib.h"
#include "mglwin\ddraw.h"

/*---------------------- Macro and type definitions -----------------------*/

#define	PC_FULLSCREEN			/* Include fullscreen mode support		*/
#define	PC_ACCELERATED			/* Include accelerated mode support		*/
#define	MAX_DD_SURF	8			/* Maximum number of DirectDraw surf's	*/

#ifndef	MAXPATH
#define	MAXPATH		80			/* Size of path variables				*/
#endif

/* Custom window messages */

#define	WM_DO_SUSPEND_APP			WM_USER
#define	WM_MGL_DEACTIVATE			0
#define	WM_MGL_RESTORE_FULLSCREEN	1
#define	WM_MGL_START_CHECK_SWITCH	2

/* Windows LOGPALETTE palette structure */

typedef struct {
	WORD 			version;
	WORD 			entries;
	PALETTEENTRY	pe[256];
	} LogPal;

/* Windows internal HBITMAP structure */

typedef struct {
	short		bmType;             /* 0 for main memory                    */
	ushort 		bmWidth;     		/* Width of bitmap in pixels        	*/
	ushort 		bmHeight;    		/* Height of bitmap in pixels			*/
	ushort		bmWidthBytes;		/* #bytes per scan line					*/
	uchar		bmPlanes;			/* # of planes in bitmap				*/
	uchar		bmBitsPixel;		/* # bits per pixel						*/
	uchar		*bmBits;			/* pointer to bits of main mem bitmap	*/
	ulong		bmWidthPlanes;		/* bmWidthBytes * bmHeight				*/
	uchar		*bmlpPDevice;		/* Pointer to associated pDevice		*/
	ushort		bmSegmentIndex;		/* Index to planes next segment 		*/
	ushort		bmScanSegment;		/* Number of scans per segment			*/
	ushort		bmFillBytes;		/* Number of unused bytes per segment	*/
	} win_BITMAP;

/* The following structure is used to communicate with Cosmo OpenGL to let
 * it know about our surface information for fullscreen modes.
 */
#pragma pack(1)
typedef struct {
	void            *frontSurface;  /* Pointer to start of front buffer         */
	void            *backSurface;   /* Pointer to start of back buffer          */
	int             width;          /* Surface width in pixels                  */
	int             height;         /* Surface height in pixels                 */
	int             bytesPerLine;   /* Scanline pitch for the surface           */
	int             bitsPerPixel;   /* Pixel bits for mode (15 = 5:5:5 HiColor) */
	pixel_format_t  pf;             /* Pixel format information                 */
	palette_t       *pal;       	/* Hardware palette to be used              */

	/* Buffer arbitration functions */
	void            (_cdecl *BeginDirectAccess)(void);
	void            (_cdecl *EndDirectAccess)(void);
	void            (_cdecl *Fill)(int x,int y,int w,int h,uint color);
	} __MGLGLSurfaceInfo;
#pragma pack()

typedef void *MGLGLSurface;
typedef void *MGLGLContext;

/* Types of offscreen buffers and blt'ing that can be used under Windows.
 * When running under WinG, we can only support 8 bit memory DC's.
 */

typedef enum {
	BLT_CreateDIBSection,
	BLT_WinG,
	};

#ifndef	__WIN386__

/* Macros to call the dynamically linked Windows functions */

#define	WinGCreateDC()						pWinGCreateDC()
#define	WinGCreateBitmap(a,b,c)				pWinGCreateBitmap(a,b,c)
#define	WinGSetDIBColorTable(a,b,c,d)		pWinGSetDIBColorTable(a,b,c,d)
#define	WinGBitBlt(a,b,c,d,e,f,g,h)			pWinGBitBlt(a,b,c,d,e,f,g,h)
#define	WinGStretchBlt(a,b,c,d,e,f,g,h,i,j)	pWinGStretchBlt(a,b,c,d,e,f,g,h,i,j)
#define	CreateDIBSection(a,b,c,d,e,f)		pCreateDIBSection(a,b,c,d,e,f)
#define	SetDIBColorTable(a,b,c,d)			pSetDIBColorTable(a,b,c,d)

/* Macros to call the dynalinked ChangeDisplaySettings functions */

#undef ChangeDisplaySettings
#undef EnumDisplaySettings
#define ChangeDisplaySettings(a,b)			pChangeDisplaySettings(a,b)
#define EnumDisplaySettings(a,b,c)			pEnumDisplaySettings(a,b,c)

/* Macros to call the dynalinked DirectDraw functions */

#define DirectDrawCreate(a,b,c)				pDirectDrawCreate(a,b,c)
#define DirectDrawCreateClipper(a,b,c)		pDirectDrawCreateClipper(a,b,c)

/* Macros to call the dyanamically linked WinDirect DLL library code when
 * compiling code for Windows (except for Win386 mode).
 */

#define	WD_startFullScreen(hwndMain,xRes,yRes)	pWD_startFullScreen(hwndMain,xRes,yRes)
#define	WD_changeResolution(xRes,yRes)			pWD_changeResolution(xRes,yRes)
#define	WD_restoreGDI()							pWD_restoreGDI()
#define	WD_setSuspendAppCallback(saveState)		pWD_setSuspendAppCallback(saveState)
#define	WD_getEvent(evt,mask)					pWD_getEvent(evt,mask)
#define	WD_peekEvent(evt,mask)					pWD_peekEvent(evt,mask)
#define	WD_postEvent(which,what,message,mod)	pWD_postEvent(which,what,message,mod)
#define	WD_flushEvent(mask)						pWD_flushEvent(mask)
#define	WD_haltEvent(evt,mask)					pWD_haltEvent(evt,mask)
#define	WD_setTimerTick(ticks)					pWD_setTimerTick(ticks)
#define	WD_isSafeMode()							pWD_isSafeMode()
#define	_WD_doSuspendApp(msg)					p_WD_doSuspendApp(msg)
#define	WD_isSafeMode()							pWD_isSafeMode()
#define	WD_registerEventProc(w)					pWD_registerEventProc(w)
#define	WD_registerFullScreenWindow(w)			pWD_registerFullScreenWindow(w)
#define	WD_setAppInstance(i)					pWD_setAppInstance(i)
#define	WD_getMousePos(x,y)						pWD_getMousePos(x,y)
#define WD_setMousePos(x,y)						pWD_setMousePos(x,y)
#define WD_setMouseCallback(mcb)				pWD_setMouseCallback(mcb)

/* Macros to call the dyanamically linked PM/Pro DLL library code when
 * compiling code for Windows
 */

#define	PM_getByte(s,o)					pPM_getByte(s,o)
#define	PM_getWord(s,o)					pPM_getWord(s,o)
#define	PM_getLong(s,o)					pPM_getLong)(s,o)
#define	PM_setByte(s,o,v)				pPM_setByte(s,o,v)
#define	PM_setWord(s,o,v)				pPM_setWord(s,o,v)
#define	PM_setLong(s,o,v)				pPM_setLong(s,o,v)
#define	PM_memcpynf(d,ss,so,n)			pPM_memcpynf(d,ss,so,n)
#define	PM_memcpyfn(ds,do,s,n)			pPM_memcpyfn(ds,do,s,n)
#define	PM_getBIOSSelector()			pPM_getBIOSSelector()
#define	PM_createSelector(b,l)			pPM_createSelector(b,l)
#define	PM_freeSelector(s)				pPM_freeSelector(s)
#define	PM_mapPhysicalAddr(b,l)			pPM_mapPhysicalAddr(b,l)
#define	PM_mapRealPointer(s,o,rs,ro)	pPM_mapRealPointer(s,o,rs,ro)
#define	PM_allocRealSeg(sz,s,o,rs,ro)	pPM_allocRealSeg(sz,s,o,rs,ro)
#define	PM_freeRealSeg(s,o)				pPM_freeRealSeg(s,o)
#define	PM_callRealMode(s,o,r,sr)		pPM_callRealMode(s,o,r,sr)
#define	PM_int86(x,i,o)					pPM_int86(x,i,o)
#define	PM_int86x(x,i,o,s)				pPM_int86x(x,i,o,s)
#define	PM_availableMemory(p,t)			pPM_availableMemory(p,t)
#define	VF_available()					pVF_available()
#define	VF_init(ba,bs,cl,bf)			pVF_init(ba,bs,cl,bf)
#define	VF_exit()						pVF_exit()
#define	PM_saveDS()						pPM_saveDS()
#define	_PM_getRMvect(i,r)				p_PM_getRMvect(i,r)
#define	_PM_setRMvect(i,r)				p_PM_setRMvect(i,r)

#endif	/* !__WIN386__ */

/*--------------------------- Global Variables ----------------------------*/

extern HINSTANCE    _MGL_hInstOpenGL;
extern HINSTANCE    _VARAPI _MGL_hInst;     /* HINSTANCE for MGL application    */
extern HWND 		_VARAPI _MGL_hwndMain;	/* Handle to main window			*/
extern HWND			_VARAPI _MGL_hwndFullScreen;/* Handle to fullscreen MGL window */
extern HWND			_VARAPI _MGL_hwndUser;	/* Handle to user fullscreen window	*/
extern WNDPROC 		_VARAPI _MGL_userEventProc;
extern uint			_VARAPI _MGL_biosSel;	/* Selector for BIOS data area		*/
extern ibool 		_VARAPI _MGL_haveWinDirect;/* True if WinDirect is available*/
extern ibool			_VARAPI _MGL_fullScreen; /* Are we in a full screen mode?	*/
extern ibool			_VARAPI _MGL_checkSwitch;/* Do switch checking?				*/
extern ibool			_VARAPI _MGL_winDirectMode;/* Are we in a WinDirect mode?	*/
extern int 			_VARAPI _MGL_bltType;	/* Type of blt'ing to use			*/
extern int 			_VARAPI _MGL_deskX;		/* Desktop resolution				*/
extern int 			_VARAPI _MGL_deskY;
extern int 			_VARAPI _MGL_mx;		/* Mouse coordinates				*/
extern int 			_VARAPI _MGL_my;
extern int			_VARAPI _MGL_xRes;		/* Video mode resolution			*/
extern int			_VARAPI _MGL_yRes;
extern int			(_VARAPI *_MGL_suspendApp)(MGLDC *dc,int flags);
extern ibool			_MGL_isBackground;		/* App is in the background			*/
extern ibool 		_MGL_haveWin31;			/* True if we have Windows 3.1		*/
extern ibool 		_MGL_haveWin95;			/* True if we have Windows 95		*/
extern ibool			_MGL_haveWinNT;			/* True if we have Windows NT		*/

#ifndef	__WIN386__

extern LPDIRECTDRAW2 		_MGL_lpDD;		/* DirectDraw2 object		*/
extern int					_MGL_numDDSurf;	/* Number of DD surfaces	*/
extern LPDIRECTDRAWSURFACE	_MGL_lpPrimary;	/* DirectDraw Primary Surf	*/
extern LPDIRECTDRAWSURFACE	_MGL_lpOffscreen;/* DirectDraw Offscreen Surf*/
extern LPDIRECTDRAWSURFACE	_MGL_lpSurf[MAX_DD_SURF];
extern LPVOID				_MGL_lpSurface[MAX_DD_SURF];
extern LPDIRECTDRAWSURFACE	_MGL_lpActive;	/* DirectDraw Active Surf	*/
extern LPDIRECTDRAWPALETTE	_MGL_lpPalette;	/* DirectDraw Palette		*/
extern DDCAPS				_MGL_ddCaps;	/* DirectDraw capabilities	*/
extern char 				_DD_szWinClassName[];

/* Dynalinks to WinG functions */

extern HDC      (WINAPI *pWinGCreateDC)(void);
extern HBITMAP  (WINAPI *pWinGCreateBitmap)(HDC WinGDC,BITMAPINFO *pHeader,void **ppBits);
extern UINT     (WINAPI *pWinGSetDIBColorTable)(HDC WinGDC,UINT StartIndex,UINT NumberOfEntries,RGBQUAD *pColors);
extern BOOL     (WINAPI *pWinGBitBlt)(HDC hdcDest,int nXOriginDest,int nYOriginDest,int nWidthDest,int nHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc);
extern BOOL     (WINAPI *pWinGStretchBlt)(HDC hdcDest,int nXOriginDest,int nYOriginDest,int nWidthDest,int nHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc,int nWidthSrc,int nHeightSrc);

/* Dynalinks to ChangeDisplaySettings functions */

extern LONG 	(WINAPI *pChangeDisplaySettings)(LPDEVMODEA lpDevMode,DWORD dwFlags);
extern BOOL 	(WINAPI *pEnumDisplaySettings)(LPCSTR lpszDeviceName,DWORD iModeNum,LPDEVMODEA lpDevMode);

/* Dynalinks to DirectDraw functions */

extern HRESULT  (WINAPI *pDirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
extern HRESULT  (WINAPI *pDirectDrawCreateClipper)( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter);

/* Dynalinks to CreateDIBSection functions */

extern HBITMAP  (WINAPI *pCreateDIBSection)(HDC,CONST BITMAPINFO *,UINT,VOID **,HANDLE,DWORD);
extern UINT     (WINAPI *pSetDIBColorTable)(HDC,UINT,UINT,CONST RGBQUAD *);

/* Dyanalinks to WinDirect functions */

extern HWND     (_ASMAPI *pWD_startFullScreen)(HWND hwndMain,int xRes,int yRes);
extern void		(_ASMAPI *pWD_changeResolution)(int xRes,int yRes);
extern void     (_ASMAPI *pWD_restoreGDI)(void);
extern void     (_ASMAPI *pWD_setSuspendAppCallback)(int (_ASMAPI *saveState)(int flags));
extern ibool     (_ASMAPI *pWD_getEvent)(event_t *evt,uint mask);
extern ibool     (_ASMAPI *pWD_peekEvent)(event_t *evt,uint mask);
extern ibool     (_ASMAPI *pWD_postEvent)(ulong which,uint what,ulong message,ulong modifiers);
extern void     (_ASMAPI *pWD_flushEvent)(uint mask);
extern void     (_ASMAPI *pWD_haltEvent)(event_t *evt,uint mask);
extern int      (_ASMAPI *pWD_setTimerTick)(int ticks);
extern ibool     (_ASMAPI *pWD_isSafeMode)(void);
extern void 	(_ASMAPI *p_WD_doSuspendApp)(MSG *orgmsg);
extern void 	(_ASMAPI *pWD_registerEventProc)(WNDPROC userWndProc);
extern void 	(_ASMAPI *pWD_registerFullScreenWindow)(HWND hwnd);
extern void 	(_ASMAPI *pWD_setAppInstance)(HINSTANCE hInstApp);
extern void 	(_ASMAPI *pWD_getMousePos)(int *x,int *y);
extern void 	(_ASMAPI *pWD_setMousePos)(int x,int y);
extern void 	(_ASMAPI *pWD_setMouseCallback)(void (_ASMAPI *mcb)(int x,int y));

/* Dynalinks to PM/Pro functions */

extern uchar    (PMAPI *pPM_getByte)(uint s, uint o);
extern ushort   (PMAPI *pPM_getWord)(uint s, uint o);
extern ulong    (PMAPI *pPM_getLong)(uint s, uint o);
extern void     (PMAPI *pPM_setByte)(uint s, uint o, uchar v);
extern void     (PMAPI *pPM_setWord)(uint s, uint o, ushort v);
extern void     (PMAPI *pPM_setLong)(uint s, uint o, ulong v);
extern void     (PMAPI *pPM_memcpynf)(void *dst,uint src_s,uint src_o,uint n);
extern void     (PMAPI *pPM_memcpyfn)(uint dst_s,uint dst_o,void *src,uint n);
extern uint     (PMAPI *pPM_getBIOSSelector)(void);
extern uint     (PMAPI *pPM_createSelector)(ulong base,ulong limit);
extern void     (PMAPI *pPM_freeSelector)(uint sel);
extern void *   (PMAPI *pPM_mapPhysicalAddr)(long base,long limit);
extern void     (PMAPI *pPM_mapRealPointer)(uint *sel,uint *off,uint r_seg,uint r_off);
extern int      (PMAPI *pPM_allocRealSeg)(uint size,uint *sel,uint *off,uint *r_seg,uint *r_off);
extern void     (PMAPI *pPM_freeRealSeg)(uint sel,uint off);
extern void     (PMAPI *pPM_callRealMode)(uint seg,uint off, RMREGS *regs,RMSREGS *sregs);
extern int      (PMAPI *pPM_int86)(int intno, RMREGS *in, RMREGS *out);
extern int      (PMAPI *pPM_int86x)(int intno, RMREGS *in, RMREGS *out,RMSREGS *sregs);
extern void     (PMAPI *pPM_availableMemory)(ulong *physical,ulong *total);
extern ibool     (PMAPI *pVF_available)(void);
extern void *   (PMAPI *pVF_init)(ulong baseAddr,int bankSize,int codeLen,void *bankFunc);
extern void     (PMAPI *pVF_exit)(void);
extern void    	(PMAPI *pPM_saveDS)(void);
extern void    	(PMAPI *pPM_setBankA)(int bank);
extern void    	(PMAPI *pPM_setBankAB)(int bank);
extern void    	(PMAPI *pPM_setCRTStart)(int x,int y,int waitVRT);
extern void 	(PMAPI *p_PM_getRMvect)(int intno, long *realisr);
extern void 	(PMAPI *p_PM_setRMvect)(int intno, long realisr);

/* Dynalinks to Mesa OpenGL MGL bindings */

extern void  	(APIENTRY *MGLMesaChooseVisual)(MGLDC *dc,MGLVisual *visual);
extern ibool		(APIENTRY *MGLMesaSetVisual)(MGLDC *dc,MGLVisual *visual);
extern ibool		(APIENTRY *MGLMesaCreateContext)(MGLDC *dc,ibool forceMemDC);
extern void 	(APIENTRY *MGLMesaDestroyContext)(MGLDC *dc);
extern void 	(APIENTRY *MGLMesaMakeCurrent)(MGLDC *dc);
extern void		(APIENTRY *MGLMesaSwapBuffers)(MGLDC *dc,ibool waitVRT);
extern void 	(APIENTRY *MGLMesaSetPaletteEntry)(MGLDC *dc,int entry,uchar red,uchar green,uchar blue);
extern void 	(APIENTRY *MGLMesaSetPalette)(MGLDC *dc,palette_t *pal,int numColors,int startIndex);
extern void 	(APIENTRY *MGLMesaRealizePalette)(MGLDC *dc,int numColors,int startIndex,int waitVRT);

/* Dynalinks to Cosmo OpenGL fullscreen MGL bindings */

extern void 	(WINAPI *mglChooseVisual)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
extern BOOL 	(WINAPI *mglCheckVisual)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
extern MGLGLContext (WINAPI *mglCreateContext)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
extern MGLGLSurface (WINAPI *mglCreateSurface)(__MGLGLSurfaceInfo *info, MGLVisual *visual);
extern BOOL 	(WINAPI *mglDeleteSurface)(MGLGLSurface surface);
extern BOOL 	(WINAPI *mglDeleteContext)(MGLGLContext ctx);
extern BOOL 	(WINAPI *mglMakeCurrent)(MGLGLContext ctx, MGLGLSurface surface);
extern void 	(WINAPI *mglSetSurfacePtr)(MGLGLSurface surface,void *frontSurface, void *backSurface);
extern PROC  	(WINAPI *mglGetProcAddress)(LPCSTR lpzProcName);

/* Dynalinks to Fullscreen hardware OpenGL extensions */

extern ibool		(WINAPI *fglDetect)(int *numModes,struct _FGL_hwInfo *hwInfo,struct _FGL_modeInfo *modes);
extern ibool		(WINAPI *fglSetVideoMode)(int mode);
extern void		(WINAPI *fglSetFocus)(ibool active);
extern void		(WINAPI *fglRestoreMode)(void);
extern void * 	(WINAPI *fglBeginDirectAccess)(int buffer);
extern void		(WINAPI *fglEndDirectAccess)(void);
extern ibool  	(WINAPI *fglSwapBuffers)(ibool waitVRT);
extern void 	(WINAPI *fglRealizePalette)(palette_t *pal,int numColors,int startIndex,int waitVRT);
extern PROC 	(WINAPI *fglGetProcAddress)(LPCSTR str);

#endif	/* !__WIN386__ */

/*------------------------- Function Prototypes ---------------------------*/

/* Enable and disable full screen operation */

void _DD_createMainWindow(void);
HWND _DD_convertUserWindow(HWND hwnd,int width,int height);
void _DD_restoreUserWindow(HWND hwnd);
void _DD_subClassWindow(HWND _MGL_hwndFullScreen);
void _DD_unsubClassWindow(HWND _MGL_hwndFullScreen);
ibool _DD_enumerateSurfaces(MGLDC *dc);
ibool _WIN_restoreFullScreenMode(MGLDC *dc);
void _WIN_restoreDisplayMode(ibool restoreGDI);

/* Initialise the event handling mechanism for windowed modes */

void _EVT_init(MGL_HWND hwnd);

/* DirectDraw suspend application callback functions */

void 	_DD_doSuspendApp(void);
void 	_MS_saveState(void);
void 	_MS_restoreState(void);

#ifdef	__WIN386__

/* Special WinG compatibility functions for Watcom C++ Win386 */

HDC     WinGCreateDC(void);
HBITMAP WinGCreateBitmap(HDC WinGDC,BITMAPINFO *pHeader,void **ppBits);
UINT    WinGSetDIBColorTable(HDC WinGDC,UINT StartIndex,UINT NumberOfEntries,RGBQUAD *pColors);
BOOL   	WinGBitBlt(HDC hdcDest,int nXOriginDest,int nYOriginDest,int nWidthDest,int nHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc);
BOOL    WinGStretchBlt(HDC hdcDest,int nXOriginDest,int nYOriginDest,int nWidthDest,int nHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc,int nWidthSrc,int nHeightSrc);

ibool	WinG_initWin386(void);

#endif	/* __WIN386__ */

#endif	/* __MGLWIN_INTERNAL_H */
