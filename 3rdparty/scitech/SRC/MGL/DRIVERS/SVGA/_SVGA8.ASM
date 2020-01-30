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
;* Description:	SVGA8, 8 bit color bank switched assembler based routines.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"
include "mgldos\biosdata.equ"

header	_SVGA8

	EXTRN	_SVGA_setBank:FPTR
	EXTRN	___EMU__scanLine:FPTR
	EXTRN	___EMU__putImage:FPTR

begdataseg	_SVGA8

	$EXTRN	_SVGA_curBank,BYTE
	$EXTRN	__MGL_dc,devCtx_s

enddataseg	_SVGA8

begcodeseg	_SVGA8

;----------------------------------------------------------------------------
; SVGA8_pixelAddrESBX	Determine buffer address of pixel in 8 bit modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				ES:_BX	->  device context
;				_CX		-	x-coordinate
;
; Exit:			ES:_BX	->	offset in buffer
; 				DX		-   64k bank number of the address
;
; Registers:	None
;
;----------------------------------------------------------------------------
procstart	SVGA8_pixelAddrESBX

		imul	[USHORT MGLDC.mi.bytesPerLine]; DX:AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@1
		dec		dl						; Decrement bank number if negative
if	flatmodel
		and		ecx,0FFFFh
endif
@@1:	add		cx,ax
		adc		dl,[BYTE MGLDC.bankOffset] ; DL := bank number
		lVidDC	_bx
		add		_bx,_cx					; ES:_BX -> start of video memory
		ret

procend		SVGA8_pixelAddrESBX

;----------------------------------------------------------------------------
; SVGA8_pixelAddr	Determine buffer address of pixel in 8 bit modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				_CX		-	x-coordinate
;
; Exit:			ES:_BX	->	offset in buffer
; 				DX		-   64k bank number of the address
;
; Registers:	None
;
;----------------------------------------------------------------------------
procstart	SVGA8_pixelAddr

		imul	[USHORT GDC.mi.bytesPerLine]; DX:AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@1
		dec		dl						; Decrement bank number if negative
if	flatmodel
		and		ecx,0FFFFh
endif
@@1:	add		cx,ax
		adc		dl,[BYTE GDC.bankOffset] ; DL := bank number
		lVid	_bx
		add		_bx,_cx					; ES:_BX -> start of video memory
		ret

procend		SVGA8_pixelAddr

;----------------------------------------------------------------------------
; color_t SVGA8_getPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine reads the value of a pixel in 8 bit packed pixel format
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
; Exit:			_DX:_AX	-	Color of pixel at (x,y)
;
;----------------------------------------------------------------------------
procstart	_SVGA8_getPixel

		ARG		x:UINT, y:UINT

		enter_c	0
		save_es

; Compute the pixel's address in video buffer

		mov		_ax,[y]
		mul		[USHORT GDC.mi.bytesPerLine]; AX := y * BytesPerLine
		add		ax,[USHORT x]
		adc		dl,[BYTE GDC.bankOffset] ; DL := bank number
        lVid	_bx
		add		_bx,_ax				; ES:_BX -> start of video memory

		cmp		dl,[_SVGA_curBank]
		je		@@NoChange
		call	_SVGA_setBank

@@NoChange:
		xor		_ax,_ax
		mov		al,[_ES _bx]		; AX := pixel value
		xor		_dx,_dx				; _DX:_AX := pixel value

		restore_es
		leave_c_nolocal
		ret

procend		_SVGA8_getPixel

;----------------------------------------------------------------------------
; void SVGA8_putPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in 8 bit VESA modes.
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
;----------------------------------------------------------------------------
procstart	_SVGA8_putPixel

		ARG		x:UINT, y:UINT

		enter_c	0
		save_es

		mov		cl,[BYTE GDC.intColor]
		mov		ch,[BYTE GDC.a.writeMode]

; Compute the pixel's address in video buffer

		mov		_ax,[y]
		mul		[USHORT GDC.mi.bytesPerLine]; AX := y * BytesPerLine
		add		ax,[USHORT x]
		adc		dl,[BYTE GDC.bankOffset] ; DL := bank number
		lVid	_bx
		add		_bx,_ax					; ES:_BX -> start of video memory

		cmp		dl,[_SVGA_curBank]
		je		@@NoChange
		call	_SVGA_setBank

@@NoChange:
		cmp		ch,MGL_REPLACE_MODE
		jne		@@1
		mov		[_ES _bx],cl		; Replace the pixel
		jmp		@@Exit

@@1:	cmp		ch,MGL_XOR_MODE
		jne		@@2
		xor		[_ES _bx],cl		; XOR the pixel
		jmp		@@Exit

@@2:	cmp		ch,MGL_OR_MODE
		jne		@@3
		or		[_ES _bx],cl		; OR the pixel
		jmp		@@Exit

@@3:	and		[_ES _bx],cl		; AND the pixel

@@Exit:	restore_es
		leave_c_nolocal
		ret

procend		_SVGA8_putPixel

;----------------------------------------------------------------------------
; void SVGA8_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
;	int bufx,int op)
;----------------------------------------------------------------------------
; Reads a scanline of information from the device context into the memory
; buffer. For non-replace mode operaitons, we simply call the C based
; routine to handle it.
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
procstart	_SVGA8_getScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	ds_eq_es_swap
		mov		_si,_bx				; DS:_SI -> display surface
		_les	_di,[buffer]		; ES:_DI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		cmp		[op],MGL_REPLACE_MODE
		jne		@@DoROP

		mov     _ax,_si				; Check for bankswitch in scanline
		add		ax,cx
		jc		@@BrokenScanline	; Yep, so special case it
		mov		_dx,_cx
		shr		_cx,2
	rep	movsd						; Move pixels in row 32 bits at a time
		mov		_cx,_dx
		and		_cx,3
	rep	movsb						; Move the last bytes in row

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@BrokenScanline:
		sub		cx,ax				; CX := bytes in first bank
		shr		_cx,1
	rep	movsw						; Store bytes in first bank
		adc		_cx,_cx
	rep	movsb
		inc		dl
		call	_SVGA_setBank		; Change to new bank
if flatmodel
ifdef	USE_SELVMEM
		xor		esi,esi
else
		sub		esi,10000h
endif
endif
		mov		cx,ax				; CX := bytes in second bank
		shr		_cx,1
	rep	movsw						; Store bytes in second bank
		adc		_cx,_cx
	rep	movsb
		jmp		@@Exit

; Handle ROP code blitting

@@DoROP:
		cmp		[op],MGL_XOR_MODE
		jne		@@1

@@LoopXOR:
		mov		al,[_si]
		add     _si,1
		jnc		@@NCX
		inc		dl
		call	_SVGA_setBank
@@NCX:	xor		[es:_di],al
		inc		_di
		loop	@@LoopXOR
		jmp		@@Exit

@@1:	cmp		[op],MGL_OR_MODE
		jne		@@LoopAND

@@LoopOR:
		mov		al,[_si]
		add     _si,1
		jnc		@@NCO
		inc		dl
		call	_SVGA_setBank
@@NCO:	or		[es:_di],al
		inc		_di
		loop	@@LoopOR
		jmp		@@Exit

@@LoopAND:
		mov		al,[_si]
		add     _si,1
		jnc		@@NCA
		inc		dl
		call	_SVGA_setBank
@@NCA:	and		[es:_di],al
		inc		_di
		loop	@@LoopAND
		jmp		@@Exit

procend		_SVGA8_getScanLine

;----------------------------------------------------------------------------
; void SVGA8_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
;	int bufx,int op)
;----------------------------------------------------------------------------
; Dumps a scanline of information to the device context from the memory
; buffer. For non-replace mode operations, we simply call the C code.
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
procstart	_SVGA8_putScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		cmp		[op],MGL_REPLACE_MODE
		jne		@@DoROP

		mov     _ax,_di				; Check for bankswitch in scanline
		add		ax,cx
		jc		@@BrokenScanline	; Yep, so special case it

		mov		_dx,_cx
		shr		_cx,2
	rep	movsd						; Move pixels in row 32 bits at a time
		mov		_cx,_dx
		and		_cx,3
	rep	movsb						; Move the last bytes in row

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@BrokenScanline:
		sub		cx,ax				; CX := bytes in first bank
		shr		_cx,1
	rep	movsw
		adc		_cx,_cx
	rep	movsb
		inc		dl
		call	_SVGA_setBank		; Change to new bank
