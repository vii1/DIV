/****************************************************************************
*
*				   WinDirect - Windows full screen interface
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
* Description:	Internal header file to define the constants used for the
*				thunking API.
*
*
****************************************************************************/

#ifndef	__WDAPI_H
#define	__WDAPI_H

#include "pmpro.h"

/* DisplayDib() error return codes */
#define DISPLAYDIB_NOERROR          0x0000  // success
#define DISPLAYDIB_NOTSUPPORTED     0x0001  // function not supported
#define DISPLAYDIB_INVALIDDIB       0x0002  // null or invalid DIB header
#define DISPLAYDIB_INVALIDFORMAT    0x0003  // invalid DIB format
#define DISPLAYDIB_INVALIDTASK      0x0004  // not called from current task

/* Flags for <wFlags> parameter of DisplayDib() */
#define DISPLAYDIB_NOPALETTE        0x0010  // don't set palette
#define DISPLAYDIB_NOCENTER         0x0020  // don't center image
#define DISPLAYDIB_NOWAIT           0x0040  // don't wait before returning
#define DISPLAYDIB_NOIMAGE          0x0080  // don't draw image
#define DISPLAYDIB_ZOOM2            0x0100  // stretch by 2
#define DISPLAYDIB_DONTLOCKTASK     0x0200  // don't lock current task
#define DISPLAYDIB_TEST             0x0400  // testing the command
#define DISPLAYDIB_NOFLIP           0x0800  // dont page flip
#define DISPLAYDIB_BEGIN            0x8000  // start of multiple calls
#define DISPLAYDIB_END              0x4000  // end of multiple calls

#define DISPLAYDIB_MODE             0x000F  // mask for display mode
#define DISPLAYDIB_MODE_DEFAULT     0x0000  // default display mode
#define DISPLAYDIB_MODE_320x200x8   0x0001  // 320-by-200
#define DISPLAYDIB_MODE_320x240x8   0x0005  // 320-by-240

/* DISPDIB entry point */
UINT FAR PASCAL DisplayDib(LPBITMAPINFOHEADER lpbi,LPSTR lpBits,WORD wFlags);

#define	DELAY_TIME					1000

/* The undocumented Death() and Resurrection() Functions */
void WINAPI Death(HDC dc);
void WINAPI Resurrection(HDC dc, WORD w1, WORD w2, WORD w3, WORD w4, WORD w5, WORD w6);

#pragma pack(1)

typedef struct {
	DWORD	ret;					/* Return value from function		*/
	DWORD	parm[5];				/* Place to store parameters		*/
	RMREGS	in,out;					/* Real mode registers				*/
	RMSREGS	sr;						/* Segment registers				*/
	char	code[100];				/* VFlat bank switching code buffer	*/
	} PM_thunkVals;

#pragma pack()

/* Universal Thunk id's */
#define	PM_INT86					0
#define	PM_INT86X					1
#define	PM_CALLREALMODE				2
#define	PM_CREATE_SELECTOR			3
#define	PM_FREE_SELECTOR			4
#define	PM_ALLOC_REAL_SEG			5
#define	PM_FREE_REAL_SEG			6
#define	PM_MAP_PHYSICAL_ADDR		7
#define	DPMI_MAP_PHYSICAL_TO_LINEAR	8
#define	DPMI_SET_SELECTOR_BASE		9
#define	DPMI_GET_SELECTOR_BASE		10
#define	DPMI_SET_SELECTOR_LIMIT		11
#define	VF_AVAILABLE32				12
#define	VF_INIT32					13
#define	VF_EXIT32					14
#define	PM_SET_BANKA				15
#define	PM_SET_BANKAB				16
#define	PM_SET_CRTSTART				17
#define	PM_AVAILABLE_MEMORY			18
#define	VF_MAPPHYSICALBUFFER		19
#define	DEATH						20
#define	RESURRECTION				21
#define	PM_SET_CRTSTARTALT			22
#define	PM_GET_DISPLAYSTARTSTATUS	23
#define	WD_INT386					24
#define	WD_ENABLE_CRTC_TRAPPING		25
#define	WD_DISABLE_CRTC_TRAPPING	26
#define	PM_INITSHAREDMEM			1000
#define	PM_KILLSHAREDMEM			1001

/* Routines used by generic WinDirect code */
LONG WINAPI WD_winProc(HWND hwnd, UINT msg, WPARAM wParam, LONG lParam);
void _WDAPI _ShutdownGDI(void);
void _WDAPI _RestoreGDI(void);
long GetTimeStamp(void);
void InitEvent(void);
void _WDAPI _WD_doSuspendApp(MSG *orgmsg);

