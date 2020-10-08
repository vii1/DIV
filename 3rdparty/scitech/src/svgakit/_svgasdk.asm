;****************************************************************************
;*
;*			The SuperVGA Kit - UniVBE Software Development Kit
;*
;*                  Copyright (C) 1996 SciTech Software
;*                          All rights reserved.
;*
;* Filename:    $Workfile:   _svgasdk.asm  $
;* Version:    	$Revision:   1.8  $
;*
;* Language:    80386 Assembler
;* Environment: IBM PC Real Mode and 16/32 bit Protected Mode.
;*
;* Description: Assembly language support routines for the UVBELib Software
;*				Development Kit. This code is intended to be used as a guide
;*				for developing software with the UVBELib SuperVGA library.
;*              All the code here assumes that the video memory selector
;*				has been cached in the GS selector before the code is
;*				called.
;*
;* $Date:   04 May 1996 04:11:02  $ $Author:   KendallB  $
;*
;****************************************************************************

		IDEAL

include "model.mac"             ; Memory model macros
include "vbeaf.inc"             ; VBE/AF Accelerator Functions
include "svga.inc"				; SuperVGA Kit structures

header  _svgasdk                ; Set up memory model

CRTC    EQU 3D4h        		; Port of CRTC registers

begdataseg	_svgasdk

DC		EQU		__SV_devCtx		; Macro to access global structure

		$EXTRN	__SV_devCtx,SV_devCtx_s
		$EXTRN  __SV_pagesize,ULONG
		$EXTRN  __SV_bankShift,UINT
		$EXTRN	__SV_setBankRM,DPTR
		$EXTRN	__SV_setBankPtr,DPTR

if pmode
		$EXTRN	__SV_setBank20,DPTR
		$EXTRN	__SV_setCRT20,DPTR
		$EXTRN	__SV_setPal20,DPTR
		$EXTRN	_VBE_MMIOSel,WORD
endif

enddataseg	_svgasdk

begcodeseg	_svgasdk

ifdef	__WINDOWS32__
		EXTRN	_PM_setBankA:FPTR
		EXTRN	_PM_setBankAB:FPTR
		EXTRN	_PM_setCRTStart:FPTR
endif

;----------------------------------------------------------------------------
; SV_pixelAddr16 - Determine address of pixel in 16 color banked modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			AH	-	bit mask
; 				ES:_BX	-	byte offset in buffer
;				CL	-	number of bits to shift left
;				_SI	-	64k bank number of the address
;
; Registers:	none.
;
;----------------------------------------------------------------------------
PROC	SV_pixelAddr16 near

		push	_dx					; Save DX
		mul		[USHORT DC.bytesperline]	; DX:AX := y * BytesPerLine
		mov		cl,bl				; CL := low-order byte of x

		shr		_bx,3				; _BX := x/8
		add		ax,bx
		adc		dl,0				; DL:_BX := y*BytesPerLine + x/8
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		adc		dl,[BYTE DC.bankOffset]; DL := bank number
		add		bx,[USHORT DC.originOffset]	; _BX := byte offset in video buffer
		adc     dl,0
		mov		_si,_dx				; _SI := bank number

		mov		ah,1				; AH := unshifted bit mask
		and		cl,7				; CL := x & 7
		xor		cl,7				; CL := # bits to shift left

		pop		_dx
		ret

ENDP	SV_pixelAddr16

;----------------------------------------------------------------------------
; SV_pixelAddr256 - Determine address of pixel in 256 color banked modes
;----------------------------------------------------------------------------
;
; Entry:		_AX		-	y-coordinate
;				_BX		-	x-coordinate
;
; Exit:			ES:_BX	-	byte offset in buffer
;				_DX		-	64k bank number of the address
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	SV_pixelAddr256	near

		mul		[USHORT DC.bytesperline]		; DX:AX := y * bytesperline
		add		ax,bx
		adc		dx,0				; DX:_BX := y * BytesPerLine + x
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		adc		dl,[BYTE DC.bankOffset]; DL := bank number
		add		bx,[USHORT DC.originOffset]
		adc     dl,0
		ret

ENDP	SV_pixelAddr256

;----------------------------------------------------------------------------
; SV_pixelAddr32k - Determine address of pixel in 32k color banked modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			_BX	-	byte offset in buffer
;				_DX	-	64k bank number of the address
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	SV_pixelAddr32k	near

		mul		[USHORT DC.bytesperline]		; DX:AX := y * bytesperline
		shl		_bx,1
		add		ax,bx
		adc		dx,0				; DX:BX := y * BytesPerLine + x * 2
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		adc		dl,[BYTE DC.bankOffset]; DL := bank number
		add		bx,[USHORT DC.originOffset]
		adc     dl,0
		ret

ENDP	SV_pixelAddr32k

;----------------------------------------------------------------------------
; SV_pixelAddr16m - Determine address of pixel in 16m color banked modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			_BX	-	byte offset in buffer
;				_DX	-	64k bank number of the address
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	SV_pixelAddr16m near

		mul		[USHORT DC.bytesperline]		; DX:AX := y * bytesperline
		add		ax,bx
		shl		_bx,1
		add		ax,bx
		adc		dx,0				; DX:BX := y * BytesPerLine + x * 3
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		adc		dl,[BYTE DC.bankOffset]; DL := bank number
		add		bx,[USHORT DC.originOffset]
		adc     dl,0
		ret

ENDP	SV_pixelAddr16m

;----------------------------------------------------------------------------
; SV_pixelAddr4G - Determine address of pixel in 4G color banked modes
;----------------------------------------------------------------------------
;
; Entry:		_AX	-	y-coordinate
;				_BX	-	x-coordinate
;
; Exit:			_BX	-	byte offset in buffer
;				_DX	-	64k bank number of the address
;
; Registers:	None.
;
;----------------------------------------------------------------------------
PROC	SV_pixelAddr4G	near

		mul		[USHORT DC.bytesperline]		; DX:AX := y * bytesperline
		shl		_bx,2
		add		ax,bx
		adc		dx,0				; DX:BX := y * BytesPerLine + x * 2
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		adc		dl,[BYTE DC.bankOffset]; DL := bank number
		add		bx,[USHORT DC.originOffset]
		adc     dl,0
		ret

ENDP	SV_pixelAddr4G

;----------------------------------------------------------------------------
; clearBanked	- Clear all banks in the video page
;----------------------------------------------------------------------------
; Entry:		EAX	- Value to clear with
;
; Registers:	All.
;----------------------------------------------------------------------------
PROC	clearBanked

		LOCAL	color:ULONG = LocalSize

		enter_c	LocalSize
		mov		[color],eax

		mov     _ax,[DC.maxy]
		inc     _ax
		mul     [USHORT DC.bytesperline]     ; DX:AX := number of bytes to fill
		mov     bx,ax
		add		bx,[USHORT DC.originOffset]	; BX := bytes in last bank to fill
		adc		dl,0
		mov     dh,dl               ; DH := number of full banks to fill

		_les	_di,[DC.videoMem]		; ES:_DI -> start of video memory
		add     di,[USHORT DC.originOffset]
		mov     dl,[BYTE DC.bankOffset]; DL := starting bank number
		cld                         ; Moves go up in memory

		force_es_eq_ds
		or      dh,dh               ; More than one bank to fill?
		jz      @@SingleBank        ; No, only fill a single bank

