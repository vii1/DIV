/****************************************************************************
*
*				 VESA BIOS Extensions/Accelerator Functions
*								Version 1.0
*
*                   Copyright (C) 1996 SciTech Software.
*                           All rights reserved.
*
* Filename:     $Workfile:   vbeaf.c  $
* Developed by:	SciTech Software
*
* Language:     ANSI C
* Environment:  IBM PC 32 bit Protected Mode.
*
* Description:  C module for the Graphics Acclerator Driver API. Uses
*				the SciTech PM/Pro library for interfacing with DOS
*				extender specific functions.
*
* $Date:   22 Apr 1996 23:49:38  $ $Author:   KendallB  $
*
****************************************************************************/

#ifdef	MGLWIN
#include "mgl.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vbeaf.h"
#include "pmode.h"

#if	!defined(__16BIT__) || defined(TESTING)

/*------------------------- Global Variables ------------------------------*/

#define	AF_DRIVERDIR	AF_DRIVERDIR_DOS

static int 			status = afOK;
static void			*IOMemMaps[4] = {NULL,NULL,NULL,NULL};
static void			*BankedMem = NULL;
static void			*LinearMem = NULL;
static AF_int32     Sel0000h = 0;
static AF_int32     Sel0040h = 0;
static AF_int32     SelA000h = 0;
static AF_int32     SelB000h = 0;
static AF_int32     SelC000h = 0;

/*-------------------------- Implementation -------------------------------*/

/* Internal assembler functions */

AF_int32	_cdecl _AF_initDriver(AF_devCtx *dc);
void 		_cdecl _AF_int86(void);
void 		_cdecl _AF_callRealMode(void);

static void backslash(char *s)
/****************************************************************************
*
* Function:		backslash
* Parameters:	s	- String to add backslash to
*
* Description:	Appends a trailing '\' pathname separator if the string
*				currently does not have one appended.
*
****************************************************************************/
{
	uint pos = strlen(s);
	if (s[pos-1] != '\\') {
		s[pos] = '\\';
		s[pos+1] = '\0';
		}
}

static long fileSize(FILE *f)
/****************************************************************************
*
* Function:		fileSize
* Parameters:	f	- Open file to determine the size of
* Returns:		Length of the file in bytes.
*
* Description:	Determines the length of the file, without altering the
*				current position in the file.
*
****************************************************************************/
{
	long	size,oldpos = ftell(f);

	fseek(f,0,SEEK_END);			/* Seek to end of file				*/
	size = ftell(f);				/* Determine the size of the file	*/
	fseek(f,oldpos,SEEK_SET);		/* Seek to old position in file		*/
	return size;					/* Return the size of the file		*/
}

