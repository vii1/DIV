; Rutinas de dibujado 3D con mapeado de textura
; Obtenidas mediante ingenier¡a inversa porque estaban ausentes del repositorio original.
; No tengo ni idea de c¢mo funcionan. Si alguien con los conocimientos para ello se presta
; para documentar este c¢digo, es m s que bienvenido.

                .586
                .MODEL FLAT

                .STACK

                .DATA

                PUBLIC _direccion_textura

                _direccion_textura  dd  0
                clock1              dd  0
                clock2              dd  0

                .CODE

                PUBLIC nucleo8_8_, nucleo8_16_, nucleo8_32_, nucleo8_64_
                PUBLIC nucleo8_128_, nucleo8_256_
                PUBLIC mask_nucleo8_8_, mask_nucleo8_16_, mask_nucleo8_32_
                PUBLIC mask_nucleo8_64_, mask_nucleo8_128_, mask_nucleo8_256_



; void __userpurge nucleo8_8(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
nucleo8_8_       PROC

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 13
                mov     esi, ebx
                shl     edx, 10
                mov     edi, [ebp+destino]
                shld    edx, eax, 3
                shl     eax, 3
                mov     ebp, [ebp+ancho]
                shl     ecx, 13
                mov     ebx, 0E0000007h
                shl     esi, 10
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 3
                shl     ecx, 3
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 3
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0E0000007h
                and     ebx, edx
                mov     [edi+ebp*2], ax
                rol     ebx, 3
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
nucleo8_8_       ENDP


; void __userpurge nucleo8_16(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
nucleo8_16_      proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 12
                mov     esi, ebx
                shl     edx, 8
                mov     edi, [ebp+destino]
                shld    edx, eax, 4
                shl     eax, 4
                mov     ebp, [ebp+ancho]
                shl     ecx, 12
                mov     ebx, 0F000000Fh
                shl     esi, 8
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 4
                shl     ecx, 4
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 4
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0F000000Fh
                and     ebx, edx
                mov     [edi+ebp*2], ax
                rol     ebx, 4
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
nucleo8_16_      endp


; void __userpurge nucleo8_32(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
nucleo8_32_      proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 11
                mov     esi, ebx
                shl     edx, 6
                mov     edi, [ebp+destino]
                shld    edx, eax, 5
                shl     eax, 5
                mov     ebp, [ebp+ancho]
                shl     ecx, 11
                mov     ebx, 0F800001Fh
                shl     esi, 6
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 5
                shl     ecx, 5
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 5
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0F800001Fh
                and     ebx, edx
                mov     [edi+ebp*2], ax
                rol     ebx, 5
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
nucleo8_32_      endp


; void __userpurge nucleo8_64(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
nucleo8_64_      proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 10
                mov     esi, ebx
                shl     edx, 4
                mov     edi, [ebp+destino]
                shld    edx, eax, 6
                shl     eax, 6
                mov     ebp, [ebp+ancho]
                shl     ecx, 10
                mov     ebx, 0FC00003Fh
                shl     esi, 4
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 6
                shl     ecx, 6
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 6
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0FC00003Fh
                and     ebx, edx
                mov     [edi+ebp*2], ax
                rol     ebx, 6
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
nucleo8_64_      endp


; void __userpurge nucleo8_128(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
nucleo8_128_     proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 9
                mov     esi, ebx
                shl     edx, 2
                mov     edi, [ebp+destino]
                shld    edx, eax, 7
                shl     eax, 7
                mov     ebp, [ebp+ancho]
                shl     ecx, 9
                mov     ebx, 0FE00007Fh
                shl     esi, 2
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 7
                shl     ecx, 7
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 7
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0FE00007Fh
                and     ebx, edx
                mov     [edi+ebp*2], ax
                rol     ebx, 7
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
nucleo8_128_     endp


; void __userpurge nucleo8_256(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
nucleo8_256_     proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                mov     esi, ebx
                mov     edi, [ebp+destino]
                shl     eax, 8
                mov     ebp, [ebp+ancho]
                shld    edx, eax, 8
                shl     eax, 8
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                neg     ebp
                shl     ecx, 8
                mov     ebx, 0FF0000FFh
                shld    esi, ecx, 8
                shl     ecx, 8
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 8
                mov     clock2, eax
                xor     eax, eax

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0FF0000FFh
                and     ebx, edx
                mov     [edi+ebp*2], ax
                rol     ebx, 8
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
nucleo8_256_     endp


mask_nucleo8_8_  proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 13
                mov     esi, ebx
                shl     edx, 10
                mov     edi, [ebp+destino]
                shld    edx, eax, 3
                shl     eax, 3
                mov     ebp, [ebp+ancho]
                shl     ecx, 13
                mov     ebx, 0E0000007h
                shl     esi, 10
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 3
                shl     ecx, 3
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 3
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0E0000007h
                and     ebx, edx
                or      eax, eax
                jz      short @cero
                mov     [edi+ebp*2], ax

@cero:
                rol     ebx, 3
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
mask_nucleo8_8_  endp


mask_nucleo8_16_ proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 12
                mov     esi, ebx
                shl     edx, 8
                mov     edi, [ebp+destino]
                shld    edx, eax, 4
                shl     eax, 4
                mov     ebp, [ebp+ancho]
                shl     ecx, 12
                mov     ebx, 0F000000Fh
                shl     esi, 8
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 4
                shl     ecx, 4
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 4
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0F000000Fh
                and     ebx, edx
                or      eax, eax
                jz      short @cero
                mov     [edi+ebp*2], ax

@cero:
                rol     ebx, 4
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
mask_nucleo8_16_ endp


; void __userpurge mask_nucleo8_32(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
mask_nucleo8_32_ proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 11
                mov     esi, ebx
                shl     edx, 6
                mov     edi, [ebp+destino]
                shld    edx, eax, 5
                shl     eax, 5
                mov     ebp, [ebp+ancho]
                shl     ecx, 11
                mov     ebx, 0F800001Fh
                shl     esi, 6
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 5
                shl     ecx, 5
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 5
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0F800001Fh
                and     ebx, edx
                or      eax, eax
                jz      short @cero
                mov     [edi+ebp*2], ax

@cero:
                rol     ebx, 5
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
mask_nucleo8_32_ endp


; void __userpurge mask_nucleo8_64(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
mask_nucleo8_64_ proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 10
                mov     esi, ebx
                shl     edx, 4
                mov     edi, [ebp+destino]
                shld    edx, eax, 6
                shl     eax, 6
                mov     ebp, [ebp+ancho]
                shl     ecx, 10
                mov     ebx, 0FC00003Fh
                shl     esi, 4
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 6
                shl     ecx, 6
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 6
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0FC00003Fh
                and     ebx, edx
                or      eax, eax
                jz      short @cero
                mov     [edi+ebp*2], ax

@cero:
                rol     ebx, 6
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
mask_nucleo8_64_ endp


; void __userpurge mask_nucleo8_128(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
mask_nucleo8_128_ proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                shl     eax, 9
                mov     esi, ebx
                shl     edx, 2
                mov     edi, [ebp+destino]
                shld    edx, eax, 7
                shl     eax, 7
                mov     ebp, [ebp+ancho]
                shl     ecx, 9
                mov     ebx, 0FE00007Fh
                shl     esi, 2
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                shld    esi, ecx, 7
                shl     ecx, 7
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 7
                mov     clock2, eax
                xor     eax, eax
                neg     ebp

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0FE00007Fh
                and     ebx, edx
                or      eax, eax
                jz      short @cero
                mov     [edi+ebp*2], ax

@cero:
                rol     ebx, 7
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
mask_nucleo8_128_ endp


; void __userpurge mask_nucleo8_256(int v@<eax>, int u@<edx>, int du@<ebx>, int dv@<ecx>, int ancho, __int16 *destino)
mask_nucleo8_256_ proc

ancho           = dword ptr  8
destino         = dword ptr  12

                push    ebp
                mov     ebp, esp
                push    esi
                push    edi
                push    eax
                push    ebx
                push    ecx
                push    edx
                mov     esi, ebx
                mov     edi, [ebp+destino]
                shl     eax, 8
                mov     ebp, [ebp+ancho]
                shld    edx, eax, 8
                shl     eax, 8
                shl     ebp, 1
                add     edi, ebp
                shr     ebp, 1
                neg     ebp
                shl     ecx, 8
                mov     ebx, 0FF0000FFh
                shld    esi, ecx, 8
                shl     ecx, 8
                mov     clock1, esi
                xchg    eax, ecx
                and     ebx, edx
                mov     esi, _direccion_textura
                rol     ebx, 8
                mov     clock2, eax
                xor     eax, eax

@bucle:
                add     ecx, clock2
                mov     ax, [esi+ebx*2]
                adc     edx, clock1
                mov     ebx, 0FF0000FFh
                and     ebx, edx
                or      eax, eax
                jz      short @cero
                mov     [edi+ebp*2], ax

@cero:
                rol     ebx, 8
                inc     ebp
                jnz     short @bucle
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                pop     edi
                pop     esi
                pop     ebp
                pop     eax
                mov     [esp+4], eax
                pop     eax
                retn
mask_nucleo8_256_ endp

                END
