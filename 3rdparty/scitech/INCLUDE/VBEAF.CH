/****************************************************************************
*
*                VESA BIOS Extensions/Accelerator Functions
*                               Version 2.0 DRAFT
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
* Developed by: SciTech Software
*
* Language:     ANSI C
* Environment:  IBM PC 32 bit Protected Mode.
*
* Description:  C module for the Graphics Acclerator Driver API. Uses
*               the SciTech PM/Pro library for interfacing with DOS
*               extender specific functions.
*
*
****************************************************************************/

#include "vbeaf.h"
#include "pmode.h"
#ifndef	__WIN32_VXD__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#if !defined(__16BIT__) || defined(TESTING)

#ifndef _MAX_PATH
#define _MAX_PATH   255
#endif

#if	defined(VTOOLDS) || defined(__WIN32_VXD__)
#define NO_FILEIO
#define	NO_GETENV
#endif

/*------------------------- Global Variables ------------------------------*/

#define AF_DRIVERDIR    AF_DRIVERDIR_DOS

static ulong		AF_size = 0;
static int          status = afOK;
static void         *IOMemMaps[4] = {NULL,NULL,NULL,NULL};
static void         *BankedMem = NULL;
static void         *LinearMem = NULL;
static ibool			loadedByUs = true;

/* Backwards compatibility selectors for VBE/AF 1.0 drivers */

static AF_int32     Sel0000h = 0;
static AF_int32     Sel0040h = 0;
static AF_int32     SelA000h = 0;
static AF_int32     SelB000h = 0;
static AF_int32     SelC000h = 0;

/* Offsets to VBE/AF 1.0 selectors in res2[] array */

#define _Sel0000h	0
#define _Sel0040h	1
#define _SelA000h	2
#define _SelB000h	3
#define _SelC000h   4

#ifndef	__WIN32_VXD__
#define SDDHELP_DeviceID    	0x3DF8
#define	SDDHELP_NAME			"SDDHELP.VXD"
#define	SDDHELP_MODULE			"SDDHELP"
#define	SDDHELP_VXD_NAME		"\\\\.\\" SDDHELP_NAME
#define	SDDHELP_VXD_MODULE		"\\\\.\\" SDDHELP_MODULE
#define	SDDHELP_GETVER			0x0000
#define	SDDHELP_GETDRV			0x000D
#define	SDDHELP_DRVSETMODE		0x0100
#define	SDDHELP_DRVRESETMODE	0x0101
#define	SDDHELP_DRV32SETMODE	0x0102
#define	SDDHELP_GETVER32		(0x80000000UL | SDDHELP_GETVER)
#define	SDDHELP_GETDRV32		(0x80000000UL | SDDHELP_GETDRV)
#define	SDDHELP_DRVSETMODE32	(0x80000000UL | SDDHELP_DRVSETMODE)
#define	SDDHELP_DRVRESETMODE32	(0x80000000UL | SDDHELP_DRVRESETMODE)

#ifdef	__WINDOWS32__
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
static HANDLE	hDevice = NULL;
#else
#include <dos.h>
static uint		SDDHelpOff = 0;
static uint		SDDHelpSel = 0;

/* Register structure passed to VDDCall function */

typedef struct {
	long	eax;
	long	ebx;
	long	ecx;
	long	edx;
	long	esi;
	long	edi;
	short	ds,es;
	} VXD_regs;

/* Parameter block for AF_setVideoMode call to VxD */

typedef struct {
	long		device;
	ulong		mode;
	long		virtualX;
	long		virtualY;
	long		_FAR_ *bytesPerLine;
	long		numBuffers;
	AF_CRTCInfo	_FAR_ *crtc;
	} SDDHELP_setModeParams;

/* External assembler function to call the helper VxD */

void _ASMAPI _AF_VxDCall(VXD_regs *regs,uint off,uint sel);
#endif
#endif

/*-------------------------- Implementation -------------------------------*/

/* Internal assembler functions */

AF_int32    _ASMAPI _AF_plugAndPlayInit(AF_devCtx *dc);
AF_int32    _ASMAPI _AF_initDriver(AF_devCtx *dc);
void        _ASMAPI _AF_int86(void);
void        _ASMAPI _AF_callRealMode(void);
void 		_ASMAPI _AF_callFuncASM(AF_DPMI_regs *r,void *funcPtr);

static void backslash(char *s)
/****************************************************************************
*
* Function:     backslash
* Parameters:   s   - String to add backslash to
*
* Description:  Appends a trailing '\' pathname separator if the string
*               currently does not have one appended.
*
****************************************************************************/
{
    uint pos = strlen(s);
    if (s[pos-1] != '\\') {
        s[pos] = '\\';
        s[pos+1] = '\0';
        }
}

