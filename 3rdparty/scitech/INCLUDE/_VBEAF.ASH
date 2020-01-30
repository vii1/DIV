;****************************************************************************
;*
;*               VESA BIOS Extensions/Accelerator Functions
;*                              Version 2.0
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
;* Developed by: SciTech Software
;*
;* Language:     80386 Assembler (TASM ideal mode)
;* Environment:  IBM PC 32 bit Protected Mode.
;*
;* Description:  Assembly language support routines for the Graphics
;*               Accelerator API. This module provides a small, sample
;*               C API showing how to call the device context functions
;*               from assembler. For time critical code the device context
;*               functions should be called directly from assembly language
;*               rather than via these C callable functions. However for
;*               many operations these C function will suffice.
;*
;*				 The C callable functions in this module are for backwards
;*				 compatibility with VBE/AF 1.0 drivers, which used an
;*				 assembler interface to call the loaded driver.
;*
;*
;****************************************************************************

		IDEAL

VBEAF_BUILD = 1					; Compile with VBE/AF 1.0 compatibility

include "model.mac"             ; Memory model macros
include "vbeaf.inc"             ; Structure definitions etc

if flatmodel

header  _vbeaf

        EXTRN   __AF_int86_C:FPTR
        EXTRN   __AF_callRealMode_C:FPTR

begdataseg  _vbeaf

gFuncPtr	cptr	?			; Pointer to function to call

enddataseg  _vbeaf

begcodeseg  _vbeaf

; The following defines the registers we pass to the VxD when we call it
; from C code.

struc 	VXD_regs_s
eax						ULONG	?
ebx						ULONG	?
ecx						ULONG	?
edx						ULONG	?
esi						ULONG	?
edi						ULONG	?
ds						USHORT	?
es						USHORT	?
ends    VXD_regs_s

ife flatmodel
VXD_regs = (VXD_regs_s PTR ES:BX)
else
VXD_regs = (VXD_regs_s PTR SS:EBX)
endif

;----------------------------------------------------------------------------
; void _AF_VxDCall(VXD_regs far *r,uint off,uint sel);
;----------------------------------------------------------------------------
procstart	__AF_VxDCall

		ARG     r:DPTR, off:UINT, sel:UINT

		enter_c	0
		push	ds
		push	es

; Load all registers from the registers structure

		_les	_bx,[r]
ife flatmodel
		push	[VXD_regs.ds]			; Push segment registers onto stack
		push	[VXD_regs.es]
endif
		mov		eax,[VXD_regs.eax]
		mov		ecx,[VXD_regs.ecx]
		mov		edx,[VXD_regs.edx]
		mov		esi,[VXD_regs.esi]
		mov		edi,[VXD_regs.edi]
		mov		ebx,[VXD_regs.ebx]		; Trashes BX structure pointer!
ife flatmodel
		pop		es
		pop		ds						; Load segment registers
endif

; Call the VxD entry point (on stack)

		call	[FCPTR ss:off]

; Save all registers back in the structure

		push	ebx						; Push EBX onto stack for later
ife flatmodel
		push	ds						; Push segment registers for later
		push	es
endif
		_les	_bx,[r]
		mov		[VXD_regs.eax],eax
		mov		[VXD_regs.ecx],ecx
		mov		[VXD_regs.edx],edx
		mov		[VXD_regs.esi],esi
		mov		[VXD_regs.edi],edi
ife flatmodel
		pop		[VXD_regs.es]
		pop		[VXD_regs.ds]
endif
		pop		[VXD_regs.ebx]			; Save value of EBX from stack

		pop		es
		pop		ds
		leave_c_nolocal
		ret

procend		__AF_VxDCall

; Macros to setup and call a generic function that takes a parameter block
; in DS:ESI given the parameters passed on the stack

MACRO   CallGeneric name

        ARG     dc:DPTR, firstParm:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     ebx,[dc]
        lea     esi,[firstParm]
        call    [AF_devCtx.name]

        pop     esi
        pop     ebx
        pop     ebp
        ret

ENDM

