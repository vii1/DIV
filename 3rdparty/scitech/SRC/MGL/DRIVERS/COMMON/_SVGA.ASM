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
;* Description:	Assembler routines common for all VESA VBE compatible video
;*				cards.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"
include "mgldos\biosdata.equ"

header	_svga

begdataseg	_svga

		$EXTRN	_VBE_version,UINT
		$EXTRN  _VBE_bankShift,UINT
		$EXTRN	_VBE_bankSwitch,DPTR
		$EXTRN	_SVGA_curBank,BYTE
		$EXTRN	_SVGA_setBankVec,DPTR

if flatmodel
		$EXTRN	_setBank20,DPTR
		$EXTRN	_setCRT20,DPTR
		$EXTRN	_setPal20,DPTR
		$EXTRN	_VBE_MMIOSel,WORD
ifdef	USING_DS
		$EXTRN	__PM_savedDS,WORD
endif
endif

enddataseg	_svga

begcodeseg	_svga

ifdef   __WINDOWS32_386__
        EXTRN   _PM_setBankA:FPTR
        EXTRN   _PM_setBankAB:FPTR
        EXTRN   _PM_setCRTStart:FPTR
endif

;----------------------------------------------------------------------------
; VBE_setBankA	Sets the read/write bank directly via window A
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
procstart	_VBE_setBankA

		push	_ax
		push	_bx
		push	_cx
		push	_dx
		mov     _ax,04F05h
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE _VBE_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		cmp		[_VBE_bankSwitch],0
		je		@@UseInt10h
		call	[_VBE_bankSwitch]	; Call the VBE directly in real mode
		jmp		@@Exit

@@UseInt10h:
ifdef	__WINDOWS32_386__
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

@@Exit:	pop		_dx
		pop		_cx
		pop		_bx
		pop		_ax
		ret

procend		_VBE_setBankA

;----------------------------------------------------------------------------
; VBE_setBankAB	Sets the read/write bank directly via Window's A & B
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
procstart	_VBE_setBankAB

		push	_ax
		push	_bx
		push	_cx
		push	_dx
		mov     _ax,04F05h
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE _VBE_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		push	_dx					; Save bank value
		cmp		[_VBE_bankSwitch],0
		je		@@UseInt10h
		call	[_VBE_bankSwitch]	; Call the VBE directly in real mode
		pop		_dx
		mov     _ax,04F05h
		mov		_bx,1				; BX := select window B
		call	[_VBE_bankSwitch]
		jmp		@@Exit

@@UseInt10h:
ifdef	__WINDOWS32_386__
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

@@Exit:	pop		_dx
		pop		_cx
		pop		_bx
		pop		_ax
		ret

procend		_VBE_setBankAB

if flatmodel

