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
;* Description:	Optimised assembly language routines for the devctx module.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

header	_devctx					; Set up memory model

begcodeseg	_devctx				; Start of code segment

;----------------------------------------------------------------------------
; color_t MGL_packColor(pixel_format_t *pf, uchar R,uchar G,uchar B)
;----------------------------------------------------------------------------
; This routine builds a packed color value in the correct format for the
; current color mode, taking 8 bit RGB values in the range 0-255.
;----------------------------------------------------------------------------
procstartdll	_MGL_packColor

		ARG		pf:DPTR, r:UCHAR, g:UCHAR, b:UCHAR

		push	_bp
		mov		_bp,_sp
		push	_bx

		xor		eax,eax			; Place to accumulate resultant color
		_les	_bx,[pf]		; ES:_BX -> pixel format buffer
		xor		edx,edx
		mov		dl,[r]
		mov		cl,[BYTE pixel_format.redAdjust]
		shr		dl,cl
		and		dl,[BYTE pixel_format.redMask]
		mov		cl,[BYTE pixel_format.redPos]
		shl		edx,cl
		or		eax,edx
		xor		edx,edx
		mov		dl,[g]
		mov		cl,[BYTE pixel_format.greenAdjust]
		shr		dl,cl
		and		dl,[BYTE pixel_format.greenMask]
		mov		cl,[BYTE pixel_format.greenPos]
		shl		edx,cl
		or		eax,edx
		xor		edx,edx
		mov		dl,[b]
		mov		cl,[BYTE pixel_format.blueAdjust]
		shr		dl,cl
		and		dl,[BYTE pixel_format.blueMask]
		mov		cl,[BYTE pixel_format.bluePos]
		shl		edx,cl
		or		eax,edx
ife flatmodel
		shld	edx,eax,16		; Place result into DX:AX
endif
		pop		_bx
		pop		_bp
		ret32

procend			_MGL_packColor

endcodeseg	_devctx

		END						; End of module

