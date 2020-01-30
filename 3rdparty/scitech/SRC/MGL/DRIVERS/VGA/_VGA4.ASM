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
;* Description:	VGA4, 4 bit color assembler based routines.
;*
;*				NOTE: We do all framebuffer arithmetic in 16 bits, so that
;*					  we get the required wraparound at the end of the
;*					  framebuffer (required to handle negative coordinates
;*					  correctly).
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"
include "mgldos\biosdata.equ"

header	_vga4

struc	VGA_state_s
v				display_vec_s	<>
oldBIOSMode		ushort			?
old50Lines		bool			?
fastDrawing		bool			?
ends	VGA_state_s
state	= (VGA_state_s PTR _ES _BX)

	EXTRN	___EMU__scanLine:FPTR

begdataseg	_vga4

	$EXTRN	__MGL_dc,devCtx_s

enddataseg	_vga4

begcodeseg	_vga4

;----------------------------------------------------------------------------
; VGA4_pixelAddrESBX	Determine buffer address of pixel in EGA/VGA modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				ES:_BX	->  device context
;				_CX		-	x-coordinate
;
; Exit:			AH		-	bit mask
;				ES:_BX	->	offset in buffer
;				CL		-	number of bits to shift left
;
; Registers:	none.
;
;----------------------------------------------------------------------------
procstart	VGA4_pixelAddrESBX

		push	_dx						; Save DX
		push	_cx
		imul	[MGLDC.mi.bytesPerLine]	; AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@NotNegative

; The x coordinate is negative, so adjust to be positive

		mov		_dx,_cx
		neg		_dx
		add		_dx,7
		shr		_dx,3
		sub		_ax,_dx					; Adjust for negative coords
		shl		_dx,3
		add		_cx,_dx					; Make coordinate positive again

@@NotNegative:
		lVidDC	_bx
		shr		_cx,3					; _CX := x/8
		add     _bx,_cx
		add		_bx,_ax

		pop		_cx
		mov		ah,1					; AH := unshifted bit mask
		and		cl,7					; CL := x & 7
		xor		cl,7					; CL := # bits to shift left

		pop		_dx
		ret

procend		VGA4_pixelAddrESBX

;----------------------------------------------------------------------------
; VGA4_pixelAddr	Determine buffer address of pixel in native EGA/VGA modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				_CX		-	x-coordinate
;
; Exit:			AH		-	bit mask
;				ES:_BX	->	offset in buffer
;				CL		-	number of bits to shift left
;
; Registers:	none.
;
;----------------------------------------------------------------------------
procstart	VGA4_pixelAddr

		push	_dx						; Save DX
		push	_cx
		imul	[GDC.mi.bytesPerLine]	; AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@NotNegative

; The x coordinate is negative, so adjust to be positive

		mov		_dx,_cx
		neg		_dx
		add		_dx,7
		shr		_dx,3
		sub		_ax,_dx					; Adjust for negative coords
		shl		_dx,3
		add		_cx,_dx					; Make coordinate positive again

@@NotNegative:
		lVid	_bx
		shr		_cx,3					; _CX := x/8
		add     _bx,_cx
		add		_bx,_ax

		pop		_cx
		mov		ah,1					; AH := unshifted bit mask
		and		cl,7					; CL := x & 7
		xor		cl,7					; CL := # bits to shift left

		pop		_dx
		ret

procend		VGA4_pixelAddr

;----------------------------------------------------------------------------
; void VGA4_beginDrawing(void);
;----------------------------------------------------------------------------
; Sets the video card into write mode 0.
;----------------------------------------------------------------------------
procstart	_VGA4_beginDrawing

		push	_ax
		push	_dx

; Configure the graphics controller (write mode 3)

		mov     ah,[BYTE GDC.a.writeMode]
		mov		dx,3CEh				; DX := Graphics Controller port addr

		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		mov		ax,0B05h			; AL := Mode register number
									; AH := Write mode 3 (bits 0,1)
									;	Read mode 1 (bit 3)
		out		dx,ax

		mov		ax,0007h			; AH := 0 (don't care for all maps;
									;	CPU reads always return 0FFH)
									; AL := 7 (Color Don't care reg number)
		out		dx,ax				; Set up Color Don't care reg

		mov		ax,0F01h			; AH := 1111b (bit plane mask for
									;  enable Set/Reset)
		out		dx,ax				; AL := Enable Set/Reset Register #

		mov		ax,0FF08h			; AH := 11111111b, AL := 8
		out		dx,ax				; restore bit mask register

		pop		_dx
		pop		_ax
		ret

procend		_VGA4_beginDrawing

;----------------------------------------------------------------------------
; void VGA4_endDrawing(void);
;----------------------------------------------------------------------------
; Reset the video card into the default BIOS state. We only actually turn
; off the fast drawing mode when the last call has been made to endDrawing.
; This allows nested calls to beginDrawing and endDrawing without any
; performance loss.
;----------------------------------------------------------------------------
procstart	_VGA4_endDrawing

		push	_ax
		push	_dx

; Restore graphics controller and return to caller

		mov		dx,3CEh				; DX := Graphics Controller port addr
		xor		ax,ax				; AH := 0, AL := 0
		out		dx,ax				; Restore Set/Reset Register

		inc		ax					; AH := 0, AL := 1
		out		dx,ax				; Restore Enable Set/Reset register

		mov		al,3				; AH := 0, AL := 3
		out		dx,ax				; Restore Data Rotate/Func select reg

		mov		al,5				; AH := 0, AL := 5
		out		dx,ax				; default mode register

		mov		ax,0F07h			; default color compare value
		out		dx,ax

		mov		ax,0FF08h			; AH := 11111111b, AL := 8
		out		dx,ax				; restore bit mask register

		pop		_dx
		pop		_ax
		ret

procend		_VGA4_endDrawing

;----------------------------------------------------------------------------
; color_t VGA4_getPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine reads the value of a pixel in native EGA/VGA graphics modes
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
; Exit:			DX:AX	-	Color of pixel at (x,y)
;
;----------------------------------------------------------------------------
procstart	_VGA4_getPixel

		ARG		x:UINT, y:UINT

		enter_c	0
		save_es

; Compute the pixel's address in video buffer

		mov		_ax,[y]
		mul		[GDC.mi.bytesPerLine]	; _DX:_AX := y * BytesPerLine
		lVid	_di
		mov		_bx,[x]
		mov		cl,bl				; CL := low-order byte of x
		shr		_bx,3				; _BX := x/8
		add     _bx,_di
		add		bx,ax				; BX := y*BytesPerLine + x/8

		mov		ch,1				; CH := unshifted bit mask
		and		cl,7				; CL := x & 7
		xor		cl,7				; CL := # bits to shift left
		shl		ch,cl				; CH := bit mask in proper position

; Read the pixel's value from all bit planes

		mov		_si,_bx				; ES:_SI -> regen buffer byte
		xor		bl,bl				; BL is used to accumulate the pixels value

		mov		dx,3CEh				; DX := Graphics controller port
		mov		ax,304h				; AH := initial bit plane
									; AL := Read Map Select register number

@@ReadPlane:
		out		dx,ax				; select bit plane
		mov		bh,[_ES _si]		; BH := byte from current bit plane
		and		bh,ch				; mask one bit
		neg		bh					; bit 7 of BH := 1 (if masked bit = 1)
									; bit 7 of BH := 0 (if masked bit = 0)
		rol		bx,1				; bit 0 of BL := next bit from pixel value
		dec		ah					; AH := next bit plane number
		jge		@@ReadPlane

		xor		_ax,_ax
		mov		al,bl				; AX := pixel value
		xor		_dx,_dx				; _DX:_AX := pixel value

		restore_es
		leave_c_nolocal
		ret

procend		_VGA4_getPixel

;----------------------------------------------------------------------------
; void VGA4_beginPixel(MGLDC *dc)
;----------------------------------------------------------------------------
; Routine setup the EGA/VGA for drawing single pixels. This _must_ be
; called before the pixel routine is used, and not other calls can be mode
; in between.
;
; Entry:		dc	- 	Device context
;
;----------------------------------------------------------------------------
procstart	_VGA4_beginPixel

		ARG		dc:DPTR

		enter_c	0

; set Graphics Controller Mode register

		mov		dx,3CEh				; GC address register port
		mov		ax,0205h			; AL := Mode register number
									; AH := Write mode 2 (bits 0,1)
									;	Read mode 0 (bit 3)
		out		dx,ax

; set data rotate/Function Select register

		_les	_bx,[dc]
		mov		ah,[BYTE MGLDC.a.writeMode]
		mov		cl,3				; Shift of 3 bits
		shl		ah,cl				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		leave_c_nolocal
		ret

procend		_VGA4_beginPixel

;----------------------------------------------------------------------------
; void VGA4_endPixel(MGLDC *dc);
;----------------------------------------------------------------------------
; Reset the video card into the current controller state after drawing
; a bunch of pixels which will have changed the controllers state.
;----------------------------------------------------------------------------
procstart	_VGA4_endPixel

		jmp		_VGA4_endDrawing

procend		_VGA4_endPixel

;----------------------------------------------------------------------------
; void VGA4_putPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native EGA/VGA graphics modes.
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
;----------------------------------------------------------------------------
procstart	_VGA4_putPixel

		ARG		x:UINT, y:UINT
		LOCAL   color:BYTE = LocalSize

		enter_c	LocalSize
		save_es

		mov		al,[BYTE GDC.intColor]
		mov		[color],al

; Compute the pixel's address in video buffer

		mov		_ax,[y]
		mul		[GDC.mi.bytesPerLine]	; _DX:_AX := y * BytesPerLine
		lVid	_di
		mov		_bx,[x]
		mov		cl,bl				; CL := low-order byte of x
		shr		_bx,3				; _BX := x/8
		add     _bx,_di
		add		bx,ax				; BX := y*BytesPerLine + x/8

		mov		ah,1				; AH := unshifted bit mask
		and		cl,7				; CL := x & 7
		xor		cl,7				; CL := # bits to shift left
		shl		ah,cl				; AH := bit mask in proper position

; set Graphics Controller Bit Mask register

		mov		dx,3CEh				; GC address register port
		mov		al,8				; AL := Bit Mask Register number
		out		dx,ax

; set the pixel value

		mov		al,[_ES _bx]		; latch one byte from each bit plane
		mov		al,[color]			; AL := pixel value
		mov		[_ES _bx],al		; update all bit planes

		restore_es
		leave_c
		ret

procend		_VGA4_putPixel

;----------------------------------------------------------------------------
; void VGA4_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,
;	int op)
;----------------------------------------------------------------------------
; Reads a scanline of information from the device context into the memory
; buffer. Note that this routine _always_ writes data to the memory buffer
; in packed pixel format. The 'bufx' value is used to determine how many
; bits each pixel value must be shifted over when writing the output
; buffer (to accomodate writing the second pixel in the buffer but not the
; first).
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
procstart	_VGA4_getScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		enter_c	0
		save_ds
		save_es

; Configure the graphics controller (read mode 0)

		mov		dx,3CEh				; GC address register port
		mov		ax,0205h			; AL := Mode register number
									; AH := Write mode 2 (bits 0,1)
									;	Read mode 0 (bit 3)
		out		dx,ax

