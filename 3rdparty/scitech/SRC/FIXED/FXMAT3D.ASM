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
;*				fixed point 3D transformation matrix manipulation routines:
;*
;*					F386_scale3D		- Concat a 3D scale matrix
;*					F386_rotatex3D		- Concat a 3D rotate X axis matrix
;*					F386_rotatey3D		- Concat a 3D rotate Y axis matrix
;*					F386_rotatez3D		- Concat a 3D rotate Z axis matrix
;*					F386_rotate3D		- Concat a 3D rotate any axis matrix
;*					F386_viewOrientation- Build a 3D view orienation matrix
;*					F386_concat4x4		- Concatenate two 4x4 matrices
;*
;*				All of the 4x4 matrix routines take into account special
;*				case matrices to obtain higher performance operation. The
;*				special case flag is passed by the higher level interface.
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros
INCLUDE "fxmacs.mac"			; Fixed point macros

header	fxmat3d				; Set up memory model

begcodeseg	fxmat3d

;----------------------------------------------------------------------------
; void F386_scale3D(long *m,long xscale,long yscale,zscale)
;----------------------------------------------------------------------------
; Concatenate a scale transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_scale3D

		ARG     m:DPTR, xscale:ULONG, yscale:ULONG, zscale:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		mov		ebx,[xscale]	; EBX := x scale factor

off = 0
		REPT	4
		mov		eax,[_si+off]
		imul	ebx				; EDX:EAX := mat[0][off] * xscale
		ROUNDIT
		mov		[_si+off],eax	; Store result
off = off + 4
		ENDM

		mov		ebx,[yscale]	; EBX := y scale factor

		REPT	4
		mov		eax,[_si+off]
		imul	ebx				; EDX:EAX := mat[1][off] * yscale
		ROUNDIT
		mov		[_si+off],eax	; Store result
off = off + 4
		ENDM

		mov		ebx,[zscale]	; EBX := z scale factor

		REPT	4
		mov		eax,[_si+off]
		imul	ebx				; EDX:EAX := mat[1][off] * yscale
		ROUNDIT
		mov		[_si+off],eax	; Store result
off = off + 4
		ENDM

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_scale3D

;----------------------------------------------------------------------------
; void F386_rotatex3D(long *T,long *m,long Sin,long Cos)
;----------------------------------------------------------------------------
; Concatenate a rotate transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_rotatex3D

		ARG     T:DPTR, m:DPTR, Sin:ULONG, Cos:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		_les	_di,[T]			; ES:_DI -> resultant temporary matrix

off = 0
		REPT	4
		mov		eax,[_si+off+16]
		imul	[Cos]
		mov		ecx,eax			; EBX:ECX := mat[1][off] * Cos
		mov		ebx,edx
		mov		eax,[_si+off+32]
		imul	[Sin]
		sub     ecx,eax			; EBX:ECX := mat[1][off] * Cos - mat[2][off] * Sin
		sbb		ebx,edx
		ROUNDIT_EBX_ECX
		mov		[_ES _di+off],eax; Store result
off = off + 4
		ENDM

off = 0
		REPT	4
		mov		eax,[_si+off+16]
		imul	[Sin]
		mov		ecx,eax			; EBX:ECX := mat[1][off] * Sin
		mov		ebx,edx
		mov		eax,[_si+off+32]
		imul	[Cos]
		add     eax,ecx			; EDX:EAX := mat[2][off] * Cos + mat[1][off] * Sin
		adc		edx,ebx
		ROUNDIT
		mov		[_ES _di+off+16],eax; Store result
off = off + 4
		ENDM

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_rotatex3D

;----------------------------------------------------------------------------
; void F386_rotatey3D(long *T,long *m,long Sin,long Cos)
;----------------------------------------------------------------------------
; Concatenate a rotate transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_rotatey3D

		ARG     T:DPTR, m:DPTR, Sin:ULONG, Cos:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		_les	_di,[T]			; ES:_DI -> resultant temporary matrix

