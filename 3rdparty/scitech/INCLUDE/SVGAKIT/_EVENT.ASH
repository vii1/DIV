;****************************************************************************
;*
;*          		WinDirect DOS Compatibility Library
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
;* Description:	Assembly language support routines for the event module.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include "model.mac"				; Memory model macros

header	_event					; Set up memory model

	EXTRN	__WD_addKeyEvent:FPTR
	EXTRN	_PM_chainPrevKey:FPTR

begdataseg	_event

	$EXTRN  __WD_biosSel,UINT

__WD_dataStart:
			PUBLIC	__WD_dataStart

KEYDOWN		EQU	1				; Key down event
KEYREPEAT	EQU	2				; Key repeat event
KEYUP		EQU	4				; Key up event
KB_HEAD		EQU WORD es:01Ah	; Keyboard buffer head in BIOS data area
KB_TAIL		EQU	WORD es:01Ch	; Keyboard buffer tail in BIOS data area
KB_START	EQU	WORD es:080h	; Start of keyboard buffer in BIOS data area
KB_END		EQU	WORD es:082h	; End of keyboard buffer in BIOS data area
TICKS		EQU	DWORD es:06Ch	; Timer tick count

extended	db	0				; Was last int for extended key code?
scancode	db	0				; Scancode of last keydown event
repeated	db	0
selGS		dw	0				; Value in GS selector for graphics

; Keyup scancode translation cache. Our object file converter for DJGPP
; cannot currently handle LIDATA records, which the 256 dup (0) operation
; generates so we manually provide 256 entries in this table.

keyupCode	dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			dw	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

__WD_dataEnd:
			PUBLIC	__WD_dataEnd

enddataseg	_event

begcodeseg	_event				; Start of code segment

__WD_codeStart:
			PUBLIC	__WD_codeStart

;----------------------------------------------------------------------------
; void _WD_keyISR(void)
;----------------------------------------------------------------------------
; Keyboard event handling routine. We have written this event handler in
; assembler because of the necessary low level manipulation that must
; take place. We determine what type of event it is, and post the
; appropriate event type to the event queue (calling some C code to handle
; this for us).
;----------------------------------------------------------------------------
procstart	__WD_keyISR

		push	es

		in		al,60h			; Get the scan code for the character
		cmp		al,0E1h
		je		@@Extended
		cmp		al,0E0h			; Extended key code?
		jne		@@NotExtended	; No, so process normally

@@Extended:
		mov		[extended],al	; Save the extended code
		call	_PM_chainPrevKey; Chain to previous handler
		jmp		@@Done

; Check to see if the scancode is the same as the previous scan code. If
; it is, then we have a keyboard repeat code.

@@NotExtended:
		mov		[repeated],0	; Clear the key repeat flag
		cmp		[scancode],al
		jne		@@NotRepeated

		mov		[repeated],1	; Set the repeated flag

@@NotRepeated:
		mov		[scancode],al	; Save scan code for later

; Call the old keyboard interrupt handler to process the keypress for us

		call	_PM_chainPrevKey

		test	[scancode],80h	; Is this a break code?
		jz		@@KeyDown		; Process as a keydown style event

; We have a key up event. Key up events are normally ignored by the BIOS,
; so we cache the value from the BIOS of the previous KEYDOWN event given
; the raw scancode, so we can look up the proper value here.

		mov		_dx,KEYUP		; This is a keyup event
		xor		_bx,_bx			; Clear out message field
		mov		bl,[scancode]
		and		bl,7Fh			; Convert to a make scan code
		shl		_bx,1
		mov     ax,[keyupCode+_bx]
		mov		[extended],0	; Clear the extended flag
		jmp		@@InsertEvent

@@KeyDown:
		mov		ax,[WORD __WD_biosSel]
		mov		es,ax			; ES -> BIOS data area
		xor		_bx,_bx
		mov		bx,[KB_HEAD]
		cmp		bx,[KB_TAIL]
		jz		@@Done			; Nothing in buffer, so all done

; A character has been translated and deposited into the keyboard buffer
; for us by the ROM BIOS keyboard handler, so now remove it from the
; buffer and post it to our event queue.

		xor		_ax,_ax
		mov     ax,[es:_bx]		; _AX := character from keyboard buffer
		inc		bx
		inc		bx
		cmp		bx,[KB_END]		; Hit the end of the keyboard buffer?
		jl		@@NotAtEnd
		mov		bx,[KB_START]

@@NotAtEnd:
		mov     [KB_HEAD],bx	; Update keyboard buffer head pointer

		xor		_bx,_bx			; Cache code in keyup translation table
		mov		bl,[scanCode]
		shl		_bx,1
		mov     [keyupCode+_bx],ax
		mov		_dx,KEYDOWN
		cmp		[repeated],0	; Is this a auto repeat key down?
		je		@@InsertEvent	; No, must be a valid keydown event

		mov		_dx,KEYREPEAT	; Yes, it is an auto repeat event

