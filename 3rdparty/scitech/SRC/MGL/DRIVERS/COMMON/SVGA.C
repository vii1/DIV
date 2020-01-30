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
* Description:	Set of device driver routines common to all VESA VBE
*				unaccelerated display drivers.
*
*               This module depends upon the generic VESA VBE interface
*               routines provided by the SuperVGA Kit VESAVBE.C module.
*               For the MGL proper, the VESAVBE.C module should be included
*               in the main library file.
*
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\common\svga.h"
#include "gtf.h"
#include <math.h>

/*--------------------------- Global Variables ----------------------------*/

#define	MAX_MODE	100

typedef struct {
	int		mode,xres,yres;
	uchar	bits;
	} mdEntry;

PRIVATE	short	detected = -1;		/* VBE already detected?			*/
PRIVATE	MGLDC	*DispDC;			/* Fullscreen Display DC			*/
PRIVATE	mdEntry	modeList[MAX_MODE];	/* List of available VBE modes		*/
PRIVATE char	OemString[40];		/* OEM String from VBE info block	*/
#if	defined(SAFE_MODE) || defined(__WINDOWS__)
PRIVATE int		saveMode;
PRIVATE	char	saveMemName[_MAX_PATH] = "";
#endif

int		_VARAPI SVGA_curBank;		/* Current bank number				*/
void	* _VARAPI SVGA_setBankVec;	/* Pointer to setBank routine		*/
int		_VARAPI VBE_version;		/* Version number of VBE interface	*/
ulong	_VARAPI VBE_capabilities;	/* Capabilities for VBE interface	*/
int		_VARAPI VBE_memory;			/* Amount of memory on board		*/
int 	_VARAPI VBE_bankShift;		/* Bank number adjust factor		*/
void	* _VARAPI VBE_bankSwitch;	/* Pointer to bank switch routine	*/
ibool	_VARAPI isNonVGA;			/* True if mode is NonVGA or not	*/
short	useVFlat = false;			/* Is VFlat available?				*/

#ifndef __16BIT__
extern short _VARAPI VBE_MMIOSel;	/* Selector to MMIO registers		*/
void _ASMAPI VBE20_setBankA(void);	/* VBE 2.0 setBank single bank		*/
void _ASMAPI VBE20_setBankAB(void);	/* VBE 2.0 setBank dual banks		*/
void _ASMAPI VBE20_setBankA_ES(void);/* VBE 2.0 setBank for MMIO		*/
void _ASMAPI VBE20_setBankAB_ES(void);/* VBE 2.0 setBank for MMIO		*/
void    	* _VARAPI setBank20 = NULL;/* Pointer to set bank routine	*/
void		* _VARAPI setCRT20 = NULL;/* Pointer to set CRT routine		*/
void		* _VARAPI setPal20 = NULL;/* Pointer to set palette routine	*/
#endif

void _ASMAPI VBE_setBankA(void);	/* VBE setBank routine single bank	*/
void _ASMAPI VBE_setBankAB(void);	/* VBE setBank routine dual banks	*/
void _ASMAPI VBE_setCRTStart(long crtStart,int bits,int bytesPerLine,
	int waitVRT);
void _ASMAPI VBE20_blastPalette(uchar *pal,int num,int index,int maxProg,
	int waitVRT);

#define	UNIVBE_STR	"Universal VESA VBE"

/*------------------------- Implementation --------------------------------*/

ibool MGLAPI SVGA_detect(int *driver)
/****************************************************************************
*
* Function:		SVGA_detect
* Returns:		True if the video card is a SVGA VBE 1.2+ SuperVGA
*
* Description:	Determines if a SVGA VBE compatible BIOS is out there, and
*				ensures that the BIOS is at least compliant with the VBE
*				1.2 standard.
*
****************************************************************************/
{
	VBE_vgaInfo		vgaInfo;
	VBE_modeInfo	modeInfo;
	int				i;
	ushort			*p;
	ushort			modes[MAX_MODE];
	mdEntry			*md;

	if (detected != -1) {
		if (*driver < grVESA)
			*driver = grVESA;
		return detected;	/* Return previous detection result		*/
		}

	detected = false;
	MGLPC_init();
#ifdef	MGLWIN
	if (!_MGL_haveWinDirect)
		return detected = false;
#endif
	if ((VBE_version = VBE_detect(&vgaInfo)) == 0)
		return false;

	/* Save the amount of memory on board and the OEM String */
	VBE_memory = vgaInfo.TotalMemory * 64;
    VBE_capabilities = vgaInfo.Capabilities;
	strncpy(OemString,vgaInfo.OemStringPtr,sizeof(OemString));
	OemString[sizeof(OemString)-1] = 0;

	/* Copy list of available video modes */
	for (i = 0, p = (ushort*)vgaInfo.VideoModePtr; *p != 0xFFFF; p++) {
		if (*p >= 0x100)
			modes[i++] = *p;
		}
	modes[i] = 0xFFFF;

	/* This is the first time this routine has been called, so
	 * grab the mode information for every available video mode and
	 * cache the important info for later.
	 */
	for (md = modeList, p = modes; *p != 0xFFFF; p++) {
		if (!VBE_getModeInfo(*p,&modeInfo))
			continue;
		if (!(modeInfo.ModeAttributes & vbeMdGraphMode))
			continue;
		md->mode = *p;
		md->xres = modeInfo.XResolution-1;
		md->yres = modeInfo.YResolution-1;
		md->bits = modeInfo.BitsPerPixel;
		md++;
		}
	md->mode = -1;

	/* If we have VBE 2.0 we can use the virtual linear buffer services. If
	 * we dont have VBE 2.0, then we cannot. Note that we also check if the
	 * VBE_getBankFunc32() succeeds, because we may not have the VBE 2.0
     * extensions that we need.
	 */
	useVFlat = false;
#ifdef	PM386
	if (VF_available()) {
#ifdef	__WINDOWS__
		if (strncmp(OemString,UNIVBE_STR,sizeof(UNIVBE_STR)-1) == 0)
#endif
			{
			int codeLen;
			void *bankFunc;
			useVFlat = VBE_getBankFunc32(&codeLen,&bankFunc,0,0);
			}
		}
#endif

	if (*driver < grVESA)
		*driver = grVESA;
	return detected = true;
}

PRIVATE ibool findMode(gmode_t *mi,VBE_modeInfo *modeInfo,ibool linear,
	ulong *flags)
