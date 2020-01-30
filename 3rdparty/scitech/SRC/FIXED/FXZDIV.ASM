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
;* Description:	This module contains routines for dividing fixed point
;*				numbers into 4.28 format with full precision using
;*				32 bit 80386 assembly code:
;*
;*                  F386_divFF      - 4.28 = 16.16 / 16.16
;*                  F386_divZF      - 4.28 = 4.28 / 16.16
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros

header	fxzdiv					; Set up memory model

begcodeseg	fxzdiv

;----------------------------------------------------------------------------
; FXZFixed F386_divFF(FXFixed dividend,FXFixed divisor);
;----------------------------------------------------------------------------
; Divides a 16.16 fixed point value by a 16.16 fixed point value,
; returning the result as a 4.28 fixed point value. This routine calculates
; the resulting 4.28 number with full precision, by converting the remainder
; of the original divide into the final fraction bits to be added to the
; result.
;----------------------------------------------------------------------------
procstart	_F386_divFF

		ARG		dividend:ULONG, divisor:ULONG

if flatmodel
		mov		edx,[esp+4]		; Access directly without stack frame
		xor		eax,eax
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	[ULONG esp+8]

; Compute remaining 12 bits of precision

		mov		ecx,eax
		shl		edx,12			; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx			; Position so result ends up in EAX
		div		[ULONG esp+8]	; Find last 12 fractional bits
		shl		eax,20			; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12		; ECX := result
		mov		eax,ecx
else
		push	_bp
		mov		_bp,_sp

		mov		edx,[dividend]
		xor		eax,eax
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	[divisor]

; Compute remaining 12 bits of precision

		mov		ecx,eax
		shl		edx,12			; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx			; Position so result ends up in EAX
		div		[divisor]		; Find last 12 fractional bits
		shl		eax,20			; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12		; ECX := result
		mov		eax,ecx

		shld	edx,eax,16		; Put result in DX:AX
		pop		_bp
endif
		ret

procend		_F386_divFF

;----------------------------------------------------------------------------
; FXZFixed F386_divZF(FXZFixed dividend,FXFixed divisor);
;----------------------------------------------------------------------------
; Divides a 4.28 fixed point value by a 16.16 fixed point value,
; returning the result as a 4.28 fixed point value. This routine calculates
; the resulting 4.28 number with full precision, by converting the remainder
; of the original divide into the final fraction bits to be added to the
; result.
;----------------------------------------------------------------------------
procstart	_F386_divZF

		ARG		dividend:ULONG, divisor:ULONG

if flatmodel
		mov		edx,[esp+4]		; Access directly without stack frame
		xor		eax,eax
		shrd	eax,edx,28		; position so that result ends up
		sar		edx,28			; in EAX
		idiv	[ULONG esp+8]

; Compute remaining 12 bits of precision

		mov		ecx,eax
		shl		edx,12			; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx			; Position so result ends up in EAX
		div		[ULONG esp+8]	; Find last 12 fractional bits
		shl		eax,20			; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12		; ECX := result
		mov		eax,ecx
else
		push	_bp
		mov		_bp,_sp

		mov		edx,[dividend]
		xor		eax,eax
		shrd	eax,edx,28		; position so that result ends up
		sar		edx,28			; in EAX
		idiv	[divisor]		; EAX := top 20 bits in 19:0

; Compute remaining 12 bits of precision

		mov		ecx,eax
		shl		edx,12			; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx			; Position so result ends up in EAX
		div		[divisor]		; Find last 12 fractional bits
		shl		eax,20			; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12		; ECX := result
		mov		eax,ecx

		shld	edx,eax,16		; Put result in DX:AX
		pop		_bp
endif
		ret

procend		_F386_divZF

endcodeseg	fxzdiv

		END						; End of module
