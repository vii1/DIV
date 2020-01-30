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
* Description:  Windows specific code bindings for the MGL.
*
*
****************************************************************************/

#define INITGUID
#include "mglwin\internal.h"

/*--------------------------- Global Variables ----------------------------*/

#define WDIRECT_DLL         "WDIR_32.DLL"

/* Globals private to this module */

int		_MGL_driverId;				/* Driver selected					*/
int		_MGL_modeId;				/* Mode selected					*/
LIST	*_MGL_dispDCList = NULL;	/* List of display contexts			*/
#ifndef	MGL_LITE
LIST	*_MGL_offDCList = NULL;		/* List of offscreen contexts		*/
#endif
LIST	*_MGL_winDCList = NULL;		/* List of windowed contexts		*/
LIST	*_MGL_memDCList = NULL;		/* List of memory contexts			*/
static	ibool	initCommon = false;
static 	ibool	initWindowed = false;
static	ibool	initFullscreen = false;
static 	ibool	inSuspendApp = false;
static 	ibool	waitActive = false;	/* True when waiting to activate	*/
static  HINSTANCE hInstWinG = NULL;	/* Handle to WinG DLL instance		*/
static  HINSTANCE hInstWinD = NULL;	/* Handle to WinDirect DLL			*/
#ifndef	__WIN386__
static  HINSTANCE hGDI = NULL;		/* Handle to GDI32 DLL instance		*/
static 	HINSTANCE hUSER = NULL;		/* Handle to USER32 DLL instance	*/
static  HINSTANCE hInstDD = NULL;	/* Handle to DirectDraw DLL 		*/
static  LONG		oldWndStyle;	/* Info about old user window		*/
static  LONG		oldExWndStyle;	/* Info about old user window		*/
static	int			oldWinPosX;		/* Old window position X coordinate	*/
static	int			oldWinPosY;		/* Old window pisition Y coordinate	*/
static	int			oldWinSizeX;	/* Old window size X				*/
static	int			oldWinSizeY;	/* Old window size Y				*/
static  WNDPROC		oldWndProc = NULL;
int				_VARAPI _MGL_mx;	/* Current mouse position			*/
int				_VARAPI _MGL_my;	/* Current mouse position			*/
ibool			_MGL_haveWin31 = false;
ibool			_MGL_haveWin95 = false;
ibool			_MGL_haveWinNT = false;
static 	ibool	_MGL_backInGDI = false;
uint			_MGL_cw_default = 0;/* Floating point control word		*/
#endif
HWND			_VARAPI _MGL_hwndMain = NULL;/* Handle to main window	*/
HWND			_VARAPI _MGL_hwndFullScreen = NULL;/* Handle to fullscreen Window */
HWND			_VARAPI _MGL_hwndUser = NULL;
int 			_VARAPI _MGL_deskX;	/* Desktop resolution				*/
int 			_VARAPI _MGL_deskY;	/* Desktop resolution				*/
int				_VARAPI _MGL_xRes;	/* Video mode resolution			*/
int				_VARAPI _MGL_yRes;	/* Video mode resolution			*/
HINSTANCE		_VARAPI _MGL_hInst = NULL;	/* Handle to MGL app instance	*/
WNDPROC			_VARAPI _MGL_userEventProc = NULL;
WNDPROC			_VARAPI _DD_orgWinProc = NULL;
int				(_VARAPI * _MGL_suspendApp)(MGLDC *dc,int flags);
uint	_VARAPI _MGL_biosSel = 0;			/* Selector for BIOS data area		*/
ibool 	_VARAPI _MGL_haveWinDirect = false;
ibool	_VARAPI _MGL_fullScreen = false;
ibool	_VARAPI _MGL_winDirectMode = 0;
int		_VARAPI _MGL_bltType;
extern	ibool	_MGL_isBackground = FALSE;
char  	_DD_szWinClassName[] = "MGLDirectDrawWindow";

#ifndef	__WIN386__

LPDIRECTDRAW	   	_MGL_lpDD1 = NULL;	/* DirectDraw object			*/
LPDIRECTDRAW2 	   	_MGL_lpDD = NULL;	/* DirectDraw2 object			*/
int				   	_MGL_numDDSurf = 0;	/* Number of DD surfaces		*/
LPDIRECTDRAWSURFACE	_MGL_lpPrimary = NULL;/* DirectDraw Primary Surf	*/
LPDIRECTDRAWSURFACE	_MGL_lpOffscreen = NULL;/* DirectDraw Offscreen Surf*/
LPDIRECTDRAWSURFACE	_MGL_lpSurf[MAX_DD_SURF] = {NULL};
LPVOID			   	_MGL_lpSurface[MAX_DD_SURF] = {NULL};
LPDIRECTDRAWSURFACE	_MGL_lpActive = NULL;/* DirectDraw Active Surf		*/
LPDIRECTDRAWPALETTE	_MGL_lpPalette = NULL;/* DirectDraw Palette			*/
DDCAPS			   	_MGL_ddCaps;		/* DirectDraw capabilities		*/

/* Dynalinks to WinG functions */

HDC     (WINAPI *pWinGCreateDC)(void);
HBITMAP (WINAPI *pWinGCreateBitmap)(HDC WinGDC,BITMAPINFO *pHeader,void **ppBits);
UINT    (WINAPI *pWinGSetDIBColorTable)(HDC WinGDC,UINT StartIndex,UINT NumberOfEntries,RGBQUAD *pColors);
BOOL    (WINAPI *pWinGBitBlt)(HDC hdcDest,int nXOriginDest,int nYOriginDest,int nWidthDest,int nHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc);
BOOL    (WINAPI *pWinGStretchBlt)(HDC hdcDest,int nXOriginDest,int nYOriginDest,int nWidthDest,int nHeightDest,HDC hdcSrc,int nXOriginSrc,int nYOriginSrc,int nWidthSrc,int nHeightSrc);

/* Dynalinks to ChangeDisplaySettings functions */

LONG 	(WINAPI *pChangeDisplaySettings)(LPDEVMODEA lpDevMode,DWORD dwFlags);
BOOL 	(WINAPI *pEnumDisplaySettings)(LPCSTR lpszDeviceName,DWORD iModeNum,LPDEVMODEA lpDevMode);

/* Dynalinks to DirectDraw functions */

