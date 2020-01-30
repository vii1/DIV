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
* Description:	Set of device driver routines common to all Accelerated
*				display drivers.
*
*               This module loads the pre-configured VBEAF.DRV driver
*				from disk, and intialises the display driver depending on
*				the features that the loaded driver provides.
*
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\common\gaccel.h"

#if	!defined(__16BIT__) || defined(TESTING)

/*------------------------- Global Variables ------------------------------*/

PRIVATE	ibool	detected = -1;			/* True if driver detected		*/
PRIVATE	ibool	useVFlat = false;		/* True if VFlat is used		*/
#if	defined(SAFE_MODE) || defined(__WINDOWS__)
PRIVATE int		saveMode;
PRIVATE	char	saveMemName[_MAX_PATH] = "";
#endif
AF_devCtx   	* _VARAPI _MGL_afPtr;	/* Pointer to loaded driver		*/

/*-------------------------- Implementation -------------------------------*/

PRIVATE void FindDriver(int *driver,char *driverId)
{ 
	if (AF->Attributes & afHaveAccel2D) {
		*driverId = grACCEL;
		if (*driver < grACCEL)
			*driver = grACCEL;
		}
	else {
		*driverId = grSVGA;
		if (*driver < grSVGA)
			*driver = grSVGA;
		}
}

ibool MGLAPI ACCEL_detect(int *driver,char *driverId)
/****************************************************************************
*
* Function:		ACCEL_detect
* Returns:		True if the video card is a supported Accelerated SuperVGA
*
* Description:	Determines if a configured ACCEL.DRV driver is available
*				to be loaded. If the driver is found, we load the driver
*				file into our applications code space.
*
*				NOTE: Also if there is a 3D-DDI driver available, the MGL
*					  rendering code will be using that, not the ACCEL.DRV
*					  file for handling all the acclerated rendering code.
*
****************************************************************************/
{
	if (detected != -1) {
		if (!detected)
        	return false;
		FindDriver(driver,driverId);
		return true;
		}
	MGLPC_init();
#ifdef	MGLWIN
	if (!_MGL_haveWinDirect)
		return detected = false;
#endif

	/* Load and initialise the accelerated driver file */
	if ((AF = AF_loadDriver(".")) == NULL) {
		switch (AF_status()) {
			case afCorruptDriver:
				MGL_fatalError("ACCEL.DRV driver file is corrupted!\n");
				break;
			case afLoadMem:
				MGL_fatalError("Not enough memory to load ACCEL.DRV file!\n");
				break;
			case afOldVersion:
				MGL_fatalError("ACCEL.DRV file is an older version!\n");
				break;
			case afMemMapError:
				MGL_fatalError("Could not map physical memory for ACCEL.DRV!\n");
				break;
			}
		/* Check if the user has registered the WinDirect driver, and if
		 * so use that driver as our accelerated driver code.
		 */
		if (!_MGL_wdDriver)
			return false;
		AF = _MGL_wdDriver;
		}

	/* We only support VBE/AF 2.0 and above */
	if (AF->Version < 0x200)
    	return detected = false;

	/* If we dont have a hardware linear buffer available, then we must
	 * virtualise the framebuffer for the accelerated graphics driver. We
	 * determine here if the VFlat virtual linear framebuffer code is
	 * functional. If VFlat cannot be used, then we wont be able to support
	 * the accelerated rendering code.
	 */
	if (AF->LinearBasePtr)
		detected = true;
	else if (VF_available())
		detected = useVFlat = (AF->SetBank32 != NULL);
	else
    	detected = false;
	if (detected)
		FindDriver(driver,driverId);
	return detected;
}

void MGLAPI ACCEL_useMode(modetab modes,int mode,int id,gmode_t *mi)
/****************************************************************************
*
* Function:		ACCEL_useMode
* Parameters:	modes	- Video mode/device driver association table
*				mode	- Index into mode association table
*				id		- Driver id to store in table
*				mi		- Mode information block to fill in
*
* Description:	Searches through the list of accelerated modes supported
*				by the loaded driver for one that matches the desired
*               resolution and pixel depth.
*
****************************************************************************/
{
	int			i;
	ibool		linear;
	AF_modeInfo	modeInfo;
	AF_int16  	*modeList = AF->AvailableModes;
    ulong		flags;

	for (i = 0; modeList[i] != -1; i++) {
		AF_getVideoModeInfo(AF,modeList[i],&modeInfo);
		if (modeInfo.XResolution == mi->xRes+1 &&
				modeInfo.YResolution == mi->yRes+1 &&
				modeInfo.BitsPerPixel == mi->bitsPerPixel)
			break;
		}
	if (modeList[i] == -1)
		return;
	mi->scratch1 = modeList[i];
	mi->scratch2 = modeInfo.Attributes;
	linear = (modeInfo.Attributes & afHaveLinearBuffer);
    flags = 0;
	if (linear)
		flags |= MGL_HAVE_LINEAR;
	if (AF->Version >= 0x200 && linear)
		modeInfo.BytesPerScanLine = modeInfo.LinBytesPerScanLine;
	mi->bytesPerLine = modeInfo.BytesPerScanLine;

	/* Copy DirectColor mask information to mi block */
	if (modeInfo.BitsPerPixel <= 8) {
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
		if ((AF->Version >= 0x200) && linear) {
			mi->redMaskSize = modeInfo.LinRedMaskSize;
			mi->redFieldPosition = modeInfo.LinRedFieldPosition;
			mi->greenMaskSize = modeInfo.LinGreenMaskSize;
			mi->greenFieldPosition = modeInfo.LinGreenFieldPosition;
			mi->blueMaskSize = modeInfo.LinBlueMaskSize;
			mi->blueFieldPosition = modeInfo.LinBlueFieldPosition;
			mi->rsvdMaskSize = modeInfo.LinRsvdMaskSize;
			mi->rsvdFieldPosition = modeInfo.LinRsvdFieldPosition;
			}
		else {
			mi->redMaskSize = modeInfo.RedMaskSize;
			mi->redFieldPosition = modeInfo.RedFieldPosition;
			mi->greenMaskSize = modeInfo.GreenMaskSize;
			mi->greenFieldPosition = modeInfo.GreenFieldPosition;
			mi->blueMaskSize = modeInfo.BlueMaskSize;
			mi->blueFieldPosition = modeInfo.BlueFieldPosition;
			mi->rsvdMaskSize = modeInfo.RsvdMaskSize;
			mi->rsvdFieldPosition = modeInfo.RsvdFieldPosition;
			}
		}

	if (modeInfo.Attributes & afHaveMultiBuffer) {
		if (AF->Version >= 0x200) {
			if (linear)
				mi->maxPage = modeInfo.LinMaxBuffers-1;
			else
				mi->maxPage = modeInfo.BnkMaxBuffers-1;
			}
		else
			mi->maxPage = modeInfo.MaxBuffers-1;
		}
	else
		mi->maxPage = 0;

	/* Extract hardware flags for the mode */
	if (AF->Version >= 0x200)
		flags |= MGL_HAVE_REFRESH_CTRL;
	if (modeInfo.Attributes & afHaveInterlaced)
		flags |= MGL_HAVE_INTERLACED;
	if (modeInfo.Attributes & afHaveDoubleScan)
		flags |= MGL_HAVE_DOUBLE_SCAN;
	if (modeInfo.Attributes & afHaveTripleBuffer)
		flags |= MGL_HAVE_TRIPLEBUFFER;
	if (modeInfo.Attributes & afHaveStereo)
		flags |= MGL_HAVE_STEREO;
	if (AF->Attributes & afHaveHWStereoSync)
    	flags |= MGL_HAVE_STEREO_HWSYNC;
	if (AF->Attributes & afHaveEVCStereoSync)
		flags |= MGL_HAVE_STEREO_EVCSYNC;
	if (modeInfo.Attributes & afHaveHWCursor)
		flags |= MGL_HAVE_HWCURSOR;
	if (modeInfo.Attributes & afHaveAccel2D)
		flags |= MGL_HAVE_ACCEL_2D;
	if (modeInfo.Attributes & afVideoXInterp)
		flags |= MGL_HAVE_VIDEO_XINTERP;
	if (modeInfo.Attributes & afVideoYInterp)
		flags |= MGL_HAVE_VIDEO_YINTERP;

	/* Check for linear buffer support */
	if (linear)
		mi->scratch1 |= afLinearBuffer;
	else if (!useVFlat)
		return;
	mi->modeFlags = flags;

	DRV_useMode(modes,mode,id,mi->maxPage+1,flags);
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

/****************************************************************************
REMARKS:
This function call the VBE/AF driver to set the graphics mode given the
passed in parameters. If the refresh rate is not set to the adapter default
setting, we attempt to find a suitable refresh rate using the GTF timing
formulas.
****************************************************************************/
ibool AF_setMode(int mode,int virtualX, int virtualY,AF_int32 *bytesPerLine,
	int numBuffers,AF_CRTCInfo *crtc)
{
	*bytesPerLine = -1;
	return AF_setVideoMode(AF,mode,virtualX,virtualY,bytesPerLine,numBuffers,crtc) == 0;
}

PUBLIC void ACCEL_saveVideoMemory(int mode)
/****************************************************************************
*
* Function:     ACCEL_saveVideoMemory
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
	AF_modeInfo	modeInfo;
	ulong		size;
	ibool		fail = false;
	AF_int32 	bpl = -1;
	FILE		*f;
	AF_palette	pal[256];

	if (saveMemName[0] != 0)
		return;

#ifndef	__WINDOWS__
	/* Dont bother to save/restore video memory when running under real DOS,
	 * only under Windows DOS boxes.
	 */
	{
	RMREGS regs;
	regs.x.ax = 0x160A;
	PM_int86(0x2F,&regs,&regs);
	if (regs.x.ax != 0)
		return;
	}
#else
	if (!WD_isSafeMode())
		return;
#endif

	/* Set the 640x480x256 graphics mode to save memory with */
	saveMode = 0x101 | (mode & afLinearBuffer) | afDontClear;
	AF_getVideoModeInfo(AF,saveMode & afModeMask,&modeInfo);
	if (!AF_setMode(saveMode,-1,-1,&bpl,1,NULL))
		return;

	/* Set the palette for the mode to black */
	memset(pal,0,sizeof(pal));
	AF_setPaletteData(AF,pal,256,0,false);

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
	if (saveMode & afLinearBuffer) {
		size = AF->TotalMemory * 1024L;
		if (fwrite(AF->LinearMem,1,size,f) != size)
			fail = true;
		}
	else
#endif
		{
		int		i,maxBlocks = AF->TotalMemory / 64;

		size = 0x8000;
		for (i = 0; i < maxBlocks; i++) {
			AF_setBank(AF,(i >> 1));
			if ((i & 1) == 0) {
				if (fwrite(AF->BankedMem,1,size,f) != size) {
					fail = true;
					break;
					}
				}
			else {
				if (fwrite((uchar*)AF->BankedMem + size,1,size,f) != size) {
					fail = true;
					break;
					}
				}
			}
		}
	fclose(f);
	if (fail) {
		remove(saveMemName);
		saveMemName[0] = 0;
		}
#endif
}

