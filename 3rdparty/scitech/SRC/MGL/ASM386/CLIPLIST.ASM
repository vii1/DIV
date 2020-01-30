;----------------------------------------------------------------------------
; void LINEAR8_drawClippedScanList(MGLDC *dc,int y,int length,short *scans);
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert lines any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call MGL_scanLine to draw each scanline for us.
;
; Note: This routine will reverse the X coordinates for each scanline if
;		necessary to make xstart < xend, and will scan convert all pixels
;		in the range [xstart,xend), so xend will not be scan converted.
;		We also ignore any scanline where xstart == xend.
;
;		We also assume that the video card is already in write mode 3.
;
;		Clipping is performed within this routine.
;
; Entry:	dc		- Device context
;			y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scans	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstartLIN	drawClippedScanList

		ARG		dc:DPTR, y:UINT, len:UINT, scans:DPTR

		LOCAL   cleft:UINT, ctop:UINT, cright:UINT, cbottom:UINT,	\
				bytesPerLine:UINT, color:USHORT = LocalSize

		enter_c	LocalSize
		use_ds
		cld							; String instructions go up

; Load the clipping rectangle from the DC onto the stack

		_les	_bx,[dc]			; ES:_BX -> device context
		mov		_ax,[MGLDC.intClipRect.left]
		mov		[cleft],_ax
		mov		_ax,[MGLDC.intClipRect.top]
		mov		[ctop],_ax
		mov		_ax,[MGLDC.intClipRect.right]
		dec		_ax
		mov		[cright],_ax
		mov		_ax,[MGLDC.intClipRect.bottom]
		dec		_ax
		mov		[cbottom],_ax
		mov		_ax,[MGLDC.mi.bytesPerLine]
		mov		[bytesPerLine],_ax
		mov		al,[BYTE MGLDC.intColor]
		mov		ah,al
		mov		[color],ax

; Clip the y extents

		mov		_si,[y]				; _SI := top coord of polygon
		mov		_di,[len]			; _DI := number of scanlines to process

		mov		_ax,[ctop]			; AX := top clipping coordinate
		mov		_dx,_ax
		sub		_dx,_si				; DX := clipRect.top - top
		jle		@@DontClipTop		; It is in range, don't clip it

; Clip the top of the polygon

		mov		_si,_ax				; Set the new top to clipRect.top
		sub		_di,_dx				; Adjust number of scanlines to process
		jle		@@Exit				; Quit if polygon above top of rectangle

; Now adjust the index into the scanline array to reflect the new top
; coordinate.

		shl		_dx,2				; DX := (newtop - oldtop) * 4
		add		[UINT scans],_dx	; Update offset into array

@@DontClipTop:
		mov		_ax,[cbottom]		; AX := clipRect.bottom
		mov		_bx,_si
		add		_bx,_di				; BX := bottom coord of polygon + 1
		dec		_bx
		sub		_bx,_ax				; BX := bottom - clipRect.bottom
		jle		@@DontClipBottom	; It is in range, don't clip it

; Clip the bottom of the polygon

		sub		_di,_bx				; Adjust the number of scanlines
		jle		@@Exit				; Quit if polygon below bottom of rectangle

; Check to see if we can proceed at full speed, otherwise jump to a slower
; routine for patterned fills etc.

@@DontClipBottom:
		_les    _bx,[dc]
		cmp		[MGLDC.a.writeMode],REPLACE_MODE
		jne		@@SlowFill
		cmp		[MGLDC.a.penStyle],SOLID_PATTERN
		jne		@@SlowFill

;----------------------------------------------------------------------------
; Fast scanline fill routine. This has been optimised for solid fills in
; replace mode.
;
; Compute address of start of first scan line in video buffer

@@FastFill:
		mov		_ax,_si				; AX := top coordinate
		mov		_cx,_di				; CX := number of scanlines to fill
		mul		[MGLDC.mi.bytesPerLine]	 ; AX := top * BytesPerLine
		add		_ax,[UINT MGLDC.originOffset]
		mov		_di,_ax
		mov		ax,gs
		mov		es,ax				; ES:DI -> first scanline in video buffer

; Get the address of the scanline array in DS:SI

		_lds	_si,[scans]			; DS:_SI -> scanline array data
		xor		_ax,_ax				; Clear accumulator

@@FillLoop:

; Check that xstart < xend, swapping if need be.

		lodsw
		mov		_bx,_ax				; BX := xstart for scanline
		lodsw						; AX := xend for scanline
		cmp		_bx,_ax				; is xstart > xend?
		jl		@@NoSwap			; no, don't swap them
		jg		@@Swap				; yes, swap them
		jmp		@@SkipLine			; ignore if xstart == xend