HRESULT (WINAPI *pDirectDrawCreate)(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
HRESULT	(WINAPI *pDirectDrawCreateClipper)( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter);

/* Dynalinks to CreateDIBSection functions */

HBITMAP (WINAPI *pCreateDIBSection)(HDC,CONST BITMAPINFO *,UINT,VOID **,HANDLE,DWORD);
UINT    (WINAPI *pSetDIBColorTable)(HDC,UINT,UINT,CONST RGBQUAD *);

/* Dyanalinks to WinDirect functions */

HWND    (_ASMAPI *pWD_startFullScreen)(HWND hwndMain,int xRes,int yRes);
void	(_ASMAPI *pWD_changeResolution)(int xRes,int yRes);
void    (_ASMAPI *pWD_restoreGDI)(void);
void    (_ASMAPI *pWD_setSuspendAppCallback)(int (_ASMAPI *saveState)(int flags));
ibool    (_ASMAPI *pWD_getEvent)(event_t *evt,uint mask);
ibool    (_ASMAPI *pWD_peekEvent)(event_t *evt,uint mask);
ibool    (_ASMAPI *pWD_postEvent)(ulong which,uint what,ulong message,ulong modifiers);
void    (_ASMAPI *pWD_flushEvent)(uint mask);
void    (_ASMAPI *pWD_haltEvent)(event_t *evt,uint mask);
int     (_ASMAPI *pWD_setTimerTick)(int ticks);
ibool    (_ASMAPI *pWD_isSafeMode)(void);
void 	(_ASMAPI *p_WD_doSuspendApp)(MSG *orgmsg);
void 	(_ASMAPI *pWD_registerEventProc)(WNDPROC userWndProc);
void 	(_ASMAPI *pWD_registerFullScreenWindow)(HWND hwnd);
void 	(_ASMAPI *pWD_setAppInstance)(HINSTANCE hInstApp);
void 	(_ASMAPI *pWD_getMousePos)(int *x,int *y);
void 	(_ASMAPI *pWD_setMousePos)(int x,int y);
void 	(_ASMAPI *pWD_setMouseCallback)(void (_ASMAPI *mcb)(int x,int y));

/* Dynalinks to PM/Pro functions */

uchar   (PMAPI *pPM_getByte)(uint s, uint o);
ushort  (PMAPI *pPM_getWord)(uint s, uint o);
ulong   (PMAPI *pPM_getLong)(uint s, uint o);
void    (PMAPI *pPM_setByte)(uint s, uint o, uchar v);
void    (PMAPI *pPM_setWord)(uint s, uint o, ushort v);
void    (PMAPI *pPM_setLong)(uint s, uint o, ulong v);
void    (PMAPI *pPM_memcpynf)(void *dst,uint src_s,uint src_o,uint n);
void    (PMAPI *pPM_memcpyfn)(uint dst_s,uint dst_o,void *src,uint n);
uint    (PMAPI *pPM_getBIOSSelector)(void);
uint    (PMAPI *pPM_createSelector)(ulong base,ulong limit);
void    (PMAPI *pPM_freeSelector)(uint sel);
void *  (PMAPI *pPM_mapPhysicalAddr)(long base,long limit);
void    (PMAPI *pPM_mapRealPointer)(uint *sel,uint *off,uint r_seg,uint r_off);
int     (PMAPI *pPM_allocRealSeg)(uint size,uint *sel,uint *off,uint *r_seg,uint *r_off);
void    (PMAPI *pPM_freeRealSeg)(uint sel,uint off);
void    (PMAPI *pPM_callRealMode)(uint seg,uint off, RMREGS *regs,RMSREGS *sregs);
int     (PMAPI *pPM_int86)(int intno, RMREGS *in, RMREGS *out);
int     (PMAPI *pPM_int86x)(int intno, RMREGS *in, RMREGS *out,RMSREGS *sregs);
void    (PMAPI *pPM_availableMemory)(ulong *physical,ulong *total);
int     (PMAPI *pWD_int386)(int intno, RMREGS *in, RMREGS *out);
ibool    (PMAPI *pVF_available)(void);
void *  (PMAPI *pVF_init)(ulong baseAddr,int bankSize,int codeLen,void *bankFunc);
void    (PMAPI *pVF_exit)(void);
void    (PMAPI *pPM_saveDS)(void);
void    (PMAPI *pPM_setBankA)(int bank);
void    (PMAPI *pPM_setBankAB)(int bank);
void    (PMAPI *pPM_setCRTStart)(int x,int y,int waitVRT);
void 	(PMAPI *p_PM_getRMvect)(int intno, long *realisr);
void 	(PMAPI *p_PM_setRMvect)(int intno, long realisr);
void * 	(PMAPI *pPM_getA0000Pointer)(void);
ibool	(PMAPI *pPM_getVESABuf)(uint *len,uint *sel,uint *off,uint *rseg,uint *roff);

#endif	/* !__WIN386__ */

extern drivertype	_VARAPI WINDC_driver;	/* Windowed DC device driver	*/

/*------------------------- Implementation --------------------------------*/

#ifndef	__WIN386__

static void *SafeProcAddress(HINSTANCE hInst,char *name)
/****************************************************************************
*
* Function:		SafeProcAddress
* Returns:		Pointer to loaded procedure address
*
* Description:	Attempts to get the address of a specified procedure, and
*				if it failes we simply bomb out with an error message.
*
****************************************************************************/
{
	char	msg[80];

	FARPROC	pFunc = GetProcAddress(hInst,name);
	if (!pFunc) {
		sprintf(msg,"GetProcAddress(\"%s\") failed!", name);
		MGL_fatalError(msg);
		}
	return pFunc;
}

static  void backslash(char *s)
{
	uint pos = strlen(s);
	if (s[pos-1] != '\\') {
		s[pos] = '\\';
		s[pos+1] = '\0';
		}
}

static  HINSTANCE SafeLoadLibrary(const char *name)
/****************************************************************************
*
* Function:		SafeLoadLibrary
* Returns:		name	- Name of library to load
*
* Description:	Attempts to load the specified library. For some reason
*				when attempting to load some DLL's under Windows 3.1 the
*				Win32s LoadLibrary displays an error message if the DLL
*				cannot be found, so we manually look for it first then try
*				to LoadLibrary it if the file actually exists.
*
****************************************************************************/
{
	WIN32_FIND_DATA	findData;
	HANDLE			hfile;
	char 			buf[_MAX_PATH];

	GetSystemDirectory(buf,sizeof(buf));
	backslash(buf);
	strcat(buf,name);
	if ((hfile = FindFirstFile(buf,&findData)) != INVALID_HANDLE_VALUE) {
		FindClose(hfile);
		return LoadLibrary(name);
		}
	return NULL;
}

#endif

static  void DetectOS(void)
/****************************************************************************
*
* Function:     DetectOS
* Description:	Determine the OS that we are running on.
*
****************************************************************************/
{
	if ((GetVersion() & 0x80000000UL) == 0) {
        _MGL_haveWin31 = false;
        _MGL_haveWin95 = false;
		_MGL_haveWinNT = true;
		}
	else {
		int verMajor = GetVersion() & 0xFF;
		if (verMajor >= 4) {
			_MGL_haveWin31 = false;
			_MGL_haveWin95 = true;
			_MGL_haveWinNT = false;
			}
		else {
			_MGL_haveWin31 = true;
			_MGL_haveWin95 = false;
			_MGL_haveWinNT = false;
			}
        }
}

static  ibool LoadWinG(void)
/****************************************************************************
*
* Function:		LoadWinG
* Returns:		True if WinG is loaded, false if not.
*
* Description:	Attempts to dynamically load WinG and create the pointers
*				to all the required WinG functions. If we cannot load WinG,
*				we return false.
*
****************************************************************************/
{
	if (!hInstWinG) {
		DetectOS();
#ifndef	__WIN386__
		if ((hInstWinG = LoadLibrary("wing32.dll")) == NULL)
			return false;
		pWinGCreateDC 			= SafeProcAddress(hInstWinG,"WinGCreateDC");
		pWinGCreateBitmap 		= SafeProcAddress(hInstWinG,"WinGCreateBitmap");
		pWinGSetDIBColorTable 	= SafeProcAddress(hInstWinG,"WinGSetDIBColorTable");
		pWinGBitBlt 			= SafeProcAddress(hInstWinG,"WinGBitBlt");
		pWinGStretchBlt 		= SafeProcAddress(hInstWinG,"WinGStretchBlt");
#else
		if (!WinG_initWin386())
			return false;
		hInstWinG = (HINSTANCE)1;
#endif
		_MGL_bltType = BLT_WinG;
		}
	return true;
}

static  ibool LoadCreateDIBSection(void)
/****************************************************************************
*
* Function:		LoadCreateDIBSection
* Returns:		True if CreateDIBSection is available
*
* Description:	Attempts to dynamically load WinG and create the pointers
*				to all the required WinG functions. If we cannot load WinG,
*				we return false.
*
****************************************************************************/
{
#ifndef	__WIN386__
	if (!hGDI) {
		DetectOS();
		if ((hGDI = GetModuleHandle("gdi32.dll")) == NULL)
			return false;
		if ((hUSER = GetModuleHandle("user32.dll")) == NULL)
			return false;
		pCreateDIBSection = SafeProcAddress(hGDI,"CreateDIBSection");
		pSetDIBColorTable = SafeProcAddress(hGDI,"SetDIBColorTable");
		pChangeDisplaySettings = SafeProcAddress(hUSER,"ChangeDisplaySettingsA");
		pEnumDisplaySettings = SafeProcAddress(hUSER,"EnumDisplaySettingsA");
		_MGL_bltType = BLT_CreateDIBSection;
        }
	return true;
#else
	return false;
#endif
}

static  void LoadWinDirect(void)
/****************************************************************************
*
* Function:		LoadWinDirect
* Parameters:	ver	- System version number
*
* Description:	Attempts to dynamically load the WinDirect DLL's and
*				create pointers to all the routines that we need. If we
*				cannot load WinDirect, or we are running under Windows NT
*				then we simply dont load it.
*
****************************************************************************/
{
	if (!hInstWinD) {
		char *noWinDirect;
		DetectOS();
		noWinDirect = getenv("MGL_NO_WINDIRECT");
		if (noWinDirect && atoi(noWinDirect) != 0) {
			_MGL_haveWinDirect = false;
			return;
			}
#ifndef	NO_ASSEMBLER
		if (_MGL_haveWinNT)
			return;
		if ((hInstWinD = LoadLibrary(WDIRECT_DLL)) == NULL) {
			_MGL_haveWinDirect = false;
			return;
			}
		pWD_startFullScreen		= SafeProcAddress(hInstWinD,"_WD_startFullScreen");
		pWD_changeResolution	= SafeProcAddress(hInstWinD,"_WD_changeResolution");
		pWD_restoreGDI 			= SafeProcAddress(hInstWinD,"_WD_restoreGDI");
		pWD_setSuspendAppCallback = SafeProcAddress(hInstWinD,"_WD_setSuspendAppCallback");
		pWD_getEvent 			= SafeProcAddress(hInstWinD,"_WD_getEvent");
		pWD_peekEvent 			= SafeProcAddress(hInstWinD,"_WD_peekEvent");
		pWD_postEvent 			= SafeProcAddress(hInstWinD,"_WD_postEvent");
		pWD_flushEvent 			= SafeProcAddress(hInstWinD,"_WD_flushEvent");
		pWD_haltEvent 			= SafeProcAddress(hInstWinD,"_WD_haltEvent");
		pWD_setTimerTick 		= SafeProcAddress(hInstWinD,"_WD_setTimerTick");
		pWD_isSafeMode			= SafeProcAddress(hInstWinD,"_WD_isSafeMode");
		p_WD_doSuspendApp 		= SafeProcAddress(hInstWinD,"__WD_doSuspendApp");
		pWD_registerEventProc   = SafeProcAddress(hInstWinD,"_WD_registerEventProc");
		pWD_registerFullScreenWindow = SafeProcAddress(hInstWinD,"_WD_registerFullScreenWindow");
		pWD_setAppInstance   	= SafeProcAddress(hInstWinD,"_WD_setAppInstance");
		pWD_getMousePos   		= SafeProcAddress(hInstWinD,"_WD_getMousePos");
		pWD_setMousePos   		= SafeProcAddress(hInstWinD,"_WD_setMousePos");
		pWD_setMouseCallback   	= SafeProcAddress(hInstWinD,"_WD_setMouseCallback");
		pPM_getByte				= SafeProcAddress(hInstWinD,"_PM_getByte");
		pPM_getWord				= SafeProcAddress(hInstWinD,"_PM_getWord");
		pPM_getLong				= SafeProcAddress(hInstWinD,"_PM_getLong");
		pPM_setByte				= SafeProcAddress(hInstWinD,"_PM_setByte");
		pPM_setWord				= SafeProcAddress(hInstWinD,"_PM_setWord");
		pPM_setLong				= SafeProcAddress(hInstWinD,"_PM_setLong");
		pPM_memcpynf			= SafeProcAddress(hInstWinD,"_PM_memcpynf");
		pPM_memcpyfn			= SafeProcAddress(hInstWinD,"_PM_memcpyfn");
		pPM_getBIOSSelector		= SafeProcAddress(hInstWinD,"_PM_getBIOSSelector");
		pPM_createSelector		= SafeProcAddress(hInstWinD,"_PM_createSelector");
		pPM_freeSelector		= SafeProcAddress(hInstWinD,"_PM_freeSelector");
		pPM_mapPhysicalAddr		= SafeProcAddress(hInstWinD,"_PM_mapPhysicalAddr");
		pPM_mapRealPointer		= SafeProcAddress(hInstWinD,"_PM_mapRealPointer");
		pPM_allocRealSeg		= SafeProcAddress(hInstWinD,"_PM_allocRealSeg");
		pPM_freeRealSeg			= SafeProcAddress(hInstWinD,"_PM_freeRealSeg");
		pPM_callRealMode		= SafeProcAddress(hInstWinD,"_PM_callRealMode");
		pPM_int86				= SafeProcAddress(hInstWinD,"_PM_int86");
		pPM_int86x				= SafeProcAddress(hInstWinD,"_PM_int86x");
		pPM_availableMemory		= SafeProcAddress(hInstWinD,"_PM_availableMemory");
		pWD_int386				= SafeProcAddress(hInstWinD,"_WD_int386");
		pVF_available			= SafeProcAddress(hInstWinD,"_VF_available");
		pVF_init				= SafeProcAddress(hInstWinD,"_VF_init");
		pVF_exit				= SafeProcAddress(hInstWinD,"_VF_exit");
		pPM_saveDS				= SafeProcAddress(hInstWinD,"_PM_saveDS");
		pPM_setBankA			= SafeProcAddress(hInstWinD,"_PM_setBankA");
		pPM_setBankAB			= SafeProcAddress(hInstWinD,"_PM_setBankAB");
		pPM_setCRTStart			= SafeProcAddress(hInstWinD,"_PM_setCRTStart");
		p_PM_getRMvect			= SafeProcAddress(hInstWinD,"__PM_getRMvect");
		p_PM_setRMvect			= SafeProcAddress(hInstWinD,"__PM_setRMvect");
		pPM_getA0000Pointer		= SafeProcAddress(hInstWinD,"_PM_getA0000Pointer");
        pPM_getVESABuf			= SafeProcAddress(hInstWinD,"_PM_getVESABuf");
		_MGL_haveWinDirect = true;
#endif
		}
}

#ifndef	__WIN386__
#define	AUTOPLAY_DRIVE_CDROM	0x20
const char *szAutoPlayKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";
const char *szAutoPlayValue = "NoDriveTypeAutoRun";

/* {secret} */
void _MGL_disableAutoPlay(void)
/****************************************************************************
*
* Function:		_MGL_disableAutoPlay
*
* Description:	Temporarily disables AutoPlay operation while we are running
*				in fullscreen graphics modes.
*
****************************************************************************/
{
	DWORD	dwAutoPlay,dwSize = sizeof(dwAutoPlay);
	HKEY	hKey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,szAutoPlayKey,0,KEY_EXECUTE | KEY_WRITE,&hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey,szAutoPlayValue,NULL,NULL,(void*)&dwAutoPlay,&dwSize);
		dwAutoPlay |= AUTOPLAY_DRIVE_CDROM;
		RegSetValueEx(hKey,szAutoPlayValue,0,REG_DWORD,(void*)&dwAutoPlay,dwSize);
		RegCloseKey(hKey);
		}
}

/* {secret} */
void _MGL_restoreAutoPlay(void)
/****************************************************************************
*
* Function:		_MGL_restoreAutoPlay
*
* Description:	Re-enables AutoPlay operation when we return to regular
*				GDI mode.
*
****************************************************************************/
{
	DWORD	dwAutoPlay,dwSize = sizeof(dwAutoPlay);
	HKEY	hKey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,szAutoPlayKey,0,KEY_EXECUTE | KEY_WRITE,&hKey) == ERROR_SUCCESS) {
		RegQueryValueEx(hKey,szAutoPlayValue,NULL,NULL,(void*)&dwAutoPlay,&dwSize);
		dwAutoPlay &= ~AUTOPLAY_DRIVE_CDROM;
		RegSetValueEx(hKey,szAutoPlayValue,0,REG_DWORD,(void*)&dwAutoPlay,dwSize);
		RegCloseKey(hKey);
		}
}
#else
/* {secret} */
void _MGL_disableAutoPlay(void) {}
/* {secret} */
void _MGL_restoreAutoPlay(void) {}
#endif


static  int _cdecl _MGL_suspendAppProc(int flags)
/****************************************************************************
*
* Function:		_MGL_suspendAppProc
* Parameters:	flags	- Flags indicating what to do
*
* Description:	This is the WinDirect suspend application callback function
*				for all MGL routine. This function takes care of most of
*				the important stuff such as saving and restoring the state
*				of the hardware etc.	
*
****************************************************************************/
{
	int				retcode;
	MGLDC			*dc = _LST_first(_MGL_dispDCList);
	static MGLDC	*prevDC;

	if (flags == MGL_DEACTIVATE) {
		if (_MGL_suspendApp) {
			if ((retcode = _MGL_suspendApp(dc,flags)) == MGL_NO_DEACTIVATE)
				return WD_NO_DEACTIVATE;
			}
		else
			retcode = WD_SUSPEND_APP;
		inSuspendApp = true;
		_MS_saveCursor();
		prevDC = _MGL_dcPtr;
		if (_MGL_isOpenGLDC(dc))
			MGL_glMakeCurrent(NULL);
		else
			MGL_makeCurrentDC(NULL);
		dc->v->d.restoreTextMode(dc);
		_MGL_restoreAutoPlay();
		return retcode;
		}
	if (flags == MGL_REACTIVATE) {
		int oldActivePage = MGL_getActivePage(dc);
		int oldVisualPage = MGL_getVisualPage(dc);
		dc->v->d.restoreGraphMode(dc);
		MGL_makeCurrentDC(dc);
		if (_MGL_isOpenGLDC(dc))
			MGL_glMakeCurrent(prevDC);
		else
			MGL_makeCurrentDC(prevDC);
		MGL_realizePalette(dc,MGL_getPaletteSize(dc),0,false);
		MGL_setActivePage(dc,oldActivePage);
		MGL_setVisualPage(dc,oldVisualPage,false);
		inSuspendApp = false;
		if (_MGL_suspendApp)
			retcode = _MGL_suspendApp(dc,flags);
		else
        	retcode = WD_SUSPEND_APP;
		_MS_restoreCursor();
		_MGL_disableAutoPlay();
		return retcode;
		}
	return MGL_SUSPEND_APP;
}

#ifndef	__WIN386__
static void ClearMessageQueue(void)
/****************************************************************************
*
* Function:     ClearMessageQueue
*
* Description:  Clears the message queue of all waiting messages.
*
****************************************************************************/
{
	MSG msg;
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
}