off = 0
		REPT	4
		mov		eax,[_si+off]
		imul	[Cos]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Cos
		mov		ebx,edx
		mov		eax,[_si+off+32]
		imul	[Sin]
		sub     ecx,eax			; EBX:ECX := mat[0][off] * Cos - mat[2][off] * Sin
		sbb		ebx,edx
		ROUNDIT_EBX_ECX
		mov		[_ES _di+off],eax; Store result
off = off + 4
		ENDM

off = 0
		REPT	4
		mov		eax,[_si+off]
		imul	[Sin]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Sin
		mov		ebx,edx
		mov		eax,[_si+off+32]
		imul	[Cos]
		add     eax,ecx			; EDX:EAX := mat[2][off] * Cos + mat[0][off] * Sin
		adc		edx,ebx
		ROUNDIT
		mov		[_ES _di+off+16],eax; Store result
off = off + 4
		ENDM

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_rotatey3D

;----------------------------------------------------------------------------
; void F386_rotatez3D(long *T,long *m,long Sin,long Cos)
;----------------------------------------------------------------------------
; Concatenate a rotate transformation efficiently to the current
; transformation matrix. The bottom row is assumed to be <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_rotatez3D

		ARG     T:DPTR, m:DPTR, Sin:ULONG, Cos:ULONG

		enter_c	0
		use_ds

		_lds	_si,[m]			; DS:_SI -> xform matrix
		_les	_di,[T]			; ES:_DI -> resultant temporary matrix

off = 0
		REPT	4
		mov		eax,[_si+off]
		imul	[Cos]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Cos
		mov		ebx,edx
		mov		eax,[_si+off+16]
		imul	[Sin]
		sub     ecx,eax			; EBX:ECX := mat[0][off] * Cos - mat[1][off] * Sin
		sbb		ebx,edx
		ROUNDIT_EBX_ECX
		mov		[_ES _di+off],eax; Store result
off = off + 4
		ENDM

off = 0
		REPT	4
		mov		eax,[_si+off]
		imul	[Sin]
		mov		ecx,eax			; EBX:ECX := mat[0][off] * Sin
		mov		ebx,edx
		mov		eax,[_si+off+16]
		imul	[Cos]
		add     eax,ecx			; EDX:EAX := mat[1][off] * Cos + mat[0][off] * Sin
		adc		edx,ebx
		ROUNDIT
		mov		[_ES _di+off+16],eax; Store result
off = off + 4
		ENDM

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_rotatez3D

;----------------------------------------------------------------------------
; void F386_rotate3D(long *T,long *T2,long *m,long Sin,long Cos,long Cos_1,
;	long x,long y,long z)
;----------------------------------------------------------------------------
; Concatenate a rotate transformation about and arbitrary axis efficiently
; to the current transformation matrix. The bottom row is assumed to be
; <0,0,1>.
;----------------------------------------------------------------------------
procstart	_F386_rotate3D

		ARG     T:DPTR, T2:DPTR, m:DPTR, Sin:ULONG, Cos:ULONG,			\
				Cos_1:ULONG, x:ULONG, y:ULONG, z:ULONG

		enter_c	0
		use_ds

		_les	_di,[T]			; ES:_DI -> resultant temporary matrix

