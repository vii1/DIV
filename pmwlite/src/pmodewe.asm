;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; PMODE/W DOS Extender - Main Code
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

.386p
locals
jumps

STACKLEN        = 40h       ; Stub stack size in para - KEEP THIS 40h OR ABOVE!

PMWVER_MAJOR    = 1
PMWVER_MINOR    = 34

OPENTYPE        = 1
VARTYPE         = 1
;DEBUG           = 1

;PMW1UNCOMP      = 1
LOADLE          = 1
LOADPMW1        = 1

INT33           = 1
INT1B           = 1

EXTMAX          = 7fffffffh
LOWMIN          = 00000h
PM_PMSTACKLEN   = 80h
PM_RMSTACKLEN   = 40h
PM_PMSTACKS     = 8
PM_RMSTACKS     = 8
PM_MODE         = 1
PM_CALLBACKS    = 20h
PM_SELECTORS    = 100h
PM_PAGETABLES   = 4
PM_PAMPAGEMAX   = 1
PM_OPTIONS      = 1

PMODE_TEXT      segment dword public use16 'CODE'
PMODE_TEXT      ends
pmcode16        segment para public use16 'CODE'
pmcode16        ends
pmstack         segment para stack use16 'STACK'
pmstack         ends

extrn   _pm_info:far, _pm_init:far, _pm_cleanup:far

extrn   _pm_pagetables:byte, _pm_selectors:word
extrn   _pm_rmstacklen:word, _pm_pmstacklen:word
extrn   _pm_rmstacks:byte, _pm_pmstacks:byte
extrn   _pm_callbacks:byte
extrn   _pm_mode:byte
extrn   _pm_maxextmem:dword
extrn   _pm_pampagemax:byte
extrn   _pm_options:byte

pmcode16        segment para public use16 'CODE'
                assume cs:pmcode16,ds:pmcode16

ifdef DEBUG
include debug.inc
endif
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; DATA
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
errmsgtbl       dw errmsg0,errmsg1,errmsg2,errmsg3
                dw errmsg4,errmsg5,errmsg6,errmsg7
                dw errmsg8,errmsg9

pmodemsg        db '[PMODE/W]: $'
errmsg0         db 'Not Enough Memory Available!',36
errmsg1         db '80386+ Not Detected!',36
errmsg2         db 'System Already In Protected Mode And No VCPI or DPMI found!',36
errmsg3         db 'DPMI Host Is Not 32bit!',36
errmsg4         db 'A20 Error!',36
errmsg5         db 'DPMI Error!',36
errmsg6         db 'Unable To Allocate Needed DPMI Selectors!',36
errmsg7         db 'Error Loading EXE!',36
errmsg8         db 'Unrecognized Fixup Data!',36
errmsg9         db '16bit Fixup Overflow!',36

_copyrightmsg   db 'PMODE/W DOS Extender v1.34',10,13
                db 'Copyright (C) 1994-2000, Charles Scheffold and Thomas Pytel'
                db 0,10,13,'$'

align 4

IFNDEF  PMW1UNCOMP
decode_bufsize          = 4096
decode_bufbase          dd ?     ;decode_buf
decode_bufthreshold     dd ?     ;decode_buf + decode_bufsize - 16
decode_bufend           dd ?     ;decode_buf + decode_bufsize
decode_filesize         dd ?
decode_handle           dw ?
ENDIF

_lowmin         dw ?

_selbuf         dw ?
_selcode        dw ?
_selzero        dw ?
_selpsp         dw ?
_selfixup       dw ?
_exehandle      dw ?
_pspseg         dw ?
_envseg         dw ?
_selrights      dw ?

_cr0valuerm     dd ?

_membase        dd ?
_memtop         dd ?

FIXUPBUFMIN      = 4000h         ; Must be a multiple of 16 or you will become
_fixupbufptr    dd ?             ; a wart on the ass of some big fat dude.
_fixupbufsize   dd ?

DTABUFSIZE       = 200h          ; Must remain at 200h or above
_dtabufptr      dd ?
_dtabufptr100h  dd ?

_exebaseoffset  dd ?

MAXOBJECTS       = 16
_objectlocs     dd MAXOBJECTS dup (?)
_objectrtoff    label   dword
_objectpageidx  dd MAXOBJECTS dup (?)
_objectrtblocks label   dword
_objectpagenum  dd MAXOBJECTS dup (?)
_objectflags    dd MAXOBJECTS dup (?)
_objectsels     dd MAXOBJECTS dup (?)

_nextobjectloc  dd ?
_numobjects     dd ?
_currentobject  dd ?

_oldfilepos1    dd ?
_datapagesloc   dd ?

_oldint31off    dd ?
_oldint31sel    dw ?

_init_csobj     dd ?
_init_csoff     dd ?
_init_ssobj     dd ?
_init_ssoff     dd ?

_pmode_process  db ?
_pmode_systype  db ?
_inpmode        db 0

borfixgdt       dd 00000000h,00000000h
                dd 0000ffffh,00009200h
 
borfixgdtaddx   dw 0fh,offset borfixgdt,0

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INTERRUPT VARIABLES
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

_termrmvect             dd ?

_int21lowbufseg         dw ?
_int21lowbufptr         dd ?

_int21vectoff           dd ?
_int21vectsel           dw ?
_int21datasel           dw ?
_int21datasel64         dw ?
_int21lowdtaseg         dw ?
_int21lowbufsel         dw ?
_int21dtasel            dw ?
_int21dtaoff            dd ?

IFDEF INT33
_int33vectoff           dd ?
_int33vectsel           dw ?
_int33mousebufsize      dd ?
_int33callback          dd 0
_int33entryflag         db 0
_int330Ccalloff         dd ?
_int330Ccallsel         dw ?
ENDIF

IFDEF INT33 OR INT1B
_int33cursorbufseg      dw ?
ENDIF

IFDEF INT1B
_int1Bcallback          dd 0
_int1Bflag              db 0
ENDIF

_int21lowbufsize        dd 1000h ; MUST Be A Multiple Of 4 Or You Will Suffer!
                                 ; (1000h Bytes Is The Minimum)

_edi                   equ 0
_esi                   equ 4
_ebp                   equ 8
_ebx                   equ 10h
_edx                   equ 14h
_ecx                   equ 18h
_eax                   equ 1ch
_flags                 equ 20h
_es                    equ 22h
_ds                    equ 24h
_fs                    equ 26h
_gs                    equ 28h
_ip                    equ 2ah
_cs                    equ 2ch
_sp                    equ 2eh
_ss                    equ 30h

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; EXE DECOMPRESSION CODE
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

IFNDEF  PMW1UNCOMP
include decode.asm
ENDIF

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; 16-BIT REAL MODE CODE - EXECUTION BEGINS HERE
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
_pm16start:
        .8086
        push cs
        pop ds
        cld

        call _pm_info                   ; Get Protected Mode Info/Detect CPU
        jc _pmstartuperr

        .386p

;-----------------------------------------------------------------------------

        cmp ch,1                        ; Borland DPMI Fix For FS/GS
        ja _noborfix1

        cli
 
        xor eax,eax
        mov ax,cs
        shl eax,4
        add dword ptr [borfixgdtaddx+2],eax
 
        lgdt fword ptr borfixgdtaddx
 
        mov eax,cr0
        or al,1
        mov cr0,eax
 
        jmp short $+2
 
        mov bx,8
        mov fs,bx
        mov gs,bx
 
        and al,0feh
        mov cr0,eax
 
        jmp short $+2
 
        sti

_noborfix1:

;-----------------------------------------------------------------------------

        mov _pspseg,es                  ; Get the PSP and Environment segments
        mov ax,es:[2ch]
        mov _envseg,ax

        xor eax,eax                     ; Get the Base and Top of Low Memory
        mov ax,es:[2]
        mov _memtop,eax
        mov ax,ss
        add ax,STACKLEN
        mov _membase,eax

IFDEF INT1B OR INT33
                                        ; INT 33h Function 9 inherits the old
        mov _int33cursorbufseg,ss       ; real mode stack for its buffer
ENDIF

        mov eax,_int21lowbufsize        ; Allocate the INT 21h Low Buffer
        call _getmem
        jc _memoryerr
        mov _int21lowbufseg,ax
        shl eax,4
        mov _int21lowbufptr,eax

        mov eax,DTABUFSIZE              ; Allocate the INT 21h Disk Transfer
        call _getmem                    ; Area (DTA) buffer. This buffer
        jc _memoryerr                   ; must remain 200h bytes or above
        mov _int21lowdtaseg,ax
        shl eax,4
        mov _dtabufptr,eax
        mov _int21dtaoff,eax
        add eax,100h
        mov _dtabufptr100h,eax

        call _openexe1                  ; Open the EXE file for processing
                                        ; NOTE: This function changes
                                        ; depending on the values of OPENTYPE
                                        ; and VARTYPE

        mov ax,PMODE_TEXT
        mov gs,ax                       ; Increase required callbacks for use
        add gs:_pm_callbacks,5          ; by PMODE/W

        test gs:_pm_options,1
        jz short _nostartmsg
        mov dx,offset _copyrightmsg
        call _pmsg                      ; Display startup message
_nostartmsg:

        call _pm_info                   ; Get Protected Mode Info again in
        jc _pmstartuperr                ; order to save the proper amount of
                                        ; low memory required by PMODEWK which
                                        ; is determined by the PM variables
                                        ; modified in _openexe2
        movzx edx,bx

        mov eax,cr0                     ; Save the real mode value of CR0
        mov _cr0valuerm,eax
        mov eax,dword ptr es:[0ah]
        mov _termrmvect,eax

        mov _pmode_process,cl
        mov _pmode_systype,ch

        db 66h,26h,0c7h,6,0ah,0         ; Install the termination handler
        dw offset _termhandler,pmcode16

