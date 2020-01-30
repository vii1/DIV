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
;*				fixed point 3D vector mapping routines:
;*
;*					F386_map3Dto3D		- Map a 3D point given a 4x4 matrix
;*					F386_map3Dto4D		- Map a 3D point given a 4x4 matrix
;*					F386_map4Dto4D		- Map a 4D point given a 4x4 matrix
;*					F386_mapVec3Dto3D	- Map a 3D vector given a 4x4 matrix
;*					F386_mapVec3Dto4D	- Map a 3D vector given a 4x4 matrix
;*					F386_mapVec4Dto4D	- Map a 4D vector given a 4x4 matrix
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

header	fxmap3d					; Set up memory model

begcodeseg	fxmap3d

;----------------------------------------------------------------------------
; void F386_map3Dto3D(long *m,long *result,long *p,bool special);
;----------------------------------------------------------------------------
; Maps a 3D point 'p' by the 4x4 transformation matrix 'm' to obtain the
; resultant point 'result'. In this routine we take advantage of the fact
; that the matrix may be in a special (no perspective transforms) and
; we can leave early. Since we are working with 3D points here, we will
; need to homogenise the resulting coordinate back into 3 space if the
; matrix is not special.
;----------------------------------------------------------------------------
procstart	_F386_map3Dto3D

		ARG 	m:DPTR, result:DPTR, p:DPTR, special:BOOL

		LOCAL   temp:ULONG:3 = LocalSize

		enter_c	LocalSize
		use_ds

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[p]			; ES:_DI -> point to transform

; Set up a macro to code the unrolled loop to perform the mapping. Here
; we perform the mapping of the x,y and z coordinates.

soff=0							; Source offset
doff=0							; Destination offset
		REPT	3				; Do once for each destination coordinate

		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result
		add		eax,[_si+soff+12]; Add in translation (mat[soff][3]).

		mov		[temp+doff],eax	; Store in destination buffer

soff=soff+16					; Point to next source offset
doff=doff+4						; Point to next destination offset
		ENDM

		cmp		[special],0		; Do we have a special case matrix?
		je		@@SlowMapping	; No, so we need to divide through by w

		_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer
		mov		_cx,3			; Need to move 3 doublewords
	rep	movsd					; Move all three longs at once

		jmp		@@Exit			; We are done...

; Compute the new w coordinate for the vector, and divide all coordinates
; through by it.

@@SlowMapping:
		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result
		add		eax,[_si+soff+12]; Add in translation (mat[soff][3]).
		mov		ecx,eax

		xor		eax,eax
		mov		edx,1
		idiv	ecx				; Find inverse of w
		mov		ecx,eax			; ECX := 1/w

		_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer

off = 0
		REPT	3
		mov		eax,[_si+off]	; EAX := next coordinate
		imul	ecx				; EAX := coord / w
		ROUNDIT					; EAX := rounded result
		mov		[_ES _di+off],eax
off = off + 4
		ENDM

@@Exit:	unuse_ds
		leave_c
		ret

procend		_F386_map3Dto3D

;----------------------------------------------------------------------------
; void F386_map3Dto4D(long *m,long *result,long *p,bool special);
;----------------------------------------------------------------------------
; Maps a 3D point 'p' by the 4x4 transformation matrix 'm' to obtain the
; resultant point 'result'. In this routine we take advantage of the fact
; that the matrix may be in a special (no perspective transforms) and
; we can leave early. The result is a homogenous coordinate that may have
; a w value other than 1.0.
;----------------------------------------------------------------------------
procstart	_F386_map3Dto4D

		ARG 	m:DPTR, result:DPTR, p:DPTR, special:BOOL

		LOCAL   temp:ULONG:4 = LocalSize

		enter_c	LocalSize
		use_ds

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[p]			; ES:_DI -> point to transform

; Set up a macro to code the unrolled loop to perform the mapping. Here
; we perform the mapping of the x,y and z coordinates.

soff=0							; Source offset
doff=0							; Destination offset
		REPT	3				; Do once for each destination coordinate

		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result
		add		eax,[_si+soff+12]; Add in translation (mat[soff][3]).

		mov		[temp+doff],eax	; Store in destination buffer

soff=soff+16					; Point to next source offset
doff=doff+4						; Point to next destination offset
		ENDM

		cmp		[special],0		; Do we have a special case matrix?
		je		@@ComputeW		; No, so we need to compute W

; The w coordinate will be the same as the W coordinate of the input
; which is 1.0, as the input point is in 3D coordinate space.

		mov		[temp+doff],10000h
		jmp		@@Exit			; We are done...

; Compute the new w coordinate for the vector.

