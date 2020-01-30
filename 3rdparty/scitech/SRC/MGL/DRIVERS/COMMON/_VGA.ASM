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
;* Description:	Assembler routines common for all VGA compatible video cards.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"

header	_vga

begcodeseg	_vga

;----------------------------------------------------------------------------
; void VGA_blastPalette(uchar *pal,int num,int index,int maxProgram,
;	int waitVRT)
;----------------------------------------------------------------------------
; Program the VGA palette. The values passed to this routine are in the
; correct format that the palette is expecting given its current state
; (ie: in 6 or 8 bit format).
;
; In order to avoid snow when setting the palette, we only ever set the
; entries during vertical retrace periods.
;
; Entry:	pal			- Palette of values to program
;           num			- Number of bytes to program
;			index		- Index to start programming at
;			maxProgram	- Number of entries to program during retrace
;			waitVRT		- True to wait for vertical retrace
;----------------------------------------------------------------------------
procstart	_VGA_blastPalette

		ARG		pal:DPTR, num:UINT, index:UINT, maxProgram:UINT, waitVRT:UINT

		enter_c	0
		use_ds
		cld

		_lds	_si,[pal]			; DS:_SI -> palette to program
		mov		_ax,[index]			; AX := first color register to program
		mov		_bx,[num]			; BX := number of colors to set

		mov		dx,3C8h				; DX := DAC Write Index register
		out		dx,al				; Index the first color register

		cmp		[waitVRT],0
		jne		@@WaitForRetrace
		mov		_cx,_bx				; Program all registers at once
		xor		_bx,_bx
		jmp		@@OuterLoop			; Skip the vertical retrace syncing

; Determine how many colors can be programmed before the onset of snow.

@@WaitForRetrace:
		mov		_cx,[maxProgram]
		cmp		_bx,_cx
		jge		@@Larger
		mov		_cx,_bx				; Set count to number of registers
@@Larger:
		sub		_bx,_cx				; BX := number of registers left to
									;  program after this loop

; Before we enter the outer loop for the first time, wait for the
; beginning of a vertical retrace to ensure that we have the maximum
; amount of time to program the color registers before the onset of
; the dreaded snow phenomena.

		mov     dx,3DAh				; DX := Input Status register
@@WaitNotVsync:
		in		al,dx
		test	al,8
		jnz		@@WaitNotVsync
@@WaitVsync:
		in		al,dx
		test	al,8
		jz		@@WaitVsync

@@OuterLoop:
		mov		dx,3C9h				; DX := DAC data register

@@NextEntry:
		mov		al,[_si+2]
		out		dx,al
		mov		al,[_si+1]
		out		dx,al
		mov		al,[_si+0]
		out		dx,al
		add		_si,4
		loop	@@NextEntry

		or		_bx,_bx
		jz		@@Exit				; We are all done

; Work out how many registers we can program this time around.

		mov		_cx,[maxprogram]
		cmp		_bx,_cx
		jge		@@Larger2
		mov		_cx,_bx				; Set count to what is left in BX
@@Larger2:
		sub		_bx,_cx				; BX := number of register left

; Wait for the start of the vertical retrace again (we assume that we
; will not be in the vertical retrace at the moment, so we simply wait
; for the start, unlike above where we waited for both the end then
; the start.

		mov		dx,3DAh				; DX := Input Status register
@@Wait:
		in		al,dx				; Wait for the leading edge pulse
		test	al,8
		jz		@@Wait
		jmp		@@OuterLoop

@@Exit:	unuse_ds
		leave_c_nolocal
		ret

procend		_VGA_blastPalette

;----------------------------------------------------------------------------
; void VGA_setCRTStart(int crtStart,int which,int waitVRT)
;----------------------------------------------------------------------------
; Routine to set the video page for active output.
;
; Entry:	crtStart	- CRTC starting address to program
;----------------------------------------------------------------------------
procstart	_VGA_setCRTStart

		ARG		crtStart:UINT, which:UINT, waitVRT:UINT

		enter_c	0

; Wait for display enable to be active (active low), to be sure that
; both halves of the start address will take place in one frame. We
; preload a few values here to save time after the DE has been
; detected.

		mov		_bx,[crtStart]		; _BX := offset into video buffer
		mov		cl,0Ch				; CL := Start Address High register
		mov		ch,bh				; CH := high byte of new address
		mov		bh,bl				; BH := low byte of new address
		mov		bl,0Dh				; BL := Start Address Low register
		mov		dx,03DAh			; DX := video status port
		test	[BYTE waitVRT],1
		jz		@@SkipWaitDE

@@WaitDE:
		in		al,dx
		test	al,1
		jnz		@@WaitDE			; Wait for Display Enable

@@SkipWaitDE:
		mov		dx,03D4h			; DX := CRTC I/O port (3D4h)

		mov		ax,bx
		out		dx,ax
		mov		ax,cx
		out		dx,ax

; Now wait for the start of the vertical sync, to ensure that the old
; page will be invisible before anything is drawn on it.

		test	[BYTE waitVRT],1
		jz		@@SkipWaitVert

		mov		dx,03DAh			; DX := video status port
@@WaitStartVert:
		in		al,dx				; Wait for start of vertical retrace
		test	al,8
		jz		@@WaitStartVert

@@SkipWaitVert:
@@Exit:	leave_c_nolocal
		ret

procend		_VGA_setCRTStart

;----------------------------------------------------------------------------
; void VGA_vSync(MGLDC *dc)
;----------------------------------------------------------------------------
; Routine to wait for a vertical sync pulse for synchronising stuff to the
; display refresh.
;----------------------------------------------------------------------------
procstart	_VGA_vSync

		mov		dx,03DAh			; DX := video status port
@@WaitDE:
		in		al,dx
		test	al,1
		jnz		@@WaitDE			; Wait for Display Enable
@@WaitStartVert:
		in		al,dx				; Wait for start of vertical retrace
		test	al,8
		jz		@@WaitStartVert
		ret

procend		_VGA_vSync

endcodeseg	_vga

		END							; End of module
