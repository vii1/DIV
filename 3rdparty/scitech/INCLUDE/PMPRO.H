/****************************************************************************
*
*						 	   PM/Pro Library
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
* Environment:	Real mode and 16/32 bit Protected Mode under MSDOS
*
* Description:	Header file for the DOS extender independant protected
*				mode programming library. This library will need to be
*				included in all programs that use SciTech Software's
*				products that are to be compiled in protected mode.
*
*				This Professional version of the library also provides
*				simplified interrupt handling, allowing all common interrupt
*				handlers to be hooked and handled directly with normal C
*				functions, both in 16 bit and 32 bit modes. Note however that
*				simplified handling does not mean slow performance! All low
*				level interrupt handling is done efficiently in assembler
*				for speed (well actually necessary to insulate the
*				application from the lack of far pointers in 32 bit PM). The
*				interrupt handlers currently supported are:
*
*					Mouse (0x33 callback)
*                   Timer Tick (0x8)
*					Keyboard (0x9 and 0x15)
*					Control C/Break (0x23/0x1B)
*					Critical Error (0x24)
*
*
****************************************************************************/

#ifndef	__PMPRO_H
#define	__PMPRO_H

#ifndef	__PMODE_H
#include "pmode.h"
#endif

/*--------------------------- Macros and Typedefs -------------------------*/

/* Define the different types of interrupt handlers that we support		*/

typedef uint (PMAPI *PM_criticalHandler)(uint axValue,uint diValue);
typedef void (PMAPI *PM_breakHandler)(uint breakHit);
typedef void (PMAPI *PM_intHandler)(void);
typedef short (PMAPI *PM_key15Handler)(short scanCode);
typedef void (PMAPI *PM_mouseHandler)(uint event, uint butstate,
	uint x,uint y,uint mickeyX,uint mickeyY);

/* Create a type for representing far pointers in both 16 and 32 bit
 * protected mode.
 */

#ifdef	PM386
typedef	struct {
	long	off;
	short	sel;
	} PMFARPTR;
#define	PMNULL	{0,0}
#else
typedef	void *PMFARPTR;
#define	PMNULL	NULL
#endif

/*--------------------------- Function Prototypes -------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Routine to install a mouse interrupt handling routine. The
 * mouse handler routine is a normal C function, and the PM library
 * will take care of passing the correct parameters to the function,
 * and switching to a local stack.
 *
 * Note that you _must_ lock the memory containing the mouse interrupt
 * handler with the PM_lockPages() function otherwise you may encounter
 * problems in virtual memory environments.
 */

int 	PMAPI PM_setMouseHandler(int mask,PM_mouseHandler mh);
void 	PMAPI PM_restoreMouseHandler(void);

/* Routine to reset the mouse driver, and re-install the current
 * mouse interrupt handler if one was currently installed (since the
 * mouse reset will automatically remove this handler.
 */

void 	PMAPI PM_resetMouseDriver(int hardReset);

/* Routine to reset the mouse driver, and re-install the current
 * mouse interrupt handler if one was currently installed (since the
 * mouse reset will automatically remove this handler.
 */

void 	PMAPI PM_resetMouseDriver(int hardReset);

/* Routines to install and remove timer and keyboard interrupt handlers.
 * The handler routines are normal C functions. If the return value from
 * the function is PM_chainInt, the previous handler will be chained
 * to, otherwise the interrupt will simply return.
 *
 * Note that you _must_ lock the memory containing the interrupt
 * handlers with the PM_lockPages() function otherwise you may encounter
 * problems in virtual memory environments.
 */

void 	PMAPI PM_setTimerHandler(PM_intHandler ih);
void 	PMAPI PM_chainPrevTimer(void);
void 	PMAPI PM_restoreTimerHandler(void);
void 	PMAPI PM_setKeyHandler(PM_intHandler ih);
void 	PMAPI PM_chainPrevKey(void);
void 	PMAPI PM_restoreKeyHandler(void);

/* Routine to install a protected mode Int 10h handling routine. The
 * installed int10 handler gets called in protected mode with DS set to
 * the current data selector, and ES:EDI pointing the the real mode DPMI
 * register structure at the time of the interrupt. The handler must be
 * written in assembler to be able to extract the real mode register values
 * from the structure passed in.
 *
 * To chain to the previous Int 10h handler you can use the normal
 * DPMI_int86() function, and the installed real mode handler will guard
 * against calling the protected mode handler twice and will simply call
 * the existing real mode interrupt.
 *
 * To allow real mode applications to call the real mode bank switch
 * function, you can call the PM_getRealBankFunc function that will return
 * the address of the real mode function that will issue an int10h to
 * perform the bank switching and reflect the call to protected mode.
 */

