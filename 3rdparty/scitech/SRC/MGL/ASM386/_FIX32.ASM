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
;* Description:	Assembly language fixed point routines. Because we use
;*				32 bit assembler code in here, we make these routines part
;*				of our hybrid 16/32 code segment, so that they will be
;*				run as USE32 code and hence will execute faster than if
;*				compiled as USE16 code.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

header	_fix32					; Setup for MGL hybrid 16/32 bit segment

begcodeseg		_fix32			; Start of hybrid 16/32 code segment

;----------------------------------------------------------------------------
; long MGL_FixMul(long a,long b);
;----------------------------------------------------------------------------
; Multiplies two 32 bit fixed point values stored in long integers
; together and returns the result.
;----------------------------------------------------------------------------
procstartdll	_MGL_FixMul

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

procend			_MGL_FixMul

;----------------------------------------------------------------------------
; long MGL_FixDiv(long dividend,long divisor);
;----------------------------------------------------------------------------
; Divides two 32 bit fixed point values stored in long integers
; and returns the result. We use a modified method of 32 bit long division
; to compute the result.
;----------------------------------------------------------------------------
procstartdll	_MGL_FixDiv

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

procend			_MGL_FixDiv

;----------------------------------------------------------------------------
; zfix32_t MGL_ZFixDiv(zfix32_t dividend,fix32_t divisor);
;----------------------------------------------------------------------------
; Divides a 4.28 bit fixed point value by a 16.16 fixed point value,
; returning the result as a 4.28 fixed point value. This routine calculates
; the resulting 4.28 number with full precision, by converting the remainder
; of the original divide into the final fraction bits to be added to the
; result.
;----------------------------------------------------------------------------
procstartdll	_MGL_ZFixDiv

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

procend			_MGL_ZFixDiv

;----------------------------------------------------------------------------
; long MGL_FixMuldiv(long a,long b,long c);
;----------------------------------------------------------------------------
; Computes the 32 bit product of the following:
;
;		(a * b) / c
;
; Maintaining full 64 bit precision for the intermediate result of (a * b).
;----------------------------------------------------------------------------
procstartdll	_MGL_FixMulDiv

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

procend			_MGL_FixMulDiv

;----------------------------------------------------------------------------
; int MGL_backfacing(fix32_t dx1,fix32_t dy1,fix32_t dx2,fix32_t dy2);
;----------------------------------------------------------------------------
; Determine whether a polygon is backfacing given two fixed point vectors.
; We need to maintain full 64 bit precision during this calculation to
; avoid overflow for screen space polygons.
;----------------------------------------------------------------------------
procstartdll	_MGL_backfacing

		ARG     dx1:ULONG, dy1:ULONG, dx2:ULONG, dy2:ULONG

		push	_bp
		mov		_bp,_sp
		push	ebx

		mov		eax,[dx1]
		imul	[dy2]			; EDX:EAX := dx1 * dy2
		mov		ebx,eax
		mov		ecx,edx			; ECX:EBX := dx1 * dy2
		mov     eax,[dx2]
		imul	[dy1]			; EDX:EAX := dx2 * dy1
		sub		eax,ebx
		mov		_ax,1			; Default to backfacing
		sbb		edx,ecx			; EDX:EAX := dx1 * dy2 - dx2 * dy1
		jns		@@Backfacing
		xor		_ax,_ax			; Polygon is frontfacing
@@Backfacing:
		pop		ebx
		pop		_bp
		ret

procend			_MGL_backfacing

endcodeseg	_fix32

		END						; End of module
