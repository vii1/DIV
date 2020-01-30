/****************************************************************************
*
*           The SuperVGA Kit - UniVBE Software Development Kit
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
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pmpro.h"
#include "svga.h"
#include "wdirect.h"
#include "gtf.h"
#ifdef	__MSDOS__
#include <dos.h>
#endif

/* If we are compiling for either 32 bit protected mode or 16 bit Windows,
 * then we can use the VBE 2.0 protected mode functions and linear
 * framebuffer code, and also VBE/AF Accelerator Functions.
 */

#if defined(PM386) || defined(__WINDOWS16__) || defined(TESTING_PM)
#define USE_VBE20
#endif

#if !defined(__16BIT__) || defined(TESTING_PM)
#define USE_VBEAF
#endif

/*---------------------------- Global Variables ---------------------------*/

#define MAXMODES    100             /* Maximum modes available in list  */

#define DC  _SV_devCtx              /* Macro to access global structure */

SV_devCtx _ASMAPI _SV_devCtx = {0};
AF_devCtx		*_SV_loadedAF = NULL;
#ifdef  USE_VBEAF
static ibool		haveAccel2D;		/* True if we have 2D acceleration	*/
#endif
static ushort  	_SV_modeList[MAXMODES];
static char    	_SV_OEMString[80];
static ibool    	_SV_old50Lines;     /* Was old mode 80x50?              */
static ibool    	_SV_convertPal;     /* True to convert palette entries  */
int     _ASMAPI _SV_bankShift;		/* Bank granularity adjust factor 	*/
long    _ASMAPI _SV_pagesize;   	/* Page size for current mode   	*/
void  	* _ASMAPI _SV_setBankRM;	/* Pointer to direct VBE bank code	*/
void 	* _ASMAPI _SV_setBankPtr;	/* Pointer to bank switch code 		*/
#ifdef	__WINDOWS__
static	HWND	hwndMain = NULL;	/* Main window for application		*/
#endif

void _ASMAPI _VBE_setBankA(void);   /* VBE setBank routine single bank  */
void _ASMAPI _VBE_setBankAB(void);  /* VBE setBank routine dual banks   */

#ifdef  USE_VBE20
void _ASMAPI _VBE20_setBankA(void);     /* VBE 2.0 setBank single bank  */
void _ASMAPI _VBE20_setBankAB(void);    /* VBE 2.0 setBank dual banks   */
void _ASMAPI _VBE20_setBankA_ES(void);  /* VBE 2.0 setBank for MMIO     */
void _ASMAPI _VBE20_setBankAB_ES(void); /* VBE 2.0 setBank for MMIO     */
void * _ASMAPI _SV_setBank20 = NULL;	/* Pointer to set bank routine	*/
void * _ASMAPI _SV_setCRT20 = NULL;		/* Pointer to set CRT routine	*/
void * _ASMAPI _SV_setPal20 = NULL;		/* Pointer to set palette rtn	*/
extern  short _ASMAPI VBE_MMIOSel;      /* Selector to MMIO registers   */
#endif

#ifdef  USE_VBEAF
void SVAPI _VBEAF10_setBank(void);	/* VBE/AF setBank function          */
void SVAPI _VBEAF20_setBank(void);	/* VBE/AF setBank function          */
#endif

extern  uchar _ASMAPI font8x16[];   /* Bitmap font definition           */
extern  SV_palette _ASMAPI VGA8_defPal[];

/*----------------------------- Implementation ----------------------------*/

/* Declare all banked framebuffer routines */

void _ASMAPI _SV_setActivePage(int page);
void _ASMAPI _SV_callSetCRT20(ulong addr,int waitFlag);
void _ASMAPI _SV_waitDEVGA(void);
void _ASMAPI _SV_waitVRTVGA(void);
void _ASMAPI _VGA_setPalette(int start,int num,SV_palette *pal,ibool waitVRT);
void _ASMAPI _VBE20_setPalette(int start,int num,SV_palette *pal,ibool waitVRT);
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

#ifdef  USE_VBE20
#ifdef  __WINDOWS16__
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

#ifdef  USE_VBEAF
void _ASMAPI _SV_setActivePageAF(int page);
void _ASMAPI _SV_setActivePageLinAF(int page);
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

#pragma warn -par

void SVAPI SV_initRMBuf(void)
/****************************************************************************
*
* Function:     SV_initRMBuf
*
* Description:  Initialises the VBE transfer buffer in real mode DC.memory.
*               This routine is called by the VESAVBE module every time
*               it needs to use the transfer buffer, so we simply allocate
*               it once and then return.
*
****************************************************************************/
{
	/* Initialise the VBE module */
	VBE_init();
}

static void SV_nop(void) {}

