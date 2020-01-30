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
;* Description:	PACKED24, 24 bit packed pixel device context routines.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"

if flatmodel

header	_packed24

	EXTRN	___EMU__scanLine:FPTR
	EXTRN	___EMU__putImage:FPTR

begdataseg	_packed24

	$EXTRN	__MGL_dc,devCtx_s

gcount			UINT	?				; Global counter variable
gcolor			UINT	?				; Global scratch color variable
gzwidth			UINT	?				; Zbuffer width in bytes
gzleftAdj		UINT	?				; Zbuffer left adjustment coord
gc1				UINT	?				; Color value
gcSlope			UINT	?				; Color slope

enddataseg	_packed24

begcodeseg	_packed24

;----------------------------------------------------------------------------
; PACKED24_pixelAddrEBX	Determine buffer address of pixel in 24 bit modes
;----------------------------------------------------------------------------
;
; Entry:		EAX		-	y-coordinate
;				ES_BX	->  device context
;				ECX		-	x-coordinate
;
; Exit:			EBX	->	offset in buffer (linear framebuffer code)
;				EBX	-> 	offset in buffer (memory DC code)
;
; Registers:	none.
;
;----------------------------------------------------------------------------
procstatic	PACKED24_pixelAddrEBX

		push	edx
		imul	[MGLDC.mi.bytesPerLine]	; AX := y * BytesPerLine
		mov     ebx,[MGLDC.surface]		; EBX -> start of video memory
		add     ebx,ecx
		shl		ecx,1
		add		ebx,ecx
		add		ebx,eax					; EBX := y * bytesPerLine + x*3
		pop		edx
		ret

procend		PACKED24_pixelAddrEBX

;----------------------------------------------------------------------------
; PACKED24_pixelAddr	Determine buffer address of pixel in 24 bit modes
;----------------------------------------------------------------------------
;
; Entry:		EAX		-	y-coordinate
;				ECX		-	x-coordinate
;
; Exit:			EBX	->	offset in buffer (linear framebuffer code)
;				EBX	-> 	offset in buffer (memory DC code)
;
; Registers:	none.
;
;----------------------------------------------------------------------------
procstatic	PACKED24_pixelAddr

		push	edx
		imul	[GDC.mi.bytesPerLine]	; AX := y * BytesPerLine
		mov     ebx,[GDC.surface]		; EBX -> start of video memory
		add     ebx,ecx
		shl		ecx,1
		add		ebx,ecx
		add		ebx,eax					; EBX := y * bytesPerLine + x*3
		pop		edx
		ret

procend		PACKED24_pixelAddr

;----------------------------------------------------------------------------
; color_t _PACKED24_getPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine reads the value of a pixel in 24 bit packed pixel format
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
; Exit:			EDX:EAX	-	Color of pixel at (x,y)
;
;----------------------------------------------------------------------------
procstart	_PACKED24_getPixel

		ARG		x:UINT, y:UINT

		enter_c	0

		mov		eax,[y]
		mul		[GDC.mi.bytesPerLine]	; EDX:EAX := y * BytesPerLine

; Compute the pixel's address in video buffer

		mov     ebx,[GDC.surface]	; EBX -> start of surface
		add		ebx,eax
		mov		eax,[x]
		add		ebx,eax
		shl		eax,1
		add		ebx,eax				; EBX := y*BytesPerLine + x*3
		xor		eax,eax
		xor		edx,edx
		mov		ax,[ebx]
		mov		dl,[ebx+2]		; DL:AX := pixel value
		shl		edx,16
		or		eax,edx
		leave_c_nolocal
		ret

procend		_PACKED24_getPixel

;----------------------------------------------------------------------------
; void _PACKED24_putPixel(int x,int y)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in 24 bit packed pixel format.
;
; Entry:		x	-	X coordinate of pixel to read
;				y	-	Y coordinate of pixel to read
;
;----------------------------------------------------------------------------
procstart	_PACKED24_putPixel

		ARG		x:UINT, y:UINT

		enter_c	0

; Compute the pixel's address in video buffer

		mov		eax,[y]
		mov     ecx,[x]
		mul		[GDC.mi.bytesPerLine]	; EAX := y * BytesPerLine
		add		eax,ecx
		shl		ecx,1
		add		eax,ecx				; EAX := y * BytesPerLine + x*3
		mov		ecx,[GDC.intColor]
		mov		dh,[BYTE GDC.a.writeMode]
		mov     ebx,[GDC.surface]	; EBX -> start of surface
		add		ebx,eax				; EBX := y*BytesPerLine + x
		mov		eax,ecx				; AX := first two bytes
		shr		ecx,16				; CL := last byte

		cmp		dh,MGL_REPLACE_MODE
		jne		@@1
		mov		[ebx],ax		; Replace the pixel
		mov		[ebx+2],cl
		jmp		@@Exit

@@1:	cmp		dh,MGL_XOR_MODE
		jne		@@2
		xor		[ebx],ax		; XOR the pixel
		xor		[ebx+2],cl
		jmp		@@Exit

@@2:	cmp		dh,MGL_OR_MODE
		jne		@@3
		or		[ebx],ax		; OR the pixel
		or		[ebx+2],cl
		jmp		@@Exit

@@3:	and		[ebx],ax		; AND the pixel
		and		[ebx+2],cl

@@Exit:	leave_c_nolocal
		ret

procend		_PACKED24_putPixel

;----------------------------------------------------------------------------
; void _PACKED24_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
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
procstart	_PACKED24_getScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		enter_c	0
		cld

; Compute position of bit block in video buffer

		mov 	ebx,[dc]
		mov     eax,[y]
		mov		ecx,[x1]
		call	PACKED24_pixelAddrEBX; EBX -> display surface
		mov		esi,ebx				; DS:ESI -> display surface
		mov 	edi,[buffer]		; EDI -> buffer in system RAM

CommonGetPutScanline:
		mov		ecx,[x2]
		sub		ecx,[x1]
		mov		eax,ecx
		shl		eax,1
		add		ecx,eax				; ECX := # pixel columns
		cmp		[op],MGL_REPLACE_MODE
		jne		@@DoROP

		xor		edx,edx
		shr		ecx,1
		adc		edx,edx				; EDX := count of odd bytes
		shr		ecx,1
	rep	movsd						; Move pixels in row 32 bits at a time
		adc		ecx,ecx
	rep	movsw						; Move the last word in row
		mov		ecx,edx
	rep	movsb						; Move the last byte in row

@@Exit:	leave_c_nolocal
		ret

; Handle ROP code blitting

@@DoROP:
		cmp		[op],MGL_XOR_MODE
		jne		@@1

		_ALIGN	4

@@LoopXOR:
		lodsb
		xor		[es:edi],al
		inc		edi
		dec		ecx
		jnz		@@LoopXOR
		jmp		@@Exit

@@1:	cmp		[op],MGL_OR_MODE
		jne		@@LoopAND

		_ALIGN	4

@@LoopOR:
		lodsb
		or		[es:edi],al
		inc		edi
		dec		ecx
		jnz		@@LoopOR
		jmp		@@Exit

		_ALIGN	4

@@LoopAND:
		lodsb
		and		[es:edi],al
		inc		edi
		dec		ecx
		jnz		@@LoopAND
		jmp		@@Exit

procend		_PACKED24_getScanLine

;----------------------------------------------------------------------------
; void _PACKED24_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,
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
procstart	_PACKED24_putScanLine

		ARG		dc:DPTR, y:UINT, x1:UINT, x2:UINT, buffer:DPTR, bufx:UINT,	\
				op:UINT

		enter_c	0
		cld

; Compute position of bit block in video buffer

		mov 	ebx,[dc]
		mov     eax,[y]
		mov		ecx,[x1]
		call	PACKED24_pixelAddrEBX; EBX -> display surface
		mov		edi,ebx				; EDI -> display surface
		mov 	esi,[buffer]		; DS:ESI -> buffer in system RAM
		jmp     CommonGetPutScanline

procend		_PACKED24_putScanline

;----------------------------------------------------------------------------
; void _PACKED24_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a line for 24 bit packed pixel devices.
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
procstart	_PACKED24_line

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, EIncr:FIX32_T, 			\
				NEIncr:FIX32_T, color:ULONG, writeMode:S_UCHAR,			\
				varDX:FIX32_T, varDY:FIX32_T, x:UINT, y:UINT = LocalSize

		enter_c	LocalSize

; Load the current color

		mov		eax,[GDC.intColor]
		mov		[color],eax
		mov		al,[BYTE GDC.a.writeMode]
		mov		[writeMode],al
		mov		esi,[GDC.mi.bytesPerLine]

; Check for vertical line

		mov		eax,[x2]
		sub		eax,[x1]			; EAX := X2 - X1
		jz		@@VertLine			; jump if vertical line...

; calcluate dy = ABS(Y2-Y1)

		mov		ebx,[y2]
		sub		ebx,[y1]			; EBX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		ebx					; EBX := Y1 - Y2
		neg		esi					; negative increment for buffer

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[vertInc],esi		; save increment
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
		mov		[y],eax				; Save initial x coordinate
		mov		ecx,[x1]
		add		ecx,8000h			; Round to integer value
		shr		ecx,16
		mov		[x],ecx				; Save initial y coordinate
		call	PACKED24_pixelAddr	; EBX -> buffer
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for vertical lines
;
;****************************************************************************

@@VertLine:
		mov		eax,[y1]
		add		eax,8000h
		shr		eax,16				; EAX := Y1
		mov		ebx,[y2]
		add		ebx,8000h
		shr		ebx,16				; EBX := Y2
		mov		ecx,ebx
		sub		ecx,eax				; ECX := dy
		jge		@@PosDy				; Jump if dy >= 0

		neg		ecx					; force dy >= 0
		mov		eax,ebx				; EAX := Y2

@@PosDy:
		inc		ecx					; ECX := # of pixels to draw
		push	ecx					; preserve this reg
		mov		ecx,[x1]
		add		ecx,8000h
		shr		ecx,16				; ECX := x
		call	PACKED24_pixelAddr	; EBX -> buffer
		pop		ecx					; restore reg
		mov		ax,[WORD color]
		mov		dh,[BYTE color+2]
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@V1

; Draw the line

		_ALIGN	4

@@Set1:	mov		[ebx],ax		; set pixel value in buffer
		mov		[ebx+2],dh
		add		ebx,esi				; increment to next position of interleave
		dec		ecx
		jnz		@@Set1				; loop down the line
		jmp		@@Exit				; We are done...

@@V1:   cmp		[writeMode],MGL_XOR_MODE
		jne		@@V2

		_ALIGN	4

@@Set2:	xor		[ebx],ax		; set pixel value in buffer
		xor		[ebx+2],dh
		add		ebx,esi				; increment to next position of interleave
		dec		ecx
		jnz		@@Set2				; loop down the line
		jmp		@@Exit				; We are done...

@@V2:	cmp		[writeMode],MGL_OR_MODE
		jne		@@Set4

		_ALIGN	4

@@Set3:	or		[ebx],ax		; set pixel value in buffer
		or		[ebx+2],dh
		add		ebx,esi				; increment to next position of interleave
		dec		ecx
		jnz		@@Set3				; loop down the line
		jmp		@@Exit				; We are done...

		_ALIGN	4

@@Set4:	and		[ebx],ax		; set pixel value in buffer
		and		[ebx+2],dh
		add		ebx,esi				; increment to next position of interleave
		dec		ecx
		jnz		@@Set4				; loop down the line
		jmp		@@Exit				; We are done...

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDY]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (x - x1)
		add		edi,eax				; EDI += 2 * dy * (x - x1)
		mov		eax,[varDX]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

@@LoDoneAdjust:
		mov		ax,[WORD color]
		mov     dh,[BYTE color+2]
		mov		dl,[writeMode]
		mov		esi,[EIncr]
		cmp		dl,MGL_REPLACE_MODE
		jne		@@SetLo2

		_ALIGN	4

@@SetLo1:
		mov		[ebx],ax		; Set pixel value in buffer
		mov		[ebx+2],dh		; Set pixel value in buffer
		inc		ebx
		inc		ebx
		inc		ebx					; Increment x coordinate

		or		edi,edi				; Test sign of d
		jg		@@LoPosDi1			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		dec		ecx
		jnz		@@SetLo1			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi1:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[vertInc]		; increment y
		dec		ecx
		jnz		@@SetLo1			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

		_ALIGN	4

@@SetLo2:
		cmp		dl,MGL_XOR_MODE
		jne		@@L1
		xor		[ebx],ax
		xor		[ebx+2],dh
		jmp		@@LOut
@@L1:   cmp		dl,MGL_OR_MODE
		jne		@@L2
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@LOut
@@L2:	and		[ebx],ax
		and		[ebx+2],dh

@@LOut:	inc		ebx
		inc		ebx
		inc		ebx					; Increment x coordinate

		or		edi,edi				; Test sign of d
		jns		@@LoPosDi2			; Jump if d >= 0

		add		edi,esi				; d := d + EIncr
		dec		ecx
		jnz		@@SetLo2			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi2:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[VertInc]		; increment y
		dec		ecx
		jnz		@@SetLo2			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						ECX = # pixels to draw
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDX]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (x - x1)
		sub		edi,eax				; EDI -= 2 * dx * (x - x1)
		mov		eax,[varDY]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

@@HiDoneAdjust:
		mov		ax,[WORD color]
		mov     dh,[BYTE color+2]
		mov		dl,[writeMode]
		mov		esi,[EIncr]
		cmp		dl,MGL_REPLACE_MODE
		jne		@@SetHi2

		_ALIGN	4

@@SetHi1:
		mov		[ebx],ax		; Set pixel value in buffer
		mov		[ebx+2],dh
		add		ebx,[VertInc]		; increment y

		or		edi,edi				; Test sign of d
		jg		@@HiPosDi1			; Jump if d > 0

		add		edi,esi				; d := d + EIncr
		dec		ecx
		jnz		@@SetHi1			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi1:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx
		inc		ebx
		inc		ebx					; Increment x
		dec		ecx
		jnz		@@SetHi1			; Loop for remaining pixels
		jmp		@@Exit

		_ALIGN	4

@@SetHi2:
		cmp		dl,MGL_XOR_MODE
		jne		@@H1
		xor		[ebx],ax
		xor		[ebx+2],dh
		jmp		@@Hout
@@H1:	cmp		dl,MGL_OR_MODE
		jne		@@H2
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@HOut
@@H2:   and		[ebx],ax
		and		[ebx+2],dh

@@HOut:	add		ebx,[VertInc]		; increment y

		or		edi,edi				; Test sign of d
		jns		@@HiPosDi2			; Jump if d >= 0

		add		edi,esi				; d := d + EIncr
		dec		ecx
		jnz		@@SetHi2			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi2:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx
		inc		ebx
		inc		ebx					; Increment x
		dec		ecx
		jnz		@@SetHi2			; Loop for remaining pixels

@@Exit:	leave_c
		ret

procend		_PACKED24_line