; Compute the address of first pixel in video buffer

		_les	_bx,[dc]
		mov		_ax,[y]
		mov		_cx,[x1]
		call	VGA4_pixelAddrESBX	; ES:_BX -> video buffer
		mov		_di,_bx				; ES:_DI -> video buffer
		shl		ah,cl				; AH := bit mask in proper position

		mov		dx,3CEh				; GC address register port
		mov		al,4				; AL := Read Map Select Register number
		out		dx,al
		inc		dx					; DX -> GC data register port
		mov		_cx,[x2]
		sub		_cx,[x1]			; CX := count of pixels to process
		_lds	_si,[buffer]		; DS:_SI -> pixel buffer

; Determine if the first pixel in buffer is to be ignored

		test	[bufx],1
		jz		@@LoopMiddle

; Handle the first lone pixel individually

		xor		bl,bl				; BL := accumulated pixel value
		mov		al,3				; Initial bit plane number

@@ReadPlaneInitial:
		out		dx,al				; Select first bit plane
		mov		bh,[_ES _di]		; BH := byte from current plane
		and		bh,ah				; Mask out specific bit
		neg		bh					; bit 7 of BH := 1 (if masked bit = 1)
									; bit 7 of BH := 0 (if masked bit = 0)
		rol		bx,1				; bit 0 of BL := next bit from pixel value
		dec		al					; AL := next bit plane number
		jge		@@ReadPlaneInitial	; Loop for all pixel planes

		cmp		[op],MGL_REPLACE_MODE
		jne		@@Mod1
		and		[BYTE _si],0F0h
		or		[_si],bl			; Save pixel value
		jmp		@@DoneMod1

@@Mod1:	cmp		[op],MGL_XOR_MODE
		jne		@@Mod2

		xor		[_si],bl
		jmp		@@DoneMod1

@@Mod2:	cmp		[op],MGL_OR_MODE
		jne		@@Mod3

		or		[_si],bl
		jmp		@@DoneMod1

@@Mod3:	or		bl,0F0h				; Combine mask and pixel value
		and		[_si],bl

@@DoneMod1:
		inc		_si
		dec		_cx
		jz		@@Exit				; Exit if we are all done
		ror		ah,1
		jnc		@@LoopMiddle
		inc		_di					; Increment video buffer address

@@LoopMiddle:
		xor		bl,bl				; BL := accumulated pixel value
		mov		al,3				; Initial bit plane number

@@ReadPlane1:
		out		dx,al				; Select first bit plane
		mov		bh,[_ES _di]		; BH := byte from current plane
		and		bh,ah				; Mask out specific bit
		neg		bh					; bit 7 of BH := 1 (if masked bit = 1)
									; bit 7 of BH := 0 (if masked bit = 0)
		rol		bx,1				; bit 0 of BL := next bit from pixel value
		dec		al					; AL := next bit plane number
		jge		@@ReadPlane1		; Loop for all pixel planes

		shl		bl,4
		cmp		[op],MGL_REPLACE_MODE
		jne		@@Mod4
		and		[BYTE _si],0Fh
		or		[_si],bl			; Save pixel value
		jmp		@@DoneMod2

@@Mod4:	cmp		[op],MGL_XOR_MODE
		jne		@@Mod5

		xor		[_si],bl
		jmp		@@DoneMod2

@@Mod5:	cmp		[op],MGL_OR_MODE
		jne		@@Mod6

		or		[_si],bl
		jmp		@@DoneMod2

@@Mod6:	or		bl,0Fh				; Combine mask and pixel value
		and		[_si],bl

@@DoneMod2:
		dec		_cx
		jz		@@Exit				; Exit if we are all done
		ror		ah,1
		jnc		@@1
		inc		_di					; Increment video buffer address

@@1:	xor		bl,bl				; BL := accumulated pixel value
		mov		al,3				; Initial bit plane number

@@ReadPlane2:
		out		dx,al				; Select first bit plane
		mov		bh,[_ES _di]		; BH := byte from current plane
		and		bh,ah				; Mask out specific bit
		neg		bh					; bit 7 of BH := 1 (if masked bit = 1)
									; bit 7 of BH := 0 (if masked bit = 0)
		rol		bx,1				; bit 0 of BL := next bit from pixel value
		dec		al					; AL := next bit plane number
		jge		@@ReadPlane2		; Loop for all pixel planes

		cmp		[op],MGL_REPLACE_MODE
		jne		@@Mod7
		and		[BYTE _si],0F0h
		or		[_si],bl			; Save pixel value
		jmp		@@DoneMod3

@@Mod7:	cmp		[op],MGL_XOR_MODE
		jne		@@Mod8

		xor		[_si],bl
		jmp		@@DoneMod2

@@Mod8:	cmp		[op],MGL_OR_MODE
		jne		@@Mod9

		or		[_si],bl
		jmp		@@DoneMod2

@@Mod9:	or		bl,0F0h				; Combine mask and pixel value
		and		[_si],bl

@@DoneMod3:
		inc		_si					; Increment destination buffer
		ror		ah,1
		jnc		@@2
		inc		_di					; Increment video buffer address
@@2:	loop	@@LoopMiddle

; Restore default graphics controller state

@@Exit:	restore_es
		restore_ds
		call	_VGA4_endDrawing
		leave_c_nolocal
		ret

procend		_VGA4_getScanLine

;----------------------------------------------------------------------------
; void VGA4_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,
;	int op)
;----------------------------------------------------------------------------
; Dumps a scanline of information to the device context from the memory
; buffer. Note that this routine _always_ reads data from the memory buffer
; in packed pixel format. The 'bufx' value is used to determine how many
; bits each pixel value must be shifted over when reading from the input
; buffer (to accomodate reading the first pixel from a non-byte aligned
; address in the memory buffer).
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
procstart	_VGA4_putScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		enter_c	0
		save_ds
		save_es

; Configure the graphics controller (write mode 0)

		mov		dx,3CEh				; GC address register port
		mov		ax,0205h			; AL := Mode register number
									; AH := Write mode 2 (bits 0,1)
									;	Read mode 0 (bit 3)
		out		dx,ax

		mov		ah,[BYTE op]		; AH := write mode operation
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

; Compute the address of first pixel in video buffer

		_les	_bx,[dc]
		mov		_ax,[y]
		mov		_cx,[x1]
		call	VGA4_pixelAddrESBX	; ES:_BX -> video buffer
		mov		_di,_bx				; ES:_DI -> video buffer
		shl		ah,cl				; AH := bit mask in proper position

		mov		dx,3CEh				; GC address register port
		mov		al,8				; AL := Bit Mask Register number
		out		dx,al
		inc		dx					; DX -> GC data register port
		mov		al,ah				; AL := bit mask in proper position
		mov		_cx,[x2]
		sub		_cx,[x1]			; CX := count of pixels to process
		_lds	_si,[buffer]		; DS:_SI -> pixel buffer

; Determine if the first pixel in buffer is to be ignored

		test	[bufx],1
		jz		@@LoopMiddle

; Handle the first lone pixel individually

		out		dx,al				; Program bit mask register
		mov		ah,[_ES _di]		; latch one byte from each bit plane
		mov		ah,[_si]			; AH := pixel value
		inc		_si
		mov		[_ES _di],ah		; update all bit planes
		dec		_cx
		jz		@@Exit				; Exit if we are all done
		ror		al,1
		jnc		@@LoopMiddle
		inc		_di					; Increment video buffer address

@@LoopMiddle:
		out		dx,al				; Program bit mask register
		mov		ah,[_ES _di]		; latch one byte from each bit plane
		mov		bh,[_si]			; BH := pixel value
		inc		_si
		ror     bh,4
		mov		[_ES _di],bh		; update all bit planes
		dec		_cx
		jz		@@Exit				; Exit if we are all done
		ror		al,1
		jnc		@@1
		inc		_di					; Increment video buffer address
@@1:	out		dx,al				; Program bit mask register
		mov		ah,[_ES _di]		; latch one byte from each bit plane
		ror     bh,4
		mov		[_ES _di],bh		; update all bit planes
		ror		al,1
		jnc		@@2
		inc		_di					; Increment video buffer address
@@2:	loop	@@LoopMiddle

; Restore default graphics controller state

@@Exit:	restore_es
		restore_ds
		call	_VGA4_endDrawing
		leave_c_nolocal
		ret

procend		_VGA4_putScanLine

;----------------------------------------------------------------------------
; void VGA4_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a line in native EGA graphics modes.
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
procstart	_VGA4_line

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, EIncr:FIX32_T, 			\
				NEIncr:FIX32_T, varDX:FIX32_T, varDY:FIX32_T, x:UINT,	\
				y:UINT = LocalSize

		enter_c	LocalSize
		save_es

		call	_VGA4_beginDrawing

; Load the current color

		mov		dx,3CEh				; DX := Graphics Controller port addr
		mov		ah,[BYTE GDC.intColor]
		xor		al,al				; AL := Set/Reset Register number
		out		dx,ax

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
		call	VGA4_pixelAddr		; AH := Bit mask
									; ES:_BX -> buffer
									; CL := # bits to shift left

		shl		ah,cl				; AH := bit mask in proper position
		mov		al,ah				; AL := bit Mask
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
		push	_cx					; preserve this reg
		mov		ecx,[x1]
		add		ecx,8000h
		shr		ecx,16				; _CX := x
		call	VGA4_pixelAddr		; AH := Bit mask
									; ES:_BX -> buffer
									; CL := # bits to shift left

; Set up Graphics controller

		shl		ah,cl				; AH := bit mask in proper position
		pop		_cx					; restore reg

; Draw the line

@@Set:	and		[_ES _bx],ah		; set pixel value in buffer
		add		bx,si				; increment to next position of interleave
		loop	@@Set				; loop down the line

		jmp		@@Exit				; We are done...

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:_BX -> video buffer
;						AL = bit mask for 1st pixel
;						AH = bit mask for 1st pixel
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
		mov		esi,[EIncr]

@@StartLo:
		mov		al,ah				; AL := bit mask for next pixel

@@BitMaskIn:
		or		al,ah				; mask current pixel position
		ror		ah,1				; Rotate pixel value
		jc		@@BitMaskOut		; Jump if mask rotated to leftmost pixel

; bit mask not shifted out

		or		edi,edi				; test sign of d
		jg		@@InPosDi			; jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@BitMaskIn

		and		[_ES _bx],al		; set remaining pixel(s)
		jmp		@@Exit

@@InPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr

		and		[_ES _bx],al		; Update bit planes

		add		bx,[WORD vertInc]	; increment
		loop	@@StartLo
		jmp		@@Exit

; bit mask shifted out

@@BitMaskOut:
		and		[_ES _bx],al		; Update bit planes
		inc		bx					; increment x

		or		edi,edi				; test sign of d
		jg		@@OutPosDi			; jump if non-negative

		add		edi,esi				; d := d + EIncr
		loop	@@StartLo
		jmp		@@Exit

@@OutPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		bx,[WORD vertInc]	; vertical increment
		loop	@@StartLo
		jmp		@@Exit

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:_BX -> video buffer
;						AL = bit mask for 1st pixel
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
		mov		esi,[EIncr]

@@StartHi:
		and		[_ES _bx],al		; update bit planes

		add		bx,[WORD vertInc]	; increment y

		or		edi,edi				; test sign of d
		jg		@@HiPosDi			; jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@StartHi
		jmp		@@Exit

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr

		ror		al,1				; rotate bit mask
		adc		bx,0				; Increment BX when mask rotated to
									;  leftmost pixel position
		loop	@@StartHi

@@Exit:	restore_es
		call	_VGA4_endDrawing
		leave_c
		ret

procend		_VGA4_line