/* Externals required by generic code*/
extern char _WD_szWinClassName[];
extern HINSTANCE _WD_hInst;
extern ibool _WD_inited;
extern BOOL	haveWin95;			

#ifndef	__WINDOWS32__
/* Flags for ChangeDisplaySettings */
#define CDS_UPDATEREGISTRY  0x00000001
#define CDS_TEST            0x00000002
#define CDS_FULLSCREEN      0x00000004

/* Return values for ChangeDisplaySettings */
#define DISP_CHANGE_SUCCESSFUL       0
#define DISP_CHANGE_RESTART          1
#define DISP_CHANGE_FAILED          -1
#define DISP_CHANGE_BADMODE         -2
#define DISP_CHANGE_NOTUPDATED      -3
#define DISP_CHANGE_BADFLAGS        -4

/* size of a device name string */
#define CCHDEVICENAME 32
#define CCHPAPERNAME  64
#define CCHFORMNAME   32

/* field selection bits */
#define DM_ORIENTATION      0x00000001L
#define DM_PAPERSIZE        0x00000002L
#define DM_PAPERLENGTH      0x00000004L
#define DM_PAPERWIDTH       0x00000008L
#define DM_SCALE            0x00000010L
#define DM_COPIES           0x00000100L
#define DM_DEFAULTSOURCE    0x00000200L
#define DM_PRINTQUALITY     0x00000400L
#define DM_COLOR            0x00000800L
#define DM_DUPLEX           0x00001000L
#define DM_YRESOLUTION      0x00002000L
#define DM_TTOPTION         0x00004000L
#define DM_COLLATE          0x00008000L
#define DM_FORMNAME         0x00010000L
#define DM_LOGPIXELS        0x00020000L
#define DM_BITSPERPEL       0x00040000L
#define DM_PELSWIDTH        0x00080000L
#define DM_PELSHEIGHT       0x00100000L
#define DM_DISPLAYFLAGS     0x00200000L
#define DM_DISPLAYFREQUENCY 0x00400000L
#define DM_ICMMETHOD        0x00800000L
#define DM_ICMINTENT        0x01000000L
#define DM_MEDIATYPE        0x02000000L
#define DM_DITHERTYPE       0x04000000L

typedef struct tagDEVMODE
{
    char  dmDeviceName[CCHDEVICENAME];
    UINT  dmSpecVersion;
    UINT  dmDriverVersion;
    UINT  dmSize;
    UINT  dmDriverExtra;
    DWORD dmFields;
    int   dmOrientation;
    int   dmPaperSize;
	int   dmPaperLength;
    int   dmPaperWidth;
    int   dmScale;
    int   dmCopies;
    int   dmDefaultSource;
    int   dmPrintQuality;
    int   dmColor;
    int   dmDuplex;
    int   dmYResolution;
	int   dmTTOption;
    int   dmCollate;
    char  dmFormName[CCHFORMNAME];
    WORD  dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmDisplayFlags;
    DWORD dmDisplayFrequency;
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
	DWORD dmReserved1;
    DWORD dmReserved2;
} DEVMODE,FAR *LPDEVMODEA;

#define	SPI_SCREENSAVERRUNNING	97
#endif

#define	WM_DO_SUSPEND_APP			WM_USER
#define	WM_WD_DEACTIVATE			0
#define	WM_WD_RESTORE_FULLSCREEN	1
#define	WM_WD_SET_FOCUS				2

/* Time to wait after switch before allowing a new Alt-Tab */

#define	ALT_TAB_DELAY				1500

/* Dynalinks to ChangeDisplaySettings functions */

#undef ChangeDisplaySettings
#undef EnumDisplaySettings
extern LONG (WINAPI *pChangeDisplaySettings)(LPDEVMODEA lpDevMode,DWORD dwFlags);
extern BOOL (WINAPI *pEnumDisplaySettings)(LPCSTR lpszDeviceName,DWORD iModeNum,LPDEVMODEA lpDevMode);
#define ChangeDisplaySettings(a,b)	pChangeDisplaySettings(a,b)
#define EnumDisplaySettings(a,b,c)	pEnumDisplaySettings(a,b,c)

ibool LoadChangeDisplaySettings(void);
void _ASMAPI _PM_setCRTStartAlt(ulong ebx,ulong ecx,ulong edx);
uint _ASMAPI _PM_getDisplayStartStatus(void);

#endif	/* __WDAPI_H */

