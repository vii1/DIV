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
;*				fixed point 2D vector mapping routines:
;*
;*					F386_map2D			- Map a 2D point with 3x3 matrix
;*					F386_mapVec2D		- Map a 2D vector with 3x3 matrix
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros
INCLUDE "fxmacs.mac"			; Fixed point macros

header	fxmap2d					; Set up memory model

begcodeseg	fxmap2d

;----------------------------------------------------------------------------
; void F386_map2D(long *m,long *result,long *p,bool special);
;----------------------------------------------------------------------------
; Maps a 2D point 'p' by the 3x3 transformation matrix 'm' to obtain the
; resultant point 'result'. The bottom row of the matrix is implicitly
; set to <0,0,1> and we dont include it in the multiplication.
;----------------------------------------------------------------------------
procstart	_F386_map2D

		ARG 	m:DPTR, result:DPTR, p:DPTR, special:BOOL

if flatmodel
m		=		DPTR esp+16		; Access directly without stack frame
result	=		DPTR esp+20		;  for speed in 32 bit PM
p		=		DPTR esp+24
special	=		DPTR esp+28
		push	ebx
else
		push	bp
		mov		bp,sp
		push	ds
endif
		push	_si
		push	_di

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[p]			; ES:_DI -> point to transform
		test	[special],1
		jnz		@@DoSpecial		; We have a special case mapping!

		mov		eax,[_si]		; EAX := mat[0][0]
		imul	[ULONG _ES _di]
		mov		ecx,eax			; EBX:ECX := p.x * mat[0][0]
		mov		ebx,edx

		mov		eax,[_si+4]		; EAX := mat[0][1]
		imul	[ULONG _ES _di+4]
		add		eax,ecx			; EDX:EAX := p.x * mat[0][0] + p.y * mat[0][1]
		adc		edx,ebx
		ROUNDIT
		add		eax,[_si+8]		; EAX := p.x * mat[0][0] + p.y * mat[0][1]
								;        + mat[0][2] = result.x
		push	eax

		mov		eax,[_si+12]	; EAX := mat[1][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[1][0]
		mov		ecx,eax			; EBX:ECX := p.x * mat[1][0]
		mov		ebx,edx

		mov		eax,[_si+12+4]	; EAX := mat[1][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[1][1]
		add		eax,ecx			; EDX:EAX := p.x * mat[1][0] + p.y * mat[1][1]
		adc		edx,ebx
		ROUNDIT
		add		eax,[_si+12+8]	; EAX := p.x * mat[1][0] + p.y * mat[1][1]
								;        + mat[1][2] = result.y
		pop		ebx
		jmp		@@StoreResult

@@DoSpecial:
		mov		eax,[_si]		; EAX := mat[0][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[0][0]
		ROUNDIT					; EAX := rounded result
		mov		ebx,eax			; EBX := p.x * mat[0][0]
		add		ebx,[_si+8]		; EBX := p.x * mat[0][0] + mat[0][2] = result.x

		mov		eax,[_si+12+4]	; EAX := mat[1][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[1][1]
		ROUNDIT					; EAX := rounded result
		add		eax,[_si+12+8]	; EAX := p.y * mat[1][1] + mat[1][2] = result.y

@@StoreResult:
		_lds	_si,[result]	; DS:_SI -> resultant point

		mov		[_si],ebx		; Store result.x
		mov		[_si+4],eax		; Store result.y

		pop		_di
		pop		_si
if flatmodel
		pop		ebx
else
		pop		ds
		pop		bp
endif
        ret

procend		_F386_map2D

;----------------------------------------------------------------------------
; void F386_mapVec2D(long *m,long *result,long *v,bool special);
;----------------------------------------------------------------------------
; Maps a 2D vector 'v' by the 3x3 transformation matrix 'm' to obtain the
; resultant vector 'result'. The bottom row of the matrix is implicitly
; set to <0,0,1> and we dont include it in the multiplication. Note that
; when mapping vectors any translational components of the matrix,
; must be left out of the calculation otherwise the resulting vector will
; incorrect.
;----------------------------------------------------------------------------
procstart	_F386_mapVec2D

		ARG 	m:DPTR, result:DPTR, v:DPTR, special:BOOL

if flatmodel
m		=		DPTR esp+16		; Access directly without stack frame for
result	=		DPTR esp+20		;  speed in 32 bit PM
v		=		DPTR esp+24
special	=		DPTR esp+28
		push	ebx
else
		push	bp
		mov		bp,sp
		push	ds
endif
		push	_si
		push	_di

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[v]			; ES:_DI -> point to transform
		test	[special],1
		jnz		@@DoSpecial		; We have a special case mapping!

		mov		eax,[_si]		; EAX := mat[0][0]
		imul	[ULONG _ES _di]
		mov		ecx,eax			; EBX:ECX := p.x * mat[0][0]
		mov		ebx,edx

		mov		eax,[_si+4]		; EAX := mat[0][1]
		imul	[ULONG _ES _di+4]
		add		eax,ecx			; EDX:EAX := p.x * mat[0][0] + p.y * mat[0][1]
		adc		edx,ebx
		ROUNDIT
		push	eax

		mov		eax,[_si+12]	; EAX := mat[1][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[1][0]
		mov		ecx,eax			; EBX:ECX := p.x * mat[1][0]
		mov		ebx,edx

		mov		eax,[_si+12+4]	; EAX := mat[1][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[1][1]
		add		eax,ecx			; EDX:EAX := p.x * mat[1][0] + p.y * mat[1][1]
		adc		edx,ebx
		ROUNDIT
		pop		ebx
		jmp		@@StoreResult

@@DoSpecial:
		mov		eax,[_si]		; EAX := mat[0][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[0][0]
		ROUNDIT					; EAX := rounded result
		mov		ebx,eax			; EBX := p.x * mat[0][0]

		mov		eax,[_si+12+4]	; EAX := mat[1][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[1][1]
		ROUNDIT					; EAX := rounded result

@@StoreResult:
		_lds	_si,[result]	; DS:_SI -> resultant point

		mov		[_si],ebx		; Store result.x
		mov		[_si+4],eax		; Store result.y

		pop		_di
		pop		_si
if flatmodel
		pop		ebx
else
		pop		ds
		pop		bp
endif
		ret

procend		_F386_mapVec2D

endcodeseg	fxmap2d

		END						; End of module