void _ASMAPI _AF_callRealMode_C(AF_DPMI_regs *dregs)
/****************************************************************************
*
* Function:     _AF_callRealMode_C
* Parameters:   dregs   - Pointer to DPMI register structure
*
* Description:  Calls a real mode procedure. This does not need to be
*               speedy, so we simply convert the registers to the format
*               expected by the PM/Pro library and let it handle it.
*
****************************************************************************/
{
    RMREGS  regs;
    RMSREGS sregs;

    regs.x.ax = (short)dregs->eax;
    regs.x.bx = (short)dregs->ebx;
    regs.x.cx = (short)dregs->ecx;
    regs.x.dx = (short)dregs->edx;
    regs.x.si = (short)dregs->esi;
    regs.x.di = (short)dregs->edi;
    sregs.es = dregs->es;
    sregs.ds = dregs->ds;

    PM_callRealMode(dregs->cs,dregs->ip,&regs,&sregs);

    dregs->eax = regs.x.ax;
    dregs->ebx = regs.x.bx;
    dregs->ecx = regs.x.cx;
    dregs->edx = regs.x.dx;
    dregs->esi = regs.x.si;
    dregs->edi = regs.x.di;
    dregs->es = sregs.es;
    dregs->ds = sregs.ds;
}

void _ASMAPI _AF_int86_C(AF_int32 intno,AF_DPMI_regs *dregs)
/****************************************************************************
*
* Function:     _AF_int86_C
* Parameters:   intno   - Interrupt number to issue
*               dregs   - Pointer to DPMI register structure
*
* Description:  Issues a real mode interrupt. This does not need to be
*               speedy, so we simply convert the registers to the format
*               expected by the PM/Pro library and let it handle it.
*
****************************************************************************/
{
    RMREGS  regs;
    RMSREGS sregs;

    regs.x.ax = (short)dregs->eax;
    regs.x.bx = (short)dregs->ebx;
    regs.x.cx = (short)dregs->ecx;
    regs.x.dx = (short)dregs->edx;
    regs.x.si = (short)dregs->esi;
    regs.x.di = (short)dregs->edi;
    sregs.es = dregs->es;
    sregs.ds = dregs->ds;

    PM_int86x(intno,&regs,&regs,&sregs);

    dregs->eax = regs.x.ax;
    dregs->ebx = regs.x.bx;
    dregs->ecx = regs.x.cx;
    dregs->edx = regs.x.dx;
    dregs->esi = regs.x.si;
    dregs->edi = regs.x.di;
    dregs->es = sregs.es;
    dregs->ds = sregs.ds;
}

AF_int32 _AF_initInternal(AF_devCtx *dc)
/****************************************************************************
*
* Function:     _AF_initInternal
* Parameters:   dc  - Pointer to device context
* Returns:      Error code.
*
* Description:  Performs internal initialisation on the AF_devCtx driver
*               block, assuming that it has been loaded correctly.
*
****************************************************************************/
{
    /* Verify that the file is the driver file we are expecting */
    if (strcmp(dc->Signature,VBEAF_DRV) != 0)
		return status = afCorruptDriver;

	/* Install the device callback functions */
    dc->Int86 = _AF_int86;
	dc->CallRealMode = _AF_callRealMode;

	/* Call the plug and play initialisation function to give the driver
	 * a chance to modify the resources required for the device before we
	 * map those resourcs into the applications address space.
	 */
	if (dc->Version >= 0x200 && _AF_plugAndPlayInit(dc) != 0)
		return status = afNotDetected;

    /* Map the memory mapped register locations for the driver. We need to
     * map up to four different locations that may possibly be needed. If
     * the base address is zero then the memory does not need to be mapped.
     */
    if (IOMemMaps[0] == NULL) {
        if (dc->IOMemoryBase[0]) {
            IOMemMaps[0] = PM_mapPhysicalAddr(dc->IOMemoryBase[0],dc->IOMemoryLen[0]-1);
            if (IOMemMaps[0] == NULL)
                return status = afMemMapError;
            }
        if (dc->IOMemoryBase[1]) {
            IOMemMaps[1] = PM_mapPhysicalAddr(dc->IOMemoryBase[1],dc->IOMemoryLen[1]-1);
            if (IOMemMaps[1] == NULL)
                return status = afMemMapError;
            }
        if (dc->IOMemoryBase[2]) {
            IOMemMaps[2] = PM_mapPhysicalAddr(dc->IOMemoryBase[2],dc->IOMemoryLen[2]-1);
            if (IOMemMaps[2] == NULL)
                return status = afMemMapError;
            }
        if (dc->IOMemoryBase[3]) {
            IOMemMaps[3] = PM_mapPhysicalAddr(dc->IOMemoryBase[3],dc->IOMemoryLen[3]-1);
            if (IOMemMaps[3] == NULL)
                return status = afMemMapError;
            }
        }
    dc->IOMemMaps[0] = IOMemMaps[0];
    dc->IOMemMaps[1] = IOMemMaps[1];
    dc->IOMemMaps[2] = IOMemMaps[2];
    dc->IOMemMaps[3] = IOMemMaps[3];

	/* Map the banked video memory area for the driver */
#ifndef	__WIN386__
	if (BankedMem == NULL && dc->BankedBasePtr) {
		BankedMem = PM_mapPhysicalAddr(dc->BankedBasePtr,0xFFFF);
		if (BankedMem == NULL)
			return status = afMemMapError;
		}
#endif
    dc->BankedMem = BankedMem;

    /* Map the linear video memory area for the driver */
    if (LinearMem == NULL && dc->LinearBasePtr) {
        LinearMem = PM_mapPhysicalAddr(dc->LinearBasePtr,dc->LinearSize*1024L - 1);
        if (LinearMem == NULL)
            return status = afMemMapError;
        }
    dc->LinearMem = LinearMem;

	/* Provide selectors to important real mode segment areas. Note that
	 * VBE/AF 2.0 does not use these selectors anymore, however we still
	 * allocate them if we have a VBE/AF 1.0 driver for backwards
	 * compatibility.
	 */
#if	!defined(VTOOLSD) && !defined(__WIN32_VXD__)
	if (dc->Version < 0x200) {
		if (Sel0000h == 0) {
			Sel0000h = PM_createSelector(0x00000L,0xFFFFFL);
			Sel0040h = PM_createSelector(0x00400L,0xFFFF);
			SelA000h = PM_createSelector(0xA0000L,0xFFFF);
			SelB000h = PM_createSelector(0xB0000L,0xFFFF);
			SelC000h = PM_createSelector(0xC0000L,0xFFFF);
			}
		dc->res2[_Sel0000h] = Sel0000h;
		dc->res2[_Sel0040h] = Sel0040h;
		dc->res2[_SelA000h] = SelA000h;
		dc->res2[_SelB000h] = SelB000h;
		dc->res2[_SelC000h] = SelC000h;
		}
#endif
	return afOK;
}

