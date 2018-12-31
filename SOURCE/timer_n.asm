; Timer interrupt handler

                CPU	386

                SEGMENT	code	CLASS=CODE	ALIGN=2 USE32

                GLOBAL	_timer_code_lock_start
                GLOBAL	_timer_code_lock_end
                GLOBAL	_timer_oldvect
                GLOBAL	_timer_count
                GLOBAL	_timer_systemcount
                GLOBAL	_timer_frequency
                GLOBAL	_timer_ds
                GLOBAL	_timer_function
                GLOBAL	timer_handler_

_timer_code_lock_start:

                align   4

_timer_ds       dw      0
_timer_oldvect:  ;df      0
                dd      0
                dw      0
_timer_count    dd      0
_timer_systemcount dw   0
_timer_frequency dw     0
_timer_function dd      0

                align   4


timer_handler_: pushad
                push    DS
                push    ES
                mov     DS, [CS:_timer_ds]
                mov     ES, [CS:_timer_ds]
                sti
                mov     AL, 20h
                out     20h, AL
                inc     DWORD [_timer_count]
                call    DWORD [_timer_function]
                mov     AX, [_timer_frequency]
                add     [_timer_systemcount], AX
                jc      timer_callold
                pop     ES
                pop     DS
                popad
                iretd
timer_callold:  pop     ES
                pop     DS
                popad
                jmp     [CS:_timer_oldvect]

_timer_code_lock_end:

                GROUP	DGROUP	code
                end