if flatmodel
ifdef	USE_SELVMEM
		xor		edi,edi
else
		sub		edi,10000h
endif
endif
		mov		cx,ax				; CX := bytes in second bank
		shr		_cx,1
	rep	movsw
		adc		_cx,_cx
	rep	movsb
		jmp		@@Exit

; Handle ROP code blitting

@@DoROP:
		cmp		[op],MGL_XOR_MODE
		jne		@@1

@@LoopXOR:
		lodsb
		xor		[es:_di],al
		add		di,1
		jnc		@@NCX
		inc		dl
		call	_SVGA_setBank
@@NCX:	loop	@@LoopXOR
		jmp		@@Exit

@@1:	cmp		[op],MGL_OR_MODE
		jne		@@LoopAND

@@LoopOR:
		lodsb
		or		[es:_di],al
		add		di,1
		jnc		@@NCO
		inc		dl
		call	_SVGA_setBank
@@NCO:	loop	@@LoopOR
		jmp		@@Exit

@@LoopAND:
		lodsb
		and		[es:_di],al
		add		di,1
		jnc		@@NCA
		inc		dl
		call	_SVGA_setBank
@@NCA:	loop	@@LoopAND
		jmp		@@Exit

procend		_SVGA8_putScanline

;----------------------------------------------------------------------------
; void SVGA8_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a line in 8 bit VESA modes.
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
procstart	_SVGA8_line

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		LOCAL	Routine:NCPTR, VertInc:ULONG, EIncr:FIX32_T, 			\
				NEIncr:FIX32_T, color:S_UCHAR, writeMode:S_UCHAR,			\
				varDX:FIX32_T, varDY:FIX32_T, x:UINT, y:UINT = LocalSize

		enter_c	LocalSize
		save_es

; Load the current color

		mov		al,[BYTE GDC.intColor]
		mov		[color],al
		mov		al,[BYTE GDC.a.writeMode]
		mov		[writeMode],al
		mov		_si,[GDC.mi.bytesPerLine]
		mov		[WORD VertInc+2],0	; Zero out sign for vertical increment

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
		mov		[BYTE VertInc+2],0FFh ; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[WORD vertInc],si	; save increment
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
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
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
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		pop		_cx					; restore reg
		cmp		dl,[_SVGA_curBank]
		je		@@NoChange
		call	_SVGA_setBank

@@NoChange:
		mov		al,[color]
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@V1

; Draw the line

@@Set1:	mov		[_ES _bx],al			; set pixel value in buffer
		add		bx,si				; increment to next position of interleave
		jc      @@ChangeBank1
@@Ret1:	loop	@@Set1				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank1:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret1

@@V1:   cmp		[writeMode],MGL_XOR_MODE
		jne		@@V2

@@Set2:	xor		[_ES _bx],al			; set pixel value in buffer
		add		bx,si				; increment to next position of interleave
		jc		@@ChangeBank2
@@Ret2:	loop	@@Set2				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank2:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret2

@@V2:	cmp		[writeMode],MGL_OR_MODE
		jne		@@Set4

@@Set3:	or		[_ES _bx],al			; set pixel value in buffer
		add		bx,si				; increment to next position of interleave
		jc		@@ChangeBank3
@@Ret3:	loop	@@Set3				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank3:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret3

@@Set4:	and		[_ES _bx],al			; set pixel value in buffer
		add		bx,si				; increment to next position of interleave
		jc		@@ChangeBank3
@@Ret4:	loop	@@Set4				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank4:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret4

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:DL:_BX -> video buffer
;						ECX = # pixels to draw
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		push	_dx
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
		test	[WORD VertInc],8000h
		jnz		@@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

@@LoDoneAdjust:
		pop		_dx
		mov		al,[color]
		mov		ah,[writeMode]
		mov     dh,[_SVGA_curBank]
		mov		esi,[EIncr]
		cmp		ah,MGL_REPLACE_MODE
		jne		@@LoopLo2

@@LoopLo1:
		cmp		dl,dh
		je		@@SetLo1

		call	_SVGA_setBank		; Change video banks
		mov		dh,dl				; Keep DH in sync

@@SetLo1:
		mov		[_ES _bx],al			; Set pixel value in buffer
		add		bx,1				; Increment x coordinate
		adc		dl,0
		or		edi,edi				; Test sign of d
		jg		@@LoPosDi1			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@LoopLo1			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi1:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		bx,[WORD vertInc]	; increment y
		adc		dl,[BYTE vertInc+2]	; Adjust bank number
		loop	@@LoopLo1			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoopLo2:
		cmp		dl,dh
		je		@@SetLo2

		call	_SVGA_setBank		; Change video banks
		mov		dh,dl				; Keep DH in sync

@@SetLo2:
		cmp		ah,MGL_XOR_MODE
		jne		@@L1
		xor		[_ES _bx],al
		jmp		@@LOut
@@L1:   cmp		ah,MGL_OR_MODE
		jne		@@L2
		or		[_ES _bx],al
		jmp		@@LOut
@@L2:	and		[_ES _bx],al

@@LOut:	add		bx,1				; Increment x coordinate
		adc		dl,0
		or		edi,edi				; Test sign of d
		jg		@@LoPosDi2			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@LoopLo2			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi2:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		bx,[WORD vertInc]	; increment y
		adc		dl,[BYTE vertInc+2]	; Adjust bank number
		loop	@@LoopLo2			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:DL:_BX -> video buffer
;						ECX = # pixels to draw
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		push	_dx
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
		test	[WORD VertInc],8000h
		jnz		@@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

@@HiDoneAdjust:
		pop		_dx
		mov		al,[color]			; AL := pixel value to fill
		mov		ah,[writeMode]
		mov		dh,[_SVGA_curBank]
		mov		esi,[EIncr]
		cmp		ah,MGL_REPLACE_MODE
		jne		@@LoopHi2

@@LoopHi1:
		cmp		dl,dh
		je		@@SetHi1

		call	_SVGA_setBank		; Change video banks
		mov		dh,dl				; Keep DH in sync

@@SetHi1:
		mov		[_ES _bx],al			; Set pixel value in buffer
		add		bx,[WORD vertInc]	; increment y
		adc		dl,[BYTE vertInc+2]
		or		edi,edi				; Test sign of d
		jg		@@HiPosDi1			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@LoopHi1			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi1:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		bx,1				; Increment x
		adc		dl,0
		loop	@@LoopHi1			; Loop for remaining pixels
		jmp		@@Exit

@@LoopHi2:
		cmp		dl,dh
		je		@@SetHi2

		call	_SVGA_setBank		; Change video banks
		mov		dh,dl				; Keep DH in sync

@@SetHi2:
		cmp		ah,MGL_XOR_MODE
		jne		@@H1
		xor		[_ES _bx],al
		jmp		@@Hout
@@H1:	cmp		ah,MGL_OR_MODE
		jne		@@H2
		or		[_ES _bx],al
		jmp		@@HOut
@@H2:   and		[_ES _bx],al

@@HOut:	add		bx,[WORD vertInc]	; increment y
		adc		dl,[BYTE vertInc+2]
		or		edi,edi				; Test sign of d
		jg		@@HiPosDi2			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@LoopHi2			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi2:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		bx,1				; Increment x
		adc		dl,0
		loop	@@LoopHi2			; Loop for remaining pixels

@@Exit:	restore_es
		leave_c
		ret

procend		_SVGA8_line