;----------------------------------------------------------------------------
; void _PACKED24_solidScanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
; Routine draws a scan line for 24 bit packed pixel deviecs. This
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
procstart	_PACKED24_solidScanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		LOCAL	count:UINT, writeMode:S_UCHAR,							\
				color:ULONG, bkColor:ULONG, penStyle:S_UCHAR = LocalSize

		enter_c	LocalSize
		cld							; String instructions go up

		mov		ecx,[x2]
		sub		ecx,[x1]			; ECX := X2 - X1 (# pixels to draw)
		jz		@@Exit				; Quit if xstart == xend
		jns		@@NoSwap			; no, don't swap them

		mov		eax,[x2]
		xchg	[x1],eax			; Swap the coordinates
		mov		[x2],eax
		neg		ecx

@@NoSwap:
		mov		[count],ecx

; Save variables out of device context into local variables

		mov     al,[BYTE GDC.a.penStyle]
		mov		[penStyle],al
		mov		al,[BYTE GDC.a.writeMode]
		mov		[writeMode],al
		mov		eax,[GDC.intColor]
		mov		[color],eax

@@ComputeAddress:
		mov		eax,[y]
		mov		ecx,[x1]
		call	PACKED24_pixelAddr	; EBX -> buffer

;----------------------------------------------------------------------------
; Draw a solid patterned line

		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		mov		dl,[writeMode]
		mov		ecx,[count]

@@LoopSolid:
		mov		[ebx],ax		; Set pixel value in buffer
		mov		[ebx+2],dh
		inc		ebx
		inc		ebx
		inc		ebx					; Increment to next pixel
		dec		ecx
		jnz		@@LoopSolid			; Loop across line
		jmp		@@Exit

@@Exit:	leave_c
		ret

procend		_PACKED24_solidScanLine

; void _PACKED24_ropSolidScanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
; Routine draws a scan line for 24 bit packed pixel deviecs. This
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
procstart	_PACKED24_ropSolidScanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		LOCAL	count:UINT, writeMode:S_UCHAR,							\
				color:ULONG, bkColor:ULONG, penStyle:S_UCHAR = LocalSize

		enter_c	LocalSize
		cld							; String instructions go up

		mov		ecx,[x2]
		sub		ecx,[x1]			; ECX := X2 - X1 (# pixels to draw)
		jz		@@Exit				; Quit if xstart == xend
		jns		@@NoSwap			; no, don't swap them

		mov		eax,[x2]
		xchg	[x1],eax			; Swap the coordinates
		mov		[x2],eax
		neg		ecx

@@NoSwap:
		mov		[count],ecx

; Save variables out of device context into local variables

		mov     al,[BYTE GDC.a.penStyle]
		mov		[penStyle],al
		mov		al,[BYTE GDC.a.writeMode]
		mov		[writeMode],al
		mov		eax,[GDC.intColor]
		mov		[color],eax

@@ComputeAddress:
		mov		eax,[y]
		mov		ecx,[x1]
		call	PACKED24_pixelAddr	; EBX -> buffer

;----------------------------------------------------------------------------
; Draw a solid patterned line

		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		mov		dl,[writeMode]
		mov		ecx,[count]

@@SolidROP:
		cmp		dl,MGL_XOR_MODE
		jne		@@S1
		xor		[ebx],ax
		xor		[ebx+2],dh
		jmp		@@SOut
@@S1:	cmp		dl,MGL_OR_MODE
		jne		@@S2
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@SOut
@@S2:	and		[ebx],ax
		and		[ebx+2],dh
@@SOut:	inc		ebx
		inc		ebx
		inc		ebx					; Increment to next pixel
		dec		ecx
		jnz		@@SolidROP			; Loop across line

@@Exit:	leave_c
		ret

procend		_PACKED24_ropSolidScanLine

;----------------------------------------------------------------------------
; void _PACKED24_pattScanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
; Routine draws a scan line for 24 bit packed pixel deviecs. This
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
procstart	_PACKED24_pattScanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		LOCAL	count:UINT, writeMode:S_UCHAR,							\
				color:ULONG, bkColor:ULONG, penStyle:S_UCHAR = LocalSize

		enter_c	LocalSize
		cld							; String instructions go up

		mov		ecx,[x2]
		sub		ecx,[x1]			; ECX := X2 - X1 (# pixels to draw)
		jz		@@Exit				; Quit if xstart == xend
		jns		@@NoSwap			; no, don't swap them

		mov		eax,[x2]
		xchg	[x1],eax			; Swap the coordinates
		mov		[x2],eax
		neg		ecx

@@NoSwap:
		mov		[count],ecx

; Save variables out of device context into local variables

		mov     al,[BYTE GDC.a.penStyle]
		mov		[penStyle],al
		mov		al,[BYTE GDC.a.writeMode]
		mov		[writeMode],al
		mov		eax,[GDC.intColor]
		mov		[color],eax

@@ComputeAddress:
		mov		eax,[y]
		mov		ecx,[x1]
		call	PACKED24_pixelAddr	; EBX -> buffer

@@Patterned:
		mov		eax,[GDC.intBackColor]
		mov		[bkColor],eax

; Get the pattern byte for this scan line from pattern array

		mov		esi,[y]
		and		esi,7				; Mask to 0-7
		mov		dl,[esi + GDC.a.penPat]

		mov		cl,[BYTE x1]
		and		cl,7				; Mask to 0-7
		rol		dl,cl				; Rotate pattern into place

		mov		ecx,[count]			; CX := bytes to set
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

		_ALIGN	4

@@TransLoop:
		rol		dl,1				; Rotate pattern mask
		jnc		@@SkipTrans			; Draw pixel if pattern bit was 1

		mov		[ebx],ax		; Set pixel value in buffer
		mov		[ebx+2],dh

@@SkipTrans:
		inc		ebx
		inc		ebx
		inc		ebx					; Increment to next pixel
		dec		ecx
		jnz		@@TransLoop			; Loop across line
		jmp		@@Exit

		_ALIGN	4

@@TransLoopROP:
		rol		dl,1				; Rotate pattern mask
		jnc		@@SkipTransROP		; Draw pixel if pattern bit was 1

		cmp		[writeMode],MGL_XOR_MODE
		jne		@@T1
		xor		[ebx],ax			; Set pixel value in buffer
		xor		[ebx+2],dh
		jmp		@@SkipTransROP
@@T1:	cmp		[writeMode],MGL_OR_MODE
		jne		@@T2
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@SkipTransROP
@@T2:	and		[ebx],ax
		and		[ebx+2],dh

@@SkipTransROP:
		inc		ebx
		inc		ebx
		inc		ebx					; Increment to next pixel
		dec		ecx
		jnz		@@TransLoopROP		; Loop across line
		jmp		@@Exit

;----------------------------------------------------------------------------
; Draw an opaque patterned line

@@DoOpaque:
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@OpaqueLoopROP

		_ALIGN	4

@@OpaqueLoop:
		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		rol		dl,1				; Rotate pattern mask
		jc		@@SetPixelOpaque	; Draw pixel if pattern bit was 1

		mov		ax,[WORD bkColor]	; AX := pixel color
		mov		dh,[BYTE bkColor+2]	; DH := top byte of pixel color

@@SetPixelOpaque:
		mov		[ebx],ax		; Set pixel value in buffer
		mov		[ebx+2],dh
		inc		ebx
		inc		ebx
		inc		ebx					; Increment to next pixel
		dec		ecx
		jnz		@@OpaqueLoop		; Loop across line
		jmp		@@Exit

		_ALIGN	4

@@OpaqueLoopROP:
		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		rol		dl,1				; Rotate pattern mask
		jc		@@SetPixelOpaqueROP	; Draw pixel if pattern bit was 1

		mov		ax,[WORD bkColor]	; AX := pixel color
		mov		dh,[BYTE bkColor+2]	; DH := top byte of pixel color

@@SetPixelOpaqueROP:
		cmp		[writeMode],MGL_XOR_MODE
		jne		@@O1
		xor		[ebx],ax
		xor		[ebx+2],dh
		jmp		@@OOut
@@O1:	cmp		[writeMode],MGL_OR_MODE
		jne		@@O2
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@OOut
@@O2:	and		[ebx],ax
		and		[ebx+2],dh

@@OOut:	inc		ebx					; Increment to next pixel
		inc		ebx
		inc		ebx
		dec		ecx
		jnz		@@OpaqueLoopROP		; Loop across line
		jmp		@@Exit

;----------------------------------------------------------------------------
; Draw a pixmap pattern scanline. For now we simply call the high level
; scanline emulation routine which will call putPixel to handle this. Not
; very fast but it works.

@@DoPixmap:
		push	[x2]
		push	[x1]
		push	[y]
		call	___EMU__scanLine
		add		esp,12

@@Exit:	leave_c
		ret

procend		_PACKED24_pattScanLine

;----------------------------------------------------------------------------
; void _PACKED24_solidFillRect(int x1,int y1,int x2,int y2)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle for 24 bit packed pixel devices
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
procstart	_PACKED24_solidFillRect

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT

		LOCAL	rows:UINT, cols:UINT = LocalSize

		enter_c	LocalSize
		cld

;---------------------------------------------------------------------------
; We are filling with a solid pattern, so do it as quickly as possible

		mov		esi,[x2]
		sub		esi,[x1]			; ESI := X2 - X1 (# pixels to draw)
		mov		[cols],esi			; Save coloumns for later
		mov		edx,[y2]
		sub		edx,[y1]			; EDX := Y2 - Y1 (# rows to draw)
		mov     [rows],edx			; Save rows for later
		mov		edi,esi
		shl		esi,1
		add		esi,edi				; ESI := bytes per line
		mov		edi,[GDC.mi.bytesPerLine]
		sub		edi,esi				; EDI := buffer increment
		mov		eax,[GDC.intColor]
		push	eax

		mov		eax,[y1]
		mov		ecx,[x1]
		call	PACKED24_pixelAddr	; EBX -> buffer
		pop		eax					; EAX := pixel color
		mov		ecx,eax
		shr		ecx,8
		mov		dh,ch				; DH:AX := pixel color

		_ALIGN	4

@@RowLoop:
		mov		ecx,[cols]			; Restore number of pixels to fill

		_ALIGN	4

@@NextPixel:
		mov		[ebx],ax		; Set pixel value in buffer
		mov		[ebx+2],dh
		inc		ebx
		inc		ebx
		inc		ebx					; Increment to next pixel
@@SEnd:	dec		ecx
		jnz		@@NextPixel			; Loop across line

@@NextRow:
		add		ebx,edi				; Increment to next row
		dec		[rows]
		jnz		@@RowLoop			; Loop for all rows

@@Exit:	leave_c
		ret

procend		_PACKED24_solidFillRect

;----------------------------------------------------------------------------
; void _PACKED24_putMonoImage(MGLDC *dc,int x,int y,int width,int height,
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
procstart	_PACKED24_putMonoImage

		ARG		dc:DPTR, x:UINT, y:UINT, byteWidth:UINT, height:UINT,		\
				image:DPTR

		LOCAL	bytesPerLine:UINT, cleft:UINT, ctop:UINT, cright:UINT, 		\
				cbottom:UINT, rowWidth:UINT, rowCount:UINT, rowAdjust:UINT,	\
				count:UINT, color:ULONG, lmask:S_UCHAR, rmask:S_UCHAR,		\
				writeMode:S_UCHAR = LocalSize

		enter_c	LocalSize
		use_ds
		cld

; Copy variables from device context onto local stack space

		mov 	ebx,[dc]
		mov		eax,[MGLDC.mi.bytesPerLine]
		mov     [bytesPerLine],eax
		mov     eax,[MGLDC.intClipRect.left]
		mov		[cleft],eax
		mov     eax,[MGLDC.intClipRect.top]
		mov		[ctop],eax
		mov     eax,[MGLDC.intClipRect.right]
		mov		[cright],eax
		mov     eax,[MGLDC.intClipRect.bottom]
		mov		[cbottom],eax
		mov		eax,[MGLDC.intColor]
		mov		[color],eax
		mov		al,[BYTE MGLDC.a.writeMode]
		mov		[writeMode],al

; Check to see if the image needs clipping. If it does, then use the
; clipping routine to draw it. Otherwise use the faster non-clipping routine.

		mov		eax,[y]				; AX := y
		mov		ebx,[x]				; BX := x
		mov		ecx,[byteWidth]
		mov		[rowWidth],ecx
		shl		ecx,3				; CX := width of image in pixels

		cmp		eax,[ctop]
		jl		@@ClipImage
		mov		edx,eax
		add		edx,[height]
		cmp		edx,[cbottom]
		jge		@@ClipImage
		cmp		ebx,[cleft]
		jl		@@ClipImage
		mov		edx,ebx
		add		edx,ecx
		cmp		edx,[cright]
		jge		@@ClipImage
		cmp		[writeMode],MGL_REPLACE_MODE
		jne		@@ClipImage
		jmp		@@DrawNonClipped

;----------------------------------------------------------------------------
; Routine for clipped monochrome images.
;
;		EAX	- y coordinate
;		EBX - x coordinate
;		ECX - width of image in pixels

@@ClipImage:
		mov		esi,[UINT image]	; ESI -> offset of image data
		mov		edx,[height]		; DX := height of image in scanlines
		dec		edx

; Clip the y coordinates of the image.

		cmp		eax,[ctop]			; Less than top coordinate?
		jge		@@TopOK				; No, top is ok.

; Clip the top of the image. If the image is entirely off the clip
; area, clip it entirely.

		add		eax,edx				; Adjust to bottom of image
		sub		eax,[ctop]			; Clip bottom edge against top
		js		@@Exit				; Entire image is clipped

		sub		eax,edx				; Adjust to top of image again
		add		edx,eax				; Adjust increment to bottom of image
		neg		eax
		mul		[BYTE byteWidth]
		cwde
		add		esi,eax				; Increment starting row in image
		mov		eax,[ctop]			; Start at top coordinate of clip area

@@TopOK:
		mov		ebx,eax				; BX := top coordinate
		add		ebx,edx				; Adjust to bottom of image
		cmp		ebx,[cbottom]		; Greater than bottom coordinate?
		jl		@@BottomOK			; No, bottom is ok.

; Clip the bottom of the image. If the image is entirely off the
; clip area, clip it entirely.

		mov		ebx,eax				; Get top of image
		sub		ebx,[cbottom]		; Clip top edge against bottom
		jge		@@Exit				; Entire image is clipped

		add		ebx,edx
		sub		edx,ebx				; Adjust number of rows to process
		dec		edx

@@BottomOK:
		inc		edx					; DX := number of rows to process
		mov		[rowCount],edx		; Save row count for later
		mov		[y],eax				; Save y coordinate
		mov		al,0FFh
		mov		[lmask],al			; Default left clipping mask
		mov		[rmask],al			; Default right clipping mask

		mov		edx,ecx				; DX := width of image in pixels
		dec		edx
		mov		ebx,[x]				; BX := x
		cmp		ebx,[cleft]			; Less than left coordinate?
		jge		@@LeftOK			; No, left is ok.

; Clip the left of the image. If the image is entirely off the clip
; area, clip it entirely.

		add		ebx,edx				; Adjust to right edge of character
		cmp		ebx,[cleft]			; Clip right edge against left edge
		js		@@Exit				; Entire image is clipped

; Build a clipping mask for the left byte of the image

		sub		ebx,edx				; Adjust back to left edge again
		mov		ecx,[cleft]
		sub		ecx,ebx				; CX := cleft - left
		mov		eax,ecx
		shr		eax,3				; AX := byte offset into row
		sub		[rowWidth],eax		; Adjust count of bytes to process
		add		esi,eax				; Adjust offset to first byte
		shl		eax,3				; AX := pixel offset into row
		add		[x],eax				; Adjust first pixel position
		mov		al,0FFh
		and		cl,7				; CL := bit offset into byte
		shr		al,cl				; AL := clipping mask in proper position
		mov		[lmask],al			; Save left clipping mask

@@LeftOK:
		mov		eax,ebx
		add		eax,edx				; Adjust to right of image
		cmp		eax,[cright]		; Greater than right edge?
		jl		@@RightOK			; No, right is ok.

; Clip the right of the image. If the image is entirely off the clip
; area, clip it entirely.

		mov		eax,ebx				; Get left of image
		cmp		eax,[cright]		; Clip left edge against right edge
		jge		@@Exit				; Entire image is clipped

; Build a clipping mask for the right byte of the image

		add		ebx,edx				; BX := right of character
		mov		ecx,ebx
		sub		ecx,[cright]		; CX := right - cright
		inc		ecx
		mov		eax,ecx
		shr		eax,3				; AX := byte offset from end of row
		sub		[rowWidth],eax		; Adjust count of bytes to process
		mov		al,0FFh
		and		cl,7				; CL := bit offset into byte
		shl		al,cl				; AL := clipping mask for proper position
		and		[rmask],al			; Save right clipping mask

; Draw the image

@@RightOK:
		mov		[UINT image],esi	; Save offset into image buffer
		mov		eax,[y]
		mov		ecx,[x]
		mov 	ebx,[dc]
		call	PACKED24_pixelAddrEBX; EBX -> buffer
		mov 	esi,[image]			; DS:ESI -> image buffer

		mov		ecx,[rowCount]		; CX := number of scanlines to process
		mov		eax,[rowWidth]
		sub		eax,[byteWidth]
		neg		eax
		mov		[rowAdjust],eax		; Save adjustment between rows
		mov		dx,[WORD color]
		mov		ah,[BYTE color+2]

		cmp		[BYTE rowWidth],1
		jne		@@NextRowClipped

		mov		al,[rmask]			; Combine left and right masks if clipped
		and		[lmask],al			;  image is only 1 byte wide

@@NextRowClipped:
		push	ebx
		push	[rowWidth]

; Load the first byte and mask with left clipping mask

		lodsb
		and		al,[lmask]
		dec		[rowWidth]			; Decrement count of bytes to do
		jmp		@@PlotByte			; Go plot it

		_ALIGN	4

@@NextByteClipped:
		lodsb						; AL := pattern for next 8 pixels

@@PlotByte:
		cmp		[writemode],MGL_REPLACE_MODE
		jne		@@PlotByteROP
		shl		al,1
		jnc		@@SKC1
		mov		[ebx],dx
		mov		[ebx+2],ah
@@SKC1:	shl		al,1
		jnc		@@SKC2
		mov		[ebx+3],dx
		mov		[ebx+5],ah
@@SKC2:	shl		al,1
		jnc		@@SKC3
		mov		[ebx+6],dx
		mov		[ebx+8],ah
@@SKC3:	shl		al,1
		jnc		@@SKC4
		mov		[ebx+9],dx
		mov		[ebx+11],ah
@@SKC4:	shl		al,1
		jnc		@@SKC5
		mov		[ebx+12],dx
		mov		[ebx+14],ah
@@SKC5:	shl		al,1
		jnc		@@SKC6
		mov		[ebx+15],dx
		mov		[ebx+17],ah
@@SKC6:	shl		al,1
		jnc		@@SKC7
		mov		[ebx+18],dx
		mov		[ebx+20],ah
@@SKC7:	shl		al,1
		jnc		@@SKC8
		mov		[ebx+21],dx
		mov		[ebx+23],ah
@@SKC8:	add		ebx,24

@@DonePlot:
		dec		[rowWidth]
		js		@@DoneRow			; Negative, so all done
		jnz		@@NextByteClipped	; Not zero, so continue

; Load the last byte for the row and mask with right clipping mask

		lodsb
		and		al,[rmask]
		dec		[rowWidth]			; Decrement count of bytes to do
		jmp		@@PlotByte			; Go plot it

		_ALIGN	4

@@DoneRow:
		pop		[rowWidth]
		pop		ebx
		add		ebx,[bytesPerLine]	; increment to next line in video buffer
		add		esi,[rowAdjust]		; increment to next line in image
		dec		ecx
		jnz		@@NextRowClipped
		jmp		@@Exit

		_ALIGN	4

@@PlotByteROP:
		push	ecx
		mov		ecx,8

		_ALIGN	4

@@NextPixelROP:
		shl		al,1
		jnc		@@SkipROP
		cmp		[writemode],MGL_XOR_MODE
		jne		@@R1
		xor		[ebx],dx
		xor		[ebx+2],ah
		jmp		@@SkipROP
@@R1:	cmp		[writemode],MGL_OR_MODE
		jne		@@R2
		or		[ebx],dx
		or		[ebx+2],ah
		jmp		@@SkipRop
@@R2:	and		[ebx],dx
		and		[ebx+2],ah
@@SkipRop:
		inc     ebx
		inc     ebx
		inc     ebx
		dec		ecx
		jnz		@@NextPixelROP

		pop		ecx
		jmp		@@DonePlot

;----------------------------------------------------------------------------
; Routine for non-clipped monochrome images. We can simply blast out the
; pixels without worrying about masking off the last few pixels in each
; line.
;
;		EAX	- y coordinate
;		EBX - x coordinate
;		ECX - width of image in pixels

@@DrawNonClipped:
		mov		ecx,ebx
		mov 	ebx,[dc]
		call	PACKED24_pixelAddrEBX; EBX -> buffer
		mov 	esi,[image]			; DS:ESI -> image bit map
		mov		ecx,[height]		; CX := height of image in scanlines

		mov		dx,[WORD color]
		mov		ah,[BYTE color+2]

		_ALIGN	4

@@NextRow:
		push	ebx
		push	[rowWidth]

; Plot all of the pixels in the current byte, using a fast unrolled loop

		_ALIGN	4

@@NextByte:
		lodsb						; AL := pattern for next 8 pixels
		shl		al,1
		jnc		@@SK1
		mov		[ebx],dx
		mov		[ebx+2],ah
@@SK1:	shl		al,1
		jnc		@@SK2
		mov		[ebx+3],dx
		mov		[ebx+5],ah
@@SK2:	shl		al,1
		jnc		@@SK3
		mov		[ebx+6],dx
		mov		[ebx+8],ah
@@SK3:	shl		al,1
		jnc		@@SK4
		mov		[ebx+9],dx
		mov		[ebx+11],ah
@@SK4:	shl		al,1
		jnc		@@SK5
		mov		[ebx+12],dx
		mov		[ebx+14],ah
@@SK5:	shl		al,1
		jnc		@@SK6
		mov		[ebx+15],dx
		mov		[ebx+17],ah
@@SK6:	shl		al,1
		jnc		@@SK7
		mov		[ebx+18],dx
		mov		[ebx+20],ah
@@SK7:	shl		al,1
		jnc		@@SK8
		mov		[ebx+21],dx
		mov		[ebx+23],ah
@@SK8:	add		ebx,24
		dec		[rowWidth]
		jnz		@@NextByte

		pop		[rowWidth]
		pop		ebx
		add		ebx,[bytesPerLine]	; increment to next line in video buffer
		dec		ecx
		jnz		@@NextRow

@@Exit:	unuse_ds
		leave_c
		ret

procend		_PACKED24_putMonoImage

;----------------------------------------------------------------------------
; void MGLAPI _PACKED24_putImage(MGLDC *dc,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,void *surface,
;	int bytesPerLine)
;----------------------------------------------------------------------------
; Blt an image from a memory buffer onto the device.
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
procstart	_PACKED24_putImage

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT, op:UINT, surface:DPTR,		\
				bytesPerLine:UINT

		LOCAL	bufAdjust:UINT, flags:UINT = LocalSize

		enter_c	LocalSize
		use_ds
		cld

; Determine special cases

		cmp		[op],MGL_REPLACE_MODE
		jne     @@ROP

		mov 	ebx,[dc]
		mov		eax,[MGLDC.flags]
		mov		[flags],eax
		mov		eax,[top]
		mul		[bytesPerLine]
		mov		ecx,[left]
		lea		ecx,[ecx+ecx*2]
		add		eax,ecx				; EAX := offset into source buffer
		mov		edi,eax
		mov		edx,[right]
		sub		edx,[left]			; EDX := number of bytes to move for 8 bpp
		mov		eax,3				; 
		mul		edx					; EDX = EDX * 3 (for 24 bpp)
		mov		edx,eax				; EDX := number of bytes to move for 24 bpp
		sub		[bytesPerLine],edx	; Adjust source buffer increment
		mov		eax,[MGLDC.mi.bytesPerLine]
		sub     eax,edx
		mov		[bufAdjust],eax		; Save destination buffer increment
		mov		eax,[dstTop]
		mov		ecx,[dstLeft]
		call	PACKED24_pixelAddrEBX; EBX -> device surface
		mov 	esi,[surface]
		add		esi,edi				; ESI -> system memory buffer
		mov		edi,ebx				; EDI -> device surface
		mov     ebx,[bottom]
		sub		ebx,[top]			; EBX := number of rows

		test	[left],3
		jne		@@NextRow
		test	[right],3
		jne		@@NextRow
		test	[flags],MGL_VIRTUAL_ACCESS
		jz		@@StartDWord
		test	[dstLeft],3
		jne		@@NextRow

; Partial Blt, where the left and right coordinates of the source are dword
; aligned and the left coordinate of the destination is dword aligned (for
; virtualised buffers as this will cause an infinite page fault loop).

@@StartDWord:
		shr		edx,2				; EDX := number of dwords to move

		_ALIGN	4

@@NextRowDWord:
		mov		ecx,edx				; CX := dwords to move
	rep	movsd
		add     esi,[bytesPerLine]	; Move to next line
		add		edi,[bufAdjust]		; Move to next line
		dec		ebx
		jnz		@@NextRowDWord
		jmp		@@Exit

; Partial Blt, which we extend to DWORD boundaries for speed

		_ALIGN	4

@@NextRow:
		mov		ecx,edx				; CX := dwords to move
		test	[flags],MGL_VIRTUAL_ACCESS
		jnz		@@ForceAlignment

@@Start:
		push	ecx
		shr		ecx,2
	rep	movsd						; Move pixels in row 32 bits at a time
		pop		ecx
		and		ecx,3
	rep	movsb						; Move the last bytes in row
		add     esi,[bytesPerLine]	; Move to next line
		add		edi,[bufAdjust]		; Move to next line
		dec		ebx
		jnz		@@NextRow

@@Exit:	unuse_ds
		leave_c
		ret

; Force DWORD alignment for transfers in a virtualised buffer

@@ForceAlignment:
		test	edi,3
		jz		@@Start
		movsb
		dec		ecx
		jnz		@@ForceAlignment
		jmp		@@Exit

@@ROP:	unuse_ds
		leave_c
		jmp		___EMU__putImage

procend		_PACKED24_putImage

;----------------------------------------------------------------------------
; void _PACKED24_getDivot(MGLDC *dc,int left,int top,int right,int bottom,
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
;	int		StartOffset;			- Starting offset in video ram
;	} divotHeader;
;
;----------------------------------------------------------------------------
procstart	_PACKED24_getDivot

		ARG		dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				divot:DPTR

		LOCAL	bytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		cld

; Compute position of bit block in video buffer

		mov 	ebx,[dc]
		mov		edx,[MGLDC.mi.bytesPerLine]
		mov		[bytesPerLine],edx	; Save bytes per line value on stack

		mov     eax,[top]
		mov		ecx,[left]
		and		cl,0FCh				; Mask to even dword boundary
		mov		[left],ecx
		call	PACKED24_pixelAddrEBX; EBX -> display surface
		mov		esi,ebx				; DS:ESI -> display surface
		mov 	edi,[divot]			; EDI -> buffer in system RAM

; Build the bit block header

		mov		eax,[right]
		add		eax,3
		and		al,0FCh				; Mask to even dword boundary
		sub		eax,[left]			; EAX := # pixel columns
		mov		edx,eax
		shl		eax,1
		add		edx,eax				; EDX := bytesperline
		mov		eax,edx
		stosw						; bytes per pixel row

		mov		eax,[bottom]
		sub		eax,[top]
		mov		ebx,eax				; EBX := # pixel rows
		stosw						; number of rows

		mov		eax,esi
		STOSINT						; Starting offset in buffer

		mov		eax,[bytesPerLine]
		sub		eax,edx				; EAX := buffer adjust for row

@@NextRow:
		mov		ecx,edx				; CX := # pixels in row
		shr		ecx,2
	rep	movsd						; Move pixels in row 32 bits a time
		add		esi,eax				; Increment to next row in video buffer
		dec		ebx
		jnz		@@NextRow			; Loop for all rows in image

@@Exit:	leave_c
		ret

procend		_PACKED24_getDivot

;----------------------------------------------------------------------------
; void _PACKED24_putDivot(MGLDC *dc,void *divot)
;----------------------------------------------------------------------------
; Routine to copy a block of video information from system memory to the
; currently active display page. The block is a special dword aligned piece
; of memory and is returned to the same position it was saved from.
;
; Entry:		dc		- Device context to dump divot to
;				divot	- Pointer to place to store image data
;
;----------------------------------------------------------------------------
procstart	_PACKED24_putDivot

		ARG		dc:DPTR, divot:DPTR

		LOCAL	BytesPerLine:UINT = LocalSize

		enter_c	LocalSize
		use_ds
		cld

		mov 	ebx,[dc]
		mov		edx,[MGLDC.mi.bytesPerLine]
		mov		[bytesPerLine],edx	; Save bytes per line value on stack
		mov 	esi,[divot]			; DS:ESI -> buffer in system RAM

; Extract position info from image header

		xor		eax,eax
		lodsw
		mov		edx,eax				; DX := # pixel columns
		lodsw
		mov     ebx,eax             ; BX := # pixel rows
		LODSINT
		mov		edi,eax				; EDI := offset in video ram
		mov		eax,[bytesPerLine]
		sub		eax,edx				; EAX := buffer adjustment

@@NextRow:
		mov		ecx,edx				; CX := # pixels in row
		shr		ecx,2
	rep	movsd						; Move all pixels in row
		add		edi,eax				; Increment to next row in video buffer
		dec		ebx
		jnz		@@NextRow			; Loop for all rows in image

		unuse_ds
		leave_c
		ret

procend		_PACKED24_putDivot

;----------------------------------------------------------------------------
; void PACKED24_stretchScan2x(void *d,void *s,int count)
;----------------------------------------------------------------------------
; Stretches a single scanline as fast as possible given a set of pixel
; replication counts.
;
;	Entry:  s			- Pointer to source scanline
;			d			- Pointer to dest scanline
;			count		- Number of pixels in source scanline
;----------------------------------------------------------------------------
procstart	_PACKED24_stretchScan2x

		ARG		d:DPTR, s:DPTR, count:UINT

		enter_c	0
		cld

		mov		esi,[s]				; ESI -> source scanline
		mov		edi,[d]				; EDI -> dest scanline
		mov		ebx,[count]			; EBX := number of pixels

		_ALIGN	4

@@NextPixel:
		mov		ax,[esi]			; Get source pixel
		mov		dl,[esi+2]
		mov		[edi],ax
		mov		[edi+2],dl
		mov		[edi+3],ax
		mov		[edi+5],dl
		add		esi,3
		add		edi,6
		dec		ebx
		jnz		@@NextPixel

@@Done:	leave_c_nolocal
		ret

procend		_PACKED24_stretchScan2x

;----------------------------------------------------------------------------
; void PACKED24_stretchScanBGR2x(void *d,void *s,int count)
;----------------------------------------------------------------------------
; Stretches a single scanline as fast as possible given a set of pixel
; replication counts.
;
;	Entry:  s			- Pointer to source scanline
;			d			- Pointer to dest scanline
;			count		- Number of pixels in source scanline
;----------------------------------------------------------------------------
procstart	_PACKED24_stretchScanBGR2x

		ARG		d:DPTR, s:DPTR, count:UINT

		enter_c	0
		cld

		mov		esi,[s]				; ESI -> source scanline
		mov		edi,[d]				; EDI -> dest scanline
		mov		ebx,[count]			; EBX := number of pixels

		_ALIGN	4

@@NextPixel:
		mov		ax,[esi]			; Get source pixel
		mov		dl,[esi+2]
		mov		[edi],dl
		mov		[edi+1],ah
		mov		[edi+2],al
		mov		[edi+3],dl
		mov		[edi+4],ah
		mov		[edi+5],al
		add		esi,3
		add		edi,6
		dec		ebx
		jnz		@@NextPixel

@@Done:	leave_c_nolocal
		ret

procend		_PACKED24_stretchScanBGR2x

;----------------------------------------------------------------------------
; void PACKED24_stretchScan(void *d,void *s,int count,int *repCounts)
;----------------------------------------------------------------------------
; Stretches a single scanline as fast as possible given a set of pixel
; replication counts.
;
;	Entry:  s			- Pointer to source scanline
;			d			- Pointer to dest scanline
;			count		- Number of pixels in source scanline
;           repCounts	- Replication count buffer
;----------------------------------------------------------------------------
procstart	_PACKED24_stretchScan

		ARG		d:DPTR, s:DPTR, count:UINT, repCounts:DPTR

		enter_c	0
		cld

		mov		esi,[s]				; ESI -> source scanline
		mov		edi,[d]				; EDI -> dest scanline
		mov		ebx,[repCounts]		; EBX -> replication count buffer

		_ALIGN	4

@@NextPixel:
		mov		ecx,[ebx]			; Replication count for this pixel
		test	ecx,ecx
		jz		@@Skip
		mov		ax,[esi]			; Get source pixel
		mov		dl,[esi+2]
@@Rep:	add		edi,3
		dec		ecx
		mov		[edi-3],ax
		mov		[edi-1],dl
		jnz		@@Rep
@@Skip:	add		esi,3
		add		ebx,4
		dec		[count]
		jnz		@@NextPixel

@@Done:	leave_c_nolocal
		ret

procend		_PACKED24_stretchScan

;----------------------------------------------------------------------------
; void PACKED24_stretchScanBGR(void *d,void *s,int count,int *repCounts)
;----------------------------------------------------------------------------
; Stretches a single scanline as fast as possible given a set of pixel
; replication counts.
;
;	Entry:  s			- Pointer to source scanline
;			d			- Pointer to dest scanline
;			count		- Number of pixels in source scanline
;           repCounts	- Replication count buffer
;----------------------------------------------------------------------------
procstart	_PACKED24_stretchScanBGR

		ARG		d:DPTR, s:DPTR, count:UINT, repCounts:DPTR

		enter_c	0
		cld

		mov		esi,[s]				; ESI -> source scanline
		mov		edi,[d]				; EDI -> dest scanline
		mov		ebx,[repCounts]		; EBX -> replication count buffer

		_ALIGN	4

@@NextPixel:
		mov		ecx,[ebx]			; Replication count for this pixel
		test	ecx,ecx
		jz		@@Skip
		mov		ax,[esi]			; Get source pixel
		mov		dl,[esi+2]
@@Rep:	add		edi,3
		dec		ecx
		mov		[edi-3],dl
		mov		[edi-2],ah
		mov		[edi-1],al
		jnz		@@Rep
@@Skip:	add		esi,3
		add		ebx,4
		dec		[count]
		jnz		@@NextPixel

@@Done:	leave_c_nolocal
		ret

procend		_PACKED24_stretchScanBGR

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; void PACKED24_solidTrap(void);
;----------------------------------------------------------------------------
; Routine draws a flat shaded trapezoid. This routine only works for
; trapezoids in REPLACE mode. The high level C code versions will be called
; in other write modes etc.
;----------------------------------------------------------------------------
procstart	_PACKED24_solidTrap

		push	ebx
		push	esi
		push	edi
		cmp		[GDC.tr.count],0
		je		@@Exit				; Bail out for zero height traps
		cld							; String instructions go up

		mov		eax,[GDC.intColor]
		shr		eax,16
		mov		[gcolor],eax
		mov		edi,[GDC.surface]	; EDI -> start of device surface
		mov		ebx,[GDC.tr.count]
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		mov		[gcount],ebx
		add		[GDC.tr.y],ebx		; Increment coordinate for next trap
		imul	[GDC.mi.bytesPerLine]; EAX := y * BytesPerLine
		add		edi,eax				; EDI -> framebuffer start address
		mov		edx,[GDC.tr.x1]		; EDX := X1
		mov		esi,[GDC.tr.x2]		; ESI := X2

; Check that xstart < xend, swapping if need be.

		_ALIGN	4

@@NextScan:
		mov		ebx,edx				; EBX := X1
		mov		eax,esi				; EAX := X2
		add		ebx,8000h			; Round
		add		eax,8000h			; Round
		shr		ebx,16				; EBX := xstart for scanline
		shr		eax,16				; EAX := xend for scanlines
		cmp		ebx,eax				; is xstart > xend?
		jz		@@SkipLine			; ignore if xstart == xend
		jl		@@NoSwap			; no, don't swap them
		xchg	eax,ebx				; swap ix1,ix2

@@NoSwap:
		push	edx
		push	edi					; Preserve EDI
		mov		ecx,eax				; ECX := xend
		sub		ecx,ebx				; ECX := xend - xstart (# pixels to draw)
		add		edi,ebx
		add		edi,ebx
		add		edi,ebx				; EDI -> first pixel to fill
		mov		eax,[GDC.intColor]
		mov		edx,[gcolor]		; DL:AX := pixel color

		_ALIGN	4

@@NextPixel:
		mov		[edi],ax			; Set pixel value in buffer
		mov		[edi+2],dl
		inc		edi
		inc		edi
		inc		edi					; Increment to next pixel
		dec		ecx
		jnz		@@NextPixel

		pop		edi					; Restore EDI
		pop		edx

		_ALIGN	4

@@SkipLine:
		add		edi,[GDC.mi.bytesPerLine]; Increment address to next scanline
		add		edx,[GDC.tr.slope1]	; x1 += slope1
		add		esi,[GDC.tr.slope2]	; x2 += slope2
		dec		[gcount]
		jnz		@@NextScan			; Loop for all scanlines

; Move the interpolated X1 and X2 values back from locals

		mov		[GDC.tr.x1],edx		; Save interpolated x1
		mov		[GDC.tr.x2],esi		; Save interpolated x2

@@Exit:	pop		edi
		pop		esi
		pop		ebx
		ret

procend		_PACKED24_solidTrap

;----------------------------------------------------------------------------
; void _PACKED24_solidDrawScanList(int y,int length,short *scans);
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
procstart	_PACKED24_solidDrawScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		LOCAL	bytesPerLine:UINT, color:ULONG = LocalSize

		enter_c	LocalSize
		use_ds
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
		mov		eax,[GDC.mi.bytesPerLine]
		mov		[bytesPerLine],eax
		mov		eax,[GDC.intColor]
		mov		[color],eax			; Save color value for later
		mov		eax,[y]				; AX := top coordinate
		mov		ecx,[len]			; CX := number of scanlines to fill
		imul	[bytesPerLine]		; AX := y * BytesPerLine
		mov		edi,eax

; Get the address of the scanline array in DS:ESI

		mov 	esi,[scans]			; DS:ESI -> scanline array data
		add		edi,[GDC.surface]

		_ALIGN	4

@@FillLoop:

; Check that xstart < xend, swapping if need be.

		lodsw
		clrhi	ax
		mov		ebx,eax				; BX := xstart for scanline
		lodsw						; AX := xend for scanline
		clrhi	ax
		cmp		ebx,eax				; is xstart > xend?
		jl		@@NoSwap			; no, don't swap them
		jg		@@Swap				; yes, swap them
		jmp		@@SkipLine			; ignore if xstart == xend

@@Swap:
		xchg	eax,ebx

@@NoSwap:
		push	ecx					; Preserve CX
		push	edi					; Preserve EDI
		push	esi

		mov		ecx,eax				; ECX := xend
		sub		ecx,ebx				; ECX := xend - xstart (# pixels to draw)
		mov		esi,ebx
		shl		ebx,1
		add		ebx,esi
		add		edi,ebx				; EDI -> first pixel to fill2

		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color

		_ALIGN	4

@@LoopSolid:
		mov		[edi],ax		; Set pixel value in buffer
		mov		[edi+2],dh
		inc		edi
		inc		edi
		inc		edi					; Increment to next pixel
		dec		ecx
		jnz		@@LoopSolid			; Loop across line

		pop		esi
		pop		edi					; Restore EDI
		pop		ecx					; Restore CX

@@SkipLine:
		add		edi,[bytesPerLine]	; Increment address to next scanline
		dec		ecx
		jnz		@@FillLoop			; Loop for all scanlines
		jmp		@@Exit

;----------------------------------------------------------------------------
; Slower scanline filling routine. This routine simply calls scanline to
; draw each scanline.

@@SlowFill:
		mov		edi,[y]				; EDI := top coordinate

; Get the address of the scanline array in DS:ESI

		mov 	esi,[scans]			; DS:ESI -> scanline array data

		_ALIGN	4

@@FillLoop2:

; Check that xstart < xend, swapping if need be.

		lodsw
		clrhi	ax
		mov		edx,eax				; DX := xstart for scanline
		lodsw						; AX := xend for scanline
		clrhi	ax
		cmp		edx,eax				; is xstart > xend?
		jl		@@NoSwap2			; no, don't swap them
		jg		@@Swap2				; yes, swap them
		jmp		@@SkipLine2			; ignore if xstart == xend

@@Swap2:
		xchg	eax,edx

@@NoSwap2:
		push	eax					; Push xend coordinate
		push	edx					; Push xstart coordinate
		push	edi					; Push y coordinate
		call	[GDC.r.cur.scanLine]
		add		esp,12

@@SkipLine2:
		inc		edi
		dec		[len]
		jnz		@@FillLoop2			; Loop for all scanlines

@@Exit:	unuse_ds
		leave_c
		ret

procend		_PACKED24_solidDrawScanList

;----------------------------------------------------------------------------
; void _PACKED24_ellipse(int left,int top,int A,int B,int clip);
;----------------------------------------------------------------------------
; Routine draws an Ellipse in 24 bit packed pixel modes.
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
procstart	_PACKED24_ellipse

		ARG		left:UINT, top:UINT, ARGa:UINT, ARGb:UINT,			\
				clip:UINT

		LOCAL	ULaddr:UINT, URaddr:UINT, LLaddr:UINT, LRaddr:UINT,	\
				VARd:ULONG, VARdx:ULONG, VARdy:ULONG,				\
				Asquared:ULONG, Bsquared:ULONG,						\
				FourAsquared:ULONG, FourBsquared:ULONG,				\
				EightAsquared:ULONG, EightBsquared:ULONG,			\
				Set4Pix:NCPTR, _topY:UINT, _botY:UINT, _left:UINT, 	\
				_right:UINT, c_left:UINT, c_top:UINT, c_right:UINT,	\
				c_bottom:UINT, bytesPerLine:UINT,					\
				color:ULONG, writeMode:S_UCHAR = LocalSize

		enter_c	LocalSize

; Load local variables from device context

		mov		eax,[GDC.mi.bytesPerLine]
		mov     [bytesPerLine],eax
		mov		eax,[GDC.intClipRect.left]
		mov		[c_left],eax
		mov		eax,[GDC.intClipRect.top]
		mov		[c_top],eax
		mov		eax,[GDC.intClipRect.right]
		mov		[c_right],eax
		mov		eax,[GDC.intClipRect.bottom]
		mov		[c_bottom],eax
		mov		eax,[GDC.intColor]
		mov		[color],eax
		mov		al,[BYTE GDC.a.writeMode]
		mov		[writeMode],al

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

; Compute initial pixel addresses

		mov		eax,[bytesPerLine]
		mul		[ARGb]				; AX := relative byte offset of B
		mov		esi,eax

		mov		eax,[top]			; AX := top
		mov		ecx,[ARGa]
		shr		ecx,1				; CX := A/2
		add		ecx,[left]			; CX := left + A/2
		call	PACKED24_pixelAddr	; EBX -> buffer

		mov		[ULaddr],ebx		; Save address of these pixels
		mov		[URaddr],ebx
		add		ebx,esi				; EBX := offset of (left + A/2,top + B)
		mov		[LLaddr],ebx		; Save address of these pixels
		mov		[LRaddr],ebx

; Compute initial pixel coordinates (used by clipping version of Set4Pixels)

		mov		eax,[ARGa]
		mov		ebx,eax				; BX := ARGa
		shr		eax,1
		add		eax,[left]			; AX := left + A/2
		mov		[_left],eax
		mov		[_right],eax

		mov		eax,[top]
		mov		[_topY],eax
		add		eax,[ARGb]
		mov		[_botY],eax

; We have two cases to check for. One where the semi-major axis is integer
; and one where it is not. We must set up different initial decision
; variables and pixel addresses for each case.

		test	ebx,1
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

; We must also adjust the initial UR and LR pixel addresses to be
; one position along in the x direction.

		inc		[URaddr]			; Adjust UR and LR pixels
		inc		[URaddr]            ; accordingly
		inc		[URaddr]
		inc		[LRaddr]
		inc		[LRaddr]
		inc		[LRaddr]

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
		cmp		[writeMode],MGL_REPLACE_MODE
		je		@@1
		mov		[Set4pix],offset Set4PixelsROP
@@1:	test	[BYTE clip],1
		jz		@@DontClip

		mov		[Set4pix],offset Clip4Pixels

; Loop while dx > dy

@@DontClip:
		xor		ecx,ecx				; CH := 0 (initial y-increment)
									; CL := 0 (initial x-increment)
		mov		edx,[VARd]			; EDX := decision variable
		mov		esi,[VARdx]			; ESI := dx
		mov		edi,[VARdy]			; EDI := dy

@@Region1:
		cmp		edi,esi
		jge		@@Region2			; Do region 2 if dy >= dx

		call	[Set4pix]

		mov		ecx,1				; CH := 0 (y-increment)
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

		mov		ecx,100h			; CH := 1 (y-increment)
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

@@Exit:	leave_c
		ret

procend		_PACKED24_ellipse

;----------------------------------------------------------------------------
; Set4Pixels	 Sets four pixels in 24 bit packed pixel modes
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

		push	eax					; Preserve these registers
		push	ebx
		push	edx
		push	esi
		push	edi

		mov		ax,[WORD color]
		mov		dh,[BYTE color+2]
		xor		esi,esi				; ESI := 0
		test	ch,ch
		jz		@@NoYinc			; jump if y-increment = 0

		mov		esi,[bytesPerLine]	; ESI := positive y increment

@@NoYinc:
		mov		edi,esi				; ESI,EDI := left horizontal increment

		test	cl,cl
		jz		@@NoXinc			; Jump if x-increment = 0

		dec		esi
		dec		esi
		dec		esi					; ESI := bytes/line - 1
		inc		edi
		inc		edi
		inc		edi					; EDI := bytes/line + 1

@@NoXinc:
		mov		ebx,[ULaddr]
		add		ebx,esi
		mov		[ULaddr],ebx		; Update UL pixel position
		mov		[ebx],ax		; Draw the UL pixel in buffer
		mov		[ebx+2],dh		; Draw the UL pixel in buffer

		mov		ebx,[LLAddr]
		sub		ebx,edi
		mov		[LLAddr],ebx		; Update LL pixel position
		mov		[ebx],ax		; Draw the LL pixel in buffer
		mov		[ebx+2],dh		; Draw the LL pixel in buffer

		mov		ebx,[URaddr]
		add		ebx,edi
		mov		[URaddr],ebx		; Update UR pixel position
		mov		[ebx],ax		; Draw the UR pixel in buffer
		mov		[ebx+2],dh		; Draw the UR pixel in buffer

		mov		ebx,[LRAddr]
		sub		ebx,esi
		mov		[LRAddr],ebx		; Update LR pixel position
		mov		[ebx],ax		; Draw the LR pixel in buffer
		mov		[ebx+2],dh		; Draw the LR pixel in buffer

		pop		edi
		pop		esi
		pop		edx
		pop		ebx
		pop		eax
		ret

ENDP	Set4Pixels

;----------------------------------------------------------------------------
; Set4PixelsROP	 Sets four pixels in 24 bit packed pixel modes using writemode
;----------------------------------------------------------------------------
;
; Entry:		CH	- y-increment (0,-1)
;				CL	- x-increment (0,1)
;
; Note that we are using the same stack frame as the main ellipse drawing
; routine, so we can access all of the variables we need on the stack
; without having to build our own frame.
;----------------------------------------------------------------------------
PROC	Set4PixelsROP	near

		push	eax					; Preserve these registers
		push	ebx
		push	edx
		push	esi
		push	edi

		mov		ax,[WORD color]
		mov		dh,[BYTE color+2]
		mov		dl,[writeMode]
		xor		esi,esi				; ESI := 0
		test	ch,ch
		jz		@@NoYinc			; jump if y-increment = 0

		mov		esi,[bytesPerLine]	; ESI := positive y increment

@@NoYinc:
		mov		edi,esi				; ESI,EDI := left horizontal increment

		test	cl,cl
		jz		@@NoXinc			; Jump if x-increment = 0

		dec		esi
		dec		esi
		dec		esi					; ESI := bytes/line - 1
		inc		edi
		inc		edi
		inc		edi					; EDI := bytes/line + 1

@@NoXinc:
		mov		ebx,[ULaddr]
		add		ebx,esi
		mov		[ULaddr],ebx		; Update UL pixel position
		cmp		dl,MGL_XOR_MODE
		jne		@@1
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],dh
		jmp		@@Out1
@@1:	cmp		dl,MGL_OR_MODE
		jne		@@2
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@Out1
@@2:	and		[ebx],ax
		and		[ebx+2],dh

@@Out1:	mov		ebx,[LLAddr]
		sub		ebx,edi
		mov		[LLAddr],ebx		; Update LL pixel position
		cmp		dl,MGL_XOR_MODE
		jne		@@3
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],dh
		jmp		@@Out2
@@3:	cmp		dl,MGL_OR_MODE
		jne		@@4
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@Out2
@@4:	and		[ebx],ax
		and		[ebx+2],dh

@@Out2:	mov		ebx,[URaddr]
		add		ebx,edi
		mov		[URaddr],ebx		; Update UR pixel position
		cmp		dl,MGL_XOR_MODE
		jne		@@5
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],dh
		jmp		@@Out3
@@5:	cmp		dl,MGL_OR_MODE
		jne		@@6
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@Out3
@@6:	and		[ebx],ax
		and		[ebx+2],dh

@@Out3:	mov		ebx,[LRAddr]
		sub		ebx,esi
		mov		[LRAddr],ebx		; Update LR pixel position
		cmp		dl,MGL_XOR_MODE
		jne		@@7
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],dh
		jmp		@@Out4
@@7:	cmp		dl,MGL_OR_MODE
		jne		@@8
		or		[ebx],ax
		or		[ebx+2],dh
		jmp		@@Out4
@@8:	and		[ebx],ax
		and		[ebx+2],dh

@@Out4:	pop		edi
		pop		esi
		pop		edx
		pop		ebx
		pop		eax
		ret

ENDP	Set4PixelsROP

;----------------------------------------------------------------------------
; Clip4Pixels	 Sets four pixels in 24 bit packed pixel modes
;----------------------------------------------------------------------------
;
; This version of Set4Pixels clips the pixel's on a pixel by pixel basis.
;
; Entry:		CH	- y-increment (0,-1)
;				CL	- x-increment (0,1)
;
;----------------------------------------------------------------------------
PROC	Clip4Pixels	near

		push	eax					; Preserve these registers
		push	ebx
		push	edx
		push	esi
		push	edi

		xor		esi,esi				; ESI := 0
		test	ch,ch
		jz		@@NoYinc			; jump if y-increment = 0

		mov		esi,[bytesPerLine]	; ESI := positive y increment

; Increment the y coordinate placemarkers

		inc		[_topY]
		dec		[_botY]

@@NoYinc:
		mov		edi,esi				; ESI := left increment,
									; EDI := right increment
		test	cl,cl
		jz		@@NoXinc			; jump if x-increment = 0

		dec		esi
		dec		esi
		dec		esi					; ESI := bytes/line - 1
		inc		edi
		inc		edi
		inc		edi					; EDI := bytes/line + 1

; Increment the x coordinate placemarkers

		inc		[_right]
		dec		[_left]

; Plot the Upper Left and Lower Left pixels

@@NoXinc:
		mov		ax,[WORD color]
		mov		ch,[BYTE color+2]
		mov		cl,[writeMode]
		mov		ebx,[ULaddr]
		add		ebx,esi
		mov		[ULaddr],ebx		; Update UL pixel position

		mov		edx,[_left]
		cmp		edx,[c_left]
		jl		@@LowerLeft			; Clip it ...
		cmp		edx,[c_right]
		jge		@@LowerLeft			; Clip it ...

		mov		edx,[_topY]
		cmp		edx,[c_top]
		jl		@@LowerLeft			; Clip it ...
		cmp		edx,[c_bottom]
		jge		@@LowerLeft			; Clip it ...

		cmp		cl,MGL_REPLACE_MODE
		jne		@@1
		mov		[ebx],ax
		mov		[ebx+2],ch
		jmp		@@LowerLeft
@@1:	cmp		cl,MGL_XOR_MODE
		jne		@@2
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],ch
		jmp		@@LowerLeft
@@2:	cmp		cl,MGL_OR_MODE
		jne		@@3
		or		[ebx],ax
		or		[ebx+2],ch
		jmp		@@LowerLeft
@@3:	and		[ebx],ax
		and		[ebx+2],ch

@@LowerLeft:
		mov		ebx,[LLAddr]
		sub		ebx,edi
		mov		[LLAddr],ebx		; Update LL pixel position

		mov		edx,[_left]
		cmp		edx,[c_left]
		jl		@@UpperRight		; Clip it ...
		cmp		edx,[c_right]
		jge		@@UpperRight		; Clip it ...

		mov		edx,[_botY]
		cmp		edx,[c_top]
		jl		@@UpperRight		; Clip it ...
		cmp		edx,[c_bottom]
		jge		@@UpperRight		; Clip it ...

		cmp		cl,MGL_REPLACE_MODE
		jne		@@4
		mov		[ebx],ax
		mov		[ebx+2],ch
		jmp		@@UpperRight
@@4:	cmp		cl,MGL_XOR_MODE
		jne		@@5
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],ch
		jmp		@@UpperRight
@@5:	cmp		cl,MGL_OR_MODE
		jne		@@6
		or		[ebx],ax
		or		[ebx+2],ch
		jmp		@@UpperRight
@@6:	and		[ebx],ax
		and		[ebx+2],ch

@@UpperRight:
		mov		ebx,[URaddr]
		add		ebx,edi
		mov		[URaddr],ebx		; Update UR pixel position

		mov		edx,[_right]
		cmp		edx,[c_left]
		jl		@@UpperLeft			; Clip it ...
		cmp		edx,[c_right]
		jge		@@UpperLeft			; Clip it ...

		mov		edx,[_topY]
		cmp		edx,[c_top]
		jl		@@UpperLeft			; Clip it ...
		cmp		edx,[c_bottom]
		jge		@@UpperLeft			; Clip it ...

		cmp		cl,MGL_REPLACE_MODE
		jne     @@7
		mov		[ebx],ax
		mov		[ebx+2],ch
		jmp		@@UpperLeft
@@7:	cmp		cl,MGL_XOR_MODE
		jne		@@8
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],ch
		jmp		@@UpperLeft
@@8:	cmp		cl,MGL_OR_MODE
		jne		@@9
		or		[ebx],ax
		or		[ebx+2],ch
		jmp		@@UpperLeft
@@9:	and		[ebx],ax
		and		[ebx+2],ch

@@UpperLeft:
		mov		ebx,[LRAddr]
		sub		ebx,esi
		mov		[LRAddr],ebx		; Update LR pixel position

		mov		edx,[_right]
		cmp		edx,[c_left]
		jl		@@Exit				; Clip it ...
		cmp		edx,[c_right]
		jge		@@Exit				; Clip it ...

		mov		edx,[_botY]
		cmp		edx,[c_top]
		jl		@@Exit				; Clip it ...
		cmp		edx,[c_bottom]
		jge		@@Exit				; Clip it ...

		cmp		cl,MGL_REPLACE_MODE
		jne		@@10
		mov		[ebx],ax
		mov		[ebx+2],ch
		jmp		@@Exit
@@10:	cmp		cl,MGL_XOR_MODE
		jne		@@11
		xor		[ebx],ax			; Draw the UL pixel in buffer
		xor		[ebx+2],ch
		jmp		@@Exit
@@11:	cmp		cl,MGL_OR_MODE
		jne		@@12
		or		[ebx],ax
		or		[ebx+2],ch
		jmp		@@Exit
@@12:	and		[ebx],ax
		and		[ebx+2],ch

@@Exit:
		pop		edi
		pop		esi
		pop		edx
		pop		ebx
		pop		eax
		ret

ENDP	Clip4Pixels

;----------------------------------------------------------------------------
; void PACKED24_srcTransScan(void *d,void *s,int count,color_t transparent)
;----------------------------------------------------------------------------
; Translates a scan of information while reversing the blue and red bytes.
;
;	Entry:  s			- Pointer to source scanline
;			d			- Pointer to dest scanline
;			count		- Number of pixels in source scanline
;			transparent	- Transparent source color to skip
;----------------------------------------------------------------------------
procstart	_PACKED24_srcTransScan

		ARG		d:DPTR, s:DPTR, count:UINT, transparent:UINT

		enter_c	0
		cld

		mov		esi,[s]				; ESI -> source scanline
		mov		edi,[d]				; EDI -> dest scanline
		mov		ecx,[count]			; EBX := number of pixels
		mov		ebx,[transparent]	; EBX := transparent pixel color
		mov		edx,ebx
		shr		edx,16				; DL := transparent pixel color high byte
		and		ebx,0FFFFh
		xor		eax,eax

		_ALIGN	4

@@NextPixel:
		mov		ax,[esi]			; Get source pixel
		mov		dh,[esi+2]
		cmp		eax,ebx
		jne		@@Do
		cmp		dl,dh
		je		@@Skip
@@Do:	mov		[edi],ax
		mov		[edi+2],dh
@@Skip:	add		esi,3
		add		edi,3
		dec		ecx
		jnz		@@NextPixel

@@Done:	leave_c_nolocal
		ret

procend		_PACKED24_srcTransScan

;----------------------------------------------------------------------------
; void PACKED24_dstTransScan(void *d,void *s,int count,color_t transparent)
;----------------------------------------------------------------------------
; Translates a scan of information while reversing the blue and red bytes.
;
;	Entry:  s			- Pointer to source scanline
;			d			- Pointer to dest scanline
;			count		- Number of pixels in source scanline
;			transparent	- Transparent source color to skip
;----------------------------------------------------------------------------
procstart	_PACKED24_dstTransScan

		ARG		d:DPTR, s:DPTR, count:UINT, transparent:UINT

		enter_c	0
		cld

		mov		esi,[s]				; ESI -> source scanline
		mov		edi,[d]				; EDI -> dest scanline
		mov		ecx,[count]			; EBX := number of pixels
		mov		ebx,[transparent]	; EBX := transparent pixel color
		mov		edx,ebx
		shr		edx,16				; DL := transparent pixel color high byte

		_ALIGN	4

@@NextPixel:
		cmp		[edi],bx
		jne		@@Skip
		cmp		[edi+2],dl
		jne		@@Skip
		mov		ax,[esi]			; Get source pixel
		mov		dh,[esi+2]
		mov		[edi],ax
		mov		[edi+2],dh
@@Skip:	add		esi,3
		add		edi,3
		dec		ecx
		jnz		@@NextPixel

@@Done:	leave_c_nolocal
		ret

procend		_PACKED24_dstTransScan

;----------------------------------------------------------------------------
; void PACKED24_translateScanBGR(void *d,void *s,int count)
;----------------------------------------------------------------------------
; Translates a scan of information while reversing the blue and red bytes.
;
;	Entry:  s			- Pointer to source scanline
;			d			- Pointer to dest scanline
;			count		- Number of pixels in source scanline
;----------------------------------------------------------------------------
procstart	_PACKED24_translateScanBGR

		ARG		d:DPTR, s:DPTR, count:UINT

		enter_c	0
		cld

		mov		esi,[s]				; ESI -> source scanline
		mov		edi,[d]				; EDI -> dest scanline
		mov		ebx,[count]			; EBX := number of pixels

		_ALIGN	4

@@NextPixel:
		mov		ax,[esi]			; Get source pixel
		mov		dl,[esi+2]
		mov		[edi],dl
		mov		[edi+1],ah
		mov		[edi+2],al
		add		esi,3
		add		edi,3
		dec		ebx
		jnz		@@NextPixel

@@Done:	leave_c_nolocal
		ret

procend		_PACKED24_translateScanBGR

endif	; !MGL_LITE

ifdef	MGL_FIX3D

;----------------------------------------------------------------------------
; void _PACKED24_cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,
;	fix32_t y2,fix32_t c2)
;----------------------------------------------------------------------------
; Routine draws a shaded line for 8 bit packed pixel devices.
;
; Differentiates between horizontal, vertical and sloping lines using
; the midpoint DDA algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				c1		- C1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;				c2		- C2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_PACKED24_cLine

		ARG		x1:FIX32_T, y1:FIX32_T, c1:ZFIX32_T, 					\
				x2:FIX32_T, y2:FIX32_T, c2:ZFIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, EIncr:FIX32_T, 			\
				NEIncr:FIX32_T, varDX:FIX32_T, varDY:FIX32_T, x:UINT, 	\
				y:UINT, cSlope:UINT = LocalSize

		enter_c	LocalSize

		mov		esi,[GDC.mi.bytesPerLine]
		mov		eax,[x2]
		sub		eax,[x1]			; EAX := X2 - X1
		mov		ebx,[y2]
		sub		ebx,[y1]			; EBX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		ebx					; EBX := Y1 - Y2
		neg		esi					; negative increment for buffer

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[vertInc],esi		; save increment
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
		mov		[y],eax				; Save initial x coordinate
		mov		ecx,[x1]
		add		ecx,8000h			; Round to integer value
		shr		ecx,16
		mov		[x],ecx				; Save initial y coordinate
		call	PACKED24_pixelAddr	; EBX -> buffer

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDY]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (x - x1)
		add		edi,eax				; EDI += 2 * dy * (x - x1)
		mov		eax,[varDX]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