#ifndef	__WIN32_VXD__
/****************************************************************************
DESCRIPTION:
Connects to the VxD and returns the version number

RETURNS:
BCD coded version number of the VxD, or 0 if not loaded (ie: 0x202 - 2.2)

REMARKS:
This function connects to the VxD (loading it if it is dynamically loadable)
and returns the version number of the VxD. You /must/ call this function
before you call any other functions in this library to open the communication
link with the helper VxD.
****************************************************************************/
static uint _ASMAPI _SDDHELP_getVersion(void)
{
#ifdef	__WINDOWS32__
	DWORD	outBuf[1];	/* Buffer to receive data from VxD	*/
	DWORD	count;		/* Count of bytes returned from VxD	*/

	/* Create a file handle for the static VxD if possible */
	hDevice = CreateFile(SDDHELP_VXD_MODULE, 0,0,0, CREATE_NEW, FILE_FLAG_DELETE_ON_CLOSE, 0);
	if (hDevice == INVALID_HANDLE_VALUE)
		return 0;

	/* Call the VxD to determine the version number */
	if (DeviceIoControl(hDevice, SDDHELP_GETVER32, NULL, 0,
			outBuf, sizeof(outBuf), &count, NULL))
		return outBuf[0];
	return 0;
#else
	PMREGS		regs;
	PMSREGS		sregs;
	VXD_regs	r;

	/* Get the static VxD entry point so we can call it */
	PM_segread(&sregs);
	regs.x.ax = 0x1684;
	regs.x.bx = SDDHELP_DeviceID;
	regs.x.di = 0;
	sregs.es = 0;
	PM_int386x(0x2F,&regs,&regs,&sregs);
	SDDHelpSel = sregs.es;
	SDDHelpOff = regs.x.di;
	if (SDDHelpSel == 0 && SDDHelpOff == 0)
		return 0;

	/* Call the VxD to determine the version number */
	memset(&r,0,sizeof(r));
	r.eax = SDDHELP_GETVER;
	_AF_VxDCall(&r,SDDHelpOff,SDDHelpSel);
	return (uint)r.eax;
#endif
}

/****************************************************************************
DESCRIPTION:
Get the current graphics driver from the VxD

PARAMETERS:
device	- Index of the graphics device to get the driver for

REMARKS:
This function returns a pointer to the common graphics driver loaded in the
helper VxD. The memory for the VxD is shared between all processes via
the VxD, so that the VxD, 16-bit code and 32-bit code all see the same
state when accessing the graphics driver.
****************************************************************************/
static AF_devCtx * _SDDHELP_getGADriver(
	int device)
{
#ifdef	__WINDOWS32__
	DWORD	inBuf[1];	/* Buffer to send data to VxD		*/
	DWORD	outBuf[2];	/* Buffer to receive data from VxD	*/
	DWORD	count;		/* Count of bytes returned from VxD	*/

	inBuf[0] = device;
	if (DeviceIoControl(hDevice, SDDHELP_GETDRV32, inBuf, sizeof(inBuf),
			outBuf, sizeof(outBuf), &count, NULL)) {
		return (AF_devCtx*)outBuf[0];
		}
	return NULL;
#else
	VXD_regs	regs;

	memset(&regs,0,sizeof(regs));
	regs.eax = SDDHELP_GETDRV;
	regs.ebx = device;
	_AF_VxDCall(&regs,SDDHelpOff,SDDHelpSel);
	if (regs.eax != 0) {
#ifdef	__WINDOWS16__
		static uint	sel = 0;

		/* Create a 16:16 far pointer for 16-bit Windows */
		if (sel)
			PM_freeSelector(sel);
		sel = PM_createSelector(regs.eax,regs.ebx);
		return (AF_devCtx*)MK_FP(sel,0);
#else
		/* Return a 32-bit linear pointer for 32-bit protected mode */
		return (AF_devCtx*)regs.eax;
#endif
		}
	return NULL;
#endif
}