PUBLIC void ACCEL_restoreVideoMemory(void)
/****************************************************************************
*
* Function:     ACCEL_restoreVideoMemory
*
* Description:  Restores the video memory from the memory buffer or
*				disk file after the mode has been reset to correctly
*				restore all cached bitmaps for Windows drivers.
*
****************************************************************************/
{
#if	defined(SAFE_MODE) || defined(__WINDOWS__)
	AF_modeInfo	modeInfo;
	ulong		size;
	FILE		*f;
	AF_int32 	bpl = -1;
	AF_palette	pal[256];

	if (saveMemName[0] == 0)
		return;

	/* Set the 640x480x256 graphics mode to restore memory with */
	AF_getVideoModeInfo(AF,saveMode & afModeMask,&modeInfo);
	if (!AF_setMode(saveMode,-1,-1,&bpl,1,NULL))
		return;

	/* Set the palette for the mode to black */
	memset(pal,0,sizeof(pal));
	AF_setPaletteData(AF,pal,256,0,false);

	/* Open the temporary file to restore the video memory from */
	if ((f = fopen(saveMemName,"rb")) == NULL)
		return;

	/* Restore entire video memory in chunks of 32Kb at a time except under
	 * 32 bit protecetd mode with a linear framebuffer in which case we
	 * can restore it directly as a single block.
	 */
#ifdef	PM386
	if (saveMode & afLinearBuffer) {
		size = AF->TotalMemory * 1024L;
		fread(AF->LinearMem,1,size,f);
		}
	else
#endif
		{
		int		i,maxBlocks = AF->TotalMemory / 64;

		size = 0x8000;
		for (i = 0; i < maxBlocks; i++) {
			AF_setBank(AF,(i >> 1));
			if ((i & 1) == 0)
				fread(AF->BankedMem,1,size,f);
			else
				fread((uchar*)AF->BankedMem + size,1,size,f);
			}
		}
	fclose(f);
	remove(saveMemName);
	saveMemName[0] = 0;
#endif
}

/****************************************************************************
REMARKS:
Call the VBE/AF driver to find the closest pixel clock.
****************************************************************************/
ulong _ASMAPI ACCEL_getClosestClock(ushort mode,ulong pixelClock)
{ return AF_getClosestPixelClock(AF,mode,pixelClock); }

