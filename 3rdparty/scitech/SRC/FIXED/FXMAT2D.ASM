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
;*				fixed point 2D transformation matrix manipulation routines:
;*
;*					F386_scale2D		- Concat a 2D scale matrix
;*					F386_scaleAbout2D	- Concat a 2D scale about matrix
;*					F386_rotate2D		- Concat a 2D rotate matrix
;*					F386_rotateAbout2D	- Concat a 2D rotate about matrix
;*					F386_concat3x3		- Concat two 3x3 matrices
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros
INCLUDE "fxmacs.mac"			; Fixed point macros

header	fxmat2d				; Set up memory model

begcodeseg	fxmat2d

;----------------------------------------------------------------------------
; void F386_scale2D(long *m,long xscale,long yscale)
;----------------------------------------------------------------------------
; Concatenate a scale transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_scale2D

		ARG     m:DPTR, xscale:ULONG, yscale:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		mov		ebx,[xscale]	; EBX := x scale factor

off = 0
		REPT	3
		mov		eax,[_si+off]
		imul	ebx				; EDX:EAX := mat[0][off] * xscale
		ROUNDIT
		mov		[_si+off],eax	; Store result
off = off + 4
		ENDM

		mov		ebx,[yscale]	; EBX := y scale factor

		REPT	3
		mov		eax,[_si+off]
		imul	ebx				; EDX:EAX := mat[1][off] * yscale
		ROUNDIT
		mov		[_si+off],eax	; Store result
off = off + 4
		ENDM

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_scale2D

;----------------------------------------------------------------------------
; void F386_scaleAbout2D(long *m,long xscale,long yscale,long x,long y)
;----------------------------------------------------------------------------
; Concatenate a scale transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_scaleAbout2D

		ARG     m:DPTR, xscale:ULONG, yscale:ULONG, x:ULONG, y:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		mov		ebx,[xscale]	; EBX := x scale factor

		mov		eax,[_si]
		imul	ebx				; EDX:EAX := mat[0][0] * xscale
		ROUNDIT
		mov		[_si],eax		; Store result
		mov		eax,[_si+4]
		imul	ebx				; EDX:EAX := mat[0][1] * xscale
		ROUNDIT
		mov		[_si+4],eax		; Store result
		mov		eax,[_si+8]
		imul	ebx				; EDX:EAX := mat[0][1] * xscale
		ROUNDIT
		sub		ebx,10000h
		neg		ebx				; EBX := 1 - xscale
		xchg	eax,ebx			; EBX := mat[0][1] * xscale
		imul	[x]				; EDX:EAX := x * (1 - xscale)
		ROUNDIT
		add		eax,ebx
		mov		[_si+8],eax		; Store result

		mov		ebx,[yscale]	; EBX := y scale factor

		mov		eax,[_si+12]
		imul	ebx				; EDX:EAX := mat[0][0] * xscale
		ROUNDIT
		mov		[_si+12],eax	; Store result
		mov		eax,[_si+16]
		imul	ebx				; EDX:EAX := mat[0][1] * xscale
		ROUNDIT
		mov		[_si+16],eax		; Store result
		mov		eax,[_si+20]
		imul	ebx				; EDX:EAX := mat[0][1] * xscale
		ROUNDIT
		sub		ebx,10000h
		neg		ebx				; EBX := 1 - xscale
		xchg	eax,ebx			; EBX := mat[0][1] * xscale
		imul	[y]				; EDX:EAX := x * (1 - xscale)
		ROUNDIT
		add		eax,ebx
		mov		[_si+20],eax	; Store result

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_scaleAbout2D

;----------------------------------------------------------------------------
; void F386_rotate2D(long *T,long *m,long Sin,long Cos)
;----------------------------------------------------------------------------
; Concatenate a rotate transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_rotate2D

		ARG     T:DPTR, m:DPTR, Sin:ULONG, Cos:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		_les	_di,[T]			; ES:_DI -> resultant temporary matrix

off = 0
		REPT	3
		mov		eax,[_si+off]
		imul	[Cos]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Cos
		mov		ebx,edx
		mov		eax,[_si+off+12]
		imul	[Sin]
		sub     ecx,eax			; EBX:ECX := mat[0][off] * Cos - mat[1][off] * Sin
		sbb		ebx,edx
		ROUNDIT_EBX_ECX
		mov		[_ES _di+off],eax; Store result
off = off + 4
		ENDM

off = 0
		REPT	3
		mov		eax,[_si+off]
		imul	[Sin]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Sin
		mov		ebx,edx
		mov		eax,[_si+off+12]
		imul	[Cos]
		add     eax,ecx			; EDX:EAX := mat[1][off] * Cos + mat[0][off] * Sin
		adc		edx,ebx
		ROUNDIT
		mov		[_ES _di+off+12],eax; Store result
off = off + 4
		ENDM

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_rotate2D

;----------------------------------------------------------------------------
; void F386_rotateAbout2D(long *T,long *m,long Sin,long Cos,long x,long y)
;----------------------------------------------------------------------------
; Concatenate a rotate transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_rotateAbout2D

		ARG     T:DPTR, m:DPTR, Sin:ULONG, Cos:ULONG, x:ULONG, y:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		_les	_di,[T]			; ES:_DI -> resultant temporary matrix

