;****************************************************************************
;*
;*				  High Speed Fixed/Floating Point Library
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
;* Description:	This module contains routines for high performance 16.16
;*				fixed point square root operations using table lookup
;*				and linear interpolation:
;*
;*                  F386_sqrt       - 16.16 square root
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros

header	fxsqrt					; Set up memory model

begdataseg	fxsqrt

		$EXTRN	_FXsqrt_table,ULONG

enddataseg	fxsqrt

begcodeseg	fxsqrt

;----------------------------------------------------------------------------
; FXFixed F386_sqrt(FXFixed f);
;----------------------------------------------------------------------------
; Computes the square root of a fixed point number using a table lookup.
;----------------------------------------------------------------------------
procstart	_F386_sqrt

		ARG		f:ULONG

if flatmodel
		push	ebx				; Save value of EBX across call
		mov		ebx,[esp+8]		; EBX := fixed point number
else
		push	_bp
		mov		_bp,_sp
		mov		ebx,[f]			; EBX := fixed point number
endif

; First we isolate the first 8 bits of the mantissa in our
; fixed point number. We do this by scanning along until we
; find the first 1 bit in the number, and shift it all right
; until this is in bit position 7. Since IEEE floating point
; numbers have an implied 1 bit in the mantissa, we mask this
; bit out and use the 7 bits as an index into the table. The
; 8th bit is a 1 if the exponent of our fixed point number
; was odd (position of the binary point with respect to the
; first 1 bit in the number).

		or		ebx,ebx
		jle		@@Invalid		; Taking sqrt of 0 or -ve number

; Search for the index of the first 1 bit in the number (start of
; the mantissa. Note that we are only working with positive numbers
; here, so we ignore the sign bit (bit 31).

		shl		ebx,1			; Mask out the sign bit
		bsr		ecx,ebx			; ECX := index of 1st 1 bit in EBX
		mov		_ax,_cx
		xor		cl,31			; CL := bits to shift left
		sub		_ax,17			; _AX := exponent for number
		shl		ebx,cl			; Normalise so first 1 bit is in bit pos 31

; Compute the index into the table by shifting the mantissa
; so that the first 1 bit ends up in bit position 7, and mask it
; out. If the exponent is odd then we index into the second half
; of the table.

		xor		edx,edx			; Clear interpolant
		shr		ebx,8
		mov		dx,bx			; EDX := interpolant
		shr		ebx,14			; _BX := top 7 bits of mantissa + implied 1
		test	_ax,1			; Is the exponent odd?
		jz		@@EvenExp		; Yes, dont modify
		and		_bx,03FCh		; Index into second half of table
		jmp		@@DoLookup

@@EvenExp:
		and		_bx,01FCh		; Index into second half of table

; Divide the exponent by two (square root it) while preserving the
; sign, then construct the square root by combining the mantissa from
; the lookup table and the square rooted exponent. To do this we add
; the implied 1 bit to the number and shift it the number of places
; indicated by the exponent. A positive exponent means shift left, negative
; means shift right.

@@DoLookup:
		sar		_ax,1			; Divide exponent by 2
		sub		_ax,7			; Adjust to 24 bits of precision in table
		mov		_cx,_ax			; _CX := exponent of final result
		mov		eax,[_FXsqrt_table+_bx+4]	; EAX := FXsqrt_table[index+1]
		mov		ebx,[_FXsqrt_table+_bx] 	; EBX := FXsqrt_table[index]
		sub     eax,ebx			; EAX := difference
		or		_cx,_cx			; Check sign of exponent
		jl		@@Neg			; Exponent is negative
		shl		eax,cl			; Shift the result to correct position
		shl		ebx,cl
		jmp		@@Interpolate
@@Neg:	neg		_cx				; Negative exponent shift right
		shr		eax,cl			; Shift the result to correct position
		shr		ebx,cl

; Add in the interpolated difference

@@Interpolate:
		imul	edx
		shrd	eax,edx,16		; EAX := diff * interpolant
		adc		eax,ebx			; Add in to result

ife flatmodel
		shld	edx,eax,16		; whole part of result in DX;
								; fractional part is already in AX
endif

@@Exit:
if flatmodel
		pop		ebx				; Restore EBX
else
		pop		_bp
endif
		ret

@@Invalid:
		xor		_ax,_ax			; Invalid - return 0
		xor		_dx,_dx
		jmp		@@Exit

procend		_F386_sqrt

endcodeseg	fxsqrt

		END						; End of module