/****************************************************************************
REMARKS:
Enables free running stereo mode. We also re-program the blue color color
index to the correct value in here, so that the developer can enable and
disable support for blue codes at runtime with calls to MGL_setBlueCodeIndex
and MGL_startStereo.
****************************************************************************/
void _ASMAPI ACCEL_startStereo(MGLDC *dc)
{
	if (dc->v->d.isStereo != _MGL_NO_STEREO) {
		if (!(dc->v->d.isStereo & _MGL_HW_STEREO_SYNC) && (_MGL_stereoSyncType == MGL_STEREO_BLUE_CODE)) {
			/* Program the blue code palette entry to pure blue */
			MGL_setPaletteEntry(dc,_MGL_blueCodeIndex,0,0,255);
			MGL_realizePalette(dc,256,0,false);
			}
		if (dc->v->d.isStereo & _MGL_HW_STEREO) {
			/* Enable hardware stereo page flipping */
			AF_enableStereoMode(AF,true);
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
void _ASMAPI ACCEL_stopStereo(MGLDC *dc)
{
	if (dc->v->d.isStereo != _MGL_NO_STEREO && dc->v->d.stereoRunning) {
		if (dc->v->d.isStereo & _MGL_HW_STEREO) {
			/* Disable hardware stereo page flipping */
			AF_enableStereoMode(AF,false);
			}
		if (dc->v->d.isStereo & _MGL_SOFT_STEREO) {
			// TODO: Disable software page flip interrupt handler in here
			}
		dc->v->d.stereoRunning = false;
		}
}

ibool MGLAPI ACCEL_setMode(MGLDC *dc,AF_int16 mode,short *oldBIOSMode,
	ibool *old50Lines,int virtualX,int virtualY,int numBuffers,ibool stereo,
	int refreshRate)
/****************************************************************************
*
* Function:		ACCEL_setMode
* Parameters:	dc			- Device context to initialise
*				mode		- Accelerated video mode to set
*				oldBIOSMode	- Place to store old BIOS mode number
*				old50Lines	- Place to store old 43 line mode flag
*				numBuffers	- Number of buffers to allocate for the mode
* Returns:		TRUE if the mode was successfully set.
*
* Description:	Initialises the video mode by calling the loaded accelerated
*				device driver to initialise the video mode. We also save
*				the state of the old video mode in the specified parameters.
*
****************************************************************************/
{
	RMREGS		regs;
	int			isStereo,v,scrolling;
	AF_int32	bytesPerLine = -1;
    AF_modeInfo	modeInfo;
	AF_CRTCInfo	crtc;

	/* If stereo mode is enabled, check what type of stereo mode we support
	 * and set things up for stereo operation.
	 */
	isStereo = _MGL_NO_STEREO;
	if (stereo) {
		isStereo = _MGL_SOFT_STEREO;
		if (dc->mi.modeFlags & MGL_HAVE_STEREO) {
			/* We have hardware stereo page flipping, so use it */
			isStereo = _MGL_HW_STEREO;
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
		mode |= afStereo;
		}
	// TODO: For now we dont support software stereo page flipping, so we
	//		 fail if this if required
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
		mode |= afMultiBuffer;
		dc->mi.maxPage = numBuffers-1;
		}
	else
		dc->mi.maxPage = 0;

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
	AF_getVideoModeInfo(AF,mode & ~(afLinearBuffer | afStereo),&modeInfo);
	if (refreshRate != MGL_DEFAULT_REFRESH && AF->Version >= 0x200) {
		int maxRefresh = GTF_getMaxRefreshRate(dc,modeInfo.MaxPixelClock,false);
		if (refreshRate > maxRefresh)
			refreshRate = maxRefresh;
		if (refreshRate < 60)
        	refreshRate = 60;
		if (GTF_computeCRTCTimings(dc,mode,refreshRate,false,&crtc,ACCEL_getClosestClock))
			mode |= afRefreshCtrl;
		}

	/* Set up for the color display to be active, and set the mode */
	v = PM_getByte(_MGL_biosSel,0x10);
	PM_setByte(_MGL_biosSel,0x10, (v & 0xCF) | 0x20);
	PM_setByte(_MGL_biosSel,0x49,0x10);
	ACCEL_saveVideoMemory(mode & ~afRefreshCtrl);
	if (scrolling) {
		if (!AF_setMode(mode,virtualX,virtualY,&bytesPerLine,numBuffers,&crtc))
			return false;
		dc->mi.xRes = virtualX - 1;
		dc->mi.yRes = virtualY - 1;
		dc->startX = dc->startY = 0;
		dc->mi.bytesPerLine = bytesPerLine;
		}
	else {
		if (!AF_setMode(mode,-1,-1,&bytesPerLine,numBuffers,&crtc))
			return false;
		}
	dc->virtualX = virtualX;
	dc->virtualY = virtualY;
	dc->numBuffers = numBuffers;

	/* Obtain pointer to device surface */
	if (mode & afLinearBuffer) {
		dc->surface = dc->surfaceStart = AF->LinearMem;
		dc->flags |= MGL_LINEAR_ACCESS;
		}
	else {
		/* See if we can use the VFlat virtual linear framebuffer */
		void *bankFunc,*p;
        int codeLen;
		AF_getBankFunc32(AF,&codeLen,&bankFunc);
        VF_exit();
		p = VF_init(AF->BankedBasePtr,AF->BankSize,codeLen,bankFunc);
		if (!p)
			MGL_fatalError("Unable to initialise virtual linear framebuffer!");
		dc->flags &= ~MGL_LINEAR_ACCESS;
		dc->flags |= MGL_VIRTUAL_ACCESS;
		dc->surfaceStart = dc->surface = p;
		}

	/* Use hardware cursor if available for all modes */
	if ((dc->mi.scratch2 & afHaveHWCursor) && AF->cFuncs.SetCursor) {
		dc->v->d.hardwareCursor 	= true;
		dc->v->d.setCursorColor 	= ACCEL_setCursorColor;
		dc->v->d.setCursor 			= ACCEL_setCursor;
		dc->v->d.setCursorPos 		= ACCEL_setCursorPos;
		dc->v->d.showCursor 		= ACCEL_showCursor;
		}

	/* Set the wide palette flag for the DC */
	if (dc->mi.scratch2 & afHave8BitDAC)
		dc->v->d.widePalette = true;
	else
		dc->v->d.widePalette = false;

	/* Use ACCEL versions of page flipping and palette programming code */
	dc->v->d.isStereo				= isStereo;
	dc->v->d.refreshRate			= refreshRate;
	dc->v->d.stereoRunning			= false;
	dc->v->d.startStereo			= ACCEL_startStereo;
	dc->v->d.stopStereo				= ACCEL_stopStereo;
	dc->v->d.setActivePage 			= ACCEL_setActivePage;
	dc->v->d.setVisualPage 			= ACCEL_setVisualPage;
	dc->v->d.setDisplayStart		= ACCEL_setDisplayStart;
	dc->r.realizePalette 			= ACCEL_realizePalette;
	dc->v->d.vSync 					= ACCEL_vSync;

	if ((dc->mi.scratch2 & afHaveAccel2D) && !dc->v->d.inited) {
    	/* Mark that we have initied the driver function variables */
    	dc->v->d.inited = true;
#ifndef	MGL_LITE
		dc->v->d.makeOffscreenDC	= ACCEL_makeOffscreenDC;
		if (AF->cFuncs.BitBltLin)
			dc->v->d.makeLinearOffscreenDC = ACCEL_makeLinearOffscreenDC;
		dc->r.setWriteMode			= ACCEL_setWriteMode;
		dc->r.setPenStyle			= ACCEL_setPenStyle;
		dc->r.setPenBitmapPattern	= ACCEL_setPenBitmapPattern;
		dc->r.setPenPixmapPattern	= ACCEL_setPenPixmapPattern;
#endif
		/* First vector all rendering routines to our stub functions that
		 * correctly arbitrate between direct memory access and accelerator
		 * access. Note that we save a complete copy of the packed pixel
		 * render vectors active before we override them with our stub
		 * functions (so we can call the original routines directly).
		 */
		_PACKED_vecs = dc->r;
		if (AF->cFuncs.EnableDirectAccess) {
			dc->r.beginDirectAccess 	= ACCEL_beginDirectAccessSLOW;
			dc->r.endDirectAccess 		= ACCEL_endDirectAccessSLOW;
			dc->r.beginPixel 			= ACCEL_beginPixelSLOW;
			dc->r.endPixel 				= ACCEL_endPixelSLOW;

			dc->r.getPixel				= ACSTUB_getPixelSLOW;
			dc->r.putPixel				= ACSTUB_putPixelSLOW;
			dc->r.putMonoImage			= ACSTUB_putMonoImageSLOW;
			dc->r.putMouseImage			= ACSTUB_putMouseImageSLOW;
			dc->r.getImage				= ACSTUB_getImageSLOW;
			dc->r.putImage				= ACSTUB_putImageSLOW;
			dc->r.getDivot 				= ACSTUB_getDivotSLOW;
			dc->r.putDivot 				= ACSTUB_putDivotSLOW;
			dc->r.stretchBlt1x2			= ACSTUB_stretchBlt1x2SLOW;
			dc->r.stretchBlt2x2			= ACSTUB_stretchBlt2x2SLOW;
			dc->r.stretchBlt			= ACSTUB_stretchBltSLOW;

			dc->r.solid.line			= ACSTUB_solidLineSLOW;
			dc->r.ropSolid.line			= ACSTUB_ropSolidLineSLOW;
			dc->r.patt.line				= ACSTUB_pattLineSLOW;
			dc->r.colorPatt.line		= ACSTUB_colorPattLineSLOW;
			dc->r.fatSolid.line			= ACSTUB_fatSolidLineSLOW;
			dc->r.fatRopSolid.line		= ACSTUB_fatRopSolidLineSLOW;
			dc->r.fatPatt.line			= ACSTUB_fatPattLineSLOW;
			dc->r.fatColorPatt.line		= ACSTUB_fatColorPattLineSLOW;
			dc->r.dither.line			= ACSTUB_ditherLineSLOW;

			dc->r.solid.scanLine		= ACSTUB_solidScanLineSLOW;
			dc->r.ropSolid.scanLine		= ACSTUB_ropSolidScanLineSLOW;
			dc->r.patt.scanLine			= ACSTUB_pattScanLineSLOW;
			dc->r.colorPatt.scanLine	= ACSTUB_colorPattScanLineSLOW;
			dc->r.fatSolid.scanLine		= ACSTUB_fatSolidScanLineSLOW;
			dc->r.fatRopSolid.scanLine	= ACSTUB_fatRopSolidScanLineSLOW;
			dc->r.fatPatt.scanLine		= ACSTUB_fatPattScanLineSLOW;
			dc->r.fatColorPatt.scanLine	= ACSTUB_fatColorPattScanLineSLOW;
			dc->r.dither.scanLine		= ACSTUB_ditherScanLineSLOW;

			dc->r.solid.fillRect		= ACSTUB_solidFillRectSLOW;
			dc->r.ropSolid.fillRect		= ACSTUB_ropSolidFillRectSLOW;
			dc->r.patt.fillRect			= ACSTUB_pattFillRectSLOW;
			dc->r.colorPatt.fillRect	= ACSTUB_colorPattFillRectSLOW;
			dc->r.fatSolid.fillRect		= ACSTUB_fatSolidFillRectSLOW;
			dc->r.fatRopSolid.fillRect	= ACSTUB_fatRopSolidFillRectSLOW;
			dc->r.fatPatt.fillRect		= ACSTUB_fatPattFillRectSLOW;
			dc->r.fatColorPatt.fillRect	= ACSTUB_fatColorPattFillRectSLOW;
			dc->r.dither.fillRect		= ACSTUB_ditherFillRectSLOW;

			dc->r.solid.drawScanList		= ACSTUB_solidDrawScanListSLOW;
			dc->r.ropSolid.drawScanList		= ACSTUB_ropSolidDrawScanListSLOW;
			dc->r.patt.drawScanList			= ACSTUB_pattDrawScanListSLOW;
			dc->r.colorPatt.drawScanList	= ACSTUB_colorPattDrawScanListSLOW;
			dc->r.fatSolid.drawScanList		= ACSTUB_fatSolidDrawScanListSLOW;
			dc->r.fatRopSolid.drawScanList	= ACSTUB_fatRopSolidDrawScanListSLOW;
			dc->r.fatPatt.drawScanList		= ACSTUB_fatPattDrawScanListSLOW;
			dc->r.fatColorPatt.drawScanList	= ACSTUB_fatColorPattDrawScanListSLOW;
			dc->r.dither.drawScanList		= ACSTUB_ditherDrawScanListSLOW;

			dc->r.solid.trap			= ACSTUB_solidTrapSLOW;
			dc->r.ropSolid.trap			= ACSTUB_ropSolidTrapSLOW;
			dc->r.patt.trap				= ACSTUB_pattTrapSLOW;
			dc->r.colorPatt.trap		= ACSTUB_colorPattTrapSLOW;
			dc->r.fatSolid.trap			= ACSTUB_fatSolidTrapSLOW;
			dc->r.fatRopSolid.trap		= ACSTUB_fatRopSolidTrapSLOW;
			dc->r.fatPatt.trap			= ACSTUB_fatPattTrapSLOW;
			dc->r.fatColorPatt.trap		= ACSTUB_fatColorPattTrapSLOW;
			dc->r.dither.trap			= ACSTUB_ditherTrapSLOW;

			dc->r.solid.drawRegion			= ACSTUB_solidDrawRegionSLOW;
			dc->r.ropSolid.drawRegion		= ACSTUB_ropSolidDrawRegionSLOW;
			dc->r.patt.drawRegion			= ACSTUB_pattDrawRegionSLOW;
			dc->r.colorPatt.drawRegion		= ACSTUB_colorPattDrawRegionSLOW;
			dc->r.fatSolid.drawRegion		= ACSTUB_fatSolidDrawRegionSLOW;
			dc->r.fatRopSolid.drawRegion	= ACSTUB_fatRopSolidDrawRegionSLOW;
			dc->r.fatPatt.drawRegion		= ACSTUB_fatPattDrawRegionSLOW;
			dc->r.fatColorPatt.drawRegion	= ACSTUB_fatColorPattDrawRegionSLOW;
			dc->r.dither.drawRegion			= ACSTUB_ditherDrawRegionSLOW;

			dc->r.solid.ellipse			= ACSTUB_solidEllipseSLOW;
			dc->r.ropSolid.ellipse		= ACSTUB_ropSolidEllipseSLOW;
			dc->r.patt.ellipse			= ACSTUB_pattEllipseSLOW;
			dc->r.colorPatt.ellipse		= ACSTUB_colorPattEllipseSLOW;
			dc->r.fatSolid.ellipse		= ACSTUB_fatSolidEllipseSLOW;
			dc->r.fatRopSolid.ellipse	= ACSTUB_fatRopSolidEllipseSLOW;
			dc->r.fatPatt.ellipse		= ACSTUB_fatPattEllipseSLOW;
			dc->r.fatColorPatt.ellipse	= ACSTUB_fatColorPattEllipseSLOW;
			dc->r.dither.ellipse		= ACSTUB_ditherEllipseSLOW;

			dc->r.solid.fillEllipse			= ACSTUB_solidFillEllipseSLOW;
			dc->r.ropSolid.fillEllipse		= ACSTUB_ropSolidFillEllipseSLOW;
			dc->r.patt.fillEllipse			= ACSTUB_pattFillEllipseSLOW;
			dc->r.colorPatt.fillEllipse		= ACSTUB_colorPattFillEllipseSLOW;
			dc->r.fatSolid.fillEllipse		= ACSTUB_fatSolidFillEllipseSLOW;
			dc->r.fatRopSolid.fillEllipse	= ACSTUB_fatRopSolidFillEllipseSLOW;
			dc->r.fatPatt.fillEllipse		= ACSTUB_fatPattFillEllipseSLOW;
			dc->r.fatColorPatt.fillEllipse	= ACSTUB_fatColorPattFillEllipseSLOW;
			dc->r.dither.fillEllipse		= ACSTUB_ditherFillEllipseSLOW;

			dc->r.solid.ellipseArc			= ACSTUB_solidEllipseArcSLOW;
			dc->r.ropSolid.ellipseArc		= ACSTUB_ropSolidEllipseArcSLOW;
			dc->r.patt.ellipseArc			= ACSTUB_pattEllipseArcSLOW;
			dc->r.colorPatt.ellipseArc		= ACSTUB_colorPattEllipseArcSLOW;
			dc->r.fatSolid.ellipseArc		= ACSTUB_fatSolidEllipseArcSLOW;
			dc->r.fatRopSolid.ellipseArc	= ACSTUB_fatRopSolidEllipseArcSLOW;
			dc->r.fatPatt.ellipseArc		= ACSTUB_fatPattEllipseArcSLOW;
			dc->r.fatColorPatt.ellipseArc	= ACSTUB_fatColorPattEllipseArcSLOW;
			dc->r.dither.ellipseArc			= ACSTUB_ditherEllipseArcSLOW;

			dc->r.solid.fillEllipseArc			= ACSTUB_solidFillEllipseArcSLOW;
			dc->r.ropSolid.fillEllipseArc		= ACSTUB_ropSolidFillEllipseArcSLOW;
			dc->r.patt.fillEllipseArc			= ACSTUB_pattFillEllipseArcSLOW;
			dc->r.colorPatt.fillEllipseArc		= ACSTUB_colorPattFillEllipseArcSLOW;
			dc->r.fatSolid.fillEllipseArc		= ACSTUB_fatSolidFillEllipseArcSLOW;
			dc->r.fatRopSolid.fillEllipseArc	= ACSTUB_fatRopSolidFillEllipseArcSLOW;
			dc->r.fatPatt.fillEllipseArc		= ACSTUB_fatPattFillEllipseArcSLOW;
			dc->r.fatColorPatt.fillEllipseArc	= ACSTUB_fatColorPattFillEllipseArcSLOW;
			dc->r.dither.fillEllipseArc			= ACSTUB_ditherFillEllipseArcSLOW;

			dc->r.stippleLine			= ACSTUB_stippleLineSLOW;
			dc->r.drawStrBitmap			= ACSTUB_drawStrBitmapSLOW;
			dc->r.drawCharVec			= ACSTUB_drawCharVecSLOW;
			dc->r.complexPolygon		= ACSTUB_complexPolygonSLOW;
			dc->r.polygon				= ACSTUB_polygonSLOW;
			dc->r.ditherPolygon			= ACSTUB_ditherPolygonSLOW;
			dc->r.translateImage		= ACSTUB_translateImageSLOW;
			dc->r.bitBlt				= ACSTUB_bitBltSLOW;
			dc->r.srcTransBlt			= ACSTUB_srcTransBltSLOW;
			dc->r.dstTransBlt			= ACSTUB_dstTransBltSLOW;
			dc->r.scanRightForColor		= ACSTUB_scanRightForColorSLOW;
			dc->r.scanLeftForColor		= ACSTUB_scanLeftForColorSLOW;
			dc->r.scanRightWhileColor	= ACSTUB_scanRightWhileColorSLOW;
			dc->r.scanLeftWhileColor	= ACSTUB_scanLeftWhileColorSLOW;

			dc->r.cLine					= ACSTUB_cLineSLOW;
			dc->r.rgbLine               = ACSTUB_rgbLineSLOW;
			dc->r.tri                   = ACSTUB_triSLOW;
			dc->r.ditherTri				= ACSTUB_ditherTriSLOW;
			dc->r.cTri                  = ACSTUB_cTriSLOW;
			dc->r.rgbTri                = ACSTUB_rgbTriSLOW;
			dc->r.quad                  = ACSTUB_quadSLOW;
			dc->r.ditherQuad			= ACSTUB_ditherQuadSLOW;
			dc->r.cQuad                 = ACSTUB_cQuadSLOW;
			dc->r.rgbQuad               = ACSTUB_rgbQuadSLOW;
			dc->r.cTrap                 = ACSTUB_cTrapSLOW;
			dc->r.rgbTrap               = ACSTUB_rgbTrapSLOW;

			dc->r.z16.zLine				= ACSTUB_z16_zLineSLOW;
			dc->r.z16.zDitherLine		= ACSTUB_z16_zDitherLineSLOW;
			dc->r.z16.czLine            = ACSTUB_z16_czLineSLOW;
			dc->r.z16.rgbzLine          = ACSTUB_z16_rgbzLineSLOW;
			dc->r.z16.zTri              = ACSTUB_z16_zTriSLOW;
			dc->r.z16.zDitherTri        = ACSTUB_z16_zDitherTriSLOW;
			dc->r.z16.czTri             = ACSTUB_z16_czTriSLOW;
			dc->r.z16.rgbzTri           = ACSTUB_z16_rgbzTriSLOW;
			dc->r.z16.zQuad             = ACSTUB_z16_zQuadSLOW;
			dc->r.z16.zDitherQuad       = ACSTUB_z16_zDitherQuadSLOW;
			dc->r.z16.czQuad            = ACSTUB_z16_czQuadSLOW;
			dc->r.z16.rgbzQuad          = ACSTUB_z16_rgbzQuadSLOW;
			dc->r.z16.zTrap             = ACSTUB_z16_zTrapSLOW;
			dc->r.z16.zDitherTrap       = ACSTUB_z16_zDitherTrapSLOW;
			dc->r.z16.czTrap            = ACSTUB_z16_czTrapSLOW;
			dc->r.z16.rgbzTrap          = ACSTUB_z16_rgbzTrapSLOW;

			dc->r.z32.zLine				= ACSTUB_z32_zLineSLOW;
			dc->r.z32.zDitherLine		= ACSTUB_z32_zDitherLineSLOW;
			dc->r.z32.czLine            = ACSTUB_z32_czLineSLOW;
			dc->r.z32.rgbzLine          = ACSTUB_z32_rgbzLineSLOW;
			dc->r.z32.zTri              = ACSTUB_z32_zTriSLOW;
			dc->r.z32.zDitherTri        = ACSTUB_z32_zDitherTriSLOW;
			dc->r.z32.czTri             = ACSTUB_z32_czTriSLOW;
			dc->r.z32.rgbzTri           = ACSTUB_z32_rgbzTriSLOW;
			dc->r.z32.zQuad             = ACSTUB_z32_zQuadSLOW;
			dc->r.z32.zDitherQuad       = ACSTUB_z32_zDitherQuadSLOW;
			dc->r.z32.czQuad            = ACSTUB_z32_czQuadSLOW;
			dc->r.z32.rgbzQuad          = ACSTUB_z32_rgbzQuadSLOW;
			dc->r.z32.zTrap             = ACSTUB_z32_zTrapSLOW;
			dc->r.z32.zDitherTrap       = ACSTUB_z32_zDitherTrapSLOW;
			dc->r.z32.czTrap            = ACSTUB_z32_czTrapSLOW;
			dc->r.z32.rgbzTrap          = ACSTUB_z32_rgbzTrapSLOW;
			}
		else {
			dc->r.beginDirectAccess 	= ACCEL_beginDirectAccess;
			dc->r.endDirectAccess 		= ACCEL_endDirectAccess;
			dc->r.beginPixel 			= ACCEL_beginPixel;
			dc->r.endPixel 				= ACCEL_endPixel;

			dc->r.getPixel				= ACSTUB_getPixel;
			dc->r.putPixel				= ACSTUB_putPixel;
			dc->r.putMonoImage			= ACSTUB_putMonoImage;
			dc->r.putMouseImage			= ACSTUB_putMouseImage;
			dc->r.getImage				= ACSTUB_getImage;
			dc->r.putImage				= ACSTUB_putImage;
			dc->r.getDivot 				= ACSTUB_getDivot;
			dc->r.putDivot 				= ACSTUB_putDivot;
			dc->r.stretchBlt1x2			= ACSTUB_stretchBlt1x2;
			dc->r.stretchBlt2x2			= ACSTUB_stretchBlt2x2;
			dc->r.stretchBlt			= ACSTUB_stretchBlt;

			dc->r.solid.line			= ACSTUB_solidLine;
			dc->r.ropSolid.line			= ACSTUB_ropSolidLine;
			dc->r.patt.line				= ACSTUB_pattLine;
			dc->r.colorPatt.line		= ACSTUB_colorPattLine;
			dc->r.fatSolid.line			= ACSTUB_fatSolidLine;
			dc->r.fatRopSolid.line		= ACSTUB_fatRopSolidLine;
			dc->r.fatPatt.line			= ACSTUB_fatPattLine;
			dc->r.fatColorPatt.line		= ACSTUB_fatColorPattLine;
			dc->r.dither.line			= ACSTUB_ditherLine;

			dc->r.solid.scanLine		= ACSTUB_solidScanLine;
			dc->r.ropSolid.scanLine		= ACSTUB_ropSolidScanLine;
			dc->r.patt.scanLine			= ACSTUB_pattScanLine;
			dc->r.colorPatt.scanLine	= ACSTUB_colorPattScanLine;
			dc->r.fatSolid.scanLine		= ACSTUB_fatSolidScanLine;
			dc->r.fatRopSolid.scanLine	= ACSTUB_fatRopSolidScanLine;
			dc->r.fatPatt.scanLine		= ACSTUB_fatPattScanLine;
			dc->r.fatColorPatt.scanLine	= ACSTUB_fatColorPattScanLine;
			dc->r.dither.scanLine		= ACSTUB_ditherScanLine;

			dc->r.solid.fillRect		= ACSTUB_solidFillRect;
			dc->r.ropSolid.fillRect		= ACSTUB_ropSolidFillRect;
			dc->r.patt.fillRect			= ACSTUB_pattFillRect;
			dc->r.colorPatt.fillRect	= ACSTUB_colorPattFillRect;
			dc->r.fatSolid.fillRect		= ACSTUB_fatSolidFillRect;
			dc->r.fatRopSolid.fillRect	= ACSTUB_fatRopSolidFillRect;
			dc->r.fatPatt.fillRect		= ACSTUB_fatPattFillRect;
			dc->r.fatColorPatt.fillRect	= ACSTUB_fatColorPattFillRect;
			dc->r.dither.fillRect		= ACSTUB_ditherFillRect;

			dc->r.solid.drawScanList		= ACSTUB_solidDrawScanList;
			dc->r.ropSolid.drawScanList		= ACSTUB_ropSolidDrawScanList;
			dc->r.patt.drawScanList			= ACSTUB_pattDrawScanList;
			dc->r.colorPatt.drawScanList	= ACSTUB_colorPattDrawScanList;
			dc->r.fatSolid.drawScanList		= ACSTUB_fatSolidDrawScanList;
			dc->r.fatRopSolid.drawScanList	= ACSTUB_fatRopSolidDrawScanList;
			dc->r.fatPatt.drawScanList		= ACSTUB_fatPattDrawScanList;
			dc->r.fatColorPatt.drawScanList	= ACSTUB_fatColorPattDrawScanList;
			dc->r.dither.drawScanList		= ACSTUB_ditherDrawScanList;

			dc->r.solid.trap			= ACSTUB_solidTrap;
			dc->r.ropSolid.trap			= ACSTUB_ropSolidTrap;
			dc->r.patt.trap				= ACSTUB_pattTrap;
			dc->r.colorPatt.trap		= ACSTUB_colorPattTrap;
			dc->r.fatSolid.trap			= ACSTUB_fatSolidTrap;
			dc->r.fatRopSolid.trap		= ACSTUB_fatRopSolidTrap;
			dc->r.fatPatt.trap			= ACSTUB_fatPattTrap;
			dc->r.fatColorPatt.trap		= ACSTUB_fatColorPattTrap;
			dc->r.dither.trap			= ACSTUB_ditherTrap;

			dc->r.solid.drawRegion			= ACSTUB_solidDrawRegion;
			dc->r.ropSolid.drawRegion		= ACSTUB_ropSolidDrawRegion;
			dc->r.patt.drawRegion			= ACSTUB_pattDrawRegion;
			dc->r.colorPatt.drawRegion		= ACSTUB_colorPattDrawRegion;
			dc->r.fatSolid.drawRegion		= ACSTUB_fatSolidDrawRegion;
			dc->r.fatRopSolid.drawRegion	= ACSTUB_fatRopSolidDrawRegion;
			dc->r.fatPatt.drawRegion		= ACSTUB_fatPattDrawRegion;
			dc->r.fatColorPatt.drawRegion	= ACSTUB_fatColorPattDrawRegion;
			dc->r.dither.drawRegion			= ACSTUB_ditherDrawRegion;

			dc->r.solid.ellipse			= ACSTUB_solidEllipse;
			dc->r.ropSolid.ellipse		= ACSTUB_ropSolidEllipse;
			dc->r.patt.ellipse			= ACSTUB_pattEllipse;
			dc->r.colorPatt.ellipse		= ACSTUB_colorPattEllipse;
			dc->r.fatSolid.ellipse		= ACSTUB_fatSolidEllipse;
			dc->r.fatRopSolid.ellipse	= ACSTUB_fatRopSolidEllipse;
			dc->r.fatPatt.ellipse		= ACSTUB_fatPattEllipse;
			dc->r.fatColorPatt.ellipse	= ACSTUB_fatColorPattEllipse;
			dc->r.dither.ellipse		= ACSTUB_ditherEllipse;

			dc->r.solid.fillEllipse			= ACSTUB_solidFillEllipse;
			dc->r.ropSolid.fillEllipse		= ACSTUB_ropSolidFillEllipse;
			dc->r.patt.fillEllipse			= ACSTUB_pattFillEllipse;
			dc->r.colorPatt.fillEllipse		= ACSTUB_colorPattFillEllipse;
			dc->r.fatSolid.fillEllipse		= ACSTUB_fatSolidFillEllipse;
			dc->r.fatRopSolid.fillEllipse	= ACSTUB_fatRopSolidFillEllipse;
			dc->r.fatPatt.fillEllipse		= ACSTUB_fatPattFillEllipse;
			dc->r.fatColorPatt.fillEllipse	= ACSTUB_fatColorPattFillEllipse;
			dc->r.dither.fillEllipse		= ACSTUB_ditherFillEllipse;

			dc->r.solid.ellipseArc			= ACSTUB_solidEllipseArc;
			dc->r.ropSolid.ellipseArc		= ACSTUB_ropSolidEllipseArc;
			dc->r.patt.ellipseArc			= ACSTUB_pattEllipseArc;
			dc->r.colorPatt.ellipseArc		= ACSTUB_colorPattEllipseArc;
			dc->r.fatSolid.ellipseArc		= ACSTUB_fatSolidEllipseArc;
			dc->r.fatRopSolid.ellipseArc	= ACSTUB_fatRopSolidEllipseArc;
			dc->r.fatPatt.ellipseArc		= ACSTUB_fatPattEllipseArc;
			dc->r.fatColorPatt.ellipseArc	= ACSTUB_fatColorPattEllipseArc;
			dc->r.dither.ellipseArc			= ACSTUB_ditherEllipseArc;

			dc->r.solid.fillEllipseArc			= ACSTUB_solidFillEllipseArc;
			dc->r.ropSolid.fillEllipseArc		= ACSTUB_ropSolidFillEllipseArc;
			dc->r.patt.fillEllipseArc			= ACSTUB_pattFillEllipseArc;
			dc->r.colorPatt.fillEllipseArc		= ACSTUB_colorPattFillEllipseArc;
			dc->r.fatSolid.fillEllipseArc		= ACSTUB_fatSolidFillEllipseArc;
			dc->r.fatRopSolid.fillEllipseArc	= ACSTUB_fatRopSolidFillEllipseArc;
			dc->r.fatPatt.fillEllipseArc		= ACSTUB_fatPattFillEllipseArc;
			dc->r.fatColorPatt.fillEllipseArc	= ACSTUB_fatColorPattFillEllipseArc;
			dc->r.dither.fillEllipseArc			= ACSTUB_ditherFillEllipseArc;

			dc->r.stippleLine			= ACSTUB_stippleLine;
			dc->r.drawStrBitmap			= ACSTUB_drawStrBitmap;
			dc->r.drawCharVec			= ACSTUB_drawCharVec;
			dc->r.complexPolygon		= ACSTUB_complexPolygon;
			dc->r.polygon				= ACSTUB_polygon;
			dc->r.ditherPolygon			= ACSTUB_ditherPolygon;
			dc->r.translateImage		= ACSTUB_translateImage;
			dc->r.bitBlt				= ACSTUB_bitBlt;
			dc->r.srcTransBlt			= ACSTUB_srcTransBlt;
			dc->r.dstTransBlt			= ACSTUB_dstTransBlt;
			dc->r.scanRightForColor		= ACSTUB_scanRightForColor;
			dc->r.scanLeftForColor		= ACSTUB_scanLeftForColor;
			dc->r.scanRightWhileColor	= ACSTUB_scanRightWhileColor;
			dc->r.scanLeftWhileColor	= ACSTUB_scanLeftWhileColor;

			dc->r.cLine					= ACSTUB_cLine;
			dc->r.rgbLine               = ACSTUB_rgbLine;
			dc->r.tri                   = ACSTUB_tri;
			dc->r.ditherTri				= ACSTUB_ditherTri;
			dc->r.cTri                  = ACSTUB_cTri;
			dc->r.rgbTri                = ACSTUB_rgbTri;
			dc->r.quad                  = ACSTUB_quad;
			dc->r.ditherQuad			= ACSTUB_ditherQuad;
			dc->r.cQuad                 = ACSTUB_cQuad;
			dc->r.rgbQuad               = ACSTUB_rgbQuad;
			dc->r.cTrap                 = ACSTUB_cTrap;
			dc->r.rgbTrap               = ACSTUB_rgbTrap;

			dc->r.z16.zLine				= ACSTUB_z16_zLine;
			dc->r.z16.zDitherLine		= ACSTUB_z16_zDitherLine;
			dc->r.z16.czLine            = ACSTUB_z16_czLine;
			dc->r.z16.rgbzLine          = ACSTUB_z16_rgbzLine;
			dc->r.z16.zTri              = ACSTUB_z16_zTri;
			dc->r.z16.zDitherTri        = ACSTUB_z16_zDitherTri;
			dc->r.z16.czTri             = ACSTUB_z16_czTri;
			dc->r.z16.rgbzTri           = ACSTUB_z16_rgbzTri;
			dc->r.z16.zQuad             = ACSTUB_z16_zQuad;
			dc->r.z16.zDitherQuad       = ACSTUB_z16_zDitherQuad;
			dc->r.z16.czQuad            = ACSTUB_z16_czQuad;
			dc->r.z16.rgbzQuad          = ACSTUB_z16_rgbzQuad;
			dc->r.z16.zTrap             = ACSTUB_z16_zTrap;
			dc->r.z16.zDitherTrap       = ACSTUB_z16_zDitherTrap;
			dc->r.z16.czTrap            = ACSTUB_z16_czTrap;
			dc->r.z16.rgbzTrap          = ACSTUB_z16_rgbzTrap;

			dc->r.z32.zLine				= ACSTUB_z32_zLine;
			dc->r.z32.zDitherLine		= ACSTUB_z32_zDitherLine;
			dc->r.z32.czLine            = ACSTUB_z32_czLine;
			dc->r.z32.rgbzLine          = ACSTUB_z32_rgbzLine;
			dc->r.z32.zTri              = ACSTUB_z32_zTri;
			dc->r.z32.zDitherTri        = ACSTUB_z32_zDitherTri;
			dc->r.z32.czTri             = ACSTUB_z32_czTri;
			dc->r.z32.rgbzTri           = ACSTUB_z32_rgbzTri;
			dc->r.z32.zQuad             = ACSTUB_z32_zQuad;
			dc->r.z32.zDitherQuad       = ACSTUB_z32_zDitherQuad;
			dc->r.z32.czQuad            = ACSTUB_z32_czQuad;
			dc->r.z32.rgbzQuad          = ACSTUB_z32_rgbzQuad;
			dc->r.z32.zTrap             = ACSTUB_z32_zTrap;
			dc->r.z32.zDitherTrap       = ACSTUB_z32_zDitherTrap;
			dc->r.z32.czTrap            = ACSTUB_z32_czTrap;
			dc->r.z32.rgbzTrap          = ACSTUB_z32_rgbzTrap;
			}
		/* Now override the appropriate rendering functions to use the
		 * accelerated versions depending on what the loaded device driver
		 * provides. Note that we also replace those stubbed functions with
		 * the original packed pixel functions or emulation functions where
		 * the packed pixel versions are built on functions that call
		 * accelerated functions.
		 */
		if (AF->cFuncs.SetClipRect) {
			dc->flags |= MGL_HW_CLIP;
			dc->r.setClipRect			= ACCEL_setClipRect;
			}
		if (AF->cFuncs.DrawScan) {
			dc->r.solid.scanLine		= ACCEL_scanLine;
			dc->r.ropSolid.scanLine		= ACCEL_scanLine;
			dc->r.fatSolid.scanLine		= ACCEL_scanLine;
			dc->r.fatRopSolid.scanLine	= ACCEL_scanLine;
#ifndef	MGL_LITE
			/* The following are emulated via accelerated scanlines */
			dc->r.fatSolid.line				= __EMU__fatPenLine;
			dc->r.fatRopSolid.line			= __EMU__fatPenLine;
			dc->r.solid.fillRect			= __EMU__fillRect;
			dc->r.ropSolid.fillRect			= __EMU__fillRect;
			dc->r.fatSolid.fillRect			= __EMU__fillRect;
			dc->r.fatRopSolid.fillRect		= __EMU__fillRect;
			dc->r.solid.drawScanList		= __EMU__drawScanList;
			dc->r.ropSolid.drawScanList		= __EMU__drawScanList;
			dc->r.fatSolid.drawScanList		= __EMU__drawScanList;
			dc->r.fatRopSolid.drawScanList	= __EMU__drawScanList;
			dc->r.solid.trap				= __EMU__trap;
			dc->r.ropSolid.trap				= __EMU__trap;
			dc->r.fatSolid.trap				= __EMU__trap;
			dc->r.fatRopSolid.trap			= __EMU__trap;
			dc->r.solid.drawRegion			= __EMU__drawRegion;
			dc->r.ropSolid.drawRegion		= __EMU__drawRegion;
			dc->r.fatSolid.drawRegion		= __EMU__drawRegion;
			dc->r.fatRopSolid.drawRegion	= __EMU__drawRegion;
			dc->r.fatSolid.ellipse			= __EMU__fatPenEllipse;
			dc->r.fatRopSolid.ellipse		= __EMU__fatPenEllipse;
			dc->r.solid.fillEllipse			= __EMU__fillEllipse;
			dc->r.ropSolid.fillEllipse		= __EMU__fillEllipse;
			dc->r.fatSolid.fillEllipse		= __EMU__fillEllipse;
			dc->r.fatRopSolid.fillEllipse	= __EMU__fillEllipse;
			dc->r.fatSolid.ellipseArc		= __EMU__fatPenEllipseArc;
			dc->r.fatRopSolid.ellipseArc	= __EMU__fatPenEllipseArc;
			dc->r.solid.fillEllipseArc		= __EMU__fillEllipseArc;
			dc->r.ropSolid.fillEllipseArc	= __EMU__fillEllipseArc;
			dc->r.fatSolid.fillEllipseArc	= __EMU__fillEllipseArc;
			dc->r.fatRopSolid.fillEllipseArc= __EMU__fillEllipseArc;
			dc->r.complexPolygon 			= __EMU__complexPolygon;
			dc->r.polygon 					= __EMU__polygon;
#endif
			}
#if 1
		if (AF->cFuncs.DrawRect) {
			dc->flags |= MGL_HW_RECT;
			dc->r.solid.fillRect		= ACCEL_fillRect;
			dc->r.ropSolid.fillRect		= ACCEL_fillRect;
			dc->r.fatSolid.fillRect		= ACCEL_fillRect;
			dc->r.fatRopSolid.fillRect	= ACCEL_fillRect;
			}
		if (AF->cFuncs.DrawLine) {
			dc->flags |= MGL_HW_LINE;
			dc->r.solid.line			= ACCEL_line;
			dc->r.ropSolid.line			= ACCEL_line;
#ifndef	MGL_LITE
			/* The following is emulated using accelerated lines */
			dc->r.drawCharVec			= __EMU__drawCharVec;
#endif
			}
		if (AF->PutMonoImage || AF->cFuncs.PutMonoImage) {
			dc->flags |= MGL_HW_MONO_BLT;
			if (AF->cFuncs.PutMonoImage)
				dc->r.putMonoImage		= ACCEL_putMonoImage;
			dc->r.putMouseImage			= ACCEL_putMouseImage;
#ifndef	MGL_LITE
			/* The following is emulated using accelerated mono bitmaps */
			dc->r.drawStrBitmap			= __EMU__drawStrBitmap;
#endif
			}

#ifndef	MGL_LITE
		if (AF->cFuncs.DrawPattScan) {
			dc->r.patt.scanLine			= ACCEL_pattScanLine;
			dc->r.fatPatt.scanLine		= ACCEL_pattScanLine;

			/* The following are emulated via accelerated scanlines */
			dc->r.patt.line				= __EMU__fatPenLine;
			dc->r.fatPatt.line			= __EMU__fatPenLine;
			dc->r.patt.fillRect			= __EMU__fillRect;
			dc->r.fatPatt.fillRect		= __EMU__fillRect;
			dc->r.patt.drawScanList		= __EMU__drawScanList;
			dc->r.fatPatt.drawScanList	= __EMU__drawScanList;
			dc->r.patt.trap				= __EMU__trap;
			dc->r.fatPatt.trap			= __EMU__trap;
			dc->r.patt.drawRegion		= __EMU__drawRegion;
			dc->r.fatPatt.drawRegion	= __EMU__drawRegion;
			dc->r.patt.ellipse			= __EMU__fatPenEllipse;
			dc->r.fatPatt.ellipse		= __EMU__fatPenEllipse;
			dc->r.patt.fillEllipse		= __EMU__fillEllipse;
			dc->r.fatPatt.fillEllipse	= __EMU__fillEllipse;
			dc->r.patt.ellipseArc		= __EMU__fatPenEllipseArc;
			dc->r.fatPatt.ellipseArc	= __EMU__fatPenEllipseArc;
			dc->r.patt.fillEllipseArc	= __EMU__fillEllipseArc;
			dc->r.fatPatt.fillEllipseArc= __EMU__fillEllipseArc;
			}
		if (AF->cFuncs.DrawColorPattScan) {
			dc->r.colorPatt.scanLine 	= ACCEL_colorPattScanLine;
			dc->r.fatColorPatt.scanLine = ACCEL_colorPattScanLine;

			/* The following are emulated via accelerated scanlines */
			dc->r.colorPatt.line				= __EMU__fatPenLine;
			dc->r.fatColorPatt.line				= __EMU__fatPenLine;
			dc->r.colorPatt.fillRect			= __EMU__fillRect;
			dc->r.fatColorPatt.fillRect			= __EMU__fillRect;
			dc->r.colorPatt.drawScanList		= __EMU__drawScanList;
			dc->r.fatColorPatt.drawScanList	 	= __EMU__drawScanList;
			dc->r.colorPatt.trap			 	= __EMU__trap;
			dc->r.fatColorPatt.trap			 	= __EMU__trap;
			dc->r.colorPatt.drawRegion		 	= __EMU__drawRegion;
			dc->r.fatColorPatt.drawRegion	 	= __EMU__drawRegion;
			dc->r.colorPatt.ellipse			 	= __EMU__fatPenEllipse;
			dc->r.fatColorPatt.ellipse		 	= __EMU__fatPenEllipse;
			dc->r.colorPatt.fillEllipse		 	= __EMU__fillEllipse;
			dc->r.fatColorPatt.fillEllipse	 	= __EMU__fillEllipse;
			dc->r.colorPatt.ellipseArc		 	= __EMU__fatPenEllipseArc;
			dc->r.fatColorPatt.ellipseArc	 	= __EMU__fatPenEllipseArc;
			dc->r.colorPatt.fillEllipseArc	 	= __EMU__fillEllipseArc;
			dc->r.fatColorPatt.fillEllipseArc	= __EMU__fillEllipseArc;
			}
		if (AF->cFuncs.DrawPattRect) {
			dc->flags |= MGL_HW_PATT_RECT;
			dc->r.patt.fillRect			= ACCEL_fillPattRect;
			dc->r.fatPatt.fillRect		= ACCEL_fillPattRect;
			}
		if (AF->cFuncs.DrawColorPattRect) {
			dc->flags |= MGL_HW_CLRPATT_RECT;
			dc->r.colorPatt.fillRect	= ACCEL_fillColorPattRect;
			dc->r.fatColorPatt.fillRect	= ACCEL_fillColorPattRect;
			}
		if (AF->cFuncs.DrawScanList) {
			dc->r.solid.drawScanList		= ACCEL_drawScanList;
			dc->r.ropSolid.drawScanList 	= ACCEL_drawScanList;
			dc->r.fatSolid.drawScanList		= ACCEL_drawScanList;
			dc->r.fatRopSolid.drawScanList	= ACCEL_drawScanList;
			}
		if (AF->cFuncs.DrawPattScanList) {
			dc->r.patt.drawScanList 	= ACCEL_drawPattScanList;
			dc->r.fatPatt.drawScanList 	= ACCEL_drawPattScanList;
			}
		if (AF->cFuncs.DrawColorPattScanList) {
			dc->r.colorPatt.drawScanList    = ACCEL_drawColorPattScanList;
			dc->r.fatColorPatt.drawScanList = ACCEL_drawColorPattScanList;
			}
		if (AF->cFuncs.DrawTrap) {
			dc->r.solid.trap			= ACCEL_trap;
			dc->r.ropSolid.trap			= ACCEL_trap;
			dc->r.fatSolid.trap			= ACCEL_trap;
			dc->r.fatRopSolid.trap		= ACCEL_trap;
			}
		if (AF->cFuncs.DrawStippleLine) {
			dc->flags |= MGL_HW_STIPPLE_LINE;
			dc->r.setLineStipple		= ACCEL_setLineStipple;
			dc->r.stippleLine			= ACCEL_stippleLine;
			if (AF->cFuncs.SetLineStippleCount)
				dc->r.setLineStippleCount 	= ACCEL_setLineStippleCount;
			}
		if (AF->cFuncs.BitBlt) {
			dc->flags |= MGL_HW_SCR_BLT;
			dc->r.bitBlt				= ACCEL_bitBlt;
			dc->r.bitBltOff				= ACCEL_bitBltOff;
			}
		if (AF->cFuncs.SrcTransBlt) {
			dc->flags |= MGL_HW_SRCTRANS_BLT;
			dc->r.srcTransBltOff		= ACCEL_srcTransBltOff;
			}
		if (AF->cFuncs.DstTransBlt) {
			dc->flags |= MGL_HW_DSTTRANS_BLT;
			dc->r.dstTransBltOff		= ACCEL_dstTransBltOff;
			}
		if (AF->cFuncs.BitBltLin)
			dc->r.bitBltLin				= ACCEL_bitBltLin;
		if (AF->cFuncs.SrcTransBltLin)
			dc->r.srcTransBltLin		= ACCEL_srcTransBltLin;
		if (AF->cFuncs.DstTransBltLin)
			dc->r.dstTransBltLin		= ACCEL_dstTransBltLin;
#ifdef	MGL_3D
		if (AF->cFuncs.DrawTri) {
			dc->flags |= MGL_HW_POLY;
			dc->r.tri					= ACCEL_tri;
			}
		if (AF->cFuncs.DrawQuad) {
			dc->flags |= MGL_HW_POLY;
			dc->r.quad					= ACCEL_quad;
			}
#endif
#endif

#endif
		}

	/* Ensure active and visual pages are set to 0 */
	dc->v->d.setActivePage(dc,0);
	dc->v->d.setVisualPage(dc,0,MGL_dontWait);
	return true;
}

void _ASMAPI ACCEL_restoreTextMode(MGLDC *dc)
/****************************************************************************
*
* Function:		ACCEL_restoreTextMode
*
* Description:	Restores the previous video mode active before graphics
*				mode was entered.
*
****************************************************************************/
{
	RMREGS		regs;
	int			v;
	ACCEL_state	*state = (ACCEL_state*)dc->v;

	/* Restore the video memory if it was saved */
	ACCEL_restoreVideoMemory();

	/* Set up to return to mono monitor if mono mode was active */
	if (state->oldBIOSMode == 0x7) {
		v = PM_getByte(_MGL_biosSel,0x10);
		PM_setByte(_MGL_biosSel,0x10, (v & 0xCF) | 0x30);
		}
	AF_restoreTextMode(AF);
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
}

ibool _ASMAPI ACCEL_restoreGraphMode(MGLDC *dc)
{
	short t1; ibool t2;
	if (!ACCEL_setMode(dc,dc->mi.scratch1,&t1,&t2,dc->virtualX,dc->virtualY,
			dc->numBuffers,dc->v->d.isStereo != _MGL_NO_STEREO,dc->v->d.refreshRate))
		return false;
	if (dc->v->d.stereoRunning)
		ACCEL_startStereo(dc);
	return true;
}

void _ASMAPI ACCEL_setActivePage(MGLDC *dc,int page)
/****************************************************************************
*
* Function:		ACCEL_setActivePage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current active page for the device context to
*				the specified page value.
*
****************************************************************************/
{
    int ipage = (page & ~MGL_RIGHT_BUFFER);

	if (ipage >= 0 && ipage <= dc->mi.maxPage) {
		dc->v->d.activePage = page;
		if (page & MGL_RIGHT_BUFFER)
			AF_setActiveBuffer(AF,ipage | afRightBuffer);
		else
			AF_setActiveBuffer(AF,ipage | afLeftBuffer);
		dc->surface = dc->surfaceStart + AF->OriginOffset;
		dc->originOffset = (ulong)dc->surface;
#ifdef	__WINDOWS__
		/* Set the new active ipage in our Windows DIB driver */
		if (dc->wm.fulldc.hdc) {
			Escape(dc->wm.fulldc.hdc,MGLDIB_SETSURFACEPTR,4,
				(void*)&dc->surface,NULL);
			}
#endif
		/* We need to reset the hardware clip rectangle for the new
		 * active buffer as this will need to be changed.
		 */
		if (DC.flags & MGL_HW_CLIP) {
			AF_setClipRect(AF,
				DC.intClipRect.left,
				DC.intClipRect.top,
				DC.intClipRect.right-1,
				DC.intClipRect.bottom-1);
			}
		}
}

void _ASMAPI ACCEL_setVisualPage(MGLDC *dc,int page,int waitVRT)
/****************************************************************************
*
* Function:		ACCEL_setVisualPage
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
			while (AF_getDisplayStartStatus(AF) == 0)
				;
			}
		/* Program the display start address */
		AF_setVisibleBuffer(AF,page,(waitVRT == MGL_waitVRT));
		}
}

void _ASMAPI ACCEL_setDisplayStart(MGLDC *dc,int x,int y,int waitFlag)
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
		dc->startX = x;
		dc->startY = y;
		if (waitFlag == MGL_tripleBuffer)
			waitFlag = MGL_waitVRT;
		AF_setDisplayStart(AF,x,y,waitFlag);
		}
}