; Fill the first partial bank

		mov     al,dl
		call    _SV_setBankASM
		mov		eax,[color]
		mov		ecx,0FFFFh
		sub		cx,[USHORT DC.originOffset]
		inc		ecx
		shr		ecx,2
	rep stosd
		_les	_di,[DC.videoMem]		; ES:_DI -> start of video memory
		inc     dl
		dec     dh
		jz		@@SingleBank		; Only two banks to fill

; Fill all of the full 64k banks

@@InnerLoop:
		mov     al,dl
		call    _SV_setBankASM
		mov		eax,[color]
		mov     _cx,4000h           ; Need to set 4000h double USHORTs per bank
		push	_di
	rep stosd
		pop		_di
		inc     dl
		dec     dh
		jnz     @@InnerLoop

; Now fill the last partial bank

@@SingleBank:
		mov     al,dl
		call    _SV_setBankASM
		mov		eax,[color]
		xor     _cx,_cx
		mov     cx,bx
		shr     _cx,2               ; _CX := number of double words to set
	rep stosd

		leave_c
		ret

ENDP	clearBanked

;----------------------------------------------------------------------------
; void clear16(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Works even if the display contains more than
; one bank, so will work for 1024x768 and 1280x1024 video modes.
;----------------------------------------------------------------------------
procstartdll   __SV_clear16

		ARG		color:ULONG

		enter_c	0

; Setup graphics controller

        mov     dx,3CEh             ; DX := Graphics Controller I/O port

		mov     ah,[BYTE color]		; AH := Background color
        xor     al,al               ; AL := 0 (Set/Reset register number)
        out     dx,ax               ; load set/reset register

        mov     ax,0F01h            ; AH := 1111b (mask for Enable set/reset)
                                    ; AL := 1 (enable Set/reset reg number)
        out     dx,ax               ; load enable set/reset reg

		call	clearBanked			; Clear the video memory page

; Restore graphics controller

        mov     dx,3CEh             ; DX := Graphics Controller I/O port
        xor     ax,ax               ; AH := 0, AL := 0
        out     dx,ax               ; Restore default Set/Reset register

        inc     ax                  ; AH := 0, AL := 1
        out     dx,ax               ; Restore enable Set/Reset register

		leave_c_nolocal
		ret

procend     __SV_clear16

;----------------------------------------------------------------------------
; void clear256(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear256

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

		call	clearBanked

		leave_c_nolocal
		ret

procend     __SV_clear256

;----------------------------------------------------------------------------
; void clear32k(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear32k

		ARG		color:ULONG

		enter_c	0

		xor		eax,eax
		mov		ax,[WORD color]
		mov		ebx,eax
		shl		ebx,16
		or		eax,ebx				; EAX = 32 bit color value
		call	clearBanked			; Clear the video memory page

		leave_c_nolocal
		ret

procend     __SV_clear32k

;----------------------------------------------------------------------------
; void clear16m(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear16m

		ARG		color:ULONG

		enter_c	0

		mov		eax,[color]
		or		eax,eax
		jnz		@@SlowClear
		call	clearBanked			; Clear the video memory page to black
		jmp		@@Exit

@@SlowClear:
		_les	_bx,[DC.videoMem]		; ES:_BX -> start of video memory
		add     bx,[USHORT DC.originOffset]
		mov     dl,[BYTE DC.bankOffset]; DL := starting bank number
		mov		ax,dx
		call	_SV_setBankASM				; Change to starting bank number

		mov		_di,[DC.maxx]
		inc		_di					; _DI := number of pixels to draw
		mov		_si,[DC.maxy]
		inc		_si					; _SI := number of lines to process
		mov		ax,[WORD color]		; AX := pixel color
		mov		dh,[BYTE color+2]	; DH := top byte of pixel color
		mov		bp,di
		shl		bp,1
		add		bp,di				; BP := bytes per physical scanline
		sub		bp,[USHORT DC.bytesperline]
		neg		bp					; BP := scanline adjust factor

@@NextScanLine:
		mov		_cx,_di

@@LoopSolid:
		cmp		bx,0FFFDh
		jae		@@BankSwitch		; Bank switch occurs!

		mov		[WORD _ES _bx],ax	; Set pixel value in buffer
		mov		[BYTE _ES _bx+2],dh
		add		_bx,3				; Increment to next pixel
		loop	@@LoopSolid			; Loop across line

@@AfterPlot:
		add		bx,bp
		jc		@@BankSwitch2
		dec		_si
		jnz		@@NextScanLine
		jmp		@@Exit

@@BankSwitch:
		call	DrawPixelAFSLOW16m
		inc		dl
		loop	@@LoopSolid			; Loop across line
		jmp		@@AfterPlot

@@BankSwitch2:
		inc		dl
		push	_ax
		mov		_ax,_dx
		call	_SV_setBankASM
		pop		_ax
		dec		_si
		jnz		@@NextScanLine

@@Exit:
		leave_c_nolocal
		ret

procend     __SV_clear16m

;----------------------------------------------------------------------------
; void clear4G(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear4G

		ARG		color:ULONG

		enter_c	0

		mov		eax,[color]
		call	clearBanked			; Clear the video memory page

		leave_c_nolocal
		ret

procend     __SV_clear4G

;----------------------------------------------------------------------------
; void putPixel16(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel16

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c	0

; Compute the pixel's address in video buffer

		mov     _ax,[y]
		mov     _bx,[x]
		mul     [USHORT DC.bytesperline]     ; DX:AX := y * BytesPerLine

		mov     cl,bl               ; CL := low-order byte of x

		shr     _bx,3               ; _BX := x/8
		add     ax,bx
		adc     dx,0                ; DX:BX := y*BytesPerLine + x/8
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		add     bx,[USHORT DC.originOffset]   ; DX:BX := byte offset in video buffer
		adc     dx,[DC.bankOffset]

		cmp     dl,[BYTE DC.curBank]
		je      @@NoChange

        mov     al,dl
		call    _SV_setBankASM

@@NoChange:
		mov     ah,1                ; AH := unshifted bit mask
        and     cl,7                ; CL := x & 7
        xor     cl,7                ; CL := # bits to shift left

; set Graphics Controller Bit Mask register

        shl     ah,cl               ; AH := bit mask in proper postion
        mov     dx,3CEh             ; GC address register port
        mov     al,8                ; AL := Bit Mask Register number
        out     dx,ax

; set Graphics Controller Mode register

        mov     ax,0205h            ; AL := Mode register number
                                    ; AH := Write mode 2 (bits 0,1)
                                    ;   Read mode 0 (bit 3)
        out     dx,ax

; set data rotate/Function Select register

        mov     ax,3                ; AL := Data Rotate/Func select reg #
        out     dx,ax

; set the pixel value

		mov     al,[_ES _bx]       	; latch one byte from each bit plane
		mov     al,[BYTE color]     ; AL := pixel value
		mov     [_ES _bx],al   		; update all bit planes

; restore default Graphics Controller registers

        mov     ax,0FF08h           ; default bit mask
        out     dx,ax

		mov     ax,0005h            ; default mode register
		out     dx,ax

		mov     ax,0003h            ; default function select
        out     dx,ax

        leave_c_nolocal
		ret

procend     __SV_putPixel16

;----------------------------------------------------------------------------
; void putPixel256(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel256

		ARG     x:UINT, y:UINT, color:ULONG

        enter_c 0

		mov     _ax,[y]
		mul     [USHORT DC.bytesperline]
		mov     _bx,[x]
		add		ax,bx
		adc		dl,[BYTE DC.bankOffset]; DL:_BX := y * BytesPerLine + x
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		add     bx,[USHORT DC.originOffset]
		adc		dl,0
		cmp     dl,[BYTE DC.curBank]
        je      @@NoChange

        mov     al,dl
		call    _SV_setBankASM

@@NoChange:
		mov     al,[BYTE color]
		mov     [_ES _bx],al        ; Replace the pixel

        leave_c_nolocal
		ret

procend     __SV_putPixel256

;----------------------------------------------------------------------------
; void putPixel32k(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel32k

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c	0

		mov     _ax,[y]
		mul     [USHORT DC.bytesperline]
		mov     _bx,[x]
		shl     _bx,1
		add		ax,bx
		adc		dl,[BYTE DC.bankOffset]; DL:_BX := y * BytesPerLine + x
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		add     bx,[USHORT DC.originOffset]
		adc		dl,0
		cmp     dl,[BYTE DC.curBank]
		je      @@NoChange

        mov     al,dl
		call    _SV_setBankASM

@@NoChange:
		mov     ax,[USHORT color]
		mov     [_ES _bx],ax        ; Replace the pixel

        leave_c_nolocal
		ret

procend     __SV_putPixel32k

;----------------------------------------------------------------------------
; void putPixel16m(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel16m

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c	0

		mov     _ax,[y]
		mul     [USHORT DC.bytesperline]
		mov     _bx,[x]
        add     ax,bx
        adc     dx,0
        shl     bx,1
        add     ax,bx
		adc     dl,[BYTE DC.bankOffset]; DX:AX := y * BytesPerLine + x * 3
		add     ax,[USHORT DC.originOffset]
		adc		dl,0
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax               ; BX := Offset in buffer
		cmp     dl,[BYTE DC.curBank]
        je      @@NoChange

        mov     al,dl
		call    _SV_setBankASM

@@NoChange:
		mov     ax,[WORD color]
		mov		dh,[BYTE color+2]
		cmp		bx,0FFFEh
		jae		@@SlowVersion		; Bank switch occurs in pixel!

		mov     [_ES _bx],ax        ; Replace the first byte
		mov		[_ES _bx+2],dh

@@Exit: leave_c_nolocal
		ret

@@SlowVersion:
		call	DrawPixelAFSLOW16m	; Draw the pixel slowly
		jmp		@@Exit

procend     __SV_putPixel16m

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
		xchg	al,dl
		call	_SV_setBankASM
		xchg	al,dl
		ret

ENDP	IncBXDL

;----------------------------------------------------------------------------
; DrawPixelAFSLOW16m	Draws a pixel split across a bank boundary correctly
;----------------------------------------------------------------------------
;
; Draws the pixel taking into account that a bank boundary occurs in the
; middle of the pixel.
;
; Entry:		DH:AX		- Color of the pixel to plot
;				GS:_BX		- Address of pixel to plot
;				DC.curBank	- Current bank number
;
; Exit:			DH:AX		- Color of pixel to plot
;				GS:_BX		- Address of pixel to plot + 3
;				DC.curBank	- Current bank number + 1
;
; Registers:	All preserved.
;
;----------------------------------------------------------------------------
PROC	DrawPixelAFSLOW16m	near

		push	_dx

		mov		dl,[BYTE DC.curBank]
		mov		[_ES _bx],al		; Replace the pixel
		call    IncBXDL
		mov		[_ES _bx],ah
		call	IncBXDL
		mov		[_ES _bx],dh
		call    IncBXDL
		pop		_dx
		ret

ENDP	DrawPixelAFSLOW16m

;----------------------------------------------------------------------------
; void putPixel4G(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel4G

		ARG     x:UINT, y:UINT, color:ULONG

		enter_c	0

		mov     _ax,[y]
		mul     [USHORT DC.bytesperline]
		mov     _bx,[x]
		shl     _bx,2
		add		ax,bx
		adc		dl,[BYTE DC.bankOffset]; DL:_BX := y * BytesPerLine + x
		_les	_bx,[DC.videoMem]		; ES:_BX -> video memory
		add		bx,ax
		add     bx,[USHORT DC.originOffset]
		adc		dl,0
		cmp     dl,[BYTE DC.curBank]
		je      @@NoChange

        mov     al,dl
		call    _SV_setBankASM

@@NoChange:
		mov     eax,[color]
		mov     [_ES _BX],eax       ; Replace the pixel

        leave_c_nolocal
		ret

procend     __SV_putPixel4G

;----------------------------------------------------------------------------
; void _SV_line16(int x1,int y1,int x2,int y2, long color)
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
procstartdll	__SV_line16

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	Routine:NCPTR, VertInc:ULONG, EIncr:UINT, 			\
				NEIncr:UINT = LocalSize

		enter_c	LocalSize
		cld

; Configure the graphics controller (write mode 3)

		mov		dx,3CEh				; DX := Graphics Controller port addr

		mov		ax,0B05h			; AL := Mode register number
									; AH := Write mode 3 (bits 0,1)
									;	Read mode 1 (bit 3)
		out		dx,ax

		xor		ah,ah				; AH := replace mode
		mov		al,3				; AL := Data Rotate/Func select reg #
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

; Load the current color

		mov		dx,3CEh				; DX := Graphics Controller port addr
		mov		ah,[BYTE color]		; Get color value into ah
		xor		al,al				; AL := Set/Reset Register number
		out		dx,ax

		mov		si,[USHORT DC.bytesperline]	; Increment for video buffer
		mov		[USHORT VertInc+2],0	; Zero out sign for vertical increment
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
		neg		si					; negative increment for buffer
		mov		[USHORT VertInc+2],0FFFFh	; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[USHORT VertInc],si	; save increment
		mov		[Routine],offset @@LoSlopeLine
		cmp		_bx,_ax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[Routine],offset @@HiSlopeLine
		xchg	_bx,_ax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		_bx,1				; _BX := 2 * dy
		mov		[EIncr],_bx			; EIncr := 2 * dy
		sub		_bx,_ax				; d = 2 * dy - dx
		mov		_di,_bx				; DI := initial decision variable
		sub		_bx,_ax
		mov		[NEIncr],_bx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	_ax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	SV_pixelAddr16		; AH := Bit mask
									; ES:_BX -> buffer
									; CL := # bits to shift left
									; _SI := bank number
		shl		ah,cl				; AH := bit mask in proper position
		mov		al,ah				; AL := bit Mask
		pop		_cx					; Restore dx
		inc		_cx					; _CX := # pixels to draw

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:_BX -> video buffer
;						AL = bit mask for 1st pixel
;						AH = bit mask for 1st pixel
;						_CX = # pixels to draw
;						DX = Graphics Controller data register port addr
;						_SI = bank number
;						_DI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
; The Graphics Controller index register should be set to point to the
; Bit Mask register.
;
;****************************************************************************

@@LoSlopeLine:

@@StartLo:
		mov		al,ah				; AL := bit mask for next pixel
		cmp		si,[USHORT DC.curBank]
		je		@@BitMaskIn

		push	_ax					; Preserve AX
		mov		_ax,_si				; AX := new bank number
		call	_SV_setBankASM				; Program this bank
		pop		_ax

@@BitMaskIn:
		or		al,ah				; mask current pixel position

		ror		ah,1				; Rotate pixel value
		jc		@@BitMaskOut		; Jump if mask rotated to leftmost pixel

; bit mask not shifted out

		or		_di,_di				; test sign of d
		jns		@@InPosDi			; jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@BitMaskIn

		and		[_ES _bx],al		; set remaining pixel(s)
		jmp		@@Exit

@@InPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr

		and		[_ES _bx],al		; Update bit planes

		add		bx,[USHORT VertInc]	; increment y
		adc		si,[USHORT VertInc+2]	; Adjust bank number
		loop	@@StartLo
		jmp		@@Exit

; bit mask shifted out

@@BitMaskOut:
		and		[_ES _bx],al		; Update bit planes
		add		bx,1				; increment x
		adc		si,0				; Adjust bank value

		or		_di,_di				; test sign of d
		jns		@@OutPosDi			; jump if non-negative

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@StartLo
		jmp		@@Exit

@@OutPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,[USHORT VertInc]	; vertical increment
		adc		si,[USHORT VertInc+2]	; Adjust bank number
		loop	@@StartLo
		jmp		@@Exit

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:_BX -> video buffer
;						AL = bit mask for 1st pixel
;						_CX = # pixels to draw
;						DX = Graphics Controller data register port addr
;						_SI = bank number
;						_DI = Initial decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
; The Graphics Controller index register should be set to point to the
; Bit Mask register.
;
;****************************************************************************

@@HiSlopeLine:

@@StartHi:
		cmp		si,[USHORT DC.curBank]
		je		@@SetHi

		push	_ax					; Preserve AX
		mov		_ax,_si				; AX := new bank number
		call	_SV_setBankASM				; Program this bank
		pop		_ax

@@SetHi:
		and		[_ES _bx],al		; update bit planes

		add		bx,[USHORT VertInc]	; increment y
		adc		si,[USHORT VertInc+2]	; Adjust bank number

		or		_di,_di				; test sign of d
		jns		@@HiPosDi			; jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@StartHi
		jmp		@@Exit

@@HiPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr

		ror		al,1				; rotate bit mask
		adc		bx,0				; Increment BX when mask rotated to
									;  leftmost pixel position
		adc		si,0				; Adjust bank number
		loop	@@StartHi

; Restore graphics controller and return to caller

@@Exit:
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

		leave_c
		ret

procend		__SV_line16

;----------------------------------------------------------------------------
; void _SV_line256(int x1,int y1,int x2,int y2, long color)
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
procstartdll	__SV_line256

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	Routine:NCPTR, VertInc:ULONG, EIncr:UINT, 			\
				NEIncr:UINT = LocalSize

		enter_c	LocalSize

		mov		si,[USHORT DC.bytesperline]	; Increment for video buffer
		mov		[USHORT VertInc+2],0	; Zero out sign for vertical increment

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
		neg		si					; negative increment for buffer
		mov		[USHORT VertInc+2],0FFFFh	; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[USHORT VertInc],si	; save increment
		mov		[Routine],offset @@LoSlopeLine
		cmp		_bx,_ax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[Routine],offset @@HiSlopeLine
		xchg	_bx,_ax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		_bx,1				; _BX := 2 * dy
		mov		[EIncr],_bx			; EIncr := 2 * dy
		sub		_bx,_ax				; d = 2 * dy - dx
		mov		_di,_bx				; _DI := initial decision variable
		sub		_bx,_ax
		mov		[NEIncr],_bx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	_ax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	SV_pixelAddr256		; ES:_BX -> buffer

		pop		_cx					; Restore dx
		inc		_cx					; CX := # pixels to draw

		jmp     [Routine]           ; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		al,[BYTE color]		; AL := pixel value to fill
		mov		dh,[BYTE DC.curBank]	; DH := current bank number

@@LoopLo:
		cmp		dl,dh
		je		@@SetLo

		mov		dh,al				; DH := color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM		; Program this bank
		mov		al,dh				; AL := color value
		mov		dh,dl				; DH := current bank number

@@SetLo:
		mov		[_ES _bx],al		; Set pixel value in buffer
		add		bx,1				; Increment x coordinate
		adc		dl,0				; Adjust bank number
		or		_di,_di				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; adjust page number
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		al,[BYTE color]		; AL := pixel value to fill
		mov		dh,[BYTE DC.curBank]	; DH := current bank number

@@LoopHi:
		cmp		dh,dl
		je		@@SetHi

		mov		dh,al				; DH := color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM				; Program this bank
		mov		al,dh				; AL := color value
		mov		dh,dl				; DH := current bank number

@@SetHi:
		mov		[_ES _bx],al		; Set pixel value in buffer
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; Adjust bank number
		or		_di,_di				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,1				; Increment x
		adc		dl,0				; Adjust bank number
		loop	@@LoopHi			; Loop for remaining pixels

@@Exit:
		leave_c
		ret

procend		__SV_line256

;----------------------------------------------------------------------------
; void _SV_line32k(int x1,int y1,int x2,int y2, long color)
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
procstartdll	__SV_line32k

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	Routine:NCPTR, VertInc:ULONG, EIncr:UINT, 			\
				NEIncr:UINT = LocalSize

		enter_c	LocalSize
		cld

		mov		si,[USHORT DC.bytesperline]	; Increment for video buffer
		mov		[USHORT VertInc+2],0	; Zero out sign for vertical increment

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
		neg		si					; negative increment for buffer
		mov		[USHORT VertInc+2],0FFFFh	; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[USHORT VertInc],si	; save increment
		mov		[Routine],offset @@LoSlopeLine
		cmp		_bx,_ax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[Routine],offset @@HiSlopeLine
		xchg	_bx,_ax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		_bx,1				; _BX := 2 * dy
		mov		[EIncr],_bx			; EIncr := 2 * dy
		sub		_bx,_ax				; d = 2 * dy - dx
		mov		_di,_bx				; _DI := initial decision variable
		sub		_bx,_ax
		mov		[NEIncr],_bx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	_ax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	SV_pixelAddr32k		; ES:_BX -> buffer

		pop		_cx					; Restore dx
		inc		_cx					; _CX := # pixels to draw

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		ax,[USHORT color]		; AX := pixel value to fill
		mov		dh,[BYTE DC.curBank]	; DH := current bank number

@@LoopLo:
		cmp		dl,dh
		je		@@SetLo

		push	_ax					; Save color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM				; Program this bank
		mov		dh,dl				; DH := current bank number
		pop		_ax					; Restore color value

@@SetLo:
		mov		[_ES _bx],ax		; Set pixel value in buffer
		add		bx,2				; Increment x coordinate
		adc		dl,0				; Adjust bank number
		or		_di,_di				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; adjust page number
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		ax,[USHORT color]		; AL := pixel value to fill
		mov		dh,[BYTE DC.curBank]	; DH := current bank number

@@LoopHi:
		cmp		dh,dl
		je		@@SetHi

		push	_ax					; Save color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM				; Program this bank
		mov		dh,dl				; DH := current bank number
		pop		_ax					; Restore color value

@@SetHi:
		mov		[_ES _bx],ax		; Set pixel value in buffer
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; Adjust bank number
		or		_di,_di				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,2				; Increment x
		adc		dl,0				; Adjust bank number
		loop	@@LoopHi			; Loop for remaining pixels

@@Exit:
		leave_c
		ret

procend		__SV_line32k

;----------------------------------------------------------------------------
; void _SV_line16m(int x1,int y1,int x2,int y2, long color)
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
procstartdll	__SV_line16m

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	Routine:NCPTR, VertInc:ULONG, EIncr:UINT, 			\
				NEIncr:UINT = LocalSize

		enter_c	LocalSize
		cld

		mov		si,[USHORT DC.bytesperline]	; Increment for video buffer
		mov		[USHORT VertInc+2],0	; Zero out sign for vertical increment

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
		neg		si					; negative increment for buffer
		mov		[USHORT VertInc+2],0FFFFh	; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[USHORT VertInc],si	; save increment
		mov		[Routine],offset @@LoSlopeLine
		cmp		_bx,_ax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[Routine],offset @@HiSlopeLine
		xchg	_bx,_ax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		_bx,1				; _BX := 2 * dy
		mov		[EIncr],_bx			; EIncr := 2 * dy
		sub		_bx,_ax				; d = 2 * dy - dx
		mov		_di,_bx				; _DI := initial decision variable
		sub		_bx,_ax
		mov		[NEIncr],_bx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	_ax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	SV_pixelAddr16m		; ES:_BX -> buffer

		pop		_cx					; Restore dx
		inc		_cx					; CX := # pixels to draw

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		ax,[USHORT color]		; AX := pixel value to fill
		mov		dh,[BYTE color+2]	; DH := top byte of pixel value

@@LoopLo:
		cmp		dl,[BYTE DC.curBank]
		je		@@SetPixelLo

		push	_ax					; Save color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM				; Program this bank
		pop		_ax					; Restore color value

@@SetPixelLo:
		cmp		bx,0FFFEh
		jae		@@BankSwitchLo

@@SetLo:
		mov		[_ES _bx],ax		; Set pixel value in buffer
		mov		[_ES _bx+2],dh
		add		bx,3				; Increment x coordinate
		adc		dl,0				; Adjust bank number

@@DonePixelLo:
		or		_di,_di				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; adjust page number
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@BankSwitchLo:
		call	DrawPixelAFSLOW16m
		inc		dl
		jmp     @@DonePixelLo

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		ax,[USHORT color]		; AL := pixel value to fill
		mov		dh,[BYTE color+2]	; DH := current bank number

@@LoopHi:
		cmp		dl,[BYTE DC.curBank]
		je		@@SetPixelHi

		push	_ax					; Save color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM				; Program this bank
		pop		_ax					; Restore color value

@@SetPixelHi:
		cmp		bx,0FFFEh
		jae		@@BankSwitchHi

@@SetHi:
		mov		[_ES _bx],ax		; Set pixel value in buffer
		mov		[_ES _bx+2],dh

@@DonePixelHi:
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; Adjust bank number
		or		_di,_di				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,3				; Increment x
		adc		dl,0				; Adjust bank number
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit

@@BankSwitchHi:
		call	DrawPixelAFSLOW16m
		sub		bx,3
		jmp     @@DonePixelHi

@@Exit:
		leave_c
		ret

procend		__SV_line16m

;----------------------------------------------------------------------------
; void _SV_line4G(int x1,int y1,int x2,int y2, long color)
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
procstartdll	__SV_line4G

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
		LOCAL	Routine:NCPTR, VertInc:ULONG, EIncr:UINT, 			\
				NEIncr:UINT = LocalSize

		enter_c	LocalSize
		push	eax
		cld

		mov		si,[USHORT DC.bytesperline]	; Increment for video buffer
		mov		[USHORT VertInc+2],0	; Zero out sign for vertical increment

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
		neg		si					; negative increment for buffer
		mov		[USHORT VertInc+2],0FFFFh	; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
		mov		[USHORT VertInc],si	; save increment
		mov		[Routine],offset @@LoSlopeLine
		cmp		_bx,_ax
		jle		@@LoSlope			; Jump if dy <= dx (Slope <= 1)
		mov		[Routine],offset @@HiSlopeLine
		xchg	_bx,_ax				; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
		shl		_bx,1				; _BX := 2 * dy
		mov		[EIncr],_bx			; EIncr := 2 * dy
		sub		_bx,_ax				; d = 2 * dy - dx
		mov		_di,_bx				; _DI := initial decision variable
		sub		_bx,_ax
		mov		[NEIncr],_bx		; NEIncr := 2 * (dy - dx)

; calculate first pixel address

		push	_ax					; preserve dx
		mov		_ax,[y1]
		mov		_bx,[x1]
		call	SV_pixelAddr4G		; ES:_BX -> buffer

		pop		_cx					; Restore dx
		inc		_cx					; _CX := # pixels to draw

		jmp		[Routine]			; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
		mov		eax,[color]			; EAX := pixel value to fill
		mov		dh,[BYTE DC.curBank]	; DH := current bank number

@@LoopLo:
		cmp		dl,dh
		je		@@SetLo

		push	_ax					; Save color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM				; Program this bank
		mov		dh,dl				; DH := current bank number
		pop		_ax					; Restore color value

@@SetLo:
		mov		[_ES _bx],eax		; Set pixel value in buffer
		add		bx,4				; Increment x coordinate
		adc		dl,0				; Adjust bank number
		or		_di,_di				; Test sign of d
		jns		@@LoPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@LoPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; adjust page number
		loop	@@LoopLo			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;						ES:_BX -> video buffer
;						_CX = # pixels to draw
;						_DX = Bank number for first pixel
;						_DI = decision variable
;						EIncr	- East pixel increment
;						NEIncr	- North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
		mov		eax,[color]			; EAX := pixel value to fill
		mov		dh,[BYTE DC.curBank]	; DH := current bank number

@@LoopHi:
		cmp		dh,dl
		je		@@SetHi

		push	_ax					; Save color value
		mov		al,dl				; AL := new bank number
		call	_SV_setBankASM				; Program this bank
		mov		dh,dl				; DH := current bank number
		pop		_ax					; Restore color value

@@SetHi:
		mov		[_ES _bx],eax		; Set pixel value in buffer
		add		bx,[USHORT VertInc]	; increment y
		adc		dl,[BYTE VertInc+2]	; Adjust bank number
		or		_di,_di				; Test sign of d
		jns		@@HiPosDi			; Jump if d >= 0

		add		_di,[EIncr]			; d := d + EIncr
		loop	@@LoopHi			; Loop for remaining pixels
		jmp		@@Exit				; We are all done

@@HiPosDi:
		add		_di,[NEIncr]		; d := d + NEIncr
		add		bx,4				; Increment x
		adc		dl,0				; Adjust bank number
		loop	@@LoopHi			; Loop for remaining pixels

@@Exit:
		pop		eax
		leave_c
		ret

procend		__SV_line4G

;----------------------------------------------------------------------------
; void _SV_setActivePage(int which)
;----------------------------------------------------------------------------
; Routine to set the video page for active output.
;
; Entry:        page    - Page number of page to use
;
;----------------------------------------------------------------------------
procstartdll   __SV_setActivePage

		ARG     which:UINT

		enter_c	0

; Calculate 18 bit address of page in video memory

		xor     eax,eax
		mov     _ax,[which]         ; EAX := page number
		mul     [__SV_pagesize]         ; EDX:EAX := result
		mov     [USHORT DC.originOffset],ax  ; Save video buffer offset
		shr     eax,16
		mov     [DC.bankOffset],ax    ; Save video bank offset
		call	_SV_setBankASM

		leave_c_nolocal
		ret

procend     __SV_setActivePage

;----------------------------------------------------------------------------
; void _SV_setVisualPage(int which,bool waitVRT)
;----------------------------------------------------------------------------
; Routine to set the visible video page.
;
; Entry:        page    - Page number of page to use
;
;----------------------------------------------------------------------------
procstartdll       __SV_setVisualPage

		ARG     which:UINT, waitVRT:S_BOOL

		enter_c	0
		push	es

		cmp		[DC.maxpage],0		; No flipping if only one page
		je		@@Exit

; Calculate 18 bit address of page in video memory

		xor     eax,eax
		mov     _ax,[which]         ; EAX := page number
		mul     [__SV_pagesize]         ; EAX := starting address in memory

if pmode
		cmp		[__SV_setCRT20],0
		je		@@NoSetCRT20

; Directly call the 32 bit protected mode CRTC setting routine. The
; interface to the 32 bit routine is different, as it takes the CRTC
; starting address in DX:CX in the same format used for standard VGA modes.

		cmp		[DC.bitsperpixel],4
		je		@@1
		shr		eax,2				; Adj to plane boundary for 8 bit+ modes
@@1:	mov     cx,ax
		shr		eax,16
		mov		dx,ax				; DX:CX := CRTC start address
		xor     _bx,_bx             ; BX := 0 - set display start
		cmp		[waitVRT],0
		je		@@2
		mov		_bx,80h				; BX := 80h - set display start with VRT
@@2:	mov		es,[_VBE_MMIOSel]	; Load selector to MMIO registers
		call	[__SV_setCRT20]		; Call the relocated code
		jmp		@@Exit
endif

@@NoSetCRT20:
		mov     edx,eax
		shr     edx,16              ; DX:AX := starting address in memory

		div     [USHORT DC.bytesperline]    ; AX := starting scanline,
									; DX := starting byte
		mov     cx,dx
		cmp     [DC.bitsperpixel],4
		je      @@16Color
		cmp     [DC.bitsperpixel],8
		je      @@SetIt
		cmp     [DC.bitsperpixel],24
		je      @@24BitColor
		cmp		[DC.bitsperpixel],32
		je		@@32BitColor

		shr     cx,1                ; CX := starting pixel in buffer
		jmp     @@SetIt

@@16Color:
		shl     cx,3                ; CX := starting pixel in buffer
		jmp     @@SetIt

@@24BitColor:
		mov		bx,ax				; Preserve AX
		xor     dx,dx
		mov     ax,cx
		mov     cx,3
		div     cx
		mov     cx,ax               ; CX := starting pixel in buffer
		mov		ax,bx				; Restore AX
		jmp		@@Setit

@@32BitColor:
		shr     cx,2                ; CX := starting pixel in buffer

@@SetIt:
		mov     bx,ax               ; BX := starting scanline in buffer

		mov     _ax,04F07h
		mov     _dx,_bx             ; DX := starting scanline number
		xor     _bx,_bx             ; BX := 0 - set display start
		cmp		[DC.VBEVersion],200h
		jl		@@NotVBE20
		cmp		[waitVRT],0
		je		@@3
		mov		_bx,80h				; BX := 80h - set display start with VRT
@@3:
ifdef	__WINDOWS32__
		push	edx
		push	ecx
		call	_PM_setCRTStart
        add		esp,8
else
		int		10h					; Set the start address
endif
		jmp		@@Exit

; For Non VBE 2.0 implementations we cannot guarantee that the
; implementation will wait for a retrace when setting the display start
; address, so we need to wait for the retrace here just to be sure. Note
; that this _assumes_ the controller is VGA compatible, so will hang in
; an infinite loop on controllers that are not VGA compatible.

@@NotVBE20:
		push	_ax
		push	_dx
		cmp		[waitVRT],0
		je		@@NoWaitDE
		mov     dx,03DAh            ; DX := video status port
@@WaitDE:
		in      al,dx
		test    al,1
		jnz     @@WaitDE	       	; Wait for Display Enable
@@NoWaitDE:
		mov     dx,03DAh            ; DX := video status port
		pop		_dx
		pop		_ax

ifdef	__WINDOWS32__
		push	edx
		push	ecx
		call	_PM_setCRTStart
        add		esp,8
else
		int		10h					; Set the start address
endif

		cmp		[waitVRT],0
		je		@@Exit
		mov     dx,03DAh            ; DX := video status port
@@WaitV:
		in      al,dx               ; Wait for start of vertical retrace
		test    al,8
		jz      @@WaitV

@@Exit:	pop		es
		leave_c_nolocal
		ret

procend     __SV_setVisualPage

;----------------------------------------------------------------------------
; void _VGA_setPalette(int start,int num,VBE_palette *pal,bool waitVRT);
;----------------------------------------------------------------------------
; Programs the VGA palette. To avoid the onset of snow, we program values
; only during a vertical retrace interval if BL bit 7 is set. This is a
; compatability routine for VBE 1.2 implemenations, and we will use the
; normal VBE 2.0 routines otherwise.
;----------------------------------------------------------------------------
procstartdll	__VGA_setPalette

		ARG		start:UINT, num:UINT, pal:DPTR, waitVRT:S_BOOL

		enter_c	0
		push	ds

; Wait for the start of the vertical retrace if specified

		cmp		[waitVRT],0
		je		@@NoWaitVRT

		mov     dx,3DAh				; DX := Input Status register
@@WaitNotVsync:
		in		al,dx
		test	al,8
		jnz		@@WaitNotVsync
@@WaitVsync:
		in		al,dx
		test	al,8
		jz		@@WaitVsync

@@NoWaitVRT:
		_lds	_di,[pal]			; DS:_DI -> palette to program
		mov		_cx,[num]			; _CX := registers to program
		mov		_ax,[start]			; AX := first color register to program
		mov		dx,3C8h				; DX := DAC Write Index register
		out		dx,al				; Index the first color register
		inc		_dx					; DX := DAC data register
		cld

@@NextEntry:
		mov		al,[_di+2]
		out		dx,al
		mov		al,[_di+1]
		out		dx,al
		mov		al,[_di+0]
		out		dx,al
		add		_di,4
		loop	@@NextEntry

@@Exit:	pop		ds
		leave_c_nolocal
		ret

procend		__VGA_setPalette

;----------------------------------------------------------------------------
; _VBE_setBankA	Sets the read/write bank directly via window A
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by making a single call to the VBE to set Window A since it has
; both read & write attributes set.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;
;----------------------------------------------------------------------------
procstartdll	__VBE_setBankA

		push	_ax
		push	_bx
		push	_cx
		mov     _ax,04F05h
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		cmp		[__SV_setBankRM],0
		je		@@UseInt10h
		call	[__SV_setBankRM]		; Call the VBE directly in real mode
		jmp		@@Exit

@@UseInt10h:
ifdef	__WINDOWS32__
		push	esi
		push	edi
		push	edx
		call	_PM_setBankA		; Special routine to thunk to 16 bit code
        pop		edx
		pop		edi
        pop		esi
else
		int     10h
endif

@@Exit:	pop		_cx
		pop		_bx
		pop		_ax
		ret

procend		__VBE_setBankA

;----------------------------------------------------------------------------
; _VBE_setBankAB	Sets the read/write bank directly via Window's A & B
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by calling the VBE twice to set both Window A and Window B, as
; the VBE reports that they have separate read/write attributes.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;
;----------------------------------------------------------------------------
procstartdll	__VBE_setBankAB

		push	_ax
		push	_bx
		push	_cx
		mov     _ax,04F05h
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		push	_dx					; Save bank value
		cmp		[__SV_setBankRM],0
		je		@@UseInt10h
		call	[__SV_setBankRM]		; Call the VBE directly in real mode
		pop		_dx
		mov     _ax,04F05h
		mov		_bx,1				; BX := select window B
		call	[__SV_setBankRM]
		jmp		@@Exit

@@UseInt10h:
ifdef	__WINDOWS32__
		pop		_dx
		push	esi
        push	edi
		push	edx
		call	_PM_setBankAB		; Special routine to thunk to 16 bit code
        pop		edx
		pop		edi
        pop		esi
else
		int		10h
		pop		_dx
		mov     _ax,04F05h
		mov		_bx,1				; BX := select window B
		int		10h
endif

@@Exit:	pop		_cx
		pop		_bx
		pop		_ax
		ret

procend		__VBE_setBankAB

if pmode

;----------------------------------------------------------------------------
; _VBE20_setBankA	Sets the read/write bank directly via window A
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by making a single call to the VBE to set Window A since it has
; both read & write attributes set.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstartdll	__VBE20_setBankA

		push	_bx
		push	_cx
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		call	[__SV_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
		ret

procend		__VBE20_setBankA

;----------------------------------------------------------------------------
; _VBE20_setBankAB	Sets the read/write bank directly via Window's A & B
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by calling the VBE twice to set both Window A and Window B, as
; the VBE reports that they have separate read/write attributes.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstartdll	__VBE20_setBankAB

		push	_bx
		push	_cx
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		push	_dx					; Save bank value
		call	[__SV_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_dx
		mov		_bx,1				; BX := select window B
		call	[__SV_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
		ret

procend		__VBE20_setBankAB

;----------------------------------------------------------------------------
; _VBE20_setBankA_ES	Sets the read/write bank directly via window A
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by making a single call to the VBE to set Window A since it has
; both read & write attributes set. This routine is different in the
; respect that the bank switching code uses MMIO registers and requires the
; ES register to be loaded with the selector to its MMIO registers.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstartdll	__VBE20_setBankA_ES

		push	es
		push	_bx
		push	_cx
		mov     es,[_VBE_MMIOSel]	; Load selector to MMIO registers
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		call	[__SV_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
		pop		es
		ret

procend		__VBE20_setBankA_ES

;----------------------------------------------------------------------------
; _VBE20_setBankAB_ES	Sets the read/write bank directly via Window's A & B
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by calling the VBE twice to set both Window A and Window B, as
; the VBE reports that they have separate read/write attributes. This
; routine is different in the respect that the bank switching code uses
; MMIO registers and requires the ES register to be loaded with the selector
; to its MMIO registers.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstartdll	__VBE20_setBankAB_ES

		push	es
		push	_bx
		push	_cx
		mov     es,[_VBE_MMIOSel]	; Load selector to MMIO registers
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		push	_dx					; Save bank value
		call	[__SV_setBank20]	; Call VBE 2.0 protected mode routine
		pop		_dx
		mov		_bx,1				; BX := select window B
		call	[__SV_setBank20]	; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
		pop		es
		ret

procend		__VBE20_setBankAB_ES

;----------------------------------------------------------------------------
; _VBEAF_setBank	Sets the read/write bank directly via VBE/AF
;----------------------------------------------------------------------------
; This function sets the read/write bank by calling the VBE/AF device driver
; bank switching function.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstartdll	__VBEAF_setBank

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.setBank]
		pop		ebx
		ret

procend		__VBEAF_setBank

;----------------------------------------------------------------------------
; void _VBE20_setPalette(int start,int num,VBE_palette *pal,bool waitFlag)
;----------------------------------------------------------------------------
; Sets the palette by directly calling the relocated VBE 2.0 palette
; setting routine in 32 bit protected mode.
;----------------------------------------------------------------------------
procstartdll	__VBE20_setPalette

		ARG		start:UINT, num:UINT, pal:DPTR, waitFlag:S_USHORT

		enter_c	0

		mov     bx,[waitFlag]		; BL := wait for retrace flag
		mov     _cx,[num]			; _CX := number of entries
		mov		_dx,[start]			; _DX := starting palette entry
if flatmodel
		mov		edi,[pal]			; ES:EDI -> palette data
		mov		eax,[__SV_setPal20]
		push	ds
		mov		ds,[_VBE_MMIOSel]	; Load selector to MMIO registers
		call	eax					; Call the VBE 2.0 palette code
		pop		ds
else
		les		di,[pal]
		movzx	edi,di				; ES:EDI -> palette data
		push	ds					; Save DS
		push	fs					; Save FS
		push	ds
		pop		fs					; FS = DS
		mov		ds,[_VBE_MMIOSel]	; Load selector to MMIO registers
		call	[fs:__SV_setPal20]	; Call the VBE 2.0 palette code
		pop		fs
		pop		ds
endif

		leave_c_nolocal
		ret

procend		__VBE20_setPalette

endif

;----------------------------------------------------------------------------
; _SV_setBankASM	Sets the read/write bank directly via the VBE
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value.
;
; Entry:		AL	- New read/write bank number
;
; Exit:			AL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstartdll	_SV_setBankASM

		push	dx
ife flatmodel
		push	ds
		mov		dx,DGROUP			; Re-load out data segment
		mov		ds,dx
endif
		mov		dx,ax				; DX := bank number
		mov		[WORD DC.curBank],ax	; Save current write bank number
		call	[__SV_setBankPtr]		; Call card specific bank routine
ife flatmodel
		pop		ds
endif
		pop		dx
		ret

procend			_SV_setBankASM

ifdef	__WINDOWS16__
procstart	SV_SETBANKASM
		jmp		_SV_setBankASM
procend		SV_SETBANKASM
endif

;----------------------------------------------------------------------------
; void SV_setBank(int bank)
;----------------------------------------------------------------------------
; Calls the current bank switching routine to change video banks.
;----------------------------------------------------------------------------
procstartdll16	_SV_setBank

		ARG		bank:UINT

		push	_bp
		mov		_bp,_sp
        mov     _ax,[bank]          ; AX := 64k bank value
		call	_SV_setBankASM
		pop		_bp
		ret

procenddll16	_SV_setBank

ifndef	__WINDOWS16__

procstartdll	_SV_beginPixel
		jmp		[DC.beginPixel]
procend			_SV_beginPixel

procstartdll	_SV_putPixel
		jmp		[DC.putPixel]
procend			_SV_putPixel

procstartdll	_SV_putPixelFast
		jmp		[DC.putPixelFast]
procend			_SV_putPixelFast

procstartdll	_SV_endPixel
		jmp		[DC.endPixel]
procend			_SV_endPixel

procstartdll	_SV_clear
		jmp		[DC.clear]
procend			_SV_clear

procstartdll	_SV_beginLine
		jmp		[DC.beginLine]
procend			_SV_beginLine

procstartdll	_SV_line
		jmp		[DC.line]
procend			_SV_line

procstartdll	_SV_lineFast
		jmp		[DC.lineFast]
procend			_SV_lineFast

procstartdll	_SV_endLine
		jmp		[DC.endLine]
procend			_SV_endLine

procstartdll	_SV_setActivePage
		jmp		[DC.setActivePage]
procend			_SV_setActivePage

procstartdll	_SV_setVisualPage
		jmp		[DC.setVisualPage]
procend			_SV_setVisualPage

procstartdll	_SV_beginDirectAccess
		jmp		[DC.beginDirectAccess]
procend			_SV_beginDirectAccess

procstartdll	_SV_endDirectAccess
		jmp		[DC.endDirectAccess]
procend			_SV_endDirectAccess

endif

if flatmodel

;----------------------------------------------------------------------------
; void _SV_beginDirectAccessAFSLOW(void)
;----------------------------------------------------------------------------
procstartdll	__SV_beginDirectAccessAFSLOW

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.EnableDirectAccess]
		pop		ebx
		ret

procend		__SV_beginDirectAccessAFSLOW

;----------------------------------------------------------------------------
; void _SV_endDirectAccessAFSLOW(void)
;----------------------------------------------------------------------------
procstartdll	__SV_endDirectAccessAFSLOW

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.DisableDirectAccess]
		pop		ebx
		ret

procend		__SV_endDirectAccessAFSLOW

;----------------------------------------------------------------------------
; void _SV_beginPixelAFSLOW(void)
;----------------------------------------------------------------------------
procstartdll	__SV_beginPixelAFSLOW

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.EnableDirectAccess]
		pop		ebx
		ret

procend		__SV_beginPixelAFSLOW

;----------------------------------------------------------------------------
; void _SV_endPixelAFSLOW(void)
;----------------------------------------------------------------------------
procstartdll	__SV_endPixelAFSLOW

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.DisableDirectAccess]
		pop		ebx
		ret

procend		__SV_endPixelAFSLOW

;----------------------------------------------------------------------------
; void _SV_beginLineAFSLOW(void)
;----------------------------------------------------------------------------
procstartdll	__SV_beginLineAFSLOW

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.EnableDirectAccess]
		pop		ebx
		ret

procend		__SV_beginLineAFSLOW

;----------------------------------------------------------------------------
; void _SV_endLineAFSLOW(void)
;----------------------------------------------------------------------------
procstartdll	__SV_endLineAFSLOW

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.DisableDirectAccess]
		pop		ebx
		ret

procend		__SV_endLineAFSLOW

;----------------------------------------------------------------------------
; void _SV_beginDirectAccessAF(void)
;----------------------------------------------------------------------------
procstartdll	__SV_beginDirectAccessAF

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.WaitTillIdle]
		pop		ebx
		ret

procend		__SV_beginDirectAccessAF

;----------------------------------------------------------------------------
; void _SV_endDirectAccessAF(void)
;----------------------------------------------------------------------------
procstartdll	__SV_endDirectAccessAF
		ret
procend		__SV_endDirectAccessAF

;----------------------------------------------------------------------------
; void _SV_beginPixelAF()
;----------------------------------------------------------------------------
procstartdll	__SV_beginPixelAF

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.WaitTillIdle]
		pop		ebx
		ret

procend		__SV_beginPixelAF

;----------------------------------------------------------------------------
; void _SV_endPixelAF()
;----------------------------------------------------------------------------
procstartdll	__SV_endPixelAF
		ret
procend		__SV_endPixelAF

;----------------------------------------------------------------------------
; void _SV_beginLineAF()
;----------------------------------------------------------------------------
procstartdll	__SV_beginLineAF

		push	ebx
		mov		ebx,[DC.AFDC]
		call	[AF_devCtx.WaitTillIdle]
		pop		ebx
		ret

procend		__SV_beginLineAF

;----------------------------------------------------------------------------
; void _SV_endLineAF()
;----------------------------------------------------------------------------
procstartdll	__SV_endLineAF
		ret
procend		__SV_endLineAF

;----------------------------------------------------------------------------
; void _SV_lineFastAF(int x1,int y1,int x2,int y2,ulong color)
;----------------------------------------------------------------------------
; Routine draws a solid, single pixel wide line on accelerated devices.
; Assumes that the hardware has already been set up with the correct write
; mode operation.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;				color	- Color to draw the line in
;----------------------------------------------------------------------------
procstartdll	__SV_lineFastAF

		ARG		x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[color]
		mov		ebx,[DC.AFDC]
		shl		[x1],16					; Convert to 16.16 fixed point
		shl		[y1],16
		shl		[x2],16
		shl		[y2],16
		lea     esi,[x1]
		call	[AF_devCtx.DrawLine]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		__SV_lineFastAF

endif

endcodeseg	_svgasdk

		END

