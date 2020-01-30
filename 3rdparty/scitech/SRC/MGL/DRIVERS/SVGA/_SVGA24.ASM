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
;* Description:	SVGA24, 24 bit bank switched assembler based routines. This
;*				code has been written to correctly handle the case where
;*              bank boundaries can occur in the middle of a pixel as
;*				efficiently as possible.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"
include "mgldos\biosdata.equ"

header	_SVGA24

	EXTRN	_SVGA_setBank:FPTR
	EXTRN	___EMU__scanLine:FPTR

begdataseg	_SVGA24

	$EXTRN	_SVGA_curBank,BYTE
	$EXTRN	__MGL_dc,devCtx_s

enddataseg	_SVGA24

begcodeseg	_SVGA24

;----------------------------------------------------------------------------
; SVGA24_pixelAddrESBX	Determine buffer address of pixel in 16 bit modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				ES_BX	->  device context
;				_CX		-	x-coordinate
;
; Exit:			ES:_BX	->	offset in buffer
; 				DX		-   64k bank number of the address
;
; Registers:	None
;
;----------------------------------------------------------------------------
procstart	SVGA24_pixelAddrESBX

		imul	[USHORT MGLDC.mi.bytesPerLine]; DX:AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@1
		dec		dl						; Decrement bank number if negative
if	flatmodel
		and		ecx,0FFFFh
endif
@@1:	add     ax,cx
		adc		dx,0
		shl		cx,1
		add		cx,ax
		adc		dl,[BYTE MGLDC.bankOffset] ; DL := bank number
		lVidDC	_bx
		add		_bx,_cx
		ret

procend		SVGA24_pixelAddrESBX

;----------------------------------------------------------------------------
; SVGA24_pixelAddr	Determine buffer address of pixel in 16 bit modes
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
procstart	SVGA24_pixelAddr

		imul	[USHORT GDC.mi.bytesPerLine]; DX:AX := y * BytesPerLine
		or		_cx,_cx
		jns		@@1
		dec		dl						; Decrement bank number if negative
if	flatmodel
		and		ecx,0FFFFh
endif
@@1:	add     ax,cx
		adc		dx,0
		shl		cx,1
		add		cx,ax
		adc		dl,[BYTE GDC.bankOffset] ; DL := bank number
		lVid	_bx
		add		_bx,_cx
		ret

procend		SVGA24_pixelAddr

;----------------------------------------------------------------------------
; color_t SVGA24_getPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine reads the value of a pixel in 16 bit packed pixel format
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
; Exit:			_DX:_AX	-	Color of pixel at (x,y)
;
;----------------------------------------------------------------------------
procstart	_SVGA24_getPixel

		ARG		x:UINT, y:UINT

		enter_c	0
		save_es

; Compute the pixel's address in video buffer

		mov		_ax,[y]
		mul		[USHORT GDC.mi.bytesPerLine]; AX := y * BytesPerLine
		mov		_cx,[x]
		add		ax,cx
		adc		dx,0
		shl		_cx,1
		add		ax,cx
		adc		dl,[BYTE GDC.bankOffset] ; DL := bank number
		lVid	_bx
		add		_bx,_ax

		cmp		dl,[_SVGA_curBank]
		je		@@NoChange
		call	_SVGA_setBank

@@NoChange:
		xor		_ax,_ax
		xor		dh,dh
		cmp		bx,0FFFDh
		jae		@@SlowVersion		; Arrgh! Bank switch in pixel!!

		mov		ax,[_ES _bx]			; AX := pixel value
		mov		dl,[_ES _bx+2]		; DL:AX := pixel value

@@Exit:
if flatmodel
		shl		edx,16
		or		eax,edx
endif
		restore_es
		leave_c_nolocal
		ret

@@SlowVersion:
		mov		al,[_ES _bx]
		call	IncBXDL
		mov		ah,[_ES _bx]		; AX := pixel value
		call	IncBXDL
		mov		dl,[_ES _bx]		; DL:AX := pixel value
		jmp		@@Exit

procend		_SVGA24_getPixel

;----------------------------------------------------------------------------
; void SVGA24_putPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in 16 bit VESA modes.
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
;----------------------------------------------------------------------------
procstart	_SVGA24_putPixel

		ARG		x:UINT, y:UINT

		enter_c	0
		save_es

; Compute the pixel's address in video buffer

		mov		_ax,[y]
		mov     _cx,[x]
		mul		[USHORT GDC.mi.bytesPerLine]; AX := y * BytesPerLine
		add		ax,cx
		adc		dx,0
		shl		_cx,1
		add		ax,cx
		adc		dl,[BYTE GDC.bankOffset] ; DL := bank number
		mov		ecx,[GDC.intColor]
		mov		dh,[BYTE GDC.a.writeMode]
		lVid	_bx
		add		_bx,_ax

		cmp		dl,[_SVGA_curBank]
		je		@@NoChange
		call	_SVGA_setBank

