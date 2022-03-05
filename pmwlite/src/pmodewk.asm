; PMODE/W DPMI/VCPI/XMS/raw protected mode interface kernel.
; Copyright (c) 1994, 1996, Tran (a.k.a. Thomas Pytel).

public  _pm_selectors, _pm_pagetables, _pm_rmstacklen, _pm_rmstacks
public  _pm_callbacks, _pm_pmstacklen, _pm_pmstacks, _pm_mode, _pm_pampagemax
public  _pm_maxextmem, _pm_options

public  pmstackbase, pmstacktop, rmstackbase, rmstacktop

public  _pm_info, _pm_init, _pm_cleanup

.386p
locals
PMODE_TEXT      segment dword public use16 'CODE'
assume  cs:PMODE_TEXT, ds:PMODE_TEXT

off             equ     offset

INT3108FUNC     = 1                     ; comment out to disable
EXCEPTIONS      = 1                     ; comment out to disable

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; DATA
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
SELCODE         = 08h                   ; PMODE_TEXT code selector
SELDATA         = 10h                   ; selector of entire memory space
SELZERO         = 18h                   ; callback DS selector
SELCALLBACKDS   = 20h                   ; callback DS selector
SELREAL         = SELDATA               ; real mode attributes selector
SELVCPICODE     = 28h                   ; VCPI call code selector
SEL40           = 40h                   ; selector 40h of BIOS data table
SELVCPITSS      = 48h                   ; TSS selector for VCPI

SYSSELECTORS    = 10                    ; number of system selectors in GDT
FREESELECTORS   = SYSSELECTORS+4        ; base of free selectors

align 4
vcpistrucaddx   dd      off vcpi_cr3    ; VCPI switch structure linear address
oldint15vector  dw      off int15       ; preserved INT 15h vector
                dw      PMODE_TEXT
int3vector      dd      off intrmatrix+3; protected mode INT 3 vector
                dw      SELCODE

selzero         dw      SELZERO         ; for immediate segreg loading
selcallbackds   dw      SELCALLBACKDS   ; for immediate segreg loading
seldata         dw      SELDATA         ; for immediate segreg loading

dpmiepmode      dd      ?               ; DPMI enter pmode addx
codebase        dd      ?               ; PMODE_TEXT linear address
vcpiswitchstack dd      ?               ; VCPI temporary mode switch stack

pmstacklen      dd      ?               ; protected mode stack length in bytes
pmstackbase     dd      ?               ; bottom of protected mode stack area
pmstacktop      dd      ?               ; top of protected mode stack area
callbackbase    dd      ?               ; base of real mode callbacks
callbackseg     dw      ?               ; segment of callbacks

rawextmemused   dw      0               ; raw extended memory used in K

int1ccallback   dd      ?               ; INT 1Ch callback address          |
int23callback   dd      ?               ; INT 23h callback address          |
int24callback   dd      ?               ; INT 24h callback address          |

rmstackbase     dw      ?               ; bottom of real mode stack area
rmstacktop      dw      ?               ; top of real mode stack area

pmstackesp      dd      ?               ; for functions 0300h, 0301h, 0302h
pmstackss       dw      ?               ;

gdtfree         dw      8*FREESELECTORS ; base of free selectors
gdtseg          dw      ?               ; segment of GDT
gdtlimit        dw      ?               ; GDT limit                          |
gdtbase         dd      ?               ; GDT base                           |

idtlimit        dw      7ffh            ; IDT limit                         |
idtbase         dd      ?               ; IDT base                          |
rmidtlimit      dw      3ffh            ; real mode IDT limit                |
rmidtbase       dd      0               ; real mode IDT base                 |

rmtopmswrout    dw      off v_rmtopmsw  ; addx of real to protected routine
pmtormswrout    dd      off v_pmtormsw  ; addx of protected to real routine

processortype   db      ?               ; processor type                     |
pmodetype       db      2               ; protected mode type                |
picslave        db      70h             ; PIC slave base interrupt          |
picmaster       db      8               ; PIC master base interrupt         |

defirqintnum    db      08h,09h,0ah,0bh,0ch,0dh,0eh,0fh
                db      70h,71h,72h,73h,74h,75h,76h,77h

cleanuptype     db      6
pagetablenum    db      ?
oldpicmasks     dw      ?

pagetablebase   dd      ?               ; base of page table area
pagetabletop    dd      ?               ; top of page table area
pagetablefree   dd      ?
pagetablepages  dd      ?

_pm_pagetables  db      ?               ; number of page tables under VCPI  |
_pm_selectors   dw      ?               ; max selectors under VCPI/XMS/raw  |
_pm_rmstacklen  dw      ?               ; real mode stack length, in para   |
_pm_pmstacklen  dw      ?               ; protected mode stack length, para |
_pm_rmstacks    db      ?               ; real mode stack nesting           |
_pm_pmstacks    db      ?               ; protected mode stack nesting      |
_pm_callbacks   db      ?               ; number of real mode callbacks     |
_pm_mode        db      ?               ; mode bits                         |
_pm_pampagemax  db      ?               ; physical address mapping pages max|
                db      ?,?             ;                                   |
_pm_options     db      ?               ; PMODE/W option bits               |
_pm_maxextmem   dd      ?               ; maximum extended mem to allocate  |

tempd0          label   dword           ; temporary variables                |
tempw0          label   word            ;                                    |
tempb0          db      ?               ;                                    |
tempb1          db      ?               ;                                    |
tempw1          label   word            ;                                    |
tempb2          db      ?               ;                                    |
tempb3          db      ?               ;                                    |
tempd1          label   dword           ;                                    |
tempw2          label   word            ;                                    |
tempb4          db      ?               ;                                    |
tempb5          db      ?               ;                                    |
tempw3          label   word            ;                                    |
tempb6          db      ?               ;                                    |
tempb7          db      ?               ;                                    |

extmemsize      dd      0
extmembase      dd      ?
extmemlast      dd      ?

ifdef   INT3108FUNC
pampagebase     dd      ?
pampagetop      dd      ?
pampagemask     dw      ?
endif

xms_callip      dw      ?               ; XMS driver offset                 |
xms_callcs      dw      ?               ; XMS driver segment                |
xms_handle      dw      ?
xms_mem         dw      0

irqhookmap      dw      0

vcpi_memused    db      0, ?            ; bit 0: XMS used

vcpi_cr3        dd      ?               ; VCPI CR3 value for protected mode  |
vcpi_gdtaddx    dd      off gdtlimit    ; linear addx of GDT limit and base  |
vcpi_idtaddx    dd      off idtlimit    ; linear addx of IDT limit and base  |
vcpi_selldt     dw      0               ; LDT selector for protected mode    |
vcpi_seltss     dw      SELVCPITSS      ; TSS selector for protected mode    |
vcpi_eip        dd      off v_rmtopmswpm; destination EIP in protected mode  |
vcpi_cs         dw      SELCODE         ; destination CS in protected mode   |

vcpi_calleip    dd      ?               ; VCPI protected mode call offset   |
vcpi_callcs     dw      SELVCPICODE     ; VCPI protected mode call selector |

inittbl         dw      r_init,x_init,v_init,d_init
cleanuptbl      dw      cleanup0,cleanup1,cleanup2,cleanup3

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int31functbl    dw      0900h,0901h,0902h
                dw      0000h,0001h,0002h,0003h,0006h,0007h,0008h,0009h
                dw      0200h,0201h,0204h,0205h
                dw      0305h,0306h,0400h
                dw      000ah,000bh,000ch
                dw      0100h,0101h,0102h
                dw      0300h,0301h,0302h
                dw      0303h,0304h
                dw      0500h,0501h,0502h,0503h
                dw      0600h,0601h,0702h,0703h

ifdef   INT3108FUNC
                dw      0800h,0801h
endif

ifdef   EXCEPTIONS
                dw      0202h,0203h
endif

                dw      05ffh

INT31FUNCNUM    = ($ - int31functbl) / 2

int31routtbl    dw      int310900,int310901,int310902
                dw      int310000,int310001,int310002,int310003
                dw      int310006,int310007,int310008,int310009
                dw      int310200,int310201,int310204,int310205
                dw      int310305,int310306,int310400
                dw      int31000a,int31000b,int31000c
                dw      int310100,int310101,int310102
                dw      int310300,int310301,int310302
                dw      int310303,int310304
int3105routtbl  dw      int310500nomem,int31fail,int31fail,int31fail
                dw      int31ok,int31ok,int31ok,int31ok

ifdef   INT3108FUNC
int3108routtbl  dw      int31ok,int31ok
endif

ifdef   EXCEPTIONS
                dw      int310202,int310203
endif

                dw      int3105ff

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
ifdef   EXCEPTIONS

align   2
exceptionivect  dw      intrmatrix + 00h, 0, SELCODE
                dw      intrmatrix + 01h, 0, SELCODE
                dw      intrmatrix + 02h, 0, SELCODE
                dw      intrmatrix + 03h, 0, SELCODE
                dw      intrmatrix + 04h, 0, SELCODE
                dw      intrmatrix + 05h, 0, SELCODE
                dw      intrmatrix + 06h, 0, SELCODE
                dw      intrmatrix + 07h, 0, SELCODE
                dw      intrmatrix + 08h, 0, SELCODE
                dw      intrmatrix + 09h, 0, SELCODE
                dw      intrmatrix + 0ah, 0, SELCODE
                dw      intrmatrix + 0bh, 0, SELCODE
                dw      intrmatrix + 0ch, 0, SELCODE
                dw      intrmatrix + 0dh, 0, SELCODE
                dw      intrmatrix + 0eh, 0, SELCODE

exceptionevect  dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE
                dw      exception_def, 0, SELCODE

exceptionecsize db      0,0,0,0,0,0,0,0,4,0,4,4,4,4,4

;úúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúú
abortrmstacktop dw      ?               ; top of real mode stack for abort

;úúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúú
aborts_hexctbl  db      '0123456789ABCDEF'

aborts_pmwabort db	'[PMODE/W]: Abort: ',0

aborts_xception db	'Exception: ',0
aborts_callback db      'No PM stack for callback!',0
aborts_IRQtoPM  db      'No PM stack for IRQ redirection!',0
aborts_INTtoRM  db      'No RM stack for INT redirection!',0
aborts_INT3103  db      'No PM stack for INT 31h AH = 03h!',0

aborts_xerrcode db      'Error code: ',0
aborts_xeax     db      'EAX: ',0
aborts_xebx     db      'EBX: ',0
aborts_xecx     db      'ECX: ',0
aborts_xedx     db      'EDX: ',0
aborts_xesi     db      'ESI: ',0
aborts_xedi     db      'EDI: ',0
aborts_xebp     db      'EBP: ',0
aborts_xesp     db      'ESP: ',0
aborts_xds      db      'DS: ',0
aborts_xes      db      'ES: ',0
aborts_xfs      db      'FS: ',0
aborts_xgs      db      'GS: ',0
aborts_xss      db      'SS: ',0
aborts_xcs      db      'CS: ',0
aborts_xeip     db      'EIP: ',0
aborts_xeflags  db      'EFLAGS: ',0
aborts_xcr0     db      'CR0: ',0
aborts_xcr2     db      'CR2: ',0

;úúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúú
aborts_tbl      dw      aborts_xception, aborts_callback, aborts_IRQtoPM
                dw      aborts_INTtoRM, aborts_INT3103

endif

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; DETECT/INIT CODE
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

intrmatrix:                             ; INT redirectors for all INTs

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Get protected mode info
; Out:
;   AX = return code:
;     0000h = successful
;     0001h = no 80386+ detected
;     0002h = system already in protected mode and no VCPI or DPMI found
;     0003h = DPMI - host is not 32bit
;   CF = set on error, if no error:
;     BX = number of paragraphs needed for protected mode data (may be 0)
;     CL = processor type:
;       02h = 80286
;       03h = 80386
;       04h = 80486
;       05h = 80586
;       06h-FFh = reserved for future use
;     CH = protected mode type:
;       00h = raw
;       01h = XMS
;       02h = VCPI
;       03h = DPMI
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
.8086
_pm_info:
        push dx si di bp ds es bx cx    ; preserve registers

        push cs                         ; DS = CS (PMODE_TEXT)
        pop ds

        call @@detect_processor         ; get processor type

        mov ax,1                        ; error code in case no processor 386+
        cmp cl,3                        ; is processor 386+?
        jae short @@04

        jmp @@infofail                  ; if no, no VCPI

.386p
@@04:
        mov processortype,cl            ; store processor type

        mov ax,4300h                    ; chek for XMS
        int 2fh
        cmp al,80h
        jne short @@01

        push es                         ; preserve ES, INT 2Fh destroys it

        mov ax,4310h                    ; get XMS driver address
        int 2fh

        mov xms_callip,bx               ; store XMS driver address
        mov xms_callcs,es

        mov ah,30h                      ; crap fix for Win95 HIMEM.SYS
        int 21h

        mov ah,8
        call dword ptr xms_callip
        mov xms_mem,ax

        pop es                          ; restore ES (buffer segment)

@@01:
        test _pm_mode,1                 ; check order of DPMI/VCPI detection
        jz short @@infof0

        call @@detect_VCPI              ; check for VCPI first
        call @@detect_DPMI              ; check for DPMI second
        jmp short @@infof2              ; neither found, go on to XMS check

@@infof0:
        call @@detect_DPMI              ; check for DPMI first
        call @@detect_VCPI              ; check for VCPI second

;-----------------------------------------------------------------------------
@@infof2:
        smsw ax                         ; AX = machine status word
        and ax,1                        ; is system in protected mode?
        mov al,2                        ; error code in case protected mode
        jnz short @@infofail            ; if in protected mode, fail

        cmp xms_mem,0                   ; if XMS present, pmode type is XMS
        seta ch

        mov bx,80h                      ; BX = memory requirement (IDT)

;-----------------------------------------------------------------------------
@@infof1:
        movzx ax,_pm_rmstacks           ; size of real mode stack area
        imul ax,_pm_rmstacklen
        add bx,ax

        movzx ax,_pm_pmstacks           ; size of protected mode stack area
        imul ax,_pm_pmstacklen
        add bx,ax

        movzx ax,_pm_callbacks          ; size of callbacks
        imul ax,25
        add ax,0fh
        shr ax,4
        add bx,ax

        mov ax,_pm_selectors            ; size of GDT
        add ax,1+FREESELECTORS+1
        shr ax,1
        add bx,ax

        jmp short @@infook              ; go to done ok

;-----------------------------------------------------------------------------
@@infofail:
        pop cx bx                       ; restore BX and CX
        stc                             ; carry set, failed
        jmp short @@infodone

;-----------------------------------------------------------------------------
@@infook:
        mov cl,processortype
        mov pmodetype,ch                ; store pmode type

        add sp,4                        ; skip BX and CX on stack
        xor ax,ax                       ; success code, also clear carry flag