;       MOV DWORD PTR ES:[0Ah], pmcode16:_termhandler

        xor ax,ax                       ; EDX = PMODEWK's memory requirements
        mov ebx,_memtop                 ;       in paragraphs
        sub ebx,_membase
        cmp bx,dx
        jb _pmstartuperr                ; Exit if there is not enough memory
        mov es,word ptr _membase        ; otherwise set ES to the block of
        add _membase,edx                ; memory

        call _pm_init                   ; Initialize and enter Protected Mode
        jc _pmstartuperr

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; BEGINNING OF 16-BIT PROTECTED MODE CODE - INITIALIZATIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

        mov _inpmode,0ffh               ; Set protected mode flag for error
                                        ; functions

        mov _selpsp,es                  ; Store PSP selector

        xor edi,edi
        or ecx,-1
        mov ax,cs
        lar dx,ax
        mov dl,dh
        and dl,60h
        or dl,9ah                       ; Allocate a zero based 32bit code
        mov dh,0c0h                     ; descriptor using the same access
        call _initdescriptor            ; rights as the current CS
        mov _selcode,ax


        and dl,NOT 8                    ; Allocate a zero based 32bit data
        mov _selrights,dx               ; descriptor using the same access
        call _initdescriptor            ; rights as the current CS

        mov gs,ax                       ; GS = 32bit data selector
        mov _selzero,ax
        mov _int21dtasel,ax
        mov _int21datasel,ax

        mov edi,_dtabufptr              ; Allocate a 32bit data descriptor
        call _initdescriptor            ; based at the start of the DTA Buffer
        mov _selbuf,ax

        mov edi,_int21lowbufptr         ; Allocate a 32bit data descriptor
        call _initdescriptor            ; based at the start of the INT 21h
        mov _int21lowbufsel,ax          ; Low Buffer

        cmp _pmode_systype,3            ; If the system type is not DPMI
        jz short _nomeminit1            ; then install DPMI memory system
        mov ax,5ffh
        int 31h
_nomeminit1:

        call _initdosext                ; Install 1Bh, 21h, and 33h handlers
        jc _exiterror

        mov ax,204h
        mov bl,31h
        int 31h
        mov _oldint31sel,cx             ; Install INT 31h handler for vendor
        mov _oldint31off,edx            ; specific API function (0A00h). This
        mov ax,205h                     ; handler is installed regardless of
        mov cx,cs                       ; the system PMODE/W is running under
        mov edx,offset _int310A00       ; and is needed in order for FPU
        int 31h                         ; emulation to work.

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; PROTECTED MODE STARTUP
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

        push gs
        pop es                          ; ES = 32bit data selector

        call _loadexe                   ; Load LE/PMW1 EXE Into Memory

        mov ah,3eh                      ; Close EXE file
        mov bx,_exehandle
        int 21h

        mov edx,_init_ssobj
        mov edx,_objectlocs[edx*4-4]
        add edx,_init_ssoff             ; Get initial SS:ESP

        mov ax,es
        mov ss,ax                       ; Setup stack selector
        mov esp,edx                     ; and ESP

        mov edx,_init_csobj
        mov edx,_objectlocs[edx*4-4]
        add edx,_init_csoff             ; Get initial CS:EIP

        push dword ptr _selcode         ; 4G 32bit CS
        push edx                        ; 32bit EIP for program start

        sub esp,32h
        mov ebp,esp
        mov ebx,_membase
        mov ax,_pspseg
        sub bx,ax
        mov word ptr [ebp+_ebx],bx
        mov word ptr [ebp+_es],ax
        mov byte ptr [ebp+_eax+1],4ah   ; Resize PMODE/W's DOS memory block
        call _realmodeint21             ; using real mode INT 21h Function 4Ah
        add esp,32h

        mov es,_selpsp                  ; ES = PSP selector
        push ss
        pop ds                          ; DS = 32bit data selector
        db 66h
        retf                            ; 32bit RETF to new CS:EIP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; SUBROUTINES
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Load The EXE Into Memory (LE or PMW1)
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
;db 'C' XOR 66h ,'.' XOR 66h ,' ' XOR 66h ,'S' XOR 66h ,'C' XOR 66h ,'H' XOR 66h ,'E' XOR 66h ,'F' XOR 66h ,'F' XOR 66h ,'O' XOR 66h ,'L' XOR 66h ,'D' XOR 66h ,'/' XOR 66h
;db 'T' XOR 66h ,'.' XOR 66h ,' ' XOR 66h ,'P' XOR 66h ,'Y' XOR 66h ,'T' XOR 66h ,'E' XOR 66h ,'L' XOR 66h

_loadexe:
        add _lowmin,(FIXUPBUFMIN SHR 4)+1
        jnc short _l1f1
        or _lowmin,0ffffh
_l1f1:

        mov gs,_selbuf                  ; Get LE/PMW1 location from EXE header
        mov edx,gs:[3ch]                ; and seek to it
        mov _exebaseoffset,edx
        call _seekfile

        mov ecx,4
        mov edx,_dtabufptr              ; Read In Executable Signature
        call _readfile

IFDEF   LOADLE
        cmp word ptr gs:[0],'EL'        ; Is it an LE?
        jz _loadLE
ENDIF
IFDEF   LOADPMW1
        cmp dword ptr gs:[0],'1WMP'     ; Is it a PMW1?
        jz _loadPMW1
ENDIF
        jmp _loaderror                  ; If none of the above, quit

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Load A PMW1 Executable - Must ONLY Be Called From _loadexe
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
IFDEF   LOADPMW1
_loadPMW1:
        mov ecx,24h
        add edx,4
        call _readfile                  ; Read in the rest of the PMW1 header

IFNDEF  PMW1UNCOMP
        mov eax,_int21lowbufptr         ; Fill the decompression variables
        mov decode_bufbase,eax
        add eax,decode_bufsize
        mov decode_bufend,eax
        sub eax,16
        mov decode_bufthreshold,eax
        mov ax,_exehandle
        mov decode_handle,ax
ENDIF

        mov eax,gs:[08h]                ; Get initial CS:EIP and SS:ESP
        mov ebx,gs:[0ch]
        mov ecx,gs:[10h]
        mov edx,gs:[14h]
        mov _init_csobj,eax             ; Store them in the proper variables
        mov _init_csoff,ebx             ; for later use
        mov _init_ssobj,ecx
        mov _init_ssoff,edx

        mov eax,gs:[24h]                ; Get the location of the actual
        add eax,_exebaseoffset          ; program data
        mov _datapagesloc,eax

        mov ebp,gs:[1ch]                ; Get the number of objects and exit
        cmp ebp,MAXOBJECTS              ; if there are too many
        ja _loaderror
        mov _numobjects,ebp
        mov edx,gs:[18h]
        add edx,_exebaseoffset
        call _seekfile                  ; Seek to the object table

        xor ebp,ebp
_loadobjects_PMW1:
        call _loadobjects1              ; Allocate memory for current object

        mov ecx,gs:[100h]
        mov edx,gs:[114h]
        cmp ecx,edx                     ; If VirtualSize > ActualSize
        jbe _nozerobss1_PMW1            ; then clear the extra memory
        push edi
        sub ecx,edx
        add edi,edx
        xor al,al
        rep stos byte ptr es:[edi]
        pop edi
_nozerobss1_PMW1:

IFDEF   PMW1UNCOMP
        mov ecx,gs:[104h]
        mov edx,edi
        call _readfile                  ; Read object data into memory
ENDIF

IFNDEF  PMW1UNCOMP
        call _int213Fbuftoggle          ; Turn off read buffer transfer
        mov ecx,gs:[104h]
        mov decode_filesize,ecx         ; Get the compressed block size
        pushad
        call decode                     ; Decompress object data to memory
        popad
        call _int213Fbuftoggle          ; Turn on read buffer transfer
ENDIF

_noreadobject1_PMW1:
        mov eax,ecx                     ; Setup object variables and allocate
        call _loadobjects2              ; any needed object descriptors


        inc ebp
        cmp ebp,_numobjects
        jnz _loadobjects_PMW1

;------------------------------------------------------------------------------

        call _initfixupbuffer           ; Initialize buffer for fixups

        xor ebp,ebp
_relocateobjects_PMW1:
        mov edx,gs:[20h]
        add edx,_exebaseoffset
        mov ebx,ds:_objectrtblocks[ebp*4]

        or ebx,ebx                      ; If there are no relocation blocks
        jz _norelocate1_PMW1            ; then proceed to the next object

        add edx,ds:_objectrtoff[ebp*4]  ; Seek to beginning of relocation info
        call _seekfile                  ; for this object

_relocate1_PMW1:
        mov ecx,4
        mov edx,_dtabufptr100h          ; Read in the size of the next
        call _readfile                  ; relocation block

;        movzx ecx,word ptr gs:[102h]
;        cmp ecx,_fixupbufsize           ; Make sure it will fit in our fixup
;        ja _loaderror                   ; buffer

IFDEF   PMW1UNCOMP
        movzx ecx,word ptr gs:[102h]
        mov edx,_fixupbufptr
        call _readfile                  ; Read in relocation block
ENDIF

IFNDEF  PMW1UNCOMP
        call _int213Fbuftoggle          ; Turn off read buffer transfer
        mov edi,_fixupbufptr
        movzx ecx,word ptr gs:[100h]
        mov decode_filesize,ecx         ; Get compressed block size
        pushad
        call decode                     ; Decompress relocation data
        popad
        call _int213Fbuftoggle          ; Turn on read buffer transfer
ENDIF
        movzx ecx,word ptr gs:[102h]
        mov _nextobjectloc,ecx
        mov _currentobject,ebp
        mov edi,ds:_objectlocs[ebp*4]   ; EDI = Location of object

        xor esi,esi                     ; ESI = 0
        push ebx ebp
