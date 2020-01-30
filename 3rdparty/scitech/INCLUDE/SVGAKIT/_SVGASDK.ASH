;****************************************************************************
;*
;*          The SuperVGA Kit - UniVBE Software Development Kit
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
;* Language:    80386 Assembler
;* Environment: IBM PC Real Mode and 16/32 bit Protected Mode.
;*
;* Description: Assembly language support routines for the UVBELib Software
;*              Development Kit. This code is intended to be used as a guide
;*              for developing software with the UVBELib SuperVGA library.
;*              All the code here assumes that the video memory selector
;*              has been cached in the GS selector before the code is
;*              called.
;*
;*
;****************************************************************************

		IDEAL

VBEAF_10_COMPAT	= 1				; VBE/AF 1.0 compatibility

include "model.mac"             ; Memory model macros
include "vbeaf.inc"             ; VBE/AF Accelerator Functions
include "svga.inc"              ; SuperVGA Kit structures

header  _svgasdk                ; Set up memory model

CRTC    EQU 3D4h                ; Port of CRTC registers

begdataseg  _svgasdk

DC      EQU     __SV_devCtx     ; Macro to access global structure

        $EXTRN  __SV_devCtx,SV_devCtx_s
        $EXTRN  __SV_pagesize,ULONG
        $EXTRN  __SV_bankShift,UINT
        $EXTRN  __SV_setBankRM,DPTR
        $EXTRN  __SV_setBankPtr,DPTR

if pmode
        $EXTRN  __SV_setBank20,DPTR
        $EXTRN  __SV_setCRT20,DPTR
        $EXTRN  __SV_setPal20,DPTR
        $EXTRN  _VBE_MMIOSel,WORD
endif

enddataseg  _svgasdk

begcodeseg  _svgasdk

ifdef   __WINDOWS32_386__
        EXTRN   _PM_setBankA:FPTR
        EXTRN   _PM_setBankAB:FPTR
        EXTRN   _PM_setCRTStart:FPTR
		EXTRN   _PM_setCRTStartAlt:FPTR
		EXTRN   _PM_getDisplayStartStatus:FPTR
endif

;----------------------------------------------------------------------------
; loadVidMem    - Macro to load video memory address into es:reg
;----------------------------------------------------------------------------
; This macro loads the video memory address in to ES:REG. For 16 bit code
; this simply loads the 32 bit far pointer into ES:REG. For 32 bit protected
; mode code this simply loads EREG with the offset of the video memory.
;
; For Watcom C++ Win386 mode this function loads EREG with the offset within
; video memory and loads the selector to the video memory into ES. Currently
; we are not able to map linear pointers to video memory in Win386 mode, so
; we need to load a selector to the video memory.
;----------------------------------------------------------------------------
MACRO   lVidMem reg
ifdef   __WIN386__
        movzx   reg,[WORD DC.videoMem]
        mov     es,[WORD (DC.videoMem)+2]
else
        _les    reg,[DC.videoMem]
endif
ENDM

; All banked access to video memory goes via the ES selector for Watcom
; Win386 mode. We also must save/restore the value of ES across functions
; that use it.

ifdef   __WIN386__
_ES     EQU ES:
endif

MACRO   save_es
ifdef   __WIN386__
        push    es
endif
ENDM

MACRO   restore_es
ifdef   __WIN386__
        pop     es
endif
ENDM

;----------------------------------------------------------------------------
; SV_pixelAddr16 - Determine address of pixel in 16 color banked modes
;----------------------------------------------------------------------------
;
; Entry:        _AX -   y-coordinate
;               _BX -   x-coordinate
;
; Exit:         AH  -   bit mask
;               ES:_BX  -   byte offset in buffer
;               CL  -   number of bits to shift left
;               _SI -   64k bank number of the address
;
; Registers:    none.
;
;----------------------------------------------------------------------------
PROC    SV_pixelAddr16 near

        push    _dx                 ; Save DX
        mul     [USHORT DC.bytesperline]    ; DX:AX := y * BytesPerLine
        mov     cl,bl               ; CL := low-order byte of x

        shr     _bx,3               ; _BX := x/8
        add     ax,bx
        adc     dl,0                ; DL:_BX := y*BytesPerLine + x/8
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        adc     dl,[BYTE DC.bankOffset]; DL := bank number
        add     bx,[USHORT DC.originOffset] ; _BX := byte offset in video buffer
        adc     dl,0
        mov     _si,_dx             ; _SI := bank number

        mov     ah,1                ; AH := unshifted bit mask
        and     cl,7                ; CL := x & 7
        xor     cl,7                ; CL := # bits to shift left

        pop     _dx
        ret

ENDP    SV_pixelAddr16

;----------------------------------------------------------------------------
; SV_pixelAddr256 - Determine address of pixel in 256 color banked modes
;----------------------------------------------------------------------------
;
; Entry:        _AX     -   y-coordinate
;               _BX     -   x-coordinate
;
; Exit:         ES:_BX  -   byte offset in buffer
;               _DX     -   64k bank number of the address
;
; Registers:    None.
;
;----------------------------------------------------------------------------
PROC    SV_pixelAddr256 near

        mul     [USHORT DC.bytesperline]        ; DX:AX := y * bytesperline
        add     ax,bx
        adc     dx,0                ; DX:_BX := y * BytesPerLine + x
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        adc     dl,[BYTE DC.bankOffset]; DL := bank number
        add     bx,[USHORT DC.originOffset]
        adc     dl,0
        ret

ENDP    SV_pixelAddr256

;----------------------------------------------------------------------------
; SV_pixelAddr32k - Determine address of pixel in 32k color banked modes
;----------------------------------------------------------------------------
;
; Entry:        _AX -   y-coordinate
;               _BX -   x-coordinate
;
; Exit:         _BX -   byte offset in buffer
;               _DX -   64k bank number of the address
;
; Registers:    None.
;
;----------------------------------------------------------------------------
PROC    SV_pixelAddr32k near

        mul     [USHORT DC.bytesperline]        ; DX:AX := y * bytesperline
        shl     _bx,1
        add     ax,bx
        adc     dx,0                ; DX:BX := y * BytesPerLine + x * 2
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        adc     dl,[BYTE DC.bankOffset]; DL := bank number
        add     bx,[USHORT DC.originOffset]
        adc     dl,0
        ret

ENDP    SV_pixelAddr32k

;----------------------------------------------------------------------------
; SV_pixelAddr16m - Determine address of pixel in 16m color banked modes
;----------------------------------------------------------------------------
;
; Entry:        _AX -   y-coordinate
;               _BX -   x-coordinate
;
; Exit:         _BX -   byte offset in buffer
;               _DX -   64k bank number of the address
;
; Registers:    None.
;
;----------------------------------------------------------------------------
PROC    SV_pixelAddr16m near

        mul     [USHORT DC.bytesperline]        ; DX:AX := y * bytesperline
        add     ax,bx
        shl     _bx,1
        add     ax,bx
        adc     dx,0                ; DX:BX := y * BytesPerLine + x * 3
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        adc     dl,[BYTE DC.bankOffset]; DL := bank number
        add     bx,[USHORT DC.originOffset]
        adc     dl,0
        ret

ENDP    SV_pixelAddr16m

;----------------------------------------------------------------------------
; SV_pixelAddr4G - Determine address of pixel in 4G color banked modes
;----------------------------------------------------------------------------
;
; Entry:        _AX -   y-coordinate
;               _BX -   x-coordinate
;
; Exit:         _BX -   byte offset in buffer
;               _DX -   64k bank number of the address
;
; Registers:    None.
;
;----------------------------------------------------------------------------
PROC    SV_pixelAddr4G  near

        mul     [USHORT DC.bytesperline]        ; DX:AX := y * bytesperline
        shl     _bx,2
        add     ax,bx
        adc     dx,0                ; DX:BX := y * BytesPerLine + x * 2
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        adc     dl,[BYTE DC.bankOffset]; DL := bank number
        add     bx,[USHORT DC.originOffset]
        adc     dl,0
        ret

ENDP    SV_pixelAddr4G

