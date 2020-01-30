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
;* Description: High performance color format translation BitBlt routines.
;*				This entire module is high performance 32 bit code, so does
;*				not run in real mode.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

ifndef	MGL_LITE

if pmode

header	_em_tran				; Setup memory model

begdataseg	_em_tran

		$EXTRN	__MGL_buf,DPTR
		$EXTRN  __MGL_div51,UCHAR
		$EXTRN  __MGL_mod51,UCHAR
		$EXTRN  __MGL_mul6,UCHAR
		$EXTRN  __MGL_mul36,UCHAR
		$EXTRN  __MGL_dither8x8,UCHAR

enddataseg	_em_tran

begcodeseg		_em_tran		; Start of code segment

;----------------------------------------------------------------------------
; void _MGL_translateRGB15to8(MGLDC *dc,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine)
;----------------------------------------------------------------------------
; Converts a 24 bit RGB bitmap to an 8 bit bitmap by doing an ordered
; 8x8 halftone dither. Although we render into a temporary buffer and then
; blt this to the destination context, the overal time spent moving data
; to the desination context is very very small (less than 1%) because
; the entire temporary scanline is still sitting in the CPU L1 cache!!
;----------------------------------------------------------------------------
procstart	__MGL_translateRGB15to8

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR, 		\
				bytesPerLine:UINT

		LOCAL	count:ULONG, lcount:ULONG, bpl:ULONG, 					\
				ydither:ULONG, y:ULONG, buf:ULONG, dstRight:UINT = LocalSize

		enter_c	LocalSize
		use_ds

; Extract dimensions and save in local variables

		mov		_bx,[right]
		sub		_bx,[left]			; BX := right - left
		clrhi16	bx
		mov		[count],ebx
		mov     _ax,[dstLeft]
		add		_ax,_bx				; AX := dstLeft + count
		mov		[dstRight],_ax
		mov		_ax,[bottom]
		sub		_ax,[top]			; AX := bottom - top
		clrhi16	ax
		mov		[lcount],eax
		mov		_ax,[bytesPerLine]
		clrhi16	ax
		mov		[bpl],eax
		_les	_di,[__MGL_buf]
		clrhi16	di
		mov		[buf],edi				; ES:EDI -> temporary buffer

		_lds	_si,[surface]
		clrhi16	si						; DS:ESI -> bitmap surface
		mov		_ax,[dstTop]
		clrhi16	ax
		mov		[y],eax

@@NextScanline:
        mov     ecx,[count]
		push	[count]
		push	esi
		mov     eax,[y]
		and		eax,7
		shl		eax,3					; EAX := (y & 7) << 3
		add		eax,offset __MGL_dither8x8
		mov		[ydither],eax
		mov		edi,[buf]

@@NextPixel:
		mov		ax,[esi]
		mov		ecx,eax
		shl		cx,6
		and		ch,0F8h					; CH := 8 bit green component
		mov		cl,al
		shl		cl,3					; CL := 8 bit blue component
		shr     ax,7
		and		al,0F8h
		mov		dl,al					; DL := 8 bit red component

; Halftone dither the extracted RGB value

		xchg	esi,[ydither]
		MGL_ditherPixel					; Dither it!
		xchg	esi,[ydither]
		mov		[_ES edi],al			; Store the dithered pixel
		inc		edi

		inc		esi
		inc		esi						; Move to next source pixel
		inc     ebx
		dec		[count]					; Count off this pixel
		jnz		@@NextPixel

; Blt the scanline to destination context from temporary buffer

		use_es
		_les	_bx,[dc]
		push	[op]
		xor		_ax,_ax
		push    _ax
		push    [__MGL_buf]
		push	[dstRight]
		push	[dstLeft]
		push	[y]
		push	[dc]
		call	[MGLDC.r.putScanline]
		_add    sp,20,28
		unuse_es

		pop		esi
		pop		[count]
		add		esi,[bpl]				; Move to next scanline
		inc		[y]
        mov     ecx,[lcount]
        dec     [lcount]                ; Count off this scanline
		jnz		@@NextScanline

@@Exit:	unuse_ds
		leave_c
		ret

procend		__MGL_translateRGB15to8

;----------------------------------------------------------------------------
; void _MGL_translateRGB16to8(MGLDC *dc,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine)
;----------------------------------------------------------------------------
; Converts a 24 bit RGB bitmap to an 8 bit bitmap by doing an ordered
; 8x8 halftone dither. Although we render into a temporary buffer and then
; blt this to the destination context, the overal time spent moving data
; to the desination context is very very small (well less than 1%) because
; the entire temporary scanline is still sitting in the CPU L1 cache!!
;----------------------------------------------------------------------------
procstart	__MGL_translateRGB16to8

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR, 		\
				bytesPerLine:UINT

		LOCAL	count:ULONG, lcount:ULONG, bpl:ULONG, 					\
				ydither:ULONG, y:ULONG, buf:ULONG, dstRight:UINT = LocalSize

		enter_c	LocalSize
		use_ds

