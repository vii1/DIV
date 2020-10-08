/****************************************************************************
*
*						   WinDirect Test Program
*
*				 	Copyright (C) 1996 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   wdtest.c  $
* Version:		$Revision:   1.7  $
* Author:		Kendall Bennett
*
* Language:     ANSI C
* Environment:	Win16/32, running on Windows 3.1 or Windows '95
*
* Description:	Windows based test program for WinDirect. We use the
*				WinDirect interface to get into a full screen video mode,
*				and the use the SuperVGA Kit routines to display full
*				screen VESA graphics from under Windows.
*
*               It also plays the SOUND.WAV file using the Windows
*               Multimedia API from the current directory. You can replace
*               this with any .WAV file you like to play that instead.
*
* $Date:   04 May 1996 03:20:38  $ $Author:   KendallB  $
*
****************************************************************************/

#define	WIN32_LEAN_AND_MEAN
#define STRICT
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include "wdirect.h"
#include "vesavbe.h"
#include "svga.h"
#include "pmode.h"
#include "wdtest.h"
#ifdef	USE_UVBELIB
#include "uvbelib.h"
#endif
#ifdef	USE_UVBELIB_ACCEL
#include "wdpro.h"
#endif
#ifdef	USE_CTL3D
#include "ctl3d.h"
#endif

#ifdef	__WINDOWS32__
#define	WIN_BITS	32
#define	NOTIFY_CODE		HIWORD(wParam)
#else
#define	WIN_BITS	16
#define	NOTIFY_CODE		HIWORD(lParam)
#endif

/*--------------------------- Global Variables ----------------------------*/

SV_devCtx	*DC;
HINSTANCE 	hInstApp;
ushort		modeNums[MAX_MODES];
BOOL		useVBEAF = false;
char		configDir[_MAX_PATH];

#include "version.c"

/*------------------------------ Implementation ---------------------------*/

bool doTest(ushort mode,bool doPalette,bool doVirtual,
	bool doRetrace,int maxProgram,bool use8BitDAC,bool useVirtualBuffer);

int KeyHit(void)
/****************************************************************************
*
* Function:		KeyHit
* Returns:      true if a key was hit
*
* Description:	Check for a key or mouse click by querying WinDirect event
*               queue and returns true if one is found. This routine is
*				called by the SuperVGA Kit test code when it needs to check
*				for a keypress.
*
****************************************************************************/
{
	WD_event evt;
	return WD_peekEvent(&evt,EVT_KEYDOWN | EVT_MOUSEDOWN);
}

int GetChar(void)
/****************************************************************************
*
* Function:		GetChar
* Returns:		Character of key pressed.
*
* Description:	Waits for keypres from the WinDirect event queue, and returns
*				the keystroke of the key. For mouse down events we return
*				an 'Enter' keystroke. This routine is called by the SuperVGA
*				Kit test code when it needs to read keypresses.
*
****************************************************************************/
{
	WD_event	evt;
    WD_haltEvent(&evt,EVT_KEYDOWN | EVT_MOUSEDOWN);
	if (evt.what == EVT_KEYDOWN)
		return WD_asciiCode(evt.message);
	return 0xD;
}