/****************************************************************************
*
* Function:		findMode
* Parameters:	mi		- Mode information block to fill in
* Returns:		True if valid mode was found
*
* Description:	Calls the SVGA VBE BIOS to determine the information for
*				the specified video mode. We search for a video mode that
*				matches our specifications of pixels resolution and depth,
*				and fill in the remainder of the modeInfo block with the
*				values returned by the VBE.
*
****************************************************************************/
{
	mdEntry			*md;

	for (md = modeList; md->mode != -1; md++) {
		if (md->xres == mi->xRes && md->yres == mi->yRes &&
				md->bits == mi->bitsPerPixel)
			break;
		}
	if (md->mode == -1)
		return false;

	/* Use the bytesPerLine value returned by the BIOS */
	VBE_getModeInfo(md->mode,modeInfo);
	if (linear && !(modeInfo->ModeAttributes & vbeMdLinear)) {
		if (!useVFlat)
			return false;
		linear = false;
		}
	if (!linear && (modeInfo->ModeAttributes & vbeMdNonBanked))
		return false;
	*flags = 0;
	if (linear)
    	*flags |= MGL_HAVE_LINEAR;
	mi->scratch2 = md->mode;
	if (VBE_version >= 0x300 && linear)
		modeInfo->BytesPerScanLine = modeInfo->LinBytesPerScanLine;
	mi->bytesPerLine = modeInfo->BytesPerScanLine;

	/* Copy DirectColor mask information to mi block */
	if (modeInfo->BitsPerPixel <= 8) {
		mi->redMaskSize 		= 8;
		mi->redFieldPosition 	= 16;
		mi->greenMaskSize 		= 8;
		mi->greenFieldPosition 	= 8;
		mi->blueMaskSize 		= 8;
		mi->blueFieldPosition 	= 0;
		mi->rsvdMaskSize 		= 0;
		mi->rsvdFieldPosition	= 0;
		}
	else {
		if (VBE_version >= 0x300 && linear) {
			mi->redMaskSize = modeInfo->LinRedMaskSize;
			mi->redFieldPosition = modeInfo->LinRedFieldPosition;
			mi->greenMaskSize = modeInfo->LinGreenMaskSize;
			mi->greenFieldPosition = modeInfo->LinGreenFieldPosition;
			mi->blueMaskSize = modeInfo->LinBlueMaskSize;
			mi->blueFieldPosition = modeInfo->LinBlueFieldPosition;
			mi->rsvdMaskSize = modeInfo->LinRsvdMaskSize;
			mi->rsvdFieldPosition = modeInfo->LinRsvdFieldPosition;
			}
		else {
			mi->redMaskSize = modeInfo->RedMaskSize;
			mi->redFieldPosition = modeInfo->RedFieldPosition;
			mi->greenMaskSize = modeInfo->GreenMaskSize;
			mi->greenFieldPosition = modeInfo->GreenFieldPosition;
			mi->blueMaskSize = modeInfo->BlueMaskSize;
			mi->blueFieldPosition = modeInfo->BlueFieldPosition;
			mi->rsvdMaskSize = modeInfo->RsvdMaskSize;
			mi->rsvdFieldPosition = modeInfo->RsvdFieldPosition;
			}
		}
	if (VBE_version >= 0x300) {
		if (linear)
			mi->maxPage = modeInfo->LinNumberOfImagePages;
		else
			mi->maxPage = modeInfo->BnkNumberOfImagePages;

		/* Extract VBE/Core 3.0 hardware flags */
        *flags |= MGL_HAVE_REFRESH_CTRL;
		if (modeInfo->ModeAttributes & vbeMdInterlaced)
			*flags |= MGL_HAVE_INTERLACED;
		if (modeInfo->ModeAttributes & vbeMdDoubleScan)
        	*flags |= MGL_HAVE_DOUBLE_SCAN;
		if (modeInfo->ModeAttributes & vbeMdTripleBuf)
			*flags |= MGL_HAVE_TRIPLEBUFFER;
		if (modeInfo->ModeAttributes & vbeMdStereo)
			*flags |= MGL_HAVE_STEREO;
		if (modeInfo->ModeAttributes & vbeMdDualDisp)
			*flags |= MGL_HAVE_STEREO_DUAL;
		if (VBE_capabilities & vbeHWStereoSync)
			*flags |= MGL_HAVE_STEREO_HWSYNC;
		if (VBE_capabilities & vbeEVCStereoSync)
			*flags |= MGL_HAVE_STEREO_EVCSYNC;
		}
	else
		mi->maxPage = modeInfo->NumberOfImagePages;
	mi->pageSize = VBE_getPageSize(modeInfo);
	mi->modeFlags = *flags;
	return true;
}

void MGLAPI SVGA_useMode(modetab modes,int mode,int id,gmode_t *mi)
/****************************************************************************
*
* Function:		SVGA_useMode
* Parameters:	modes	- Video mode/device driver association table
*				mode	- Index into mode association table
*				id		- Driver id to store in table
*				mi		- Mode information block to fill in
*
* Description:	Calls the SVGA VBE BIOS to determine the information for
*				the specified video mode. We search for a video mode that
*				matches our specifications of pixels resolution and depth,
*				and fill in the remainder of the modeInfo block with the
*				values returned by the VBE.
*
****************************************************************************/
{
	VBE_modeInfo	modeInfo;
	ulong			flags;

	if (findMode(mi,&modeInfo,false,&flags))
		DRV_useMode(modes,mode,id,mi->maxPage+1,flags);
}

void MGLAPI SVGALIN_useMode(modetab modes,int mode,int id,gmode_t *mi,
	ibool useVirtual)
/****************************************************************************
*
* Function:		SVGALIN_useMode
* Parameters:	modes		- Video mode/device driver association table
*				mode		- Index into mode association table
*				id			- Driver id to store in table
*				mi			- Mode information block to fill in
*				useVirtual	- True if driver can use virtualised framebuffer
*
* Description:	Calls the SVGA VBE BIOS to determine the information for
*				the specified video mode. We search for a video mode that
*				matches our specifications of pixels resolution and depth,
*				and fill in the remainder of the modeInfo block with the
*				values returned by the VBE.
*
****************************************************************************/
{
	VBE_modeInfo	modeInfo;
    ulong			flags;

	if (findMode(mi,&modeInfo,true,&flags)) {
		if (modeInfo.ModeAttributes & vbeMdLinear) 
			mi->scratch2 |= vbeLinearBuffer;
		else if (useVirtual && useVFlat)
			mi->scratch2 |= VBE_VIRTUAL_LINEAR;
		else
			return;
		DRV_useMode(modes,mode,id,mi->maxPage+1,flags);
		}
}