void _ASMAPI ACCEL_vSync(MGLDC *dc)
/****************************************************************************
*
* Function:		ACCEL_vSync
* Parameters:	dc	- MGL Device context
*
* Description:	Syncs to a vertical interrupt. For VBE 2.0 implementations
*				we can simply set the visual page to the same value while
*				waiting for the vertical interrupt, otherwise we use a
*				VGA style sync routine.
*
****************************************************************************/
{
	AF_setVisibleBuffer(AF,dc->v->d.visualPage,1);
}

void _ASMAPI ACCEL_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,
	int waitVRT)
{
	int			count;
	uchar		*p = (uchar*)&pal[index];
	int			maxProg = dc->v->d.maxProgram;

	/* Determine if we need to wait for the vertical retrace */
	if (!waitVRT)
		count = num;
	else
		count = (num > maxProg) ? maxProg : num;
	while (num) {
		AF_setPaletteData(AF,(AF_palette*)p,count,index,waitVRT);
		index += count;
		p += count*4;
		num -= count;
		count = (num > maxProg) ? maxProg : num;
		}

#ifdef	__WINDOWS__
	/* Download the new palette data to our Windows DIB driver */
	if (dc->wm.fulldc.hdc) {
		Escape(dc->wm.fulldc.hdc,MGLDIB_SETPALETTE,sizeof(RGBQUAD)*256,
			(void*)dc->colorTab,NULL);
		}
#endif
}