/****************************************************************************
DESCRIPTION:
Tell the VxD to set a mode using the graphics driver

HEADER:
sddhelp.h

PARAMETERS:
device			- Index of the graphics device to reload set the mode for
mode			- Number of mode to set
virtualX		- Virtual X dimension for the mode if virtual scrolling
virtualY		- Virtual Y dimension for the mode if virtual scrolling
bytesPerLine	- Place to return the bytesPerLine for the mode
numBuffers		- Number of display buffers to create
crtc			- Pointer to CRTC information block if applicable

REMARKS:
This function tells the VxD to reload the graphics driver from disk, since
the configuration program has generated a new graphics driver with update
options or configuration information.
****************************************************************************/
AF_int32 AFAPI AF_setVideoMode(
	AF_devCtx *dc,
	AF_uint32 mode,
	AF_int32 virtualX,
	AF_int32 virtualY,
	AF_int32 *bytesPerLine,
	AF_int32 numBuffers,
	AF_CRTCInfo *crtc)
{
	if (loadedByUs) {
		return dc->cFuncs.SetVideoMode(dc,mode,virtualX,virtualY,
			bytesPerLine,numBuffers,crtc);
		}
	else {
#ifdef	__WINDOWS32__
		DWORD	inBuf[7];	/* Buffer to send data to VxD		*/
		DWORD	outBuf[1];	/* Buffer to receive data from VxD	*/
		DWORD	count;		/* Count of bytes returned from VxD	*/

		inBuf[0] = 0;
		inBuf[1] = mode;
		inBuf[2] = virtualX;
		inBuf[3] = virtualY;
		inBuf[4] = (ulong)bytesPerLine;
		inBuf[5] = numBuffers;
		inBuf[6] = (ulong)crtc;
		if (DeviceIoControl(hDevice, SDDHELP_DRVSETMODE32, inBuf, sizeof(inBuf),
				outBuf, sizeof(outBuf), &count, NULL)) {
			return outBuf[0];
			}
		return 0;
#else
		SDDHELP_setModeParams	p;
		VXD_regs				regs;

		memset(&regs,0,sizeof(regs));
#ifdef	__WINDOWS16__
		regs.eax = SDDHELP_DRVSETMODE;
#else
		regs.eax = SDDHELP_DRV32SETMODE;
#endif
		regs.ebx = (ulong)&p;
		p.device = 0;
		p.mode = mode;
		p.virtualX = virtualX;
		p.virtualY = virtualY;
		p.bytesPerLine = bytesPerLine;
		p.numBuffers = numBuffers;
		p.crtc = crtc;
		_AF_VxDCall(&regs,SDDHelpOff,SDDHelpSel);
		return regs.eax;
#endif
		}
}

/****************************************************************************
DESCRIPTION:
Tell the VxD to restore text mode using the loaded driver.

HEADER:
sddhelp.h

PARAMETERS:
dc			- Device context

REMARKS:
This function tells the VxD to reload the graphics driver from disk, since
the configuration program has generated a new graphics driver with update
options or configuration information.
****************************************************************************/
void AFAPI AF_restoreTextMode(
	AF_devCtx *dc)
{
	if (loadedByUs) {
		dc->cFuncs.RestoreTextMode(dc);
		}
	else {
#ifdef	__WINDOWS32__
		DWORD	inBuf[1];	/* Buffer to send data to VxD		*/
		DWORD	count;		/* Count of bytes returned from VxD	*/

		inBuf[0] = 0;
		DeviceIoControl(hDevice, SDDHELP_DRVRESETMODE32, inBuf, sizeof(inBuf),
			NULL, 0, &count, NULL);
#else
		VXD_regs	regs;

		memset(&regs,0,sizeof(regs));
		regs.eax = SDDHELP_DRVRESETMODE;
		regs.ebx = 0;
		_AF_VxDCall(&regs,SDDHelpOff,SDDHelpSel);
#endif
		}
}

#endif

