;****************************************************************************
;*
;*			The SuperVGA Kit - UniVBE Software Development Kit
;*
;*                  Copyright (C) 1996 SciTech Software
;*                          All rights reserved.
;*
;* Filename:    $Workfile:   _linsdk.asm  $
;* Version:     $Revision:   1.3  $
;*
;* Language:    80386 Assembler
;* Environment: IBM PC 32 bit Protected Mode.
;*
;* Description: Assembly language support routines for the SuperVGA Kit
;*				UniVBE Software Development Kit. This module contains the
;*				32 bit linear frame buffer drawing routines.
;*
;*				Note that even though this code is 32 bit protected mode,
;*				it can be called directly from 16 bit protected mode code
;*				using a 16 bit far pointer with the selector set to a USE32
;*				code segment (which the code in this segment is). To convert
;*				the CS selector to a proper 32 bit selector, call the
;*				function _linksdk_enable32() before calling any functions
;*              in this segment. You only need to call this routine once
;*              for the entire execution of the program (but you may call it
;*              more than once - it will simply return if the selector is
;*              already 32 bit).
;*
;* $Date:   19 Apr 1996 20:06:58  $ $Author:   KendallB  $
;*
;****************************************************************************

		IDEAL

include "model.mac"             ; Memory model macros
include "vbeaf.inc"             ; VBE/AF Accelerator Functions
include	"svga.inc"				; SuperVGA Kit structures

if pmode

header	_linsdk

CRTC    EQU 3D4h        		; Port of CRTC registers

begdataseg	_linsdk

DC		EQU		__SV_devCtx		; Macro to access global structure

		$EXTRN	__SV_devCtx,SV_devCtx_s
		$EXTRN  __SV_pagesize,ULONG
		$EXTRN  __SV_bankShift,UINT
		$EXTRN	__SV_setBankRM,DPTR
		$EXTRN	__SV_setBankPtr,DPTR

enddataseg	_linsdk

begcodeseg32	_linsdk

; Define the _linksdk_enable32 function to convert the CS selector to 32 bit.

define_enable32	_linsdk

; Macro to load the linear framebuffer video memory selector into the ES
; register for 16 bit Windows code.

MACRO	load_es_videoSel
ife flatmodel
		mov		es,[(WORD DC.videoMem)+2]; ES := linear framebuffer selector
endif
ENDM

;----------------------------------------------------------------------------
; PixelAddr256	Determine buffer address of pixel in _linsdk 256 color modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			EBX	-	byte offset in buffer
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	PixelAddr256	near

		clrhi16	ax					; EAX := y
		clrhi16	bx					; EBX := x
		mul		[DC.bytesperline]	; EAX := y * bytesperline
		add		ebx,eax				; EBX := y * BytesPerLine + x
		add		ebx,[DC.originOffset]
		ret

ENDP	PixelAddr256

;----------------------------------------------------------------------------
; PixelAddr32k	Determine buffer address of pixel in _linsdk 32k color modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			EBX	-	byte offset in buffer
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	PixelAddr32k	near

		clrhi16	ax					; EAX := y
		clrhi16	bx					; EBX := x
		mul		[DC.bytesperline]	; EAX := y * bytesperline
		shl		ebx,1
		add		ebx,eax				; EBX := y * BytesPerLinef + x * 2
		add		ebx,[DC.originOffset]
		ret

ENDP	PixelAddr32k

;----------------------------------------------------------------------------
; PixelAddr16m	Determine buffer address of pixel in _linsdk 16m color modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			EBX	-	byte offset in buffer
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	PixelAddr16m	near

		clrhi16	ax					; EAX := y
		clrhi16	bx					; EBX := x
		mul		[DC.bytesperline]	; EAX := y * bytesperline
		add		eax,ebx
		shl		ebx,1
		add		ebx,eax				; EBX := y * BytesPerLine + x * 3
		add		ebx,[DC.originOffset]
		ret