void CenterWindow(HWND hWndCenter, HWND parent, BOOL repaint)
/****************************************************************************
*
* Function:		CenterWindow
* Parameters:	hWndCenter	- Window to center
*				parent		- Handle for parent window
*               repaint     - true if window should be re-painted
*
* Description:	Centers the specified window within the bounds of the
*				specified parent window. If the parent window is NULL, then
*				we center it using the Desktop window.
*
****************************************************************************/
{
	HWND	hWndParent = (parent ? parent : GetDesktopWindow());
	RECT	RectParent;
	RECT	RectCenter;
	int		CenterX,CenterY,Height,Width;

	GetWindowRect(hWndParent, &RectParent);
	GetWindowRect(hWndCenter, &RectCenter);

	Width = (RectCenter.right - RectCenter.left);
	Height = (RectCenter.bottom - RectCenter.top);
	CenterX = ((RectParent.right - RectParent.left) - Width) / 2;
	CenterY = ((RectParent.bottom - RectParent.top) - Height) / 2;

	if ((CenterX < 0) || (CenterY < 0)) {
		/* The Center Window is smaller than the parent window. */
		if (hWndParent != GetDesktopWindow()) {
			/* If the parent window is not the desktop use the desktop size. */
			CenterX = (GetSystemMetrics(SM_CXSCREEN) - Width) / 2;
			CenterY = (GetSystemMetrics(SM_CYSCREEN) - Height) / 2;
			}
		CenterX = (CenterX < 0) ? 0: CenterX;
		CenterY = (CenterY < 0) ? 0: CenterY;
		}
	else {
		CenterX += RectParent.left;
		CenterY += RectParent.top;
		}

	/* Copy the values into RectCenter */
	RectCenter.left = CenterX;
	RectCenter.right = CenterX + Width;
	RectCenter.top = CenterY;
	RectCenter.bottom = CenterY + Height;

	/* Move the window to the new location */
	MoveWindow(hWndCenter, RectCenter.left, RectCenter.top,
			(RectCenter.right - RectCenter.left),
			(RectCenter.bottom - RectCenter.top), repaint);
}

BOOL WINAPI _export NotDetectedDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
/****************************************************************************
*
* Function:		NotDetectedDlgProc
*
* Description:	Dialog procedure for Not Detected dialog box.
*
****************************************************************************/
{
	switch (msg) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_OK)
				EndDialog(hwnd,true);
			break;
		case WM_INITDIALOG:
			CenterWindow(hwnd,NULL,false);
			return true;
		}
	lParam = lParam;
	return false;
}

BOOL WINAPI _export SVGAInfoDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
/****************************************************************************
*
* Function:		SVGAInfoDlgProc
*
* Description:	Dialog procedure for Not Detected dialog box.
*
****************************************************************************/
{
	char		buf[MAX_STR];
	VBE_vgaInfo	vgaInfo;

	switch (msg) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_OK)
                EndDialog(hwnd,true);
			break;
		case WM_INITDIALOG:
            CenterWindow(hwnd,NULL,false);
			VBE_detect(&vgaInfo);
			SetDlgItemText(hwnd,IDC_OEMSTRING,vgaInfo.OemStringPtr);
			sprintf(buf,"%d.%d", vgaInfo.VESAVersion >> 8, vgaInfo.VESAVersion & 0xFF);
			SetDlgItemText(hwnd,IDC_VBEVERSION,buf);
			sprintf(buf,"%d Kb", vgaInfo.TotalMemory * 64);
			SetDlgItemText(hwnd,IDC_VBEMEMORY,buf);
			if (vgaInfo.VESAVersion >= 0x200) {
				sprintf(buf,"0x%04X", vgaInfo.OemSoftwareRev);
				SetDlgItemText(hwnd,IDC_OEMSOFTWAREREV,buf);
				SetDlgItemText(hwnd,IDC_OEMVENDORNAME,vgaInfo.OemVendorNamePtr);
				SetDlgItemText(hwnd,IDC_OEMPRODUCTNAME,vgaInfo.OemProductNamePtr);
				SetDlgItemText(hwnd,IDC_OEMPRODUCTREV,vgaInfo.OemProductRevPtr);
				if (DC->linearAddr) {
					sprintf(buf,"Yes (located at %d Mb)", (ulong)DC->linearAddr >> 20);
					SetDlgItemText(hwnd,IDC_LINEARBUFFER,buf);
					}
				else if (DC->haveVirtualBuffer)
					SetDlgItemText(hwnd,IDC_LINEARBUFFER,"No (virtualised in software)");
				else
					SetDlgItemText(hwnd,IDC_LINEARBUFFER,"No");
				}
			else {
				SetDlgItemText(hwnd,IDC_OEMSOFTWAREREV,"N/A");
				SetDlgItemText(hwnd,IDC_OEMVENDORNAME,"N/A");
				SetDlgItemText(hwnd,IDC_OEMPRODUCTNAME,"N/A");
				SetDlgItemText(hwnd,IDC_OEMPRODUCTREV,"N/A");
				SetDlgItemText(hwnd,IDC_LINEARBUFFER,"No");
				}
			SetDlgItemText(hwnd,IDC_8BITDAC,(DC->haveWideDAC) ? "Yes" : "No");
            return true;
		}
	lParam = lParam;
    return false;
}