void _ASMAPI ACCEL_setCursor(cursor_t *curs)
{ AF_setCursor(AF,(AF_cursor*)curs); }

void _ASMAPI ACCEL_setCursorPos(int x,int y)
{ AF_setCursorPos(AF,x,y); }

void _ASMAPI ACCEL_showCursor(ibool visible)
{ AF_showCursor(AF,visible); }

void _ASMAPI ACCEL_setCursorColor(MGLDC *dc,color_t cursorColor)
/****************************************************************************
*
* Function:     ACCEL_setCursorColor
* Parameters:	dc			- Device context
*				cursorColor	- New MGL color to set cursor to
*
* Description:  This routine sets the cursor color as a color index or
*				as the RGB values depending on the display mode.
*
****************************************************************************/
{
	if (dc->mi.bitsPerPixel == 8)
		AF_setCursorColor(AF,cursorColor,0,0);
	else {
		uchar red,green,blue;
		MGL_unpackColorFast(&dc->pf,cursorColor,red,green,blue);
		AF_setCursorColor(AF,red,green,blue);
		}
}

void _ASMAPI ACCEL_setClipRect(MGLDC *dc,int left,int top,int right,int bottom)
/****************************************************************************
*
* Function:		ACCEL_setClipRect
* Parameters:	left,top,right,bottom
*
* Description:	Sets the hardware clipping rectangle to the specified value.
*				First we set the internal clipping rectangles for the DC
*				with the default driver routines, and set the hardware clip
*				rectangle if supported by the hardware.
*
****************************************************************************/
{
	DRV_setClipRect(dc,left,top,right,bottom);
	if (dc->flags & MGL_HW_CLIP) {
		AF_setClipRect(AF,
			dc->intClipRect.left,
			dc->intClipRect.top,
			dc->intClipRect.right-1,
			dc->intClipRect.bottom-1);
		}
}