; Create the temporary 3x3 sub-rotation matrix

		mov		eax,[x]
		imul	eax
		ROUNDIT					; EAX := x*x
		imul	[Cos_1]
		ROUNDIT					; EAX := x*x*(1-Cos)
		add		eax,[Cos]		; EAX := Cos + x*x(1-Cos)
		mov		[_ES _di],eax	; T[0][0] = EAX

		mov		eax,[x]
		imul	[y]
		ROUNDIT					; EAX := x*y
		imul	[Cos_1]
		ROUNDIT
		mov		ebx,eax			; EBX := x*y*(1-Cos)
		mov		eax,[z]
		imul	[Sin]
		ROUNDIT					; EAX := z*Sin
		sub		ebx,eax			; EBX := x*y*(1-Cos) - z*Sin
		mov		[_ES _di+4],ebx	; T[0][1] = EBX
		neg		ebx				; EBX := -T[0][1]
		mov		[_ES _di+12],ebx; T[1][0] = -T[0][1]

		mov		eax,[x]
		imul	[z]
		ROUNDIT					; EAX := x*z
		imul	[Cos_1]
		ROUNDIT
		mov		ebx,eax			; EBX := x*z*(1-Cos)
		mov		eax,[y]
		imul	[Sin]
		ROUNDIT					; EAX := y*Sin
		add		eax,ebx			; EAX := x*z*(1-Cos) + y*Sin
		mov		[_ES _di+8],eax	; T[0][2] = EAX
		neg		eax				; EAX := -T[0][2]
		mov		[_ES _di+24],eax; T[2][0] = -T[0][2]

		mov		eax,[y]
		imul	eax
		ROUNDIT					; EAX := y*y
		imul	[Cos_1]
		ROUNDIT					; EAX := y*y*(1-Cos)
		add		eax,[Cos]		; EAX := Cos + y*y(1-Cos)
		mov		[_ES _di+16],eax; T[1][1] = EAX

		mov		eax,[y]
		imul	[z]
		ROUNDIT					; EAX := y*z
		imul	[Cos_1]
		ROUNDIT
		mov		ebx,eax			; EBX := y*z*(1-Cos)
		mov		eax,[x]
		imul	[Sin]
		ROUNDIT					; EAX := x*Sin
		sub		ebx,eax			; EBX := y*z*(1-Cos) - x*Sin
		mov		[_ES _di+20],ebx; T[1][2] = EBX
		neg		ebx				; EBX := -T[1][2]
		mov		[_ES _di+28],ebx; T[2][1] = -T[1][2]

		mov		eax,[z]
		imul	eax
		ROUNDIT					; EAX := z*z
		imul	[Cos_1]
		ROUNDIT					; EAX := z*z*(1-Cos)
		add		eax,[Cos]		; EAX := Cos + z*z(1-Cos)
		mov		[_ES _di+32],eax; T[2][2] = EAX

		_lds	_si,[T]			; DS:_SI -> sub-matrix T
		_les	_di,[m]			; ES:_DI -> original transformation matrix

; Now concatentate this rotation matrix with the upper left 3x3
; sub-matrix of the current transformation matrix

roff=0							; Row offset starts at zero
troff=0
		REPT	3				; Repeat once for all rows

coff=0							; Column offset starts at zero
		REPT	3				; Do once for each column

		mov		eax,[_si+troff]	; EAX := T[troff][0]
		imul	[ULONG _ES _di+coff]; EDX:EAX := T[troff][0] * m[0][coff]
		mov		ecx,eax
		mov		ebx,edx			; EBX:ECX := running total

		mov		eax,[_si+troff+4]; EAX := T[troff][1]
		imul	[ULONG _ES _di+16+coff]; EDX:EAX := T[troff][1] * m[1][coff]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+troff+8]; EAX := T[troff][2]
		imul	[ULONG _ES _di+32+coff]; EDX:EAX := T[troff][2] * m[2][coff]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT

		use_ds
		_lds    _bx,[T2]
		mov		[_bx+roff+coff],eax	; T2[roff][coff] := EAX
		unuse_ds

coff=coff+4						; Point to next column offset

		ENDM					; End of inner column loop

roff=roff+16					; Point to next row
troff=troff+12

		ENDM					; End of outer row loop

		unuse_ds
		leave_c_nolocal
		ret

procend		_F386_rotate3D

