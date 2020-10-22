
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
; Decompress from disk to memory.
; In:
;   EDI -> buffer to decode to
;   DS - data selector
;   ES - buffer and output selector
; Out:
;   EAX, EBX, ECX, EDX, ESI, EDI, EBP - ?
;°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
decode:
        mov esi,decode_bufend
        call decode_read

        movs byte ptr es:[edi],byte ptr es:[esi]
        xor dl,dl

        jmp short decode_decode

;-----------------------------------------------------------------------------
decode_extended:
        call decode_getbit
        jnc short decode_length

        call decode_getbit
        adc cl,1
        shl cl,1

@@00l:
        call decode_getbit
        rcl bh,1
        loop @@00l

;-----------------------------------------------------------------------------
decode_length:
        mov dh,2
        mov cl,4

@@00l:
        inc dh
        call decode_getbit
        jc short decode_movestringdh

        loop @@00l

        call decode_getbit
        jnc short decode_length3bit

        lods byte ptr es:[esi]
        mov cl,al
        add ecx,15
        jmp short decode_movestring

;תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת
decode_length3bit:
        xor dh,dh
        mov cl,3

@@00l:
        call decode_getbit
        rcl dh,1
        loop @@00l

        add dh,7

;-----------------------------------------------------------------------------
decode_movestringdh:
        mov cl,dh
        jmp short decode_movestring

;תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת
decode_movestring2:
        mov cl,2

;תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת
decode_movestring:
        neg ebx
        dec ebx

@@00l:
        mov al,es:[edi+ebx]
        stos byte ptr es:[edi]
        loop @@00l

;-----------------------------------------------------------------------------
decode_decode:
        cmp esi,decode_bufthreshold
        jb short @@00

        call decode_read

@@00:
        call decode_getbit
        jc short code

        movs byte ptr es:[edi],byte ptr es:[esi]
        jmp decode_decode

;תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת
code:
        xor ebx,ebx
        lods byte ptr es:[esi]
        mov bl,al

        call decode_getbit
        jc short decode_extended

        call decode_getbit
        jc short decode_code11

        dec ebx
        jns decode_movestring2

;-----------------------------------------------------------------------------
        ret

;תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת
decode_code11:
        mov cl,3

@@00l:
        call decode_getbit
        rcl bh,1
        loop @@00l

        jmp short decode_movestring2

;ִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִ
decode_getbit:
        dec dl
        jns short @@00

        lods dword ptr es:[esi]
        mov ebp,eax
        mov dl,31

@@00:
        shl ebp,1

        ret

;ִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִִ
decode_read:
        push edx edi

        mov edi,decode_bufbase
        push edi

        mov ecx,decode_bufend
        sub ecx,esi
        mov edx,ecx
        mov eax,ecx
        rep movs byte ptr es:[edi],byte ptr es:[esi]

        pop esi

;        lea edx,[esi+eax]
        mov ecx,decode_bufsize
        sub ecx,eax

        mov eax,decode_filesize
        cmp ecx,eax
        jbe short @@00

        mov ecx,eax

@@00:
        mov bx,decode_handle
        mov ah,3fh

;        push ds
;        push es
;        pop ds

        int 21h

;        pop ds

        sub decode_filesize,eax

        pop edi edx
        xor ecx,ecx
        ret

