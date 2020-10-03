/****************************************************************************
*
*						      PM/Pro Library
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
* Environment:	IBM PC (MSDOS) Real mode and 16/32 bit Protected Mode
*
* Description:	Module implementing DOS extender independant protected
*				mode programming. This module will need to be included in
*				all programs that use SciTech Software's products that
*				are to be compiled in protected mode, and will need to be
*				compiled with the correct defines for the DOS extender that
*				you will be using (or with no defines for real mode
*				emulation of these routines).
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pmpro.h"

#ifdef __REALDOS__
#include <dos.h>

/*--------------------------- Global variables ----------------------------*/

#ifndef	REALMODE
static int	globalDataStart;
#endif

PM_criticalHandler	_ASMAPI _PM_critHandler = NULL;
PM_breakHandler		_ASMAPI _PM_breakHandler = NULL;
PM_intHandler		_ASMAPI _PM_timerHandler = NULL;
PM_intHandler		_ASMAPI _PM_keyHandler = NULL;
PM_key15Handler		_ASMAPI _PM_key15Handler = NULL;
PM_mouseHandler		_ASMAPI _PM_mouseHandler = NULL;
PM_intHandler		_ASMAPI _PM_int10Handler = NULL;
int					_ASMAPI _PM_mouseMask;

uint    	_ASMAPI _PM_ctrlCSel;				/* Location of Ctrl-C flag		*/
uint    	_ASMAPI _PM_ctrlCOff;
uint    	_ASMAPI _PM_ctrlBSel;				/* Location of Ctrl-Break flag	*/
uint    	_ASMAPI _PM_ctrlBOff;
uint		_ASMAPI _PM_critSel;				/* Location of Critical error Bf*/
uint    	_ASMAPI _PM_critOff;
PMFARPTR	_ASMAPI _PM_prevTimer = PMNULL;		/* Previous timer handler		*/
PMFARPTR	_ASMAPI _PM_prevKey = PMNULL;		/* Previous key handler			*/
PMFARPTR	_ASMAPI _PM_prevKey15 = PMNULL;		/* Previous key15 handler		*/
PMFARPTR	_ASMAPI _PM_prevBreak = PMNULL;		/* Previous break handler		*/
PMFARPTR	_ASMAPI _PM_prevCtrlC = PMNULL;		/* Previous CtrlC handler		*/
PMFARPTR	_ASMAPI _PM_prevCritical = PMNULL;	/* Previous critical handler	*/
long 		_ASMAPI _PM_prevRealTimer;			/* Previous real mode timer		*/
long 		_ASMAPI _PM_prevRealKey;			/* Previous real mode key		*/
long 		_ASMAPI _PM_prevRealKey15;			/* Previous real mode key15		*/
long 		_ASMAPI _PM_prevRealInt10;			/* Previous real mode int 10h 	*/

#ifndef	REALMODE
static int	globalDataEnd;
#endif

/*----------------------------- Implementation ----------------------------*/

/* Globals for locking interrupt handlers in _pmpro.asm */

#ifndef	REALMODE
extern int	_ASMAPI _PM_pmproDataStart;
extern int	_ASMAPI _PM_pmproDataEnd;
void _ASMAPI _PM_pmproCodeStart(void);
void _ASMAPI _PM_pmproCodeEnd(void);
#endif

/* Protected mode interrupt handlers, also called by PM callbacks below	*/

void _ASMAPI _PM_timerISR(void);
void _ASMAPI _PM_keyISR(void);
void _ASMAPI _PM_key15ISR(void);
void _ASMAPI _PM_breakISR(void);
void _ASMAPI _PM_ctrlCISR(void);
void _ASMAPI _PM_criticalISR(void);
void _ASMAPI _PM_mouseISR(void);
void _ASMAPI _PM_int10PMCB(void);

/* Protected mode DPMI callback handlers */

void _ASMAPI _PM_mousePMCB(void);

/* Routine to install a mouse handler function */

void _ASMAPI _PM_setMouseHandler(int mask);

/* Routine to allocate DPMI real mode callback routines */

void _ASMAPI _DPMI_allocateCallback(void (_ASMAPI *pmcode)(),void *rmregs,long *RMCB);
void _ASMAPI _DPMI_freeCallback(long RMCB);

/* DPMI helper functions in PMLITE.C */

ulong	PMAPI DPMI_mapPhysicalToLinear(ulong physAddr,ulong limit);
int 	PMAPI DPMI_setSelectorBase(ushort sel,ulong linAddr);
ulong 	PMAPI DPMI_getSelectorBase(ushort sel);
int 	PMAPI DPMI_setSelectorLimit(ushort sel,ulong limit);
uint 	PMAPI DPMI_createSelector(ulong base,ulong limit);
void 	PMAPI DPMI_freeSelector(uint sel);
int 	PMAPI DPMI_lockLinearPages(ulong linear,ulong len);
int 	PMAPI DPMI_unlockLinearPages(ulong linear,ulong len);

void PMAPI _PM_getRMvect(int intno, long *realisr);
void PMAPI _PM_setRMvect(int intno, long realisr);

/*-------------------------------------------------------------------------*/
/* Generic routines common to all environments							   */
/*-------------------------------------------------------------------------*/

void PMAPI PM_resetMouseDriver(int hardReset)
{
	RMREGS			regs;
	PM_mouseHandler	oldHandler = _PM_mouseHandler;

	PM_restoreMouseHandler();
	regs.x.ax = hardReset ? 0 : 33;
	PM_int86(0x33, &regs, &regs);
	if (oldHandler)
		PM_setMouseHandler(_PM_mouseMask, oldHandler);
}

#ifndef REALMODE

static void PMAPI lockPMHandlers(void)
{
	static int 	locked = 0;
	int			stat = 0;

	/* Lock all of the code and data used by our protected mode interrupt
	 * handling routines, so that it will continue to work correctly
	 * under real mode.
	 */
	if (!locked) {
		PM_saveDS();
		stat  = !PM_lockDataPages(&globalDataStart,(int)&globalDataEnd-(int)&globalDataStart);
		stat |= !PM_lockDataPages(&_PM_pmproDataStart,(int)&_PM_pmproDataEnd - (int)&_PM_pmproDataStart);
		stat |= !PM_lockCodePages((__codePtr)_PM_pmproCodeStart,(int)_PM_pmproCodeEnd-(int)_PM_pmproCodeStart);
		if (stat) {
			printf("Page locking services failed - interrupt handling not safe!\n");
			exit(1);
			}
		locked = 1;
		}
}

#endif

/*-------------------------------------------------------------------------*/
/* DOS Real Mode support.												   */
/*-------------------------------------------------------------------------*/

#ifdef REALMODE

#ifndef	MK_FP
#define MK_FP(s,o)  ( (void far *)( ((ulong)(s) << 16) + \
					(ulong)(o) ))