MACRO   CallGeneric1Color name

        ARG     dc:DPTR, color:UINT, firstParm:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     eax,[color]
        mov     ebx,[dc]
        lea     esi,[firstParm]
        call    [AF_devCtx.name]

        pop     esi
        pop     ebx
        pop     ebp
        ret

ENDM

MACRO   CallGeneric2Color name

        ARG     dc:DPTR, color:UINT, backColor:UINT, firstParm:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     eax,[color]
        mov     ebx,[dc]
        mov     ecx,[backColor]
        lea     esi,[firstParm]
        call    [AF_devCtx.name]

        pop     esi
        pop     ebx
        pop     ebp
        ret

ENDM

;----------------------------------------------------------------------------
; _AF_int86     Issue a real mode interrupt
;----------------------------------------------------------------------------
; Entry:        BL      - Interrupt number
;               DS:EDI  - Pointer to DPMI register structure
;
; Exit:         DS:EDI  - Pointer to modifed DPMI register structure
;----------------------------------------------------------------------------
procstart   __AF_int86

		movzx   ebx,bl
        push    edi
        push    ebx
        call    __AF_int86_C            ; Call C version to handle it
        add     esp,8
        ret

procend     __AF_int86

;----------------------------------------------------------------------------
; _AF_callRealMode  Issue a real mode interrupt
;----------------------------------------------------------------------------
; Entry:        BL      - Interrupt number
;               DS:EDI  - Pointer to DPMI register structure
;
; Exit:         DS:EDI  - Pointer to modifed DPMI register structure
;----------------------------------------------------------------------------
procstart   __AF_callRealMode

		push    edi
        call    __AF_callRealMode_C     ; Call C version to handle it
        add     esp,4
        ret

procend     __AF_callRealMode

;----------------------------------------------------------------------------
; AF_int32 _AF_initDriver(AF_devCtx *dc);
;----------------------------------------------------------------------------
procstartdll    __AF_initDriver

		ARG     dc:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx

		mov     ebx,[dc]
		mov     eax,[AF_devCtx.InitDriver]
		add     eax,ebx
		call    eax

		pop     ebx
		pop     ebp
		ret

procend         __AF_initDriver

;----------------------------------------------------------------------------
; AF_int32 _AF_plugAndPlayInit(AF_devCtx *dc);
;----------------------------------------------------------------------------
procstartdll    __AF_plugAndPlayInit

		ARG     dc:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx

		mov     ebx,[dc]
		mov     eax,[AF_devCtx.PlugAndPlayInit]
		add     eax,ebx
		call    eax

		pop     ebx
		pop     ebp
		ret

procend         __AF_plugAndPlayInit

AF_regs = (AF_DPMI_regs_s PTR DS:EAX)

;----------------------------------------------------------------------------
; void _AF_callFuncASM(AF_DPMI_regs *r,void *funcPtr);
;----------------------------------------------------------------------------
procstart	__AF_callFuncASM

		ARG     r:DPTR, funcPtr:CPTR

		enter_c	0

; Load all registers from the registers structure

		mov		eax,[funcPtr]
		mov		[gfuncPtr],eax
		mov		eax,[r]
		mov		ebx,[AF_regs.ebx]
		mov		ecx,[AF_regs.ecx]
		mov		edx,[AF_regs.edx]
		mov		esi,[AF_regs.esi]
		mov		edi,[AF_regs.edi]
		push	ebp
		mov		ebp,[AF_regs.ebp]		; EBP -> AF_devCtx block
		mov		eax,[AF_regs.eax]		; Trashes EAX structure pointer!

; Call the function

		call	[gfuncPtr]

; Save all registers back in the structure

		pop		ebp
		push	eax						; Push EAX onto stack for later
		mov		eax,[r]
		mov		[AF_regs.ebx],ebx
		mov		[AF_regs.ecx],ecx
		mov		[AF_regs.edx],edx
		mov		[AF_regs.esi],esi
		mov		[AF_regs.edi],edi
		pop		[AF_regs.eax]			; Save value of EAX from stack

		leave_c_nolocal
		ret

procend		__AF_callFuncASM

