
;=======================================================================
;   Cabecera de todos los EXE generados por DIV Games Studio 2.0+
;
;   Existen dos versiones: Para 386+ o Pentium+
;=======================================================================

;== Code ===============================================================

        	TITLE   DIV_STUB
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

code      segment para 'CODE'     ;definici¢n del segmento de c¢digo

          assume cs:code, ds:data, ss:stackseg

exec      proc far

          mov  ax,data            ;cargar direcci¢n de segmento del segm.
          mov  ds,ax              ;de datos al registro DS

          call setfree            ;liberar memoria no ocupada

          call get_cpuid          ;obtener el tipo de procesador

          cmp  cpu_type,5         ;si no es adecuado, emitir un error

          mov  dx,offset no586    ;cambiar esto para variar la versi¢n
          jae  cont0
          jmp  error_m

;	copy command line into bottom of stack
;

cont0:   	mov	di,es			          ; point es to PSP
         	mov	ds,di			          ; ...
         	mov	di,81H			        ; DOS command buffer __psp:80
         	mov	cl,-1[di]		        ; get length of command
         	mov	ch,0
         	cld				              ; set direction forward
         	mov	al,' '
         	repe scasb
         	lea	si,-1[di]

         	mov	dx,data
         	mov	es,dx			          ; es:di is destination
         	mov di,offset buffer
         	mov	es:par,di	          ; stash lpCmdLine pointer
         	je	noparm
         	inc	cx
         	rep	movsb
noparm:   sub	al,al
         	stosb				            ; store NULLCHAR
         	mov	al,0			          ; assume no pgm name
         	stosb				            ; . . .
         	dec	di			            ; back up pointer 1
;
;	get DOS version number
;
        	mov	ah,30h
        	int	21h
        	mov	cx,di			          ; remember address of pgm name
        	cmp	al,3			          ; if DOS version 3 or higher
        	jb	nopgmname		        ; then
;
;	copy the program name into bottom of stack
;
        	mov	ds,ds:2ch		        ; get segment addr of environment area
        	sub	si,si			          ; offset 0
        	xor	bp,bp			          ; no87 not present!
L0:	      mov	ax,[si] 		        ; get first part of environment var
        	or	ax,2020H		        ; lower case
        	cmp	ax,"on" 		        ; if first part is 'NO'
        	jne	L1			            ; - then
        	mov	ax,2[si]		        ; - get second part
        	cmp	ax,"78" 		        ; - if second part is '87'
        	jne	L1			            ; - then
        	inc	bp			            ; - - set bp to indicate NO87
L1:	      cmp	byte ptr [si],0 	  ; end of string ?
        	lodsb
        	jne	L1			            ; until end of string
        	cmp	byte ptr [si],0 	  ; end of all strings ?
        	jne	L0			            ; if not, then skip next string
        	lodsb
        	inc	si			            ; - point to program name
        	inc	si			            ; - . . .
L2:	      cmp	byte ptr [si],0 	  ; - end of pgm name ?
        	movsb				            ; - copy a byte
        	jne	L2			            ; - until end of pgm name
nopgmname:				                ; endif
        	mov	es:prg,cx  	        ; stash LpPgmName pointer

          mov ax,data
          mov ds,ax

          mov  dx,offset prgname  ;direcci¢n de offset del nombre del programa
          mov  si,prg             ;direcci¢n de offset de la l¡nea de comandos
          call exeprg             ;llamar programa

          jnc  exit_ok            ;no hay eror ---> finaliza

          mov  dx,offset err0
          mov  ah,09h
          int  21h

          mov  dx,offset prgname
          mov  ah,09h
          int  21h

          mov  dx,offset err1
          mov  ah,09h
          int  21h

          mov  ax,4C00h
          int  21h

error_m:  push dx
          mov dx,offset err0
          mov ah,09h
          int 21h
          pop dx
          mov  ah,09h
          int  21h
          mov  dx,offset err2
          mov  ah,09h
          int  21h
          mov  ax,4C00h
          int  21h

exit_ok:  cmp al,26
          je  no_error

          mov ax,3
          int 10h

no_error: mov  ax,4C00h
          int  21h

exec      endp

;-- SETFREE: liberar el espacio de memoria no ocupado ------------------
;-- Entrada  : ES = Direcci¢n del PSP
;-- Salida   : no tiene
;-- Registro : AX, BX, CL y FLAGS se modifican
;-- Info     : Como el segmento de Stack siempre es el £ltimo segmento
;              del archivo EXE, ES:0000 apunta al inicio y SS:SP
;              al final del programa en memoria. Con ello se puede
;              calcular la longitud del programa

setfree   proc near

          mov  bx,ss              ;restar entre s¡ las dos direcciones
          mov  ax,es              ;de segmento. De ello resulta el
          sub  bx,ax              ;n£mero de p rrafos del PSP hasta
                                  ;el Stack
          mov  ax,sp              ;ya que el puntero del Stack se encuentra
          add  ax,15              ;al final del segmento de Stack, indica su
          mov  cl,4               ;Sumar la longitud del Stack
          shr  ax,cl
          add  bx,ax              ;a¤adir a la longitud actual

          mov  ah,4ah             ;pasar nueva longitud al DOS
          int  21h

          ret                     ;volver al invocador

