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
;* Description:	This module contains routines for manipulating 32 bit
;*				fixed point numbers (16.16 format) using 32 bit 80386
;*				assembly code:
;*
;*                  F386_mul        - 16.16 Fixed point multiplication
;*                  F386_div        - 16.16 Fixed point division
;*                  F386_muldiv     - 16.16 Fixed point multiply then divide
;*                  F386_square     - 16.16 Fixed point square
;*                  F386_oneOver    - 16.16 Fixed point inversion
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros

header	fxmisc					; Set up memory model

begcodeseg	fxmisc

;----------------------------------------------------------------------------
; FXFixed F386_mul(FXFixed a,FXFixed b);
;----------------------------------------------------------------------------
; Multiplies two 32 bit fixed point values stored in FXFixed integers
; together and returns the result.
;----------------------------------------------------------------------------
procstart	_F386_mul

		ARG		a:ULONG, b:ULONG

if flatmodel
		mov		eax,[esp+4]		; Access directly without stack frame
		imul	[ULONG esp+8]
		add		eax,8000h		; Round by adding 2^-17
		adc		edx,0			; Whole part of result is in DX
		shrd	eax,edx,16		; EAX := a * b
else
		push	_bp
		mov		_bp,_sp
		mov		eax,[a]
		imul	[b]				; EDX:EAX = result
		add		eax,8000h		; Round by adding 2^-17
		adc		edx,0			; Whole part of result is in DX
		shr		eax,16			; Shift the fractional part into AX
		pop		_bp
endif
		ret

procend		_F386_mul

;----------------------------------------------------------------------------
; FXFixed F386_Div(FXFixed dividend,FXFixed divisor);
;----------------------------------------------------------------------------
; Divides two 32 bit fixed point values stored in FXFixed integers
; and returns the result. We use a modified method of 32 bit FXFixed division
; to compute the result.
;----------------------------------------------------------------------------
procstart	_F386_div

		ARG		dividend:ULONG, divisor:ULONG

if flatmodel
		mov		edx,[esp+4]		; Access directly without stack frame
		xor		eax,eax
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	[ULONG esp+8]
else
		push	_bp
		mov		_bp,_sp

		mov		edx,[dividend]
		xor		eax,eax
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	[divisor]

		shld	edx,eax,16		; whole part of result in DX;
								; fractional part is already in AX
		pop		_bp
endif
		ret

procend		_F386_div

;----------------------------------------------------------------------------
; FXFixed F386_muldiv(FXFixed a,FXFixed b,FXFixed c);
;----------------------------------------------------------------------------
; Computes the 32 bit product of the following:
;
;		(a * b) / c
;
; Maintaining full 64 bit precision for the intermediate result of (a * b).
;----------------------------------------------------------------------------
procstart	_F386_muldiv

		ARG		a:ULONG, b:ULONG, c:ULONG

if flatmodel
		mov		eax,[esp+4]		; Access directly without stack frame
		imul	[ULONG esp+8]	; EDX:EAX := 64 bit dividend
		idiv	[ULONG esp+12]	; Divide the 64 bit dividend
else
		push	_bp
		mov		_bp,_sp
		mov		eax,[a]
		imul	[b]				; EDX:EAX := 64 bit dividend
		idiv	[c]				; Divide the 64 bit dividend
		shld	edx,eax,16		; whole part of result in DX;
								; fractional part is already in AX
		pop		_bp
endif
		ret

procend		_F386_muldiv

;----------------------------------------------------------------------------
; FXFixed F386_square(FXFixed a);
;----------------------------------------------------------------------------
; Squares a 32 bit fixed point value and returns the result.
;----------------------------------------------------------------------------
procstart	_F386_square

		ARG		a:ULONG

if flatmodel
		mov		eax,[esp+4]		; Access directly without stack frame
		imul	eax
		add		eax,8000h		; Round by adding 2^-17
		adc		edx,0			; Whole part of result is in DX
		shrd	eax,edx,16		; EAX := a * b
else
		push	_bp
		mov		_bp,_sp
		mov		eax,[a]
		imul	eax				; EDX:EAX = result
		add		eax,8000h		; Round by adding 2^-17
		adc		edx,0			; Whole part of result is in DX
		shr		eax,16			; Shift the fractional part into AX
		pop		_bp
endif
		ret

procend		_F386_square

;----------------------------------------------------------------------------
; FXFixed F386_oneOver(FXFixed divisor);
;----------------------------------------------------------------------------
; Computes the value of 1.0 / divisor and returns the result.
;----------------------------------------------------------------------------
procstart	_F386_oneOver

		ARG		divisor:ULONG

if flatmodel
		xor		eax,eax
		mov		edx,1
		idiv	[ULONG esp+4]
else
		push	_bp
		mov		_bp,_sp

		xor		eax,eax
		mov		edx,1
		idiv	[divisor]

		shld	edx,eax,16		; whole part of result in DX;
								; fractional part is already in AX
		pop		_bp
endif
		ret

procend		_F386_oneOver

endcodeseg	fxmisc

		END						; End of module