; Extract dimensions and save in local variables

		mov		_bx,[right]
		sub		_bx,[left]			; BX := right - left
		clrhi16	bx
		mov		[count],ebx
		mov     _ax,[dstLeft]
		add		_ax,_bx				; AX := dstLeft + count
		mov		[dstRight],_ax
		mov		_ax,[bottom]
		sub		_ax,[top]			; AX := bottom - top
		clrhi16	ax
		mov		[lcount],eax
		mov		_ax,[bytesPerLine]
		clrhi16	ax
		mov		[bpl],eax
		_les	_di,[__MGL_buf]
		clrhi16	di
		mov		[buf],edi				; ES:EDI -> temporary buffer

		_lds	_si,[surface]
		clrhi16	si						; DS:ESI -> bitmap surface
		mov		_ax,[dstTop]
		clrhi16	ax
		mov		[y],eax

@@NextScanline:
        mov     ecx,[count]
		push	[count]
		push	esi
		mov     eax,[y]
		and		eax,7
		shl		eax,3					; EAX := (y & 7) << 3
		add		eax,offset __MGL_dither8x8
		mov		[ydither],eax
		mov		ebx,[left]				; EBX := X dither index
		mov		edi,[buf]

@@NextPixel:
		mov		ax,[esi]
		mov		cx,ax
		shl		cx,5
		and		ch,0FCh					; CH := 8 bit green component
		mov		cl,al
		shl		cl,3					; CL := 8 bit blue component
		shr     ax,8
		and		al,0F8h
		mov		dl,al					; DL := 8 bit red component

; Halftone dither the extracted RGB value

		xchg	esi,[ydither]
		MGL_ditherPixel					; Dither it!
		xchg	esi,[ydither]
		mov		[_ES edi],al			; Store the dithered pixel
		inc		edi

		inc		esi
		inc		esi						; Move to next source pixel
		inc     ebx
		dec		[count]					; Count off this pixel
		jnz		@@NextPixel

; Blt the scanline to destination context from temporary buffer

		use_es
		_les	_bx,[dc]
		push	[op]
		xor		_ax,_ax
		push    _ax
		push    [__MGL_buf]
		push	[dstRight]
		push	[dstLeft]
		push	[y]
		push	[dc]
		call	[MGLDC.r.putScanline]
		_add    sp,20,28
		unuse_es

		pop		esi
		pop		[count]
		add		esi,[bpl]				; Move to next scanline
		inc		[y]
        mov     ecx,[lcount]
        dec     [lcount]                ; Count off this scanline
		jnz		@@NextScanline

@@Exit:	unuse_ds
		leave_c
		ret

procend		__MGL_translateRGB16to8

;----------------------------------------------------------------------------
; void _MGL_translateRGB24to8(MGLDC *dc,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine)
;----------------------------------------------------------------------------
; Converts a 24 bit RGB bitmap to an 8 bit bitmap by doing an ordered
; 8x8 halftone dither. Although we render into a temporary buffer and then
; blt this to the destination context, the overal time spent moving data
; to the desination context is very very small (well less than 1%) because
; the entire temporary scanline is still sitting in the CPU L1 cache!!
;----------------------------------------------------------------------------
procstart	__MGL_translateRGB24to8

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR, 		\
				bytesPerLine:UINT

		LOCAL	count:ULONG, lcount:ULONG, bpl:ULONG, 					\
				ydither:ULONG, y:ULONG, buf:ULONG, dstRight:UINT = LocalSize

		enter_c	LocalSize
		use_ds

; Extract dimensions and save in local variables

		mov		_bx,[right]
		sub		_bx,[left]			; BX := right - left
		clrhi16	bx
		mov		[count],ebx
		mov     _ax,[dstLeft]
		add		_ax,_bx				; AX := dstLeft + count
		mov		[dstRight],_ax
		mov		_ax,[bottom]
		sub		_ax,[top]			; AX := bottom - top
		clrhi16	ax
		mov		[lcount],eax
		mov		_ax,[bytesPerLine]
		clrhi16	ax
		mov		[bpl],eax
		_les	_di,[__MGL_buf]
		clrhi16	di
		mov		[buf],edi				; ES:EDI -> temporary buffer

		_lds	_si,[surface]
		clrhi16	si						; DS:ESI -> bitmap surface
		mov		_ax,[dstTop]
		clrhi16	ax
		mov		[y],eax

@@NextScanline:
        mov     ecx,[count]
		push	[count]
		push	esi
		mov     eax,[y]
		and		eax,7
		shl		eax,3					; EAX := (y & 7) << 3
		add		eax,offset __MGL_dither8x8
		mov		[ydither],eax
		mov		ebx,[left]				; EBX := X dither index
		mov		edi,[buf]