static void RestoreFullScreen(void)
/****************************************************************************
*
* Function:     RestoreFullScreen
*
* Description:	Reactivate all the surfaces for DirectDraw and set the
*				system back up for fullscreen rendering.  
*
****************************************************************************/
{
	static	ibool firstTime = true;

    if (firstTime) {
		int 	oldActivePage,oldVisualPage;
        ibool	isActive;
		MGLDC	*dc = _LST_first(_MGL_dispDCList);
		MGLDC	*cntDC = _MGL_dcPtr;

		/* Clear the message queue while waiting for the surfaces to be
		 * restored.
		 */
		firstTime = false;
		while (1) {
			/* Restore all the surfaces that we have lost */
			ClearMessageQueue();
			isActive = (GetActiveWindow() == _MGL_hwndFullScreen);
			if (isActive) {
            	if (_MGL_lpPrimary) {
					if (IDirectDrawSurface_Restore(_MGL_lpPrimary) != DD_OK)
						isActive = false;
					else if (_MGL_lpOffscreen)
						IDirectDrawSurface_Restore(_MGL_lpOffscreen); 
					}
				if (_MGL_winDirectMode == -1)
					_WIN_restoreFullScreenMode(dc);
				if (_MGL_isFSOpenGLDC(dc))
					fglSetFocus(true);
				if (isActive) {
					/* Restore our fullscreen window and the users display */
					if (dc->mi.bitsPerPixel == 8) {
						HDC hdc = GetDC(NULL);
						SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
						ReleaseDC(NULL,hdc);
						}
					oldActivePage = MGL_getActivePage(dc);
					oldVisualPage = MGL_getVisualPage(dc);
					MGL_makeCurrentDC(NULL);
					MGL_makeCurrentDC(dc);
					MGL_makeCurrentDC(cntDC);
					MGL_realizePalette(dc,MGL_getPaletteSize(dc),0,false);
					if (_MGL_lpPrimary)
						_DD_enumerateSurfaces(dc);
					MGL_setActivePage(dc,oldActivePage);
					MGL_setVisualPage(dc,oldVisualPage,false);
					if (_MGL_suspendApp)
						_MGL_suspendApp(dc,MGL_REACTIVATE);
					_MS_restoreState();
					_MGL_disableAutoPlay();
					_MGL_backInGDI = false;
					waitActive = false;
					firstTime = true;
					return;
					}
				}
			Sleep(200);
			}
		}
}

/* {secret} */
void _DD_doSuspendApp(void)
/****************************************************************************
*
* Function:     _DD_doSuspendApp
* Parameters:	flags	- Flags indicating which key started the suspend
*
* Description:  Suspends the application by switching back to the GDI
*				desktop, allowing normal application code to be processed,
*				and then waiting for the application activate command to
*				bring us back to fullscreen mode with our window minimised.
*
*				This version only gets called if we have not captured the
*				screen switch in our activate message loops and will
*				occur if we lose a surface for some reason while rendering.
*				This should not normally happen, but it is included just to
*				be sure (note that this code will always spin loop, and we
*				cannot disable the spin looping from this version).
*
****************************************************************************/
{
	static	ibool firstTime = true;
	MGLDC	*dc = _LST_first(_MGL_dispDCList);

	if (firstTime) {
		if (_MGL_suspendApp)
			_MGL_suspendApp(dc,MGL_DEACTIVATE);
		if (_MGL_isFSOpenGLDC(dc))
			fglSetFocus(false);
		_MS_saveState();
		firstTime = false;
		if (dc->mi.bitsPerPixel == 8) {
        	HDC hdc = GetDC(NULL);
			SetSystemPaletteUse(hdc, SYSPAL_STATIC);
            ReleaseDC(NULL,hdc);
			}
		_MGL_restoreAutoPlay();
		_MGL_backInGDI = true;
		}
	RestoreFullScreen();
    firstTime = true;
}

/* {secret} */
void _DD_deactivate(void)
/****************************************************************************
*
* Function:     _DD_deactivate
* Parameters:	flags	- Flags indicating which key started the suspend
*
* Description:  Suspends the application by switching back to the GDI
*				desktop, allowing normal application code to be processed,
*				and then waiting for the application activate command to
*				bring us back to fullscreen mode with our window minimised.
*
****************************************************************************/
{
	MGLDC	*dc = _LST_first(_MGL_dispDCList);
	int		retCode = MGL_SUSPEND_APP;

	if (_MGL_backInGDI)
		return;
	if (_MGL_suspendApp)
		retCode = _MGL_suspendApp(dc,MGL_DEACTIVATE);
	if (_MGL_isFSOpenGLDC(dc))
		fglSetFocus(false);
	_MS_saveState();
	if (dc->mi.bitsPerPixel == 8) {
		HDC hdc = GetDC(NULL);
		SetSystemPaletteUse(hdc, SYSPAL_STATIC);
		ReleaseDC(NULL,hdc);
		}
	if (_MGL_winDirectMode == -1)
		_WIN_restoreDisplayMode(true);
	_MGL_restoreAutoPlay();
	_MGL_backInGDI = true;

	/* Now process messages normally until we are re-activated */
	waitActive = true;
	if (retCode != MGL_NO_SUSPEND_APP) {
		while (waitActive) {
			ClearMessageQueue();
			Sleep(200);
			}
		}
}

/* {secret} */
LONG WINAPI DD_winProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam)
/****************************************************************************
*
* Function:		DD_winProc
*
* Description:  Main Window proc for the full screen DirectDraw Window that
*				we create while running in full screen mode. Here we
*				capture all mouse and keyboard events for the window and
*				plug them into our event queue.
*
****************************************************************************/
{
	int		isAlt,doSwitch;
	int		where_x,where_y;

	switch (msg) {
		case WM_SYSCHAR:
			/* Stop Alt-Space from pausing our game */
			return 0;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (HIWORD(lParam) & KF_REPEAT) {
				if (msg == WM_SYSKEYDOWN)
                	return 0;
				break;
				}
			/* Fall through for keydown events */
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (_MGL_winDirectMode == -1) {
            	/* Handle Alt-Tabbing for fullscreen DIB modes */
				isAlt = (HIWORD(lParam) & KF_ALTDOWN);
				doSwitch = false;
				if (isAlt && wParam == VK_TAB)
					doSwitch = true;			/* Alt-Tab */
				else if (isAlt && wParam == VK_ESCAPE)
					doSwitch = true;			/* Alt-Esc */
				else if (wParam == VK_ESCAPE) {
					if (GetKeyState(VK_CONTROL) & 0x8000U)
						doSwitch = true;		/* Ctrl-Esc */
					}
				if (doSwitch) {
					MGLDC *dc = _LST_first(_MGL_dispDCList);
					/* Check the app wants to block switching and block it */
					if (!_MGL_haveWinNT && (_MGL_suspendApp && (_MGL_suspendApp(dc,MGL_DEACTIVATE) == MGL_NO_DEACTIVATE)))
						return 0;
					ShowWindow(_MGL_hwndFullScreen,SW_MINIMIZE);
					}
				}
			if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) {
				if ((HIWORD(lParam) & KF_ALTDOWN) && wParam == VK_RETURN)
                	break;
				/* We ignore the remainder of the sysyetm keys to stop the
				 * system menu from being activated from the keyboard and pausing
				 * our app while fullscreen (ie: pressing the Alt key).
				 */
				return 0;
				}
			break;
		case WM_SYSCOMMAND:
			switch (wParam & ~0x0F) {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
					/* Ignore screensaver requests in fullscreen modes */
					return 0;
				}
			break;
		case WM_MOUSEMOVE:
			where_x = (LOWORD(lParam) * _MGL_xRes) / _MGL_deskX;
			where_y = (HIWORD(lParam) * _MGL_yRes) / _MGL_deskY;
			if (_MGL_mx != where_x || _MGL_my != where_y)
				_MS_moveCursor(_MGL_mx = where_x,_MGL_my = where_y);
			break;
		case WM_SIZE:
			if (waitActive && _MGL_backInGDI && (wParam != SIZE_MINIMIZED)) {
				/* Start the re-activation process */
				PostMessage(hwnd,WM_DO_SUSPEND_APP,WM_MGL_RESTORE_FULLSCREEN,0);
				}
			else if (!waitActive && _MGL_fullScreen && !_MGL_backInGDI && (wParam == SIZE_MINIMIZED)) {
				/* Start the de-activation process */
				PostMessage(hwnd,WM_DO_SUSPEND_APP,WM_MGL_DEACTIVATE,0);
				}
			break;
		case WM_DO_SUSPEND_APP:
			switch (wParam) {
				case WM_MGL_RESTORE_FULLSCREEN:
					RestoreFullScreen();
					break;
				case WM_MGL_DEACTIVATE:
					_DD_deactivate();
					break;
				}
			return 0;
		}
	if (oldWndProc)
		return oldWndProc(hwnd,msg,wParam,lParam);
	else if (_MGL_userEventProc)
		return _MGL_userEventProc(hwnd,msg,wParam,lParam);
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

/* {secret} */
LONG WINAPI DD_subWinProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam)
/****************************************************************************
*
* Function:		DD_subWinProc
*
* Description:  Main Window proc for the full screen WinDirect Window that
*				we create while running in full screen mode. Here we
*				capture all mouse and keyboard events for the window and
*				plug them into our event queue.
*
****************************************************************************/
{
	int		isAlt,doSwitch;

	switch (msg) {
		case WM_KEYUP:
		case WM_SYSKEYUP:
			/* Filter out the keyup events in DirectDraw that can cause us to
			 * lose our surfaces when a switch occurs.
			 */
			isAlt = (HIWORD(lParam) & KF_ALTDOWN);
			doSwitch = false;
			if (isAlt && wParam == VK_TAB)
				doSwitch = true;			/* Alt-Tab */
			else if (isAlt && wParam == VK_ESCAPE)
				doSwitch = true;			/* Alt-Esc */
			else if (wParam == VK_ESCAPE) {
				if (GetKeyState(VK_CONTROL) & 0x8000U)
					doSwitch = true;		/* Ctrl-Esc */
				}
			if (doSwitch) {
				MGLDC *dc = _LST_first(_MGL_dispDCList);
				/* Check the app wants to block switching and block it */
				if (!_MGL_haveWinNT && (_MGL_suspendApp && (_MGL_suspendApp(dc,MGL_DEACTIVATE) == MGL_NO_DEACTIVATE)))
					return 0;
				}
			break;
		}
	return _DD_orgWinProc(hwnd,msg,wParam,lParam);
}

/* {secret} */
void _DD_subClassWindow(HWND hwnd)
/****************************************************************************
*
* Function:		_DD_subClassWindow
* Parameters:	hwnd	- Handle of DirectDraw window to subclass
*
* Description:	Subclasses the DirectDraw window after DirectDraw has
*				gone into exclusive mode so that we can stop DirectDraw
*				from allowing us to Alt-Tab away from the application. Note
*				that this may get called after the window has already been
*				subclassed once.
*
****************************************************************************/
{
	WNDPROC	oldWinProc = (WNDPROC)SetWindowLong(hwnd,
		GWL_WNDPROC, (LPARAM)DD_subWinProc);
	if (!_DD_orgWinProc)
		_DD_orgWinProc = oldWinProc;
}

/* {secret} */
void _DD_unsubClassWindow(HWND hwnd)
/****************************************************************************
*
* Function:		_DD_unsubClassWindow
* Parameters:	hwnd	- Handle of DirectDraw window to subclass
*
* Description:	Subclasses the DirectDraw window after DirectDraw has
*				gone into exclusive mode so that we can stop DirectDraw
*				from allowing us to Alt-Tab away from the application. Note
*				that this may get called after the window has already been
*				subclassed once.
*
****************************************************************************/
{
	SetWindowLong(hwnd,GWL_WNDPROC, (LPARAM)_DD_orgWinProc);
	_DD_orgWinProc = NULL;
}

/* {secret} */
HWND _DD_convertUserWindow(HWND hwnd,int width,int height)
/****************************************************************************
*
* Function:		_DD_convertUserWindow
* Parameters:	hwnd	- User window to convert
*				width	- Window of the fullscreen window
*				height	- Height of the fullscreen window
* Returns:		Handle to converted fullscreen Window.
*
* Description:  This function takes the original user window handle and
*				modifies the size, position and attributes for the window
*				to convert it into a fullscreen window that we can use.
*
****************************************************************************/
{
	RECT	window;

	GetWindowRect(hwnd,&window);
    oldWinPosX = window.left;
	oldWinPosY = window.top;
    oldWinSizeX = window.right - window.left;
    oldWinSizeY = window.bottom - window.top;
	oldWndStyle = SetWindowLong(hwnd,GWL_STYLE,WS_POPUP | WS_SYSMENU);
	oldExWndStyle = SetWindowLong(hwnd,GWL_EXSTYLE,WS_EX_APPWINDOW);
    ShowWindow(hwnd,SW_SHOW);
	MoveWindow(hwnd,0,0,width,height,TRUE);
	SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	oldWndProc = (WNDPROC)SetWindowLong(hwnd,GWL_WNDPROC, (LPARAM)DD_winProc);
	return hwnd;
}