@@NoChange:
		mov		ax,cx				; AX := first two bytes
		shr		ecx,16				; CL := last byte
		cmp		bx,0FFFEh
		jae		@@SlowVersion		; Bank switch occurs in pixel!

		cmp		dh,MGL_REPLACE_MODE
		jne		@@1
		mov		[_ES _bx],ax		; Replace the pixel
		mov		[_ES _bx+2],cl
		jmp		@@Exit

@@1:	cmp		dh,MGL_XOR_MODE
		jne		@@2
		xor		[_ES _bx],ax		; XOR the pixel
		xor		[_ES _bx+2],cl
		jmp		@@Exit

@@2:	cmp		dh,MGL_OR_MODE
		jne		@@3
		or		[_ES _bx],ax		; OR the pixel
		or		[_ES _bx+2],cl
		jmp		@@Exit

@@3:	and		[_ES _bx],ax		; AND the pixel
		and		[_ES _bx+2],cl
		jmp		@@Exit

@@SlowVersion:
		mov		dl,dh				; DL := write mode operation
		mov		dh,cl				; DH := top byte of color value
		call	DrawPixelSlow		; Draw the pixel slowly

@@Exit:	restore_es
		leave_c_nolocal
		ret

procend		_SVGA24_putPixel

;----------------------------------------------------------------------------
; void SVGA24_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
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
procstart	_SVGA24_getScanLine

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
		call	SVGA24_pixelAddrESBX; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	ds_eq_es_swap
		mov		_si,_bx				; DS:_SI -> display surface
		_les	_di,[buffer]		; ES:_DI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		_ax,_cx
		shl		_cx,1
		add		_cx,_ax				; _CX := bytes to move
		cmp		[op],MGL_REPLACE_MODE
		jne		@@DoROP

		mov     _ax,_si				; Check for bankswitch in scanline
		add		ax,cx
		jc		@@BrokenScanline	; Yep, so special case it
		xor		_dx,_dx
		shr		_cx,1
		adc		_dx,_dx				; _DX := count of odd bytes
		shr		_cx,1
	rep	movsd						; Move pixels in row 32 bits at a time
		adc		_cx,_cx
	rep	movsw						; Move the last word in row
		mov		_cx,_dx
	rep	movsb						; Move the last byte in row

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
	rep	movsw						; Store bytes in first bank
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
@@NCX:	xor		[_ES _di],al
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
@@NCO:	or		[_ES _di],al
		inc		_di
		loop	@@LoopOR
		jmp		@@Exit

@@LoopAND:
		mov		al,[_si]
		add     _si,1
		jnc		@@NCA
		inc		dl
		call	_SVGA_setBank
@@NCA:	and		[_ES _di],al
		inc		_di
		loop	@@LoopAND
		jmp		@@Exit

procend		_SVGA24_getScanLine

;----------------------------------------------------------------------------
; void SVGA24_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
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
procstart	_SVGA24_putScanLine

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
		call	SVGA24_pixelAddrESBX; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		_ax,_cx
		shl		_cx,1
		add		_cx,_ax				; _CX := bytes to move
		cmp		[op],MGL_REPLACE_MODE
		jne		@@DoROP

		mov     _ax,_di				; Check for bankswitch in scanline
		add		ax,cx
		jc		@@BrokenScanline	; Yep, so special case it
		xor		_dx,_dx
		shr		_cx,1
		adc		_dx,_dx				; _DX := count of odd bytes
		shr		_cx,1
	rep	movsd						; Move pixels in row 32 bits at a time
		adc		_cx,_cx
	rep	movsw						; Move the last word in row
		mov		_cx,_dx
	rep	movsb						; Move the last byte in row

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
		xor		edi,edi
else
		sub		edi,10000h
endif
endif
		mov		cx,ax				; CX := bytes in second bank
		shr		_cx,1
	rep	movsw						; Store bytes in first bank
		adc		_cx,_cx
	rep	movsb
		jmp		@@Exit

; Handle ROP code blitting

@@DoROP:
		cmp		[op],MGL_XOR_MODE
		jne		@@1

@@LoopXOR:
		lodsb
		xor		[_ES _di],al
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
		or		[_ES _di],al
		add		di,1
		jnc		@@NCO
		inc		dl
		call	_SVGA_setBank
@@NCO:	loop	@@LoopOR
		jmp		@@Exit