AF_devCtx * AFAPI AF_loadDriver(const char *driverDir)
/****************************************************************************
*
* Function:     AF_loadDriver
* Parameters:   driverDir   - Directory to load the driver file from
* Returns:      Pointer to the loaded driver file.
*
* Description:  Loads the driver file and intialises the device context
*               ready for use. If the driver file cannot be found, or the
*               driver does not detect the installed hardware, we return
*               NULL and the application can get the status code with
*               AF_status().
*
****************************************************************************/
{
    char        filename[_MAX_PATH];
	AF_FILE  	f;
	AF_devCtx   *dc;

    /* Reset status flag */
	status = afOK;
	loadedByUs = false;

	/* TODO: This needs to be modified to look for multiple VBE/AF
	 * 		 drivers in the VBE/AF drivers directory, to enumate the PCI
	 *		 bus display devices and to match up the device information
	 *       with the available VBE/AF drivers.
	 */

	/* First try to obtain a copy of the VBE/AF driver from the resident
	 * helper VxD rather than directly loading another copy from disk. This
	 * allows us to use a single global version of the driver for everything
	 * so that driver can be used to keep track of the state of the graphics
	 * hardware correctly.
	 */
#ifndef	__WIN32_VXD__
	if (_SDDHELP_getVersion() > 0 && (dc = _SDDHELP_getGADriver(0)) != NULL) {
		/* Return a pointer to the driver loaded in the helper VxD */
		loadedByUs = false;
		return dc;
		}
#endif

	/* Try if the default operating system location first */
    strcpy(filename,AF_DRIVERDIR);
	strcat(filename,VBEAF_DRV);
	if ((f = AF_openFile(filename)) == 0) {
#ifndef	NO_GETENV
        /* Now try to find the driver in the VBEAF_PATH directory */
        if (getenv(VBEAF_PATH)) {
            strcpy(filename, getenv(VBEAF_PATH));
            backslash(filename);
            strcat(filename,VBEAF_DRV);
			if ((f = AF_openFile(filename)) != 0)
				goto FoundDriver;
			}
#endif
		/* Else try in the specified path */
		if (driverDir) {
			strcpy(filename, driverDir);
			backslash(filename);
			strcat(filename,VBEAF_DRV);
			if ((f = AF_openFile(filename)) != 0)
                goto FoundDriver;
            }

        /* Driver file was not found */
        status = afDriverNotFound;
        return NULL;
        }

    /* Allocate memory for driver file and load it */
FoundDriver:
	AF_size = AF_getFileSize(f);
	if ((dc = malloc(AF_size)) == NULL) {
		status = afLoadMem;
		AF_closeFile(f);
		return NULL;
		}
	AF_readFile(f,dc,AF_size);
	AF_closeFile(f);

    /* Perform internal initialisation */
    if (_AF_initInternal(dc) != afOK)
        goto Error;

	/* Now call the driver to detect the installed hardware and initialise
     * the driver.
     */
    if (_AF_initDriver(dc) != 0) {
        status = afNotDetected;
        goto Error;
		}

	/* Indicate that we loaded the driver so we can free the memory later */
	loadedByUs = true;
    return dc;

Error:
    free(dc);
    return NULL;
}

ulong AFAPI AF_getDriverSize(void)
{ return AF_size; }

void AFAPI AF_unloadDriver(AF_devCtx *dc)
/****************************************************************************
*
* Function:     AF_unloadDriver
* Parameters:   dc  - Pointer to device context
*
* Description:  Unloads the loaded device driver.
*
****************************************************************************/
{
	if (loadedByUs) {
		free(dc);
		loadedByUs = false;
		}
}

/* Set of code stubs used to build the final bank switch code */

PRIVATE uchar AF_bankFunc32_Start[] = {
    0x53,0x51,                  /*  push    ebx,ecx     */
    0x8B,0xD0,                  /*  mov     edx,eax     */
    };

PRIVATE uchar AF_bankFunc32_End[] = {
    0x59,0x5B,                  /*  pop     ecx,ebx     */
    };

PRIVATE uchar bankFunc32[100];

#define copy(p,b,a) memcpy(b,a,sizeof(a)); (p) = (b) + sizeof(a)

ibool AFAPI AF_getBankFunc32(AF_devCtx *dc,int *codeLen,void **bankFunc)
/****************************************************************************
*
* Function:     VBE_getBankFunc32
* Parameters:   dc          - VBE/AF device context block
*               codeLen     - Place to store length of code
*               bankFunc    - Place to store pointer to bank switch code
* Returns:      True on success, false if not compatible.
*
* Description:  Creates a local 32 bit bank switch function from the
*               VBE/AF bank switch code that is compatible with the
*               virtual flat framebuffer devices (does not have a return
*               instruction at the end and takes the bank number in EAX
*               not EDX).
*
****************************************************************************/
{
	int     len;
	uchar   *code;
    uchar   *p;

    if (dc->SetBank32) {
        code = (uchar*)dc->SetBank32;
        len = dc->SetBank32Len-1;
        copy(p,bankFunc32,AF_bankFunc32_Start);
        memcpy(p,code,len);
        p += len;
        copy(p,p,AF_bankFunc32_End);
        *codeLen = p - bankFunc32;
        *bankFunc = bankFunc32;
        return true;
        }
    return false;
}