;----------------------------------------------------------------------------
; clearBanked   - Clear all banks in the video page
;----------------------------------------------------------------------------
; Entry:        EAX - Value to clear with
;
; Registers:    All.
;----------------------------------------------------------------------------
PROC    clearBanked

        LOCAL   color:ULONG = LocalSize

        enter_c LocalSize
        mov     [color],eax

        mov     _ax,[DC.maxy]
        inc     _ax
        mul     [USHORT DC.bytesperline]     ; DX:AX := number of bytes to fill
        mov     bx,ax
        add     bx,[USHORT DC.originOffset] ; BX := bytes in last bank to fill
        adc     dl,0
        mov     dh,dl               ; DH := number of full banks to fill

        lVidMem _di                 ; ES:_DI -> video memory
        add     di,[USHORT DC.originOffset]
        mov     dl,[BYTE DC.bankOffset]; DL := starting bank number
        cld                         ; Moves go up in memory

		or      dh,dh               ; More than one bank to fill?
        jz      @@SingleBank        ; No, only fill a single bank

; Fill the first partial bank

		call    _SV_setBankASMDL
        mov     eax,[color]
        mov     ecx,0FFFFh
        sub     cx,[USHORT DC.originOffset]
        inc     ecx
        shr     ecx,2
    rep stosd
        lVidMem _di                 ; ES:_DI -> video memory
        inc     dl
        dec     dh
        jz      @@SingleBank        ; Only two banks to fill

; Fill all of the full 64k banks

@@InnerLoop:
		call    _SV_setBankASMDL
        mov     eax,[color]
        mov     _cx,4000h           ; Need to set 4000h double USHORTs per bank
        push    _di
    rep stosd
        pop     _di
        inc     dl
        dec     dh
        jnz     @@InnerLoop

; Now fill the last partial bank

@@SingleBank:
		call    _SV_setBankASMDL
        mov     eax,[color]
        xor     _cx,_cx
        mov     cx,bx
        shr     _cx,2               ; _CX := number of double words to set
    rep stosd

        leave_c
        ret

ENDP    clearBanked

