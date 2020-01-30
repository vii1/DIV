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
;* Description:	VGAX, 8 bit VGA ModeX device context routines for the
;*				undocumented VGA 256 color planar modes.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"
include "mgldos\biosdata.equ"

header	_vgax

SC_INDEX 			=	03C4h   	; Sequencer Controller Index
SC_DATA 			=	03C5h   	; Sequencer Controller Data Reg Index
GC_INDEX			=	03CEh		; Graphics Controller Index
GC_DATA				=	03CFh		; Graphics Controller Data Reg Index

struc	VGAX_state_s
v				display_vec_s	<>
oldBIOSMode		ushort			?
old50Lines		bool			?
fastDrawing		bool			?
ends	VGAX_state_s
state	= (VGAX_state_s PTR _ES _BX)

	EXTRN	___EMU__scanLine:FPTR
	EXTRN	___EMU__putImage:FPTR

begdataseg	_vgax

	$EXTRN	__MGL_dc,devCtx_s		; Global current device context
	$EXTRN	_VGAX_leftMask,UCHAR	; Left clipping mask lookup table
	$EXTRN	_VGAX_rightMask,UCHAR	; Right clipping mask lookup table
	$EXTRN	__MGL_mirrorTable,UCHAR	; Byte reversal lookup table

pattern		UCHAR	?				; Pattern scratch global
gcount		UINT	?				; Global counter variable
gcolor		ULONG	?				; Scratch global

enddataseg	_vgax

begcodeseg	_vgax

;----------------------------------------------------------------------------
; VGAX_pixelAddrESBX	Determine buffer address of pixel in VGAX modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				ES:_BX	->  device context
;				_CX		-	x-coordinate
;
; Exit:			ES:_BX	->	offset in buffer
;				_CX		-	Plane number for pixel
;
; Registers:	None
;
;----------------------------------------------------------------------------
procstart	VGAX_pixelAddrESBX

		push	_dx						; Save DX
		push	_cx
		imul	[MGLDC.mi.bytesPerLine]	; AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@NotNegative

; The x coordinate is negative, so adjust to be positive

		mov		_dx,_cx
		neg		_dx
		add		_dx,3
		shr		_dx,2
		sub		_ax,_dx					; Adjust for negative coords
		shl		_dx,2
		add		_cx,_dx					; Make coordinate positive again

@@NotNegative:
		lVidDC	_bx
		shr		_cx,2					; _CX := x/4
		add		_bx,_cx
		add		_bx,_ax
		pop		_cx
		and		_cx,3					; _CX := plane number of pixel
		pop		_dx
		ret

procend		VGAX_pixelAddrESBX

;----------------------------------------------------------------------------
; VGAX_pixelAddr	Determine buffer address of pixel in native VGAX modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				_CX		-	x-coordinate
;
; Exit:			ES:_BX	->	offset in buffer
;				_CX		-	Plane number for pixel
;
; Registers:	None
;
;----------------------------------------------------------------------------
procstart	VGAX_pixelAddr

		push	_dx						; Save DX
		push	_cx
		imul	[GDC.mi.bytesPerLine]	; AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@NotNegative

; The x coordinate is negative, so adjust to be positive

		mov		_dx,_cx
		neg		_dx
		add		_dx,3
		shr		_dx,2
		sub		_ax,_dx					; Adjust for negative coords
		shl		_dx,2
		add		_cx,_dx					; Make coordinate positive again

@@NotNegative:
		lVid	_bx
		shr		_cx,2					; _CX := x/4
		add		_bx,_cx
		add		_bx,_ax
		pop		_cx
		and		_cx,3					; _CX := plane number of pixel
		pop		_dx
		ret

procend		VGAX_pixelAddr

;----------------------------------------------------------------------------
; void VGAX_beginDrawing(void);
;----------------------------------------------------------------------------
; Sets the video card into write mode 0 and read mode 1. Note that we must
; ensure that the video card is still in the byte oriented mode.
;----------------------------------------------------------------------------
procstart	_VGAX_beginDrawing

		push	_ax
		push	_dx

; Configure the graphics controller

		mov     ah,[BYTE GDC.a.writeMode]
		mov		dx,GC_INDEX			; DX := GC address register port
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		mov		ax,04805h			; AL := Mode register number
									; AH := write mode 0, read mode 1, for
									; ModeX
		out		dx,ax

		mov		ax,0007h			; AH := 0 (don't care for all maps;
									;	CPU reads always return 0FFH)
									; AL := 7 (Color Don't care reg number)
		out		dx,ax				; Set up Color Don't care reg

		pop		_dx
		pop		_ax
		ret

procend		_VGAX_beginDrawing

;----------------------------------------------------------------------------
; void VGAX_endDrawing(void);
;----------------------------------------------------------------------------
; Reset the video card into the default BIOS state. We only actually turn
; off the fast drawing mode when the last call has been made to endDrawing.
; This allows nested calls to beginDrawing and endDrawing without any
; performance loss.
;----------------------------------------------------------------------------
procstart	_VGAX_endDrawing

		push	_ax
		push	_dx

; Restore graphics controller and return to caller

		mov		dx,GC_INDEX			; DX := GC address register port
		mov		ax,04005h			; AH := 40h, AL := 5
		out		dx,ax				; Restore default mode register

		mov		ax,0F07h			; AH := 0Fh, AL := 7
		out		dx,ax				; Set up Color Don't care reg

		mov		ax,0003h			; AH := 0, AL := 3
		out		dx,ax				; Restore Data Rotate/Func select reg

		pop		_dx
		pop		_ax
		ret

procend		_VGAX_endDrawing

;----------------------------------------------------------------------------
; void VGAX_beginPixel(MGLDC *dc)
;----------------------------------------------------------------------------
; Routine setup the VGA ModeX for drawing single pixels. This _must_ be
; called before the pixel routine is used, and no other calls can be mode
; in between.
;----------------------------------------------------------------------------
procstart	_VGAX_beginPixel

;		jmp		_VGAX_endDrawing	; Restore to default state
		ret

procend		_VGAX_beginPixel

;----------------------------------------------------------------------------
; void VGAX_endPixel(void);
;----------------------------------------------------------------------------
; Reset the video card into the current controller state after drawing
; a bunch of pixels which will have changed the controllers state.
;----------------------------------------------------------------------------
procstart	_VGAX_endPixel

		ret

procend		_VGAX_endPixel

;----------------------------------------------------------------------------
; color_t VGAX_getPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine reads the value of a pixel in native EGA/VGA graphics modes
;
; Entry:		x		-	X coordinate of pixel to read
;				y		-	Y coordinate of pixel to read
;
; Exit:			DX:AX	-	Color of pixel at (x,y)
;
;----------------------------------------------------------------------------
procstart	_VGAX_getPixel

		ARG		x:UINT, y:UINT

		enter_c	0
		save_es

; Compute the pixel's address in video buffer

		mov		_ax,[y]
		mul		[GDC.mi.bytesPerLine]	; _AX := y * BytesPerLine
		lVid	_di
		mov		_bx,[x]
		mov		cl,bl				; CL := low-order byte of x
		shr		_bx,2				; _BX := x/4
		and		cl,3				; CL := plane mask of pixel
		add     _bx,_di
		add		_bx,_ax				; BX := y*BytesPerLine + x/4

; Select the plane to read data from.

		mov		dx,GC_INDEX			; DX := GC address register port
		mov		ah,cl				; AH := plane to enable
		mov		al,04				; AL := read map select reg number
		out		dx,ax				; Select which plane to read from

; Read the pixel's value

		xor		_ax,_ax
		mov		al,[_ES _bx]		; _AX := pixel value
		xor		_dx,_dx				; _DX:_AX := pixel value

		restore_es
		leave_c_nolocal
		ret

procend		_VGAX_getPixel

;----------------------------------------------------------------------------
; void VGAX_putPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native EGA/VGA graphics modes.
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
;----------------------------------------------------------------------------
procstart	_VGAX_putPixel

		ARG		x:UINT, y:UINT

		enter_c	0
		save_es

; Compute the pixel's address in video buffer

		mov		_si,[UINT GDC.intColor]
		mov		ch,[BYTE GDC.a.writeMode]
		mov		_ax,[y]
		mul		[GDC.mi.bytesPerLine]	; _AX := y * BytesPerLine
		lVid	_di
		mov		_bx,[x]
		mov		cl,bl				; CL := low-order byte of x
		shr		_bx,2				; _BX := x/4
		and		cl,3				; CL := plane mask of pixel
		add     _bx,_di
		add		_bx,_ax				; BX := y*BytesPerLine + x/4

; Select the plane to write data to, so only one pixel is affected.

		mov		dx,SC_DATA			; DX := Sequencer Controller data reg
		mov		al,01h
		shl		al,cl				; AL := plane mask in position
		out		dx,al				; Set mask for one plane only

		mov		_ax,_si
		cmp		ch,MGL_REPLACE_MODE
		jne		@@NotReplace

		mov		[_ES _bx],al		; Replace the pixel
		jmp		@@Exit

; If we are drawing the pixel in AND, OR or XOR mode, we must also
; tell the graphics controller which plane to read data from to accomplish
; the correct logical combination. We could stuff around the with the
; VGA's ALU, but this would be even slower.

@@NotReplace:
		mov		dx,GC_INDEX			; DX := GC address register port
		mov		ah,cl				; AH := plane to enable
		mov		al,04				; AL := read map select reg number
		out		dx,ax				; Select which plane to read from
		mov		_ax,_si				; AL := pixel value

		cmp		ch,MGL_XOR_MODE
		jne		@@2

		xor		[_ES _bx],al		; XOR the pixel
		jmp		@@Exit

@@2:	cmp		ch,MGL_OR_MODE
		jne		@@3

		or		[_ES _bx],al		; OR the pixel
		jmp		@@Exit

@@3:	and		[_ES _bx],al		; AND the pixel

@@Exit:	restore_es
		leave_c_nolocal
		ret

procend		_VGAX_putPixel

;----------------------------------------------------------------------------
; void VGAX_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,
;	int op)
;----------------------------------------------------------------------------
; Reads a scanline of information from the device context into the memory
; buffer.
;
; The write mode operation determines how the data will be combined into the
; memory buffer.
;
; Entry:	dc		- Device context
;			y		- Scanline to get data from
;			x1		- Starting coordinate in scanline
;			y2		- Ending coordinate in scanline
;			buffer	- Pointer to buffer to store data in
;			bufx	- X coordinate in buffer
;			op		- Write mode op to use during transfer
;
;----------------------------------------------------------------------------
procstart	_VGAX_getScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		enter_c	0
		save_ds
		save_es

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	VGAX_pixelAddrESBX	; _ES _BX -> display surface
									; CL := plane # for first pixel
		mov		ah,cl				; AH := plane number for first pixel
		mov		_si,_bx				; ES:_SI -> display surface
		_lds	_di,[buffer]		; DS:_DI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		dx,GC_INDEX
		mov		al,04				; AL := read map select reg number
		out		dx,al
		inc		_dx
		mov		al,ah
		cmp		[op],MGL_REPLACE_MODE
		jne		@@NextPixelROP

