;****************************************************************************
;*
;*				 VESA BIOS Extensions/Accelerator Functions
;*								Version 1.0
;*
;*                   Copyright (C) 1996 SciTech Software.
;*                           All rights reserved.
;*
;* Filename:     $Workfile:   _vbeaf.asm  $
;* Developed by: SciTech Software
;*
;* Language:     80386 Assembler (TASM ideal mode)
;* Environment:  IBM PC 32 bit Protected Mode.
;*
;* Description:  Assembly language support routines for the Graphics
;*				 Accelerator API. This module provides a small, sample
;*				 C API showing how to call the device context functions
;*				 from assembler. For time critical code the device context
;*				 functions should be called directly from assembly language
;*				 rather than via these C callable functions. However for
;*				 many operations these C function will suffice.
;*
;* $Date:   28 Mar 1996 13:51:04  $ $Author:   KendallB  $
;*
;****************************************************************************

		IDEAL

include "model.mac"             ; Memory model macros
include "vbeaf.inc"				; Structure definitions etc

if flatmodel

header	_vbeaf

		EXTRN	__AF_int86_C:FPTR
		EXTRN	__AF_callRealMode_C:FPTR

begcodeseg	_vbeaf

; Macros to setup and call a generic function that takes a parameter block
; in DS:ESI given the parameters passed on the stack

MACRO   CallGeneric name

		ARG		dc:DPTR, firstParm:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[dc]
		lea		esi,[firstParm]
		call	[AF_devCtx.name]

		pop		esi
		pop		ebx
		pop		ebp
		ret

ENDM

MACRO   CallGeneric1Color name

		ARG		dc:DPTR, color:UINT, firstParm:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[color]
		mov		ebx,[dc]
		lea		esi,[firstParm]
		call	[AF_devCtx.name]

		pop		esi
		pop		ebx
		pop		ebp
		ret

ENDM

MACRO   CallGeneric2Color name

		ARG		dc:DPTR, color:UINT, backColor:UINT, firstParm:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[color]
		mov		ebx,[dc]
		mov		ecx,[backColor]
		lea		esi,[firstParm]
		call	[AF_devCtx.name]

		pop		esi
		pop		ebx
		pop		ebp
		ret

ENDM

;----------------------------------------------------------------------------
; _AF_int86 	Issue a real mode interrupt
;----------------------------------------------------------------------------
; Entry:		BL		- Interrupt number
;               DS:EDI	- Pointer to DPMI register structure
;
; Exit:			DS:EDI	- Pointer to modifed DPMI register structure
;----------------------------------------------------------------------------
procstart	__AF_int86

		movzx	ebx,bl
		push    edi
		push	ebx
		call	__AF_int86_C			; Call C version to handle it
		add		esp,8
		ret

procend		__AF_int86

;----------------------------------------------------------------------------
; _AF_callRealMode 	Issue a real mode interrupt
;----------------------------------------------------------------------------
; Entry:		BL		- Interrupt number
;               DS:EDI	- Pointer to DPMI register structure
;
; Exit:			DS:EDI	- Pointer to modifed DPMI register structure
;----------------------------------------------------------------------------
procstart	__AF_callRealMode

		push    edi
		call	__AF_callRealMode_C		; Call C version to handle it
		add		esp,4
		ret

procend		__AF_callRealMode

;----------------------------------------------------------------------------
; AF_int32 _AF_initDriver(AF_devCtx *dc);
;----------------------------------------------------------------------------
procstartdll	__AF_initDriver

		ARG     dc:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ebx,[dc]
		mov		eax,[AF_devCtx.initDriver]
		add		eax,ebx
		call	eax

		pop		ebx
		pop		ebp
		ret

procend   		__AF_initDriver

;----------------------------------------------------------------------------
; AF_int32 AF_getVideoModeInfo(AF_devCtx *dc,AF_int16 mode,
;	AF_modeInfo *modeInfo)
;----------------------------------------------------------------------------
procstartdll	_AF_getVideoModeInfo

		ARG     dc:DPTR, mode:S_USHORT, modeInfo:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	edi

		movzx	eax,[mode]
		mov		ebx,[dc]
		mov     edi,[modeInfo]
		call	[AF_devCtx.GetVideoModeInfo]

		pop		edi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_getVideoModeInfo

;----------------------------------------------------------------------------
; AF_int32 AF_setVideoMode(AF_devCtx *dc,AF_int16 mode,
;	AF_int32 *bytesPerLine,int numBuffers)
;----------------------------------------------------------------------------
procstartdll	_AF_setVideoMode

		ARG		dc:DPTR, mode:S_USHORT, bytesPerLine:DPTR, numBuffers:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		movzx	eax,[mode]
		mov		ebx,[dc]
		mov		ecx,[bytesPerLine]
		mov		ecx,[ecx]
		mov		edi,[numBuffers]
		call	[AF_devCtx.SetVideoMode]
		mov		edx,[bytesPerLine]
		mov		[edx],ecx

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setVideoMode

