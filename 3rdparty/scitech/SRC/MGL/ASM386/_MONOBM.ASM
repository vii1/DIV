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
;* Description:	Assembly language support routines for the bitmap
;*				manipulation routines in the monobm.c module.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

ifndef	MGL_LITE

header	_monobm					; Set up memory model

begdataseg	_monobm

; Rotation lookup table defined in monobm.c

		$EXTRN	__MGL_rotTable,ULONG
		$EXTRN	__MGL_mirrorTable,UCHAR

enddataseg	_monobm

begcodeseg	_monobm				; Start of code segment

;----------------------------------------------------------------------------
; void _MGL_rotateFull8x8AClockwise(uchar *dst,int dstStep,uchar *src,
;	int srcStep);
;----------------------------------------------------------------------------
;
; Rotates a full 8x8 bitmap tile anti-clockwise by using table lookup. The
; bit extraction is done a nybble at a time to reduce the table sizes.
;
; Entry:  	dst		- Starting address is detination bitmap
;			dstStep	- Difference in bytes between rows in dest bitmap
; 			src		- Starting address is source bitmap
;			srcStep	- Difference in bytes between rows in source bitmap
;
;----------------------------------------------------------------------------
procstart	__MGL_rotateFull8x8AClockwise

		ARG 	dst:DPTR, dstStep:UINT, src:DPTR, srcStep:UINT

		enter_c	0

		_les	_si,[src]				; ES:SI -> source bitmap
		xor		ecx,ecx					; ECX := lo
		xor		edx,edx					; EDX := hi

; Construct the 64 bits in the 8x8 tile in ECX and EDX

off = 7 * 16*4
		REPT    8

		mov		al,[_ES _si]			; AL := next byte in source
		mov		_bx,_ax
		add		_si,[srcStep]
		and		_bx,0Fh
		shl     _bx,2
		or		ecx,[_bx+__MGL_rotTable+off]	; lo |= _MGL_rotTabel[off][*src & 0xF]
		mov		_bx,_ax
		and		_bx,0F0h
		shr		_bx,2
		or		edx,[_bx+__MGL_rotTable+off]	; hi |= _MGL_rotTabel[off][*src >> 4]

off = off - 16*4
		ENDM

; Now store the 64 bits in the 8x8 destination tile

		_les	_di,[dst]				; ES:DI -> destination bitmap

		mov		eax,ecx					; EAX := lo
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 4th byte
		add		_di,[dstStep]			; Increment to next byte

		mov		eax,edx					; EAX := hi
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 4th byte

		leave_c_nolocal
		ret

procend		__MGL_rotateFull8x8AClockwise

;----------------------------------------------------------------------------
; void _MGL_rotatePartial8x8AClockwise(uchar *dst,int dstStep,uchar *src,
;	int srcStep,int height);
;----------------------------------------------------------------------------
;
; Rotates a Partial 8x8 bitmap tile anti-clockwise by using table lookup. The
; bit extraction is done a nybble at a time to reduce the table sizes.
;
; Entry:  	dst		- Starting address is detination bitmap
;			dstStep	- Difference in bytes between rows in dest bitmap
; 			src		- Starting address is source bitmap
;			srcStep	- Difference in bytes between rows in source bitmap
;			height	- Maximum height of source bitmap
;
;----------------------------------------------------------------------------
procstart	__MGL_rotatePartial8x8AClockwise

		ARG 	dst:DPTR, dstStep:UINT, src:DPTR, srcStep:UINT, height:UINT

		enter_c	0

		_les	_si,[src]				; ES:SI -> source bitmap
		xor		ecx,ecx					; ECX := lo
		xor		edx,edx					; EDX := hi

; Construct the 64 bits in the 8x8 tile in ECX and EDX

off = 7 * 16*4
		REPT    8

		dec		[height]				; Check to ensure within height
		js		@@StoreIt
		mov		al,[_ES _si]			; AL := next byte in source
		mov		_bx,_ax
		add		_si,[srcStep]
		and		_bx,0Fh
		shl     _bx,2
		or		ecx,[_bx+__MGL_rotTable+off]	; lo |= _MGL_rotTabel[off][*src & 0xF]
		mov		_bx,_ax
		and		_bx,0F0h
		shr		_bx,2
		or		edx,[_bx+__MGL_rotTable+off]	; hi |= _MGL_rotTabel[off][*src >> 4]

off = off - 16*4
		ENDM

; Now store the 64 bits in the 8x8 destination tile

@@StoreIt:
		_les	_di,[dst]				; ES:DI -> destination bitmap

		mov		eax,ecx					; EAX := lo
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 4th byte
		add		_di,[dstStep]			; Increment to next byte

		mov		eax,edx					; EAX := hi
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		shr		eax,8
		mov		[_ES _di],al			; Store 4th byte

		leave_c_nolocal
		ret

procend	__MGL_rotatePartial8x8AClockwise

;----------------------------------------------------------------------------
; void _MGL_rotateFull8x8Clockwise(uchar *dst,int dstStep,uchar *src,
;	int srcStep);
;----------------------------------------------------------------------------
;
; Rotates a full 8x8 bitmap tile clockwise by using table lookup. The
; bit extraction is done a nybble at a time to reduce the table sizes.
;
; Entry:  	dst		- Starting address is detination bitmap
;			dstStep	- Difference in bytes between rows in dest bitmap
; 			src		- Starting address is source bitmap
;			srcStep	- Difference in bytes between rows in source bitmap
;
;----------------------------------------------------------------------------
procstart	__MGL_rotateFull8x8Clockwise

		ARG 	dst:DPTR, dstStep:UINT, src:DPTR, srcStep:UINT

		enter_c	0

		_les	_si,[src]				; ES:SI -> source bitmap
		xor		ecx,ecx					; ECX := lo
		xor		edx,edx					; EDX := hi