;----------------------------------------------------------------------------
; void SVGA8_scanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
; Routine draws a scan line in 8 bit VESA graphics modes. This
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
procstart	_SVGA8_scanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		LOCAL	count:UINT, writeMode:S_UCHAR,					\
				color:S_UCHAR, bkColor:S_UCHAR, penStyle:S_UCHAR,				\
				curBank:S_UCHAR = LocalSize

		enter_c	LocalSize
		save_es
		cld							; String instructions go up

		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := X2 - X1 (# pixels to draw)
		jz		@@Exit				; Quit if xstart == xend
		jns		@@NoSwap			; no, don't swap them

		mov		_ax,[x2]
		xchg	[x1],_ax			; Swap the coordinates
		mov		[x2],_ax
		neg		_cx

@@NoSwap:
		mov		[count],_cx

; Save variables out of device context into local variables

		mov     al,[BYTE GDC.a.penStyle]
		mov		[penStyle],al
		mov		al,[BYTE GDC.a.writeMode]
		mov		[writeMode],al
		mov		al,[BYTE GDC.intColor]
		mov		[color],al

		mov		_ax,[y]
		mov		_cx,[x1]
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		cmp		dl,[_SVGA_curBank]
		je		@@NC1
		call	_SVGA_setBank		; Change to starting bank number
@@NC1:	cmp		[penStyle],MGL_BITMAP_SOLID
		jne		@@Patterned

;----------------------------------------------------------------------------
; Draw a solid patterned line

		mov		al,[color]			; AL := pixel color
		mov		ah,[writeMode]
		mov		_cx,[count]
		cmp		ah,MGL_REPLACE_MODE
		jne		@@SolidROP

		mov		ah,al
		mov		_di,_bx
		mov		_si,_di				; Check to see if bank switch will occur
		add		si,cx
		jc		@@BrokenScanLine	; Yes, bank switch on this scanline!
		test	_di,1
		jz		@@WordAligned
		stosb						; Store first byte to ensure word
		dec		_cx					;  alignment during next phase
@@WordAligned:
		shr		_cx,1
	rep	stosw						; Store all middle bytes fast!
		adc		_cx,_cx
	rep	stosb						; Store the last byte
		jmp		@@Exit

; Handle the case where a bank switch can occur in the middle of a scanline

@@BrokenScanline:
		sub		cx,si				; CX := bytes in first bank
	rep	stosb						; Store bytes in first bank
		inc		dl
		call	_SVGA_setBank		; Change to new bank
if flatmodel
ifdef	USE_SELVMEM
		xor		_di,_di
else
		mov		_di,[GDC.surface]
endif
endif
		mov		cx,si				; CX := bytes in second bank
	rep	stosb						; Store bytes in second bank
		jmp		@@Exit

@@SolidROP:
		cmp		ah,MGL_XOR_MODE
		jne		@@S1
		xor		[_ES _bx],al
		jmp		@@SOut
@@S1:	cmp		ah,MGL_OR_MODE
		jne		@@S2
		or		[_ES _bx],al
		jmp		@@SOut
@@S2:	and		[_ES _bx],al
@@SOut:	add		bx,1				; Increment to next pixel
		jc		@@ChangeBank1
@@SEnd:	loop	@@SolidROP			; Loop across line
		jmp		@@Exit

@@ChangeBank1:
		inc		dl					; Change to the new bank
		call	_SVGA_setBank
		jmp		@@SEnd

@@Patterned:
		mov		al,[BYTE GDC.intBackColor]
		mov		[bkColor],al

; Get the pattern byte for this scan line from pattern array

		mov		_si,[y]
		and		_si,7				; Mask to 0-7
		mov		[curBank],dl		; Save current bank number
		mov		dl,[_si + GDC.a.penPat]

		mov		cl,[BYTE x1]
		and		cl,7				; Mask to 0-7
		rol		dl,cl				; Rotate pattern into place

		mov		_cx,[count]			; CX := bytes to set
		cmp		[penstyle],MGL_BITMAP_TRANSPARENT
		je		@@DoTransparent
		cmp		[penstyle],MGL_BITMAP_OPAQUE
		je		@@DoOpaque
		jmp		@@DoPixmap

;----------------------------------------------------------------------------
; Draw a transparent patterned line

@@DoTransparent:
		mov		al,[color]			; AL := pixel color
		mov		ah,[writeMode]
		cmp		ah,MGL_REPLACE_MODE
		jne		@@TransLoopROP

@@TransLoop:
		rol		dl,1				; Rotate pattern mask
		jnc		@@SkipTrans			; Draw pixel if pattern bit was 1

		mov		[_ES _bx],al			; Set pixel value in buffer

@@SkipTrans:
		add		bx,1				; Increment to next pixel
		jc		@@ChangeBank2		; Handle bank switch
@@TEnd:	loop	@@TransLoop			; Loop across line
		jmp		@@Exit

@@ChangeBank2:
		push	_dx
		mov		dl,[curBank]
		inc		dl
		call	_SVGA_setBank
		pop		_dx
		jmp		@@TEnd

@@TransLoopROP:
		rol		dl,1				; Rotate pattern mask
		jnc		@@SkipTransROP		; Draw pixel if pattern bit was 1

		cmp		ah,MGL_XOR_MODE
		jne		@@T1
		xor		[_ES _bx],al			; Set pixel value in buffer
		jmp		@@SkipTransROP
@@T1:	cmp		ah,MGL_OR_MODE
		jne		@@T2
		or		[_ES _bx],al
		jmp		@@SkipTransROP
@@T2:	and		[_ES _bx],al

@@SkipTransROP:
		add		bx,1				; Increment to next pixel
		jc		@@ChangeBank3
@@TRE:	loop	@@TransLoopROP		; Loop across line
		jmp		@@Exit

@@ChangeBank3:
		push	_dx
		mov		dl,[curBank]
		inc		dl
		call	_SVGA_setBank
		pop		_dx
		jmp		@@TRE

;----------------------------------------------------------------------------
; Draw an opaque patterned line

@@DoOpaque:
		mov		ah,[color]			; AH := pixel color
		mov		dh,[writeMode]
		cmp		dh,MGL_REPLACE_MODE
		jne		@@OpaqueLoopROP
		mov		dh,[bkColor]

@@OpaqueLoop:
		mov		al,ah				; AL := foreground color
		rol		dl,1				; Rotate pattern mask
		jc		@@SetPixelOpaque	; Draw pixel if pattern bit was 1

		mov		al,dh				; AL := background color

@@SetPixelOpaque:
		mov		[_ES _bx],al			; Set pixel value in buffer
		add		bx,1				; Increment to next pixel
		jc		@@ChangeBank4
@@OEnd:	loop	@@OpaqueLoop		; Loop across line
		jmp		@@Exit

@@ChangeBank4:
		push	_dx
		mov		dl,[curBank]
		inc		dl
		call	_SVGA_setBank
		pop		_dx
		jmp		@@OEnd

@@OpaqueLoopROP:
		mov		al,ah				; AL := foreground color
		rol		dl,1				; Rotate pattern mask
		jc		@@SetPixelOpaqueROP	; Draw pixel if pattern bit was 1

		mov		al,[bkColor]		; AL := background color

@@SetPixelOpaqueROP:
		cmp		dh,MGL_XOR_MODE
		jne		@@O1
		xor		[_ES _bx],al
		jmp		@@OOut
@@O1:	cmp		dh,MGL_OR_MODE
		jne		@@O2
		or		[_ES _bx],al
		jmp		@@OOut
@@O2:	and		[_ES _bx],al

@@OOut:	add		bx,1				; Increment to next pixel
		jc		@@ChangeBank5
@@ORE:	loop	@@OpaqueLoopROP		; Loop across line
		jmp		@@Exit

@@ChangeBank5:
		push	_dx
		mov		dl,[curBank]
		inc		dl
		call	_SVGA_setBank
		pop		_dx
		jmp		@@ORE

;----------------------------------------------------------------------------
; Draw a pixmap pattern scanline. For now we simply call the high level
; scanline emulation routine which will call putPixel to handle this. Not
; very fast but it works.

@@DoPixmap:
		push	[x2]
		push	[x1]
		push	[y]
		call	___EMU__scanLine
		_add	sp,6,12

@@Exit:	restore_es
		leave_c
		ret

procend		_SVGA8_scanLine

;----------------------------------------------------------------------------
; void SVGA8_fillRect(int x1,int y1,int x2,int y2)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in 8 bit VESA modes.
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
procstart	_SVGA8_fillRect

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT

		LOCAL	rows:UINT, cols:UINT = LocalSize

		enter_c	LocalSize
		save_es
		cld

		cmp		[GDC.a.writeMode],MGL_REPLACE_MODE
		jne     @@ScanlineFill
		cmp		[GDC.a.penStyle],MGL_BITMAP_SOLID
		je		@@FastFill

;---------------------------------------------------------------------------
; If we are filling with anything but a solid pattern, then repeatedly
; call the scanline routine to draw the rectangle.

@@ScanlineFill:
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
		mov		_si,[x2]
		sub		_si,[x1]			; _SI := X2 - X1 (# pixels to draw)
		mov		[cols],_si			; Save coloumns for later
		mov		_dx,[y2]
		sub		_dx,[y1]			; _DX := Y2 - Y1 (# rows to draw)
		mov     [rows],_dx			; Save rows for later
		mov		_di,[GDC.mi.bytesPerLine]
		sub		_di,_si				; _DI := buffer increment
		mov		al,[BYTE GDC.intColor]
		mov		ah,al
		push	_ax

		mov		_ax,[y1]
		mov		_cx,[x1]
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		xchg	_di,_bx				; ES:_DI -> buffer
									; _BX := buffer increment
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	pop		_ax					; AL := pixel color value
									; AH := pixel color value
@@RowLoop:
		mov		_cx,[cols]			; Restore number of pixels to fill
		mov		_si,_di				; Check to see if a bank switch will occur
		add		si,cx
		jc		@@BrokenScanline	; Yes, bank switch occurs in scanline
		test	_di,1
		jz		@@WordAligned
		stosb						; Store first pixel if odd to ensure
		dec		_cx					;  word alignment for 'rep stosw'
		jcxz	@@NextRow
@@WordAligned:
		shr		_cx,1
	rep stosw						; Fill in the row
		adc		_cx,_cx
	rep	stosb						; Store the last byte in row

@@NextRow:
		add		di,bx				; Increment to next row
		jc		@@ChangeBanks		; Need to change video banks
		dec		[rows]
		jnz		@@RowLoop			; Loop for all rows
		jmp		@@Exit

; Handle the case where a bank switch can occur in the middle of a scanline

@@BrokenScanline:
		sub		cx,si				; _CX := bytes in first bank
	rep	stosb						; Store bytes in first bank
		inc		dl
		call	_SVGA_setBank
if flatmodel
ifdef	USE_SELVMEM
		xor		_di,_di
else
		mov		_di,[GDC.surface]
endif
endif
		mov		cx,si				; CX := bytes in second bank
	rep	stosb						; Store bytes in second bank
		jmp		@@NextRow

@@ChangeBanks:
		inc		dl
		call	_SVGA_setBank
		dec		[rows]
		jnz		@@RowLoop			; Loop for all rows

@@Exit:	restore_es
		leave_c
		ret

procend		_SVGA8_fillRect

;----------------------------------------------------------------------------
; void SVGA8_putMonoImage(MGLDC *dc,int x,int y,int width,int height,
;	uchar far *image)
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
procstart	_SVGA8_putMonoImage

		ARG		dc:DPTR, x:UINT, y:UINT, byteWidth:UINT, height:UINT,		\
				image:DPTR

		LOCAL	bytesPerLine:UINT, cleft:UINT, ctop:UINT, cright:UINT, 		\
				cbottom:UINT, rowWidth:UINT, rowCount:UINT, rowAdjust:UINT,	\
				count:UINT, color:S_UCHAR, lmask:S_UCHAR, rmask:S_UCHAR,			\
				writeMode:S_UCHAR, bank:S_UCHAR = LocalSize

		enter_c	LocalSize
		save_ds
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
		mov		al,[BYTE MGLDC.a.writeMode]
		mov		[writeMode],al

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
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@ClipImage
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
		call	SVGA8_pixelAddrESBX	; ES:_BX -> buffer
		cmp		dl,[_SVGA_curBank]
		je		@@NC1
		call	_SVGA_setBank		; Program starting bank number
@@NC1:	mov		[bank],dl
		_lds	_si,[image]			; DS:_SI -> image buffer
		mov		_ax,[rowWidth]
		mov		_di,_ax
		shl		_di,3
		sub		_di,[bytesPerLine]
		neg		_di					; DI := buffer adjustment
		mov		dh,[writeMode]		; DH := write mode operation
		mov		dl,[color]			; DH := foreground color
		mov		_cx,[rowCount]		; CX := number of scanlines to process
		sub		_ax,[byteWidth]
		neg		_ax
		mov		[rowAdjust],_ax		; Save adjustment between rows
		cmp		[BYTE rowWidth],1
		jne		@@NextRowClipped

		mov		al,[rmask]			; Combine left and right masks if clipped
		and		[lmask],al			;  image is only 1 byte wide

@@NextRowClipped:
		mov		ah,[BYTE rowWidth]	; AH := width of image in bytes

; Load the first byte and mask with left clipping mask

		lodsb
		and		al,[lmask]
		dec		ah					; Decrement count of bytes to do
		jmp		@@PlotByte			; Go plot it

@@NextByteClipped:
		lodsb						; AL := pattern for next 8 pixels

@@PlotByte:
		cmp		dh,MGL_REPLACE_MODE
		jne		@@PlotByteROP
		push	_si
		mov		_si,_bx
		add		si,8				; Check for broken scanline
		jc		@@BrokenScanlineClipped
		shl		al,1
		jnc		@@SKC1
		mov		[_ES _bx],dl
@@SKC1:	shl		al,1
		jnc		@@SKC2
		mov		[_ES _bx+1],dl
@@SKC2:	shl		al,1
		jnc		@@SKC3
		mov		[_ES _bx+2],dl
@@SKC3:	shl		al,1
		jnc		@@SKC4
		mov		[_ES _bx+3],dl
@@SKC4:	shl		al,1
		jnc		@@SKC5
		mov		[_ES _bx+4],dl
@@SKC5:	shl		al,1
		jnc		@@SKC6
		mov		[_ES _bx+5],dl
@@SKC6:	shl		al,1
		jnc		@@SKC7
		mov		[_ES _bx+6],dl
@@SKC7:	shl		al,1
		jnc		@@SKC8
		mov		[_ES _bx+7],dl
@@SKC8:	add		_bx,8

@@SEnd:	pop		_si
@@DonePlot:
		dec		ah
		js		@@DoneRow			; Negative, so all done
		jnz		@@NextByteClipped	; Not zero, so continue

; Load the last byte for the row and mask with right clipping mask

		lodsb
		and		al,[rmask]
		dec		ah					; Decrement count of bytes to do
		jmp		@@PlotByte			; Go plot it

@@DoneRow:
		add		_si,[rowAdjust]		; increment to next line in image
		add		bx,di				; increment to next line in video buffer
		jc		@@ChangeBankClipped	; Moved out of bank?
		loop	@@NextRowClipped
		jmp		@@Exit

@@ChangeBankClipped:
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
		loop	@@NextRowClipped
		jmp		@@Exit

@@BrokenScanlineClipped:
		push	_cx
		mov		_cx,8
@@NextPixelClipped:
		shl		al,1
		jnc		@@SKC9
		mov		[_ES _bx],dl
@@SKC9:	add		bx,1
		jnc		@@NC2
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
@@NC2:	loop	@@NextPixelClipped
		pop		_cx
		jmp		@@SEnd

@@PlotByteROP:
		push	_cx
		mov		_cx,8

@@NextPixelROP:
		shl		al,1
		jnc		@@SkipROP
		cmp		dh,MGL_XOR_MODE
		jne		@@R1
		xor		[_ES _bx],dl
		jmp		@@SkipROP
@@R1:	cmp		dh,MGL_OR_MODE
		jne		@@R2
		or		[_ES _bx],dl
		jmp		@@SkipRop
@@R2:	and		[_ES _bx],dl
@@SkipRop:
		add    	bx,1
		jc		@@ChangeBankROP
@@REnd:	loop	@@NextPixelROP

		pop		_cx
		jmp		@@DonePlot

@@ChangeBankROP:
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
		jmp		@@REnd

;----------------------------------------------------------------------------
; Routine for non-clipped monochrome images. We can simply blast out the
; pixels without worrying about masking off the last few pixels in each
; line.
;
;		_AX	- y coordinate
;		_BX - x coordinate
;		_CX - width of image in pixels

@@DrawNonClipped:
		mov		_di,[bytesPerLine]
		sub		_di,_cx				; _DI := buffer adjustment
		shr		_cx,3
		mov		dh,cl				; DH := width of image in bytes
		mov		dl,[color]			; DL := foreground color
		mov		_cx,_bx
		_les	_bx,[dc]
		push	_dx
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> buffer
		cmp		dl,[_SVGA_curBank]
		je		@@NC3
		call	_SVGA_setBank		; Program starting bank number
@@NC3:	mov		[bank],dl			; Save current bank number
		pop		_dx
		_lds	_si,[image]			; DS:_SI -> image bit map
		mov		_cx,[height]		; CX := height of image in scanlines

@@NextRow:
		mov		ah,dh				; AH := width of image in bytes

; Plot all of the pixels in the current byte, using a fast unrolled loop

@@NextByte:
		lodsb						; AL := pattern for next 8 pixels
		push	_si
		mov		_si,_bx
		add		si,8				; Check for broken scanline
		jc		@@BrokenScanLine
		shl		al,1
		jnc		@@SK1
		mov		[_ES _bx],dl
@@SK1:	shl		al,1
		jnc		@@SK2
		mov		[_ES _bx+1],dl
@@SK2:	shl		al,1
		jnc		@@SK3
		mov		[_ES _bx+2],dl
@@SK3:	shl		al,1
		jnc		@@SK4
		mov		[_ES _bx+3],dl
@@SK4:	shl		al,1
		jnc		@@SK5
		mov		[_ES _bx+4],dl
@@SK5:	shl		al,1
		jnc		@@SK6
		mov		[_ES _bx+5],dl
@@SK6:	shl		al,1
		jnc		@@SK7
		mov		[_ES _bx+6],dl
@@SK7:	shl		al,1
		jnc		@@SK8
		mov		[_ES _bx+7],dl
@@SK8:	add		_bx,8
@@Next:	dec		ah
		pop		_si
		jnz		@@NextByte

		add		bx,di				; increment to next line in video buffer
		jc		@@ChangeBank		; Need to change bank number
		loop	@@NextRow
		jmp		@@Exit

@@ChangeBank:
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
		loop	@@NextRow
		jmp		@@Exit

@@BrokenScanline:
		push	_cx
		mov		_cx,8
@@NextPixel:
		shl		al,1
		jnc		@@SK9
		mov		[_ES _bx],dl
@@SK9:	add		bx,1
		jnc		@@NC4
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
@@NC4:	loop	@@NextPixel
		pop		_cx
		jmp		@@Next

@@Exit:	restore_es
		restore_ds
		leave_c
		ret

procend		_SVGA8_putMonoImage

if flatmodel

;----------------------------------------------------------------------------
; void MGLAPI SVGA8_putImage(MGLDC *dc,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,void *surface,
;	int bytesPerLine)
;----------------------------------------------------------------------------
; Blt an image from a memory buffer onto the device. We handle a number of
; special cases as quickly as possible in here:
;
;		- Full size Blt (from a another device of the same dimensions)
;		- Bit where the destination is DWORD aligned on both x coordinates.
;		- Partial Blt in REPLACE mode
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
procstart	_SVGA8_putImage

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR,		\
				bytesPerLine:UINT

		LOCAL	bufAdjust:UINT, count:UINT = LocalSize

		enter_c	LocalSize
		save_ds
		save_es
		cld

; Determine special cases

		cmp		[op],MGL_REPLACE_MODE
		jne     @@ROP

		_les	_bx,[dc]
		mov		_ax,[top]
		mul		[bytesPerLine]
		add		_ax,[left]			; _AX := offset into source buffer
		mov		_di,_ax
		mov		_dx,[right]
		sub		_dx,[left]			; _DX := number of bytes to move
		mov		[count],_dx
		sub		[bytesPerLine],_dx	; Adjust source buffer increment
		mov		_ax,[MGLDC.mi.bytesPerLine]
		sub     _ax,_dx
		mov		[bufAdjust],_ax		; Save destination buffer increment
		mov		_ax,[dstTop]
		mov		_cx,[dstLeft]
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> device surface
		cmp		dl,[_SVGA_curBank]
		je		@@NCF
		call	_SVGA_setBank
@@NCF:	_lds	_si,[surface]
		add		_si,_di				; DS:_SI -> system memory buffer
		mov		_di,_bx				; ES:_DI -> device surface
		mov     _bx,[bottom]
		sub		_bx,[top]			; _BX := number of rows

		test	[left],3
		jne		@@NextRow
		test	[right],3
		jne		@@NextRow

; Partial Blt, where the left and right coordinates of the source are dword
; aligned.

@@NextRowDWord:
		mov		_ax,_di
		mov		_cx,[count]			; CX := dwords to move
		add		ax,cx
		jc		@@BrokenScanlineDWord
		shr		_cx,2
	rep	movsd

@@DoneMoveDWord:
		add     _si,[bytesPerLine]	; Move to next line
		add		di,[WORD bufAdjust]	; Move to next line
		jnc		@@NCDW
		inc		dl
		call	_SVGA_setBank
@@NCDW:	dec		_bx
		jnz		@@NextRowDWord
		jmp		@@Exit

@@BrokenScanlineDWord:
		sub		cx,ax				; CX := bytes in first bank
		push	_cx
		shr		_cx,2
	rep	movsd						; Move pixels in row 32 bits at a time
		pop		_cx
		and		_cx,3
	rep	movsb						; Move the last bytes in row
		inc		dl
		call	_SVGA_setBank		; Change to new bank
if flatmodel
ifdef	USE_SELVMEM
		xor		edi,edi
else
		sub		edi,10000h
endif
endif
		mov		cx,ax				; CX := bytes in second bank
		push	_cx
		shr		_cx,2
	rep	movsd						; Move pixels in row 32 bits at a time
		pop		_cx
		and		_cx,3
	rep	movsb						; Move the last bytes in row
		jmp		@@DoneMoveDWord

; Partial Blt, which we extend to word boundaries for speed

@@NextRow:
		mov		_ax,_di
		mov		_cx,[count]			; CX := bytes to move
		add		ax,cx
		jc		@@BrokenScanline
		push	_cx
		shr		_cx,2
	rep	movsd						; Move pixels in row 32 bits at a time
		pop		_cx
		and		_cx,3
	rep	movsb						; Move the last bytes in row

@@DoneMove:
		add     _si,[bytesPerLine]	; Move to next line
		add		di,[WORD bufAdjust]	; Move to next line
		jnc		@@NCW
		inc		dl
		call	_SVGA_setBank
@@NCW:	dec		_bx
		jnz		@@NextRow
		jmp		@@Exit

@@BrokenScanline:
		sub		cx,ax				; CX := bytes in first bank
		shr		_cx,1
	rep	movsw
		adc		_cx,_cx
	rep	movsb
		inc		dl
		call	_SVGA_setBank		; Change to new bank
if flatmodel
ifdef	USE_SELVMEM
		xor		edi,edi
else
		sub		edi,10000h
endif
endif
		mov		cx,ax				; CX := bytes in second bank
		shr		_cx,1
	rep	movsw
		adc		_cx,_cx
	rep	movsb
		jmp		@@DoneMove

@@Exit:	restore_es
		restore_ds
		leave_c
		ret

; Simply call the C version to handle this

@@ROP:	restore_es
		restore_ds
		leave_c
		jmp		___EMU__putImage

procend		_SVGA8_putImage

endif

;----------------------------------------------------------------------------
; void SVGA8_getDivot(MGLDC *dc,int left,int top,int right,int bottom,
;	void *divot)
;----------------------------------------------------------------------------
; Routine to get a block of video information from the current display
; page. This routine extends the block to a dword aligned boundary.
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
;	short	bankOffset;				- Starting bank in video ram
;	} divotHeader;
;
;----------------------------------------------------------------------------
procstart	_SVGA8_getDivot

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				divot:DPTR

		LOCAL	bytesPerLine:UINT, bank:S_UCHAR, bufferAdjust:S_USHORT = LocalSize

		enter_c	LocalSize
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov		_dx,[MGLDC.mi.bytesPerLine]
		mov		[bytesPerLine],_dx	; Save bytes per line value on stack

		mov     _ax,[top]
		mov		_cx,[left]
		and		cl,0FCh				; Mask to even word boundary
		mov		[left],_cx
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC1
		call	_SVGA_setBank
@@NC1:	mov		[bank],dl
		ds_eq_es_swap
		mov		_si,_bx				; DS:_SI -> display surface
		_les	_di,[divot]			; ES:_DI -> buffer in system RAM

; Build the bit block header

		mov		_ax,[right]
		add		_ax,3
		and		al,0FCh				; Mask to even DWORD boundary
		sub		_ax,[left]			; _AX := # pixel columns
		mov		_dx,_ax				; _DX := # pixel columns
		stosw						; bytes per pixel row

		mov		_ax,[bottom]
		sub		_ax,[top]
		mov		_bx,_ax				; _BX := # pixel rows
		stosw						; number of rows
		mov		_ax,_si
		STOSINT						; Starting offset in buffer
		mov		al,[bank]
		stosw						; Starting bank in buffer

		mov		_ax,[bytesPerLine]
		sub		_ax,_dx				; _AX := buffer adjust for row
		mov		[bufferAdjust],ax

; Now check to see if we are about to go off the end of the 64k system
; RAM segment, and adjust the segment if we are... (16 bit mode only)

@@NextRow:
ife flatmodel
		mov		cx,di
		add		cx,dx
		jnc		@@NoWrap			; No wrap, so continue

; The pointer will wrap while reading this row, so adjust the segment
; number and normalise the pointer.

		push	bx
		mov		cx,di
		shr		cx,4				; CX := paragraph's in segment
		mov		bx,es
		add		bx,cx
		shl		cx,4
		sub		di,cx				; Adjust offset
		mov		es,bx				; ES:DI -> new system RAM segment
		pop		bx

@@NoWrap:
endif

; Check to see if a bank switch occurs during the transfer of this scanline,
; special casing it if it does.

		mov		_cx,_dx				; CX := # pixels in row
		mov		_ax,_si
		add		ax,cx
		jc		@@BrokenScanline	; Bank switch will occur
		shr		_cx,2
	rep	movsd						; Move all pixels in row

@@CompleteRow:
		add		si,[bufferAdjust]	; Increment to next row in video buffer
		jc		@@ChangeBanks		; Bank switch occurs
		dec		_bx
		jnz		@@NextRow			; Loop for all rows in image
		jmp		@@Exit

@@ChangeBanks:
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
		dec		_bx
		jnz		@@NextRow
		jmp		@@Exit

@@BrokenScanline:
		sub		cx,ax				; CX := bytes in first bank
		shr		_cx,2
	rep	movsd						; Move dwords in first bank
		mov		cx,ax				; CX := bytes in second bank
		shr		_cx,2
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
if flatmodel
ifdef	USE_SELVMEM
		xor		esi,esi
else
		sub		esi,10000h
endif
endif
	rep	movsd						; Move data in second bank
		jmp		@@CompleteRow

@@Exit: restore_es
		restore_ds
		leave_c
		ret

procend		_SVGA8_getDivot

;----------------------------------------------------------------------------
; void SVGA8_putDivot(MGLDC *dc,void *divot)
;----------------------------------------------------------------------------
; Routine to copy a block of video information from system memory to the
; currently active display page. The block is a special dword aligned piece
; of memory and is returned to the same position it was saved from.
;
; Entry:		dc		- Device context to dump divot to
;				divot	- Pointer to place to store image data
;
;----------------------------------------------------------------------------
procstart	_SVGA8_putDivot

		ARG		dc:DPTR, divot:DPTR

		LOCAL	BytesPerLine:UINT, bank:S_UCHAR, bufferAdjust:S_USHORT = LocalSize

		enter_c	LocalSize
		save_ds
		save_es
		cld

		_les	_bx,[dc]
		mov		_dx,[MGLDC.mi.bytesPerLine]
		mov		[bytesPerLine],_dx	; Save bytes per line value on stack
		_lds	_si,[divot]			; DS:_SI -> buffer in system RAM
ifdef	USING_DS
		mov		es,[WORD (MGLDC.surface)+2]
endif

; Extract position info from image header

		xor		_ax,_ax
		lodsw
		mov		_dx,_ax				; DX := # pixel columns
		lodsw
		mov     _bx,_ax             ; BX := # pixel rows
		LODSINT
		mov		_di,_ax				; ES:_DI := offset in video ram
		mov		_ax,[bytesPerLine]
		sub		_ax,_dx				; _AX := buffer adjustment
		mov		[bufferAdjust],ax
		lodsw
		mov		[bank],al			; Save starting bank number
		xchg	dl,al
		call	_SVGA_setBank
		xchg	dl,al

; Now check to see if we are about to go off the end of the 64k system
; RAM segment, and adjust the segment if we are...

@@NextRow:
ife flatmodel
		mov		cx,si
		add		cx,dx
		jnc		@@NoWrap			; No wrap, so continue

; The pointer will wrap while reading this row, so adjust the segment
; number and normalise the pointer.

		push	bx
		mov		cx,si
		shr		cx,4				; CX := paragraph's in segment
		mov		bx,ds
		add		bx,cx
		shl		cx,4
		sub		si,cx				; Adjust offset
		mov		ds,bx				; DS:SI -> new system RAM segment
		pop		bx

@@NoWrap:
endif
		mov		_cx,_dx				; CX := # pixels in row
		mov		ax,di
		add		ax,cx				; Check for bank switch
		jc		@@BrokenScanline
		shr		_cx,2
	rep	movsd						; Move all pixels in row

@@CompleteRow:
		add		di,[bufferAdjust]	; Increment to next row in video buffer
		jc		@@ChangeBanks
		dec		_bx
		jnz		@@NextRow			; Loop for all rows in image
		jmp		@@Exit

@@ChangeBanks:
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
		dec		_bx
		jnz		@@NextRow
		jmp		@@Exit

@@BrokenScanline:
		sub		cx,ax				; CX := bytes in first bank
		shr		_cx,2
	rep	movsd						; Move bytes in first bank
		mov		cx,ax				; CX := bytes in second bank
		shr		_cx,2
		xchg	dl,[bank]
		inc		dl
		call	_SVGA_setBank
		xchg	dl,[bank]
if flatmodel
ifdef	USE_SELVMEM
		xor		edi,edi
else
		sub		edi,10000h
endif
endif
	rep	movsd						; Move data in second bank
		jmp		@@CompleteRow

@@Exit:	restore_es
		restore_ds
		leave_c
		ret

procend		_SVGA8_putDivot

;----------------------------------------------------------------------------
; void SVGA8_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer)
;----------------------------------------------------------------------------
; Stretches a scanline of information to the device context from the memory
; buffer. Works only in REPLACE mode.
;
; Entry:	dc		- Device context
;			y		- Scanline to put data to
;			x1		- Starting coordinate in scanline
;			count	- Number of pixels in source scanline
;			buffer	- Pointer to buffer to get data from
;
;----------------------------------------------------------------------------
procstart	_SVGA8_stretchScanLine2x

		ARG		dc:DPTR, y:UINT, x1:UINT, count:UINT, buffer:DPTR

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[count]			; _CX := # pixel columns

		mov     _ax,_di				; Check for bankswitch in scanline
		mov		_bx,_cx
		shl		_bx,1
		add		ax,bx
		jc		@@BrokenScanline	; Yep, so special case it

@@NextPixel:
		mov		al,[_si]
		mov		ah,al
		mov		[_ES _di],ax
		inc		_si
		inc		_di
		inc		_di
		dec		_cx
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@BrokenScanline:
		mov		al,[_si]
		mov		[_ES _di],al
		add		di,1
		jc		@@ChangeBank1
@@B1:	mov		[_ES _di],al
		add		di,1
		jc		@@ChangeBank2
@@B2:	inc		_si
		dec		_cx
		jnz		@@BrokenScanline
		jmp		@@Exit

@@ChangeBank1:
		inc		dl
		call	_SVGA_setBank		; Change to new bank
		jmp		@@B1

@@ChangeBank2:
		inc		dl
		call	_SVGA_setBank		; Change to new bank
		jmp		@@B2

procend		_SVGA8_stretchScanLine2x

;----------------------------------------------------------------------------
; void SVGA8_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,
;	int *repCounts)
;----------------------------------------------------------------------------
; Stretches a scanline of information to the device context from the memory
; buffer. Works only in REPLACE mode.
;
; Entry:	dc			- Device context
;			y			- Scanline to put data to
;			x1			- Starting coordinate in scanline
;			count		- Number of pixels in source scanline
;			buffer		- Pointer to buffer to get data from
;			repCounts	- Array of pixel repetition counts
;
;----------------------------------------------------------------------------
procstart	_SVGA8_stretchScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, count:UINT, buffer:DPTR,	\
				repCounts:DPTR

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		_lfs	_bx,[repCounts]		; FS:_BX -> repetition count buffer
		mov		_bp,[count]			; _BP := # pixel columns