;-----------------------------------------------------------------------------
@@infodone:
        pop es ds bp di si dx           ; restore other registers
        retf                            ; return

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
@@detect_DPMI:                          ; detect a DPMI host
        pop bp                          ; pop return address from stack

        mov ax,1687h                    ; check for DPMI
        int 2fh

        or ax,ax                        ; DPMI present?
        jnz short @@detect_DPMIdone     ; if no, exit routine

        mov al,3                        ; error code in case DPMI not 32bit
        test bl,1                       ; is DPMI 32bit?
        jz @@infofail                   ; if no, fail

        mov al,1                        ; error code in case no processor 386+
        cmp cl,3                        ; is processor 386+? (redundant)
        jb @@infofail                   ; if no, fail

        mov word ptr dpmiepmode[0],di   ; store DPMI initial mode switch addx
        mov word ptr dpmiepmode[2],es

        mov bx,si                       ; BX = number of paragraphs needed
        mov ch,3                        ; pmode type is 3 (DPMI)

        jmp @@infook                    ; go to done ok

@@detect_DPMIdone:
        jmp bp                          ; return to calling routine

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
@@detect_VCPI:                          ; detect a VCPI server
        pop bp                          ; pop return address from stack

        xor ax,ax                       ; get INT 67h vector
        mov es,ax
        mov ax,es:[67h*4]
        or ax,es:[67h*4+2]              ; is vector NULL
        jz short @@detect_VCPIdone      ; if yes, no VCPI

        mov ax,0de00h                   ; call VCPI installation check
        int 67h
        or ah,ah                        ; AH returned as 0?
        jnz short @@detect_VCPIdone     ; if no, no VCPI

        mov dx,xms_mem
        mov cx,dx

        jcxz short @@02

        mov ah,9
        call dword ptr xms_callip
        mov di,dx

.errnz  ($-off intrmatrix) lt 100h
rmirqvector     = $                     ; backed up real mode IRQ vectors

        or ax,ax
        jnz short @@02

        xor cx,cx

@@02:
        mov ax,0de03h
        int 67h

        movzx esi,cx
        shr esi,2
        lea esi,[esi+edx+3ffh]
        shr esi,10
        inc si

        jcxz short @@03

        mov dx,di
        mov ah,0ah
        call dword ptr xms_callip

@@03:
        movzx bx,_pm_pagetables         ; BX = VCPI page table memory needed
        cmp bx,si
        jb short @@00

        mov bx,si

@@00:
        mov pagetablenum,bl

        xchg bl,bh                      ; 100h paragraphs per page table
        add bx,100h+0ffh+7+80h          ; + page dir + align buf + TSS + IDT

.errnz  ($-rmirqvector) lt 4*10h
passupintregs   = $

        mov ch,2                        ; pmode type is 2 (VCPI)

        jmp @@infof1                    ; go to figure other memory needed

@@detect_VCPIdone:
        jmp bp                          ; return to calling routine

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
.8086
@@detect_processor:                     ; get processor: 286, 386, 486, or 586
        xor cl,cl                       ; processor type 0 in case of exit

        pushf                           ; transfer FLAGS to BX
        pop bx

        mov ax,bx                       ; try to clear high 4 bits of FLAGS
        and ah,0fh

        push ax                         ; transfer AX to FLAGS
        popf
        pushf                           ; transfer FLAGS back to AX
        pop ax

        and ah,0f0h                     ; isolate high 4 bits
        cmp ah,0f0h
        je short @@detect_processordone ; if bits are set, CPU is 8086/8

        mov cl,2                        ; processor type 2 in case of exit

        or bh,0f0h                      ; try to set high 4 bits of FLAGS

        push bx                         ; transfer BX to FLAGS
        popf
        pushf                           ; transfer FLAGS to AX
        pop ax

        and ah,0f0h                     ; isolate high 4 bits
        jz short @@detect_processordone ; if bits are not set, CPU is 80286

.386p
        inc cx                          ; processor type 3 in case of exit

        push eax ebx                    ; preserve 32bit registers

        pushfd                          ; transfer EFLAGS to EBX
        pop ebx

        mov eax,ebx                     ; try to flip AC bit in EFLAGS
        xor eax,40000h

        push eax                        ; transfer EAX to EFLAGS
        popfd
        pushfd                          ; transfer EFLAGS back to EAX
        pop eax

        xor eax,ebx                     ; AC bit fliped?
        jz short @@detect_processordone2; if no, CPU is 386

        inc cx                          ; processor type 4 in case of exit

        mov eax,ebx                     ; try to flip ID bit in EFLAGS
        xor eax,200000h

        push eax                        ; transfer EAX to EFLAGS
        popfd
        pushfd                          ; transfer EFLAGS back to EAX
        pop eax

        xor eax,ebx                     ; ID bit fliped?
        jz short @@detect_processordone2; if no, CPU is 486

.586p
        push ecx edx

        mov eax,1                       ; get CPU features
        cpuid

        and ah,0fh

        pop edx ecx

        mov cl,ah                       ; processor type from CPUID

@@detect_processordone2:
        pop ebx eax                     ; restore 32bit registers

@@detect_processordone:
        ret                             ; return

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Initialize protected mode
; In:
;   ES = real mode segment for protected mode data (ignored if not needed)
; Out:
;   AX = return code:
;     0004h = could not enable A20 gate
;     0005h = DPMI - could not enter 32bit protected mode
;     0006h = DPMI - could not allocate needed selectors
;   CF = set on error, if no error:
;     ESP = high word clear
;     CS = 16bit selector for real mode CS with limit of 64k
;     SS = selector for real mode SS with limit of 64k
;     DS = selector for real mode DS with limit of 64k
;     ES = selector for PSP with limit of 100h
;     FS = 0 (NULL selector)
;     GS = 0 (NULL selector)
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
.386p
_pm_init:
        pushad
        push ds
        mov bp,sp
        cld

        push cs                         ; DS = PMODE_TEXT
        pop ds

        call getrmirqvectors

        mov eax,PMODE_TEXT              ; set base addx of PMODE_TEXT

.errnz  ($-passupintregs) lt 32h+40h
vcpisstacktop:                          ; top of VCPI mode switch stack

        shl eax,4
        mov codebase,eax

        add vcpi_gdtaddx,eax            ; adjust addresses for VCPI structure
        add vcpi_idtaddx,eax
        add vcpistrucaddx,eax

        movzx bx,pmodetype              ; jump to appropriate init code
        shl bx,1
        jmp inittbl[bx]

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
d_init:                                 ; DPMI protected mode init
        call backupints

        pop ds                          ; get original caller DS from stack
        mov ax,1                        ; enter DPMI protected mode
        call cs:dpmiepmode
        push ds                         ; put DS back onto stack
        jnc short dvxr_init             ; error? if not, go on with init

        mov bx,6                        ; error entering protected mode, set
        cmp ax,8011h                    ;  error code and abort
        stc
        je short init_done
        dec bx                          ; error code 5, not 6

;-----------------------------------------------------------------------------
init_done:                              ; return with return code
        mov [bp+30],bx
        pop ds
        popad
        retf

.errnz  ($-vcpisstacktop) lt 44

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
dvxr_init:                              ; DPMI/VCPI/XMS/raw common init tail
        mov cx,1                        ; allocate selector for return code
        xor ax,ax
        int 31h
        jnc short @@dvxr_initf0

        mov ah,4ch                      ; could not allocate selector
        int 21h                         ; terminate immediately

@@dvxr_initf0:
        mov bp,sp                       ; BP = SP for screwing with the stack
        mov bx,ax                       ; new code descriptor for return

        mov ax,0007h                    ; set base address of calling segment
        mov dx,[bp+36]
        mov cx,dx
        shl dx,4
        shr cx,12
        int 31h

        inc ax                          ; set selector limit of 64k
        xor cx,cx
        mov dx,0ffffh
        int 31h

        inc ax                          ; set selector type and access rights
        mov dx,cs                       ; get DPL from current CPL, and access
        lar cx,dx                       ;  rights and type from current CS
        shr cx,8                        ; type is already 16bit code segment
        int 31h

@@dvxr_initdone:
        mov [bp+36],bx                  ; store selector in return address
        xor bx,bx                       ; init successful, carry clear
        jmp init_done

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
backupints:
        push es

        push cs
        pop es

        xor cx,cx
        mov ds,cx                       ; back up real mode INT vectors

        mov di,off rmintvector
        xor si,si
        mov ch,2
        rep movs word ptr es:[di],word ptr ds:[si]

        in al,0a1h                      ; back up PIC masks
        mov ah,al
        in al,21h
        mov cs:oldpicmasks,ax

        pop es
        ret

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
extmemadjust:
        mov eax,cs:_pm_maxextmem
        cmp esi,eax
        jb short @@00

        and ah,0fch
        mov esi,eax

@@00:
        ret

rmintvector     label   dword           ; backed up real mode INT vectors

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
enablea20:                              ; hardware enable gate A20
        pushf
        push fs gs
        cli

        xor ax,ax                       ; set A20 test segments 0 and 0ffffh
        mov fs,ax
        dec ax
        mov gs,ax

        call enablea20test              ; is A20 already enabled?
        jz short @@enablea20done        ; if yes, done

        in al,92h                       ; PS/2 A20 enable
        or al,2
        jmp short $+2
        jmp short $+2
        jmp short $+2
        out 92h,al

        call enablea20test              ; is A20 enabled?
        jz short @@enablea20done        ; if yes, done

        call enablea20kbwait            ; AT A20 enable
        jnz short @@enablea20f0

        mov al,0d1h
        out 64h,al

        call enablea20kbwait
        jnz short @@enablea20f0

        mov al,0dfh
        out 60h,al

        call enablea20kbwait

@@enablea20f0:                          ; wait for A20 to enable
        mov cx,800h                     ; do 800h tries

@@enablea20l0:
        call enablea20test              ; is A20 enabled?
        jz @@enablea20done              ; if yes, done

        call enablea20readtick
        mov ah,al

@@enablea20l1:                          ; wait a single tick
        call enablea20readtick

        cmp al,ah
        je @@enablea20l1

        loop @@enablea20l0              ; loop for another try

        push bp
        mov bp,sp                       ; error, A20 did not enable
        mov bx,4                        ; error code 4
        mov word ptr [bp+8],off init_done       ; set init_done return address
        pop bp

@@enablea20done:
        pop gs fs
        popf
        ret

;-----------------------------------------------------------------------------
enablea20readtick:
        in al,40h                       ; get current tick counter

        jmp short $+2
        jmp short $+2
        jmp short $+2

        in al,40h

        ret

;-----------------------------------------------------------------------------
enablea20kbwait:                        ; wait for safe to write to 8042
        xor cx,cx

@@enablea20kbwaitl0:
        jmp short $+2
        jmp short $+2
        jmp short $+2

        in al,64h                       ; read 8042 status

        test al,2                       ; buffer full?
        loopnz @@enablea20kbwaitl0      ; if yes, loop

        ret

;-----------------------------------------------------------------------------
enablea20test:                          ; test for enabled A20
        mov al,fs:[0]                   ; get byte from 0:0
        mov ah,al                       ; preserve old byte
        not al                          ; modify byte
        xchg al,gs:[10h]                ; put modified byte to 0ffffh:10h
        cmp ah,fs:[0]                   ; set zero if byte at 0:0 not modified
        mov gs:[10h],al                 ; put back old byte at 0ffffh:10h

        ret                             ; return, zero if A20 enabled

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
getrmirqvectors:
        push ds es

        mov ax,870h

        push cs
        pop es
        xor cx,cx
        mov ds,cx

        mov di,off rmirqvector

@@00l:
        mov cl,ah
        mov si,cx
        shl si,2
        mov cl,2*8
        rep movs word ptr es:[di],word ptr ds:[si]

        shl ax,8
        jnz @@00l

        pop es ds
        ret

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
r_init:                                 ; raw protected mode init
        mov ah,88h                      ; how much extended memory free
        int 15h

        or ax,ax                        ; if none, done with raw init
        jz xr_init

        movzx esi,ax                    ; convert AX K to ptr to top of mem
        add esi,400h
        shl esi,10

        call enablea20                  ; enable A20

        push ds                         ; preserve ES (buffer segment)

        xor cx,cx                       ; ES -> 0 (interrupt vector table)
        mov ds,cx
        lds bx,dword ptr ds:[4*19h]     ; ES:BX -> int vector table

        mov edx,'SIDV'
        mov eax,100000h                 ; initial free extended memory base
        cmp dword ptr ds:[bx+12h],edx   ; VDISK memory allocation?
        jne short @@r_initf0            ; if present, get base of free mem

        mov eax,dword ptr ds:[bx+2ch]   ; get first free byte of extended mem
        add eax,03ffh                   ; align on kilobyte
        and eax,0fffc00h                ; address is only 24bit

@@r_initf0:
        dec cx                          ; ES -> 0ffffh for ext mem addressing
        mov ds,cx

        cmp dword ptr ds:[13h],edx      ; VDISK memory allocation?
        jne short @@r_initf1            ; if present, get base of free mem

        movzx ebx,word ptr ds:[2eh]     ; get first free K of extended memory
        shl ebx,10                      ; adjust K to bytes

        cmp eax,ebx                     ; pick larger of 2 addresses
        ja short @@r_initf1

        mov eax,ebx

@@r_initf1:
        pop ds                          ; restore ES (buffer segment)

        mov extmembase,eax              ; store base of raw extended memory
        sub esi,eax
        jbe xr_init

        call extmemadjust
        mov extmemsize,esi

        shr esi,10
        mov rawextmemused,si

        push es

        inc cx
        mov es,cx

        call cleanup0
        mov dword ptr oldint15vector,eax

        pop es

        xor al,al
        jmp short xr_init2              ; go to XMS/raw continue init

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
x_init:                                 ; XMS protected mode init
        mov ah,3                        ; enable A20
        call dword ptr xms_callip

        mov bx,4                        ; error code 0004h in case of error
        cmp ax,1                        ; error enabling A20?
        jc init_done                    ; if yes, exit with error 0004h

        movzx esi,xms_mem
        shl esi,10
        call extmemadjust

        or esi,esi
        jz short xr_init

        mov edx,esi
        shr edx,10
        mov ah,9
        call dword ptr xms_callip

        or ax,ax
        jz short xr_init

        mov xms_handle,dx

        mov ah,0ch
        call dword ptr xms_callip

        or ax,ax
        jz short @@00

        shl edx,16
        mov dx,bx

        mov extmemsize,esi
        mov extmembase,edx

@@00:
        mov al,2

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
xr_init2:
        mov cleanuptype,al