#endif

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
	PM_saveDS();
	_PM_mouseHandler = mh;
	_PM_setMouseHandler(_PM_mouseMask = mask);
	return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
	union REGS		regs;

	if (_PM_mouseHandler) {
		regs.x.ax = 33;
		int86(0x33, &regs, &regs);
		_PM_mouseHandler = NULL;
		}
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
	_PM_getRMvect(0x8, (long*)&_PM_prevTimer);
	_PM_timerHandler = th;
	_PM_setRMvect(0x8, (long)_PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
	if (_PM_timerHandler) {
		_PM_setRMvect(0x8, (long)_PM_prevTimer);
		_PM_timerHandler = NULL;
		}
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
	_PM_getRMvect(0x9, (long*)&_PM_prevKey);
	_PM_keyHandler = kh;
	_PM_setRMvect(0x9, (long)_PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
	if (_PM_keyHandler) {
		_PM_setRMvect(0x9, (long)_PM_prevKey);
		_PM_keyHandler = NULL;
		}
}

void PMAPI PM_setKey15Handler(PM_key15Handler kh)
{
	_PM_getRMvect(0x15, (long*)&_PM_prevKey15);
	_PM_key15Handler = kh;
	_PM_setRMvect(0x15, (long)_PM_key15ISR);
}

void PMAPI PM_restoreKey15Handler(void)
{
	if (_PM_key15Handler) {
		_PM_setRMvect(0x15, (long)_PM_prevKey15);
		_PM_key15Handler = NULL;
		}
}

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
	static int	ctrlCFlag,ctrlBFlag;
	void		*p;

	p = &ctrlCFlag;					/* Need this for buggy compilers	*/
	_PM_ctrlCSel = FP_SEG(p);
	_PM_ctrlCOff = FP_OFF(p);
	p = &ctrlBFlag;					/* Need this for buggy compilers	*/
	_PM_ctrlBSel = FP_SEG(p);
	_PM_ctrlBOff = FP_OFF(p);
	_PM_getRMvect(0x1B, (long*)&_PM_prevBreak);
	_PM_getRMvect(0x23, (long*)&_PM_prevCtrlC);
	_PM_breakHandler = bh;
	_PM_setRMvect(0x1B, (long)_PM_breakISR);
	_PM_setRMvect(0x23, (long)_PM_ctrlCISR);
}

void PMAPI PM_installBreakHandler(void)
{
	PM_installAltBreakHandler(NULL);
}

void PMAPI PM_restoreBreakHandler(void)
{
	if (_PM_prevBreak) {
		_PM_setRMvect(0x1B, (long)_PM_prevBreak);
		_PM_setRMvect(0x23, (long)_PM_prevCtrlC);
		_PM_prevBreak = NULL;
		_PM_breakHandler = NULL;
		}
}

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
	static	short critBuf[2];
	void	*p;

	p = critBuf;					/* Need this for buggy compilers	*/
	_PM_critSel = FP_SEG(p);
	_PM_critOff = FP_OFF(p);
	_PM_getRMvect(0x24, (long*)&_PM_prevCritical);
	_PM_critHandler = ch;
	_PM_setRMvect(0x24, (long)_PM_criticalISR);
}

void PMAPI PM_installCriticalHandler(void)
{
	PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
	if (_PM_prevCritical) {
		_PM_setRMvect(0x24, (long)_PM_prevCritical);
		_PM_prevCritical = NULL;
		_PM_critHandler = NULL;
		}
}

int PMAPI PM_lockDataPages(void *p,uint len)
{
	p = p;	len = len;		/* Do nothing for real mode	*/
	return 1;
}

int PMAPI PM_unlockDataPages(void *p,uint len)
{
	p = p;	len = len;		/* Do nothing for real mode	*/
	return 1;
}

int PMAPI PM_lockCodePages(void (*p)(),uint len)
{
	p = p;	len = len;		/* Do nothing for real mode	*/
	return 1;
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len)
{
	p = p;	len = len;		/* Do nothing for real mode	*/
	return 1;
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	long t;
	_PM_getRMvect(intno,&t);
	*isr = (void*)t;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
	PM_saveDS();
	_PM_setRMvect(intno,(long)isr);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	_PM_setRMvect(intno,(long)isr);
}

#endif

/*-------------------------------------------------------------------------*/
/* Borland DPMI16 DOS Power Pack support.								   */
/*-------------------------------------------------------------------------*/

#if defined(__WINDOWS16__) || defined(DPMI16)

static long prevRealBreak;		/* Previous real mode break handler		*/
static long prevRealCtrlC;		/* Previous real mode CtrlC handler		*/
static long prevRealCritical;	/* Prev real mode critical handler		*/

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
	lockPMHandlers();			/* Ensure our handlers are locked	*/

	_PM_mouseHandler = mh;
	_PM_setMouseHandler(_PM_mouseMask = mask);
	return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
	union REGS		regs;

	if (_PM_mouseHandler) {
		regs.x.ax = 33;
		int86(0x33, &regs, &regs);
		_PM_mouseHandler = NULL;
		}
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	union REGS	regs;

	regs.x.ax = 0x204;
	regs.h.bl = intno;
	int86(0x31,&regs,&regs);
	*isr = MK_FP(regs.x.cx, regs.x.dx);
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
	PM_saveDS();
	PM_restorePMvect(intno, (void*)isr);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	union REGS	regs;

	regs.x.ax = 0x205;
	regs.h.bl = intno;
	regs.x.cx = FP_SEG(isr);
	regs.x.dx = FP_OFF(isr);
	int86(0x31,&regs,&regs);
}

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
	PM_getPMvect(intno,pmisr);
	_PM_getRMvect(intno,realisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
	_PM_setRMvect(intno,realisr);
	PM_restorePMvect(intno,pmisr);
}

