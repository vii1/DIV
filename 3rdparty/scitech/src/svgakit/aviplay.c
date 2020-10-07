/****************************************************************************
*
*			  AVIPlay - AVIDirect Fullscreen SuperVGA Video Player
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Filename:     $Workfile:   aviplay.c  $
* Version:      $Revision:   1.4  $
*
* Language:     ANSI C
* Environment:  Win16/Win32
*
* Description:  Simple test application using the MCIWnd class to play
*				AVI files using an installable draw procedure. The draw
*				procedure is provided by AVIDirect and renders directly
*				to full screen WinDirect video modes.
*
* $Date:   10 Apr 1996 18:15:22  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdlib.h>
#include <windows.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <digitalv.h>
#include <mciavi.h>
#include <vfw.h>
#include <mciwnd.h>
#include <stdio.h>
#include "aviplay.h"
#include "avidirec.h"

#ifdef	USE_UVBELIB
#include "uvbelib.h"
#endif

#ifdef	__WINDOWS32__
#define	WIN_BITS	32
#define	NOTIFY_CODE		HIWORD(wParam)
#else
#define	WIN_BITS	16
#define	NOTIFY_CODE		HIWORD(lParam)
#endif

/*--------------------------- Global Variables ----------------------------*/

char    szAppName[]  = "AVIDirect Fullscreen Video Player";
char    szDocClass[] = MCIWND_WINDOW_CLASS;
char    szOpenFilter[] = "Video Files\0*.avi\0";
HANDLE  hInstApp;                   /* Instance handle 					*/
HWND    hwndApp;                    /* Handle to parent window 			*/
HWND    hwndMCI;                    /* Handle to MCI client window 		*/
FARPROC	lpfnHook = NULL;			/* Hook for the draw procedure		*/
bool	playingFullScreen = false;	/* True when playing fullscreen		*/
char	configDir[_MAX_PATH];

/*------------------------------ Implementation ---------------------------*/

void fatalError(char *msg)
{
	MessageBox(NULL, msg, "AVIDirect Player", MB_ICONEXCLAMATION);
    exit(1);
}