;-----------------------------------------------------------------------------
xr_init:                                ; XMS/raw common init tail
        mov rmtopmswrout,off xr_rmtopmsw; set XMS/raw mode switch addresses
        mov pmtormswrout,off xr_pmtormsw

        jmp vxr_init                    ; go to VCPI/XMS/raw continue init

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
v_init:                                 ; VCPI protected mode init
        mov ax,0de0ah                   ; get PIC mappings
        int 67h
        mov ch,bl
        mov word ptr picslave,cx

        call getrmirqvectors

        mov dx,es                       ; align data area on page
        add dx,0ffh
        xor dl,dl
        mov es,dx

        movzx eax,dx                    ; set base and top of page table area
        inc ah
        mov fs,ax                       ; FS = segment of first page table

        shl eax,4
        mov pagetablebase,eax
        movzx ecx,pagetablenum
        shl ecx,12
        add eax,ecx
        mov pagetabletop,eax

        xor di,di                       ; clear page dir and first page table
        mov cx,1000h
        xor ax,ax
        rep stos word ptr es:[di]

        mov gs,dx                       ; GS = segment of page directory
        push fs                         ; ES = segment of first page table
        pop es

        push ss                         ; stack space for VCPI descriptors
        pop ds
        sub sp,8*3
        mov si,sp

        xor edi,edi                     ; get VCPI protected mode interface
        mov ax,0de01h
        int 67h

        push cs                         ; DS = PMODE_TEXT
        pop ds

        mov eax,pagetablebase
        add eax,edi
        add pagetablefree,eax

        mov vcpi_calleip,ebx            ; store protected mode VCPI call EIP
        movzx si,dh                     ; get physical address of page dir
        shl si,2                        ;  from first page table for CR3
        lods dword ptr fs:[si]
        mov vcpi_cr3,eax

@@v_initl0:
        and byte ptr es:[di+1],0f1h     ; clear bits 9-11 in copied page table
        sub di,4
        jnc @@v_initl0

        mov dx,es                       ; DX = current page table segment
        xor ebx,ebx                     ; index in page dir, also loop counter
        jmp short @@v_initf0

@@v_initl1:
        xor di,di                       ; clear page table
        mov ch,8
        xor ax,ax
        rep stos word ptr es:[di]

@@v_initf0:
        inc dh                          ; increment page table segment
        mov es,dx

        lods dword ptr fs:[si]          ; set physical address of page table
        mov gs:[ebx*4],eax              ;  in page directory

        inc bx                          ; increment index in page directory
        cmp bl,pagetablenum             ; at end of page tables?
        jb @@v_initl1                   ; if no, loop

;-----------------------------------------------------------------------------
        push dx                         ; preserve seg of TSS for later use

        mov eax,vcpi_cr3                ; set CR3 in TSS
        mov es:[1ch],eax
        mov dword ptr es:[64h],680000h  ; set offset of I/O permission bitmap

        add dx,7                        ; increment next data area ptr
        mov es,dx

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
vxr_init:                               ; VCPI/XMS/raw common init tail
        xor eax,eax                     ; set IDT base address
        mov ax,es
        shl eax,4
        mov idtbase,eax
        mov ax,es

        movzx bx,_pm_rmstacks           ; set top and base of real mode stack
        imul bx,_pm_rmstacklen          ;  area for interrupt redirection
        add ax,80h                      ;  from protected mode
        mov rmstackbase,ax
        add ax,bx
        mov rmstacktop,ax

ifdef   EXCEPTIONS
        mov abortrmstacktop,ax          ; store stack top for possible abort
endif

        xor di,di                       ; set up IDT
        mov dx,word ptr picslave
        xor ecx,ecx

@@vxr_initl0:
        mov bl,cl                       ; isolate high 5 bits of int num
        and bl,0f8h

ifdef   EXCEPTIONS
        cmp cl,0eh                      ; one of the low 0eh interrupts?
        ja short @@00                   ; if no, skip exception init

        lea eax,[10000h*SELCODE+4*ecx+off exception_intlowentry]

        cmp bl,dh                       ; IRQs mapped normal?
        jne short @@02                  ; if no, normal exception redirection

        imul eax,ecx,13h                ; address of exception/IRQ entry
        lea eax,[10000h*SELCODE+eax-13h*8+off exception_irqlowentry]

@@02:
        stos dword ptr es:[di]          ; store low dword of IDT entry

        jmp short @@01
endif

@@00:
        mov bh,bl
        mov eax,ecx
        and al,7

        cmp bl,dl                       ; one of the high IRQs?
        jne short @@04

        mov bh,70h

@@04:
        cmp bl,dh                       ; one of the low IRQs?
        jne short @@03

        mov bh,8

@@03:
        or al,bh
        lea eax,[SELCODE*10000h+eax+off intrmatrix]
        stos dword ptr es:[di]          ; store low dword of IDT entry

        mov ah,8fh                      ; trap gate type

        cmp bh,8                        ; default IRQ mapping?
        je short @@01                   ; if yes, interrupt gate type

        cmp bh,70h                      ; default IRQ mapping?
        jne short @@vxr_initl0f0        ; if no, trap gate type

@@01:
        mov ah,8eh                      ; interrupt gate type

@@vxr_initl0f0:
        and eax,0000ff00h               ; prepare gate type
        stos dword ptr es:[di]          ; store high dword of IDT entry

        inc cl                          ; increment interrupt number
        jnz @@vxr_initl0                ; loop if more interrupts to go

        mov word ptr es:[8*31h],off int31    ; protected mode INT 31h
        mov word ptr es:[8*3],off intr  ; protected mode INT 3

        mov es,rmstacktop               ; set next data area ptr to end of
                                        ;  real mode stack area
;-----------------------------------------------------------------------------
        xor eax,eax                     ; set protected mode stack area base
        mov ax,es                       ;  for callbacks
        shl eax,4
        mov pmstackbase,eax

        movzx ecx,_pm_pmstacklen        ; set protected mode stack area top
        movzx ebx,_pm_pmstacks          ;  for callbacks
        shl ecx,4
        mov pmstacklen,ecx              ; protected mode stack size in bytes
        imul ebx,ecx
        add ebx,eax
        mov pmstacktop,ebx              ; protected mode stack area top

        movzx cx,_pm_callbacks          ; CX = number of callbacks
        or cl,cl                        ; any callbacks?
        jz short @@vxr_initf3           ; if no, done with this part

        mov callbackbase,ebx            ; top of stacks is base of callbacks
        shr ebx,4                       ; BX = seg of callback area
        mov callbackseg,bx

        mov ds,bx                       ; ES = seg of callback area
        xor di,di                       ; location within callback seg
        mov ax,6866h

@@vxr_initl1:
        mov word ptr ds:[di],6066h      ; PUSHAD instruction
        mov byte ptr ds:[di+2],ah       ; PUSH WORD instruction
        mov word ptr ds:[di+3],0        ; immediate 0 used as free flag
        mov word ptr ds:[di+5],ax       ; PUSH DWORD instruction
        mov byte ptr ds:[di+11],0b9h    ; MOV CX,? instruction
        mov word ptr ds:[di+14],ax      ; PUSH DWORD instruction
        mov byte ptr ds:[di+20],0eah    ; JMP FAR PTR ?:? intruction
        mov word ptr ds:[di+21],off callback
        mov word ptr ds:[di+23],PMODE_TEXT

        add di,25                       ; increment ptr to callback
        dec cl                          ; decrement loop counter
        jnz @@vxr_initl1                ; if more callbacks to do, loop

        push cs
        pop ds

	add di,0fh			; align next data area on paragraph
	shr di,4
        add bx,di
        mov es,bx                       ; set ES to base of next data area
@@vxr_initf3:

;-----------------------------------------------------------------------------
        xor eax,eax                     ; set GDT base address
        mov ax,es                       ;  for callbacks
        mov gdtseg,ax                   ; store segment of GDT
        shl eax,4
        mov gdtbase,eax

        mov cx,_pm_selectors            ; set GDT limit
        shl cx,3
        add cx,8*5+8*SYSSELECTORS-1
        mov gdtlimit,cx

        xor di,di                       ; clear GDT with all 0
        inc cx
        shr cx,1
        xor eax,eax
        rep stos word ptr es:[di]

        cmp pmodetype,2                 ; if under VCPI, do VCPI GDT set up
        jne short @@vxr_initf1

        pop ax                          ; restore TSS seg from stack
        shl eax,4                       ; set up TSS selector in GDT
        mov dword ptr es:[SELVCPITSS+2],eax
        mov byte ptr es:[SELVCPITSS],67h
        mov byte ptr es:[SELVCPITSS+5],89h

        mov eax,off vcpisstacktop       ; set up temporary switch stack
        add eax,codebase
        mov vcpiswitchstack,eax

        mov di,SELVCPICODE              ; copy 3 VCPI descriptors from stack
        mov si,sp                       ;  to GDT
        mov cl,4*3
        rep movs word ptr es:[di],word ptr ss:[si]

        add sp,8*3                      ; adjust stack

@@vxr_initf1:
        push es
        pop ds

        mov ax,-1
        mov dx,0df92h
        mov word ptr ds:[SELZERO],ax    ; set SELZERO descriptor
        mov word ptr ds:[SELZERO+5],dx
        mov word ptr ds:[SELCALLBACKDS],ax      ; set callback DS descriptor
        mov word ptr ds:[SELCALLBACKDS+5],dx

        mov dx,1092h                    ; DX = access rights
        mov cx,ax                       ; CX = limit (64k)
        mov bx,SEL40                    ; BX = index to SEL40 descriptor
        mov ax,40h                      ; set SEL40 descriptor
        call vxr_initsetdsc

        mov bx,SELDATA                  ; BX = index to SELDATA descriptor
        mov ax,cs                       ; set SELDATA descriptor (PMODE_TEXT)
        call vxr_initsetdsc

        mov dl,9ah                      ; DX = access rights
        mov bl,SELCODE                  ; BX = index to SELCODE descriptor
        call vxr_initsetdsc2            ; set SELCODE descriptor (PMODE_TEXT)

        mov bl,8*SYSSELECTORS           ; BX = base of free descriptors
        push bx                         ; store selector

        mov ax,ss                       ; set caller SS descriptor
        mov dx,5092h
        call vxr_initsetdsc

        mov ax,bx                       ; set caller DS descriptor and put DS
        xchg ax,[bp]                    ;  selector on stack for exit
        call vxr_initsetdsc

        push bx                         ; get PSP segment
        mov ah,51h
        int 21h
        mov si,bx
        pop bx

        mov fs,si                       ; set caller environment descriptor
        mov ax,fs:[2ch]
        or ax,ax                        ; is environment seg 0?
        jz short @@vxr_initf0           ; if yes, dont convert to descriptor
        mov fs:[2ch],bx                 ; store selector value in PSP
        call vxr_initsetdsc

@@vxr_initf0:
        mov ax,si                       ; set caller PSP descriptor
        xor ch,ch                       ; limit is 100h bytes
        call vxr_initsetdsc

.errnz  ($-off rmintvector) lt 400h

abortstackbase   label   dword          ; abort protected mode stack base

;-----------------------------------------------------------------------------
        push cs                         ; set up INT redirector matrix
        pop es
        mov di,off intrmatrix
        inc cx
        mov al,0cch
        rep stos byte ptr es:[di]

        call backupints

        lea ebp,[ebx-8]                 ; BP = PSP descriptor, just set
        pop dx                          ; DX = SS descriptor, from stack
        mov cx,SELZERO                  ; CX = ES descriptor, SELZERO
        movzx ebx,sp                    ; EBX = SP, current SP - same stack
        mov ax,SELCODE                  ; AX = DS descriptor, SELCODE
        mov edi,off vxr_pinit           ; target EIP
        mov si,ax                       ; target CS is SELCODE, same segment

        jmp cs:rmtopmswrout             ; jump to mode switch routine

;-----------------------------------------------------------------------------
vxr_pinit:                              ; VCPI/XMS/raw protected mode init
        mov ebx,codebase                ; get callbacks for DOS error ints
        dw 6667h,0bb8dh,passupintregs,0 ; LEA EDI,[EBX+passupintregs]
        mov ax,303h

        mov esi,off int1c               ; INT 1Ch callback
        int 31h

        push cx dx

        mov esi,off int23               ; INT 23h callback
        int 31h

        push cx dx

        mov esi,off int24               ; INT 24h callback
        int 31h

        push cx dx

        mov es,bp                       ; ES = PSP selector
        mov ds,seldata                  ; DS = SELDATA selector

        pop int24callback               ; store callback addresses
        pop int23callback
        pop int1ccallback

        jmp dvxr_init

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
vxr_initsetdsc:                         ; set descriptor for VCPI/XMS/raw init
        movzx eax,ax                    ; EAX = base of segment
        shl eax,4

;-----------------------------------------------------------------------------
vxr_initsetdsc2:
        mov word ptr ds:[bx],cx         ; limit = CX
        mov dword ptr ds:[bx+2],eax     ; base address = EAX
        mov word ptr ds:[bx+5],dx       ; access rights = DX

        add bx,8                        ; increment descriptor index
        ret

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
int15:                                  ; real mode INT 15h handler
        cmp ah,88h                      ; function 88h?
        je short @@int15f0              ; if yes, need to process

        jmp dword ptr cs:oldint15vector ; no, go on to old INT 15h handler

@@int15f0:
        pushf                           ; call old int 15h handler
        call dword ptr cs:oldint15vector

        sub ax,cs:rawextmemused         ; adjust AX by extended memory used

        push bp                         ; clear carry flag on stack for IRET
        mov bp,sp
        and byte ptr [bp+6],0feh
        pop bp

        iret                            ; return with new AX extended memory

abortstacktop   = $
.errnz  ($-off abortstackbase) lt 0a0h

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Clean up after protected mode
; Out:
;   EAX,EBX,ECX,EDX,ESI,EDI,EBP - ?
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_pm_cleanup:
        push ds es
        cld

        push cs
        pop ds

        xor cx,cx
        mov es,cx

        mov ax,oldpicmasks              ; old PIC masks

        call restoreRMints              ; restore real mode INT stuff

        movzx bx,cleanuptype
        call cleanuptbl[bx]

        pop es ds
        retf

;-----------------------------------------------------------------------------
cleanup0:
        mov eax,dword ptr oldint15vector
        xchg eax,es:[4*15h]

;-----------------------------------------------------------------------------
cleanup3:
        ret

;-----------------------------------------------------------------------------
cleanup1:
        mov dx,xms_handle
        mov ah,0dh
        call dword ptr xms_callip

        mov ah,0ah
        call dword ptr xms_callip

        ret

;-----------------------------------------------------------------------------
cleanup2:
        mov esi,pagetablefree
        mov ecx,pagetablepages
        or ecx,ecx
        jz short @@00

        mov ebx,esi
        and si,0fh
        shr ebx,4
        mov ds,bx
        mov di,8000h

@@00l:
        mov edx,dword ptr ds:[si]
        and dx,0f000h
        mov ax,0de05h
        int 67h

        add si,4
        cmp si,di
        jb short @@01

        sub si,di
        add bh,8
        mov ds,bx

