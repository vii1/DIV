/****************************************************************************
*
*			The SuperVGA Kit - UniVBE Software Development Kit
*
*                   Copyright (C) 1996 SciTech Software
*                           All rights reserved.
*
* Filename:     $Workfile:   svgasdk.c  $
* Version:      $Revision:   1.16  $
*
* Language:     ANSI C
* Environment:  IBM PC (MSDOS) Real Mode and 16/32 bit Protected Mode.
*
* Description:  Simple library to collect together the functions in the
*               SuperVGA test library for use in other C programs. The
*               support is reasonably low level, so you can do what you
*               want. The set of routines in this source file are general
*               SuperVGA routines and are independant of the video mode
*               selected.
*
*               MUST be compiled in the LARGE or FLAT models.
*
* $Date:   03 May 1996 22:52:18  $ $Author:   KendallB  $
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "pmpro.h"
#include "svga.h"

/* If we are compiling for either 32 bit protected mode or 16 bit Windows,
 * then we can use the VBE 2.0 protected mode functions and linear
 * framebuffer code, and also VBE/AF Accelerator Functions.
 */

#if	defined(PM386) || defined(__WINDOWS16__)
#define	USE_VBE20
#endif

#ifndef	__16BIT__
#define	USE_VBEAF
#endif

/*---------------------------- Global Variables ---------------------------*/

#define MAXMODES    100				/* Maximum modes available in list  */

#define	DC	_SV_devCtx				/* Macro to access global structure	*/

PUBLIC  SV_devCtx _ASMAPI _SV_devCtx = {0};
PRIVATE	AF_devCtx *loadedDC = NULL;	/* DC of loaded VBE/AF driver		*/
PRIVATE	uint 	VESABuf_len = 1024;	/* Length of VESABuf				*/
PRIVATE	uint 	VESABuf_sel = 0;	/* Selector for VESABuf         	*/
PRIVATE	uint 	VESABuf_off;		/* Offset for VESABuf           	*/
PRIVATE	uint 	VESABuf_rseg;		/* Real mode segment of VESABuf 	*/
PRIVATE	uint 	VESABuf_roff;		/* Real mode offset of VESABuf  	*/
PRIVATE	ushort	_SV_modeList[MAXMODES];
PRIVATE	char    _SV_OEMString[80];
PRIVATE bool    _SV_old50Lines;		/* Was old mode 80x50?              */
PRIVATE	bool	_SV_convertPal;		/* True to convert palette entries	*/
PUBLIC	int     _ASMAPI _SV_bankShift;/* Bank granularity adjust factor	*/
PUBLIC	long    _ASMAPI _SV_pagesize;	/* Page size for current mode	*/
PUBLIC	void    * _ASMAPI _SV_setBankRM;/* Pointer to direct VBE bank code*/
PUBLIC	void	* _ASMAPI _SV_setBankPtr;/* Pointer to bank switch code	*/

void _ASMAPI _VBE_setBankA(void);	/* VBE setBank routine single bank	*/
void _ASMAPI _VBE_setBankAB(void);	/* VBE setBank routine dual banks	*/

#ifdef	USE_VBE20
void _ASMAPI _VBE20_setBankA(void);		/* VBE 2.0 setBank single bank	*/
void _ASMAPI _VBE20_setBankAB(void);	/* VBE 2.0 setBank dual banks	*/
void _ASMAPI _VBE20_setBankA_ES(void);	/* VBE 2.0 setBank for MMIO		*/
void _ASMAPI _VBE20_setBankAB_ES(void);	/* VBE 2.0 setBank for MMIO		*/
PUBLIC	void * _ASMAPI _SV_setBank20 = NULL;/* Pointer to set bank routine*/
PUBLIC	void * _ASMAPI _SV_setCRT20 = NULL;/* Pointer to set CRT routine*/
PUBLIC	void * _ASMAPI _SV_setPal20 = NULL;/* Pointer to set palette rtn*/
extern	short _ASMAPI VBE_MMIOSel;		/* Selector to MMIO registers	*/
#endif

#ifdef	USE_VBEAF
void SVAPI _VBEAF_setBank(void);	/* VBE/AF setBank function			*/
#endif

extern	uchar _ASMAPI font8x16[];	/* Bitmap font definition			*/
extern	SV_palette _ASMAPI VGA8_defPal[];

/*----------------------------- Implementation ----------------------------*/

/* Declare all banked framebuffer routines */

