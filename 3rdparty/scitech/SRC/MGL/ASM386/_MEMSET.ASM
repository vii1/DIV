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
;* Description:	Assembly language memory block clearing routines. In the
;*				hybrid 16/32 model and 32 bit flat models these routines
;*				can work with blocks up to 4Gb in size.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

header	_hyrid32				; Setup for MGL hybrid 16/32 bit segment

begcodeseg	_memset		; Start of hybrid 16/32 code segment

;----------------------------------------------------------------------------
; void _MGL_memset(void *p,int c,uint n)
;----------------------------------------------------------------------------
; Set a block of 'n' words to the value 'c'. The number of bytes can be no
; more than 64k in 16 bit real mode, but can be any size in 32 bit modes.
;----------------------------------------------------------------------------
procstart	__MGL_memset

		ARG		p:DPTR, c:UINT, n:UINT

		enter_c	0

		_les    _di,[p]			; ES:_DI -> memory block
		clrhi16	di				; ES:EDI -> memory block
		mov		_cx,[n]
		clrhi16	cx				; ECX := count in bytes
		mov		al,[BYTE c]
		mov     ah,al			; AX := value to store
		mov     ebx,eax
		shl		eax,16
		mov		ax,bx			; EAX := value to store
		cld

@@ForceAlignment:
		test	_di,3
		jz		@@Aligned
		mov		[_di],al
		inc		_di
		dec		_cx
		jnz		@@ForceAlignment

@@Aligned:
		push	_cx
		shr		_cx,2
	rep	stosd					; Store all middle WORD's fast!
		pop		_cx
		and		_cx,3
	rep	stosb					; Store the last bytes if any

@@Done:	leave_c_nolocal
		ret

procend		__MGL_memset

;----------------------------------------------------------------------------
; void _MGL_memsetw(void *p,int c,uint n)
;----------------------------------------------------------------------------
; Set a block of 'n' words to the value 'c'. The number of bytes can be no
; more than 64k in 16 bit real mode, but can be any size in 32 bit modes.
;----------------------------------------------------------------------------
procstart	__MGL_memsetw

		ARG		p:DPTR, c:UINT, n:UINT

		enter_c	0

		_les    _di,[p]			; ES:_DI -> memory block
		clrhi16	di				; ES:EDI -> memory block
		mov		_cx,[n]
		clrhi16	cx				; ECX := count
		mov		ax,[WORD c]
		mov    	_bx,_ax
		shl		eax,16
		mov		ax,bx			; EAX := value to store
		cld

		test	_di,1
		jz		@@Aligned
		jcxz	@@Done			; Exit if count of 0
		stosw					; Force dword alignment
		dec		_cx
@@Aligned:
		shr		_cx,1
	rep	stosd					; Store all middle DWORD's fast!
		adc		_cx,_cx
	rep	stosw					; Store the last word if any

@@Done:	leave_c_nolocal
		ret

procend	 	__MGL_memsetw

;----------------------------------------------------------------------------
; void _MGL_memsetl(void *p,long c,uint n)
;----------------------------------------------------------------------------
; Set a block of 'n' double words to the value 'c'. The number of double
; words can be no more than 64k in 16 bit modes, but can be any size in 32
; bit modes.
;----------------------------------------------------------------------------
procstart	__MGL_memsetl

		ARG		p:DPTR, c:ULONG, n:UINT

		enter_c	0

		_les    _di,[p]			; ES:_DI -> memory block
		clrhi16	di				; ES:EDI -> memory block
		mov		_cx,[n]
		clrhi16	cx				; ECX := count
		mov		eax,[c]			; EAX := value to store
		cld
	rep	stosd					; Store all middle DWORD's fast!
		leave_c_nolocal
		ret

procend	 	__MGL_memsetl

;----------------------------------------------------------------------------
; void _MGL_memcpy(void *p,void *s,uint n)
;----------------------------------------------------------------------------
procstartdll	_MGL_memcpy

		ARG		p:DPTR, s:DPTR, n:UINT

		enter_c	0
		use_ds

		_les    _di,[p]			; ES:_DI -> memory block
		clrhi16	di				; ES:EDI -> memory block
		_lds    _si,[s]			; DS:_SI -> memory block
		clrhi16	si				; DS:ESI -> memory block
		mov		_cx,[n]
		clrhi16	cx				; ECX := count in bytes
		cld

		mov		eax,ecx
		shr		ecx,2
	rep	movsd
		mov		ecx,eax
		and		ecx,3
	rep	movsb

@@Done:	unuse_ds
		leave_c_nolocal
		ret

procend			_MGL_memcpy

;----------------------------------------------------------------------------
; void MGL_memcpyVIRTSRC(void *p,void *s,uint n)
;----------------------------------------------------------------------------
procstartdll	_MGL_memcpyVIRTSRC

		ARG		p:DPTR, s:DPTR, n:UINT

		enter_c	0
		use_ds

		_les    _di,[p]			; ES:_DI -> memory block
		clrhi16	di				; ES:EDI -> memory block
		_lds    _si,[s]			; DS:_SI -> memory block
		clrhi16	si				; DS:ESI -> memory block
		mov		_cx,[n]
		clrhi16	cx				; ECX := count in bytes
		cld

; Force DWORD alignment for transfers in a virtualised buffer

@@ForceAlignment:
		test	esi,3
		jz		@@Start
		movsb
		dec		ecx
		jnz		@@ForceAlignment

@@Start:
		mov		eax,ecx
		shr		ecx,2
	rep	movsd
		mov		ecx,eax
		and		ecx,3
	rep	movsb

@@Done:	unuse_ds
		leave_c_nolocal
		ret

procend			_MGL_memcpyVIRTSRC

;----------------------------------------------------------------------------
; void MGL_memcpyVIRTDST(void *p,void *s,uint n)
;----------------------------------------------------------------------------
procstartdll	_MGL_memcpyVIRTDST

		ARG		p:DPTR, s:DPTR, n:UINT

		enter_c	0
		use_ds

		_les    _di,[p]			; ES:_DI -> memory block
		clrhi16	di				; ES:EDI -> memory block
		_lds    _si,[s]			; DS:_SI -> memory block
		clrhi16	si				; DS:ESI -> memory block
		mov		_cx,[n]
		clrhi16	cx				; ECX := count in bytes
		cld

; Force DWORD alignment for transfers in a virtualised buffer

@@ForceAlignment:
		test	edi,3
		jz		@@Start
		movsb
		dec		ecx
		jnz		@@ForceAlignment

@@Start:
		mov		eax,ecx
		shr		ecx,2
	rep	movsd
		mov		ecx,eax
		and		ecx,3
	rep	movsb

@@Done:	unuse_ds
		leave_c_nolocal
		ret

procend			_MGL_memcpyVIRTDST

endcodeseg		_memset

		END						; End of module