; Calculate color slope

@@LoDoneAdjust:
		mov		edx,[c2]
		sub		edx,[c1]			; EDX := c2 - c1
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		sar		edx,16				; in EAX
		idiv	[varDX]
		mov		[cSlope],eax		; ESI := color slope
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[c1]
		mov		esi,[GDC.shadeTab]	; ESI -> current shade table

		_ALIGN	4

@@SetLo:
		mov		eax,edx
		add		edx,[cSlope]		; Interpolate color
		shr		eax,16
		mov		eax,[esi + eax*4]	; Translates via shade table
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al			; Set pixel value in buffer
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx

		or		edi,edi				; Test sign of d
		jg		@@LoPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[vertInc]		; increment y
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDX]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (x - x1)
		sub		edi,eax				; EDI -= 2 * dx * (x - x1)
		mov		eax,[varDY]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

; Calculate color slope

@@HiDoneAdjust:
		mov		edx,[c2]
		sub		edx,[c1]			; EDX := c2 - c1
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		sar		edx,16				; in EAX
		idiv	[varDX]
		mov		[cSlope],eax		; ESI := color slope
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[c1]
		mov		esi,[GDC.shadeTab]	; ESI -> current shade table

		_ALIGN	4

@@SetHi:
		mov		eax,edx
		add		edx,[cSlope]		; Interpolate color
		shr		eax,16
		mov		eax,[esi + eax*4]	; Translates via shade table
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al			; Set pixel value in buffer
		add		ebx,[vertInc]		; increment y

		or		edi,edi				; Test sign of d
		jg		@@HiPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels

@@Exit:	leave_c
		ret

procend		_PACKED24_cLine

;----------------------------------------------------------------------------
; void _PACKED24_zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,
;	fix32_t y2,zfix32_t z2)
;----------------------------------------------------------------------------
; Routine draws a zbuffered line for 8 bit packed pixel devices.
;
; Differentiates between horizontal, vertical and sloping lines using
; the midpoint DDA algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				z1		- Z1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;				z2		- Z2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_PACKED24_zLine16

		ARG		x1:FIX32_T, y1:FIX32_T, z1:ZFIX32_T, 					\
				x2:FIX32_T, y2:FIX32_T, z2:ZFIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, zinc:UINT, EIncr:FIX32_T, 	\
				NEIncr:FIX32_T, varDX:FIX32_T, varDY:FIX32_T, x:UINT, 	\
				y:UINT, zSlope:UINT = LocalSize

		enter_c	LocalSize

		mov		esi,[GDC.mi.bytesPerLine]
		mov		edi,[GDC.zwidth]
		lea		edi,[edi*2]
		mov		eax,[x2]
		sub		eax,[x1]			; EAX := X2 - X1
		mov		ebx,[y2]
		sub		ebx,[y1]			; EBX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		ebx					; EBX := Y1 - Y2
		neg		esi					; negative increment for buffer
		neg		edi					; negative increment for zbuffer

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[vertInc],esi		; save increment
		mov		[zinc],edi
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
		mov		[y],eax				; Save initial x coordinate
		mov		ecx,[x1]
		add		ecx,8000h			; Round to integer value
		shr		ecx,16
		mov		[x],ecx				; Save initial y coordinate
		call	PACKED24_pixelAddr	; EBX -> buffer