static void setISR(int intno, void (* PMAPI pmisr)())
{
	lockPMHandlers();			/* Ensure our handlers are locked	*/
	PM_setPMvect(intno,pmisr);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
	getISR(0x8, &_PM_prevTimer, &_PM_prevRealTimer);
	_PM_timerHandler = th;
	setISR(0x8, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
	if (_PM_timerHandler) {
		restoreISR(0x8, _PM_prevTimer, _PM_prevRealTimer);
		_PM_timerHandler = NULL;
		}
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
	getISR(0x9, &_PM_prevKey, &_PM_prevRealKey);
	_PM_keyHandler = kh;
	setISR(0x9, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
	if (_PM_keyHandler) {
		restoreISR(0x9, _PM_prevKey, _PM_prevRealKey);
		_PM_keyHandler = NULL;
		}
}

void PMAPI PM_setKey15Handler(PM_key15Handler kh)
{
	getISR(0x15, &_PM_prevKey15, &_PM_prevRealKey15);
	_PM_key15Handler = kh;
	setISR(0x15, _PM_key15ISR);
}

void PMAPI PM_restoreKey15Handler(void)
{
	if (_PM_key15Handler) {
		restoreISR(0x15, _PM_prevKey15, _PM_prevRealKey15);
		_PM_key15Handler = NULL;
		}
}

/* Real mode Ctrl-C and Ctrl-Break handler. This handler simply sets a
 * flag in the real mode code segment and exits. We save the location
 * of this flag in real mode memory so that both the real mode and
 * protected mode code will be modifying the same flags.
 */

static uchar ctrlHandler[] = {
	0x00,0x00,0x00,0x00,			/* 	ctrlBFlag						*/
	0x66,0x2E,0xC7,0x06,0x00,0x00,
	0x01,0x00,0x00,0x00,			/*	mov     [cs:ctrlBFlag],1		*/
	0xCF,							/*	iretf							*/
	};

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
	uint	rseg,roff;

	getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
	getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
	_PM_breakHandler = bh;
	setISR(0x1B, _PM_breakISR);
	setISR(0x23, _PM_ctrlCISR);

	/* Hook the real mode vectors for these handlers, as these are not
	 * normally reflected by the DPMI server up to protected mode
	 */
	PM_allocRealSeg(sizeof(ctrlHandler)*2, &_PM_ctrlBSel, &_PM_ctrlBOff,
		&rseg, &roff);
	PM_memcpyfn(_PM_ctrlBSel,_PM_ctrlBOff,ctrlHandler,
		sizeof(ctrlHandler));
	PM_memcpyfn(_PM_ctrlBSel,_PM_ctrlBOff+sizeof(ctrlHandler),
		ctrlHandler,sizeof(ctrlHandler));
	_PM_ctrlCSel = _PM_ctrlBSel;
	_PM_ctrlCOff = _PM_ctrlBOff + sizeof(ctrlHandler);
	_PM_setRMvect(0x1B,((long)rseg << 16) | roff+4);
	_PM_setRMvect(0x23,((long)rseg << 16) | roff+sizeof(ctrlHandler)+4);
}

void PMAPI PM_installBreakHandler(void)
{
	PM_installAltBreakHandler(NULL);
}

void PMAPI PM_restoreBreakHandler(void)
{
	if (_PM_prevBreak) {
		restoreISR(0x1B, _PM_prevBreak, prevRealBreak);
		restoreISR(0x23, _PM_prevCtrlC, prevRealCtrlC);
		PM_freeRealSeg(_PM_ctrlBSel, _PM_ctrlBOff);
		_PM_prevBreak = NULL;
		_PM_breakHandler = NULL;
		}
}

/* Real mode Critical Error handler. This handler simply saves the AX and
 * DI values in the real mode code segment and exits. We save the location
 * of this flag in real mode memory so that both the real mode and
 * protected mode code will be modifying the same flags.
 */

static uchar criticalHandler[] = {
	0x00,0x00,						/* 	axCode							*/
	0x00,0x00,						/* 	diCode							*/
	0x2E,0xA3,0x00,0x00,			/*	mov		[cs:axCode],ax			*/
	0x2E,0x89,0x3E,0x02,0x00,		/*	mov		[cs:diCode],di			*/
	0xB8,0x03,0x00,					/*	mov		ax,3					*/
	0xCF,							/*	iretf							*/
	};

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
	uint	rseg,roff;

	getISR(0x24, &_PM_prevCritical, &prevRealCritical);
	_PM_critHandler = ch;
	setISR(0x24, _PM_criticalISR);

	/* Hook the real mode vector, as this is not normally reflected by the
	 * DPMI server up to protected mode.
	 */
	PM_allocRealSeg(sizeof(criticalHandler)*2, &_PM_critSel, &_PM_critOff,
		&rseg, &roff);
	PM_memcpyfn(_PM_critSel,_PM_critOff,criticalHandler,
		sizeof(criticalHandler));
	_PM_setRMvect(0x24,((long)rseg << 16) | roff+4);
}

void PMAPI PM_installCriticalHandler(void)
{
	PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
	if (_PM_prevCritical) {
		restoreISR(0x24, _PM_prevCritical, prevRealCritical);
		PM_freeRealSeg(_PM_critSel, _PM_critOff);
		_PM_prevCritical = NULL;
		_PM_critHandler = NULL;
		}
}

int PMAPI PM_lockDataPages(void *p,uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_lockLinearPages(FP_OFF(p) + DPMI_getSelectorBase(sregs.ds),len);
}

int PMAPI PM_unlockDataPages(void *p,uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_unlockLinearPages(FP_OFF(p) + DPMI_getSelectorBase(sregs.ds),len);
}

int PMAPI PM_lockCodePages(void (*p)(),uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_lockLinearPages(FP_OFF(p) + DPMI_getSelectorBase(sregs.cs),len);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_unlockLinearPages(FP_OFF(p) + DPMI_getSelectorBase(sregs.cs),len);
}

#endif

/*-------------------------------------------------------------------------*/
/* Phar Lap TNT DOS Extender support.									   */
/*-------------------------------------------------------------------------*/

#ifdef TNT

#include <pldos32.h>
#include <pharlap.h>
#include <hw386.h>

static long prevRealBreak;		/* Previous real mode break handler		*/
static long prevRealCtrlC;		/* Previous real mode CtrlC handler		*/
static long prevRealCritical;	/* Prev real mode critical handler		*/
static uint mouseSel,mouseOff;

/* The following real mode routine is used to call a 32 bit protected
 * mode FAR function from real mode. We use this for passing up control
 * from the real mode mouse callback to our protected mode code.
 */

static UCHAR realHandler[] = {		/* Real mode code generic handler	*/
	0x00,0x00,0x00,0x00,			/* __PM_callProtp					*/
	0x00,0x00,						/* __PM_protCS						*/
	0x00,0x00,0x00,0x00,			/* __PM_protHandler					*/
	0x66,0x60,						/*	pushad							*/
	0x1E,							/*	push	ds						*/
	0x6A,0x00,						/*	push	0						*/
	0x6A,0x00,						/*	push	0						*/
	0x2E,0xFF,0x36,0x04,0x00,		/*	push    [cs:__PM_protCS]		*/
	0x66,0x2E,0xFF,0x36,0x06,0x00,  /*  push    [cs:__PM_protHandler]	*/
	0x2E,0xFF,0x1E,0x00,0x00,       /*  call    [cs:__PM_callProtp]  	*/
	0x83,0xC4,0x0A,					/*	add		sp,10					*/
	0x1F,							/* 	pop		ds						*/
	0x66,0x61,                      /*  popad                           */
	0xCB,							/*	retf							*/
	};

/* The following functions installs the above realmode callback mechanism
 * in real mode memory for calling the protected mode routine.
 */