@@ComputeW:
		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result
		add		eax,[_si+soff+12]; Add in translation (mat[soff][3]).

		mov		[temp+doff],eax	; Store w coordinate

@@Exit:	_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer
		mov		_cx,4			; Need to move 4 doublewords
	rep	movsd					; Move all three longs at once

		unuse_ds
		leave_c
		ret

procend		_F386_map3Dto4D

;----------------------------------------------------------------------------
; void F386_map4Dto4D(long *m,long *result,long *p,bool special);
;----------------------------------------------------------------------------
; Maps a 4D point 'p' by the 4x4 transformation matrix 'm' to obtain the
; resultant point 'result'. In this routine we take advantage of the fact
; that the matrix may be in a special (no perspective transforms) and
; we can leave early. The result may very well be a homogenouse coordinate.
;----------------------------------------------------------------------------
procstart	_F386_map4Dto4D

		ARG 	m:DPTR, result:DPTR, p:DPTR, special:BOOL

		LOCAL   temp:ULONG:4 = LocalSize

		enter_c	LocalSize
		use_ds

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[p]			; ES:_DI -> point to transform

; Set up a macro to code the unrolled loop to perform the mapping. Here
; we perform the mapping of the x,y and z coordinates.

soff=0							; Source offset
doff=0							; Destination offset
		REPT	3				; Do once for each destination coordinate

		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+12]; EAX := mat[soff][3]
		imul	[ULONG _ES _di+12]; EDX:EAX = p.w * mat[soff][3]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result

		mov		[temp+doff],eax	; Store in destination buffer

soff=soff+16					; Point to next source offset
doff=doff+4						; Point to next destination offset
		ENDM

		cmp		[special],0		; Do we have a special case matrix?
		je		@@ComputeW		; No, so we need to compute W

; The w coordinate will be the same as the W coordinate of the input
; point, so just copy it across

		mov		eax,[ULONG _ES _di+12]
		mov		[temp+doff],eax
		jmp		@@Exit			; We are done...

; Compute the new w coordinate for the vector.

@@ComputeW:
		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+12]; EAX := mat[soff][3]
		imul	[ULONG _ES _di+12]; EDX:EAX = p.w * mat[soff][3]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result

		mov		[temp+doff],eax	; Store w coordinate

@@Exit:	_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer
		mov		_cx,4			; Need to move 4 doublewords
	rep	movsd					; Move all three longs at once

		unuse_ds
		leave_c
		ret

procend		_F386_map4Dto4D

;----------------------------------------------------------------------------
; void F386_mapVec3Dto3D(long *m,long *result,long *v,bool special);
;----------------------------------------------------------------------------
; Maps a 3D vector 'v' by the 4x4 transformation matrix 'm' to obtain the
; resultant vector 'result'. In this routine we take advantage of the fact
; that the matrix may be in a special (no perspective transforms) and
; we can leave early. Since we are working with 3D vectors here, we will
; need to homogenise the resulting coordinate back into 3 space if the
; matrix is not special. Note that since we are applying this to a vector,
; we cannot include the translational components.
;----------------------------------------------------------------------------
procstart	_F386_mapVec3Dto3D

		ARG 	m:DPTR, result:DPTR, p:DPTR, special:BOOL

		LOCAL   temp:ULONG:3 = LocalSize

		enter_c	LocalSize
		use_ds

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[p]			; ES:_DI -> point to transform

; Set up a macro to code the unrolled loop to perform the mapping. Here
; we perform the mapping of the x,y and z coordinates.

soff=0							; Source offset
doff=0							; Destination offset
		REPT	3				; Do once for each destination coordinate

		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result

		mov		[temp+doff],eax	; Store in destination buffer

soff=soff+16					; Point to next source offset
doff=doff+4						; Point to next destination offset
		ENDM

		cmp		[special],0		; Do we have a special case matrix?
		je		@@SlowMapping	; No, so we need to divide through by w

		_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer
		mov		_cx,3			; Need to move 3 doublewords
	rep	movsd					; Move all three longs at once

		jmp		@@Exit			; We are done...

; Compute the new w coordinate for the vector, and divide all coordinates
; through by it.

@@SlowMapping:
		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result
		mov		ecx,eax

		xor		eax,eax
		mov		edx,1
		idiv	ecx				; Find inverse of w
		mov		ecx,eax			; ECX := 1/w

		_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer

off = 0
		REPT	3
		mov		eax,[_si+off]	; EAX := next coordinate
		imul	ecx				; EAX := coord / w
		ROUNDIT					; EAX := rounded result
		mov		[_ES _di+off],eax
off = off + 4
		ENDM

@@Exit:	unuse_ds
		leave_c
		ret

procend		_F386_mapVec3Dto3D