;----------------------------------------------------------------------------
; void VGA4_scanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
; Routine draws a scan line in native EGA graphics modes. This
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
procstart	_VGA4_scanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		LOCAL	masks:UINT, addr:UINT, count:UINT, pattern:BYTE,	\
				writeMode:UINT, color:UINT, bkColor:UINT, 			\
				penStyle:UINT = LocalSize

		enter_c	LocalSize
		cld							; String instructions go up
		save_ds
		save_es

		call	_VGA4_beginDrawing

		mov     _ax,[GDC.a.penStyle]
		mov		[penStyle],_ax
		cmp     _ax,MGL_PIXMAP
		je		@@PixmapScanline

		mov		_ax,[x2]
		cmp		[x1],_ax
		je		@@Exit				; Quit if xstart == xend
		jl		@@NoSwap			; no, don't swap them

		xchg	[x1],_ax			; Swap the coordinates
		mov		[x2],_ax

@@NoSwap:
		dec		[x2]				; Decrement xend coordinate

; Save variables out of device context into local variables

		mov		_ax,[GDC.a.writeMode]
		mov		[writeMode],_ax
		mov		_ax,[UINT GDC.intColor]
		mov		[color],_ax
		mov		_ax,[UINT GDC.intBackColor]
		mov		[bkColor],_ax

; Get the pattern byte for this scan line from pattern array

		mov		dx,3CEh				; DX := Graphics Controller port addr
		mov		_si,[y]
		and		_si,7				; Mask to 0-7
		mov		cl,[_si + GDC.a.penPat]
		mov		[pattern],cl		; Store in pattern byte

; Compute address of scan line in video buffer

		mov		_ax,[y]
		mov		_cx,[x1]
		call	VGA4_pixelAddr		; AH := bit mask
									; ES:_BX -> buffer
									; CL := # bits to shift left
		mov		[addr],_bx			; Save buffer offset for later
		ds_eq_es					; DS,ES -> VGA frame buffer

		not		ah
		shl		ah,cl				; AH := reverse bit mask for first byte
		not		ah					; AH := bit mask for first byte
		mov		al,0FFh				; AL := unshifted bit mask for rightmost byte

		mov		_cx,[x2]
		and		cl,7
		xor		cl,7				; CL := number of bits to shift left
		shl		al,cl				; AL := bit mask for last byte
		mov		[masks],_ax			; Save mask for later