int installCallback(void (PMAPI *pmCB)(),uint *psel, uint *poff,
	uint *rseg, uint *roff)
{
	CONFIG_INF	config;
	REALPTR		realBufAdr,callProtp;
	ULONG		bufSize;
	FARPTR		protBufAdr;

	/* Get address of real mode routine to call up to protected mode	*/
	_dx_rmlink_get(&callProtp, &realBufAdr, &bufSize, &protBufAdr);
	_dx_config_inf(&config, (UCHAR*)&config);

	/* Fill in the values in the real mode code segment so that it will
	 * call the correct routine.
	 */
	*((REALPTR*)&realHandler[0]) = callProtp;
	*((USHORT*)&realHandler[4]) = config.c_cs_sel;
	*((ULONG*)&realHandler[6]) = (ULONG)pmCB;

	/* Copy the real mode handler to real mode memory	*/
	if (!PM_allocRealSeg(sizeof(realHandler),psel,poff,rseg,roff))
		return 0;
	PM_memcpyfn(*psel,*poff,realHandler,sizeof(realHandler));

	/* Skip past global variabls in real mode code segment */
	*roff += 0x0A;
	return 1;
}

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
	RMREGS		regs;
	RMSREGS		sregs;
	uint	rseg,roff;

	lockPMHandlers();			/* Ensure our handlers are locked	*/

	if (!installCallback(_PM_mouseISR, &mouseSel, &mouseOff, &rseg, &roff))
		return 0;
	_PM_mouseHandler = mh;

	/* Install the real mode mouse handler	*/
	sregs.es = rseg;
	regs.x.dx = roff;
	regs.x.cx = _PM_mouseMask = mask;
	regs.x.ax = 0xC;
	PM_int86x(0x33, &regs, &regs, &sregs);
	return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
	RMREGS	regs;

	if (_PM_mouseHandler) {
		regs.x.ax = 33;
		PM_int86(0x33, &regs, &regs);
		PM_freeRealSeg(mouseSel,mouseOff);
		_PM_mouseHandler = NULL;
		}
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	FARPTR	ph;

	_dx_pmiv_get(intno, &ph);
	isr->sel = FP_SEL(ph);
	isr->off = FP_OFF(ph);
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
	CONFIG_INF	config;
	FARPTR		ph;

	PM_saveDS();
	_dx_config_inf(&config, (UCHAR*)&config);
	FP_SET(ph,(uint)isr,config.c_cs_sel);
	_dx_pmiv_set(intno,ph);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	FARPTR	ph;

	FP_SET(ph,isr.off,isr.sel);
	_dx_pmiv_set(intno,ph);
}

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
	PM_getPMvect(intno,pmisr);
	_PM_getRMvect(intno, realisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
	_PM_setRMvect(intno,realisr);
	PM_restorePMvect(intno,pmisr);
}

static void setISR(int intno, void (PMAPI *isr)())
{
	CONFIG_INF	config;
	FARPTR		ph;

	lockPMHandlers();			/* Ensure our handlers are locked	*/

	_dx_config_inf(&config, (UCHAR*)&config);
	FP_SET(ph,(uint)isr,config.c_cs_sel);
	_dx_apmiv_set(intno,ph);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
	getISR(0x8, &_PM_prevTimer, &_PM_prevRealTimer);
	_PM_timerHandler = th;
	setISR(0x8, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
	if (_PM_timerHandler) {
		restoreISR(0x8, _PM_prevTimer, _PM_prevRealTimer);
		_PM_timerHandler = NULL;
		}
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
	getISR(0x9, &_PM_prevKey, &_PM_prevRealKey);
	_PM_keyHandler = kh;
	setISR(0x9, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
	if (_PM_keyHandler) {
		restoreISR(0x9, _PM_prevKey, _PM_prevRealKey);
		_PM_keyHandler = NULL;
		}
}

void PMAPI PM_setKey15Handler(PM_key15Handler kh)
{
	getISR(0x15, &_PM_prevKey15, &_PM_prevRealKey15);
	_PM_key15Handler = kh;
	setISR(0x15, _PM_key15ISR);
}

void PMAPI PM_restoreKey15Handler(void)
{
	if (_PM_key15Handler) {
		restoreISR(0x15, _PM_prevKey15, _PM_prevRealKey15);
		_PM_key15Handler = NULL;
		}
}

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
	static int	ctrlCFlag,ctrlBFlag;
	CONFIG_INF	config;

	_dx_config_inf(&config, (UCHAR*)&config);
	_PM_ctrlCSel = config.c_ds_sel;
	_PM_ctrlCOff = (uint)&ctrlCFlag;
	_PM_ctrlBSel = config.c_ds_sel;
	_PM_ctrlBOff = (uint)&ctrlBFlag;
	getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
	getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
	_PM_breakHandler = bh;
	setISR(0x1B, _PM_breakISR);
	setISR(0x23, _PM_ctrlCISR);
}

void PMAPI PM_installBreakHandler(void)
{
	PM_installAltBreakHandler(NULL);
}

void PMAPI PM_restoreBreakHandler(void)
{
	if (_PM_prevBreak.sel) {
		restoreISR(0x1B, _PM_prevBreak, prevRealBreak);
		restoreISR(0x23, _PM_prevCtrlC, prevRealCtrlC);
		_PM_prevBreak.sel = 0;
		_PM_breakHandler = NULL;
		}
}

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
	static short	critBuf[2];
	CONFIG_INF		config;

	_dx_config_inf(&config, (UCHAR*)&config);
	_PM_critSel = config.c_ds_sel;
	_PM_critOff = (uint)critBuf;
	getISR(0x24, &_PM_prevCritical, &prevRealCritical);
	_PM_critHandler = ch;
	setISR(0x24, _PM_criticalISR);
}

void PMAPI PM_installCriticalHandler(void)
{
	PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
	if (_PM_prevCritical.sel) {
		restoreISR(0x24, _PM_prevCritical, prevRealCritical);
		_PM_prevCritical.sel = 0;
		_PM_critHandler = NULL;
		}
}

int PMAPI PM_lockDataPages(void *p,uint len)
{
	return (_dx_lock_pgsn(p,len) == 0);
}

int PMAPI PM_unlockDataPages(void *p,uint len)
{
	return (_dx_ulock_pgsn(p,len) == 0);
}

int PMAPI PM_lockCodePages(void (*p)(),uint len)
{
	CONFIG_INF	config;
	FARPTR		fp;

	_dx_config_inf(&config, (UCHAR*)&config);
	FP_SET(fp,p,config.c_cs_sel);
	return (_dx_lock_pgs(fp,len) == 0);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len)
{
	CONFIG_INF	config;
	FARPTR		fp;

	_dx_config_inf(&config, (UCHAR*)&config);
	FP_SET(fp,p,config.c_cs_sel);
	return (_dx_ulock_pgs(fp,len) == 0);
}

#endif

/*-------------------------------------------------------------------------*/
/* Symantec C++ DOSX and FlashTek X-32/X-32VM support					   */
/*-------------------------------------------------------------------------*/

#if	defined(DOSX) || defined(X32VM)

#ifdef	X32VM
#include <x32.h>
#endif

static long prevRealBreak;		/* Previous real mode break handler		*/
static long prevRealCtrlC;		/* Previous real mode CtrlC handler		*/
static long prevRealCritical;	/* Prev real mode critical handler		*/

static uint mouseSel = 0,mouseOff;

/* The following real mode routine is used to call a 32 bit protected
 * mode FAR function from real mode. We use this for passing up control
 * from the real mode mouse callback to our protected mode code.
 */