/* {secret} */
void _DD_restoreUserWindow(HWND hwnd)
/****************************************************************************
*
* Function:		_DD_restoreUserWindow
* Parameters:	hwnd	- User window to convert
*
* Description:  This function restores the original attributes of the
*				user window and put's it back into it's original state
*				before it was converted to a fullscreen window.
*
****************************************************************************/
{
	SetWindowLong(hwnd,GWL_WNDPROC, (LPARAM)oldWndProc);
	SetWindowLong(hwnd,GWL_EXSTYLE,oldExWndStyle);
	SetWindowLong(hwnd,GWL_STYLE,oldWndStyle);
	SetWindowPos(hwnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow(hwnd,SW_SHOW);
	MoveWindow(hwnd,oldWinPosX,oldWinPosY,oldWinSizeX,oldWinSizeY,TRUE);
	oldWndProc = NULL;
}

/* {secret} */
void _DD_setMousePos(int x,int y)
/****************************************************************************
*
* Function:		_DD_setMousePos
* Parameters:	x,y	- New mouse coodinate
*
* Description:	Updates the internal variables for the mouse cursor
*				coordinate and also calls Windows to move the mouse cursor.
*
****************************************************************************/
{
	/* Scale coordinates up to desktop coordinates first */
	int scaledX = (x * _MGL_deskX) / _MGL_xRes;
	int scaledY = (y * _MGL_deskY) / _MGL_yRes;

	/* Scale coordinates back to screen coordinates again */
	_MGL_mx = (scaledX * _MGL_xRes) / _MGL_deskX;
	_MGL_my = (scaledY * _MGL_yRes) / _MGL_deskY;
	_MS_moveCursor(_MGL_mx,_MGL_my);
	SetCursorPos(scaledX,scaledY);
}

static  HRESULT CALLBACK SizeMemCallBack(LPDDSURFACEDESC s, LPVOID lpv)
/****************************************************************************
*
* Function:		SizeMemCallBack
* Parameters:	s	- Pointer to surface description for this mode
*				lpv	- Unused callback pointer
*
* Description:	Memory sizing enumeration callback, called for every display
*				mode that DirectDraw supports. Because DirectDraw fucks up 
*				the amount of available video memory (under NT it reports
*				0!), we check the list of available modes and find the 
*				video memory size that is required to hold the largest
*				available display mode and up the memory size to that if it
*				is lower.
*
****************************************************************************/
{
	/* Ignore ModeX surfaces */
	if (!(s->ddsCaps.dwCaps & DDSCAPS_MODEX)) {
    	/* Compute the amount of video memory required for this mode */
		ulong size = s->dwWidth * s->dwHeight;
		switch (s->ddpfPixelFormat.b.dwRGBBitCount) {
			case 16:	size *= 2;	break;
			case 24:	size *= 3;	break;
			case 32:	size *= 4;	break;
			}

		/* Round the memory up to nearest Mb mark */
		if (size > (4096 * 1024L))
			size = 8192 * 1024L;
		else if (size > (2048 * 1024L))
			size = 4096 * 1024L;
		else if (size > (1024 * 1024L))
			size = 2048 * 1024L;
		else if (size > (512 * 1024L))
			size = 1024 * 1024L;
		if (size > _MGL_ddCaps.dwVidMemTotal)
			_MGL_ddCaps.dwVidMemTotal = size;
		}
	return DDENUMRET_OK;
}
#endif	/* !__WIN386__ */

static  ibool LoadDirectDraw(void)
/****************************************************************************
*
* Function:		LoadDirectDraw
* Parameters:	ver	- System version number
*
* Description:	Attempts to dynamically load the DirectDraw DLL's and
*				create pointers to all the routines that we need.
*
****************************************************************************/
{
#ifndef	__WIN386__
	if (!hInstDD) {
		WNDCLASS	cls;
		HDC			hdc;
		int			bits;

		hdc = GetDC(NULL);
		bits = GetDeviceCaps(hdc,BITSPIXEL);
		ReleaseDC(NULL,hdc);
		if (bits < 8)
        	return false;
		DetectOS();
		if ((hInstDD = SafeLoadLibrary("ddraw.dll")) == NULL)
			return false;
		pDirectDrawCreate		= (void*)GetProcAddress(hInstDD,"DirectDrawCreate");
		pDirectDrawCreateClipper= (void*)GetProcAddress(hInstDD,"DirectDrawCreateClipper");
		if (!pDirectDrawCreate || !pDirectDrawCreateClipper)
			return false;

		/* Create a Window class for the fullscreen window in here, since we need
		 * to register one that will do all our event handling for us.
		 */
		cls.hCursor        	= LoadCursor(NULL,IDC_ARROW);
		cls.hIcon          	= LoadIcon(_MGL_hInst,MAKEINTRESOURCE(1));
		cls.lpszMenuName   	= NULL;
		cls.lpszClassName  	= _DD_szWinClassName;
		cls.hbrBackground	= GetStockObject(BLACK_BRUSH);
		cls.hInstance      	= _MGL_hInst;
		cls.style          	= CS_DBLCLKS;
		cls.lpfnWndProc     = (LPVOID)DD_winProc;
		cls.cbWndExtra     	= 0;
		cls.cbClsExtra     	= 0;
		if (!RegisterClass(&cls))
			MGL_fatalError("Unable to register Window Class!");
		}

	if (!_MGL_lpDD) {
		/* Create the DirectDraw object and get it's capabilities */
		if (DirectDrawCreate(NULL, &_MGL_lpDD1, NULL) != DD_OK) {
			_MGL_lpDD1 = NULL;
			return false;
			}
		/* Create the DirectDraw2 object */
		if (IDirectDraw_QueryInterface(_MGL_lpDD1,&IID_IDirectDraw2, (void*)&_MGL_lpDD) != DD_OK) {
			_MGL_lpDD = NULL;
            IDirectDraw_Release(_MGL_lpDD1);
			return false;
            }

		/* Get the capabilities of fullscreen, exclusive mode operation */
		_MGL_ddCaps.dwSize = sizeof(DDCAPS);
		_MGL_ddCaps.dwCaps = 0;
		IDirectDraw2_GetCaps(_MGL_lpDD,&_MGL_ddCaps,NULL);

		/* Find the available memory. Note that DirectDraw drivers probably
		 * need to use a small amount of offscreen memory at the end of
		 * display memory for hardware cursors etc, we we subtract a small
		 * amount of memory off the total memory to avoid this problem.
		 */
		IDirectDraw2_EnumDisplayModes(_MGL_lpDD,0,NULL,NULL,SizeMemCallBack);
		_MGL_ddCaps.dwVidMemTotal -= 8 * 1024L;
		}
	return true;
#else
	return false;
#endif
}

/* {secret} */
void MGLPC_init(void)
/****************************************************************************
*
* Function:		MGLPC_init
*
* Description:	Initialise the basic PC display device driver stuff. If
*				we have not already done so, we obtain a pointer to the
*				start of video memory and the BIOS data area.
*
****************************************************************************/
{
	GET_DEFAULT_CW();
	if (!_MGL_hInst)
		_MGL_hInst = GetModuleHandle(NULL);
	LoadDirectDraw();
	LoadWinDirect();
	LoadCreateDIBSection();
	if (_MGL_haveWinDirect)
		WD_setSuspendAppCallback(_MGL_suspendAppProc);
	if (!_MGL_biosSel && _MGL_haveWinDirect) {
		_MGL_biosSel = PM_getBIOSSelector();
        VBE_init();
		PM_saveDS();
		}
	RESET_DEFAULT_CW();
}

/* {secret} */
void MGLPC_destroyFullScreenMode(void)
/****************************************************************************
*
* Function:		MGLPC_destroyFullScreenMode
*
* Description:	Destroys the fullscreen window when switching back to
*				the desktop or creating a new windowed mode.
*
****************************************************************************/
{
	if (_MGL_hwndFullScreen) {
		if (_MGL_winDirectMode == 1) {
			WD_restoreGDI();
			WD_registerFullScreenWindow(NULL);
			_MGL_winDirectMode = 0;
			}
#ifndef __WIN386__
		else {
			if (_MGL_winDirectMode == -1) {
				ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
				}
			else {
				_DD_unsubClassWindow(_MGL_hwndFullScreen);
				IDirectDraw2_SetCooperativeLevel(_MGL_lpDD,_MGL_hwndFullScreen,DDSCL_NORMAL);
				IDirectDraw2_RestoreDisplayMode(_MGL_lpDD);
				}
			if (_MGL_hwndUser)
				_DD_restoreUserWindow(_MGL_hwndFullScreen);
			else
				DestroyWindow(_MGL_hwndFullScreen);

			/* Show and enable the main application window */
			if (_MGL_hwndMain) {
				EnableWindow(_MGL_hwndMain,TRUE);
				ShowWindow(_MGL_hwndMain,SW_SHOW);
				SetActiveWindow(_MGL_hwndMain);
				SetWindowPos(_MGL_hwndMain,HWND_TOP,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
				SetForegroundWindow(_MGL_hwndMain);
				}

			/* Restore the arrow cursor */
			ShowCursor(TRUE);
			SetCursor(LoadCursor(NULL,IDC_ARROW));
			}
#endif
		_MGL_restoreAutoPlay();
		}
	RESET_DEFAULT_CW();
	_MGL_hwndFullScreen = NULL;
}

/* {secret} */
void MGLPC_startFullScreen(int xRes,int yRes)
/****************************************************************************
*
* Function:		MGLPC_startFullScreen
* Parameters:	xRes,yRes	- Resolution of video mode to be started
*
* Description:	Starts the full screen video mode by shutting down GDI
*				using the WinDirect package. Note that during the suspend
*				app callback, this is handled by WinDirect not us.
*
****************************************************************************/
{
	if (!inSuspendApp) {
		HDC	hdc = GetDC(NULL);
		WD_registerFullScreenWindow(_MGL_hwndUser);
		WD_registerEventProc(_MGL_userEventProc);
		WD_setAppInstance(_MGL_hInst);

		/* Destroy DirectDraw mode if we are currently running in it */
		if (_MGL_winDirectMode != 1)
			MGLPC_destroyFullScreenMode();

		/* Only create the fullscreen Window if we currently do not have it
		 * created. Hence if we are switching between fullscreen modes we never
		 * actually switch out of WinDirect mode and we will never see the GDI
		 * desktop until we really want to switch back to it.
		 */
		if (!_MGL_hwndFullScreen) {
			_MGL_hwndFullScreen = WD_startFullScreen(_MGL_hwndMain,xRes,yRes);
			_MGL_winDirectMode = 1;
			}
		else
        	WD_changeResolution(xRes,yRes);
#ifdef	__WINDOWS32__
		SetForegroundWindow(_MGL_hwndFullScreen);
#endif

		/* Initialise the fullscreen event handling routines */
		_EVT_init(_MGL_hwndFullScreen);

		/* Save the Windows desktop resolution so we can scale coordinates */
		_MGL_deskX = GetDeviceCaps(hdc,HORZRES);
		_MGL_deskY = GetDeviceCaps(hdc,VERTRES);
		_MGL_xRes = xRes;
		_MGL_yRes = yRes;
		_MGL_fullScreen = true;
		ReleaseDC(NULL,hdc);
		RESET_DEFAULT_CW();
		}
}

/* {secret} */
void MGLPC_disableFullScreen(void)
/****************************************************************************
*
* Function:		MGLPC_disableFullScreen
*
* Description:	Restores normal system operation after going full screen.
*				Note that during the suspend app callback, this is handled
*				by WinDirect not us.
*
****************************************************************************/
{
	if (!inSuspendApp) {
		WD_registerEventProc(NULL);
		_MGL_fullScreen = false;
		}
}

/*******************************************************************************
DESCRIPTION:
Declare a fatal error and exit gracefully.

HEADER:
mgraph.h

PARAMETERS:
msg	- Message to display

REMARKS:
A fatal internal error has occurred, so we shutdown the graphics systems, display
the error message and quit. You should call this function to display your own
internal fatal errors.
*******************************************************************************/
void MGLAPI MGL_fatalError(
	const char *msg)
{
	MGL_exit();
	MessageBox(NULL, msg, "MGL Fatal Error!", MB_ICONEXCLAMATION);
	exit(EXIT_FAILURE);
}

/*******************************************************************************
DESCRIPTION:
Create a new windowed device context.

HEADER:
mglwin.h

PARAMETERS:
hwnd	- Window handle with which to associate new device context

RETURNS:
Pointer to the allocated windowed device context, or NULL if not enough memory.

REMARKS:
Creates a new windowed device context for drawing information into a window on
the Windows desktop. When you create a Windowed device context, you associate 
it with a standard Windows HWND for the window that you wish MGL to display 
it’s output on. Windowed device contexts are special device contexts in that you 
cannot directly access the surface for the device, nor can you actually use the MGL 
rasterizing functions to draw on the device surface. The only rasterizing functions
supported are the MGL_bitBlt and MGL_stretchBlt for blt’ing data from memory device contexts to the window on
the desktop.

However in order to change the color palette values for the data copied to the
window, you must use the MGL palette functions on the windowed display device
context. Note that MGL automatically takes care of creating a proper Windows
identity palette for the windowed device context, so as long as you program the
same palette values for the windowed device and the memory device you should get
the maximum performance blt’ing speed.

MGL automatically uses the highest performance method for implementing the
BitBlt operations under Windows, and requires the WinG library to be installed if
the target platform is Windows 3.1 or Windows NT 3.1. When using WinG, the
only pixel depth supported for Windowed device contexts is 8 bits per pixel, and the
only pixel format valid for BitBlt operations is 8 bit memory device contexts. You
can still create memory device contexts with higher pixel formats, but you will need
to Blt the data to a real 8 bit DC before you can display it in the window.

If the target platform is Windows 95 or Windows NT 3.5 or later, MGL will use
CreateDIBSection for maximum performance, and can create and Blt memory
device contexts of any pixel depth to the display device context. For maximum
performance you should create your memory device contexts with the same pixel
format used by the windowed display device context.

Note that if you wish to only use windowed output and you intend to target
Windows NT for your application, you must use the MGL_initWindowed function
to initialize MGL. This will not attempt to load the WinDirect full screen support
DLL’s, which are not compatible with Windows NT.

SEE ALSO:
MGL_createMemoryDC, MGL_createDisplayDC, MGL_destroyDC,
MGL_setWinDC, MGL_activatePalette, MGL_initWindowed
*******************************************************************************/
MGLDC * MGLAPI MGL_createWindowedDC(
	MGL_HWND hwnd)
{
	MGLDC	*dc;

	if ((dc = _LST_newNode(sizeof(MGLDC))) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}
	if (_MGL_winDCList == NULL) {
		if ((_MGL_winDCList = _LST_create()) == NULL) {
			_MGL_result = grNoMem;
			goto Error;
			}
		}
	/* Destroy the fullscreen window and switch back to GDI mode when we are
	 * switching to windowed mode.
	 */
	MGLPC_destroyFullScreenMode();

	// TODO: Determine which windowed DC driver to use by detecting if we
    //		 have DirectDraw and then basic blits. 

	if (!_MGL_initDC(dc,&WINDC_driver,hwnd,-1,-1,1,false,MGL_DEFAULT_REFRESH))
		goto Error;
	_MGL_deskX = 0;
	_MGL_deskY = 0;
	RESET_DEFAULT_CW();

	/* Add the new DC to the start of the DC chain */
	_LST_addAfter(_MGL_winDCList,dc,LST_HEAD(_MGL_winDCList));
	return dc;

Error:
	if (_MGL_winDCList && _MGL_winDCList->count == 0) {
		_LST_destroy(_MGL_winDCList,_LST_freeNode);
		_MGL_winDCList = NULL;
		}
	_LST_freeNode(dc);
	return NULL;
}

/* {secret} */
static  ibool _MGL_destroyWindowedDC(MGLDC *dc)
/****************************************************************************
*
* Function:		_MGL_destroyWindowedDC
* Parameters:	dc	- Device context to destroy
* Returns:		True if the context was destroyed, false on error.
*
* Description:	Searches for the windowed device context in the list of
*				currently allocated contexts, and destroys it if it
*				exists.
*
****************************************************************************/
{
	MGLDC	*prev;

	/* Remove the DC from the list of allocated memory DC's				*/

	if ((prev = _LST_findNode(_MGL_winDCList,dc)) == NULL) {
		_MGL_result = grInvalidDC;
		return false;
		}
	_LST_removeNext(_MGL_winDCList,prev);

	/* Free up all memory occupied by the DC */
	MGL_free(dc->colorTab);
	dc->v->w.destroy(dc);
	_LST_freeNode(dc);

	if (_MGL_winDCList->count == 0) {
		/* TODO: Delete DirectDraw provider and primary surface if we have created it */
//		if (_MGL_lpDCIPrimary) {
//			dciDestroy(_MGL_lpDCIPrimary);
//			_MGL_lpDCIPrimary = NULL;
//			}
//		if (_MGL_hDCIMan) {
//			dciCloseProvider(_MGL_hDCIMan);
//			_MGL_hDCIMan = NULL;
//			}
		/* This is the last windowed device context, so destroy the list */
		_LST_destroy(_MGL_winDCList,_LST_freeNode);
		_MGL_winDCList = NULL;
		}

	return true;
}

/*******************************************************************************
DESCRIPTION:
Destroy a given device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to be destroyed

RETURNS:
True if context was destroyed, false on error.

REMARKS:
Destroys a specified device context, freeing up all resources allocated by the device
context. This can fail for a number of reasons, so check the MGL_result code to
determine the cause of the failure.

If the device context that was destroyed was the last active display device context,
the video mode is reset back to the original video mode (or back to the normal GDI
desktop for Windows). Note that calling MGL_exit automatically destroys all
currently allocated device contexts.

SEE ALSO:
MGL_createDisplayDC, MGL_createOffscreenDC, MGL_createLinearOffscreenDC,
MGL_createMemoryDC, MGL_createWindowedDC
*******************************************************************************/
ibool MGLAPI MGL_destroyDC(
	MGLDC *dc)
{
	if (!dc)
		return true;
	if (dc == _MGL_dcPtr)
		MGL_makeCurrentDC(NULL);
	switch (dc->deviceType) {
		case MGL_DISPLAY_DEVICE:
		case MGL_FULLDIB_DEVICE:
		case MGL_OPENGL_DEVICE:
		case MGL_FS_OPENGL_DEVICE:
			return _MGL_destroyDisplayDC(dc);
#ifndef	MGL_LITE
		case MGL_OFFSCREEN_DEVICE:
		case MGL_LINOFFSCREEN_DEVICE:
			return _MGL_destroyOffscreenDC(dc);
#endif
		case MGL_WINDOWED_DEVICE:
			return _MGL_destroyWindowedDC(dc);
		case MGL_MEMORY_DEVICE:
			return _MGL_destroyMemoryDC(dc);
		}
	_MGL_result = grInvalidDC;
	return false;
}

/****************************************************************************
DESCRIPTION:
Activates the Windows palette for a windowed DC.

HEADER:
mglwin.h

PARAMETERS:
dc	- Device context (must be a Windowed DC)
unrealize	- True if the palette should be unrealized first before realizing it.

RETURNS:
True if the palette has changed and the window will be repainted, or false if the
palette did not change.

REMARKS:
Activates the Windows palette for a windowed device context for an MGL
windowed application running under Windows. This function is usually in response
to the WM_PALETTECHANGED and WM_QUERYNEWPALETTE messages as
follows:

If you are chaining between SYSPAL_STATIC and SYSPAL_NOSTATIC modes,
you should always pass true for the unrealize flag to ensure that the palette is
correctly updated.

SEE ALSO:
MGL_setWinDC
****************************************************************************/
ibool MGLAPI MGL_activatePalette(
	MGLDC *dc,
	ibool unrealize)
{
	ibool		palChanged = false;
	HPALETTE	hPalOld;

	if (dc->wm.windc.hpal) {
		HDC hdc = GetDC(dc->wm.windc.hwnd);
		if (unrealize)
			UnrealizeObject(dc->wm.windc.hpal);
		hPalOld = SelectPalette(hdc,dc->wm.windc.hpal, false);
		palChanged = RealizePalette(hdc);
		SelectPalette(hdc,hPalOld, false);
		ReleaseDC(dc->wm.windc.hwnd, hdc);
		}
	return palChanged;
}

/*******************************************************************************
DESCRIPTION:
Associate a window manager device context with an MGL device context.

HEADER:
mglwin.h

PARAMETERS:
dc	- MGL windowed device context to use
hdc	- Handle to window manager device context to associate

RETURNS:
True if the application’s palette has changed, false if not.

REMARKS:
This function is used to tell MGL what window manager device context should
actually be used for all subsequent BitBlt calls to transfer data from memory device
contexts to the windowed device context. Because the window manager device
contexts are usually allocated on the fly for every window repaint operation, this
function is usually called immediately after the window gets a repaint event and
before the windowed DC is used to transfer the memory device context data to the
window. A typical Windows WM_PAINT handler would be coded as follows:

	case WM_PAINT:
		hdc = BeginPaint(hwnd,&ps);
		MGL_setWinDC(winDC,hdc);
		// Do rasterizing code in here //
		MGL_bitBlt(winDC,memDC,r,0,0,MGL_REPLACE_MODE);
		EndPaint(hwnd,&ps);
		return 0;

SEE ALSO:
MGL_activatePalette
********************************************************************************/
ibool MGLAPI MGL_setWinDC(
	MGLDC *dc,
	MGL_HDC hdc)
{
	ibool	palChanged = false;

	if (dc->deviceType == MGL_WINDOWED_DEVICE) {
		/* For Windows 3.1 we have to set the device context for GDI output
		 * every time that we want to draw something (we dont cache the DC
		 * for the Window).
         */
		if (_MGL_haveWin31)
			dc->wm.windc.hdc = (HDC)hdc;

		/* If we have a palette for the DC, then select & realize it,
		 * returning true if the palette changed.
		 */
		if (dc->wm.windc.hpal) {
			SelectPalette(dc->wm.windc.hdc,dc->wm.windc.hpal,false);
			palChanged = RealizePalette(dc->wm.windc.hdc);
			}
		}
	return palChanged;
}

/*******************************************************************************
DESCRIPTION:
Resizes the windowed device context after a WM_SIZE	message has been captured.

HEADER:


PARAMETERS:
dc	- MGL device context to resize

REMARKS:
This is more efficient than re-creating the windowed device context for every
WM_SIZE message (especially if OpenGL has been initialized for the device
context).

*******************************************************************************/
void MGLAPI MGL_resizeWinDC(
	MGLDC *dc)
{
	if (dc->deviceType == MGL_WINDOWED_DEVICE) {
		RECT	r;
		rect_t	view;
		GetClientRect(dc->wm.windc.hwnd,&r);
		dc->mi.xRes = r.right - r.left - 1;
		dc->mi.yRes = r.bottom - r.top - 1;
		view.left = view.top = 0;
		view.right = dc->mi.xRes+1;
		view.bottom = dc->mi.yRes+1;
		dc->size = view;
		MGL_setViewportDC(dc,view);
		}
}

/* {secret} */
long _MGL_allocateDIB(MGLDC *dc,winBITMAPINFO *info)
/****************************************************************************
*
* Function:		_MGL_allocateDIB
* Parameters:	dc		- Device context.
*				info	- Pointer to bitmap info header and palette
* Returns:		Total size of the device context surface
*
* Description:	Allocates the memory for the device surface. When running
*				under Windows we allocate a DIB using either WinG or
*				CreateDIBSection. If we are running under WinG and we have
*				a non-8 bit surface, then we simply allocate the bitmap
*				surface using MGL_malloc(). Under WinG the only surfaces we
*				can Blt to the display are 8 bit surfaces.
*
*				If we are running in full screen mode, we allocate our
*				DIB's with a simple malloc since we dont need to interface
*				with GDI at all.
*
****************************************************************************/
{
	HBITMAP	hbm;

	switch (_MGL_bltType) {
		case BLT_CreateDIBSection: {
			HDC	hdc = GetDC(NULL);
			dc->wm.memdc.hdc = CreateCompatibleDC(hdc);
			if ((hbm = CreateDIBSection(dc->wm.memdc.hdc,(LPBITMAPINFO)info,DIB_RGB_COLORS,&dc->surface,0,0)) == NULL) {
				/* If CreateDIBSection fails, it could well be because Win95 and Memphis dont
				 * support extended RGB bitmap formats, so if we are in a fullscreen mode
				 * then try to allocate the bitmap using a regular malloc so that we can
				 * still support this for non-windowed apps.
				 */
				if (_MGL_fullScreen && (dc->mi.bitsPerPixel >= 24 && info->header.biCompression == winBI_BITFIELDS)) {
					if ((dc->surface = MGL_malloc(info->header.biSizeImage)) == NULL) {
						_MGL_result = grNoMem;
						return 0;
						}
					// TODO: Need to add support for creating an MGLDIB DC for the
					//		 memory bitmap so we can use GDI for these bitmaps.
					dc->wm.memdc.hdc = NULL;
					dc->wm.memdc.hbm = NULL;
					}
				else {
					_MGL_result = grNoMem;
					return 0;
                    }
				}
			else {
				SelectObject(dc->wm.memdc.hdc,hbm);
				dc->wm.memdc.hbm = hbm;
				ReleaseDC(NULL,hdc);
                }
			} break;
		case BLT_WinG:
			if (_MGL_fullScreen || dc->mi.bitsPerPixel > 8) {
				if ((dc->surface = MGL_malloc(info->header.biSizeImage)) == NULL) {
					_MGL_result = grNoMem;
					return 0;
					}
				// TODO: Need to work out how to use the WinG stuff to create
				//       memory bitmaps that we can get a WinDC for in Windows
				//       3.1 using the WinG DIB driver.
				dc->wm.memdc.hdc = NULL;
				dc->wm.memdc.hbm = NULL;
				}
			else {
				dc->wm.memdc.hdc = WinGCreateDC();
				if ((hbm = WinGCreateBitmap(dc->wm.memdc.hdc,(LPBITMAPINFO)info,&dc->surface)) == NULL) {
					DeleteDC(dc->wm.memdc.hdc);
					_MGL_result = grNoMem;
					return 0;
				}
				SelectObject(dc->wm.memdc.hdc,dc->wm.memdc.hbm);
				dc->wm.memdc.hbm = hbm;
				}
			break;
		}

	/* Clear the DIB surface, as this is not always guaranteed to be cleared */
	MGL_memset(dc->surface,0,info->header.biSizeImage);
	return info->header.biSizeImage;
}

/* {secret} */
void _MGL_freeDIB(MGLDC *dc)
/****************************************************************************
*
* Function:		MGL_freeDIB
* Parameters:	dc	- Device context
*
* Description:	Destroys the packed pixel device surface.
*
****************************************************************************/
{
	switch (_MGL_bltType) {
		case BLT_CreateDIBSection:
			if (dc->wm.memdc.hbm) {
				SelectObject(dc->wm.memdc.hdc,NULL);
				DeleteDC(dc->wm.memdc.hdc);
				DeleteObject(dc->wm.memdc.hbm);
				}
			else
				MGL_free(dc->surface);
			break;
		case BLT_WinG:
			if (dc->wm.memdc.hbm) {
				SelectObject(dc->wm.memdc.hdc,NULL);
				DeleteDC(dc->wm.memdc.hdc);
				DeleteObject(dc->wm.memdc.hbm);
				}
			else
				MGL_free(dc->surface);
			break;
		}
}

static  void MGL_clearSystemPalette(void)
/****************************************************************************
*
* Function:		MGL_clearSystemPalette
*
* Description:	Clears out the system palette so that we can ensure our
*				identity palettes are set up correctly for fast bitmap
*				blt'ing performance.
*
****************************************************************************/
{
	LogPal		Palette = { 0x300, 256 };
	HPALETTE	ScreenPalette;
	HDC 		ScreenDC;
	int 		Counter;

	/* Reset everything in the system palette to black */
	for (Counter = 0; Counter < 256; Counter++) {
		Palette.pe[Counter].peRed = 0;
		Palette.pe[Counter].peGreen = 0;
		Palette.pe[Counter].peBlue = 0;
		Palette.pe[Counter].peFlags = PC_NOCOLLAPSE;
		}

	/* Create, select, realize, deselect, and delete the palette */
	ScreenDC = GetDC(NULL);
	ScreenPalette = CreatePalette((LOGPALETTE *)&Palette);
	if (ScreenPalette) {
		ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
		RealizePalette(ScreenDC);
		ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
		DeleteObject(ScreenPalette);
		}
	ReleaseDC(NULL, ScreenDC);
	return;
}

static ibool _MGL_initCommon(const char *mglpath)
/****************************************************************************
*
* Function:		MGL_initCommon
* Parameters:	mglpath			- Path to root directory for MGL files
* Returns:		True if all went well.
*
* Description:	Initialises all common code for Windowed and full screen
*				output.
*
****************************************************************************/
{
	_MGL_result = grOK;

	GET_DEFAULT_CW();
	MGL_exit();

#ifdef  SHAREWARE
	MessageBox(NULL,
		"This program is using a free evaluation copy of the MGL for Windows "
		"and CANNOT be distributed in any shape or form. "
		"To distribute applications using the MGL you MUST purchase "
		"a retail copy of the MGL from SciTech Software.",
		"MGL for Windows Evaluation Copy",
		MB_ICONEXCLAMATION);
#endif

	if (initCommon)
		return true;
	_MGL_initMalloc();

	/* Create the internal scratch buffer */
	if ((_MGL_buf = MGL_malloc(_MGL_bufSize)) == NULL)
		MGL_fatalError("Not enough memory to allocate scratch buffer!\n");

#ifndef	MGL_LITE
	/* Create all region algebra memory pools */
	_MGL_createSegmentList();
	_MGL_createSpanList();
	_MGL_createRegionList();
#endif

	/* Save the path to all MGL files for later */
	strcpy(_MGL_path, mglpath);
	return initCommon = true;
}

/****************************************************************************
DESCRIPTION:
Initializes MGL for windowed only output.

HEADER:
mglwin.h

PARAMETERS:
mglpath	- Path to standard MGL resource files

RETURNS:
True if successful, false on error.

REMARKS:
This function initializes MGL for windowed only operation. The standard
MGL_init function sets up MGL for fullscreen output mode, which is not available
under Windows NT. Also if you are only interested in using MGL in a real
operating system window rather than in fullscreen mode, then using the init
function will not load any of the fullscreen graphics drivers and DLL's.

The mglpath variable is used by MGL to locate all MGL resources files in their
standard locations. The value passed in here should point to the base directory
where all the standard MGL resources are located, which may simply be the current
directory (i.e. a value of "."). When MGL is searching for resource files (bitmaps,
icons, fonts and cursors) it will first attempt to find the files just be using the
filename itself. Hence if you wish to look for a specific bitmap or font file, you
should pass the full pathname to the file that you are interested in. If the filename is
a simple relative filename (i.e. "MYFONT.FNT"), MGL will then search in the
standard directories relative to the path specified in mglpath. As a final resort MGL
will also look for the files relative to the MGL_ROOT environment variable, which
can be set to point to a standard location where all MGL files will be stored on the
user's machine. The standard locations that MGL will look for the resource files are
as follows:

Type	- Base pathname
Bitmaps	- mglpath\BITMAPS
Fonts	- mglpath\FONTS
Icons	- mglpath\ICONS
Cursor	- mglpath\CURSORS

If anything went wrong during the initialization process, MGL will return a result
code via the MGL_result routine. You can then use this result code to determine the
cause of the problem, and use the MGL_errorMsg routine to display an appropriate
error message for the user.

SEE ALSO:
MGL_init, MGL_result
****************************************************************************/
ibool MGLAPI MGL_initWindowed(
	const char *mglpath)
{
	int verMajor = GetVersion() & 0xFF;
	int	verMinor = (GetVersion() >> 8) & 0xFF;

	if (initWindowed)
		return true;

	if (!_MGL_initCommon(mglpath))
		return false;

	/* Determine what system we are running under and what our blt'ing options
	 * are and set things up accordingly.
	 */
#ifdef	__WIN386__
	if (verMajor == 3 && verMinor == 95)
		verMajor = 4;
#endif
	if (verMajor >= 4) {
		/* Running on Windows '95 so we can use CreateDIBSection */
        // TODO: Load DirectDraw for Windows blits
//		if (!LoadDirectDraw())
			if (!LoadCreateDIBSection())
				if (!LoadWinG())
					MGL_fatalError("Cannot load WinG. This application requires WinG to be loaded!");
		}
	else if (verMinor == 10) {
		/* Under Windows 3.1 with Win32s or Windows NT 3.1 we *must* have
		 * WinG installed, as the MGL only works with top down DIB's which
		 * Win 3.1 does not support.
		 */
		if (!LoadWinG())
			MGL_fatalError("Cannot load WinG. This application requires WinG to be loaded!");
		}
	else {
		/* Under NT we can use CreateDIBSection */
        // TODO: Load DirectDraw for Windows blits
//		if (!LoadDirectDraw())
			if (!LoadCreateDIBSection())
				if (!LoadWinG())
					MGL_fatalError("Cannot load WinG. This application requires WinG to be loaded!");
		}

	/* Clear the Windows system palette so we can get an identity palette */
	MGL_clearSystemPalette();
	return initWindowed = true;
}

/*******************************************************************************
DESCRIPTION:
Changes the current fullscreen mode or switches to windowed mode.

HEADER:
mgldos.h, mglwin.h

PARAMETERS:
mode	- New display mode to use

RETURNS:
True if the mode is available, false if mode is invalid.

REMARKS:
This function changes the current fullscreen display mode used by MGL, or informs
MGL that you are about to switch to windowed mode (for Windows versions). The
application should destroy all display and offscreen device contexts currently
allocated before calling this function, and then re-create all the required device
contexts for the new mode after calling this function. A typical code sequence to
change display modes would be as follows:

	MGLDC *dc;
	... init MGL and create DC as per normal ...
	MGL_destroyDC(dc);
	MGL_changeDisplayMode(grSVGA_640x480x256);
	dc = MGL_createDisplayDC();
	... mode has been changed to the new mode ...


Note that if there are any active display device contexts and offscreen device
contexts when this function is called, they will be destroyed by this call and the
system will be reset back to text mode. However none of the device contexts will be
re-created and it is up to the application to recreate all necessary device contexts.

If you are using this function to change display modes on the fly in MGL and you
wish to allow the user to switch to a windowed mode under Windows, you must call
this function with the grWINDOWED parameter before you create your windowed
window, or call MGL_exit after finishing in fullscreen modes. For example the
following code might be used to switch to a windowed mode.

	// Destroy the existing fullscreen mode and DC's
	MGL_destroyDC (mgldc);
	MGL_destroyDC (memdc);
	mgldc = memdc = NULL;

	// Signal to MGL that we are going windowed
	MGL_changeDisplayMode(grWINDOWED);

	// Create the windowed window
	window = CreateWindow(...);
	ShowWindow(window, SW_SHOWDEFAULT);

SEE ALSO:
MGL_init, MGL_createDisplayDC
*******************************************************************************/
ibool MGLAPI MGL_changeDisplayMode(
	int mode)
{
	_MGL_result = grOK;

	/* Check if the mode is currently available */
	if (mode == grWINDOWED) {
		MGLPC_destroyFullScreenMode();
		}
	else {
		if (mode >= grMAXMODE || _MGL_availableModes[mode].driver == 0xFF) {
			_MGL_result = grInvalidMode;
			return false;
			}
		_MGL_modeId = mode;

		/* Uncache the current DC */
		MGL_makeCurrentDC(NULL);

#ifndef	MGL_LITE
		/* Destroy all active offscreen contexts */
		while (_MGL_offDCList)
			_MGL_destroyOffscreenDC(_LST_first(_MGL_offDCList));
#endif

		/* Destroy all active display contexts, which restores text mode */
		while (_MGL_dispDCList)
			_MGL_destroyDisplayDC(_LST_first(_MGL_dispDCList));
		}
	return true;
}


/****************************************************************************
DESCRIPTION:
Initializes MGL for fullscreen operation.

HEADER:
mgldos.h, mglwin.h

PARAMETERS:
pDriver	- Place to store detected graphics device driver id
pMode	- Place to store suggested graphics mode id
mglpath	- Path to standard MGL resource files

RETURNS:
True if successful, false on error.

REMARKS:
This function initializes MGL for fullscreen operation, and sets it up to use the
specified fullscreen device driver and video mode. If you pass the value grDETECT
in the driver parameter, this function will automatically call the MGL_detectGraph
routine to detect the installed video hardware, and initialize itself accordingly
(please refer to MGL_detectGraph for more information on the detection process). The
video mode used in this case will be the one suggested by the installed video device
driver, and will be returned in the mode parameter.

Before you can call this routine, you must ensure that you have registered the
appropriate device drivers that you are interested in using in your code with the
MGL_registerDriver function. The process of registering the device drivers ensure
that the code will be linked in when you link your application (by default the code
will not be linked, to save space in the resulting executable). You can simply call
MGL_registerAllDispDrivers.  If you wish to start a video mode other than the
default one suggested by the video device driver, you should call the
MGL_detectGraph routine first, and pass the value returned for driver and your
selected video mode in mode to initialize the specific mode. The fullscreen device
drivers currently supported by MGL are enumerated in MGL_driverType.

The mglpath variable is used by MGL to locate all MGL resources files in their
standard locations. The value passed in here should point to the base directory
where all the standard MGL resources are located, which may simply be the current
directory (i.e. a value of "."). When MGL is searching for resource files (bitmaps,
icons, fonts and cursors) it will first attempt to find the files just by using the
filename itself. Hence if you wish to look for a specific bitmap or font file, you
should pass the full pathname to the file that you want. If the filename is a simple
relative filename (i.e. "MYFONT.FNT"), MGL will then search in the standard
directories relative to the path specified in mglpath. As a final resort MGL will also
look for the files relative to the MGL_ROOT environment variable, which can be
set to point to a standard location where all MGL files will be stored on the user's
machine. The standard locations that MGL will look for the resource files are as
follows:

Resource	- Base pathname
Bitmaps		- mglpath\BITMAPS
Fonts		- mglpath\FONTS
Icons		- mglpath\ICONS
Cursors		- mglpath\CURSORS

If anything went wrong during the initialization process, MGL will return a result
code via the MGL_result routine. You can then use this result code to determine the
cause of the problem, and use the MGL_errorMsg routine to display an appropriate
error message for the user.

SEE ALSO:
MGL_initWindowed, MGL_detectGraph, MGL_result
****************************************************************************/
ibool MGLAPI MGL_init(
	int *pDriver,
	int *pMode,
	const char *mglpath)
{
	if (initFullscreen)
		MGL_fatalError("Nested call to MGL_init!");

	if (!MGL_initWindowed(mglpath))
		return false;

	MGL_detectGraph(pDriver,pMode);
	if (*pDriver == grNONE) {
		_MGL_result = grNotDetected;
		return false;
		}

	/* Save selected driver and mode number for later */
	_MGL_driverId = *pDriver;
	if (*pMode != grDETECT) {
		if (!MGL_changeDisplayMode(*pMode))
			return false;
		}
	return initFullscreen = true;
}

/*******************************************************************************
DESCRIPTION:
Closes down the graphics subsystem.

HEADER:
mgraph.h

REMARKS:
This function closes down the graphics subsystem, deallocating any memory
allocated for use by MGL, and restoring the system back into the original text mode
that was active before MGL was started. This routine also properly removes all
interrupt handlers and other system services that MGL hooked when it was
initialized.

You must call this routine before you exit your application, to ensure that the
system is properly terminated.

SEE ALSO:
MGL_init, MGL_initWindowed
*******************************************************************************/
void MGLAPI MGL_exit(void)
{
	HDC	hdc;

	/* Uncache the current DC */
	MGL_makeCurrentDC(NULL);

	/* Destroy all active memory device contexts */
	while (_MGL_memDCList)
		_MGL_destroyMemoryDC(_LST_first(_MGL_memDCList));

	/* Destroy all active windowed contexts */
	while (_MGL_winDCList)
		_MGL_destroyWindowedDC(_LST_first(_MGL_winDCList));

#ifndef	MGL_LITE
	/* Destroy all active offscreen contexts */
	while (_MGL_offDCList)
		_MGL_destroyOffscreenDC(_LST_first(_MGL_offDCList));
#endif

	/* Destroy all active display contexts, which restores GDI */
	while (_MGL_dispDCList)
		_MGL_destroyDisplayDC(_LST_first(_MGL_dispDCList));

	/* Destroy the fullscreen window and switch back to GDI mode on exit */
	MGLPC_destroyFullScreenMode();

	/* Unload OpenGL DLL's */
	MGL_glUnloadOpenGL();

	/* Destroy the user window before shutting down DirectDraw! */
	if (initFullscreen && _MGL_hwndUser)
		DestroyWindow(_MGL_hwndUser);

	/* Ensure system is switched back to static palette mode */
	hdc = GetDC(NULL);
	SetSystemPaletteUse(hdc, SYSPAL_STATIC);
	ReleaseDC(NULL,hdc);

#ifndef	MGL_LITE
	/* Destroy all region algebra memory pools */
	_MGL_freeSegmentList();
	_MGL_freeSpanList();
	_MGL_freeRegionList();
#endif

	if (_MGL_buf) {
		MGL_free(_MGL_buf);
		_MGL_buf = NULL;
		}

#ifndef	__WIN386__
	/* Delete the DirectDraw object if we have created it */
	if (_MGL_lpDD) {
		IDirectDraw_Release(_MGL_lpDD1);
		_MGL_lpDD = NULL;
		_MGL_lpDD1 = NULL;
		}
#endif

	/* Remove WinDirect suspend application callback */
	MGL_setSuspendAppCallback(NULL);

	initCommon = initWindowed = initFullscreen = false;
}

/****************************************************************************
DESCRIPTION:
Function to call when your app becomes active.

HEADER:
mglwin.h

PARAMETERS:
winDC	- Currently active windowed DC
active	- True if app is active, false if not

REMARKS:
Tracks whether a game application is currently active for palette management.
Forces a repaint if the system palette usage is set to no-static.
****************************************************************************/
void MGLAPI MGL_appActivate(
	MGLDC *winDC,
	ibool active)
{
	if (winDC && MGL_isWindowedDC(winDC)) {
		HDC	hdc = GetDC(NULL);
		if (_MGL_isBackground != 2)
			_MGL_isBackground = !active;
		if (GetSystemPaletteUse(hdc) == SYSPAL_NOSTATIC && MGL_getBitsPerPixel(winDC) <= 8) {
			/* Re-realize the hardware palette and repaint on activate */
			MGL_realizePalette(winDC,MGL_getPaletteSize(winDC),0,-1);
			InvalidateRect(winDC->wm.windc.hwnd,NULL,TRUE);
			}
		ReleaseDC(NULL, hdc);
        }
}

/*******************************************************************************
DESCRIPTION:
Generate a beep on the PC speaker.

HEADER:
mgldos.h, mglwin.h

PARAMETERS:
freq	- frequency of the beep
milliseconds	- length of beep in milliseconds

REMARKS:
Beeps the PC speaker at the specified frequency for the specified length of time.
*******************************************************************************/
void MGLAPI MGL_beep(
	int freq,
	int msecs)
{
	MessageBeep(-1);
	freq = freq;
	msecs = msecs;
}

/*******************************************************************************
DESCRIPTION:
Do nothing for a specified amount of time.

HEADER:
mgldos.h, mglwin.h

PARAMETERS:
milliseconds	- Number of milliseconds to delay

REMARKS:
Delay processing for the specified number of milliseconds.
*******************************************************************************/
void MGLAPI MGL_delay(
	int msecs)
{
	ulong	tmStart;

	tmStart = timeGetTime();
	while ((timeGetTime() - tmStart) < msecs)
		;
}

/*******************************************************************************
DESCRIPTION:
Get the current timer tick count.

HEADER:
mgldos.h, mglwin.h

RETURNS:
Current timer tick count as a 32 bit integer.

REMARKS:
This function returns the current timer tick as a 32-bit integer value.
The number of ticks in a single second can be determined with the
MGL_getTickResolution function.

SEE ALSO:
MGL_getTickResolution
*******************************************************************************/
ulong MGLAPI MGL_getTicks(void)
{
	return timeGetTime();
}

/*******************************************************************************
DESCRIPTION:
Get duration of a timer tick.

HEADER:
mgldos.h, mglwin.h

RETURNS:
Number of seconds in a timer tick * 1,000,000.

REMARKS:
This function returns an unsigned long value indicating the duration of a timer tick
in seconds multiplied by one million. The duration of a timer tick changes
depending on the target environment, so you should use this function to convert the
value to a standard representation.

SEE ALSO:
MGL_getTicks
*******************************************************************************/
ulong MGLAPI MGL_getTickResolution(void)
{
	return 1000UL;
}

/*******************************************************************************
DESCRIPTION:
Sets the fullscreen suspend application callback function.

HEADER:
mgraph.h

PARAMETERS:
saveState	- New suspend app callback to be used.

REMARKS:
This function is used to register an application suspend callback function. This is
used in fullscreen modes under Windows and is called by MGL when the
application’s fullscreen window has lost the input focus and the system has returned
to the normal GDI desktop. The focus can be lost due to the user hitting a System
Key combination such as Alt-Tab or Ctrl-Esc which forces your fullscreen
application into the background. MGL takes care of all the important details such as
saving and restoring the state of the hardware, so all your suspend application
callback needs to do is save the current state of your program so that when the
request is made to re-activate your application, you can redraw the screen and
continue from where you left off.

When MGL detects that your application has been suspended it will call the
registered callback with a combination of the following flags:

Flag			- Meaning
MGL_DEACTIVATE	- This flag will be sent when your application has lost
				  the input focus and has been suspended.
MGL_REACTIVATE	- This flag will be sent when the user re-activates
				  your fullscreen application again indicating that the
				  fullscreen mode has now been restored and the application
				  must redraw the display ready to continue on.

By default if you have not installed a suspend callback handler, MGL will simply
restore the display to the original state with the screen cleared to black when the
application is re-activated. If your application is a game or animation that is
continuously updating the screen, you won’t need to do anything as the next frame
in the animation will re-draw the screen correctly. If however your application is
caching bitmaps in offscreen video memory, all of the bitmaps will need to be
restored to the offscreen display device context when the application is restored (the
offscreen memory will be cleared to black also).

Note:	By the time your callback is called, the display memory may have already
		been lost under DirectDraw so you cannot save and restore the contents of the
		display memory, but must be prepared to redraw the entire display when the
		callback is called with the MGL_REACTIVATE flag set.
*******************************************************************************/
void MGLAPI MGL_setSuspendAppCallback(
	MGL_suspend_cb_t saveState)
{
	_MGL_suspendApp = saveState;
}

/*******************************************************************************
DESCRIPTION:
Sets the application main window handle.

HEADER:
mglwin.h

PARAMETERS:
hwndMain	- New main window handle to set

REMARKS:
This function informs MGL what your application’s main window handle is. If you
have a dialog box or main window for your application and you want to start a
fullscreen graphics mode under Windows without destroying your main window,
you can pass the window handle to MGL using this function so it will automatically
ensure that the main application window is removed from the focus chain while
running in fullscreen mode. The main purpose of this function is to ensure that your
main window cannot become active while running in fullscreen modes, which will
cause the system to switch out of the fullscreen mode back to normal GDI mode.

If you don’t call this function, you should ensure that when you start a fullscreen
graphics mode that your application currently has no windows open on the desktop.
If you don’t wish to destroy the windows you should call this function, or ensure
that the windows which have been hidden are disabled
(ShowWindow(hwnd,FALSE) and EnableWindow(false)) before you start the
fullscreen mode.
*******************************************************************************/
void MGLAPI MGL_setMainWindow(
	MGL_HWND hwnd)
{ _MGL_hwndMain = (HWND)hwnd; }


/*******************************************************************************
DESCRIPTION:
Sets the application instance handle.

HEADER:
mglwin.h

PARAMETERS:
hInstApp	- Handle to the application instance

REMARKS:
This function sets the application instance handle for MGL, which is necessary for
some internal MGL operations. You must call this function under Windows before
you call MGL_init, and if you do not MGL will exit with an error message
requesting that you do so.

*******************************************************************************/
void MGLAPI MGL_setAppInstance(
	MGL_HINSTANCE hInstApp)
{ _MGL_hInst = hInstApp; }

/*******************************************************************************
DESCRIPTION:
Registers a user window with the MGL to be used for fullscreen modes

HEADER:
mglwin.c

PARAMETERS:
hwnd	- Handle to user window to use for fullscreen modes.

REMARKS:
This function allows the application to create the window used for fullscreen
modes, and let the MGL know about the window so it will use that window
instead of creating it's own fullscreen window. By default when you create
a fullscreen device context, the MGL will create a fullscreen window that
covers the entire desktop that is used to capture Windows events such as
keyboard events, mouse events and activation events. However in some situations
it is beneficial to have only a single window that is used for all fullscreen
graphics modes, as well as windows modes (primarily to be able to properly
support DirectSound via a single window).

If you have registered a fullscreen window with the MGL, the MGL will take
that window, zoom it fullscreen and modify the window styles and attributes
to remove the title bar and other window decorations. When the MGL then
returns from fullscreen mode back to GDI mode, it will restore the window
back to the original position, style and state it was in before the fullscreen
mode was created. This allows you to create a single window with the MGL and
use it for both windowed modes and fullscreen modes. 

Note:	When the MGL exits via a call to MGL exit, the fullscreen window that
		you passed in will be destroyed (necessary to work around bugs in
		DirectDraw).
*******************************************************************************/
void MGLAPI MGL_registerFullScreenWindow(
	HWND hwnd)
{ _MGL_hwndUser = hwnd; }

/*******************************************************************************
DESCRIPTION:
Registers a user supplied window procedure for event handling.

HEADER:
mglwin.h

PARAMETERS:
userWndProc 	- Point to user supplied Window Procedure

REMARKS:
This function registers a user supplied window procedure with MGL that will be
used for event handling purposes. By default MGL applications can simply use the
EVT_* event handling functions that are common to both the DOS and Windows
versions of MGL. However developers porting Windows specific code from
DirectDraw may wish to use their existing window specific event handling code.
This function allows you to do this by telling MGL to use your window procedure
for all event processing.

SEE ALSO:
EVT_getNext
*******************************************************************************/
void MGLAPI MGL_registerEventProc(
	WNDPROC userWndProc)
{ _MGL_userEventProc = userWndProc; }

/*******************************************************************************
DESCRIPTION:
Returns the current fullscreen window handle.

HEADER:
mglwin.h

RETURNS:
Current fullscreen window handle.

REMARKS:
This function returns the handle to the current fullscreen window. When you are
running in fullscreen modes under Windows, MGL always maintains a fullscreen,
topmost window that is used for event handling.

If you are using the DirectSound libraries for sound output, you will need to inform
DirectSound what your fullscreen window is so that it can correctly mute the sound
for your application when the focus is lost to another application. If you do not do
this, when you switch to fullscreen modes all sound output via DirectSound will be
muted (assuming your are requesting exclusive mode).

SEE ALSO:
MGL_registerEventProc
*******************************************************************************/
MGL_HWND MGLAPI MGL_getFullScreenWindow(void)
{ return _MGL_hwndFullScreen; }

/*******************************************************************************
DESCRIPTION:
Returns a Windows HDC for drawing on a device context using GDI

HEADER:
mglwin.h

PARAMETERS:
dc	- Device context to obtain the Windows HDC for

RETURNS:
Windows compatible device context handle (HDC) for the device context.

REMARKS:
This function allows you to obtain a handle to a Windows compatible device
context (HDC) for any MGL device context. Using the returned HDC, you can
then call regular GDI drawing functions to draw on the device context,
instead of using the MGL drawing functions. This is most useful for drawing
objects that are not supported by the MGL, such as Bezier curves and using
TrueType fonts.

Note: It is possible that in some instances (mostly under Windows 3.1) that
	  a valid HDC cannot be created, and this function will return NULL.

Note: In order for this function to work for fullscreen device contexts under
	  Windows 95, the MGLDIB.DRV driver must be installed into the
	  WINDOWS\SYSTEM directory on the target machine.
*******************************************************************************/
HDC MGLAPI MGL_getWinDC(
	MGLDC *dc)
{ return (HDC)dc->r.getWinDC(dc); }

static ibool _MGL_isDirectDrawDevice(MGLDC *dc,int deviceType)
{
	if (dc->deviceType != deviceType)
		return false;
	if (strncmp(MGL_modeDriverName(dc->v->d.graphMode),"DDRAW",4) != 0)
		return false;
	return true;
}

/*******************************************************************************
DESCRIPTION:
Returns the DirectDraw object for a fullscreen video mode

HEADER:
mglwin.h

PARAMETERS:
dc	- Device context to get DirectDraw object for

RETURNS:
Pointer to the LPDIRECTDRAW object.

REMARKS:
This function returns a pointer to the LPDIRECTDRAW object (the main DirectDraw
object used by the MGL) for fullscreen graphics modes. This is intended to
allow application programmers to do custom operations with DirectDraw when the
MGL is running in DirectDraw modes (such as creating a Direct3D object and
using Direct3D with the MGL).

If the fullscreen graphics mode is not a DirectDraw mode, this function
returns NULL.

Note:	If you obtain a copy of the DirectDraw object, dont create your
		own surfaces, but use the surfaces already created by the MGL (you
		can attach your own z-buffers and texture memory surfaces to these
        if you wish to use Direct3D directly).

SEE ALSO:
MGL_getDirectDrawPrimarySurface
*******************************************************************************/
MGL_LPDD MGLAPI MGL_getDirectDrawObject(
	MGLDC *dc)
{
	if (_MGL_isDirectDrawDevice(dc,MGL_DISPLAY_DEVICE))
		return _MGL_lpDD;
    return NULL;
}
    
/*******************************************************************************
DESCRIPTION:
Returns the DirectDraw primary surface object for a fullscreen video mode

HEADER:
mglwin.h

PARAMETERS:
dc	- Device context to get DirectDraw primary surface object for

RETURNS:
Pointer to the LPDIRECTDRAWSURFACE object.

REMARKS:
This function returns a pointer to the LPDIRECTDRAWSURFACE object for the
primary surface used in MGL fullscreen graphics modes. The primary surface
is the main surface that all the back buffer surfaces are attached to (the
number of attached buffers will depend on the number you requested when you
called MGL_createDisplayDC). Note that the primary surface may not be the
currently active surface for drawing; to get this surface call
MGL_getDirectDrawActiveSurface instead.

If the fullscreen graphics mode is not a DirectDraw mode, this function
returns NULL.

SEE ALSO:
MGL_getDirectDrawActiveSurface, MGL_getDirectDrawOffscreenSurface
*******************************************************************************/
MGL_LPDDSURF MGLAPI MGL_getDirectDrawPrimarySurface(
	MGLDC *dc)
{
	if (_MGL_isDirectDrawDevice(dc,MGL_DISPLAY_DEVICE))
		return _MGL_lpPrimary;
	return NULL;
}

/*******************************************************************************
DESCRIPTION:
Returns the DirectDraw active surface object for a fullscreen video mode

HEADER:
mglwin.h

PARAMETERS:
dc	- Device context to get DirectDraw active surface object for

RETURNS:
Pointer to the LPDIRECTDRAWSURFACE object.

REMARKS:
This function returns a pointer to the LPDIRECTDRAWSURFACE object for the
active surface used in MGL fullscreen graphics modes. The active surface
is the currently active surface that all drawing is done to, which may not
be the same as the primary surface created by the MGL.

If the fullscreen graphics mode is not a DirectDraw mode, this function
returns NULL.

SEE ALSO:
MGL_getDirectDrawPrimarySurface, MGL_getDirectDrawOffscreenSurface
*******************************************************************************/
MGL_LPDDSURF MGLAPI MGL_getDirectDrawActiveSurface(
	MGLDC *dc)
{
	if (_MGL_isDirectDrawDevice(dc,MGL_DISPLAY_DEVICE))
		return _MGL_lpActive;
	return NULL;
}

/*******************************************************************************
DESCRIPTION:
Returns the DirectDraw offscreen surface object for a fullscreen video mode

HEADER:
mglwin.h

PARAMETERS:
dc	- Device context to get DirectDraw offscreen surface object for

RETURNS:
Pointer to the LPDIRECTDRAWSURFACE object.

REMARKS:
This function returns a pointer to the LPDIRECTDRAWSURFACE object for the
offscreen surface used in MGL fullscreen graphics modes. The offscreen surface
is the offscreen video memory surface that the MGL creates when you make a
call to MGL_createOffscreenDC and the installed DirectDraw drivers support
hardware BitBlt acceleration. If the device context passed in is not an
offscreen display device context, this function returns NULL. 

If the fullscreen graphics mode is not a DirectDraw mode, this function
returns NULL.

Note:	The MGL offscreen DirectDraw surface will never be made visible on
		the screen, and is only used for caching bitmaps in offscreen
        video memory to be moved around using hardware acceleration.

SEE ALSO:
MGL_getDirectDrawPrimarySurface
*******************************************************************************/
MGL_LPDDSURF MGLAPI MGL_getDirectDrawOffscreenSurface(
	MGLDC *dc)
{
	if (_MGL_isDirectDrawDevice(dc,MGL_DISPLAY_DEVICE))
		return _MGL_lpOffscreen;
	return NULL;
}

/*******************************************************************************
DESCRIPTION:
Returns the DirectDraw palette object for a fullscreen video mode

HEADER:
mglwin.h

PARAMETERS:
dc	- Device context to get DirectDraw palette object for

RETURNS:
Pointer to the LPDIRECTDRAWPALETTE object.

REMARKS:
This function returns a pointer to the LPDIRECTDRAWPALETTE object used in
MGL fullscreen graphics modes. The DirectDraw palette object is the palette
that is created by the MGL for use in 8 bits per pixel modes, and will be NULL
if the video mode is greater than 8 bits per pixel.

If the fullscreen graphics mode is not a DirectDraw mode, this function
returns NULL.

SEE ALSO:
MGL_getDirectDrawPrimarySurface
*******************************************************************************/
MGL_LPDDPAL MGLAPI MGL_getDirectDrawPalette(
	MGLDC *dc)
{
	if (_MGL_isDirectDrawDevice(dc,MGL_DISPLAY_DEVICE))
		return _MGL_lpPalette;
	return NULL;
}

#ifndef __WIN386__

/* Code stubs to PM/Pro library functions so that external code can
 * directly link to these functions as well. This allows code to use
 * the PM/Pro library directly (like our UVBELib linkable libraries) with
 * the MGL without requiring to link explicitly to the PM/Pro library
 * DLL's.
 */

#undef	PM_getByte
#undef	PM_getWord
#undef	PM_getLong
#undef	PM_setByte
#undef	PM_setWord
#undef	PM_setLong
#undef	PM_memcpynf
#undef	PM_memcpyfn
#undef	PM_getBIOSSelector
#undef	PM_createSelector
#undef	PM_freeSelector
#undef	PM_mapPhysicalAddr
#undef	PM_mapRealPointer
#undef	PM_allocRealSeg
#undef	PM_freeRealSeg
#undef	PM_callRealMode
#undef	PM_int86
#undef	PM_int86x
#undef	PM_availableMemory
#undef	WD_int386
#undef	VF_available
#undef	VF_init
#undef	VF_exit
#undef	PM_setBankA
#undef	PM_setBankAB
#undef	PM_setCRTStart
#undef	_PM_getRMvect
#undef	_PM_setRMvect

/* {secret} */
uchar PMAPI PM_getByte(uint s, uint o)
{ LoadWinDirect(); return pPM_getByte(s,o); }

/* {secret} */
ushort PMAPI PM_getWord(uint s, uint o)
{ LoadWinDirect(); return pPM_getWord(s,o); }

/* {secret} */
ulong PMAPI PM_getLong(uint s, uint o)
{ LoadWinDirect(); return pPM_getLong(s,o); }

/* {secret} */
void PMAPI PM_setByte(uint s, uint o, uchar v)
{ LoadWinDirect(); pPM_setByte(s,o,v); }

/* {secret} */
void PMAPI PM_setWord(uint s, uint o, ushort v)
{ LoadWinDirect(); pPM_setWord(s,o,v); }

/* {secret} */
void PMAPI PM_setLong(uint s, uint o, ulong v)
{ LoadWinDirect(); pPM_setLong(s,o,v); }

/* {secret} */
void PMAPI PM_memcpynf(void *dst,uint src_s,uint src_o,uint n)
{ LoadWinDirect(); pPM_memcpynf(dst,src_s,src_o,n); }

/* {secret} */
void PMAPI PM_memcpyfn(uint dst_s,uint dst_o,void *src,uint n)
{ LoadWinDirect(); pPM_memcpyfn(dst_s,dst_o,src,n); }

/* {secret} */
uint PMAPI PM_getBIOSSelector(void)
{ LoadWinDirect(); return pPM_getBIOSSelector(); }

/* {secret} */
uint PMAPI PM_createSelector(ulong base,ulong limit)
{ LoadWinDirect(); return pPM_createSelector(base,limit); }

/* {secret} */
void PMAPI PM_freeSelector(uint sel)
{ LoadWinDirect(); pPM_freeSelector(sel); }

/* {secret} */
void * PMAPI PM_mapPhysicalAddr(ulong base,ulong limit)
{ LoadWinDirect(); return pPM_mapPhysicalAddr(base,limit); }

/* {secret} */
void PMAPI PM_mapRealPointer(uint *sel,uint *off,uint r_seg,uint r_off)
{ LoadWinDirect(); pPM_mapRealPointer(sel,off,r_seg,r_off); }

/* {secret} */
int PMAPI PM_allocRealSeg(uint size,uint *sel,uint *off,uint *r_seg,uint *r_off)
{ LoadWinDirect(); return pPM_allocRealSeg(size,sel,off,r_seg,r_off); }

/* {secret} */
void PMAPI PM_freeRealSeg(uint sel,uint off)
{ LoadWinDirect(); pPM_freeRealSeg(sel,off); }

/* {secret} */
void PMAPI PM_callRealMode(uint seg,uint off, RMREGS *regs,RMSREGS *sregs)
{ LoadWinDirect(); pPM_callRealMode(seg,off,regs,sregs); }

/* {secret} */
int PMAPI PM_int86(int intno, RMREGS *in, RMREGS *out)
{ LoadWinDirect(); return pPM_int86(intno,in,out); }

/* {secret} */
int PMAPI PM_int86x(int intno, RMREGS *in, RMREGS *out,RMSREGS *sregs)
{ LoadWinDirect(); return pPM_int86x(intno,in,out,sregs); }

/* {secret} */
void PMAPI PM_availableMemory(ulong *physical,ulong *total)
{ LoadWinDirect(); pPM_availableMemory(physical,total); }

/* {secret} */
int PMAPI WD_int386(int intno, RMREGS *in, RMREGS *out)
{ LoadWinDirect(); return pWD_int386(intno,in,out); }

/* {secret} */
ibool PMAPI VF_available(void)
{ LoadWinDirect(); return pVF_available(); }

/* {secret} */
void * PMAPI VF_init(ulong baseAddr,int bankSize,int codeLen,void *bankFunc)
{ LoadWinDirect(); return pVF_init(baseAddr,bankSize,codeLen,bankFunc); }

/* {secret} */
void PMAPI VF_exit(void)
{ LoadWinDirect(); pVF_exit(); }

/* {secret} */
void PMAPI _PM_getRMvect(int intno, long *realisr)
{ LoadWinDirect(); p_PM_getRMvect(intno,realisr); }

/* {secret} */
void PMAPI _PM_setRMvect(int intno, long realisr)
{ LoadWinDirect(); p_PM_setRMvect(intno,realisr); }

/* {secret} */
void PMAPI PM_setBankA(int bank)
{ pPM_setBankA(bank); }

/* {secret} */
void PMAPI PM_setBankAB(int bank)
{ pPM_setBankAB(bank); }

/* {secret} */
void PMAPI PM_setCRTStart(int x,int y,int waitVRT)
{ pPM_setCRTStart(x,y,waitVRT); }

/* {secret} */
void * PMAPI PM_getA0000Pointer(void)
{ return pPM_getA0000Pointer(); }

ibool PMAPI PM_getVESABuf(uint *len,uint *sel,uint *off,uint *rseg,uint *roff)
{ return pPM_getVESABuf(len,sel,off,rseg,roff); }

#endif	/* !__WIN386__ */