@@LoopAND:
		lodsb
		and		[_ES _di],al
		add		di,1
		jnc		@@NCA
		inc		dl
		call	_SVGA_setBank
@@NCA:	loop	@@LoopAND
		jmp		@@Exit

procend		_SVGA24_putScanline

;----------------------------------------------------------------------------
; IncBXDL	Increment the BX/DL offset bank number combination
;----------------------------------------------------------------------------
;
; This routine is called in place to increment the value of DL:BX where
; DL is the current bank offset, and BX is the current frame buffer offset.
; The routine also ensures that the bank boundary is correctly crossed. It
; is slow, but only gets called for about less than 10 pixels on the entire
; display page.
;
; Entry:		BX	- Video buffer offset
;				DL	- Video bank number
;
; Exit:			BX	- New buffer offset (+1)
;				DL	- New bank bumber (carried over from BX)
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	IncBXDL	near

		add		bx,1
		adc		dl,0
		call	_SVGA_setBank
		ret

ENDP	IncBXDL

;----------------------------------------------------------------------------
; DrawPixelSlow		Draws the pixel slowly with correct drawing mode
;----------------------------------------------------------------------------
;
; Draws the pixel taking into account that a bank boundary occurs in the
; middle of the pixel.
;
; Entry:		DH:AX	- Color of the pixel to plot
;				DL		- Write mode operation
;				ES:BX	- Address of pixel to plot
;				CurBank	- Current bank number
;
; Exit:			DH:AX	- Color of pixel to plot
;				DL		- Write mode operation
;				ES:BX	- Address of pixel to plot + 3
;				CurBank	- Current bank number + 1
;
; Registers:	All preserved.
;
;----------------------------------------------------------------------------
PROC	DrawPixelSlow	near

		push	_cx
		push	_dx
		mov		cl,dl			; CL := write mode op
if flatmodel
		mov		dl,[_SVGA_curBank]
else
		push	ds
		push	ax
		mov		ax,DGROUP
		mov		ds,ax
		pop		ax
		mov		dl,[_SVGA_curBank]
		pop		ds
endif
		cmp		cl,MGL_REPLACE_MODE
		jne		@@1
		mov		[_ES _bx],al	; Replace the pixel
		call    IncBXDL
		mov		[_ES _bx],ah
		call	IncBXDL
		mov		[_ES _bx],dh
		call    IncBXDL
		jmp		@@Exit

@@1:	cmp		cl,MGL_XOR_MODE
		jne		@@2
		xor		[_ES _bx],al	; XOR the pixel
		call    IncBXDL
		xor		[_ES _bx],ah
		call    IncBXDL
		xor		[_ES _bx],dh
		call    IncBXDL
		jmp		@@Exit

@@2:	cmp		cl,MGL_OR_MODE
		jne		@@3
		or		[_ES _bx],al	; OR the pixel
		call    IncBXDL
		or		[_ES _bx],ah
		call    IncBXDL
		or		[_ES _bx],dh
		call    IncBXDL
		jmp		@@Exit

@@3:	and		[_ES _bx],al	; AND the pixel
		call    IncBXDL
		and		[_ES _bx],ah
		call    IncBXDL
		and		[_ES _bx],dh
		call    IncBXDL

@@Exit:
		pop		_dx
		pop		_cx
		ret

ENDP	DrawPixelSlow

;----------------------------------------------------------------------------
; void SVGA24_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a line in 24 bit VESA modes.
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
procstart	_SVGA24_line

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		LOCAL	Routine:NCPTR, VertInc:ULONG, EIncr:FIX32_T, 			\
				NEIncr:FIX32_T, color:ULONG, writeMode:S_UCHAR,			\
				varDX:FIX32_T, varDY:FIX32_T, x:UINT, y:UINT = LocalSize

		enter_c	LocalSize
		save_es

; Load the current color

		mov		eax,[GDC.intColor]
		mov		[color],eax
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
		call	SVGA24_pixelAddr		; ES:DL:_BX -> buffer
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
		call	SVGA24_pixelAddr		; ES:DL:_BX -> buffer
		pop		_cx					; restore reg
		cmp		dl,[_SVGA_curBank]
		je		@@NoChange
		call	_SVGA_setBank

@@NoChange:
		mov		ax,[WORD color]
		mov		dh,[BYTE color+2]
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@V1

; Draw the line

@@Set1: cmp		bx,0FFFDh
		jae     @@BankSwitch1
@@R1:	mov		[_ES _bx],ax			; set pixel value in buffer
		mov		[_ES _bx+2],dh
		add		bx,si				; increment to next position of interleave
		jc      @@ChangeBank1