@@01:
        dec ecx
        jnz @@00l

        push cs
        pop ds

@@00:
        test vcpi_memused,1
        jnz cleanup1

        ret

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
restoreRMints:
        out 21h,al                      ; set PICs
        mov al,ah
        out 0a1h,al

        xor di,di                       ; restore real mode INT vectors
        mov si,off rmintvector
        mov cx,44h
        rep movs word ptr es:[di],word ptr ds:[si]
        add di,4
        add si,4
        mov cx,1bah
        rep movs word ptr es:[di],word ptr ds:[si]

        ret

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; PROTECTED MODE KERNEL CODE
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
v_rmtopmsw:                             ; VCPI real to protected switch
        pushf                           ; store FLAGS
        cli
        push cs                         ; DS = PMODE_TEXT
        pop ds
        pop tempw0                      ; move FLAGS from stack to temp
        mov tempw1,ax                   ; store AX (protected mode DS)
        mov tempw2,si                   ; store SI (protected mode CS)
        mov esi,vcpistrucaddx           ; ESI = linear addx of VCPI structure
        mov ax,0de0ch                   ; VCPI switch to protected mode
        int 67h
v_rmtopmswpm:
        mov ss,dx                       ; load protected mode SS:ESP
        mov esp,ebx
        mov ds,cs:tempw1                ; load protected mode DS
        mov es,cx                       ; load protected mode ES
        xor ax,ax
        mov fs,ax                       ; load protected mode FS with NULL
        mov gs,ax                       ; load protected mode GS with NULL
        pushfd                          ; store EFLAGS
        mov ax,cs:tempw0                ; move bits 0-11 of old FLAGS onto
        and ah,0fh                      ;  stack for IRETD
        mov [esp],ax
        push cs:tempd1                  ; store protected mode target CS
        push edi                        ; store protected mode target EIP
        iretd                           ; go to targed addx in protected mode

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
v_pmtormsw:                             ; VCPI protected to real switch
        pushf                           ; store FLAGS
        cli
        push ax                         ; store AX (real mode DS)
        mov ds,cs:selzero               ; DS -> 0 (beginning of memory)
        movzx ebx,bx                    ; clear high word of EBX, real mode SP
        movzx edx,dx                    ; clear high word of EDX, real mode SS
        mov eax,cs:vcpiswitchstack      ; EAX -> top of temporary switch stack
        mov dword ptr ds:[eax+32],0     ; store real mode GS
        mov dword ptr ds:[eax+28],0     ; store real mode FS
        movzx ecx,cx                    ; clear high word of ECX, real mode ES
        mov ds:[eax+20],ecx             ; store real mode ES
        pop cx                          ; move real mode DS from protected
        mov ds:[eax+24],ecx             ;  mode stack to VCPI call stack
        mov ds:[eax+16],edx             ; store real mode SS
        mov ds:[eax+12],ebx             ; store real mode SP
        mov dword ptr ds:[eax+4],PMODE_TEXT             ; store real mode CS
        mov dword ptr ds:[eax+0],off @@v_pmtormswf0     ; store real mode IP
        pop bx                          ; restore FLAGS from stack
        mov ss,cs:selzero               ; SS -> 0 (beginning of memory)
        mov esp,eax                     ; ESP = stack ptr for VCPI call
        mov ax,0de0ch                   ; VCPI switch to real mode (V86)
	call fword ptr cs:vcpi_calleip
@@v_pmtormswf0:
        push bx                         ; store old FLAGS
	push si 			; store target CS in real mode
	push di 			; store target IP in real mode
	iret				; go to target addx in real mode

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
xr_rmtopmsw:                            ; XMS/raw real to protected switch
        pushfd                          ; store EFLAGS
        cli
        push ax                         ; store AX (protected mode DS)
        lidt fword ptr cs:idtlimit      ; load protected mode IDT
        lgdt fword ptr cs:gdtlimit      ; load protected mode GDT
        mov eax,cr0                     ; switch to protected mode
        or al,1
        mov cr0,eax
        db 0eah                         ; JMP FAR PTR SELCODE:$+4
        dw $+4,SELCODE                  ;  (clear prefetch que)
        pop ds                          ; load protected mode DS
        mov es,cx                       ; load protected mode ES
        xor ax,ax
        mov fs,ax                       ; load protected mode FS with NULL
        mov gs,ax                       ; load protected mode GS with NULL
        pop eax
        mov ss,dx                       ; load protected mode SS:ESP
        mov esp,ebx
        and ah,0bfh                     ; set NT=0 in old EFLAGS
        push ax                         ; set current FLAGS
        popf
        push eax                        ; store old EFLAGS
        push esi                        ; store protected mode target CS
        push edi                        ; store protected mode target EIP
        iretd                           ; go to targed addx in protected mode

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
xr_pmtormsw:                            ; XMS/raw protected to real switch
        pushf                           ; store FLAGS
        cli
        push ax                         ; store AX (real mode DS)
        mov ax,SELDATA
        mov ds,ax                       ; DS -> PMODEW_TEXT
        pop ds:tempw0                   ; move real mode DS from stack to temp
        pop ds:tempw1                   ; move FLAGS from stack to temp
        mov es,ax                       ; load descriptors with real mode seg
        mov fs,ax                       ;  attributes
        mov gs,ax
        mov ss,ax                       ; load descriptor with real mode attr
        movzx esp,bx                    ; load real mode SP, high word 0
        lidt fword ptr ds:rmidtlimit    ; load real mode IDT
        mov eax,cr0                     ; switch to real mode
        and al,0feh
        mov cr0,eax
        db 0eah                         ; JMP FAR PTR PMODE_TEXT:$+4
        dw $+4,PMODE_TEXT               ;  (clear prefetch que)
        mov ss,dx                       ; load real mode SS
        mov ds,cs:tempw0                ; load real mode DS
        mov es,cx                       ; load real mode ES
        xor ax,ax
        mov fs,ax                       ; load real mode FS with NULL
        mov gs,ax                       ; load real mode GS with NULL
        push cs:tempw1                  ; store old FLAGS
        push si                         ; store real mode target CS
        push di                         ; store real mode target IP
        iret                            ; go to target addx in real mode

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
vxr_saverestorepm:                      ; VCPI/XMS/raw save/restore status
        db 66h                          ; no save/restore needed, 32bit RETF

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
vxr_saverestorerm:                      ; VCPI/XMS/raw save/restore status
        retf                            ; no save/restore needed, 16bit RETF

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
callback:                               ; real mode callback handler
        mov ax,sp                       ; preserve SS:SP for callback
        push ss
        push ax

        push gs fs ds es                ; preserve real mode regs for callback
        pushf                           ; preserve FLAGS for callback

        cli
        cld

        push cs
        pop ds

        mov ebp,ds:pmstacktop           ; EBP = ESP for protected mode
        mov ebx,ebp                     ; set EBX to next stack location
        sub ebx,ds:pmstacklen
        mov ds:pmstacktop,ebx           ; update ptr for possible reenterancy

        cmp ebx,ds:pmstackbase          ; exceeded protected mode stack space?
        jb abort_callback               ; if yes, abort

        xor eax,eax                     ; EAX = base address of SS
        mov ax,ss
        shl eax,4

	movzx ebx,sp			; EBX = current linear SS:SP
        add ebx,eax

        mov es,ds:gdtseg                ; set for protected mode callback DS
	or eax,92000000h		;  base address in GDT
        mov es:[SELCALLBACKDS+2],eax

        mov ax,SELZERO                  ; DS selector for protected mode
        mov dx,ax                       ; SS selector = DS selector
        mov si,SELCODE                  ; target protected mode CS:EIP
        mov edi,off @@callbackf0

        jmp ds:rmtopmswrout             ; go to protected mode

@@callbackf0:
        mov edi,[esp+14]                ; EDI -> register structure from stack

        lea esi,[esp+24]                ; copy general registers from stack
        mov ecx,8                       ;  to register structure
        rep movs dword ptr es:[edi],dword ptr ds:[esi]

        mov esi,esp                     ; copy FLAGS, ES, DS, FG, and GS
        mov cl,5
        rep movs word ptr es:[edi],word ptr ds:[esi]

        lods dword ptr ds:[esi]         ; EAX = real mode SS:SP from stack
        add ax,42                       ; adjust SP for stuff on stack
        mov es:[edi+4],eax              ; put in register structure

        mov ds,cs:selcallbackds         ; DS = callback DS selector
        sub edi,42                      ; EDI -> register structure
        movzx esi,ax                    ; ESI = old real mode SP
        xchg esp,ebp                    ; ESP = protected mode stack

        pushfd                          ; push flags for IRETD from callback
        db 66h                          ; push 32bit CS for IRETD
        push cs
        dw 6866h,@@callbackf1,0         ; push 32bit EIP for IRETD

        movzx eax,word ptr [ebp+22]     ; EAX = target CS of callback
        push eax                        ; push 32bit CS for RETF to callback
        push dword ptr [ebp+18]         ; push 32bit EIP for retf

        db 66h                          ; 32bit RETF to callback
        retf

@@callbackf1:
        cli
        cld

        push es                         ; DS:ESI = register structure
        pop ds
        mov esi,edi

        mov es,cs:selzero               ; ES -> 0 (beginning of memory)

        movzx ebx,word ptr [esi+2eh]    ; EBX = real mode SP from structure
        movzx edx,word ptr [esi+30h]    ; EDX = real mode SS from structure
        sub bx,42                       ; subtract size of vars to be put

        mov ebp,[esi+0ch]               ; EBP = pushed ESP from real mode
        mov bp,bx                       ; EBP = old high & new low word of ESP

        mov edi,edx                     ; EDI -> real mode base of stack
        shl edi,4                       ;  of vars to be stored
        add edi,ebx

        mov ecx,8                       ; copy general registers to stack
        rep movs dword ptr es:[edi],dword ptr ds:[esi]

        mov eax,[esi+6]                 ; EAX = return FS and GS for real mode
        stos dword ptr es:[edi]         ; store on real mode stack for return

        mov eax,[esi+10]                ; EAX = return CS:IP for real mode
        stos dword ptr es:[edi]         ; store on real mode stack for return
        mov ax,[esi]                    ; AX = return FLAGS for real mode
        stos word ptr es:[edi]          ; store on real mode stack for return

        mov ax,[esi+4]                  ; AX = return DS for real mode
        mov cx,[esi+2]                  ; CX = return ES for real mode

        mov di,off @@callbackf2
        mov si,PMODE_TEXT               ; real mode target CS:IP

        db 66h                          ; JMP DWORD PTR, as in 32bit offset,
        jmp word ptr cs:pmtormswrout    ;  not seg:16bit offset

@@callbackf2:
        mov esp,ebp                     ; restore total ESP, old high word

        mov eax,cs:pmstacklen           ; restore top of protected mode stack
        add cs:pmstacktop,eax

        popad                           ; get callback return general regs
        pop fs gs                       ; get callback return FS and GS values
        iret                            ; go to callback return CS:IP

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
rmirqmatrix:
rept    16
        push ax
        call near ptr rmintr
endm

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
rmintr:
        pop ax
        sub ax,off rmirqmatrix+4
        shr ax,2

        pushad
        push ds es fs gs

        push cs
        pop ds

        mov ebp,ds:pmstacktop
        mov ebx,ebp
        sub ebx,ds:pmstacklen
        mov ds:pmstacktop,ebx

        cmp ebx,ds:pmstackbase
        jb abort_IRQtoPM

        xor al,8
        mov si,ax
        and al,7
        shr si,3
        or al,ds:picslave[si]
        mov ds:@@rmintrintnum,al

        mov edi,off @@rmintrf0
        mov si,SELCODE
        mov dx,SELZERO
        mov cx,dx
        mov ax,dx

        mov bp,ss
        shl ebp,16
        mov bp,sp

        jmp cs:rmtopmswrout

@@rmintrf0:
        db 0cdh
@@rmintrintnum  db      ?

        shld edx,ebp,16
        mov bx,bp

        mov di,off @@rmintrf1
        mov si,PMODE_TEXT

        db 66h
        jmp word ptr cs:pmtormswrout

@@rmintrf1:
        mov eax,cs:pmstacklen
        add cs:pmstacktop,eax

        mov ax,[esp+20]
        shl eax,16
        or esp,eax

        pop gs fs es ds
        popad
        pop ax
        iret

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
intr:                                   ; general interrupt redirector
        cmp word ptr [esp+4],SELCODE    ; INT redirection or internal INT 3?
        je short intrredirect           ; if not INT 3, jump to redirection

ifdef   EXCEPTIONS
        jmp exception_intlowentry+4*3   ; go to exception 3 handler
else
        jmp fword ptr cs:int3vector     ; INT 3, jump to INT 3 vector
endif

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
intrredirect:
        mov [esp+8],eax                 ; store EAX for later POPAD
        mov eax,[esp]                   ; get address in redirection matrix
        add esp,8                       ; discard EIP and CS from INT 3

        push ecx edx ebx esp ebp esi edi; store rest of registers for POPAD
        push ds es fs gs
        cld

        push cs
        pop ds

        mov edi,ds:codebase             ; EDI = offset of PMODE_TEXT from 0

        mov dx,ds:rmstacktop            ; DX = SS for real mode redirection
        movzx ebp,dx                    ; EBP -> top of real mode stack
        shl ebp,4

        mov bx,ds:_pm_rmstacklen        ; get size of real mode stack
        sub dx,bx                       ; adjust DX to next stack location
        shl bx,4                        ; set real mode SP to top of stack

        mov ds,ds:selzero               ; DS -> 0 (beginning of memory)
        mov ds:rmstacktop[edi],dx       ; update ptr for possible reenterancy

        cmp dx,cs:rmstackbase           ; exceeded real mode stack space?
        jb abort_INTtoRM                ; if yes, abort

        mov ds:[ebp-2],ss               ; store SS:ESP on real mode stack
        mov ds:[ebp-6],esp

        sub ax,off intrmatrix+1         ; AX = int number
        mov ah,al                       ; AH = high 5 bits of int number
        and ah,0f8h

        cmp ah,8                        ; low IRQ?
        je short intrirq                ; if yes, do IRQ

        cmp ah,70h                      ; high IRQ?
        jne short intrint               ; if no, do INT (with general regs)

        sub ah,8

;-----------------------------------------------------------------------------
intrirq:                                ; an IRQ redirection
        mov dword ptr ds:[ebp-10],PMODE_TEXT
        mov word ptr ds:[ebp-12],off @@intrirqf0

        movzx ecx,al
        mov edi,dword ptr ds:[ecx*4]

        sub al,ah
        mov cl,al
        bt cs:irqhookmap,cx
        jnc short @@intrirqf2

        mov edi,dword ptr cs:[rmirqvector+ecx*4]