_relocate2_PMW1:
        lods byte ptr fs:[esi]
        mov dl,al                       ; DL = Relocation Type
        test dl,20h
        jnz _unknownerror

        lods dword ptr fs:[esi]         ; ES:[EBX+EDI] -> Source
        mov ebx,eax

        xor eax,eax
        lods byte ptr fs:[esi]
        mov ecx,eax                     ; ECX = Target Object Number
        mov ebp,_objectlocs[ecx*4-4]    ; EBP = Target Object Offset

        lods dword ptr fs:[esi]         ; EAX= Target Offset

        call _relocateitem              ; Process relocation item

        cmp esi,_nextobjectloc          ; If not done, process the next
        jnz _relocate2_PMW1             ; relocation item

        pop ebp ebx

        dec ebx                         ; If not done, load the next block of
        jnz _relocate1_PMW1             ; relocation data

_norelocate1_PMW1:
        inc ebp
        cmp ebp,_numobjects             ; If there are more objects, then
        jnz _relocateobjects_PMW1       ; process them

        ret
ENDIF

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Load An LE Executable - Must ONLY Be Called From _loadexe
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
IFDEF   LOADLE
_loadLE:
        mov ecx,0c0h
        add edx,4
        call _readfile                  ; Read in the rest of the LE header

        mov eax,gs:[18h]                ; Get initial CS:EIP and SS:ESP
        mov ebx,gs:[1ch]
        mov ecx,gs:[20h]
        mov edx,gs:[24h]
        mov _init_csobj,eax             ; Store them in the proper variables
        mov _init_csoff,ebx             ; for later use
        mov _init_ssobj,ecx
        mov _init_ssoff,edx

        mov eax,gs:[80h]                ; Get the location of the actual
        mov _datapagesloc,eax           ; program data

        mov ebp,gs:[44h]                ; Get the number of objects and exit
        cmp ebp,MAXOBJECTS              ; if there are too many
        ja _loaderror
        mov _numobjects,ebp
        mov edx,gs:[40h]
        add edx,_exebaseoffset
        call _seekfile                  ; Seek to the object table

        xor ebp,ebp
_loadobjects_LE:
        call _loadobjects1              ; Allocate memory for current object

        mov ecx,gs:[100h]
        mov eax,gs:[110h]
        shl eax,12

        mov edx,ebp
        inc edx
        cmp edx,_numobjects
        jnz _loadobjects_LE_lastpage2
        cmp dword ptr gs:[110h],1
        jz _loadobjects_LE_lastpage1
        sub eax,1000h
        add eax,gs:[2ch]
        jmp short _loadobjects_LE_lastpage2
_loadobjects_LE_lastpage1:
        mov eax,gs:[2ch]
_loadobjects_LE_lastpage2:

        mov edx,eax
        sub edx,ecx
        jnc short _nozerobss1           ; Zero uninitialized data if necessary
        pushad
        sub ecx,eax
        add edi,eax
        xor al,al
        rep stos byte ptr es:[edi]
        popad
        mov edx,ecx
        sub edx,eax
        sub ecx,edx
        jz _noreadobject1
_nozerobss1:
        mov edx,edi
        call _readfile                  ; Read object data into memory
_noreadobject1:

        mov eax,ecx
        mov edx,ecx
        and ax,0f000h
        and edx,0fffh
        jz short _loadobj1              ; Adjust data page additive size
        add eax,1000h                   ; accordingly
_loadobj1:
                                        ; Setup object variables and allocate
        call _loadobjects2              ; any needed object descriptors

        inc ebp
        cmp ebp,_numobjects
        jnz _loadobjects_LE

;------------------------------------------------------------------------------

        call _initfixupbuffer           ; Initialize buffer for fixups

        xor ebp,ebp
_relocateobjects:
        mov edx,gs:[48h]
        add edx,_exebaseoffset

        cmp dword ptr ds:_objectpagenum[ebp*4],0
        jz _norelocate2

        mov eax,ds:_objectpageidx[ebp*4]
        lea edx,[edx+eax*4-4]           ; Seek to the first page table index
        call _seekfile                  ; for this object

        xor ebx,ebx
_relocate1:
        mov ecx,4                       ; Read the next object page map entry
        mov edx,_dtabufptr100h
        call _readfile
        call _filepos                   ; Save file position for later

        movzx eax,word ptr gs:[101h]
        or ax,ax
        jz _norelocate1                 ; Skip if no relocation on this page
        xchg al,ah

        mov edx,gs:[68h]                ; Seek to the fixup page table
        add edx,_exebaseoffset
        lea edx,[edx+eax*4-4]
        call _seekfile

        mov ecx,8                       ; Read 2 fixup entries
        mov edx,_dtabufptr100h
        call _readfile
        mov eax,gs:[100h]               ; If the two entries are identical
        cmp eax,gs:[104h]               ; then there is no relocation here
        jz _norelocate1                 ; and we can skip this page

        mov esi,gs:[100h]
        mov edx,gs:[6ch]
        add edx,_exebaseoffset
        add edx,esi                     ; Seek to the first fixup record
        call _seekfile                  ; for this page

        mov ecx,gs:[104h]
        sub ecx,gs:[100h]               ; If the total size of the fixup data
        cmp ecx,_fixupbufsize           ; for this page is larger than our
        ja _loaderror                   ; buffer, then exit with an error
        mov edx,_fixupbufptr
        call _readfile                  ; Read in the fixup data for this page

        mov edi,ds:_objectlocs[ebp*4]
        mov eax,ebx
        shl eax,12
        add edi,eax                     ; EDI = Offset of the current 4k page

        mov _nextobjectloc,ecx
        mov _currentobject,ebp
        push ebx ebp
        xor esi,esi                     ; ESI = 0
_relocate2:
        lods word ptr fs:[esi]
        mov dx,ax                       ; DX = Fixup flags/type
        test dx,0720h
        jnz _unknownerror

        lods word ptr fs:[esi]
        movsx ebx,ax                    ; ES:[EBX+EDI] -> Source

        xor eax,eax
        lods byte ptr fs:[esi]
        test dh,40h
        jz short _relocate2a
        mov ah,fs:[esi]
        inc esi
_relocate2a:
        mov ecx,eax                     ; ECX = Target Object Number
        mov ebp,_objectlocs[ecx*4-4]    ; EBP = Target Object Offset

        mov al,dl
        and al,0fh
        cmp al,2
        jz _relocate2b
        xor eax,eax
        lods word ptr fs:[esi]
        test dh,10h
        jz short _relocate2b
        rol eax,16
        lods word ptr fs:[esi]
        rol eax,16
_relocate2b:                            ; EAX = Target Offset

        call _relocateitem              ; Process relocation item

        cmp esi,_nextobjectloc          ; If not done, process the next
        jnz _relocate2                  ; relocation item
        pop ebp ebx
_norelocate1:

        mov edx,_oldfilepos1
        call _seekfile                  ; Restore file position
        inc ebx
        cmp ebx,ds:_objectpagenum[ebp*4]
        jnz _relocate1                  ; If not done, process next page
_norelocate2:
        inc ebp
        cmp ebp,_numobjects
        jnz _relocateobjects            ;If not done, process next object

        ret
ENDIF

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; SHARED LOADER ROUTINES USED BY _loadLE AND _loadPMW1
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Allocate Memory For An Object And Seek To The Object Data
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_loadobjects1:
        mov ecx,18h                     ; Read info for this object
        mov edx,_dtabufptr100h
        call _readfile
        call _filepos                   ; Save file position for later

        mov eax,gs:[100h]               ; Allocate some memory for this object
        movzx edx,_lowmin
        mov edi,_memtop
        sub edi,_membase
        mov ecx,eax
        add ecx,15
        shr ecx,4

        sub edi,ecx                     ; If there is not enough low memory
        jc _loadtryhimem                ; then try extended memory

        cmp edi,edx                     ; If there is enough, make sure that
        jae _loadtrylowmem              ; _lowmin is satisfied

        test byte ptr gs:[108h+1],100000b
        jz _loadtrylowmem               ; Load 16bit objects into low memory

        call _gethimem                  ; Try extended memory if _lowmin is
        jnc _loadlowmemok               ; is not satisfied
_loadtrylowmem:
        mov ebx,_membase
        and ebx,0ffh                    ; Make sure memory is 4096 byte aligned
        jz _loadlowmemok1
        dec bx
        xor bl,0ffh
        shl bx,4
        add eax,ebx
_loadlowmemok1:
        call _getmem                    ; Allocate low memory for this object
        jc short _loadtryhimem
        shl eax,4
        add eax,ebx                     ; Align block on 4096 bytes
        jmp short _loadlowmemok
_loadtryhimem:
        test byte ptr gs:[108h+1],100000b
        jz _memoryerr                   ; If this is a 16bit object, exit
        mov eax,gs:[100h]
        call _gethimem                  ; Allocate DPMI memory
        jc _memoryerr
_loadlowmemok:
        mov edi,eax                     ; EDI = Memory block for this object

        mov edx,_datapagesloc
        call _seekfile                  ; Seek to current object data location
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Setup Descriptors For Objects
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_loadobjects2:
        add _datapagesloc,eax           ; Update information for object pages
        mov bx,bp
        shl bx,2

        mov _objectlocs[bx],edi         ; Store object offset

        mov eax,gs:[108h]
        mov _objectflags[bx],eax        ; Store object flags

        test eax,10000000000000b        ; If object is 32bit, store default
        jz _loadobj1a                   ; code & data selectors
        mov ax,_selcode
        mov word ptr _objectsels[bx],ax
        mov ax,_selzero
        mov word ptr _objectsels[bx+2],ax
        jmp short _loadobj1b
_loadobj1a:
        or ecx,-1                       ; If object is 16bit then allocate
        mov dx,_selrights               ; new code and data descriptors
        call _initdescriptor
        mov word ptr _objectsels[bx+2],ax
        or dl,8
        call _initdescriptor
        mov word ptr _objectsels[bx],ax