void _ASMAPI _SV_setActivePage(int page);
void _ASMAPI _SV_setVisualPage(int page,bool waitVRT);
void _ASMAPI _VGA_setPalette(int start,int num,SV_palette *pal,bool waitVRT);
void _ASMAPI _VBE20_setPalette(int start,int num,SV_palette *pal,bool waitVRT);
void _ASMAPI _SV_clear16(ulong color);
void _ASMAPI _SV_clear256(ulong color);
void _ASMAPI _SV_clear32k(ulong color);
void _ASMAPI _SV_clear16m(ulong color);
void _ASMAPI _SV_clear4G(ulong color);
void _ASMAPI _SV_putPixel16(int x,int y,ulong color);
void _ASMAPI _SV_putPixel256(int x,int y,ulong color);
void _ASMAPI _SV_putPixel32k(int x,int y,ulong color);
void _ASMAPI _SV_putPixel16m(int x,int y,ulong color);
void _ASMAPI _SV_putPixel4G(int x,int y,ulong color);
void _ASMAPI _SV_line16(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_line256(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_line32k(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_line16m(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_line4G(int x1,int y1,int x2,int y2,ulong color);

/* Declare all the linear framebuffer routines */

#ifdef	USE_VBE20
#ifdef	__WINDOWS16__
void _ASMAPI linsdk_enable32(void);
#endif
void _ASMAPI _SV_setActivePageLin(int page);
void _ASMAPI _SV_clear256Lin(ulong color);
void _ASMAPI _SV_clear32kLin(ulong color);
void _ASMAPI _SV_clear16mLin(ulong color);
void _ASMAPI _SV_clear4GLin(ulong color);
void _ASMAPI _SV_putPixel256Lin(int x,int y,ulong color);
void _ASMAPI _SV_putPixel32kLin(int x,int y,ulong color);
void _ASMAPI _SV_putPixel16mLin(int x,int y,ulong color);
void _ASMAPI _SV_putPixel4GLin(int x,int y,ulong color);
void _ASMAPI _SV_line256Lin(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_line32kLin(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_line16mLin(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_line4GLin(int x1,int y1,int x2,int y2,ulong color);
#endif

/* Declare all VBE/AF specific functions */

#ifdef	USE_VBEAF
void _ASMAPI _SV_setActivePageAF(int page);
void _ASMAPI _SV_setActivePageLinAF(int page);
void _ASMAPI _SV_setVisualPageAF(int page,bool waitVRT);
void _ASMAPI _SV_beginDirectAccessAFSLOW(void);
void _ASMAPI _SV_endDirectAccessAFSLOW(void);
void _ASMAPI _SV_beginPixelAFSLOW(void);
void _ASMAPI _SV_endPixelAFSLOW(void);
void _ASMAPI _SV_beginLineAFSLOW(void);
void _ASMAPI _SV_endLineAFSLOW(void);
void _ASMAPI _SV_beginDirectAccessAF(void);
void _ASMAPI _SV_endDirectAccessAF(void);
void _ASMAPI _SV_beginPixelAF(void);
void _ASMAPI _SV_endPixelAF(void);
void _ASMAPI _SV_beginLineAF(void);
void _ASMAPI _SV_endLineAF(void);
void _ASMAPI _SV_clearAF(ulong color);
void _ASMAPI _SV_putPixelAF(int x,int y,ulong color);
void _ASMAPI _SV_lineFastAF(int x1,int y1,int x2,int y2,ulong color);
void _ASMAPI _SV_softLineAF(int x1,int y1,int x2,int y2,ulong color);
#endif

/*----------------------------- Implementation ----------------------------*/

PRIVATE void ExitVBEBuf(void)
{
	if (VESABuf_sel)
		PM_freeRealSeg(VESABuf_sel,VESABuf_off);
	VESABuf_sel = 0;
}

void SVAPI SV_initRMBuf(void)
/****************************************************************************
*
* Function:	  	SV_initRMBuf
*
* Description:	Initialises the VBE transfer buffer in real mode DC.memory.
*				This routine is called by the VESAVBE module every time
*				it needs to use the transfer buffer, so we simply allocate
*				it once and then return.
*
****************************************************************************/
{
	if (!VESABuf_sel) {
		/* Allocate a global buffer for communicating with the VESA VBE */
		if (!PM_allocRealSeg(VESABuf_len, &VESABuf_sel, &VESABuf_off,
				&VESABuf_rseg, &VESABuf_roff))
			exit(1);
		VBE_init(VESABuf_len,VESABuf_sel,VESABuf_off,VESABuf_rseg,VESABuf_roff);
		atexit(ExitVBEBuf);
		}
}

void SV_nop(void) {}

PRIVATE SV_devCtx *InitVBE(void)
/****************************************************************************
*
* Function:     InitVBE
* Returns:      Pointer to device context block or 0 if no SuperVGA.
*
* Description:  Performs initialisation for VBE based devices.
*
****************************************************************************/
{
	VBE_vgaInfo		vgaInfo;
	VBE_modeInfo	modeInfo;
	int				i;
	ushort   		*p,tmodeList[MAXMODES];

	SV_initRMBuf();
	if ((DC.VBEVersion = VBE_detect(&vgaInfo)) == 0)
		return NULL;

	/* Copy relevent information from the mode block into our globals.
	 * Note that the video mode list _may_ be built in the information
	 * block that we have passed, so we _must_ copy this from here
	 * into our our storage if we want to continue to use it. Note
	 * that we filter out the mode 0x6A, which some BIOSes include as
	 * well as the 0x102 mode for 800x600x16.
	 */
	for (i = 0, p = vgaInfo.VideoModePtr; *p != 0xFFFF; p++) {
		if (*p >= 0x100)
			tmodeList[i++] = *p;
		}
	tmodeList[i] = -1;

	/* Now build our global list of available video modes, filtering out
	 * those modes that are not available or not graphics modes. A VBE
	 * implementation may put modes in the mode list that are not available
	 * on the current hardware configuration, so we need to check for this.
	 */
	DC.haveMultiBuffer = false;
	DC.haveVirtualScroll = false;
	for (i = 0,p = tmodeList; *p != 0xFFFF; p++) {
		if (!VBE_getModeInfo(*p,&modeInfo))
			continue;
		if ((modeInfo.ModeAttributes & vbeMdGraphMode) == 0)
			continue;
		if (!DC.linearAddr && (modeInfo.ModeAttributes & vbeMdLinear))
			DC.linearAddr = modeInfo.PhysBasePtr;
		if (modeInfo.NumberOfImagePages > 0) {
			/* If we have any graphics mode reports that it has more than
			 * 1 display page available, then we assume double buffering
			 * and virtual scrolling are available. Note that under VBE
			 * 1.2 and 2.0 these are one and the same, but under VBE/AF
			 * you can have cards that do double buffering but not virtual
			 * scrolling (and vice versa).
			 */
			DC.haveMultiBuffer = true;
			DC.haveVirtualScroll = true;
			}
		DC.modeList[i++] = *p;
		}
	DC.modeList[i] = -1;
	DC.memory = vgaInfo.TotalMemory * 64;
	DC.haveWideDAC = ((int)vgaInfo.Capabilities & vbe8BitDAC) != 0;
	strcpy(DC.OEMString,vgaInfo.OemStringPtr);

#ifndef	REALMODE
	if (DC.VBEVersion >= 0x200 && VF_available()) {
		int		codeLen;
		void 	*bankFunc;
		DC.haveVirtualBuffer = VBE_getBankFunc32(&codeLen,&bankFunc,0,0);
		}
	else
		DC.haveVirtualBuffer = false;
#endif

#ifdef	__WINDOWS16__
    linsdk_enable32();    /* Enable 32 bit linear framebuffer module  */
#endif
	return &DC;
}

PUBLIC SV_devCtx * SVAPI SV_init(bool useVBEAF)
/****************************************************************************
*
* Function:     SV_init
* Parameters:	useVBEAF	- True to use the VBE/AF driver if found
* Returns:      Pointer to device context block, or NULL if SVGA not found.
*
* Description:  Detects if a VESA VBE compliant SuperVGA is out there, and
*               initialises the library if one is. The VBE version number
*               is specified with the major version number in the high
*               byte and the minor version number in the low byte. So
*               version 1.2 is the number 0x102.
*
*				If we detect VBE/AF Accelerator Functions support, we
*               initialise the library to use VBE/AF rather than VBE 1.2/2.0
*				and return a DC.VBEVersion number of 2.0. It is up to the
*				application program to then check the DC.AFDC pointer to
*				determine if VBE/AF is available or not.
*
****************************************************************************/
{
	/* Initialise global pointers in global device context block */
	DC.modeList = _SV_modeList;
	DC.OEMString = _SV_OEMString;

#ifdef	USE_VBEAF
	DC.AFDC = NULL;
#endif

	if (useVBEAF) {
#ifdef	USE_VBEAF
		int			i;
		short 		*p;
		AF_modeInfo	modeInfo;

		if (loadedDC)
			DC.AFDC = loadedDC;
		else
			DC.AFDC = loadedDC = AF_loadDriver(NULL);
		if (DC.AFDC) {
			/* Copy list of available modes */
			for (i = 0,p = DC.AFDC->AvailableModes; *p != -1; p++) {
				if (AF_getVideoModeInfo(DC.AFDC,*p,&modeInfo) != 0)
					continue;
				DC.modeList[i++] = *p;
				}
			DC.modeList[i] = -1;

			/* Obtain configuration information from driver */
			DC.haveMultiBuffer = (DC.AFDC->Attributes & afHaveMultiBuffer);
			DC.haveVirtualScroll = (DC.AFDC->Attributes & afHaveVirtualScroll);
			DC.haveWideDAC = (DC.AFDC->Attributes & afHave8BitDAC);
			DC.haveAccel2D = (DC.AFDC->Attributes & afHaveAccel2D);
			DC.haveHWCursor = (DC.AFDC->Attributes & afHaveHWCursor);
			DC.linearAddr = DC.AFDC->LinearBasePtr;
			DC.memory = DC.AFDC->TotalMemory;
			strcpy(DC.OEMString,DC.AFDC->OemVendorName);
			DC.haveVirtualBuffer = (DC.AFDC->SetBank32 != NULL);
#ifdef	__WINDOWS16__
			linsdk_enable32();
#endif
			DC.VBEVersion = 0x200;
			return &DC;
			}
#endif
		}
	return InitVBE();
}

PUBLIC void SVAPI SV_exit(void)
/****************************************************************************
*
* Function:     SV_exit
*
* Description:	Clean up correctly for the SuperVGA Kit. Needed for
*				DLL versions when running under Windows 3.1 to properly
*				clean up after the DLL (exit functions dont seem to work).
*
****************************************************************************/
{
	ExitVBEBuf();
}

PUBLIC bool SVAPI SV_getModeInfo(ushort mode,SV_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     SV_getModeInfo
* Parameters:   mode		- Mode number for the mode
*				modeInfo	- Place to store the mode information
* Returns:		True if for valid mode, false if invalid.
*
* Description:	Obtains mode information for the specified mode number.
*				Note that we either call the VBE BIOS or the VBE/AF driver
*				for this information, massaging the information returned
*				into our common buffer format.
*
****************************************************************************/
{
	/* Mask off any extra flags from the mode number before we pass it
	 * onto the BIOS. We also ignore any non-VBE modes.
	 */
	if ((mode &= 0x1FF) < 0x100)
		return false;

	if (DC.AFDC) {
#ifdef	USE_VBEAF
		AF_modeInfo		miAF;

		if (AF_getVideoModeInfo(DC.AFDC,mode,&miAF) != 0)
			return false;
		modeInfo->Attributes = miAF.Attributes;
		modeInfo->XResolution = miAF.XResolution;
		modeInfo->YResolution = miAF.YResolution;
		modeInfo->BytesPerScanLine = miAF.BytesPerScanLine;
		modeInfo->BitsPerPixel = miAF.BitsPerPixel;
		modeInfo->NumberOfPages = miAF.MaxBuffers;
		modeInfo->RedMaskSize = miAF.RedMaskSize;
		modeInfo->RedFieldPosition = miAF.RedFieldPosition;
		modeInfo->GreenMaskSize = miAF.GreenMaskSize;
		modeInfo->GreenFieldPosition = miAF.GreenFieldPosition;
		modeInfo->BlueMaskSize = miAF.BlueMaskSize;
		modeInfo->BlueFieldPosition = miAF.BlueFieldPosition;
		modeInfo->RsvdMaskSize = miAF.RsvdMaskSize;
		modeInfo->RsvdFieldPosition = miAF.RsvdFieldPosition;
#endif
		}
	else {
		VBE_modeInfo	miVBE;

		if (!VBE_getModeInfo(mode,&miVBE))
			return false;
		if (!(miVBE.ModeAttributes & vbeMdGraphMode))
			return false;
		modeInfo->XResolution = miVBE.XResolution;
		modeInfo->YResolution = miVBE.YResolution;
		modeInfo->BytesPerScanLine = miVBE.BytesPerScanLine;
		modeInfo->BitsPerPixel = miVBE.BitsPerPixel;
		modeInfo->NumberOfPages = miVBE.NumberOfImagePages+1;
		modeInfo->RedMaskSize = miVBE.RedMaskSize;
		modeInfo->RedFieldPosition = miVBE.RedFieldPosition;
		modeInfo->GreenMaskSize = miVBE.GreenMaskSize;
		modeInfo->GreenFieldPosition = miVBE.GreenFieldPosition;
		modeInfo->BlueMaskSize = miVBE.BlueMaskSize;
		modeInfo->BlueFieldPosition = miVBE.BlueFieldPosition;
		modeInfo->RsvdMaskSize = miVBE.RsvdMaskSize;
		modeInfo->RsvdFieldPosition = miVBE.RsvdFieldPosition;

		/* Fill in Attributes field depending on what the controller can
		 * support.
		 */
		modeInfo->Attributes = svIsVBEMode;
		if (modeInfo->NumberOfPages > 1) {
			modeInfo->Attributes |= svHaveMultiBuffer;
			modeInfo->Attributes |= svHaveVirtualScroll;
			}
		if (!(miVBE.ModeAttributes & vbeMdNonBanked))
			modeInfo->Attributes |= svHaveBankedBuffer;
		if (miVBE.ModeAttributes & vbeMdLinear)
			modeInfo->Attributes |= svHaveLinearBuffer;
		if (miVBE.ModeAttributes & vbeMdNonVGA)
			modeInfo->Attributes |= svNonVGAMode;
		}
	modeInfo->Mode = mode;
	return true;
}

PUBLIC int SVAPI SV_getModeName(char *buf,SV_modeInfo *mi,ushort mode,bool useLinear)
/****************************************************************************
*
* Function:     SV_getModeName
* Parameters:   buf			- Buffer to put mode name into
*				mi			- Pointer to VBE mode info block
*				mode		- Mode number for the mode
*				useLinear	- True if we should the linear buffer if available
* Returns:		Mode number to set mode with (with appropriate flags added)
*
* Description:	Puts the name of the video mode in a standard format into
*				the string buffer, and returns the mode number to be used
*				to set the video mode.
*
****************************************************************************/
{
	char	buf2[80];
	int		attr = mi->Attributes;

	/* Ignore linear only modes if linear buffering not available */
	if (!(attr & svHaveBankedBuffer) && !useLinear)
		return 0;

	/* Build the name of the mode */
	sprintf(buf,"%4d x %4d %d bit (%2d page",mi->XResolution,mi->YResolution,
		mi->BitsPerPixel,mi->NumberOfPages);
	if (useLinear) {
		if ((attr & svHaveBankedBuffer) && (attr & svHaveLinearBuffer))
			sprintf(buf2,", Banked+Linear");
		else if (attr & svHaveLinearBuffer)
			sprintf(buf2,", Linear Only");
		else
			sprintf(buf2,", Banked Only");
#ifdef	USE_VBE20
		/* Use the linear framebuffer mode if available */
		if (attr & svHaveLinearBuffer)
			mode |= svLinearBuffer;
#endif
		}
	else
		sprintf(buf2,", Banked Only");
	strcat(buf,buf2);
	if (attr & svHaveAccel2D)
		strcat(buf,", Accelerated");
	if (attr & svNonVGAMode)
		strcat(buf,", NonVGA)");
	else
		strcat(buf,")");
	return mode;
}

PRIVATE bool SetModeVBE(ushort mode,bool use8BitDAC,bool useVirtualBuffer)
/****************************************************************************
*
* Function:     SetModeVBE
* Parameters:   mode    	- SuperVGA video mode to set.
*				use8BitDAC	- True to set up the 8 bit DAC mode
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode. This routine
*               assumes that the library and SuperVGA have been initialised
*               with the SV_init() routine first.
*
****************************************************************************/
{
	VBE_modeInfo	modeInfo;
	RMREGS      	regs;
	int				imode = mode & svModeMask;
	int				cntMode = VBE_getVideoMode();

	if (imode < 0x100 && imode != 0x13)
		return false;
	if (imode != cntMode && cntMode <= 3) {
		_SV_old50Lines = false;             /* Default to 25 line mode      */
		if (cntMode == 0x3) {
			regs.x.ax = 0x1130;
			regs.x.bx = 0;
			regs.x.dx = 0;
			PM_int86(0x10,&regs,&regs);
			_SV_old50Lines = (regs.h.dl == 49);
			}
		}

	if (!VBE_setVideoMode(mode))		/* Set the video mode			*/
		return false;

	/* Initialise global variables for current video mode dimensions	*/

	if (imode == 0x13) {
		/* Special case for VGA mode 13h */
		DC.maxx = 319;
		DC.maxy = 199;
		DC.bytesperline = 320;
		DC.bitsperpixel = 8;
		DC.maxpage = 0;
		_SV_pagesize = 0x10000L;
		_SV_bankShift = 0;
		}
	else {
		VBE_getModeInfo(imode,&modeInfo);
		DC.maxx = modeInfo.XResolution-1;
		DC.maxy = modeInfo.YResolution-1;
		DC.bytesperline = modeInfo.BytesPerScanLine;
		DC.bitsperpixel = modeInfo.BitsPerPixel;
		DC.maxpage = modeInfo.NumberOfImagePages;
		_SV_pagesize = VBE_getPageSize(&modeInfo);
		_SV_bankShift = 0;
		while ((64 >> _SV_bankShift) != modeInfo.WinGranularity)
			_SV_bankShift++;
		}
	DC.curBank = -1;

	/* Emulate RGB modes using a 3 3 2 palette arrangement by default */

	DC.redMask = 0x7;	DC.redPos = 5;		DC.redAdjust = 5;
	DC.greenMask = 0x7;	DC.greenPos = 2;	DC.greenAdjust = 5;
	DC.blueMask = 0x3;	DC.bluePos = 0;		DC.blueAdjust = 6;

	if (imode != 0x13 && modeInfo.MemoryModel == vbeMemRGB) {
		/* Save direct color info mask positions etc */

		DC.redMask = (0xFF >> (DC.redAdjust = 8 - modeInfo.RedMaskSize));
		DC.redPos = modeInfo.RedFieldPosition;
		DC.greenMask = (0xFF >> (DC.greenAdjust = 8 - modeInfo.GreenMaskSize));
		DC.greenPos = modeInfo.GreenFieldPosition;
		DC.blueMask = (0xFF >> (DC.blueAdjust = 8 - modeInfo.BlueMaskSize));
		DC.bluePos = modeInfo.BlueFieldPosition;
		}

	/* Set up a pointer to the appopriate bank switching code to use */
	if (imode == 0x13) {
		_SV_setBankPtr = (void*)SV_nop;
		}
	else {
		if ((modeInfo.WinAAttributes & 0x7) != 0x7) {
#ifdef	USE_VBE20
			if (DC.VBEVersion >= 0x200) {
				if (VBE_MMIOSel)
					_SV_setBankPtr = _VBE20_setBankAB_ES;
				else _SV_setBankPtr = _VBE20_setBankAB;
				}
			else
#endif
				_SV_setBankPtr = _VBE_setBankAB;
			}
		else {
#ifdef	USE_VBE20
			if (DC.VBEVersion >= 0x200) {
				if (VBE_MMIOSel)
					_SV_setBankPtr = _VBE20_setBankA_ES;
				else _SV_setBankPtr = _VBE20_setBankA;
				}
			else
#endif
				_SV_setBankPtr = _VBE_setBankA;
			}

#ifdef	REALMODE
		_SV_setBankRM = (void *)modeInfo.WinFuncPtr;
#else
		_SV_setBankRM = NULL;
#endif
		}

	if (imode == 0x13) {
		if ((DC.videoMem = PM_mapPhysicalAddr(0xA0000L,0xFFFF)) == NULL) {
			SV_restoreMode();
			exit(1);
			}
		}
	else {
		if ((DC.videoMem = VBE_getBankedPointer(&modeInfo)) == NULL) {
			SV_restoreMode();
			exit(1);
			}
		}
	DC.virtualBuffer = false;
	DC.setActivePage = _SV_setActivePage;
	DC.setVisualPage = _SV_setVisualPage;
#ifdef	USE_VBE20
	VBE_freePMCode();
	_SV_setBank20 = VBE_getSetBank();
	_SV_setCRT20 = VBE_getSetDisplayStart();
	_SV_setPal20 = VBE_getSetPalette();
	if ((mode & vbeLinearBuffer) && imode != 0x13) {
#ifdef	PM386
		if ((DC.videoMem = VBE_getLinearPointer(&modeInfo)) == NULL) {
			SV_restoreMode();
			exit(1);
			}
#else
		uint videoSel = VBE_getLinearSelector(&modeInfo);
		if (!videoSel) {
			SV_restoreMode();
			exit(1);
			}
		DC.videoMem = MK_FP(videoSel,0);
#endif
		DC.setActivePage = _SV_setActivePageLin;
		}
	else if (DC.haveVirtualBuffer && useVirtualBuffer && DC.bitsperpixel > 4) {
		/* See if we can use the VFlat virtual linear framebuffer. This
		 * does however require VBE 2.0 and the virtual flat linear
		 * framebuffer device support. 
		 */
		void *bankFunc,*p;
		int codeLen;
		VBE_getBankFunc32(&codeLen,&bankFunc,0,_SV_bankShift);
		VF_exit();
		if ((p = VF_init((ulong)modeInfo.WinASegment << 4,modeInfo.WinSize,
				codeLen,bankFunc)) != NULL) {
			DC.videoMem = p;
			DC.setActivePage = _SV_setActivePageLin;
			DC.virtualBuffer = true;
			mode |= vbeLinearBuffer;
			}
		}
#endif

	/* Now set up the vectors to the correct routines for the video
	 * mode type.
	 */
	switch (DC.bitsperpixel) {
		case 4:
			DC.clear = _SV_clear16;
			DC.putPixel = _SV_putPixel16;
			DC.line = _SV_line16;
			DC.maxcolor = DC.defcolor = 15;
			DC.bytesperpixel = 1;
			break;
		case 8:
			DC.clear = _SV_clear256;
			DC.putPixel = _SV_putPixel256;
			DC.line = _SV_line256;
			DC.maxcolor = 255;
			DC.defcolor = 15;
			DC.bytesperpixel = 1;
#ifdef	USE_VBE20
			if (mode & vbeLinearBuffer) {
				DC.clear = _SV_clear256Lin;
				DC.putPixel = _SV_putPixel256Lin;
				DC.line = _SV_line256Lin;
				}
#endif
			break;
		case 15:
		case 16:
			DC.clear = _SV_clear32k;
			DC.putPixel = _SV_putPixel32k;
			DC.line = _SV_line32k;
			DC.maxcolor = DC.defcolor = SV_rgbColor(0xFF,0xFF,0xFF);
			DC.bytesperpixel = 2;
#ifdef	USE_VBE20
			if (mode & vbeLinearBuffer) {
				DC.clear = _SV_clear32kLin;
				DC.putPixel = _SV_putPixel32kLin;
				DC.line = _SV_line32kLin;
				}
#endif
			break;
		case 24:
			DC.clear = _SV_clear16m;
			DC.putPixel = _SV_putPixel16m;
			DC.line = _SV_line16m;
			DC.maxcolor = DC.defcolor = SV_rgbColor(0xFF,0xFF,0xFF);
			DC.bytesperpixel = 3;
#ifdef	USE_VBE20
			if (mode & vbeLinearBuffer) {
				DC.clear = _SV_clear16mLin;
				DC.putPixel = _SV_putPixel16mLin;
				DC.line = _SV_line16mLin;
				}
#endif
			break;
		case 32:
			DC.clear = _SV_clear4G;
			DC.putPixel = _SV_putPixel4G;
			DC.line = _SV_line4G;
			DC.maxcolor = DC.defcolor = SV_rgbColor(0xFF,0xFF,0xFF);
			DC.bytesperpixel = 4;
#ifdef	USE_VBE20
			if (mode & vbeLinearBuffer) {
				DC.clear = _SV_clear4GLin;
				DC.putPixel = _SV_putPixel4GLin;
				DC.line = _SV_line4GLin;
				}
#endif
			break;
		}
	DC.beginDirectAccess = (void*)SV_nop;
	DC.endDirectAccess = (void*)SV_nop;
	DC.beginPixel = (void*)SV_nop;
	DC.putPixelFast = DC.putPixel;
	DC.endPixel = (void*)SV_nop;
	DC.beginLine = (void*)SV_nop;
	DC.lineFast = DC.line;
	DC.endLine = (void*)SV_nop;

	PM_saveDS();
	if (DC.bitsperpixel == 8) {
		/* Program the default VGA palette */
		_SV_convertPal = true;
		if (use8BitDAC && VBE_setDACWidth(8))
			_SV_convertPal = false;
		SV_setPalette(0,256,VGA8_defPal,-1);
		}
	DC.setActivePage(0);
	DC.setVisualPage(0,false);
	SV_clear(0);
	return true;
}

#ifdef	USE_VBEAF
PRIVATE bool SetModeAF(ushort mode,int virtualX,int virtualY,
	bool useVirtualBuffer,int numBuffers)
/****************************************************************************
*
* Function:     SetModeAF
* Parameters:   mode    - SuperVGA video mode to set.
* Returns:      True if the mode was set, false if not.
*
* Description:
*
****************************************************************************/
{
	AF_modeInfo	modeInfo;
	AF_int32 	bpl = -1;
	int			cntMode,imode = mode & svModeMask;
	RMREGS  	regs;

	/* Grab information about the current video mode */
	if (imode < 0x100)
		return false;
	regs.h.ah = 0x0F;
	PM_int86(0x10,&regs,&regs);
	cntMode = regs.h.al & 0x7F;
	if (imode != cntMode && cntMode <= 3) {
		_SV_old50Lines = false;             /* Default to 25 line mode      */
		if (cntMode == 0x3) {
			regs.x.ax = 0x1130;
			regs.x.bx = 0;
			regs.x.dx = 0;
			PM_int86(0x10,&regs,&regs);
			_SV_old50Lines = (regs.h.dl == 49);
			}
		}

	/* Turn off the double buffer flag if the user requested double
	 * buffering but only one pages is available. If we dont do this, the
	 * VBE/AF mode set code will fail the mode set.
	 */
	AF_getVideoModeInfo(DC.AFDC,imode,&modeInfo);
	if (!(modeInfo.Attributes & afHaveMultiBuffer))
		mode &= ~svMultiBuffer;

	/* Set the video mode */
	if (mode & afVirtualScroll) {
		if (AF_setVirtualVideoMode(DC.AFDC,mode,virtualX,virtualY,&bpl,
				numBuffers) != 0)
			return false;
		}
	else {
		if (AF_setVideoMode(DC.AFDC,mode,&bpl,numBuffers) != 0)
			return false;
		}

	/* Initialise global variables for current video mode dimensions	*/
	DC.bytesperline = bpl;
	DC.maxx = modeInfo.XResolution-1;
	DC.maxy = modeInfo.YResolution-1;
	DC.bitsperpixel = modeInfo.BitsPerPixel;
	DC.maxpage = modeInfo.MaxBuffers-1;
	_SV_pagesize = 0;
	_SV_bankShift = 0;
	DC.curBank = -1;

	/* Save direct color info mask positions etc. We emulate 8 bit RGB modes
	 * using a 3 3 2 palette arrangement by default
	 */
	DC.redMask = 0x7;		DC.redPos = 5;		DC.redAdjust = 5;
	DC.greenMask = 0x7;	DC.greenPos = 2;	DC.greenAdjust = 5;
	DC.blueMask = 0x3;		DC.bluePos = 0;	DC.blueAdjust = 6;
	if (DC.bitsperpixel > 8) {
		DC.redMask = (0xFF >> (DC.redAdjust = 8 - modeInfo.RedMaskSize));
		DC.redPos = modeInfo.RedFieldPosition;
		DC.greenMask = (0xFF >> (DC.greenAdjust = 8 - modeInfo.GreenMaskSize));
		DC.greenPos = modeInfo.GreenFieldPosition;
		DC.blueMask = (0xFF >> (DC.blueAdjust = 8 - modeInfo.BlueMaskSize));
		DC.bluePos = modeInfo.BlueFieldPosition;
		}

	/* Set up a pointer to the appopriate bank switching code to use */
	DC.virtualBuffer = false;
	_SV_setBankPtr = _VBEAF_setBank;
	DC.videoMem = DC.AFDC->BankedMem;
	DC.setActivePage = _SV_setActivePageAF;
	DC.setVisualPage = _SV_setVisualPageAF;
	if (mode & svLinearBuffer) {
		DC.videoMem = DC.AFDC->LinearMem;
		DC.setActivePage = _SV_setActivePageLinAF;
		}
	else if (DC.haveVirtualBuffer && useVirtualBuffer && DC.AFDC->SetBank32) {
		/* See if we can use the VFlat virtual linear framebuffer */
		void *bankFunc,*p;
		int codeLen;
		AF_getBankFunc32(DC.AFDC,&codeLen,&bankFunc);
		VF_exit();
		if ((p = VF_init(DC.AFDC->BankedBasePtr,DC.AFDC->BankSize,
				codeLen,bankFunc)) != NULL) {
			DC.videoMem = p;
			DC.setActivePage = _SV_setActivePageLinAF;
			DC.virtualBuffer = true;
			mode |= svLinearBuffer;
			}
		}

	/* Now set up the vectors to the correct routines for the video
	 * mode type.
	 */
	switch (DC.bitsperpixel) {
		case 8:
			DC.clear = _SV_clear256;
			DC.putPixel = _SV_putPixel256;
			DC.line = _SV_line256;
			DC.maxcolor = 255;
			DC.defcolor = 15;
			DC.bytesperpixel = 1;
			if (mode & svLinearBuffer) {
				DC.clear = _SV_clear256Lin;
				DC.putPixel = _SV_putPixel256Lin;
				DC.line = _SV_line256Lin;
				}
			break;
		case 15:
		case 16:
			DC.clear = _SV_clear32k;
			DC.putPixel = _SV_putPixel32k;
			DC.line = _SV_line32k;
			DC.maxcolor = DC.defcolor = SV_rgbColor(0xFF,0xFF,0xFF);
			DC.bytesperpixel = 2;
			if (mode & svLinearBuffer) {
				DC.clear = _SV_clear32kLin;
				DC.putPixel = _SV_putPixel32kLin;
				DC.line = _SV_line32kLin;
				}
			break;
		case 24:
			DC.clear = _SV_clear16m;
			DC.putPixel = _SV_putPixel16m;
			DC.line = _SV_line16m;
			DC.maxcolor = DC.defcolor = SV_rgbColor(0xFF,0xFF,0xFF);
			DC.bytesperpixel = 3;
			if (mode & svLinearBuffer) {
				DC.clear = _SV_clear16mLin;
				DC.putPixel = _SV_putPixel16mLin;
				DC.line = _SV_line16mLin;
				}
			break;
		case 32:
			DC.clear = _SV_clear4G;
			DC.putPixel = _SV_putPixel4G;
			DC.line = _SV_line4G;
			DC.maxcolor = DC.defcolor = SV_rgbColor(0xFF,0xFF,0xFF);
			DC.bytesperpixel = 4;
			if (mode & svLinearBuffer) {
				DC.clear = _SV_clear4GLin;
				DC.putPixel = _SV_putPixel4GLin;
				DC.line = _SV_line4GLin;
				}
			break;
		}

	/* Setup support for accelerator functions if we have these. Note that
	 * we transparently support acceleration of screen clearing and line
	 * drawing functions only. You can call the VBE/AF function to draw
	 * other primitives directly but we dont have software emulation for
	 * the routines if they are not available in hardware.
	 */
	DC.beginDirectAccess = (void*)SV_nop;
	DC.endDirectAccess = (void*)SV_nop;
	DC.beginPixel = (void*)SV_nop;
	DC.putPixelFast = DC.putPixel;
	DC.endPixel = (void*)SV_nop;
	DC.beginLine = (void*)SV_nop;
	DC.lineFast = DC.line;
	DC.endLine = (void*)SV_nop;
	if (modeInfo.Attributes & afHaveAccel2D) {
		if (DC.AFDC->EnableDirectAccess) {
			DC.beginDirectAccess = _SV_beginDirectAccessAFSLOW;
			DC.endDirectAccess = _SV_endDirectAccessAFSLOW;
			DC.beginPixel = _SV_beginPixelAFSLOW;
			DC.endPixel = _SV_endPixelAFSLOW;
			DC.beginLine = _SV_beginLineAFSLOW;
			DC.endLine = _SV_endLineAFSLOW;
			}
		else {
			DC.beginDirectAccess = _SV_beginDirectAccessAF;
			DC.endDirectAccess = _SV_endDirectAccessAF;
			DC.beginPixel = _SV_beginPixelAF;
			DC.endPixel = _SV_endPixelAF;
			DC.beginLine = _SV_beginLineAF;
			DC.endLine = _SV_endLineAF;
			}
		DC.clear = _SV_clearAF;
		DC.putPixel = _SV_putPixelAF;
		if (DC.AFDC->DrawLine) {
			/* Accelerated line drawing */
			DC.beginLine = (void*)SV_nop;
			DC.line = _SV_lineFastAF;
			DC.lineFast = _SV_lineFastAF;
			DC.endLine = (void*)SV_nop;
			}
		else {
			/* Software line drawing */
			DC.line = _SV_softLineAF;
			}

		/* Set default mix */
		AF_setMix(DC.AFDC,AF_REPLACE_MIX,AF_REPLACE_MIX);
		}

	PM_saveDS();
	if (DC.bitsperpixel == 8) {
		/* Program the default VGA palette. Note that VBE/AF always takes
		 * palette values in 8 bits per pixel format.
		 */
		_SV_convertPal = false;
		SV_setPalette(0,256,VGA8_defPal,-1);
		}
	DC.setActivePage(0);
	DC.setVisualPage(0,false);
	SV_clear(0);
	return true;
}

void SVAPI SV_setACCELDriver(AF_devCtx *driver)
{
	loadedDC = driver;
}
#endif

PUBLIC bool SVAPI SV_setMode(ushort mode,bool use8BitDAC,
	bool useVirtualBuffer,bool numBuffers)
/****************************************************************************
*
* Function:     SV_setMode
* Parameters:   mode    			- SuperVGA video mode to set.
*				use8BitDAC			- Turn on 8 bit DAC mode (for VBE modes only)
*				useVirtualBuffer	- True to use virtual linear buffer
*				numBuffers			- Number of image buffers to use
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode. This routine
*               assumes that the library and SuperVGA have been initialised
*               with the SV_init() routine first.
*
*				If the use8BitDAC flag is set, the 8 bit DAC mode will be
*				use for VBE graphics modes. For VBE/AF the 8 bit DAC mode
*				is used automatically if it is available, so this flag is
*				ignored.
*
****************************************************************************/
{
#ifdef	USE_VBEAF
	if (DC.AFDC)
		return SetModeAF(mode,-1,-1,useVirtualBuffer,numBuffers);
#endif
	return SetModeVBE(mode &= ~svMultiBuffer,use8BitDAC,useVirtualBuffer);
}

PUBLIC bool SVAPI SV_setVirtualMode(ushort mode,int virtualX,int virtualY,
	bool use8BitDAC,bool useVirtualBuffer,int numBuffers)
/****************************************************************************
*
* Function:     SV_setVirtualMode
* Parameters:   mode    - SuperVGA video mode to set.
*				virtualX	- Virtual screen X coordinate
*				virtualY	- Virtual screen Y coordinate
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode with virtual
*				scrolling enabled for the specified dimensions. If virtual
*				scrolling cannot be obtained correctly for this mode,
*				we return false.
*
****************************************************************************/
{
	int	newbytes;

#ifdef	USE_VBEAF
	if (DC.AFDC) {
		return SetModeAF(mode | afVirtualScroll,virtualX,virtualY,
			useVirtualBuffer,numBuffers);
		}
#endif
	if (!SetModeVBE(mode,use8BitDAC,useVirtualBuffer))
		return false;
	if (!VBE_setPixelsPerLine(virtualX,&newbytes,&virtualX,&virtualY)) {
		SV_restoreMode();
		return false;
		}
	DC.bytesperline = newbytes;
	DC.maxx = virtualX-1;
	DC.maxy = virtualY-1;
	return true;
}

PUBLIC void SVAPI SV_restoreMode(void)
/****************************************************************************
*
* Function:     SV_restoreMode
*
* Description:  Restore the previous video mode in use before the SuperVGA
*               mode was set. This routine will also restore the 50 line
*               display mode if this mode was previously set.
*
****************************************************************************/
{
	RMREGS	regs;

#ifdef	USE_VBE20
	VF_exit();
	VBE_freePMCode();				/* Free up protected mode code	*/
#endif
#ifdef	USE_VBEAF
	if (DC.AFDC)
		AF_restoreTextMode(DC.AFDC);
	else
#endif
		VBE_setVideoMode(0x3);		/* Reset to text mode       	*/
	if (_SV_old50Lines) {
		regs.x.ax = 0x1112;
		regs.x.bx = 0;
		PM_int86(0x10,&regs,&regs);	/* Restore 50 line mode         */
		}
}

PUBLIC void SVAPI SV_setPalette(int start,int num,SV_palette *pal,int maxProg)
/****************************************************************************
*
* Function:		SV_setPalette
* Parameters:   start   - Starting palette index to program
*               num     - Number of palette indexes to program
*               pal     - Palette buffer containing values
*               waitVRT - Wait for vertical retrace flag
* Returns:      True on success, false on failure
*
* Description:  Sets the palette by interleaving blocks of values with
*               the vertical retrace interval. We use the VBE 2.0 palette
*               interface routines if possible, and have a VGA style
*               routine for VBE 1.2 implementations.
*
*				Note that this routine does automatic conversion from 8
*				bits per primary formay to the 6 bit format required by
*				some cards if necessary. All data should be passed in 8
*				bits per primary format.
*
****************************************************************************/
{
	int		i,waitFlag,count;
	uchar	temp[1024],*t,*p = (uchar*)&pal[start];

	/* First copy the palette values into our temporary palette,
	 * translating from the internal 8 bit format to the VGA 6 bit
	 * format (if we have a wide palette, then we simply skip this step)
	 */
	if (_SV_convertPal) {
		for (i = 0,t = temp; i < num; i++) {
			*t++ = *p++ >> 2;
			*t++ = *p++ >> 2;
			*t++ = *p++ >> 2;
			t++; p++;
			}
		p = temp;
		}

	/* Determine if we need to wait for the vertical retrace */
	if (maxProg == -1) {
		waitFlag = 0x00;
		count = num;
		}
	else {
		waitFlag = 0x80;
		count = (num > maxProg) ? maxProg : num;
		}

	while (num) {
#ifdef	USE_VBEAF
		if (DC.AFDC)
			AF_setPaletteData(DC.AFDC,(AF_palette*)p,count,start,waitFlag == 0x80);
		else
#endif
		if (DC.VBEVersion < 0x200)
			_VGA_setPalette(start,count,(SV_palette*)p,waitFlag);
#ifdef	USE_VBE20
		else if (_SV_setPal20)
			_VBE20_setPalette(start,count,(SV_palette*)p,waitFlag);
#endif
		else
			VBE_setPalette(start,count,(VBE_palette*)p,waitFlag);
		start += count;
		p += count;
		num -= count;
		count = (num > maxProg) ? maxProg : num;
		}
}

PUBLIC ulong SVAPI SV_rgbColor(uchar r,uchar g,uchar b)
/****************************************************************************
*
* Function:     rgbColor
*
* Returns:      Value representing the color. The value is converted from
*               24 bit RGB space into the appropriate color for the
*               video mode.
*
****************************************************************************/
{
	return ((ulong)((r >> DC.redAdjust) & DC.redMask) << DC.redPos)
		 | ((ulong)((g >> DC.greenAdjust) & DC.greenMask) << DC.greenPos)
		 | ((ulong)((b >> DC.blueAdjust) & DC.blueMask) << DC.bluePos);
}

PUBLIC void SVAPI SV_writeText(int x,int y,char *str,ulong color)
/****************************************************************************
*
* Function:     writeText
* Parameters:   x,y     - Position to begin drawing string at
*               str     - String to draw
*
* Description:  Draws a string using the BIOS 8x16 video font by plotting
*               each pixel in the characters individually. This should
*               work for all video modes.
*
****************************************************************************/
{
    uchar           byte;
	int             i,j,k,length,ch;
	uchar          	*font;

	font = font8x16;
	length = strlen(str);
	DC.beginPixel();
	for (k = 0; k < length; k++) {
        ch = str[k];
		for (j = 0; j < 16; j++) {
            byte = *(font + ch * 16 + j);
            for (i = 0; i < 8; i++) {
                if ((byte & 0x80) != 0)
					DC.putPixelFast(x+i,y+j,color);
				byte <<= 1;
				}
			}
		x += 8;
		}
	DC.endPixel();
}

void SVAPI SV_setDisplayStart(int x,int y,bool waitVRT)
/****************************************************************************
*
* Function:     SV_setDisplayStart
* Parameters:   x,y - Position of the first pixel to display
*
* Description:  Sets the new starting display position to implement
*               hardware scrolling.
*
****************************************************************************/
{
#ifdef  USE_VBEAF
	if (DC.AFDC)
		AF_setDisplayStart(DC.AFDC,x,y,waitVRT);
	else
#endif
		VBE_setDisplayStart(x,y,waitVRT);
}

#ifdef	USE_VBEAF

PUBLIC void _ASMAPI _SV_setActivePageAF(int page)
{
	ulong	addr;

	/* Set active buffer and update internal pointers */
	AF_setActiveBuffer(DC.AFDC,page);
	addr = DC.AFDC->OriginOffset;
	DC.originOffset = (ushort)addr;
	SV_setBank(DC.bankOffset = (addr >> 16));

	/* Set hardware clip rectangle to new active page */
	if (DC.AFDC->SetClipRect)
		AF_setClipRect(DC.AFDC,0,0,DC.maxx,DC.maxy);
}

PUBLIC void _ASMAPI _SV_setActivePageLinAF(int page)
{
	/* Set active buffer and update internal pointers */
	AF_setActiveBuffer(DC.AFDC,page);
	DC.originOffset = (ulong)DC.videoMem + DC.AFDC->OriginOffset;

	/* Set hardware clip rectangle to new active page */
	if (DC.AFDC->SetClipRect)
		AF_setClipRect(DC.AFDC,0,0,DC.maxx,DC.maxy);
}

PUBLIC void _ASMAPI _SV_setVisualPageAF(int page,bool waitVRT)
{
	AF_setVisibleBuffer(DC.AFDC,page,waitVRT);
}

PUBLIC void _ASMAPI _SV_clearAF(ulong color)
{
	AF_drawRect(DC.AFDC,color,0,0,DC.maxx+1,DC.maxy+1);
}

PUBLIC void _ASMAPI _SV_putPixelAF(int x,int y,ulong color)
{
	DC.beginPixel();
	DC.putPixelFast(x,y,color);
	DC.endPixel();
}

PUBLIC void _ASMAPI _SV_softLineAF(int x1,int y1,int x2,int y2,ulong color)
{
	DC.beginLine();
	DC.lineFast(x1,y1,x2,y2,color);
	DC.endLine();
}

#endif

#ifdef	__WINDOWS16__

/* Windows specific stubs to handle different calling conventions for
 * 16 bit DLL's. We export all functions as pascal calling conventions,
 * and we do an internal thunk to the C calling conventions that we
 * require internally.
 */

void _ASMAPI _SV_setBank(int bank);
int _ASMAPI _SV_queryCpu(void);

void SVAPI SV_setBank(int bank)
{ _SV_setBank(bank); }

int SVAPI SV_queryCpu(void)
{ return _SV_queryCpu(); }

void SVAPI SV_beginPixel(void)
{ DC.beginPixel(); }

void SVAPI SV_putPixel(int x,int y,ulong color)
{ DC.putPixel(x,y,color); }

void SVAPI SV_putPixelFast(int x,int y,ulong color)
{ DC.putPixelFast(x,y,color); }

void SVAPI SV_endPixel(void)
{ DC.endPixel(); }

void SVAPI SV_clear(ulong color)
{ DC.clear(color); }

void SVAPI SV_line(int x1,int y1,int x2,int y2,ulong color)
{ DC.line(x1,y1,x2,y2,color); }

void SVAPI SV_beginLine(void)
{ DC.beginLine(); }

void SVAPI SV_lineFast(int x1,int y1,int x2,int y2,ulong color)
{ DC.lineFast(x1,y1,x2,y2,color); }

void SVAPI SV_endLine(void)
{ DC.endLine(); }

void SVAPI SV_setActivePage(int page)
{ DC.setActivePage(page); }

void SVAPI SV_setVisualPage(int page,bool waitVRT)
{ DC.setVisualPage(page,waitVRT); }

void SVAPI SV_beginDirectAccess(void)
{ DC.beginDirectAccess(); }

void SVAPI SV_endDirectAccess(void)
{ DC.endDirectAccess(); }

#endif