@@intrirqf2:
        shld esi,edi,16
        sub bx,6+6                      ; adjust real mode SP for stored vars

        db 66h                          ; JMP DWORD PTR, as in 32bit offset,
        jmp word ptr cs:pmtormswrout    ;  not seg:16bit offset

@@intrirqf0:
        mov ax,SELZERO                  ; DS selector value for protected mode
        mov cx,ax                       ; ES selector value for protected mode
        pop ebx                         ; get protected mode SS:ESP from stack
        pop dx
        mov si,SELCODE                  ; target CS:EIP in protected mode
        mov edi,off @@intrirqf1

        jmp cs:rmtopmswrout             ; go back to protected mode

@@intrirqf1:
        mov edi,cs:codebase             ; restore top of real mode stack
        mov ax,cs:_pm_rmstacklen
        add ds:rmstacktop[edi],ax

        pop gs fs es ds                 ; restore all registers
        popad
        iretd

;-----------------------------------------------------------------------------
intrint:                                ; an INT redirection
        mov ds:@@intrintintnum[edi],al  ; modify code with interrupt number

        mov es,cs:selzero               ; copy registers from protected mode
        lea edi,[ebp-28h]               ;  stack to real mode stack

        mov ax,[esp+30h]                ; copy flags from protected mode stack
        stos word ptr es:[edi]

        lea esi,[esp+8]
        mov ecx,8
        rep movs dword ptr es:[edi],dword ptr ss:[esi]

        sub bx,28h                      ; adjust real mode SP for stored vars

        mov di,off @@intrintf0
        mov si,PMODE_TEXT               ; real mode target CS:IP

        db 66h                          ; JMP DWORD PTR, as in 32bit offset,
        jmp word ptr cs:pmtormswrout    ;  not seg:16bit offset

@@intrintf0:
        popf
        popad                           ; load regs with int call values

        db 0cdh                         ; INT @@intrirqintnum
@@intrintintnum db      ?

        pushad                          ; store registers on stack
        pushf                           ; store flags on stack
        cli

        xor eax,eax                     ; EAX = linear ptr to SS
        mov ax,ss
        shl eax,4
        movzx ebp,sp                    ; EBP = SP

        mov ebx,[bp+22h]                ; get protected mode SS:ESP from stack
        mov dx,[bp+26h]

        add ebp,eax                     ; EBP -> stored regs on stack

        mov ax,SELZERO                  ; DS selector value for protected mode
        mov cx,ax                       ; ES selector value for protected mode
        mov si,SELCODE                  ; target CS:EIP in protected mode
        mov edi,off @@intrintf1

        jmp cs:rmtopmswrout             ; go back to protected mode

@@intrintf1:
        mov edi,cs:codebase             ; restore top of real mode stack
        mov ax,cs:_pm_rmstacklen
        add ds:rmstacktop[edi],ax

        mov ax,ds:[ebp]                 ; move return FLAGS from real mode
        and ax,8d5h                     ;  stack to protected mode stack
        mov bx,[esp+30h]
        and bx,not 8d5h
        or ax,bx
        mov [esp+30h],ax

        mov eax,ebp                     ; restore return registers from real
        mov edi,[eax+2]                 ;  mode stack
        mov esi,[eax+6]
        mov ebp,[eax+10]
        mov ebx,[eax+18]
        mov edx,[eax+22]
        mov ecx,[eax+26]
        mov eax,[eax+30]

        pop gs fs es ds                 ; restore segment regs
        add esp,20h                     ; skip old general registers on stack
        iretd

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
int1c:                                  ; INT 1Ch pass-up interface
        call passupintinit

        int 1ch

;-----------------------------------------------------------------------------
passupintdone:
        mov ax,6

;-----------------------------------------------------------------------------
passupintdone2:
        cli
        cld

        add [esp+2eh],ax

        mov eax,32h
        mov ecx,eax
        mov esi,esp

        push edi

        push es
        pop ds
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop edi

        add esp,eax

        iretd

;-----------------------------------------------------------------------------
passupintinit:
        pop bp

        mov eax,ds:[esi]
        mov es:[edi+2ah],eax
        mov ax,ds:[esi+4]
        mov es:[edi+20h],ax

        mov esi,edi
        mov ecx,32h
        sub esp,ecx
        mov edi,esp

        push esi

        push es
        pop ds
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop edi

        jmp bp

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
int23:                                  ; DOS INT 23h pass-up interface
        call passupintinit

        mov ebp,esp

        clc
        sti
        int 23h

        mov esp,ebp

        setc al
        mov ah,[esp+20h]
        and ah,0feh
        or al,ah
        mov [esp+20h],al

        mov ax,4
        jmp passupintdone2

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
int24:                                  ; DOS INT 24h pass-up mode interface
        mov dx,ds
        mov bx,si

        call passupintinit

        push edi

        mov ds,dx
        lea si,[bx+6+20]
        std

        lods dword ptr ds:[si]
        push eax
        lods dword ptr ds:[si]
        push eax
        lods dword ptr ds:[si]
        push eax
        lods dword ptr ds:[si]
        push eax
        lods dword ptr ds:[si]
        push eax
        lods dword ptr ds:[si]
        push eax

        mov bp,es
        movzx esi,word ptr [esp+24+4+8]
        movzx eax,word ptr [esp+24+4+4]
        shl esi,4
        add esi,eax
        mov di,[esp+24+4]
        mov ax,[esp+24+4+1ch]

        sti
        int 24h

        add esp,24
        pop edi

        mov [esp+1ch],al

        jmp passupintdone

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; EXCEPTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
ifdef   EXCEPTIONS

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
exception_intlowentry:                  ; raw exception entry points

irp     num,<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>
        push &num
        jmp short exception_intlow
endm

.errnz  ($-off exception_intlowentry) ne 4*15

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
exception_intlow:
        jmp exception

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
exception_irqlowentry:                  ; common IRQ/exception entry points

irp     num,<0,1,2,3,4,5,6>
        push ax

        mov ax,(100h * (&num + 8)) + 0bh
        out 20h,al
        in al,20h

        test al,1 shl &num
        jz short exception_irqlow

        pop ax
        jmp fword ptr cs:exceptionivect[6*8+6*&num]
endm

.errnz  ($-off exception_irqlowentry) ne 13h*7

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
exception_irqlow:
        shr ax,8                        ; store exception number on stack
        xchg ax,[esp]

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
exception:                              ; main exception dispatch
        push 0
        pushad                          ; store registers for possible dump

        movzx si,byte ptr [esp+32+2]    ; SI = exception number
        movzx ebx,cs:exceptionecsize[si]; EBX = error code field size
        mov [esp+32],bl                 ; store error code field size

        mov ax,ss                       ; exception stack frame SS
        push eax
        lea eax,[esp+4*1+32+4+4*3]      ; exception stack frame ESP
        push eax
        mov eax,[esp+4*2+32+4+ebx+8]    ; exception stack frame EFLAGS
        push eax
        mov ax,[esp+4*3+32+4+ebx+4]     ; exception stack frame CS
        push eax
        mov eax,[esp+4*4+32+4+ebx+0]    ; exception stack frame EIP
        push eax
        mov eax,[esp+4*5+32+4]          ; exception stack frame error code
        push eax

        lea esi,[2*esi+esi]             ; EDI = position of exception address
        shl si,1

        push large cs                   ; push addresses for exception
        push large offset exception_done
        push dword ptr cs:exceptionevect[si+4]
        push dword ptr cs:exceptionevect[si]

        mov esi,[esp+4*4+4*6+4]         ; registers at point of exception
        mov ebx,[esp+4*4+4*6+16]
        mov eax,[esp+4*4+4*6+28]

        db 66h                          ; 32bit RETF to exception handler
        retf

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
exception_done:                         ; end of exception entry point
        cli                             ; exception handler returned

        mov [esp+4*6+24],ebx            ; preserve returned EAX and EBX
        mov [esp+4*6+28],eax

        movzx ebx,byte ptr [esp+4*6+32] ; EBX = error code field size

        mov eax,[esp+4]                 ; copy return EIP from exception frame
        mov [esp+4*6+32+4+ebx+0],eax
        mov eax,[esp+8]                 ; copy return CS from exception frame
        mov [esp+4*6+32+4+ebx+4],eax

        lea eax,[esp+ebx+4*6+32+4]      ; calculate stack pointer for exit
        mov [esp+4*6+32],eax            ; store on stack

        add esp,4*6+4*6                 ; clean up stack and regs and return
        pop ebx
        pop eax
        mov esp,[esp]

        iretd

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
; [PMODE/W]: Abort: 00
; Exception: 0C
; Error code: 73127443
; EAX: 00001234  EBX: 12345678  ECX: 00001234  EDX: 12345678
; ESI: 00001234  EDI: 12345678  EBP: 03657245  ESP: 12345678
; DS: 1234  ES: 2345  FS: 8583  GS: 1295  SS: 2587  CS: 1746
; EIP: 04571346
; EFLAGS: 10538692
; CR0: 86425789  CR2: 01362334

exception_def:                          ; default exception handler (aborts)
        push es ds                      ; preserve selectors for dump

        xor bp,bp                       ; start abort sequence
        call abort_start

        mov dx,[esp+2*2+4*8+32+2]       ; DX = exception number
        push dx                         ; preserve exception number

        call abort_puthexnum2           ; put exception number

        call abort_nextline

        pop bx                          ; BX = exception number

        cmp cs:exceptionecsize[bx],0    ; error code present?
        je short @@00                   ; if no, skip error code put

        mov si,offset aborts_xerrcode   ; put error code
        call abort_putstr
        mov edx,[esp+2*2+2*4]
        call abort_puthexnum8

        call abort_nextline

@@00:
        mov si,offset aborts_xeax       ; put EAX
        call abort_putstr
        mov edx,[esp+2*2+4*8+28]
        call abort_puthexnum8

        mov si,offset aborts_xebx       ; put EBX
        call abort_putstr
        mov edx,[esp+2*2+4*8+16]
        call abort_puthexnum8

        mov si,offset aborts_xecx       ; put ECX
        call abort_putstr
        mov edx,[esp+2*2+4*8+24]
        call abort_puthexnum8

        mov si,offset aborts_xedx       ; put EDX
        call abort_putstr
        mov edx,[esp+2*2+4*8+20]
        call abort_puthexnum8

        call abort_nextline

        mov si,offset aborts_xesi       ; put ESI
        call abort_putstr
        mov edx,[esp+2*2+4*8+4]
        call abort_puthexnum8

        mov si,offset aborts_xedi       ; put EDI
        call abort_putstr
        mov edx,[esp+2*2+4*8+0]
        call abort_puthexnum8

        mov si,offset aborts_xebp       ; put EBP
        call abort_putstr
        mov edx,[esp+2*2+4*8+8]
        call abort_puthexnum8

        mov si,offset aborts_xesp       ; put ESP
        call abort_putstr
        mov edx,[esp+2*2+18h]
        call abort_puthexnum8

        call abort_nextline

        mov si,offset aborts_xds        ; put DS
        call abort_putstr
        pop dx
        call abort_puthexnum4

        mov si,offset aborts_xes        ; put ES
        call abort_putstr
        pop dx
        call abort_puthexnum4

        mov si,offset aborts_xfs        ; put FS
        call abort_putstr
        mov dx,fs
        call abort_puthexnum4

        mov si,offset aborts_xgs        ; put GS
        call abort_putstr
        mov dx,gs
        call abort_puthexnum4

        mov si,offset aborts_xss        ; put SS
        call abort_putstr
        mov dx,ss
        call abort_puthexnum4

        mov si,offset aborts_xcs        ; put CS
        call abort_putstr
        mov dx,[esp+10h]
        call abort_puthexnum4

        call abort_nextline

        mov si,offset aborts_xeip       ; put EIP
        call abort_putstr
        mov edx,[esp+0ch]
        call abort_puthexnum8

        call abort_nextline

        mov si,offset aborts_xeflags    ; put EFLAGS
        call abort_putstr
        mov edx,[esp+14h]
        call abort_puthexnum8

        call abort_nextline

        mov si,offset aborts_xcr0       ; put CR0
        call abort_putstr
        mov edx,cr0
        call abort_puthexnum8

        mov si,offset aborts_xcr2       ; put CR2
        call abort_putstr
        mov edx,cr2
        call abort_puthexnum8

        jmp abort_exit                  ; finish abort

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
abort_callback:                         ; ran out of PM stacks in callback
        mov bp,1                        ; abort string 1

;úúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúúú
abort_rm:                               ; real mode abort
        xor ax,ax
        xor cx,cx
        mov dx,SELDATA
        mov ebx,off abortstacktop
        mov si,SELCODE
        mov edi,off abort_pm

        jmp cs:rmtopmswrout

;-----------------------------------------------------------------------------
abort_IRQtoPM:                          ; ran out of PM stacks in RM->PM IRQ
        mov bp,2                        ; abort string 2

        jmp short abort_rm

;-----------------------------------------------------------------------------
abort_INTtoRM:                          ; ran out of RM stacks in PM->RM INT
        mov bp,3                        ; abort string 3

        jmp short abort_pm

;-----------------------------------------------------------------------------
abort_INT3103:                          ; ran out of RM stacks in INT 31h 03h
        mov bp,4                        ; abort string 4

;-----------------------------------------------------------------------------
abort_pm:                               ; protected mode abort
        call abort_start

        jmp abort_exit

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
abort_putstr:
        lods byte ptr ds:[si]

        or al,al
        jz short @@00

        stos word ptr es:[edi]

        jmp short abort_putstr

@@00:
        ret

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
abort_nextline:
        add bp,2*80
        mov di,bp

        ret

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
abort_puthexnum:
        xor bx,bx

@@00l:
        rol edx,4
        mov bl,dl
        and bl,0fh
        mov al,aborts_hexctbl[bx]

        stos word ptr es:[edi]

        dec cl
        jnz @@00l

        mov al,' '
        stos word ptr es:[edi]
        stos word ptr es:[edi]

        ret

;-----------------------------------------------------------------------------
abort_puthexnum2:
        shl edx,24
        mov cl,2

        jmp short abort_puthexnum

;-----------------------------------------------------------------------------
abort_puthexnum4:
        shl edx,16
        mov cl,4

        jmp short abort_puthexnum

