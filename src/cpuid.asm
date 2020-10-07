
	TITLE   CPUID
	DOSSEG
	.model  small
	.186

OPND32 MACRO op_code, op_erand
	db      66h     ; Force 32-bit operand size
  IFNB <op_code>
	db      op_code
    IFNB <op_erand>
	dd      op_erand; 32-bit immediate value
    ENDIF
  ENDIF
ENDM

_CPUID MACRO
	db      0fh     ; Hardcoded opcode for CPUID instruction
	db      0a2h
ENDM

FAMILY_MASK     equ     0f00h
FAMILY_SHIFT    equ     8
MODEL_MASK      equ     0f0h
MODEL_SHIFT     equ     4
STEPPING_MASK   equ     0fh

	.data
  PUBLIC _cpu_type

vendor_id       db      12 dup (?)
_cpu_type       db      0
modell          db      ?
stepping        db      ?
id_flag         db      0
intel_proc      db      0
feature_flags   dw      2 dup (0)

intel_id        db      "GenuineIntel"

	.code
  PUBLIC get_cpuid_

get_cpuid_ proc

  push ds
  push es
  push bp
  push si
  push di
  push dx
  push cx
  push bx
  push ax
  mov     ax, @data
	mov     ds, ax          ; set segment register
	mov     es, ax          ; set segment register
	pushf                   ; save for restoration at end
	call    get_cpuid
	popf
  pop ax
  pop bx
  pop cx
  pop dx
  pop di
  pop si
  pop bp
  pop es
  pop ds
  ret

get_cpuid_ endp

get_cpuid proc
;
;       This procedure determines the type of CPU in a system
;       and sets the cpu_type variable with the appropriate
;       value.
;       All registers are used by this procedure, none are preserved.

;       Intel 8086 CPU check
;       Bits 12-15 of the FLAGS register are always set on the
;       8086 processor.
;
check_8086:
	pushf                   ; push original FLAGS
	pop     ax              ; get original FLAGS
	mov     cx, ax          ; save original FLAGS
	and     ax, 0fffh       ; clear bits 12-15 in FLAGS
	push    ax              ; save new FLAGS value on stack
	popf                    ; replace current FLAGS value
	pushf                   ; get new FLAGS
	pop     ax              ; store new FLAGS in AX
	and     ax, 0f000h      ; if bits 12-15 are set, then CPU
	cmp     ax, 0f000h      ;   is an 8086/8088
	mov     _cpu_type, 0     ; turn on 8086/8088 flag
	jne     check_80286     ; jump if CPU is not 8086/8088
	jmp     end_get_cpuid
;       Intel 286 CPU check
;       Bits 12-15 of the FLAGS register are always clear on the
;       Intel 286 processor in real-address mode.
;
check_80286:
	or      cx, 0f000h      ; try to set bits 12-15
	push    cx              ; save new FLAGS value on stack
	popf                    ; replace current FLAGS value
	pushf                   ; get new FLAGS
	pop     ax              ; store new FLAGS in AX
	and     ax, 0f000h      ; if bits 12-15 clear, CPU=80286
	mov     _cpu_type, 2     ; turn on 80286 flag
	jnz     check_80386     ; if no bits set, CPU is 80286
	jmp     end_get_cpuid
;       Intel386 CPU check
;       The AC bit, bit #18, is a new bit introduced in the EFLAGS
;       register on the Intel486 DX CPU to generate alignment faults.
;       This bit cannot be set on the Intel386 CPU.
;
check_80386:
;       It is now safe to use 32-bit opcode/operands
	mov     bx, sp          ; save current stack pointer to align
	and     sp, not 3       ; align stack to avoid AC fault
	OPND32
	pushf                   ; push original EFLAGS
	OPND32
	pop     ax              ; get original EFLAGS
	OPND32
	mov     cx, ax          ; save original EFLAGS
	OPND32  35h, 40000h     ; flip AC bit in EFLAGS
	OPND32
	push    ax              ; save new EFLAGS value on stack
	OPND32
	popf                    ; replace current EFLAGS value
	OPND32
	pushf                   ; get new EFLAGS
	OPND32
	pop     ax              ; store new EFLAGS in EAX
	OPND32
	xor     ax, cx          ; can't toggle AC bit, CPU=80386
	mov     _cpu_type, 3     ; turn on 80386 CPU flag
	mov     sp, bx          ; restore original stack pointer
	jz      end_get_cpuid   ; jump if 80386 CPU
	and     sp, not 3       ; align stack to avoid AC fault
	OPND32
	push    cx
	OPND32
	popf                    ; restore AC bit in EFLAGS first
	mov     sp, bx          ; restore original stack pointer

;       Intel486 DX CPU, Intel487 SX NDP, and Intel486 SX CPU check
;       Checking for ability to set/clear ID flag (Bit 21) in EFLAGS
;       which indicates the presence of a processor
;       with the ability to use the CPUID instruction.
;
check_80486:
	mov     _cpu_type, 4     ; turn on 80486 CPU flag
	OPND32
	mov     ax, cx          ; get original EFLAGS
	OPND32  35h, 200000h    ; flip ID bit in EFLAGS
	OPND32
	push    ax              ; save new EFLAGS value on stack
	OPND32
	popf                    ; replace current EFLAGS value
	OPND32
	pushf                   ; get new EFLAGS
	OPND32
	pop     ax              ; store new EFLAGS in EAX
	OPND32
	xor     ax, cx          ; can't toggle ID bit,
	je      end_get_cpuid   ;   CPU=80486

;       Execute CPUID instruction to determine vendor, family,
;       model and stepping.
;
check_vendor:
	mov     id_flag, 1              ; set flag indicating use of CPUID inst.
	OPND32
	xor     ax, ax                  ; set up input for CPUID instruction
	_CPUID                           ; macro for CPUID instruction
	OPND32
	mov     word ptr vendor_id, bx  ; setup to test for vendor id
	OPND32
	mov     word ptr vendor_id[+4], dx
	OPND32
	mov     word ptr vendor_id[+8], cx
	mov     si, offset vendor_id
	mov     di, offset intel_id
	mov     cx, length intel_id
compare:
	repe    cmpsb                   ; compare vendor id to "GenuineIntel"
	or      cx, cx
	jnz     end_get_cpuid           ; if not zero, not an Intel CPU,

intel_processor:
	mov     intel_proc, 1

cpuid_data:
	OPND32
	cmp     ax, 1                   ; make sure 1 is a valid input
					; value for CPUID
	jl      end_get_cpuid           ; if not, jump to end
	OPND32
	xor     ax, ax                  ; otherwise, use as input to CPUID
	OPND32
	inc     ax                      ; and get stepping, model and family
	_CPUID
	mov     stepping, al
	and     stepping, STEPPING_MASK ; isolate stepping info

	and     al, MODEL_MASK          ; isolate model info
	shr     al, MODEL_SHIFT
	mov     modell, al

	and     ax, FAMILY_MASK         ; mask everything but family
	shr     ax, FAMILY_SHIFT
	mov     _cpu_type, al            ; set cpu_type with family

	OPND32
	mov     feature_flags, dx       ; save feature flag data

end_get_cpuid:
	ret
get_cpuid endp

	end