void RefreshModeList(HWND hwnd)
/****************************************************************************
*
* Function:		RefreshModeList
* Parameters:	hwnd	- Handle to dialog box window
*
* Description:	Refreshes the list of available video modes in the video
*				mode list box given the newly selected pixel depth.
*
****************************************************************************/
{
	char			buf[MAX_STR];
	int				bits,i;
	ushort			*m,mode;
	SV_modeInfo		mi;
	HWND			hwndLst = GetDlgItem(hwnd,IDC_MODELIST);
	BOOL			useLinear = IsDlgButtonChecked(hwnd,IDC_USELINEAR);

	if (IsDlgButtonChecked(hwnd,IDC_4BIT))			bits = 4;
	else if (IsDlgButtonChecked(hwnd,IDC_8BIT))		bits = 8;
	else if (IsDlgButtonChecked(hwnd,IDC_15BIT))	bits = 15;
	else if (IsDlgButtonChecked(hwnd,IDC_16BIT))	bits = 16;
	else if (IsDlgButtonChecked(hwnd,IDC_24BIT))	bits = 24;
	else if (IsDlgButtonChecked(hwnd,IDC_32BIT))	bits = 32;

	SendMessage(hwndLst,LB_RESETCONTENT,0,0);
	for (m = DC->modeList, i = 0; *m != 0xFFFF; m++) {
		if (!SV_getModeInfo(*m,&mi))
			continue;
		if (mi.BitsPerPixel != bits)
			continue;
		if ((mode = SV_getModeName(buf,&mi,*m,useLinear)) == 0)
			continue;
		SendMessage(hwndLst,LB_ADDSTRING,0,(LONG)buf);
		modeNums[i++] = mode;
		}
	SendMessage(hwndLst,LB_SETCURSEL,0,0);
}

void TestMode(HWND hwnd)
/****************************************************************************
*
* Function:		TestMode
* Parameters:	hwnd	- Handle to dialog box window
*
* Description:	Refreshes the list of available video modes in the video
*				mode list box given the newly selected pixel depth.
*
****************************************************************************/
{
	BOOL	doPalette = IsDlgButtonChecked(hwnd,IDC_DOPALETTE);
	BOOL	doVirtual = IsDlgButtonChecked(hwnd,IDC_DOVIRTUAL);
	BOOL	doRetrace = IsDlgButtonChecked(hwnd,IDC_DORETRACE);
	HWND	hwndLst = GetDlgItem(hwnd,IDC_MODELIST);
    BOOL	useVirtualBuffer = IsDlgButtonChecked(hwnd,IDC_USELINEAR);
	ushort	mode;

	if (SendMessage(hwndLst,LB_GETCOUNT,0,0)) {
		mode = SendMessage(hwndLst,LB_GETCURSEL,0,0);
		WD_startFullScreen(hwnd,640,480);
		doTest(modeNums[mode],doPalette,doVirtual,doRetrace,256,
			DC->haveWideDAC,useVirtualBuffer);
		WD_restoreGDI();
		}
}