static char realHandler[] = {		/* Real mode code generic handler	*/
	0x00,0x00,0x00,0x00,			/* __PM_callProtp					*/
	0x00,0x00,						/* __PM_protCS						*/
	0x00,0x00,0x00,0x00,			/* __PM_protHandler					*/
	0x1E,							/*	push	ds						*/
	0x6A,0x00,						/*	push	0						*/
	0x6A,0x00,						/*	push	0						*/
	0x2E,0xFF,0x36,0x04,0x00,		/*	push    [cs:__PM_protCS]		*/
	0x66,0x2E,0xFF,0x36,0x06,0x00,  /*  push    [cs:__PM_protHandler]	*/
	0x2E,0xFF,0x1E,0x00,0x00,       /*  call    [cs:__PM_callProtp]  	*/
	0x83,0xC4,0x0A,					/*	add		sp,10					*/
	0x1F,							/* 	pop		ds						*/
	0xCB,							/*	retf							*/
	};

/* The following functions installs the above realmode callback mechanism
 * in real mode memory for calling the protected mode routine.
 */

int installCallback(void (PMAPI *pmCB)(),uint *psel, uint *poff,
	uint *rseg, uint *roff)
{
	PMREGS			regs;
	PMSREGS			sregs;

	regs.x.ax = 0x250D;
	PM_segread(&sregs);
	PM_int386x(0x21,&regs,&regs,&sregs);	/* Get RM callback address	*/

	/* Fill in the values in the real mode code segment so that it will
	 * call the correct routine.
	 */
	*((ulong*)&realHandler[0]) = regs.e.eax;
	*((ushort*)&realHandler[4]) = sregs.cs;
	*((ulong*)&realHandler[6]) = (ulong)pmCB;

	/* Copy the real mode handler to real mode memory (only allocate the
	 * buffer once since we cant dealloate it with X32).
	 */
	if (*psel == 0) {
		if (!PM_allocRealSeg(sizeof(realHandler),psel,poff,rseg,roff))
			return 0;
		}
	PM_memcpyfn(*psel,*poff,realHandler,sizeof(realHandler));

	/* Skip past global variables in real mode code segment */
	*roff += 0x0A;
	return 1;
}

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
	RMREGS		regs;
	RMSREGS		sregs;
	uint	rseg,roff;

	lockPMHandlers();			/* Ensure our handlers are locked	*/

	if (!installCallback(_PM_mouseISR, &mouseSel, &mouseOff, &rseg, &roff))
		return 0;
	_PM_mouseHandler = mh;

	/* Install the real mode mouse handler	*/
	sregs.es = rseg;
	regs.x.dx = roff;
	regs.x.cx = _PM_mouseMask = mask;
	regs.x.ax = 0xC;
	PM_int86x(0x33, &regs, &regs, &sregs);
	return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
	RMREGS	regs;

	if (_PM_mouseHandler) {
		regs.x.ax = 33;
		PM_int86(0x33, &regs, &regs);
		_PM_mouseHandler = NULL;
		}
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	PMREGS	regs;
	PMSREGS	sregs;

	PM_segread(&sregs);
	regs.x.ax = 0x2502;			/* Get PM interrupt vector				*/
	regs.x.cx = intno;
	PM_int386x(0x21, &regs, &regs, &sregs);
	isr->sel = sregs.es;
	isr->off = regs.e.ebx;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
	PMFARPTR	pmisr;
	PMSREGS		sregs;

	PM_saveDS();
	PM_segread(&sregs);
	pmisr.sel = sregs.cs;
	pmisr.off = (uint)isr;
	PM_restorePMvect(intno, pmisr);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	PMREGS	regs;
	PMSREGS	sregs;

	PM_segread(&sregs);
	regs.x.ax = 0x2505;			/* Set PM interrupt vector				*/
	regs.x.cx = intno;
	sregs.ds = isr.sel;
	regs.e.edx = isr.off;
	PM_int386x(0x21, &regs, &regs, &sregs);
}

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
	PM_getPMvect(intno,pmisr);
	_PM_getRMvect(intno,realisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
	PMREGS	regs;
	PMSREGS	sregs;

	PM_segread(&sregs);
	regs.x.ax = 0x2507;			/* Set real and PM vectors				*/
	regs.x.cx = intno;
	sregs.ds = pmisr.sel;
	regs.e.edx = pmisr.off;
	regs.e.ebx = realisr;
	PM_int386x(0x21, &regs, &regs, &sregs);
}

static void setISR(int intno, void *isr)
{
	PMREGS	regs;
	PMSREGS	sregs;

	lockPMHandlers();			/* Ensure our handlers are locked		*/

	PM_segread(&sregs);
	regs.x.ax = 0x2506;       	/* Hook real and protected vectors  	*/
	regs.x.cx = intno;
	sregs.ds = sregs.cs;
	regs.e.edx = (uint)isr;
	PM_int386x(0x21, &regs, &regs, &sregs);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
	getISR(0x8, &_PM_prevTimer, &_PM_prevRealTimer);
	_PM_timerHandler = th;
	setISR(0x8, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
	if (_PM_timerHandler) {
		restoreISR(0x8, _PM_prevTimer, _PM_prevRealTimer);
		_PM_timerHandler = NULL;
		}
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
	getISR(0x9, &_PM_prevKey, &_PM_prevRealKey);
	_PM_keyHandler = kh;
	setISR(0x9, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
	if (_PM_keyHandler) {
		restoreISR(0x9, _PM_prevKey, _PM_prevRealKey);
		_PM_keyHandler = NULL;
		}
}

void PMAPI PM_setKey15Handler(PM_key15Handler kh)
{
	getISR(0x15, &_PM_prevKey15, &_PM_prevRealKey15);
	_PM_key15Handler = kh;
	setISR(0x15, _PM_key15ISR);
}

void PMAPI PM_restoreKey15Handler(void)
{
	if (_PM_key15Handler) {
		restoreISR(0x15, _PM_prevKey15, _PM_prevRealKey15);
		_PM_key15Handler = NULL;
		}
}

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
	static int	ctrlCFlag,ctrlBFlag;
	PMSREGS     sregs;

	PM_segread(&sregs);
	_PM_ctrlCSel = sregs.ds;
	_PM_ctrlCOff = (uint)&ctrlCFlag;
	_PM_ctrlBSel = sregs.ds;
	_PM_ctrlBOff = (uint)&ctrlBFlag;
	getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
	getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
	_PM_breakHandler = bh;
	setISR(0x1B, _PM_breakISR);
	setISR(0x23, _PM_ctrlCISR);
}

void PMAPI PM_installBreakHandler(void)
{
	PM_installAltBreakHandler(NULL);
}

void PMAPI PM_restoreBreakHandler(void)
{
	if (_PM_prevBreak.sel) {
		restoreISR(0x1B, _PM_prevBreak, prevRealBreak);
		restoreISR(0x23, _PM_prevCtrlC, prevRealCtrlC);
		_PM_prevBreak.sel = 0;
		_PM_breakHandler = NULL;
		}
}

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
	static short	critBuf[2];
	PMSREGS     	sregs;

	PM_segread(&sregs);
	_PM_critSel = sregs.ds;
	_PM_critOff = (uint)critBuf;
	getISR(0x24, &_PM_prevCritical, &prevRealCritical);
	_PM_critHandler = ch;
	setISR(0x24, _PM_criticalISR);
}

void PMAPI PM_installCriticalHandler(void)
{
	PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
	if (_PM_prevCritical.sel) {
		restoreISR(0x24, _PM_prevCritical, prevRealCritical);
		_PM_prevCritical.sel = 0;
		_PM_critHandler = NULL;
		}
}

