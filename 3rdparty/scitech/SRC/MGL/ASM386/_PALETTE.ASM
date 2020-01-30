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
;* Description:	Fast palette rotation and fading routines.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

header	_palette				; Set up memory model

begcodeseg	_palette

;----------------------------------------------------------------------------
; void MGL_rotatePalette(MGLDC *dc,int numColors,int startIndex,
;	int direction)
;----------------------------------------------------------------------------
; Routine to rotate the values in the entire palette, in the specified
; direction.
;----------------------------------------------------------------------------
procstartdll	_MGL_rotatePalette

		ARG		dc:DPTR, numColors:UINT, startIndex:UINT, direction:UINT

		enter_c 0
		use_ds

		cld						; Moves go up in memory
		_les	_bx,[dc]		; ES:_BX -> device context
		mov		_cx,[numColors]	; _CX := number of colors in palette
		_lds	_si,[MGLDC.colorTab]	; DS:_SI -> palette to rotate
		mov		_ax,[startIndex]
		shl		_ax,2
		add		_si,_ax			; DS:_SI -> first entry to rotate

		dec		_cx				; _CX := colors - 1

		cmp     [BYTE direction],MGL_ROTATE_UP
		je		@@Forward

		std						; Moves go down in memory
		mov		_ax,_cx
		shl		_ax,2
		add		_si,_ax			; DS:_SI -> last byte in palette

@@Forward:
		es_eq_ds
		mov		_di,_si			; ES:_DI -> destination position

		lodsd					; Load first color and save
		mov		edx,eax
	rep	movsd					; Move middle colors
		mov		[es:_di],edx	; Store last color

		unuse_ds
		leave_c_nolocal
		ret

procend			_MGL_rotatePalette

;----------------------------------------------------------------------------
; bool MGL_fadePalette(MGLDC *dc,palette *fullIntensity,int numColors,
;   int startIndex, uchar intensity)
;----------------------------------------------------------------------------
; Routine to fade values in the palette by the specified intensity. We do
; this by scaling each of the color values by the intensity factor.
;----------------------------------------------------------------------------
procstartdll	_MGL_fadePalette

		ARG		dc:DPTR, fullIntensity:DPTR, numColors:UINT,		\
				startIndex:UINT, intensity:BYTE

		enter_c 0
		use_ds

		cld						; Moves go up in memory
		_les	_bx,[dc]		; ES:_BX -> device context
		_les	_di,[MGLDC.colorTab]; ES:_DI -> device context palette
		mov		_ax,[startIndex]
		shl		_ax,2
		add		_di,_ax			; ES:_DI -> first entry in dest. palette
		_lds	_si,[fullIntensity]	; DS:_SI -> full intensity palette
		mov		bl,[intensity]	; BX := intensity value
		mov		_cx,[numColors]	; _CX := number of entries to adjust

		xor		_bp,_bp			; _BP := flag for zero palette
		mov		bh,0FFh			; BH := maximum palette value

@@MainLoop:
		mov		al,[_si]
		mul		bl				; AX := value * intensity
		div		bh				; AL := (value * intensity) / 255
		mov		[es:_di],al
		xor		ah,ah
		or		bp,ax			; Save flag for zero palette
		mov		al,[_si+1]
		mul		bl				; AX := value * intensity
		div		bh				; AL := (value * intensity) / 255
		mov		[es:_di+1],al
		xor		ah,ah
		or		bp,ax			; Save flag for zero palette
		mov		al,[_si+2]
		mul		bl				; AX := value * intensity
		div		bh				; AL := (value * intensity) / 255
		mov		[es:_di+2],al
		xor		ah,ah
		or		bp,ax			; Save flag for zero palette
		add		_si,4			; Move to next color
		add		_di,4
		loop	@@MainLoop

		mov		_ax,_bp			; Return the zero flag
		unuse_ds
		leave_c_nolocal
		ret

procend			_MGL_fadePalette

endcodeseg	_palette

		END							; End of module