; Compute zbuffer starting address

		mov		ecx,[GDC.zwidth]
		mov		eax,[y]				; EAX := top coordinate
		lea		ecx,[ecx*2]
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		imul	ecx					; EAX := y * zwidth
		mov		edx,[x]
		add		esi,eax				; ESI -> zbuffer start address
		sub		edx,[GDC.size.left]
		lea		esi,[esi + edx*2]

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDY]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (x - x1)
		add		edi,eax				; EDI += 2 * dy * (x - x1)
		mov		eax,[varDX]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

; Calculate zbuffer slope

@@LoDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		shr		eax,3				; Convert to 16 bit value in AX
		mov		[zSlope],eax
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[z1]			; EDX := zbuffer coordinate
		shr		edx,15				; Convert to 16 bit value

		_ALIGN	4

@@SetLo:
		cmp		dx,[esi]
		jle		@@SkipLo
		mov		eax,[GDC.intColor]
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al			; Set pixel value in buffer
		mov		[esi],dx			; Set zbuffer coordinate
@@SkipLo:
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		add		dx,[WORD zSlope]	; Increment zbuffer coordinate

		or		edi,edi				; Test sign of d
		jg		@@LoPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDX]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (x - x1)
		sub		edi,eax				; EDI -= 2 * dx * (x - x1)
		mov		eax,[varDY]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