;----------------------------------------------------------------------------
; void F386_concat4x4(long *result,long *m1,long *m2,bool special);
;----------------------------------------------------------------------------
; Concatenate (multiplies) the two fixed point 4x4 matrices together. If
; the matrix multiplication is special, then the bottom row of the matrix
; will be simply <0,0,0,1>.
;
; Note that we compute the result of each element in the matrix with 64 bit
; precision, and round the result once at the end (faster and more accurate).
;----------------------------------------------------------------------------
procstart	_F386_concat4x4

		ARG 	result:DPTR, m1:DPTR, m2:DPTR, special:BOOL

		enter_c	0
		use_ds

		_lds	_si,[m1]			; DS:_SI -> matrix m1
		_les	_di,[m2]			; ES:_DI -> matrix m2

; Set up two imbedded macros to code the unrolled loops to multiply the
; matrices.

roff=0							; Row offset starts at zero
		REPT	3				; Repeat once for all rows except last

coff=0							; Column offset starts at zero
		REPT	4				; Do once for each column

		mov		eax,[_si+roff]	; EAX := m1.mat[roff][0]
		imul	[ULONG _ES _di+coff]; EDX:EAX := m1.mat[roff][0] * m2.mat[0][coff]
		mov		ecx,eax
		mov		ebx,edx			; EBX:ECX := running total

		mov		eax,[_si+roff+4]; EAX := m1.mat[roff][1]
		imul	[ULONG _ES _di+16+coff]; EDX:EAX := m1.mat[roff][1] * m2.mat[1][coff]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+roff+8]; EAX := m1.mat[roff][2]
		imul	[ULONG _ES _di+32+coff]; EDX:EAX := m1.mat[roff][2] * m2.mat[2][coff]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+roff+12]; EAX := m1.mat[roff][3]
		imul	[ULONG _ES _di+48+coff]; EDX:EAX := m1.mat[roff][3] * m2.mat[3][coff]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT

		use_ds
		_lds    _bx,[result]
		mov		[_bx+roff+coff],eax	; result.mat[roff][coff] := EAX
		unuse_ds

coff=coff+4						; Point to next column offset

		ENDM					; End of inner column loop

roff=roff+16					; Point to next row

		ENDM					; End of outer row loop

		cmp		[special],0		; Do we have a special case matrix mult?
		je		@@ComputeBot	; No, compute Bottom row of matrix

; Set the bottom row of the matrix to be <0,0,0,1>

		use_ds
		_lds    _bx,[result]
		xor 	eax,eax
		mov		[_bx+roff],eax	; Store each zero
		mov		[_bx+roff+4],eax; Store each zero
		mov		[_bx+roff+8],eax; Store each zero
		or		eax,10000h
		mov		[_bx+roff+12],eax; Store the 1.0
		unuse_ds
		jmp		@@Exit

; Compute the bottom row for the matrix

@@ComputeBot:

coff=0							; Column offset starts at zero
		REPT	4				; Do once for each column

		mov		eax,[_si+roff]	; EAX := m1.mat[roff][0]
		imul	[ULONG _ES _di+coff]; EDX:EAX := m1.mat[roff][0] * m2.mat[0][coff]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+roff+4]; EAX := m1.mat[roff][1]
		imul	[ULONG _ES _di+16+coff]; EDX:EAX := m1.mat[roff][1] * m2.mat[1][coff]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+roff+8]; EAX := m1.mat[roff][2]
		imul	[ULONG _ES _di+32+coff]; EDX:EAX := m1.mat[roff][2] * m2.mat[2][coff]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+roff+12]; EAX := m1.mat[roff][3]
		imul	[ULONG _ES _di+48+coff]; EDX:EAX := m1.mat[roff][3] * m2.mat[3][coff]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT

		use_ds
		_lds    _bx,[result]
		mov		[_bx+roff+coff],eax	; result.mat[roff][coff] := EAX
		unuse_ds

coff=coff+4						; Point to next column offset

		ENDM					; End of inner column loop

@@Exit:	unuse_ds
		leave_c_nolocal
		ret

procend		_F386_concat4x4

endcodeseg	fxmat3d

		END						; End of module