void _ASMAPI ACCEL_putMouseImage(MGLDC *dc,int x,int y,int byteWidth,
	int height,uchar *andMask,uchar *xorMask,color_t color)
/****************************************************************************
*
* Function:		ACCEL_putMouseImage
* Parameters:	dc			- Device context
* Parameters:	x			- X coordinate to place image at
*				y			- Y coordinate to place image at
*				byteWidth	- Width of the image in bytes (8 pixels)
*				height		- Height of the image
*				andMask		- Pointer to the AND mask data
*				xorMask		- Pointer to the XOR mask data
*				color		- Color to draw in
*
* Description:	Draws a mouse image at the specified position (x,y). Note
*				that this version for accelerator devices is coded to call
*				the original packed pixel function directly to draw to
*				video memory. This has two benefits; the first is that we
*				can avoid the expensive calls to re-program the hardware
*				clip rectangle, and the second is that because the mouse
*				images is relatively small, the overheads of setting up the
*				accelerated blit make it about the same speed to do it in
*				software or using the accelerator.
*
****************************************************************************/
{
	int		oldop = dc->a.writeMode;
	color_t	oldcolor = dc->intColor;
	rect_t	oldclip = dc->intClipRect;

	dc->intClipRect.left = 0;
	dc->intClipRect.right = dc->mi.xRes+1;
	dc->intClipRect.top = 0;
	dc->intClipRect.bottom = dc->mi.yRes+1;

	dc->r.beginDirectAccess();
	dc->a.writeMode = MGL_REPLACE_MODE;
	dc->intColor = 0;
	_PACKED_vecs.putMonoImage(dc,x,y,byteWidth,height,andMask);
	dc->a.writeMode = MGL_XOR_MODE;
	dc->intColor = color;
	_PACKED_vecs.putMonoImage(dc,x,y,byteWidth,height,xorMask);
	dc->r.endDirectAccess();

	dc->a.writeMode = oldop;
	dc->intColor = oldcolor;
	dc->intClipRect = oldclip;
}