@@Ret1:	loop	@@Set1				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank1:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret1

@@BankSwitch1:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		add		bx,si
		sub		bx,3
		inc		dl
		jmp		@@R1

@@V1:   cmp		[writeMode],MGL_XOR_MODE
		jne		@@V2

@@Set2: cmp		bx,0FFFDh
		jae		@@BankSwitch2
@@R2:	xor		[_ES _bx],ax			; set pixel value in buffer
		xor		[_ES _bx+2],dh
		add		bx,si				; increment to next position of interleave
		jc		@@ChangeBank2
@@Ret2:	loop	@@Set2				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank2:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret2

@@BankSwitch2:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		add		bx,si
		sub		bx,3
		inc		dl
		jmp		@@R2

@@V2:	cmp		[writeMode],MGL_OR_MODE
		jne		@@Set4

@@Set3: cmp		bx,0FFFDh
		jae		@@BankSwitch3
@@R3:	or		[_ES _bx],ax			; set pixel value in buffer
		or		[_ES _bx+2],dh
		add		bx,si				; increment to next position of interleave
		jc		@@ChangeBank3
@@Ret3:	loop	@@Set3				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank3:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret3

@@BankSwitch3:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		add		bx,si
		sub		bx,3
		inc		dl
		jmp		@@R3

@@Set4: cmp		bx,0FFFDh
		jae		@@BankSwitch4
@@R4:	and		[_ES _bx],ax			; set pixel value in buffer
		and		[_ES _bx+2],dh
		add		bx,si				; increment to next position of interleave
		jc		@@ChangeBank3
@@Ret4:	loop	@@Set4				; loop down the line
		jmp		@@Exit				; We are done...

@@ChangeBank4:
		inc		dl
		call	_SVGA_setBank
		jmp		@@Ret4

@@BankSwitch4:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		add		bx,si
		sub		bx,3
		inc		dl
		jmp		@@R4

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
		mov		ax,[WORD color]
		mov     dh,[BYTE color+2]
		mov		esi,[EIncr]
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@LoopLo2

@@LoopLo1:
		cmp		dl,[_SVGA_curBank]
		je		@@SetLo1

		call	_SVGA_setBank		; Change video banks

@@SetLo1:
		cmp		bx,0FFFDh
		jae		@@BankSwitchLo1
		mov		[_ES _bx],ax			; Set pixel value in buffer
		mov		[_ES _bx+2],dh
		add		bx,3				; Increment x coordinate
		adc		dl,0

@@DonePixelLo1:
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

@@BankSwitchLo1:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		inc		dl
		jmp     @@DonePixelLo1

@@LoopLo2:
		cmp		dl,[_SVGA_curBank]
		je		@@SetLo2

		call	_SVGA_setBank		; Change video banks

@@SetLo2:
		cmp		bx,0FFFDh
		jae		@@BankSwitchLo2
		cmp		[writeMode],MGL_XOR_MODE
		jne		@@L1
		xor		[_ES _bx],ax
		xor		[_ES _bx+2],dh
		jmp		@@LOut
@@L1:   cmp		[writeMode],MGL_OR_MODE
		jne		@@L2
		or		[_ES _bx],ax
		or		[_ES _bx+2],dh
		jmp		@@LOut
@@L2:	and		[_ES _bx],ax
		and		[_ES _bx+2],dh

@@LOut:	add		bx,3				; Increment x coordinate
		adc		dl,0

@@DonePixelLo2:
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

@@BankSwitchLo2:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		inc		dl
		jmp     @@DonePixelLo1

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
		mov		ax,[WORD color]		; DH:AX := pixel value to fill
		mov		dh,[BYTE color+2]
		mov		esi,[EIncr]
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@LoopHi2

@@LoopHi1:
		cmp		dl,[_SVGA_curBank]
		je      @@SetHi1

		call	_SVGA_setBank

@@SetHi1:
		cmp		bx,0FFFDh
		jae		@@BankSwitchHi1

		mov		[_ES _bx],ax			; Set pixel value in buffer
		mov		[_ES _bx+2],dh

@@DonePixelHi1:
		add		bx,[WORD vertInc]	; increment y
		adc		dl,[BYTE vertInc+2]
		or		edi,edi				; Test sign of d
		jg		@@HiPosDi1			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@LoopHi1			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi1:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		bx,3				; Increment x
		adc		dl,0
		loop	@@LoopHi1			; Loop for remaining pixels
		jmp		@@Exit

@@BankSwitchHi1:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		sub		bx,3
		jmp     @@DonePixelHi1

@@LoopHi2:
		cmp		dl,[_SVGA_curBank]
		je		@@SetHi2

		call	_SVGA_setBank		; Change video banks