_loadobj1b:
        mov eax,gs:[10ch]
        mov _objectpageidx[bx],eax      ; Store object page table index
        mov eax,gs:[110h]
        mov _objectpagenum[bx],eax      ; Store number of pages in object

        mov edx,_oldfilepos1
        call _seekfile                  ; Seek to original file position
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Setup Pointers For The Fixup Buffer And Allocate A Descriptor For It
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_initfixupbuffer:
        mov edi,_membase                ; Allocate buffer for fixup information
        shl edi,4                       ; to later be discarded
        mov _fixupbufptr,edi
        or ecx,-1
        mov dx,_selrights
        call _initdescriptor            ; Allocate a descriptor for the buffer
        mov _selfixup,ax
        mov fs,ax
        mov eax,_memtop
        sub eax,_membase
        shl eax,4
        mov _fixupbufsize,eax
        cmp eax,FIXUPBUFMIN             ; If the fixup buffer is too small
        jb _memoryerr                   ; exit with an error
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Relocate One Item
; In:
;   EAX - Target Offset
;   EBP - Zero Based Target Beginning Offset
;    DL - Fixup Type
;   ECX - Target Object Number
;   ES:[EBX+EDI] -> Pointer To Relocation Source
; Out:
;
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_r_table        dw offset _r_byteoffset
                dw offset _r_unknown
                dw offset _r_wordsegment
                dw offset _r_16bitfarptr
                dw offset _r_unknown
                dw offset _r_16bitoffset
                dw offset _r_32bitfarptr
                dw offset _r_32bitoffset
                dw offset _r_nearcalljmp
_relocateitem:
        push esi
        movzx esi,dl
        and esi,0fh
        cmp esi,8
        ja _unknownerror
        or ebx,ebx
        js _r_done
        jmp _r_table[esi*2]

_r_byteoffset:
        mov es:[ebx+edi],al
        jmp _r_done
_r_wordsegment:                         ; Segment/Selector
        call _r_segmentfixup
        mov es:[edi+ebx],bp
        jmp _r_done
_r_16bitfarptr:                         ; 16bit Far Pointer
        call _r_segmentfixup
        mov es:[ebx+edi],ax
        mov es:[ebx+edi+2],bp
        jmp _r_done
_r_16bitoffset:                         ; 16bit Offset
        mov es:[ebx+edi],ax
        jmp _r_done
_r_32bitfarptr:                         ; 32bit Far Pointer
        add eax,ebp
        mov es:[ebx+edi],eax
        call _r_segmentfixup
        mov es:[ebx+edi+4],bp
        jmp _r_done
_r_32bitoffset:                         ; 32bit Offset
        add eax,ebp
        mov es:[ebx+edi],eax
        jmp _r_done
_r_nearcalljmp:                         ; 16/32bit Near Call or Jump
        add eax,ebp
        lea esi,[ebx+edi+4]
        sub eax,esi
        mov ecx,_currentobject
        test word ptr _objectflags[ecx*4],10000000000000b
        jnz _r_nearcalljmp32
        lea ecx,[eax+32768+2]
        test ecx,0ffff0000h
        jnz _16biterror
        mov es:[ebx+edi],ax
        jmp _r_done
_r_nearcalljmp32:
        mov es:[ebx+edi],eax
        jmp _r_done
_r_unknown:
        jmp _unknownerror
_r_done:
        pop esi
        ret

_r_segmentfixup:
        mov esi,_currentobject
        test word ptr _objectflags[esi*4],1000000000000b
        jnz _r_wordsegment2
        test byte ptr _objectflags[ecx*4-4],100b
        jz _r_wordsegment1
        mov bp,word ptr _objectsels[ecx*4-4]
        jmp short _r_wordsegment3
_r_wordsegment1:
        mov bp,word ptr _objectsels[ecx*4-2]
        jmp short _r_wordsegment3
_r_wordsegment2:
        shr ebp,4
;        test dl,10h
;        jz _r_wordsegment3
        test ebp,0ffff0000h
        jnz _16biterror
_r_wordsegment3:
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Setup A New Descriptor
; In:
;   EDI - Base Address
;    DX - Access Rights
;   ECX - Limit
; Out:
;    AX - Selector
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_initdescriptor:
        push ebx ecx edx ebp
        mov ebp,ecx
        xor ax,ax
        mov cx,1
        int 31h                         ; Allocate 1 descriptor
        jc short _initdescerr

        xchg bx,ax
        mov ax,9
        mov cx,dx
        int 31h                         ; Set descriptor access rights
        jc short _initdescerr

        mov ax,7
        mov ecx,edi
        mov dx,cx
        shr ecx,16
        int 31h                         ; Set descriptor base address
        jc short _initdescerr

        mov ax,8
        mov ecx,ebp
        mov dx,cx
        shr ecx,16
        int 31h                         ; Set descriptor limit
        jc short _initdescerr
        mov ax,bx
        clc
_initdescerr:
        pop ebp edx ecx ebx
        jc _descriptorerr
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Quickie Read To Buffer
; In:
;   ECX - Number Of Bytes To Read
;   EDX - Zero Based Buffer Offset
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_readfile:
        push eax bx ds
        mov ah,3fh
        mov bx,_exehandle
        mov ds,_selzero
        int 21h
        pop ds bx eax
        jc _loaderror
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Seek To File Position
; In:
;    EDX - Position
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_seekfile:
        push ax bx ecx dx
        mov ecx,edx
        shr ecx,16
        mov ax,4200h
        mov bx,_exehandle
        int 21h
        pop dx ecx bx ax
        jc _loaderror
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Get Current File Position
; In:
;    None
; Out:
;   _oldfilepos1 = File Position
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_filepos:
        push bx
        xor cx,cx
        xor dx,dx
        mov ax,4201h
        mov bx,_exehandle
        int 21h
        jc _loaderror
        mov word ptr _oldfilepos1[0],ax
        mov word ptr _oldfilepos1[2],dx
        pop bx
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Get Some Low Memory
; In:
;    EAX - Number Of Bytes To Get
; Out:
;   EAX - SEG:OFF Pointer (Offset Always 0)
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_getmem:
        add eax,15
        shr eax,4
        add eax,_membase
        cmp eax,_memtop
        ja _getmemerr
        xchg eax,_membase
        clc
        jmp short $+3
_getmemerr:
        stc
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Get Some High Memory
; In:
;    EAX - Number Of Bytes To Get
; Out:
;   EAX - Absolute Address Of Block
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_gethimem:
        push bx cx si di edx eax
        add eax,4095
        mov cx,ax
        shr eax,16
        mov bx,ax
        mov ax,501h
        int 31h                         ; Call DPMI for some memory
        jc short _gethimemerr
        mov dx,bx                       ; Align it on a 4096 byte boundary
        shl edx,16
        mov dx,cx
        mov bx,dx
        and bx,4095
        jz _gethimem1
        dec bx
        xor bx,4095
_gethimem1:
        movzx eax,bx
        push eax
        add eax,[esp+4]
        mov cx,ax
        shr eax,16
        mov bx,ax
        mov ax,503h
        int 31h                         ; Resize the block if necessary
        jc short _gethimem2
        mov dx,bx
        shl edx,16
        mov dx,cx
_gethimem2:
        pop eax
        add edx,eax
        mov [esp],edx
        clc
_gethimemerr:
        pop eax edx di si cx bx
        ret

_16biterror:
        mov ax,9
        jmp short _pmstartuperr
_descriptorerr:
        mov ax,6
        jmp short _pmstartuperr
_exiterror:
        mov ax,5
        jmp short _pmstartuperr
_unknownerror:
        mov ax,8
        jmp short _pmstartuperr
_memoryerr:
        xor ax,ax
        jmp short _pmstartuperr
_loaderror:
        mov ax,7
_pmstartuperr:
        .8086
        mov si,ax
        add si,ax
        push cs
        pop ds
        mov dx,offset pmodemsg
        call _pmsg
        mov dx,errmsgtbl[si]
        call _pmsg
        mov ah,2
        mov dl,10
        int 21h
        add dl,3
        int 21h
        mov ax,4cffh
        int 21h

_pmsg:
        cmp _inpmode,0ffh
        jz _pmerr1
        mov ah,9
        int 21h
        ret
_pmerr1:
        .386p
        sub esp,32h
        mov ebp,esp
        mov byte ptr [ebp+_eax+1],9
        mov word ptr [ebp+_ds],pmcode16
        mov word ptr [ebp+_edx],dx
        call _realmodeint21
        add esp,32h
        ret

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INT 1Bh, 21h, AND 33h EXTENSIONS
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Initialize All DOS/Mouse Extensions
; In:
;    GS - Zero-Based 4G Data Selector
;    DS - Data Selector Based At Current Segment
;    ES - PSP Selector
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_initdosext:
        pushad
        cld
        sub esp,32h
        mov ebp,esp
        mov _int21datasel64,ds          ; Store the 16bit data selector

        mov dx,_int21lowdtaseg
        mov [ebp+_ds],dx
        mov byte ptr [ebp+_eax+1],1ah
        mov word ptr [ebp+_edx],0       ; Set the real mode DTA segment to
        call _realmodeint21             ; our DTA buffer

        mov ax,204h
        mov bl,21h
        int 31h
        mov _int21vectsel,cx
        mov _int21vectoff,edx
        mov ax,205h
        mov cx,cs
        mov edx,offset _int21handler
        int 31h                         ; Hook protected mode INT 21h

IFDEF   INT33
        cmp dword ptr gs:[33h*4],0      ; Check mouse driver interrupt vector
        jnz _nomousedriver1             ; and continue if its not 0
        mov word ptr ds:_int33handler,0cf66h
        jmp _nomousedriver2
_nomousedriver1:
;        xor ax,ax
        mov ax,21h
        int 33h                         ; Reset the mouse
;        or ax,ax
        cmp ax,21h
        jz _nomousedriver3              ; Don't install handler if it fails
        mov ax,15h
        int 33h                         ; Check mouse storage requirements
        movzx ebx,bx
        mov _int33mousebufsize,ebx
        cmp bx,1000h
        ja _initdosexterror             ; Quit if its too large
        mov esi,offset _int330Cpmentry
        call _initgetcallback
        mov word ptr _int33callback,dx
        mov word ptr _int33callback+2,cx
_nomousedriver2:
        mov ax,204h
        mov bl,33h
        int 31h
        mov _int33vectsel,cx
        mov _int33vectoff,edx
        mov ax,205h
        mov cx,cs
        mov edx,offset _int33handler
        int 31h                         ; Hook protected mode INT 33h