;-----------------------------------------------------------------------------
abort_puthexnum8:
        mov cl,8

        jmp short abort_puthexnum

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
abort_start:                            ; abort, stabilize system
        cli
        cld

        push bp                         ; store abort number
        push bp

        mov ds,cs:seldata               ; DS -> PMODE_TEXT
        mov es,selzero                  ; ES -> 0 (beginning of memory)

        mov ax,abortrmstacktop          ; copy old top of real mode stack
        mov rmstacktop,ax

        mov al,20h                      ; clear pending IRQs
        out 20h,al
        out 0a0h,al

        mov ax,0ffffh                   ; mask off all IRQs

        call restoreRMints              ; restore real mode INT environment

        mov ax,3                        ; set text mode
        int 10h

        mov ebp,0b8000h                 ; EBP -> current TTY row start
        mov edi,ebp                     ; EDI -> current TTY column
        mov ah,7                        ; AH = text color

        mov si,off aborts_pmwabort      ; put first string of abort
        call abort_putstr

        pop dx                          ; put abort number
        call abort_puthexnum2

        call abort_nextline

        pop bx                          ; put abort string
        xor bh,bh
        add bl,bl
        mov si,aborts_tbl[bx]

        jmp abort_putstr                ; put abort string and return

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
abort_exit:                             ; exit after abnormal termination
        mov ah,2                        ; position cursor at bottom of screen
        xor bh,bh
        mov dx,0800h
        int 10h

        mov ah,4ch                      ; DOS terminate
        int 21h

else
;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
abort_callback:                         ; ran out of PM stacks in callback
abort_IRQtoPM:                          ; ran out of PM stacks in RM->PM IRQ
abort_INTtoRM:                          ; ran out of RM stacks in PM->RM INT
abort_INT3103:                          ; ran out of RM stacks in INT 31h 03h
        cli                             ; make sure we are not interrupted

        in al,61h                       ; beep
        or al,3
        out 61h,al

        jmp $                           ; now hang
endif

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INT 31h INTERFACE
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
int31:                                  ; protected mode INT 31h handler
        cli
        cld
        push ds es fs gs                ; push regs needed
        pushad
        mov ds,cs:selzero               ; DS -> 0 (beginning of memory)
        mov fs,cs:seldata               ; FS -> PMODE_TEXT

        push bx
        mov bx,(INT31FUNCNUM-1)*2       ; number of functions to check
@@int31l0:
        cmp ax,cs:int31functbl[bx]      ; found function value?
        jne short @@int31l0c

        mov bx,cs:int31routtbl[bx]      ; yes, go to appropriate handler
        xchg bx,[esp]
        ret

@@int31l0c:
        dec bx                          ; no, continue loop
        dec bx
        jns @@int31l0

        pop bx                          ; no function found
        jmp short int31fail

;-----------------------------------------------------------------------------
int31failbx:                            ; INT 31h return fail with BX,AX
        mov word ptr [esp+16],bx        ; put CX onto stack for POPAD

;-----------------------------------------------------------------------------
int31failax:                            ; INT 31h return fail with AX
        mov word ptr [esp+28],ax        ; put AX onto stack for POPAD

;-----------------------------------------------------------------------------
int31fail:                              ; INT 31h return fail, pop all regs
        popad
        pop gs fs es ds

;-----------------------------------------------------------------------------
int31failnopop:                         ; INT 31h return fail with carry set
        or byte ptr [esp+8],1           ; set carry in EFLAGS on stack
        iretd

;-----------------------------------------------------------------------------
int31okedx:                             ; INT 31h return ok with EDX,CX,AX
        mov [esp+20],edx                ; put EDX onto stack for POPAD
        jmp short int31okcx

;-----------------------------------------------------------------------------
int31okcx:                              ; INT 31h return ok with CX,DX,AX
        mov [esp+24],cx                 ; put CX onto stack for POPAD

;-----------------------------------------------------------------------------
int31okdx:                              ; INT 31h return ok with DX,AX
        mov [esp+20],dx                 ; put DX onto stack for POPAD

;-----------------------------------------------------------------------------
int31okax:                              ; INT 31h return ok with AX
        mov [esp+28],ax                 ; put AX onto stack for POPAD

;-----------------------------------------------------------------------------
int31ok:                                ; INT 31h return ok, pop all regs
        popad

;-----------------------------------------------------------------------------
int31oknopopgeneral:                    ; INT 31h return ok, pop all segregs
        pop gs fs es ds

;-----------------------------------------------------------------------------
int31oknopop:                           ; INT 31h return ok with carry clear
        and byte ptr [esp+8],0feh       ; clear carry in EFLAGS on stack
        iretd

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; DESCRIPTOR FUNCTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;-----------------------------------------------------------------------------
int31testsel:                           ; test for valid selector BX
        pop bp                          ; pop return address

        cmp bx,cs:gdtlimit              ; selector BX out of range?
        ja int31fail                    ; if yes, fail with error 8022h

        mov edi,cs:gdtbase              ; get base of GDT
        and bl,0f8h                     ; mask offset table index and RPL
        movzx ebx,bx                    ; EBX = selector index

        test byte ptr ds:[edi+ebx+6],10h; is descriptor used?
        jz int31fail                    ; if descriptor not used, fail 8022h

        jmp bp                          ; return ok

;-----------------------------------------------------------------------------
int31testaccess:                        ; test access bits in CX
        pop bp                          ; pop return address

        test ch,20h                     ; test MUST BE 0 bit in CH
        jnz int31fail                   ; if not 0, error 8021h

        test cl,90h                     ; test present and MUST BE 1 bits
        jz int31fail                    ; if both 0, error 8021h
        jpo int31fail                   ; if unequal, error 8021h

        test cl,60h                     ; test DPL
        jnz int31fail                   ; if not 0, error 8021h

        test cl,8                       ; if code, more tests needed
        jz short @@int31testselok       ; if data, skip code tests

        test cl,6                       ; test conforming and readable bits
        jz int31fail                    ; if both 0, error 8021h
        jpe int31fail                   ; if equal, error 8021h

@@int31testselok:
        jmp bp                          ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310000:                              ; allocate descriptors
        or cx,cx                        ; if CX = 0, error 8021h
        jz int31fail

        mov edx,cs:gdtbase              ; get base of GDT
        movzx eax,cs:gdtlimit           ; EAX = last selector index
        and al,0f8h

        mov bx,cx                       ; BX = number of selectors to find
@@int310000l0:
        test byte ptr ds:[edx+eax+6],10h; is descriptor used?
        jnz short @@int310000l0f0

        dec bx                          ; found free descriptor, dec counter
        jnz short @@int310000l0f1       ; continue if need to find more

        mov ebx,eax                     ; found all descriptors requested
@@int310000l1:
        mov dword ptr ds:[edx+ebx],0    ; set entire new descriptor
        mov dword ptr ds:[edx+ebx+4],109200h
        add bx,8                        ; increment selector index
        dec cx                          ; dec counter of descriptors to mark
        jnz @@int310000l1               ; loop if more to mark

        jmp int31okax                   ; return ok, with AX

@@int310000l0f0:
        mov bx,cx                       ; reset number of selectors to find

@@int310000l0f1:
        sub ax,8                        ; dec current selector counter
        cmp ax,8*SYSSELECTORS           ; more descriptors to go?
        jae @@int310000l0               ; if yes, loop

        jmp int31fail                   ; did not find descriptors

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310001:                              ; free descriptor
        call int31testsel               ; test for valid selector BX

        and byte ptr ds:[edi+ebx+6],0efh; mark descriptor as free

        mov cx,4                        ; zero any segregs loaded with BX
        lea ebp,[esp+32]                ; EBP -> selectors on stack
@@int310001l0:
        cmp word ptr [ebp],bx           ; selector = BX?
        jne short @@int310001l0f0       ; if no, continue loop

        mov word ptr [ebp],0            ; zero selector on stack

@@int310001l0f0:
        add ebp,2                       ; increment selector ptr
        loop @@int310001l0              ; loop

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310002:                              ; segment to descriptor
        mov ecx,09200000h               ; convert seg value to descriptor val
        mov cx,bx
        shl ecx,4

        mov edi,cs:gdtbase              ; EDI -> base of GDT
        mov eax,8*FREESELECTORS

        jmp short @@00

@@00l:
        cmp dword ptr ds:[edi+eax+2],ecx
        je short @@01

        add ax,8

@@00:
        cmp ax,cs:gdtfree
        jb @@00l

        test byte ptr ds:[edi+eax+6],10h; is descriptor used?
        jnz int31fail                   ; if used, fail

        mov word ptr ds:[edi+eax],-1    ; set up descriptor
        mov dword ptr ds:[edi+eax+2],ecx
        mov word ptr ds:[edi+eax+6],10h

        add fs:[gdtfree],8              ; set new base of free selectors

@@01:
        jmp int31okax                   ; return ok with selector

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310003:                              ; get selector increment value mov
        mov ax,8                        ; selector increment value is 8
        jmp int31okax                   ; return ok, with AX

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310006:                              ; get segment base address
        call int31testsel               ; test for valid selector BX

        mov dx,word ptr ds:[edi+ebx+2]  ; low word of 32bit linear address
        mov cl,byte ptr ds:[edi+ebx+4]  ; high word of 32bit linear address
        mov ch,byte ptr ds:[edi+ebx+7]

        jmp int31okcx                   ; return ok, with CX, DX, AX

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310007:                              ; set segment base address
        call int31testsel               ; test for valid selector BX

        mov word ptr ds:[edi+ebx+2],dx  ; low word of 32bit linear address
        mov byte ptr ds:[edi+ebx+4],cl  ; high word of 32bit linear address
        mov byte ptr ds:[edi+ebx+7],ch

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310008:                              ; set segment limit
        call int31testsel               ; test for valid selector BX

        cmp cx,0fh                      ; limit greater than 1M?
        jbe short @@int310008f0

        mov ax,dx                       ; yup, limit greater than 1M
        and ax,0fffh
        cmp ax,0fffh                    ; low 12 bits set?
        jne int31fail                   ; if no, error 8021h

        shrd dx,cx,12                   ; DX = low 16 bits of page limit
        shr cx,12                       ; CL = high 4 bits of page limit
        or cl,80h                       ; set granularity bit in CL

@@int310008f0:
        mov word ptr ds:[edi+ebx],dx    ; put low word of limit
        and byte ptr ds:[edi+ebx+6],70h ; mask off G and high nybble of limit
        or byte ptr ds:[edi+ebx+6],cl   ; put high nybble of limit

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310009:                              ; set descriptor access rights
        call int31testsel               ; test for valid selector BX

        call int31testaccess            ; test access bits in CX

        or ch,10h                       ; set AVL bit, descriptor used
        and ch,0f0h                     ; mask off low nybble of CH
        and byte ptr ds:[edi+ebx+6],0fh ; mask off high nybble access rights
        or byte ptr ds:[edi+ebx+6],ch   ; or in high access rights byte
        mov byte ptr ds:[edi+ebx+5],cl  ; put low access rights byte

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int31000a:                              ; create alias descriptor
        call int31testsel               ; test for valid selector BX

        mov cx,1                        ; allocate descriptor
        xor ax,ax
        int 31h
        jc int31fail                    ; if failed, descriptor unavailable

        push ax                         ; preserve allocated selector

        push ds                         ; copy descriptor and set type data
        pop es
        movzx edi,ax                    ; EDI = target selector
        mov esi,cs:gdtbase              ; ESI -> GDT
        add edi,esi                     ; adjust to target descriptor in GDT
        add esi,ebx                     ; adjust to source descriptor in GDT

        movs dword ptr es:[edi],dword ptr ds:[esi]      ; copy descriptor
        lods dword ptr ds:[esi]
        mov ah,92h                      ; set descriptor type - R/W up data
        stos dword ptr es:[edi]

        pop ax                          ; restore allocated selector

        jmp int31okax                   ; return ok, with AX

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int31000b:                              ; get descriptor
        call int31testsel               ; test for valid selector BX

        lea esi,[edi+ebx]               ; ESI -> descriptor in GDT
        mov edi,[esp]                   ; get EDI buffer ptr from stack
        movs dword ptr es:[edi],dword ptr ds:[esi]      ; copy descriptor
        movs dword ptr es:[edi],dword ptr ds:[esi]

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int31000c:                              ; set descriptor
        call int31testsel               ; test for valid selector BX

        mov esi,[esp]                   ; ESI = EDI buffer ptr from stack
        mov cx,es:[esi+5]               ; get access rights from descriptor
        call int31testaccess            ; test access bits in CX

        push ds                         ; swap DS and ES, target and source
        push es
        pop ds
        pop es

        add edi,ebx                     ; adjust EDI to descriptor in GDT
        movs dword ptr es:[edi],dword ptr ds:[esi]      ; copy descriptor
        movs word ptr es:[edi],word ptr ds:[esi]
        lods word ptr ds:[esi]
        or al,10h                       ; set descriptor AVL bit
        stos word ptr es:[esi]

        jmp int31ok                     ; return ok

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; DOS MEMORY BLOCK FUNCTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310100:                              ; allocate DOS memory block
        mov ah,48h
        call int3101doscall
        jc int31failbx

        mov dx,ax

        mov bx,ds
        mov ax,0ah
        int 31h
        jnc short @@int310100f0

        mov ah,49h
        call int3101doscall
        jmp int31fail

@@int310100f0:
        mov bx,ax
        push dx

        mov cx,dx
        shl dx,4
        shr cx,12
        mov ax,7
        int 31h

        mov dx,bx
        pop ax
        jmp int31okdx

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310101:                              ; free DOS memory block
        mov si,dx
        mov ah,49h
        call int3101dossegcall
        mov bx,si
        jc int31failax

        jmp int310001

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310102:                              ; resize DOS memory block
        mov ah,4ah
        call int3101dossegcall
        jc int31failbx

        jmp int31ok

;-----------------------------------------------------------------------------
int3101dossegcall:
        pop bp

        push ax bx
        mov bx,dx
        mov ax,6
        int 31h
        pop bx ax
        jc int31fail

        shrd dx,cx,4
        push bp

;-----------------------------------------------------------------------------
int3101doscall:
        xor ecx,ecx
        push ecx
        sub esp,0ah
        push dx
        push cx
        pushad

        push ss
        pop es
        mov edi,esp

        mov bl,21h
        mov ax,300h
        int 31h

        mov bx,[esp+10h]
        mov ax,[esp+1ch]

        lea esp,[esp+32h]
        pop bp
        jc int31fail

        bt word ptr [esp-14h],0
        jmp bp

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INTERRUPT FUNCTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;-----------------------------------------------------------------------------
int31remapIRQnum:                       ; remap logical IRQ number to physical
        push ax cx

        mov cx,cs:word ptr picslave
        mov bh,bl
        and bh,0f8h
        mov al,bl
        and al,7

        cmp bh,70h
        jne short @@00

        mov bl,cl
        jmp short @@01

@@00:
        cmp bh,8
        jne short @@01

        mov bl,ch

@@01:
        or bl,al

        pop cx ax
        ret

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310200:                              ; get real mode interrupt vector
        movzx ebx,bl                    ; EBX = BL (interrupt number)
        shl bx,2                        ; BX -> interrupt vector

        mov dx,ds:[ebx]                 ; load real mode vector offset
        mov cx,ds:[ebx+2]               ; load real mode vector segment

        jmp int31okcx                   ; return ok, with CX, DX, AX

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310201:                              ; set real mode interrupt vector
        movzx ebx,bl                    ; EBX = BL (interrupt number)
        shl bx,2                        ; BX -> interrupt vector

        mov ds:[ebx],dx                 ; set real mode vector offset
        mov ds:[ebx+2],cx               ; set real mode vector segment

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310204:                              ; get protected mode interrupt vector
        call int31remapIRQnum           ; remap logical IRQ number to physical

        movzx ebx,bl                    ; EBX = BL (interrupt number)