@@NextPixel:
		mov		_cx,[_FS _bx]		; Replication count for this pixel
		jcxz	@@Skip
		mov		al,[_si]			; Get source pixel
@@Rep:	mov		[_ES _di],al
		add		di,1
		jc		@@ChangeBanks
@@B:	dec		_cx
		jnz		@@Rep
@@Skip:	inc		_si
		add		_bx,size UINT
		dec		_bp
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@ChangeBanks:
		inc		dl
		call	_SVGA_setBank		; Change to new bank
		jmp		@@B

procend		_SVGA8_stretchScanLine

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; void SVGA8_drawScanList(int y,int length,short *scans);
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
; Entry:	dc		- Device context
;			y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_SVGA8_drawScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		LOCAL	bytesPerLine:S_USHORT, color:S_USHORT = LocalSize

		enter_c	LocalSize
		save_ds
		save_es
		cld							; String instructions go up

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
		mov		_ax,[GDC.mi.bytesPerLine]
		mov		[bytesPerLine],ax
		mov		al,[BYTE GDC.intColor]
		mov		ah,al
		mov		[color],ax			; Save color value for later
		mov		_ax,[y]				; AX := top coordinate
		xor		_cx,_cx
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		mov		_di,_bx
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		dh,dl				; DH := current bank number