_nomousedriver3:
ENDIF

IFDEF   INT1B
        db 66h,65h,0c7h,6,6ch,0
        dw offset _int1Brmhandler,pmcode16

;       MOV DWORD PTR GS:[6Ch], pmcode16:_int1Brmhandler

        mov esi,offset _int1Bpmchandler
        call _initgetcallback
        mov word ptr _int1Bcallback,dx
        mov word ptr _int1Bcallback+2,cx
        mov ax,205h
        mov bl,1bh
        mov cx,cs
        mov edx,offset _int1Bhandler
        int 31h                         ; Hook protected mode INT 1Bh
ENDIF
        add esp,32h
        clc
        jmp _initdosextnoerror
_initdosexterror:
        add esp,32h
        stc
_initdosextnoerror:
        popad
_ret:
        ret

IFDEF   INT1B OR INT33
_initgetcallback:
        push ds es
        mov ax,303h
        movzx edi,_int33cursorbufseg
        shl edi,4
        add edi,64
        push gs
        pop es
        push cs
        pop ds
        int 31h                         ; Allocate a real mode callback
        pop es ds
        jnc _ret
        add esp,2
        jmp short _initdosexterror
ENDIF
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INT 31H HANDLER
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 31h Service 0A00h - Get API Entry Point
; In:
;   AX - 0A00h
; Out:
;   CF=0
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int310A00:
        cmp ax,0a00h
        jnz short _int31not0A00
        and byte ptr [esp+8],NOT 1
        iretd
_int31not0A00:

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 31h Service 0EEFFh - Get DOS Extender Information
; In:
;   AX - 0EEFFh
; Out:
;  EAX - 'PMDW'
;  ES:EBX -> PMODE/W Copyright String
;   CH - Protected Mode System Type (0=Raw, 1=XMS, 2=VCPI, 3=DPMI)
;   CL - Processor Type (3=386, 4=486, 5=586)
;   DH - Extender MAJOR Version (Binary)
;   DL - Extender MINOR Version (Binary)
;   CF=0
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
        cmp ax,0eeffh
        jnz short _int31not0EEFF
        mov eax,'PMDW'
        mov es,cs:_selzero
        xor ebx,ebx
        mov bx,pmcode16
        shl ebx,4
        add ebx,large offset _copyrightmsg
        mov ch,cs:_pmode_systype
        mov cl,cs:_pmode_process
        mov dh,PMWVER_MAJOR
        mov dl,PMWVER_MINOR
        and byte ptr [esp+8],NOT 1
        iretd
_int31not0EEFF:
        jmp fword ptr cs:_oldint31off


;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INT 33h Handler
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
IFDEF   INT33
_int33handler:
        pushad
        push es
        cld
        cmp ax,9
        jz _int3309
        cmp ax,0ch
        jz _int330C
        cmp ax,16h
        jz _int3316
        cmp ax,17h
        jz _int3317
        pop es
        popad
        jmp fword ptr cs:_int33vectoff

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 33h Function 09h - Set Graphics Cursor Block
; In:
;   AX - 09h
;   BX - Horizontal Hot Spot
;   CX - Vertical Hot Spot
;   ES:EDX -> Mask
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int3309:
        sub esp,32h
        mov ebp,esp
        push ds
        mov word ptr [ebp+_eax],ax
        mov word ptr [ebp+_ebx],bx
        mov word ptr [ebp+_ecx],cx
        push es
        pop ds
        mov esi,edx
        movzx edi,cs:_int33cursorbufseg
        mov word ptr [ebp+_es],di
        mov word ptr [ebp+_edx],0
        shl edi,4
        mov es,cs:_int21datasel
        mov ecx,64
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        call _realmodeint33
        pop ds
        add esp,32h
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 33h Function 0Ch - Set Interrupt Subroutine
; In:
;   AX - 0Ch
;   CX - Call Mask
;   ES:EDX -> Routine
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int330C:
        sub esp,32h
        mov ebp,esp
        push ds
        mov ds,cs:_int21datasel64
        mov word ptr [ebp+_eax],ax
        mov word ptr [ebp+_ecx],cx
        mov word ptr [ebp+_es],pmcode16
        mov word ptr [ebp+_edx],offset _int330Crmentry
        xor eax,eax
        mov ax,es
        or eax,edx
        jnz short _int330C1
        mov word ptr [ebp+_es],dx
        mov word ptr [ebp+_edx],dx
;        mov word ptr [ebp+_ecx],dx
_int330C1:
        cli
        mov _int330Ccallsel,es
        mov _int330Ccalloff,edx
        call _realmodeint33
        sti
        pop ds
        add esp,32h
        jmp _endint21handler

_int330Crmentry:
        push ax
        mov al,1
        xchg al,cs:_int33entryflag
        cmp al,1
        jz short _int330Crmexit
        pop ax
        jmp dword ptr cs:_int33callback
_int330Crmreturn:
        mov cs:_int33entryflag,0
        retf
_int330Crmexit:
        pop ax
        retf

_int330Cpmentry:
        push ss
        push esp
        pushad
        push ds es
        cld

        xor eax,eax
        mov ax,ss
        lar eax,eax
        test eax,400000h
        jnz short _int330Cos2crap
        movzx esp,sp
_int330Cos2crap:

;        xor eax,eax
;        mov ax,ss
;        lar eax,eax
;        test eax,400000h
;        jnz short _int330Cos2crap
;        push es edi
;        sub sp,8
;
;        movzx edi,sp
;        mov bx,ss
;        mov es,bx
;        mov ax,0bh
;        int 31h
;
;        or byte ptr es:[edi+6],40h
;        mov ax,0ch
;        mov bx,cs:_selfixup
;        int 31h
;
;        movzx esp,sp
;        mov ss,bx
;
;        add esp,8
;        pop edi es
;_int330Cos2crap:

        mov word ptr es:[edi+_cs],pmcode16
        mov word ptr es:[edi+_ip],offset _int330Crmreturn
        movzx eax,word ptr es:[edi+_eax]
        movzx ebx,word ptr es:[edi+_ebx]
        movzx ecx,word ptr es:[edi+_ecx]
        movzx edx,word ptr es:[edi+_edx]
        movzx esi,word ptr es:[edi+_esi]
        movzx edi,word ptr es:[edi+_edi]
        call fword ptr cs:_int330Ccalloff
        pop es ds
        popad
        lss esp,fword ptr [esp]
        add esp,2
        iretd

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 33h Function 16h - Save Mouse Driver State
; In:
;   AX - 16h
;   ES:EDX -> State Buffer
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int3316:
        sub esp,32h
        mov ebp,esp
        push ds
        mov word ptr [ebp+_eax],ax
        mov ax,cs:_int21lowbufseg
        mov word ptr [ebp+_es],ax
        mov word ptr [ebp+_edx],0
        push edx
        call _realmodeint33
        pop edx
        mov edi,edx
        mov ds,cs:_int21datasel
        mov esi,cs:_int21lowbufptr
        mov ecx,cs:_int33mousebufsize
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop ds
        add esp,32h
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 33h Function 17h - Restore Mouse Driver State
; In:
;   AX - 17h
;   ES:EDX -> State Buffer
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int3317:
        sub esp,32h
        mov ebp,esp
        push ds
        mov word ptr [ebp+_eax],ax
        push es
        pop ds
        mov esi,edx
        mov es,cs:_int21datasel
        mov edi,cs:_int21lowbufptr
        mov ecx,cs:_int33mousebufsize
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        mov ax,cs:_int21lowbufseg
        mov word ptr [ebp+_es],ax
        mov word ptr [ebp+_edx],0
        call _realmodeint33
        pop ds
        add esp,32h
        jmp _endint21handler
ENDIF
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INT 21h Handler
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
_int21handler:
        pushad
        push es
IFDEF   INT1B
        cmp cs:_int1Bflag,1
        jnz _int21no1B
        mov word ptr [esp+1eh],4c00h
;        jmp _int21tend
        jmp _int214C
_int21no1B:
ENDIF
        cld
        cmp ah,40h
        jb short _int21t1
        ja short _int21t2
        jz _int2140
_int21t1:
        cmp ah,39h
        jb short _int21t1a
        ja short _int21t1b
        jz _int2139
_int21t1a:
        cmp ah,9
        jz _int2109
        cmp ah,1ah
        jz _int211A
        cmp ah,1bh
        jz _int211B
        cmp ah,1ch
        jz _int211C
        cmp ah,1fh
        jz _int211F
        cmp ah,25h
        jz _int2125
        cmp ah,2fh
        jz _int212F
        cmp ah,32h
        jz _int2132
        cmp ah,34h
        jz _int2134
        cmp ah,35h
        jz _int2135
        jmp _int21tend
_int21t1b:
        cmp ah,3ah
        jz _int213A
        cmp ah,3bh
        jz _int213B
        cmp ah,3ch
        jz _int213C
        cmp ah,3dh
        jz _int213D
        cmp ah,3fh
        jz _int213F
        jmp short _int21tend
_int21t2:
        cmp ah,4ah
        jb short _int21t2a
        ja short _int21t2b
        jz _int214A
_int21t2a:
        cmp ah,41h
        jz _int2141
        cmp ah,43h
        jz _int2143
        cmp ah,47h
        jz _int2147
        cmp ah,48h
        jz _int2148
        cmp ah,49h
        jz _int2149
        jmp short _int21tend
_int21t2b:
        cmp ah,4bh
        jz _int214B
        cmp ah,4ch
        jz _int214C
        cmp ah,4eh
        jz _int214E
        cmp ah,4fh
        jz _int214F
        cmp ah,51h
        jz _int2151
        cmp ah,56h
        jz _int2156
        cmp ah,5bh
        jz _int213C
        cmp ah,62h
        jz _int2162
        cmp ah,0ffh
        jz _int21FF