; Construct the 64 bits in the 8x8 tile in ECX and EDX

off = 0
		REPT    8

		mov		al,[_ES _si]			; AL := next byte in source
		mov		_bx,_ax
		add		_si,[srcStep]
		and		_bx,0Fh
		shl     _bx,2
		or		ecx,[_bx+__MGL_rotTable+off]	; lo |= _MGL_rotTabel[off][*src & 0xF]
		mov		_bx,_ax
		and		_bx,0F0h
		shr		_bx,2
		or		edx,[_bx+__MGL_rotTable+off]	; hi |= _MGL_rotTabel[off][*src >> 4]

off = off + 16*4
		ENDM

; Now store the 64 bits in the 8x8 destination tile

		_les	_di,[dst]				; ES:DI -> destination bitmap

		mov		eax,edx					; EAX := hi
		rol		eax,8
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 4th byte
		add		_di,[dstStep]			; Increment to next byte

		mov		eax,ecx					; EAX := lo
		rol		eax,8
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 4th byte

		leave_c_nolocal
		ret

procend		__MGL_rotateFull8x8Clockwise

;----------------------------------------------------------------------------
; void _MGL_rotateFull8x8Clockwise(uchar *dst,int dstStep,uchar *src,
;	int srcStep,int height);
;----------------------------------------------------------------------------
;
; Rotates a full 8x8 bitmap tile clockwise by using table lookup. The
; bit extraction is done a nybble at a time to reduce the table sizes.
;
; Entry:  	dst		- Starting address is detination bitmap
;			dstStep	- Difference in bytes between rows in dest bitmap
; 			src		- Starting address is source bitmap
;			srcStep	- Difference in bytes between rows in source bitmap
;			height	- Maximum height of source bitmap
;
;----------------------------------------------------------------------------
procstart	__MGL_rotatePartial8x8Clockwise

		ARG 	dst:DPTR, dstStep:UINT, src:DPTR, srcStep:UINT, height:UINT

		enter_c	0

		_les	_si,[src]				; ES:SI -> source bitmap
		xor		ecx,ecx					; ECX := lo
		xor		edx,edx					; EDX := hi

; Construct the 64 bits in the 8x8 tile in ECX and EDX

off = 0
		REPT    8

		dec		[height]				; Check to ensure within height
		js		@@StoreIt
		mov		al,[_ES _si]			; AL := next byte in source
		mov		_bx,_ax
		add		_si,[srcStep]
		and		_bx,0Fh
		shl     _bx,2
		or		ecx,[_bx+__MGL_rotTable+off]	; lo |= _MGL_rotTabel[off][*src & 0xF]
		mov		_bx,_ax
		and		_bx,0F0h
		shr		_bx,2
		or		edx,[_bx+__MGL_rotTable+off]	; hi |= _MGL_rotTabel[off][*src >> 4]

off = off + 16*4
		ENDM

; Now store the 64 bits in the 8x8 destination tile

@@StoreIt:
		_les	_di,[dst]				; ES:DI -> destination bitmap

		mov		eax,edx					; EAX := hi
		rol		eax,8
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 4th byte
		add		_di,[dstStep]			; Increment to next byte

		mov		eax,ecx					; EAX := lo
		rol		eax,8
		mov		[_ES _di],al			; Store 1st byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 2nd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 3rd byte
		add		_di,[dstStep]			; Increment to next byte
		rol		eax,8
		mov		[_ES _di],al			; Store 4th byte

		leave_c_nolocal
		ret

procend		__MGL_rotatePartial8x8Clockwise

;----------------------------------------------------------------------------
; void _MGL_rotateBitmap180(uchar *dst,uchar *src,int byteWidth,int height);
;----------------------------------------------------------------------------
;
; Rotates an entire bitmap by 180 degrees using table lookup to mirror the
; byte values.
;
; Entry:  	dst			- Starting address of detination bitmap
; 			src			- Starting address of source bitmap
;			byteWidth	- Width of bitmap in bytes
;			height		- Maximum height of source bitmap
;
;----------------------------------------------------------------------------
procstart	__MGL_rotateBitmap180

		ARG 	dst:DPTR, src:DPTR, byteWidth:UINT, height:UINT

		enter_c	0

		mov		_ax,[byteWidth]
		mul		[height]				; _AX := byteWidth * height
		mov		_cx,_ax					; _CX := count of bytes to process
		_lfs	_si,[src]				; FS:_SI -> source bitmap
		_les	_di,[dst]				; ES:_DI -> destination bitmap
		add		_di,_cx                 ; Move to end of dest buffer
		xor		_bx,_bx					; Clear out index register

@@MirrorLoop:
		dec		_di						; Decrement dest bitmap ptr
		mov     bl,[_FS _si]			; _BX := index into table
		inc		_si						; Increment source bitmap ptr
		mov		bl,[_bx+__MGL_mirrorTable]; Lookup mirrored value
		mov     [_ES _di],bl			; Store value
		loop	@@MirrorLoop

		leave_c_nolocal
		ret

procend		__MGL_rotateBitmap180

endcodeseg	_monobm

endif   ; !MGL_LITE

		END							; End of module