void AFAPI AF_OEMExt(AF_devCtx *dc,AF_DPMI_regs *regs)
/****************************************************************************
*
* Function:     AF_OEMExt
* Parameters:   dc   	- VBE/AF device context block
*				regs	- Pointer to register parameter block
*
* Description:  Calls the VBE/AF 2.0 OEM extension interface function. This
*				provides a standard interface for OEM vendors to provided
*				extended functionality within their VBE/AF drivers that
*				can be used by their operating system tools and drivers.
*
*				This should only be used for functionality that is outside
*				of the VBE/AF standard specification.
*
****************************************************************************/
{
	if (dc->Version >= 0x200) {
		/* We pass the pointer to our device context block in EBP to this
		 * function, since all the other registers are defined by their
		 * real mode interface counterparts and cannot be used.
		 */
		regs->ebp = (ulong)dc;
		_AF_callFuncASM(regs,dc->OEMExt);
		}
}

void AFAPI AF_SupplementalExt(AF_devCtx *dc,AF_DPMI_regs *regs)
/****************************************************************************
*
* Function:     AF_SupplementalExt
* Parameters:   dc   	- VBE/AF device context block
*				regs	- Pointer to register parameter block
*
* Description:  Calls the VBE/AF 2.0 supplemental extension interface
*				function. This provides a standard interface to all VBE
*				supplemental functions such as VBE/PM power management
*				and VBE/DDC Display Data Channel.
*
****************************************************************************/
{
	if (dc->Version >= 0x200) {
		/* We pass the pointer to our device context block in EBP to this
		 * function, since all the other registers are defined by their
		 * real mode interface counterparts and cannot be used.
		 */
		regs->ebp = (ulong)dc;
		_AF_callFuncASM(regs,dc->SupplementalExt);
		}
}
int AFAPI AF_PMdetect(AF_devCtx *dc,AF_int16 *supportedStates)
/****************************************************************************
*
* Function:     AF_PMdetect
* Parameters:   dc   			- VBE/AF device context block
*				supportedStates	- Place to return the supported DPMS states
* Returns:		VBE/PM version number
*
* Description:  Detects if the VBE/PM extensions are provided by the
*				loaded VBE/AF driver. If the driver is not 2.0 or higher,
*				this function will return not detected.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Detect if the VBE/PM interface is active */
	regs.eax = 0x4F10;
	regs.ebx = 0;
	regs.edi = 0;
	AF_SupplementalExt(dc,&regs);
	if (regs.eax != 0x004F)
		return 0;

	/* Get the supported states and return version number */
	*supportedStates = (regs.ebx >> 8) & 0xFF;
	return (regs.ebx & 0xFF);
}

void AFAPI AF_PMsetState(AF_devCtx *dc,AF_int16 state)
/****************************************************************************
*
* Function:     AF_PMsetState
* Parameters:   dc   	- VBE/AF device context block
*				state	- New DPMS state to set
*
* Description:  Sets the requested DPMS state via the driver.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F10;
	regs.ebx = 0x01 | (state << 8);
	AF_SupplementalExt(dc,&regs);
}

int AFAPI AF_PMgetState(AF_devCtx *dc)
/****************************************************************************
*
* Function:     AF_PMgetState
* Parameters:   dc   	- VBE/AF device context block
*				state	- New DPMS state to set
*
* Description:  Sets the requested DPMS state via the driver.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F10;
	regs.ebx = 0x02;
	AF_SupplementalExt(dc,&regs);
	return (regs.ebx >> 8) & 0xFF;
}

int AFAPI AF_DDCdetect(AF_devCtx *dc,AF_uint8 *transferTime,
	AF_uint8 *DDCtype)
/****************************************************************************
*
* Function:     AF_DDCdetect
* Parameters:   dc   			- VBE/AF device context block
*				transferTime	- Approx time to transfer one EDID block
*				DDCtype			- Type of DDC transfers supported
* Returns:		Version number of DDC interface detected.
*
* Description:  Detects if the VBE/DDC extensions are provided by the
*				loaded VBE/AF driver. If the driver is not 2.0 or higher,
*				this function will return not detected.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Detect if the VBE/DDC interface is active */
	regs.eax = 0x4F15;
	regs.ebx = 0;
	regs.ecx = 0;
	regs.edi = 0;
	AF_SupplementalExt(dc,&regs);
	if (regs.eax != 0x004F)
		return 0;

	/* Get the supported DDC level */
	*DDCtype = regs.ebx & 0xFF;
	*transferTime = (regs.ebx >> 8) & 0xFF;
	return (regs.ecx & 0xFFFF);
}

int AFAPI AF_DDCreadEDID(AF_devCtx *dc,AF_int16 block,AF_uint8 *edid,
	AF_int32 monitorPort)
