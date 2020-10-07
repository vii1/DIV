.386p
                NAME    divasm
DGROUP          GROUP   CONST,CONST2,_DATA,_BSS
_TEXT           SEGMENT PARA PUBLIC USE32 'CODE'
                ASSUME  CS:_TEXT ,DS:DGROUP,SS:DGROUP
                PUBLIC  memcpyb_,call_,get_t_

; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
; //      Mขdulo de acceso a video
; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
;
; #include "global.h"
;
; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
; //      Memcpy con rep movsb
; //อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ
;
; void memcpyb(byte * d, byte * s, int n) {
memcpyb_:       PUSH    ECX
                PUSH    ESI
                PUSH    EDI

;   memcpy(d,s,n);
                MOV     ECX,EBX
                MOV     ESI,EDX
                MOV     EDI,EAX
                ;REPNE   MOVSB
                REP MOVSB

; }
;
                POP     EDI
                POP     ESI
                POP     ECX
                RET

call_:          JMP NEAR PTR EAX

get_t_:         pusha
                push cs
                pop eax
                and eax,0fffffffch
                mov kk2,eax
                lea eax,get_t2
                mov kk1,eax
                call pword ptr [kk1]
                popa
                ret

get_t2:         mov ecx,10h
                db 0fh,032h
                retf

_TEXT           ENDS

CONST           SEGMENT DWORD PUBLIC USE32 'DATA'
CONST           ENDS

CONST2          SEGMENT DWORD PUBLIC USE32 'DATA'
CONST2          ENDS

_DATA           SEGMENT DWORD PUBLIC USE32 'DATA'
kk1             dd 0
kk2             dd 0
_DATA           ENDS

_BSS            SEGMENT PARA PUBLIC USE32 'BSS'
_BSS            ENDS

                END