static SV_devCtx *InitVBE(void)
/****************************************************************************
*
* Function:     InitVBE
* Returns:      Pointer to device context block or 0 if no SuperVGA.
*
* Description:  Performs initialisation for VBE based devices.
*
****************************************************************************/
{
    VBE_vgaInfo     vgaInfo;
    VBE_modeInfo    modeInfo;
    int             i;
    ushort          *p,tmodeList[MAXMODES];

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
    for (i = 0, p = vgaInfo.VideoModePtr; (*p != 0xFFFF && i < MAXMODES); p++) {
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
	DC.haveTripleBuffer = false;
	DC.haveStereo = false;
	DC.linearAddr = 0;
	for (i = 0,p = tmodeList; (*p != 0xFFFF && i < MAXMODES); p++) {
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
		if (modeInfo.ModeAttributes & vbeMdTripleBuf)
			DC.haveTripleBuffer = true;
		if (modeInfo.ModeAttributes & vbeMdStereo)
			DC.haveStereo = true;
		DC.modeList[i++] = *p;
		}
	DC.modeList[i] = -1;
	DC.memory = vgaInfo.TotalMemory * 64;
	DC.haveRefreshCtrl = (DC.VBEVersion >= 0x300);
	DC.haveWideDAC = ((int)vgaInfo.Capabilities & vbe8BitDAC) != 0;
	DC.haveNonVGA = ((int)vgaInfo.Capabilities & vbeNonVGA) != 0;
	DC.haveHWStereoSync = ((int)vgaInfo.Capabilities & vbeHWStereoSync) != 0;
	DC.haveEVCStereoSync = ((int)vgaInfo.Capabilities & vbeEVCStereoSync) != 0;
	DC.useBlueCode = !DC.haveHWStereoSync;
	DC.blueCodeIndex = 255;
	strcpy(DC.OEMString,vgaInfo.OemStringPtr);

#ifndef REALMODE
	if (VF_available()) {
		int     codeLen;
	   void    *bankFunc;
        DC.haveVirtualBuffer = VBE_getBankFunc32(&codeLen,&bankFunc,0,0);
        }
    else
        DC.haveVirtualBuffer = false;
#endif

#ifdef  __WINDOWS16__
    linsdk_enable32();    /* Enable 32 bit linear framebuffer module  */
#endif
    return &DC;
}

SV_devCtx * SVAPI SV_init(ibool useVBEAF)
/****************************************************************************
*
* Function:     SV_init
* Parameters:   useVBEAF    - True to use the VBE/AF driver if found
* Returns:      Pointer to device context block, or NULL if SVGA not found.
*
* Description:  Detects if a VESA VBE compliant SuperVGA is out there, and
*               initialises the library if one is. The VBE version number
*               is specified with the major version number in the high
*               byte and the minor version number in the low byte. So
*               version 1.2 is the number 0x102.
*
*               If we detect VBE/AF Accelerator Functions support, we
*               initialise the library to use VBE/AF rather than VBE 1.2/2.0
*               and return a DC.VBEVersion number of 2.0. It is up to the
*               application program to then check the DC.AFDC pointer to
*               determine if VBE/AF is available or not.
*
****************************************************************************/
{
    /* Initialise global pointers in global device context block */
    DC.modeList = _SV_modeList;
    DC.OEMString = _SV_OEMString;

#ifdef  USE_VBEAF
	DC.AFDC = NULL;
	if (!_SV_loadedAF) {
		if (DC.loadedAF)
			AF_unloadDriver(DC.loadedAF);
		DC.loadedAF = AF_loadDriver(NULL);
		}
	else
		DC.loadedAF = _SV_loadedAF;
#endif

	if (useVBEAF) {
#ifdef  USE_VBEAF
		int         i;
		short       *p;
		AF_modeInfo modeInfo;

		if ((DC.AFDC = DC.loadedAF) != NULL) {
            /* Copy list of available modes */
            for (i = 0,p = DC.AFDC->AvailableModes; (*p != -1 && i < MAXMODES); p++) {
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
			DC.haveRefreshCtrl = (DC.AFDC->Version >= 0x200);
			DC.haveTripleBuffer = (DC.AFDC->Attributes & afHaveTripleBuffer);
			DC.haveStereo = (DC.AFDC->Attributes & afHaveStereo);
			DC.haveHWStereoSync = (DC.AFDC->Attributes & afHaveHWStereoSync);
			DC.haveEVCStereoSync = (DC.AFDC->Attributes & afHaveEVCStereoSync);
			DC.useBlueCode = !DC.haveHWStereoSync;
			DC.blueCodeIndex = 255;
			DC.haveNonVGA = (DC.AFDC->Attributes & afNonVGAMode);
			DC.linearAddr = DC.AFDC->LinearBasePtr;
            DC.memory = DC.AFDC->TotalMemory;
            strcpy(DC.OEMString,DC.AFDC->OemVendorName);
            DC.haveVirtualBuffer = (DC.AFDC->SetBank32 != NULL);
#ifdef  __WINDOWS16__
            linsdk_enable32();
#endif
            DC.VBEVersion = 0x200;
            return &DC;
            }
#endif
        }
	return InitVBE();
}

void SVAPI SV_exit(void)
/****************************************************************************
*
* Function:     SV_exit
*
* Description:  Clean up correctly for the SuperVGA Kit. Needed for
*               DLL versions when running under Windows 3.1 to properly
*               clean up after the DLL (exit functions dont seem to work).
*
****************************************************************************/
{
}

ibool SVAPI SV_getModeInfo(ushort mode,SV_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     SV_getModeInfo
* Parameters:   mode        - Mode number for the mode
*               modeInfo    - Place to store the mode information
* Returns:      True if for valid mode, false if invalid.
*
* Description:  Obtains mode information for the specified mode number.
*               Note that we either call the VBE BIOS or the VBE/AF driver
*               for this information, massaging the information returned
*               into our common buffer format.
*
****************************************************************************/
{
    /* Mask off any extra flags from the mode number before we pass it
	 * onto the BIOS.
     */
	mode &= svModeMask;

	/* Clear initial mode info block to all zeroes */
	memset(modeInfo,0,sizeof(*modeInfo));
	if (DC.AFDC) {
#ifdef  USE_VBEAF
        AF_modeInfo     miAF;

		if (AF_getVideoModeInfo(DC.AFDC,mode,&miAF) != 0)
            return false;
		modeInfo->Attributes = miAF.Attributes;
        modeInfo->XResolution = miAF.XResolution;
        modeInfo->YResolution = miAF.YResolution;
        modeInfo->BitsPerPixel = miAF.BitsPerPixel;

        modeInfo->BytesPerScanLine = miAF.BytesPerScanLine;
        modeInfo->NumberOfPages = miAF.MaxBuffers;
        modeInfo->RedMaskSize = miAF.RedMaskSize;
        modeInfo->RedFieldPosition = miAF.RedFieldPosition;
        modeInfo->GreenMaskSize = miAF.GreenMaskSize;
        modeInfo->GreenFieldPosition = miAF.GreenFieldPosition;
        modeInfo->BlueMaskSize = miAF.BlueMaskSize;
        modeInfo->BlueFieldPosition = miAF.BlueFieldPosition;
        modeInfo->RsvdMaskSize = miAF.RsvdMaskSize;
        modeInfo->RsvdFieldPosition = miAF.RsvdFieldPosition;
		if (DC.AFDC->Version >= 0x200) {
			/* VBE/AF 2.0 spec and later */
			modeInfo->LinBytesPerScanLine = miAF.LinBytesPerScanLine;
            modeInfo->BnkNumberOfPages = miAF.BnkMaxBuffers;
            modeInfo->LinNumberOfPages = miAF.LinMaxBuffers;
            modeInfo->LinRedMaskSize = miAF.LinRedMaskSize;
            modeInfo->LinRedFieldPosition = miAF.LinRedFieldPosition;
            modeInfo->LinGreenMaskSize = miAF.LinGreenMaskSize;
            modeInfo->LinGreenFieldPosition = miAF.LinGreenFieldPosition;
			modeInfo->LinBlueMaskSize = miAF.LinBlueMaskSize;
			modeInfo->LinBlueFieldPosition = miAF.LinBlueFieldPosition;
			modeInfo->LinRsvdMaskSize = miAF.LinRsvdMaskSize;
			modeInfo->LinRsvdFieldPosition = miAF.LinRsvdFieldPosition;
			modeInfo->MaxPixelClock	= miAF.MaxPixelClock;
			modeInfo->VideoCapabilities = miAF.VideoCapabilities;
			modeInfo->VideoMinXScale = miAF.VideoMinXScale;
			modeInfo->VideoMinYScale = miAF.VideoMinYScale;
			modeInfo->VideoMaxXScale = miAF.VideoMaxXScale;
			modeInfo->VideoMaxYScale = miAF.VideoMaxYScale;
			}
		else {
            /* Older version of the VBE/AF spec so use banked values */
            modeInfo->LinBytesPerScanLine = miAF.BytesPerScanLine;
            modeInfo->BnkNumberOfPages = miAF.MaxBuffers;
            modeInfo->LinNumberOfPages = miAF.MaxBuffers;
            modeInfo->LinRedMaskSize = miAF.RedMaskSize;
            modeInfo->LinRedFieldPosition = miAF.RedFieldPosition;
            modeInfo->LinGreenMaskSize = miAF.GreenMaskSize;
            modeInfo->LinGreenFieldPosition = miAF.GreenFieldPosition;
			modeInfo->LinBlueMaskSize = miAF.BlueMaskSize;
			modeInfo->LinBlueFieldPosition = miAF.BlueFieldPosition;
            modeInfo->LinRsvdMaskSize = miAF.RsvdMaskSize;
            modeInfo->LinRsvdFieldPosition = miAF.RsvdFieldPosition;
            }
#endif
        }
    else {
        VBE_modeInfo    miVBE;

        if (!VBE_getModeInfo(mode,&miVBE))
            return false;
        if (!(miVBE.ModeAttributes & vbeMdGraphMode))
            return false;
        modeInfo->XResolution = miVBE.XResolution;
        modeInfo->YResolution = miVBE.YResolution;
        modeInfo->BitsPerPixel = miVBE.BitsPerPixel;
        modeInfo->BytesPerScanLine = miVBE.BytesPerScanLine;
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
		if (miVBE.ModeAttributes & vbeMdDoubleScan)
			modeInfo->Attributes |= svHaveDoubleScan;
		if (miVBE.ModeAttributes & vbeMdInterlaced)
			modeInfo->Attributes |= svHaveInterlaced;
		if (miVBE.ModeAttributes & vbeMdTripleBuf)
			modeInfo->Attributes |= svHaveTripleBuffer;
		if (miVBE.ModeAttributes & vbeMdStereo)
			modeInfo->Attributes |= svHaveStereo;
		if (miVBE.ModeAttributes & vbeMdDualDisp)
			modeInfo->Attributes |= svHaveDualDisp;

		if (DC.VBEVersion >= 0x300) {
			/* VBE 3.0 and later extra information for properly handling
             * the linear framebuffer modes.
             */
            modeInfo->LinBytesPerScanLine = miVBE.LinBytesPerScanLine;
            modeInfo->BnkNumberOfPages = miVBE.BnkNumberOfImagePages+1;
            modeInfo->LinNumberOfPages = miVBE.LinNumberOfImagePages+1;
            modeInfo->LinRedMaskSize = miVBE.LinRedMaskSize;
            modeInfo->LinRedFieldPosition = miVBE.LinRedFieldPosition;
            modeInfo->LinGreenMaskSize = miVBE.LinGreenMaskSize;
            modeInfo->LinGreenFieldPosition = miVBE.LinGreenFieldPosition;
            modeInfo->LinBlueMaskSize = miVBE.LinBlueMaskSize;
            modeInfo->LinBlueFieldPosition = miVBE.LinBlueFieldPosition;
            modeInfo->LinRsvdMaskSize = miVBE.LinRsvdMaskSize;
            modeInfo->LinRsvdFieldPosition = miVBE.LinRsvdFieldPosition;
			modeInfo->MaxPixelClock	= miVBE.MaxPixelClock;
			}
        else {
            /* Older VBE 2.0 version of the spec, so use the values
             * for the banked modes.
             */
            modeInfo->LinBytesPerScanLine = miVBE.BytesPerScanLine;
            modeInfo->BnkNumberOfPages = miVBE.NumberOfImagePages+1;
            modeInfo->LinNumberOfPages = miVBE.NumberOfImagePages+1;
            modeInfo->LinRedMaskSize = miVBE.RedMaskSize;
            modeInfo->LinRedFieldPosition = miVBE.RedFieldPosition;
            modeInfo->LinGreenMaskSize = miVBE.GreenMaskSize;
            modeInfo->LinGreenFieldPosition = miVBE.GreenFieldPosition;
            modeInfo->LinBlueMaskSize = miVBE.BlueMaskSize;
            modeInfo->LinBlueFieldPosition = miVBE.BlueFieldPosition;
            modeInfo->LinRsvdMaskSize = miVBE.RsvdMaskSize;
            modeInfo->LinRsvdFieldPosition = miVBE.RsvdFieldPosition;
            }
        }
	modeInfo->Mode = mode;
	return true;
}

int SVAPI SV_getModeName(char *buf,SV_modeInfo *mi,ushort mode,ibool useLinear)
/****************************************************************************
*
* Function:     SV_getModeName
* Parameters:   buf         - Buffer to put mode name into
*               mi          - Pointer to VBE mode info block
*               mode        - Mode number for the mode
*               useLinear   - True if we should the linear buffer if available
* Returns:      Mode number to set mode with (with appropriate flags added)
*
* Description:  Puts the name of the video mode in a standard format into
*               the string buffer, and returns the mode number to be used
*               to set the video mode.
*
****************************************************************************/
{
    char    buf1[80],buf2[80];
    int     bankedAndLinear,attr = mi->Attributes;

    /* Ignore linear only modes if linear buffering not available */
    if (!(attr & svHaveBankedBuffer) && !useLinear)
        return 0;

    /* Build the name of the mode */
    if (useLinear) {
		if ((attr & svHaveBankedBuffer) && (attr & svHaveLinearBuffer)) {
            sprintf(buf2,", Banked+Linear");
            bankedAndLinear = true;
            }
        else if (attr & svHaveLinearBuffer) {
            sprintf(buf2,", Linear Only");
            bankedAndLinear = false;
            }
        else {
            sprintf(buf2,", Banked Only");
            bankedAndLinear = false;
            }
#ifdef  USE_VBE20
        /* Use the linear framebuffer mode if available */
        if (attr & svHaveLinearBuffer)
            mode |= svLinearBuffer;
#endif
        }
    else {
        sprintf(buf2,", Banked Only");
        bankedAndLinear = false;
        }
	if (bankedAndLinear && (mi->BnkNumberOfPages != mi->LinNumberOfPages))
		sprintf(buf1,"%d/%d", mi->BnkNumberOfPages, mi->LinNumberOfPages);
	else {
		if (useLinear && (attr & svHaveLinearBuffer))
			sprintf(buf1,"%d", mi->LinNumberOfPages);
		else
			sprintf(buf1,"%d", mi->BnkNumberOfPages);
		}
	sprintf(buf,"%dx%dx%d, %s page%s",mi->XResolution,mi->YResolution,
		mi->BitsPerPixel,buf1,buf2);
	if (attr & svHaveAccel2D)
		strcat(buf,", Accel");
	if (attr & svHaveTripleBuffer)
		strcat(buf,", TripleBuf");
	if (attr & svHaveStereo)
		strcat(buf,", Stereo");
	if (attr & svNonVGAMode)
		strcat(buf,", NonVGA");
	return mode;
}

static ibool SetModeVBE(ushort mode,ibool use8BitDAC,ibool useVirtualBuffer,
	ibool setPalette,SV_CRTCInfo *crtc)
/****************************************************************************
*
* Function:     SetModeVBE
* Parameters:   mode        - SuperVGA video mode to set.
*               use8BitDAC  - True to set up the 8 bit DAC mode
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode. This routine
*               assumes that the library and SuperVGA have been initialised
*               with the SV_init() routine first.
*
****************************************************************************/
{
    VBE_modeInfo    modeInfo;
    RMREGS          regs;
    int             imode = mode & svModeMask;
    int             cntMode = VBE_getVideoMode();

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

	/* Set the video mode */
	if (!VBE_setVideoModeExt(mode,(VBE_CRTCInfo*)crtc))
        return false;

    /* Initialise global variables for current video mode dimensions    */
	if (imode == 0x13) {
        /* Special case for VGA mode 13h */
		DC.maxx = 319;
        DC.maxy = 199;
        DC.bytesperline = 320;
        DC.bitsperpixel = 8;
        DC.maxpage = 0;
        _SV_pagesize = 0x10000L;
		_SV_bankShift = 0;
		DC.isNonVGA = false;
        }
    else {
        VBE_getModeInfo(imode,&modeInfo);
        DC.maxx = modeInfo.XResolution-1;
        DC.maxy = modeInfo.YResolution-1;
        DC.bitsperpixel = modeInfo.BitsPerPixel;
		if (DC.VBEVersion >= 0x300) {
			/* VBE 3.0 extensions for linear buffer info */
			if (mode & vbeLinearBuffer) {
				DC.maxpage = modeInfo.LinNumberOfImagePages;
				DC.bytesperline = modeInfo.LinBytesPerScanLine;
				}
			else {
				DC.maxpage = modeInfo.BnkNumberOfImagePages;
				DC.bytesperline = modeInfo.BytesPerScanLine;
				}
            }
        else {
            DC.bytesperline = modeInfo.BytesPerScanLine;
            DC.maxpage = modeInfo.NumberOfImagePages;
            }
		_SV_pagesize = (long)modeInfo.BytesPerScanLine * (long)modeInfo.YResolution;
		_SV_bankShift = 0;
        while ((64 >> _SV_bankShift) != modeInfo.WinGranularity)
			_SV_bankShift++;

		/* If the controller is reported as NonVGA, then we must use the
		 * provided VBE 2.0 palette programming functions. Otherwise we
		 * can simply program the VGA palette registers directly.
		 */
		DC.isNonVGA = ((modeInfo.ModeAttributes & vbeMdNonVGA) != 0);
		}
	DC.curBank = -1;

    /* Emulate RGB modes using a 3 3 2 palette arrangement by default */
	DC.redMask = 0x7;   DC.redPos = 5;      DC.redAdjust = 5;
    DC.greenMask = 0x7; DC.greenPos = 2;    DC.greenAdjust = 5;
	DC.blueMask = 0x3;  DC.bluePos = 0;     DC.blueAdjust = 6;

    if (imode != 0x13 && modeInfo.MemoryModel == vbeMemRGB) {
        /* Save direct color info mask positions etc */

		if ((mode & vbeLinearBuffer) && (DC.VBEVersion >= 0x300)) {
			/* VBE 3.0 extended linear mode color format information */
            DC.redMask = (0xFF >> (DC.redAdjust = 8 - modeInfo.LinRedMaskSize));
            DC.redPos = modeInfo.LinRedFieldPosition;
            DC.greenMask = (0xFF >> (DC.greenAdjust = 8 - modeInfo.LinGreenMaskSize));
            DC.greenPos = modeInfo.LinGreenFieldPosition;
            DC.blueMask = (0xFF >> (DC.blueAdjust = 8 - modeInfo.LinBlueMaskSize));
            DC.bluePos = modeInfo.LinBlueFieldPosition;
            }
        else {
            /* Banked color format information */
            DC.redMask = (0xFF >> (DC.redAdjust = 8 - modeInfo.RedMaskSize));
            DC.redPos = modeInfo.RedFieldPosition;
            DC.greenMask = (0xFF >> (DC.greenAdjust = 8 - modeInfo.GreenMaskSize));
            DC.greenPos = modeInfo.GreenFieldPosition;
            DC.blueMask = (0xFF >> (DC.blueAdjust = 8 - modeInfo.BlueMaskSize));
            DC.bluePos = modeInfo.BlueFieldPosition;
            }
        }

	/* Get pointer to video memory */
	if (imode == 0x13) {
#ifdef  __WIN386__
        ushort sel = PM_createSelector(0xA0000L,0xFFFF);
        if (!sel) {
            SV_restoreMode();
            exit(1);
            }
        DC.videoMem = (void*)((ulong)sel << 16);
#else
        if ((DC.videoMem = PM_mapPhysicalAddr(0xA0000L,0xFFFF)) == NULL) {
            SV_restoreMode();
            exit(1);
            }
#endif
        }
    else {
        if ((DC.videoMem = VBE_getBankedPointer(&modeInfo)) == NULL) {
            SV_restoreMode();
            exit(1);
			}
        }
    DC.virtualBuffer = false;
    DC.setActivePage = _SV_setActivePage;
#ifdef  USE_VBE20
    VBE_freePMCode();
    _SV_setBank20 = VBE_getSetBank();
    _SV_setCRT20 = VBE_getSetDisplayStart();
    _SV_setPal20 = VBE_getSetPalette();
    if ((mode & vbeLinearBuffer) && imode != 0x13) {
#ifdef  PM386
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

	/* Set up a pointer to the appopriate bank switching code to use */
	_SV_setBankPtr = (void*)SV_nop;
	if (imode != 0x13) {
		if ((modeInfo.WinAAttributes & 0x7) != 0x7) {
#ifdef  USE_VBE20
			if (_SV_setBank20) {
				if (VBE_MMIOSel)
                    _SV_setBankPtr = _VBE20_setBankAB_ES;
                else
                    _SV_setBankPtr = _VBE20_setBankAB;
                }
            else
#endif
                _SV_setBankPtr = _VBE_setBankAB;
            }
        else {
#ifdef  USE_VBE20
			if (_SV_setBank20) {
                if (VBE_MMIOSel)
                    _SV_setBankPtr = _VBE20_setBankA_ES;
                else
                    _SV_setBankPtr = _VBE20_setBankA;
                }
            else
#endif
                _SV_setBankPtr = _VBE_setBankA;
            }

#ifdef  REALMODE
        _SV_setBankRM = (void *)modeInfo.WinFuncPtr;
#else
        _SV_setBankRM = NULL;
#endif
        }

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
#ifdef  USE_VBE20
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
#ifdef  USE_VBE20
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
#ifdef  USE_VBE20
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
#ifdef  USE_VBE20
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
        if (setPalette)
            SV_setPalette(0,256,VGA8_defPal,-1);
        else {
            SV_palette tmp[256];
            memset(tmp,0,sizeof(tmp));
            SV_setPalette(0,256,tmp,-1);
            }
        }
	SV_setActivePage(0);
	SV_setVisualPage(0,svDontWait);
	if ((mode & svDontClear) == 0)
        SV_clear(0);
    return true;
}

#ifdef  USE_VBEAF
static ibool SetModeAF(ushort mode,int virtualX,int virtualY,
	ibool useVirtualBuffer,int numBuffers,ibool setPalette,SV_CRTCInfo *crtc)
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
	AF_modeInfo modeInfo;
	AF_int32    bpl = -1;
	int         cntMode,imode = mode & svModeMask;
	RMREGS      regs;

	/* Grab information about the current video mode */
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

    /* Set the video mode - note that we dont clear the video memory as
     * we need to save and restore the video memory if we are running
     * under Windows to fix problems with buggy Windows drivers that cache
     * bitmaps in offscreen memory, but dont bother to save/restore them
	 * across a DOS box.
     */
    if (mode & afVirtualScroll) {
		if (AF_setVideoMode(DC.AFDC,mode,virtualX,virtualY,&bpl,
				numBuffers,(AF_CRTCInfo*)crtc) != 0)
			return false;
		DC.maxx = DC.AFDC->BufferEndX-1;
		DC.maxy = DC.AFDC->BufferEndY-1;
		}
	else {
		if (AF_setVideoMode(DC.AFDC,mode,-1,-1,&bpl,numBuffers,(AF_CRTCInfo*)crtc) != 0)
			return false;
		DC.maxx = modeInfo.XResolution-1;
		DC.maxy = modeInfo.YResolution-1;
		}

	/* Initialise global variables for current video mode dimensions    */
	DC.bytesperline = bpl;
	DC.bitsperpixel = modeInfo.BitsPerPixel;

	if (DC.AFDC->Version >= 0x200) {
        if (mode & vbeLinearBuffer)
            DC.maxpage = modeInfo.LinMaxBuffers-1;
        else
            DC.maxpage = modeInfo.BnkMaxBuffers-1;
        }
    else {
        DC.maxpage = modeInfo.MaxBuffers-1;
        }

    _SV_pagesize = 0;
    _SV_bankShift = 0;
    DC.curBank = -1;

    /* Save direct color info mask positions etc. We emulate 8 bit RGB modes
     * using a 3 3 2 palette arrangement by default
     */
    DC.redMask = 0x7;       DC.redPos = 5;      DC.redAdjust = 5;
    DC.greenMask = 0x7; DC.greenPos = 2;    DC.greenAdjust = 5;
    DC.blueMask = 0x3;      DC.bluePos = 0; DC.blueAdjust = 6;
    if (DC.bitsperpixel > 8) {
		if ((mode & vbeLinearBuffer) && (DC.AFDC->Version >= 0x200)) {
            DC.redMask = (0xFF >> (DC.redAdjust = 8 - modeInfo.LinRedMaskSize));
            DC.redPos = modeInfo.LinRedFieldPosition;
            DC.greenMask = (0xFF >> (DC.greenAdjust = 8 - modeInfo.LinGreenMaskSize));
			DC.greenPos = modeInfo.LinGreenFieldPosition;
            DC.blueMask = (0xFF >> (DC.blueAdjust = 8 - modeInfo.LinBlueMaskSize));
            DC.bluePos = modeInfo.LinBlueFieldPosition;
            }
        else {
            DC.redMask = (0xFF >> (DC.redAdjust = 8 - modeInfo.RedMaskSize));
            DC.redPos = modeInfo.RedFieldPosition;
            DC.greenMask = (0xFF >> (DC.greenAdjust = 8 - modeInfo.GreenMaskSize));
            DC.greenPos = modeInfo.GreenFieldPosition;
            DC.blueMask = (0xFF >> (DC.blueAdjust = 8 - modeInfo.BlueMaskSize));
            DC.bluePos = modeInfo.BlueFieldPosition;
            }
        }

    /* Set up a pointer to the appopriate bank switching code to use */
	DC.virtualBuffer = false;
	if (DC.AFDC->Version >= 0x200)
		_SV_setBankPtr = _VBEAF20_setBank;
	else
		_SV_setBankPtr = _VBEAF10_setBank;
	DC.videoMem = DC.AFDC->BankedMem;
    DC.setActivePage = _SV_setActivePageAF;
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
	if (DC.AFDC->Version >= 0x200 && modeInfo.Attributes & afHaveAccel2D) {
		haveAccel2D = true;
		if (DC.AFDC->cFuncs.EnableDirectAccess) {
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
		if (DC.AFDC->cFuncs.DrawLine) {
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
	else
		haveAccel2D = false;

    PM_saveDS();
    if (DC.bitsperpixel == 8) {
        /* Program the default VGA palette. Note that VBE/AF always takes
         * palette values in 8 bits per pixel format.
         */
        _SV_convertPal = false;
        if (setPalette)
            SV_setPalette(0,256,VGA8_defPal,-1);
        else {
            SV_palette tmp[256];
            memset(tmp,0,sizeof(tmp));
            SV_setPalette(0,256,tmp,-1);
            }
        }
	SV_setActivePage(0);
	SV_setVisualPage(0,svDontWait);
    if ((mode & svDontClear) == 0)
        SV_clear(0);
    return true;
}

void SVAPI SV_setACCELDriver(AF_devCtx *driver)
{
	_SV_loadedAF = driver;
}
#endif

#if defined(SAFE_MODE) || defined(__WINDOWS__)

/* The following Safe Mode can be enabled to save/restore the entire
 * video memory to disk before the video mode is started. This can solve
 * some problems with Windows drivers for code running in a DOS box or
 * in a WinDirect Window. This does however cause a significant slowdown
 * in the code, which is why it is currently only enabled for Windows
 * apps when the safe mode option of WinDirect is on.
 */

static int     saveMode;
static char    saveMemName[_MAX_PATH] = "";

static void backslash(char *s)
{
    uint pos = strlen(s);
    if (s[pos-1] != '\\') {
        s[pos] = '\\';
        s[pos+1] = '\0';
        }
}

static void SaveVideoMemory(int mode)
/****************************************************************************
*
* Function:     SaveVideoMemory
*
* Description:  Starts a 640x480x256 color graphics mode and streams all
*               video memory to a tempory disk file. This is necessary so
*               we can restore the video memory back to the state it was
*               in originally to avoid problems with buggy Windows drivers
*               that cache bitmaps in offscreen video memory but do not
*               correctly restore the memory when a DOS box or WinDirect
*               application returns to GDI mode.
*
*               If we are starting a banked video mode, we start a banked
*               version of 640x480x256 and save the memory with that. For
*               linear modes we start the linear version instead.
*
****************************************************************************/
{
    ulong   size;
    ibool    fail = false;
    FILE    *f;

    if (saveMemName[0] != 0)
        return;

#ifndef __WINDOWS__
    /* Dont bother to save/restore video memory when running under real DOS,
     * only under Windows DOS boxes.
	 */
    {
    RMREGS  regs;
    regs.x.ax = 0x160A;
    PM_int86(0x2F,&regs,&regs);
    if (regs.x.ax != 0)
        return;
    }
#else
    if (!WD_isSafeMode())
        return;
#endif

    saveMode = 0x101 | (mode & svLinearBuffer) | svDontClear;
#ifdef  USE_VBEAF
    if (DC.AFDC) {
		if (!SetModeAF(saveMode,-1,-1,false,1,false,NULL))
            return;
        }
    else
#endif
		if (!SetModeVBE(saveMode,false,false,false,NULL))
            return;

    /* Open a temporary file to save the video memory to */
    if (getenv("TEMP"))
        strcpy(saveMemName,getenv("TEMP"));
    else if (getenv("TMP"))
        strcpy(saveMemName,getenv("TEMP"));
    else
        strcpy(saveMemName,"c:\\");
    backslash(saveMemName);
    strcat(saveMemName,tmpnam(NULL));
    if ((f = fopen(saveMemName,"wb")) == NULL)
        return;

    /* Save entire video memory in chunks of 32Kb at a time except under
     * 32 bit protecetd mode with a linear framebuffer in which case we
     * can save it directly as a single block.
     */
#ifdef  PM386
    if (saveMode & svLinearBuffer) {
        size = DC.memory * 1024L;
        if (fwrite(DC.videoMem,1,size,f) != size)
            fail = true;
		}
    else
#endif
        {
#ifndef __WIN386__
        int     i,maxBlocks = DC.memory / 64;

        size = 0x8000;
        for (i = 0; i < maxBlocks; i++) {
            SV_setBank(i >> 1);
            if ((i & 1) == 0) {
                if (fwrite((uchar*)DC.videoMem,1,size,f) != size) {
                    fail = true;
                    break;
                    }
                }
            else {
                if (fwrite((uchar*)DC.videoMem + size,1,size,f) != size) {
                    fail = true;
                    break;
                    }
                }
            }
#endif
        }
    fclose(f);
    if (fail) {
        unlink(saveMemName);
        saveMemName[0] = 0;
		}
}

static void RestoreVideoMemory(void)
/****************************************************************************
*
* Function:     RestoreVideoMemory
*
* Description:  Restores the video memory from the memory buffer or
*               disk file after the mode has been reset to correctly
*               restore all cached bitmaps for Windows drivers.
*
****************************************************************************/
{
    ulong   size;
    FILE    *f;

	if (saveMemName[0] == 0)
		return;

#ifdef  USE_VBEAF
	if (DC.AFDC) {
		if (!SetModeAF(saveMode,-1,-1,false,1,false,NULL))
			return;
        }
    else
#endif
		if (!SetModeVBE(saveMode,false,false,false,NULL))
            return;

    /* Open the temporary file to restore the video memory from */
    if ((f = fopen(saveMemName,"rb")) == NULL)
        return;

    /* Restore entire video memory in chunks of 32Kb at a time except under
     * 32 bit protecetd mode with a linear framebuffer in which case we
     * can restore it directly as a single block.
     */
#ifdef  PM386
    if (saveMode & svLinearBuffer) {
        size = DC.memory * 1024L;
        fread(DC.videoMem,1,size,f);
        }
    else
#endif
        {
#ifndef __WIN386__
        int     i,maxBlocks = DC.memory / 64;

        size = 0x8000;
        for (i = 0; i < maxBlocks; i++) {
            SV_setBank(i >> 1);
            if ((i & 1) == 0)
                fread((uchar*)DC.videoMem,1,size,f);
            else
                fread((uchar*)DC.videoMem + size,1,size,f);
            }
#endif
        }
    fclose(f);
    unlink(saveMemName);
	saveMemName[0] = 0;
}
#else
#define SaveVideoMemory(m)
#define RestoreVideoMemory()
#endif

ibool SVAPI SV_setModeExt(ushort mode,ibool use8BitDAC,
	ibool useVirtualBuffer,int numBuffers,SV_CRTCInfo *crtc)
/****************************************************************************
*
* Function:     SV_setModeExt
* Parameters:   mode                - SuperVGA video mode to set.
*               use8BitDAC          - Turn on 8 bit DAC mode (for VBE modes only)
*               useVirtualBuffer    - True to use virtual linear buffer
*               numBuffers          - Number of image buffers to use
*               crtc				- CRTC information for refresh control
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode. This routine
*               assumes that the library and SuperVGA have been initialised
*               with the SV_init() routine first.
*
*               If the use8BitDAC flag is set, the 8 bit DAC mode will be
*               use for VBE graphics modes. For VBE/AF the 8 bit DAC mode
*               is used automatically if it is available, so this flag is
*               ignored.
*
*				This extended mode set function allows the user to pass in
*				the svRefreshCtrl and svStereo flags for the video mode.
*               If the svRefreshCtrl bit is set for the video mode, the
*				values in the 'crtc' parameter are passed to the BIOS to
*				do refresh rate control. If the bit is not set, the crtc
*				parameter is ignored and may be set to NULL. Note that the
*				svRefreshCtrl flag requires a VBE/Core 3.0 or VBE/AF 2.0 or
*				higher implementation and will fail on older implementations!
*
*				If the svStereo flag is set, the mode is initialized for
*				stereo LC shutter glasses. If the hardware has hardware
*				stereo capabilities (requires VBE/Core 3.0 or VBE/AF 2.0 or
*				higher) this will be used, otherwise stereo page flipping
*				will be emulated in software using a timer interrupt driven
*				mechanism.
*
****************************************************************************/
{
	ushort		imode = mode;
	SV_modeInfo	mi;

	if (!SV_getModeInfo(mode & svModeMask,&mi))
		return false;
	if ((mode & svRefreshCtrl) && (!DC.haveRefreshCtrl || !crtc))
		return false;

	/* We remove the svStereo flag if we dont have hardware
	 * stereo support from the mode number we pass to the BIOS.
	 */
	if ((mode & svStereo) && !(mi.Attributes & svHaveStereo))
		imode &= ~svStereo;

	/* Set the video mode */
#ifdef	__WINDOWS__
	WD_startFullScreen(hwndMain,mi.XResolution,mi.YResolution);
#else
	WD_startFullScreen(NULL,mi.XResolution,mi.YResolution);
#endif
	SaveVideoMemory(imode);
#ifdef  USE_VBEAF
	if (DC.AFDC) {
		if (!SetModeAF(imode,-1,-1,useVirtualBuffer,numBuffers,true,crtc))
			return false;
		}
	else
#endif
		{
		if ((mode & svStereo) && (numBuffers*2 > mi.NumberOfPages)) {
			return false;
			}
		else if (numBuffers > mi.NumberOfPages)
			return false;
		if (!SetModeVBE(imode &= ~(svMultiBuffer | svStereo),use8BitDAC,useVirtualBuffer,true,crtc))
			return false;
		}
	WD_inFullScreen();

	/* Initialize software stereo page flipping if we dont have hardware
	 * support.
	 */
	DC.isStereo = svNoStereo;
	DC.stereoRunning = false;
	if ((mode & svStereo)) {
		if (!(mi.Attributes & svHaveStereo)) {
			/* Enable software stereo page flipping using a timer interrupt */
			DC.isStereo = svSoftStereo;
			DC.useBlueCode = true;

			// TODO: Enable support for software stereo in here
			}
		else {
			/* Enable support for hardware stereo mode */
			if (mi.Attributes & svHaveDualDisp)
				DC.isStereo = svHWStereoDual;
			else
				DC.isStereo = svHWStereo;
			}
		if (DC.useBlueCode && mi.BitsPerPixel == 8) {
			/* Enable support for blue code software stereo sync signal */
			VGA8_defPal[DC.blueCodeIndex].red = 0;
			VGA8_defPal[DC.blueCodeIndex].green = 0;
			VGA8_defPal[DC.blueCodeIndex].blue = 0xFF;
			SV_setPalette(0,256,VGA8_defPal,-1);
			}
		SV_startStereo();
		}

	/* Set the maximum page index depending on the buffers requeseted */
	DC.maxpage = numBuffers-1;
	return true;
}

ibool SVAPI SV_setMode(ushort mode,ibool use8BitDAC,
	ibool useVirtualBuffer,int numBuffers)
/****************************************************************************
*
* Function:     SV_setMode
* Parameters:   mode                - SuperVGA video mode to set.
*               use8BitDAC          - Turn on 8 bit DAC mode (for VBE modes only)
*               useVirtualBuffer    - True to use virtual linear buffer
*               numBuffers          - Number of image buffers to use
* Returns:      True if the mode was set, false if not.
*
* Description:  Compatibility wrapper for the old mode set function.
*
****************************************************************************/
{
	return SV_setModeExt(mode,use8BitDAC,useVirtualBuffer,numBuffers,NULL);
}

ibool SVAPI SV_setVirtualMode(ushort mode,int virtualX,int virtualY,
    ibool use8BitDAC,ibool useVirtualBuffer,int numBuffers)
/****************************************************************************
*
* Function:     SV_setVirtualMode
* Parameters:   mode    - SuperVGA video mode to set.
*               virtualX    - Virtual screen X coordinate
*               virtualY    - Virtual screen Y coordinate
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode with virtual
*               scrolling enabled for the specified dimensions. If virtual
*               scrolling cannot be obtained correctly for this mode,
*               we return false.
*
****************************************************************************/
{
    int newbytes;
	SV_modeInfo	mi;

	if (!SV_getModeInfo(mode & svModeMask,&mi))
		return false;
#ifdef	__WINDOWS__
	WD_startFullScreen(hwndMain,mi.XResolution,mi.YResolution);
#else
	WD_startFullScreen(NULL,mi.XResolution,mi.YResolution);
#endif
	SaveVideoMemory(mode);
#ifdef  USE_VBEAF
    if (DC.AFDC) {
        return SetModeAF(mode | afVirtualScroll,virtualX,virtualY,
				useVirtualBuffer,numBuffers,true,NULL);
		}
	else
#endif
		{
		if (!SetModeVBE(mode,use8BitDAC,useVirtualBuffer,true,NULL))
			return false;
		if (!VBE_setPixelsPerLine(virtualX,&newbytes,&virtualX,&virtualY)) {
			SV_restoreMode();
			return false;
			}
		}
    DC.bytesperline = newbytes;
    DC.maxx = virtualX-1;
    DC.maxy = virtualY-1;
    return true;
}

void SVAPI SV_restoreMode(void)
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
    RMREGS  regs;

    RestoreVideoMemory();           
#ifdef  USE_VBE20
    VF_exit();
    VBE_freePMCode();               /* Free up protected mode code  */
#endif
#ifdef  USE_VBEAF
    if (DC.AFDC)
        AF_restoreTextMode(DC.AFDC);
    else
#endif
        VBE_setVideoMode(0x3);      /* Reset to text mode           */
    if (_SV_old50Lines) {
        regs.x.ax = 0x1112;
        regs.x.bx = 0;
        PM_int86(0x10,&regs,&regs); /* Restore 50 line mode         */
		}
	WD_restoreGDI();
}

static ibool UseDoubleScan(SV_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     UseDoubleScan
*
* Description:  Determines if we should use a double scan mode for the
*				graphics mode. This is true if we have a low resolution
*				mode and the mode supports double scanning.
*
****************************************************************************/
{
	if (modeInfo->Attributes & svHaveDoubleScan) {
		if (modeInfo->XResolution < 640 && modeInfo->YResolution <= 384)
			return true;
		}
	return false;
}

/****************************************************************************
DESCRIPTION:
Computes the maximum refresh rate for the graphics mode

HEADER:
svga.h

PARAMETERS:
modeInfo	- SuperVGA mode information block for the mode
interlaced	- True if the mode should be interlaced or not

RETURNS:
Maximum refresh rate supported by the graphics adapter, in Hz.

REMARKS:
This function computes the maximum refresh rate for the graphics modes
specified by the passed in mode information block. The maximum refresh
rate is computed from the information returned by the VBE BIOS or VBE/AF
driver, using CRTC values computed using the VESA GTF formulas.

Note: This function only computes the maximum refresh rate that the
	  graphics adapter can support, and does not take into account the
	  maximum refresh rate that the attached monitor can handle. It is
	  up to the application programmer to use VBE/DDC functions to
	  deteremine the monitor's operational limits to restrict the maximum
	  refresh rate according to the monitor specifications.

SEE ALSO:
SV_computeCRTCTimings
****************************************************************************/
float SVAPI SV_getMaxRefreshRate(SV_modeInfo *modeInfo,ibool interlaced)
{
	GTF_timings	gtf;
	int			xRes = modeInfo->XResolution;
	int			yRes = modeInfo->YResolution;
	ibool		doubleScan;

	if (DC.haveRefreshCtrl) {
		doubleScan = UseDoubleScan(modeInfo);
		if (doubleScan)
			yRes *= 2;
		GTF_calcTimings(xRes,yRes,modeInfo->MaxPixelClock / 1000000.0,
			GTF_lockPF,false,interlaced,&gtf);
		return gtf.vFreq;
		}
	return 0;
}

static double round(double v)
{
	return floor(v + 0.5);
}

/****************************************************************************
DESCRIPTION:
Compute a set of CRTC values given a specified mode and refresh rate

HEADER:
svga.h

PARAMETERS:
modeInfo	- SuperVGA mode information block for the mode
refreshRate	- Desired refresh rate to generate timings for
interlaced	- True if the CRTC timings should be interlaced
crtc		- Place to store the computed CRTC timings
pureGTF		- True to use pure GTF formulas

RETURNS:
True on success, or false if refresh rate is out of range.

REMARKS:
This function computes a set of CRTC timings for the specific graphics
mode at the specified refresh rate. The CRTC timings are computed using
the VESA GTF timing formulas, combined with information returned by the
VBE/Core 3.0 BIOS or VBE/AF driver. Note that the exact refresh rate that
you get will not be exactly what you requested, and the exact value is
returned in the crtc information block block.

If the pureGTF parameter is set to true, we will leave the CRTC timings
as is after the GTF formulas have computed the appropriate timings for
the refresh rate given the closest available pixel clock. This means that
we may not be able to hit a specific refresh rate exactly, but the CRTC
timings will be GTF compliant. In some cases it may be more beneficial to
hit an exact refresh rate on the nail (such as when viewing sterero images)
and in order to do this we will modify the horizontal and vertical totals
slightly to compensate for the granularity in the pixel clock (which in
some cases may be quite large).

SEE ALSO:
SV_getMaxRefreshRate
****************************************************************************/
ibool SVAPI SV_computeCRTCTimings(SV_modeInfo *modeInfo,float refreshRate,
	ibool interlaced,SV_CRTCInfo *crtc,ibool pureGTF)
{
	GTF_timings	gtf,bestGTF;
	int			xRes = modeInfo->XResolution;
	int			yRes = modeInfo->YResolution;
	float		closestRefresh,refresh,minRefresh,maxRefresh;
	ulong		clock,closestClock;
	ibool		doubleScan;

	if (DC.haveRefreshCtrl) {
		/* If we have a double scanned mode, then double the vertical
		 * timing parameters.
		 */
		doubleScan = UseDoubleScan(modeInfo);
		if (modeInfo->XResolution == 512 && modeInfo->YResolution == 384 && refreshRate > 75)
			doubleScan = false;
		if (doubleScan)
			yRes *= 2;

		/* Now search for the closest refresh rate we can get that is
		 * within -1Hz or +5Hz of the desired refresh rate.
		 */
		minRefresh = refreshRate-1;
		maxRefresh = refreshRate+5;
		closestRefresh = 0;
		closestClock = -1;
		for (refresh = minRefresh; refresh < maxRefresh; refresh += 0.1) {
			/* First compute the GTF timings given the specified refresh rate */
			GTF_calcTimings(xRes,yRes,refresh,GTF_lockVF,false,interlaced,&gtf);

			/* Now find the closest pixel clock for the computed timings */
#ifdef	USE_VBEAF
			if (DC.AFDC) {
				clock = AF_getClosestPixelClock(DC.AFDC,modeInfo->Mode,
					(ulong)(gtf.dotClock * 1000000.0));
				}
			else
#endif
				{
				clock = VBE_getClosestClock(modeInfo->Mode,
					(ulong)(gtf.dotClock * 1000000.0));
				}
			if (clock == (ulong)-1)
				continue;

			/* Now run the GTF timings once more given the closest pixel clock */
			GTF_calcTimings(xRes,yRes,clock / 1000000.0,GTF_lockPF,false,interlaced,&gtf);
			if (fabs(gtf.vFreq-refreshRate) < fabs(closestRefresh-refreshRate)) {
				closestRefresh = gtf.vFreq;
				closestClock = clock;
				bestGTF = gtf;
				}
			}
		if (closestClock == (ulong)-1)
			return false;

		/* Now modify the horizontal and vertical total's to achieve the
		 * exact refresh rate, since if we have a table driven clock we
		 * really want to hit the refresh rate on the nail (not necessarily
		 * the GTF compatible timings) for legacy graphics adapters. All
		 * modern graphics adapters have programmable PLL's with fine
		 * pixel clock granularities and wont need any adjustment.
		 */

		// TODO: This needs to be modified as it produces incorrect CRTC
		//		 timings for discrete clock cards that have clocks that are
		//		 not very close to what we want.
//		if (!pureGTF) {
//			bestGTF.v.vTotal = round((float)closestClock / (bestGTF.h.hTotal * refreshRate));
//			bestGTF.h.hTotal = round((float)closestClock / (bestGTF.v.vTotal * refreshRate));
//			bestGTF.h.hTotal = round(bestGTF.h.hTotal / 8) * 8;
//			}

		/* Now return the computed timings to the application */
		crtc->HorizontalTotal = bestGTF.h.hTotal;
		crtc->HorizontalSyncStart = bestGTF.h.hSyncStart;
		crtc->HorizontalSyncEnd = bestGTF.h.hSyncEnd;
		crtc->VerticalTotal = bestGTF.v.vTotal;
		crtc->VerticalSyncStart = bestGTF.v.vSyncStart;
		crtc->VerticalSyncEnd = bestGTF.v.vSyncEnd;
		crtc->PixelClock = closestClock;
		crtc->RefreshRate = (ushort)(bestGTF.vFreq * 100.0);
        crtc->HorizontalScanRate = bestGTF.hFreq;
		crtc->Flags = 0;
		if (interlaced)
			crtc->Flags |= svInterlaced;
		if (doubleScan)
			crtc->Flags |= svDoubleScan;
		if (xRes <= 640 && yRes <= 400) {
			/* Get the sync polarities for the mode. For low resolution modes
			 * below 640x480, we need to use specific sync polarities since
			 * they are used to determine the vertical sizing for compatibility
			 * with VGA CRTC timings.
			 */
			if (yRes == 350)
				crtc->Flags |= svVSyncNeg;
			else if (yRes == 200 || yRes == 384 || yRes == 400)
				crtc->Flags |= svHSyncNeg;
			else
				crtc->Flags |= svHSyncNeg | svVSyncNeg;
			}
		else {
			/* Use polarities returned from GTF formulas */
			if (bestGTF.hSyncPol == '-')
				crtc->Flags |= svHSyncNeg;
			if (bestGTF.vSyncPol == '-')
				crtc->Flags |= svVSyncNeg;
			}

		return true;
		}
	return false;
}

/****************************************************************************
DESCRIPTION:
Compute a set of CRTC values given a specified mode and horizontal frequency

HEADER:
svga.h

PARAMETERS:
modeInfo	- SuperVGA mode information block for the mode
refreshRate	- Desired refresh rate to generate timings for
crtc		- Place to store the computed CRTC timings

RETURNS:
True on success, or false if refresh rate is out of range.

REMARKS:
This function computes a set of CRTC timings for the specific graphics
mode at the specified horizontal frequency. The CRTC timings are computed
using the VESA GTF timing formulas as a starting point, combined with
information returned by the VBE/Core 3.0 BIOS or VBE/AF driver. Note that
because we are trying to hit an exact horizontal frequency with this
function, we modify the horizontal and vertical timings to produce the
exact horizontal frequency and only really use the GTF formulas to find a
good starting point for the CRTC timings.

SEE ALSO:
SV_getMaxRefreshRate, SV_computeCRTCTimings
****************************************************************************/
ibool SVAPI SV_computeCRTCTimingsHFreq(SV_modeInfo *modeInfo,float hFreq,
	ibool hSyncNeg,ibool vSyncNeg,SV_CRTCInfo *crtc)
{
	GTF_timings	gtf;
	int			xRes = modeInfo->XResolution;
	int			yRes = modeInfo->YResolution;
	ulong		clock;
	ibool		doubleScan;

	if (DC.haveRefreshCtrl) {
		/* If we have a double scanned mode, then double the vertical
		 * timing parameters.
		 */
		doubleScan = UseDoubleScan(modeInfo);
		if (doubleScan)
			yRes *= 2;

		/* First compute the GTF timings given the specified h frequency */
		GTF_calcTimings(xRes,yRes,hFreq,GTF_lockHF,false,false,&gtf);

		/* Now find the closest pixel clock for the computed timings */
#ifdef	USE_VBEAF
		if (DC.AFDC) {
			clock = AF_getClosestPixelClock(DC.AFDC,modeInfo->Mode,
				(ulong)(gtf.dotClock * 1000000.0));
			}
		else
#endif
			{
			clock = VBE_getClosestClock(modeInfo->Mode,
				(ulong)(gtf.dotClock * 1000000.0));
			}
		if (clock == (ulong)-1)
			return false;

		/* Now force the horizontal total to achieve the desired horizontal
		 * frequency, and then adjust the vertical total to get the
		 * appropriate GTF refresh rate.
		 */
		gtf.h.hTotal = round((float)clock / (hFreq * 1000));
		gtf.h.hTotal = round(gtf.h.hTotal / 8) * 8;
		gtf.v.vTotal = round((float)clock / (gtf.h.hTotal * gtf.vFreq));

		// TODO: Perhaps we need to modify this code to allow the user to
		//		 adjust the refresh rate but keep the horizontal frequency
		//		 the same by adjusting the pixel clock or the vertical total?

		/* Now return the computed timings to the application */
		crtc->HorizontalTotal = gtf.h.hTotal;
		crtc->HorizontalSyncStart = gtf.h.hSyncStart;
		crtc->HorizontalSyncEnd = gtf.h.hSyncEnd;
		crtc->VerticalTotal = gtf.v.vTotal;
		crtc->VerticalSyncStart = gtf.v.vSyncStart;
		crtc->VerticalSyncEnd = gtf.v.vSyncEnd;
		crtc->PixelClock = clock;
		crtc->RefreshRate = (ushort)(gtf.vFreq * 100.0);
		crtc->HorizontalScanRate = gtf.hFreq;
		crtc->Flags = 0;
		if (doubleScan)
			crtc->Flags |= svDoubleScan;
		if (xRes <= 640 && yRes <= 400) {
			/* Get the sync polarities for the mode. For low resolution modes
			 * below 640x480, we need to use specific sync polarities since
			 * they are used to determine the vertical sizing for compatibility
			 * with VGA CRTC timings.
			 */
			if (yRes == 350)
				crtc->Flags |= svVSyncNeg;
			else if (yRes == 200 || yRes == 384 || yRes == 400)
				crtc->Flags |= svHSyncNeg;
			else
				crtc->Flags |= svHSyncNeg | svVSyncNeg;
			}
		else {
			/* Use polarities for fixed frequency monitors */
			if (hSyncNeg)
				crtc->Flags |= svHSyncNeg;
			if (vSyncNeg)
				crtc->Flags |= svVSyncNeg;
			}

		return true;
		}
	return false;
}

void SVAPI SV_startStereo(void)
/****************************************************************************
*
* Function:     SV_startStereo
*
* Description:  Start stereo page flipping mode. After a stereo mode has
*				been started the system automatically starts in stereo mode,
*				and stereo mode can be enabled and disabled at any time
*				with this function and the SV_stopStereo function.
*
****************************************************************************/
{
	if (DC.isStereo != svNoStereo && !DC.stereoRunning) {
		if (DC.isStereo >= svHWStereo) {
			/* Enable hardware stereo support */
#ifdef	USE_VBEAF
			if (DC.AFDC)
				AF_enableStereoMode(DC.AFDC,true);
			else
#endif
				VBE_enableStereoMode();
			}
		else if (DC.isStereo == svSoftStereo) {
			// TODO: Implement this for software stereo
			}
		DC.stereoRunning = true;
		}
}

void SVAPI SV_stopStereo(void)
/****************************************************************************
*
* Function:     SV_stopStereo
*
* Description:  Stop stereo page flipping mode. After a stereo mode has
*				been started the system automatically starts in stereo mode,
*				and stereo mode can be enabled and disabled at any time
*				with this function and the SV_startStereo function.
*
****************************************************************************/
{
	if (DC.isStereo != svNoStereo && DC.stereoRunning) {
		if (DC.isStereo >= svHWStereo) {
			/* Disable hardware stereo mode */
#ifdef	USE_VBEAF
			if (DC.AFDC)
				AF_enableStereoMode(DC.AFDC,false);
			else
#endif
				VBE_disableStereoMode();
			}
		else if (DC.isStereo == svSoftStereo) {
			// TODO: Implement this for software stereo
			}
		DC.stereoRunning = false;
		}
}

void SVAPI SV_useBlueCode(ibool flag)
/****************************************************************************
*
* Function:     SV_useBlueCode
* Parameters:   flag	- True to use blue code system, false if not
*
* Description:  This function lets the SuperVGA Kit know whether it should
*				be using the blue code system for synchronising with LC
*				shutter glasses. By default when using software stereo or
*				when using hardware stereo without hardware stereo sync
*				signalling, we use the blue code system to synchronise the
*				left and right images with the LCD shutter glasses.
*
*				If the BIOS reports that it has the capability to work with
*				hardware stereo sync signalling, the blue code capability is
*				is turned off by default. However you may want to provide
*				the end user with the option of using the blue code system
*				as they may have older LC glasses that dont have the correct
*				connectors to work with the graphics cards hardware
*				signalling connector.
*
****************************************************************************/
{
	DC.useBlueCode = flag;
}

void SVAPI SV_setBlueCodeIndex(int index)
/****************************************************************************
*
* Function:     SV_setBlueCodeIndex
* Parameters:   flag	- True to use blue code system, false if not
*
* Description:  This function lets the SuperVGA Kit know which color index
*				should be used for drawing the blue code sync lines for
*               LC stereo glasses when in color index modes (ie: < 8bpp).
*				The application is responsible for ensuring that the value
*				selected has the blue value set to full intensity (the values
*				in the other color channels are not important, so pure white
*				will work just as well as pure blue). By default the
*				SuperVGA Kit sets the blue code index to 255 and programs
*				the palette entry for this value to pure blue.
*
*				When in HiColor and TrueColor modes this value is ignored,
*				as we set the RGB value to pure blue directly when drawing
*				the blue codes on the screen.
*
****************************************************************************/
{
	DC.blueCodeIndex = index;
}

void SVAPI SV_setActivePage(int page)
/****************************************************************************
*
* Function:     SV_setActivePage
* Parameters:   page	- New video page to make active
*
* Description:  Sets the active video page. If we are running in stereo
*				mode you can also pass the svLeftBuffer and svRightBuffer
*				commands to draw the stereo images to video memory.
*
****************************************************************************/
{
	DC.activePage = page;
	if (DC.AFDC == NULL && DC.isStereo != svNoStereo) {
		if (page & svRightBuffer)
			page = ((page & ~svRightBuffer) * 2) + 1;
		else
			page *= 2;
		}
	DC.setActivePage(page);
}

void SVAPI SV_setVisualPage(int page,int waitVRT)
/****************************************************************************
*
* Function:     SV_setVisualPage
* Parameters:   page	- New visual page to display
*
* Description:  Sets the visual display page. If we are running in stereo
*				mode then and we are using the blue code signalling system,
*				we draw the blue codes on the pages that are about to become
*				active before we display them.
*
****************************************************************************/
{
	RMREGS  regs;
	int 	oldActivePage = DC.activePage;
	ulong	addr,blueCodeColor;

	DC.visualPage = page;
	if (DC.stereoRunning && DC.useBlueCode) {
		/* Get the color for the blue code */
		if (DC.maxcolor > 255)
			blueCodeColor = SV_rgbColor(0,0,255);
		else
			blueCodeColor = DC.blueCodeIndex;

		/* Draw the left image blue code (25% of width) */
		SV_setActivePage(page);
		SV_line(0,DC.maxy,DC.maxx / 4,DC.maxy,blueCodeColor);
		SV_line(DC.maxx / 4,DC.maxy,DC.maxx,DC.maxy,0);

		/* Draw the right image blue code (75% of width) */
		SV_setActivePage(page | svRightBuffer);
		SV_line(0,DC.maxy,(DC.maxx * 3) / 4,DC.maxy,blueCodeColor);
		SV_line((DC.maxx * 3) / 4,DC.maxy,DC.maxx,DC.maxy,0);
		SV_setActivePage(oldActivePage);
		}
	if (DC.maxpage <= 1 && waitVRT == svTripleBuffer)
		waitVRT = svWaitVRT;
#ifdef	USE_VBEAF
	if (DC.AFDC) {
		/* Ensure that previous display start has taken hold if we have hardware
		 * triple buffering. We do this by polling the GetDisplayStartStatus
		 * function until it tells us that the last frame has taken hold. In most
		 * cases unless the framerate is running close to the refresh rate of
		 * the video mode, this function loop will terminate immediately and we
		 * never wait. If we do wait, it is because we are running too fast!
		 */
		if (DC.haveTripleBuffer && (waitVRT == svTripleBuffer)) {
			while (AF_getDisplayStartStatus(DC.AFDC) == 0)
				;
			}

		/* Call the VBE/AF driver to do the flip */
		AF_setVisibleBuffer(DC.AFDC,page,(waitVRT == svWaitVRT));
		}
	else
#endif
		{
		/* Adjust for stereo page flipping */
		if (DC.isStereo != svNoStereo)
			page *= 2;

		/* Ensure that previous display start has taken hold if we have hardware
		 * triple buffering. We do this by polling the GetDisplayStartStatus
		 * function until it tells us that the last frame has taken hold. In most
		 * cases unless the framerate is running close to the refresh rate of
		 * the video mode, this function loop will terminate immediately and we
		 * never wait. If we do wait, it is because we are running too fast!
		 */
		if (DC.haveTripleBuffer && (waitVRT == svTripleBuffer)) {
			do {
				regs.x.ax = 0x4F07;
				regs.x.bx = 0x0004;
				PM_int86(0x10,&regs,&regs);
				} while (regs.x.cx == 0);
			}
		addr = page * _SV_pagesize;
#ifndef	REALMODE
		if (_SV_setCRT20 && DC.isStereo < svHWStereoDual) {
			/* VBE/Core 2.0 and higher page flip via the 32-bit protected
			 * mode display start function.
			 */
			int waitFlag;
			if (DC.bitsperpixel > 4)
				addr >>= 2;			/* Adjust to plane boundary for 8bpp+ */
			waitFlag = 0;
			if (waitVRT == svWaitVRT)
				waitFlag = 0x80;
			_SV_callSetCRT20(addr,waitFlag);
			}
		else
#endif
		if (DC.VBEVersion >= 0x300) {
			/* VBE/Core 3.0 page flip via the real mode interface, for
			 * both the alternate display start and the stereo dual
			 * display start functions.
			 */
			regs.x.ax = 0x4F07;
			if (waitVRT == svWaitVRT)
				regs.x.bx = 0x82;
			else
				regs.x.bx = 0x02;
			regs.e.ecx = addr;
			if (DC.isStereo == svHWStereoDual) {
				regs.x.bx++;
				regs.e.edx = addr+_SV_pagesize;
				}
			PM_int86(0x10,&regs,&regs);
			}
		else {
			/* VBE/Core 2.0 and lower page flip using the real mode
			 * interface. We have to compute the (x,y) pixel address
			 * of the display start and pass this to the VBE driver.
			 */
			int startY = addr / DC.bytesperline;
			int startX = addr % DC.bytesperline;
			switch (DC.bitsperpixel) {
				case 4:		startX <<= 3;	break;
				case 15:
				case 16:	startX >>= 1;	break;
				case 24:	startX /= 3;	break;
				case 32:	startX >>= 2;	break;
				}
			regs.x.ax = 0x4F07;
			if (DC.VBEVersion >= 0x200 && waitVRT != svDontWait)
				regs.x.bx = 0x80;
			else
				regs.x.bx = 0x00;
			regs.x.cx = startX;
			regs.x.dx = startY;
			if (DC.VBEVersion < 0x200 && (waitVRT != svDontWait))
				_SV_waitDEVGA();
			PM_int86(0x10,&regs,&regs);
			if (DC.VBEVersion < 0x200 && (waitVRT != svDontWait))
				_SV_waitVRTVGA();
			}
		}
}

void SVAPI SV_setPalette(int start,int num,SV_palette *pal,int maxProg)
/****************************************************************************
*
* Function:     SV_setPalette
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
*               Note that this routine *requires* all input palette values
*               to be in 8 bits per primary format, and does automatic
*               conversion from 8 bits per primary formay to the 6 bit
*               format required by some cards if necessary.
*
****************************************************************************/
{
    int     i,waitFlag,count;
    uchar   temp[1024],*t,*p = (uchar*)&pal[start];

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
#ifdef  USE_VBEAF
        if (DC.AFDC)
            AF_setPaletteData(DC.AFDC,(AF_palette*)p,count,start,waitFlag == 0x80);
        else
#endif
		if (!DC.isNonVGA)
            _VGA_setPalette(start,count,(SV_palette*)p,waitFlag);
#ifdef  USE_VBE20
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

 ulong SVAPI SV_rgbColor(uchar r,uchar g,uchar b)
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

void SVAPI SV_writeText(int x,int y,char *str,ulong color)
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
    uchar           *font;

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

void SVAPI SV_setDisplayStart(int x,int y,ibool waitVRT)
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

#ifdef  USE_VBEAF

void _ASMAPI _SV_setActivePageAF(int page)
{
    ulong   addr;

    /* Set active buffer and update internal pointers */
    AF_setActiveBuffer(DC.AFDC,page);
    addr = DC.AFDC->OriginOffset;
    DC.originOffset = (ushort)addr;
    SV_setBank(DC.bankOffset = (addr >> 16));

    /* Set hardware clip rectangle to new active page */
	if (haveAccel2D && DC.AFDC->Version >= 0x200 && DC.AFDC->cFuncs.SetClipRect)
        AF_setClipRect(DC.AFDC,0,0,DC.maxx,DC.maxy);
}

void _ASMAPI _SV_setActivePageLinAF(int page)
{
    /* Set active buffer and update internal pointers */
    AF_setActiveBuffer(DC.AFDC,page);
    DC.originOffset = (ulong)DC.videoMem + DC.AFDC->OriginOffset;

    /* Set hardware clip rectangle to new active page */
	if (haveAccel2D && DC.AFDC->Version >= 0x200 && DC.AFDC->cFuncs.SetClipRect)
		AF_setClipRect(DC.AFDC,0,0,DC.maxx,DC.maxy);
}

void _ASMAPI _SV_beginDirectAccessAFSLOW(void)
{ AF_enableDirectAccess(DC.AFDC); }

void _ASMAPI _SV_endDirectAccessAFSLOW(void)
{ AF_disableDirectAccess(DC.AFDC); }

void _ASMAPI _SV_beginPixelAFSLOW(void)
{ AF_enableDirectAccess(DC.AFDC); }

void _ASMAPI _SV_endPixelAFSLOW(void)
{ AF_disableDirectAccess(DC.AFDC); }

void _ASMAPI _SV_beginLineAFSLOW(void)
{ AF_enableDirectAccess(DC.AFDC); }

void _ASMAPI _SV_endLineAFSLOW(void)
{ AF_disableDirectAccess(DC.AFDC); }

void _ASMAPI _SV_beginDirectAccessAF(void)
{ AF_waitTillIdle(DC.AFDC); }

void _ASMAPI _SV_endDirectAccessAF(void)
{}

void _ASMAPI _SV_beginPixelAF(void)
{ AF_waitTillIdle(DC.AFDC); }

void _ASMAPI _SV_endPixelAF(void)
{}

void _ASMAPI _SV_beginLineAF(void)
{ AF_waitTillIdle(DC.AFDC); }

void _ASMAPI _SV_endLineAF(void)
{}

void _ASMAPI _SV_clearAF(ulong color)
{
    AF_drawRect(DC.AFDC,color,0,0,DC.maxx+1,DC.maxy+1);
}

void _ASMAPI _SV_putPixelAF(int x,int y,ulong color)
{
    DC.beginPixel();
    DC.putPixelFast(x,y,color);
    DC.endPixel();
}

void _ASMAPI _SV_lineFastAF(int x1,int y1,int x2,int y2,ulong color)
{ AF_drawLine(DC.AFDC,color,x1 << 16,y1 << 16,x2 << 16,y2 << 16); }

void _ASMAPI _SV_softLineAF(int x1,int y1,int x2,int y2,ulong color)
{
	DC.beginLine();
	DC.lineFast(x1,y1,x2,y2,color);
	DC.endLine();
}

#endif

#ifdef  __WINDOWS16__

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

void SVAPI SV_beginDirectAccess(void)
{ DC.beginDirectAccess(); }

void SVAPI SV_endDirectAccess(void)
{ DC.endDirectAccess(); }

#endif

#ifdef	__WINDOWS__
void SVAPI SV_setMainWindow(HWND hwnd)
{ hwndMain = hwnd; }
#endif