int PMAPI PM_lockDataPages(void *p,uint len)
{
	return (_x386_memlock(p,len) == 0);
}

int PMAPI PM_unlockDataPages(void *p,uint len)
{
	return (_x386_memunlock(p,len) == 0);
}

int PMAPI PM_lockCodePages(void (*p)(),uint len)
{
	return (_x386_memlock(p,len) == 0);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len)
{
	return (_x386_memunlock(p,len) == 0);
}

#endif

/*-------------------------------------------------------------------------*/
/* Borland's DPMI32 DOS Power Pack Extender support.					   */
/*-------------------------------------------------------------------------*/

#ifdef  DPMI32
#define	GENERIC_DPMI32			/* Use generic 32 bit DPMI routines	*/

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	PMREGS	regs;

	regs.x.ax = 0x204;
	regs.h.bl = intno;
	PM_int386(0x31,&regs,&regs);
	isr->sel = regs.x.cx;
	isr->off = regs.e.edx;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
	PMSREGS	sregs;
	PMREGS	regs;

	PM_saveDS();
	regs.x.ax = 0x205;			/* Set protected mode vector		*/
	regs.h.bl = intno;
	PM_segread(&sregs);
	regs.x.cx = sregs.cs;
	regs.e.edx = (uint)isr;
	PM_int386(0x31,&regs,&regs);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	PMREGS	regs;

	regs.x.ax = 0x205;
	regs.h.bl = intno;
	regs.x.cx = isr.sel;
	regs.e.edx = isr.off;
	PM_int386(0x31,&regs,&regs);
}

/* The following is supposed to work, according to all the Borland
 * documentation and the sample code that they give us. However it
 * never does work 100% (never in the MGL code) so we install the
 * mouse handler ourselves using the real mode callback stuff. This
 * unfortunately has the effect of causing TD32's mouse handling stuff
 * to be hooked out.
 */
#if 0
#define	MOUSE_SUPPORTED			/* DPMI32 directly supports mouse	*/
int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
	lockPMHandlers();			/* Ensure our handlers are locked	*/

	_PM_mouseHandler = mh;
	_PM_setMouseHandler(_PM_mouseMask = mask);
	return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
	PMREGS	regs;

    if (_PM_mouseHandler) {
		regs.x.ax = 33;
		PM_int386(0x33, &regs, &regs);
		_PM_mouseHandler = NULL;
		}
}
#endif

#endif

/*-------------------------------------------------------------------------*/
/* Watcom C/C++ with Rational DOS/4GW support.							   */
/*-------------------------------------------------------------------------*/

#ifdef	DOS4GW
#define	GENERIC_DPMI32			/* Use generic 32 bit DPMI routines	*/

#define	MOUSE_SUPPORTED			/* DOS4GW directly supports mouse	*/

/* We use the normal DOS services to save and restore interrupts handlers
 * for Watcom C++, because using the direct DPMI functions does not
 * appear to work properly. At least if we use the DPMI functions, we
 * dont get the auto-passup feature that we need to correctly trap
 * real and protected mode interrupts without installing Bi-model
 * interrupt handlers.
 */

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	PMREGS	regs;
	PMSREGS	sregs;

	PM_segread(&sregs);
	regs.h.ah = 0x35;
	regs.h.al = intno;
	PM_int386x(0x21,&regs,&regs,&sregs);
	isr->sel = sregs.es;
	isr->off = regs.e.ebx;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
	PMREGS	regs;
	PMSREGS	sregs;

	PM_saveDS();
	PM_segread(&sregs);
	regs.h.ah = 0x25;
	regs.h.al = intno;
	sregs.ds = sregs.cs;
	regs.e.edx = (uint)isr;
	PM_int386x(0x21,&regs,&regs,&sregs);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	PMREGS	regs;
	PMSREGS	sregs;

	PM_segread(&sregs);
	regs.h.ah = 0x25;
	regs.h.al = intno;
	sregs.ds = isr.sel;
	regs.e.edx = isr.off;
	PM_int386x(0x21,&regs,&regs,&sregs);
}

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
	lockPMHandlers();			/* Ensure our handlers are locked	*/

	_PM_mouseHandler = mh;
	_PM_setMouseHandler(_PM_mouseMask = mask);
	return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
	PMREGS	regs;

    if (_PM_mouseHandler) {
		regs.x.ax = 33;
		PM_int386(0x33, &regs, &regs);
		_PM_mouseHandler = NULL;
		}
}

#endif

/*-------------------------------------------------------------------------*/
/* DJGPP port of GNU C++ support.										   */
/*-------------------------------------------------------------------------*/

#ifdef DJGPP
#define	GENERIC_DPMI32			/* Use generic 32 bit DPMI routines	*/

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	PMREGS	regs;

	regs.x.ax = 0x204;
	regs.h.bl = intno;
	PM_int386(0x31,&regs,&regs);
	isr->sel = regs.x.cx;
	isr->off = regs.e.edx;
}

void PMAPI PM_setPMvect(int intno, PM_intHandler isr)
{
	PMSREGS	sregs;
	PMREGS	regs;

	PM_saveDS();
	regs.x.ax = 0x205;			/* Set protected mode vector		*/
	regs.h.bl = intno;
	PM_segread(&sregs);
	regs.x.cx = sregs.cs;
	regs.e.edx = (uint)isr;
	PM_int386(0x31,&regs,&regs);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	PMREGS	regs;

	regs.x.ax = 0x205;
	regs.h.bl = intno;
	regs.x.cx = isr.sel;
	regs.e.edx = isr.off;
	PM_int386(0x31,&regs,&regs);
}

#endif

/*-------------------------------------------------------------------------*/
/* Generic 32 bit DPMI routines											   */
/*-------------------------------------------------------------------------*/

#if	defined(GENERIC_DPMI32)

static long prevRealBreak;		/* Previous real mode break handler		*/
static long prevRealCtrlC;		/* Previous real mode CtrlC handler		*/
static long prevRealCritical;	/* Prev real mode critical handler		*/

#ifndef	MOUSE_SUPPORTED

/* The following real mode routine is used to call a 32 bit protected
 * mode FAR function from real mode. We use this for passing up control
 * from the real mode mouse callback to our protected mode code.
 */

static long	mouseRMCB;			/* Mouse real mode callback address		*/
static uint	mouseSel,mouseOff;
static char	mouseRegs[0x32];	/* Real mode regs for mouse callback	*/
static uchar mouseHandler[] = {
	0x00,0x00,0x00,0x00,		/* _realRMCB							*/
	0x2E,0xFF,0x1E,0x00,0x00,	/*  call    [cs:_realRMCB]	  			*/
	0xCB,						/*	retf								*/
	};