;----------------------------------------------------------------------------
; VBE20_setBankA	Sets the read/write bank directly via window A
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
procstart	_VBE20_setBankA

		push	_ax
		push	_bx
		push	_cx
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE _VBE_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		call	[_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
        pop		_ax
		ret

procend		_VBE20_setBankA

;----------------------------------------------------------------------------
; VBE20_setBankAB	Sets the read/write bank directly via Window's A & B
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
procstart	_VBE20_setBankAB

		push	_ax
		push	_bx
		push	_cx
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE _VBE_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		push	_dx					; Save bank value
		call	[_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_dx
		mov		_bx,1				; BX := select window B
		call	[_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
        pop		_ax
		ret

procend		_VBE20_setBankAB

;----------------------------------------------------------------------------
; VBE20_setBankA_ES	Sets the read/write bank directly via window A
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
procstart	_VBE20_setBankA_ES

		push	es
        push	_ax
		push	_bx
		push	_cx
		mov		bx,[_VBE_MMIOSel]	; Load selector to MMIO registers
		mov     es,bx
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE _VBE_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		call	[_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
        pop		_ax
		pop		es
		ret

procend		_VBE20_setBankA_ES

;----------------------------------------------------------------------------
; VBE20_setBankAB_ES	Sets the read/write bank directly via Window's A & B
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by calling the VBE twice to set both Window A and Window B, as
; the VBE reports that they have separate read/write attributes. This
; routine is different in the respect that the bank switching code uses
; MMIO registers and requires the ES register to be loaded with the
; selector to its MMIO registers.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstart	_VBE20_setBankAB_ES

		push	es
        push	_ax
		push	_bx
		push	_cx
		mov		bx,[_VBE_MMIOSel]	; Load selector to MMIO registers
		mov     es,bx
		xor		_bx,_bx				; BX := select window A
		mov		cl,[BYTE _VBE_bankShift]; Adjust to VBE granularity
		shl		dl,cl				; DX := VBE bank number
		push	_dx					; Save bank value
		call	[_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_dx
		mov		_bx,1				; BX := select window B
		call	[_setBank20]		; Call VBE 2.0 protected mode routine
		pop		_cx
		pop		_bx
        pop		_ax
		pop		es
		ret

procend		_VBE20_setBankAB_ES

endif

;----------------------------------------------------------------------------
; SVGA_setBank	Sets the read/write bank directly via the VBE
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value.
;
; Entry:		DL	- New read/write bank number
;
; Exit:			DL	- New read/write bank number
;
; Registers:	All preserved!
;----------------------------------------------------------------------------
procstart	_SVGA_setBank

ife	flatmodel
		push	ds
		push	ax
		mov		ax,DGROUP
		mov		ds,ax
		pop		ax
else
ifdef	USING_DS
		push	ds
		push	es
		mov		ds,[cs:__PM_savedDS]	; We can access the proper DS through CS
		mov		es,[cs:__PM_savedDS]
endif
endif
		push	_dx
		mov		[_SVGA_curBank],dl	; Save current write bank number
		call	[_SVGA_setBankVec]	; Call card specific bank routine
		pop		_dx
ife	flatmodel
		pop		ds
else
ifdef	USING_DS
		pop		es
		pop		ds
endif
endif
		ret

procend		_SVGA_setBank

;----------------------------------------------------------------------------
; void SVGA_setBankC(int bank)
;----------------------------------------------------------------------------
; Sets the new read/write bank number from C
;----------------------------------------------------------------------------
procstart   _SVGA_setBankC

		ARG     bank:UINT

		push    _bp
		mov     _bp,_sp
		mov     _dx,[bank]
		call    _SVGA_setBank
		pop     _bp
		ret

procend     _SVGA_setBankC

;----------------------------------------------------------------------------
; SV_setBank	Sets the read/write bank directly via the VBE
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
procstart	_SV_setBankASM

ife	flatmodel
		push	ds
		push	ax
		mov		ax,DGROUP
		mov		ds,ax
		pop		ax
else
ifdef	USING_DS
		push	ds
		push	es
		mov		ds,[cs:__PM_savedDS]	; We can access the proper DS through CS
		mov		es,[cs:__PM_savedDS]	; We can access the proper DS through CS
endif
endif
		push	_dx
		mov		[_SVGA_curBank],al	; Save current write bank number
		mov		dl,al
		call	[_SVGA_setBankVec]	; Call card specific bank routine
		pop		_dx
ife	flatmodel
		pop		ds
else
ifdef	USING_DS
		pop		es
		pop		ds
endif
endif
		ret

procend		_SV_setBankASM

;----------------------------------------------------------------------------
; void SV_setBank(int bank)
;----------------------------------------------------------------------------
; Sets the new read/write bank number from C
;----------------------------------------------------------------------------
procstart   _SV_setBank

		ARG     bank:UINT

		push    _bp
		mov     _bp,_sp
		mov     _dx,[bank]
		call    _SVGA_setBank
		pop     _bp
		ret

procend     _SV_setBank

;----------------------------------------------------------------------------
; void VBE_setCRTStart(long crtStart,int bits,int waitVRT)
;----------------------------------------------------------------------------
; Routine to set the visible video page.
;
; Entry:	crtStart	- CRT starting address to program
;----------------------------------------------------------------------------
procstart	_VBE_setCRTStart

		ARG     crtStart:ULONG, bits:UINT, bytesPerLine:UINT,		\
				waitVRT:UINT

		enter_c	0
		push	es

if flatmodel
		cmp		[_setCRT20],0
		je		@@NotVBE20

; Directly call the 32 bit protected mode CRTC setting routine. The
; interface to the 32 bit routine is different, as it takes the CRTC
; starting address in DX:CX in the same format used for standard VGA modes.

		mov		eax,[crtStart]		; EAX := CRTC starting address
		cmp		[bits],4
		je		@@1
		shr		eax,2				; Adj to plane boundary for 8 bit+ modes
@@1:	mov     cx,ax
		shr		eax,16
		mov		dx,ax				; DX:CX := CRTC start address
		xor     _bx,_bx             ; BX := 0 - set display start
		cmp		[waitVRT],0
		je		@@2
		mov		_bx,80h				; BX := 80h - set display start with VRT
@@2:    mov     es,[_VBE_MMIOSel]   ; Load selector to MMIO registers
		call	[_setCRT20]			; Call the relocated code
		jmp		@@Exit
endif

@@NotVBE20:
		mov		ax,[WORD crtStart]
		mov		dx,[WORD crtStart+2]
		div     [USHORT bytesperline]	; AX := starting scanline,
										; DX := starting byte
		mov     cx,dx
		cmp     [bits],4
		je      @@4Bit
		cmp     [bits],8
		je      @@SetIt
		cmp     [bits],24
		je      @@24Bit
		cmp     [bits],32
		je      @@32Bit

		shr     cx,1                ; CX := starting pixel in buffer
		jmp     @@SetIt

@@4Bit:
		shl     cx,3                ; CX := starting pixel in buffer
		jmp     @@SetIt

@@24Bit:
		mov		bx,ax				; Preserve AX
		xor     dx,dx
		mov     ax,cx
		mov     cx,3
		div     cx
		mov     cx,ax               ; CX := starting pixel in buffer
		mov		ax,bx				; Restore AX
		jmp		@@SetIt

@@32Bit:
		shr     cx,2                ; CX := starting pixel in buffer

@@SetIt:
		mov     bx,ax               ; BX := starting scanline in buffer
		mov     _ax,04F07h
		mov     _dx,_bx             ; DX := starting scanline number
		xor     _bx,_bx             ; BX := 0 - set display start
		cmp		[_VBE_version],200h
		jl		@@VBE12
		cmp		[waitVRT],0
		je		@@3
		mov		_bx,80h				; BX := 80h - set display start with VRT
ifdef	__WINDOWS32_386__
@@3:	push	ebx
		push	edx
		push	ecx
		call	_PM_setCRTStart
        add		esp,12
else
@@3:	int     10h                 ; Set the display start address
endif
		jmp		@@Exit

; For Non VBE 2.0 implementations we cannot guarantee that the
; implementation will wait for a retrace when setting the display start
; address, so we need to wait for the retrace here just to be sure. Note
; that this _assumes_ the controller is VGA compatible, so will hang in
; an infinite loop on controllers that are not VGA compatible.

@@VBE12:
		push    _ax
        push    _dx
        cmp     [waitVRT],0
        je      @@NoWaitDE
        mov     dx,03DAh            ; DX := video status port
@@WaitDE:
        in      al,dx
        test    al,1
        jnz     @@WaitDE            ; Wait for Display Enable
@@NoWaitDE:
        mov     dx,03DAh            ; DX := video status port
        pop     _dx
        pop     _ax

ifdef	__WINDOWS32_386__
		push	ebx
		push	edx
		push	ecx
		call	_PM_setCRTStart
		add		esp,12
else
		int     10h                 ; Set the display start address
endif
		cmp     [waitVRT],0
        je      @@Exit
        mov     dx,03DAh            ; DX := video status port
@@WaitV:
        in      al,dx               ; Wait for start of vertical retrace
        test    al,8
        jz      @@WaitV

@@Exit:	pop		es
		leave_c_nolocal
		ret

procend     _VBE_setCRTStart

;----------------------------------------------------------------------------
; void VBE20_blastPalette(uchar *pal,int num,int index,int maxProgram,
;	int waitVRT)
;----------------------------------------------------------------------------
; Program the SuperVGA palette. The values passed to this routine are in the
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
;----------------------------------------------------------------------------
procstart	_VBE20_blastPalette

		ARG		pal:DPTR, num:UINT, index:UINT, maxProgram:UINT, waitVRT:UINT

		enter_c	0

		_les    _di,[pal]			; ES:_DI -> buffer to restore from
		mov     _cx,[num]			; _CX := entries to program
		mov		_dx,[index]			; _DX := starting index
		mov     _si,[maxprogram]	; _SI := maxprogram value

; Determine how many colors can be programmed before the onset of snow.

		xor		_ax,_ax
		xor		_bx,_bx
		cmp		[waitVRT],0
		je		@@SkipWait			; Skip VRT syncing

		mov		bl,80h				; Set flag to sync to VRT
		mov		_ax,_cx				; AX := number of colors to set

; Work out how many registers we can program this time around.

@@OuterLoop:
		mov		_cx,_si
		cmp		_ax,_cx
		jge		@@Larger
		mov		_cx,_ax				; Set count to what is left in AX
@@Larger:
		sub		_ax,_cx				; AX := number of registers left to
									;  program after this loop
@@SkipWait:
		push	_ax
		push	_cx
		push	_dx
		push	_si
		push	_di
if flatmodel
		mov     eax,[_setPal20]
        push    ds
        mov     ds,[_VBE_MMIOSel]   ; Load selector to MMIO registers
        call    eax                 ; Call the VBE 2.0 palette code
        pop     ds
else
		mov		ax,4F09h			; Set palette function number
		int		10h					; Program the palette values
endif
		pop		_di
		pop		_si
		pop		_dx
		pop		_cx
		pop		_ax
		add     _dx,_cx				; Increment starting palette index
		shl		_cx,2
		add		_di,_cx				; Increment to next position in buffer
		or		_ax,_ax
		jnz		@@OuterLoop			; We are all done

		leave_c_nolocal
		ret

procend		_VBE20_blastPalette

endcodeseg	_svga

		END							; End of module