; Get the address of the scanline array in DS:SI

		_lds	_si,[scans]			; DS:_SI -> scanline array data
		mov		_cx,[len]			; CX := number of scanlines to fill

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
		push	_cx					; Preserve CX
		push	_di					; Preserve DI
		push	_si

		mov		_cx,_ax				; _CX := xend
		sub		_cx,_bx				; _CX := xend - xstart (# pixels to draw)
		add		di,bx				; ES:_DI -> first pixel to fill2
		jnc		@@NC2

; Bank switch occurs before the scanline is drawn

		mov		dl,dh
		inc		dl
		call	_SVGA_setBank

@@NC2:	mov		ax,[color]			; AL,AH := pixel color
		mov		_si,_di
		add		si,cx
		jc		@@BrokenScanLine	; Bank switch occurs in scanline

		test	_di,1
		jz		@@WordAligned
		stosb						; Store first byte to ensure word
		dec		_cx					;  alignment during next phase
@@WordAligned:
		shr		_cx,1
	rep	stosw						; Store all middle bytes fast!
		adc		_cx,_cx
	rep	stosb						; Store the last byte
		jmp		@@DoneFill

; Handle the case where a bank switch can occur in the middle of a scanline

@@BrokenScanline:
		sub		cx,si				; CX := bytes in first bank
	rep	stosb						; Store bytes in first bank
		mov		dl,dh
		inc		dl
		call	_SVGA_setBank		; Change to new bank
if flatmodel
ifdef	USE_SELVMEM
		xor		_di,_di
else
		mov		_di,[GDC.surface]
endif
endif
		mov		cx,si				; CX := bytes in second bank
	rep	stosb						; Store bytes in second bank

@@DoneFill:
		pop		_si
		pop		_di					; Restore DI
		pop		_cx					; Restore CX

@@SkipLine:
		add		di,[bytesPerLine]	; Increment address to next scanline
		jc		@@ChangeBanks
		loop	@@FillLoop			; Loop for all scanlines
		jmp		@@Exit

@@ChangeBanks:
		inc		dh
		cmp		dl,dh
		je		@@NC3				; Bank switch has already been handled!
		mov		dl,dh
		call	_SVGA_setBank
@@NC3:	loop	@@FillLoop
		jmp		@@Exit

;----------------------------------------------------------------------------
; Slower scanline filling routine. This routine simply calls scanline to
; draw each scanline.

@@SlowFill:
		mov		_di,[y]				; DI := top coordinate

; Get the address of the scanline array in ES:SI

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
		leave_c
		ret

procend		_SVGA8_drawScanList

;----------------------------------------------------------------------------
; void SVGA8_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,
;	color_t transparent)
;----------------------------------------------------------------------------
; Renders a scanline of a bitmap with transparency.
;
; Entry:	dc			- Device context
;			y			- Scanline to put data to
;			x1			- Starting coordinate in scanline
;			x2			- Second coordinate in scanline
;			buffer		- Pointer to buffer to get data from
;			transparent	- Transparent source color to ignore
;
;----------------------------------------------------------------------------
procstart	_SVGA8_putSrcTransparent

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR,			\
				transparent:UINT

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		_bx,[UINT transparent]

		mov     _ax,_di				; Check for bankswitch in scanline
		add		ax,cx
		jc		@@BrokenScanline	; Yep, so special case it
		xor		_ax,_ax