int PMAPI PM_setMouseHandler(int mask, PM_mouseHandler mh)
{
	RMREGS		regs;
	RMSREGS		sregs;
	uint	rseg,roff;

	lockPMHandlers();			/* Ensure our handlers are locked	*/

	/* Copy the real mode handler to real mode memory	*/
	if (!PM_allocRealSeg(sizeof(mouseHandler),&mouseSel,&mouseOff,&rseg,&roff))
		return 0;
	PM_memcpyfn(mouseSel,mouseOff,mouseHandler,sizeof(mouseHandler));
	_DPMI_allocateCallback(_PM_mousePMCB, mouseRegs, &mouseRMCB);
	PM_setLong(mouseSel,mouseOff,mouseRMCB);

	/* Install the real mode mouse handler	*/
	_PM_mouseHandler = mh;
	sregs.es = rseg;
	regs.x.dx = roff+4;
	regs.x.cx = _PM_mouseMask = mask;
	regs.x.ax = 0xC;
	PM_int86x(0x33, &regs, &regs, &sregs);
	return 1;
}

void PMAPI PM_restoreMouseHandler(void)
{
	RMREGS	regs;

	if (_PM_mouseHandler) {
		regs.x.ax = 33;
		PM_int86(0x33, &regs, &regs);
		PM_freeRealSeg(mouseSel,mouseOff);
		_DPMI_freeCallback(mouseRMCB);
		_PM_mouseHandler = NULL;
		}
}

#endif

static void getISR(int intno, PMFARPTR *pmisr, long *realisr)
{
	PM_getPMvect(intno,pmisr);
	_PM_getRMvect(intno,realisr);
}

static void restoreISR(int intno, PMFARPTR pmisr, long realisr)
{
	_PM_setRMvect(intno,realisr);
	PM_restorePMvect(intno,pmisr);
}

static void setISR(int intno, PM_intHandler pmisr)
{
	lockPMHandlers();			/* Ensure our handlers are locked	*/
	PM_setPMvect(intno,pmisr);
}

void PMAPI PM_setTimerHandler(PM_intHandler th)
{
	getISR(0x8, &_PM_prevTimer, &_PM_prevRealTimer);
	_PM_timerHandler = th;
	setISR(0x8, _PM_timerISR);
}

void PMAPI PM_restoreTimerHandler(void)
{
	if (_PM_timerHandler) {
		restoreISR(0x8, _PM_prevTimer, _PM_prevRealTimer);
		_PM_timerHandler = NULL;
		}
}

void PMAPI PM_setKeyHandler(PM_intHandler kh)
{
	getISR(0x9, &_PM_prevKey, &_PM_prevRealKey);
	_PM_keyHandler = kh;
	setISR(0x9, _PM_keyISR);
}

void PMAPI PM_restoreKeyHandler(void)
{
	if (_PM_keyHandler) {
		restoreISR(0x9, _PM_prevKey, _PM_prevRealKey);
		_PM_keyHandler = NULL;
		}
}

void PMAPI PM_setKey15Handler(PM_key15Handler kh)
{
	getISR(0x15, &_PM_prevKey15, &_PM_prevRealKey15);
	_PM_key15Handler = kh;
	setISR(0x15, _PM_key15ISR);
}

void PMAPI PM_restoreKey15Handler(void)
{
	if (_PM_key15Handler) {
		restoreISR(0x15, _PM_prevKey15, _PM_prevRealKey15);
		_PM_key15Handler = NULL;
		}
}

/* The following real mode routine is used to call a 32 bit protected
 * mode interrupt function from real mode. We use this for passing up control
 * from the real mode mouse callback to our protected mode code.
 */

static long	 int10RMCB;			/* int10 real mode callback address		*/
static uint	 int10Sel,int10Off;
static uint	 int10RSeg,int10ROff;
static uchar int10Regs[0x32];	/* Real mode regs for int10 callback	*/
static uchar int10Handler[] = {
	0x00,0x00,0x00,0x00,			/* _realRMCB						*/
	0x00,0x00,0x00,0x00,			/* _prevInt10						*/
	0x00,							/* _guard							*/
	0x2E,0x80,0x3E,0x08,0x00,0x00,	/*	cmp		[BYTE cs:_guard],0		*/
	0x75,0x1C,						/*  jne		@@Skip					*/
	0x80,0xFC,0x00,				    /*	cmp	    ah,0					*/
	0x74,0x05,				     	/*  je		@@DoIt					*/
	0x80,0xFC,0x4F,				    /*  cmp	    ah,4Fh					*/
	0x75,0x12,				     	/*	jne		@@Skip					*/
	0x2E,0xC6,0x06,0x08,0x00,0x01,	/*	mov		[BYTE cs:_guard],1		*/
	0x2E,0xFF,0x1E,0x00,0x00,		/*  call    [cs:_realRMCB]	  		*/
	0x2E,0xC6,0x06,0x08,0x00,0x00,	/*	mov		[BYTE cs:_guard],0		*/
	0xCF,							/*	iretf							*/
									/* @@Skip:							*/
	0x2E,0xFF,0x2E,0x04,0x00,		/*  jmp     [cs:_prevInt10]			*/

	0x50,							/*  push	ax						*/
	0xB8,0x05,0x4F,					/*  mov		ax,4F05h				*/
	0xCD,0x10,						/*	int		10h						*/
	0x58,							/* 	pop		ax						*/
	0xCB,							/*	retf							*/
	};

int PMAPI PM_setInt10Handler(PM_intHandler ih)
{
	lockPMHandlers();			/* Ensure our handlers are locked	*/

	/* Copy the real mode handler to real mode memory */
	if (!PM_allocRealSeg(sizeof(int10Handler),&int10Sel,&int10Off,&int10RSeg,&int10ROff))
		return 0;
	PM_memcpyfn(int10Sel,int10Off,int10Handler,sizeof(int10Handler));
	_DPMI_allocateCallback(_PM_int10PMCB, int10Regs, &int10RMCB);
	PM_setLong(int10Sel,int10Off,int10RMCB);
	_PM_getRMvect(0x10,&_PM_prevRealInt10);
	PM_setLong(int10Sel,int10Off+4,_PM_prevRealInt10);

	/* Install the real mode int10 handler */
	_PM_int10Handler = ih;
	_PM_setRMvect(0x10,((long)int10RSeg << 16) | (int10ROff+9));
	return 1;
}

long PMAPI PM_getRealBankFunc(void)
{
	return ((long)int10RSeg << 16) | (int10ROff+0x32);
}

void PMAPI PM_restoreInt10Handler(void)
{
	if (_PM_int10Handler) {
		_PM_setRMvect(0x10,_PM_prevRealInt10);
		PM_freeRealSeg(int10Sel,int10Off);
		_DPMI_freeCallback(int10RMCB);
		_PM_int10Handler = NULL;
		}
}

/* Real mode Ctrl-C and Ctrl-Break handler. This handler simply sets a
 * flag in the real mode code segment and exit. We save the location
 * of this flag in real mode memory so that both the real mode and
 * protected mode code will be modifying the same flags.
 */

#ifndef	DOS4GW
static uchar ctrlHandler[] = {
	0x00,0x00,0x00,0x00,			/* 	ctrlBFlag						*/
	0x66,0x2E,0xC7,0x06,0x00,0x00,
	0x01,0x00,0x00,0x00,			/*	mov     [cs:ctrlBFlag],1		*/
	0xCF,							/*	iretf							*/
	};