/****************************************************************************
*
* Function:		AF_DDCreadEDID
* Parameters:   dc   	- VBE/AF device context block
*				block	- EDID block to read
*               edid	- Pointer to EDID block to fill in
* Returns:		True on success, false on failure
*
* Description:	Requests to read a specific EDID block from the monitor
*				over the DDC data channel.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Read EDID block */
	regs.eax = 0x4F15;
	regs.ebx = 1;
	regs.ecx = monitorPort;
	regs.edx = block;
	regs.edi = (ulong)edid;
	AF_SupplementalExt(dc,&regs);
	return (regs.eax == 0x004F);
}

int AFAPI AF_DDCgetEDID_A2Size(AF_devCtx *dc,AF_int32 monitorPort)
/****************************************************************************
*
* Function:		AF_DDCgetEDID_A2Size
* Parameters:   dc   	- VBE/AF device context block
* Returns:		Size of the Plug & Display EDID block
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Read EDID block */
	regs.eax = 0x4F15;
	regs.ebx = 3;
	regs.ecx = monitorPort;
	regs.edx = 0xFFFF;
	AF_SupplementalExt(dc,&regs);
	if (regs.eax == 0x004F)
		return (regs.edx & 0xFFFF);
	return 0;
}

int AFAPI AF_DDCreadEDID_A2(AF_devCtx *dc,AF_int16 block,AF_uint8 *edid,
	AF_int32 monitorPort)
/****************************************************************************
*
* Function:		AF_DDCreadEDID_A2
* Parameters:   dc   	- VBE/AF device context block
*				block	- EDID block to read
*               edid	- Pointer to EDID block to fill in
* Returns:		True on success, false on failure
*
* Description:	Requests to read a specific EDID block from the monitor
*				over the DDC data channel.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Read EDID block */
	regs.eax = 0x4F15;
	regs.ebx = 3;
	regs.ecx = monitorPort;
	regs.edx = block;
	regs.edi = (ulong)edid;
	AF_SupplementalExt(dc,&regs);
	return (regs.eax == 0x004F);
}

int AFAPI AF_DDCgetEDID_A6Size(AF_devCtx *dc,AF_int32 monitorPort)
/****************************************************************************
*
* Function:		AF_DDCgetEDID_A6Size
* Parameters:   dc   	- VBE/AF device context block
* Returns:		Size of the Plug & Display EDID block
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Read EDID block */
	regs.eax = 0x4F15;
	regs.ebx = 4;
	regs.ecx = monitorPort;
	regs.edx = 0xFFFF;
	AF_SupplementalExt(dc,&regs);
	if (regs.eax == 0x004F)
		return (regs.edx & 0xFFFF);
	return 0;
}

int AFAPI AF_DDCreadEDID_A6(AF_devCtx *dc,AF_int16 block,AF_uint8 *edid,
	AF_int32 monitorPort)
/****************************************************************************
*
* Function:		AF_DDCreadEDID_A6
* Parameters:   dc   	- VBE/AF device context block
*				block	- EDID block to read
*               edid	- Pointer to EDID block to fill in
* Returns:		True on success, false on failure
*
* Description:	Requests to read a specific EDID block from the monitor
*				over the DDC data channel.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Read EDID block */
	regs.eax = 0x4F15;
	regs.ebx = 4;
	regs.ecx = monitorPort;
	regs.edx = block;
	regs.edi = (ulong)edid;
	AF_SupplementalExt(dc,&regs);
	return (regs.eax == 0x004F);
}

int	AFAPI AF_SCIdetect(AF_devCtx *dc,AF_uint8 *capabilities,
	AF_int32 *numMonitorPorts)
/****************************************************************************
*
* Function:     AF_SCIdetect
* Parameters:   dc   			- VBE/AF device context block
*				capabilities	- Place to store VBE/SCI capabilities
*				numMonitorPorts	- Place to store number of ports supported
* Returns:		Version number of VBE/SCI interface detected.
*
* Description:  Detects if the VBE/SCI extensions are provided by the
*				loaded VBE/AF driver. If the driver is not 2.0 or higher,
*				this function will return not detected.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	/* Detect if the VBE/SCI interface is active */
	regs.eax = 0x4F15;
	regs.ebx = 0x10;
	AF_SupplementalExt(dc,&regs);
	if (regs.eax != 0x004F)
		return 0;

	/* Get the supported DDC level */
	*capabilities = regs.ebx & 0xFF;
	*numMonitorPorts = (regs.edx & 0xFFFF);
	return (regs.ecx & 0xFFFF);
}