;----------------------------------------------------------------------------
; void F386_mapVec3Dto4D(long *m,long *result,long *v,bool special);
;----------------------------------------------------------------------------
; Maps a 3D vector 'v' by the 4x4 transformation matrix 'm' to obtain the
; resultant vector 'result'. In this routine we take advantage of the fact
; that the matrix may be in a special (no perspective transforms) and
; we can leave early. The result is a homogenous coordinate that may have
; a w value other than 1.0.
;----------------------------------------------------------------------------
procstart	_F386_mapVec3Dto4D

		ARG 	m:DPTR, result:DPTR, p:DPTR, special:BOOL

		LOCAL   temp:ULONG:4 = LocalSize

		enter_c	LocalSize
		use_ds

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[p]			; ES:_DI -> point to transform

; Set up a macro to code the unrolled loop to perform the mapping. Here
; we perform the mapping of the x,y and z coordinates.

soff=0							; Source offset
doff=0							; Destination offset
		REPT	3				; Do once for each destination coordinate

		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result

		mov		[temp+doff],eax	; Store in destination buffer

soff=soff+16					; Point to next source offset
doff=doff+4						; Point to next destination offset
		ENDM

		cmp		[special],0		; Do we have a special case matrix?
		je		@@ComputeW		; No, so we need to compute W

; The w coordinate will be the same as the W coordinate of the input
; which is 0, as the input vector is in 3D coordinate space.

		mov		[temp+doff],0h
		jmp		@@Exit			; We are done...

; Compute the new w coordinate for the vector.

@@ComputeW:
		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result

		mov		[temp+doff],eax	; Store w coordinate

@@Exit:	_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer
		mov		_cx,4			; Need to move 4 doublewords
	rep	movsd					; Move all three longs at once

		unuse_ds
		leave_c
		ret

procend		_F386_mapVec3Dto4D

;----------------------------------------------------------------------------
; void F386_mapVec4Dto4D(long *m,long *result,long *v,bool special);
;----------------------------------------------------------------------------
; Maps a 4D vector 'v' by the 4x4 transformation matrix 'm' to obtain the
; resultant vector 'result'. In this routine we take advantage of the fact
; that the matrix may be in a special (no perspective transforms) and
; we can leave early. The result may very well be a homogenouse coordinate.
;----------------------------------------------------------------------------
procstart	_F386_mapVec4Dto4D

		ARG 	m:DPTR, result:DPTR, p:DPTR, special:BOOL

		LOCAL   temp:ULONG:4 = LocalSize

		enter_c	LocalSize
		use_ds

		_lds	_si,[m]			; DS:_SI -> transform matrix
		_les	_di,[p]			; ES:_DI -> point to transform

; Set up a macro to code the unrolled loop to perform the mapping. Here
; we perform the mapping of the x,y and z coordinates.

soff=0							; Source offset
doff=0							; Destination offset
		REPT	3				; Do once for each destination coordinate

		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result

		mov		[temp+doff],eax	; Store in destination buffer

soff=soff+16					; Point to next source offset
doff=doff+4						; Point to next destination offset
		ENDM

		cmp		[special],0		; Do we have a special case matrix?
		je		@@ComputeW		; No, so we need to compute W

; The w coordinate will be the same as the W coordinate of the input
; point, so just copy it across

		mov		eax,[ULONG _ES _di+12]
		mov		[temp+doff],eax
		jmp		@@Exit			; We are done...

; Compute the new w coordinate for the vector.

@@ComputeW:
		mov		eax,[_si+soff]	; EAX := mat[soff][0]
		imul	[ULONG _ES _di]	; EDX:EAX := p.x * mat[soff][0]
		mov		ecx,eax			; Set running total
		mov		ebx,edx

		mov		eax,[_si+soff+4]; EAX := mat[soff][1]
		imul	[ULONG _ES _di+4]; EDX:EAX = p.y * mat[soff][1]
		add		ecx,eax			; Update running total
		adc		ebx,edx

		mov		eax,[_si+soff+8]; EAX := mat[soff][2]
		imul	[ULONG _ES _di+8]; EDX:EAX = p.z * mat[soff][2]
		add		eax,ecx			; Update running total
		adc		edx,ebx
		ROUNDIT					; EAX := rounded result

		mov		[temp+doff],eax	; Store w coordinate

@@Exit:	_les	_di,[result]	; ES:_DI -> resultant point
		lea		_si,[temp]		; DS:_SI -> temporary buffer
		mov		_cx,4			; Need to move 4 doublewords
	rep	movsd					; Move all three longs at once

		unuse_ds
		leave_c
		ret

procend		_F386_mapVec4Dto4D

endcodeseg	fxmap3d

		END						; End of module
