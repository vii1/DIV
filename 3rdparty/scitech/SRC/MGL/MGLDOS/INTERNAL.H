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
* Description:	Internal header file for the MGLDOS bindings for the MGL.
*				For all MGLDOS display drivers, we cache the selector for
*				the display memory in GS.
*
*
****************************************************************************/

#ifndef	__MGLDOS_INTERNAL_H
#define	__MGLDOS_INTERNAL_H

#include "pmpro.h"
#include "vesavbe.h"

/*---------------------- Macro and type definitions -----------------------*/

#define	IBMPC					/* Use IBM PC assembler routines		*/
#define	PC_FULLSCREEN			/* Include fullscreen mode support		*/
#define	PC_ACCELERATED			/* Include accelerated mode support		*/

/* Macro to normalise a huge pointer */

#if	defined(REALMODE) || defined(PM286)
#define	NORMALISE_PTR(p) (p = MK_FP(FP_SEG(p) + FP_OFF(p)/16,FP_OFF(p) & 0xF))
#else
#define	NORMALISE_PTR(p)
#endif

/*--------------------------- Global Variables ----------------------------*/

extern uint	_VARAPI _MGL_biosSel;	/* Selector for BIOS data area		*/
extern ibool _VARAPI _EVT_installed;	/* True if event handling installed	*/

/*------------------------- Function Prototypes ---------------------------*/

/* Routines for generating sounds and delaying */

void _ASMAPI _MGL_sound(int freq);
void _ASMAPI _MGL_nosound(void);
void _ASMAPI _MGL_delay(int milliseconds);

/* Internal event management routines */

void _EVT_init(void);
void _EVT_suspend(void);
void _EVT_resume(void);
void _EVT_maskKeyCode(event_t *evt);
int _ASMAPI _EVT_getKeyCode(void);

#endif	/* __MGLDOS_INTERNAL_H */
