;****************************************************************************
;*
;*						MegaGraph Graphics Library
;*
;*  ========================================================================
;*
;*    The contents of this file are subject to the SciTech MGL Public
;*    License Version 1.0 (the "License"); you may not use this file
;*    except in compliance with the License. You may obtain a copy of
;*    the License at http://www.scitechsoft.com/mgl-license.txt
;*
;*    Software distributed under the License is distributed on an
;*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
;*    implied. See the License for the specific language governing
;*    rights and limitations under the License.
;*
;*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
;*
;*    The Initial Developer of the Original Code is SciTech Software, Inc.
;*    All Rights Reserved.
;*
;*  ========================================================================
;*
;*
;* Language:	80386 Assembler
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	Random number generation routines in 386 assembler code.
;*				Generate random numbers with a range of either 2^15 or
;*				2^32 using the same seed value.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

header	_random					; Setup for MGL memory model

begdataseg	_random

seedlo      dd	1               ; 64 bit random number seed value
seedhi		dd	0

enddataseg	_random

begcodeseg	_random				; Start of code segment

;----------------------------------------------------------------------------
; void MGL_srand(unsigned seed)
;----------------------------------------------------------------------------
; Re-seeds the random number generation routines.
;----------------------------------------------------------------------------
procstartdll	_MGL_srand

		ARG		seed:UINT

		push	_bp
		mov		_bp,_sp
		mov		_ax,[seed]
ife flatmodel
		movzx	eax,ax
endif
		mov		[seedlo],eax
		mov		[seedhi],0
		pop		_bp
		ret

procend			_MGL_srand

;----------------------------------------------------------------------------
; ulong lrand(void)
;----------------------------------------------------------------------------
; Generate a random number between 0 and 2^64-1.
;----------------------------------------------------------------------------
proc	lrand	near

		use_ebx
		push	esi
		mov		eax,[seedlo]
		mov		esi,[seedhi]
		mov		ecx,015A4h
		mov		ebx,04E35h
		test	eax,eax
		jz		@@1
		mul		ebx
@@1:	xchg	ecx,eax
		mul		esi
		add		eax,ecx
		xchg	eax,esi
		mul		ebx
		add		edx,esi
		add		eax,1
		adc		edx,0
		mov		[seedlo],eax
		mov		[seedhi],edx
		mov		eax,edx
		pop		esi
		unuse_ebx
		ret

procend	lrand

;----------------------------------------------------------------------------
; ushort MGL_random(ushort max)
;----------------------------------------------------------------------------
; Return a random number between [0,max-1]
;----------------------------------------------------------------------------
procstartdll	_MGL_random

		ARG		max:S_USHORT

		push	_bp
		mov		_bp,_sp

		call	lrand
		xor     edx,edx
		mov		cx,[max]
		movzx	ecx,cx
		div    	ecx
		mov		_ax,_dx
		pop		_bp
		ret

procend			_MGL_random

;----------------------------------------------------------------------------
; ulong MGL_randoml(ulong max)
;----------------------------------------------------------------------------
; Return a random number between [0,max-1]
;----------------------------------------------------------------------------
procstartdll	_MGL_randoml

		ARG		max:ULONG

		push	_bp
		mov		_bp,_sp

		call	lrand
		xor     edx,edx
		div    	[max]
		mov		eax,edx
ife flatmodel
		shld	edx,eax,16
endif
		pop		_bp
		ret

procend			_MGL_randoml

endcodeseg	_random

		END						; End of module
