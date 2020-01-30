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
* Description:	Device driver helper routines common to all VGA compatible
*				device drivers.
*
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\common\vga.h"

/*------------------------- Implementation --------------------------------*/

ibool MGLAPI VGA_detect(int *driver)
/****************************************************************************
*
* Function:		VGA_detect
* Returns:		True if the card is a VGA compatible card.
*
* Description:	Attempts to determine if the video card is VGA compatible
*				or not.
*
****************************************************************************/
{
	RMREGS	regs;

	MGLPC_init();
#ifdef	MGLWIN
	if (!_MGL_haveWinDirect)
		return false;
#endif
	regs.x.ax = 0x1A00;
	PM_int86(0x10,&regs,&regs);
	if (regs.h.al != 0x1A)
		return false;				/* PS2 style BIOS is out there		*/

	/* Check for active or inactive VGA color video card installed		*/
	if (regs.h.bl == 0x08 || regs.h.bl == 0x07 ||
			regs.h.bh == 0x08 || regs.h.bl == 0x07) {
		if (*driver < grVGA)
			*driver = grVGA;
		return true;
		}
	return false;
}

void MGLAPI VGA_useMode(modetab modes,int mode,int id,gmode_t *mi)
/****************************************************************************
*
* Function:		VGA_useMode
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
	/* Set direct color mode info for 8 bit dithered operation */
	mi->redMaskSize 		= 8;
	mi->redFieldPosition 	= 16;
	mi->greenMaskSize 		= 8;
	mi->greenFieldPosition 	= 8;
	mi->blueMaskSize 		= 8;
	mi->blueFieldPosition 	= 0;
	mi->rsvdMaskSize 		= 0;
	mi->rsvdFieldPosition	= 0;
	DRV_useMode(modes,mode,id,mi->maxPage+1,0);
}

ibool MGLAPI VGA_setMode(MGLDC *dc,int AX,int BX,short *oldBIOSMode,
	ibool *old50Lines,void (*saveVideoMemory)(int mode))
/****************************************************************************
*
* Function:		VGA_setMode
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
	RMREGS		regs;
	int			v;
	static void	*videoMem = NULL;

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

	/* Set up for the color display to be active, and set the mode */
	v = PM_getByte(_MGL_biosSel,0x10);
	PM_setByte(_MGL_biosSel,0x10, (v & 0xCF) | 0x20);
	if (saveVideoMemory)
		saveVideoMemory(BX);
	PM_setByte(_MGL_biosSel,0x49,0x10);
	regs.x.ax = AX;
	regs.x.bx = BX;
	PM_int86(0x10,&regs,&regs);

	if (!videoMem) {
#if	defined(__WIN386__) || defined(DJGPP)
		ushort sel = PM_createSelector(0xA0000UL,0xFFFF);
        if (!sel) {
			MGLPC_disableFullScreen();
			MGL_fatalError("Unable to obtain video memory pointer!\n");
			}
		videoMem = (void*)((ulong)sel << 16);
#else
		if ((videoMem = PM_mapPhysicalAddr(0xA0000UL,0xFFFF)) == NULL) {
			MGLPC_disableFullScreen();
			MGL_fatalError("Unable to obtain video memory pointer!\n");
			}
#endif
		}
	if (dc) {
    	dc->v->d.isStereo			= _MGL_NO_STEREO;
		dc->surface 				= dc->surfaceStart = videoMem;
		dc->v->d.setActivePage 		= VGA_setActivePage;
		dc->v->d.setVisualPage 		= VGA_setVisualPage;
		dc->v->d.setDisplayStart	= DRV_stubVector;
		dc->r.realizePalette 		= VGA_realizePalette;
		dc->v->d.vSync 				= VGA_vSync;
		dc->r.beginDirectAccess		= DRV_stubVector;
		dc->r.endDirectAccess		= DRV_stubVector;
		}
	return TRUE;
}

void MGLAPI VGA_restoreMode(ibool oldBIOSMode,ibool old50Lines,
	void (*restoreVideoMemory)(void))
/****************************************************************************
*
* Function:		VGA_restoreMode
* Parameters:	oldBIOSMode	- Old BIOS mode number
*				old50Lines	- True if old mode was 43/50 line mode
*
* Description:	Restores the previous video mode active before graphics
*				mode was entered.
*
****************************************************************************/
{
	RMREGS	regs;
	int		v;

	/* Reset display start address */
	VGA_setCRTStart(0, 0, 0);

	/* Restore the video memory if it was saved */
	if (restoreVideoMemory)
		restoreVideoMemory();

	/* Set up to return to mono monitor if mono mode was active */
	if (oldBIOSMode == 0x7) {
		v = PM_getByte(_MGL_biosSel,0x10);
		PM_setByte(_MGL_biosSel,0x10, (v & 0xCF) | 0x30);
		}

	regs.x.ax = 0x3;
	PM_int86(0x10,&regs,&regs);     	/* Set text mode       			*/
	if (old50Lines) {
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
}

void _ASMAPI VGA_setActivePage(MGLDC *dc,int page)
/****************************************************************************
*
* Function:		VGA_setActivePage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current active page for the device context to
*				the specified page value.
*
****************************************************************************/
{
	if (page >= 0 && page <= dc->mi.maxPage) {
		dc->v->d.activePage = page;
		dc->originOffset = dc->mi.pageSize * page;
		dc->surface = dc->surfaceStart + dc->originOffset;
#ifdef	__WINDOWS__
		/* Set the new active page in our Windows DIB driver */
		if (dc->wm.fulldc.hdc) {
			Escape(dc->wm.fulldc.hdc,MGLDIB_SETSURFACEPTR,4,
				(void*)&dc->surface,NULL);
			}
#endif
		}
}

void _ASMAPI VGA_setVisualPage(MGLDC *dc,int page,int waitVRT)
/****************************************************************************
*
* Function:		VGA_setVisualPage
* Parameters:	dc		- Device context
*				page	- Active page number
*
* Description:	Sets the current visual page for the device context to
*				the specified page value.
*
****************************************************************************/
{
	if (page >= 0 && page <= dc->mi.maxPage) {
		dc->v->d.visualPage = page;
		if (waitVRT == MGL_tripleBuffer)
			waitVRT = MGL_waitVRT;
		VGA_setCRTStart(dc->mi.pageSize * page, page, waitVRT);
		}
}

void _ASMAPI VGA_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,
	int waitVRT)
/****************************************************************************
*
* Function:		VGA4_realizePalette
* Parameters:	dc		- Device context
*               pal		- Palette of values to program
*               num		- Number of entries to program
*				index	- Index to start programming at
*
* Description:	Program the VGA palette. First we need to translate the
*				values from the MGL internal format into the 6 bit RGB
*				format used by the VGA.
*
****************************************************************************/
{
	int			i;
	uchar		temp[1024],*t,*p = (uchar*)&pal[index];
	display_vec	*d = &dc->v->d;

	/* First copy the palette values into our temporary palette, translating
	 * from the internal 8 bit format to the VGA 6 bit format (if we have
	 * a wide palette, then we simply skip this step)
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
	VGA_blastPalette(p,num,index,d->maxProgram,waitVRT);
#ifdef	__WINDOWS__
	/* Download the new palette data to our Windows DIB driver */
	if (dc->wm.fulldc.hdc) {
		Escape(dc->wm.fulldc.hdc,MGLDIB_SETPALETTE,sizeof(RGBQUAD)*256,
			(void*)dc->colorTab,NULL);
		}
#endif
}