int		PMAPI PM_setInt10Handler(PM_intHandler ih);
long	PMAPI PM_getRealBankFunc(void);
void 	PMAPI PM_restoreInt10Handler(void);

/* Routines to hook and unhook the alternate Int 15h keyboard intercept
 * callout routine. Your event handler will need to return the following:
 *
 *	scanCode 	- Let the BIOS process scan code (chains to previous handler)
 *	0 			- You have processed the scan code so flush from BIOS
 *
 * Note that this is not available under all DOS extenders, but does
 * work under real mode, DOS4GW and X32-VM. It does not work under the
 * PowerPack 32 bit DOS extenders. If you figure out how to do it let us know!
 */

void 	PMAPI PM_setKey15Handler(PM_key15Handler ih);
void 	PMAPI PM_restoreKey15Handler(void);

/* Routines to install and remove the control c/break interrupt handlers.
 * Interrupt handling is performed by the PM/Pro library, and you can call
 * the supplied routines to test the status of the Ctrl-C and Ctrl-Break
 * flags. If you pass the value TRUE for 'clearFlag' to these routines,
 * the internal flags will be reset in order to catch another Ctrl-C or
 * Ctrl-Break interrupt.
 */

void 	PMAPI PM_installBreakHandler(void);
int 	PMAPI PM_ctrlCHit(int clearFlag);
int		PMAPI PM_ctrlBreakHit(int clearFlag);
void 	PMAPI PM_restoreBreakHandler(void);

/* Routine to install an alternate break handler that will call your
 * code directly. This is not available under all DOS extenders, but does
 * work under real mode, DOS4GW and X32-VM. It does not work under the
 * PowerPack 32 bit DOS extenders. If you figure out how to do it let us know!
 *
 * Note that you should either install one or the other, but not both!
 */

void 	PMAPI PM_installAltBreakHandler(PM_breakHandler bh);

/* Routines to install and remove the critical error handler. The interrupt
 * is handled by the PM/Pro library, and the operation will always be failed.
 * You can check the status of the critical error handler with the
 * appropriate function. If you pass the value TRUE for 'clearFlag', the
 * internal flag will be reset ready to catch another critical error.
 */

void 	PMAPI PM_installCriticalHandler(void);
int 	PMAPI PM_criticalError(int *axValue, int *diValue, int clearFlag);
void 	PMAPI PM_restoreCriticalHandler(void);

/* Routine to install an alternate critical handler that will call your
 * code directly. This is not available under all DOS extenders, but does
 * work under real mode, DOS4GW and X32-VM. It does not work under the
 * PowerPack 32 bit DOS extenders. If you figure out how to do it let us know!
 *
 * Note that you should either install one or the other, but not both!
 */

void 	PMAPI PM_installAltCriticalHandler(PM_criticalHandler);

/* Routine to lock and unlock regions of memory under a virtual memory
 * environment. These routines _must_ be used to lock all hardware
 * and mouse interrupt handlers installed, _AND_ any global data that
 * these handler manipulate, so that they will always be present in memory
 * to handle the incoming interrupts.
 *
 * Note that it is important to call the correct routine depending on
 * whether the area being locked is code or data, so that under 32 bit
 * PM we will get the selector value correct.
 */

typedef void (*__codePtr)();

int 	PMAPI PM_lockDataPages(void *p,uint len);
int 	PMAPI PM_unlockDataPages(void *p,uint len);
int 	PMAPI PM_lockCodePages(__codePtr p,uint len);
int 	PMAPI PM_unlockCodePages(__codePtr p,uint len);

/* Functions to manage protected mode only interrupt handlers */

void 	PMAPI PM_getPMvect(int intno, PMFARPTR *isr);
void 	PMAPI PM_setPMvect(int intno, PM_intHandler ih);
void 	PMAPI PM_restorePMvect(int intno, PMFARPTR isr);

/* Functions to install and remove the virtual linear framebuffer
 * emulation code. For unsupported DOS extenders and when running under
 * a DPMI host like Windows or OS/2, this function will return a NULL.
 */

ibool 	PMAPI VF_available(void);
void * 	PMAPI VF_init(ulong baseAddr,int bankSize,int codeLen,void *bankFunc);
void 	PMAPI VF_exit(void);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#endif /* __PMPRO_H */