PRIVATE ibool SVGA_set8BitPalette(void)
/****************************************************************************
*
* Function:		SVGA_set8BitPalette
* Returns:		True if 8 bit wide palette has been set.
*
* Description:	Attempts to set the system into the 8 bit wide palette
*				mode if supported by the VBE. Returns true on success, false
*				otherwise.
*
****************************************************************************/
{
	if (!VBE_setDACWidth(8))
		return false;
	if (VBE_getDACWidth() != 8)
		return false;
	return true;
}

#if	defined(SAFE_MODE) || defined(__WINDOWS__)
PRIVATE void backslash(char *s)
{
	uint pos = strlen(s);
	if (s[pos-1] != '\\') {
		s[pos] = '\\';
		s[pos+1] = '\0';
		}
}
#endif

#pragma warn -par
PUBLIC void SVGA_saveVideoMemory(int mode)
/****************************************************************************
*
* Function:     SVGA_saveVideoMemory
*
* Description:  Starts a 640x480x256 color graphics mode and streams all
*				video memory to a tempory disk file. This is necessary so
*				we can restore the video memory back to the state it was
*				in originally to avoid problems with buggy Windows drivers
*				that cache bitmaps in offscreen video memory but do not
*				correctly restore the memory when a DOS box or WinDirect
*				application returns to GDI mode.
*
*				If we are starting a banked video mode, we start a banked
*				version of 640x480x256 and save the memory with that. For
*				linear modes we start the linear version instead.
*
****************************************************************************/
{
#if	defined(SAFE_MODE) || defined(__WINDOWS__)
	RMREGS			regs;
	VBE_modeInfo	modeInfo;
	ulong			size;
	ibool			fail = false;
	FILE			*f;
	VBE_palette		pal[256];

	if (saveMemName[0] != 0)
		return;

#ifndef	__WINDOWS__
	/* Dont bother to save/restore video memory when running under real DOS,
	 * only under Windows DOS boxes.
	 */
	regs.x.ax = 0x160A;
	PM_int86(0x2F,&regs,&regs);
	if (regs.x.ax != 0)
		return;
#else
	if (!WD_isSafeMode())
		return;
#endif

	/* Set the 640x480x256 graphics mode to save memory with */
	saveMode = 0x101 | (mode & vbeLinearBuffer) | vbeDontClear;
	VBE_getModeInfo(saveMode & vbeModeMask,&modeInfo);
	regs.x.ax = 0x4F02;
	regs.x.bx = saveMode;
	PM_int86(0x10,&regs,&regs);
	if (regs.x.ax != 0x4F)
		return;

	/* Set the palette for the mode to black */
	memset(pal,0,sizeof(pal));
	if (!isNonVGA)
		VBE_setPalette(0,256,pal,false);
	else
		VGA_blastPalette((uchar*)pal,256,0,256,false);

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
#ifdef	PM386
	if (saveMode & vbeLinearBuffer) {
		void *p = VBE_getLinearPointer(&modeInfo);
		if (!p)
			MGL_fatalError("Unable to obtain pointer to linear framebuffer!");
		size = VBE_memory * 1024L;
		if (fwrite(p,1,size,f) != size)
			fail = true;
		}
	else
#endif
		{
#if	!defined(__WIN386__) && !defined(DJGPP)
		int		i,bankShift,maxBlocks = VBE_memory / 64;
		uchar	*p = PM_mapPhysicalAddr(0xA0000UL,0xFFFF);

		if (!p)
			MGL_fatalError("Unable to obtain video memory pointer!\n");
		bankShift = 0;
		while ((64 >> bankShift) != modeInfo.WinGranularity)
			bankShift++;
		size = 0x8000;
		for (i = 0; i < maxBlocks; i++) {
			VBE_setBank(0,(i >> 1) << bankShift);
			VBE_setBank(1,(i >> 1) << bankShift);
			if ((i & 1) == 0) {
				if (fwrite(p,1,size,f) != size) {
					fail = true;
					break;
					}
				}
			else {
				if (fwrite(p + size,1,size,f) != size) {
					fail = true;
					break;
					}
				}
			}
#endif
		}
	fclose(f);
	if (fail) {
		remove(saveMemName);
		saveMemName[0] = 0;
		}
#endif
}

PUBLIC void SVGA_restoreVideoMemory(void)
/****************************************************************************
*
* Function:     SVGA_restoreVideoMemory
*
* Description:  Restores the video memory from the memory buffer or
*				disk file after the mode has been reset to correctly
*				restore all cached bitmaps for Windows drivers.
*
****************************************************************************/
{
#if	defined(SAFE_MODE) || defined(__WINDOWS__)
	RMREGS			regs;
	VBE_modeInfo	modeInfo;
	ulong			size;
	FILE			*f;
	VBE_palette		pal[256];

	if (saveMemName[0] == 0)
		return;

	/* Set the 640x480x256 graphics mode to restore memory with */
	VBE_getModeInfo(saveMode & vbeModeMask,&modeInfo);
	regs.x.ax = 0x4F02;
	regs.x.bx = saveMode;
	PM_int86(0x10,&regs,&regs);
	if (regs.x.ax != 0x4F)
		return;

	/* Set the palette for the mode to black */
	memset(pal,0,sizeof(pal));
	if (!isNonVGA)
		VBE_setPalette(0,256,pal,false);
	else
		VGA_blastPalette((uchar*)pal,256,0,256,false);

	/* Open the temporary file to restore the video memory from */
	if ((f = fopen(saveMemName,"rb")) == NULL)
    	return;

	/* Restore entire video memory in chunks of 32Kb at a time except under
	 * 32 bit protecetd mode with a linear framebuffer in which case we
	 * can restore it directly as a single block.
	 */
#ifdef	PM386
	if (saveMode & vbeLinearBuffer) {
		void *p = VBE_getLinearPointer(&modeInfo);
		if (!p)
			MGL_fatalError("Unable to obtain pointer to linear framebuffer!");
		size = VBE_memory * 1024L;
		fread(p,1,size,f);
		}
	else
#endif
		{
#if	!defined(__WIN386__) && !defined(DJGPP)
		int		i,bankShift,maxBlocks = VBE_memory / 64;
		uchar	*p = PM_mapPhysicalAddr(0xA0000UL,0xFFFF);

		if (!p)
			MGL_fatalError("Unable to obtain video memory pointer!\n");
		bankShift = 0;
		while ((64 >> bankShift) != modeInfo.WinGranularity)
			bankShift++;
		size = 0x8000;
		for (i = 0; i < maxBlocks; i++) {
			VBE_setBank(0,(i >> 1) << bankShift);
			VBE_setBank(1,(i >> 1) << bankShift);
			if ((i & 1) == 0)
				fread(p,1,size,f);
			else
				fread(p + size,1,size,f);
			}
#endif
		}
	fclose(f);
	remove(saveMemName);
	saveMemName[0] = 0;
#endif
}