; determine byte offset of first and last pixel in the line

		mov		_ax,[x2]			; _AX := X2
		mov		_bx,[x1]			; _CX := X1
		shr		_ax,3				; _AX := byte offset of X2
		shr		_bx,3				; _BX := byte offset of X1
		sub		_ax,_bx				; _AX := (# bytes in line)-1
		mov		[count],_ax			; Save for later

; If we are doing a solid pattern, or an opaque pattern, we must fill the
; scan line first with a solid color. For solid patterns, this is the
; foreground color, for opaque patterns this is the background color.

		mov		ah,[BYTE color]		; Get color value into ah
		cmp		[penstyle],MGL_BITMAP_TRANSPARENT
		je		@@DoTransparent
		cmp		[penstyle],MGL_BITMAP_SOLID
		je		@@Start

; We are doing an opaque pattern, so use the background color, not the
; foreground color.

		mov		ah,[BYTE bkColor]	; Get color value into ah

; Set pixels in the scan line.

@@Start:
		xor		al,al
		out		dx,ax				; Set the color

		mov		_bx,[masks]
		mov		_cx,[count]
		mov		_di,[addr]
		mov		_si,_di				; DS:SI -> video buffer
		mov		al,8
		out		dx,al				; Set controller to accept bitmasks
		inc		dx

		or		bh,bh
		js		@@FillMiddle		; jump if byte-aligned (X1 is leftmost
									;  pixel in byte)
		or		_cx,_cx
		jnz		@@FillStart			; Jump if more than one byte

		and		bl,bh				; BL := bit mask for the line
		jmp		@@FillEnd

@@FillStart:
		mov		al,bh				; AL := bit mask for 1st byte
		out		dx,al				; update Graphics controller
		movsb						; update bit planes
		dec		_cx

; Draw the middle section of the line

@@FillMiddle:
		jcxz	@@FillEnd
		mov		ax,0FFFFh			; AX := pattern mask
		out		dx,al				; update Bit mask Register

		cmp		[writeMode],MGL_REPLACE_MODE
		je		@@FillMiddleQuick

	rep	movsb						; update all pixels at once
		jmp		@@FillEnd

@@FillMiddleQuick:
		test	_di,1
		jz		@@WordAligned
		stosb						; Store first pixel if odd to ensure
		dec		_cx					;  word alignment for 'rep stosw'
@@WordAligned:
		shr		_cx,1
	rep	stosw						; update all pixels at once
		rcl		_cx,1
	rep	stosb
		mov		_si,_di

; set the pixel in the rightmost byte of the line

@@FillEnd:
		mov		al,bl				; AL := bit mask for last byte
		out		dx,al				; update graphics controller
		movsb						; Update the bit planes

		dec		dx					; DX := 3CEh, GC controller port

; For solid lines, we are done, otherwise do the patterned line

		cmp		[penstyle],MGL_BITMAP_SOLID
		je		@@Exit

; We are doing an opaque pattern, so use the foreground color for the
; pattern.

		mov		ah,[BYTE color]		; Get color value into ah

; Set pixels in the scan line.

@@DoTransparent:
		xor		al,al
		out		dx,ax				; Set the color

		mov		_bx,[masks]
		mov		_cx,[count]
		mov		_di,[addr]
		mov		_si,_di				; DS:SI -> video buffer
		mov		al,8
		out		dx,al				; Set controller to accept bitmasks
		inc		dx

		or		bh,bh
		js		@@FillMiddle2		; jump if byte-aligned (X1 is leftmost
									;  pixel in byte)
		or		_cx,_cx
		jnz		@@FillStart2		; Jump if more than one byte

		and		bl,bh				; BL := bit mask for the line
		jmp		@@FillEnd2

@@FillStart2:
		mov		al,bh				; AH := bit mask for 1st byte
		and		al,[pattern]
		out		dx,al				; update Graphics controller

		movsb						; update bit planes
		dec		_cx

; Draw the middle section of the line

@@FillMiddle2:
		jcxz	@@FillEnd2
		mov		al,[pattern]		; AL := pattern mask
		out		dx,al				; update Bit mask Register
	rep	movsb						; update all pixels at once

; set the pixel in the rightmost byte of the line

@@FillEnd2:
		mov		al,bl				; AH := bit mask for last byte
		and		al,[pattern]		; Mask with pattern byte
		out		dx,al				; update graphics controller
		movsb						; Update the bit planes


@@Exit:	mov		dx,3CEh
		mov		ax,0FF08h			; Set default pattern mask
		out		dx,ax

		restore_es
		restore_ds
		call	_VGA4_endDrawing
		leave_c
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

procend		_VGA4_scanLine

;----------------------------------------------------------------------------
; void VGA4_fillRect(int x1,int y1,int x2,int y2)
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
; This routine scan converts only solid filled rectangles. If the current
; write mode operation is replace, we use the fast REP STOSB instruction to
; fill the middle of the rectangle, otherwise we use the slower REP MOVSB
; instruction which works for XOR, AND and OR mode operation. To fill
; rectangles in transparent and opaque bitmap patterns and pixmap patterns,
; we call the scanLine routine.
;----------------------------------------------------------------------------
procstart	_VGA4_fillRect

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT

		LOCAL	masks:UINT, addr:UINT, count:UINT,				\
				bytesPerLine:UINT, writeMode:UINT = LocalSize

		enter_c	LocalSize
		cld
		save_ds
		save_es

		cmp		[GDC.a.penStyle],MGL_BITMAP_SOLID
		je		@@FastFill

;---------------------------------------------------------------------------
; If we are filling with anything but a solid pattern, then repeatedly
; call the scanline routine to draw the rectangle.

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
		jmp		@@Exit

;---------------------------------------------------------------------------
; We are filling with a solid pattern, so do it as quickly as possible

@@FastFill:
		mov		_ax,[GDC.mi.bytesPerLine]
		mov		[bytesPerLine],_ax	; Save bytes per line value
		mov		_ax,[GDC.a.writeMode]
		mov		[writeMode],_ax

		mov		dx,3CEh				; DX := Graphics Controller port addr

		mov		ah,[BYTE GDC.intColor]
		xor		al,al				; AL := Set/Reset Register number
		out		dx,ax

		mov		ax,0005h			; AH := 0, AL := 5
		out		dx,ax				; default mode register

		mov		ax,0F01h			; AH := 1111b (bit plane mask for
									;  enable Set/Reset)
		out		dx,ax				; AL := Enable Set/Reset Register #

		mov		ah,[BYTE writeMode]
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		dec		[x2]
		dec		[y2]				; Adjust endpoints

		mov		_ax,[y1]
		mov		_cx,[x1]
		call	VGA4_pixelAddr		; AH := bit mask
									; ES:BX -> buffer
									; CL := # bits to shift left
		mov		[addr],_bx			; Save buffer offset for later
		ds_eq_es					; DS,ES -> VGA frame buffer

		not		ah
		shl		ah,cl				; AH := reverse bit mask for first byte
		not		ah					; BH := bit mask for first byte
		mov		al,0FFh				; BL := unshifted bit mask for rightmost byte

		mov		_cx,[x2]
		and		cl,7
		xor		cl,7				; CL := number of bits to shift left
		shl		al,cl				; AL := bit mask for last byte
		mov		[masks],_ax			; Save mask for later

; determine byte offset of first and last pixel in the scanlines

		mov		_ax,[x2]			; AX := X2
		mov		_bx,[x1]			; BX := X1

		shr		_ax,3				; AX := byte offset of X2
		shr		_bx,3				; BX := byte offset of X1
		sub		_ax,_bx				; AX := (# bytes in line)-1
		mov		[count],_ax			; Save for later

		mov		_si,[y2]
		sub		_si,[y1]
		inc		_si					; SI = number of scan lines to fill

; Fill the rectangle

		mov		_bx,[masks]
		mov		al,8				; AL := Bit mask register number

		mov		_cx,[count]
		or		bh,bh
		js		@@FillMiddle		; jump if byte-aligned (X1 is leftmost
									;  pixel in byte)
		or		_cx,_cx
		jnz		@@FillStart			; Jump if more than one byte

		and		bl,bh				; BL := bit mask for the line
		jmp		@@FillEnd

; Fill the first leading partial strip of bytes for the rectangle

@@FillStart:
		mov		ah,bh				; AH := bit mask for 1st byte
		out		dx,ax				; update Graphics controller
		mov		_cx,_si				; CX := number of scan lines
		mov		_di,[addr]			; DS:DI -> buffer address
		mov		_bx,[bytesPerLine]

@@LoopStart:
		and		[_di],ah
		add		_di,_bx				; Increment to next scan line
		loop	@@LoopStart

; Set up for the middle section

		mov		_cx,[count]			; Tick off this byte and ...
		dec		_cx
		inc		[addr]				; ... move to next one

; Draw the middle section of the rectangle

@@FillMiddle:
		mov		ah,0FFh				; AH := pattern mask
		out		dx,ax				; update Bit mask Register
		mov		_dx,[bytesPerLine]
		mov		_bx,_si				; BX := number of scan lines
		mov		_di,[addr]

		cmp		[writeMode],MGL_REPLACE_MODE
		je		@@FillMiddleQuick

; Fill the section a little slower, but allowing the use of XOR mode etc.

		push	_si					; Save number of scan lines

@@LoopMiddle1:
		mov		_si,_di				; ES:DI, DS:SI -> video buffer
		push	_cx					; Save number of bytes to set
		push	_di					; Save buffer offsets

	rep	movsb						; update all pixels at once

		pop		_di
		add		_di,_dx				; Increment to next scan line
		pop		_cx
		dec		_bx
		jg		@@LoopMiddle1

		pop		_si
		jmp		@@SetupTrailing

@@FillMiddleQuick:
		push	_cx					; Save number of bytes to set
		push	_di					; Save buffer offsets
		jcxz	@@Nop				; Do nothing if CX := 0
		test	_di,1
		jz		@@WordAligned
		stosb						; Store first pixel if odd to ensure
		dec		_cx					;  word alignment for 'rep stosw'
@@WordAligned:
		shr		_cx,1
	rep	stosw						; update all pixels at once
		rcl		_cx,1
	rep	stosb

@@Nop:
		pop		_di
		add		_di,_dx				; Increment to next scan line
		pop		_cx
		dec		_bx
		jg		@@FillMiddleQuick

; Set up for trailing bytes

@@SetUpTrailing:
		add		[addr],_cx			; Move address to last byte
		mov		_bx,[masks]			; Put masks in bx

; set the pixel in the rightmost bytes of the rectangle

@@FillEnd:
		mov		ah,bl				; AH := bit mask for last byte
		mov		dx,3CEh
		out		dx,ax				; update Graphics controller
		mov		_cx,_si				; CX := number of scan lines
		mov		_di,[addr]			; DS:DI -> buffer address
		mov		_bx,[bytesPerLine]

@@LoopEnd:
		and		[_di],ah
		add		_di,_bx				; Increment to next scan line
		loop	@@LoopEnd

; Restore graphics controller and return to caller

@@Exit:	restore_es
		restore_ds
		call	_VGA4_endDrawing
		leave_c
		ret

procend		_VGA4_fillRect

;----------------------------------------------------------------------------
; void VGA4_putMonoImage(MGLDC *dc,int x,int y,int width,int height,
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
procstart	_VGA4_putMonoImage

		ARG		dc:DPTR, x:UINT, y:UINT, byteWidth:UINT, height:UINT,		\
				image:DPTR

		LOCAL	lmask:S_UCHAR, rmask:S_UCHAR, bytesPerLine:UINT, 				\
				cleft:UINT, ctop:UINT, cright:UINT, cbottom:UINT,			\
				rowWidth:UINT, rowCount:UINT, rowAdjust:UINT,				\
				count:UINT, color:S_UCHAR = LocalSize

		enter_c	LocalSize
		cld
		save_ds
		save_es

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

; Setup graphics controller registers

		mov		dx,03CEh			; Graphics controller port address

		mov		ax,0B05h			; AH := 0B (read mode 1, write mode 3)
									; AL := 5 (mode register number)
		out		dx,ax

		mov		ah,[BYTE color]		; Get color value into ah
		xor		al,al				; AL := Set/Reset Register number
		out		dx,ax

		mov		ah,[BYTE MGLDC.a.writeMode]
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

		mov		ax,0007h			; AH := Color don't care bits
									; AL := Color don't care register
		out		dx,ax				; "don't care" for all bit planes

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
		jmp		@@DrawNonClipped

;----------------------------------------------------------------------------
; Routine for clipped monochrome images.
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
		call	VGA4_pixelAddrESBX	; ES:_BX -> buffer
		inc		cl
		mov		_di,_bx				; ES:_DI -> video buffer
		_lds	_si,[image]			; DS:_SI -> image buffer

		mov		_ax,[rowWidth]
		mov		[count],_ax			; Save count of bytes per row
		sub		_ax,[byteWidth]
		neg		_ax
		mov		[rowAdjust],_ax		; Save adjustment between rows
		mov		dx,3CFh				; DX := GC Data Reg port
		cmp		[BYTE count],1
		jne		@@NextRowClipped

		mov		al,[rmask]			; Combine left and right masks if clipped
		and		al,[lmask]			;  image is only 1 byte wide
		mov		[lmask],al
		mov		[rmask],al

@@NextRowClipped:
		push	_di					; Save buffer offset
		mov		bh,[BYTE count]		; BH := count of bytes to draw

; Plot the first partial 8 pixels from the first byte to be loaded

		mov		ch,[lmask]			; CH := left clipping mask
		mov		ah,[_si]
		and		ah,ch				; Clip with left clip mask
		xor		al,al
		rol		ax,cl				; AL := bits to set in first byte
		and		[_ES _di],al		; Set the pixels
		inc		di
		dec		bh
		jz		@@DoEndClipped		; Jump to end if only 1 byte in row

		dec		bh
		jz		@@DoSecondLast		; Jump if only 2 bytes in row

@@DoMiddleClipped:
		lods	[WORD _si]			; AX := pattern for next 8 pixels
		and		al,ch				; Clip with left clip mask
		dec		_si
		rol		ax,cl				; AL := bits to set for this byte
		and		[_ES _di],al		; Set the pixels
		inc		di
		mov		ch,0FFh				; Reset left clip mask
		dec		bh
		jnz		@@DoMiddleClipped	; Loop for all pixels in middle section

; Plot the last-1 byte in the row

@@DoSecondLast:
		lods	[WORD _si]
		dec		_si
		and		ah,[rmask]
		and		al,ch				; Clip with left mask
		rol		ax,cl				; AL := bits to set in last-1 byte
		and		[_ES _di],al
		inc		di

; Plot the last partial 8 pixels in the row

@@DoEndClipped:
		lods	[BYTE _si]			; AL := pattern for last 8 pixels
		and		al,[rmask]			; Clip with right clip mask
		shl		_ax,cl				; AL := bits to set for last byte
		and		[_ES _di],al

		pop		_di
		add		di,[WORD bytesPerLine]; Increment to next row in buffer
		add		_si,[rowAdjust]		; Increment to next line in image
		dec		[rowCount]
		jnz		@@NextRowClipped	; Loop for all rows in image
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
		call	VGA4_pixelAddrESBX	; ES:_BX -> buffer
		inc		cl
		and		cl,7				; CL := # bits to shift left
		mov		_di,_bx				; ES:_DI -> video buffer
		_lds	_si,[image]			; DS:_SI -> image bit map
		mov		_ax,[bytesPerLine]
		sub		_ax,[byteWidth]		; AX := buffer adjustment to next row
		mov		[rowAdjust],_ax		; Save buffer adjustment for later
		mov		dx,3CFh				; DX := GC Data Reg port

; Select output routine depending on whether image is byte-aligned

		or		cl,cl				; test if byte aligned
		jz		@@DoAligned			; Jump if image is byte aligned

; Routine for non-byte aligned images.

		dec		[rowAdjust]			; Adjust row adjustment by 1

@@StartNonAligned:
		mov		bh,[BYTE byteWidth]	; BH := count of bytes to output

; Plot the first partial 8 pixels from the first byte to be loaded

		mov		ah,[_si]
		xor		al,al
		rol		ax,cl				; AL := bits to set in first byte
		and		[_ES _di],al		; Set the pixels
		inc		_di
		dec		bh
		jz		@@DoEnd				; Jump to end if only 1 byte in row

@@DoMiddle:
		lods	[WORD _si]			; AX := pattern for next 8 pixels
		dec		_si
		rol		ax,cl				; AL := bits to set for this byte
		and		[_ES _di],al		; Set the pixels
		inc		_di
		dec		bh
		jnz		@@DoMiddle			; Loop for all pixels in middle section

; Plot the last partial 8 pixels in the row

@@DoEnd:
		lods	[BYTE _si]			; AL := pattern for last 8 pixels
		shl		ax,cl				; AL := bits to set for last byte
		and		[_ES _di],al
		inc		_di

		add		_di,[rowAdjust]		; increment to next line in video buffer
		dec		[height]
		jnz		@@StartNonAligned	; Loop for all rows in image
		jmp		@@Exit

; Routine for byte aligned images.

@@DoAligned:
		mov		_cx,[height]		; CX := number of rows to process
		mov		bl,[color]			; BL := color value to set

@@StartAligned:
		mov		bh,[BYTE byteWidth]	; BH := count of bytes to output

@@NextByte:
		lodsb						; AL := next byte of image data
		and		[_ES _di],al		; Set the pixels
		inc		_di
		dec		bh
		jnz		@@NextByte
		add		_di,[rowAdjust]		; increment to next line in video buffer
		loop	@@StartAligned		; Loop for all rows in image

; Restore default graphics controller state

@@Exit:	restore_es
		restore_ds
		call	_VGA4_endDrawing
		leave_c
		ret

procend		_VGA4_putMonoImage

;----------------------------------------------------------------------------
; void VGA4_getDivot(MGLDC *dc,int left,int top,int right,int bottom,
;	void *divot)
;----------------------------------------------------------------------------
; Routine to get a block of video information from the current display
; page. This routine extends the block to a byte aligned boundary.
;
; Entry:		dc		- Device context to read from
;				left	- Left coordinate of image
;				top		- Top coordinate of image
;				right	- Right coordinate of image
;				bottom	- Bottom coordinate of image
;				divot	- Pointer to place to store image data
;
; Copies a block of data from the currently active display page into a
; buffer in system memory.
;
; We build a small header for the block containing information
; about the data in the block. The structure of this header is:
;
; struct {
;	short	bytesPerRow;			- Width of image in bytes
;	short	Rows;					- Number of rows in image
;	short	StartOffset;			- Starting offset in video ram
;	} divotHeader;
;
;----------------------------------------------------------------------------
procstart	_VGA4_getDivot

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				divot:DPTR

		LOCAL	PixelRows:UINT, StartOffset:UINT, BytesPerRow:UINT,		\
				BytesPerLine:UINT, BufferAdjust:UINT = LocalSize

		enter_c	LocalSize
		cld
		save_ds
		save_es

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov		_dx,[MGLDC.mi.bytesPerLine]
		mov		[BytesPerLine],_dx	; Save bytes per line value on stack
		mov		_ax,[top]
		mul		_dx					; AX := top * BytesPerLine
		mov		_si,[left]
		shr		_si,3				; SI := left / 8
		mov		_bx,[right]
		dec		_bx					; Adjust right coordinate
		shr		_bx,3				; BX := right / 8
		sub		_bx,_si				; BX := BytesPerRow-1
		inc		_bx					; BX := BytesPerRow
		mov		[BytesPerRow],_bx	; Save bytes per row value
		sub		_bx,[BytesPerLine]
		neg		_bx					; BX := BytesPerLine - BytesPerRow
		mov		[BufferAdjust],_bx
		add		_si,_ax				; SI := top * BytesPerLine + left / 8
		mov		[StartOffset],_si	; Save start offset value

		_les	_bx,[dc]
ifdef	USE_SELVMEM
		add		si,[USHORT MGLDC.surface]
		mov		ds,[WORD (MGLDC.surface)+2]
else
		add		_si,[UINT MGLDC.surface]
endif
ife flatmodel
		mov		ds,[WORD (MGLDC.surface)+2]
endif

		mov		_cx,[bottom]
		sub		_cx,[top]			; CX := rows in image
		mov		[PixelRows],_cx		; Save number of rows

		_les	_di,[divot]			; ES:_DI -> buffer in system RAM

; Build the bit block header

		mov		_ax,[BytesPerRow]
		stosw						; bytes per pixel row
		mov		_ax,[PixelRows]
		stosw						; number of rows
		mov		_ax,[StartOffset]
		stosw						; Starting offset in page

; Set up graphics controller

		mov		dx,3CEh				; DX := Graphics Controller address

		mov		ax,0005h			; AH := 0 (read mode 0,write mode 0)
									; AL := 5 (Mode register number)
		out		dx,ax				; set mode

		mov		ax,0304h			; AH := 3 (first bit plane to read)
									; AL := 4 (Read Map Select reg number)
		mov		_bx,[BufferAdjust]

; Copy from video buffer to system RAM

@@NextPlane:
		out		dx,ax				; Select next memory map to read
		push	[PixelRows]			; Preserve rows in image
		push	_si					; preserve offset of (left,top)

@@NextRow:

ife	flatmodel

; Now check to see if we are about to go off the end of the 64k system
; RAM segment, and adjust the segment if we are...

		mov		cx,di
		add		cx,[BytesPerRow]
		jnc		@@NoWrap			; No wrap, so continue

; The pointer will wrap while reading this row, so adjust the segment
; number and normalise the pointer.

		push	bx
		mov		cx,di
		shr		cx,4				; AX := paragraph's in segment
		mov		bx,es
		add		bx,cx
		shl		cx,4
		sub		di,cx				; Adjust offset
		mov		es,bx				; ES:DI -> new system RAM segment
		pop		bx

endif

@@NoWrap:
		mov		_cx,[BytesPerRow]
	rep	movsb						; Move all data in row FAST!
		add		_si,_bx				; Increment to next position in video mem

		dec		[PixelRows]
		jnz		@@NextRow			; Loop for all rows in each bit plane

		pop		_si					; DS:SI -> start of bit block
		pop		[PixelRows]			; Restore number of pixel rows
		dec		ah
		jns		@@NextPlane			; Loop across bit planes

@@Exit:	restore_es
		restore_ds
		leave_c
		ret

procend		_VGA4_getDivot

;----------------------------------------------------------------------------
; void VGA4_putDivot(MGLDC *dc,void *divot)
;----------------------------------------------------------------------------
; Routine to copy a block of video information from system memory to the
; currently active display page. The block is a special byte aligned piece
; of memory and is returned to the same position it was saved from.
;
; Entry:		dc		- Device context to dump divot to
;				divot	- Pointer to place to store image data
;
;----------------------------------------------------------------------------
procstart	_VGA4_putDivot

		ARG		dc:DPTR, divot:DPTR

		LOCAL	PixelRows:UINT, BytesPerRow:UINT, BytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		cld
		save_ds
		save_es

		_les	_bx,[dc]
		mov		_dx,[MGLDC.mi.bytesPerLine]
		mov		[BytesPerLine],_dx	; Save bytes per line value on stack
		_lds	_si,[divot]			; DS:_SI -> buffer in system RAM

; Extract position info from image header

		xor		_ax,_ax
		lodsw
		mov		[BytesPerRow],_ax
		sub		_ax,[BytesPerLine]
		neg		_ax
		push	_ax					; Save buffer adjustment
		lodsw
		mov		[PixelRows],_ax
		lodsw
		lVid	_di
		add		_di,_ax				; ES:_DI -> video buffer
		pop		_bx					; BX := BytesPerLine - BytesPerRow

; Set up graphics controller

		mov		dx,3CEh				; DX := Graphics Controller address

		mov		ax,0005h			; AH := 0 (read mode 0,write mode 0)
									; AL := 5 (Mode register number)
		out		dx,ax				; set mode

		mov		dl,0C4h				; DX := 3C4h (sequencer port)
		mov		ax,0802h			; AH := 1000b (value for Map Mask reg)
									; AL := 2 (Map Mask register number)

; Copy from video buffer to system RAM

@@NextPlane:
		out		dx,ax				; Enable one bit plane for writes
		push	[PixelRows]			; Preserve rows in image
		push	_di					; preserve offset of (left,top)

@@NextRow:

ife flatmodel

; Now check to see if we are about to go off the end of the 64k system
; RAM segment, and adjust the segment if we are...

		mov		cx,si
		add		cx,[BytesPerRow]
		jnc		@@NoWrap			; No wrap, so continue

; The pointer will wrap while writing this row, so adjust the segment
; number and normalise the pointer.

		push	bx
		mov		cx,si
		shr		cx,4				; AX := paragraph's in segment
		mov		bx,ds
		add		bx,cx
		shl		cx,4
		sub		si,cx				; Adjust offset
		mov		ds,bx				; DS:SI -> new system RAM segment
		pop		bx

endif

@@NoWrap:
		mov		_cx,[BytesPerRow]
	rep	movsb						; Move all data in row FAST!
		add		_di,_bx				; Increment to next position in video mem

		dec		[PixelRows]
		jnz		@@NextRow			; Loop for all rows in each bit plane

		pop		_di					; DS:SI -> start of bit block
		pop		[PixelRows]			; Restore number of pixel rows
		shr		ah,1				; AH := new Map Mask value
		jnz		@@NextPlane

; Restore Graphics controller state

		mov		ax,0F02h			; default Map Mask value
		out		dx,ax

		restore_es
		restore_ds
		leave_c
		ret

procend		_VGA4_putDivot

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; void VGA4_drawScanList(int y,int length,short *scans);
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
procstart	_VGA4_drawScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		enter_c	0
		cld							; String instructions go up
		save_ds
		save_es

		call	_VGA4_beginDrawing

; Load the current drawing color

		mov		dx,3CEh				; DX := Graphics Controller port addr
		mov		ah,[BYTE GDC.intColor]; AH := foreground color
		xor		al,al				; AL := Set/Reset register number
		out		dx,ax

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
		mov		_cx,[len]			; CX := number of scanlines to fill
		imul	[GDC.mi.bytesPerLine]	; AX := y * BytesPerLine
ifdef	USE_SELVMEM
		add		ax,[USHORT GDC.surface]
else
		add		_ax,[UINT GDC.surface]
endif
		mov		_di,_ax

; Get the address of the scanline array in DS:SI

		_les	_si,[scans]			; ES:_SI -> scanline array data

; Put BytesPerLine value into BP for fast access, trashing the stack
; frame pointer (we don't need it anymore).

		mov		_bp,[GDC.mi.bytesPerLine]
ifdef	USING_DS
		mov		ax,[WORD (GDC.surface) + 2]
		mov     bx,es
		mov		ds,bx				; DS:SI -> scanline array data
		mov		es,ax				; ES:DI -> first scanline in video buffer
endif

@@FillLoop:

; Check that xstart < xend, swapping if need be.

		lodsw
		clrhi	ax
		mov		_bx,_ax				; BX := xstart for scanline
		lodsw						; AX := xend for scanline
		clrhi	ax
		cmp		_bx,_ax				; is xstart > xend?
		jl		@@NoSwap			; no, don't swap them
		jg		@@Swap				; yes, swap them
		jmp		@@SkipLine			; ignore if xstart == xend

@@Swap:
		xchg	_ax,_bx

@@NoSwap:
		dec		_ax					; Decrement the xend coordinate
		push	_cx					; Preserve CX
		push	_di					; Preserve DI

; Build bit mask for first and last bytes in the line

		mov		dx,0FFFFh			; DX := unshifted bit masks
		mov		cl,bl				; CL := low order byte of xstart
		and		cl,7				; CL := number of bits to shift right
		shr		dh,cl				; DH := bit mask for first byte

		mov		cl,al				; CL := low order byte of xend
		and		cl,7
		xor		cl,7				; CL := number of bits to shift left
		shl		dl,cl				; DL := bit mask for last byte

; determine byte offset of first and last pixel in the line

		shr		_ax,3				; AX := byte offset of xend
		shr		_bx,3				; BX := byte offset of xstart
		add		_di,_bx				; ES:_DI -> first byte in buffer
		xchg	_bx,_dx				; BX := bitmasks for scanline
		mov		_cx,_ax
		sub		_cx,_dx				; CX := (# bytes in line)-1

		je		@@OneByte			; Jump if only one byte

		and		[_ES _di],bh		; Update first byte
		inc		_di
		dec		_cx
		jcxz	@@LastByte
		mov		al,0FFh				; Solid for all middle ones
	rep	stosb						; Draw all middle pixels

@@LastByte:
		and		[_ES _di],bl		; Update last byte
		jmp		@@DoneLine

@@OneByte:
		and		bl,bh
		and		[_ES _di],bl

@@DoneLine:
		pop		_di					; Restore DI
		pop		_cx					; Restore CX

@@SkipLine:
		add		_di,_bp				; Increment address to next scanline
		loop	@@FillLoop			; Loop for all scanlines
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
		restore_ds
		call	_VGA4_endDrawing
		leave_c_nolocal
		ret

procend		_VGA4_drawScanList

;----------------------------------------------------------------------------
; void VGA4_ellipse(int left,int top,int A,int B,int clip);
;----------------------------------------------------------------------------
; Routine draws an Ellipse in native EGA graphics modes.
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
procstart	_VGA4_ellipse

		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT,			\
				clip:UINT

		LOCAL	ULaddr:UINT, URaddr:UINT, LLaddr:UINT, LRaddr:UINT,	\
				VARd:ULONG, VARdx:ULONG, VARdy:ULONG,				\
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

		mov		dx,3CEh				; DX := Graphics Controller port addr

		mov		ax,0B05h			; AL := Mode register number
									; AH := Write mode 3 (bits 0,1)
									;	Read mode 1 (bit 3)
		out		dx,ax

		mov		ax,0007h			; AH := 0 (don't care for all maps;
									;	CPU reads always return 0FFH)
									; AL := 7 (Color Don't care reg number)
		out		dx,ax				; Set up Color Don't care reg

		mov		ah,[BYTE GDC.a.color]
		xor		al,al				; AL := Set/Reset Register number
		out		dx,ax

		mov		ax,0F01h			; AH := 1111b (bit plane mask for
									;  enable Set/Reset)
		out		dx,ax				; AL := Enable Set/Reset Register #

		mov		ah,[BYTE GDC.a.writeMode]
		shl		ah,3				; Put in correct position
		mov		al,3				; AL := Data Rotate/Func select reg #
		out		dx,ax

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
		call	VGA4_pixelAddr		; AH := bit mask
									; ES:BX -> buffer
									; CL := # bits to shift left
		shl		ah,cl				; AH := bit mask for first pixel
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
		ror		ah,1				; AH := bit mask rotated horizontally
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
		call	_VGA4_endDrawing
		leave_c
		ret

procend		_VGA4_ellipse

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
		push	_si
		push	_di

		xor		_bx,_bx				; BX := 0
		test	ch,ch
		jz		@@NoYinc			; jump if y-increment = 0

		mov		_bx,[bytesPerLine]	; BX := positive y increment

@@NoYinc:

; Plot the Upper Left and Lower Left pixels

		xor		_si,_si				; SI := 0
		mov		ah,[LMask]

		rol		ah,cl				; AH := bit mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around
		neg		_si					; SI := 0 or -1

		mov		_di,_si				; SI,DI := left horizontal increment

		add		_si,[ULaddr]		; SI := upper left addr + horiz incr
		add		_si,_bx				; SI := new upper left addr
		add		_di,[LLaddr]
		sub		_di,_bx				; DI := new lower left addr

		mov		[LMask],ah			; Update these variables
		mov		[ULaddr],_si
		mov		[LLaddr],_di

		and		[_ES _si],ah		; Update upper left pixel
		and		[_ES _di],ah		; Update lower left pixel

; Plot the Upper Right and Lower Right pixels

		xor		_si,_si				; SI := 0
		mov		ah,[RMask]

		ror		ah,cl				; AH := bit mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around

		mov		_di,_si				; SI,DI := right horizontal increment

		add		_si,[URaddr]		; SI := upper right addr + horiz incr
		add		_si,_bx				; SI := new upper right addr
		add		_di,[LRaddr]
		sub		_di,_bx				; DI := new lower right addr

		mov		[RMask],ah			; Update these variables
		mov		[URaddr],_si
		mov		[LRaddr],_di

		and		[_ES _si],ah		; Update upper right pixel
		and		[_ES _di],ah		; Update lower right pixel

		pop		_di
		pop		_si
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
		xor		_si,_si				; SI := 0
		mov		ah,[LMask]

		rol		ah,cl				; AH := bit mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around
		neg		_si					; SI := 0 or -1

		mov		_di,_si				; SI,DI := left horizontal increment

		add		_si,[ULaddr]		; SI := upper left addr + horiz incr
		add		_si,_bx				; SI := new upper left addr
		add		_di,[LLaddr]
		sub		_di,_bx				; DI := new lower left addr

		mov		[LMask],ah			; Update these variables
		mov		[ULaddr],_si
		mov		[LLaddr],_di

		mov		dh,ah

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

		and		[_ES _si],dh		; Update upper left pixel

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

		and		[_ES _di],dh		; Update lower left pixel

; Plot the Upper Right and Lower Right pixels

@@UpperRight:
		xor		_si,_si				; SI := 0
		mov		ah,[RMask]

		ror		ah,cl				; AH := bit mask rotated horizontally
		rcl		_si,1				; SI := 1 if bit mask rotated around

		mov		_di,_si				; SI,DI := right horizontal increment

		add		_si,[URaddr]		; SI := upper right addr + horiz incr
		add		_si,_bx				; SI := new upper right addr
		add		_di,[LRaddr]
		sub		_di,_bx				; DI := new lower right addr

		mov		[RMask],ah			; Update these variables
		mov		[URaddr],_si
		mov		[LRaddr],_di

		mov		dh,ah

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

		and		[_ES _si],dh		; Update upper right pixel

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

		and		[_ES _di],dh		; Update lower right pixel

@@Exit:
		pop		_di
		pop		_si
		pop		_dx
		pop		_bx
		pop		_ax
		ret

ENDP	Clip4Pixels

;----------------------------------------------------------------------------
; void VGA4_alignedBitBlt(MGLDC *dc,int left8,int top,int right8,int bottom,
;	int dstLeft8,int dstTop,int incx,int incy,int op)
;----------------------------------------------------------------------------
; Entry:	dc			- Device context
; 			left8		- Left coordinate of are to Blt from (byte aligned)
;			top			- Top coordinate of image to Blt from
;			right		- Right coordinate of image to Blt from (byte aligned)
;			bottom		- Bottom coordinate of image to Blt from
;			dstLeft8	- Left coordinate to Blt to (byte aligned)
;			dstTop		- Right coordinate to Blt to
;			incx		- Do we increment or decrement in x?
;			incy		- Do we increment or decrement in y?
;			op			- Write mode to use during Blt
;
; Blts a block of image data from one place on the device context to
; another place on the device context. The values of 'incy' will
; have been set by the high level code to ensure that the transfer will take
; place correctly for overlapping regions.
;
; This routine is optimised for blitting areas of the display that are
; byte aligned for both the left and right coordinates. For this case, we
; can perform the blt very quickly by utilising the VGA latches as a fast
; transfer buffer.
;----------------------------------------------------------------------------
procstart	_VGA4_alignedBitBlt

		ARG		dc:DPTR, left8:UINT, top:UINT, right8:UINT, bottom:UINT,	\
				dstLeft8:UINT, dstTop:UINT, incx:UINT, incy:UINT, op:UINT

		LOCAL	BytesPerLine:UINT, BufferAdjust:UINT = LocalSize

		enter_c	LocalSize
		save_ds
		save_es

; Set up graphics controller

		mov		dx,3CEh				; DX := Graphics Controller address port
		mov		ax,0105h			; AH := 1 (read mode 0, write mode 1)
									; AL := 5 (Mode register number)
		out		dx,ax				; Set up mode

; Establish image buffer addressing

		_les	_bx,[dc]
		mov		_ax,[MGLDC.mi.bytesPerLine]
		mov		[BytesPerLine],_ax	; Save bytes per line value
		mov		_ax,[top]
		mov		_cx,[left8]
		call	VGA4_pixelAddrESBX	; ES:_BX -> first source pixel
		mov		_si,_bx				; ES:_SI -> first source pixel
		_les	_bx,[dc]
		mov		_ax,[dstTop]
		mov		_cx,[dstLeft8]
		call	VGA4_pixelAddrESBX	; ES:_BX -> first destination pixel
		mov		_di,_bx				; ES:_DI -> first destination pixel
		ds_eq_es					; DS,ES -> VGA frame buffer

		mov		_dx,[right8]
		sub		_dx,[left8]
		shr		_dx,3				; DX := bytes per row
		mov		_bx,[bottom]
		sub		_bx,[top]			; BX := # pixel rows

		mov		cl,[BYTE incx]		; CL := increment x?
		mov		ch,[BYTE incy]		; CH := increment y?

		mov		_ax,[BytesPerLine]
		mov		[BufferAdjust],_ax
		sub		[BufferAdjust],_dx	; BufferAdjust := buffer adjust for row

		cld							; Clear direction flag (moves go up)
		or		cl,cl
		jnz		@@IncX

; Adjust to decrement the x values, rather than increment them

		std							; Change direction flag (moves go down)
		add		_si,_dx				; DS:_SI -> last pixel in source row
		dec		_si
		add		_di,_dx				; ES:_DI -> last pixel in dest row
		dec		_di
		mov		_ax,_dx
		add		_ax,[BytesPerLine]
		mov		[BufferAdjust],_ax

@@IncX:
		or		ch,ch
		jnz		@@IncY

; Adjust to decrement the y values, rather than increment them

		mov		[BufferAdjust],_dx
		or		cl,cl				; Incrementing X?
		jz		@@1
		neg		[BufferAdjust]		; Yes, so adjust starting count
@@1:	mov		_ax,[BytesPerLine]
		sub		[BufferAdjust],_ax
		mov		_ax,_bx
		dec		_ax					; AX := height-1
		push	_dx
		mul		[BytesPerLine]		; AX := BytesPerLine * (height-1)
		pop		_dx
		add		_si,_ax				; DS:_SI -> last row in source
		add		_di,_ax				; ES:_DI -> last row in dest

@@IncY:
		push	_bp
		mov		_bp,[BufferAdjust]	; BP := buffer adjustment

@@NextRow:
		mov		_cx,_dx				; CX := # pixels in row
	rep	movsb						; Move last byte if it exists
		add		_si,_bp				; Increment to next row in video buffer
		add		_di,_bp
		dec		_bx
		jnz		@@NextRow			; Loop for all rows in image

; Restore default graphics controller state

@@Exit:	pop		_bp
		mov		dx,3CEh				; DX := Graphics Controller address port
		mov		ax,0005h			; default mode register value
		out		dx,ax

		restore_es
		restore_ds
		leave_c
		ret

procend		_VGA4_alignedBitBlt

;----------------------------------------------------------------------------
; void VGA4_readImage(MGLDC *dc,int left,int top,int right,int bottom,
;	void *image)
;----------------------------------------------------------------------------
; Routine to get a block of video information from the current display
; page. This image is used by the high level bitBlt code to move data
; between display memory locations via a system memory buffer as fast as
; possible without clipping.
;
; Entry:	dc		- Device context to draw into
;			left	- Left coordinate of image
;			top		- Top coordinate of image
;			right	- Right coordinate of image
;			bottom	- Bottom coordinate of image
;			image	- Pointer to place to store image data
;
; Copies a block of data from the currently active display page into a
; buffer in system memory.
;
; We build a small 6 byte header for the block containing information
; about the data in the block. The structure of this header is:
;
;		dw		PixelRows		- Number of rows in image
;		dw		BytesPerRow		- Width of image in bytes
;		db		Filler
;		db		LastByteMask	- Bit mask for last byte in row
;
;----------------------------------------------------------------------------
procstart	_VGA4_readImage

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				image:DPTR

		LOCAL	PixelRows:UINT, PixelRowLen:UINT, BytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		cld
		save_ds
		save_es

; Compute dimensions of bit block

		mov		_ax,[right]
		dec		_ax					; Adjust right coordinate
		sub		_ax,[left]
		mov		cx,0FF07h			; CH := unshifted bit mask
									; CL := AND mask for AL
		and		cl,al				; CL := number of pixels in last byte
									;	of row
		xor		cl,7				; CL := number of bits to shift
		shl		ch,cl				; CH := bit mask for last byte of row
		mov		cl,ch
		push	_cx					; save on stack

		shr		_ax,3
		inc		_ax					; AX := number of bytes per row
		push	_ax					; save on stack

		mov		_ax,[bottom]
		sub		_ax,[top]			; AX := number of pixel rows
		push	_ax					; save on stack

; Establish addressing

		_les	_bx,[dc]
		mov		_ax,[MGLDC.mi.bytesPerLine]
		mov		[BytesPerLine],_ax	; Save bytes per line value on stack
		mov		_ax,[top]
		mov		_cx,[left]
		call	VGA4_pixelAddrESBX	; ES:_BX -> video buffer
		xor		cl,7				; CL := number of bits to shift left
		mov		_si,_bx				;
		ds_eq_es					; DS:_SI -> video buffer

		_les	_di,[image]			; ES:_DI -> buffer in system RAM

; Build the 6 byte bit block header

		pop		_ax
		mov		[PixelRows],_ax
		stosw						; Byte 0-1 := number of pixel rows
		pop		_ax
		mov		[PixelRowLen],_ax
		stosw						; Byte 4-5 := bytes per pixel row
		pop		_ax
		mov		ch,al				; CH := bit mask for last byte in row
		stosb
		stosb

; Set up graphics controller

		mov		dx,3CEh				; DX := Graphics Controller address

		mov		ax,0005h			; AH := 0 (read mode 0,write mode 0)
									; AL := 5 (Mode register number)
		out		dx,ax				; set mode

		mov		al,4				; AL := 4 (Read Map Select reg number)
		out		dx,al				; Program plane mask register
		inc		dx
		mov		al,3				; AH := 3 (first bit plane to read)

; Copy from video buffer to system RAM

@@NextPlane:
		out		dx,al				; Select next memory map to read
		push	_ax					; preserve memory map number
		push	[PixelRows]			; preserve number of pixel rows
		push	_si					; preserve offset of (left,top)

@@NextRow:
		mov		_bx,[PixelRowLen]
		push	_si					; Preserve SI at start of pixel row

@@NextByte:
		lodsw						; AL := next byte in video buffer
									; AH := (next bytes) + 1
		dec		_si					; DS:SI -> (next byte) + 1
		rol		ax,cl				; AL := next 4 pixels in row
		stosb						; copy to system RAM
		dec		_bx					; loop across row
		jnz		@@NextByte

		and		[_ES _di-1],ch		; mask last byte in row
		pop		_si					; DS:SI -> start of row
		add		_si,[BytesPerLine]	; DS:SI -> start of next row

		dec		[PixelRows]
		jnz		@@NextRow			; loop down rows

		pop		_si					; DS:SI -> start of bit block
		pop		[PixelRows]			; restore number of pixel rows
		pop		_ax					; AH := last map read
									; AL := Read Map select reg number
		dec		al
		jns		@@NextPlane			; Loop across bit planes

@@Exit:	restore_es
		restore_ds
		leave_c
		ret

procend		_VGA4_readImage

;----------------------------------------------------------------------------
; void VGA4_writeImage(MGLDC *dc,int dstLeft,int dstTop,void *image,int op)
;----------------------------------------------------------------------------
; Routine to copy a block of video information from system memory to the
; currently active display page. All of the information required to draw
; the image apart from the destination starting location is stored in the
; image block.
;
; Entry:	dc		- Device context to draw into
;			dstLeft	- Left coordinate of image
;			dstTop	- Top coordinate of image
;			image	- Pointer to place to get image data from
;			op		- Raster operation used to combine image data
;----------------------------------------------------------------------------
procstart	_VGA4_writeImage

		ARG		dc:DPTR, dstleft:UINT, dsttop:UINT, image:DPTR, op:UINT

		LOCAL	PixelRows:UINT, PixelRowLen:UINT, RowCounter:UINT,		\
				StartMask:UINT, EndMaskL:UINT, EndMaskR:UINT, 			\
				BytesPerLine:UINT, StartOffset:UINT = LocalSize

		enter_c	LocalSize
		cld
		save_ds
		save_es

; Establish addressing

		_les	_bx,[dc]
		mov		_ax,[MGLDC.mi.bytesPerLine]
		mov		[BytesPerLine],_ax	; Save bytes per line value on stack
		mov		_ax,[dstTop]
		mov		_cx,[dstLeft]
		call	VGA4_pixelAddrESBX	; ES:_BX -> video buffer
		inc		cl
		and		cl,7				; CL := number of bits to shift left
		mov		_di,_bx				; ES:_DI -> (left,top) in video buffer

		_lds	_si,[image]			; DS:_SI -> buffer in system RAM

; Obtain dimensions of bit block from header

		xor		_ax,_ax
		lodsw						; _AX := number of pixel rows in image
		mov		[PixelRows],_ax
		lodsw						; _AX := bytes per pixel row
		mov		[PixelRowLen],_ax
		lodsb						; AL := bit mask for last byte in row
		lodsb
		mov		ch,al

; Set up Graphics Controller

		mov		ah,[BYTE op]		; Get write mode function into ah
		mov		dx,3CEh				; DX := Graphics Controller port
		shl		ah,3				; Put in correct position
		mov		al,3				; Select register
		out		dx,ax

		mov		ax,0805h			; AH := 8 (read mode 1, write mode 0)
		out		dx,ax

		mov		ax,0007h			; AH := 0 (don't care for all maps;
									;	CPU reads always return 0FFH)
									; AL := 7 (Color Don't care reg number)
		out		dx,ax				; Set up Color Don't care reg

		mov		ax,0FF08h			; AH := 0FFh (value for bit mask reg)
		out		dx,ax				; Set up bit mask reg

		mov		bx,0FFh				; BH := 0 (mask for first byte in row)
									; BL := 0FFh
		mov		al,ch				; AL := mask for last byte in row
		cbw							; AH := 0FFh (mask for last-1 byte)

		cmp		[PixelRowLen],1
		jne		@@ManyBytes

		mov		bl,ch
		mov		ah,ch				; AH := mask for last-1 byte
		xor		al,al				; AL := 0 (mask for last byte)

@@ManyBytes:
		shl		_ax,cl				; Shift masks into position
		shl		_bx,cl

		mov		bl,al				; Save masks along with ...
		mov		al,8				; Bit mask register number
		mov		[EndMaskL],_ax
		mov		ah,bl
		mov		[EndMaskR],_ax
		mov		ah,bh
		mov		[StartMask],_ax

		mov		dx,03C4h			; DX := 3C4h (sequencer port)
		mov		ax,0802h			; AH := 1000b (value for Map Mask reg)
									; AL := 2 (Map Mask register number)

; Set pixels row by row in the bit planes

		mov		[StartOffset],_di

@@NextPlane2:
		out		dx,ax
		push	_ax					; preserve Map Mask value
		mov		_di,[StartOffset]
		mov		dl,0CEh				; DX := 3CEh (Graphics Controller port)

		mov		_ax,[PixelRows]
		mov		[RowCounter],_ax	; Initialise loop counter

; Set pixels at start of row in currently enabled bit plane

@@NextRow2:
		mov		_bx,[PixelRowLen]	; BX := bytes per pixel row
		push	_di					; preserve offset of start of pixel row
		push	_si					; preserve offset of row in bit block

		mov		_ax,[StartMask]
		out		dx,ax				; Set Bit Mask reg for first byte of row

		lodsw						; AH := 2nd byte of pixels
									; AL := 1st byte of pixels
		dec		_si					; DS:SI -> 2nd byte of pixels
		test	cl,cl
		jnz		@@NotLeft			; jump if not left-aligned

		dec		_bx					; BX := bytes per row - 1
		jnz		@@TwoBytes			; jump if at least two bytes per row
		jmp		@@DoEnd				; jump if only one byte per row

@@NotLeft:
		rol		ax,cl				; AH := left part of 1st byte,
									;	right part of 2nd byte
									; AL := right part of 1st byte,
									;	left part of 2nd byte
		and		[_ES _di],ah		; set pixels for left part of first byte
		inc		_di
		dec		_bx					; BX := bytes per row - 2

@@TwoBytes:
		push	_ax					; Preserve pixels
		mov		ax,0FF08h
		out		dx,ax				; set bit mask reg for succeeding bytes
		pop		_ax

		dec		_bx
		jng		@@DoEnd				; jump if only 1 or 2 bytes in pixel row

; Use slower code if we are not in replace mode

		cmp		[op],MGL_REPLACE_MODE
		jne		@@SlowMiddle

@@FastMiddle:
		mov		[_ES _di],al		; Set pixels in right part of current
									; 	byte and left part of next byte
		inc		_di

		lodsw						; AH := next+1 byte of pixels
		dec		_si					; AL := next byte of pixels
		rol		ax,cl				; AH := left part of next byte,
									;	right part of next+1 byte
									; AL := right part of next byte,
									;	left part of next+1 byte
		dec		_bx
		jnz		@@FastMiddle		; loop across rows
		jmp		@@DoEnd

@@SlowMiddle:
		and		[_ES _di],al		; Set pixels in right part of current
									; 	byte and left part of next byte
		inc		_di

		lodsw						; AH := next+1 byte of pixels
		dec		_si					; AL := next byte of pixels
		rol		ax,cl				; AH := left part of next byte,
									;	right part of next+1 byte
									; AL := right part of next byte,
									;	left part of next+1 byte
		dec		_bx
		jnz		@@SlowMiddle		; loop across rows

; Set pixels at end of row

@@DoEnd:
		mov		_bx,_ax				; BH := right part of last byte,
									;	left part of last-1 byte
									; BL := left part of last byte,
									;	right part of last-1 byte
		mov		_ax,[EndMaskL]		; AH := mask for last-1 byte
									; AL := Bit Mask reg number
		out		dx,ax				; Set bit mask register
		and		[_ES _di],bl		; set pixels for last-1 byte

		mov		_ax,[EndMaskR]		; Mask for last byte in pixel row
		out		dx,ax				; Last byte in pixel row
		and		[_ES _di+1],bh		; set pixels for last byte

		pop		_si
		add		_si,[PixelRowLen]	; DS:SI -> next row in bit block

		pop		_di
		add		_di,[BytesPerLine]	; ES:DI -> next pixel row in buffer
		dec		[RowCounter]
		jnz		@@NextRow2			; loop down pixel rows

		pop		_ax					; AX := current Map Mask value
		mov		dl,0C4h				; DX := 3C4h
		shr		ah,1				; AH := next Map Mask value
		jnz		@@NextPlane2		; loop across bit planes

@@Exit:

; Restore Graphics controller state

		mov		ax,0F02h			; default Map Mask value
		out		dx,ax

		mov		dl,0CEh				; DX := 3CEh
		mov		ax,0003h			; default Data Rotate/Func select
		out		dx,ax

		mov		ax,0005h			; default mode value
		out		dx,ax

		mov		ax,0F07h			; default color compare value
		out		dx,ax

		mov		ax,0FF08h			; default Bit Mask value
		out		dx,ax

		restore_es
		restore_ds
		leave_c
		ret

procend		_VGA4_writeImage

;----------------------------------------------------------------------------
; int VGA4_scanLeftForColor(int x,int y,color_t border)
;----------------------------------------------------------------------------
; Routine to scan left in the video buffer from a starting location until
; we come across a specified border pixel color. We scan only until we reach
; the edge of the display. No clipping is performed, but can be performed
; after the routine has finished (it is VERY fast!).
;
; Entry:	x		- Starting x coordinate
;			y		- Starting y coordinate
;			border	- Border value to search for
;
; Exit:		AX		- Coordinate of first pixel of the desired color. This
;					  will be -1 if no pixels exist on the scan line.
;
;----------------------------------------------------------------------------
procstart	_VGA4_scanLeftForColor

		ARG		x:UINT, y:UINT, border:COLOR_T

		enter_c	0
		save_es
		std							; Move down in memory

; Calculate pixel address of (0,y)

		mov		_ax,[y]				; AX := y
		xor		_cx,_cx				; CX := 0
		call	VGA4_pixelAddr		; ES:_BX -> buffer
		mov		_di,_bx				; ES:_DI -> buffer

; Calculate offset of x in row

		mov		_ax,[x]
		mov		_si,_ax				; SI,AX := x
		shr		_si,3				; SI := offset of x in row y
		add		_di,_si				; DI := offset of x in buffer

; Calculate a bit mask for the first byte to scan

		mov		cl,al
		and		cl,7				; CL := x & 7
		xor		cl,7
		mov		ch,0FFh
		shl		ch,cl				; CH := bit mask for first scanned byte

; Configure the graphics controller

		mov		dx,03CEh			; DX := Graphics Controller port addr
		mov		ah,[BYTE border]	; AH := pixel value for Color Compare reg
		mov		al,2				; AL := Color Compare Reg number
		out		dx,ax

		mov		ax,0805h			; AH := 00001000b (read mode 1)
		out		dx,ax				; AL := mode reg number

		mov		ax,0F07h			; AH := 0F (color don't care value)
		out		dx,ax				; AL := 07 (color don't care reg number)

; Inspect the first byte for border pixels

		mov		al,[_ES _di]		; AL := nonzero bits corresponding to
									;	border pixels
		dec		_di					; ES:DI -> next byte to scan
		and		al,ch				; Apply bit mask
		jnz		@@FoundBorder		; Jump if border pixel(s) found

; scan remainder of line for border pixels

		mov		_cx,_si				; CX := number of bytes to scan
		repz	scasb				; scan until nonzero byte read
									;	(border pixels found).

; Compute x value of border pixels

		mov		al,[_ES _di+1]		; AL := last byte compared

@@FoundBorder:
		sub		_di,_bx
		inc		_di					; DI := offset of byte containing
									;	the border pixel
		shl		_di,3				; DI := x-coordinate of 1st pixel in byte
		mov		_cx,8				; CX := loop limit

@@NextPixel:
		shr		al,1				; isolate first border pixel
		jc		@@Found				; We have found the pixel
		loop	@@NextPixel			; Loop for all pixels
		mov		_di,-1				; No pixel was found
		jmp		@@Exit

@@Found:
		dec		_cx
		add		_di,_cx

; Restore default graphics controller state and return to caller

@@Exit:
		mov		ax,0002h			; AH := 0 (default color compare value)
		out		dx,ax				; restore color compare value

		mov		ax,0005h			; AH := 0, AL := 5
		out		dx,ax				; restore mode reg

		mov		_ax,_di				; AX := return value

		restore_es
		leave_c_nolocal
		ret

procend		_VGA4_scanLeftForColor

;----------------------------------------------------------------------------
; int VGA4_scanRightForColor(int x,int y,color_t border)
;----------------------------------------------------------------------------
; Routine to scan right in the video buffer from a starting location until
; we come across a specified border pixel color. We scan only until we reach
; the end of the display. No clipping is performed, but can be performed
; after the routine has finished (it is VERY fast!).
;
; Entry:	x		- Starting x coordinate
;			y		- Starting y coordinate
;			border	- Border value to search for
;
; Exit:		AX		- Coordinate of first pixel of the desired color. This
;					  will be one larger than the maximum coordinate if
;					  no pixels exist on the scan line.
;
;----------------------------------------------------------------------------
procstart	_VGA4_scanRightForColor

		ARG		x:UINT, y:UINT, border:COLOR_T

		LOCAL	bytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		save_es
		cld

; Save device context values onto local stack

		mov		_ax,[GDC.mi.bytesPerLine]
		mov		[bytesPerLine],_ax

; Calculate pixel address of (0,y)

		mov		_ax,[y]				; AX := y
		xor		_cx,_cx				; CX := 0
		call	VGA4_pixelAddr		; ES:_BX -> buffer
		mov		_di,_bx				; ES:_DI -> buffer

; Calculate offset of x in row

		mov		_ax,[x]
		mov		_si,_ax				; SI,AX := x
		shr		_si,3				; SI := offset of x in row y
		add		_di,_si				; DI := offset of x in buffer

; Calculate a bit mask for the first byte to scan

		mov		cl,al
		and		cl,7				; CL := x & 7
		mov		ch,0FFh
		shr		ch,cl				; CH := bit mask for first scanned byte

; Configure the graphics controller

		mov		dx,03CEh			; DX := Graphics Controller port addr
		mov		ah,[BYTE border]	; AH := pixel value for Color Compare reg
		mov		al,2				; AL := Color Compare Reg number
		out		dx,ax

		mov		ax,0805h			; AH := 00001000b (read mode 1)
		out		dx,ax				; AL := mode reg number

		mov		ax,0F07h			; AH := 0F (color don't care value)
		out		dx,ax				; AL := 07 (color don't care reg number)

; Inspect the first byte for border pixels

		mov		al,[_ES _di]		; AL := nonzero bits corresponding to
									;	border pixels
		inc		_di					; ES:DI -> next byte to scan
		and		al,ch				; Apply bit mask
		jnz		@@FoundBorder		; Jump if border pixel(s) found

; scan remainder of line for border pixels

		mov		_cx,[bytesPerLine]
		sub		_cx,_si				; CX := BytesPerLine - (byte offset of
									;	starting pixel)
		dec		_cx					; CX := # of bytes to scan
		repz	scasb				; scan until nonzero byte read
									;	(border pixels found).

; Compute x value of border pixels

		mov		al,[_ES _di-1]		; AL := last byte compared

@@FoundBorder:
		sub		_di,_bx				; DI := offset of byte past the one
									;	which contains a border pixel
		shl		_di,3				; DI := x-coordinate of 1st pixel in byte
		mov		_cx,8				; CX := loop limit

@@NextPixel:
		shl		al,1				; isolate first border pixel
		jc		@@Found
		loop	@@NextPixel

@@Found:
		sub		_di,_cx				; DI := x-coordinate of border pixel

; Restore default graphics controller state and return to caller

		mov		ax,0002h			; AH := 0 (default color compare value)
		out		dx,ax				; restore color compare value

		mov		ax,0005h			; AH := 0, AL := 5
		out		dx,ax				; restore mode reg

		mov		_ax,_di				; AX := return value

		restore_es
		leave_c
		ret

procend		_VGA4_scanRightForColor

;----------------------------------------------------------------------------
; int VGA4_scanLeftWhileColor(int x,int y,color_t oldcolor)
;----------------------------------------------------------------------------
; Routine to scan left in the video buffer from a starting location while
; the pixel color remains the same as the specified color. We scan only
; until we reach the edge of the display. No clipping is performed, but
; can be performed after the routine has finished (it is VERY fast!).
;
; Entry:	x			- Starting x coordinate
;			y			- Starting y coordinate
;			oldcolor	- Color value to search with
;
; Exit:		AX		- Coordinate of first pixel of the desired color. This
;					  will be -1 if no pixels exist on the scan line.
;
;----------------------------------------------------------------------------
procstart	_VGA4_scanLeftWhileColor

		ARG		x:UINT, y:UINT, oldcolor:COLOR_T

		enter_c	0
		save_es
		std							; Move down in memory

; Calculate pixel address of (0,y)

		mov		_ax,[y]				; AX := y
		xor		_cx,_cx				; CX := 0
		call	VGA4_pixelAddr		; ES:_BX -> buffer
		mov		_si,_bx				; ES:_SI -> buffer

; Calculate offset of x in row

		mov		_ax,[x]
		mov		_di,_ax				; DI,AX := x
		shr		_di,3				; DI := offset of x in row y
		add		_si,_di				; SI := offset of x in buffer

; Calculate a bit mask for the first byte to scan

		mov		cl,al
		and		cl,7				; CL := x & 7
		xor		cl,7
		mov		ch,0FFh
		shl		ch,cl				; CH := bit mask for first scanned byte

; Configure the graphics controller

		mov		dx,03CEh			; DX := Graphics Controller port addr
		mov		ah,[BYTE oldcolor]	; AH := pixel value for Color Compare reg
		mov		al,2				; AL := Color Compare Reg number
		out		dx,ax

		mov		ax,0805h			; AH := 00001000b (read mode 1)
		out		dx,ax				; AL := mode reg number

		mov		ax,0F07h			; AH := 0F (color don't care value)
		out		dx,ax				; AL := 07 (color don't care reg number)

; Inspect the first byte for non-interior pixels

		lods	[BYTE _ES _si]		; AL := zero bits corresponding to
									; 	non-interior pixels
		not		al					; AL := nonzero bits corresponding to
									;	non-interior pixels
		and		al,ch				; Apply bit mask
		jnz		@@FoundBorder		; Jump if non-interior pixel(s) found

; scan remainder of line for non-interior pixels

		mov		_cx,_di				; CX := number of bytes to scan
@@NextByte:
		lods	[BYTE _ES _si]		; AL := zero bits corresponding to
									; 	non-interior pixels
		not		al					; AL := nonzero bits corresponding to
									;	non-interior pixels
		or		al,al
		jnz		@@FoundBorder		; Jump if non-interior pixel(s) found
		loop	@@NextByte

; Compute x value of non-interior pixels

		mov		al,[_ES _si+1]		; AL := last byte compared
		not		al

@@FoundBorder:
		sub		_si,_bx
		inc		_si					; SI := offset of byte containing
									;	the border pixel
		shl		_si,3				; SI := x-coordinate of 1st pixel in byte
		mov		_cx,8				; CX := loop limit

@@NextPixel:
		shr		al,1				; isolate first border pixel
		jc		@@Found				; We have found the pixel
		loop	@@NextPixel			; Loop for all pixels
		mov		_si,-1				; No pixel was found
		jmp		@@Exit

@@Found:
		dec		_cx
		add		_si,_cx

; Restore default graphics controller state and return to caller

@@Exit:
		mov		ax,0002h			; AH := 0 (default color compare value)
		out		dx,ax				; restore color compare value

		mov		ax,0005h			; AH := 0, AL := 5
		out		dx,ax				; restore mode reg

		mov		_ax,_si				; AX := return value

		restore_es
		leave_c_nolocal
		ret

procend		_VGA4_scanLeftWhileColor

;----------------------------------------------------------------------------
; int VGA4_scanRightWhileColor(int x,int y,color_t oldcolor)
;----------------------------------------------------------------------------
; Routine to scan right in the video buffer from a starting location while
; the pixel color remains the same as the specified color. We scan only
; until we reach the end of the display. No clipping is performed, but can
; be performed after the routine has finished.
;
; Entry:	x			- Starting x coordinate
;			y			- Starting y coordinate
;			oldcolor	- Border value to search width
;
; Exit:		AX		- Coordinate of first pixel of the desired color. This
;					  will be one larger than the maximum coordinate if
;					  no pixels exist on the scan line.
;
;----------------------------------------------------------------------------
procstart	_VGA4_scanRightWhileColor

		ARG		x:UINT, y:UINT, oldcolor:COLOR_T

		LOCAL	bytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		save_es
		cld

; Save device context values onto local stack

		mov		_ax,[GDC.mi.bytesPerLine]
		mov		[bytesPerLine],_ax

; Calculate pixel address of (0,y)

		mov		_ax,[y]				; AX := y
		xor		_cx,_cx				; CX := 0
		call	VGA4_pixelAddr		; ES:_BX -> buffer
		mov		_si,_bx				; ES:_SI -> buffer

; Calculate offset of x in row

		mov		_ax,[x]
		mov		_di,_ax				; DI,AX := x
		shr		_di,3				; DI := offset of x in row y
		add		_si,_di				; SI := offset of x in buffer

; Calculate a bit mask for the first byte to scan

		mov		cl,al
		and		cl,7				; CL := x & 7
		mov		ch,0FFh
		shr		ch,cl				; CH := bit mask for first scanned byte

; Configure the graphics controller

		mov		dx,03CEh			; DX := Graphics Controller port addr
		mov		ah,[BYTE oldcolor]	; AH := pixel value for Color Compare reg
		mov		al,2				; AL := Color Compare Reg number
		out		dx,ax

		mov		ax,0805h			; AH := 00001000b (read mode 1)
		out		dx,ax				; AL := mode reg number

		mov		ax,0F07h			; AH := 0F (color don't care value)
		out		dx,ax				; AL := 07 (color don't care reg number)

; Inspect the first byte for non-interior pixels

		lods	[BYTE _ES _si]		; AL := zero bits corresponding to
									;	non-interior pixels
		not		al					; AL := nonzero bits corresponding to
									;	non-interior pixels
		and		al,ch				; Apply bit mask
		jnz		@@FoundBorder		; Jump if border pixel(s) found

; scan remainder of line for border pixels

		mov		_cx,[bytesPerLine]
		sub		_cx,_di				; CX := BytesPerLine - (byte offset of
									;	starting pixel)
		dec		_cx					; CX := # of bytes to scan
@@NextByte:
		lods	[BYTE _ES _si]		; AL := zero bits corresponding to
									;	non-interior pixels
		not		al					; AL := nonzero bits corresponding to
									;	non-interior pixels
		or		al,al
		jnz		@@FoundBorder
		loop	@@NextByte

; Compute x value of border pixels

		mov		al,[_ES _si-1]		; AL := last byte compared
		not		al

@@FoundBorder:
		sub		_si,_bx				; SI := offset of byte past the one
									;	which contains a border pixel
		shl		_si,3				; SI := x-coordinate of 1st pixel in byte
		mov		_cx,8				; CX := loop limit

@@NextPixel:
		shl		al,1				; isolate first border pixel
		jc		@@Found
		loop	@@NextPixel

@@Found:
		sub		_si,_cx				; SI := x-coordinate of border pixel

; Restore default graphics controller state and return to caller

		mov		ax,0002h			; AH := 0 (default color compare value)
		out		dx,ax				; restore color compare value

		mov		ax,0005h			; AH := 0, AL := 5
		out		dx,ax				; restore mode reg

		mov		_ax,_si				; AX := return value

		restore_es
		leave_c
		ret

procend		_VGA4_scanRightWhileColor

endif	; !MGL_LITE

endcodeseg	_vga4

		END							; End of module