@@NextPixel:
		mov		al,[_si]
		cmp		_ax,_bx
		je		@@Skip
		mov		[_ES _di],al
@@Skip:	inc		_si
		inc		_di
		dec		_cx
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@BrokenScanline:
		xor		_ax,_ax
@@NextBroken:
		mov		al,[_si]
		cmp		_ax,_bx
		je		@@Skp2
		mov		[_ES _di],al
@@Skp2:	inc		_si
		add		di,1
		jc		@@ChangeBanks
@@B:	dec		_cx
		jnz		@@NextBroken
		jmp		@@Exit

@@ChangeBanks:
		inc		dl
		call	_SVGA_setBank		; Change to new bank
		jmp		@@B

procend		_SVGA8_putSrcTransparent

;----------------------------------------------------------------------------
; void SVGA8_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,
;	color_t transparent)
;----------------------------------------------------------------------------
; Renders a scanline of a bitmap with transparency.
;
; Entry:	dc			- Device context
;			y			- Scanline to put data to
;			x1			- Starting coordinate in scanline
;			x2			- Second coordinate in scanline
;			buffer		- Pointer to buffer to get data from
;			transparent	- Transparent source color to ignore
;
;----------------------------------------------------------------------------
procstart	_SVGA8_putDstTransparent

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR,			\
				transparent:UINT

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA8_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		_bx,[UINT transparent]

		mov     _ax,_di				; Check for bankswitch in scanline
		add		ax,cx
		jc		@@BrokenScanline	; Yep, so special case it