;----------------------------------------------------------------------------
; AF_int32 AF_setVirtualVideoMode(AF_devCtx *dc,AF_int16 mode,
;	AF_int32 virtualX,AF_int32 virtualY,AF_int32 *bytesPerLine,int numBuffers)
;----------------------------------------------------------------------------
procstartdll	_AF_setVirtualVideoMode

		ARG		dc:DPTR, mode:S_USHORT, virtualX:UINT, virtualY:UINT,	\
				bytesPerLine:DPTR, numBuffers:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		movzx	eax,[mode]
		mov		ebx,[dc]
		mov		ecx,[bytesPerLine]
		mov		ecx,[ecx]
		mov		edx,[virtualX]
		mov		esi,[virtualY]
		mov		edi,[numBuffers]
		call	[AF_devCtx.SetVideoMode]
		mov		edx,[bytesPerLine]
		mov		[edx],ecx

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_setVirtualVideoMode

;----------------------------------------------------------------------------
; void AF_restoreTextMode(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll	_AF_restoreTextMode

		ARG		dc:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ebx,[dc]
		call	[AF_devCtx.RestoreTextMode]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_restoreTextMode

;----------------------------------------------------------------------------
; void AF_setBank(AF_devCtx *dc,AF_int32 bank)
;----------------------------------------------------------------------------
procstartdll	_AF_setBank

		ARG		dc:DPTR, bank:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ebx,[dc]
		mov		edx,[bank]
		call	[AF_devCtx.SetBank]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setBank

;----------------------------------------------------------------------------
; void AF_waitTillIdle(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll	_AF_waitTillIdle

		ARG		dc:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ebx,[dc]
		call	[AF_devCtx.WaitTillIdle]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_waitTillIdle

;----------------------------------------------------------------------------
; void AF_enableDirectAccess(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll	_AF_enableDirectAccess

		ARG		dc:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ebx,[dc]
		call	[AF_devCtx.EnableDirectAccess]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_enableDirectAccess

;----------------------------------------------------------------------------
; void AF_disableDirectAccess(AF_devCtx *dc)
;----------------------------------------------------------------------------
procstartdll	_AF_disableDirectAccess

		ARG		dc:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ebx,[dc]
		call	[AF_devCtx.DisableDirectAccess]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_disableDirectAccess

;----------------------------------------------------------------------------
; void AF_setDisplayStart(AF_devCtx *dc,AF_int32 x,AF_int32 y,
;	AF_int32 waitVRT)
;----------------------------------------------------------------------------
procstartdll	_AF_setDisplayStart

		ARG		dc:DPTR, x:UINT, y:UINT, waitVRT:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		eax,[waitVRT]
		mov		ebx,[dc]
		mov		ecx,[x]
		mov		edx,[y]
		call	[AF_devCtx.SetDisplayStart]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setDisplayStart

;----------------------------------------------------------------------------
; void AF_setActiveBuffer(AF_devCtx *dc,AF_int32 index)
;----------------------------------------------------------------------------
procstartdll	_AF_setActiveBuffer

		ARG		dc:DPTR, index:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		eax,[index]
		mov		ebx,[dc]
		call	[AF_devCtx.SetActiveBuffer]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setActiveBuffer

;----------------------------------------------------------------------------
; void AF_setVisibleBuffer(AF_devCtx *dc,AF_int32 index,AF_int32 waitVRT)
;----------------------------------------------------------------------------
procstartdll	_AF_setVisibleBuffer

		ARG		dc:DPTR, index:UINT, waitVRT:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		eax,[index]
		mov		ebx,[dc]
		mov		edx,[waitVRT]
		call	[AF_devCtx.SetVisibleBuffer]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setVisibleBuffer

;----------------------------------------------------------------------------
; void AF_setPaletteData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,
;	AF_int32 index,AF_int32 waitVRT)
;----------------------------------------------------------------------------
procstartdll	_AF_setPaletteData

		ARG		dc:DPTR, pal:DPTR, num:UINT, index:UINT, waitVRT:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[waitVRT]
		mov		ebx,[dc]
		mov		ecx,[num]
		mov		edx,[index]
		mov		esi,[pal]
		call	[AF_devCtx.SetPaletteData]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_setPaletteData

;----------------------------------------------------------------------------
; void AF_setGammaCorrectData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,
;	AF_int32 index)
;----------------------------------------------------------------------------
procstartdll	_AF_setGammaCorrectData

		ARG		dc:DPTR, pal:DPTR, num:UINT, index:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[index]
		mov		ebx,[dc]
		mov		ecx,[num]
		mov		esi,[pal]
		call	[AF_devCtx.SetGammaCorrectData]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_setGammaCorrectData

;----------------------------------------------------------------------------
; void AF_setCursor(AF_devCtx *dc,AF_cursor *cursor)
;----------------------------------------------------------------------------
procstartdll	_AF_setCursor

		ARG		dc:DPTR, cursor:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[dc]
		mov		esi,[cursor]
		call	[AF_devCtx.SetCursor]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_setCursor

;----------------------------------------------------------------------------
; void AF_setCursorPos(AF_devCtx *dc,AF_int32 x,AF_int32 y)
;----------------------------------------------------------------------------
procstartdll	_AF_setCursorPos

		ARG		dc:DPTR, x:UINT, y:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		eax,[x]
		mov		ebx,[dc]
		mov		ecx,[y]
		call	[AF_devCtx.SetCursorPos]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setCursorPos

;----------------------------------------------------------------------------
; void AF_setCursorColor(AF_devCtx *dc,AF_uint8 red,AF_uint8 green,
;	AF_uint8 blue)
;----------------------------------------------------------------------------
procstartdll	_AF_setCursorColor

		ARG		dc:DPTR, red:S_UCHAR, green:S_UCHAR, blue:S_UCHAR

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		al,[red]
		mov		ah,[green]
		mov		ebx,[dc]
		mov		cl,[blue]
		call	[AF_devCtx.SetCursorColor]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setCursorColor

;----------------------------------------------------------------------------
; void AF_showCursor(AF_devCtx *dc,AF_int32 visible)
;----------------------------------------------------------------------------
procstartdll	_AF_showCursor

		ARG		dc:DPTR, visible:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		eax,[visible]
		mov		ebx,[dc]
		call	[AF_devCtx.ShowCursor]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_showCursor

;----------------------------------------------------------------------------
; void AF_setMix(AF_devCtx *dc,AF_int32 foreMix,AF_int32 backMix)
;----------------------------------------------------------------------------
procstartdll	_AF_setMix

		ARG		dc:DPTR, foreMix:UINT, backMix:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		eax,[foreMix]
		mov		ebx,[dc]
		mov		ecx,[backMix]
		call	[AF_devCtx.SetMix]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setMix

;----------------------------------------------------------------------------
; void AF_set8x8Pattern(AF_devCtx *dc,AF_pattern *pattern)
;----------------------------------------------------------------------------
procstartdll	_AF_set8x8MonoPattern

		ARG		dc:DPTR, pattern:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[dc]
		mov		esi,[pattern]
		call	[AF_devCtx.Set8x8MonoPattern]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_set8x8MonoPattern

;----------------------------------------------------------------------------
; void AF_setLineStipple(AF_devCtx *dc,AF_stipple stipple)
;----------------------------------------------------------------------------
procstartdll	_AF_setLineStipple

		ARG		dc:DPTR, stipple:S_USHORT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ax,[stipple]
		mov		ebx,[dc]
		call	[AF_devCtx.SetLineStipple]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_setLineStipple

;----------------------------------------------------------------------------
; void AF_setClipRect(AF_devCtx *dc,AF_int32 minx,AF_int32 miny,
;	AF_int32 maxx,AF_int32 maxy)
;----------------------------------------------------------------------------
procstartdll	_AF_setClipRect

		CallGeneric	SetClipRect

procend   		_AF_setClipRect

;----------------------------------------------------------------------------
; void AF_drawScan(AF_devCtx *dc,AF_int32 color,AF_int32 y,AF_int32 x1,
;	AF_int32 x2)
;----------------------------------------------------------------------------
procstartdll	_AF_drawScan

		ARG		dc:DPTR, color:UINT, y:UINT, x1:UINT, x2:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[y]
		mov		ebx,[dc]
		mov		ecx,[x1]
		mov		edx,[x2]
		mov		esi,[color]
		call	[AF_devCtx.DrawScan]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_drawScan

;----------------------------------------------------------------------------
; void AF_drawPattScan(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,
;	AF_int32 y,AF_int32 x1,AF_int32 x2)
;----------------------------------------------------------------------------
procstartdll	_AF_drawPattScan

		ARG		dc:DPTR, color:UINT, backColor:UINT, y:UINT, x1:UINT, x2:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi
		push	edi

		mov		eax,[y]
		mov		ebx,[dc]
		mov		ecx,[x1]
		mov		edx,[x2]
		mov		esi,[color]
		mov		edi,[backColor]
		call	[AF_devCtx.DrawPattScan]

		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

procend   		_AF_drawPattScan

;----------------------------------------------------------------------------
; void AF_drawScanList(AF_devCtx *dc,AF_color color,AF_int32 y,AF_int32 len,
;	AF_int16 *scans)
;----------------------------------------------------------------------------
procstartdll	_AF_drawScanList

		ARG		dc:DPTR, color:UINT, y:UINT, len:UINT, scans:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		eax,[y]
		mov		ebx,[dc]
		mov		ecx,[len]
		mov		esi,[scans]
		mov		edx,[color]
		call	[AF_devCtx.DrawScanList]

		pop		ebx
		pop		ebp
		ret

procend   		_AF_drawScanList

;----------------------------------------------------------------------------
; void AF_drawRect(AF_devCtx *dc,AF_color color,AF_int32 left,AF_int32 top,
;	AF_int32 width,AF_int32 height)
;----------------------------------------------------------------------------
procstartdll	_AF_drawRect

		CallGeneric1Color DrawRect

procend   		_AF_drawRect

;----------------------------------------------------------------------------
; void AF_drawPattRect(AF_devCtx *dc,AF_color foreColor,AF_color backColor,
;	AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height)
;----------------------------------------------------------------------------
procstartdll	_AF_drawPattRect

		CallGeneric2Color DrawPattRect

procend   		_AF_drawPattRect

;----------------------------------------------------------------------------
; void AF_drawLine(AF_devCtx *dc,AF_color color,AF_fix32 x1,AF_fix32 y1,
;	AF_fix32 x2,AF_fix32 y2)
;----------------------------------------------------------------------------
procstartdll	_AF_drawLine

		CallGeneric1Color DrawLine

procend   		_AF_drawLine

;----------------------------------------------------------------------------
; void AF_drawStippleLine(AF_devCtx *dc,AF_color foreColor,AF_color backColor,
;	AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2)
;----------------------------------------------------------------------------
procstartdll	_AF_drawStippleLine

		CallGeneric2Color DrawStippleLine

procend   		_AF_drawStippleLine

;----------------------------------------------------------------------------
; void AF_drawTrap(AF_devCtx *dc,AF_color color,AF_int32 y,AF_int32 count,
;	AF_fix32 x1,AF_fix32 x2,AF_fix32 slope1,AF_fix32 slope2)
;----------------------------------------------------------------------------
procstartdll	_AF_drawTrap

		CallGeneric1Color DrawTrap

procend   		_AF_drawTrap

;----------------------------------------------------------------------------
; void AF_drawTri(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,
;	AF_fxpoint *v3,AF_fix32 xOffset,AF_fix32 yOffset)
;----------------------------------------------------------------------------
procstartdll	_AF_drawTri

		CallGeneric1Color DrawTri

procend   		_AF_drawTri

;----------------------------------------------------------------------------
; void AF_drawQuad(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,
;	AF_fxpoint *v3,AF_fix32 xOffset,AF_fix32 yOffset)
;----------------------------------------------------------------------------
procstartdll	_AF_drawQuad

		CallGeneric1Color DrawQuad

procend   		_AF_drawQuad

;----------------------------------------------------------------------------
; void AF_putMonoImage(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,
;	AF_int32 x,AF_int32 y,AF_int32 byteWidth,AF_int32 height,AF_uint8 *image)
;----------------------------------------------------------------------------
procstartdll	_AF_putMonoImage

		CallGeneric2Color PutMonoImage

procend   		_AF_putMonoImage

;----------------------------------------------------------------------------
; void AF_bitBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,
;	AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op)
;----------------------------------------------------------------------------
procstartdll	_AF_bitBlt

		CallGeneric	BitBlt

