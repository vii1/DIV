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
;*				fixed point trig operations using table lookup and linear
;*				interpolation:
;*
;*                  F386_sin	- 16.16 sine
;*					F386_cos	- 16.16 cosine
;*					F386_tan	- 16.16 tangent
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros

header	fxtrig					; Set up memory model

begdataseg	fxtrig

		$EXTRN	_FXsin_table,ULONG

enddataseg	fxtrig

begcodeseg	fxtrig

;----------------------------------------------------------------------------
; FXFixed F386_sin(FXFixed angle)
;----------------------------------------------------------------------------
; Computes the sine of a fixed point angle and returns the result in fixed
; point. We use table lookup to find the closest values, then interpolate
; the result to get the accuracy we need.
;----------------------------------------------------------------------------
procstart	_F386_sin

		ARG		angle:ULONG

		enter_c	0

; Ensure that the angle is within the range 0-360 degrees

		mov		eax,[angle]

@@WhileLess:
		cmp		eax,0
		jge		@@WhileLarger
		add		eax,1680000h	; Add 360 degrees
		jmp		@@WhileLess

@@WhileLarger:
		cmp		eax,1680000h
		jle		@@Convert
		sub		eax,1680000h	; Subtract 360 degrees
		jmp		@@WhileLarger

; Convert angle so that 90 degrees = 256. This gives us the following
; as the resulting number:
;
;   bits 25-26     16-24         0-15
;  +----------+-------------+-------------+
;  |.quadrant.|.table index.|.interpolant.|
;  +----------+-------------+-------------+

@@Convert:
		mov		edx,2D82Dh		; EDX := REAL(256.0 / 90.0) = 2.84444..
		imul	edx
		shrd	eax,edx,16		; EAX := angle * 2.8444...
		adc		eax,0

		mov		ebx,eax
		mov		esi,eax
		mov     edx,eax

		shr		ebx,14			; EBX := 8 bit table index
		and		ebx,03FCh
		test	esi,01000000h	; Handle quadrants 1 & 3
		jz		@@Quad02
		not		edx				; Negate interpolation factor
		xor		ebx,03FCh		; Index table in reverse order

@@Quad02:
		mov     ecx,[_FXsin_table + _bx]	; ECX := FXsin_table[index]
		mov     eax,[_FXsin_table + _bx+4]	; EAX := FXsin_table[index+1]
		sub		eax,ecx					; EAX := diff

; Compute the interpolation factor and add in the interpolated difference

		and		edx,0FFFFh		; Mask out interpolation value
		imul	edx
		shrd	eax,edx,16		; EAX := diff * (a & 0xFFFF)
		adc		ecx,eax			; Add in to result

; Handle quadrants 3 & 4 where the values are negated

		test	esi,02000000h
		jz		@@Positive
		neg		ecx				; Negate the result

@@Positive:
		mov		eax,ecx
ife flatmodel
		shld	edx,eax,16		; Position so result ends up in DX:AX
endif

		leave_c_nolocal
		ret

procend		_F386_sin

;----------------------------------------------------------------------------
; FXFixed F386_cos(FXFixed angle)
;----------------------------------------------------------------------------
; Computes the cosine of a fixed point angle and returns the result in fixed
; point. Simply calls the F386_sin routine to perform the calculation.
;----------------------------------------------------------------------------
procstart	_F386_cos

		ARG		angle:ULONG

if flatmodel
		add		[ULONG esp+4],5A0000h	; Access directly on stack
		jmp		_F386_sin
else
		push	_bp
		mov		_bp,_sp

		mov		eax,[angle]
		add		eax,5A0000h		; EAX := angle + REAL(90)
		push	eax
		call    _F386_sin
		pop     ecx

		pop		_bp
		ret
endif

procend		_F386_cos

;----------------------------------------------------------------------------
; FXFixed F386_tan(FXFixed angle)
;----------------------------------------------------------------------------
; Computes the tangent of a fixed point angle and returns the result in
; fixed point.
;----------------------------------------------------------------------------
procstart	_F386_tan

		ARG		angle:ULONG

if flatmodel
		mov		eax,[esp+4]		; Access directly on stack frame
else
		push	_bp
		mov		_bp,_sp

		mov		eax,[angle]
endif

; Ensure that the angle is within the range 0-360 degrees

@@WhileLess:
		cmp		eax,0
		jge		@@WhileLarger
		add		eax,1680000h	; Add 360 degrees
		jmp		@@WhileLess

@@WhileLarger:
		cmp		eax,1680000h
		jle		@@OK
		sub		eax,1680000h	; Subtract 360 degrees
		jmp		@@WhileLarger

@@OK:	cmp		eax,5A0000h		; EAX == REAL(90)?
		je		@@Invalid
		cmp		eax,10E0000h	; EAX == REAL(270)?
		je		@@Invalid

		push	eax
		call    _F386_sin
		pop     ecx
		push	eax

		add		ecx,5A0000h
		push	ecx
		call	_F386_sin
		pop		ecx
		pop		edx				; EDX := FXsin(angle)
		mov		ecx,eax			; ECX := FXcos(angle)

		xor		eax,eax
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	ecx				; EAX := FXsin(angle) / FXcos(angle)

if flatmodel
@@Exit:	ret
else
@@Exit:	shld	edx,eax,16		; Return result in DX:AX
		pop		_bp
		ret
endif

@@Invalid:
		mov		eax,7FFFFFFFh
		jmp		@@Exit

procend		_F386_tan

endcodeseg	fxtrig

		END						; End of module