/****************************************************************************
REMARKS:
Enables free running stereo mode. We also re-program the blue color color
index to the correct value in here, so that the developer can enable and
disable support for blue codes at runtime with calls to MGL_setBlueCodeIndex
and MGL_startStereo.
****************************************************************************/
void _ASMAPI SVGA_startStereo(MGLDC *dc)
{
	if (dc->v->d.isStereo != _MGL_NO_STEREO) {
		if (!(dc->v->d.isStereo & _MGL_HW_STEREO_SYNC) && (_MGL_stereoSyncType == MGL_STEREO_BLUE_CODE)) {
			/* Program the blue code palette entry to pure blue */
			MGL_setPaletteEntry(dc,_MGL_blueCodeIndex,0,0,255);
			MGL_realizePalette(dc,256,0,false);
			}
		if (dc->v->d.isStereo & _MGL_HW_STEREO) {
			/* Enable hardware stereo page flipping */
			VBE_enableStereoMode();
			}
		if (dc->v->d.isStereo & _MGL_SOFT_STEREO) {
			// TODO: Enable software page flip interrupt handler in here
			}
		dc->v->d.stereoRunning = true;
		MGL_setVisualPage(dc,dc->v->d.visualPage,MGL_dontWait);
		}
}

/****************************************************************************
REMARKS:
Disables free running stereo mode, so the display controller always
displays from the left eye image.
****************************************************************************/
void _ASMAPI SVGA_stopStereo(MGLDC *dc)
{
	if (dc->v->d.isStereo != _MGL_NO_STEREO && dc->v->d.stereoRunning) {
		if (dc->v->d.isStereo & _MGL_HW_STEREO) {
			/* Disable hardware stereo page flipping */
			VBE_disableStereoMode();
			}
		if (dc->v->d.isStereo & _MGL_SOFT_STEREO) {
			// TODO: Disable software page flip interrupt handler in here
			}
		dc->v->d.stereoRunning = false;
		}
}

/****************************************************************************
REMARKS:
Determines if the graphics mode should be double scanned or not.
****************************************************************************/
ibool GTF_useDoubleScan(MGLDC *dc,int refreshRate)
{
	if (dc->mi.modeFlags & MGL_HAVE_DOUBLE_SCAN) {
		if (dc->mi.xRes < 639 && dc->mi.yRes <= 383) {
        	/* Don't double scan 512x384 at high refresh rates */   
			if (dc->mi.xRes == 511 && dc->mi.yRes == 383 && refreshRate > 80)
            	return false;
			return true;
			}
		}
	return false;
}

/****************************************************************************
REMARKS:
This function computes the maximum refresh rate for the graphics modes
specified by the passed in mode information block. The maximum refresh
rate is computed from the information returned by the VBE BIOS or VBE/AF
driver, using CRTC values computed using the VESA GTF formulas.
****************************************************************************/
int GTF_getMaxRefreshRate(MGLDC *dc,ulong maxPixelClock,ibool interlaced)
{
	GTF_timings	gtf;
	int			xRes = dc->mi.xRes+1;
	int			yRes = dc->mi.yRes+1;
	ibool		doubleScan;

	doubleScan = GTF_useDoubleScan(dc,-1);
	if (doubleScan)
		yRes *= 2;
	GTF_calcTimings(xRes,yRes,maxPixelClock / 1000000.0,GTF_lockPF,false,interlaced,&gtf);
	return (int)gtf.vFreq;
}

static double round(double v)
{
	return floor(v + 0.5);
}

/****************************************************************************
REMARKS:
This function computes a set of CRTC timings for the specific graphics
mode at the specified refresh rate. The CRTC timings are computed using
the VESA GTF timing formulas, combined with information returned by the
VBE/Core 3.0 BIOS or VBE/AF driver. Note that the exact refresh rate that
you get will not be exactly what you requested, and the exact value is
returned in the crtc information block block.
****************************************************************************/
ibool GTF_computeCRTCTimings(MGLDC *dc,int mode,int refreshRate,ibool interlaced,
	AF_CRTCInfo *crtc,ulong (_ASMAPI *getClosestPixelClock)(ushort mode,ulong pixelClock))
{
	GTF_timings	gtf,bestGTF;
	int			xRes = dc->mi.xRes+1;
	int			yRes = dc->mi.yRes+1;
	float		closestRefresh,refresh,minRefresh,maxRefresh;
	ulong		clock,closestClock;
	ibool		doubleScan;

	/* If we have a double scanned mode, then double the vertical
	 * timing parameters.
	 */
	doubleScan = GTF_useDoubleScan(dc,refreshRate);
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

        /* Now find the closest pixel clock */
		clock = getClosestPixelClock(mode,(long)(gtf.dotClock * 1000000.0));
		if (clock == 0xFFFFFFFF)
			continue;

		/* Now run the GTF timings once more given the closest pixel clock */
		GTF_calcTimings(xRes,yRes,clock / 1000000.0,GTF_lockPF,false,interlaced,&gtf);
		if (fabs(gtf.vFreq-refreshRate) < fabs(closestRefresh-refreshRate)) {
			closestRefresh = gtf.vFreq;
			closestClock = clock;
			bestGTF = gtf;
			}
		}
	if (closestClock == 0xFFFFFFFF)
		return false;

	/* Now modify the horizontal and vertical total's to achieve the
	 * exact refresh rate, since if we have a table driven clock we
	 * really want to hit the refresh rate on the nail (not necessarily
	 * the GTF compatible timings) for legacy graphics adapters. All
	 * modern graphics adapaters have programmable PLL's with fine
	 * pixel clock granularities and wont need any adjustment.
	 *
	 * Note that we make any major adjustments uising the horizontal
	 * total first, and then any minor adjustments using the vertical
	 * total.
	 */
	/* TODO: This does not work properly for discrete clock cards that have
	 * 		 pixel clocks that are not close to what we want. We need a better
	 *		 way of handling this.
     */