@@SetHi2:
		cmp		bx,0FFFDh
		jae		@@BankSwitchHi2

		cmp		[writeMode],MGL_XOR_MODE
		jne		@@H1
		xor		[_ES _bx],ax
		xor		[_ES _bx+2],dh
		jmp		@@Hout
@@H1:	cmp		[writeMode],MGL_OR_MODE
		jne		@@H2
		or		[_ES _bx],ax
		or		[_ES _bx+2],dh
		jmp		@@HOut
@@H2:   and		[_ES _bx],ax
		and		[_ES _bx+2],dh

@@HOut:	add		bx,[WORD vertInc]	; increment y
		adc		dl,[BYTE vertInc+2]
		or		edi,edi				; Test sign of d
		jg		@@HiPosDi2			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		loop	@@LoopHi2			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi2:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		bx,3				; Increment x
		adc		dl,0
		loop	@@LoopHi2			; Loop for remaining pixels
		jmp		@@Exit

@@BankSwitchHi2:
		push	edx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		sub		bx,3
		jmp     @@HOut

@@Exit:	restore_es
		leave_c
		ret

procend		_SVGA24_line

;----------------------------------------------------------------------------
; void SVGA24_scanLine(int y,int x1,int x2)
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
procstart	_SVGA24_scanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		LOCAL	count:UINT, writeMode:S_UCHAR,							\
				color:ULONG, bkColor:ULONG, penStyle:S_UCHAR,			\
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
		mov		eax,[GDC.intColor]
		mov		[color],eax

; Get the pattern byte for this scan line from pattern array

@@ComputeAddress:
		mov		_ax,[y]
		mov		_cx,[x1]
		call	SVGA24_pixelAddr	; ES:DL:_BX -> buffer
		cmp		dl,[_SVGA_curBank]
		je		@@NC1
		call	_SVGA_setBank		; Change to starting bank number
@@NC1:	cmp		[penStyle],MGL_BITMAP_SOLID
		jne		@@Patterned

;----------------------------------------------------------------------------
; Draw a solid patterned line

		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		mov		dl,[writeMode]
		mov		_cx,[count]			; _CX := number of pixels to draw
		cmp		dl,MGL_REPLACE_MODE
		jne		@@SolidROP

@@LoopSolid:
		cmp		bx,0FFFDh
		jae		@@BankSwitch1		; Bank switch occurs!

		mov		[_ES _bx],ax			; Set pixel value in buffer
		mov		[_ES _bx+2],dh
		add		bx,3				; Increment to next pixel
@@SEnd:	loop	@@LoopSolid			; Loop across line
		jmp		@@Exit

@@BankSwitch1:
		call	DrawPixelSlow
		jmp		@@SEnd

@@SolidROP:
		cmp		bx,0FFFDh
		jae		@@BankSwitch2		; Bank switch occurs!

		cmp		dl,MGL_XOR_MODE
		jne		@@S1
		xor		[_ES _bx],ax		; Set pixel value in buffer
		xor		[_ES _bx+2],dh
		jmp		@@SOut
@@S1:	cmp		dl,MGL_OR_MODE
		jne		@@S2
		or		[_ES _bx],ax		; Set pixel value in buffer
		or		[_ES _bx+2],dh
		jmp		@@SOut
@@S2:	and		[_ES _bx],ax		; Set pixel value in buffer
		and		[_ES _bx+2],dh
@@SOut:	add		bx,3				; Increment to next pixel
@@SRE:	loop	@@SolidROP			; Loop across line
		jmp		@@Exit

@@BankSwitch2:
		call	DrawPixelSlow
		jmp		@@SRE

@@Patterned:
		mov		eax,[GDC.intBackColor]
		mov		[bkColor],eax

; Get the pattern byte for this scan line from pattern array

		mov		_si,[y]
		and		_si,7				; Mask to 0-7
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
		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@TransLoopROP

@@TransLoop:
		rol		dl,1				; Rotate pattern mask
		jnc		@@SkipTrans			; Draw pixel if pattern bit was 1

		cmp		bx,0FFFDh
		jae		@@BankSwitch3		; Bank switch occurs!
		mov		[_ES _bx],ax		; Set pixel value in buffer
		mov		[_ES _bx+2],dh

@@SkipTrans:
		add		bx,3				; Increment to next pixel
		jc		@@ChangeBank2
@@TEnd:	loop	@@TransLoop			; Loop across line
		jmp		@@Exit

@@ChangeBank2:
		push	_dx
		mov		dl,[_SVGA_curBank]
		inc		dl
		call	_SVGA_setBank
		pop		_dx
		jmp		@@TEnd