#ifndef	MGL_LITE

void _ASMAPI ACCEL_OFF_setClipRect(MGLDC *dc,int left,int top,int right,int bottom)
/****************************************************************************
*
* Function:		ACCEL_OFF_setClipRect
* Parameters:	left,top,right,bottom
*
* Description:	Sets the hardware clipping rectangle to the specified value.
*				First we set the internal clipping rectangles for the DC
*				with the default driver routines, and set the hardware clip
*				rectangle if supported by the hardware.
*
****************************************************************************/
{
	DRV_setClipRect(dc,left,top,right,bottom);
	if (dc->flags & MGL_HW_CLIP) {
		AF_setClipRect(AF,
			dc->intClipRect.left,
			dc->intClipRect.top + AF->OffscreenStartY,
			dc->intClipRect.right-1,
			dc->intClipRect.bottom-1 + AF->OffscreenStartY);
		}
}

PRIVATE void MakeOffscreenDC(MGLDC *dc)
/****************************************************************************
*
* Function:		MakeOffscreenDC
* Parameters:	dc				- Device context to convert
* Returns:		True if succes, false on failure.
*
* Description:	Creates a linear offscreen device context .
*
****************************************************************************/
{
	/* Initialise dimensions of the offscreen memory region */
	dc->mi.xRes = AF->BufferEndX;
	dc->mi.yRes = AF->OffscreenEndY - AF->OffscreenStartY;
	dc->mi.maxPage = 0;
	dc->surface = dc->surfaceStart + AF->OffscreenOffset;
	dc->size.left = 0;
	dc->size.top = 0;
	dc->size.right = dc->mi.xRes+1;
	dc->size.bottom = dc->mi.yRes+1;

	/* Re-vector drawing code to use offscreen memory routines */
	if (AF->cFuncs.SetClipRect)
		dc->r.setClipRect 				= ACCEL_OFF_setClipRect;
	if (AF->cFuncs.DrawScan) {
		dc->r.solid.scanLine 			= ACCEL_OFF_scanLine;
		dc->r.ropSolid.scanLine 		= ACCEL_OFF_scanLine;
		dc->r.fatSolid.scanLine			= ACCEL_OFF_scanLine;
		dc->r.fatRopSolid.scanLine		= ACCEL_OFF_scanLine;
		}
	if (AF->cFuncs.DrawRect) {
		dc->r.solid.fillRect			= ACCEL_OFF_fillRect;
		dc->r.ropSolid.fillRect			= ACCEL_OFF_fillRect;
		dc->r.fatSolid.fillRect			= ACCEL_OFF_fillRect;
		dc->r.fatRopSolid.fillRect		= ACCEL_OFF_fillRect;
		}
	if (AF->cFuncs.DrawLine) {
		dc->r.solid.line				= ACCEL_OFF_line;
		dc->r.ropSolid.line				= ACCEL_OFF_line;
		}
	if (AF->cFuncs.PutMonoImage) {
		dc->r.putMonoImage				= ACCEL_OFF_putMonoImage;
		}

#ifndef	MGL_LITE
	if (AF->cFuncs.DrawPattScan) {
		dc->r.patt.scanLine 			= ACCEL_OFF_pattScanLine;
		dc->r.fatPatt.scanLine 			= ACCEL_OFF_pattScanLine;
		}
	if (AF->cFuncs.DrawColorPattScan) {
		dc->r.colorPatt.scanLine 		= ACCEL_OFF_colorPattScanLine;
		dc->r.fatColorPatt.scanLine 	= ACCEL_OFF_colorPattScanLine;
		}
	if (AF->cFuncs.DrawPattRect) {
		dc->r.patt.fillRect				= ACCEL_OFF_fillPattRect;
		dc->r.fatPatt.fillRect			= ACCEL_OFF_fillPattRect;
		}
	if (AF->cFuncs.DrawColorPattRect) {
		dc->r.colorPatt.fillRect		= ACCEL_OFF_fillColorPattRect;
		dc->r.fatColorPatt.fillRect		= ACCEL_OFF_fillColorPattRect;
		}
	if (AF->cFuncs.DrawScanList) {
		dc->r.solid.drawScanList		= ACCEL_OFF_drawScanList;
		dc->r.ropSolid.drawScanList 	= ACCEL_OFF_drawScanList;
		dc->r.fatSolid.drawScanList		= ACCEL_OFF_drawScanList;
		dc->r.fatRopSolid.drawScanList 	= ACCEL_OFF_drawScanList;
		}
	if (AF->cFuncs.DrawPattScanList) {
		dc->r.patt.drawScanList 	= ACCEL_OFF_drawPattScanList;
		dc->r.fatPatt.drawScanList 	= ACCEL_OFF_drawPattScanList;
		}
	if (AF->cFuncs.DrawColorPattScanList) {
		dc->r.colorPatt.drawScanList = ACCEL_OFF_drawColorPattScanList;
		dc->r.fatColorPatt.drawScanList = ACCEL_OFF_drawColorPattScanList;
		}
	if (AF->cFuncs.DrawTrap) {
		dc->r.solid.trap				= ACCEL_OFF_trap;
		dc->r.ropSolid.trap				= ACCEL_OFF_trap;
		dc->r.fatSolid.trap				= ACCEL_OFF_trap;
		dc->r.fatRopSolid.trap			= ACCEL_OFF_trap;
		}
	if (AF->cFuncs.DrawStippleLine) {
		dc->r.stippleLine				= ACCEL_OFF_stippleLine;
		}
	if (AF->cFuncs.BitBlt) {
		dc->r.bitBlt					= ACCEL_OFF_bitBlt;
		}
#ifdef	MGL_3D
	if (AF->cFuncs.DrawTri) {
		dc->r.tri						= ACCEL_OFF_tri;
		}
	if (AF->cFuncs.DrawQuad) {
		dc->r.quad						= ACCEL_OFF_quad;
		}
#endif
#endif
}

ibool _ASMAPI ACCEL_makeOffscreenDC(MGLDC *dc)
/****************************************************************************
*
* Function:		ACCEL_makeOffscreenDC
* Parameters:	dc				- Device context to convert
* Returns:		True if succes, false on failure.
*
* Description:
*
****************************************************************************/
{
	if (!(dc->flags & MGL_HW_SCR_BLT)) {
		_MGL_result = grNoHardwareBlt;
		return false;
		}
	if (!AF->OffscreenOffset) {
		_MGL_result = grNoOffscreenMem;
		return false;
		}
	dc->deviceType = MGL_OFFSCREEN_DEVICE;
	MakeOffscreenDC(dc);
	return true;
}

ibool _ASMAPI ACCEL_makeLinearOffscreenDC(MGLDC *dc)
/****************************************************************************
*
* Function:		ACCEL_makeLinearOffscreenDC
* Parameters:	dc				- Device context to convert
* Returns:		True if succes, false on failure.
*
* Description:	Creates a linear offscreen device context .
*
****************************************************************************/
{
	if (!(dc->flags & MGL_HW_SCR_BLT)) {
		_MGL_result = grNoHardwareBlt;
		return false;
		}
	if (!AF->OffscreenOffset) {
		_MGL_result = grNoOffscreenMem;
		return false;
		}
	dc->deviceType = MGL_LINOFFSCREEN_DEVICE;
	MakeOffscreenDC(dc);
	return true;
}

#endif	/* !MGL_LITE */

AF_devCtx * MGLAPI _MGL_getAFDC(void)
{ return _MGL_afPtr; }

/****************************************************************************
VBE/AF device driver interface functions
****************************************************************************/

#ifndef	MGL_LITE
void _ASMAPI ACCEL_setWriteMode(int mode)
{
	DC.a.writeMode = mode;
	if (DC.a.penStyle == MGL_BITMAP_OPAQUE)
		AF_setMix(AF,mode,mode);
	else
		AF_setMix(AF,mode,AF_NOP_MIX);
	_MGL_setRenderingVectors();
}

void _ASMAPI ACCEL_setPenStyle(int style)
{
	DC.a.penStyle = style;
	if (DC.a.penStyle == MGL_BITMAP_OPAQUE)
		AF_setMix(AF,DC.a.writeMode,DC.a.writeMode);
	else
		AF_setMix(AF,DC.a.writeMode,AF_NOP_MIX);
	_MGL_setRenderingVectors();
}

void _ASMAPI ACCEL_setPenBitmapPattern(const pattern_t *pat)
{
	memcpy(&DC.a.penPat,pat,sizeof(DC.a.penPat));
	if (AF->cFuncs.Set8x8MonoPattern)
		AF_set8x8MonoPattern(AF,(AF_pattern*)pat);
}

void _ASMAPI ACCEL_setPenPixmapPattern(const pixpattern_t *pat)
{
	memcpy(&DC.a.penPixPat,pat,sizeof(DC.a.penPixPat));
	if (AF->cFuncs.Set8x8ColorPattern) {
		AF_set8x8ColorPattern(AF,0,(AF_color*)pat);
		AF_use8x8ColorPattern(AF,0);
		}
}

void _ASMAPI ACCEL_setLineStipple(ushort stipple)
{
	DC.a.lineStipple = stipple;
	AF_setLineStipple(AF,stipple);
}

void _ASMAPI ACCEL_setLineStippleCount(uint count)
{
	DC.a.stippleCount = count;
    AF_setLineStippleCount(AF,count);
}

#endif	/* !MGL_LITE */

void _ASMAPI ACCEL_beginDirectAccess(void)
{ AF_waitTillIdle(AF); }

void _ASMAPI ACCEL_endDirectAccess(void)
{}