ifdef   EXCEPTIONS
        cmp bl,0eh                      ; exception interrupt?
        ja short @@00                   ; if no, process normally

        lea ebx,[ebx*2+ebx]             ; adjust for location in vector table
        shl bx,1

        mov edx,dword ptr cs:exceptionivect[bx] ; get offset of vector
        mov cx,word ptr cs:exceptionivect[bx+4] ; get selector of vector

        jmp int31okedx                  ; return ok, with EDX, CX, AX
else
        cmp bl,3                        ; INT 3 vector?
        je short @@int310204f00         ; if yes, go to special INT 3 handling
endif

;-----------------------------------------------------------------------------
@@00:
        shl ebx,3                       ; adjust for location in IDT
        add ebx,cs:idtbase              ; add base of IDT

        mov edx,dword ptr ds:[ebx+4]    ; get high word of offset
        mov dx,word ptr ds:[ebx]        ; get low word of offset
        mov cx,word ptr ds:[ebx+2]      ; get selector

        jmp int31okedx                  ; return ok, with EDX, CX, AX

;-----------------------------------------------------------------------------
ifndef  EXCEPTIONS
@@int310204f00:
        mov edx,dword ptr cs:int3vector[0]      ; get offset of INT 3
        mov cx,word ptr cs:int3vector[4]        ; get selector of INT 3

        jmp int31okedx                  ; return ok, with EDX, CX, AX
endif

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310205:                              ; set protected mode interrupt vector
        call int31remapIRQnum           ; remap logical IRQ number to physical

        xchg bx,cx                      ; swap int number with int selector
        call int31testsel               ; test for valid selector BX
        xchg bx,cx                      ; swap int number with int selector

        mov bh,bl                       ; isolate IRQ base interrupt number
        and bh,0f8h
        mov ax,word ptr cs:picslave     ; AX = PIC interrupt numbers

        cmp bh,ah                       ; low IRQ?
        je short @@int310205f2

        cmp bh,al                       ; high IRQ?
        jne short @@int310205f1

        sub bh,8

@@int310205f2:
        movzx eax,bl                    ; EAX = IRQ number
        sub al,bh
        movzx edi,fs:defirqintnum[eax]  ; EDI = default IRQ interrupt number

        cmp cx,SELCODE                  ; setting to default handler?
        je short @@int310205f3

        bts fs:irqhookmap,ax            ; no, set up real mode pass-up hook
        lea esi,[rmirqmatrix+eax*4]
        db 66h,81h,0c6h                 ; ADD ESI,PMODE_TEXT << 16
        dw 0,PMODE_TEXT                 ;
        mov ds:[edi*4],esi

        jmp short @@int310205f1

@@int310205f3:
        btr fs:irqhookmap,ax            ; resetting default handler
        jnc short @@int310205f1

        mov esi,dword ptr cs:[rmirqvector+eax*4]
        mov ds:[edi*4],esi              ; restore real mode vector to old one

;-----------------------------------------------------------------------------
@@int310205f1:                          ; check for real mode pass-up INTs
        movzx ebx,bl                    ; EBX = BL (interrupt number)

        xor si,si                       ; index 0, INT 1Ch

        cmp bl,1ch                      ; INT 1Ch timer interrupt?
        je short @@int310205f4

        add si,4                        ; index 1, INT 23h

        cmp bl,23h                      ; DOS break interrupt?
        je short @@int310205f4

        add si,4                        ; index 2, INT 24h

        cmp bl,24h                      ; DOS critical error interrupt?
        jne short @@int310205f5

@@int310205f4:
        lea edi,[4*ebx]                 ; EDI = rel mode interrupt vector addx
        mov eax,cs:rmintvector[di]      ; get old RM pass up int vector

        cmp cx,SELCODE                  ; resetting original vector?
        je short @@int310205f6          ; if yes, restore RM int vector

        mov eax,cs:int1ccallback[si]    ; get callback address for pass up int

@@int310205f6:
        mov ds:[di],eax                 ; store new RM interrupt address

;-----------------------------------------------------------------------------
@@int310205f5:                          ; set actual vector
ifdef   EXCEPTIONS
        cmp bl,0eh                      ; exception interrupt?
        ja short @@00                   ; if no, process normally

        lea ebx,[ebx*2+ebx]             ; adjust for location in vector table
        shl bx,1

        mov dword ptr fs:exceptionivect[bx],edx ; set offset of vector
        mov word ptr fs:exceptionivect[bx+4],cx ; set selector of vector

        jmp int31ok                     ; return ok
else
        cmp bl,3                        ; INT 3 vector?
        jne short @@00                  ; if no, normal interrupt vector set

        mov dword ptr fs:int3vector[0],edx      ; set offset of INT 3
        mov word ptr fs:int3vector[4],cx        ; set selector of INT 3

        jmp int31ok                     ; return ok
endif

;-----------------------------------------------------------------------------
@@00:
        shl ebx,3                       ; adjust for location in PM IDT
        add ebx,cs:idtbase              ; add base of IDT

        mov word ptr ds:[ebx],dx        ; set low word of offset
        shr edx,16
        mov word ptr ds:[ebx+6],dx      ; set high word of offset
        mov word ptr ds:[ebx+2],cx      ; set selector

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310900:                              ; get and disable interrupt state
        btc word ptr [esp+28h+8],9      ; test and clear IF bit in EFLAGS

;-----------------------------------------------------------------------------
int310900f0:
        setc al                         ; set AL = carry (IF flag from EFLAGS)

;-----------------------------------------------------------------------------
int310900f1:
        add esp,20h                     ; adjust stack

        jmp int31oknopopgeneral         ; return ok, dont pop general regs

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310901:                              ; get and enable interrupt state
        bts word ptr [esp+28h+8],9      ; test and set IF bit in EFLAGS

        jmp int310900f0                 ; return ok, dont pop registers

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310902:                              ; get interrupt state
        bt word ptr [esp+28h+8],9       ; just test IF bit in EFLAGS

        jmp int310900f0                 ; return ok, dont pop registers

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; REAL/PROTECTED MODE TRANSLATION FUNCTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310300:                              ; simulate real mode interrupt
        movzx ebx,bl                    ; get real mode INT CS:IP
        mov ebp,dword ptr ds:[ebx*4]
        jmp short int31030201           ; go to common code

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310301:                              ; call real mode FAR procedure
                                        ; same start as function 0302h
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310302:                              ; call real mode IRET procedure
        mov ebp,dword ptr es:[edi+2ah]  ; get target CS:IP from structure

;-----------------------------------------------------------------------------
int31030201:                            ; common to 0300h, 0301h, and 0302h
        movzx ebx,word ptr es:[edi+2eh] ; EBX = SP from register structure
        movzx edx,word ptr es:[edi+30h] ; EDX = SS from register structure

        mov ax,bx                       ; check if caller provided stack
        or ax,dx
        jnz short @@int31030201f3       ; if yes, go on to set stack

        mov dx,cs:rmstacktop            ; DX = SS for real mode redirection
        mov bx,cs:_pm_rmstacklen        ; get size of real mode stack
        sub dx,bx                       ; adjust DX to next stack location

        cmp dx,cs:rmstackbase           ; exceeded real mode stack space?
        jb abort_INT3103                ; if yes, abort

        mov fs:rmstacktop,dx            ; update ptr for possible reenterancy
        shl bx,4                        ; adjust BX from paragraphs to bytes

@@int31030201f3:
        lea edi,[edx*4]                 ; EDI -> top of real mode stack
        lea edi,[edi*4+ebx]

        mov ax,ss                       ; preserve top of stack parms for
        xchg ax,fs:pmstackss            ;  possible reenterancy
        push ax
        lea eax,[esp-4]
        xchg eax,fs:pmstackesp
        push eax

        movzx eax,cx                    ; EAX = length of stack parms
        shl ax,1
        sub bx,2eh                      ; adjust real mode SP for needed vars
        sub bx,ax                       ; adjust real mode SP for stack parms

        push ds                         ; swap DS and ES
        push es
        pop ds
        pop es

        std                             ; string copy backwards

        sub edi,2                       ; copy stack parms from protected mode
        movzx ecx,cx                    ;  stack to real mode stack
        lea esi,[esp+ecx*2+3ah-2]
        rep movs word ptr es:[edi],word ptr ss:[esi]

        mov esi,[esp+6]                 ; ESI = offset of structure from stack
        mov ax,[esi+20h]                ; AX = FLAGS from register structure

        cmp byte ptr [esp+34],1         ; check AL on stack for function code
        je short @@int31030201f4        ; if function 0301h, go on

        and ah,0fch                     ; 0300h or 0302h, clear IF and TF flag
        stos word ptr es:[edi]          ; store data for real mode return IRET

        dec bx                          ; decrement stack pointer for FLAGS
        dec bx

@@int31030201f4:
        cld                             ; string copy forward

        lea edi,[edx*4]                 ; EDI -> bottom of stack
        lea edi,[edi*4+ebx]

        mov cl,8                        ; copy general regs to real mode stack
        rep movs dword ptr es:[edi],dword ptr ds:[esi]

        add esi,6                       ; copy FS and GS to real mode stack
        movs dword ptr es:[edi],dword ptr ds:[esi]

        mov word ptr es:[edi+8],PMODE_TEXT      ; return address from call
        mov word ptr es:[edi+6],off @@int31030201f1

        mov es:[edi+4],ax               ; store FLAGS for real mode IRET maybe
        mov dword ptr es:[edi],ebp      ; put call address to real mode stack

        mov cx,[esi-8]                  ; real mode ES from register structure
        mov ax,[esi-6]                  ; real mode DS from register structure

        mov di,off @@int31030201f0
        mov si,PMODE_TEXT               ; real mode target CS:IP

        db 66h                          ; JMP DWORD PTR, as in 32bit offset,
        jmp word ptr cs:pmtormswrout    ;  not seg:16bit offset

@@int31030201f0:                        ; real mode INT, FAR, or IRET call
        popad                           ; load regs with call values
        pop fs gs

        iret                            ; go to call address

@@int31030201f1:
        push gs fs ds es                ; store registers on stack
        pushf                           ; store flags on stack
        pushad
        cli

        mov ax,ss                       ; EAX = linear ptr to SS
        movzx eax,ax
        shl eax,4

        movzx ebp,sp                    ; EBP -> stored regs on stack
        add ebp,eax

        mov ebx,cs:pmstackesp           ; get protected mode SS:ESP
        mov dx,cs:pmstackss

        mov edi,off @@int31030201f2     ; target CS:EIP in protected mode
        mov si,SELCODE
        mov cx,SELDATA                  ; ES selector value for protected mode
        mov ax,SELZERO                  ; DS selector value for protected mode

        jmp cs:rmtopmswrout             ; go back to protected mode

@@int31030201f2:
        cld

        push es                         ; FS = ES (SELDATA)
        pop fs

        pop es:pmstackesp               ; restore old stack parameter length
        pop es:pmstackss

        mov es,[esp+36]                 ; get structure ES:EDI from stack
        mov edi,[esp]
        mov esi,ebp                     ; copy return regs from real mode
        mov ecx,15h                     ;  stack to register structure
        rep movs word ptr es:[edi],word ptr ds:[esi]

        cmp dword ptr es:[edi+4],0      ; stack provided by caller?
        jne int31ok                     ; if yes, done now

        mov ax,cs:_pm_rmstacklen        ; restore top of real mode stack
        add fs:rmstacktop,ax

        jmp int31ok                     ; return ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310303:                              ; allocate real mode callback address
        mov bl,cs:_pm_callbacks         ; CL = total number of callbacks
        or bl,bl                        ; are there any?
        jz int31fail                    ; if no, error 8015h

        mov edx,cs:callbackbase         ; EDX -> base of callbacks
        mov ecx,edx                     ; for later use

@@int310303l0:
        cmp word ptr [edx+3],0          ; is this callback free?
        jz short @@int310303f0          ; if yes, allocate

        add edx,25                      ; increment ptr to callback
        dec bl                          ; decrement loop counter
        jnz @@int310303l0               ; if more callbacks to check, loop

        jmp int31fail                   ; no free callback, error 8015h

@@int310303f0:
        mov bx,[esp+38]                 ; BX = caller DS from stack
        mov [edx+3],bx                  ; store callback parms in callback
        mov [edx+7],esi
        mov [edx+12],es
        mov [edx+16],edi

        sub edx,ecx                     ; DX = offset of callback
        shr ecx,4                       ; CX = segment of callback

        jmp int31okcx                   ; return ok, with CX, DX, AX

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310304:                              ; free real mode callback address
        cmp cx,cs:callbackseg           ; valid callback segment?
        jne int31fail                   ; if no, error 8024h

        movzx ebx,dx                    ; EBX = offset of callback

        xor ax,ax                       ; check if valid offset
        xchg dx,ax
        mov cx,25
        div cx

        or dx,dx                        ; is there a remainder
        jnz int31fail                   ; if yes, not valid, error 8024h

        or ah,ah                        ; callback index too big?
        jnz int31fail                   ; if yes, not valid, error 8024h

        cmp al,cs:_pm_callbacks         ; callback index out of range?
        jae int31fail                   ; if yes, not valid, error 8024h

        add ebx,cs:callbackbase         ; EBX -> callback
        mov word ptr [ebx+3],0          ; set callback as free

        jmp int31ok                     ; return ok

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; MISC FUNCTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310305:                              ; get state save/restore addresses
        xor ax,ax                       ; size needed is none
        mov cx,off vxr_saverestorerm    ; same offset of 16bit RETF
        mov edi,off vxr_saverestorepm   ; offset of simple 32bit RETF

;-----------------------------------------------------------------------------
int310305f0:
        mov si,cs                       ; selector of routine is this one
        mov bx,PMODE_TEXT               ; real mode seg of RETF

        jmp int310900f1

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310306:                              ; get raw mode switch addresses
        mov edi,cs:pmtormswrout         ; offset in this seg of rout
        mov cx,cs:rmtopmswrout          ; offset of rout in real mode

        jmp int310305f0

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310400:                              ; get version
        mov ax,5ah                      ; return version 0.9
        mov bx,3                        ; capabilities
        mov cl,cs:processortype         ; processor type
        mov dx,word ptr cs:picslave     ; master and slave PIC values

        jmp int310900f1

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; EXTENDED MEMORY FUNCTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int3105ff:                              ; initialize protected mode system
        cmp cs:pmodetype,2
        jne @@00

