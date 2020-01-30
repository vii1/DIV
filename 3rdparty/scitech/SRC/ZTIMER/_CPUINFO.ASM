;****************************************************************************
;*
;*                           Zen Timer Library
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
;* Language:    80386 Assembler
;* Environment: Intel 32 bit Protected Mode.
;*
;* Description: Code to determine the Intel processor type.
;*
;*
;****************************************************************************

        IDEAL

INCLUDE "model.mac"             	; Memory model macros

header  _cpuinfo              		; Set up memory model

begcodeseg  _cpuinfo          		; Start of code segment

MACRO	mCPU_ID
db	00Fh,0A2h
ENDM

MACRO	mRDTSC
db	00Fh,031h
ENDM

CPU_IntelClone	=	08000h			; CPU clone flag

P386                            	; Turn on 386 instructions

intel_id	db	"GenuineIntel"		; Intel vendor ID

;----------------------------------------------------------------------------
; bool _CPU_check80386(void)
;----------------------------------------------------------------------------
; Determines if we have an i386 processor.
;----------------------------------------------------------------------------
procstart	__CPU_check80386

		enter_c	0

		xor		edx,edx				; EDX = 0, not an 80386
		mov 	bx, sp
		and		sp, not 3
		pushfd						; Push original EFLAGS
		pop     eax					; Get original EFLAGS
		mov     ecx, eax			; Save original EFLAGS
		xor     eax, 40000h			; Flip AC bit in EFLAGS
		push    eax             	; Save new EFLAGS value on
									;   stack
		popfd                   	; Replace current EFLAGS value
		pushfd						; Get new EFLAGS
		pop     eax             	; Store new EFLAGS in EAX
		xor     eax, ecx        	; Can't toggle AC bit,
									;   processor=80386
		jnz     @@Done				; Jump if not an 80386 processor
		inc		edx					; We have an 80386

@@Done:	push	ecx
		popfd
		mov		sp, bx
		mov		eax, edx
		leave_c
		ret

procend		__CPU_check80386

;----------------------------------------------------------------------------
; bool _CPU_check80486(void)
;----------------------------------------------------------------------------
; Determines if we have an i486 processor.
;----------------------------------------------------------------------------
procstart	__CPU_check80486

		enter_c	0

; Distinguish between the i486 and Pentium by the ability to set the ID flag
; in the EFLAGS register. If the ID flag is set, then we can use the CPUID
; instruction to determine the final version of the chip. Otherwise we
; simply have an 80486.

; Distinguish between the i486 and Pentium by the ability to set the ID flag
; in the EFLAGS register. If the ID flag is set, then we can use the CPUID
; instruction to determine the final version of the chip. Otherwise we
; simply have an 80486.

		pushfd						; Get original EFLAGS
		pop		eax
		mov 	ecx, eax
		xor		eax, 200000h		; Flip ID bit in EFLAGS
		push	eax					; Save new EFLAGS value on stack
		popfd						; Replace current EFLAGS value
		pushfd						; Get new EFLAGS
		pop     eax					; Store new EFLAGS in EAX
		xor     eax, ecx			; Can not toggle ID bit,
		jnz     @@1					; Processor=80486
		mov		eax,1				; We dont have a Pentium
		jmp		@@Done
@@1:	mov		eax,0				; We have Pentium or later
@@Done:	leave_c
		ret

procend		__CPU_check80486

;----------------------------------------------------------------------------
; bool _CPU_haveCPUID(void)
;----------------------------------------------------------------------------
; Determines if we have support for the CPUID instruction.
;----------------------------------------------------------------------------
procstart	__CPU_haveCPUID

		enter_c	0

if flatmodel
		pushfd						; Get original EFLAGS
		pop		eax
		mov 	ecx, eax
		xor		eax, 200000h		; Flip ID bit in EFLAGS
		push	eax					; Save new EFLAGS value on stack
		popfd						; Replace current EFLAGS value
		pushfd						; Get new EFLAGS
		pop     eax					; Store new EFLAGS in EAX
		xor     eax, ecx			; Can not toggle ID bit,
		jnz     @@1					; Processor=80486
		mov		eax,0				; We dont have CPUID support
		jmp		@@Done