ENDP	PixelAddr16m

;----------------------------------------------------------------------------
; PixelAddr4G	Determine buffer address of pixel in _linsdk 32k color modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			EBX	-	byte offset in buffer
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	PixelAddr4G	near

		clrhi16	ax					; EAX := y
		clrhi16	bx					; EBX := x
		mul		[DC.bytesperline]	; EAX := y * bytesperline
		shl		ebx,2
		add		ebx,eax				; EBX := y * BytesPerLine + x * 2
		add		ebx,[DC.originOffset]
		ret

ENDP	PixelAddr4G

;----------------------------------------------------------------------------
; clearLinear	- Clears the linear framebuffer page
;----------------------------------------------------------------------------
; Entry:		EAX	- Value to clear buffer with
;
; Registers:	All.
;----------------------------------------------------------------------------
PROC	clearLinear

		load_es_videoSel
		push	eax
		mov     _ax,[DC.maxy]
		clrhi16	ax					; EAX := maxy
		inc     eax
		mul		[DC.bytesperline]	; EAX := number of bytes to fill
		mov     ecx,eax             ; ECX := bytes in last bank to fill
		mov		edi,[DC.originOffset]	; EDI -> start of video memory
		cld                         ; Moves go up in memory
		pop		eax
		shr     ecx,2               ; ECX := number of double USHORTs to set
	rep stosd

		ret

ENDP	clearLinear