;-----------------------------------------------------------------------------
        movzx esi,cs:pagetablenum
        shl esi,12

        mov ecx,cs:pagetablebase
        mov ebx,cs:pagetablefree
        add esi,ecx
        sub esi,ebx

        shl esi,10

        call extmemadjust
        xor ebp,ebp
        shr esi,12
        jz short @@01

        mov fs:cleanuptype,4

        mov eax,ebx
        sub eax,ecx
        shl eax,10
        mov fs:extmembase,eax

@@00l:
        mov ax,0de04h
        call fword ptr cs:vcpi_calleip

        or ah,ah
        jnz short @@01

        and dh,0f0h
        mov dl,7
        mov [ebx],edx

        inc ebp
        add ebx,4

        dec esi
        jnz @@00l

@@01:
        mov fs:pagetablepages,ebp

        jz @@03

;-----------------------------------------------------------------------------
        mov esi,ebp
        shl esi,12
        sub esi,cs:_pm_maxextmem
        jae @@03

        neg esi
        shr esi,12
        jz @@03

        movzx ecx,cs:xms_mem
        or cx,cx
        jz @@03

        movzx eax,cs:pagetablenum
        shl eax,12
        add eax,cs:pagetablebase
        sub eax,ebx
        shr eax,2

        cmp esi,eax
        jb short @@06

        mov esi,eax

@@06:
        push ebx edi

        push ss
        pop es
        sub esp,32h
        mov edi,esp

        xor ebx,ebx
        mov [esp+20h],bx
        mov [esp+2eh],ebx
        mov eax,dword ptr cs:xms_callip
        mov [esp+2ah],eax

        shl esi,2
        cmp esi,ecx
        jb short @@05

        mov esi,ecx

@@05:
        mov [esp+14h],si
        mov byte ptr [esp+1dh],9
        xor cx,cx
        mov ax,301h
        int 31h

        cmp [esp+1ch],bx
        je short @@04

        mov ax,[esp+14h]
        mov fs:xms_handle,ax
        or fs:vcpi_memused,1

        mov byte ptr [esp+1dh],0ch
        mov ax,301h
        int 31h

        cmp [esp+1ch],bx
        je short @@04

;-----------------------------------------------------------------------------
        mov ax,[esp+14h]
        shl eax,16
        mov ax,[esp+10h]

        lea ebx,[eax+0fffh]
        and bx,0f000h
        sub ebx,eax
        add eax,ebx

        shl esi,10
        sub esi,ebx
        shr esi,12
        jz short @@04

        push ds
        pop es

        mov edi,[esp+32h+4]
        or al,7

@@02l:
        stos dword ptr es:[edi]

        inc ebp
        add eax,1000h

        dec esi
        jnz @@02l

        mov [esp+32h+4],edi

@@04:
        add esp,32h

        pop edi ebx

;-----------------------------------------------------------------------------
@@03:
        mov eax,cs:vcpi_cr3
        mov cr3,eax

ifdef   INT3108FUNC
        movzx ecx,cs:_pm_pampagemax
        sub ebp,ecx
        jnc short @@02

        add ecx,ebp
        xor ebp,ebp
endif

@@02:
        shl ebp,12
        mov fs:extmemsize,ebp

ifdef   INT3108FUNC
        or cl,cl
        jz @@00

        push edi

        push ds
        pop es

        mov esi,ecx
        shl esi,2
        neg esi
        add esi,ebx

        mov edx,cs:pagetablebase

        mov edi,esi
        sub edi,edx
        shl edi,10
        push edi

        mov ebp,ecx

        shl ecx,10
        xor eax,eax
        rep stos dword ptr es:[edi]
        push edi

        lea edi,[edx-800h]

        xor ebx,ebx
        cmp cs:processortype,3
        seta bl
        cmp bl,1
        cmc
        rcl bx,4

@@01l:
        lods dword ptr ds:[esi]
        or eax,ebx
        stos dword ptr es:[edi]

        dec ebp
        jnz @@01l

        pop ecx
        pop eax
        pop edi

        mov fs:pampagetop,ecx
        mov fs:pampagebase,eax
        mov fs:pampagemask,bx

        mov fs:int3108routtbl[0],off int310800
        mov fs:int3108routtbl[2],off int310801

        mov eax,cs:vcpi_cr3
        mov cr3,eax
endif

;-----------------------------------------------------------------------------
@@00:
        mov esi,cs:extmemsize
        mov edx,cs:extmembase

        or esi,esi
        jz int31ok

        sub esi,12
        lea ebx,[edx+esi+8]
        mov fs:extmemlast,ebx

        mov eax,80000000h
        mov [edx],eax
        mov [edx+4],esi
        mov [ebx],eax

        mov fs:int3105routtbl[0],off int310500
        mov fs:int3105routtbl[2],off int310501
        mov fs:int3105routtbl[4],off int310502
        mov fs:int3105routtbl[6],off int310503

        jmp int31ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310500:                              ; get free memory information
        mov ebp,cs:extmemlast
        mov esi,cs:extmembase
        xor eax,eax

@l00l:
        mov ebx,[esi]
        btr ebx,31
        jc short @l01

        cmp ebx,eax
        jb short @l01

        mov eax,ebx

@l01:
        lea esi,[esi+ebx+4]

        cmp esi,ebp
        jb @l00l

@l00:
        jmp short int310500mem

;-----------------------------------------------------------------------------
int310500nomem:
        xor eax,eax

;-----------------------------------------------------------------------------
int310500mem:
        stos dword ptr es:[edi]
        mov ecx,0bh
        or eax,-1
        rep stos dword ptr es:[edi]

        jmp int31ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310501:                              ; allocate a block of memory
        call int3105blocksize

        mov eax,ebx
        call _pma_alloc

int310501got:
        or eax,eax
        jz int31fail

        mov di,ax
        mov cx,ax
        shr eax,16
        mov si,ax
        mov bx,ax

        mov [esp+4],si                  ; put SI onto stack for POPAD
        mov [esp],di                    ; put DI onto stack for POPAD
        mov [esp+16],bx                 ; put BX onto stack for POPAD
        mov [esp+24],cx                 ; put CX onto stack for POPAD

        jmp int31ok

;-----------------------------------------------------------------------------
int3105blocksize:
        pop bp

        cmp bx,7fffh
        jae int31fail

        shl ebx,16
        mov bx,cx

        jmp bp

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310502:                              ; free a block of memory
        shl esi,16
        mov si,di
        mov eax,esi
        call _pma_free

        jmp int31ok

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310503:                              ; resize a block of memory
        call int3105blocksize

        shl esi,16
        mov si,di
        mov eax,esi

        push off int310501got
        jmp _pma_realloc

;-----------------------------------------------------------------------------
_pma_alloc:                             ; allocate a block
        push ebx
        push edi

        add eax,3
        and al,0fch

        mov ebp,cs:extmemlast
        mov edi,cs:extmembase

@a00l:
        mov ebx,[edi]
        cmp ebx,eax
        jl short @a01
        je short @a02

        sub ebx,eax
        sub ebx,4
        mov [edi+eax+4],ebx

@a02:
        or eax,80000000h
        mov [edi],eax
        lea eax,[edi+4]
        jmp short @a00

@a01:
        and ebx,7fffffffh
        lea edi,[edi+ebx+4]

        cmp edi,ebp
        jb @a00l

@a03:
        xor eax,eax

@a00:
        pop edi
        pop ebx
        ret

;-----------------------------------------------------------------------------
_pma_free:                              ; free a block
        call pma_findblock
        jnc short @f00

        mov ecx,[edi+ebx+4]
        btr ecx,31
        jc short @f01

        lea ebx,[ebx+ecx+4]

@f01:
        mov ecx,[esi]
        btr ecx,31
        jc short @f02

        mov edi,esi
        lea ebx,[ebx+ecx+4]

@f02:
        mov [edi],ebx

@f00:
        ret

;-----------------------------------------------------------------------------
_pma_realloc:                           ; reallocate (resize) a block
        push ds
        pop es

        lea edx,[ebx+3]
        and dl,0fch

        call pma_findblock
        mov eax,0
        jnc @r00

        cmp ebx,edx
        ja @r02
        je @r01

        mov ecx,[edi+ebx+4]
        btr ecx,31
        jc short @r03

        lea eax,[ecx+ebx+4]
        cmp eax,edx
        ja @r04
        je @r05

        mov ecx,[esi]
        btr ecx,31
        jc short @r06

        lea ecx,[ecx+eax+4]
        cmp ecx,edx
        jb short @r06

        mov ebx,eax
        jmp short @r07

@r03:
        mov ecx,[esi]
        btr ecx,31
        jc short @r06

        lea ecx,[ecx+ebx+4]
        cmp ecx,edx
        jb short @r06

@r07:
        xchg edi,esi
        add esi,4
        lea eax,[edx+80000000h]
        stos dword ptr es:[edi]
        mov eax,edi

        xchg ecx,ebx
        shr ecx,2
        rep movs dword ptr es:[edi],dword ptr ds:[esi]

        sub ebx,edx
        je short @r00

        sub ebx,4
        mov [eax+edx],ebx
        jmp short @r00

@r06:
        mov eax,edx
        call _pma_alloc
        or eax,eax
        jz short @r00

        xchg edi,eax
        lea esi,[eax+4]
        mov edx,edi

        mov ecx,ebx
        shr ecx,2
        rep movs dword ptr es:[edi],dword ptr ds:[esi]

        add eax,4
        call _pma_free

        mov eax,edx
        jmp short @r00

@r04:
        mov ebx,eax

@r02:
        sub ebx,edx
        sub ebx,4
        lea eax,[edx+ebx+4+4]

        mov ecx,[edi+eax]
        btr ecx,31
        jc short @r09

        lea ebx,[ebx+ecx+4]

@r09:
        mov [edi+edx+4],ebx

@r05:
        or edx,80000000h
        mov [edi],edx

@r01:
        lea eax,[edi+4]

@r00:
        ret

;-----------------------------------------------------------------------------
pma_findblock:                          ; find a block and test if used
        mov ebp,cs:extmemlast
        mov edi,cs:extmembase
        lea ebx,[eax-4]

@fb00l:
        cmp edi,ebx
        je short @fb00

        mov esi,edi
        mov ecx,[edi]
        and ecx,7fffffffh
        lea edi,[edi+ecx+4]

        cmp edi,ebp
        jb @fb00l

@fb01:
        clc
        ret

@fb00:
        mov ebx,[edi]
        btr ebx,31
        ret

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; PHYSICAL ADDRESS MAPPING FUNCTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
ifdef   INT3108FUNC

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310800:
        push ds
        pop es

        shl ebx,16
        mov bx,cx

        shl esi,16
        mov si,di

        cmp esi,0
        jle int31fail

        lea esi,[esi+ebx+0fffh]

        push ebx

        shr ebx,12
        shr esi,12
        sub ebx,esi
        neg ebx

        call pamlmalloc

        pop eax

        cmp edx,ebx
        jb int31fail

        mov edx,eax
        and ax,0f000h
        mov ecx,eax
        mov al,7
        or ax,cs:pampagemask

        mov edi,esi

@@00l:
        stos dword ptr es:[edi]
        add eax,1000h

        dec ebx
        jnz @@00l

        or byte ptr [esi+1],2           ; set first allocated page as first
        or byte ptr [edi-3],4           ; set last allocated page as last

        sub esi,cs:pampagebase
        shl esi,10
        bts esi,31

        sub edx,ecx
        add edx,esi

        mov [esp+24],dx                 ; put CX onto stack for POPAD
        shr edx,16
        mov [esp+16],dx                 ; put BX onto stack for POPAD

;-----------------------------------------------------------------------------
int3108done:
        mov eax,cs:vcpi_cr3
        mov cr3,eax

        jmp int31ok

;-----------------------------------------------------------------------------
pamlmalloc:                             ; check for linear memory block
        mov edi,cs:pampagebase          ; EDI = search ptr in page table

        movzx ecx,cs:_pm_pampagemax     ; ECX = count of pages to search
        shl ecx,10

        xor edx,edx                     ; EDX = largest linear block found
        xor eax,eax                     ; EAX = search unit, free entry (0)

        push ebx                        ; preserve EBX, memory requested

@@00l:
        jecxz short @@01                ; if no more entries, done

        repne scas dword ptr es:[edi]   ; search for first next free entry
        jne short @@01                  ; if no more free, go on

        mov ebp,ecx                     ; EBP = current count
        lea ebx,[edi-4]                 ; EBX = start of free block

        repe scas dword ptr es:[edi]    ; search for end of free linear block
        jne short @@00                  ; if previous entry not free, go on

        inc ebp                         ; previous entry free, extra one

@@00:
        sub ebp,ecx                     ; EBP = number of free pages in block

        cmp ebp,edx                     ; new block larger than last largest?
        jb @@00l                        ; if no, loop

        mov esi,ebx                     ; ESI = ptr to largest block found
        mov edx,ebp                     ; size of new largest block found

        cmp ebp,[esp]                   ; block sufficient for memory request?
        jb @@00l                        ; if no, loop

@@01:
        pop ebx                         ; restore EBX, memory requested

        ret                             ; return

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310801:
        push ds
        pop es

        btc bx,15
        jnc int31fail

        shl ebx,16
        mov bx,cx
        shr ebx,10
        and bl,0fch

        add ebx,cs:pampagebase
        cmp ebx,cs:pampagetop
        jae int31fail

        test byte ptr [ebx+1],2
        jz int31fail

@@00l:
        xor eax,eax
        xchg eax,[ebx]

        add ebx,4
        test ah,4

        jz @@00l

        jmp int3108done
endif

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; EXCEPTIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
ifdef   EXCEPTIONS

;-----------------------------------------------------------------------------
int31exceptionnum:                      ; test and set up exception number
        pop bp

        cmp bl,20h                      ; exception number too high?
        jae int31fail                   ; if yes, fail

        xor bh,bh                       ; BX = position in vector table
        lea ebx,[2*ebx+ebx]

        cmp bl,6*0eh                    ; check if exception supported

        lea ebx,[2*ebx+off exceptionevect]

        jmp bp

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310202:                              ; get processor exception vector
        call int31exceptionnum          ; process exception number

        mov cx,cs                       ; CX:EDX = NULL exception vector
        mov edx,0

        ja short @@00                   ; jump if exception not supported

        mov edx,cs:[bx]                 ; CX:EDX = exception vector
        mov cx,cs:[bx+4]

@@00:
        jmp int31okedx                  ; return ok, with EDX, CX, AX

;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int310203:                              ; get processor exception vector
        xchg bx,cx                      ; swap exception number with selector
        call int31testsel               ; test for valid selector BX
        xchg bx,cx                      ; swap exception selector with number

        call int31exceptionnum          ; process exception number
        ja int31ok                      ; done if exception not supported

        mov fs:[bx],edx                 ; store exception vector
        mov fs:[bx+4],cx

        jmp int31ok                     ; return ok
endif

PMODE_TEXT      ends
end