@@BankSwitch3:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		jmp		@@TEnd

@@TransLoopROP:
		rol		dl,1				; Rotate pattern mask
		jnc		@@SkipTransROP		; Draw pixel if pattern bit was 1

		cmp		bx,0FFFDh
		jae		@@BankSwitch4		; Bank switch occurs!
		cmp		[writeMode],MGL_XOR_MODE
		jne		@@T1
		xor		[_ES _bx],ax
		xor		[_ES _bx+2],dh
		jmp		@@SkipTransROP
@@T1:	cmp		dh,MGL_OR_MODE
		jne		@@T2
		or		[_ES _bx],ax
		or		[_ES _bx+2],dh
		jmp		@@SkipTransROP
@@T2:	and		[_ES _bx],ax
		and		[_ES _bx+2],dh

@@SkipTransROP:
		add		bx,3				; Increment to next pixel
		jc		@@ChangeBank3
@@TRE:	loop	@@TransLoopROP		; Loop across line
		jmp		@@Exit

@@ChangeBank3:
		push	_dx
		mov		dl,[_SVGA_curBank]
		inc		dl
		call	_SVGA_setBank
		pop		_dx
		jmp		@@TRE

@@BankSwitch4:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		jmp		@@TRE

;----------------------------------------------------------------------------
; Draw an opaque patterned line

@@DoOpaque:
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@OpaqueLoopROP

@@OpaqueLoop:
		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		rol		dl,1				; Rotate pattern mask
		jc		@@SetPixelOpaque	; Draw pixel if pattern bit was 1

		mov		ax,[WORD bkColor]	; AX := pixel color
		mov		dh,[BYTE bkColor+2]	; DH := top byte of pixel color

@@SetPixelOpaque:
		cmp		bx,0FFFDh
		jae		@@BankSwitch5		; Bank switch occurs!
		mov		[_ES _bx],ax		; Set pixel value in buffer
		mov		[_ES _bx+2],dh
		add		bx,3				; Increment to next pixel
@@OEnd:	loop	@@OpaqueLoop		; Loop across line
		jmp		@@Exit

@@BankSwitch5:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
		pop		_dx
		jmp		@@OEnd

@@OpaqueLoopROP:
		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		rol		dl,1				; Rotate pattern mask
		jc		@@SetPixelOpaqueROP	; Draw pixel if pattern bit was 1

		mov		ax,[WORD bkColor]	; AX := pixel color
		mov		dh,[BYTE bkColor+2]	; DH := top byte of pixel color

@@SetPixelOpaqueROP:
		cmp		bx,0FFFDh
		jae		@@BankSwitch6		; Bank switch occurs!
		cmp		[writeMode],MGL_XOR_MODE
		jne		@@TR1
		xor		[_ES _bx],ax
		xor		[_ES _bx+2],dh
		jmp		@@OOut
@@TR1:	cmp		dh,MGL_OR_MODE
		jne		@@TR2
		or		[_ES _bx],ax
		or		[_ES _bx+2],dh
		jmp		@@OOut
@@TR2:	and		[_ES _bx],ax
		and		[_ES _bx+2],dh

@@OOut:	add		bx,3				; Increment to next pixel
@@ORE:	loop	@@OpaqueLoopROP		; Loop across line
		jmp		@@Exit

@@BankSwitch6:
		push	_dx
		mov		dl,[writeMode]
		call	DrawPixelSlow
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

procend		_SVGA24_scanLine

;----------------------------------------------------------------------------
; void SVGA24_fillRect(int x1,int y1,int x2,int y2)
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
procstart	_SVGA24_fillRect

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
		mov		_di,_si
		shl		_si,1
		add		_si,_di				; _SI := bytes per line
		mov		_di,[GDC.mi.bytesPerLine]
		sub		_di,_si				; _DI := buffer increment
		mov		eax,[GDC.intColor]
		push	eax

		mov		_ax,[y1]
		mov		_cx,[x1]
		call	SVGA24_pixelAddr	; ES:DL:_BX -> buffer

		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	pop		eax					; EAX := pixel color
		mov		ecx,eax
		shr		ecx,8
		mov		dh,ch				; DH:AX := pixel color

@@RowLoop:
		mov		_cx,[cols]			; Restore number of pixels to fill

@@NextPixel:
		cmp		bx,0FFFDh
		jae		@@BankSwitch		; Bank switch in pixel!!

		mov		[_ES _bx],ax			; Set pixel value in buffer
		mov		[_ES _bx+2],dh
		add		bx,3				; Increment to next pixel
@@SEnd:	loop	@@NextPixel			; Loop across line