#endif

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
#ifndef	DOS4GW
	uint	rseg,roff;
#else
	static int	ctrlCFlag,ctrlBFlag;
	PMSREGS     sregs;

	PM_segread(&sregs);
	_PM_ctrlCSel = sregs.ds;
	_PM_ctrlCOff = (uint)&ctrlCFlag;
	_PM_ctrlBSel = sregs.ds;
	_PM_ctrlBOff = (uint)&ctrlBFlag;
#endif

	getISR(0x1B, &_PM_prevBreak, &prevRealBreak);
	getISR(0x23, &_PM_prevCtrlC, &prevRealCtrlC);
	_PM_breakHandler = bh;
	setISR(0x1B, _PM_breakISR);
	setISR(0x23, _PM_ctrlCISR);

#ifndef	DOS4GW
	/* Hook the real mode vectors for these handlers, as these are not
	 * normally reflected by the DPMI server up to protected mode
	 */
	PM_allocRealSeg(sizeof(ctrlHandler)*2, &_PM_ctrlBSel, &_PM_ctrlBOff,
		&rseg, &roff);
	PM_memcpyfn(_PM_ctrlBSel,_PM_ctrlBOff,ctrlHandler,
		sizeof(ctrlHandler));
	PM_memcpyfn(_PM_ctrlBSel,_PM_ctrlBOff+sizeof(ctrlHandler),
		ctrlHandler,sizeof(ctrlHandler));
	_PM_ctrlCSel = _PM_ctrlBSel;
	_PM_ctrlCOff = _PM_ctrlBOff + sizeof(ctrlHandler);
	_PM_setRMvect(0x1B,((long)rseg << 16) | (roff+4));
	_PM_setRMvect(0x23,((long)rseg << 16) | (roff+sizeof(ctrlHandler)+4));
#endif
}

void PMAPI PM_installBreakHandler(void)
{
	PM_installAltBreakHandler(NULL);
}

void PMAPI PM_restoreBreakHandler(void)
{
	if (_PM_prevBreak.sel) {
		restoreISR(0x1B, _PM_prevBreak, prevRealBreak);
		restoreISR(0x23, _PM_prevCtrlC, prevRealCtrlC);
#ifndef	DOS4GW
		PM_freeRealSeg(_PM_ctrlBSel, _PM_ctrlBOff);
		_PM_prevBreak.sel = 0;
		_PM_breakHandler = NULL;
#endif
		}
}

/* Real mode Critical Error handler. This handler simply saves the AX and
 * DI values in the real mode code segment and exits. We save the location
 * of this flag in real mode memory so that both the real mode and
 * protected mode code will be modifying the same flags.
 */

#ifndef	DOS4GW
static uchar criticalHandler[] = {
	0x00,0x00,						/* 	axCode							*/
	0x00,0x00,						/* 	diCode							*/
	0x2E,0xA3,0x00,0x00,			/*	mov		[cs:axCode],ax			*/
	0x2E,0x89,0x3E,0x02,0x00,		/*	mov		[cs:diCode],di			*/
	0xB8,0x03,0x00,					/*	mov		ax,3					*/
	0xCF,							/*	iretf							*/
	};
#endif

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
#ifndef	DOS4GW
	uint	rseg,roff;
#else
	static	short	critBuf[2];
	PMSREGS     	sregs;

	PM_segread(&sregs);
	_PM_critSel = sregs.ds;
	_PM_critOff = (uint)critBuf;
#endif

	getISR(0x24, &_PM_prevCritical, &prevRealCritical);
	_PM_critHandler = ch;
	setISR(0x24, _PM_criticalISR);

#ifndef	DOS4GW
	/* Hook the real mode vector, as this is not normally reflected by the
	 * DPMI server up to protected mode.
	 */
	PM_allocRealSeg(sizeof(criticalHandler)*2, &_PM_critSel, &_PM_critOff,
		&rseg, &roff);
	PM_memcpyfn(_PM_critSel,_PM_critOff,criticalHandler,
		sizeof(criticalHandler));
	_PM_setRMvect(0x24,((long)rseg << 16) | (roff+4));
#endif
}

void PMAPI PM_installCriticalHandler(void)
{
	PM_installAltCriticalHandler(NULL);
}

void PMAPI PM_restoreCriticalHandler(void)
{
	if (_PM_prevCritical.sel) {
		restoreISR(0x24, _PM_prevCritical, prevRealCritical);
		PM_freeRealSeg(_PM_critSel, _PM_critOff);
		_PM_prevCritical.sel = 0;
		_PM_critHandler = NULL;
		}
}

int PMAPI PM_lockDataPages(void *p,uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_lockLinearPages((uint)p + DPMI_getSelectorBase(sregs.ds),len);
}

int PMAPI PM_unlockDataPages(void *p,uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_unlockLinearPages((uint)p + DPMI_getSelectorBase(sregs.ds),len);
}

int PMAPI PM_lockCodePages(void (*p)(),uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_lockLinearPages((uint)p + DPMI_getSelectorBase(sregs.cs),len);
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len)
{
	PMSREGS	sregs;
	PM_segread(&sregs);
	return DPMI_unlockLinearPages((uint)p + DPMI_getSelectorBase(sregs.cs),len);
}

#endif

#else /* !__REALDOS__ */

#ifndef	__WINDOWS__
#include <stdio.h>

int PMAPI PM_setMouseHandler(int mask,PM_mouseHandler mh)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

void PMAPI PM_restoreMouseHandler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_resetMouseDriver(int hardReset)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_setTimerHandler(PM_intHandler ih)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_chainPrevTimer(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_restoreTimerHandler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_setKeyHandler(PM_intHandler ih)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_chainPrevKey(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_restoreKeyHandler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

int	PMAPI PM_setInt10Handler(PM_intHandler ih)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

long PMAPI PM_getRealBankFunc(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

void PMAPI PM_restoreInt10Handler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_setKey15Handler(PM_key15Handler ih)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_restoreKey15Handler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_installBreakHandler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

int PMAPI PM_ctrlCHit(int clearFlag)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

int	PMAPI PM_ctrlBreakHit(int clearFlag)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

void PMAPI PM_restoreBreakHandler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_installAltBreakHandler(PM_breakHandler bh)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_installCriticalHandler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

int PMAPI PM_criticalError(int *axValue, int *diValue, int clearFlag)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

void PMAPI PM_restoreCriticalHandler(void)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_installAltCriticalHandler(PM_criticalHandler ch)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

int PMAPI PM_lockDataPages(void *p,uint len)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

int PMAPI PM_unlockDataPages(void *p,uint len)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

int PMAPI PM_lockCodePages(__codePtr p,uint len)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

int PMAPI PM_unlockCodePages(__codePtr p,uint len)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
	return 0;
}

void PMAPI PM_getPMvect(int intno, PMFARPTR *isr)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_setPMvect(int intno, PM_intHandler ih)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

void PMAPI PM_restorePMvect(int intno, PMFARPTR isr)
{
	printf("Function not supported on this OS!\n");
	exit(-1);
}

#endif

#endif /* !__REALDOS__ */
