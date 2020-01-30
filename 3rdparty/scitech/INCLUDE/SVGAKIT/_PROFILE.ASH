;****************************************************************************
;*
;*          The SuperVGA Kit - UniVBE Software Development Kit
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
;* Environment: IBM PC Real Mode and 16/32 bit Protected Mode.
;*
;* Description: Assembly language test routines for the SuperVGA Kit
;*              PROFILE.EXE program.
;*
;*
;****************************************************************************

        IDEAL

INCLUDE "model.mac"             ; Memory model macros

header  _profile                    ; Set up memory model

        EXTRN   _SV_setBankASM:FPTR

begcodeseg  _profile

;----------------------------------------------------------------------------
; void bltImage(void *videoMem,char *p,int numBanks,int lastBytes)
;----------------------------------------------------------------------------
; Blts and image from a memory bank to the display. This routine does
; a simple full screen Blt, and is intended to test the speed of performing
; such Blts in 32 bit protected mode for specific resolutions. This code
; will only work in 32 bit protected mode, as we need to work with _huge_
; memory blocks in a linear fashion (it will however work with the 320x200
; 256 color standard VGA mode in real mode).
;----------------------------------------------------------------------------
procstart   _bltImage

        ARG     videoMem:DPTR, p:DPTR, numBanks:UINT, lastBytes:UINT

        enter_c 0
        use_ds

        _les    _di,[videoMem]      ; EDI -> start of video memory
        _lds    _si,[p]             ; DS:_SI -> memory block to Blt
        xor     dl,dl               ; DL := first bank number
        mov     dh,[BYTE numBanks]  ; DH := number of full banks to fill
        or      dh,dh
        jz      @@DoPartial         ; Only a single bank to handle

; Move all of the full 64k banks first

@@OuterLoop:
        mov     al,dl
        call    _SV_setBankASM
        mov     _cx,4000h           ; Need to set 4000h DWORDS per bank
        push    _di
    rep movsd
        pop     _di
        inc     dl
        dec     dh
        jnz     @@OuterLoop

; Now fill the last partial bank

@@DoPartial:
        mov     al,dl
        call    _SV_setBankASM
        mov     _cx,[lastBytes]
        shr     _cx,2               ; _CX := number of DWORDs to set
    rep movsd

        unuse_ds
        leave_c_nolocal
        ret

procend     _bltImage

;----------------------------------------------------------------------------
; void clearBufLin(void *buffer,long value,uint len)
;----------------------------------------------------------------------------
procstart   _clearBufLin

        ARG     buffer:DPTR, value:ULONG, len:UINT

        enter_c 0

		_les    _di,[buffer]    ; ES:_DI -> memory block
		mov     eax,[value]     ; EAX := value to store
		mov		ah,al
		mov		ecx,eax
		shl		ecx,16
		or		eax,ecx
		mov     _cx,[len]
		shr     _cx,2           ; Convert from bytes to DWORDS
		cld
	rep stosd                   ; Store all middle DWORD's fast!

		leave_c_nolocal
		ret

procend     _clearBufLin

;----------------------------------------------------------------------------
; void clearBufDownLin(void *buffer,long value,uint len)
;----------------------------------------------------------------------------
procstart   _clearBufDownLin

		ARG     buffer:DPTR, value:ULONG, len:UINT

		enter_c 0

		_les    _di,[buffer]    ; ES:_DI -> memory block
		mov     eax,[value]     ; EAX := value to store
		mov		ah,al
		mov		ecx,eax
		shl		ecx,16
		or		eax,ecx
		mov     _cx,[len]
		add		_di,_cx
		sub		_di,4			; ES:_DI -> end of memory block
		shr     _cx,2           ; Convert from bytes to DWORDS
		std						; Go down in video memory
	rep stosd                   ; Store all middle DWORD's fast!

		leave_c_nolocal
		ret

procend     _clearBufDownLin

;----------------------------------------------------------------------------
; void readBufLin(void *buffer,uint len)
;----------------------------------------------------------------------------
procstart   _readBufLin

		ARG     buffer:DPTR, len:UINT

		enter_c 0

		_les    _di,[buffer]    ; ES:_DI -> memory block
		mov     _cx,[len]
		shr		_cx,2			; Convert to DWORDs

@@Loop:	mov		eax,[_ES _di]
		add		_di,4
		dec		_cx
		jnz		@@Loop

		leave_c_nolocal
		ret

procend     _readBufLin

;----------------------------------------------------------------------------
; void copyBufLin(void *buffer,char *image,uint len)
;----------------------------------------------------------------------------
procstart   _copyBufLin

		ARG     buffer:DPTR, image:DPTR, len:UINT

		enter_c 0
		use_ds

		_les    _di,[buffer]    ; ES:_DI -> destination block
		_lds    _si,[image]     ; DS:_SI -> source block
		mov     _cx,[len]
		shr     _cx,2           ; Convert from bytes to DWORDS
		cld
	rep movsd                   ; Copy all DWORDS's fast!

		unuse_ds
		leave_c_nolocal
		ret

procend     _copyBufLin

endcodeseg  _profile

		END