_int21tend:
        pop es
        popad
        jmp fword ptr cs:_int21vectoff

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; DOS Termination Handler
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_termhandler:
        pushfd
        pushad
;        mov ah,4dh                             ; TSRs will no longer work
;        int 21h                                ; since this has been removed
;        cmp ah,3
;        jz _termnocr0
        call _pm_cleanup
        mov eax,cr0
        cmp eax,cs:_cr0valuerm
        jz short _termnocr0
        mov eax,cs:_cr0valuerm
        mov cr0,eax
_termnocr0:
        popad
        popfd
        movzx esp,sp
        jmp dword ptr cs:_termrmvect

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 9 - Output Character String
; In:
;   AH - 09h
;   DS:EDX -> $ Terminated String
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2109:
        sub esp,32h
        mov ebp,esp

        push ds
        pop es
        mov edi,edx

        or ecx,-1
        mov al,'$'
        repnz scas byte ptr es:[edi]
        not ecx

        cmp ecx,cs:_int21lowbufsize
        jae _int2109a
        call _int2109sub1
        jmp short _int2109end
_int2109a:
        mov edi,cs:_int21lowbufsize
        dec edi
        xchg ecx,ebx
_int2109b:
        mov ecx,edi
        push edi ebx
        call _int2109sub1
        pop ebx edi
        add edx,edi
        sub ebx,edi
        cmp ebx,edi
        ja _int2109b
        mov ecx,ebx
        call _int2109sub1
_int2109end:
        add esp,32h
        jmp _endint21handler

_int2109sub1:
        mov es,cs:_int21datasel
        mov edi,cs:_int21lowbufptr
        mov esi,edx
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        mov byte ptr es:[edi],'$'
        mov ax,cs:_int21lowbufseg
        mov [ebp+_ds],ax
        mov byte ptr [ebp+_eax+1],9
        mov word ptr [ebp+_edx],0
        call _realmodeint21
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 1Ah - Set Disk Transfer Area
; In:
;   AH - 1Ah
;   DS:EDX -> Buffer For DTA
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int211A:
        mov es,cs:_int21datasel64
        mov es:_int21dtasel,ds
        mov es:_int21dtaoff,edx
        sub esp,32h
        mov ebp,esp
        mov dx,cs:_int21lowdtaseg
        mov [ebp+_ds],dx
        mov byte ptr [ebp+_eax+1],1ah
        mov word ptr [ebp+_edx],0
        call _realmodeint21
        add esp,32h
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 1Bh - Get FAT Information For Default Drive
; In:
;   AH - 1Bh
; Out:
;   AL - Sectors Per Cluster
;   ECX - Bytes Per Sector
;   EDX - Clusters Per Disk
;   DS:EBX -> FAT ID Byte
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int211B:
        jmp _int211C

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 1Ch - Get FAT Information For Drive
; In:
;   AH - 1Ch
;   DL - Drive Code
; Out:
;   AL - Sectors Per Cluster
;   ECX - Bytes Per Sector
;   EDX - Clusters Per Disk
;   DS:EBX -> FAT ID Byte
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int211C:
        sub esp,32h
        mov ebp,esp
        mov byte ptr [ebp+_eax+1],ah
        mov byte ptr [ebp+_edx],dl
        push ax
        call _realmodeint21
        pop ax

        cmp ah,1fh
        jae _int211C1F
        movzx eax,word ptr [ebp+_ecx]
        mov [ebp+50h-4],eax
        movzx eax,word ptr [ebp+_edx]
        mov [ebp+50h-8],eax
_int211C1F:
        mov al,[ebp+_eax]
        mov [ebp+50h],al
        movzx eax,word ptr [ebp+_ds]
        shl eax,4
        movzx ebx,word ptr [ebp+_ebx]
        add eax,ebx
        mov [ebp+50h-12],eax
        mov ds,cs:_int21datasel

        add esp,32h
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 1Fh - Get Default Disk Parameter Block
; In:
;   AH - 1Fh
; Out:
;   AL - Error Status
;   DS:EBX -> Disk Parameter Block
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int211F:
        jmp _int211C

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 25h - Set Interrupt Vector
; In:
;   AH - 25h
;   AL - Interrupt Number
;   DS:EDX -> Interrupt Routine
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2125:
        mov bl,al
        mov ax,205h
        mov cx,ds
        int 31h
        jc _endint21error
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 2Fh - Get Disk Transfer Area
; In:
;   AH - 2Fh
; Out:
;   ES:EBX -> DTA
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int212F:
        mov ebx,cs:_int21dtaoff
        mov ax,cs:_int21dtasel
        mov [esp],ax
        mov [esp+18],ebx
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 32h - Get Disk Parameter Block
; In:
;   AH - 32h
;   DL - Drive Number
; Out:
;   AL - Error Status
;   DS:EBX -> Disk Parameter Block
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2132:
        jmp _int211C

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 34h - Get Address Of InDOS Flag
; In:
;   AH - 34h
; Out:
;   ES:EBX -> InDOS Flag
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2134:
        sub esp,32h
        mov ebp,esp
        mov byte ptr [ebp+_eax+1],ah
        call _realmodeint21

        movzx eax,word ptr [ebp+_es]
        shl eax,4
        movzx ebx,word ptr [ebp+_ebx]
        add eax,ebx
        mov [ebp+50h-12],eax
        mov ax,cs:_int21datasel
        mov [ebp+32h],ax

        add esp,32h
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 35h - Get Interrupt Vector
; In:
;   AH - 35h
;   AL - Interrupt Number
; Out:
;   ES:EBX -> Interrupt Routine
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2135:
        mov bl,al
        mov ax,204h
        int 31h
        mov [esp],cx
        mov [esp+18],edx
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 39h - Create Subdirectory
; In:
;   AH - 39h
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2139:
        sub esp,32h
        mov ebp,esp
_int2139a:
        mov byte ptr [ebp+_eax+1],ah
_int2139b:
        call _int21bufferpath
        jmp _endint21error_eax50h_0

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 3Ah - Remove Subdirectory
; In:
;   AH - 3Ah
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int213A:
        jmp _int2139

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 3Bh - Set Directory
; In:
;   AH - 3Bh
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int213B:
        jmp _int2139

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 3Ch - Create File
; In:
;   AH - 3Ch
;   CX - Attribute
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Handle or Error Code If CF=1
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int213C:
        sub esp,32h
        mov ebp,esp
        mov word ptr [ebp+_ecx],cx
        jmp short _int213Da

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 3Dh - Open File
; In:
;   AH - 3Dh
;   AL - Open Code
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Handle or Error Code If CF=1
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int213D:
        sub esp,32h
        mov ebp,esp
_int213Da:
        mov word ptr [ebp+_eax],ax

        call _int21bufferpath
        movzx eax,word ptr [ebp+_eax]
        mov dword ptr [ebp+50h],eax

        test byte ptr [ebp+_flags],1
        jnz _int213Derr
        add esp,32h
        jmp _endint21handler
_int213Derr:
        add esp,32h
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 3Fh - Read File
; In:
;   AH - 3Fh
;   BX - File Handle
;   ECX - Number Of Bytes To Read
;   DS:EDX -> Buffer To Read To
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Number Of Bytes Read or Error Code If CF=1
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int213F:
        push ds
        sub esp,32h
        mov ebp,esp
        mov word ptr [ebp+_ebx],bx

        push ds
        pop es
        mov edi,edx
        mov ds,cs:_int21datasel

        mov ax,cs:_int21lowbufseg
        mov [ebp+_ds],ax
        mov word ptr [ebp+_edx],0

_int213Fmod1:
        db 4 dup(90h)

;       90h 90h 90h 90h | 67h 01h 55h 14h
;       nop nop nop nop | add word ptr [ebp+_edx],dx

        cmp ecx,cs:_int21lowbufsize
        ja _int213Fa
        mov byte ptr [ebp+_eax+1],3fh
        mov word ptr [ebp+_ecx],cx
        push edi
        call _realmodeint21
        pop edi
        movzx eax,word ptr [ebp+_eax]
        mov [ebp+32h+4+28],eax
        test byte ptr [ebp+_flags],1
        jnz _int213Ferr
        mov ecx,eax
        mov esi,cs:_int21lowbufptr
        jecxz _int213Fend
_int213Fmod2:
        rep movs byte ptr es:[edi],byte ptr ds:[esi]

;       F3h 67h A4h | 90h 90h 90h
;       rep movs... | nop nop nop

        jmp _int213Fend
_int213Fa:
        mov ebx,ecx
        xor edx,edx
_int213Fb:
        mov byte ptr [ebp+_eax+1],3fh
        mov eax,cs:_int21lowbufsize
        mov word ptr [ebp+_ecx],ax
        push ebx edi
        call _realmodeint21
        pop edi ebx
        movzx eax,word ptr [ebp+_eax]
        test byte ptr [ebp+_flags],1
        jz _int213Fc
        mov [ebp+32h+4+28],eax
        jmp _int213Ferr
_int213Fc:
        add edx,eax
        mov esi,cs:_int21lowbufptr
        mov ecx,eax
        jecxz _int213Fd
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        cmp eax,cs:_int21lowbufsize
        jnz _int213Fd
        sub ebx,eax
        cmp ebx,cs:_int21lowbufsize
        jae _int213Fb
        or ebx,ebx
        jz _int213Fd
        mov byte ptr [ebp+_eax+1],3fh
        mov word ptr [ebp+_ecx],bx
        push edi
        call _realmodeint21
        pop edi
        movzx eax,word ptr [ebp+_eax]
        test byte ptr [ebp+_flags],1
        jz _int213Fc1
        mov [ebp+32h+4+28],eax
        jmp _int213Ferr
_int213Fc1:
        add edx,eax
        mov esi,cs:_int21lowbufptr
        mov ecx,eax
        jecxz _int213Fd
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
_int213Fd:
        mov [ebp+32h+4+28],edx
_int213Fend:
        add esp,32h
        pop ds
        jmp _endint21handler