procend   		_AF_bitBlt

;----------------------------------------------------------------------------
; void AF_bitBltLin(AF_devCtx *dc,AF_int32 srcOfs,
;	AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,
;	AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op)
;----------------------------------------------------------------------------
procstartdll	_AF_bitBltLin

		CallGeneric	BitBltLin

procend   		_AF_bitBltLin

;----------------------------------------------------------------------------
; void AF_srcTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,
;	AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll	_AF_srcTransBlt

		CallGeneric	SrcTransBlt

procend   		_AF_srcTransBlt

;----------------------------------------------------------------------------
; void AF_dstTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,
;	AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll	_AF_dstTransBlt

		CallGeneric	DstTransBlt

procend   		_AF_dstTransBlt

;----------------------------------------------------------------------------
; void AF_srcTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,
;	AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,
;	AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll	_AF_srcTransBltLin

		CallGeneric	SrcTransBltLin

procend   		_AF_srcTransBltLin

;----------------------------------------------------------------------------
; void AF_dstTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,
;	AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,
;	AF_int32 dstLeft,AF_int32 dstTop,AF_color transparent)
;----------------------------------------------------------------------------
procstartdll	_AF_dstTransBltLin

		CallGeneric	DstTransBltLin

procend   		_AF_dstTransBltLin

endcodeseg	_vbeaf

endif

		END