/*	bestGTF.v.vTotal = round((float)closestClock / (bestGTF.h.hTotal * refreshRate));
	bestGTF.h.hTotal = round((float)closestClock / (bestGTF.v.vTotal * refreshRate));
	bestGTF.h.hTotal = round(bestGTF.h.hTotal / 8) * 8;*/

	/* Now return the computed timings to the application */
	crtc->HorizontalTotal = bestGTF.h.hTotal;
	crtc->HorizontalSyncStart = bestGTF.h.hSyncStart;
	crtc->HorizontalSyncEnd = bestGTF.h.hSyncEnd;
	crtc->VerticalTotal = bestGTF.v.vTotal;
	crtc->VerticalSyncStart = bestGTF.v.vSyncStart;
	crtc->VerticalSyncEnd = bestGTF.v.vSyncEnd;
	crtc->PixelClock = closestClock;
	crtc->RefreshRate = (ushort)(bestGTF.vFreq * 100.0);
	crtc->Flags = 0;
	if (interlaced)
		crtc->Flags |= afInterlaced;
	if (doubleScan)
		crtc->Flags |= afDoubleScan;
	if (xRes <= 640 && yRes <= 400) {
		/* Get the sync polarities for the mode. For low resolution modes
		 * below 640x480, we need to use specific sync polarities since
		 * they are used to determine the vertical sizing for compatibility
		 * with VGA CRTC timings.
		 */
		if (yRes == 350)
			crtc->Flags |= afVSyncNeg;
		else if (yRes == 200 || yRes == 384 || yRes == 400)
			crtc->Flags |= afHSyncNeg;
		else
			crtc->Flags |= afHSyncNeg | afVSyncNeg;
		}
	else {
		/* Use polarities returned from bestGTF formulas */
		if (bestGTF.hSyncPol == '-')
			crtc->Flags |= afHSyncNeg;
		if (bestGTF.vSyncPol == '-')
			crtc->Flags |= afVSyncNeg;
		}
	return true;
}

ibool MGLAPI SVGA_setMode(MGLDC *dc,int AX,int BX,short *oldBIOSMode,
	ibool *old50Lines,int virtualX,int virtualY,int numBuffers,ibool stereo,
	int refreshRate)