void _ASMAPI ACCEL_beginPixel(MGLDC *dc)
{ AF_waitTillIdle(AF); }

void _ASMAPI ACCEL_endPixel(MGLDC *dc)
{}

void _ASMAPI ACCEL_beginDirectAccessSLOW(void)
{
	if (++_MGL_surfLock == 0)
		AF_enableDirectAccess(AF);
}

void _ASMAPI ACCEL_endDirectAccessSLOW(void)
{
	if (--_MGL_surfLock == -1)
		AF_disableDirectAccess(AF);
}

void _ASMAPI ACCEL_beginPixelSLOW(MGLDC *dc)
{
	if (++_MGL_surfLock == 0)
		AF_enableDirectAccess(AF);
}

void _ASMAPI ACCEL_endPixelSLOW(MGLDC *dc)
{
	if (--_MGL_surfLock == -1)
		AF_disableDirectAccess(AF);
}

void _ASMAPI ACCEL_scanLine(int y,int x1,int x2)
{ AF_drawScan(AF,DC.intColor,y,x1,x2); }

#ifndef	MGL_LITE
void _ASMAPI ACCEL_OFF_scanLine(int y,int x1,int x2)
{ AF_drawScan(AF,DC.intColor,y + AF->OffscreenStartY,x1,x2); }

void _ASMAPI ACCEL_pattScanLine(int y,int x1,int x2)
{ AF_drawPattScan(AF,DC.intColor,DC.intBackColor,y,x1,x2); }

void _ASMAPI ACCEL_OFF_pattScanLine(int y,int x1,int x2)
{ AF_drawPattScan(AF,DC.intColor,DC.intBackColor,y + AF->OffscreenStartY,x1,x2); }

void _ASMAPI ACCEL_colorPattScanLine(int y,int x1,int x2)
{ AF_drawColorPattScan(AF,y,x1,x2); }

void _ASMAPI ACCEL_OFF_colorPattScanLine(int y,int x1,int x2)
{ AF_drawColorPattScan(AF,y + AF->OffscreenStartY,x1,x2); }
#endif	/* !MGL_LITE */

void _ASMAPI ACCEL_fillRect(int x1,int y1,int x2,int y2)
{ AF_drawRect(AF,DC.intColor,x1,y1,x2-x1,y2-y1); }

#ifndef	MGL_LITE
void _ASMAPI ACCEL_OFF_fillRect(int x1,int y1,int x2,int y2)
{ AF_drawRect(AF,DC.intColor,x1,y1 + AF->OffscreenStartY,x2-x1,y2-y1); }

void _ASMAPI ACCEL_fillPattRect(int x1,int y1,int x2,int y2)
{ AF_drawPattRect(AF,DC.intColor,DC.intBackColor,x1,y1,x2-x1,y2-y1); }

void _ASMAPI ACCEL_OFF_fillPattRect(int x1,int y1,int x2,int y2)
{ AF_drawPattRect(AF,DC.intColor,DC.intBackColor,x1,y1 + AF->OffscreenStartY,x2-x1,y2-y1); }

void _ASMAPI ACCEL_fillColorPattRect(int x1,int y1,int x2,int y2)
{ AF_drawColorPattRect(AF,x1,y1,x2-x1,y2-y1); }

void _ASMAPI ACCEL_OFF_fillColorPattRect(int x1,int y1,int x2,int y2)
{ AF_drawColorPattRect(AF,x1,y1 + AF->OffscreenStartY,x2-x1,y2-y1); }
#endif	/* !MGL_LITE */

void _ASMAPI ACCEL_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image)
{
	int		srcX,srcY;
	rect_t	d;

	/* Clip to destination device context */
	d.left = x;						d.top = y;
	d.right = x + (byteWidth<<3);	d.bottom = y + height;
	if (dc->a.clip) {
		if (!MGL_sectRect(dc->intClipRect,d,&d))
			return;
		srcX = d.left-x;
		srcY = d.top-y;
		}
	else {
		srcX = srcY = 0;
		}
	AF_putMonoImage(AF,dc->intColor,dc->intBackColor,d.left,d.top,
		byteWidth, srcX, srcY, d.right-d.left, d.bottom-d.top,
		image);
}

#ifndef	MGL_LITE
void _ASMAPI ACCEL_OFF_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image)
{
	int		srcX,srcY;
	rect_t	d;

	/* Clip to destination device context */
	d.left = x;						d.top = y;
	d.right = x + (byteWidth<<3);	d.bottom = y + height;
	if (dc->a.clip) {
		if (!MGL_sectRect(dc->intClipRect,d,&d))
			return;
		srcX = d.left-x;
		srcY = d.top-y;
		}
	else {
		srcX = srcY = 0;
		}
	AF_putMonoImage(AF,dc->intColor,dc->intBackColor,d.left,
		d.top+AF->OffscreenStartY,byteWidth,srcX,srcY,
		d.right-d.left, d.bottom-d.top,image);
}
#endif	/* !MGL_LITE */

void _ASMAPI ACCEL_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
{ AF_drawLine(AF,DC.intColor,x1,y1,x2,y2); }

#ifndef	MGL_LITE
void _ASMAPI ACCEL_OFF_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
{ AF_drawLine(AF,DC.intColor,x1,y1 + (AF->OffscreenStartY<<16),x2,y2 + (AF->OffscreenStartY<<16)); }

void _ASMAPI ACCEL_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
{ AF_drawStippleLine(AF,DC.intColor,DC.intBackColor,x1,y1,x2,y2); }

void _ASMAPI ACCEL_OFF_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
{ AF_drawStippleLine(AF,DC.intColor,DC.intBackColor,x1,y1 + (AF->OffscreenStartY<<16),x2,y2 + (AF->OffscreenStartY<<16)); }

void _ASMAPI ACCEL_drawScanList(int y,int length,short *scans)
{ AF_drawScanList(AF,DC.intColor,y,length,scans); }

void _ASMAPI ACCEL_OFF_drawScanList(int y,int length,short *scans)
{ AF_drawScanList(AF,DC.intColor,y+AF->OffscreenStartY,length,scans); }

void _ASMAPI ACCEL_drawPattScanList(int y,int length,short *scans)
{ AF_drawPattScanList(AF,DC.intColor,DC.intBackColor,y,length,scans); }

void _ASMAPI ACCEL_OFF_drawPattScanList(int y,int length,short *scans)
{ AF_drawPattScanList(AF,DC.intColor,DC.intBackColor,y+AF->OffscreenStartY,length,scans); }

void _ASMAPI ACCEL_drawColorPattScanList(int y,int length,short *scans)
{ AF_drawColorPattScanList(AF,y,length,scans); }

void _ASMAPI ACCEL_OFF_drawColorPattScanList(int y,int length,short *scans)
{ AF_drawColorPattScanList(AF,y+AF->OffscreenStartY,length,scans); }

void _ASMAPI ACCEL_trap(void)
{ AF_drawTrap(AF,DC.intColor,(AF_trap*)&DC.tr); }

void _ASMAPI ACCEL_OFF_trap(void)
{
	DC.tr.y += AF->OffscreenStartY;
	AF_drawTrap(AF,DC.intColor,(AF_trap*)&DC.tr);
	DC.tr.y -= AF->OffscreenStartY;
}

void _ASMAPI ACCEL_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset)
{ AF_drawTri(AF,DC.intColor,(AF_fxpoint*)v1,(AF_fxpoint*)v2,(AF_fxpoint*)v3,xOffset,yOffset); }

void _ASMAPI ACCEL_OFF_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset)
{ AF_drawTri(AF,DC.intColor,(AF_fxpoint*)v1,(AF_fxpoint*)v2,(AF_fxpoint*)v3,xOffset,yOffset + (AF->OffscreenStartY<<16)); }

void _ASMAPI ACCEL_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset)
{ AF_drawQuad(AF,DC.intColor,(AF_fxpoint*)v1,(AF_fxpoint*)v2,(AF_fxpoint*)v3,(AF_fxpoint*)v4,xOffset,yOffset); }

void _ASMAPI ACCEL_OFF_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset)
{ AF_drawQuad(AF,DC.intColor,(AF_fxpoint*)v1,(AF_fxpoint*)v2,(AF_fxpoint*)v3,(AF_fxpoint*)v4,xOffset,yOffset + (AF->OffscreenStartY<<16)); }

void _ASMAPI ACCEL_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy)
{ AF_bitBlt(AF,left,top,right-left,bottom-top,dstLeft,dstTop,op); }

void _ASMAPI ACCEL_OFF_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy)
{ AF_bitBlt(AF,left,top+AF->OffscreenStartY,right-left,bottom-top,dstLeft,dstTop+AF->OffscreenStartY,op); }

void _ASMAPI ACCEL_bitBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op)
{ AF_bitBlt(AF,left,top+AF->OffscreenStartY,right-left,bottom-top,dstLeft,dstTop,op); }

void _ASMAPI ACCEL_bitBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op)
{
	int		srcX,srcY;
	rect_t	d;

	/* Clip to destination device context */
	d.left = dstLeft;				d.top = dstTop;
	d.right = dstRight;				d.bottom = dstBottom;
	if (dst->a.clip) {
		if (!MGL_sectRect(dst->intClipRect,d,&d))
			return;
		srcX = d.left-dstLeft;
		srcY = d.top-dstTop;
		}
	else {
		srcX = srcY = 0;
		}
	AF_bitBltLin(AF,srcOfs+AF->OffscreenOffset,src->mi.bytesPerLine,
		srcX,srcY,d.right-d.left,d.bottom-d.top,
		d.left,d.top,op);
}

void _ASMAPI ACCEL_srcTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
{ AF_srcTransBlt(AF,left,top+AF->OffscreenStartY,right-left,bottom-top,dstLeft,dstTop,op,transparent); }

void _ASMAPI ACCEL_srcTransBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op,color_t transparent)
{
	int		srcX,srcY;
	rect_t	d;

	/* Clip to destination device context */
	d.left = dstLeft;				d.top = dstTop;
	d.right = dstRight;				d.bottom = dstBottom;
	if (dst->a.clip) {
		if (!MGL_sectRect(dst->intClipRect,d,&d))
			return;
		srcX = d.left-dstLeft;
		srcY = d.top-dstTop;
		}
	else {
		srcX = srcY = 0;
		}
	AF_srcTransBltLin(AF,srcOfs+AF->OffscreenOffset,src->mi.bytesPerLine,
		srcX, srcY, d.right-d.left, d.bottom-d.top,
		d.left,d.top,op,transparent);
}

void _ASMAPI ACCEL_dstTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
{ AF_dstTransBlt(AF,left,top+AF->OffscreenStartY,right-left,bottom-top,dstLeft,dstTop,op,transparent); }

void _ASMAPI ACCEL_dstTransBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op,color_t transparent)
{
	int		srcX,srcY;
	rect_t	d;

	/* Clip to destination device context */
	d.left = dstLeft;				d.top = dstTop;
	d.right = dstRight;				d.bottom = dstBottom;
	if (dst->a.clip) {
		if (!MGL_sectRect(dst->intClipRect,d,&d))
		srcX = d.left-dstLeft;
		srcY = d.top-dstTop;
		}
	else {
		srcX = srcY = 0;
		}
	AF_dstTransBltLin(AF,srcOfs+AF->OffscreenOffset,src->mi.bytesPerLine,
		srcX, srcY, d.right-d.left, d.bottom-d.top,
		d.left,d.top,op,transparent);
}

#endif	/* !MGL_LITE */

#endif