setfree   endp

;-- EXEPRG: llamar otro programa ---------------------------------------
;-- Entrada  : DS:DX = direcci¢n del nombre del programa
;--            DS:SI = direcci¢n de la l¡nea de comandos
;-- Salida   : Bandera de Carry = 1 : Error (AX = c¢digo de error)
;-- Registro : AX y FLAGS se modifican
;-- Info     : nombre de programa y l¡nea de comandos deben estar presentes
;--            como cadena ASCII y deben ser terminados por el c¢d. ASCII 0

exeprg    proc near

          ;guardar l¡nea de comandos pasada en un buffer de comandos propio --
          ;y ontar los caracteres                                     --

          push bx                 ;guardar todos los registros que
          push cx                 ;son destruidos por la llamada a la
          push dx                 ;instrucci¢n DOS EXEC
          push di
          push si
          push bp
          push ds
          push es

          mov  di,offset comline+1;direcci¢n de los caracteres en la l¡nea de comandos
          push ds                 ;DS al Stack
          pop  es                 ;recuperar como ES
          xor  bl,bl              ;fijar contador de caracteres a 0
copypara: lodsb                   ;leer un car cter
          or   al,al              ;este es el c¢digo NUL (Fin)
          je   copyende           ;SI --> copiado suficiente
          stosb                   ;guardar en el nuevo buffer
          inc  bl                 ;incrementar contador de caracteres
          cmp  bl,126             ;¨alcanzado el m ximo?
          jne  copypara           ;NO ---> seguir

copyende: mov  comline,bl         ;guardar n£mero de caracteres
          mov  byte ptr [di],13   ;terminar la l¡nea de comandos

          mov  merkss,ss          ;SS y SP se han de guardar en variables
          mov  merksp,sp          ;en el segmento de c¢digo

          mov  bx,offset parblock ;ES:BX apunta al Parameter-Block
          mov  ax,4B00h           ;n£mero de funci¢n para funci¢n EXEC
          int  21h                ;llamar funci¢n DOS

          cli                     ;apagar interrupciones durante un instante
          mov  ss,merkss          ;reponer segmento de Stack y puntero
          mov  sp,merksp          ;de Stack a sus valores antiguos
          sti                     ;volver a activar interrupciones

          pop  es                 ;obtener de nuevo todos los registros del Stack
          pop  ds
          pop  bp
          pop  si
          pop  di
          pop  dx
          pop  cx
          pop  bx

          jc   exeend             ;¨Error? SI --> Final
          mov  ah,4dh             ;no hay error, obtener c¢digo de
          int  21h                ;fin del programa ejecutado

exeend:   ret                     ;volver al invocador

exeprg    endp

;-- GET_CPUID: Obtiene el tipo de procesador ---------------------------
;-- Entrada  : Ninguna
;-- Salida   : cpu_type

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
	mov     cpu_type, 0     ; turn on 8086/8088 flag
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
	mov     cpu_type, 2     ; turn on 80286 flag
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
	mov     cpu_type, 3     ; turn on 80386 CPU flag
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
	mov     cpu_type, 4     ; turn on 80486 CPU flag
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
	mov     cpu_type, 5     ; turn on Pentium CPU flag

end_get_cpuid:
	ret

get_cpuid endp

;== Datos ==============================================================

data      segment para 'DATA'     ;definici¢n del segmento de datos

parblock:  ;equ this word           ;bloque de par metros para la funci¢n EXEC
          dw 0                    ;mismo Environment-Block
          dw offset comline       ;direcci¢n de offset y de segmento
          dw seg data             ;de la l¡nea de comandos modificada
          dd 0                    ;no hay datos en PSP #1
          dd 0                    ;no hay datos en PSP #2

no586     db "Pentium$"
no386     db "Intel386$"

prgname   db "DIV32RUN.DLL",0,'$' ;nombre del programa a llamar
err0      db "Error: $"
err2      db "(tm) or greater processor "
err1      db "not found.$"

cpu_type  db ?

merkss    dw ?                    ;acoge SS durante la llamada de programa
merksp    dw ?                    ;acoge SP durante la llamada de programa

par       dw ?
prg       dw ?

buffer    db 256 dup (?)

comline   db 128 dup (?)          ;acoge la l¡nea de comandos modificada

data      ends                    ;fin del segm. de datos

;== Stack ==============================================================

          ;--- el segmento de Stack se ha de colocar aqu¡, al final
          ;--- del archivo para que al final de la carga del programa
          ;--- est‚ detr s del segm. de datos y c¢digo en la memoria

stackseg  segment para stack 'STACK' ;Definici¢n del segm. de stack

          dw 256 dup (?)          ;el stack mide 256 Words

stackseg  ends                    ;fin del segmento de stack

;== Fin ================================================================

code      ends                    ;Fin del segemento de c¢digo
          end  exec               ;comenzar la ejecuci¢n con EXEC