/****************************************************************************
*
* Function:		SVGA_setMode
* Parameters:	dc			- Device context to initialise
*				AX,BX		- Register values
*				oldBIOSMode	- Place to store old BIOS mode number
*				old50Lines	- Place to store old 43 line mode flag
* Returns:		TRUE if the mode was successfully set.
*
* Description:	Initialises the video mode by calling the VIDEO BIOS with
*				the specified values of AX and BX. We also save the state
*				of the old video mode in the specified parameters.
*
****************************************************************************/
{
	RMREGS			regs;
	int				v,maxScans,scrolling,dualBanks,isStereo;
	VBE_modeInfo	modeInfo;
	static void		*videoMem = NULL;
    AF_CRTCInfo		crtc;

	/* If stereo mode is enabled, check what type of stereo mode we support
	 * and set things up for stereo operation.
	 */
	isStereo = _MGL_NO_STEREO;
	if (stereo) {
		isStereo = _MGL_SOFT_STEREO;
		if (dc->mi.modeFlags & MGL_HAVE_STEREO) {
			/* We have hardware stereo page flipping, so use it */  
			isStereo = _MGL_HW_STEREO;
			if (dc->mi.modeFlags & MGL_HAVE_STEREO_DUAL)
            	isStereo |= _MGL_HW_STEREO_DUAL;
			}
		if (!(dc->mi.modeFlags & MGL_HAVE_STEREO_HWSYNC) || (_MGL_stereoSyncType & MGL_STEREO_IGNORE_HW_STEREO)) {
			/* We dont have hardware stereo sync (or is has been disabled).
			 * If we are not using the StereoGraphics blue code sync system,
			 * then we must fall back on a software page flip interrupt
			 * handler to send the sync signal out to the LC shutter glasses
			 * over the parallel port or the serial port (depending on the
			 * type of glasses.
			 */
			if ((_MGL_stereoSyncType != MGL_STEREO_BLUE_CODE) && (isStereo & _MGL_HW_STEREO))
				isStereo |= _MGL_SOFT_STEREO;
			}
		else {
			/* We have hardware stereo sync, so set the flag for this */
            isStereo |= _MGL_HW_STEREO_SYNC;
        	}
		numBuffers *= 2;
		}
	// TODO: For now we dont support software stereo page flipping, so we
	//		 fail if this is required
	if (isStereo & _MGL_SOFT_STEREO)
    	return false;

	/* Check that we are withing the virtual dimensions */
	if (numBuffers == 0)
    	return false;
	if (virtualX != -1 && virtualY != -1) {
		if (virtualX <= dc->mi.xRes || virtualY <= dc->mi.yRes)
			return false;
		scrolling = true;
		}
	else
		scrolling = false;

	/* Determine if we need to add the MultiBuffer flag */
	if (numBuffers && ((numBuffers-1) <= dc->mi.maxPage) && dc->mi.maxPage > 0) {
		if (stereo)
			dc->mi.maxPage = (numBuffers/2)-1;
		else
			dc->mi.maxPage = numBuffers-1;
		}
	else
		dc->mi.maxPage = 0;

	/* Enable fullscreen mode */
	MGLPC_init();
	MGLPC_startFullScreen(dc->mi.xRes+1,dc->mi.yRes+1);

	/* Save the old video mode state */
	regs.h.ah = 0x0F;
	PM_int86(0x10,&regs,&regs);
	*oldBIOSMode = regs.h.al & 0x7F;
	*old50Lines = false;
	if (*oldBIOSMode == 0x3) {
		regs.x.ax = 0x1130;
		regs.x.bx = 0;
		regs.x.dx = 0;
		PM_int86(0x10,&regs,&regs);
		*old50Lines = (regs.h.dl == 42 || regs.h.dl == 49);
		}

	/* Reset to 80x25 text mode first to ensure things work properly */
	regs.x.ax = 0x83;
	PM_int86(0x10,&regs,&regs);

	/* Compute CRTC timings if refresh rate is specified */
	VBE_getModeInfo(BX & ~vbeLinearBuffer,&modeInfo);
	if (refreshRate != MGL_DEFAULT_REFRESH && VBE_version >= 0x300) {
		int maxRefresh = GTF_getMaxRefreshRate(dc,modeInfo.MaxPixelClock,false);
		if (refreshRate > maxRefresh)
			refreshRate = maxRefresh;
		if (refreshRate < 60)
        	refreshRate = 60;
		if (GTF_computeCRTCTimings(dc,BX,refreshRate,false,&crtc,VBE_getClosestClock))
			BX |= vbeRefreshCtrl;
		}

	/* Set up for the color display to be active, and set the mode */
	v = PM_getByte(_MGL_biosSel,0x10);
	PM_setByte(_MGL_biosSel,0x10, (v & 0xCF) | 0x20);
	SVGA_saveVideoMemory(BX);
	PM_setByte(_MGL_biosSel,0x49,0x10);
	if (!VBE_setVideoModeExt(BX,(VBE_CRTCInfo*)&crtc))
		return false;

	/* Enable 8-bit wide palette */
	if (dc && dc->mi.maxColor <= 0xFF && !stereo)
		dc->v->d.widePalette = SVGA_set8BitPalette();
	else
    	dc->v->d.widePalette = false;

	/* Use SVGA versions of page flipping and palette programming code */
	dc->v->d.isStereo			= isStereo;
    dc->v->d.refreshRate		= refreshRate;
    dc->v->d.stereoRunning		= false;
	dc->v->d.startStereo		= SVGA_startStereo;
    dc->v->d.stopStereo			= SVGA_stopStereo;
	dc->v->d.setActivePage 		= SVGA_setActivePage;
	dc->v->d.setVisualPage 		= SVGA_setVisualPage;
	dc->v->d.setDisplayStart	= SVGA_setDisplayStart;
	dc->r.realizePalette 		= SVGA_realizePalette;
	dc->v->d.vSync 				= SVGA_vSync;
	VBE_bankShift = 0;
	while ((64 >> VBE_bankShift) != modeInfo.WinGranularity)
		VBE_bankShift++;
	SVGA_curBank = -1;

	/* If the controller is reported as NonVGA, then we must use the
	 * provided VBE 2.0 palette programming functions. Otherwise we
	 * can simply program the VGA palette registers directly.
	 */
	isNonVGA = ((modeInfo.ModeAttributes & vbeMdNonVGA) != 0);

	/* Find out if the VBE driver requires dual banks or not */
	dualBanks = ((modeInfo.WinAAttributes & 0x7) != 0x7);

	/* Set up for VBE 2.0 32 bit protected mode bank switching and
	 * linear framebuffer access
	 */
	VBE_freePMCode();

	/* Only use the VBE 2.0 protected mode functions if UniVBE is installed
	 * under Windows as some drivers may not work in this environment.
	 */
#ifdef	__WINDOWS__
	if (strncmp(OemString,UNIVBE_STR,sizeof(UNIVBE_STR)-1) == 0)
#endif
		{
		setBank20 = (void*)VBE_getSetBank();
		setCRT20 = (void*)VBE_getSetDisplayStart();
		setPal20 = (void*)VBE_getSetPalette();
		}
	if (dc->mi.scratch2 & vbeLinearBuffer) {
		void *p = VBE_getLinearPointer(&modeInfo);
		if (!p)
			MGL_fatalError("Unable to obtain pointer to linear framebuffer!");
		dc->surfaceStart = dc->surface = p;
		dc->v->d.setActivePage	= SVGALIN_setActivePage;
        dc->flags |= MGL_LINEAR_ACCESS;
		}
	else if (dc->mi.bitsPerPixel > 4 && (dc->mi.scratch2 & VBE_VIRTUAL_LINEAR)) {
		void 	*bankFunc,*p;
		int 	codeLen;
		VBE_getBankFunc32(&codeLen,&bankFunc,dualBanks,VBE_bankShift);
        VF_exit();
		p = VF_init((ulong)modeInfo.WinASegment << 4,modeInfo.WinSize,codeLen,bankFunc);
		if (!p)
			MGL_fatalError("Unable to initialise virtual linear framebuffer!");
		dc->flags &= ~MGL_LINEAR_ACCESS;
		dc->flags |= MGL_VIRTUAL_ACCESS;
		dc->surfaceStart = dc->surface = p;
		dc->v->d.setActivePage	= SVGALIN_setActivePage;
		}
	else {
		if (!videoMem) {
			if ((videoMem = PM_mapPhysicalAddr(0xA0000UL,0xFFFF)) == NULL) {
				MGLPC_disableFullScreen();
				MGL_fatalError("Unable to obtain video memory pointer!\n");
				}
			}
		dc->surfaceStart = dc->surface = videoMem;
		}

	/* Get pointer to bank switch function to use */
	VBE_bankSwitch = NULL;
	if (dualBanks) {
		if (setBank20) {
			if (VBE_MMIOSel)
				SVGA_setBankVec = VBE20_setBankAB_ES;
			else
				SVGA_setBankVec = VBE20_setBankAB;
			}
		else
			SVGA_setBankVec = VBE_setBankAB;
		}
	else {
		if (setBank20) {
			if (VBE_MMIOSel)
				SVGA_setBankVec = VBE20_setBankA_ES;
			else
				SVGA_setBankVec = VBE20_setBankA;
			}
		else
			SVGA_setBankVec = VBE_setBankA;
		}

	/* Setup for virtual scrolling if specified */
	DispDC = NULL;
	if (scrolling) {
		if (!VBE_setPixelsPerLine(virtualX,&dc->mi.bytesPerLine,&virtualX,&maxScans)) {
			SVGA_restoreTextMode(dc);
			return false;
			}
		dc->mi.xRes = virtualX - 1;
		dc->mi.yRes = virtualY - 1;
		dc->startX = dc->startY = 0;
		dc->mi.maxPage = (maxScans / virtualY);
		modeInfo.BytesPerScanLine = dc->mi.bytesPerLine;
		dc->mi.pageSize = VBE_getPageSize(&modeInfo);
		dc->CRTCBase = 0;
		}
	dc->virtualX = virtualX;
	dc->virtualY = virtualY;
	DispDC = dc;

	/* Ensure active and visual pages are set to 0 */
	dc->v->d.setActivePage(dc,0);
	dc->v->d.setVisualPage(dc,0,MGL_dontWait);
	return true;
}

void MGLAPI SVGA_setVirtualValues(int bytes,int pixels,int maxScans)
/****************************************************************************
*
* Function:		SVGA_setVirtualValues
* Parameters:	dc			- Device context to initialise
*
* Description:	For compatibility with SuperVGA Kit applications that call
*				the VBE_setPixelsPerLine functions directly, we call this
*				internal function to correctly inform the MGL of the changes
*				to the display mode. Note that only 1 page will be
*				available, so to combine page flipping and virtual scrolling
*				you must call the proper MGL functions to set up a virtual
*				scrolling mode.
*
****************************************************************************/
{
	if (DispDC) {
		DispDC->mi.xRes = (DispDC->virtualX = pixels) - 1;
		DispDC->mi.yRes = (DispDC->virtualY = maxScans) - 1;
		DispDC->startX = DispDC->startY = 0;
		DispDC->mi.maxPage = 0;
		DispDC->mi.bytesPerLine = bytes;
		DispDC->CRTCBase = 0;
		MGL_makeSubDC(DispDC,0,0,pixels,maxScans);
		}
}