;----------------------------------------------------------------------------
; AF_int32 AF_getVideoModeInfo(AF_devCtx *dc,AF_int16 mode,
;   AF_modeInfo *modeInfo)
;----------------------------------------------------------------------------
procstartdll    _AF10_getVideoModeInfo

        ARG     dc:DPTR, mode:S_USHORT, modeInfo:DPTR

        push    ebp
        mov     ebp,esp
        push    ebx
        push    edi

        movzx   eax,[mode]
        mov     ebx,[dc]
        mov     edi,[modeInfo]
        call    [AF_devCtx.GetVideoModeInfo]

        pop     edi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_getVideoModeInfo

;----------------------------------------------------------------------------
; AF_int32 AF_setVideoMode(AF_devCtx *dc,AF_int16 mode,
;   AF_int32 virtualX,AF_int32 virtualY,AF_int32 *bytesPerLine,int numBuffers)
;----------------------------------------------------------------------------
procstartdll    _AF10_setVideoMode

		ARG     dc:DPTR, mode:S_USHORT, virtualX:UINT, virtualY:UINT,   \
				bytesPerLine:DPTR, numBuffers:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi
		push    edi

		movzx   eax,[mode]
		mov     ebx,[dc]
		mov     ecx,[bytesPerLine]
		mov     ecx,[ecx]
		mov     edx,[virtualX]
		mov     esi,[virtualY]
		mov     edi,[numBuffers]
		call    [AF_devCtx.SetVideoMode]
		mov     edx,[bytesPerLine]
		mov     [edx],ecx

		pop     edi
		pop     esi
		pop     ebx
		pop     ebp
		ret

procend         _AF10_setVideoMode