@@Swap:
		xchg	_ax,_bx

@@NoSwap:
		push	_cx					; Preserve CX
		push	_di					; Preserve DI

		mov		_cx,[cleft]			; CX := left clipping coordinate
		mov		_dx,[cright]		; DX := right clipping coordinate
		inc		_dx					; Adjust right clipping coord

; Clip xstart against the right clipping edge

		cmp		_bx,_dx				; is xstart < clipRect.right?
		jg		@@DoneLine			; No, entire scanline is out of range

; Clip xend against the left clipping edge

		cmp		_ax,_cx				; is xend > clipRect.left?
		jl		@@DoneLine			; No, entire scanline is out of range

; Clip xstart against the left clipping edge

		cmp		_bx,_cx				; is xstart >= clipRect.left?
		jg		@@DontClipLeft		; Yes, xstart is within range

		mov		_bx,_cx				; Clip xstart to clipRect.left

; Clip xend against the right clipping edge

@@DontClipLeft:
		cmp		_ax,_dx				; is xend <= clipRect.right?
		jle		@@DontClipRight		; Yes, xend is within range

		mov		_ax,_dx				; Clip xend to clipRect.right

@@DontClipRight:
		mov		_cx,_ax				; _CX := xend
		sub		_cx,_bx				; _CX := xend - xstart (# pixels to draw)
		jcxz	@@DoneLine			; Clipped line is zero length
		add		_di,_bx				; ES:_DI -> first pixel to fill

		mov		ax,[color]			; AL,AH := pixel color

		test	_di,1
		jz		@@WordAligned
		stosb						; Store first byte to ensure word
		dec		_cx					;  alignment during next phase
@@WordAligned:
		shr		_cx,1
	rep	stosw						; Store all middle bytes fast!
		adc		_cx,_cx
	rep	stosb						; Store the last byte

@@DoneLine:
		pop		_di					; Restore DI
		pop		_cx					; Restore CX

@@SkipLine:
		add		_di,[bytesPerLine]	; Increment address to next scanline
		loop	@@FillLoop			; Loop for all scanlines
		jmp		@@Exit

;----------------------------------------------------------------------------
; Slower scanline filling routine. This routine simply calls scanline to
; draw each scanline.

@@SlowFill:
		mov		[len],_di			; len := number of scanlines to fill
		mov		_di,_si				; DI := top coordinate

; Get the address of the scanline array in DS:SI

		_lds	_si,[scans]			; DS:_SI -> scanline array data
		xor		_ax,_ax				; Clear out accumulator

@@FillLoop2:

; Check that xstart < xend, swapping if need be.

		lodsw
		mov		_bx,_ax				; BX := xstart for scanline
		lodsw						; AX := xend for scanline
		cmp		_bx,_ax				; is xstart > xend?
		jl		@@NoSwap2			; no, don't swap them
		jg		@@Swap2				; yes, swap them
		jmp		@@SkipLine2			; ignore if xstart == xend

@@Swap2:
		xchg	_ax,_bx

@@NoSwap2:
		dec		_ax					; Decrement the xend coordinate
		mov		_cx,[cleft]			; CX := left clipping coordinate
		mov		_dx,[cright]		; DX := right clipping coordinate

; Clip xstart against the right clipping edge

		cmp		_bx,_dx				; is xstart < clipRect.right?
		jg		@@DoneLine2			; No, entire scanline is out of range

; Clip xend against the left clipping edge

		cmp		_ax,_cx				; is xend > clipRect.left?
		jl		@@DoneLine2			; No, entire scanline is out of range

; Clip xstart against the left clipping edge

		cmp		_bx,_cx				; is xstart >= clipRect.left?
		jge		@@DontClipLeft2		; Yes, xstart is within range

		mov		_bx,_cx				; Clip xstart to clipRect.left

; Clip xend against the right clipping edge

@@DontClipLeft2:
		cmp		_ax,_dx				; is xend <= clipRect.right?
		jle		@@DontClipRight2	; Yes, xend is within range

		mov		_ax,_dx				; Clip xend to clipRect.right

@@DontClipRight2:
		inc		_ax					; Increment xend coordinate
		push	_ax					; Push xend coordinate
		push	_bx					; Push xstart coordinate
		push	_di
		push	[dc]
		_les	_bx,[dc]
		call	[MGLDC.scanLine]
		_add	sp,10,16

@@DoneLine2:

@@SkipLine2:
		inc		_di
		dec		[len]
		jnz		@@FillLoop2			; Loop for all scanlines

@@Exit:
		unuse_ds
		leave_c
		ret

procendLIN		drawClippedScanList