_int213Ferr:
        add esp,32h
        pop ds
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 3Fh Read Buffer Toggle
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int213Fbuftoggle:
        xor dword ptr ds:[_int213Fmod1],84C591F7h
        xor word ptr ds:[_int213Fmod2],0F763h
        xor byte ptr ds:[_int213Fmod2+2],34h
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 40h - Write File
; In:
;   AH - 40h
;   BX - File Handle
;   ECX - Number Of Bytes To Write
;   DS:EDX -> Buffer To Write From
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Number Of Bytes Written or Error Code If CF=1
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2140:
        sub esp,32h
        mov ebp,esp
        mov word ptr [ebp+_ebx],bx

        mov esi,edx
        mov es,cs:_int21datasel

        mov ax,cs:_int21lowbufseg
        mov [ebp+_ds],ax
        mov word ptr [ebp+_edx],0

        cmp ecx,cs:_int21lowbufsize
        ja _int2140a
        mov byte ptr [ebp+_eax+1],40h
        mov word ptr [ebp+_ecx],cx
        movzx ecx,cx
        jecxz _int2140z
        mov edi,cs:_int21lowbufptr
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
_int2140z:
        call _realmodeint21
        movzx eax,word ptr [ebp+_eax]
        mov [ebp+32h+2+28],eax
        test byte ptr [ebp+_flags],1
        jnz _int2140err
        jmp _int2140end
_int2140a:
        mov ebx,ecx
        xor edx,edx
_int2140b:
        mov byte ptr [ebp+_eax+1],40h
        mov eax,cs:_int21lowbufsize
        mov word ptr [ebp+_ecx],ax
        movzx ecx,ax
        mov edi,cs:_int21lowbufptr
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        push ebx
        call _realmodeint21
        pop ebx
        movzx eax,word ptr [ebp+_eax]
        test byte ptr [ebp+_flags],1
        jz _int2140c
        mov [ebp+32h+2+28],eax
        jmp _int2140err
_int2140c:
        add edx,eax
        cmp eax,cs:_int21lowbufsize
        jnz _int2140d
        sub ebx,eax
        cmp ebx,cs:_int21lowbufsize
        jae _int2140b
        or ebx,ebx
        jz _int2140d
        mov byte ptr [ebp+_eax+1],40h
        mov word ptr [ebp+_ecx],bx
        movzx ecx,bx
        mov edi,cs:_int21lowbufptr
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        call _realmodeint21
        movzx eax,word ptr [ebp+_eax]
        test byte ptr [ebp+_flags],1
        jz _int2140c1
        mov [ebp+32h+2+28],eax
        jmp _int2140err
_int2140c1:
        add edx,eax
_int2140d:
        mov [ebp+32h+2+28],edx
_int2140end:
        add esp,32h
        jmp _endint21handler
_int2140err:
        add esp,32h
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 41h - Delete File
; In:
;   AH - 41h
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2141:
        jmp _int2139

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 43h - Get/Set File Attributes
; In:
;   AH - 43h
;   AL - Function Code
;   CX - Desired Attributes
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Error Code If CF=1
;   ECX - Current Attributes
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2143:
        sub esp,32h
        mov ebp,esp
        mov word ptr [ebp+_eax],ax
        mov word ptr [ebp+_ecx],cx
        call _int21bufferpath
        movzx ecx,word ptr [ebp+_ecx]
        mov [ebp+50h-4],ecx
        jmp _endint21error_eax50h_0

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 47h - Get Directory Path
; In:
;   AH - 47h
;   DL - Drive Number
;   DS:ESI -> Buffer For Path
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Error Code If CF=1
;   DS:ESI -> Buffer For Path (Filled If CF=0)
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2147:
        sub esp,32h
        mov ebp,esp
        mov byte ptr [ebp+_eax+1],ah
        mov byte ptr [ebp+_edx],dl

        mov dword ptr [ebp+50h],0

        mov ax,cs:_int21lowbufseg
        mov [ebp+_ds],ax
        mov word ptr [ebp+_esi],0
        call _realmodeint21

        test byte ptr [ebp+_flags],1
        jnz _int2147err
        mov edi,cs:_int21lowbufptr
        mov es,cs:_int21datasel
        or ecx,-1
        xor al,al
        repnz scas byte ptr es:[edi]
        not ecx
        push ds
        sub edi,ecx
        xchg esi,edi
        mov ax,es
        mov bx,ds
        mov es,bx
        mov ds,ax
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop ds
        add esp,32h
        jmp _endint21handler
_int2147err:
        movzx eax,word ptr [ebp+_eax]
        mov dword ptr [ebp+50h],eax
        add esp,32h
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 48h - Allocate Memory
; In:
;   AH - 48h
;   BX - Paragraphs To Allocate
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Selector To Memory If CF=0 or Error Code If CF=1
;   EBX - Maximum Paragraphs Available If CF=1
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2148:
        mov ax,100h
        int 31h
        movzx edx,dx
        mov dword ptr [esp+1eh],edx
        jnc _endint21handler
        movzx eax,ax
        mov dword ptr [esp+1eh],eax
        movzx ebx,bx
        mov dword ptr [esp+1eh-12],ebx
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 49h - Free Memory
; In:
;   AH - 49h
;   ES - Selector
; Out:
;   CF=0 Success
;   CF=1 Error
;   ES  - Null Selector
;   EAX - Error Code If CF=1
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2149:
        mov ax,101h
        mov dx,es
        int 31h
        jnc _int2149a
        movzx eax,ax
        mov dword ptr [esp+1eh],eax
        jmp _endint21error
_int2149a:
        mov word ptr [esp],0
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 4Ah - Change Memory Block Allocation
; In:
;   AH - 4Ah
;   BX - Total Paragraphs To Allocate
;   ES - Selector
; Out:
;   CF=0 Success
;   CF=1 Error
;   EAX - Error Code If CF=1
;   EBX - Maximum Paragraphs Available If CF=1
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int214A:
        mov ax,102h
        mov dx,es
        int 31h
        jnc _endint21handler
        movzx eax,ax
        mov dword ptr [esp+1eh],eax
        movzx ebx,bx
        mov dword ptr [esp+1eh-12],ebx
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 4Bh, Function 00h - Load Program
; In:
;   AH - 4Bh
;   AL - 00h
;   DS:EDX -> Path Name
;   ES:EBX -> Parameter Block
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int214B:
        sub esp,32h
        mov ebp,esp
        mov word ptr [ebp+_eax],ax

        or al,al
        jnz _int21err_leave_eax

        push es
        mov ax,ds
        mov es,ax
        mov edi,edx

        or ecx,-1
        xor al,al
        repnz scas byte ptr es:[edi]
        not ecx

        mov es,cs:_int21datasel
        mov edi,cs:_int21lowbufptr
        add edi,2048
        mov esi,edx
        rep movs byte ptr es:[edi],byte ptr ds:[esi]

        mov es,cs:_int21datasel
        mov edi,cs:_int21lowbufptr
        mov ecx,22
        xor al,al
        rep stos byte ptr es:[edi]
        sub edi,22
        mov ax,cs:_int21lowbufseg
        mov word ptr es:[edi+2],22
        mov word ptr es:[edi+4],ax
        mov word ptr es:[edi+6],5ch
        mov word ptr es:[edi+10],6ch
        mov ax,cs:_pspseg
        mov word ptr es:[edi+8],ax
        mov word ptr es:[edi+12],ax
        pop es

        push es ds
        mov ds,es:[ebx+10]
        mov esi,es:[ebx+6]
        mov es,cs:_int21datasel
        mov edi,cs:_int21lowbufptr
        add edi,22
        movzx ecx,byte ptr [esi]
        inc cx
        inc cx
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop ds es

        push ds es
        mov edi,es:[ebx]
        mov es,es:[ebx+4]
        mov esi,edi
        or ecx,-1
        xor al,al
_int214B00a:
        repnz scas byte ptr es:[edi]
        dec ecx
        scas byte ptr es:[edi]
        jnz _int214B00a
        not ecx
        mov ax,100h
        mov ebx,ecx
        shr ebx,4
        inc bx
        int 31h
        jc _int214B00err1
        push es
        pop ds
        mov es,dx
        xor edi,edi
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        mov es,cs:_int21lowbufsel
        mov es:[0],ax
        clc
_int214B00err1:
        pop es ds
        jc _int21err_leave_eax

        mov ax,cs:_int21lowbufseg
        mov [ebp+_ds],ax
        mov [ebp+_es],ax
        mov word ptr [ebp+_ebx],0
        mov word ptr [ebp+_edx],2048

        mov eax,cr0
        test al,4
        jz _int214Bnocp1
        and al,NOT 4
        mov cr0,eax
        call _realmodeint21
        mov eax,cr0
        or al,4
        mov cr0,eax
        jmp short _int214Bnocp2
_int214Bnocp1:
        call _realmodeint21
_int214Bnocp2:
        mov ax,101h
        int 31h
        jmp _endint21error_eax50h_0

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 4Ch - Terminate Process
; In:
;   AH - 4Ch
;   AL - Error Code
; Out:
;   None
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int214C:
IFDEF   INT33
        mov edx,cs:_int33callback
        mov ecx,edx
        shr ecx,16
        mov ax,304h
        int 31h
_int214C_no33:
ENDIF
IFDEF   INT1B
        mov edx,cs:_int1Bcallback
        mov ecx,edx
        shr ecx,16
        mov ax,304h
        int 31h
_int214C_no1B:
ENDIF
        jmp _int21tend

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 4Eh - Search For First Filename Match
; In:
;   AH - 4Eh
;   CX - File Attribute
;   DS:EDX -> ASCIIZ Path Name
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int214E:
        sub esp,32h
        mov ebp,esp
        mov word ptr [ebp+_ecx],cx
        mov byte ptr [ebp+_eax+1],ah

        call _int21bufferpath

        mov dword ptr [ebp+50h],0

        test byte ptr [ebp+_flags],1
        jnz _int214Eerr
        push ds
        mov ds,cs:_int21datasel
        movzx esi,cs:_int21lowdtaseg
        shl esi,4
        mov es,cs:_int21dtasel
        mov edi,cs:_int21dtaoff
        mov ecx,43
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop ds
        add esp,32h
        jmp _endint21handler
