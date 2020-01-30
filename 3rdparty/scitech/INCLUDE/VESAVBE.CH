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
* Environment:  IBM PC Real Mode and 16/32 bit Protected Mode.
*
* Description:  Module to implement a C callable interface to the standard
*               VESA VBE routines. You should rip out this module and use it
*               directly in your own applications, or you can use the
*               high level SDK functions.
*
*               MUST be compiled in the LARGE or FLAT models.
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vesavbe.h"
#include "pmode.h"
#ifdef	__MSDOS__
#include <dos.h>
#endif
#ifdef	__WINDOWS__
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/*---------------------------- Global Variables ---------------------------*/

#define VBE_SUCCESS     0x004F

static int      	VBEVersion;     /* VBE version detected             */
static int			VBEMemory;      /* Amount of memory on board        */
static uint     	VESABuf_len = 1024;/* Length of the VESABuf buffer	*/
static uint     	VESABuf_sel = 0;/* Selector for VESABuf             */
static uint     	VESABuf_off;    /* Offset for VESABuf               */
static uint     	VESABuf_rseg;   /* Real mode segment of VESABuf     */
static uint     	VESABuf_roff;	/* Real mode offset of VESABuf      */
static ibool		haveRiva128;	/* True if we have a Riva128		*/

#ifndef REALMODE
short _ASMAPI 		VBE_MMIOSel = 0;/* Selector to MMIO registers		*/
static VBE_pmInfo 	*pmInfo = NULL; /* Global PM code block             */
static VBE_pmInfo  	*pmInfo32 = NULL;
static char        	localBuf[512];	/* Global PM string translate buf   */
#ifdef  __WINDOWS16__
static ushort      code32sel = 0;  /* 16 bit PM 32 bit code selector   */
#endif
#endif

/*----------------------------- Implementation ----------------------------*/

/* static function in WinDirect for passing 32-bit registers to BIOS */
int PMAPI WD_int386(int intno, RMREGS *in, RMREGS *out);

void VBEAPI VBE_init(void)
/****************************************************************************
*
* Function:     VBE_init
*
* Description:  Initialises the VBE transfer buffer in real mode DC.memory.
*               This routine is called by the VESAVBE module every time
*               it needs to use the transfer buffer, so we simply allocate
*               it once and then return.
*
****************************************************************************/
{
	if (!VESABuf_sel) {
		/* Allocate a global buffer for communicating with the VESA VBE */
		if (!PM_getVESABuf(&VESABuf_len, &VESABuf_sel, &VESABuf_off, &VESABuf_rseg, &VESABuf_roff)) {
#ifdef  __WINDOWS__
			MessageBox(NULL,
				"Real mode memory allocation failed!",
				"VESAVBE.C Fatal Error!", MB_ICONEXCLAMATION);
#else
			printf("VESAVBE.C: Real mode memory allocation failed!\n");
#endif
			exit(1);
			}
		}
}

void VBEAPI VBE_getRMBuf(uint *len,uint *sel,uint *off,uint *rseg,uint *roff)
/****************************************************************************
*
* Function:     VBE_getRMBuf
*
* Description:  This function returns the location and length of the real
*				mode memory buffer for calling real mode functions.
*
****************************************************************************/
{
	*len = VESABuf_len;
	*sel = VESABuf_sel;
	*off = VESABuf_off;
	*rseg = VESABuf_rseg;
	*roff = VESABuf_roff;
}

void VBEAPI VBE_callESDI(RMREGS *regs, void *buffer, int size)
/****************************************************************************
*
* Function:     VBE_callESDI
* Parameters:   regs    - Registers to load when calling VBE
*               buffer  - Buffer to copy VBE info block to
*               size    - Size of buffer to fill
*
* Description:  Calls the VESA VBE and passes in a buffer for the VBE to
*               store information in, which is then copied into the users
*               buffer space. This works in protected mode as the buffer
*               passed to the VESA VBE is allocated in conventional
*               memory, and is then copied into the users memory block.
*
****************************************************************************/
{
    RMSREGS sregs;

    if (!VESABuf_sel) {
#ifdef  __WINDOWS__
    MessageBox(NULL,
        "You *MUST* call VBE_init() before you can call the VESAVBE.C module!",
        "VESAVBE.C Fatal Error!", MB_ICONEXCLAMATION);
#else
        printf("You *MUST* call VBE_init() before you can call the VESAVBE.C module!\n");
#endif
        exit(1);
        }
    sregs.es = (ushort)VESABuf_rseg;
    regs->x.di = (ushort)VESABuf_roff;
    PM_memcpyfn(VESABuf_sel, VESABuf_off, buffer, size);
    PM_int86x(0x10, regs, regs, &sregs);
    PM_memcpynf(buffer, VESABuf_sel, VESABuf_off, size);
}

#ifndef REALMODE
static char *VBE_copyStrToLocal(char *p,char *realPtr)
/****************************************************************************
*
* Function:     VBE_copyStrToLocal
* Parameters:   p       - Flat model buffer to copy to
*               realPtr - Real mode pointer to copy
* Returns:      Pointer to the next byte after string
*
* Description:  Copies the string from the real mode location pointed to
*               by 'realPtr' into the flat model buffer pointed to by
*               'p'. We return a pointer to the next byte past the copied
*               string.
*
****************************************************************************/
{
    uint    sel,off;
    uchar   v;

    PM_mapRealPointer(&sel,&off,
        (uint)((ulong)realPtr >> 16),
        (uint)((ulong)realPtr & 0xFFFF));
    while ((v = PM_getByte(sel,off)) != 0) {
		*p++ = v;
		off++;
		}
	*p++ = 0;
    return p;
}

