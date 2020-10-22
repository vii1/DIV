;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; PMODE/W Version Check Utility - Main Code
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

.model small
.code
assume cs:_TEXT,ds:_TEXT,ss:_TEXT
org 100h
jumps

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; DATA
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
_start:
        jmp _main

_usagemsg1      db 'PMODE/W Version Check Utility v1.34',10,13
                db 'Copyright (C) 1995-2000, Charles Scheffold and Thomas Pytel.'
                db 10,13,10,13,'$'

_usagemsg2      db 'Usage: PMWVER <filename>',10,13,'$'

_readerrmsg     db '[PMWVER]: Error reading file!',10,13,'$'

_notpmmsg       db '[PMWVER]: This is not a PMODE/W executable!',10,13,'$'

_pmwstring      db 'PMODE/W'

_pmwvermsg      db '[PMWVER]: PMODE/W Version $'

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; CODE
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
_main:
        cld
        push cs
        pop ds
        mov ah,9
        mov dx,offset _usagemsg1
        int 21h

        mov ax,es
        mov ds,ax
        mov ax,cs
        mov es,ax
        mov di,offset _filebuf
        mov si,80h
        lodsb
        or al,al
        jz _openerr
_open1:
        lodsb
        cmp al,20h
        jz short _open1
        or cx,-1
        jmp short _open3
_open2:
        lodsb
_open3:
        cmp al,20h
        jz short _open4
        cmp al,0dh
        jz short _open4
        stosb
        dec cx
        jnz short _open2
        jmp _openerr
_open4:
        push cs
        pop ds
        mov byte ptr [di],0
        mov dx,offset _filebuf
        mov ax,3dc0h
        int 21h
        jc _openerr
        mov _handle,ax

        push cs
        pop ds

        mov cx,10
        call _readfile

        mov si,offset _filebuf
        cmp word ptr [si],'ZM'
        jnz _notpmerr
        mov dl,byte ptr [si+8]
        xor dh,dh
        shl dx,4
        add dx,21
        call _seekfile

        mov cx,16
        call _readfile

        push cs
        pop es
        mov di,offset _pmwstring
        mov cx,7
        repz cmpsb
        jnz _notpmerr

        xor bl,bl
        mov si,offset _filebuf
        mov di,offset _filebuf+64
_findver1:
        lodsb
        cmp al,'0'
        jb short _findver1
        cmp al,'9'
        ja short _findver1
_findver2:
        cmp al,20h
        jz short _findver3
        cmp bl,5
        jae short _findver3
        stosb
        lodsb
        inc bl
        jmp short _findver2
_findver3:
        mov ax,0d0ah
        stosw
        mov byte ptr [di],'$'

        mov ah,9
        mov dx,offset _pmwvermsg
        int 21h
        mov ah,9
        mov dx,offset _filebuf+64
        int 21h

        mov ax,4c00h
        int 21h

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Quickie Read Into Buffer
; In:
;    CX - Length
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_readfile:
        mov ah,3fh
        mov dx,offset _filebuf
        mov bx,_handle
        int 21h
        jc _readerr
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Quickie Seek File
; In:
;    DX - Offset From File Start
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_seekfile:
        xor cx,cx
        mov ax,4200h
        mov bx,_handle
        int 21h
        jc _readerr
        ret

;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Some Kind Of Error Handling
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
_notpmerr:
        mov dx,offset _notpmmsg
        jmp short _err
_readerr:
        mov dx,offset _readerrmsg
        jmp short _err
_openerr:
        mov dx,offset _usagemsg2
_err:
        push cs
        pop ds
        mov ah,9
        int 21h
        mov ax,4c00h
        int 21h

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
; UNINITIALIZED DATA
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

_filebuf        db 128 dup (?)
_handle         dw ?


end     _start
