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
;* Description: High performance zbuffer clearing code.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

ifdef	MGL_3D

header	_em_zbuf

begdataseg	_em_zbuf

		$EXTRN	__MGL_dc,devCtx_s

enddataseg	_em_zbuf

begcodeseg	_em_zbuf		; Start of hybrid 16/32 code segment

if flatmodel

;----------------------------------------------------------------------------
; void __EMU__zClear16(int left,int top,int right,int bottom,
;	zfix32_t clearVal);
;----------------------------------------------------------------------------
; Clears a selected portion of the software zbuffer.
;----------------------------------------------------------------------------
procstart	___EMU__zClear16

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, clearVal:ZFIX32_T

		enter_c	0
		cld

		mov		eax,[y1]
		mov		ecx,[x1]
		sub     eax,[GDC.size.top]
		sub     ecx,[GDC.size.left]
		mul		[GDC.zwidth]		; EDX:EAX := offset into zbuffer
		mov		edi,eax
		add		edi,ecx
		shl		edi,1
		add		edi,[GDC.zBuffer]	; EDI -> zbuffer

		mov		esi,[x2]
		sub		esi,[x1]			; ESI := X2 - X1 (# pixels to draw)
		mov		edx,[y2]
		sub		edx,[y1]			; EDX := Y2 - Y1 (# rows to draw)
		mov		eax,[clearVal]		; EAX := 4.28 fixed point clear value
		shr		eax,15				; Convert to 16 bits and drop sign
		mov     ebx,eax
		shl		eax,16
		mov		ax,bx				; EAX := combined value for 2 pixels
		mov		ebx,[GDC.zwidth]
		sub		ebx,esi
		shl		ebx,1				; EBX := buffer increment in bytes

@@RowLoop:
		mov		ecx,esi				; Restore number of pixels to fill
		test	edi,2
		jz		@@WordAligned
		stosw						; Store first pixel if odd to ensure
		dec		ecx					;  word alignment for 'rep stosw'
		jcxz	@@NextRow
@@WordAligned:
		shr		ecx,1
	rep stosd						; Fill in the row
		adc		ecx,ecx
	rep	stosw						; Store the last byte in row

@@NextRow:
		add		edi,ebx				; Increment to next row
		dec		edx
		jnz		@@RowLoop			; Loop for all rows

@@Exit:	leave_c_nolocal
		ret

procend		___EMU__zClear16

;----------------------------------------------------------------------------
; void __EMU__zClear(int left,int top,int right,int bottom,
;	zfix32_t clearVal);
;----------------------------------------------------------------------------
; Clears a selected portion of the software zbuffer.
;----------------------------------------------------------------------------
procstart	___EMU__zClear32

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, clearVal:ZFIX32_T

		enter_c	0
		cld

		mov		eax,[y1]
		mov		ecx,[x1]
		sub     eax,[GDC.size.top]
		sub     ecx,[GDC.size.left]
		mul		[GDC.zwidth]		; EDX:EAX := offset into zbuffer
		mov		edi,eax
		add		edi,ecx
		shl		edi,2
		add		edi,[GDC.zBuffer]	; EDI -> zbuffer

		mov		esi,[x2]
		sub		esi,[x1]			; ESI := X2 - X1 (# pixels to draw)
		mov		edx,[y2]
		sub		edx,[y1]			; EDX := Y2 - Y1 (# rows to draw)
		mov		eax,[clearVal]		; EAX := 4.28 fixed point clear value
		mov		ebx,[GDC.zwidth]
		sub		ebx,esi
		shl		ebx,2				; EBX := buffer increment in bytes

@@RowLoop:
		mov		ecx,esi				; Restore number of pixels to fill
	rep stosd						; Fill in the row

@@NextRow:
		add		edi,ebx				; Increment to next row
		dec		edx
		jnz		@@RowLoop			; Loop for all rows

@@Exit:	leave_c_nolocal
		ret

procend		___EMU__zClear32

endif	; flatmodel

endcodeseg	_em_zbuf

endif	; MGL_3D

		END						; End of module