off = 0
		REPT	2
		mov		eax,[_si+off]
		imul	[Cos]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Cos
		mov		ebx,edx
		mov		eax,[_si+off+12]
		imul	[Sin]
		sub     ecx,eax			; EBX:ECX := mat[0][off] * Cos - mat[1][off] * Sin
		sbb		ebx,edx
		ROUNDIT_EBX_ECX
		mov		[_ES _di+off],eax; Store result
off = off + 4
		ENDM

		mov		eax,[_si+off]
		imul	[Cos]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Cos
		mov		ebx,edx
		mov		eax,[_si+off+12]
		imul	[Sin]
		sub     ecx,eax			; EBX:ECX := mat[0][off] * Cos - mat[1][off] * Sin
		sbb		ebx,edx
		mov		eax,10000h
		sub		eax,[Cos]		; EAX := 1 - Cos
		imul	[x]				; EAX := x * (1-Cos)
		add		ecx,eax			; Add to total
		adc     ebx,edx
		mov		eax,[Sin]
		imul	[y]				; EAX := y * Sin
		add     eax,ecx
		adc		edx,ebx
		ROUNDIT
		mov		[_ES _di+off],eax; Store result

off = 0
		REPT	2
		mov		eax,[_si+off]
		imul	[Sin]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Sin
		mov		ebx,edx
		mov		eax,[_si+off+12]
		imul	[Cos]
		add     eax,ecx			; EDX:EAX := mat[1][off] * Cos + mat[0][off] * Sin
		adc		edx,ebx
		ROUNDIT
		mov		[_ES _di+off+12],eax; Store result
off = off + 4
		ENDM

		mov		eax,[_si+off]
		imul	[Sin]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Sin
		mov		ebx,edx
		mov		eax,[_si+off+12]
		imul	[Cos]
		add     ecx,eax			; EBX:ECX := mat[1][off] * Cos + mat[1][off] * Sin
		adc		ebx,edx
		mov		eax,10000h
		sub		eax,[Cos]		; EAX := 1 - Cos
		imul	[y]				; EAX := y * (1-Cos)
		add		ecx,eax			; Add to total
		adc		ebx,edx
		mov		eax,[Sin]
		imul	[x]				; EAX := x * Sin
		sub     ecx,eax
		sbb		ebx,edx
		ROUNDIT_EBX_ECX
		mov		[_ES _di+off+12],eax; Store result

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_rotateAbout2D

;----------------------------------------------------------------------------
; void F386_concat3x3(long *result,long *m1,long *m2);
;----------------------------------------------------------------------------
; Concatenate (multiplies) the two fixed point 3x3 matrices together. We dont
; both to work out the bottom row of the matrix as it is always implicitly
; set to <0,0,1>.
;
; Note that we compute the result of each element in the matrix with 64 bit
; precision, and round the result once at the end (faster and more accurate).
;----------------------------------------------------------------------------
procstart	_F386_concat3x3

		ARG 	result:DPTR, m1:DPTR, m2:DPTR

		enter_c	0
		use_ds

		_lds	_si,[m1]		; DS:_SI -> matrix m1
		_les	_di,[m2]		; ES:_DI -> matrix m2

; Set up two imbedded macros to code the unrolled loops to multiply the
; matrices.

roff=0							; Row offset starts at zero
		REPT	2				; Repeat once for each row

coff=0							; Column offset starts at zero
		REPT	2				; Do once for each of the first 2 columns

		mov		eax,[_si+roff]	; EAX := m1.mat[roff][0]
		imul	[ULONG _ES _di+coff]; EDX:EAX := m1.mat[roff][0] * m2.mat[0][coff]
		mov		ecx,eax
		mov		ebx,edx			; EBX:ECX := running total

		mov		eax,[_si+roff+4]; EAX := m1.mat[roff][1]
		imul	[ULONG _ES _di+12+coff]; EDX:EAX := m1.mat[roff][1] * m2.mat[1][coff]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT

		use_ds
		_lds    _bx,[result]
		mov		[_bx+roff+coff],eax	; result.mat[roff][coff] := EAX
		unuse_ds

coff=coff+4						; Point to next column offset

		ENDM					; End of inner column loop

; Now do the third column by assuming that the all entries at [2][2] are
; set to 1.

		mov		eax,[_si+roff]	; EAX := m1.mat[roff][0]
		imul	[ULONG _ES _di+coff]; EDX:EAX := m1.mat[roff][0] * m2.mat[0][coff]
		mov		ecx,eax
		mov		ebx,edx			; EBX:ECX := running total

		mov		eax,[_si+roff+4]; EAX := m1.mat[roff][1]
		imul	[ULONG _ES _di+12+coff]; EDX:EAX := m1.mat[roff][1] * m2.mat[1][coff]
		add		eax,ecx			; Update running total
		adc     edx,ebx
		ROUNDIT					; EAX := rounded result
		add		eax,[_si+roff+8]; Add in the translation

		use_ds
		_lds    _bx,[result]
		mov		[_bx+roff+coff],eax	; result.mat[roff][coff] := EAX
		unuse_ds

coff=coff+4						; Point to next column

roff=roff+12					; Point to next row

		ENDM					; End of outer row loop

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_concat3x3

endcodeseg	fxmat2d

		END						; End of module