@@NextPixel:
		out		dx,al				; Select next plane for this pixel
		mov		ah,[_ES _si]		; AH := pixel value read from buffer
		mov		[_di],ah			; Save pixel value
		inc		al
		and		al,3
		jnz		@@1
		inc		_si					; Increment framebuffer address
@@1:	inc		_di					; Increment storage buffer address
		loop	@@NextPixel			; Loop for all pixels

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@NextPixelROP:
		out		dx,al				; Select next plane for this pixel
		mov		ah,[_ES _si]		; AH := pixel value read from buffer
		cmp		[op],MGL_XOR_MODE
		jne		@@2
		xor		[_di],ah
		jmp		@@DonePixel
@@2:	cmp		[op],MGL_OR_MODE
		jne		@@3
		or		[_di],ah
		jmp		@@DonePixel
@@3:	and		[_di],ah

@@DonePixel:
		inc		al
		and		al,3
		jnz		@@4
		inc		_si					; Increment framebuffer address
@@4:	inc		_di					; Increment storage buffer address
		loop	@@NextPixelROP		; Loop for all pixels
		jmp		@@Exit

procend		_VGAX_getScanLine

;----------------------------------------------------------------------------
; void VGAX_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,
;	int op)
;----------------------------------------------------------------------------
; Dumps a scanline of information to the device context from the memory
; buffer.
;
; The write mode operation determines how the data will be combined into the
; display context.
;
; Entry:	dc		- Device context
;			y		- Scanline to put data to
;			x1		- Starting coordinate in scanline
;			y2		- Ending coordinate in scanline
;			buffer	- Pointer to buffer to get data from
;			bufx	- X coordinate in buffer
;			op		- Write mode op to use during transfer
;
;----------------------------------------------------------------------------
procstart	_VGAX_putScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		LOCAL	srcBuf:UINT, dstBuf:UINT = LocalSize

		enter_c	LocalSize
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	VGAX_pixelAddrESBX	; _ES _BX -> display surface
									; CL := plane # for first pixel
		mov		ah,11h
		shl		ah,cl				; AH := plane mask in position
		mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		dx,SC_DATA			; DX := SC data register port
		mov		al,ah				; AL := plane mask in position
		cmp		[op],MGL_REPLACE_MODE
		jne		@@DoROP

; Force alignment to a 4 pixel boundary for starting pixel

		test	al,0Fh
		jz		@@DoAligned			; Scanline is already aligned

@@NextPixelStart:
		out		dx,al				; Set plane for this pixel
		movsb						; Move this pixel
		dec		_cx
		jz		@@Exit
		rol		al,1
		jc		@@DoAligned
		dec		_di					; Adjust dest buffer back
		jmp		@@NextPixelStart	; Loop for all starting pixels

@@DoAligned:
		cmp		_cx,16
		jle		@@NextPixelEnd		; Dont attempt to unroll the loops
		mov		[srcBuf],_si		; Save buffer offsets
		mov		[dstBuf],_di

@@NextPlane:
		mov		_bx,_cx
		shr		_bx,4				; BL := count of 4 pixel lots to process
		out		dx,al				; Set plane for this pixel
		mov		bh,al				; BH := plane mask value
		mov		_si,[srcBuf]		; Restore buffer offsets
		mov		_di,[dstBuf]

@@NextPixels:
		mov		al,[_si+8]
		mov		ah,[_si+12]
		shl		eax,16
		mov		al,[_si]
		mov		ah,[_si+4]
		mov		[_ES _di],eax		; Store the 4 pixels
		add		_si,16
		add		_di,4
		dec     bl
		jnz		@@NextPixels		; Loop for next 4 pixels

		inc		[srcBuf]
		mov		al,bh
		rol		al,1
		jnc		@@NextPlane			; Process for all 4 planes

		sub		_si,3
		and		_cx,0Fh				; Determine number of last few pixels
		jz		@@Exit

@@NextPixelEnd:
		out		dx,al				; Set plane for this pixel
		movsb						; Move this pixel
		rol		al,1
		cmc
		sbb		_di,0				; Adjust increment back if necessary
		loop    @@NextPixelEnd		; Loop for all ending pixels

@@Exit:	restore_es
		restore_ds
		leave_c
		ret

; Handle ROP code blitting

@@DoROP:

; Setup graphics controller for output

		push	_ax
		mov		dx,GC_INDEX			; DX := GC address register port
		mov		ax,04805h			; AL := Mode register number
									; AH := write mode 0, read mode 1
		out		dx,ax

		mov		ax,0007h			; AH := 0 (don't care for all maps;
									;	CPU reads always return 0FFH)
									; AL := 7 (Color Don't care reg number)
		out		dx,ax				; Set up Color Don't care reg

		mov		ah,[BYTE op]		; Get write mode function into ah
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		pop		_ax
		mov		dx,SC_DATA			; DX := Sequencer Controller data reg

@@NextPixelROP:
		out		dx,al				; Set plane for this pixel
		mov		ah,[_si]
		and		[_ES _di],ah		; Plot the pixel
		inc		_si
		rol		al,1
		adc		_di,0				; Increment buffer address
		loop    @@NextPixelROP		; Loop for all ending pixels

		call	_VGAX_endDrawing
		jmp		@@Exit

procend		_VGAX_putScanLine

;----------------------------------------------------------------------------
; void VGAX_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA ModeX graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Vertical
; lines are special cased (horizontal lines are special cased by the
; scanline rendering routine). The sloping lines are drawn using the
; Midpoint line algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_VGAX_line

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, EIncr:FIX32_T, 			\
				NEIncr:FIX32_T, varDX:FIX32_T, varDY:FIX32_T, x:UINT,	\
				y:UINT = LocalSize

		enter_c	LocalSize
		save_es

		call	_VGAX_beginDrawing

		mov		dx,SC_DATA
		mov		_si,[GDC.mi.bytesPerLine]

; Check for vertical line

		mov		eax,[x2]
		sub		eax,[x1]			; EAX := X2 - X1
		jz		@@VertLine			; jump if vertical line...

; calcluate dy = ABS(Y2-Y1)

		mov		ebx,[y2]
		sub		ebx,[y1]			; EBX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		ebx					; EBX := Y1 - Y2
		neg		_si					; negative increment for buffer

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[vertInc],_si		; save increment
		mov		[routine],offset @@LoSlopeLine
		cmp		ebx,eax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[routine],offset @@HiSlopeLine
		xchg	ebx,eax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		mov		[varDX],eax			; Save dx for later
		mov		[varDY],ebx			; Save dy for later
		shl		ebx,1				; EBX := 2 * dy
		mov		[EIncr],ebx			; EIncr := 2 * dy
		sub		ebx,eax				; d = 2 * dy - dx
		mov		edi,ebx				; EDI := initial decision variable
		sub		ebx,eax
		mov		[NEIncr],ebx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		mov		eax,[y1]
		add		eax,8000h			; Round to integer value
		shr		eax,16
		mov		[y],_ax				; Save initial x coordinate
		mov		ecx,[x1]
		add		ecx,8000h			; Round to integer value
		shr		ecx,16
		mov		[x],_cx				; Save initial y coordinate
		call	VGAX_pixelAddr		; ES:_BX -> buffer
									; CL := plane # for first pixel1

		mov		al,11h
		rol		al,cl				; AL := bit plane mask in position
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		_cx					; _CX := # pixels to draw

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for vertical lines
;
;****************************************************************************

@@VertLine:
		mov		eax,[y1]
		add		eax,8000h
		shr		eax,16				; _AX := Y1
		mov		ebx,[y2]
		add		ebx,8000h
		shr		ebx,16				; _BX := Y2
		mov		_cx,_bx
		sub		_cx,_ax				; _CX := dy
		jge		@@PosDy				; Jump if dy >= 0

		neg		_cx					; force dy >= 0
		mov		_ax,_bx				; _AX := Y2

@@PosDy:
		inc		_cx					; _CX := # of pixels to draw
		mov		_di,_cx				; preserve this reg
		mov		ecx,[x1]
		add		ecx,8000h
		shr		ecx,16				; _CX := x
		call	VGAX_pixelAddr		; ES:_BX -> buffer
									; CL := plane number for first pixel

; Select the plane to write data to

		mov		al,01h
		shl		al,cl				; AL := plane mask in position
		out		dx,al				; Set mask for one plane only

		mov		al,[BYTE GDC.intColor]; AL := pixel value
		mov		_cx,_di				; restore reg
		cmp		[BYTE GDC.a.writeMode],MGL_REPLACE_MODE
		jne     @@VertLoopSlow

@@VertLoop:
		mov		[_ES _bx],al		; Set pixel value in buffer
		add		_bx,_si
		dec		_cx
		jnz		@@VertLoop			; loop down the line
		jmp		@@Exit

@@VertLoopSlow:
		and		[_ES _bx],al		; Set pixel value in buffer (XOR, OR etc)
		add		_bx,_si
		dec		_cx
		jnz		@@VertLoopSlow		; loop down the line
		jmp		@@Exit

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:_BX -> video buffer
;						AL  = plane mask for 1st pixel
;						ECX = # pixels to draw
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		push	_ax
		mov		eax,[varDY]
		shl		eax,1
		mov     _dx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (x - x1)
		add		edi,eax				; EDI += 2 * dy * (x - x1)
		mov		eax,[varDX]
		shl		eax,1
		mov		_dx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (y - y1)
		mov		_dx,[VertInc]
		or		_dx,_dx
		js      @@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

@@LoDoneAdjust:
		pop		_ax
		mov		ah,al				; AH := bit plane mask in position
		mov		dx,SC_DATA
		mov		esi,[EIncr]
		cmp		[BYTE GDC.a.writeMode],MGL_REPLACE_MODE
		jne		@@StartLoSlow

@@StartLo:
		mov		al,ah				; AL := plane mask for first pixel

@@PlaneMaskIn:
		or		al,ah				; Collect current bit mask postion
		rol		ah,1				; Rotate plane mask value
		jc		@@PlaneMaskOut		; Jump if done 4 pixels

; Plane mask has not shifted out

		or		edi,edi				; Test sign of d
		jg		@@InPosDi			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@PlaneMaskIn		; Loop for remaining pixels

		out		dx,al				; Set the plane mask
		mov		al,[BYTE GDC.intColor]
		mov		[_ES _bx],al		; Set remaining pixels
		jmp		@@Exit				; We are all done

@@InPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr

		out		dx,al				; Set the plane mask
		mov		al,[BYTE GDC.intColor]
		mov		[_ES _bx],al		; Update pixels

		add		_bx,[VertInc]		; increment y
		loop	@@StartLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

; Plane mask has shifted out

@@PlaneMaskOut:
		out		dx,al				; Set the plane mask
		mov		al,[BYTE GDC.intColor]
		mov		[_ES _bx],al		; Set pixels in buffer
		inc		_bx					; Increment buffer offset coordinate

		or		edi,edi				; Test sign of d
		jg		@@OutPosDi			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@StartLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@OutPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		_bx,[VertInc]		; increment y
		loop	@@StartLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@StartLoSlow:
		mov		al,ah				; AL := plane mask for first pixel

@@PlaneMaskInSlow:
		or		al,ah				; Collect current bit mask postion
		rol		ah,1				; Rotate plane mask value
		jc		@@PlaneMaskOutSlow	; Jump if done 4 pixels

; Plane mask has not shifted out

		or		edi,edi				; Test sign of d
		jg		@@InPosDiSlow		; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@PlaneMaskInSlow	; Loop for remaining pixels

		out		dx,al				; Set the plane mask
		mov		al,[BYTE GDC.intColor]
		and		[_ES _bx],al		; Set remaining pixels
		jmp		@@Exit				; We are all done

@@InPosDiSlow:
		add		edi,[NEIncr]		; d := d + NEIncr

		out		dx,al				; Set the plane mask
		mov		al,[BYTE GDC.intColor]
		and		[_ES _bx],al		; Update pixels

		add		_bx,[VertInc]		; increment y
		loop	@@StartLoSlow		; Loop for remaining pixels
		jmp		@@Exit				; We are all done

; Plane mask has shifted out

@@PlaneMaskOutSlow:
		out		dx,al				; Set the plane mask
		mov		al,[BYTE GDC.intColor]
		and		[_ES _bx],al		; Set pixels in buffer
		inc		_bx					; Increment buffer offset coordinate

		or		edi,edi				; Test sign of d
		jg		@@OutPosDiSlow		; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@StartLoSlow		; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@OutPosDiSlow:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		_bx,[VertInc]		; increment y
		loop	@@StartLoSlow		; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:_BX -> video buffer
;						AL  = plane mask for 1st pixel
;						ECX = # pixels to draw
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		push	_ax
		mov		eax,[varDX]
		shl		eax,1
		mov     _dx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (x - x1)
		sub		edi,eax				; EDI -= 2 * dx * (x - x1)
		mov		eax,[varDY]
		shl		eax,1
		mov		_dx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (y - y1)
		mov		_dx,[VertInc]
		or		_dx,_dx
		js      @@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

@@HiDoneAdjust:
		pop		_ax
		mov		dx,SC_DATA
		mov		esi,[EIncr]

		mov		ah,[BYTE GDC.intColor]; AH := pixel value to fill
		cmp		[BYTE GDC.a.writeMode],MGL_REPLACE_MODE
		jne		@@LoopHiSlow

@@LoopHi:
		out		dx,al				; Set the plane mask for pixel

@@SetHi:
		mov		[_ES _bx],ah		; Set pixel value in buffer
		add		_bx,[VertInc]		; increment y

		or		edi,edi				; Test sign of d
		jg		@@HiPosDi			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@SetHi				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		rol		al,1
		adc		_bx,0				; Increment buffer offset
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit

@@LoopHiSlow:
		out		dx,al				; Set the plane mask for pixel

@@SetHiSlow:
		and		[_ES _bx],ah			; Set pixel value in buffer
		add		_bx,[VertInc]		; increment y

		or		edi,edi				; Test sign of d
		jg		@@HiPosDiSlow		; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@SetHiSlow			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDiSlow:
		add		edi,[NEIncr]		; d := d + NEIncr
		rol		al,1
		adc		_bx,0				; Increment buffer offset
		loop	@@LoopHiSlow		; Loop for remaining pixels
		jmp		@@Exit

@@Exit:	restore_es
		call	_VGAX_endDrawing
		leave_c
		ret

procend		_VGAX_line

;----------------------------------------------------------------------------
; void VGAX_scanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
; Routine draws a scan line in native VGA ModeX graphics modes. This
; routine will work for all types of scanlines, patterned, XOR mode etc.
;
; Note: This routine will reverse the X coordinates for each scanline if
;		necessary to make x1 < x1, and will scan convert all pixels
;		in the range [x1,x2), so x2 will not be scan converted.
;		We also ignore any scanline where xstart == xend.
;
; Entry:		y		- y coordinate of scan line to draw
;				x1		- X1 coordinate of scan line to draw
;				x2		- X2 coordinate of scan line to draw
;
;----------------------------------------------------------------------------
procstart	_VGAX_scanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		enter_c	0
		save_es
		cld							; String instructions go up

		call	_VGAX_beginDrawing

		cmp     [GDC.a.penStyle],MGL_PIXMAP
		je		@@PixmapScanline

		mov		_ax,[x2]
		cmp		[x1],_ax
		je		@@Exit				; Quit if xstart == xend
		jl		@@NoSwap			; no, don't swap them

		xchg	[x1],_ax			; Swap the coordinates
		mov		[x2],_ax

@@NoSwap:
		dec		[x2]				; Decrement xend coordinate
		mov		dx,SC_DATA			; DX := Sequencer Controller Data Reg
		cmp		[GDC.a.penStyle],MGL_BITMAP_SOLID
		je      @@SkipPattern		; Skip pattern loading step

; Lookup the pattern byte for this scanline and reverse it

		mov		_si,[y]
		xor		_ax,_ax
		and		_si,7				; Mask to 0-7
		mov		al,[_si+GDC.a.penPat]
		mov		_si,_ax
		mov		al,[_si+__MGL_mirrorTable] ; Reverse the pattern byte
		mov		cl,[BYTE x1]
		and		cl,4
		rol		al,cl				; Move into correct pos for first pixel
		mov		[pattern],al		; Store in pattern byte

; Compute address of scan line in video buffer

@@SkipPattern:
		mov		_ax,[y]
		mov		_cx,[x1]
		call	VGAX_pixelAddr		; ES:_BX -> buffer
									; CL := plane # for pixel
		mov		_di,_bx				; ES:_DI -> buffer start

; Compute the left and right plane clipping masks for the line

		mov		_si,_cx				; SI := plane number for x1
		mov		bh,[_VGAX_leftMask+_si]; BH := left clipping mask
		mov		_si,[x2]
		and		_si,3				; _SI := plane number for x2
		mov		bl,[_VGAX_rightMask+_si]; BL := right clipping mask

; Determine the number of pixels in the scanline

		mov		_cx,[x2]			; _CX := X2
		mov		_ax,[x1]			; _AX := X1
		and		_ax,not 011b
		sub		_cx,_ax
		shr		_cx,2				; CX := (# bytes in line)-1
		jnz		@@MasksSet			; There's more than one byte to fill

		and		bh,bl				; There is only one byte, so combine
									; the left and right clipping masks
@@MasksSet:
		mov		al,[BYTE GDC.a.penStyle]
		cmp		al,MGL_BITMAP_TRANSPARENT
		je		@@DoTransparent

		mov		ah,[BYTE GDC.intColor]	; AH := pixel value
		cmp		al,MGL_BITMAP_SOLID
		je		@@SolidFill

		mov		ah,[BYTE GDC.intBackColor]	; AH := background color

;----------------------------------------------------------------------------
; Draw a solid patterned line
;
;				BL,BH	= Left and right plane clipping masks
;				_CX		= Number of address to fill - 1
;				DX		= Sequencer controller data reg
;               ES:_DI	-> first pixel in video buffer
;
;----------------------------------------------------------------------------

@@SolidFill:
		mov		al,ah
		mov		[WORD gcolor],ax
		mov		[WORD gcolor+2],ax	; Save color value
		push	_cx					; Save these for later
		push	_di

		mov		al,bh				; AL := left clip mask
		out		dx,al				; Set the left edge clip mask

		cmp		[BYTE GDC.a.writeMode],MGL_REPLACE_MODE
		jne		@@SlowSolidFill		; Do a slower fill for XOR etc...

		mov		al,ah				; AL := color value to program
		stosb						; Draw the left edge
		dec		_cx					; Count of this byte
		js		@@DoneSolid			; Thats the only byte
		jz		@@DoRightEdge		; There are only two bytes

		mov		al,0Fh				; Middle addresses are drawn 4 each
		out		dx,al				; Set the middle pixels plane mask
		mov		al,ah				; AL := color value to program

@@ForceAlignment:
		test	_di,3
		jz		@@Aligned
		mov		[_ES _di],al
		inc		_di
		dec		_cx
		jnz		@@ForceAlignment

@@Aligned:
		mov		eax,[gcolor]
		push	_cx
		shr		_cx,2
	rep	stosd						; Store all middle DWORD's fast!
		pop		_cx
		and		_cx,3
	rep	stosb						; Store the last bytes if any

@@DoRightEdge:
		mov		al,bl				; AL := right clip mask
		out		dx,al				; Set the right clip mask
		mov		al,ah				; AL := color value
		mov		[_ES _di],al		; Draw the right edge

@@DoneSolid:
		pop		_di					; Restore these
		pop		_cx

		cmp		[BYTE GDC.a.penStyle],MGL_BITMAP_OPAQUE
		jne		@@Exit
		jmp		@@DoTransparent

; Slower routine for XOR, AND, OR mode.

@@SlowSolidFill:
		and		[_ES _di],ah			; Draw the left edge
		inc		_di
		dec		_cx						; Count of this byte
		js		@@DoneSolid				; Thats the only byte
		jz		@@DoRightEdgeSlow		; There are only two bytes

		mov		al,0Fh					; Middle addresses are drawn 4 each
		out		dx,al					; Set the middle pixels plane mask

@@LoopSolid:
		and		[_ES _di],ah			; Fill 4 pixels at a time
		inc		_di
		dec		_cx
		jnz		@@LoopSolid

@@DoRightEdgeSlow:
		mov		al,bl					; AL := right clip mask
		out		dx,al					; Set the right clip mask
		and		[_ES _di],ah			; Draw the right edge
		jmp		@@DoneSolid

;----------------------------------------------------------------------------
; Draw a transparent patterned line
;
;				BL,BH	= Left and right plane clipping masks
;				_CX		= Number of address to fill - 1
;				DX		= Sequencer controller data reg
;               ES:_DI	-> first pixel in video buffer
;
;----------------------------------------------------------------------------

@@DoTransparent:
		mov		ah,[pattern]		; AH := pattern byte
		mov		al,bh				; AL := left clip mask
		and		al,ah				; AL := left clip mask masked to pattern
		out		dx,al				; Set the left edge clip mask
		mov		bh,ah				; BH := plane mask for second 4 pixels
		rol		ah,4				; AH := plane mask for first 4 pixels
		mov		al,[BYTE GDC.intColor]

		cmp		[BYTE GDC.a.writeMode],MGL_REPLACE_MODE
		jne		@@SlowTransFill		; Do a slower fill for XOR etc...

		mov		[_ES _di],al		; Draw the left edge
		inc		_di
		dec		_cx					; Count of this byte
		js		@@Exit				; Thats the only byte
		push	_bx
		jz		@@DoRightEdgeTrans	; There are only two bytes

		mov		bl,[BYTE GDC.intColor]; BL := color for pixel

@@LoopTrans:
		mov		al,ah
		out		dx,al				; Set the plane mask for first 4
		xchg	ah,bh				; Swap plane masks
		mov		[_ES _di],bl		; Set the pixels
		inc		_di					; Increment x
		dec		_cx					; Count off this byte
		jz		@@DoRightEdgeTrans	; We are all done

		mov		al,ah
		out		dx,al				; Set the plane mask for second 4
		xchg	ah,bh				; Swap plane masks
		mov		[_ES _di],bl		; Set the pixels
		inc		_di					; Increment x
		dec		_cx
		jnz		@@LoopTrans			; Loop for all pixels

@@DoRightEdgeTrans:
		pop		_bx

		mov		al,bl				; AL := right clip mask
		and		al,ah				; Mask to pattern byte
		out		dx,al				; Set the right clip mask
		mov		al,[BYTE GDC.intColor]
		mov		[_ES _di],al		; Draw the right edge
		jmp		@@Exit

@@SlowTransFill:
		and		[_ES _di],al		; Draw the left edge
		inc		_di
		dec		_cx					; Count of this byte
		js		@@Exit				; Thats the only byte
		push	_bx
		jz		@@DoRightEdgeSlowTrans	; There are only two bytes

		mov		bl,[BYTE GDC.intColor]	; BL := color for pixel

@@LoopSlowTrans:
		mov		al,ah
		out		dx,al				; Set the plane mask for first 4
		xchg	ah,bh				; Swap plane masks
		and		[_ES _di],bl		; Set the pixels
		inc		_di					; Increment x
		dec		_cx					; Count off this byte
		jz		@@DoRightEdgeSlowTrans	; We are all done

		mov		al,ah
		out		dx,al				; Set the plane mask for second 4
		xchg	ah,bh				; Swap plane masks
		and		[_ES _di],bl		; Set the pixels
		inc		_di					; Increment x
		dec		_cx
		jnz		@@LoopSlowTrans		; Loop for all pixels

@@DoRightEdgeSlowTrans:
		pop		_bx

		mov		al,bl				; AL := right clip mask
		and		al,ah				; Mask to pattern byte
		out		dx,al				; Set the right clip mask
		mov		al,[BYTE GDC.intColor]
		and		[_ES _di],al		; Draw the right edge

@@Exit:	restore_es
		call	_VGAX_endDrawing
		leave_c_nolocal
		ret

;----------------------------------------------------------------------------
; Draw a pixmap pattern scanline. For now we simply call the high level
; scanline emulation routine which will call putPixel to handle this. Not
; very fast but it works.

@@PixmapScanline:
		push	[x2]
		push	[x1]
		push	[y]
		call	___EMU__scanLine
		_add	sp,6,12
		jmp		@@Exit

procend		_VGAX_scanLine

;----------------------------------------------------------------------------
; void VGAX_fillRect(int x1,int y1,int x2,int y2)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in native EGA/VGA graphics modes.
;
; Entry:		x1		- X1 coordinate of rectangle to draw
;				y1		- Y1 coordinate of rectangle to draw
;				x2		- X2 coordinate of scan line to draw
;				y2		- Y2 coordinate of rectangle to draw
;
; NOTE: We assume that x1 < x2 AND y1 < y2 at all times. This is ensured by
;		the high level interface code of the library, so we do not bother
;		to check this again. Note also that this routine includes the
;		right and bottom edges of the rectangle.
;
; This routine scan converts only solid filled rectangles. To fill
; rectangles in transparent and opaque bitmap patterns and pixmap patterns,
; we call the scanLine routine.
;----------------------------------------------------------------------------
procstart	_VGAX_fillRect

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT

		enter_c	0
		save_es
		push	_bp
		cld

		cmp		[GDC.a.writemode],MGL_REPLACE_MODE
		jne		@@ScanLineFill
		cmp		[GDC.a.penStyle],MGL_BITMAP_SOLID
		je		@@FastFill

;---------------------------------------------------------------------------
; If we are filling with anything but a solid pattern, then repeatedly
; call the scanline routine to draw the rectangle.

@@ScanlineFill:
; TODO!!
;		call	[GDC.r.beginDrawing]
		mov		_si,[y1]
		mov		_di,[y2]

@@ScanLoop:
		push	[x2]
		push	[x1]
		push	_si
		call	[GDC.r.cur.scanLine]
		_add	sp,6,12

		inc		_si
		cmp		_si,_di
		jl		@@ScanLoop

; TODO!!
;		call	[GDC.r.endDrawing]
		jmp		@@Exit

;---------------------------------------------------------------------------
; We are filling with a solid pattern, so do it as quickly as possible

@@FastFill:
		mov		dx,SC_DATA			; DX := Sequencer Controller Data Reg
		mov		_ax,[y1]
		mov		_cx,[x1]
		call	VGAX_pixelAddr		; _ES _BX -> buffer
									; CL := plane number for first pixel
		mov		_di,_bx				; ES:_DI -> buffer start

; Compute the left and right plane clipping masks for the line

		xor		ch,ch
		mov		_si,_cx				; SI := plane number for x1
		mov		bh,[_VGAX_leftMask+_si]; BH := left clipping mask
		mov		_si,[x2]
		dec		_si					; Adjust righ coordinate
		and		_si,3				; SI := plane number for x2
		mov		bl,[_VGAX_rightMask+_si]; BL := right clipping mask

; Compute the number of addresses to be filled for the scanline

		mov		_cx,[x2]
		mov		_si,[x1]
		cmp		_cx,_si
		jle		@@Exit				; Skip if 0 or negative width
		dec		_cx					; Adjust end coordinate
		and		_si,not 011b
		sub		_cx,_si
		shr		_cx,2				; _CX := # address to fill - 1
		jnz		@@MasksSet			; There's more than one byte to fill

		and		bh,bl				; There is only one byte, so combine
									; the left and right clipping masks
@@MasksSet:
		mov		_si,[y2]
		sub		_si,[y1]  			; _SI := height of rectangle
		jle		@@Exit				; Skip if 0 or negative height

		mov		al,[BYTE GDC.intColor]; AH := color to fill with
		mov		ah,al
		mov		[WORD gcolor],ax
		mov		[WORD gcolor+2],ax	; Save color value
		mov		_bp,[GDC.mi.bytesPerLine]; Stack frame isn't needed any more
		sub		_bp,_cx
		dec		_bp      			; _BP := buffer increment per scanline

@@NextRow:
		push	_cx      			; Save width of rectangle

		mov		al,bh				; AL := left clip mask
		out		dx,al				; Set the left edge clip mask
		mov		[_ES _di],ah
		inc		_di					; Draw the left edge
		dec		_cx					; Count of this byte
		js		@@LoopBottom		; Thats the only byte
		jz		@@DoRightEdge		; There are only two bytes

		mov		al,0Fh				; Middle addresses are drawn 4 each
		out		dx,al				; Set the middle pixels plane mask

@@ForceAlignment:
		test	_di,3
		jz		@@Aligned
		mov		[_ES _di],ah
		inc		_di
		dec		_cx
		jnz		@@ForceAlignment

@@Aligned:
		mov		eax,[gcolor]
		push	_cx
		shr		_cx,2
	rep	stosd						; Store all middle DWORD's fast!
		pop		_cx
		and		_cx,3
	rep	stosb						; Store the last bytes if any

@@DoRightEdge:
		mov		al,bl				; AL := right clip mask
		out		dx,al				; Set the right clip mask
		mov		[_ES _di],ah
		inc		_di					; Draw the right edge

@@LoopBottom:
		add		_di,_bp				; ES:_DI -> next scanline
		pop		_cx      			; Restore scanline width
		dec		_si      			; Count off this scanline
		jnz		@@NextRow			; loop for all rows

@@Exit:	pop		_bp
		restore_es
		leave_c_nolocal
		ret

procend		_VGAX_fillRect

;----------------------------------------------------------------------------
; void VGAX_putMonoImage(MGLDC *dc,int x,int y,int width,int height,
;   uchar *image)
;----------------------------------------------------------------------------
; Routine to blit a monochromatic image from system memory into display
; memory, setting all 1 bits to the foreground color and leaving all
; zero bits the same. The image is drawn on the currently active display
; page, and will be clipped to the current clipping rectangle.
;
; Entry:		dc		- Device context
;				x		- X coordinate to place image at
;				y		- Y coordinate to place image at
;				width	- Width of the image in bytes
;				height	- Height of the image
;				image	- Pointer to the image data
;
;----------------------------------------------------------------------------
procstart	_VGAX_putMonoImage

		ARG		dc:DPTR, x:UINT, y:UINT, byteWidth:UINT, height:UINT,		\
				image:DPTR

		LOCAL	lmask:S_UCHAR, rmask:S_UCHAR, bytesPerLine:UINT, 				\
				cleft:UINT, ctop:UINT, cright:UINT, cbottom:UINT,			\
				rowWidth:UINT, rowCount:UINT, rowAdjust:UINT,				\
				count:UINT, writeMode:UINT, color:S_UCHAR = LocalSize

		enter_c	LocalSize
		save_es
		cld

; Copy variables from device context onto local stack space

		_les	_bx,[dc]
		mov		_ax,[MGLDC.mi.bytesPerLine]
		mov     [bytesPerLine],_ax
		mov     _ax,[MGLDC.intClipRect.left]
		mov		[cleft],_ax
		mov     _ax,[MGLDC.intClipRect.top]
		mov		[ctop],_ax
		mov     _ax,[MGLDC.intClipRect.right]
		mov		[cright],_ax
		mov     _ax,[MGLDC.intClipRect.bottom]
		mov		[cbottom],_ax
		mov		al,[BYTE MGLDC.intColor]
		mov		[color],al

; Configure the graphics controller

		mov		ah,[BYTE MGLDC.a.writeMode] ; Get write mode function in AH
		mov		[BYTE writeMode],ah
		mov		dx,GC_INDEX			; DX := GC address register port
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		mov		ax,04805h			; AL := Mode register number
									; AH := write mode 0, read mode 1, for
									; ModeX
		out		dx,ax

		mov		ax,0007h			; AH := 0 (don't care for all maps;
									;	CPU reads always return 0FFH)
									; AL := 7 (Color Don't care reg number)
		out		dx,ax				; Set up Color Don't care reg

; Check to see if the image needs clipping. If it does, then use the
; clipping routine to draw it. Otherwise use the faster non-clipping routine.

		mov		_ax,[y]				; AX := y
		mov		_bx,[x]				; BX := x
		mov		_cx,[byteWidth]
		mov		[rowWidth],_cx
		shl		_cx,3				; CX := width of image in pixels

		cmp		_ax,[ctop]
		jl		@@ClipImage
		mov		_dx,_ax
		add		_dx,[height]
		cmp		_dx,[cbottom]
		jge		@@ClipImage
		cmp		_bx,[cleft]
		jl		@@ClipImage
		mov		_dx,_bx
		add		_dx,_cx
		cmp		_dx,[cright]
		jge		@@ClipImage
		cmp		[BYTE writeMode],MGL_REPLACE_MODE
		jne		@@ClipImage
		jmp		@@DrawNonClipped

;----------------------------------------------------------------------------
; Routine for clipped monochrome images (including XOR,OR etc)
;
;		_AX	- y coordinate
;		_BX - x coordinate
;		_CX - width of image in pixels

@@ClipImage:
		mov		_si,[UINT image]	; _SI -> offset of image data
		mov		_dx,[height]		; DX := height of image in scanlines
		dec		_dx

; Clip the y coordinates of the image.

		cmp		_ax,[ctop]			; Less than top coordinate?
		jge		@@TopOK				; No, top is ok.

; Clip the top of the image. If the image is entirely off the clip
; area, clip it entirely.

		add		_ax,_dx				; Adjust to bottom of image
		sub		_ax,[ctop]			; Clip bottom edge against top
		js		@@Exit				; Entire image is clipped

		sub		_ax,_dx				; Adjust to top of image again
		add		_dx,_ax				; Adjust increment to bottom of image
		neg		_ax
		mul		[BYTE byteWidth]
		cwde
		add		_si,_ax				; Increment starting row in image
		mov		_ax,[ctop]			; Start at top coordinate of clip area

@@TopOK:
		mov		_bx,_ax				; BX := top coordinate
		add		_bx,_dx				; Adjust to bottom of image
		cmp		_bx,[cbottom]		; Greater than bottom coordinate?
		jl		@@BottomOK			; No, bottom is ok.

; Clip the bottom of the image. If the image is entirely off the
; clip area, clip it entirely.

		mov		_bx,_ax				; Get top of image
		sub		_bx,[cbottom]		; Clip top edge against bottom
		jge		@@Exit				; Entire image is clipped

		add		_bx,_dx
		sub		_dx,_bx				; Adjust number of rows to process
		dec		_dx

@@BottomOK:
		inc		_dx					; DX := number of rows to process
		mov		[rowCount],_dx		; Save row count for later
		mov		[y],_ax				; Save y coordinate
		mov		al,0FFh
		mov		[lmask],al			; Default left clipping mask
		mov		[rmask],al			; Default right clipping mask

		mov		_dx,_cx				; DX := width of image in pixels
		dec		_dx
		mov		_bx,[x]				; BX := x
		cmp		_bx,[cleft]			; Less than left coordinate?
		jge		@@LeftOK			; No, left is ok.

; Clip the left of the image. If the image is entirely off the clip
; area, clip it entirely.

		add		_bx,_dx				; Adjust to right edge of character
		cmp		_bx,[cleft]			; Clip right edge against left edge
		js		@@Exit				; Entire image is clipped

; Build a clipping mask for the left byte of the image

		sub		_bx,_dx				; Adjust back to left edge again
		mov		_cx,[cleft]
		sub		_cx,_bx				; CX := cleft - left
		mov		_ax,_cx
		shr		_ax,3				; AX := byte offset into row
		sub		[rowWidth],_ax		; Adjust count of bytes to process
		add		_si,_ax				; Adjust offset to first byte
		shl		_ax,3				; AX := pixel offset into row
		add		[x],_ax				; Adjust first pixel position
		mov		al,0FFh
		and		cl,7				; CL := bit offset into byte
		shr		al,cl				; AL := clipping mask in proper position
		mov		[lmask],al			; Save left clipping mask

@@LeftOK:
		mov		_ax,_bx
		add		_ax,_dx				; Adjust to right of image
		cmp		_ax,[cright]		; Greater than right edge?
		jl		@@RightOK			; No, right is ok.

; Clip the right of the image. If the image is entirely off the clip
; area, clip it entirely.

		mov		_ax,_bx				; Get left of image
		cmp		_ax,[cright]		; Clip left edge against right edge
		jge		@@Exit				; Entire image is clipped

; Build a clipping mask for the right byte of the image

		add		_bx,_dx				; BX := right of character
		mov		_cx,_bx
		sub		_cx,[cright]		; CX := right - cright
		inc		_cx
		mov		_ax,_cx
		shr		_ax,3				; AX := byte offset from end of row
		sub		[rowWidth],_ax		; Adjust count of bytes to process
		mov		al,0FFh
		and		cl,7				; CL := bit offset into byte
		shl		al,cl				; AL := clipping mask for proper position
		and		[rmask],al			; Save right clipping mask

; Draw the image

@@RightOK:
		mov		[UINT image],_si	; Save offset into image buffer
		mov		_ax,[y]
		mov		_cx,[x]
		_les	_bx,[dc]
		call	VGAX_pixelAddrESBX	; _ES _BX -> buffer
									; CL := plane # of first pixel
		mov		_di,_bx				; _ES _DI -> video buffer
		_lfs	_si,[image]			; FS:_SI -> image buffer

		mov		_ax,[rowWidth]
		mov		[count],_ax			; Save count of bytes per row
		sub		_ax,[byteWidth]
		neg		_ax
		mov		[rowAdjust],_ax		; Save adjustment between rows
		mov		dx,SC_DATA
		mov		ah,[BYTE color]		; AH := pixel color value
		mov		al,11h
		rol		al,cl				; AL := bit plane mask in position
		mov		cl,al				; CL := plane masks (both nybbles)
		cmp		[BYTE count],1
		jne		@@NextRowClipped

		mov		al,[rmask]			; Combine left and right masks if clipped
		and		al,[lmask]			;  image is only 1 byte wide
		mov		[lmask],al
		mov		[rmask],al

@@NextRowClipped:
		push	_cx					; Save pixel plane masks
		push	_di					; Save buffer offset
		mov		bh,[BYTE count]		; BH := count of bytes to draw

; Load the first byte and mask with left clipping mask

		mov		al,[_FS _si]
		inc		_si
		and		al,[lmask]
		dec		bh					; Decrement count of bytes to do
		jmp		@@PlotByte			; Go plot it

@@NextByteClipped:
		mov		al,[_FS _si]		; AL := pattern for next 8 pixels
		inc		_si

@@PlotByte:
		mov		ch,8
@@Next:	shl		al,1
		jnc		@@Skip
		xchg	al,cl				; AL := plane mask number
		out		dx,al				; Set the plane mask
		xchg	al,cl
		and		[_ES _di],ah		; Plot this pixel
@@Skip:	rol		cl,1				; Rotate plane mask
		adc		_di,0				; Adjust buffer offset
		dec		ch
		jnz		@@Next				; Loop for all pixels in byte

		dec		bh
		js		@@DoneRow			; Negative, so all done
		jnz		@@NextByteClipped	; Not zero, so continue

; Load the last byte for the row and mask with right clipping mask

		mov		al,[_FS _si]
		inc		_si
		and		al,[rmask]
		dec		bh					; Decrement count of bytes to do
		jmp		@@PlotByte			; Go plot it

@@DoneRow:
		pop		_di					; Restore buffer offset
		pop		_cx
		add		_di,[bytesPerLine]	; increment to next line in video buffer
		add		_si,[rowAdjust]		; increment to next line in image
		dec		[rowCount]
		jnz		@@NextRowClipped
		jmp		@@Exit

;----------------------------------------------------------------------------
; Routine for non-clipped monochrome images. We can simply blast out the
; pixels without worrying about masking off the last few pixels in each
; line.
;
;		_AX	- y coordinate
;		_BX - x coordinate
;		_CX - width of image in pixels

@@DrawNonClipped:
		mov		_cx,_bx
		_les	_bx,[dc]
		call	VGAX_pixelAddrESBX	; _ES _BX -> buffer
									; CL := plane # for first pixel
		mov		_di,_bx				; _ES _DI -> video buffer
		_lfs	_si,[image]			; FS:_SI -> image bit map
		mov		_ax,[bytesPerLine]
		sub		_ax,[byteWidth]
		sub		_ax,[byteWidth]		; _AX := buffer adjustment to next row
		mov		[rowAdjust],_ax		; Save buffer adjustment for later
		mov		dx,SC_DATA			; DX := Sequencer Controller Data reg

; Select output routine depending on whether image is byte-aligned

		mov		_bx,offset __MGL_mirrorTable	; DS:_BX -> byte reversal lookup table
		or		cl,cl				; test if byte aligned
		jz		@@DoAligned			; Jump if image is byte aligned

; Routine for non-byte aligned images.

		mov		ch,[BYTE color]		; CH := foreground color

@@StartNonAligned:
		push	[byteWidth]

; Plot the first partial 4 pixels from the first byte to be loaded

		mov		al,[_FS _si]
		xlat	[BYTE _bx]
		shl		al,cl				; Shift to correct position
		out		dx,al				; Set the plane mask
		mov		[_ES _di],ch
		inc		_di
		xor		cl,3				; Adjust CL for middle section
		inc		cl
		dec		[byteWidth]
		jz		@@DoEnd				; Jump if only 1 byte in image row

@@DoMiddle:
		mov		ax,[_FS _si]		; _AX := pattern for next 8 pixels
		inc		_si					; Increment to next byte
		xlat	[BYTE _bx]			; Reverse the bit pattern
		xchg	al,ah
		xlat	[BYTE _bx]			; AX := reversed bit pattern
		xchg	al,ah
		shr		_ax,cl				; AL := pixels to set
		out		dx,al				; Update plane mask register
		mov		[_ES _di],ch		; Update first byte
		inc		_di
		rol		al,4				; Rotate plane mask 4 bits
		out		dx,al				; Update plane mask register
		mov		[_ES _di],ch		; Update second byte
		inc		_di
		dec		[byteWidth]
		jnz		@@DoMiddle			; Loop for all bytes in middle

; Plot the last possible 8 pixels in the row

@@DoEnd:
		mov		al,[_FS _si]		; AL := pattern for last 8 pixels
		inc		_si
		xlat	[BYTE _bx]			; Reverse the bit pattern
		shr		al,cl				; AL := pixels to set
		dec		cl
		xor		cl,3
		out		dx,al				; Update plane mask register
		mov		[_ES _di],ch		; Update first byte
		inc		di
		rol		al,4				; Rotate plane mask 4 bits
		out		dx,al				; Update plane mask register
		mov		[_ES _di],ch		; Update second byte

		pop		[byteWidth]			; Restore byte count
		add		_di,[rowAdjust]		; increment to next line in video buffer
		dec		[height]			; Loop for all rows in image
		jnz		@@StartNonAligned
		jmp		@@Exit

; Routine for byte aligned images.

@@DoAligned:
		mov		ah,[color]			; AH := color value to set

@@StartAligned:
		mov		_cx,[byteWidth]		; CX := count of bytes to output

@@NextByte:
		mov		al,[_FS _si]		; AL := next byte of image data
		inc		_si
		xlat	[BYTE _bx]			; Reverse the bit pattern
		out		dx,al				; Update plane mask register
		mov		[_ES _di],ah		; Update first byte
		inc		_di
		rol		al,4				; Rotate plane mask 4 bits
		out		dx,al				; Update plane mask register
		mov		[_ES _di],ah		; Update second byte
		inc		_di
		loop	@@NextByte			; Loop for all bytes in image

		add		_di,[rowAdjust]		; increment to next line in video buffer
		dec		[height]
		jnz		@@StartAligned		; Loop for all rows in image

; Restore default graphics controller state

@@Exit:	restore_es
		call	_VGAX_endDrawing
		leave_c
		ret

procend		_VGAX_putMonoImage

;----------------------------------------------------------------------------
; void VGAX_putImage(MGLDC *dc,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,void *surface,
;	int bytesPerLine,MGLDC *src)
;----------------------------------------------------------------------------
; Blt an image from a memory buffer onto the device. We handle a number of
; special cases as quickly as possible in here:
;
;		- Full size Blt (from a another device of the same dimensions)
;		- 4 pixel aligned partial Blt in REPLACE mode
;
;	Entry:	dc				- Device context to Blt to
;			left			- Left coordinate of are to Blt from
;			top				- Top coordinate of image to Blt from
;			right			- Right coordinate of image to Blt from
;			bottom			- Bottom coordinate of image to Blt from
;			dstLeft			- Left coordinate to Blt to
;			dstTop			- Right coordinate to Blt to
;			op				- Write mode to use during Blt
;			surface			- Surface to blt image from
;			bytesPerLine	- bytesPerLine value for surface
;----------------------------------------------------------------------------
procstart	_VGAX_putImage

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR,		\
				bytesPerLine:UINT, src:DPTR

		LOCAL	bufAdjust:UINT, count:UINT, srcBuf:UINT, 				\
				dstBuf:UINT = LocalSize

		enter_c	LocalSize
		save_ds
		save_es
		cld

; Determine special cases

		_les	_bx,[dc]
		cmp		[op],MGL_REPLACE_MODE
		jne     @@DoROP
		cmp		[left],0
		jne		@@NotFull
		cmp		[top],0
		jne		@@NotFull
		cmp		[dstLeft],0
		jne		@@NotFull
		cmp		[dstTop],0
		jne		@@NotFull
		mov     _ax,[MGLDC.mi.xRes]
		inc		_ax
		cmp		[right],_ax
		jne		@@NotFull
		mov     _ax,[MGLDC.mi.yRes]
		inc		_ax
		cmp		[bottom],_ax
		jne		@@NotFull

; Full size Blt, so simply copy the entire image verbatim

		mov		_ax,[MGLDC.mi.bytesPerLine]
		mul		[bottom]			; _AX := bytes to move
		shr		_ax,2				; _AX := 4 pixel lots to move
		mov		_cx,_ax				; _CX := bytes to move
		mov		dx,SC_DATA			; DX := SC data register port
		mov		al,11h				; AL := plane mask in position
		lVidDC	_di					; ES:_DI -> video buffer
		_lds	_si,[surface]		; DS:_SI -> system memory buffer

		mov		[srcBuf],_si		; Save buffer offsets
		mov		[dstBuf],_di

@@NextPlane:
		mov		_bx,_cx				; BX := count of 4 pixel lots to process
		out		dx,al				; Set plane for this pixel
		mov		_si,[srcBuf]		; Restore buffer offsets
		mov		_di,[dstBuf]
		push	_ax

@@NextPixels:
		mov		al,[_si+8]
		mov		ah,[_si+12]
		shl		eax,16
		mov		al,[_si]
		mov		ah,[_si+4]
		mov		[_ES _di],eax		; Store the 4 pixels
		add		_si,16
		add		_di,4
		dec     _bx
		jnz		@@NextPixels		; Loop for next 4 pixels

		pop		_ax
		inc		[srcBuf]
		rol		al,1
		jnc		@@NextPlane			; Process for all 4 planes
		jmp		@@Exit

; Check for and handle 4 pixel aligned partial Blt's

@@NotFull:
		test	[left],3			; Left aligned to 4 pixel boundary?
		jnz		@@DoROP				; Nope, so handle generically
		test	[right],3			; Right aligned to 4 pixel boundary?
		jnz		@@DoROP				; Nope, so handle generically

		mov		_ax,[top]
		mul		[bytesPerLine]
		add		_ax,[left]			; _AX := offset into source buffer
		mov		_di,_ax
		mov		_dx,[right]
		sub		_dx,[left]			; _DX := number of bytes to move
		cmp		_dx,16
		jle		@@DoROP				; Too few bytes for unrolled loops!
		mov		[count],_dx
		mov		_ax,[MGLDC.mi.bytesPerLine]
		mov		[bufAdjust],_ax		; Save destination buffer increment
		mov		_ax,[dstTop]
		mov		_cx,[dstLeft]
		call	VGAX_pixelAddrESBX	; _ES _BX -> device surface
		_lds	_si,[surface]
		add		_si,_di				; DS:_SI -> system memory buffer
		mov		_di,_bx				; ES:_DI -> device surface
		mov     _cx,[bottom]
		sub		_cx,[top]			; _CX := number of rows
		mov		dx,SC_DATA

@@NextRow:
		mov		al,11h				; AL := plane mask in position
		mov		[srcBuf],_si		; Save buffer offsets
		mov		[dstBuf],_di
		push	_si
		push	_di

@@NextPlanePartial:
		mov		_bx,[count]
		shr		_bx,4				; BL := count of 4 pixel lots to process
		out		dx,al				; Set plane for this pixel
		mov		bh,al				; BH := plane mask value
		mov		_si,[srcBuf]		; Restore buffer offsets
		mov		_di,[dstBuf]

@@NextPixelsPartial:
		mov		al,[_si+8]
		mov		ah,[_si+12]
		shl		eax,16
		mov		al,[_si]
		mov		ah,[_si+4]
		mov		[_ES _di],eax		; Store the 4 pixels
		add		_si,16
		add		_di,4
		dec     bl
		jnz		@@NextPixelsPartial	; Loop for next 4 pixels

		inc		[srcBuf]
		mov		al,bh
		rol		al,1
		jnc		@@NextPlanePartial	; Process for all 4 planes

@@DoneMove:
		pop		_di
		pop		_si
		add     _si,[bytesPerLine]	; Move to next line
		add		_di,[bufAdjust]		; Move to next line
		dec		_cx
		jnz		@@NextRow

@@Exit:	restore_es
		restore_ds
		leave_c
		ret

; Simply call the C version to handle this

@@DoROP:
		push	[src]
		push	[bytesPerLine]
		push	[surface]
		push	[op]
		push	[dstTop]
		push	[dstLeft]
		push	[bottom]
		push	[right]
		push	[top]
		push	[left]
		push	[dc]
		call	___EMU__putImage
		_add	sp,28,44
		jmp		@@Exit

procend		_VGAX_putImage

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; void VGAX_drawScanList(int y,int length,short *scans);
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call dc->scanLine to draw each scanline for us.
;
; Note: This routine will reverse the X coordinates for each scanline if
;		necessary to make xstart < xend, and will scan convert all pixels
;		in the range [xstart,xend), so xend will not be scan converted.
;		We also ignore any scanline where xstart == xend.
;
;		We also assume that the video card is already in write mode 3.
;
;		No clipping at all is performed within this routine.
;
; Entry:	y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_VGAX_drawScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		enter_c	0
		save_es
		cld							; String instructions go up

		call	_VGAX_beginDrawing

; Check to see if we can proceed at full speed, otherwise jump to a slower
; routine for patterned fills etc.

		cmp		[GDC.a.writeMode],MGL_REPLACE_MODE
		jne		@@SlowFill
		cmp		[GDC.a.penStyle],MGL_BITMAP_SOLID
		jne		@@SlowFill

;----------------------------------------------------------------------------
; Fast scanline fill routine. This has been optimised for solid fills in
; replace mode.
;
; Compute address of start of first scan line in video buffer

@@FastFill:
		mov		_ax,[y]				; AX := top coordinate
		imul	[GDC.mi.bytesPerLine]	; AX := y * BytesPerLine
ifdef	USE_SELVMEM
		add		ax,[USHORT GDC.surface]
else
		add		_ax,[UINT GDC.surface]
endif
		mov		_di,_ax				; _ES _DI -> frame buffer

; Get the address of the scanline array in ES:_SI

		_les	_si,[scans]			; ES:_SI -> scanline array data
		xor		_bx,_bx

@@FillLoop:

; Check that xstart < xend, swapping if need be.

		mov		dx,[_ES _si]		; DX := xstart for scanline
		clrhi	dx
		mov		ax,[_ES _si+2]		; AX := xend for scanline
		clrhi	ax
		add		_si,4
		cmp		_dx,_ax				; is xstart > xend?
		jl		@@NoSwap			; no, don't swap them
		jg		@@Swap				; yes, swap them
		jmp		@@SkipLine			; ignore if xstart == xend

@@Swap:
		xchg	_ax,_dx

@@NoSwap:
		push	_si
		push	_di					; Preserve DI

; Find offset into buffer

		mov		_si,_dx
		shr		_si,2
		add		_di,_si

; Compute the left and right plane clipping masks for the line

		mov		_si,_dx
		and		_si,3				; _SI := plane number for xstart
		mov		bl,[_VGAX_leftMask+_si]; BL := left clipping mask
		mov		_si,_ax
		dec		_si					; Adjust righ coordinate
		and		_si,3				; _SI := plane number for x2
		mov		bh,[_VGAX_rightMask+_si]; BH := right clipping mask

; Compute the number of addresses to be filled for the scanline

		dec		_ax					; Adjust xend coordinate
		and		_dx,not 011b
		sub		_ax,_dx
		shr		_ax,2				; _AX := # address to fill - 1
		mov		_cx,_ax				; _CX := # address to fill - 1
		mov		dx,SC_DATA			; DX := Sequencer Controller data reg
		mov		_ax,_bx				; _AX := clipping masks
		mov		bh,[BYTE GDC.intColor]; BH := color to fill with
		jz		@@OneByte			; There's only one byte to fill

		out		dx,al				; Set the left edge clip mask
		mov		[_ES _di],bh
		inc		_di
		dec		_cx					; Count off this byte
		jz		@@DoRightEdge		; There are only two bytes

		mov		al,0Fh				; Middle addresses are drawn 4 each
		out		dx,al				; Set the middle pixels plane mask
		mov		al,bh				; AL := color value to program
ifdef	USING_DS
		push	es					; Save ES
		mov		es,[USHORT (GDC.surface)+2]
endif

@@ForceAlignment:
		test	_di,3
		jz		@@Aligned
		mov		[_ES _di],al
		inc		_di
		dec		_cx
		jnz		@@ForceAlignment

@@Aligned:
		push	_cx
		shr		_cx,2
	rep	stosd						; Store all middle DWORD's fast!
		pop		_cx
		and		_cx,3
	rep	stosb						; Store the last bytes if any

ifdef	USING_DS
		pop		es					; Restore ES
endif

@@DoRightEdge:
		mov		al,ah				; AL := right clip mask
		out		dx,al				; Set the right clip mask
		mov		[_ES _di],bh		; Draw the right edge
		jmp		@@DoneLine

@@OneByte:
		and		al,ah				; There is only one byte to draw
		out		dx,al
		mov		[_ES _di],bh

@@DoneLine:
		pop		_di					; Restore DI
		pop		_si

@@SkipLine:
		add		_di,[GDC.mi.bytesPerLine]; Increment address to next scanline
		dec		[len]
		jnz		@@FillLoop			; Loop for all scanlines
		jmp		@@Exit

;----------------------------------------------------------------------------
; Slower scanline filling routine. This routine simply calls scanline to
; draw each scanline.

@@SlowFill:
		mov		_di,[y]				; DI := top coordinate

; Get the address of the scanline array in DS:SI

		_les	_si,[scans]			; ES:_SI -> scanline array data

@@FillLoop2:

; Check that xstart < xend, swapping if need be.

		lods	[WORD _ES _si]
		clrhi	ax
		mov		_dx,_ax				; DX := xstart for scanline
		lods	[WORD _ES _si]		; AX := xend for scanline
		clrhi	ax
		cmp		_dx,_ax				; is xstart > xend?
		jl		@@NoSwap2			; no, don't swap them
		jg		@@Swap2				; yes, swap them
		jmp		@@SkipLine2			; ignore if xstart == xend

@@Swap2:
		xchg	_ax,_dx

@@NoSwap2:
		use_es
		push	_ax					; Push xend coordinate
		push	_dx					; Push xstart coordinate
		push	_di					; Push y coordinate
		call	[GDC.r.cur.scanLine]
		_add	sp,6,12
		unuse_es

@@SkipLine2:
		inc		_di
		dec		[len]
		jnz		@@FillLoop2			; Loop for all scanlines

@@Exit:	restore_es
		call	_VGAX_endDrawing
		leave_c_nolocal
		ret

procend		_VGAX_drawScanList

;----------------------------------------------------------------------------
; void VGAX_ellipse(int left,int top,int A,int B,int clip);
;----------------------------------------------------------------------------
; Routine draws an Ellipse in native VGA ModeX graphics modes.
;
; The ellipse is scan converted by using the Midpoint algorithm. This
; scan converts ellipses with integer major and minor axes, where the
; semi-major and semi-minor axes may be non-integer (since the semi-axes are
; half the value of the major axes!).
;
; Entry:		left	- Left coorindate of ellipse rectangle
;				top		- Top coodinate of ellipse rectangle
;				A		- Major axis
;				B		- Minor axis
;				clip	- True if ellipse should be clipped
;
;----------------------------------------------------------------------------
procstart	_VGAX_ellipse

		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT,			\
				clip:UINT

		LOCAL	ULaddr:UINT, URaddr:UINT, LLaddr:UINT, LRaddr:UINT,	\
				VARd:ULONG, VARdx:ULONG, VARdy:ULONG, 				\
				Asquared:ULONG, Bsquared:ULONG,						\
				FourAsquared:ULONG, FourBsquared:ULONG,				\
				EightAsquared:ULONG, EightBsquared:ULONG,			\
				Set4Pix:NCPTR, _topY:UINT, _botY:UINT, _left:UINT, 	\
				_right:UINT, LMask:S_UCHAR, RMask:S_UCHAR, 			\
				c_left:UINT, c_top:UINT, c_right:UINT,				\
				c_bottom:UINT, bytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		save_es

; Load local variables from device context

		mov		_ax,[GDC.mi.bytesPerLine]
		mov     [bytesPerLine],_ax
		mov		_ax,[GDC.intClipRect.left]
		mov		[c_left],_ax
		mov		_ax,[GDC.intClipRect.top]
		mov		[c_top],_ax
		mov		_ax,[GDC.intClipRect.right]
		mov		[c_right],_ax
		mov		_ax,[GDC.intClipRect.bottom]
		mov		[c_bottom],_ax

; Configure the graphics controller

		mov		ah,[BYTE GDC.a.writeMode] ; Get write mode function in AH
		mov		dx,GC_INDEX			; DX := GC address register port
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		mov		ax,04805h			; AL := Mode register number
									; AH := write mode 0, read mode 1, for
									; ModeX
		out		dx,ax

		mov		ax,0007h			; AH := 0 (don't care for all maps;
									;	CPU reads always return 0FFH)
									; AL := 7 (Color Don't care reg number)
		out		dx,ax				; Set up Color Don't care reg

; Compute initial constants

		loadint	ax,[ARGa]
		mul		eax
		mov		[Asquared],eax		; Compute A^2
		shl		eax,2				; Multiply by 4
		mov		[FourAsquared],eax	; Compute 4*A^2
		shl		eax,1				; Multiply by 8
		mov		[EightAsquared],eax	; Compute 8*A^2

		loadint	ax,[ARGb]
		mul		eax
		mov     [Bsquared],eax		; Compute B^2
		shl		eax,2				; Multiply by 4
		mov		[FourBsquared],eax	; Compute 4*B^2
		shl		eax,1				; Multiply by 8
		mov		[EightBsquared],eax	; Compute 8*B^2

; The initial value of dx is common to both types of ellipses, so we set
; that here.

		loadint	ax,[ARGb]
		mul		[FourAsquared]
		mov		[VARdx],eax			; dx = FourAsquared * B

; Compute initial pixel addresses and bit masks

		mov		_ax,[bytesPerLine]
		mul		[ARGb]				; AX := relative byte offset of B
		mov		_si,_ax

		mov		_ax,[top]			; AX := top
		mov		_cx,[ARGa]
		shr		_cx,1				; CX := A/2
		add		_cx,[left]			; CX := left + A/2
		call	VGAX_pixelAddr		; ES:_BX -> buffer
									; CL := plane # for first pixel
		mov		ah,11h
		rol		ah,cl				; AH := plane mask in position
		mov		[LMask],ah			; Save bit mask
		mov		[RMask],ah

		mov		[ULaddr],_bx		; Save address of these pixels
		mov		[URaddr],_bx
		add		_bx,_si				; _BX := offset of (left + A/2,top + B)
		mov		[LLaddr],_bx		; Save address of these pixels
		mov		[LRaddr],_bx

; Compute initial pixel coordinates (used by clipping version of Set4Pixels)

		mov		_ax,[ARGa]
		mov		_bx,_ax				; BX := ARGa
		shr		_ax,1
		add		_ax,[left]			; AX := left + A/2
		mov		[_left],_ax
		mov		[_right],_ax

		mov		_ax,[top]
		mov		[_topY],_ax
		add		_ax,[ARGb]
		mov		[_botY],_ax

; We have two cases to check for. One where the semi-major axis is integer
; and one where it is not. We must set up different initial decision
; variables and pixel addresses for each case.

		test	_bx,1
		jz		@@IntegerMajorAxis

; We have a non-integer semi-major axis

		mov		ebx,[FourBsquared]
		mov		eax,[Bsquared]
		mov		[VARdy],ebx			; dy = FourBsquared
		mov		ecx,eax
		shl		ecx,3				; ECX := 8*Bsquared
		add		ecx,eax				; ECX := 9*Bsquared
		mov		ebx,[Asquared]
		xor		eax,eax
		add		ecx,ebx				; ECX := Asquared + 9*Bsquared
		loadint	ax,[ARGb]
		shl		eax,1				; EAX := 2*B
		mul		ebx					; EAX = 2*Asquared*B
		sub		ecx,eax
		mov		[VARd],ecx			; d = Asquared - 2*Asquared*B + 9*Bsquared

; We must also adjust the initial UR and LR pixel masks and addresses to be
; one position along in the x direction.

		xor		_si,_si
		mov		ah,[RMask]
		rol		ah,1				; AH := bit mask rotated horizontally
		rcl		_si,1				; _SI := 1 if bit mask rotated around
		mov		[RMask],ah
		add		[URaddr],_si		; Adjust UR and LR pixels
		add		[LRaddr],_si		; accordingly

; And adjust the Right Pixel coordinates accordingly

		inc		[_right]
		jmp		@@CheckClip

@@IntegerMajorAxis:
		xor		eax,eax
		mov		ecx,[FourBsquared]
		mov		[VARdy],eax			; dy = 0
		add		ecx,[Asquared]		; ECX := Asquared + FourBsquared
		loadint	ax,[ARGb]
		shl		eax,1				; EAX := 2*B
		mul		[Asquared]			; EAX = 2*Asquared*B
		sub		ecx,eax
		mov		[VARd],ecx			; d = Asquared - 2*Asquared*B
									;		+ FourBsquared

@@CheckClip:
		mov		[Set4pix],offset Set4Pixels
		test	[BYTE clip],1
		jz		@@DontClip

		mov		[Set4pix],offset Clip4Pixels

; Loop while dx > dy

@@DontClip:
		xor		_cx,_cx				; CH := 0 (initial y-increment)
									; CL := 0 (initial x-increment)
		mov		edx,[VARd]			; EDX := decision variable
		mov		esi,[VARdx]			; ESI := dx
		mov		edi,[VARdy]			; EDI := dy

@@Region1:
		cmp		edi,esi
		jge		@@Region2			; Do region 2 if dy >= dx

		call	[Set4pix]

		mov		_cx,1				; CH := 0 (y-increment)
									; CL := 1 (x-increment)
		or		edx,edx
		js		@@DlessThanZero		; jump if d < 0

		mov		ch,1				; increment in y direction
		sub		esi,[EightAsquared]	; dx -= EightAsquared
		sub		edx,esi				; d -= dx

@@DlessThanZero:
		add		edi,[EightBsquared]	; dy += EightBsquared
		add		edx,edi				; d += dy
		add		edx,[FourBsquared]	; d += dy + FourBsquared
		jmp		@@Region1

; Plot pixels from current (x,y) while dx >= 0
;
; First we must adjust the decision variable to the new midpoint position
; since we have changed regions.

@@Region2:
		mov		eax,[Asquared]
		sub		eax,[Bsquared]		; EAX := Asquared-Bsquared
		mov		ebx,eax
		shl		eax,1
		add		eax,ebx				; EAX := 3*(Asquared-Bsquared)
		mov		ebx,esi
		add		ebx,edi				; EBX := dx + dy
		sar		ebx,1				; EBX := (dx + dy)/2
		sub		eax,ebx
		add		edx,eax				; d -= 3*(Asquared-Bsquared) -
									;		(dx + dy)/2
; loop while dx >= 0

@@LoopRegion2:
		call	[Set4pix]

		mov		_cx,100h			; CH := 1 (y-increment)
									; CL := 0 (x-increment)
		or		edx,edx
		jns		@@DgreatThanZero	; jump if d >= 0

		mov		cl,1				; increment in x direction

		add		edi,[EightBsquared]	; dy += EightBsquared
		add		edx,edi				; d += dy

@@DgreatThanZero:
		sub		esi,[EightAsquared]	; dx -= EightAsquared
		sub		edx,esi				; d -= dx
		add		edx,[FourAsquared]	; d += FourAsquared - dx
		or		esi,esi
		jns		@@LoopRegion2		; Loop if dx >= 0

; Restore default graphics controller state and return to caller

@@Exit:	restore_es
		call	_VGAX_endDrawing
		leave_c
		ret

procend		_VGAX_ellipse

;----------------------------------------------------------------------------
; Set4Pixels	 Sets four pixels in EGA/VGA native graphics modes
;----------------------------------------------------------------------------
;
; Entry:		CH	- y-increment (0,-1)
;				CL	- x-increment (0,1)
;
; Note that we are using the same stack frame as the main ellipse drawing
; routine, so we can access all of the variables we need on the stack
; without having to build our own frame.
;----------------------------------------------------------------------------
PROC	Set4Pixels	near

		push	_ax					; Preserve these registers
		push	_bx
		push	_dx
		push	_si
		push	_di

		mov		dx,SC_DATA			; DX := Sequencer Controller Data Reg
		mov		ah,[BYTE GDC.intColor]; AH := pixel color value

		xor		_bx,_bx				; BX := 0
		test	ch,ch
		jz		@@NoYinc			; jump if y-increment = 0

		mov		_bx,[bytesPerLine]	; BX := positive y increment

@@NoYinc:

; Plot the Upper Left and Lower Left pixels

		xor		_si,_si				; SI := 0
		mov		al,[LMask]

		ror		al,cl				; AL := plane mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around
		neg		_si					; SI := 0 or -1

		mov		_di,_si				; SI,DI := left horizontal increment

		add		_si,[ULaddr]		; SI := upper left addr + horiz incr
		add		_si,_bx				; SI := new upper left addr
		add		_di,[LLaddr]
		sub		_di,_bx				; DI := new lower left addr

		mov		[LMask],al			; Update these variables
		mov		[ULaddr],_si
		mov		[LLaddr],_di

		out		dx,al				; Set the plane mask
		cmp		[BYTE GDC.a.writeMode],MGL_REPLACE_MODE
		jne		@@DoLeftROP
		mov		[_ES _si],ah			; Update upper left pixel
		mov		[_ES _di],ah			; Update lower left pixel
		jmp		@@DoneLeft

@@DoLeftROP:
		and		[_ES _si],ah			; Update upper left pixel
		and		[_ES _di],ah			; Update lower left pixel

; Plot the Upper Right and Lower Right pixels

@@DoneLeft:
		xor		_si,_si				; SI := 0
		mov		al,[RMask]

		rol		al,cl				; AL := plane mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around

		mov		_di,_si				; SI,DI := right horizontal increment

		add		_si,[URaddr]		; SI := upper right addr + horiz incr
		add		_si,_bx				; SI := new upper right addr
		add		_di,[LRaddr]
		sub		_di,_bx				; DI := new lower right addr

		mov		[RMask],al			; Update these variables
		mov		[URaddr],_si
		mov		[LRaddr],_di

		out		dx,al				; Set plane mask
		cmp		[BYTE GDC.a.writeMode],MGL_REPLACE_MODE
		jne		@@DoRightROP
		mov		[_ES _si],ah			; Update upper right pixel
		mov		[_ES _di],ah			; Update lower right pixel
		jmp		@@DoneRight

@@DoRightROP:
		and		[_ES _si],ah			; Update upper right pixel
		and		[_ES _di],ah			; Update lower right pixel

@@DoneRight:
		pop		_di
		pop		_si
		pop		_dx
		pop		_bx
		pop		_ax
		ret

ENDP	Set4Pixels

;----------------------------------------------------------------------------
; Clip4Pixels	 Sets four pixels in EGA/VGA native graphics modes
;----------------------------------------------------------------------------
;
; This version of Set4Pixels clips the pixel's on a pixel by pixel basis.
;
; Entry:		CH	- y-increment (0,-1)
;				CL	- x-increment (0,1)
;
;----------------------------------------------------------------------------
PROC	Clip4Pixels	near

		push	_ax					; Preserve these registers
		push	_bx
		push	_dx
		push	_si
		push	_di

		mov		dx,SC_DATA			; DX := Sequencer Controller Data Reg
		xor		_bx,_bx				; BX := 0
		test	ch,ch
		jz		@@NoYinc			; jump if y-increment = 0

		mov		_bx,[bytesPerLine]	; BX := positive y increment

; Increment the y coordinate placemarkers

		inc		[_topY]
		dec		[_botY]

@@NoYinc:
		test	cl,cl
		jz		@@NoXinc			; jump if x-increment = 0

; Increment the x coordinate placemarkers

		inc		[_right]
		dec		[_left]

; Plot the Upper Left and Lower Left pixels

@@NoXinc:
		mov		ch,[BYTE GDC.intColor]; CH := pixel color value
		xor		_si,_si				; SI := 0
		mov		al,[LMask]

		ror		al,cl				; AL := plane mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around
		neg		_si					; SI := 0 or -1

		mov		_di,_si				; SI,DI := left horizontal increment

		add		_si,[ULaddr]		; SI := upper left addr + horiz incr
		add		_si,_bx				; SI := new upper left addr
		add		_di,[LLaddr]
		sub		_di,_bx				; DI := new lower left addr

		mov		[LMask],al			; Update these variables
		mov		[ULaddr],_si
		mov		[LLaddr],_di

		out		dx,al				; Set the plane mask

		mov		_ax,[_left]
		cmp		_ax,[c_left]
		jl		@@LowerLeft			; Clip it ...
		cmp		_ax,[c_right]
		jge		@@LowerLeft			; Clip it ...

		mov		_ax,[_topY]
		cmp		_ax,[c_top]
		jl		@@LowerLeft			; Clip it ...
		cmp		_ax,[c_bottom]
		jge		@@LowerLeft			; Clip it ...

		and		[_ES _si],ch			; Update upper left pixel

@@LowerLeft:
		mov		_ax,[_left]
		cmp		_ax,[c_left]
		jl		@@UpperRight		; Clip it ...
		cmp		_ax,[c_right]
		jge		@@UpperRight		; Clip it ...

		mov		_ax,[_botY]
		cmp		_ax,[c_top]
		jl		@@UpperRight		; Clip it ...
		cmp		_ax,[c_bottom]
		jge		@@UpperRight		; Clip it ...

		and		[_ES _di],ch			; Update lower left pixel

; Plot the Upper Right and Lower Right pixels

@@UpperRight:
		xor		_si,_si				; SI := 0
		mov		al,[RMask]

		rol		al,cl				; AL := plane mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around

		mov		_di,_si				; SI,DI := right horizontal increment

		add		_si,[URaddr]		; SI := upper right addr + horiz incr
		add		_si,_bx				; SI := new upper right addr
		add		_di,[LRaddr]
		sub		_di,_bx				; DI := new lower right addr

		mov		[RMask],al			; Update these variables
		mov		[URaddr],_si
		mov		[LRaddr],_di

		out		dx,al				; Set plane mask

		mov		_ax,[_right]
		cmp		_ax,[c_left]
		jl		@@UpperLeft			; Clip it ...
		cmp		_ax,[c_right]
		jge		@@UpperLeft			; Clip it ...

		mov		_ax,[_topY]
		cmp		_ax,[c_top]
		jl		@@UpperLeft			; Clip it ...
		cmp		_ax,[c_bottom]
		jge		@@UpperLeft			; Clip it ...

		and		[_ES _si],ch			; Update upper right pixel

@@UpperLeft:
		mov		_ax,[_right]
		cmp		_ax,[c_left]
		jl		@@Exit				; Clip it ...
		cmp		_ax,[c_right]
		jge		@@Exit				; Clip it ...

		mov		_ax,[_botY]
		cmp		_ax,[c_top]
		jl		@@Exit				; Clip it ...
		cmp		_ax,[c_bottom]
		jge		@@Exit				; Clip it ...

		and		[_ES _di],ch			; Update lower right pixel

@@Exit:
		pop		_di
		pop		_si
		pop		_dx
		pop		_bx
		pop		_ax
		ret

ENDP	Clip4Pixels

endif	; !MGL_LITE

endcodeseg	_vgax

		END							; End of module