; Calculate zbuffer slope

@@HiDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		shr		eax,3				; Convert to 16 bits in AX
		mov		[zSlope],eax
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		eax,[GDC.intColor]
		mov		edx,[z1]			; EDX := zbuffer coordinate
		shr		edx,15				; Convert to 16 bit value

		_ALIGN	4

@@SetHi:
		cmp		dx,[esi]
		jle		@@SkipHi
		mov		eax,[GDC.intColor]
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al			; Set pixel value in buffer
		mov		[esi],dx			; Set zbuffer coordinate
@@SkipHi:
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer
		add		dx,[WORD zSlope]	; Increment zbuffer coordinate

		or		edi,edi				; Test sign of d
		jg		@@HiPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels

@@Exit:	leave_c
		ret

procend		_PACKED24_zLine16

;----------------------------------------------------------------------------
; void _PACKED24_zLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,
;	fix32_t y2,zfix32_t z2)
;----------------------------------------------------------------------------
; Routine draws a zbuffered line for 8 bit packed pixel devices.
;
; Differentiates between horizontal, vertical and sloping lines using
; the midpoint DDA algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				z1		- Z1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;				z2		- Z2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_PACKED24_zLine32

		ARG		x1:FIX32_T, y1:FIX32_T, z1:ZFIX32_T, 					\
				x2:FIX32_T, y2:FIX32_T, z2:ZFIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, zinc:UINT, EIncr:FIX32_T, 	\
				NEIncr:FIX32_T, varDX:FIX32_T, varDY:FIX32_T, x:UINT, 	\
				y:UINT, zSlope:UINT = LocalSize

		enter_c	LocalSize

		mov		esi,[GDC.mi.bytesPerLine]
		mov		edi,[GDC.zwidth]
		lea		edi,[edi*4]
		mov		eax,[x2]
		sub		eax,[x1]			; EAX := X2 - X1
		mov		ebx,[y2]
		sub		ebx,[y1]			; EBX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		ebx					; EBX := Y1 - Y2
		neg		esi					; negative increment for buffer
		neg		edi					; negative increment for zbuffer

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[vertInc],esi		; save increment
		mov		[zinc],edi
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
		mov		[y],eax				; Save initial x coordinate
		mov		ecx,[x1]
		add		ecx,8000h			; Round to integer value
		shr		ecx,16
		mov		[x],ecx				; Save initial y coordinate
		call	PACKED24_pixelAddr	; EBX -> buffer

; Compute zbuffer starting address

		mov		ecx,[GDC.zwidth]
		mov		eax,[y]				; EAX := top coordinate
		lea		ecx,[ecx*4]
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		imul	ecx					; EAX := y * zwidth
		mov		edx,[x]
		add		esi,eax				; ESI -> zbuffer start address
		sub		edx,[GDC.size.left]
		lea		esi,[esi + edx*4]

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDY]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (x - x1)
		add		edi,eax				; EDI += 2 * dy * (x - x1)
		mov		eax,[varDX]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

; Calculate zbuffer slope

@@LoDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		mov		ecx,eax
		shl		edx,12				; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx				; Position so result ends up in EAX
		div		[varDX]				; Find last 12 fractional bits
		shl		eax,20				; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12			; ECX := result
		mov		[zSlope],ecx
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[z1]			; EDX := zbuffer coordinate

		_ALIGN	4

@@SetLo:
		cmp		edx,[esi]
		jle		@@SkipLo
		mov		eax,[GDC.intColor]
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al			; Set pixel value in buffer
		mov		[esi],edx			; Set zbuffer coordinate
@@SkipLo:
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		inc		esi
		inc		esi
		add		edx,[zSlope]		; Increment zbuffer coordinate

		or		edi,edi				; Test sign of d
		jg		@@LoPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDX]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (x - x1)
		sub		edi,eax				; EDI -= 2 * dx * (x - x1)
		mov		eax,[varDY]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

; Calculate zbuffer slope

@@HiDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		mov		ecx,eax
		shl		edx,12				; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx				; Position so result ends up in EAX
		div		[varDX]				; Find last 12 fractional bits
		shl		eax,20				; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12			; ECX := result
		mov		[zSlope],ecx
		mov		ecx,[varDX]
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		eax,[GDC.intColor]
		mov		edx,[z1]			; EDX := zbuffer coordinate

		_ALIGN	4

@@SetHi:
		cmp		edx,[esi]
		jle		@@SkipHi
		mov		eax,[GDC.intColor]
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al			; Set pixel value in buffer
		mov		[esi],edx			; Set zbuffer coordinate
@@SkipHi:
		add		edx,[zSlope]		; Increment zbuffer coordinate
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer

		or		edi,edi				; Test sign of d
		jg		@@HiPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		inc		esi
		inc		esi
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels

@@Exit:	leave_c
		ret

procend		_PACKED24_zLine32

;----------------------------------------------------------------------------
; void _PACKED24_czLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,
;	fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2)
;----------------------------------------------------------------------------
; Routine draws a zbuffered line for 8 bit packed pixel devices.
;
; Differentiates between horizontal, vertical and sloping lines using
; the midpoint DDA algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				z1		- Z1 coordinate of line to draw (fixed point)
;				c1		- C1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;				z2		- Z2 coordinate of line to draw (fixed point)
;				c2		- c2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_PACKED24_czLine16

		ARG		x1:FIX32_T, y1:FIX32_T, z1:ZFIX32_T, c1:FIX32_T,		\
				x2:FIX32_T, y2:FIX32_T, z2:ZFIX32_T, c2:FIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, zinc:UINT, EIncr:FIX32_T, 	\
				NEIncr:FIX32_T, varDX:FIX32_T, varDY:FIX32_T, x:UINT, 	\
				y:UINT, zSlope:UINT, cSlope:UINT = LocalSize

		enter_c	LocalSize

		mov		esi,[GDC.mi.bytesPerLine]
		mov		edi,[GDC.zwidth]
		lea		edi,[edi*2]
		mov		eax,[x2]
		sub		eax,[x1]			; EAX := X2 - X1
		mov		ebx,[y2]
		sub		ebx,[y1]			; EBX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		ebx					; EBX := Y1 - Y2
		neg		esi					; negative increment for buffer
		neg		edi					; negative increment for zbuffer

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[vertInc],esi		; save increment
		mov		[zinc],edi
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
		mov		[y],eax				; Save initial x coordinate
		mov		ecx,[x1]
		add		ecx,8000h			; Round to integer value
		shr		ecx,16
		mov		[x],ecx				; Save initial y coordinate
		call	PACKED24_pixelAddr	; EBX -> buffer

; Compute zbuffer starting address

		mov		ecx,[GDC.zwidth]
		mov		eax,[y]				; EAX := top coordinate
		lea		ecx,[ecx*2]
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		imul	ecx					; EAX := y * zwidth
		mov		edx,[x]
		add		esi,eax				; ESI -> zbuffer start address
		sub		edx,[GDC.size.left]
		lea		esi,[esi + edx*2]

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDY]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (x - x1)
		add		edi,eax				; EDI += 2 * dy * (x - x1)
		mov		eax,[varDX]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

; Calculate zbuffer slope

@@LoDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		shr		eax,3				; Adjust to 16 bit value
		mov		[zSlope],eax
		mov		edx,[c2]
		sub		edx,[c1]			; EDX := c2 - c1
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		mov		ecx,[varDX]
		sar		edx,16				; in EAX
		idiv	ecx
		mov		[cSlope],eax		; ESI := color slope
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[z1]			; EDX := zbuffer coordinate
		shr		edx,15				; Adjust to 16 bit value

		_ALIGN	4

@@SetLo:
		cmp		dx,[esi]
		jle		@@SkipLo
		mov		eax,[c1]
		shr		eax,16
		lea		eax,[eax*4]
		add		eax,[GDC.shadeTab]
		mov		eax,[eax]			; Translate via shade table
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al
		mov		[esi],dx			; Set zbuffer coordinate
@@SkipLo:
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		add		dx,[WORD zSlope]	; Increment zbuffer coordinate
		mov		eax,[c1]
		add		eax,[gcSlope]		; Interpolate color
		mov		[c1],eax

		or		edi,edi				; Test sign of d
		jg		@@LoPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDX]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (x - x1)
		sub		edi,eax				; EDI -= 2 * dx * (x - x1)
		mov		eax,[varDY]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

; Calculate zbuffer slope

@@HiDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		shr		eax,3				; Adjust to 16 bit value in AX
		mov		[zSlope],eax
		mov		edx,[c2]
		sub		edx,[c1]			; EDX := c2 - c1
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		mov		ecx,[varDX]
		sar		edx,16				; in EAX
		idiv	ecx
		mov		[cSlope],eax		; ESI := color slope
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[z1]			; EDX := zbuffer coordinate
		shr		edx,15				; Adjust to 16 bit value

		_ALIGN	4

@@SetHi:
		cmp		dx,[esi]
		jle		@@SkipHi
		mov		eax,[c1]
		shr		eax,16
		lea		eax,[eax*4]
		add		eax,[GDC.shadeTab]
		mov		eax,[eax]			; Translate via shade table
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al
		mov		[esi],dx			; Set zbuffer coordinate
@@SkipHi:
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer
		mov		eax,[c1]
		add		eax,[gcSlope]		; Interpolate color
		mov		[c1],eax
		add		dx,[WORD zSlope]	; Increment zbuffer coordinate

		or		edi,edi				; Test sign of d
		jg		@@HiPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		inc		esi
		inc		esi
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels

@@Exit:	leave_c
		ret

procend		_PACKED24_czLine16

