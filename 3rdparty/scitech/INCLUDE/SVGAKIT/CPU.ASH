;****************************************************************************
;*
;*                        The Universal VESA VBE
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
;* Language:    8086 Assembler
;* Environment: IBM PC Real Mode and 16/32 bit Protected Mode.
;*
;* Description: Autodetection routine to determine the type of CPU installed
;*              in the system.
;*
;*
;****************************************************************************

        IDEAL

INCLUDE "model.mac"             ; Memory model macros

header  cpu                     ; Set up memory model

;****************************************************************************
;
; Equates used by queryCPU routine.
;
;****************************************************************************

; Central Processing Unit type codes

CPU86       =   0               ; 8086/88 type processor
CPU186      =   1               ; 80186 type processor
CPU286      =   2               ; 80286 type processor
CPU286p     =   3               ; 80286 type processor in protected mode
CPU386      =   4               ; 80386 type processor
CPU386p     =   5               ; 80386 type processor in protected mode
CPU486      =   6               ; 80486 type processor
CPU486p     =   7               ; 80486 type processor in protected mode
CPU586      =   8               ; Pentium type processor
CPU586p     =   9               ; Pentium type processor in protected mode

begcodeseg  cpu                 ; Start of code segment

P386                            ; Turn on 386 instructions

;----------------------------------------------------------------------------
; cpu_type SV_queryCpu(void)
;----------------------------------------------------------------------------
; Determine type of processor present.
;----------------------------------------------------------------------------
procstartdll16  _SV_queryCpu

        push    _bx
        push    _bp             ; We MUST save bp for initialization code...

ife flatmodel
        mov     _ax,CPU86       ; Default to 8086/8088 processor
        push    sp
        pop     bx              ; BX holds the value of SP or SP-2
        cmp     bx,sp           ; 88/86/186 pushes the value of SP-2
        je      @@Check286      ; Must be a 286/386/486 type processor
        mov     cl,32           ; 186 uses count mod 32 = 0
        shl     bx,cl           ; 86 shifts 32 bits left so ax = 0
        jz      @@Done          ; zero: shifted out all bits so 86/88
        mov     _ax,CPU186      ; nonzero: no shift, so 186
        jz      @@Done

@@Check286:                     ; First check for 386/486 in 32 bit mode
        pushf                   ; Test for 16 or 32 operand size:
        mov     bx,sp           ;  pushed 2 or 4 bytes of flags
        popf
        inc     bx
        inc     bx
        cmp     bx,sp           ; did pushf change sp by 2?
        jnz     @@Check386      ; 32 bit push, so it is a 386/486

        sub     sp,6            ; Is it a 286/386/486 in 16 bit mode?
        mov     bp,sp
        sgdt    [QWORD ptr bp]  ; 80286/386/486 specific instrucion
        add     sp,4            ; Get global descriptor table
        pop     bx
        inc     bh              ; Third word of GDT = -1 for 286
        jnz     @@Check486      ; We have a 386/486

        mov     _ax,CPU286      ; We have a 286
        jmp     @@TestPROT

@@Check386:
endif

; Distinguish an 80386 from an 80486. Bit 18 (40000H) of EFLAGS register
; is used only in the 486. This code flips it and tests if anything happened.

        mov     edx,esp         ; Save stack pointer
        and     esp,not 3       ; Align stack pointer to prevent a fault
                                ;  when we set the AC flag on a 486
        pushfd                  ; Copy the EFLAGS register
        pop     eax             ;   into register eax
        mov     ecx,eax         ; Save the original EFLAGS value
        xor     eax,40000H      ; Flip the AC flag bit
        push    eax             ; Try to put the modified value back
        popfd                   ;   into the EFLAGS register
        pushfd                  ; Copy the EFLAGS register again
        pop     eax             ;   into eax
        xor     eax,ecx         ; Compare the old and new AC bits
        shr     eax,18          ; Shift and mask to get the AC comparison bit
        and     eax,1           ;   in the low order position of eax
        push    ecx
        popfd                   ; Restore EFLAGS that were saved on entry
        mov     esp,edx         ; And restore stack pointer to saved value
        mov     bx,ax           ; and move into bx

; At this point bx = 0 for a 386, or bx = 1 for a 486

        test    bx,bx
        jnz     @@Check486      ; Check for the presence of a 486/P5
        mov     _ax,CPU386
        jnz     @@TestPROT      ; We have a 386

@@Check486:

; Distinguish between the i486 and Pentium by the ability to set the ID flag
; in the EFLAGS register. If the ID flag is set, then we can use the CPUID
; instruction to determine the final version of the chip. Otherwise we
; simply have an 80486.

        pushfd                  ; push original EFLAGS
        pop     eax             ; get original EFLAGS in eax
        mov     ecx,eax         ; save original EFLAGS in ecx
        or      eax,200000h     ; flip ID bit in EFLAGS
        push    eax             ; save for EFLAGS
        popfd                   ; copy to EFLAGS
        pushfd                  ; push EFLAGS
        pop     eax             ; get new EFLAGS value
        xor     eax,ecx
        mov     _ax,CPU486
        jz      @@TestProt      ; We have an old i486 (flag would not set)

; Now execute the CPUID instruction to determine the vendor, family, model
; and stepping (some new 486's answer to the CPUID test also).

        mov     eax,1           ; set up for CPUID instruction
;       cpuid
db      0Fh, 0A2h               ; Opcodes for CPUID instruction
        and     eax,0F00H       ; mask everything but family
        shr     eax,8
        cmp     al,5
        mov     _ax,CPU486
        jl      @@TestProt      ; We have an i486
        mov     _ax,CPU586      ; We have a Pentium(tm)

@@TestPROT:
        smsw    cx              ; protected? machine status -> cx
        ror     cx,1            ; protection bit -> carry flag
        jnc     @@Done          ; Real mode if no carry
        inc     _ax             ; Protected: return value + 1

@@Done:
        pop     _bp             ; Restore bp
        pop     _bx
        ret                     ; We are done

procenddll16        _SV_queryCpu

endcodeseg  cpu

        END                     ; End of module