_int214Eerr:
        movzx eax,word ptr [ebp+_eax]
        mov dword ptr [ebp+50h],eax
        add esp,32h
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 4Fh - Search For Next Filename Match
; In:
;   AH - 4Fh
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int214F:
        sub esp,32h
        mov ebp,esp
        mov byte ptr [ebp+_eax+1],ah

        push ds
        mov es,cs:_int21datasel
        movzx edi,cs:_int21lowdtaseg
        shl edi,4
        mov ds,cs:_int21dtasel
        mov esi,cs:_int21dtaoff
        mov ecx,43
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop ds

        call _realmodeint21

        mov dword ptr [ebp+50h],0

        test byte ptr [ebp+_flags],1
        jnz _int214Ferr
        push ds
        mov ds,cs:_int21datasel
        movzx esi,cs:_int21lowdtaseg
        shl esi,4
        mov es,cs:_int21dtasel
        mov edi,cs:_int21dtaoff
        mov ecx,43
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop ds
        add esp,32h
        jmp _endint21handler
_int214Ferr:
        movzx eax,word ptr [ebp+_eax]
        mov dword ptr [ebp+50h],eax
        add esp,32h
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 51h - Get PSP Segment
; In:
;   AH - 51h
; Out:
;   BX - PSP Segment Address
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2151:
        mov ax,cs:_pspseg
        mov word ptr [esp+18],ax
        jmp _endint21handler

;        sub esp,32h
;        mov ebp,esp
;        mov byte ptr [ebp+_eax+1],ah
;        call _realmodeint21
;
;        mov bx,[ebp+_ebx]
;        mov word ptr [ebp+50h-12],bx
;
;        add esp,32h
;        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 56h - Rename File
; In:
;   AH - 56h
;   DS:EDX -> Old Filename
;   ES:EDI -> New Filename
; Out:
;   CF=0 Success
;   CF=1 Error
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2156:
        sub esp,32h
        mov ebp,esp
        mov byte ptr [ebp+_eax+1],ah

        or ecx,-1
        xor al,al
        repnz scas byte ptr es:[edi]
        not ecx
        sub edi,ecx
        push ds
        mov esi,edi
        mov ax,es
        mov ds,ax
        mov es,cs:_int21datasel
        mov edi,cs:_int21lowbufptr
        rep movs byte ptr es:[edi],byte ptr ds:[esi]
        pop ds

        mov ecx,edi
        mov ebx,cs:_int21lowbufptr
        sub ecx,ebx
        xchg ecx,ebx

        mov ax,ds
        mov es,ax
        mov esi,edx
        xchg esi,edi
        or ecx,-1
        xor al,al
        repnz scas byte ptr es:[edi]
        not ecx
        sub edi,ecx
        xchg esi,edi
        mov es,cs:_int21datasel
        rep movs byte ptr es:[edi],byte ptr ds:[esi]

        mov ax,cs:_int21lowbufseg
        mov [ebp+_ds],ax
        mov [ebp+_es],ax
        mov word ptr [ebp+_edi],0
        mov word ptr [ebp+_edx],bx
        call _realmodeint21

        jmp _endint21error_eax50h_0

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service 62h - Get PSP Segment (returns selector)
; In:
;   AH - 62h
; Out:
;   BX - PSP Segment Address
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int2162:
        mov ax,cs:_selpsp
        mov word ptr [esp+18],ax
        jmp _endint21handler

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; INT 21h Service FFh - DOS Extender Check
; In:
;   AH - FFh
;   DX - 78h
; Out:
;   GS - Info Segment
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int21FF:
        cmp dx,78h
        jnz _int21FFend
        mov gs,cs:_int21lowbufsel
        mov dword ptr [esp+1eh],4734FFFFh
        jmp _endint21handler
_int21FFend:
        jmp _int21tend

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; End Of INT 21h Handler
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_endint21handler:
        pop es
        popad
        and byte ptr [esp+8],NOT 1
        iretd
_endint21error:
        pop es
        popad
        or byte ptr [esp+8],1
        iretd

_endint21error_eax50h_0:
        mov dword ptr [ebp+50h],0
_endint21error_eax50h:
        test byte ptr [ebp+_flags],1
        jnz _int21err_eax50h
        add esp,32h
        jmp _endint21handler
_int21err_eax50h:
        movzx eax,word ptr [ebp+_eax]
        mov dword ptr [ebp+50h],eax
_int21err_leave_eax:
        add esp,32h
        jmp _endint21error

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Call Real Mode Interrupt Routine
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
IFDEF   INT33
_realmodeint33:
        mov bx,33h
        jmp short _realmodeintq
ENDIF
_realmodeint21:
        mov bx,21h
_realmodeintq:
        push es
        xor eax,eax
        mov [ebp+_sp],eax
        mov [ebp+_flags],ax
        push ss
        pop es
        mov edi,ebp
        mov ax,300h
        xor cx,cx
        int 31h
        pop es
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Subroutine For Some INT 21h Functions
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_int21bufferpath:
        push ds
        pop es
        mov edi,edx

        or ecx,-1
        xor al,al
        repnz scas byte ptr es:[edi]
        not ecx

        mov es,cs:_int21datasel
        mov edi,cs:_int21lowbufptr
        mov esi,edx
        rep movs byte ptr es:[edi],byte ptr ds:[esi]

        mov ax,cs:_int21lowbufseg
        mov [ebp+_ds],ax
        mov word ptr [ebp+_edx],0
        call _realmodeint21
        ret

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; INT 1Bh Handler
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
IFDEF   INT1B
_int1Brmhandler:
        jmp dword ptr cs:_int1Bcallback
_int1Brmreturn:
        iret

_int1Bpmchandler:
        mov word ptr es:[edi+_cs],pmcode16
        mov word ptr es:[edi+_ip],offset _int1Brmreturn
        int 1bh
        iretd

_int1Bhandler:
        push ds
        mov ds,cs:_int21datasel64
        mov _int1Bflag,1
        pop ds
        iretd
ENDIF


;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; SUPPLEMENTAL REAL MODE ROUTINES
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
IF OPENTYPE EQ 1
_openexe1:
        push ds es
        mov es,_envseg
        push _int21lowbufseg
        pop ds
        xor di,di

        or cx,-1
        xor al,al
@@00l:
        repne scasb
        scasb
        jne @@00l

        add di,2
        mov si,di

        or cx,-1
        repne scasb
        not cx

        xor di,di
        push es ds
        pop es ds

        rep movsb

        push es
        pop ds

        xor dx,dx
        mov ax,3dc0h
        int 21h
        pop es ds
        jc _loaderror
        jmp _openexe2
ENDIF

IF OPENTYPE EQ 2
_openexe1:
        push ds
        xor ax,ax
        mov ds,ax
        mov dx,4c0h
        mov ax,3dc0h
        int 21h
        pop ds
        jc _loaderror
        jmp _openexe2
ENDIF

IF OPENTYPE EQ 3
_filename       db 128 dup (?)
_openexe1:
        cld
        push es ds
        mov ds,cs:_pspseg
        mov ax,cs
        mov es,ax
        mov di,offset _filename
        mov si,80h
        lodsb
        or al,al
        jz _openexe3err
_openexe3a1:
        lodsb
        cmp al,20h
        jz _openexe3a1
        mov cx,-1
        jmp _openexe3b
_openexe3a:
        lodsb
_openexe3b:
        cmp al,20h
        jz _openexe3c
        cmp al,0dh
        jz _openexe3c
        stosb
        dec cx
        jnz _openexe3a
        jmp _openexe3err
_openexe3c:
        xor al,al
        stosb
        xor bl,bl
        dec si
        mov ax,ds
        mov es,ax
        mov di,81h
_openexe3d:
        lodsb
        or al,al
        cmp al,0dh
        jz _openexe3e
        inc bl
        stosb
        jmp _openexe3d
_openexe3e:
        stosb
        mov es:[80h],bl
        mov dx,offset _filename
        push cs
        pop ds
        mov ax,3dc0h
        int 21h
        pop ds es
        jc _loaderror
        jmp _openexe2
_openexe3err:
        pop ds es
        jmp _loaderror
ENDIF

IF VARTYPE EQ 1
_openexe2:
        mov _exehandle,ax
        push ds
        mov bx,ax
        mov cx,40h+21
        mov ds,_int21lowdtaseg
        xor dx,dx
        mov ah,3fh
        int 21h
        jc _loaderror
        push es
        mov si,ds:[8]
        shl si,4
        mov ax,PMODE_TEXT
        mov es,ax
        mov di,offset _pm_pagetables
        mov cx,19
        rep movsb
        lodsw
        pop es ds
        mov _lowmin,ax
        ret
ENDIF

IF VARTYPE EQ 2
_openexe2:
        mov _exehandle,ax
        mov _lowmin,LOWMIN
        push ds
        mov bx,ax
        mov cx,40h
        mov ds,_int21lowdtaseg
        xor dx,dx
        mov ah,3fh
        int 21h
        jc _loaderror
        mov ax,PMODE_TEXT
        mov ds,ax
        mov _pm_pmstacklen,PM_PMSTACKLEN
        mov _pm_rmstacklen,PM_RMSTACKLEN
        mov _pm_pmstacks,PM_PMSTACKS
        mov _pm_rmstacks,PM_RMSTACKS
        mov _pm_mode,PM_MODE
        mov _pm_selectors,PM_SELECTORS
        mov _pm_callbacks,PM_CALLBACKS
        mov _pm_pagetables,PM_PAGETABLES
        mov _pm_maxextmem,EXTMAX
        mov _pm_pampagemax,PM_PAMPAGEMAX
        mov _pm_options,PM_OPTIONS
        pop ds
        ret
ENDIF

pmcode16        ends

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; STACK
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
pmstack         segment para stack use16 'STACK'
                db      STACKLEN*16 dup(?)
pmstack         ends

end _pm16start