; Insert the event into the event queue

@@InsertEvent:
		push	ds
		pop		es				; ES -> Data segment again
		push	_ax				; Message = key pressed
		push	_dx				; What = event type
		call	__WD_addKeyEvent; Insert in key event queue
		_add	sp,4,8

@@Done:	pop		es
		ret

procend		__WD_keyISR

;----------------------------------------------------------------------------
; int _WD_getKeyCode(void)
;----------------------------------------------------------------------------
; Returns the key code for the next available key by calling the BIOS
; directly.
;----------------------------------------------------------------------------
procstart	__WD_getKeyCode

		mov		ah,11h			; Function - Check keyboard status
		int		16h				; Call BIOS
		mov		_ax,0
		jz		@@Done			; No, key aint available

		mov		ah,10h			; Function - Get keyboard character
		int		16h				; Call BIOS
		clrhi	ax
@@Done:
		ret

procend		__WD_getKeyCode

;----------------------------------------------------------------------------
; void _WD_winBugFix(void)
;----------------------------------------------------------------------------
; Call the Int 16h BIOS keyboard handler to get around bugs when running
; under Windows 3.1 DOS boxes. The bug is not present under Win 95 DOS
; boxes.
;----------------------------------------------------------------------------
procstart	__WD_winBugFix

		mov		ah,11h			; Function - Check keyboard status
		int		16h				; Call BIOS
		ret

procend		__WD_winBugFix

;----------------------------------------------------------------------------
; int _WD_disableInt(void);
;----------------------------------------------------------------------------
; Return processor interrupt status and disable interrupts.
;----------------------------------------------------------------------------
procstart	__WD_disableInt

        pushf                   ; Put flag word on stack
        cli                     ; Disable interrupts!
        pop     _ax             ; deposit flag word in return register
		ret

procend		__WD_disableInt

;----------------------------------------------------------------------------
; void _WD_restoreInt(int ps);
;----------------------------------------------------------------------------
; Restore processor interrupt status.
;----------------------------------------------------------------------------
procstart	__WD_restoreInt

		ARG		ps:UINT

        push    _bp
        mov     _bp,_sp         ; Set up stack frame
        push    [ps]
        popf                    ; Restore processor status (and interrupts)
        pop     _bp
		ret

procend		__WD_restoreInt

;----------------------------------------------------------------------------
; void _WD_enableInt(void)
;----------------------------------------------------------------------------
; Unconditionally enable processor interrupts.
;----------------------------------------------------------------------------
procstart	__WD_enableInt

		sti
		ret

procend		__WD_enableInt

;----------------------------------------------------------------------------
; void _WD_sound(unsigned frequency)
;----------------------------------------------------------------------------
; Outputs a sound of the specified frequency to the speaker. Most C
; compilers provide this function, but some including MSC do not.
;----------------------------------------------------------------------------
procstart	__WD_sound

		ARG		frequency:UINT

		enter_c	0
        mov     _bx,[frequency]
        mov     _ax,34DDh
        mov     _dx,0012h
        cmp     _dx,_bx
		jnb     @@Done
		div     bx
        mov     _bx,_ax
		in      al,61h
		test    al,3
		jne     @@1
		or      al,3
		out     61h,al
		mov     al,0B6h
		out     43h,al
@@1:	mov     al,bl
		out     42h,al
		mov     al,bh
		out     42h,al

@@Done:
		leave_c_nolocal
		ret

procend		__WD_sound

;----------------------------------------------------------------------------
; void _WD_nosound(void)
;----------------------------------------------------------------------------
; Turns off the speaker
;----------------------------------------------------------------------------
procstart	__WD_nosound

		in      al,61H
		and     al,0fcH
		out     61H,al
		ret

procend		__WD_nosound

; Macro to delay briefly to ensure that enough time has elapsed between
; successive I/O accesses so that the device being accessed can respond
; to both accesses even on a very fast PC.

macro	DELAY
		jmp		$+2
		jmp		$+2
		jmp		$+2
endm

;----------------------------------------------------------------------------
; unsigned _WD_readtimer(void)
;----------------------------------------------------------------------------
; Read the value of the PC timer 0 and return it.
;----------------------------------------------------------------------------
procstart	__WD_readtimer

		pushf					; Save interrupt flag
		cli						; Disable interrupts
		mov		al,0h			; Latch timer 0
		out		43h,al
		DELAY
		in		al,40h			; least significant byte
		DELAY
		mov		ah,al
		in		al,40h			; most significant byte
		xchg	ah,al
		neg		ax				; Convert from countdown remaining
		clrhi	ax
		popf
		ret

procend		__WD_readtimer

__WD_codeEnd:
			PUBLIC	__WD_codeEnd

endcodeseg	_event

		END							; End of module