@@NextPixel:
		cmp		[_ES _di],bl
		jne		@@Skip
		mov		al,[_si]
		mov		[_ES _di],al
@@Skip:	inc		_si
		inc		_di
		dec		_cx
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@BrokenScanline:
@@NextBroken:
		cmp		[_ES _di],bl
		jne		@@Skp2
		mov		al,[_si]
		mov		[_ES _di],al
@@Skp2:	inc		_si
		add		di,1
		jc		@@ChangeBanks
@@B:	dec		_cx
		jnz		@@NextBroken
		jmp		@@Exit

@@ChangeBanks:
		inc		dl
		call	_SVGA_setBank		; Change to new bank
		jmp		@@B

procend		_SVGA8_putDstTransparent

;----------------------------------------------------------------------------
; int SVGA8_scanLeftForColor(int x,int y,color_t border)
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
procstart	_SVGA8_scanLeftForColor

		ARG		x:UINT, y:UINT, border:COLOR_T

		enter_c	0
		save_es
		std							; Move down in memory

; Calculate pixel address (x,y)

		mov		_ax,[y]				; AX := y
		mov		_cx,[x]
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> buffer
		mov		al,[BYTE border]	; AL := border pixel to search for
		mov		_cx,[x]
		inc		_cx					; CX := number of bytes to scan
		mov		_si,_di
		sub		si,cx
		jc		@@BrokenScanline	; Bank switch occurs in scanline