@@NextPixel:
		mov     cx,[esi]				; CL := 8 bit blue component
										; CH := 8 bit green component
		mov		dl,[esi+2]				; DL := 8 bit red component

; Halftone dither the extracted RGB value

		xchg	esi,[ydither]
		MGL_ditherPixel					; Dither it!
		xchg	esi,[ydither]
		mov		[_ES edi],al			; Store the dithered pixel
		inc		edi

		inc		esi
		inc		esi
		inc		esi						; Move to next source pixel
		inc     ebx
		dec		[count]					; Count off this pixel
		jnz		@@NextPixel

; Blt the scanline to destination context from temporary buffer

		use_es
		_les	_bx,[dc]
		push	[op]
		xor		_ax,_ax
		push    _ax
		push    [__MGL_buf]
		push	[dstRight]
		push	[dstLeft]
		push	[y]
		push	[dc]
		call	[MGLDC.r.putScanline]
		_add    sp,20,28
		unuse_es

		pop		esi
		pop		[count]
		add		esi,[bpl]				; Move to next scanline
		inc		[y]
        mov     ecx,[lcount]
        dec     [lcount]                ; Count off this scanline
		jnz		@@NextScanline

@@Exit:	unuse_ds
		leave_c
		ret

procend		__MGL_translateRGB24to8

;----------------------------------------------------------------------------
; void _MGL_translateBGR24to8(MGLDC *dc,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine)
;----------------------------------------------------------------------------
; Converts a 24 bit RGB bitmap to an 8 bit bitmap by doing an ordered
; 8x8 halftone dither. Although we render into a temporary buffer and then
; blt this to the destination context, the overal time spent moving data
; to the desination context is very very small (well less than 1%) because
; the entire temporary scanline is still sitting in the CPU L1 cache!!
;----------------------------------------------------------------------------
procstart	__MGL_translateBGR24to8

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR, 		\
				bytesPerLine:UINT

		LOCAL	count:ULONG, lcount:ULONG, bpl:ULONG, 					\
				ydither:ULONG, y:ULONG, buf:ULONG, dstRight:UINT = LocalSize

		enter_c	LocalSize
		use_ds

; Extract dimensions and save in local variables

		mov		_bx,[right]
		sub		_bx,[left]			; BX := right - left
		clrhi16	bx
		mov		[count],ebx
		mov     _ax,[dstLeft]
		add		_ax,_bx				; AX := dstLeft + count
		mov		[dstRight],_ax
		mov		_ax,[bottom]
		sub		_ax,[top]			; AX := bottom - top
		clrhi16	ax
		mov		[lcount],eax
		mov		_ax,[bytesPerLine]
		clrhi16	ax
		mov		[bpl],eax
		_les	_di,[__MGL_buf]
		clrhi16	di
		mov		[buf],edi				; ES:EDI -> temporary buffer

		_lds	_si,[surface]
		clrhi16	si						; DS:ESI -> bitmap surface
		mov		_ax,[dstTop]
		clrhi16	ax
		mov		[y],eax

@@NextScanline:
        mov     ecx,[count]
		push	[count]
		push	esi
		mov     eax,[y]
		and		eax,7
		shl		eax,3					; EAX := (y & 7) << 3
		add		eax,offset __MGL_dither8x8
		mov		[ydither],eax
		mov		ebx,[left]				; EBX := X dither index
		mov		edi,[buf]

@@NextPixel:
		mov     cx,[esi+1]				; CL := 8 bit blue component
		xchg	ch,cl					; CH := 8 bit green component
		mov		dl,[esi]				; DL := 8 bit red component

; Halftone dither the extracted RGB value

		xchg	esi,[ydither]
		MGL_ditherPixel					; Dither it!
		xchg	esi,[ydither]
		mov		[_ES edi],al			; Store the dithered pixel
		inc		edi

		inc		esi
		inc		esi
		inc		esi						; Move to next source pixel
		inc     ebx
		dec		[count]					; Count off this pixel
		jnz		@@NextPixel

; Blt the scanline to destination context from temporary buffer

		use_es
		_les	_bx,[dc]
		push	[op]
		xor		_ax,_ax
		push    _ax
		push    [__MGL_buf]
		push	[dstRight]
		push	[dstLeft]
		push	[y]
		push	[dc]
		call	[MGLDC.r.putScanline]
		_add    sp,20,28
		unuse_es

		pop		esi
		pop		[count]
		add		esi,[bpl]				; Move to next scanline
		inc		[y]
        mov     ecx,[lcount]
        dec     [lcount]                ; Count off this scanline
		jnz		@@NextScanline

@@Exit:	unuse_ds
		leave_c
		ret

procend		__MGL_translateBGR24to8

endcodeseg		_em_tran

endif	; pmode

endif	; !MGL_LITE

		END						; End of module