static void VBE_copyShortToLocal(ushort *p,ushort *realPtr)
/****************************************************************************
*
* Function:     VBE_copyShortToLocal
* Parameters:   p       - Flat model buffer to copy to
*               realPtr - Real mode pointer to copy
*
* Description:  Copies the mode table from real mode memory to the flat
*               model buffer.
*
****************************************************************************/
{
    uint    sel,off;
    ushort  v;

    PM_mapRealPointer(&sel,&off,
        (uint)((ulong)realPtr >> 16),
        (uint)((ulong)realPtr & 0xFFFF));
    while ((v = PM_getWord(sel,off)) != 0xFFFF) {
        *p++ = v;
        off += 2;
        }
	*p = 0xFFFF;
}
#endif

int VBEAPI VBE_detectEXT(VBE_vgaInfo *vgaInfo,ibool forceUniVBE)
/****************************************************************************
*
* Function:     VBE_detect
* Parameters:   vgaInfo - Place to store the VGA information block
* Returns:      VBE version number, or 0 if not detected.
*
* Description:  Detects if a VESA VBE is out there and functioning
*               correctly. If we detect a VBE interface we return the
*               VGAInfoBlock returned by the VBE and the VBE version number.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F00;     /* Get SuperVGA information */
    if (forceUniVBE) {
        regs.x.bx = 0x1234;
        regs.x.cx = 0x4321;
        }
    else {
        regs.x.bx = 0;
        regs.x.cx = 0;
        }
	strncpy(vgaInfo->VESASignature,"VBE2",4);
    VBE_callESDI(&regs, vgaInfo, sizeof(*vgaInfo));
    if (regs.x.ax != VBE_SUCCESS)
        return 0;
    if (strncmp(vgaInfo->VESASignature,"VESA",4) != 0)
        return 0;

    /* Check for bogus BIOSes that return a VBE version number that is
	 * not correct, and fix it up. We also check the OemVendorNamePtr for a
	 * valid value, and if it is invalid then we also reset to VBE 1.2.
	 */
	if (vgaInfo->VESAVersion >= 0x200 && vgaInfo->OemVendorNamePtr == 0)
		vgaInfo->VESAVersion = 0x102;
#ifndef REALMODE
    /* Relocate all the indirect information (mode tables, OEM strings
	 * etc) from the low 1Mb memory region into a static buffer in
	 * our default data segment. We do this to insulate the application
	 * from mapping the strings from real mode to protected mode.
	 */
	{
		char *p,*p2;
		p2 = VBE_copyStrToLocal(localBuf,vgaInfo->OemStringPtr);
		vgaInfo->OemStringPtr = localBuf;
		if (vgaInfo->VESAVersion >= 0x200) {
			p = VBE_copyStrToLocal(p2,vgaInfo->OemVendorNamePtr);
			vgaInfo->OemVendorNamePtr = p2;
			p2 = VBE_copyStrToLocal(p,vgaInfo->OemProductNamePtr);
			vgaInfo->OemProductNamePtr = p;
			p = VBE_copyStrToLocal(p2,vgaInfo->OemProductRevPtr);
			vgaInfo->OemProductRevPtr = p2;
			VBE_copyShortToLocal((ushort*)p,vgaInfo->VideoModePtr);
			vgaInfo->VideoModePtr = (ushort*)p;
			}
		else {
			VBE_copyShortToLocal((ushort*)p2,vgaInfo->VideoModePtr);
			vgaInfo->VideoModePtr = (ushort*)p2;
			}
	}
#endif
	VBEMemory = vgaInfo->TotalMemory * 64;

	/* Check for Riva128 based cards since they have broken triple buffering
	 * and stereo support.
	 */
	haveRiva128 = false;
	if (vgaInfo->VESAVersion >= 0x300 &&
		   (strstr(vgaInfo->OemStringPtr,"NVidia") == 0 ||
			strstr(vgaInfo->OemStringPtr,"Riva") == 0)) {
		haveRiva128 = true;
		}

	return (VBEVersion = vgaInfo->VESAVersion);
}

int VBEAPI VBE_detect(VBE_vgaInfo *vgaInfo)
/****************************************************************************
*
* Function:     VBE_detect
* Parameters:   vgaInfo - Place to store the VGA information block
* Returns:      VBE version number, or 0 if not detected.
*
* Description:  Detects if a VESA VBE is out there and functioning
*               correctly. If we detect a VBE interface we return the
*               VGAInfoBlock returned by the VBE and the VBE version number.
*
****************************************************************************/
{
    return VBE_detectEXT(vgaInfo,false);
}