; Scan video buffer for the first occurance of the border pixel

		repne	scasb				; scan until border pixel is found
		jne		@@NotFound			; Border pixel was not found

		mov		_ax,_cx				; Return coordinate of pixel found
		jmp		@@Exit

; Handle the case where a bankswitch occurs in a scanline

@@BrokenScanline:
		cmp		[_ES _di],al
		je		@@Found
		sub		di,1
		jc		@@ChangeBank
		loop	@@BrokenScanline
		jmp		@@NotFound

@@Found:
		mov		_ax,_cx
		dec		_ax
		jmp		@@Exit

@@ChangeBank:
		dec		dl
		call	_SVGA_setBank
		mov		_di,0FFFFh
		loop	@@BrokenScanline

@@NotFound:
		mov		_ax,-1				; Indicate failure of search

@@Exit:	restore_es
		leave_c_nolocal
		ret

procend		_SVGA8_scanLeftForColor

;----------------------------------------------------------------------------
; int SVGA8_scanRightForColor(int x,int y,color_t border)
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
procstart	_SVGA8_scanRightForColor

		ARG		x:UINT, y:UINT, border:COLOR_T

		LOCAL	bytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		save_es
		cld

; Calculate pixel address (x,y)

		mov		_ax,[GDC.mi.bytesPerLine]
		mov		[bytesPerLine],_ax
		mov		_ax,[y]				; AX := y
		mov		_cx,[x]
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:DI -> buffer
		mov		al,[BYTE border]	; AL := border pixel to search for
		mov     _cx,[x]
		sub		_cx,[bytesPerLine]
		neg		_cx					; _CX := BytesPerLine - x
		mov		_si,_di
		add		si,cx
		jc		@@BrokenScanline	; Bank switch occurs in scanline

; Scan video buffer for the first occurance of the border pixel

		repne	scasb				; scan until border pixel is found
		mov		_ax,[bytesPerLine]			; Flag the border was not found
		jne		@@Exit				; Border pixel was not found

		sub		_ax,_cx
		dec		_ax					; Return coordinate of pixel found
		jmp		@@Exit

@@BrokenScanline:
		cmp		[_ES _di],al
		je		@@Found
		add		di,1
		jc		@@ChangeBank
		loop	@@BrokenScanline
		jmp		@@NotFound

@@ChangeBank:
		inc		dl
		call	_SVGA_setBank
if flatmodel
ifdef	USE_SELVMEM
		xor		_di,_di
else
		mov		_di,[GDC.surface]
endif
endif
		loop	@@BrokenScanline

@@NotFound:
		mov		_ax,[bytesPerLine]
		jmp		@@Exit

@@Found:
		mov		_ax,[bytesPerLine]
		sub		_ax,_cx

@@Exit: restore_es
		leave_c
		ret

procend		_SVGA8_scanRightForColor

;----------------------------------------------------------------------------
; int SVGA8_scanLeftWhileColor(int x,int y,color_t oldcolor)
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
procstart	_SVGA8_scanLeftWhileColor

		ARG		x:UINT, y:UINT, oldcolor:COLOR_T

		enter_c	0
		save_es
		std							; Move down in memory

; Calculate pixel address (x,y)

		mov		_ax,[y]				; AX := y
		mov		_cx,[x]
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		cmp     dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> buffer
		mov		al,[BYTE oldColor]	; AL := border pixel to search for
		mov		_cx,[x]
		inc		_cx					; CX := number of bytes to scan
		mov		_si,_di
		sub     si,cx
		jc		@@BrokenScanline

; Scan video buffer for the first occurance of the border pixel

		repe	scasb				; scan until pixel color changes
		je		@@NotFound			; Border pixel was not found

		mov		_ax,_cx				; Return coordinate of pixel found
		jmp		@@Exit

@@BrokenScanline:
		cmp		[_ES _di],al
		jne		@@Found
		sub		di,1
		jc		@@ChangeBanks
		loop	@@BrokenScanline
		jmp		@@NotFound

@@Found:
		mov		_ax,_cx
		dec		_ax
		jmp		@@Exit

@@ChangeBanks:
		dec		dl
		call	_SVGA_setBank
		mov		_di,0FFFFh
		loop	@@BrokenScanline

@@NotFound:
		mov		_ax,-1				; Inidicate failure of search

@@Exit:	restore_es
		leave_c_nolocal
		ret

procend		_SVGA8_scanLeftWhileColor

;----------------------------------------------------------------------------
; int SVGA8_scanRightWhileColor(int x,int y,color_t oldcolor)
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
procstart	_SVGA8_scanRightWhileColor

		ARG		x:UINT, y:UINT, oldcolor:COLOR_T

		LOCAL	bytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		save_es
		cld

; Calculate pixel address (x,y)

		mov		_ax,[GDC.mi.bytesPerLine]
		mov		[bytesPerLine],_ax
		mov		_ax,[y]				; AX := y
		mov		_cx,[x]
		call	SVGA8_pixelAddr		; ES:DL:_BX -> buffer
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:DI -> buffer
		mov		al,[BYTE oldColor]	; AL := border pixel to search for
		mov     _cx,[x]
		sub		_cx,[bytesPerLine]
		neg		_cx					; _CX := BytesPerLine - x
		mov		_si,_di
		add		si,cx
		jc		@@BrokenScanline

; Scan video buffer for the first occurance of the border pixel

		repe	scasb				; scan until pixel color changes
		mov		_ax,[bytesPerLine]			; Flag the border was not found
		je		@@Exit				; Border pixel was not found

		sub		_ax,_cx
		dec		_ax					; Return coordinate of pixel found
		jmp		@@Exit

@@BrokenScanline:
		cmp		[_ES _di],al
		jne		@@Found
		add		di,1
		jc		@@ChangeBanks
		loop	@@BrokenScanline
		jmp		@@NotFound

@@ChangeBanks:
		inc		dl
		call	_SVGA_setBank
if flatmodel
ifdef	USE_SELVMEM
		xor		_di,_di
else
		mov		_di,[GDC.surface]
endif
endif
		loop	@@BrokenScanline

@@NotFound:
		mov		_ax,[bytesPerLine]
		jmp		@@Exit

@@Found:
		mov		_ax,[bytesPerLine]
		sub		_ax,_cx

@@Exit:	restore_es
		leave_c
		ret

procend		_SVGA8_scanRightWhileColor

endif	; !MGL_LITE

endcodeseg	_SVGA8

		END							; End of module