@@1:	mov		eax,1				; We have CPUID support
else
		mov		eax,0				; CPUID requires 32-bit pmode
endif
@@Done:	leave_c
		ret

procend		__CPU_haveCPUID

;----------------------------------------------------------------------------
; uint _CPU_checkCPUID(void)
;----------------------------------------------------------------------------
; Determines the CPU type using the CPUID instruction.
;----------------------------------------------------------------------------
procstart	__CPU_checkCPUID

		enter_c	0

		xor     eax, eax			; Set up for CPUID instruction
		mCPU_ID						; Get and save vendor ID
		cmp     eax, 1				; Make sure 1 is valid input for CPUID
		jl      @@Fail				; We dont have the CPUID instruction
		xor		eax,eax				; Assume Genuine Intel
		cmp		[DWORD intel_id], ebx
		jne		@@NotGenuineIntel
		cmp		[DWORD intel_id+4], edx
		jne		@@NotGenuineIntel
		cmp		[DWORD intel_id+8], ecx
		je		@@HaveGenuineIntel

@@NotGenuineIntel:
		mov		eax,CPU_IntelClone	; Set the clone flag

@@HaveGenuineIntel:
		push	eax
		xor     eax, eax
		inc		eax
		mCPU_ID						; Get family/model/stepping/features
		and		eax, 0F00h
		shr     eax, 8				; Isolate family
		and		eax, 0Fh
		pop		ecx
		or		eax,ecx				; Combine in the clone flag
@@Done:	leave_c
		ret

@@Fail:	xor		eax,eax
		jmp		@@Done

procend		__CPU_checkCPUID

;----------------------------------------------------------------------------
; uint _CPU_getCPUIDModel(void)
;----------------------------------------------------------------------------
; Determines the CPU type using the CPUID instruction.
;----------------------------------------------------------------------------
procstart	__CPU_getCPUIDModel

		enter_c	0

		xor     eax, eax			; Set up for CPUID instruction
		mCPU_ID						; Get and save vendor ID
		cmp     eax, 1				; Make sure 1 is valid input for CPUID
		jl      @@Fail				; We dont have the CPUID instruction
		xor     eax, eax
		inc		eax
		mCPU_ID						; Get family/model/stepping/features
		and		eax, 0F0h
		shr     eax, 4				; Isolate model
@@Done:	leave_c
		ret

@@Fail:	xor		eax,eax
		jmp		@@Done

procend		__CPU_getCPUIDModel

;----------------------------------------------------------------------------
; uint _CPU_getCPUIDFeatures(void)
;----------------------------------------------------------------------------
; Determines the CPU type using the CPUID instruction.
;----------------------------------------------------------------------------
procstart	__CPU_getCPUIDFeatures

		enter_c	0

		xor     eax, eax			; Set up for CPUID instruction
		mCPU_ID						; Get and save vendor ID
		cmp     eax, 1				; Make sure 1 is valid input for CPUID
		jl      @@Fail				; We dont have the CPUID instruction
		xor     eax, eax
		inc		eax
		mCPU_ID						; Get family/model/stepping/features
		mov		eax, edx
@@Done:	leave_c
		ret

@@Fail:	xor		eax,eax
		jmp		@@Done

procend		__CPU_getCPUIDFeatures

;----------------------------------------------------------------------------
; bool _CPU_checkClone(void)
;----------------------------------------------------------------------------
; Checks if the i386 or i486 processor is a clone or genuine Intel.
;----------------------------------------------------------------------------
procstart	__CPU_checkClone

		enter_c	0

		mov		ax,5555h			; Check to make sure this is a 32-bit processor
		xor 	dx,dx
		mov 	cx,2h
		div		cx					; Perform Division
		clc
		jnz 	@@NoClone
		jmp		@@Clone
@@NoClone:
		stc
@@Clone:
		pushfd
		pop 	eax          		; Get the flags
		and 	eax,1
		xor	 	eax,1        		; EAX=0 is probably Intel, EAX=1 is a Clone

		leave_c
		ret