void _ASMAPI SVGA_restoreTextMode(MGLDC *dc)
/****************************************************************************
*
* Function:		SVGA_restoreTextMode
* Parameters:	oldBIOSMode	- Old BIOS mode number
*				old50Lines	- True if old mode was 43/50 line mode
*
* Description:	Restores the previous video mode active before graphics
*				mode was entered.
*
****************************************************************************/
{
	RMREGS		regs;
	int			v;
	SVGA_state	*state = (SVGA_state*)dc->v;

	/* Reset display start address */
	VBE_setCRTStart(0, dc->mi.bitsPerPixel, dc->mi.bytesPerLine, 0);

	/* Restore the video memory if it was saved */
	SVGA_restoreVideoMemory();

	/* Set up to return to mono monitor if mono mode was active */
	if (state->oldBIOSMode == 0x7) {
		v = PM_getByte(_MGL_biosSel,0x10);
		PM_setByte(_MGL_biosSel,0x10, (v & 0xCF) | 0x30);
		}
	regs.x.ax = 0x3;
	PM_int86(0x10,&regs,&regs);     	/* Set text mode       			*/
	if (state->old50Lines) {
		regs.x.ax = 0x1112;
		regs.x.bx = 0;
		PM_int86(0x10,&regs,&regs);		/* Restore 50 line mode         */

		regs.x.ax = 0x1130;
		regs.x.bx = 0;
		regs.x.dx = 0;
		PM_int86(0x10,&regs,&regs);
		if (regs.h.dl == 42) {
			/* This is the EGA 43 line mode, so turn off cursor emulation
			 * for the EGA, and set the default cursor start/end scanlines.
			 */
			v = PM_getByte(_MGL_biosSel,0x87);
			PM_setByte(_MGL_biosSel,0x87,v | 0x1);
			regs.h.ah = 0x1;
			regs.x.cx = 0x0607;
			PM_int86(0x10,&regs,&regs);
			}
		}
	MGLPC_disableFullScreen();
	VF_exit();
	VBE_freePMCode();                   /* Free up protected mode code  */
}

ibool _ASMAPI SVGA_restoreGraphMode(MGLDC *dc)
{
	short t1; ibool t2;
	if (!SVGA_setMode(dc,dc->mi.scratch1,dc->mi.scratch2,&t1,&t2,
			dc->virtualX,dc->virtualY,dc->mi.maxPage+1,
			dc->v->d.isStereo != _MGL_NO_STEREO,dc->v->d.refreshRate))
		return false;
	if (dc->v->d.stereoRunning)
    	SVGA_startStereo(dc);
	return true;
}

void _ASMAPI SVGA_setActivePage(MGLDC *dc,int page)
/****************************************************************************
*
* Function:		SVGA_setActivePage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current active page for the device context to
*				the specified page value.
*
****************************************************************************/
{
	long 	size;
	int		maxPage = dc->mi.maxPage;
	int 	ipage = (page & ~MGL_RIGHT_BUFFER);

	if (dc->v->d.isStereo != _MGL_NO_STEREO)
		maxPage = (maxPage+1)*2-1;
	if (ipage >= 0 && ipage <= dc->mi.maxPage) {
		dc->v->d.activePage = page;
		if (dc->v->d.isStereo != _MGL_NO_STEREO) {
			if (page & MGL_RIGHT_BUFFER)
				ipage = ((page & ~MGL_RIGHT_BUFFER) * 2) + 1;
			else
				ipage *= 2;
			}
		size = dc->mi.pageSize * ipage;
		dc->surface = dc->surfaceStart + (dc->originOffset = (size & 0xFFFF));
		SVGA_setBankC(dc->bankOffset = size >> 16);
		}
}

#ifdef	PM386
void _ASMAPI SVGALIN_setActivePage(MGLDC *dc,int page)
/****************************************************************************
*
* Function:		SVGALIN_setActivePage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current active page for the device context to
*				the specified page value.
*
****************************************************************************/
{
	int	maxPage = dc->mi.maxPage;
    int ipage = (page & ~MGL_RIGHT_BUFFER);

	if (dc->v->d.isStereo != _MGL_NO_STEREO)
		maxPage = (maxPage+1)*2-1;
	if (ipage >= 0 && ipage <= dc->mi.maxPage) {
		dc->v->d.activePage = page;
		if (dc->v->d.isStereo != _MGL_NO_STEREO) {
			if (page & MGL_RIGHT_BUFFER)
				ipage = ((page & ~MGL_RIGHT_BUFFER) * 2) + 1;
			else
				ipage *= 2;
			}
		dc->surface = dc->surfaceStart + dc->mi.pageSize * ipage;
		dc->originOffset = (ulong)dc->surface;
#ifdef	__WINDOWS__
		/* Set the new active page in our Windows DIB driver */
		if (dc->wm.fulldc.hdc) {
			Escape(dc->wm.fulldc.hdc,MGLDIB_SETSURFACEPTR,4,
				(void*)&dc->surface,NULL);
			}
#endif
		}
}
#endif

