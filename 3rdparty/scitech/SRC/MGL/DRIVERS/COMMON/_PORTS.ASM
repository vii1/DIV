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
;* Language:    80386 Assembler
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	Simplified I/O port manipulation routines.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"

header	_ports

begcodeseg	_ports

;----------------------------------------------------------------------------
; int UV_inp(int port)
;----------------------------------------------------------------------------
; Reads a byte from the specified port
;----------------------------------------------------------------------------
procstart	_UV_inp

		ARG		port:UINT

		push	_bp
		mov		_bp,_sp
		xor		_ax,_ax
		mov		_dx,[port]
		in		al,dx
		pop		_bp
		ret

procend		_UV_inp

;----------------------------------------------------------------------------
; int UV_inpw(int port)
;----------------------------------------------------------------------------
; Reads a word from the specified port
;----------------------------------------------------------------------------
procstart	_UV_inpw

		ARG		port:UINT

		push	_bp
		mov		_bp,_sp
		xor		_ax,_ax
		mov		_dx,[port]
		in		ax,dx
		pop		_bp
		ret

procend		_UV_inpw

;----------------------------------------------------------------------------
; ulong UV_inpd(int port)
;----------------------------------------------------------------------------
; Reads a word from the specified port
;----------------------------------------------------------------------------
procstart	_UV_inpd

		ARG		port:UINT

		push	_bp
		mov		_bp,_sp
		xor		_ax,_ax
		mov		_dx,[port]
		in		eax,dx
ife flatmodel
		shld	edx,eax,16		; DX:AX = result
endif
		pop		_bp
		ret

procend		_UV_inpd

;----------------------------------------------------------------------------
; void UV_outp(int port,int value)
;----------------------------------------------------------------------------
; Write a byte to the specified port.
;----------------------------------------------------------------------------
procstart	_UV_outp

		ARG		port:UINT, value:UINT

		push	_bp
		mov		_bp,_sp
		mov		_dx,[port]
		mov		_ax,[value]
		out		dx,al
		pop		_bp
		ret

procend		_UV_outp

;----------------------------------------------------------------------------
; void UV_outpw(int port,int value)
;----------------------------------------------------------------------------
; Write a word to the specified port.
;----------------------------------------------------------------------------
procstart	_UV_outpw

		ARG		port:UINT, value:UINT

		push	_bp
		mov		_bp,_sp
		mov		_dx,[port]
		mov		_ax,[value]
		out		dx,ax
		pop		_bp
		ret

procend		_UV_outpw

;----------------------------------------------------------------------------
; void UV_outpd(int port,ulong value)
;----------------------------------------------------------------------------
; Write a word to the specified port.
;----------------------------------------------------------------------------
procstart	_UV_outpd

		ARG		port:UINT, value:ULONG

		push	_bp
		mov		_bp,_sp
		mov		_dx,[port]
		mov		eax,[value]
		out		dx,eax
		pop		_bp
		ret

procend		_UV_outpd

MACRO	IODELAY
	local	DLY1,DLY2
	jmp	short DLY1
DLY1:
	jmp short DLY2
DLY2:
ENDM

;----------------------------------------------------------------------------
; uchar UV_readCMOS(int index)
;----------------------------------------------------------------------------
; Read the value of a specific CMOS register. We do this with both
; normal interrupts and NMI disabled.
;----------------------------------------------------------------------------
procstart	_UV_readCMOS

		ARG		index:UINT

		push	_bp
		mov		_bp,_sp
		mov		al,[BYTE index]
		or		al,80h				; Add disable NMI flag
		cli
		out		70h,al
		IODELAY
		in		al,71h
		mov		ah,al
		xor		al,al
		IODELAY
		out		70h,al				; Re-enable NMI
		sti
		mov		al,ah				; Return value in AL
		pop		_bp
		ret

procend		_UV_readCMOS


;----------------------------------------------------------------------------
; void UV_enable(void)
;----------------------------------------------------------------------------
procstart	_UV_enable

		sti
		ret

procend		_UV_enable

;----------------------------------------------------------------------------
; void UV_disable(void)
;----------------------------------------------------------------------------
procstart	_UV_disable

		cli
		ret

procend		_UV_disable

endcodeseg	_ports

		END							; End of module