procend		__CPU_checkClone

;----------------------------------------------------------------------------
; ulong _CPU_quickRDTSC(void)
;----------------------------------------------------------------------------
; Reads the time stamp counter and returns the low order 32-bits
;----------------------------------------------------------------------------
procstart	__CPU_quickRDTSC

		mRDTSC
		ret

procend		__CPU_quickRDTSC

;----------------------------------------------------------------------------
; void _CPU_runBSFLoop(int interations)
;----------------------------------------------------------------------------
; Runs a loop of BSF instructions for the specified number of iterations
;----------------------------------------------------------------------------
procstart	__CPU_runBSFLoop

		ARG		iterations:UINT

		push	_bp
		mov		_bp,_sp
		push	_bx

		mov		_dx,[iterations]
		mov 	eax,80000000h
		mov		_bx,_dx

		ALIGN	4

@@loop:	bsf 	ecx,eax
		dec		_bx
		jnz		@@loop

		pop		_bx
		pop		_bp
		ret

procend		__CPU_runBSFLoop

;----------------------------------------------------------------------------
; void  _CPU_readTimeStamp(CPU_largeInteger *time);
;----------------------------------------------------------------------------
; Reads the time stamp counter and returns the 64-bit result.
;----------------------------------------------------------------------------
procstart	__CPU_readTimeStamp

		mRDTSC
		mov		ecx,[esp+4]		; Access directly without stack frame
		mov		[ecx],eax
		mov		[ecx+4],edx
		ret

procend		__CPU_readTimeStamp

;----------------------------------------------------------------------------
; ulong _CPU_diffTime64(CPU_largeInteger *t1,CPU_largeInteger *t2,CPU_largeInteger *t)
;----------------------------------------------------------------------------
; Computes the difference between two 64-bit numbers.
;----------------------------------------------------------------------------
procstart	__CPU_diffTime64

		ARG		t1:DPTR, t2:DPTR, t:DPTR

		enter_c	0

        mov		ecx,[t2]
		mov 	eax,[ecx]		; EAX := t2.low
		mov		ecx,[t1]
		sub 	eax,[ecx]
		mov 	edx,eax			; EDX := low difference
        mov		ecx,[t2]
		mov 	eax,[ecx+4]		; ECX := t2.high
		mov		ecx,[t1]
		sbb 	eax,[ecx+4]		; EAX := high difference

		mov		ebx,[t]			; Store the result
		mov		[ebx],edx		; Store low part
		mov		[ebx+4],eax		; Store high part
		mov		eax,edx			; Return low part
ife flatmodel
		shld	edx,eax,16		; Return in DX:AX
endif
		leave_c
		ret

procend		__CPU_diffTime64

;----------------------------------------------------------------------------
; ulong _CPU_calcMicroSec(CPU_largeInteger *count,ulong freq);
;----------------------------------------------------------------------------
; Computes the value in microseconds for the elapsed time with maximum
; precision. The formula we use is:
;
;	us = (((diff * 0x100000) / freq) * 1000000) / 0x100000)
;
; The power of two multiple before the first divide allows us to scale the
; 64-bit difference using simple shifts, and then the divide brings the
; final result into the range to fit into a 32-bit integer.
;----------------------------------------------------------------------------
procstart	__CPU_calcMicroSec

		ARG		count:DPTR, freq:DPTR

		enter_c	0

		mov		ecx,[count]
		mov		eax,[ecx]		; EAX := low part
		mov		edx,[ecx+4]		; EDX := high part
		shld	edx,eax,20
		shl		eax,20			; diff * 0x100000
		div		[freq]			; (diff * 0x100000) / freq
		mov		ecx,1000000
        xor		edx,edx
		mul		ecx				; ((diff * 0x100000) / freq) * 1000000)
		shrd	eax,edx,20		; ((diff * 0x100000) / freq) * 1000000) / 0x100000
ife flatmodel
		shld	edx,eax,16		; Return in DX:AX
endif
		leave_c
		ret

procend		__CPU_calcMicroSec

endcodeseg  _cpuinfo

		END                     ; End of module