;----------------------------------------------------------------------------
; void clear16(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Works even if the display contains more than
; one bank, so will work for 1024x768 and 1280x1024 video modes.
;----------------------------------------------------------------------------
procstartdll   __SV_clear16

        ARG     color:ULONG

        enter_c 0
        save_es

; Setup graphics controller

        mov     dx,3CEh             ; DX := Graphics Controller I/O port

        mov     ah,[BYTE color]     ; AH := Background color
        xor     al,al               ; AL := 0 (Set/Reset register number)
        out     dx,ax               ; load set/reset register

        mov     ax,0F01h            ; AH := 1111b (mask for Enable set/reset)
                                    ; AL := 1 (enable Set/reset reg number)
        out     dx,ax               ; load enable set/reset reg

        call    clearBanked         ; Clear the video memory page

; Restore graphics controller

        mov     dx,3CEh             ; DX := Graphics Controller I/O port
        xor     ax,ax               ; AH := 0, AL := 0
        out     dx,ax               ; Restore default Set/Reset register

        inc     ax                  ; AH := 0, AL := 1
        out     dx,ax               ; Restore enable Set/Reset register

        restore_es
        leave_c_nolocal
        ret

procend     __SV_clear16

;----------------------------------------------------------------------------
; void clear256(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear256

        ARG     color:ULONG

        enter_c 0
        save_es

        xor     eax,eax
        mov     al,[BYTE color]
        mov     ebx,eax
        shl     ebx,8
        or      eax,ebx
        mov     ebx,eax
        shl     ebx,16
        or      eax,ebx             ; EAX = 32 bit color value

        call    clearBanked

        restore_es
        leave_c_nolocal
        ret

procend     __SV_clear256

;----------------------------------------------------------------------------
; void clear32k(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear32k

        ARG     color:ULONG

        enter_c 0
        save_es

        xor     eax,eax
        mov     ax,[WORD color]
        mov     ebx,eax
        shl     ebx,16
        or      eax,ebx             ; EAX = 32 bit color value
        call    clearBanked         ; Clear the video memory page

        restore_es
        leave_c_nolocal
        ret

procend     __SV_clear32k

;----------------------------------------------------------------------------
; void clear16m(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear16m

        ARG     color:ULONG

        enter_c 0
        save_es

        mov     eax,[color]
        or      eax,eax
        jnz     @@SlowClear
        call    clearBanked         ; Clear the video memory page to black
        jmp     @@Exit

@@SlowClear:
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,[USHORT DC.originOffset]
        mov     dl,[BYTE DC.bankOffset]; DL := starting bank number
		call    _SV_setBankASMDL    ; Change to starting bank number

        mov     _di,[DC.maxx]
        inc     _di                 ; _DI := number of pixels to draw
        mov     _si,[DC.maxy]
        inc     _si                 ; _SI := number of lines to process
        mov     ax,[WORD color]     ; AX := pixel color
        mov     dh,[BYTE color+2]   ; DH := top byte of pixel color
        mov     bp,di
        shl     bp,1
        add     bp,di               ; BP := bytes per physical scanline
        sub     bp,[USHORT DC.bytesperline]
        neg     bp                  ; BP := scanline adjust factor

@@NextScanLine:
        mov     _cx,_di

@@LoopSolid:
        cmp     bx,0FFFDh
        jae     @@BankSwitch        ; Bank switch occurs!

        mov     [WORD _ES _bx],ax   ; Set pixel value in buffer
        mov     [BYTE _ES _bx+2],dh
        add     _bx,3               ; Increment to next pixel
        loop    @@LoopSolid         ; Loop across line

@@AfterPlot:
        add     bx,bp
        jc      @@BankSwitch2
        dec     _si
        jnz     @@NextScanLine
        jmp     @@Exit

@@BankSwitch:
        call    DrawPixelAFSLOW16m
        inc     dl
        loop    @@LoopSolid         ; Loop across line
        jmp     @@AfterPlot

@@BankSwitch2:
        inc     dl
		call    _SV_setBankASMDL
		dec     _si
        jnz     @@NextScanLine

@@Exit:
        restore_es
        leave_c_nolocal
        ret

procend     __SV_clear16m

;----------------------------------------------------------------------------
; void clear4G(long color)
;----------------------------------------------------------------------------
; Routine to clear the screen. Assumes pages begin on bank boundaries
; for simplicity of coding.
;----------------------------------------------------------------------------
procstartdll   __SV_clear4G

        ARG     color:ULONG

        enter_c 0
        save_es

        mov     eax,[color]
        call    clearBanked         ; Clear the video memory page

        restore_es
        leave_c_nolocal
        ret

procend     __SV_clear4G

;----------------------------------------------------------------------------
; void putPixel16(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel16

        ARG     x:UINT, y:UINT, color:ULONG

        enter_c 0
        save_es

; Compute the pixel's address in video buffer

        mov     _ax,[y]
        mov     _bx,[x]
        mul     [USHORT DC.bytesperline]     ; DX:AX := y * BytesPerLine

        mov     cl,bl               ; CL := low-order byte of x

        shr     _bx,3               ; _BX := x/8
        add     ax,bx
        adc     dx,0                ; DX:BX := y*BytesPerLine + x/8
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        add     bx,[USHORT DC.originOffset]   ; DX:BX := byte offset in video buffer
        adc     dx,[DC.bankOffset]

        cmp     dl,[BYTE DC.curBank]
        je      @@NoChange

		call    _SV_setBankASMDL

@@NoChange:
        mov     ah,1                ; AH := unshifted bit mask
        and     cl,7                ; CL := x & 7
        xor     cl,7                ; CL := # bits to shift left

; set Graphics Controller Bit Mask register

        shl     ah,cl               ; AH := bit mask in proper postion
        mov     dx,3CEh             ; GC address register port
        mov     al,8                ; AL := Bit Mask Register number
        out     dx,ax

; set Graphics Controller Mode register

        mov     ax,0205h            ; AL := Mode register number
                                    ; AH := Write mode 2 (bits 0,1)
                                    ;   Read mode 0 (bit 3)
        out     dx,ax

; set data rotate/Function Select register

        mov     ax,3                ; AL := Data Rotate/Func select reg #
        out     dx,ax

; set the pixel value

        mov     al,[_ES _bx]        ; latch one byte from each bit plane
        mov     al,[BYTE color]     ; AL := pixel value
        mov     [_ES _bx],al        ; update all bit planes

; restore default Graphics Controller registers

        mov     ax,0FF08h           ; default bit mask
        out     dx,ax

        mov     ax,0005h            ; default mode register
        out     dx,ax

        mov     ax,0003h            ; default function select
        out     dx,ax

        restore_es
        leave_c_nolocal
        ret

procend     __SV_putPixel16

;----------------------------------------------------------------------------
; void putPixel256(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel256

        ARG     x:UINT, y:UINT, color:ULONG

        enter_c 0
        save_es

        mov     _ax,[y]
        mul     [USHORT DC.bytesperline]
        mov     _bx,[x]
        add     ax,bx
        adc     dl,[BYTE DC.bankOffset]; DL:_BX := y * BytesPerLine + x
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        add     bx,[USHORT DC.originOffset]
        adc     dl,0
        cmp     dl,[BYTE DC.curBank]
        je      @@NoChange

		call    _SV_setBankASMDL

@@NoChange:
        mov     al,[BYTE color]
        mov     [_ES _bx],al        ; Replace the pixel

        restore_es
        leave_c_nolocal
        ret

procend     __SV_putPixel256

;----------------------------------------------------------------------------
; void putPixel32k(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel32k

        ARG     x:UINT, y:UINT, color:ULONG

        enter_c 0
        save_es

        mov     _ax,[y]
        mul     [USHORT DC.bytesperline]
        mov     _bx,[x]
        shl     _bx,1
        add     ax,bx
        adc     dl,[BYTE DC.bankOffset]; DL:_BX := y * BytesPerLine + x
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        add     bx,[USHORT DC.originOffset]
        adc     dl,0
        cmp     dl,[BYTE DC.curBank]
        je      @@NoChange

		call    _SV_setBankASMDL

@@NoChange:
        mov     ax,[USHORT color]
        mov     [_ES _bx],ax        ; Replace the pixel

        restore_es
        leave_c_nolocal
        ret

procend     __SV_putPixel32k

;----------------------------------------------------------------------------
; void putPixel16m(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel16m

        ARG     x:UINT, y:UINT, color:ULONG

        enter_c 0
        save_es

        mov     _ax,[y]
        mul     [USHORT DC.bytesperline]
        mov     _bx,[x]
        add     ax,bx
        adc     dx,0
        shl     bx,1
        add     ax,bx
        adc     dl,[BYTE DC.bankOffset]; DX:AX := y * BytesPerLine + x * 3
        add     ax,[USHORT DC.originOffset]
        adc     dl,0
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax               ; BX := Offset in buffer
        cmp     dl,[BYTE DC.curBank]
        je      @@NoChange

		call    _SV_setBankASMDL

@@NoChange:
        mov     ax,[WORD color]
        mov     dh,[BYTE color+2]
        cmp     bx,0FFFEh
        jae     @@SlowVersion       ; Bank switch occurs in pixel!

        mov     [_ES _bx],ax        ; Replace the first byte
        mov     [_ES _bx+2],dh

@@Exit: restore_es
        leave_c_nolocal
        ret

@@SlowVersion:
        call    DrawPixelAFSLOW16m  ; Draw the pixel slowly
        jmp     @@Exit

procend     __SV_putPixel16m

;----------------------------------------------------------------------------
; IncBXDL   Increment the BX/DL offset bank number combination
;----------------------------------------------------------------------------
;
; This routine is called in place to increment the value of DL:BX where
; DL is the current bank offset, and BX is the current frame buffer offset.
; The routine also ensures that the bank boundary is correctly crossed. It
; is slow, but only gets called for about less than 10 pixels on the entire
; display page.
;
; Entry:        BX  - Video buffer offset
;               DL  - Video bank number
;
; Exit:         BX  - New buffer offset (+1)
;               DL  - New bank bumber (carried over from BX)
;
; Registers:    None.
;
;----------------------------------------------------------------------------
PROC    IncBXDL near

        add     bx,1
        adc     dl,0
		call    _SV_setBankASMDL
		ret

ENDP    IncBXDL

;----------------------------------------------------------------------------
; DrawPixelAFSLOW16m    Draws a pixel split across a bank boundary correctly
;----------------------------------------------------------------------------
;
; Draws the pixel taking into account that a bank boundary occurs in the
; middle of the pixel.
;
; Entry:        DH:AX       - Color of the pixel to plot
;               GS:_BX      - Address of pixel to plot
;               DC.curBank  - Current bank number
;
; Exit:         DH:AX       - Color of pixel to plot
;               GS:_BX      - Address of pixel to plot + 3
;               DC.curBank  - Current bank number + 1
;
; Registers:    All preserved.
;
;----------------------------------------------------------------------------
PROC    DrawPixelAFSLOW16m  near

        push    _dx

        mov     dl,[BYTE DC.curBank]
        mov     [_ES _bx],al        ; Replace the pixel
        call    IncBXDL
        mov     [_ES _bx],ah
        call    IncBXDL
        mov     [_ES _bx],dh
        call    IncBXDL
        pop     _dx
        ret

ENDP    DrawPixelAFSLOW16m

;----------------------------------------------------------------------------
; void putPixel4G(int x,int y,long color)
;----------------------------------------------------------------------------
; Routine sets the value of a pixel in native VGA graphics modes.
;
; Entry:        x       -   X coordinate of pixel to draw
;               y       -   Y coordinate of pixel to draw
;               color   -   Color of pixel to draw
;
;----------------------------------------------------------------------------
procstartdll   __SV_putPixel4G

        ARG     x:UINT, y:UINT, color:ULONG

        enter_c 0
        save_es

        mov     _ax,[y]
        mul     [USHORT DC.bytesperline]
        mov     _bx,[x]
        shl     _bx,2
        add     ax,bx
        adc     dl,[BYTE DC.bankOffset]; DL:_BX := y * BytesPerLine + x
        lVidMem _bx                 ; ES:_BX -> video memory
        add     bx,ax
        add     bx,[USHORT DC.originOffset]
        adc     dl,0
        cmp     dl,[BYTE DC.curBank]
        je      @@NoChange

		call    _SV_setBankASMDL

@@NoChange:
        mov     eax,[color]
        mov     [_ES _BX],eax       ; Replace the pixel

        restore_es
        leave_c_nolocal
        ret

procend     __SV_putPixel4G

;----------------------------------------------------------------------------
; void _SV_line16(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:        x1      - X1 coordinate of line to draw
;               y1      - Y1 coordinate of line to draw
;               x2      - X2 coordinate of line to draw
;               y2      - Y2 coordinate of line to draw
;               color   - color to draw the line in
;
;----------------------------------------------------------------------------
procstartdll    __SV_line16

        ARG     x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
        LOCAL   Routine:NCPTR, VertInc:ULONG, EIncr:UINT,           \
                NEIncr:UINT = LocalSize

        enter_c LocalSize
        save_es
        cld

; Configure the graphics controller (write mode 3)

        mov     dx,3CEh             ; DX := Graphics Controller port addr

        mov     ax,0B05h            ; AL := Mode register number
                                    ; AH := Write mode 3 (bits 0,1)
                                    ;   Read mode 1 (bit 3)
        out     dx,ax

        xor     ah,ah               ; AH := replace mode
        mov     al,3                ; AL := Data Rotate/Func select reg #
        out     dx,ax

        mov     ax,0007h            ; AH := 0 (don't care for all maps;
                                    ;   CPU reads always return 0FFH)
                                    ; AL := 7 (Color Don't care reg number)
        out     dx,ax               ; Set up Color Don't care reg

        mov     ax,0F01h            ; AH := 1111b (bit plane mask for
                                    ;  enable Set/Reset)
        out     dx,ax               ; AL := Enable Set/Reset Register #

        mov     ax,0FF08h           ; AH := 11111111b, AL := 8
        out     dx,ax               ; restore bit mask register

; Load the current color

        mov     dx,3CEh             ; DX := Graphics Controller port addr
        mov     ah,[BYTE color]     ; Get color value into ah
        xor     al,al               ; AL := Set/Reset Register number
        out     dx,ax

        mov     si,[USHORT DC.bytesperline] ; Increment for video buffer
        mov     [USHORT VertInc+2],0    ; Zero out sign for vertical increment
        mov     _ax,[x2]
        sub     _ax,[x1]            ; _AX := X2 - X1

; Force X1 < X2

        jns     @@X2Greater         ; Jump if X2 > X1
        neg     _ax                 ; _AX := X1 - X2

        mov     _bx,[x2]            ; Exchange X1 and X2
        xchg    _bx,[x1]
        mov     [x2],_bx

        mov     _bx,[y2]            ; Exchange Y1 and Y2
        xchg    _bx,[y1]
        mov     [y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
        mov     _bx,[y2]
        sub     _bx,[y1]            ; _BX := Y2 - Y1
        jns     @@Y2Greater         ; Jump if slope is positive

        neg     _bx                 ; _BX := Y1 - Y2
        neg     si                  ; negative increment for buffer
        mov     [USHORT VertInc+2],0FFFFh   ; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
        mov     [USHORT VertInc],si ; save increment
        mov     [Routine],offset @@LoSlopeLine
        cmp     _bx,_ax
        jle     @@LoSlope           ; Jump if dy <= dx (Slope <= 1)
        mov     [Routine],offset @@HiSlopeLine
        xchg    _bx,_ax             ; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
        shl     _bx,1               ; _BX := 2 * dy
        mov     [EIncr],_bx         ; EIncr := 2 * dy
        sub     _bx,_ax             ; d = 2 * dy - dx
        mov     _di,_bx             ; DI := initial decision variable
        sub     _bx,_ax
        mov     [NEIncr],_bx        ; NEIncr := 2 * (dy - dx)

; calculate first pixel address

        push    _ax                 ; preserve dx
        mov     _ax,[y1]
        mov     _bx,[x1]
        call    SV_pixelAddr16      ; AH := Bit mask
                                    ; ES:_BX -> buffer
                                    ; CL := # bits to shift left
                                    ; _SI := bank number
        shl     ah,cl               ; AH := bit mask in proper position
        mov     al,ah               ; AL := bit Mask
        pop     _cx                 ; Restore dx
        inc     _cx                 ; _CX := # pixels to draw

        jmp     [Routine]           ; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;                       ES:_BX -> video buffer
;                       AL = bit mask for 1st pixel
;                       AH = bit mask for 1st pixel
;                       _CX = # pixels to draw
;                       DX = Graphics Controller data register port addr
;                       _SI = bank number
;                       _DI = Initial decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
; The Graphics Controller index register should be set to point to the
; Bit Mask register.
;
;****************************************************************************

@@LoSlopeLine:

@@StartLo:
        mov     al,ah               ; AL := bit mask for next pixel
        cmp     si,[USHORT DC.curBank]
        je      @@BitMaskIn

        push    _ax                 ; Preserve AX
        mov     _ax,_si             ; AX := new bank number
		call    _SV_setBankASM      ; Program this bank
        pop     _ax

@@BitMaskIn:
        or      al,ah               ; mask current pixel position

        ror     ah,1                ; Rotate pixel value
        jc      @@BitMaskOut        ; Jump if mask rotated to leftmost pixel

; bit mask not shifted out

        or      _di,_di             ; test sign of d
        jns     @@InPosDi           ; jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@BitMaskIn

        and     [_ES _bx],al        ; set remaining pixel(s)
        jmp     @@Exit

@@InPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr

        and     [_ES _bx],al        ; Update bit planes

        add     bx,[USHORT VertInc] ; increment y
        adc     si,[USHORT VertInc+2]   ; Adjust bank number
        loop    @@StartLo
        jmp     @@Exit

; bit mask shifted out

@@BitMaskOut:
        and     [_ES _bx],al        ; Update bit planes
        add     bx,1                ; increment x
        adc     si,0                ; Adjust bank value

        or      _di,_di             ; test sign of d
        jns     @@OutPosDi          ; jump if non-negative

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@StartLo
        jmp     @@Exit

@@OutPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,[USHORT VertInc] ; vertical increment
        adc     si,[USHORT VertInc+2]   ; Adjust bank number
        loop    @@StartLo
        jmp     @@Exit

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;                       ES:_BX -> video buffer
;                       AL = bit mask for 1st pixel
;                       _CX = # pixels to draw
;                       DX = Graphics Controller data register port addr
;                       _SI = bank number
;                       _DI = Initial decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
; The Graphics Controller index register should be set to point to the
; Bit Mask register.
;
;****************************************************************************

@@HiSlopeLine:

@@StartHi:
        cmp     si,[USHORT DC.curBank]
        je      @@SetHi

        push    _ax                 ; Preserve AX
        mov     _ax,_si             ; AX := new bank number
		call    _SV_setBankASM      ; Program this bank
        pop     _ax

@@SetHi:
        and     [_ES _bx],al        ; update bit planes

        add     bx,[USHORT VertInc] ; increment y
        adc     si,[USHORT VertInc+2]   ; Adjust bank number

        or      _di,_di             ; test sign of d
        jns     @@HiPosDi           ; jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@StartHi
        jmp     @@Exit

@@HiPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr

        ror     al,1                ; rotate bit mask
        adc     bx,0                ; Increment BX when mask rotated to
                                    ;  leftmost pixel position
        adc     si,0                ; Adjust bank number
        loop    @@StartHi

; Restore graphics controller and return to caller

@@Exit:
        mov     dx,3CEh             ; DX := Graphics Controller port addr
        xor     ax,ax               ; AH := 0, AL := 0
        out     dx,ax               ; Restore Set/Reset Register

        inc     ax                  ; AH := 0, AL := 1
        out     dx,ax               ; Restore Enable Set/Reset register

        mov     al,3                ; AH := 0, AL := 3
        out     dx,ax               ; Restore Data Rotate/Func select reg

        mov     al,5                ; AH := 0, AL := 5
        out     dx,ax               ; default mode register

        mov     ax,0F07h            ; default color compare value
        out     dx,ax

        mov     ax,0FF08h           ; AH := 11111111b, AL := 8
        out     dx,ax               ; restore bit mask register

        restore_es
        leave_c
        ret

procend     __SV_line16

;----------------------------------------------------------------------------
; void _SV_line256(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:        x1      - X1 coordinate of line to draw
;               y1      - Y1 coordinate of line to draw
;               x2      - X2 coordinate of line to draw
;               y2      - Y2 coordinate of line to draw
;               color   - color to draw the line in
;
;----------------------------------------------------------------------------
procstartdll    __SV_line256

        ARG     x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
        LOCAL   Routine:NCPTR, VertInc:ULONG, EIncr:UINT,           \
                NEIncr:UINT = LocalSize

        enter_c LocalSize
        save_es

        mov     si,[USHORT DC.bytesperline] ; Increment for video buffer
        mov     [USHORT VertInc+2],0    ; Zero out sign for vertical increment

        mov     _ax,[x2]
        sub     _ax,[x1]            ; _AX := X2 - X1

; Force X1 < X2

        jns     @@X2Greater         ; Jump if X2 > X1
        neg     _ax                 ; _AX := X1 - X2

        mov     _bx,[x2]            ; Exchange X1 and X2
        xchg    _bx,[x1]
        mov     [x2],_bx

        mov     _bx,[y2]            ; Exchange Y1 and Y2
        xchg    _bx,[y1]
        mov     [y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
        mov     _bx,[y2]
        sub     _bx,[y1]            ; _BX := Y2 - Y1
        jns     @@Y2Greater         ; Jump if slope is positive

        neg     _bx                 ; _BX := Y1 - Y2
        neg     si                  ; negative increment for buffer
        mov     [USHORT VertInc+2],0FFFFh   ; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
        mov     [USHORT VertInc],si ; save increment
        mov     [Routine],offset @@LoSlopeLine
        cmp     _bx,_ax
        jle     @@LoSlope           ; Jump if dy <= dx (Slope <= 1)
        mov     [Routine],offset @@HiSlopeLine
        xchg    _bx,_ax             ; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
        shl     _bx,1               ; _BX := 2 * dy
        mov     [EIncr],_bx         ; EIncr := 2 * dy
        sub     _bx,_ax             ; d = 2 * dy - dx
        mov     _di,_bx             ; _DI := initial decision variable
        sub     _bx,_ax
        mov     [NEIncr],_bx        ; NEIncr := 2 * (dy - dx)

; calculate first pixel address

        push    _ax                 ; preserve dx
        mov     _ax,[y1]
        mov     _bx,[x1]
        call    SV_pixelAddr256     ; ES:_BX -> buffer

        pop     _cx                 ; Restore dx
        inc     _cx                 ; CX := # pixels to draw

        jmp     [Routine]           ; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
        mov     al,[BYTE color]     ; AL := pixel value to fill
        mov     dh,[BYTE DC.curBank]    ; DH := current bank number

@@LoopLo:
        cmp     dl,dh
        je      @@SetLo

        mov     dh,al               ; DH := color value
		call    _SV_setBankASMDL    ; Program this bank
        mov     al,dh               ; AL := color value
        mov     dh,dl               ; DH := current bank number

@@SetLo:
        mov     [_ES _bx],al        ; Set pixel value in buffer
        add     bx,1                ; Increment x coordinate
        adc     dl,0                ; Adjust bank number
        or      _di,_di             ; Test sign of d
        jns     @@LoPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@LoPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; adjust page number
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
        mov     al,[BYTE color]     ; AL := pixel value to fill
        mov     dh,[BYTE DC.curBank]    ; DH := current bank number

@@LoopHi:
        cmp     dh,dl
        je      @@SetHi

        mov     dh,al               ; DH := color value
		call    _SV_setBankASMDL    ; Program this bank
        mov     al,dh               ; AL := color value
        mov     dh,dl               ; DH := current bank number

@@SetHi:
        mov     [_ES _bx],al        ; Set pixel value in buffer
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; Adjust bank number
        or      _di,_di             ; Test sign of d
        jns     @@HiPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopHi            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@HiPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,1                ; Increment x
        adc     dl,0                ; Adjust bank number
        loop    @@LoopHi            ; Loop for remaining pixels

@@Exit: restore_es
        leave_c
        ret

procend     __SV_line256

;----------------------------------------------------------------------------
; void _SV_line32k(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:        x1      - X1 coordinate of line to draw
;               y1      - Y1 coordinate of line to draw
;               x2      - X2 coordinate of line to draw
;               y2      - Y2 coordinate of line to draw
;               color   - color to draw the line in
;
;----------------------------------------------------------------------------
procstartdll    __SV_line32k

        ARG     x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
        LOCAL   Routine:NCPTR, VertInc:ULONG, EIncr:UINT,           \
                NEIncr:UINT = LocalSize

        enter_c LocalSize
        save_es
        cld

        mov     si,[USHORT DC.bytesperline] ; Increment for video buffer
        mov     [USHORT VertInc+2],0    ; Zero out sign for vertical increment

        mov     _ax,[x2]
        sub     _ax,[x1]            ; _AX := X2 - X1

; Force X1 < X2

        jns     @@X2Greater         ; Jump if X2 > X1
        neg     _ax                 ; _AX := X1 - X2

        mov     _bx,[x2]            ; Exchange X1 and X2
        xchg    _bx,[x1]
        mov     [x2],_bx

        mov     _bx,[y2]            ; Exchange Y1 and Y2
        xchg    _bx,[y1]
        mov     [y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
        mov     _bx,[y2]
        sub     _bx,[y1]            ; _BX := Y2 - Y1
        jns     @@Y2Greater         ; Jump if slope is positive

        neg     _bx                 ; _BX := Y1 - Y2
        neg     si                  ; negative increment for buffer
        mov     [USHORT VertInc+2],0FFFFh   ; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
        mov     [USHORT VertInc],si ; save increment
        mov     [Routine],offset @@LoSlopeLine
        cmp     _bx,_ax
        jle     @@LoSlope           ; Jump if dy <= dx (Slope <= 1)
        mov     [Routine],offset @@HiSlopeLine
        xchg    _bx,_ax             ; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
        shl     _bx,1               ; _BX := 2 * dy
        mov     [EIncr],_bx         ; EIncr := 2 * dy
        sub     _bx,_ax             ; d = 2 * dy - dx
        mov     _di,_bx             ; _DI := initial decision variable
        sub     _bx,_ax
        mov     [NEIncr],_bx        ; NEIncr := 2 * (dy - dx)

; calculate first pixel address

        push    _ax                 ; preserve dx
        mov     _ax,[y1]
        mov     _bx,[x1]
        call    SV_pixelAddr32k     ; ES:_BX -> buffer

        pop     _cx                 ; Restore dx
        inc     _cx                 ; _CX := # pixels to draw

        jmp     [Routine]           ; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
        mov     ax,[USHORT color]       ; AX := pixel value to fill
        mov     dh,[BYTE DC.curBank]    ; DH := current bank number

@@LoopLo:
        cmp     dl,dh
        je      @@SetLo

        push    _ax                 ; Save color value
		call    _SV_setBankASMDL    ; Program this bank
        mov     dh,dl               ; DH := current bank number
        pop     _ax                 ; Restore color value

@@SetLo:
        mov     [_ES _bx],ax        ; Set pixel value in buffer
        add     bx,2                ; Increment x coordinate
        adc     dl,0                ; Adjust bank number
        or      _di,_di             ; Test sign of d
        jns     @@LoPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@LoPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; adjust page number
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
        mov     ax,[USHORT color]       ; AL := pixel value to fill
        mov     dh,[BYTE DC.curBank]    ; DH := current bank number

@@LoopHi:
        cmp     dh,dl
        je      @@SetHi

        push    _ax                 ; Save color value
		call    _SV_setBankASMDL    ; Program this bank
        mov     dh,dl               ; DH := current bank number
        pop     _ax                 ; Restore color value

@@SetHi:
        mov     [_ES _bx],ax        ; Set pixel value in buffer
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; Adjust bank number
        or      _di,_di             ; Test sign of d
        jns     @@HiPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopHi            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@HiPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,2                ; Increment x
        adc     dl,0                ; Adjust bank number
        loop    @@LoopHi            ; Loop for remaining pixels

@@Exit: restore_es
        leave_c
        ret

procend     __SV_line32k

;----------------------------------------------------------------------------
; void _SV_line16m(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:        x1      - X1 coordinate of line to draw
;               y1      - Y1 coordinate of line to draw
;               x2      - X2 coordinate of line to draw
;               y2      - Y2 coordinate of line to draw
;               color   - color to draw the line in
;
;----------------------------------------------------------------------------
procstartdll    __SV_line16m

        ARG     x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
        LOCAL   Routine:NCPTR, VertInc:ULONG, EIncr:UINT,           \
                NEIncr:UINT = LocalSize

        enter_c LocalSize
        save_es
        cld

        mov     si,[USHORT DC.bytesperline] ; Increment for video buffer
        mov     [USHORT VertInc+2],0    ; Zero out sign for vertical increment

        mov     _ax,[x2]
        sub     _ax,[x1]            ; _AX := X2 - X1

; Force X1 < X2

        jns     @@X2Greater         ; Jump if X2 > X1
        neg     _ax                 ; _AX := X1 - X2

        mov     _bx,[x2]            ; Exchange X1 and X2
        xchg    _bx,[x1]
        mov     [x2],_bx

        mov     _bx,[y2]            ; Exchange Y1 and Y2
        xchg    _bx,[y1]
        mov     [y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
        mov     _bx,[y2]
        sub     _bx,[y1]            ; _BX := Y2 - Y1
        jns     @@Y2Greater         ; Jump if slope is positive

        neg     _bx                 ; _BX := Y1 - Y2
        neg     si                  ; negative increment for buffer
        mov     [USHORT VertInc+2],0FFFFh   ; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
        mov     [USHORT VertInc],si ; save increment
        mov     [Routine],offset @@LoSlopeLine
        cmp     _bx,_ax
        jle     @@LoSlope           ; Jump if dy <= dx (Slope <= 1)
        mov     [Routine],offset @@HiSlopeLine
        xchg    _bx,_ax             ; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
        shl     _bx,1               ; _BX := 2 * dy
        mov     [EIncr],_bx         ; EIncr := 2 * dy
        sub     _bx,_ax             ; d = 2 * dy - dx
        mov     _di,_bx             ; _DI := initial decision variable
        sub     _bx,_ax
        mov     [NEIncr],_bx        ; NEIncr := 2 * (dy - dx)

; calculate first pixel address

        push    _ax                 ; preserve dx
        mov     _ax,[y1]
        mov     _bx,[x1]
        call    SV_pixelAddr16m     ; ES:_BX -> buffer

        pop     _cx                 ; Restore dx
        inc     _cx                 ; CX := # pixels to draw

        jmp     [Routine]           ; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
        mov     ax,[USHORT color]       ; AX := pixel value to fill
        mov     dh,[BYTE color+2]   ; DH := top byte of pixel value

@@LoopLo:
        cmp     dl,[BYTE DC.curBank]
        je      @@SetPixelLo

        push    _ax                 ; Save color value
		call    _SV_setBankASMDL    ; Program this bank
        pop     _ax                 ; Restore color value

@@SetPixelLo:
        cmp     bx,0FFFEh
        jae     @@BankSwitchLo

@@SetLo:
        mov     [_ES _bx],ax        ; Set pixel value in buffer
        mov     [_ES _bx+2],dh
        add     bx,3                ; Increment x coordinate
        adc     dl,0                ; Adjust bank number

@@DonePixelLo:
        or      _di,_di             ; Test sign of d
        jns     @@LoPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@LoPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; adjust page number
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@BankSwitchLo:
        call    DrawPixelAFSLOW16m
        inc     dl
        jmp     @@DonePixelLo

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
        mov     ax,[USHORT color]       ; AL := pixel value to fill
        mov     dh,[BYTE color+2]   ; DH := current bank number

@@LoopHi:
        cmp     dl,[BYTE DC.curBank]
        je      @@SetPixelHi

        push    _ax                 ; Save color value
		call    _SV_setBankASMDL    ; Program this bank
        pop     _ax                 ; Restore color value

@@SetPixelHi:
        cmp     bx,0FFFEh
        jae     @@BankSwitchHi

@@SetHi:
        mov     [_ES _bx],ax        ; Set pixel value in buffer
        mov     [_ES _bx+2],dh

@@DonePixelHi:
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; Adjust bank number
        or      _di,_di             ; Test sign of d
        jns     @@HiPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopHi            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@HiPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,3                ; Increment x
        adc     dl,0                ; Adjust bank number
        loop    @@LoopHi            ; Loop for remaining pixels
        jmp     @@Exit

@@BankSwitchHi:
        call    DrawPixelAFSLOW16m
        sub     bx,3
        jmp     @@DonePixelHi

@@Exit: restore_es
        leave_c
        ret

procend     __SV_line16m

;----------------------------------------------------------------------------
; void _SV_line4G(int x1,int y1,int x2,int y2, long color)
;----------------------------------------------------------------------------
; Routine draws a line in native VGA graphics modes.
;
; Differentiates between horizontal, vertical and sloping lines. Horizontal
; and vertical lines are special cases and can be drawn extremely quickly.
; The sloping lines are drawn using the Midpoint line algorithm.
;
; Entry:        x1      - X1 coordinate of line to draw
;               y1      - Y1 coordinate of line to draw
;               x2      - X2 coordinate of line to draw
;               y2      - Y2 coordinate of line to draw
;               color   - color to draw the line in
;
;----------------------------------------------------------------------------
procstartdll    __SV_line4G

        ARG     x1:UINT, y1:UINT, x2:UINT, y2:UINT, color:ULONG
        LOCAL   Routine:NCPTR, VertInc:ULONG, EIncr:UINT,           \
                NEIncr:UINT = LocalSize

        enter_c LocalSize
        save_es
        push    eax
        cld

        mov     si,[USHORT DC.bytesperline] ; Increment for video buffer
        mov     [USHORT VertInc+2],0    ; Zero out sign for vertical increment

        mov     _ax,[x2]
        sub     _ax,[x1]            ; _AX := X2 - X1

; Force X1 < X2

        jns     @@X2Greater         ; Jump if X2 > X1
        neg     _ax                 ; _AX := X1 - X2

        mov     _bx,[x2]            ; Exchange X1 and X2
        xchg    _bx,[x1]
        mov     [x2],_bx

        mov     _bx,[y2]            ; Exchange Y1 and Y2
        xchg    _bx,[y1]
        mov     [y2],_bx

; calcluate dy = ABS(Y2-Y1)

@@X2Greater:
        mov     _bx,[y2]
        sub     _bx,[y1]            ; _BX := Y2 - Y1
        jns     @@Y2Greater         ; Jump if slope is positive

        neg     _bx                 ; _BX := Y1 - Y2
        neg     si                  ; negative increment for buffer
        mov     [USHORT VertInc+2],0FFFFh   ; ensure vert increment is negative

; select appropriate routine for slope of line

@@Y2Greater:
        mov     [USHORT VertInc],si ; save increment
        mov     [Routine],offset @@LoSlopeLine
        cmp     _bx,_ax
        jle     @@LoSlope           ; Jump if dy <= dx (Slope <= 1)
        mov     [Routine],offset @@HiSlopeLine
        xchg    _bx,_ax             ; exchange dy and dx

; calculate initial decision variable and increments

@@LoSlope:
        shl     _bx,1               ; _BX := 2 * dy
        mov     [EIncr],_bx         ; EIncr := 2 * dy
        sub     _bx,_ax             ; d = 2 * dy - dx
        mov     _di,_bx             ; _DI := initial decision variable
        sub     _bx,_ax
        mov     [NEIncr],_bx        ; NEIncr := 2 * (dy - dx)

; calculate first pixel address

        push    _ax                 ; preserve dx
        mov     _ax,[y1]
        mov     _bx,[x1]
        call    SV_pixelAddr4G      ; ES:_BX -> buffer

        pop     _cx                 ; Restore dx
        inc     _cx                 ; _CX := # pixels to draw

        jmp     [Routine]           ; jump to appropriate routine

;****************************************************************************
;
; Routine for dy <= dx (slope <= 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@LoSlopeLine:
        mov     eax,[color]         ; EAX := pixel value to fill
        mov     dh,[BYTE DC.curBank]    ; DH := current bank number

@@LoopLo:
        cmp     dl,dh
        je      @@SetLo

        push    _ax                 ; Save color value
		call    _SV_setBankASMDL    ; Program this bank
        mov     dh,dl               ; DH := current bank number
        pop     _ax                 ; Restore color value

@@SetLo:
        mov     [_ES _bx],eax       ; Set pixel value in buffer
        add     bx,4                ; Increment x coordinate
        adc     dl,0                ; Adjust bank number
        or      _di,_di             ; Test sign of d
        jns     @@LoPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@LoPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; adjust page number
        loop    @@LoopLo            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

;****************************************************************************
;
; Routine for dy > dx (slope > 1)
;                       ES:_BX -> video buffer
;                       _CX = # pixels to draw
;                       _DX = Bank number for first pixel
;                       _DI = decision variable
;                       EIncr   - East pixel increment
;                       NEIncr  - North East pixel increment
;
;****************************************************************************

@@HiSlopeLine:
        mov     eax,[color]         ; EAX := pixel value to fill
        mov     dh,[BYTE DC.curBank]    ; DH := current bank number

@@LoopHi:
        cmp     dh,dl
        je      @@SetHi

        push    _ax                 ; Save color value
		call    _SV_setBankASMDL    ; Program this bank
        mov     dh,dl               ; DH := current bank number
        pop     _ax                 ; Restore color value

@@SetHi:
        mov     [_ES _bx],eax       ; Set pixel value in buffer
        add     bx,[USHORT VertInc] ; increment y
        adc     dl,[BYTE VertInc+2] ; Adjust bank number
        or      _di,_di             ; Test sign of d
        jns     @@HiPosDi           ; Jump if d >= 0

        add     _di,[EIncr]         ; d := d + EIncr
        loop    @@LoopHi            ; Loop for remaining pixels
        jmp     @@Exit              ; We are all done

@@HiPosDi:
        add     _di,[NEIncr]        ; d := d + NEIncr
        add     bx,4                ; Increment x
        adc     dl,0                ; Adjust bank number
        loop    @@LoopHi            ; Loop for remaining pixels

@@Exit: pop     eax
        restore_es
        leave_c
        ret

procend     __SV_line4G

;----------------------------------------------------------------------------
; void _SV_setActivePage(int which)
;----------------------------------------------------------------------------
; Routine to set the video page for active output.
;
; Entry:        page    - Page number of page to use
;
;----------------------------------------------------------------------------
procstartdll   __SV_setActivePage

        ARG     which:UINT

        enter_c 0

; Calculate 18 bit address of page in video memory

		xor     eax,eax
		mov     _ax,[which]        		; EAX := page number
		mul     [__SV_pagesize]   		; EDX:EAX := result
		mov     [USHORT DC.originOffset],ax  ; Save video buffer offset
		shr     eax,16
		mov     [DC.bankOffset],ax    	; Save video bank offset
        call    _SV_setBankASM

        leave_c_nolocal
        ret

procend     __SV_setActivePage

if pmode
;----------------------------------------------------------------------------
; void _SV_callSetCRT20(ulong addr,int waitFlag)
;----------------------------------------------------------------------------
; Call the VBE 2.0 32-bit display start function.
;----------------------------------------------------------------------------
procstart	__SV_callSetCRT20

		ARG     addr:ULONG, waitFlag:UINT

		enter_c 0
		push    es

		mov		eax,[addr]
		mov     cx,ax
		shr     eax,16
		mov     dx,ax               ; DX:CX := CRTC start address
		mov		_bx,[waitFlag]
		mov     es,[_VBE_MMIOSel]   ; Load selector to MMIO registers
		call    [__SV_setCRT20]     ; Call the relocated code

		pop     es
		leave_c_nolocal
		ret

procend     __SV_callSetCRT20
endif

;----------------------------------------------------------------------------
; void __SV_waitDEVGA(void)
;----------------------------------------------------------------------------
; Wait for display enable for VGA compatible controllers
;----------------------------------------------------------------------------
procstart	__SV_waitDEVGA

		mov     dx,03DAh            ; DX := video status port
@@Wait:	in      al,dx
		test    al,1
		jnz     @@Wait            	; Wait for Display Enable
		ret

procend     __SV_waitDEVGA

;----------------------------------------------------------------------------
; void __SV_waitVRTVGA(void)
;----------------------------------------------------------------------------
; Wait for retrace for VGA compatible controllers
;----------------------------------------------------------------------------
procstart	__SV_waitVRTVGA

		mov     dx,03DAh            ; DX := video status port
@@Wait:	in      al,dx               ; Wait for start of vertical retrace
		test    al,8
		jz      @@Wait
		ret

procend     __SV_waitVRTVGA

;----------------------------------------------------------------------------
; void _VGA_setPalette(int start,int num,VBE_palette *pal,bool waitVRT);
;----------------------------------------------------------------------------
; Programs the VGA palette. To avoid the onset of snow, we program values
; only during a vertical retrace interval if BL bit 7 is set. This is a
; compatability routine for VBE 1.2 implemenations, and we will use the
; normal VBE 2.0 routines otherwise.
;----------------------------------------------------------------------------
procstartdll    __VGA_setPalette

        ARG     start:UINT, num:UINT, pal:DPTR, waitVRT:S_BOOL

        enter_c 0
        push    ds

; Wait for the start of the vertical retrace if specified

        cmp     [waitVRT],0
        je      @@NoWaitVRT

        mov     dx,3DAh             ; DX := Input Status register
@@WaitNotVsync:
        in      al,dx
        test    al,8
        jnz     @@WaitNotVsync
@@WaitVsync:
        in      al,dx
        test    al,8
        jz      @@WaitVsync

@@NoWaitVRT:
        _lds    _di,[pal]           ; DS:_DI -> palette to program
        mov     _cx,[num]           ; _CX := registers to program
        mov     _ax,[start]         ; AX := first color register to program
        mov     dx,3C8h             ; DX := DAC Write Index register
        out     dx,al               ; Index the first color register
        inc     _dx                 ; DX := DAC data register
        cld

@@NextEntry:
        mov     al,[_di+2]
        out     dx,al
        mov     al,[_di+1]
        out     dx,al
        mov     al,[_di+0]
        out     dx,al
        add     _di,4
        loop    @@NextEntry

@@Exit: pop     ds
        leave_c_nolocal
        ret

procend     __VGA_setPalette

;----------------------------------------------------------------------------
; _VBE_setBankA Sets the read/write bank directly via window A
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by making a single call to the VBE to set Window A since it has
; both read & write attributes set.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;
;----------------------------------------------------------------------------
procstartdll    __VBE_setBankA

        push    _ax
        push    _bx
        push    _cx
        mov     _ax,04F05h
        xor     _bx,_bx             ; BX := select window A
        mov     cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
        shl     dl,cl               ; DX := VBE bank number
        cmp     [__SV_setBankRM],0
        je      @@UseInt10h
        call    [__SV_setBankRM]        ; Call the VBE directly in real mode
        jmp     @@Exit

@@UseInt10h:
ifdef   __WINDOWS32_386__
        push    esi
        push    edi
        push    edx
        call    _PM_setBankA        ; Special routine to thunk to 16 bit code
        pop     edx
        pop     edi
        pop     esi
else
        int     10h
endif

@@Exit: pop     _cx
		pop     _bx
		pop     _ax
		ret

procend     __VBE_setBankA

;----------------------------------------------------------------------------
; _VBE_setBankAB    Sets the read/write bank directly via Window's A & B
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by calling the VBE twice to set both Window A and Window B, as
; the VBE reports that they have separate read/write attributes.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    __VBE_setBankAB

		push    _ax
		push    _bx
		push    _cx
		mov     _ax,04F05h
		xor     _bx,_bx             ; BX := select window A
		mov     cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
		shl     dl,cl               ; DX := VBE bank number
		push    _dx                 ; Save bank value
		cmp     [__SV_setBankRM],0
		je      @@UseInt10h
        call    [__SV_setBankRM]        ; Call the VBE directly in real mode
        pop     _dx
        mov     _ax,04F05h
        mov     _bx,1               ; BX := select window B
        call    [__SV_setBankRM]
        jmp     @@Exit

@@UseInt10h:
ifdef   __WINDOWS32_386__
        pop     _dx
        push    esi
        push    edi
        push    edx
        call    _PM_setBankAB       ; Special routine to thunk to 16 bit code
        pop     edx
        pop     edi
        pop     esi
else
        int     10h
        pop     _dx
        mov     _ax,04F05h
        mov     _bx,1               ; BX := select window B
        int     10h
endif

@@Exit: pop     _cx
        pop     _bx
        pop     _ax
        ret

procend     __VBE_setBankAB

if pmode

;----------------------------------------------------------------------------
; _VBE20_setBankA   Sets the read/write bank directly via window A
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by making a single call to the VBE to set Window A since it has
; both read & write attributes set.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    __VBE20_setBankA

		push	_ax
        push    _bx
        push    _cx
        xor     _bx,_bx             ; BX := select window A
        mov     cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
        shl     dl,cl               ; DX := VBE bank number
        call    [__SV_setBank20]        ; Call VBE 2.0 protected mode routine
        pop     _cx
		pop     _bx
		pop		_ax
        ret

procend     __VBE20_setBankA

;----------------------------------------------------------------------------
; _VBE20_setBankAB  Sets the read/write bank directly via Window's A & B
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by calling the VBE twice to set both Window A and Window B, as
; the VBE reports that they have separate read/write attributes.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    __VBE20_setBankAB

		push	_ax
        push    _bx
        push    _cx
        xor     _bx,_bx             ; BX := select window A
        mov     cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
        shl     dl,cl               ; DX := VBE bank number
        push    _dx                 ; Save bank value
        call    [__SV_setBank20]        ; Call VBE 2.0 protected mode routine
        pop     _dx
        mov     _bx,1               ; BX := select window B
        call    [__SV_setBank20]        ; Call VBE 2.0 protected mode routine
		pop     _cx
		pop     _bx
		pop		_ax
        ret

procend     __VBE20_setBankAB

;----------------------------------------------------------------------------
; _VBE20_setBankA_ES    Sets the read/write bank directly via window A
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by making a single call to the VBE to set Window A since it has
; both read & write attributes set. This routine is different in the
; respect that the bank switching code uses MMIO registers and requires the
; ES register to be loaded with the selector to its MMIO registers.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    __VBE20_setBankA_ES

		push    es
		push	_ax
        push    _bx
        push    _cx
        mov     es,[_VBE_MMIOSel]   ; Load selector to MMIO registers
        xor     _bx,_bx             ; BX := select window A
        mov     cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
        shl     dl,cl               ; DX := VBE bank number
        call    [__SV_setBank20]        ; Call VBE 2.0 protected mode routine
        pop     _cx
		pop     _bx
		pop		_ax
        pop     es
        ret

procend     __VBE20_setBankA_ES

;----------------------------------------------------------------------------
; _VBE20_setBankAB_ES   Sets the read/write bank directly via Window's A & B
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value, by calling the VBE twice to set both Window A and Window B, as
; the VBE reports that they have separate read/write attributes. This
; routine is different in the respect that the bank switching code uses
; MMIO registers and requires the ES register to be loaded with the selector
; to its MMIO registers.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    __VBE20_setBankAB_ES

		push    es
		push	_ax
        push    _bx
        push    _cx
        mov     es,[_VBE_MMIOSel]   ; Load selector to MMIO registers
        xor     _bx,_bx             ; BX := select window A
        mov     cl,[BYTE __SV_bankShift]; Adjust to VBE granularity
        shl     dl,cl               ; DX := VBE bank number
        push    _dx                 ; Save bank value
        call    [__SV_setBank20]    ; Call VBE 2.0 protected mode routine
        pop     _dx
        mov     _bx,1               ; BX := select window B
        call    [__SV_setBank20]    ; Call VBE 2.0 protected mode routine
        pop     _cx
		pop     _bx
		pop		_ax
        pop     es
        ret

procend     __VBE20_setBankAB_ES

;----------------------------------------------------------------------------
; _VBEAF_setBank    Sets the read/write bank directly via VBE/AF
;----------------------------------------------------------------------------
; This function sets the read/write bank by calling the VBE/AF device driver
; bank switching function.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    __VBEAF10_setBank

		push    ebx
		mov     ebx,[DC.AFDC]
		call    [AF_devCtx.setBank]
		pop     ebx
		ret

procend     	__VBEAF10_setBank

;----------------------------------------------------------------------------
; _VBEAF_setBank    Sets the read/write bank directly via VBE/AF
;----------------------------------------------------------------------------
; This function sets the read/write bank by calling the VBE/AF device driver
; bank switching function.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    __VBEAF20_setBank

		push    ebx
		mov     ebx,[DC.AFDC]
		push	edx
		push	ebx
		call	[AF_devCtx.cFuncs.SetBank]
		pop		ebx
		pop		edx
		pop     ebx
		ret

procend     	__VBEAF20_setBank

;----------------------------------------------------------------------------
; void _VBE20_setPalette(int start,int num,VBE_palette *pal,bool waitFlag)
;----------------------------------------------------------------------------
; Sets the palette by directly calling the relocated VBE 2.0 palette
; setting routine in 32 bit protected mode.
;----------------------------------------------------------------------------
procstartdll    __VBE20_setPalette

        ARG     start:UINT, num:UINT, pal:DPTR, waitFlag:S_USHORT

        enter_c 0

        mov     bx,[waitFlag]       ; BL := wait for retrace flag
        mov     _cx,[num]           ; _CX := number of entries
        mov     _dx,[start]         ; _DX := starting palette entry
if flatmodel
        mov     edi,[pal]           ; ES:EDI -> palette data
        mov     eax,[__SV_setPal20]
        push    ds
        mov     ds,[_VBE_MMIOSel]   ; Load selector to MMIO registers
        call    eax                 ; Call the VBE 2.0 palette code
        pop     ds
else
		les     di,[pal]
        movzx   edi,di              ; ES:EDI -> palette data
        push    ds                  ; Save DS
        push    fs                  ; Save FS
        push    ds
        pop     fs                  ; FS = DS
        mov     ds,[_VBE_MMIOSel]   ; Load selector to MMIO registers
        call    [fs:__SV_setPal20]  ; Call the VBE 2.0 palette code
        pop     fs
        pop     ds
endif

        leave_c_nolocal
        ret

procend     __VBE20_setPalette

endif

;----------------------------------------------------------------------------
; _SV_setBankASM    Sets the read/write bank directly via the VBE
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value.
;
; Entry:        AL  - New read/write bank number
;
; Exit:         AL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    _SV_setBankASM

		push    _dx
ife flatmodel
		push    ds
		mov     dx,DGROUP           ; Re-load out data segment
		mov     ds,dx
endif
		mov     dx,ax               ; DX := bank number
		mov     [WORD DC.curBank],ax    ; Save current write bank number
		call    [__SV_setBankPtr]       ; Call card specific bank routine
ife flatmodel
		pop     ds
endif
		pop     _dx
		ret

procend         _SV_setBankASM

;----------------------------------------------------------------------------
; _SV_setBankASMDL    Sets the read/write bank directly via the VBE
;----------------------------------------------------------------------------
; This routine sets both the read and write bank numbers to the same
; value.
;
; Entry:        DL  - New read/write bank number
;
; Exit:         DL  - New read/write bank number
;
; Registers:    All preserved!
;----------------------------------------------------------------------------
procstartdll    _SV_setBankASMDL

		push	_dx
ife flatmodel
		push    ds
		push	ax
		mov     ax,DGROUP           ; Re-load out data segment
		mov     ds,ax
		pop		ax
endif
		mov     [BYTE DC.curBank],dl    ; Save current write bank number
		call    [__SV_setBankPtr]       ; Call card specific bank routine
ife flatmodel
		pop     ds
endif
		pop		_dx
		ret

procend         _SV_setBankASMDL

ifdef   __WINDOWS16__
procstart   SV_SETBANKASM
		jmp     _SV_setBankASM
procend     SV_SETBANKASM
endif

;----------------------------------------------------------------------------
; void SV_setBank(int bank)
;----------------------------------------------------------------------------
; Calls the current bank switching routine to change video banks.
;----------------------------------------------------------------------------
procstartdll16  _SV_setBank

        ARG     bank:UINT

        push    _bp
        mov     _bp,_sp
        mov     _ax,[bank]          ; AX := 64k bank value
        call    _SV_setBankASM
        pop     _bp
        ret

procenddll16    _SV_setBank

ifndef  __WINDOWS16__

procstartdll    _SV_beginPixel
        jmp     [DC.beginPixel]
procend         _SV_beginPixel

procstartdll    _SV_putPixel
        jmp     [DC.putPixel]
procend         _SV_putPixel

procstartdll    _SV_putPixelFast
        jmp     [DC.putPixelFast]
procend         _SV_putPixelFast

procstartdll    _SV_endPixel
        jmp     [DC.endPixel]
procend         _SV_endPixel

procstartdll    _SV_clear
        jmp     [DC.clear]
procend         _SV_clear

procstartdll    _SV_beginLine
        jmp     [DC.beginLine]
procend         _SV_beginLine

procstartdll    _SV_line
        jmp     [DC.line]
procend         _SV_line

procstartdll    _SV_lineFast
        jmp     [DC.lineFast]
procend         _SV_lineFast

procstartdll    _SV_endLine
        jmp     [DC.endLine]
procend         _SV_endLine

procstartdll    _SV_beginDirectAccess
        jmp     [DC.beginDirectAccess]
procend         _SV_beginDirectAccess

procstartdll    _SV_endDirectAccess
        jmp     [DC.endDirectAccess]
procend         _SV_endDirectAccess

endif

;----------------------------------------------------------------------------
; VGA_vSyncPresent	Checks if the vSync pulse is active
;----------------------------------------------------------------------------
; Checks if the vSync pulse is active, but looping around 64000 times
; checking for the sync pulse. If we dont get a sync pulse then we return
; a condition of false (the high level code will make multiple calls to
; this function and time out over a large period of time).
;
; Exit:		_AX	- 1 if vSync present, 0 if not found this time round
;----------------------------------------------------------------------------
procstart    _VGA_vSyncPresent

		push	_bx
		push	_cx
		push	_dx

		mov     dx,03DAh            ; DX := video status port
		mov		_cx,64000			; Loop 64000 times

		in      al,dx               ; Read current status of retrace bit
		and		al,8
		mov		bl,al

@@Wait:	in      al,dx               ; Wait for start of vertical retrace
		and		al,8
		cmp     al,bl
		jne    	@@Found
		loop	@@Wait

		xor		_ax,_ax				; Loop timed out
		jmp		@@Exit

@@Found:mov		_ax,1
@@Exit:	pop		_dx
		pop		_cx
		pop		_bx
		ret

procend      _VGA_vSyncPresent

; Macro to delay briefly to ensure that enough time has elapsed between
; successive I/O accesses so that the device being accessed can respond
; to both accesses even on a very fast PC.

macro	DELAY_TIMER
		jmp		$+2
		jmp		$+2
		jmp		$+2
endm

;----------------------------------------------------------------------------
; DDC_delay	- Delay for a specified number of microseconds
;----------------------------------------------------------------------------
; Delays for the specified number of microseconds, by directly programming
; the 8253 timer chips. This will probably not work correctly in ring 3 code
; if the timer ports are virtualised (such as under Windows, OS/2 and Unix),
; so we may need to call back to a VBE/AF supplied function for these systems
; or make sure the mode set code runs in ring 0 space so we have direct
; access to the timer ports.
;
; Entry:		CX	- Number of microseconds to delay for
; Registers:	All preserved
;----------------------------------------------------------------------------
procstart	_DDC_delay

		ARG		delay:UINT

		enter_c	0
		push	_ax
		push	_cx
		push	_dx
		mov		_cx,[delay]

; Start timer 2 counting

		in      al,61h
		or      al,1
		out     61h,al

; Set the timer 2 count to 0 again to start the timing interval.

		mov		al,10110100b		; set up to load initial (timer 2)
		out		43h,al				; timer count
		DELAY_TIMER
		sub		al,al
		out		42h,al				; load count lsb
		DELAY_TIMER
		out		42h,al				; load count msb

@@LoopStart:
		mov		al,00000000b		; latch timer 0
		out		43h,al
		DELAY_TIMER
		in		al,42h				; least significant byte
		DELAY_TIMER
		mov		ah,al
		in		al,42h				; most significant byte
		xchg	ah,al
		neg		ax					; Convert from countdown remaining
									;  to elapsed count
		cmp     ax,cx				; Has delay expired?
		jl		@@LoopStart			; No, so loop till done

; Stop timer 2 from counting

		in      al,61H
		and     al,not 1
		out     61H,al

		pop		_dx
		pop		_cx
		pop		_ax
		leave_c_nolocal
		ret

procend		_DDC_delay

endcodeseg  _svgasdk

        END