@@NextRow:
		add		bx,di				; Increment to next row
		jc		@@ChangeBanks		; Need to change video banks
		dec		[rows]
		jnz		@@RowLoop			; Loop for all rows
		jmp		@@Exit

@@BankSwitch:
		push    _dx
		mov		dl,MGL_REPLACE_MODE
		call	DrawPixelSlow
		pop		_dx
		inc		dl
		jmp		@@SEnd

@@ChangeBanks:
		inc		dl
		call	_SVGA_setBank
		dec		[rows]
		jnz		@@RowLoop			; Loop for all rows

@@Exit:	restore_es
		leave_c
		ret

procend		_SVGA24_fillRect

;----------------------------------------------------------------------------
; void SVGA24_getDivot(MGLDC *dc,int left,int top,int right,int bottom,
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
procstart	_SVGA24_getDivot

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
		and		cl,0FCh				; Mask to even dword boundary
		mov		[left],_cx
		call	SVGA24_pixelAddrESBX; ES:DL:_BX -> display surface
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
		mov		_dx,_ax
		shl		_ax,1
		add		_dx,_ax				; _DX := bytesperline
		mov		_ax,_dx
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

procend		_SVGA24_getDivot

;----------------------------------------------------------------------------
; void SVGA24_putDivot(MGLDC *dc,void *divot)
;----------------------------------------------------------------------------
; Routine to copy a block of video information from system memory to the
; currently active display page. The block is a special dword aligned piece
; of memory and is returned to the same position it was saved from.
;
; Entry:		dc		- Device context to dump divot to
;				divot	- Pointer to place to store image data
;
;----------------------------------------------------------------------------
procstart	_SVGA24_putDivot

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

procend		_SVGA24_putDivot

;----------------------------------------------------------------------------
; void SVGA24_stretchScanLine2x(MGLDC *dc,int y,int x1,int count,void *buffer)
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
procstart	_SVGA24_stretchScanLine2x

		ARG		dc:DPTR, y:UINT, x1:UINT, count:UINT, buffer:DPTR

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA24_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[count]			; _CX := # pixel columns

@@NextPixel:
		mov		ax,[_si]
		mov		dh,[_si+2]
		cmp     di,-6
		jae		@@DrawSlow
		mov		[_ES _di],ax
		mov		[_ES _di+2],dh
		mov		[_ES _di+3],ax
		mov		[_ES _di+5],dh
		add		_di,6
@@B:	inc		_si
		inc		_si
		inc		_si
		dec		_cx
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@DrawSlow:
		xchg	_bx,_di
		xor		dl,dl				; DL := write mode for slow pixel draw
		call	DrawPixelSlow
		xchg	_bx,_di
		jmp		@@B

procend		_SVGA24_stretchScanLine2x

;----------------------------------------------------------------------------
; void SVGA24_stretchScanLine(MGLDC *dc,int y,int x1,int count,void *buffer,
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
procstart	_SVGA24_stretchScanLine

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
		call	SVGA24_pixelAddrESBX	; ES:DL:_BX -> display surface
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
		mov		ax,[_si]			; Get source pixel
		mov		dh,[_si+2]
@@Rep:  cmp		di,-3
		jae		@@DrawSlow
		mov		[_ES _di],ax
		mov		[_ES _di+2],dh
		inc		_di
		inc		_di
		inc		_di
@@B:	dec		_cx
		jnz		@@Rep
@@Skip:	inc		_si
		inc		_si
		inc		_si
		add		_bx,size UINT
		dec		_bp
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@DrawSlow:
		xchg	_bx,_di
		xor		dl,dl				; DL := write mode for slow pixel draw
		call	DrawPixelSlow
		xchg	_bx,_di
		jmp		@@B

procend		_SVGA24_stretchScanLine

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; void SVGA24_drawScanList(int y,int length,short *scans);
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
procstart	_SVGA24_drawScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		LOCAL	bytesPerLine:S_USHORT, color:ULONG = LocalSize

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
		mov		eax,[GDC.intColor]
		mov		[color],eax			; Save color value for later
		mov		_ax,[y]				; AX := top coordinate
		xor		_cx,_cx
		call	SVGA24_pixelAddr	; ES:DL:_BX -> buffer
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
		mov		si,bx
		shl		bx,1
		add		bx,si
		add		di,bx				; ES:_DI -> first pixel to fill2
		jnc		@@NC2

; Bank switch occurs before the scanline is drawn

		mov		dl,dh
		inc		dl
		call	_SVGA_setBank

@@NC2:	mov		ax,[WORD color]		; AX := pixel color
		xchg	dh,[BYTE color+2]	; DH := top byte of pixel color

@@LoopSolid:
		cmp		di,0FFFDh
		jae		@@BankSwitch1		; Bank switch occurs!

		mov		[_ES _di],ax			; Set pixel value in buffer
		mov		[_ES _di+2],dh
		add		di,3				; Increment to next pixel
		loop	@@LoopSolid			; Loop across line
		jmp		@@DoneFill

@@BankSwitch1:
		push	_dx
		mov		dl,MGL_REPLACE_MODE
		xchg	_bx,_di
		call	DrawPixelSlow
		xchg	_bx,_di
		pop		_dx
		inc		dl					; We have changed banks now
		loop	@@LoopSolid

@@DoneFill:
		xchg	dh,[BYTE color+2]	; DH := bank number
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

procend		_SVGA24_drawScanList

;----------------------------------------------------------------------------
; void SVGA24_putSrcTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,
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
procstart	_SVGA24_putSrcTransparent

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR,			\
				transparent:COLOR_T

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA24_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		bx,[WORD transparent]
		mov		dl,[BYTE transparent+2]

@@NextPixel:
		mov		ax,[_si]
		mov		dh,[_si+2]
		cmp		ax,bx
		jne		@@Draw
		cmp     dl,dh
		je		@@Skip
@@Draw:	cmp		di,-3
		jae		@@DrawSlow
		mov		[_ES _di],ax
		mov		[_ES _di+2],dh
@@Skip:	add		_di,3
		jc		@@ChangeBanks
@@B:	inc		_si
		inc		_si
		inc		_si
		dec		_cx
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@DrawSlow:
		push	_dx
		xchg	_bx,_di
		xor		dl,dl				; DL := write mode for slow pixel draw
		call	DrawPixelSlow
		xchg	_bx,_di
		pop		_dx
		jmp		@@B

@@ChangeBanks:
		push	_dx
if flatmodel
		mov		dl,[_SVGA_curBank]
else
		push	ds
		push	ax
		mov		ax,DGROUP
		mov		ds,ax
		pop		ax
		mov		dl,[_SVGA_curBank]
		pop		ds
endif
		inc		dl
		call	_SVGA_setBank		; Change to new bank
		pop		_dx
		jmp		@@B

procend		_SVGA24_putSrcTransparent

;----------------------------------------------------------------------------
; void SVGA24_putDstTransparent(MGLDC *dc,int y,int x1,int x2,void *buffer,
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
procstart	_SVGA24_putDstTransparent

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR,			\
				transparent:COLOR_T

		enter_c	0
		save_ds
		save_es
		cld

; Compute position of bit block in video buffer

		_les	_bx,[dc]
		mov     _ax,[y]
		mov		_cx,[x1]
		call	SVGA24_pixelAddrESBX	; ES:DL:_BX -> display surface
		cmp		dl,[_SVGA_curBank]
		je		@@NC
		call	_SVGA_setBank
@@NC:	mov		_di,_bx				; ES:_DI -> display surface
		_lds	_si,[buffer]		; DS:_SI -> buffer in system RAM
		mov		_cx,[x2]
		sub		_cx,[x1]			; _CX := # pixel columns
		mov		bx,[WORD transparent]
		mov		dl,[BYTE transparent+2]

@@NextPixel:
		cmp		[_ES _di],bx
		jne		@@Skip
		cmp     [_ES _di],dh
		jne		@@Skip
		mov		ax,[_si]
		mov		dh,[_si+2]
		cmp		di,-3
		jae		@@DrawSlow
		mov		[_ES _di],ax
		mov		[_ES _di+2],dh
@@Skip:	add		_di,3
		jc		@@ChangeBanks
@@B:	inc		_si
		inc		_si
		inc		_si
		dec		_cx
		jnz		@@NextPixel

@@Exit:	restore_es
		restore_ds
		leave_c_nolocal
		ret

@@DrawSlow:
		push	_dx
		xchg	_bx,_di
		xor		dl,dl				; DL := write mode for slow pixel draw
		call	DrawPixelSlow
		xchg	_bx,_di
		pop		_dx
		jmp		@@B

@@ChangeBanks:
		push	_dx
if flatmodel
		mov		dl,[_SVGA_curBank]
else
		push	ds
		push	ax
		mov		ax,DGROUP
		mov		ds,ax
		pop		ax
		mov		dl,[_SVGA_curBank]
		pop		ds
endif
		inc		dl
		call	_SVGA_setBank		; Change to new bank
		pop		_dx
		jmp		@@B

procend		_SVGA24_putDstTransparent

endif	; !MGL_LITE

endcodeseg	_SVGA24

		END							; End of module
