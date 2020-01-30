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
;*				fixed point logarithm operations using table lookup and
;*				linear interpolation:
;*
;*					F386_log10	- 16.16 base 10 logarithm
;*                  F386_log	- 16.16 natural logarithm
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros

header	fxlog					; Set up memory model

begdataseg	fxlog

		$EXTRN	_FXlog10_table,USHORT
		$EXTRN	_FXlog2_table,ULONG

enddataseg	fxlog

begcodeseg	fxlog

;----------------------------------------------------------------------------
; FXFixed F386_log10(FXFixed f);
;----------------------------------------------------------------------------
; Computes the common logarithm of a fixed point number using a table
; lookup and linear interpolation.
;----------------------------------------------------------------------------
procstart	_F386_log10

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
; bit out and use the 7 bits as an index into the table.

		or		ebx,ebx
		jle		@@Invalid		; Taking log10 of 0 or -ve number

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
; out.

		xor		edx,edx			; Clear interpolant
		shr		ebx,8
		mov		dx,bx			; EDX := interpolant
		shr		ebx,15			; _BX := top 7 bits of mantissa + implied 1
		and		_bx,0FEh		; Mask out index

; Lookup the values for the 7 bits of mantissa in the table, and
; linearly interpolate between the two entries.

		mov		_cx,_ax			; _CX := exponent of final result
		xor		eax,eax			; Clear EAX
								; EBX high word is clear here!
		mov		ax,[_FXlog10_table+_bx+2]	; EAX := FXlog10_table[index+1]
		mov		bx,[_FXlog10_table+_bx] 	; EBX := FXlog10_table[index]
		sub     eax,ebx			; EAX := difference
		imul	edx				; Interpolate
		shrd	eax,edx,16		; EAX := diff * interpolant
		adc		eax,ebx			; Add in to result

; Now find the appropriate power of 2 logarithm to add to the final
; result.

		or		_cx,_cx
		js		@@Neg

		mov		_bx,_cx
		shl		_bx,2			; _BX = index into powers of 2 table
		add		eax,[_FXlog2_table+_bx]
		jmp		@@Done

@@Neg:
		neg		_cx
		mov		_bx,_cx
		shl		_bx,2			; _BX = index into powers of 2 table
		sub		eax,[_FXlog2_table+_bx]

@@Done:
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

procend		_F386_log10

;----------------------------------------------------------------------------
; FXFixed F386_log(FXFixed f);
;----------------------------------------------------------------------------
; Computes the common logarithm of a fixed point number using a table
; lookup and linear interpolation.
;----------------------------------------------------------------------------
procstart	_F386_log

		ARG		f:ULONG

if flatmodel
		mov		eax,[esp+4]		; Access directly on stack
else
		push	_bp
		mov		_bp,_sp
		mov		eax,[f]
endif
		push	eax
		call    _F386_log10
		pop     ecx
		mov		edx,24D76h		; EDX := REAL(2.302585093)
		imul	edx
		add		eax,8000h		; Round by adding 2^-17
		adc		edx,0			; Whole part of result is in DX
if flatmodel
		shrd	eax,edx,16		; EAX := f * REAL(2.302585093)
else
		shr		eax,16			; Shift the fractional part into AX
		pop		_bp
endif
		ret

procend		_F386_log

endcodeseg	fxlog

		END						; End of module