;----------------------------------------------------------------------------
; void clear256Lin(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstart32   __SV_clear256Lin

		ARG		color:ULONG

		enter_c 0

		xor		eax,eax
		mov		al,[BYTE color]
		mov		ebx,eax
		shl		ebx,8
		or		eax,ebx
		mov		ebx,eax
		shl		ebx,16
		or		eax,ebx				; EAX = 32 bit color value
		call	clearLinear

		leave_c_nolocal
		ret32

procend     __SV_clear256Lin

;----------------------------------------------------------------------------
; void clear32kLin(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstart32   __SV_clear32kLin

		ARG		color:ULONG

		enter_c	0

		xor		eax,eax
		mov		ax,[WORD color]
		mov		ebx,eax
		shl		ebx,16
		or		eax,ebx				; EAX = 32 bit color value
		call	clearLinear

		leave_c_nolocal
		ret32

procend     __SV_clear32kLin

;----------------------------------------------------------------------------
; void clear16mLin(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstart32   __SV_clear16mLin

		ARG		color:ULONG

		enter_c	0

		mov		eax,[color]
		or		eax,eax
		jnz		@@SlowClear
		call	clearLinear			; Clear the video memory page to black
		jmp		@@Exit

@@SlowClear:
		load_es_videoSel
		mov		ebx,[DC.originOffset]	; ES:EBX -> start of video memory
		mov		_di,[DC.maxx]
		clrhi16	di
		inc		edi					; EDI := number of pixels to draw
		mov		_si,[DC.maxy]
		clrhi16	si
		inc		esi					; ESI := number of lines to process
		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		mov		ebp,edi
		shl		ebp,1
		add		ebp,edi				; EBP := bytes per physical scanline
		sub		ebp,[DC.bytesperline]
		neg		ebp					; EBP := scanline adjust factor

@@NextScanLine:
		mov		ecx,edi

@@LoopSolid:
		mov		[BYTE _ES ebx],al  	; Set pixel value in buffer
		mov		[BYTE _ES ebx+1],ah
		mov		[BYTE _ES ebx+2],dh
		add		ebx,3				; Increment to next pixel
		loop	@@LoopSolid			; Loop across line

@@AfterPlot:
		add		ebx,ebp
		dec		esi
		jnz		@@NextScanLine

@@Exit:	leave_c_nolocal
		ret32

procend     __SV_clear16mLin

;----------------------------------------------------------------------------
; void clear4GLin(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstart32   __SV_clear4GLin

		ARG		color:ULONG

		enter_c	0

		mov		eax,[color]
		call	clearLinear

		leave_c_nolocal
		ret32

procend     __SV_clear4GLin

;----------------------------------------------------------------------------
; void putPixel256Lin(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstart32   __SV_putPixel256Lin

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c 0

		load_es_videoSel
		mov     _ax,[y]
		mov		_bx,[x]
		clrhi16	ax					; EAX := y * BytesPerLine
		clrhi16	bx					; EBX := x
		mul     [DC.bytesperline]
		add     ebx,[DC.originOffset]
		add		ebx,eax             ; EBX := y * BytesPerLine + x
		mov     al,[BYTE color]
		mov     [_ES ebx],al        ; Replace the pixel

		leave_c_nolocal
		ret32

procend     __SV_putPixel256Lin

;----------------------------------------------------------------------------
; void putPixel32kLin(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstart32   __SV_putPixel32kLin

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c	0

		load_es_videoSel
		mov     _ax,[y]
		mov		_bx,[x]
		clrhi16	ax					; EAX := y * BytesPerLine
		clrhi16	bx					; EBX := x
		mul     [DC.bytesperline]
		shl		ebx,1
		add     ebx,[DC.originOffset]
		add		ebx,eax             ; EBX := y * BytesPerLine + x
		mov     ax,[WORD color]
		mov     [_ES ebx],ax        ; Replace the pixel

		leave_c_nolocal
		ret32

procend     __SV_putPixel32kLin

;----------------------------------------------------------------------------
; void putPixel16mLin(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstart32   __SV_putPixel16mLin

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c	0

		load_es_videoSel
		mov     _ax,[y]
		mov		_bx,[x]
		clrhi16	ax					; EAX := y * BytesPerLine
		clrhi16	bx					; EBX := x
		mul     [DC.bytesperline]
		add     eax,ebx
		shl     ebx,1
		add     eax,ebx
		add     eax,[DC.originOffset]
		mov     ebx,eax             ; EBX := y * BytesPerLine + x * 3

		mov     eax,[color]
		mov     [_ES ebx],al        ; Replace the first byte
		mov     [_ES ebx+1],ah
		shr		eax,16
		mov		[_ES ebx+2],al

@@Exit: leave_c_nolocal
		ret32

procend     __SV_putPixel16mLin

;----------------------------------------------------------------------------
; void putPixel4GLin(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstart32   __SV_putPixel4GLin

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c	0

		load_es_videoSel
		mov     _ax,[y]
		mov		_bx,[x]
		clrhi16	ax					; EAX := y * BytesPerLine
		clrhi16	bx					; EBX := x
		mul     [DC.bytesperline]
		shl		ebx,2
		add     ebx,[DC.originOffset]
		add		ebx,eax             ; EBX := y * BytesPerLine + x
		mov     eax,[color]
		mov     [_ES ebx],eax       ; Replace the pixel

		leave_c_nolocal
		ret32

procend     __SV_putPixel4GLin

;----------------------------------------------------------------------------
; void _SV_line256Lin(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw
;				y1		- Y1 coordinate of line to draw
;				x2		- X2 coordinate of line to draw
;				y2		- Y2 coordinate of line to draw
;				color	- color to draw the line in
;
;----------------------------------------------------------------------------
procstart32	__SV_line256Lin

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	LoSlope:USHORT, VertInc:ULONG, EIncr:ULONG, 			\
				NEIncr:ULONG = LocalSize

		enter_c	LocalSize

		mov		esi,[DC.bytesperline]	; ESI = Increment for video buffer

		mov		_ax,[x2]
		sub		_ax,[x1]			; _AX := X2 - X1

; Force X1 < X2

		jns		@@X2Greater			; Jump if X2 > X1
		neg		_ax					; _AX := X1 - X2

		mov		_bx,[x2]			; Exchange X1 and X2
		xchg	_bx,[x1]
		mov		[x2],_bx

		mov		_bx,[y2]			; Exchange Y1 and Y2
		xchg	_bx,[y1]
		mov		[y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
		mov		_bx,[y2]
		sub		_bx,[y1]			; _BX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		_bx					; _BX := Y1 - Y2
		neg		esi					; negative increment for buffer

; select appropriate routine for slope of line

@@Y2Greater:
		clrhi16	ax					; EAX := dx
		clrhi16	bx					; EBX := dy
		mov		[VertInc],esi		; save increment
		mov		[LoSlope],1
		cmp		ebx,eax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[LoSlope],0
		xchg	ebx,eax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		ebx,1				; _BX := 2 * dy
		mov		[EIncr],ebx			; EIncr := 2 * dy
		sub		ebx,eax				; d = 2 * dy - dx
		mov		edi,ebx				; EDI := initial decision variable
		sub		ebx,eax
		mov		[NEIncr],ebx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	eax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	PixelAddr256		; EBX -> buffer
		load_es_videoSel

		pop		ecx					; Restore dx
		inc		ecx					; CX := # pixels to draw
		test	[LoSlope],1
		jz		@@HiSlopeLine		; This is a high slope line

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		al,[BYTE color]		; AL := pixel value to fill

@@LoopLo:
		mov		[_ES ebx],al		; Set pixel value in buffer
		inc		ebx					; Increment x coordinate
		or		edi,edi				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[VertInc]		; increment y
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		al,[BYTE color]		; AL := pixel value to fill

@@LoopHi:
		mov		[_ES ebx],al		; Set pixel value in buffer
		add		ebx,[VertInc]		; increment y
		or		edi,edi				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx					; Increment x
		loop	@@LoopHi			; Loop for remaining pixels

@@Exit:
		leave_c
		ret32

procend		__SV_line256Lin

;----------------------------------------------------------------------------
; void _SV_line32kLin(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw
;				y1		- Y1 coordinate of line to draw
;				x2		- X2 coordinate of line to draw
;				y2		- Y2 coordinate of line to draw
;				color	- color to draw the line in
;
;----------------------------------------------------------------------------
procstart32	__SV_line32kLin

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	LoSlope:ULONG, VertInc:ULONG, EIncr:ULONG, 			\
				NEIncr:ULONG = LocalSize

		enter_c	LocalSize
		cld

		mov		esi,[DC.bytesperline]	; ESI = Increment for video buffer

		mov		_ax,[x2]
		sub		_ax,[x1]			; _AX := X2 - X1

; Force X1 < X2

		jns		@@X2Greater			; Jump if X2 > X1
		neg		_ax					; _AX := X1 - X2

		mov		_bx,[x2]			; Exchange X1 and X2
		xchg	_bx,[x1]
		mov		[x2],_bx

		mov		_bx,[y2]			; Exchange Y1 and Y2
		xchg	_bx,[y1]
		mov		[y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
		mov		_bx,[y2]
		sub		_bx,[y1]			; _BX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		_bx					; _BX := Y1 - Y2
		neg		esi					; negative increment for buffer

; select appropriate routine for slope of line

@@Y2Greater:
		clrhi16	ax					; EAX := dx
		clrhi16	bx					; EBX := dy
		mov		[VertInc],esi		; save increment
		mov		[LoSlope],1
		cmp		ebx,eax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[LoSlope],0
		xchg	ebx,eax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		ebx,1				; EBX := 2 * dy
		mov		[EIncr],ebx			; EIncr := 2 * dy
		sub		ebx,eax				; d = 2 * dy - dx
		mov		edi,ebx				; EDI := initial decision variable
		sub		ebx,eax
		mov		[NEIncr],ebx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	eax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	PixelAddr32k		; EBX -> buffer
		load_es_videoSel

		pop		ecx					; Restore dx
		inc		ecx					; ECX := # pixels to draw

		test	[LoSlope],1
		jz		@@HiSlopeLine		; This is a high slope line

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		ax,[USHORT color]	; AX := pixel value to fill

@@LoopLo:
		mov		[_ES ebx],ax		; Set pixel value in buffer
		inc		ebx					; Increment x coordinate
		inc		ebx
		or		edi,edi				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[VertInc]		; increment y
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		ax,[USHORT color]		; AL := pixel value to fill

@@LoopHi:
		mov		[_ES ebx],ax		; Set pixel value in buffer
		add		ebx,[VertInc]		; increment y
		or		edi,edi				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx
		inc		ebx					; Increment x
		loop	@@LoopHi			; Loop for remaining pixels

@@Exit:
		leave_c
		ret32

procend		__SV_line32kLin

;----------------------------------------------------------------------------
; void _SV_line16mLin(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw
;				y1		- Y1 coordinate of line to draw
;				x2		- X2 coordinate of line to draw
;				y2		- Y2 coordinate of line to draw
;				color	- color to draw the line in
;
;----------------------------------------------------------------------------
procstart32	__SV_line16mLin

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	LoSlope:ULONG, VertInc:ULONG, EIncr:ULONG, 			\
				NEIncr:ULONG = LocalSize

		enter_c	LocalSize
		cld

		mov		esi,[DC.bytesperline]	; ESI = Increment for video buffer

		mov		_ax,[x2]
		sub		_ax,[x1]			; _AX := X2 - X1

; Force X1 < X2

		jns		@@X2Greater			; Jump if X2 > X1
		neg		_ax					; _AX := X1 - X2

		mov		_bx,[x2]			; Exchange X1 and X2
		xchg	_bx,[x1]
		mov		[x2],_bx

		mov		_bx,[y2]			; Exchange Y1 and Y2
		xchg	_bx,[y1]
		mov		[y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
		mov		_bx,[y2]
		sub		_bx,[y1]			; _BX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		_bx					; _BX := Y1 - Y2
		neg		esi					; negative increment for buffer

; select appropriate routine for slope of line

@@Y2Greater:
		clrhi16	ax					; EAX := dx
		clrhi16	bx					; EBX := dy
		mov		[VertInc],esi		; save increment
		mov		[LoSlope],1
		cmp		ebx,eax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[LoSlope],0
		xchg	ebx,eax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		ebx,1				; _BX := 2 * dy
		mov		[EIncr],ebx			; EIncr := 2 * dy
		sub		ebx,eax				; d = 2 * dy - dx
		mov		edi,ebx				; EDI := initial decision variable
		sub		ebx,eax
		mov		[NEIncr],ebx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	eax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	PixelAddr16m		; EBX -> buffer
		load_es_videoSel

		pop		ecx					; Restore dx
		inc		ecx					; ECX := # pixels to draw

		test	[LoSlope],1
		jz		@@HiSlopeLine		; This is a high slope line

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		ax,[USHORT color]		; AX := pixel value to fill
		mov		dh,[BYTE color+2]	; DH := top byte of pixel value

@@LoopLo:
		mov		[_ES ebx],al		; Set pixel value in buffer
		mov		[_ES ebx+1],ah		; Set pixel value in buffer
		mov		[_ES ebx+2],dh
		inc		ebx
		inc		ebx
		inc		ebx					; Increment x coordinate

@@DonePixelLo:
		or		edi,edi				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[VertInc]		; increment y
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		ax,[USHORT color]	; AL := pixel value to fill
		mov		dh,[BYTE color+2]	; DH := current bank number

@@LoopHi:
		mov		[_ES ebx],al		; Set pixel value in buffer
		mov		[_ES ebx+1],ah		; Set pixel value in buffer
		mov		[_ES ebx+2],dh

@@DonePixelHi:
		add		ebx,[VertInc]		; increment y
		or		edi,edi				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx
		inc		ebx
		inc		ebx					; Increment x coordinate
		loop	@@LoopHi			; Loop for remaining pixels

@@Exit:
		leave_c
		ret32

procend		__SV_line16mLin

;----------------------------------------------------------------------------
; void _SV_line4GLin(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw
;				y1		- Y1 coordinate of line to draw
;				x2		- X2 coordinate of line to draw
;				y2		- Y2 coordinate of line to draw
;				color	- color to draw the line in
;
;----------------------------------------------------------------------------
procstart32	__SV_line4GLin

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	LoSlope:ULONG, VertInc:ULONG, EIncr:ULONG, 			\
				NEIncr:ULONG = LocalSize

		enter_c	LocalSize
		push	eax
		cld

		mov		esi,[DC.bytesperline]	; ESI = Increment for video buffer

		mov		_ax,[x2]
		sub		_ax,[x1]			; _AX := X2 - X1

; Force X1 < X2

		jns		@@X2Greater			; Jump if X2 > X1
		neg		_ax					; _AX := X1 - X2

		mov		_bx,[x2]			; Exchange X1 and X2
		xchg	_bx,[x1]
		mov		[x2],_bx

		mov		_bx,[y2]			; Exchange Y1 and Y2
		xchg	_bx,[y1]
		mov		[y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
		mov		_bx,[y2]
		sub		_bx,[y1]			; _BX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		_bx					; _BX := Y1 - Y2
		neg		esi					; negative increment for buffer

; select appropriate routine for slope of line

@@Y2Greater:
		clrhi16	ax					; EAX := dx
		clrhi16	bx					; EBX := dy
		mov		[VertInc],esi		; save increment
		mov		[LoSlope],1
		cmp		ebx,eax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[LoSlope],0
		xchg	ebx,eax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		ebx,1				; _BX := 2 * dy
		mov		[EIncr],ebx			; EIncr := 2 * dy
		sub		ebx,eax				; d = 2 * dy - dx
		mov		edi,ebx				; EDI := initial decision variable
		sub		ebx,eax
		mov		[NEIncr],ebx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	eax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	PixelAddr4G			; EBX -> buffer
		load_es_videoSel

		pop		ecx					; Restore dx
		inc		ecx					; ECX := # pixels to draw

		test	[LoSlope],1
		jz		@@HiSlopeLine		; This is a high slope line

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		eax,[color]			; EAX := pixel value to fill

@@LoopLo:
		mov		[_ES ebx],eax		; Set pixel value in buffer
		inc		ebx
		inc		ebx
		inc		ebx
		inc		ebx					; Increment x coordinate
		or		edi,edi				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[VertInc]		; increment y
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		eax,[color]			; EAX := pixel value to fill

@@LoopHi:
		mov		[_ES ebx],eax		; Set pixel value in buffer
		add		ebx,[VertInc]		; increment y
		or		edi,edi				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		edi,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx
		inc		ebx
		inc		ebx
		inc		ebx					; Increment x coordinate
		loop	@@LoopHi			; Loop for remaining pixels

@@Exit:
		pop		eax
		leave_c
		ret32

procend		__SV_line4GLin

;----------------------------------------------------------------------------
; void _SV_setActivePageLin(int which)
;----------------------------------------------------------------------------
; Routine to set the video page for active output.
;
; Entry:        page    - Page number of page to use
;
;----------------------------------------------------------------------------
procstart32	__SV_setActivePageLin

		ARG     which:UINT

		enter_c	0

; Calculate 18 bit address of page in video memory. Note that it is in this
; routine that we add the offset to the linear framebuffer into the
; DC.originOffset variable, to remove the need to load it at runtime.

		mov     _ax,[which]         ; _AX := page number
		clrhi16	ax					; EAX := page number
		mul     [__SV_pagesize]     ; EDX:EAX := result
if flatmodel
		add		eax,[DC.videoMem]
endif
		mov     [DC.originOffset],eax	; Save video buffer offset

		leave_c_nolocal
		ret32

procend     __SV_setActivePageLin

endcodeseg32	_linsdk

endif

		END