;----------------------------------------------------------------------------
; void _PACKED24_czLine32(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,
;	fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2)
;----------------------------------------------------------------------------
; Routine draws a zbuffered line for 8 bit packed pixel devices.
;
; Differentiates between horizontal, vertical and sloping lines using
; the midpoint DDA algorithm.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				z1		- Z1 coordinate of line to draw (fixed point)
;				c1		- C1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;				z2		- Z2 coordinate of line to draw (fixed point)
;				c2		- c2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_PACKED24_czLine32

		ARG		x1:FIX32_T, y1:FIX32_T, z1:ZFIX32_T, c1:FIX32_T,		\
				x2:FIX32_T, y2:FIX32_T, z2:ZFIX32_T, c2:FIX32_T

		LOCAL	Routine:NCPTR, VertInc:UINT, zinc:UINT, EIncr:FIX32_T, 	\
				NEIncr:FIX32_T, varDX:FIX32_T, varDY:FIX32_T, x:UINT, 	\
				y:UINT, zSlope:UINT, cSlope:UINT = LocalSize

		enter_c	LocalSize

		mov		esi,[GDC.mi.bytesPerLine]
		mov		edi,[GDC.zwidth]
		lea		edi,[edi*4]
		mov		eax,[x2]
		sub		eax,[x1]			; EAX := X2 - X1
		mov		ebx,[y2]
		sub		ebx,[y1]			; EBX := Y2 - Y1
		jns		@@Y2Greater			; Jump if slope is positive

		neg		ebx					; EBX := Y1 - Y2
		neg		esi					; negative increment for buffer
		neg		edi					; negative increment for zbuffer

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[vertInc],esi		; save increment
		mov		[zinc],edi
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
		mov		[y],eax				; Save initial x coordinate
		mov		ecx,[x1]
		add		ecx,8000h			; Round to integer value
		shr		ecx,16
		mov		[x],ecx				; Save initial y coordinate
		call	PACKED24_pixelAddr	; EBX -> buffer

; Compute zbuffer starting address

		mov		ecx,[GDC.zwidth]
		mov		eax,[y]				; EAX := top coordinate
		lea		ecx,[ecx*4]
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		imul	ecx					; EAX := y * zwidth
		mov		edx,[x]
		add		esi,eax				; ESI -> zbuffer start address
		sub		edx,[GDC.size.left]
		lea		esi,[esi + edx*4]

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDY]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (x - x1)
		add		edi,eax				; EDI += 2 * dy * (x - x1)
		mov		eax,[varDX]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@LoNegativeInc
		sub		edi,eax				; EDI -= 2 * dx * (y - y1)
		jmp		@@LoDoneAdjust
@@LoNegativeInc:
		add		edi,eax				; EDI += 2 * dx * (y - y1)

; Calculate zbuffer slope

@@LoDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		mov		ecx,eax
		shl		edx,12				; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx				; Position so result ends up in EAX
		div		[varDX]				; Find last 12 fractional bits
		shl		eax,20				; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12			; ECX := result
		mov		[zSlope],ecx
		mov		edx,[c2]
		sub		edx,[c1]			; EDX := c2 - c1
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		mov		ecx,[varDX]
		sar		edx,16				; in EAX
		idiv	ecx
		mov		[cSlope],eax		; ESI := color slope
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[z1]			; EDX := zbuffer coordinate

		_ALIGN	4

@@SetLo:
		cmp		edx,[esi]
		jle		@@SkipLo
		mov		eax,[c1]
		shr		eax,16
		lea		eax,[eax*4]
		add		eax,[GDC.shadeTab]
		mov		eax,[eax]			; Translate via shade table
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al
		mov		[esi],edx			; Set zbuffer coordinate
@@SkipLo:
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		inc		esi
		inc		esi
		add		edx,[zSlope]		; Increment zbuffer coordinate
		mov		eax,[c1]
		add		eax,[gcSlope]		; Interpolate color
		mov		[c1],eax

		or		edi,edi				; Test sign of d
		jg		@@LoPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer
		dec		ecx
		jnz		@@SetLo				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						EBX -> video buffer
;						EDI = Initial decision variable
;						ESI -> zbuffer
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:

; Adjust the initial decision variable depending on the difference
; between the real line and the initial starting point we have selected

		mov		eax,[varDX]
		shl		eax,1
		mov     edx,[x]
		shl		edx,16
		sub		edx,[x1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dx * (x - x1)
		sub		edi,eax				; EDI -= 2 * dx * (x - x1)
		mov		eax,[varDY]
		shl		eax,1
		mov		edx,[y]
		shl		edx,16
		sub     edx,[y1]
		imul	edx
		shrd	eax,edx,16			; EAX := 2 * dy * (y - y1)
		mov		edx,[VertInc]
		or		edx,edx
		js      @@HiNegativeInc
		add		edi,eax				; EDI += 2 * dy * (y - y1)
		jmp		@@HiDoneAdjust
@@HiNegativeInc:
		sub		edi,eax				; EDI -= 2 * dy * (y - y1)

; Calculate zbuffer slope

@@HiDoneAdjust:
		mov		edx,[z2]
		sub		edx,[z1]			; EDX := z2 - z1
		xor		eax,eax
		shrd	eax,edx,28			; position so that result ends up
		sar		edx,28				; in EAX
		idiv	[varDX]
		mov		ecx,eax
		shl		edx,12				; Normalise remainder portion
		mov		eax,edx
		xor		edx,edx				; Position so result ends up in EAX
		div		[varDX]				; Find last 12 fractional bits
		shl		eax,20				; EAX := bottom 12 bits in 32:20
		shld    ecx,eax,12			; ECX := result
		mov		[zSlope],ecx
		mov		edx,[c2]
		sub		edx,[c1]			; EDX := c2 - c1
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		mov		ecx,[varDX]
		sar		edx,16				; in EAX
		idiv	ecx
		mov		[cSlope],eax		; ESI := color slope
		shr		ecx,16				; Truncate to integer value
		inc		ecx					; ECX := # pixels to draw
		mov		edx,[z1]			; EDX := zbuffer coordinate

		_ALIGN	4

@@SetHi:
		cmp		edx,[esi]
		jle		@@SkipHi
		mov		eax,[c1]
		shr		eax,16
		lea		eax,[eax*4]
		add		eax,[GDC.shadeTab]
		mov		eax,[eax]			; Translate via shade table
		mov		[ebx],ax			; Set pixel value in buffer
		shr		eax,16
		mov		[ebx+2],al
		mov		[esi],edx			; Set zbuffer coordinate
@@SkipHi:
		add		ebx,[vertInc]		; increment y
		add		esi,[zinc]			; increment zbuffer
		mov		eax,[c1]
		add		eax,[gcSlope]		; Interpolate color
		mov		[c1],eax
		add		edx,[zSlope]		; Increment zbuffer coordinate

		or		edi,edi				; Test sign of d
		jg		@@HiPosDi			; Jump if d > 0

		add		edi,[EIncr]			; d := d + EIncr
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		edi,[NEIncr]		; d := d + NEIncr
		inc		ebx					; Increment x coordinate
		inc		ebx
		inc		ebx
		inc		esi					; Increment zbuffer coordinate
		inc		esi
		inc		esi
		inc		esi
		dec		ecx
		jnz		@@SetHi				; Loop for remaining pixels

@@Exit:	leave_c
		ret

procend		_PACKED24_czLine32

;----------------------------------------------------------------------------
; void PACKED24_cTrap(void)
;----------------------------------------------------------------------------
; Routine draws a smooth shaded trapezoid. This routine only works for
; trapezoids in REPLACE mode. The high level C code versions will be called
; in other write modes etc.
;----------------------------------------------------------------------------
procstart	_PACKED24_cTrap

		push	ebp
		push	ebx
		push	esi
		push	edi
		cmp		[GDC.tr.count],0
		je		@@Exit				; Bail out for zero height traps
		cld							; String instructions go up

		mov		edi,[GDC.surface]	; EDI -> start of device surface
		mov		ebx,[GDC.tr.count]
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		mov		[gcount],ebx
		add		[GDC.tr.y],ebx		; Increment coordinate for next trap
		imul	[GDC.mi.bytesPerLine]; EAX := y * BytesPerLine
		add		edi,eax				; EDI -> framebuffer start address
		mov		edx,[GDC.tr.x1]		; EDX := X1
		mov		esi,[GDC.tr.x2]		; ESI := X2
		mov		ebp,[GDC.shadeTab]	; EBP := Current shade table

; Check that xstart < xend, swapping if need be.

		_ALIGN	4

@@NextScan:
		push	edx
		push	esi
		push	edi

		mov		ebx,edx				; EBX := X1
		mov		eax,esi				; EAX := X2
		add		ebx,8000h			; Round
		add		eax,8000h			; Round
		sub     esi,edx				; ESI := X2 - X1
		shr		ebx,16				; EBX := xstart for scanline
		shr		eax,16				; EAX := xend for scanline
		mov		ecx,[GDC.tr.c1]		; Get scanline color values
		mov     edx,[GDC.tr.c2]
		cmp		ebx,eax				; is xstart > xend?
		jz		@@DoneScan			; ignore if xstart == xend
		jl		@@NoSwap			; no, don't swap them
		xchg	eax,ebx				; swap ix1,ix2
		xchg	ecx,edx				; swap ic1,ic2
		neg		esi					; Negate dx value

; Compute the color slope for the current scanline

@@NoSwap:
		sub		eax,ebx				; EAX := xend - xstart (# pixels to draw)
		add		edi,ebx				
		add		edi,ebx
		add		edi,ebx				; EDI -> first pixel to fill
		mov		ebx,ecx				; EBX := ic1
		sub		edx,ecx				; EDX := ic2 - ic1
		mov		ecx,eax				; ECX := # pixels to draw
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		sar		edx,16				; in EAX
		idiv	esi
		mov		esi,eax				; ESI := color slope

; Blast out pixels in a tight loop. Unrolling this loop is actually slower
; on Pentium chips.

		_ALIGN	4

@@NextPixel:
		mov		eax,ebx				; Get fixed point color value
		add		ebx,esi				; Interpolate color for next pixel
		shr		eax,16				; Convert to integer
		mov		eax,[ebp+eax*4]		; Translate color via shade table
		mov		[edi],ax			; Store the current color value
		shr		eax,16
		mov		[edi+2],al
		inc		edi
		inc		edi
		inc		edi
		dec		ecx
		jnz		@@NextPixel

		_ALIGN	4

@@DoneScan:
		pop		edi
		pop		esi
		pop		edx
		add		edi,[GDC.mi.bytesPerLine]; Increment address to next scanline
		mov		eax,[GDC.tr.cSlope1]
		mov		ebx,[GDC.tr.cSlope2]
		add		edx,[GDC.tr.slope1]	; x1 += slope1
		add		esi,[GDC.tr.slope2]	; x2 += slope2
		add		[GDC.tr.c1],eax		; c1 += cSlope1
		add		[GDC.tr.c2],ebx		; c2 += cSlope2
		dec		[gcount]
		jnz		@@NextScan			; Loop for all scanlines

; Move the interpolated X1 and X2 values back from locals

		mov		[GDC.tr.x1],edx		; Save interpolated x1
		mov		[GDC.tr.x2],esi		; Save interpolated x2

@@Exit:	pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_PACKED24_cTrap

;----------------------------------------------------------------------------
; void PACKED24_zTrap16(void)
;----------------------------------------------------------------------------
; Routine draws a flat shaded zbuffered trapezoid. This routine only works for
; trapezoids in REPLACE mode. The high level C code versions will be called
; in other write modes etc.
;----------------------------------------------------------------------------
procstart	_PACKED24_zTrap16

		push	ebp
		push	ebx
		push	esi
		push	edi
		cmp		[GDC.tr.count],0
		je		@@Exit				; Bail out for zero height traps
		cld							; String instructions go up

; Compute framebuffer starting address

		mov		edi,[GDC.surface]	; EDI -> start of device surface
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		imul	[GDC.mi.bytesPerLine]; EAX := y * BytesPerLine
		add		edi,eax				; EDI -> framebuffer start address

; Compute zbuffer starting address

		mov		eax,[GDC.zwidth]
		shl		eax,1
		mov		[gzwidth],eax		; Save zbuffer increment
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		mov		ebx,[GDC.tr.count]
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		add		[GDC.tr.y],ebx		; Increment coordinate for next trap
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		imul	[gzwidth]			; EAX := y * zbytesPerLine
		add		esi,eax				; ESI -> zbuffer start address
		mov		eax,[GDC.size.left]
		mov		[gzleftAdj],eax		; Save left zbuffer coord adjustment

; Check that xstart < xend, swapping if need be.

		_ALIGN	4

@@NextScan:
		push	esi
		push	edi					; Preserve EDI
		mov		ebx,[GDC.tr.x1]		; EBX := X1
		mov		eax,[GDC.tr.x2]		; EAX := X2
		add		ebx,8000h			; Round
		add		eax,8000h			; Round
		shr		ebx,16				; EBX := xstart for scanline
		shr		eax,16				; EAX := xend for scanline
		cmp		ebx,eax				; is xstart > xend?
		jz		@@DoneScan			; ignore if xstart == xend
		jl		@@NoSwap			; no, don't swap them
		xchg	eax,ebx				; swap ix1,ix2

@@NoSwap:
		mov		ecx,eax				; ECX := xend
		sub		ecx,ebx				; ECX := xend - xstart (# pixels to draw)
		add		edi,ebx
		add		edi,ebx
		add		edi,ebx				; EDI -> first pixel in framebuffer
		lea		edx,[ebx*2]
		sub		ebx,[gzleftAdj]		; Adjust left zbuffer coord
		add		esi,ebx
		add		esi,ebx				; ESI -> first pixel in zbuffer
		add		edx,[GDC.zOffset]	; EDX -> zOffset[x1]
		mov		ebp,[GDC.tr.z0]		; EBX := initial z value
		shr		ebp,15				; Convert to 16 bit value

; Blast out pixels in an unrolled loop.

		_ALIGN	4

numloops = 8

@@NextPixel:
off = 0
REPT	numloops
LOCAL	@@Skip
		mov		ebx,ebp				; Get initial z value
		add		bx,[edx+off*2]		; Interpolate z
		cmp     bx,[esi+off*2]		; Compare z value
		jle		@@Skip				; Z value is smaller
		mov		eax,[GDC.intColor]
		mov		[edi+off*3],ax		; Store color value
		shr		eax,16
		mov		[edi+off*3+2],al	; Store color value
		mov		[esi+off*2],bx		; Store z value
@@Skip:	dec		ecx
		jz		@@DoneScan
off = off + 1
ENDM
		add     edx,off*2			; Increment z offset buffer
		add		esi,off*2			; Increment z buffer
		add		edi,off*3			; Increment framebuffer
		jmp		@@NextPixel

		_ALIGN	4

@@DoneScan:
		pop		edi
		pop		esi
		add		edi,[GDC.mi.bytesPerLine]; Increment address to next scanline
		add		esi,[gzwidth]		; Increment zbuffer address
		mov     eax,[GDC.tr.x1]		; Interpolate edges
		mov     ebx,[GDC.tr.x2]
		mov		ecx,[GDC.tr.z0]
		add		eax,[GDC.tr.slope1]	; x1 += slope1
		add		ebx,[GDC.tr.slope2]	; x2 += slope2
		add		ecx,[GDC.tr.dZdY]	; z0 += dCdY
		mov		[GDC.tr.x1],eax
		mov		[GDC.tr.x2],ebx
		mov		[GDC.tr.z0],ecx
		dec		[GDC.tr.count]
		jnz		@@NextScan			; Loop for all scanlines

@@Exit:	pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_PACKED24_zTrap16

;----------------------------------------------------------------------------
; void PACKED24_zTrap32(void)
;----------------------------------------------------------------------------
; Routine draws a flat shaded zbuffered trapezoid. This routine only works for
; trapezoids in REPLACE mode. The high level C code versions will be called
; in other write modes etc.
;----------------------------------------------------------------------------
procstart	_PACKED24_zTrap32

		push	ebp
		push	ebx
		push	esi
		push	edi
		cmp		[GDC.tr.count],0
		je		@@Exit				; Bail out for zero height traps
		cld							; String instructions go up

; Compute framebuffer starting address

		mov		edi,[GDC.surface]	; EDI -> start of device surface
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		imul	[GDC.mi.bytesPerLine]; EAX := y * BytesPerLine
		add		edi,eax				; EDI -> framebuffer start address

; Compute zbuffer starting address

		mov		eax,[GDC.zwidth]
		shl		eax,2
		mov		[gzwidth],eax		; Save zbuffer increment
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		mov		ebx,[GDC.tr.count]
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		add		[GDC.tr.y],ebx		; Increment coordinate for next trap
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		imul	[gzwidth]			; EAX := y * zbytesPerLine
		add		esi,eax				; ESI -> zbuffer start address
		mov		eax,[GDC.size.left]
		mov		[gzleftAdj],eax		; Save left zbuffer coord adjustment

; Check that xstart < xend, swapping if need be.

		_ALIGN	4

@@NextScan:
		push	esi
		push	edi					; Preserve EDI
		mov		ebx,[GDC.tr.x1]		; EBX := X1
		mov		eax,[GDC.tr.x2]		; EAX := X2
		add		ebx,8000h			; Round
		add		eax,8000h			; Round
		shr		ebx,16				; EBX := xstart for scanline
		shr		eax,16				; EAX := xend for scanline
		cmp		ebx,eax				; is xstart > xend?
		jz		@@DoneScan			; ignore if xstart == xend
		jl		@@NoSwap			; no, don't swap them
		xchg	eax,ebx				; swap ix1,ix2

@@NoSwap:
		mov		ecx,eax				; ECX := xend
		sub		ecx,ebx				; ECX := xend - xstart (# pixels to draw)
		add		edi,ebx
		add		edi,ebx
		add		edi,ebx				; EDI -> first pixel in framebuffer
		lea		edx,[ebx*4]
		sub		ebx,[gzleftAdj]		; Adjust left zbuffer coord
		add		esi,ebx
		add		esi,ebx
		add		esi,ebx
		add		esi,ebx				; ESI -> first pixel in zbuffer
		add		edx,[GDC.zOffset]	; EDX -> zOffset[x1]
		mov		ebx,[GDC.tr.z0]		; EBX := initial z value

; Blast out pixels in an unrolled loop.

		_ALIGN	4

numloops = 8

@@NextPixel:
off = 0
REPT	numloops
LOCAL	@@Skip
		mov		ebp,ebx				; Get initial z value
		add		ebp,[edx+off*4]		; Interpolate z
		cmp     ebp,[esi+off*4]		; Compare z value
		jle		@@Skip				; Z value is smaller
		mov		eax,[GDC.intColor]
		mov		[edi+off*3],ax		; Store color value
		shr		eax,16
		mov		[edi+off*3+2],al	; Store color value
		mov		[esi+off*4],ebp		; Store z value
@@Skip:	dec		ecx
		jz		@@DoneScan
off = off + 1
ENDM
		add     edx,off*4			; Increment z offset buffer
		add		esi,off*4			; Increment z buffer
		add		edi,off*3			; Increment framebuffer
		jmp		@@NextPixel

		_ALIGN	4

@@DoneScan:
		pop		edi
		pop		esi
		add		edi,[GDC.mi.bytesPerLine]; Increment address to next scanline
		add		esi,[gzwidth]		; Increment zbuffer address
		mov     eax,[GDC.tr.x1]		; Interpolate edges
		mov     ebx,[GDC.tr.x2]
		mov		ecx,[GDC.tr.z0]
		add		eax,[GDC.tr.slope1]	; x1 += slope1
		add		ebx,[GDC.tr.slope2]	; x2 += slope2
		add		ecx,[GDC.tr.dZdY]	; z0 += dCdY
		mov		[GDC.tr.x1],eax
		mov		[GDC.tr.x2],ebx
		mov		[GDC.tr.z0],ecx
		dec		[GDC.tr.count]
		jnz		@@NextScan			; Loop for all scanlines

@@Exit:	pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_PACKED24_zTrap32

;----------------------------------------------------------------------------
; void PACKED24_czTrap16(void)
;----------------------------------------------------------------------------
; Routine draws a smooth shaded zbuffered trapezoid. This routine only works
; for trapezoids in REPLACE mode. The high level C code versions will be
; called in other write modes etc.
;----------------------------------------------------------------------------
procstart	_PACKED24_czTrap16

		push	ebp
		push	ebx
		push	esi
		push	edi
		cmp		[GDC.tr.count],0
		je		@@Exit				; Bail out for zero height traps
		cld							; String instructions go up

; Compute framebuffer starting address

		mov		edi,[GDC.surface]	; EDI -> start of device surface
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		imul	[GDC.mi.bytesPerLine]; EAX := y * BytesPerLine
		add		edi,eax				; EDI -> framebuffer start address

; Compute zbuffer starting address

		mov		eax,[GDC.zwidth]
		shl		eax,1
		mov		[gzwidth],eax		; Save zbuffer increment
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		mov		ebx,[GDC.tr.count]
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		add		[GDC.tr.y],ebx		; Increment coordinate for next trap
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		imul	[gzwidth]			; EAX := y * zbytesPerLine
		add		esi,eax				; ESI -> zbuffer start address
		mov		eax,[GDC.size.left]
		mov		[gzleftAdj],eax		; Save left zbuffer coord adjustment

; Check that xstart < xend, swapping if need be.

		_ALIGN	4

@@NextScan:
		push	esi
		push	edi

		mov		eax,[GDC.tr.x2]		; EAX := X2
		mov		ebx,[GDC.tr.x1]		; EBX := X1
		mov		ebp,eax
		sub     ebp,ebx				; EBP := X2 - X1
		add		ebx,8000h			; Round
		add		eax,8000h			; Round
		shr		ebx,16				; EBX := xstart for scanline
		shr		eax,16				; EAX := xend for scanline
		mov		ecx,[GDC.tr.c1]		; Get scanline color values
		mov     edx,[GDC.tr.c2]
		cmp		ebx,eax				; is xstart > xend?
		jz		@@DoneScan			; ignore if xstart == xend
		jl		@@NoSwap			; no, don't swap them
		xchg	eax,ebx				; swap ix1,ix2
		xchg	ecx,edx				; swap ic1,ic2
		neg		ebp                 ; Negate dx value

@@NoSwap:
		sub		eax,ebx				; EAX := xend - xstart (# pixels to draw)
		add		edi,ebx
		add		edi,ebx
		add		edi,ebx				; EDI -> first pixel to fill
		mov		[gc1],ecx			; Save ic1 color value
		sub		edx,ecx				; EDX := ic2 - ic1
		mov		ecx,eax				; ECX := # pixels to draw
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		sar		edx,16				; in EAX
		idiv	ebp
		mov		[gcSlope],eax		; Save color slope
		mov		ebp,[gc1]			; EBP := initial fixed point color value
		lea		edx,[ebx*2]
		sub		ebx,[gzleftAdj]		; Adjust left zbuffer coord
		add		esi,ebx
		add		esi,ebx				; ESI -> first pixel in zbuffer
		add		edx,[GDC.zOffset]	; EDX -> zOffset[x1]
		mov		ebx,[GDC.tr.z0]		; EBX := initial z value
		shr		ebx,15				; Convert to 16 bit value

; Blast out pixels to the framebuffer

		_ALIGN	4

numloops = 8

@@NextPixel:
off = 0
REPT	numloops
LOCAL	@@Skip
		mov		eax,ebx				; Get initial z value
		add		ax,[edx+off*2]		; Interpolate z
		cmp     ax,[esi+off*2]		; Compare z value
		jle		@@Skip				; Z value is smaller
		mov		[esi+off*2],ax		; Store z value
		mov		eax,ebp				; Get fixed point color value
		shr		eax,16				; Convert to integer
		lea		eax,[eax*4]			; Translate color via shade table
		add		eax,[GDC.shadeTab]
		mov		eax,[eax]
		mov		[edi+off*3],ax		; Store color value
		shr		eax,16
		mov		[edi+off*3+2],al	; Store color value
@@Skip:	add		ebp,[gcSlope]		; Interpolate color
		dec		ecx
		jz		@@DoneScan
off = off + 1
ENDM
		add     edx,off*2			; Increment z offset buffer
		add		esi,off*2			; Increment z buffer
		add		edi,off*3			; Increment framebuffer
		jmp		@@NextPixel

		_ALIGN	4

@@DoneScan:
		pop		edi
		pop		esi
		add		edi,[GDC.mi.bytesPerLine]; Increment address to next scanline
		add		esi,[gzwidth]		; Increment zbuffer address
		mov     eax,[GDC.tr.x1]		; Interpolate edges
		mov     ebx,[GDC.tr.x2]
		mov		ecx,[GDC.tr.z0]
		add		eax,[GDC.tr.slope1]	; x1 += slope1
		add		ebx,[GDC.tr.slope2]	; x2 += slope2
		add		ecx,[GDC.tr.dZdY]	; z0 += dCdY
		mov		[GDC.tr.x1],eax
		mov		[GDC.tr.x2],ebx
		mov		[GDC.tr.z0],ecx
		mov		eax,[GDC.tr.cSlope1]
		mov		ebx,[GDC.tr.cSlope2]
		add		[GDC.tr.c1],eax		; c1 += cSlope1
		add		[GDC.tr.c2],ebx		; c2 += cSlope2
		dec		[GDC.tr.count]
		jnz		@@NextScan			; Loop for all scanlines

@@Exit:	pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_PACKED24_czTrap16

;----------------------------------------------------------------------------
; void PACKED24_czTrap32(void)
;----------------------------------------------------------------------------
; Routine draws a smooth shaded zbuffered trapezoid. This routine only works
; for trapezoids in REPLACE mode. The high level C code versions will be
; called in other write modes etc.
;----------------------------------------------------------------------------
procstart	_PACKED24_czTrap32

		push	ebp
		push	ebx
		push	esi
		push	edi
		cmp		[GDC.tr.count],0
		je		@@Exit				; Bail out for zero height traps
		cld							; String instructions go up

; Compute framebuffer starting address

		mov		edi,[GDC.surface]	; EDI -> start of device surface
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		imul	[GDC.mi.bytesPerLine]; EAX := y * BytesPerLine
		add		edi,eax				; EDI -> framebuffer start address

; Compute zbuffer starting address

		mov		eax,[GDC.zwidth]
		shl		eax,2
		mov		[gzwidth],eax		; Save zbuffer increment
		mov		esi,[GDC.zbuffer]	; ESI -> start of zbuffer
		mov		ebx,[GDC.tr.count]
		mov		eax,[GDC.tr.y]		; EAX := top coordinate
		add		[GDC.tr.y],ebx		; Increment coordinate for next trap
		sub		eax,[GDC.size.top]	; EAX := top zbuffer coordinate
		imul	[gzwidth]			; EAX := y * zbytesPerLine
		add		esi,eax				; ESI -> zbuffer start address
		mov		eax,[GDC.size.left]
		mov		[gzleftAdj],eax		; Save left zbuffer coord adjustment

; Check that xstart < xend, swapping if need be.

		_ALIGN	4

@@NextScan:
		push	esi
		push	edi

		mov		eax,[GDC.tr.x2]		; EAX := X2
		mov		ebx,[GDC.tr.x1]		; EBX := X1
		mov		ebp,eax
		sub     ebp,ebx				; EBP := X2 - X1
		add		ebx,8000h			; Round
		add		eax,8000h			; Round
		shr		ebx,16				; EBX := xstart for scanline
		shr		eax,16				; EAX := xend for scanline
		mov		ecx,[GDC.tr.c1]		; Get scanline color values
		mov     edx,[GDC.tr.c2]
		cmp		ebx,eax				; is xstart > xend?
		jz		@@DoneScan			; ignore if xstart == xend
		jl		@@NoSwap			; no, don't swap them
		xchg	eax,ebx				; swap ix1,ix2
		xchg	ecx,edx				; swap ic1,ic2
		neg		ebp                 ; Negate dx value

@@NoSwap:
		sub		eax,ebx				; EAX := xend - xstart (# pixels to draw)
		add		edi,ebx
		add		edi,ebx
		add		edi,ebx				; EDI -> first pixel to fill
		mov		[gc1],ecx			; Save ic1 color value
		sub		edx,ecx				; EDX := ic2 - ic1
		mov		ecx,eax				; ECX := # pixels to draw
		xor		eax,eax
		shrd	eax,edx,16			; position so that result ends up
		sar		edx,16				; in EAX
		idiv	ebp
		mov		[gcSlope],eax		; Save color slope
		mov		ebp,[gc1]			; EBP := initial fixed point color value
		lea		edx,[ebx*4]
		sub		ebx,[gzleftAdj]		; Adjust left zbuffer coord
		add		esi,ebx
		add		esi,ebx
		add		esi,ebx
		add		esi,ebx				; ESI -> first pixel in zbuffer
		add		edx,[GDC.zOffset]	; EDX -> zOffset[x1]
		mov		ebx,[GDC.tr.z0]		; EBX := initial z value

; Blast out pixels to the framebuffer

		_ALIGN	4

numloops = 8

@@NextPixel:
off = 0
REPT	numloops
LOCAL	@@Skip
		mov		eax,ebx				; Get initial z value
		add		eax,[edx+off*4]		; Interpolate z
		cmp     eax,[esi+off*4]		; Compare z value
		jle		@@Skip				; Z value is smaller
		mov		[esi+off*4],eax		; Store z value
		mov		eax,ebp				; Get fixed point color value
		shr		eax,16				; Convert to integer
		lea		eax,[eax*4]			; Translate color via shade table
		add		eax,[GDC.shadeTab]
		mov		eax,[eax]
		mov		[edi+off*3],ax		; Store color value
		shr		eax,16
		mov		[edi+off*3+2],al	; Store color value
@@Skip:	add		ebp,[gcSlope]		; Interpolate color
		dec		ecx
		jz		@@DoneScan
off = off + 1
ENDM
		add     edx,off*4			; Increment z offset buffer
		add		esi,off*4			; Increment z buffer
		add		edi,off*3			; Increment framebuffer
		jmp		@@NextPixel

		_ALIGN	4

@@DoneScan:
		pop		edi
		pop		esi
		add		edi,[GDC.mi.bytesPerLine]; Increment address to next scanline
		add		esi,[gzwidth]		; Increment zbuffer address
		mov     eax,[GDC.tr.x1]		; Interpolate edges
		mov     ebx,[GDC.tr.x2]
		mov		ecx,[GDC.tr.z0]
		add		eax,[GDC.tr.slope1]	; x1 += slope1
		add		ebx,[GDC.tr.slope2]	; x2 += slope2
		add		ecx,[GDC.tr.dZdY]	; z0 += dCdY
		mov		[GDC.tr.x1],eax
		mov		[GDC.tr.x2],ebx
		mov		[GDC.tr.z0],ecx
		mov		eax,[GDC.tr.cSlope1]
		mov		ebx,[GDC.tr.cSlope2]
		add		[GDC.tr.c1],eax		; c1 += cSlope1
		add		[GDC.tr.c2],ebx		; c2 += cSlope2
		dec		[GDC.tr.count]
		jnz		@@NextScan			; Loop for all scanlines

@@Exit:	pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_PACKED24_czTrap32

endif	; MGL_FIX3D

endcodeseg	_packed24

endif

		END							; End of module