ibool VBEAPI VBE_getModeInfo(int mode,VBE_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     VBE_getModeInfo
* Parameters:   mode        - VBE mode to get information for
*               modeInfo    - Place to store VBE mode information
* Returns:      True on success, false if function failed.
*
* Description:  Obtains information about a specific video mode from the
*               VBE. You should use this function to find the video mode
*               you wish to set, as the new VBE 2.0 mode numbers may be
*               completely arbitrary.
*
****************************************************************************/
{
    RMREGS  regs;
    int     bits;

    regs.x.ax = 0x4F01;             /* Get mode information         */
    regs.x.cx = (ushort)mode;
    VBE_callESDI(&regs, modeInfo, sizeof(*modeInfo));
    if (regs.x.ax != VBE_SUCCESS)
        return false;
    if ((modeInfo->ModeAttributes & vbeMdAvailable) == 0)
        return false;

	/* Map out triple buffer and stereo flags for NVidia Riva128
	 * chips.
	 */
	if (haveRiva128) {
		modeInfo->ModeAttributes &= ~vbeMdTripleBuf;
		modeInfo->ModeAttributes &= ~vbeMdStereo;
		}

	/* Support old style RGB definitions for VBE 1.1 BIOSes */
    bits = modeInfo->BitsPerPixel;
    if (modeInfo->MemoryModel == vbeMemPK && bits > 8) {
        modeInfo->MemoryModel = vbeMemRGB;
        switch (bits) {
			case 15:
                modeInfo->RedMaskSize = 5;
                modeInfo->RedFieldPosition = 10;
                modeInfo->GreenMaskSize = 5;
                modeInfo->GreenFieldPosition = 5;
                modeInfo->BlueMaskSize = 5;
                modeInfo->BlueFieldPosition = 0;
                modeInfo->RsvdMaskSize = 1;
                modeInfo->RsvdFieldPosition = 15;
                break;
            case 16:
                modeInfo->RedMaskSize = 5;
                modeInfo->RedFieldPosition = 11;
                modeInfo->GreenMaskSize = 5;
                modeInfo->GreenFieldPosition = 5;
                modeInfo->BlueMaskSize = 5;
                modeInfo->BlueFieldPosition = 0;
                modeInfo->RsvdMaskSize = 0;
                modeInfo->RsvdFieldPosition = 0;
                break;
            case 24:
                modeInfo->RedMaskSize = 8;
                modeInfo->RedFieldPosition = 16;
                modeInfo->GreenMaskSize = 8;
                modeInfo->GreenFieldPosition = 8;
                modeInfo->BlueMaskSize = 8;
                modeInfo->BlueFieldPosition = 0;
                modeInfo->RsvdMaskSize = 0;
                modeInfo->RsvdFieldPosition = 0;
                break;
            }
        }

    /* Convert the 32k direct color modes of VBE 1.2+ BIOSes to
     * be recognised as 15 bits per pixel modes.
     */
    if (bits == 16 && modeInfo->RsvdMaskSize == 1)
        modeInfo->BitsPerPixel = 15;
	return true;
}

long VBEAPI VBE_getPageSize(VBE_modeInfo *mi)
/****************************************************************************
*
* Function:     VBE_getPageSize
* Parameters:   mi  - Pointer to mode information block
* Returns:      Caculated page size in bytes rounded to correct boundary
*
* Description:  Computes the page size in bytes for the specified mode
*               information block, rounded up to the appropriate boundary
*               (8k, 16k, 32k or 64k). Pages >= 64k in size are always
*               rounded to the nearest 64k boundary (so the start of a
*               page is always bank aligned).
*
****************************************************************************/
{
    long size;

    size = (long)mi->BytesPerScanLine * (long)mi->YResolution;
    if (mi->BitsPerPixel == 4) {
        /* We have a 16 color video mode, so round up the page size to
         * 8k, 16k, 32k or 64k boundaries depending on how large it is.
         */

        size = (size + 0x1FFFL) & 0xFFFFE000L;
        if (size != 0x2000) {
            size = (size + 0x3FFFL) & 0xFFFFC000L;
            if (size != 0x4000) {
                size = (size + 0x7FFFL) & 0xFFFF8000L;
                if (size != 0x8000)
                    size = (size + 0xFFFFL) & 0xFFFF0000L;
                }
            }
        }
    else size = (size + 0xFFFFL) & 0xFFFF0000L;
    return size;
}

ibool VBEAPI VBE_setVideoModeExt(int mode,VBE_CRTCInfo *crtc)
/****************************************************************************
*
* Function:     VBE_setVideoModeExt
* Parameters:   mode    - SuperVGA video mode to set.
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode. This version
*				includes support for the VBE/Core 3.0 refresh rate control
*				mechanism.
*
****************************************************************************/
{
	RMREGS  regs;

	if (VBEVersion < 0x200 && mode < 0x100) {
        /* Some VBE implementations barf terribly if you try to set non-VBE
         * video modes with the VBE set mode call. VBE 2.0 implementations
         * must be able to handle this.
         */
        regs.h.al = (ushort)mode;
        regs.h.ah = 0;
        PM_int86(0x10,&regs,&regs);
        }
	else {
		if (VBEVersion < 0x300 && (mode & vbeRefreshCtrl))
			return false;
        regs.x.ax = 0x4F02;
		regs.x.bx = (ushort)mode;
		if ((mode & vbeRefreshCtrl) && crtc)
			VBE_callESDI(&regs, crtc, sizeof(*crtc));
		else
			PM_int86(0x10,&regs,&regs);
        if (regs.x.ax != VBE_SUCCESS)
            return false;
        }
    return true;
}

ibool VBEAPI VBE_setVideoMode(int mode)
/****************************************************************************
*
* Function:     VBE_setVideoMode
* Parameters:   mode    - SuperVGA video mode to set.
* Returns:      True if the mode was set, false if not.
*
* Description:  Attempts to set the specified video mode.
*
****************************************************************************/
{
	return VBE_setVideoModeExt(mode,NULL);
}

int VBEAPI VBE_getVideoMode(void)
/****************************************************************************
*
* Function:     VBE_getVideoMode
* Returns:      Current video mode
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F03;
    PM_int86(0x10,&regs,&regs);
    if (regs.x.ax != VBE_SUCCESS)
        return -1;
    return regs.x.bx;
}

ibool VBEAPI VBE_setBank(int window,int bank)
/****************************************************************************
*
* Function:     VBE_setBank
* Parameters:   window  - Window to set
*               bank    - Bank number to set window to
* Returns:      True on success, false on failure.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F05;
    regs.h.bh = 0;
    regs.h.bl = window;
    regs.x.dx = bank;
    PM_int86(0x10,&regs,&regs);
    return regs.x.ax == VBE_SUCCESS;
}

int VBEAPI VBE_getBank(int window)
/****************************************************************************
*
* Function:     VBE_setBank
* Parameters:   window  - Window to read
* Returns:      Bank number for the window (-1 on failure)
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F05;
    regs.h.bh = 1;
    regs.h.bl = window;
    PM_int86(0x10,&regs,&regs);
    if (regs.x.ax != VBE_SUCCESS)
        return -1;
    return regs.x.dx;
}

ibool VBEAPI VBE_setPixelsPerLine(int pixelsPerLine,int *newBytes,
    int *newPixels,int *maxScanlines)
/****************************************************************************
*
* Function:     VBE_setPixelsPerLine
* Parameters:   pixelsPerLine   - Pixels per scanline
*               newBytes        - Storage for bytes per line value set
*               newPixels       - Storage for pixels per line value set
*               maxScanLines    - Storage for maximum number of scanlines
* Returns:      True on success, false on failure
*
* Description:  Sets the scanline length for the video mode to the specified
*               number of pixels per scanline. If you need more granularity
*               in TrueColor modes, use the VBE_setBytesPerLine routine
*               (only valid for VBE 2.0).
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 0;
    regs.x.cx = pixelsPerLine;
    PM_int86(0x10,&regs,&regs);
    *newBytes = regs.x.bx;
    *newPixels = regs.x.cx;
    *maxScanlines = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_setBytesPerLine(int bytesPerLine,int *newBytes,
    int *newPixels,int *maxScanlines)
/****************************************************************************
*
* Function:     VBE_setBytesPerLine
* Parameters:   pixelsPerLine   - Pixels per scanline
*               newBytes        - Storage for bytes per line value set
*               newPixels       - Storage for pixels per line value set
*               maxScanLines    - Storage for maximum number of scanlines
* Returns:      True on success, false on failure
*
* Description:  Sets the scanline length for the video mode to the specified
*               number of bytes per scanline (valid for VBE 2.0 only).
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 2;
    regs.x.cx = bytesPerLine;
    PM_int86(0x10,&regs,&regs);
    *newBytes = regs.x.bx;
    *newPixels = regs.x.cx;
    *maxScanlines = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_getScanlineLength(int *bytesPerLine,int *pixelsPerLine,
    int *maxScanlines)
/****************************************************************************
*
* Function:     VBE_getScanlineLength
* Parameters:   bytesPerLine    - Storage for bytes per scanline
*               pixelsPerLine   - Storage for pixels per scanline
*               maxScanLines    - Storage for maximum number of scanlines
* Returns:      True on success, false on failure
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 1;
    PM_int86(0x10,&regs,&regs);
    *bytesPerLine = regs.x.bx;
    *pixelsPerLine = regs.x.cx;
    *maxScanlines = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_getMaxScanlineLength(int *maxBytes,int *maxPixels)
/****************************************************************************
*
* Function:     VBE_getMaxScanlineLength
* Parameters:   maxBytes    - Maximum scanline width in bytes
*               maxPixels   - Maximum scanline width in pixels
* Returns:      True if successful, false if function failed
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F06;
    regs.h.bl = 3;
    PM_int86(0x10,&regs,&regs);
    *maxBytes = regs.x.bx;
    *maxPixels = regs.x.cx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_setDisplayStart(int x,int y,ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setDisplayStart
* Parameters:   x,y 	- Position of the first pixel to display
*				waitVRT	- True to wait for retrace, false if not
* Returns:      True if function was successful.
*
* Description:  Sets the new starting display position to implement
*               hardware scrolling.
*
****************************************************************************/
{
	RMREGS  regs;

	regs.x.ax = 0x4F07;
	if (waitVRT)
		regs.x.bx = 0x80;
	else regs.x.bx = 0x00;
	regs.x.cx = x;
	regs.x.dx = y;
	PM_int86(0x10,&regs,&regs);
	return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_getDisplayStart(int *x,int *y)
/****************************************************************************
*
* Function:     VBE_getDisplayStart
* Parameters:   x,y - Place to store starting address value
* Returns:      True if function was successful.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F07;
    regs.x.bx = 0x01;
    PM_int86(0x10,&regs,&regs);
    *x = regs.x.cx;
    *y = regs.x.dx;
    return regs.x.ax == VBE_SUCCESS;
}

ibool VBEAPI VBE_setDisplayStartAlt(ulong startAddr,ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setDisplayStartAlt
* Parameters:   startAddr	- 32-bit starting address in display memory
*               waitVRT		- True to wait for vertical retrace, false if not
* Returns:      True if function was successful, false if not supported.
*
* Description:  Sets the new starting display position to the specified
*               32-bit display start address. Note that this function is
*				different the the version above, since it takes a 32-bit
*				byte offset in video memory as the starting address which
*				gives the programmer maximum control over the stat address.
*
*				NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
	RMREGS  regs;

	if (VBEVersion >= 0x300) {
		regs.x.ax = 0x4F07;
		regs.x.bx = waitVRT ? 0x82 : 0x02;
		regs.e.ecx = startAddr;
#ifdef	__WINDOWS32__
		WD_int386(0x10,&regs,&regs);
#else
		PM_int86(0x10,&regs,&regs);
#endif
		return regs.x.ax == VBE_SUCCESS;
		}
	return false;
}

int	VBEAPI VBE_getDisplayStartStatus(void)
/****************************************************************************
*
* Function:     VBE_getDisplayStartStatus
* Returns:      0 if last flip not occurred, 1 if already flipped
*				-1 if not supported
*
* Description:  Returns the status of the previous display start request.
*				If this function is supported the programmer can implement
*				hardware triple buffering using this function.
*
*				NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
	RMREGS  regs;

	if (VBEVersion >= 0x300) {
		regs.x.ax = 0x4F07;
		regs.x.bx = 0x0004;
		PM_int86(0x10,&regs,&regs);
		if (regs.x.ax == VBE_SUCCESS)
			return (regs.x.cx != 0);
		}
	return -1;
}

ibool VBEAPI VBE_enableStereoMode(void)
/****************************************************************************
*
* Function:     VBE_enableStereoMode
* Returns:      True if stereo mode enabled, false if not supported.
*
* Description:  Puts the system into hardware stereo mode for LC shutter
*				glasses, where the display swaps between two display start
*				addresses every vertical retrace.
*
*				NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
	RMREGS  regs;

	if (VBEVersion >= 0x300) {
		regs.x.ax = 0x4F07;
		regs.x.bx = 0x0005;
		PM_int86(0x10,&regs,&regs);
		return regs.x.ax == VBE_SUCCESS;
		}
	return false;
}

ibool VBEAPI VBE_disableStereoMode(void)
/****************************************************************************
*
* Function:     VBE_disableStereoMode
* Returns:      True if stereo mode disabled, false if not supported.
*
* Description:	Puts the system back into normal, non-stereo display mode
*				after having stereo mode enabled.
*
*				NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
	RMREGS  regs;

	if (VBEVersion >= 0x300) {
		regs.x.ax = 0x4F07;
		regs.x.bx = 0x0006;
		PM_int86(0x10,&regs,&regs);
		return regs.x.ax == VBE_SUCCESS;
		}
	return false;
}

ibool VBEAPI VBE_setStereoDisplayStart(ulong leftAddr,ulong rightAddr,
	ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setStereoDisplayStart
* Parameters:   leftAddr	- 32-bit start address for left image
* 				rightAddr	- 32-bit start address for right image
*               waitVRT		- True to wait for vertical retrace, false if not
* Returns:      True if function was successful, false if not supported.
*
* Description:  Sets the new starting display position to the specified
*               32-bit display start address. Note that this function is
*				different the the version above, since it takes a 32-bit
*				byte offset in video memory as the starting address which
*				gives the programmer maximum control over the stat address.
*
*				NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
	RMREGS  regs;

	if (VBEVersion >= 0x300) {
		regs.x.ax = 0x4F07;
		regs.x.bx = waitVRT ? 0x83 : 0x03;
		regs.e.ecx = leftAddr;
		regs.e.edx = rightAddr;
#ifdef	__WINDOWS32__
		WD_int386(0x10,&regs,&regs);
#else
		PM_int86(0x10,&regs,&regs);
#endif
		return regs.x.ax == VBE_SUCCESS;
		}
	return false;
}

ulong VBEAPI VBE_getClosestClock(ushort mode,ulong pixelClock)
/****************************************************************************
*
* Function:     VBE_getClosestClock
* Parameters:	mode		- VBE mode to be used (include vbeLinearBuffer)
*				pixelClock	- Desired pixel clock
* Returns:      Closest pixel clock to desired clock (-1 if not supported)
*
* Description:  Calls the VBE/Core 3.0 interface to determine the closest
*				pixel clock to the requested value. The BIOS will always
*				search for a pixel clock that is no more than 1% below the
*				requested clock or somewhere higher than the clock. If the
*				clock is higher note that it may well be many Mhz higher
*				that requested and the application will have to check that
*				the returned value is suitable for it's needs. This function
*				returns the actual pixel clock that will be programmed by
*				the hardware.
*
*				Note that if the pixel clock will be used with a linear
*				framebuffer mode, make sure you pass in the linear
*				framebuffer flag to this function.
*
*				NOTE: Requires VBE/Core 3.0
*
****************************************************************************/
{
	RMREGS  regs;

	if (VBEVersion >= 0x300) {
		regs.x.ax = 0x4F0B;
		regs.h.bl = 0x00;
		regs.e.ecx = pixelClock;
		regs.x.dx = mode;
#ifdef	__WINDOWS32__
		WD_int386(0x10,&regs,&regs);
#else
		PM_int86(0x10,&regs,&regs);
#endif
		if (regs.x.ax == VBE_SUCCESS)
			return regs.e.ecx;
		}
	return -1;
}

ibool VBEAPI VBE_setDACWidth(int width)
/****************************************************************************
*
* Function:     VBE_setDACWidth
* Parameters:   width   - Width to set the DAC to
* Returns:      True on success, false on failure
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F08;
    regs.h.bl = 0x00;
    regs.h.bh = width;
    PM_int86(0x10,&regs,&regs);
    return regs.x.ax == VBE_SUCCESS;
}

int VBEAPI VBE_getDACWidth(void)
/****************************************************************************
*
* Function:     VBE_getDACWidth
* Returns:      Current width of the palette DAC
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F08;
    regs.h.bl = 0x01;
    PM_int86(0x10,&regs,&regs);
    if (regs.x.ax != VBE_SUCCESS)
        return -1;
    return regs.h.bh;
}

ibool VBEAPI VBE_setPalette(int start,int num,VBE_palette *pal,ibool waitVRT)
/****************************************************************************
*
* Function:     VBE_setPalette
* Parameters:   start   - Starting palette index to program
*               num     - Number of palette indexes to program
*               pal     - Palette buffer containing values
*               waitVRT - Wait for vertical retrace flag
* Returns:      True on success, false on failure
*
* Description:  Sets a block of palette registers by calling the VBE 2.0
*               BIOS. This function will fail on VBE 1.2 implementations.
*
****************************************************************************/
{
    RMREGS  regs;

    regs.x.ax = 0x4F09;
    regs.h.bl = waitVRT ? 0x80 : 0x00;
    regs.x.cx = num;
    regs.x.dx = start;
    VBE_callESDI(&regs, pal, sizeof(VBE_palette) * num);
    return regs.x.ax == VBE_SUCCESS;
}

void * VBEAPI VBE_getBankedPointer(VBE_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     VBE_getBankedPointer
* Parameters:   modeInfo    - Mode info block for video mode
* Returns:      Selector to the linear framebuffer (0 on failure)
*
* Description:  Returns a near pointer to the VGA framebuffer area.
*
****************************************************************************/
{
    /* We just map the pointer every time, since the pointer will always
     * be in real mode memory, so we wont actually be mapping any real
     * memory.
     *
     * NOTE: We cannot currently map a near pointer to the banked frame
     *       buffer for Watcom Win386, so we create a 16:16 far pointer to
     *       the video memory. All the assembler code will render to the
     *       video memory by loading the selector rather than using a
     *       near pointer.
     */
    ulong seg = (ushort)modeInfo->WinASegment;
	if (seg != 0) {
#ifdef  __WIN386__
		return (void*)((ulong)PM_createSelector(seg << 4,0xFFFF) << 16);
#else
		if (seg == 0xA000)
			return (void*)PM_getA0000Pointer();
		else
			return (void*)PM_mapPhysicalAddr(seg << 4,0xFFFF);
#endif
		}
    return NULL;
}

#ifndef REALMODE
#ifdef  __WINDOWS16__
ushort VBEAPI VBE_getLinearSelector(VBE_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     VBE_getLinearSelector
* Parameters:   modeInfo    - Mode info block for video mode
* Returns:      Selector to the linear framebuffer (0 on failure)
*
* Description:  Returns a selector to the linear framebuffer for the video
*               mode. Because the linear framebuffer does not change
*               location, we simply cache the first selector created for
*               the life of the application. The selector is created to
*               be 4Mb in length, regardless of the size of the linear
*               framebuffer.
*
****************************************************************************/
{
	static ushort linSel8 = 0;
	static ushort linSel15 = 0;
	static ushort linSel16 = 0;
	static ushort linSel24 = 0;
	static ushort linSel32 = 0;
	switch (modeInfo->BitsPerPixel) {
		case 8:
			if (!linSel8)
				linSel8 = PM_createSelector(modeInfo->PhysBasePtr,(4096 * 1024L)-1);
			return linSel8;
		case 15:
			if (!linSel15)
				linSel15 = PM_createSelector(modeInfo->PhysBasePtr,(4096 * 1024L)-1);
			return linSel15;
		case 16:
			if (!linSel16)
				linSel16 = PM_createSelector(modeInfo->PhysBasePtr,(4096 * 1024L)-1);
			return linSel16;
		case 24:
			if (!linSel24)
				linSel24 = PM_createSelector(modeInfo->PhysBasePtr,(4096 * 1024L)-1);
			return linSel24;
		case 32:
			if (!linSel32)
				linSel32 = PM_createSelector(modeInfo->PhysBasePtr,(4096 * 1024L)-1);
			return linSel32;
		}
	return 0;
}
#endif

void * VBEAPI VBE_getLinearPointer(VBE_modeInfo *modeInfo)
/****************************************************************************
*
* Function:     VBE_getLinearPointer
* Parameters:   modeInfo    - Mode info block for video mode
* Returns:      Selector to the linear framebuffer (0 on failure)
*
* Description:  Returns a near pointer to the linear framebuffer for the video
*               mode.
*
****************************************************************************/
{
	static void *linPtr8 = NULL;
	static void *linPtr15 = NULL;
	static void *linPtr16 = NULL;
	static void *linPtr24 = NULL;
	static void *linPtr32 = NULL;
	switch (modeInfo->BitsPerPixel) {
		case 8:
			if (!linPtr8)
				linPtr8 = PM_mapPhysicalAddr(modeInfo->PhysBasePtr,(VBEMemory * 1024L)-1);
			return linPtr8;
		case 15:
			if (!linPtr15)
				linPtr15 = PM_mapPhysicalAddr(modeInfo->PhysBasePtr,(VBEMemory * 1024L)-1);
			return linPtr15;
		case 16:
			if (!linPtr16)
				linPtr16 = PM_mapPhysicalAddr(modeInfo->PhysBasePtr,(VBEMemory * 1024L)-1);
			return linPtr16;
		case 24:
			if (!linPtr24)
				linPtr24 = PM_mapPhysicalAddr(modeInfo->PhysBasePtr,(VBEMemory * 1024L)-1);
			return linPtr24;
		case 32:
			if (!linPtr32)
				linPtr32 = PM_mapPhysicalAddr(modeInfo->PhysBasePtr,(VBEMemory * 1024L)-1);
			return linPtr32;
		}
	return NULL;
}

#ifdef  __WINDOWS16__
#include <dos.h>

/* The following is a small 32 bit protected mode code stub that will
 * call the associated VBE 2.0 32 bit near function (within the same
 * 64K segment) and then perform a RETF to return to the calling 16 bit
 * protected mode code.
 */

static char code32stub[] = {
    0x2E,0xFF,0x15,0x00,0x00,0x00,0x00, /*  call    near [cs:addr]  */
    0x66,0xCB,                          /*  16:16 retf              */
    0x00,0x00,0x00,0x00                 /*  [addr]                  */
    };

static void InitPMCode(void)
/****************************************************************************
*
* Function:     InitPMCode  - 16 bit protected mode version
*
* Description:  Finds the address of and relocates the protected mode
*               code block from the VBE 2.0 into a local memory block. The
*               memory block is allocated with malloc() and must be freed
*               with VBE_freePMCode() after graphics processing is complete.
*
*               Note that this buffer _must_ be recopied after each mode set,
*               as the routines will change depending on the underlying
*               video mode.
*
****************************************************************************/
{
    RMREGS      regs;
    RMSREGS     sregs;
    uint        sel,off;
    char        *p;

	if (!pmInfo && VBEVersion >= 0x200) {
        regs.x.ax = 0x4F0A;
        regs.x.bx = 0;
        PM_int86x(0x10,&regs,&regs,&sregs);
        if (regs.x.ax != VBE_SUCCESS)
            return;
        if ((pmInfo = malloc(regs.x.cx + sizeof(code32stub)*3)) == NULL)
            return;
        if ((pmInfo32 = malloc(regs.x.cx)) == NULL)
            return;

        /* Relocate the block into our local data segment */
        PM_mapRealPointer(&sel,&off,sregs.es,regs.x.di);
        PM_memcpynf(pmInfo,sel,off,regs.x.cx);
        PM_memcpynf(pmInfo32,sel,off,regs.x.cx);

        /* Copy the three 16 bit code stubs to the end of the code segment
         * and plug in the values to call the appropriate routines. Then
         * fix up the pmInfo offsets to point to these routines, not the
         * 32 bit near functions.
         */
        p = (char*)pmInfo + regs.x.cx;
        *((ushort*)&code32stub[3]) = FP_OFF(p) + 9;
        *((ulong*)&code32stub[9]) = FP_OFF(pmInfo) + pmInfo->setWindow;
        memcpy(p,code32stub,sizeof(code32stub));
        pmInfo->setWindow = FP_OFF(p) - FP_OFF(pmInfo);

        p += sizeof(code32stub);
        *((ushort*)&code32stub[3]) = FP_OFF(p) + 9;
        *((ulong*)&code32stub[9]) = FP_OFF(pmInfo) + pmInfo->setDisplayStart;
        memcpy(p,code32stub,sizeof(code32stub));
        pmInfo->setDisplayStart = FP_OFF(p) - FP_OFF(pmInfo);

        p += sizeof(code32stub);
        *((ushort*)&code32stub[3]) = FP_OFF(p) + 9;
        *((ulong*)&code32stub[9]) = FP_OFF(pmInfo) + pmInfo->setPalette;
        memcpy(p,code32stub,sizeof(code32stub));
        pmInfo->setPalette = FP_OFF(p) - FP_OFF(pmInfo);

        /* Now create a 32 bit code segment alias descriptor for the VBE
         * 2.0 32 bit code so we can call it directly from 16 bit PM code
         */
        code32sel = PM_createCode32Alias(FP_SEG(pmInfo));

        /* Read the IO priveledge info and determine if we need to
         * pass a selector to MMIO registers to the bank switch code.
         * Application code can simply check the value of VBE_MMIOSel
         * and if this is non-zero then you must pass this value in ES
         * when you call the bank switch code.
         */
        if (pmInfo->IOPrivInfo && !VBE_MMIOSel) {
            ushort *p = (ushort*)((uchar*)pmInfo + pmInfo->IOPrivInfo);
            while (*p != 0xFFFF)
                p++;
            p++;
            if (*p != 0xFFFF) {
                /* We have an memory mapped IO location listed, which
                 * we need to use for mapping the memory mapped
                 * registers
                 */
                ulong base = *((ulong*)p);
				ushort len = *(p+2)-1;
				VBE_MMIOSel = PM_createSelector(base,len);
                }
            }
        }
}

#else

static void InitPMCode(void)
/****************************************************************************
*
* Function:     InitPMCode  - 32 bit protected mode version
*
* Description:  Finds the address of and relocates the protected mode
*               code block from the VBE 2.0 into a local memory block. The
*               memory block is allocated with malloc() and must be freed
*               with VBE_freePMCode() after graphics processing is complete.
*
*               Note that this buffer _must_ be recopied after each mode set,
*               as the routines will change depending on the underlying
*               video mode.
*
****************************************************************************/
{
    RMREGS      regs;
    RMSREGS     sregs;
    uint        sel,off;

	if (!pmInfo && VBEVersion >= 0x200) {
        regs.x.ax = 0x4F0A;
        regs.x.bx = 0;
        PM_int86x(0x10,&regs,&regs,&sregs);
        if (regs.x.ax != VBE_SUCCESS)
            return;
        if ((pmInfo = malloc(regs.x.cx)) == NULL)
            return;
        pmInfo32 = pmInfo;

        /* Relocate the block into our local data segment */
        PM_mapRealPointer(&sel,&off,sregs.es,regs.x.di);
        PM_memcpynf(pmInfo,sel,off,regs.x.cx);

        /* Read the IO priveledge info and determine if we need to
         * pass a selector to MMIO registers to the bank switch code.
         * Application code can simply check the value of VBE_MMIOSel
         * and if this is non-zero then you must pass this value in ES
         * when you call the bank switch code.
         */
        if (pmInfo->IOPrivInfo && !VBE_MMIOSel) {
            ushort *p = (ushort*)((uchar*)pmInfo + pmInfo->IOPrivInfo);
            while (*p != 0xFFFF)
                p++;
            p++;
            if (*p != 0xFFFF) {
                /* We have an memory mapped IO location listed, which
                 * we need to use for mapping the memory mapped
				 * registers. Note that we simply allocate a selector to
				 * a 512kb region of memory, since the stupid VBE 2.0 spec
				 * only defines a maximum length of 64Kb for the selector.
				 * 512Kb should cover all known graphics hardware.
                 */
				ulong base = *((ulong*)p);
				VBE_MMIOSel = PM_createSelector(base,(512 * 1024L)-1);
				}
            }
        }
}
#endif

void * VBEAPI VBE_getSetBank(void)
/****************************************************************************
*
* Function:     VBE_getSetBank
* Returns:      Pointer to the 32 VBE 2.0 bit bank switching routine.
*
****************************************************************************/
{
	if (VBEVersion >= 0x200) {
        InitPMCode();
        if (pmInfo)
#ifdef  __WINDOWS16__
            return MK_FP(code32sel,FP_OFF(pmInfo) + pmInfo->setWindow);
#else
            return (uchar*)pmInfo + pmInfo->setWindow;
#endif
        }
    return NULL;
}

void * VBEAPI VBE_getSetDisplayStart(void)
/****************************************************************************
*
* Function:     VBE_getSetDisplayStart
* Returns:      Pointer to the 32 VBE 2.0 bit CRT start address routine.
*
****************************************************************************/
{
	if (VBEVersion >= 0x200) {
        InitPMCode();
        if (pmInfo)
#ifdef  __WINDOWS16__
            return MK_FP(code32sel,FP_OFF(pmInfo) + pmInfo->setDisplayStart);
#else
            return (uchar*)pmInfo + pmInfo->setDisplayStart;
#endif
        }
    return NULL;
}

void * VBEAPI VBE_getSetPalette(void)
/****************************************************************************
*
* Function:     VBE_getSetPalette
* Returns:      Pointer to the 32 VBE 2.0 bit palette programming routine.
*
****************************************************************************/
{
	if (VBEVersion >= 0x200) {
        InitPMCode();
        if (pmInfo)
#ifdef  __WINDOWS16__
            return MK_FP(code32sel,FP_OFF(pmInfo) + pmInfo->setPalette);
#else
            return (uchar*)pmInfo + pmInfo->setPalette;
#endif
        }
    return NULL;
}

void VBEAPI VBE_freePMCode(void)
/****************************************************************************
*
* Function:     VBE_freePMCode
*
* Description:  This routine frees the protected mode code blocks that
*               we copied from the VBE 2.0 interface. This routine must
*               be after you have finished graphics processing to free up
*               the memory occupied by the routines. This is necessary
*               because the PM info memory block must be re-copied after
*               every video mode set from the VBE 2.0 implementation.
*
****************************************************************************/
{
    if (pmInfo) {
        free(pmInfo);
        pmInfo = NULL;
#ifdef  __WINDOWS16__
        free(pmInfo32);
        PM_freeSelector(code32sel);
#endif
		}
}

/* Set of code stubs used to build the final bank switch code */

#define VBE12_adjustOffset	12

#ifndef	__WINDOWS__
static uchar VBE12A_bankFunc32[] = {
	0x53,0x51,                  /*  push    ebx,ecx     */
	0x8B,0xD0,                  /*  mov     edx,eax     */
	0xB8,0x05,0x4F,0x00,0x00,   /*  mov		eax,4F05h	*/
	0x33,0xDB,                  /*  xor     ebx,ebx     */
	0xB1,0x00,                  /*  mov     cl,0        */
	0xD2,0xE2,                  /*  shl     dl,cl       */
	0xCD,0x10,                  /*  int		10h			*/
	0x59,0x5B,                  /*  pop     ecx,ebx     */
	};

static uchar VBE12AB_bankFunc32[] = {
	0x53,0x51,                  /*  push    ebx,ecx     */
	0x8B,0xD0,                  /*  mov     edx,eax     */
	0xB8,0x05,0x4F,0x00,0x00,   /*  mov		eax,4F05h	*/
	0x33,0xDB,                  /*  xor     ebx,ebx     */
	0xB1,0x00,                  /*  mov     cl,0        */
	0xD2,0xE2,                  /*  shl     dl,cl       */
	0x52,                  		/*  push	edx			*/
	0xCD,0x10,                  /*  int		10h			*/
	0x5A,                  		/*  pop		edx			*/
	0xB8,0x05,0x4F,0x00,0x00,   /*  mov		eax,4F05h	*/
	0xBB,0x01,0x00,0x00,0x00,   /*  mov		ebx,1		*/
	0xCD,0x10,                  /*  int		10h			*/
	0x59,0x5B,                  /*  pop     ecx,ebx     */
	};
#endif    

#define VBE20_adjustOffset  7

static uchar VBE20A_bankFunc32_Start[] = {
	0x53,0x51,                  /*  push    ebx,ecx     */
	0x8B,0xD0,                  /*  mov     edx,eax     */
	0x33,0xDB,                  /*  xor     ebx,ebx     */
	0xB1,0x00,                  /*  mov     cl,0        */
	0xD2,0xE2,                  /*  shl     dl,cl       */
    };

static uchar VBE20_bankFunc32_End[] = {
    0x59,0x5B,                  /*  pop     ecx,ebx     */
    };

static uchar bankFunc32[100];

#define copy(p,b,a) memcpy(b,a,sizeof(a)); (p) = (b) + sizeof(a)

ibool VBEAPI VBE_getBankFunc32(int *codeLen,void **bankFunc,int dualBanks,
    int bankAdjust)
/****************************************************************************
*
* Function:     VBE_getBankFunc32
* Parameters:   codeLen     - Place to store length of code
*               bankFunc    - Place to store pointer to bank switch code
*               dualBanks   - True if dual banks are in effect
*               bankAdjust  - Bank shift adjustment factor
* Returns:      True on success, false if not compatible.
*
* Description:  Creates a local 32 bit bank switch function from the
*               VBE 2.0 bank switch code that is compatible with the
*               virtual flat framebuffer devices (does not have a return
*               instruction at the end and takes the bank number in EAX
*               not EDX). Note that this 32 bit code cannot include int 10h
*               instructions, so we can only do this if we have VBE 2.0
*               or later.
*
*               Note that we need to know the length of the 32 bit
*               bank switch function, which the standard VBE 2.0 spec
*               does not provide. In order to support this we have
*               extended the VBE 2.0 pmInfo structure in UniVBE 5.2 in a
*               way to support this, and we hope that this will become
*               a VBE 2.0 ammendment.
*
*               Note also that we cannot run the linear framebuffer
*               emulation code with bank switching routines that require
*               a selector to the memory mapped registers passed in ES.
*
****************************************************************************/
{
    int     len;
    uchar   *code;
    uchar   *p;

	InitPMCode();
	if (VBEVersion >= 0x200 && pmInfo32 && !VBE_MMIOSel) {
        code = (uchar*)pmInfo32 + pmInfo32->setWindow;
        if (pmInfo32->extensionSig == VBE20_EXT_SIG)
			len = pmInfo32->setWindowLen-1;
        else {
            /* We are running on a system without the UniVBE 5.2 extension.
             * We do as best we can by scanning through the code for the
             * ret function to determine the length. This is not foolproof,
             * but is the best we can do.
             */
            p = code;
            while (*p != 0xC3)
                p++;
            len = p - code;
            }
        copy(p,bankFunc32,VBE20A_bankFunc32_Start);
        memcpy(p,code,len);
        p += len;
		copy(p,p,VBE20_bankFunc32_End);
        *codeLen = p - bankFunc32;
        bankFunc32[VBE20_adjustOffset] = (uchar)bankAdjust;
        *bankFunc = bankFunc32;
        return true;
		}
#ifndef	__WINDOWS__
	else {
		/* Set up for virtual linear framebuffering with VBE 1.2 devices
		 * if we are not building for Windows.
		 */
		if (dualBanks) {
			VBE12AB_bankFunc32[VBE12_adjustOffset] = (uchar)bankAdjust;
			*codeLen = sizeof(VBE12AB_bankFunc32);
			*bankFunc = VBE12AB_bankFunc32;
			}
		else {
			VBE12A_bankFunc32[VBE12_adjustOffset] = (uchar)bankAdjust;
			*codeLen = sizeof(VBE12A_bankFunc32);
			*bankFunc = VBE12A_bankFunc32;
			}
		return true;
		}
#else
	return false;
#endif
}

#endif