void _ASMAPI SVGA_setVisualPage(MGLDC *dc,int page,int waitVRT)
/****************************************************************************
*
* Function:		SVGA_setVisualPage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current visual page for the device context to
*				the specified page value.
*
****************************************************************************/
{
	int		oldActivePage;
	color_t	oldColor,blueCodeColor;

	if (page >= 0 && page <= dc->mi.maxPage) {
		dc->v->d.visualPage = page;
		if (dc->v->d.stereoRunning && (_MGL_stereoSyncType == MGL_STEREO_BLUE_CODE)) {
			/* Get the color for the blue code */
			if (dc->mi.maxColor > 255)
				blueCodeColor = MGL_packColor(&dc->pf,0,0,255);
			else
				blueCodeColor = _MGL_blueCodeIndex;

			/* Save the old color and active page for later */
			oldColor = dc->a.color;
			oldActivePage = dc->v->d.activePage;

			/* Draw the left image blue code (25% of width) */
			MGL_setActivePage(dc,page);
            dc->r.setColor(blueCodeColor);
			dc->r.solid.scanLine(dc->mi.yRes,0,dc->mi.xRes/4);
			dc->r.setColor(0);
			dc->r.solid.scanLine(dc->mi.yRes,dc->mi.xRes/4,dc->mi.xRes);

			/* Draw the right image blue code (75% of width) */
			MGL_setActivePage(dc,page | MGL_RIGHT_BUFFER);
			dc->r.setColor(blueCodeColor);
			dc->r.solid.scanLine(dc->mi.yRes,0,(dc->mi.xRes*3)/4);
			dc->r.setColor(0);
			dc->r.solid.scanLine(dc->mi.yRes,(dc->mi.xRes*3)/4,dc->mi.xRes);

			/* Restore the old color and active page */
            dc->r.setColor(oldColor);
			MGL_setActivePage(dc,oldActivePage);
			}

		/* Disable triple buffering if only two pages */
		if (dc->mi.maxPage == 1 && waitVRT == MGL_tripleBuffer)
			waitVRT = MGL_waitVRT;

        /* Wait until the previous flip has occured if doing triple buffering */
		if ((dc->mi.modeFlags & MGL_HAVE_TRIPLEBUFFER) && (waitVRT == MGL_tripleBuffer)) {
			while (VBE_getDisplayStartStatus() == 0)
				;
			}
		if (dc->virtualX != -1) {
        	/* Virtual scrolling is active, so use set display start */
			dc->CRTCBase = (dc->mi.pageSize * page);
			dc->v->d.setDisplayStart(dc,dc->startX,dc->startY,waitVRT);
			}
		else {
			/* Program the display start address */
        	ulong addr = dc->mi.pageSize * page;
			if (VBE_version >= 0x300) {
				if (dc->v->d.stereoRunning && (dc->v->d.isStereo & _MGL_HW_STEREO_DUAL))
					VBE_setStereoDisplayStart(addr,addr+dc->mi.pageSize,(waitVRT == MGL_waitVRT));
				else
					VBE_setDisplayStartAlt(addr,(waitVRT == MGL_waitVRT));
				}
			else
				VBE_setCRTStart(addr, dc->mi.bitsPerPixel,dc->mi.bytesPerLine,(waitVRT == MGL_waitVRT));
			}
		}
}

void _ASMAPI SVGA_setDisplayStart(MGLDC *dc,int x,int y,int waitFlag)
/****************************************************************************
*
* Function:		MGL_setDisplayStart
* Parameters:	dc			- Device context
*				x,y			- Display start coordinates
*				waitFlag	- Flag whether to wait for retrace.
*
* Description:	Sets the current display start address for the screen. The
*				value of waitFlag can be one of the following:
*
*					-1	- Set coordinates but dont change hardware
*					0	- Set coordinates and hardware; no wait for retrace
*					1	- Set coordinates and hardware; wait for retrace
*
*				Passing a waitFlag of -1 is used to implement double
*				buffering and virtual scrolling at the same time. You call
*				this function first to set the display start X and Y
*				coordinates, then you call SVGA_setVisualPage() to swap
*				display pages and the new start address takes hold with that
*				call.
*
****************************************************************************/
{
	if (dc->virtualX != -1) {
		ulong start;

		/* Update current display start coordinates */
		dc->startX = x;
		dc->startY = y;
		if (waitFlag == -1)
			return;

		/* Compute starting address for display mode */
		switch (dc->mi.bitsPerPixel) {
			case 4:
				start = dc->CRTCBase + dc->mi.bytesPerLine * y + x / 8;
				break;
			case 8:
				start = dc->CRTCBase + dc->mi.bytesPerLine * y + x;
				break;
			case 15:
			case 16:
				start = dc->CRTCBase + dc->mi.bytesPerLine * y + x * 2;
				break;
			case 24:
				start = dc->CRTCBase + dc->mi.bytesPerLine * y + x * 3;
				break;
			default: /* case 32: */
				start = dc->CRTCBase + dc->mi.bytesPerLine * y + x * 4;
				break;
			}
		if (VBE_version >= 0x300)
			VBE_setDisplayStartAlt(start,(waitFlag == MGL_waitVRT));
		else
			VBE_setCRTStart(start, dc->mi.bitsPerPixel, dc->mi.bytesPerLine,(waitFlag == MGL_waitVRT));
		}
}

void _ASMAPI SVGA_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,
	int waitVRT)
/****************************************************************************
*
* Function:		SVGA_realizePalette
* Parameters:	dc		- Device context
*               pal		- Palette of values to program
*               num		- Number of entries to program
*				index	- Index to start programming at
*
* Description:	Program the VGA palette. First we need to translate the
*				values from the MGL internal format into the 6 bit RGB
*				format used by the EGA.
*
****************************************************************************/
{
	int			i,count;
	uchar		temp[1024],*t,*p = (uchar*)&pal[index];
	display_vec	*d = &dc->v->d;

	/* First copy the palette values into our temporary palette,
	 * translating from the internal 8 bit format to the VGA 6 bit
	 * format (if we have a wide palette, then we simply skip this step)
	 */
	if (!d->widePalette) {
		for (i = 0,t = temp; i < num; i++) {
			*t++ = *p++ >> 2;
			*t++ = *p++ >> 2;
			*t++ = *p++ >> 2;
			t++; p++;
			}
		p = temp;
		}

	if (isNonVGA) {
#ifndef __16BIT__
		if (setPal20)
			VBE20_blastPalette(p,num,index,d->maxProgram,waitVRT);
		else
#endif
			{
			/* Determine if we need to wait for the vertical retrace */
			if (!waitVRT)
				count = num;
			else
				count = (num > d->maxProgram) ? d->maxProgram : num;
			while (num) {
				VBE_setPalette(index,count,(VBE_palette*)p,waitVRT);
				index += count;
				p += count*4;
				num -= count;
				count = (num > d->maxProgram) ? d->maxProgram : num;
				}
			}
#ifdef	__WINDOWS__
		/* Download the new palette data to our Windows DIB driver */
		if (dc->wm.fulldc.hdc) {
			Escape(dc->wm.fulldc.hdc,MGLDIB_SETPALETTE,sizeof(RGBQUAD)*256,
				(void*)dc->colorTab,NULL);
			}
#endif
		}
	else
		VGA_realizePalette(dc,pal,num,index,waitVRT);
}

void _ASMAPI SVGA_vSync(MGLDC *dc)
/****************************************************************************
*
* Function:		SVGA_vSync
* Parameters:	dc	- MGL Device context
*
* Description:	Syncs to a vertical interrupt. For VBE 2.0 implementations
*				we can simply set the visual page to the same value while
*				waiting for the vertical interrupt, otherwise we use a
*				VGA style sync routine.
*
****************************************************************************/
{
	if (VBE_version >= 0x200)
		MGL_setVisualPage(dc,dc->v->d.visualPage, MGL_waitVRT);
	else
		VGA_vSync(dc);
}

void * _ASMAPI SVGA_getDefaultPalette(MGLDC *dc)
{ dc = dc; return _VGA8_defPal; }