;----------------------------------------------------------------------------
; void AF_restoreTextMode(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll    _AF10_restoreTextMode

		ARG     dc:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx

		mov     ebx,[dc]
		call    [AF_devCtx.RestoreTextMode]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_restoreTextMode

;----------------------------------------------------------------------------
; void AF_setBank(AF_devCtx *dc,AF_int32 bank)
;----------------------------------------------------------------------------
procstartdll    _AF10_setBank

        ARG     dc:DPTR, bank:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     ebx,[dc]
        mov     edx,[bank]
        call    [AF_devCtx.SetBank]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_setBank

;----------------------------------------------------------------------------
; void AF_waitTillIdle(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll    _AF10_waitTillIdle

        ARG     dc:DPTR

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     ebx,[dc]
        call    [AF_devCtx.WaitTillIdle]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_waitTillIdle

;----------------------------------------------------------------------------
; void AF_enableDirectAccess(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll    _AF10_enableDirectAccess

        ARG     dc:DPTR

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     ebx,[dc]
        call    [AF_devCtx.EnableDirectAccess]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_enableDirectAccess

;----------------------------------------------------------------------------
; void AF_disableDirectAccess(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll    _AF10_disableDirectAccess

        ARG     dc:DPTR

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     ebx,[dc]
        call    [AF_devCtx.DisableDirectAccess]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_disableDirectAccess

;----------------------------------------------------------------------------
; void AF_setDisplayStart(AF_devCtx *dc,AF_int32 x,AF_int32 y,
;   AF_int32 waitVRT)
;----------------------------------------------------------------------------
procstartdll    _AF10_setDisplayStart

        ARG     dc:DPTR, x:UINT, y:UINT, waitVRT:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     eax,[waitVRT]
        mov     ebx,[dc]
        mov     ecx,[x]
        mov     edx,[y]
        call    [AF_devCtx.SetDisplayStart]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_setDisplayStart

;----------------------------------------------------------------------------
; void AF_setActiveBuffer(AF_devCtx *dc,AF_int32 index)
;----------------------------------------------------------------------------
procstartdll    _AF10_setActiveBuffer

        ARG     dc:DPTR, index:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     eax,[index]
        mov     ebx,[dc]
        call    [AF_devCtx.SetActiveBuffer]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_setActiveBuffer

;----------------------------------------------------------------------------
; void AF_setVisibleBuffer(AF_devCtx *dc,AF_int32 index,AF_int32 waitVRT)
;----------------------------------------------------------------------------
procstartdll    _AF10_setVisibleBuffer

        ARG     dc:DPTR, index:UINT, waitVRT:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     eax,[index]
        mov     ebx,[dc]
        mov     edx,[waitVRT]
        call    [AF_devCtx.SetVisibleBuffer]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_setVisibleBuffer

;----------------------------------------------------------------------------
; void AF_setPaletteData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,
;   AF_int32 index,AF_int32 waitVRT)
;----------------------------------------------------------------------------
procstartdll    _AF10_setPaletteData

        ARG     dc:DPTR, pal:DPTR, num:UINT, index:UINT, waitVRT:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     eax,[waitVRT]
        mov     ebx,[dc]
        mov     ecx,[num]
        mov     edx,[index]
        mov     esi,[pal]
        call    [AF_devCtx.SetPaletteData]

        pop     esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_setPaletteData

;----------------------------------------------------------------------------
; void AF_setGammaCorrectData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,
;   AF_int32 index)
;----------------------------------------------------------------------------
procstartdll    _AF10_setGammaCorrectData

        ARG     dc:DPTR, pal:DPTR, num:UINT, index:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     eax,[index]
        mov     ebx,[dc]
        mov     ecx,[num]
        mov     esi,[pal]
        call    [AF_devCtx.SetGammaCorrectData]

        pop     esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_setGammaCorrectData

;----------------------------------------------------------------------------
; void AF_setCursor(AF_devCtx *dc,AF_cursor *cursor)
;----------------------------------------------------------------------------
procstartdll    _AF10_setCursor

        ARG     dc:DPTR, cursor:DPTR

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     ebx,[dc]
        mov     esi,[cursor]
        call    [AF_devCtx.SetCursor]

        pop     esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_setCursor

;----------------------------------------------------------------------------
; void AF_setCursorPos(AF_devCtx *dc,AF_int32 x,AF_int32 y)
;----------------------------------------------------------------------------
procstartdll    _AF10_setCursorPos

        ARG     dc:DPTR, x:UINT, y:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     eax,[x]
        mov     ebx,[dc]
        mov     ecx,[y]
        call    [AF_devCtx.SetCursorPos]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_setCursorPos

;----------------------------------------------------------------------------
; void AF_setCursorColor(AF_devCtx *dc,AF_uint8 red,AF_uint8 green,
;   AF_uint8 blue)
;----------------------------------------------------------------------------
procstartdll    _AF10_setCursorColor

        ARG     dc:DPTR, red:S_UCHAR, green:S_UCHAR, blue:S_UCHAR

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     al,[red]
        mov     ah,[green]
        mov     ebx,[dc]
        mov     cl,[blue]
        call    [AF_devCtx.SetCursorColor]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_setCursorColor

;----------------------------------------------------------------------------
; void AF_showCursor(AF_devCtx *dc,AF_int32 visible)
;----------------------------------------------------------------------------
procstartdll    _AF10_showCursor

        ARG     dc:DPTR, visible:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     eax,[visible]
        mov     ebx,[dc]
        call    [AF_devCtx.ShowCursor]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_showCursor

;----------------------------------------------------------------------------
; void AF_setMix(AF_devCtx *dc,AF_int32 foreMix,AF_int32 backMix)
;----------------------------------------------------------------------------
procstartdll    _AF10_setMix

        ARG     dc:DPTR, foreMix:UINT, backMix:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     eax,[foreMix]
        mov     ebx,[dc]
        mov     ecx,[backMix]
        call    [AF_devCtx.SetMix]

        pop     ebx
		pop     ebp
		ret

procend         _AF10_setMix

;----------------------------------------------------------------------------
; void AF_set8x8MonoPattern(AF_devCtx *dc,AF_pattern *pattern)
;----------------------------------------------------------------------------
procstartdll    _AF10_set8x8MonoPattern

		ARG     dc:DPTR, pattern:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi

		mov     ebx,[dc]
		mov     esi,[pattern]
		call    [AF_devCtx.Set8x8MonoPattern]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend         _AF10_set8x8MonoPattern

;----------------------------------------------------------------------------
; void AF_set8x8ColorPattern(AF_devCtx *dc,AF_color *pattern)
;----------------------------------------------------------------------------
procstartdll    _AF10_set8x8ColorPattern

        ARG     dc:DPTR, pattern:DPTR

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     ebx,[dc]
        mov     esi,[pattern]
		call    [AF_devCtx.Set8x8ColorPattern]

        pop     esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_set8x8ColorPattern

;----------------------------------------------------------------------------
; void AF_setLineStipple(AF_devCtx *dc,AF_stipple stipple)
;----------------------------------------------------------------------------
procstartdll    _AF10_setLineStipple

        ARG     dc:DPTR, stipple:S_USHORT

        push    ebp
        mov     ebp,esp
        push    ebx

        mov     ax,[stipple]
        mov     ebx,[dc]
        call    [AF_devCtx.SetLineStipple]

        pop     ebx
        pop     ebp
        ret

procend         _AF10_setLineStipple

;----------------------------------------------------------------------------
; void AF_setClipRect(AF_devCtx *dc,AF_int32 minx,AF_int32 miny,
;   AF_int32 maxx,AF_int32 maxy)
;----------------------------------------------------------------------------
procstartdll    _AF10_setClipRect

        CallGeneric SetClipRect

procend         _AF10_setClipRect

;----------------------------------------------------------------------------
; void AF_drawScan(AF_devCtx *dc,AF_int32 color,AF_int32 y,AF_int32 x1,
;   AF_int32 x2)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawScan

        ARG     dc:DPTR, color:UINT, y:UINT, x1:UINT, x2:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     eax,[y]
        mov     ebx,[dc]
        mov     ecx,[x1]
        mov     edx,[x2]
        mov     esi,[color]
        call    [AF_devCtx.DrawScan]

        pop     esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_drawScan

;----------------------------------------------------------------------------
; void AF_drawPattScan(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,
;   AF_int32 y,AF_int32 x1,AF_int32 x2)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawPattScan

        ARG     dc:DPTR, color:UINT, backColor:UINT, y:UINT, x1:UINT, x2:UINT

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi
        push    edi

        mov     eax,[y]
        mov     ebx,[dc]
        mov     ecx,[x1]
        mov     edx,[x2]
        mov     esi,[color]
        mov     edi,[backColor]
        call    [AF_devCtx.DrawPattScan]

        pop     edi
        pop     esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_drawPattScan

;----------------------------------------------------------------------------
; void AF_drawColorPattScan(AF_devCtx *dc,AF_int32 y,AF_int32 x1,AF_int32 x2)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawColorPattScan

		ARG     dc:DPTR, y:UINT, x1:UINT, x2:UINT

        push    ebp
        mov     ebp,esp
        push    ebx

		mov     eax,[y]
		mov     ebx,[dc]
		mov     ecx,[x1]
		mov     edx,[x2]
		call    [AF_devCtx.DrawColorPattScan]

		pop     ebx
        pop     ebp
        ret

procend         _AF10_drawColorPattScan

;----------------------------------------------------------------------------
; void AF_drawScanList(AF_devCtx *dc,AF_color color,AF_int32 y,AF_int32 len,
;   AF_int16 *scans)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawScanList

        ARG     dc:DPTR, color:UINT, y:UINT, len:UINT, scans:DPTR

        push    ebp
        mov     ebp,esp
		push    ebx
		push	esi

        mov     eax,[y]
        mov     ebx,[dc]
        mov     ecx,[len]
        mov     esi,[scans]
        mov     edx,[color]
        call    [AF_devCtx.DrawScanList]

		pop		esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_drawScanList

;----------------------------------------------------------------------------
; void AF_drawColorPattScanList(AF_devCtx *dc,AF_int32 y,AF_int32 len,
;   AF_int16 *scans)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawColorPattScanList

		ARG     dc:DPTR, y:UINT, len:UINT, scans:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push	esi

		mov     eax,[y]
		mov     ebx,[dc]
		mov     ecx,[len]
		mov     esi,[scans]
		call    [AF_devCtx.DrawScanList]

		pop		esi
		pop     ebx
		pop     ebp
		ret

procend         _AF10_drawColorPattScanList

;----------------------------------------------------------------------------
; void AF_drawRect(AF_devCtx *dc,AF_color color,AF_int32 left,AF_int32 top,
;   AF_int32 width,AF_int32 height)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawRect

        CallGeneric1Color DrawRect

procend         _AF10_drawRect

;----------------------------------------------------------------------------
; void AF_drawPattRect(AF_devCtx *dc,AF_color foreColor,AF_color backColor,
;   AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawPattRect

        CallGeneric2Color DrawPattRect

procend         _AF10_drawPattRect

;----------------------------------------------------------------------------
; void AF_drawColorPattRect(AF_devCtx *dc,AF_int32 left,AF_int32 top,
;   AF_int32 width,AF_int32 height)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawColorPattRect

		CallGeneric DrawColorPattRect

procend         _AF10_drawColorPattRect

;----------------------------------------------------------------------------
; void AF_drawLine(AF_devCtx *dc,AF_color color,AF_fix32 x1,AF_fix32 y1,
;   AF_fix32 x2,AF_fix32 y2)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawLine

        CallGeneric1Color DrawLine

procend         _AF10_drawLine

;----------------------------------------------------------------------------
; void AF_drawStippleLine(AF_devCtx *dc,AF_color foreColor,AF_color backColor,
;   AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawStippleLine

        CallGeneric2Color DrawStippleLine

procend         _AF10_drawStippleLine

;----------------------------------------------------------------------------
; void AF_drawTrap(AF_devCtx *dc,AF_color color,AF_trap *trap);
;----------------------------------------------------------------------------
procstartdll    _AF10_drawTrap

		ARG     dc:DPTR, color:UINT, trap:DPTR

        push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

        mov     eax,[color]
        mov     ebx,[dc]
		mov		esi,[trap]
		call    [AF_devCtx.DrawTrap]

		pop     esi
        pop     ebx
        pop     ebp
        ret

procend         _AF10_drawTrap

;----------------------------------------------------------------------------
; void AF_drawTri(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,
;   AF_fxpoint *v3,AF_fix32 xOffset,AF_fix32 yOffset)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawTri

        CallGeneric1Color DrawTri

procend         _AF10_drawTri

;----------------------------------------------------------------------------
; void AF_drawQuad(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,
;   AF_fxpoint *v3,AF_fix32 xOffset,AF_fix32 yOffset)
;----------------------------------------------------------------------------
procstartdll    _AF10_drawQuad

        CallGeneric1Color DrawQuad

procend         _AF10_drawQuad

;----------------------------------------------------------------------------
; void AF10_putMonoImage(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,
;   AF_int32 x,AF_int32 y,AF_int32 byteWidth,AF_int32 height,AF_uint8 *image)
;----------------------------------------------------------------------------
procstartdll    _AF10_putMonoImage

		CallGeneric2Color PutMonoImage

procend         _AF10_putMonoImage

;----------------------------------------------------------------------------
; void AF_bitBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,
;   AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op)
;----------------------------------------------------------------------------
procstartdll    _AF10_bitBlt

        CallGeneric BitBlt

procend         _AF10_bitBlt

;----------------------------------------------------------------------------
; void AF_bitBltLin(AF_devCtx *dc,AF_int32 srcOfs,
;   AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,
;   AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op)
;----------------------------------------------------------------------------
procstartdll    _AF10_bitBltLin

        CallGeneric BitBltLin

procend         _AF10_bitBltLin

;----------------------------------------------------------------------------
; void AF_srcTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,
;   AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll    _AF10_srcTransBlt

        CallGeneric SrcTransBlt

procend         _AF10_srcTransBlt

;----------------------------------------------------------------------------
; void AF_dstTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,
;   AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll    _AF10_dstTransBlt

        CallGeneric DstTransBlt

procend         _AF10_dstTransBlt

;----------------------------------------------------------------------------
; void AF_srcTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,
;   AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,
;   AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll    _AF10_srcTransBltLin

        CallGeneric SrcTransBltLin

procend         _AF10_srcTransBltLin

;----------------------------------------------------------------------------
; void AF_dstTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,
;   AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,
;   AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll    _AF10_dstTransBltLin

        CallGeneric DstTransBltLin

procend         _AF10_dstTransBltLin

endcodeseg  _vbeaf

endif

		END