void _cdecl _AF_callRealMode_C(AF_DPMI_regs *dregs)
/****************************************************************************
*
* Function:		_AF_callRealMode_C
* Parameters:	dregs	- Pointer to DPMI register structure
*
* Description:	Calls a real mode procedure. This does not need to be
*				speedy, so we simply convert the registers to the format
*               expected by the PM/Pro library and let it handle it.
*
****************************************************************************/
{
	RMREGS	regs;
	RMSREGS	sregs;

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

void _cdecl _AF_int86_C(AF_int32 intno,AF_DPMI_regs *dregs)
/****************************************************************************
*
* Function:		_AF_int86_C
* Parameters:	intno	- Interrupt number to issue
* 				dregs	- Pointer to DPMI register structure
*
* Description:	Issues a real mode interrupt. This does not need to be
*				speedy, so we simply convert the registers to the format
*               expected by the PM/Pro library and let it handle it.
*
****************************************************************************/
{
	RMREGS	regs;
	RMSREGS	sregs;

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
* Function:		_AF_initInternal
* Parameters:	dc	- Pointer to device context
* Returns:		Error code.
*
* Description:	Performs internal initialisation on the AF_devCtx driver
*				block, assuming that it has been loaded correctly.
*
****************************************************************************/
{
	/* Verify that the file is the driver file we are expecting */
	if (strcmp(dc->Signature,VBEAF_DRV) != 0)
		return status = afCorruptDriver;
	if (dc->Version < VBEAF_VERSION)
		return status = afOldVersion;

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
	if (BankedMem == NULL && dc->BankedBasePtr) {
		BankedMem = PM_mapPhysicalAddr(dc->BankedBasePtr,0xFFFF);
		if (BankedMem == NULL)
			return status = afMemMapError;
		}
	dc->BankedMem = BankedMem;

	/* Map the linear video memory area for the driver */
	if (LinearMem == NULL && dc->LinearBasePtr) {
		LinearMem = PM_mapPhysicalAddr(dc->LinearBasePtr,dc->LinearSize*1024L - 1);
		if (LinearMem == NULL)
			return status = afMemMapError;
		}
	dc->LinearMem = LinearMem;

	/* Provide selectors to important real mode segment areas */
	if (Sel0000h == 0) {
		Sel0000h = PM_createSelector(0x00000L,0xFFFFFL);
		Sel0040h = PM_createSelector(0x00400L,0xFFFF);
		SelA000h = PM_createSelector(0xA0000L,0xFFFF);
		SelB000h = PM_createSelector(0xB0000L,0xFFFF);
		SelC000h = PM_createSelector(0xC0000L,0xFFFF);
		}
	dc->Sel0000h = Sel0000h;
	dc->Sel0040h = Sel0040h;
	dc->SelA000h = SelA000h;
	dc->SelB000h = SelB000h;
	dc->SelC000h = SelC000h;

	/* Install the device callback functions */
	dc->Int86 = _AF_int86;
	dc->CallRealMode = _AF_callRealMode;
	return afOK;
}

AF_devCtx * AFAPI AF_loadDriver(const char *driverDir)
/****************************************************************************
*
* Function:		AF_loadDriver
* Parameters:	driverDir	- Directory to load the driver file from
* Returns:		Pointer to the loaded driver file.
*
* Description:  Loads the driver file and intialises the device context
*				ready for use. If the driver file cannot be found, or the
*				driver does not detect the installed hardware, we return
*				NULL and the application can get the status code with
*				AF_status().
*
****************************************************************************/
{
	char		filename[_MAX_PATH];
	FILE		*f;
	int			size;
	AF_devCtx	*dc;

	/* Reset status flag */
	status = afOK;

	/* Try if the default operating system location first */
	strcpy(filename,AF_DRIVERDIR);
	strcat(filename,VBEAF_DRV);
	if ((f = fopen(filename,"rb")) == NULL) {
		/* Now try to find the driver in the VBEAF_PATH directory */
		if (getenv(VBEAF_PATH)) {
			strcpy(filename, getenv(VBEAF_PATH));
			backslash(filename);
			strcat(filename,VBEAF_DRV);
			if ((f = fopen(filename,"rb")) != NULL)
				goto FoundDriver;
			}

		/* Else try in the specified path */
		if (driverDir) {
			strcpy(filename, driverDir);
			backslash(filename);
			strcat(filename,VBEAF_DRV);
			if ((f = fopen(filename,"rb")) != NULL)
				goto FoundDriver;
			}

		/* Driver file was not found */
		status = afDriverNotFound;
		return NULL;
		}

	/* Allocate memory for driver file and load it */
FoundDriver:
	size = fileSize(f);
	if ((dc = malloc(size+16)) == NULL) {
		status = afLoadMem;
		fclose(f);
		return NULL;
		}
	fread(dc,1,size,f);
	fclose(f);

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
	return dc;

Error:
	free(dc);
	return NULL;
}

void AFAPI AF_unloadDriver(AF_devCtx *dc)
/****************************************************************************
*
* Function:		AF_unloadDriver
* Parameters:	dc	- Pointer to device context
*
* Description:	Unloads the loaded device driver.
*
****************************************************************************/
{
	free(dc);
}

/* Set of code stubs used to build the final bank switch code */

PRIVATE	uchar AF_bankFunc32_Start[] = {
	0x53,0x51,					/*	push	ebx,ecx		*/
	0x8B,0xD0,					/*	mov		edx,eax		*/
	};

PRIVATE	uchar AF_bankFunc32_End[] = {
	0x59,0x5B,					/*	pop		ecx,ebx		*/
	};

PRIVATE	uchar bankFunc32[100];

#define	copy(p,b,a)	memcpy(b,a,sizeof(a)); (p) = (b) + sizeof(a)

bool AFAPI AF_getBankFunc32(AF_devCtx *dc,int *codeLen,void **bankFunc)
/****************************************************************************
*
* Function:		VBE_getBankFunc32
* Parameters:	dc			- VBE/AF device context block
*				codeLen		- Place to store length of code
*				bankFunc	- Place to store pointer to bank switch code
* Returns:		True on success, false if not compatible.
*
* Description:	Creates a local 32 bit bank switch function from the
*				VBE/AF bank switch code that is compatible with the
*				virtual flat framebuffer devices (does not have a return
*				instruction at the end and takes the bank number in EAX
*				not EDX).
*
****************************************************************************/
{
	int		len;
	uchar	*code = 0;
	uchar	*p;

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

AF_int32 AFAPI AF_status(void)
{ return status; }

const char * AFAPI AF_errorMsg(int status)
/****************************************************************************
*
* Function:		AF_errorMsg
* Returns:		String describing error condition.
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

#endif	/* !defined(__16BIT__) */