BOOL WINAPI _export MainDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
/****************************************************************************
*
* Function:		MainDlgProc
*
* Description:	Dialog procedure for main dialog box.
*
****************************************************************************/
{
	char	buf[MAX_STR];
	char	format[MAX_STR];

	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_OK:
					break;
				case IDC_CANCEL:
                    EndDialog(hwnd,true);
					break;
				case IDC_SVGAINFO:
					DialogBox(hInstApp,MAKEINTRESOURCE(IDD_SVGAINFO),hwnd,SVGAInfoDlgProc);
					break;
				case IDC_MODELIST:
					if (NOTIFY_CODE != LBN_DBLCLK)
						break;
				case IDC_TESTMODE:
					TestMode(hwnd);
					break;
				case IDC_4BIT:
				case IDC_8BIT:
				case IDC_15BIT:
				case IDC_16BIT:
				case IDC_24BIT:
				case IDC_32BIT:
				case IDC_USELINEAR:
					RefreshModeList(hwnd);
					break;
				}
			break;
		case WM_INITDIALOG:
            CenterWindow(hwnd,NULL,false);
			GetDlgItemText(hwnd,IDC_VERSIONSTR,format,sizeof(format));
			sprintf(buf,format,WIN_BITS,release_major,release_minor,release_date);
			SetDlgItemText(hwnd,IDC_VERSIONSTR,buf);
			SetDlgItemText(hwnd,IDC_COPYRIGHTSTR,copyright_str);
            CheckDlgButton(hwnd,IDC_4BIT,true);
			if (DC->linearAddr || DC->haveVirtualBuffer)
                CheckDlgButton(hwnd,IDC_USELINEAR,true);
			else {
            	/* Hardware linear buffer or virtual buffer is not available */
                EnableWindow(GetDlgItem(hwnd,IDC_USELINEAR),false);
                }
            CheckDlgButton(hwnd,IDC_DORETRACE,true);
            CheckDlgButton(hwnd,IDC_DOVIRTUAL,true);
            CheckDlgButton(hwnd,IDC_DOPALETTE,true);
			RefreshModeList(hwnd);
            return true;
		}
	lParam = lParam;
    return false;
}

bool HaveWin95(void)
{
	int verMajor = GetVersion() & 0xFF;
#ifdef	__WINDOWS32__
	return (verMajor >= 4);
#else
    int verMinor = (GetVersion() >> 8) & 0xFF;
    return (verMajor >= 3 && verMinor >= 95);
#endif
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR szCmdLine,int sw)
{
#if	defined(USE_UVBELIB) || defined(USE_UVBELIB_ACCEL)
	char	szModuleName[260];
	int 	errCode;
#ifdef	USE_UVBELIB_ACCEL
	AF_devCtx	*af = NULL;
#endif
#endif

#ifdef	USE_UVBELIB
	GetModuleFileName(hInst,szModuleName,sizeof(szModuleName));
	UV_getDriverPath(szModuleName, configDir);
	if ((errCode = UV_install(configDir,true,true)) != UV_ok) {
		switch (errCode) {
			case UV_noDetect:
			case UV_errNotValid:
			case UV_errOldVersion:
			case UV_errNoMemory:
			case UV_errNoRealMemory:
			case UV_errCheckInstall:
				MessageBox(NULL,
					"Fatal error installing UVBELib!",
					"Fatal Error!",
					MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL);
				exit(1);
				break;
			}
		}
#endif
#ifdef	USE_UVBELIB_ACCEL
	GetModuleFileName(hInst,szModuleName,sizeof(szModuleName));
	UV_getDriverPath(szModuleName, configDir);
	if ((af = WD_init(configDir)) != NULL)
		SV_setACCELDriver(af);
	else {
		errCode = WD_status();
		if (errCode != afNotDetected && errCode != afDriverNotFound) {
			printf("UVBELib/Accel: %s\n", WD_errorMsg(errCode));
			exit(1);
			}
		}
#endif
#ifdef	USE_CTL3D
	if (!HaveWin95()) {
		Ctl3dRegister(hInst);
		Ctl3dAutoSubclass(hInst);
		}
#endif

	hInstApp = hInst;
	DC = SV_init(useVBEAF);
	if (!DC || DC->VBEVersion < 0x102) 
		DialogBox(hInstApp,MAKEINTRESOURCE(IDD_NOTDETECTED),NULL,NotDetectedDlgProc);
	else
		DialogBox(hInstApp,MAKEINTRESOURCE(IDD_MAINDLG),NULL,MainDlgProc);

#ifdef	USE_CTL3D
	if (!HaveWin95())
		Ctl3dUnregister(hInst);
#endif
#ifdef	USE_UVBELIB
	UV_exit();
#endif
#ifdef	USE_UVBELIB_ACCEL
	WD_exit(af);
#endif
	hPrev = hPrev;
	szCmdLine = szCmdLine;
	return 0;
}