BOOL CALLBACK AppAbout(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
/****************************************************************************
*
* Function:		AppAbout
*
* Description:	Dialog procedure for About dialog box.
*
****************************************************************************/
{
	char	buf[MAX_STR];
	char	format[MAX_STR];

	switch (msg) {
		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
				EndDialog(hwnd,TRUE);
			break;
		case WM_INITDIALOG:
			GetDlgItemText(hwnd,IDC_VERSIONSTR,format,sizeof(format));
			sprintf(buf,format,WIN_BITS);
			SetDlgItemText(hwnd,IDC_VERSIONSTR,buf);
			return TRUE;
		}
	lParam = lParam;
	return FALSE;
}

void OpenMovie(HWND hwnd)
/****************************************************************************
*
* Function:		OpenMovie
*
* Description:	Prompts the user for a movie file to open and opens
*				the MCIWindow for the movie.
*
****************************************************************************/
{
	char         	filename[256];
	OPENFILENAME	ofn;

	filename[0] = 0;

	/* Prompt user for file to open */
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = szOpenFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = "Open";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (GetOpenFileName(&ofn))
		MCIWndOpen(hwndMCI, filename, 0);
}

void InstallDrawHandler(bool install)
/****************************************************************************
*
* Function:		InstallDrawHandler
*
* Description:	This is where we install the draw handler for displaying
*				the data in full screen video mode.
*
****************************************************************************/
{
	MCI_DGV_SETVIDEO_PARMS	dgv;
	UINT            		uDevice;

	/* Important note: the callback will be called using
	 * a different stack segment, so we have to call
	 * MakeProcInstance, and can't rely on "smart callbacks"!
	 */
	if (!lpfnHook)
		lpfnHook = MakeProcInstance((FARPROC)AVIDirectDrawProc,hInstApp);

	if ((uDevice = MCIWndGetDeviceID(hwndMCI)) != 0) {
    	if (install)
			dgv.dwValue = (DWORD)lpfnHook;
		else
			dgv.dwValue = NULL;
		dgv.dwItem = MCI_AVI_SETVIDEO_DRAW_PROCEDURE;
		mciSendCommand(uDevice, MCI_SETVIDEO,
			MCI_DGV_SETVIDEO_ITEM | MCI_DGV_SETVIDEO_VALUE,
			(DWORD)&dgv);
		}
}

LONG CALLBACK AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
/****************************************************************************
*
* Function:		AppWndProc
*
* Description:	Main window proc for application.
*
****************************************************************************/
{
	RECT	r;

	switch (msg) {
		case WM_COMMAND:
			switch(wParam) {
				case MENU_ABOUT:
					DialogBox(hInstApp,MAKEINTRESOURCE(IDD_ABOUT),hwnd,(DLGPROC)AppAbout);
					break;
				case MENU_EXIT:
					PostMessage(hwnd,WM_CLOSE,0,0L);
					break;
				case MENU_OPEN:
					OpenMovie(hwnd);
					break;
				case MOVIE_PLAY:
					MCIWndPlay(hwndMCI);
					break;
				case MOVIE_PLAYFULL:
					MCIWndStop(hwndMCI);
					InstallDrawHandler(true);
					playingFullScreen = true;
					MCIWndSendString(hwndMCI,"play fullscreen");
					break;
				case MOVIE_CONFIGURE:
                	AVIDirectConfigure(hwnd);
                	break;
				}
			break;
		case WM_PALETTECHANGED:
		case WM_QUERYNEWPALETTE:
		case WM_ACTIVATE:
			/* Forward these to the MCIWnd so it can do the right thing */
			if (hwndMCI)
				return SendMessage(hwndMCI, msg, wParam, lParam);
			break;
	   	case WM_CREATE:
			hwndMCI = MCIWndCreate(hwnd, hInstApp,
				WS_CHILD | WS_VISIBLE | MCIWNDF_NOTIFYMEDIA |
				MCIWNDF_NOTIFYSIZE/* | MCIWNDF_NOMENU*/, 0);
			break;
		case WM_SIZE:
			if (hwndMCI)
				MoveWindow(hwndMCI,0,0,LOWORD(lParam),HIWORD(lParam),TRUE);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case MCIWNDM_NOTIFYMEDIA:
		   	if (hwndMCI == 0)
				hwndMCI = (HWND)wParam;
           	break;
		case MCIWNDM_NOTIFYSIZE:
			if (hwndMCI == 0)
				hwndMCI = (HWND)wParam;
			GetWindowRect(hwndMCI, &r);
			AdjustWindowRect(&r, GetWindowLong(hwnd, GWL_STYLE), TRUE);
			SetWindowPos(hwnd,NULL,0,0,r.right-r.left,r.bottom-r.top+1,
				SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
			break;
		case WM_PAINT:
			/* Remove our draw handler whenever we get a WM_PAINT, as this
			 * means we are back in Windowed mode again.
             */
			InstallDrawHandler(false);
			break;
		}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

BOOL AppInit(HANDLE hInst, HANDLE hPrev, int sw)
/****************************************************************************
*
* Function:		AppInit
*
* Description:	Main window proc for application.
*
****************************************************************************/
{
    WNDCLASS    cls;
    WORD        wVer;

    /* First let's make sure we are running on 1.1 */
    wVer = HIWORD(VideoForWindowsVersion());
    if (wVer < 0x010a){
		/* Oops, we are too old, blow out of here */
		MessageBeep(MB_ICONHAND);
		MessageBox(NULL, "Video for Windows version is too old",
        	"AVIDirect Error", MB_OK|MB_ICONSTOP);
		return FALSE;
		}

	/* Save instance handle for DialogBox */
    hInstApp = hInst;

    if (!hPrev) {
        cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon          = LoadIcon(hInst,"AppIcon");
        cls.lpszMenuName   = "AppMenu";
        cls.lpszClassName  = szAppName;
        cls.hbrBackground  = (HBRUSH)COLOR_APPWORKSPACE+1;
        cls.hInstance      = hInst;
        cls.style          = 0;
        cls.lpfnWndProc    = (WNDPROC)AppWndProc;
        cls.cbClsExtra     = 0;
        cls.cbWndExtra     = 0;

        if (!RegisterClass(&cls))
            return FALSE;
    	}

	hwndApp = CreateWindow(szAppName,szAppName,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,CW_USEDEFAULT, 0, 160, 120,
		NULL,NULL,hInst,NULL);
	ShowWindow(hwndApp,sw);
	return TRUE;
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int sw)
{
	MSG     msg;
    HDC		hdc;
#ifdef	USE_UVBELIB
	char	szModuleName[260];
	int 	errCode;
#endif

	hdc = GetDC(NULL);
	if (GetDeviceCaps(hdc,BITSPIXEL) < 8) {
		MessageBox(NULL,
			"Due to a bug in Video for Windows, AVIDirect cannot currently\n"
			"run in 16 color graphics modes. Please change to a 256 color or\n"
			"higher video mode to play videos with AVIDirect.",
			"AVIDirect", MB_ICONEXCLAMATION);
		ReleaseDC(NULL,hdc);
		exit(1);
		}
	ReleaseDC(NULL,hdc);

#ifdef	USE_UVBELIB
	GetModuleFileName(hInstance,szModuleName,sizeof(szModuleName));
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

	if (!AppInit(hInstance,hPrevInstance,sw))
	   return FALSE;

	/* Polling messages from event queue */
	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE)) {
            if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		else
			WaitMessage();
		}
	szCmdLine = szCmdLine;

#ifdef	USE_UVBELIB
	UV_exit();
#endif
	return msg.wParam;
}