void AFAPI AF_SCIbegin(AF_devCtx *dc,AF_int32 monitorPort)
/****************************************************************************
*
* Function:     AF_SCIbegin
* Parameters:   dc   		- VBE/AF device context block
*				monitorPort	- Monitor port to control via I2C
*
* Description:  Begins control of the I2C data at the specified monitor
*				port.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F15;
	regs.ebx = 0x11;
	regs.ecx = monitorPort;
	AF_SupplementalExt(dc,&regs);
}

void AFAPI AF_SCIend(AF_devCtx *dc,AF_int32 monitorPort)
/****************************************************************************
*
* Function:     AF_SCIend
* Parameters:   dc   		- VBE/AF device context block
*				monitorPort	- Monitor port to control via I2C
*
* Description:  Ends control of the I2C data at the specified monitor
*				port.
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F15;
	regs.ebx = 0x12;
	regs.ecx = monitorPort;
	AF_SupplementalExt(dc,&regs);
}

void AFAPI AF_SCIwriteSCL(AF_devCtx *dc,AF_int32 monitorPort,AF_int32 bit)
/****************************************************************************
*
* Function:     AF_SCIwriteSCL
* Parameters:   dc   		- VBE/AF device context block
*				monitorPort	- Monitor port to control via I2C
*				bit			- Bit value to write to port (0 or 1)
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F15;
	regs.ebx = 0x13;
	regs.ecx = monitorPort;
	regs.edx = bit;
	AF_SupplementalExt(dc,&regs);
}

void AFAPI AF_SCIwriteSDA(AF_devCtx *dc,AF_int32 monitorPort,AF_int32 bit)
/****************************************************************************
*
* Function:     AF_SCIwriteSDA
* Parameters:   dc   		- VBE/AF device context block
*				monitorPort	- Monitor port to control via I2C
*				bit			- Bit value to write to port (0 or 1)
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F15;
	regs.ebx = 0x14;
	regs.ecx = monitorPort;
	regs.edx = bit;
	AF_SupplementalExt(dc,&regs);
}

int AFAPI AF_SCIreadSCL(AF_devCtx *dc,AF_int32 monitorPort)
/****************************************************************************
*
* Function:     AF_SCIreadSCL
* Parameters:   dc   		- VBE/AF device context block
*				monitorPort	- Monitor port to control via I2C
* Returns:		Bit value read from port
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F15;
	regs.ebx = 0x15;
	regs.ecx = monitorPort;
	AF_SupplementalExt(dc,&regs);
	return regs.edx & 0x1;
}

int AFAPI AF_SCIreadSDA(AF_devCtx *dc,AF_int32 monitorPort)
/****************************************************************************
*
* Function:     AF_SCIreadSDA
* Parameters:   dc   		- VBE/AF device context block
*				monitorPort	- Monitor port to control via I2C
* Returns:		Bit value read from port
*
****************************************************************************/
{
	AF_DPMI_regs	regs;

	regs.eax = 0x4F15;
	regs.ebx = 0x16;
	regs.ecx = monitorPort;
	AF_SupplementalExt(dc,&regs);
	return regs.edx & 0x1;
}

AF_int32 AFAPI AF_status(void)
{ return status; }

const char * AFAPI AF_errorMsg(int status)
/****************************************************************************
*
* Function:     AF_errorMsg
* Returns:      String describing error condition.
*
****************************************************************************/
{
    const char *msg[] = {
        "No error",
        "Graphics hardware not detected",
        "Driver file not found",
        "File loaded was not a driver file",
        "Not enough memory to load driver",
        "Driver file is an older version",
        "Could not map physical memory areas",
        };
    if (status >= afOK && status < afMaxError)
        return msg[status];
    return "Unknown error!";
}

/* Internal file I/O functions using standard C I/O */

#ifndef	NO_FILEIO
/****************************************************************************
PARAMETERS:
filename	- Name of file to open

RETURNS:
VBE/AF generic file handle for opened file, 0 on error.

REMARKS:
This function open a file for reading given the filename. If an error occurs
this function simply returns 0.
****************************************************************************/
AF_FILE AF_openFile(char *filename)
{
	return (AF_FILE)fopen(filename,"rb");
}

/****************************************************************************
PARAMETERS:
_f	- VBE/AF file handle for file to get size of

RETURNS:
Size of the file in bytes.

REMARKS:
This function determines the size of the open file, without changing the
current position in the file.
****************************************************************************/
long AF_getFileSize(AF_FILE _f)
{
	FILE	*f = (FILE*)_f;
	long    size,oldpos = ftell(f);
	fseek(f,0,SEEK_END);            /* Seek to end of file              */
	size = ftell(f);                /* Determine the size of the file   */
	fseek(f,oldpos,SEEK_SET);       /* Seek to old position in file     */
	return size;                    /* Return the size of the file      */
}

/****************************************************************************
PARAMETERS:
f		- VBE/AF file handle to read data from
buf		- Buffer to read data into
size	- Size of buffer to read data into

RETURNS:
Number of bytes actually read from the file (<= size).

REMARKS:
This function reads up to size bytes from the file, and returns the actual
number of bytes read.
****************************************************************************/
long AF_readFile(AF_FILE f,void *buf,long size)
{
	return fread(buf,1,size,(FILE*)f);
}

/****************************************************************************
PARAMETERS:
f		- VBE/AF file handle to close

REMARKS:
This function simply close the open file handle.
****************************************************************************/
void AF_closeFile(AF_FILE f)
{
	fclose((FILE*)f);
}
#endif	/* NO_FILEIO */

#endif  /* !defined(__16BIT__) */
