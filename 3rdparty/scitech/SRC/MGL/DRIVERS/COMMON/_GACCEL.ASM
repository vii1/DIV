;****************************************************************************
;*
;*						MegaGraph Graphics Library
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
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	Generic accelerated rendering routines, that call upon
;*				the VBE/AF driver to perform the acceleration.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"
include "vbeaf.inc"

if	flatmodel

header	_gaccel

struc	ACCEL_state_s
v				display_vec_s	<>
oldBIOSMode		ushort			?
old43Lines		bool			?
fastDrawing		bool			?
ends	ACCEL_state_s
state	= (ACCEL_state_s PTR EBX)

	EXTRN	__MGL_setRenderingVectors:FPTR

begdataseg	_gaccel

	$EXTRN  __MGL_afPtr,DPTR
	$EXTRN	__MGL_dc,devCtx_s
	$EXTRN	__MGL_surfLock,UINT

AF	EQU		AF_devCtx

enddataseg	_gaccel

begcodeseg	_gaccel

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; void ACCEL_setWriteMode(int mode)
;----------------------------------------------------------------------------
; Set the current write mode operation for the accelerated device. We
; also set the current hardware mix mode at the same time to avoid doing
; this all the time during the rendering operations. Note that we update
; the background mix differently depending on the current fill style.
;----------------------------------------------------------------------------
procstart	_ACCEL_setWriteMode

		ARG		mode:UINT

		enter_c	0

		mov		ebx,[__MGL_afPtr]	; EBX -> AF_devCtx
		mov		edx,[GDC.a.penStyle]
		mov		eax,[mode]
		mov		[GDC.a.writeMode],eax
		mov		ecx,eax
		cmp		edx,MGL_BITMAP_OPAQUE
		je		@@1
		mov		ecx,AF_NOP_MIX		; NOP mix for all but opaque backgrounds
@@1:	call	[AF.SetMix]			; Set the current mix mode
		call	__MGL_setRenderingVectors

		leave_c_nolocal
		ret

procend		_ACCEL_setWriteMode

;----------------------------------------------------------------------------
; void ACCEL_setPenStyle(int style)
;----------------------------------------------------------------------------
; Set the current pen style for the accelerated device. We also need to
; update the current mix mode operations to correctly ensure that the
; mixes are set up properly for transparent and opaque mix modes.
;----------------------------------------------------------------------------
procstart	_ACCEL_setPenStyle

		ARG		style:UINT

		enter_c	0

		mov		edx,[style]
		mov		[GDC.a.penStyle],edx
		mov		ebx,[__MGL_afPtr]	; EBX -> AF_devCtx
		mov		eax,[GDC.a.writeMode]
		mov		ecx,eax
		cmp		edx,MGL_BITMAP_OPAQUE
		je		@@1
		mov		ecx,AF_NOP_MIX		; NOP mix for all but opaque backgrounds
@@1:	call	[AF.SetMix]			; Set the current mix mode
		call	__MGL_setRenderingVectors

		leave_c_nolocal
		ret

procend		_ACCEL_setPenStyle

;----------------------------------------------------------------------------
; void ACCEL_setPenBitmapPattern(const pattern_t *pat)
;----------------------------------------------------------------------------
; Set the current pen bitmap pattern. We cache the value in the current
; device context as well as set it in the accelerated driver code.
;----------------------------------------------------------------------------
procstart	_ACCEL_setPenBitmapPattern

		ARG		pat:DPTR

		enter_c	0

		mov		ebx,[__MGL_afPtr]	; EBX -> AF_devCtx
		mov		esi,[pat]
		mov     eax,[esi]
		mov		[DWORD GDC.a.penPat],eax
		mov     eax,[esi+4]
		mov		[DWORD (GDC.a.penPat)+4],eax
		cmp		[AF.Set8x8MonoPattern],0
		je		@@Exit
		call	[AF.Set8x8MonoPattern]

@@Exit:	leave_c_nolocal
		ret

procend		_ACCEL_setPenBitmapPattern

;----------------------------------------------------------------------------
; void ACCEL_setPenPixmapPattern(const pixpattern_t *pat)
;----------------------------------------------------------------------------
; Set the current pen pixmap pattern. We cache the value in the current
; device context as well as set it in the accelerated driver code.
;----------------------------------------------------------------------------
procstart	_ACCEL_setPenPixmapPattern

		ARG		pat:DPTR

		enter_c	0

		mov		ebx,[__MGL_afPtr]	; EBX -> AF_devCtx
		mov		esi,[pat]
		lea		edi,[GDC.a.penPixPat]
		mov		ecx,64
	rep	movsd						; Copy pattern into our pattern cache
		cmp		[AF.Set8x8ColorPattern],0
		je		@@Exit
		call	[AF.Set8x8ColorPattern]

@@Exit:	leave_c_nolocal
		ret

procend		_ACCEL_setPenPixmapPattern

;----------------------------------------------------------------------------
; void ACCEL_setLineStipple(ushort stipple)
;----------------------------------------------------------------------------
procstart	_ACCEL_setLineStipple

		ARG     stipple:S_USHORT

		push    ebp
		mov     ebp,esp
		push    ebx

		movzx   eax,[stipple]
		mov		[GDC.a.lineStipple],eax
		mov     ebx,[__MGL_afPtr]
		call    [AF_devCtx.SetLineStipple]

        pop     ebx
        pop     ebp
        ret

procend		_ACCEL_setLineStipple

;----------------------------------------------------------------------------
; void ACCEL_setLineStippleCount(int count)
;----------------------------------------------------------------------------
procstart	_ACCEL_setLineStippleCount

; TODO: Implement this when the VBE/AF 2.0 driver spec is complete as we
;		need a 2.0 drive to be able to download the stipple and set the
;		count individually.

		ret

procend		_ACCEL_setLineStippleCount

endif	; !MGL_LITE

;----------------------------------------------------------------------------
; void ACCEL_beginDirectAccess(void)
;----------------------------------------------------------------------------
procstart	_ACCEL_beginDirectAccess

		push	ebx
		mov		ebx,[__MGL_afPtr]	; EBX -> AF_devCtx
		call	[AF.WaitTillIdle]	; Wait till engine idle
		pop		ebx
		ret

procend		_ACCEL_beginDirectAccess

;----------------------------------------------------------------------------
; void ACCEL_endDirectAccess(void)
;----------------------------------------------------------------------------
procstart	_ACCEL_endDirectAccess
		ret
procend		_ACCEL_endDirectAccess

;----------------------------------------------------------------------------
; void ACCEL_beginDirectAccessSLOW(void)
;----------------------------------------------------------------------------
procstart	_ACCEL_beginDirectAccessSLOW

		push	ebx
		mov		ebx,[__MGL_afPtr]
		inc		[__MGL_surfLock]
		jnz		@@1
		call	[AF.EnableDirectAccess]
@@1:	pop		ebx
		ret

procend		_ACCEL_beginDirectAccessSLOW

;----------------------------------------------------------------------------
; void ACCEL_endDirectAccessSLOW(void)
;----------------------------------------------------------------------------
procstart	_ACCEL_endDirectAccessSLOW

		push	ebx
		mov		ebx,[__MGL_afPtr]
		dec		[__MGL_surfLock]
		jns		@@2
		call	[AF.DisableDirectAccess]
@@2:	pop		ebx
		ret

procend		_ACCEL_endDirectAccessSLOW

;----------------------------------------------------------------------------
; void ACCEL_beginPixel(MGLDC *dc)
;----------------------------------------------------------------------------
procstart	_ACCEL_beginPixel

		push	ebx
		mov		ebx,[__MGL_afPtr]	; EBX -> AF_devCtx
		call	[AF.WaitTillIdle]	; Wait till engine idle
		pop		ebx
		ret

procend		_ACCEL_beginPixel

;----------------------------------------------------------------------------
; void ACCEL_endPixel(MGLDC *dc)
;----------------------------------------------------------------------------
procstart	_ACCEL_endPixel
		ret
procend		_ACCEL_endPixel

;----------------------------------------------------------------------------
; void ACCEL_beginPixelSLOW(MGLDC *dc)
;----------------------------------------------------------------------------
procstart	_ACCEL_beginPixelSLOW

		push	ebx
		mov		ebx,[__MGL_afPtr]
		inc		[__MGL_surfLock]
		jnz		@@1
		call	[AF.EnableDirectAccess]
@@1:	pop		ebx
		ret

procend		_ACCEL_beginPixelSLOW

;----------------------------------------------------------------------------
; void ACCEL_endPixelSLOW(MGLDC *dc)
;----------------------------------------------------------------------------
procstart	_ACCEL_endPixelSLOW

		push	ebx
		mov		ebx,[__MGL_afPtr]
		dec		[__MGL_surfLock]
		jns		@@2
		call	[AF.DisableDirectAccess]
@@2:	pop		ebx
		ret

procend		_ACCEL_endPixelSLOW

;----------------------------------------------------------------------------
; void _ACCEL_scanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
procstart	_ACCEL_scanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

		mov		eax,[y]
		mov		ebx,[__MGL_afPtr]
		mov		ecx,[x1]
		mov		edx,[x2]
		mov		esi,[GDC.intColor]	; Load the current foreground color
		call	[AF.DrawScan]

		pop     esi
        pop     ebx
		pop     ebp
		ret

procend		_ACCEL_scanLine

ifndef	MGL_LITE
;----------------------------------------------------------------------------
; void _ACCEL_OFF_scanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
; Routine draws a scan line for 8 bit packed pixel deviecs. This
; routine will work for all types of scanlines, patterned, XOR mode etc.
;
; Note: This routine will reverse the X coordinates for each scanline if
;		necessary to make x1 < x1, and will scan convert all pixels
;		in the range [x1,x2), so x2 will not be scan converted.
;		We also ignore any scanline where xstart == xend.
;
; Entry:		y		- y coordinate of scan line to draw
;				x1		- X1 coordinate of scan line to draw
;				x2		- X2 coordinate of scan line to draw
;
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_scanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		push    ebp
        mov     ebp,esp
        push    ebx
        push    esi

		mov		eax,[y]
		add		eax,[AF.OffscreenStartY]
		mov		ebx,[__MGL_afPtr]
		mov		ecx,[x1]
		mov		edx,[x2]
		mov		esi,[GDC.intColor]	; Load the current foreground color
		call	[AF.DrawScan]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_OFF_scanLine

;----------------------------------------------------------------------------
; void _ACCEL_pattScanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
procstart	_ACCEL_pattScanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi
		push    edi

		mov     eax,[y]
		mov		ebx,[__MGL_afPtr]
		mov     ecx,[x1]
		mov     edx,[x2]
		mov		esi,[GDC.intColor]
		mov		edi,[GDC.intBackColor]
		call	[AF.DrawPattScan]

		pop     edi
		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_pattScanLine

;----------------------------------------------------------------------------
; void _ACCEL_OFF_pattScanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_pattScanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi
		push    edi

		mov     eax,[y]
		add		eax,[AF.OffscreenStartY]
		mov		ebx,[__MGL_afPtr]
		mov     ecx,[x1]
		mov     edx,[x2]
		mov		esi,[GDC.intColor]
		mov		edi,[GDC.intBackColor]
		call	[AF.DrawPattScan]

		pop     edi
		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_OFF_pattScanLine

;----------------------------------------------------------------------------
; void _ACCEL_colorPattScanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
procstart	_ACCEL_colorPattScanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		push    ebp
        mov     ebp,esp
        push    ebx

		mov     eax,[y]
		mov		ebx,[__MGL_afPtr]
		mov     ecx,[x1]
		mov     edx,[x2]
		call	[AF.DrawColorPattScan]

		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_colorPattScanLine

;----------------------------------------------------------------------------
; void _ACCEL_OFF_colorPattScanLine(int y,int x1,int x2)
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_colorPattScanLine

		ARG		y:UINT, x1:UINT, x2:UINT

		push    ebp
        mov     ebp,esp
        push    ebx

		mov     eax,[y]
		add		eax,[AF.OffscreenStartY]
		mov		ebx,[__MGL_afPtr]
		mov     ecx,[x1]
		mov     edx,[x2]
		call	[AF.DrawColorPattScan]

		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_OFF_colorPattScanLine

endif	; !MGL_LITE

;----------------------------------------------------------------------------
; void ACCEL_fillRect(int left,int top,int right,int bottom)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in accelerated modes.
;
; Entry:		left	- X1 coordinate of rectangle to draw
;				top		- Y1 coordinate of rectangle to draw
;				right	- X2 coordinate of scan line to draw
;				bottom	- Y2 coordinate of rectangle to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_fillRect

		ARG		left:UINT, top:UINT, right:UINT, bottom:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi

		mov		eax,[left]
		sub		[right],eax				; Convert to width
		mov		eax,[top]
		sub		[bottom],eax			; Convert to height
		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		lea		esi,[left]
		call	[AF.DrawRect]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_fillRect

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; void ACCEL_OFF_fillRect(int left,int top,int right,int bottom)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in accelerated modes.
;
; Entry:		left	- X1 coordinate of rectangle to draw
;				top		- Y1 coordinate of rectangle to draw
;				right	- X2 coordinate of scan line to draw
;				bottom	- Y2 coordinate of rectangle to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_fillRect

		ARG		left:UINT, top:UINT, right:UINT, bottom:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi

		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		add		eax,[AF.OffscreenStartY]
		mov		[top],eax
		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		lea		esi,[left]
		call	[AF.DrawRect]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_OFF_fillRect

;----------------------------------------------------------------------------
; void ACCEL_fillPattRect(int left,int top,int right,int bottom)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in accelerated modes.
;
; Entry:		left	- X1 coordinate of rectangle to draw
;				top		- Y1 coordinate of rectangle to draw
;				right	- X2 coordinate of scan line to draw
;				bottom	- Y2 coordinate of rectangle to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_fillPattRect

		ARG		left:UINT, top:UINT, right:UINT, bottom:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi

		mov		eax,[left]
		sub		[right],eax				; Convert to width
		mov		eax,[top]
		sub		[bottom],eax			; Convert to height
		mov		eax,[GDC.intColor]
		mov		ecx,[GDC.intBackColor]
		mov		ebx,[__MGL_afPtr]
		lea		esi,[left]
		call	[AF.DrawPattRect]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_fillPattRect

;----------------------------------------------------------------------------
; void ACCEL_OFF_fillPattRect(int left,int top,int right,int bottom)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in accelerated modes.
;
; Entry:		left	- X1 coordinate of rectangle to draw
;				top		- Y1 coordinate of rectangle to draw
;				right	- X2 coordinate of scan line to draw
;				bottom	- Y2 coordinate of rectangle to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_fillPattRect

		ARG		left:UINT, top:UINT, right:UINT, bottom:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi

		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		add		eax,[AF.OffscreenStartY]
		mov		[top],eax
		mov		eax,[GDC.intColor]
		mov		ecx,[GDC.intBackColor]
		mov		ebx,[__MGL_afPtr]
		lea		esi,[left]
		call	[AF.DrawPattRect]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_OFF_fillPattRect

;----------------------------------------------------------------------------
; void ACCEL_fillColorPattRect(int left,int top,int right,int bottom)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in accelerated modes.
;
; Entry:		left	- X1 coordinate of rectangle to draw
;				top		- Y1 coordinate of rectangle to draw
;				right	- X2 coordinate of scan line to draw
;				bottom	- Y2 coordinate of rectangle to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_fillColorPattRect

		ARG		left:UINT, top:UINT, right:UINT, bottom:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi

		mov		eax,[left]
		sub		[right],eax				; Convert to width
		mov		eax,[top]
		sub		[bottom],eax			; Convert to height
		mov		ebx,[__MGL_afPtr]
		lea		esi,[left]
		call	[AF.DrawColorPattRect]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_fillColorPattRect

;----------------------------------------------------------------------------
; void ACCEL_OFF_fillColorPattRect(int left,int top,int right,int bottom)
;----------------------------------------------------------------------------
; Routine draws a filled rectangle in accelerated modes.
;
; Entry:		left	- X1 coordinate of rectangle to draw
;				top		- Y1 coordinate of rectangle to draw
;				right	- X2 coordinate of scan line to draw
;				bottom	- Y2 coordinate of rectangle to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_fillColorPattRect

		ARG		left:UINT, top:UINT, right:UINT, bottom:UINT

		push    ebp
		mov     ebp,esp
		push    ebx
		push    esi

		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		add		eax,[AF.OffscreenStartY]
		mov		[top],eax
		mov		ebx,[__MGL_afPtr]
		lea		esi,[left]
		call	[AF.DrawPattRect]

		pop     esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_OFF_fillColorPattRect

endif	; !MGL_LITE

;----------------------------------------------------------------------------
; void _ACCEL_putMonoImage(MGLDC *dc,int x,int y,int width,int height,
;	uchar *image)
;----------------------------------------------------------------------------
; Routine to blit a monochromatic image from system memory into display
; memory, setting all 1 bits to the foreground color and leaving all
; zero bits the same. The image is drawn on the currently active display
; page, and will be clipped to the current clipping rectangle.
;
; Entry:		dc		- Device context
;				x		- X coordinate to place image at
;				y		- Y coordinate to place image at
;				width	- Width of the image in bytes
;				height	- Height of the image
;				image	- Pointer to the image data
;----------------------------------------------------------------------------
procstart	_ACCEL_putMonoImage

		ARG		dc:DPTR, x:UINT, y:UINT, byteWidth:UINT, height:UINT,		\
				image:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[GDC.intColor]
		lea     esi,[x]
		call	[AF.PutMonoImage]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_putMonoImage

ifndef	MGL_LITE
;----------------------------------------------------------------------------
; void _ACCEL_OFF_putMonoImage(MGLDC *dc,int x,int y,int width,int height,
;	uchar *image)
;----------------------------------------------------------------------------
; Routine to blit a monochromatic image from system memory into display
; memory, setting all 1 bits to the foreground color and leaving all
; zero bits the same. The image is drawn on the currently active display
; page, and will be clipped to the current clipping rectangle.
;
; Entry:		dc		- Device context
;				x		- X coordinate to place image at
;				y		- Y coordinate to place image at
;				width	- Width of the image in bytes
;				height	- Height of the image
;				image	- Pointer to the image data
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_putMonoImage

		ARG		dc:DPTR, x:UINT, y:UINT, byteWidth:UINT, height:UINT,		\
				image:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[GDC.intColor]
		mov		esi,[AF.OffscreenStartY]
		add		[y],esi
		lea     esi,[x]
		call	[AF.PutMonoImage]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_OFF_putMonoImage
endif

;----------------------------------------------------------------------------
; void _ACCEL_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a solid, single pixel wide line on accelerated devices.
; Assumes that the hardware has already been set up with the correct write
; mode operation.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_ACCEL_line

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		lea     esi,[x1]
		call	[AF.DrawLine]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_line

ifndef	MGL_LITE
;----------------------------------------------------------------------------
; void _ACCEL_OFF_line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a solid, single pixel wide line on accelerated devices.
; Assumes that the hardware has already been set up with the correct write
; mode operation.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_line

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		mov		esi,[AF.OffscreenStartY]
		shl		esi,16
		add		[y1],esi
		add		[y2],esi
		lea     esi,[x1]
		call	[AF.DrawLine]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_OFF_line

;----------------------------------------------------------------------------
; void _ACCEL_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a solid, single pixel wide stippled line on accelerated
; devices. Assumes that the hardware has already been set up with the
; correct write mode operation, stipple and stipple starting count.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_ACCEL_stippleLine

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		lea     esi,[x1]
		call	[AF.DrawStippleLine]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_stippleLine

;----------------------------------------------------------------------------
; void _ACCEL_OFF_stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2)
;----------------------------------------------------------------------------
; Routine draws a solid, single pixel wide stippled line on accelerated
; devices. Assumes that the hardware has already been set up with the
; correct write mode operation, stipple and stipple starting count.
;
; Entry:		x1		- X1 coordinate of line to draw (fixed point)
;				y1		- Y1 coordinate of line to draw (fixed point)
;				x2		- X2 coordinate of line to draw (fixed point)
;				y2		- Y2 coordinate of line to draw (fixed point)
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_stippleLine

		ARG		x1:FIX32_T, y1:FIX32_T, x2:FIX32_T, y2:FIX32_T

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		mov		esi,[AF.OffscreenStartY]
		shl		esi,16
		add		[y1],esi
		add		[y2],esi
		lea     esi,[x1]
		call	[AF.DrawStippleLine]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_OFF_stippleLine

;----------------------------------------------------------------------------
; void ACCEL_drawScanList(int y,int length,short *scans)
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call dc->scanLine to draw each scanline for us.
;
; Entry:	y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_drawScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push	esi

		mov		eax,[y]
		mov		ebx,[__MGL_afPtr]
		mov		ecx,[len]
		mov		esi,[scans]
		mov		edx,[GDC.intColor]	; Load the current foreground color
		call	[AF.DrawScanList]

		pop		esi
		pop     ebx
        pop     ebp
        ret

procend		_ACCEL_drawScanList

;----------------------------------------------------------------------------
; void ACCEL_OFF_drawScanList(int y,int length,short *scans)
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call dc->scanLine to draw each scanline for us.
;
; Entry:	y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_drawScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push	esi

		mov		eax,[y]
		mov		ebx,[__MGL_afPtr]
		add		eax,[AF.OffscreenStartY]
		mov		ecx,[len]
		mov		esi,[scans]
		mov		edx,[GDC.intColor]	; Load the current foreground color
		call	[AF.DrawScanList]

		pop		esi
		pop     ebx
        pop     ebp
        ret

procend		_ACCEL_OFF_drawScanList

;----------------------------------------------------------------------------
; void ACCEL_drawPattScanList(int y,int length,short *scans)
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call dc->scanLine to draw each scanline for us.
;
; Entry:	y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_drawPattScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push	esi
		push	edi

		mov		eax,[y]
		mov		ebx,[__MGL_afPtr]
		mov		ecx,[len]
		mov		esi,[scans]
		mov		edx,[GDC.intColor]
		mov		edi,[GDC.intBackColor]
		call	[AF.DrawPattScanList]

		pop		edi
		pop		esi
		pop     ebx
        pop     ebp
        ret

procend		_ACCEL_drawPattScanList

;----------------------------------------------------------------------------
; void ACCEL_OFF_drawPattScanList(int y,int length,short *scans)
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call dc->scanLine to draw each scanline for us.
;
; Entry:	y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_drawPattScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push	esi
		push	edi

		mov		eax,[y]
		mov		ebx,[__MGL_afPtr]
		add		eax,[AF.OffscreenStartY]
		mov		ecx,[len]
		mov		esi,[scans]
		mov		edx,[GDC.intColor]
		mov		edi,[GDC.intBackColor]
		call	[AF.DrawPattScanList]

		pop		edi
		pop		esi
		pop     ebx
        pop     ebp
        ret

procend		_ACCEL_OFF_drawPattScanList

;----------------------------------------------------------------------------
; void ACCEL_drawColorPattScanList(int y,int length,short *scans)
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call dc->scanLine to draw each scanline for us.
;
; Entry:	y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_drawColorPattScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push	esi
		push	edi

		mov		eax,[y]
		mov		ebx,[__MGL_afPtr]
		mov		ecx,[len]
		mov		esi,[scans]
		call	[AF.DrawColorPattScanList]

		pop		edi
		pop		esi
		pop     ebx
        pop     ebp
        ret

procend		_ACCEL_drawColorPattScanList

;----------------------------------------------------------------------------
; void ACCEL_OFF_drawColorPattScanList(int y,int length,short *scans)
;----------------------------------------------------------------------------
; Draws all the pixels in the list of horizontal lines passed. This
; routine will scan convert any style of scanline list. If we are
; in replace mode with a solid pattern, it can be fully optimised for speed.
; Otherwise we simply call dc->scanLine to draw each scanline for us.
;
; Entry:	y		- Top coordinate for first scanline
;			length	- Number of scanlines in list
;			scan	- Array of scanlines to draw
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_drawColorPattScanList

		ARG		y:UINT, len:UINT, scans:DPTR

		push    ebp
		mov     ebp,esp
		push    ebx
		push	esi
		push	edi

		mov		eax,[y]
		mov		ebx,[__MGL_afPtr]
		add		eax,[AF.OffscreenStartY]
		mov		ecx,[len]
		mov		esi,[scans]
		call	[AF.DrawColorPattScanList]

		pop		edi
		pop		esi
		pop     ebx
		pop     ebp
		ret

procend		_ACCEL_OFF_drawColorPattScanList

;----------------------------------------------------------------------------
; void _ACCEL_trap(void)
;----------------------------------------------------------------------------
; Routine draws a flat shaded trapezoid. Assumes that the current mix
; mode has already been set up for the hardware.
;----------------------------------------------------------------------------
procstart	_ACCEL_trap

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov     eax,[GDC.intColor]
		lea		esi,[GDC.tr]
		call	[AF.DrawTrap]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_trap

;----------------------------------------------------------------------------
; void _ACCEL_OFF_trap(void)
;----------------------------------------------------------------------------
; Routine draws a flat shaded trapezoid. Assumes that the current mix
; mode has already been set up for the hardware.
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_trap

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi
		push	edi

		mov		ebx,[__MGL_afPtr]
		mov     eax,[GDC.intColor]
		mov     edi,[AF.OffscreenStartY]
		add		[GDC.tr.y],edi
		lea		esi,[GDC.tr]
		call	[AF.DrawTrap]
		sub		[GDC.tr.y],edi

		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_OFF_trap

;----------------------------------------------------------------------------
; void _ACCEL_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,
;	fix32_t yOffset)
;----------------------------------------------------------------------------
; Routine draws a solid, flat shaded triangle on accelerated devices.
; Assumes that the hardware has already been set up with the correct write
; mode operation.
;----------------------------------------------------------------------------
procstart	_ACCEL_tri

		ARG     v1:DPTR, v2:DPTR, v3:DPTR, xOffset:UINT, yOffset:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		lea     esi,[v1]
		call	[AF.DrawTri]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_tri

;----------------------------------------------------------------------------
; void _ACCEL_OFF_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
;   fix32_t xOffset,fix32_t yOffset)
;----------------------------------------------------------------------------
; Routine draws a solid, flat shaded triangle on accelerated devices.
; Assumes that the hardware has already been set up with the correct write
; mode operation.
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_tri

		ARG     v1:DPTR, v2:DPTR, v3:DPTR, xOffset:UINT, yOffset:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		mov		esi,[AF.OffscreenStartY]
		shl		esi,16
		add		[yOffset],esi
		lea     esi,[v1]
		call	[AF.DrawTri]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_OFF_tri

;----------------------------------------------------------------------------
; void _ACCEL_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,
;	fix32_t yOffset)
;----------------------------------------------------------------------------
; Routine draws a solid, flat shaded quad on accelerated devices.
; Assumes that the hardware has already been set up with the correct write
; mode operation.
;----------------------------------------------------------------------------
procstart	_ACCEL_quad

		ARG     v1:DPTR, v2:DPTR, v3:DPTR, v4:DPTR, xOffset:UINT, yOffset:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		lea     esi,[v1]
		call	[AF.DrawQuad]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_quad

;----------------------------------------------------------------------------
; void _ACCEL_OFF_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,
;	fix32_t yOffset)
;----------------------------------------------------------------------------
; Routine draws a solid, flat shaded quad on accelerated devices.
; Assumes that the hardware has already been set up with the correct write
; mode operation.
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_quad

		ARG     v1:DPTR, v2:DPTR, v3:DPTR, v4:DPTR, xOffset:UINT, yOffset:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		eax,[GDC.intColor]
		mov		ebx,[__MGL_afPtr]
		mov		esi,[AF.OffscreenStartY]
		shl		esi,16
		add		[yOffset],esi
		lea     esi,[v1]
		call	[AF.DrawQuad]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_OFF_quad

;----------------------------------------------------------------------------
; void ACCEL_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,
;	int dstLeft,int dstTop,int op,int incx,int incy)
;----------------------------------------------------------------------------
; Routine to perform a screen to screen bitBlt operation for an accelerated
; device.
;----------------------------------------------------------------------------
procstart	_ACCEL_bitBlt

		ARG     dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		lea     esi,[left]
		call	[AF.BitBlt]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_bitBlt

;----------------------------------------------------------------------------
; void ACCEL_OFF_bitBlt(MGLDC *dc,int left,int top,int right,int bottom,
;	int dstLeft,int dstTop,int op,int incx,int incy)
;----------------------------------------------------------------------------
; Routine to perform a screen to screen bitBlt operation for an accelerated
; device.
;----------------------------------------------------------------------------
procstart	_ACCEL_OFF_bitBlt

		ARG     dc:DPTR, left:UINT, top:UINT, right:UINT, bottom:UINT,	\
				dstLeft:UINT, dstTop:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		mov		esi,[AF.OffscreenStartY]
		add		eax,esi
		add		[dstTop],esi
		mov		[top],eax
		lea     esi,[left]
		call	[AF.BitBlt]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_OFF_bitBlt

;----------------------------------------------------------------------------
; void ACCEL_bitBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op);
;----------------------------------------------------------------------------
; Routine to perform a screen to screen bitBlt operation for an accelerated
; device between the offscreen buffer and the display DC.
;----------------------------------------------------------------------------
procstart	_ACCEL_bitBltOff

		ARG     dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT,		\
				bottom:UINT, dstLeft:UINT, dstTop:UINT, op:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		add		eax,[AF.OffscreenStartY]
		mov		[top],eax
		lea     esi,[left]
		call	[AF.BitBlt]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_bitBltOff

;----------------------------------------------------------------------------
; void ACCEL_bitBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,
; 	int dstLeft,int dstTop,int dstRight,int dstBottom,int op);
;----------------------------------------------------------------------------
; Routine to perform a screen to screen bitBlt operation for an accelerated
; device between a linear offscreen buffer and the display DC.
;----------------------------------------------------------------------------
procstart	_ACCEL_bitBltLin

		ARG     dst:DPTR, src:DPTR, srcOfs:UINT, dstLeft:UINT, 			\
				dstTop:UINT, dstRight:UINT, dstBottom:UINT, op:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		pushad

		mov		ebx,[__MGL_afPtr]
		mov		eax,[AF.OffscreenOffset]
		add		[srcOfs],eax
		mov		eax,[dstLeft]
		sub		[dstRight],eax
		mov		eax,[dstTop]
		sub		[dstBottom],eax
		lea     esi,[srcOfs]
		call	[AF.BitBltLin]

		popad

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_bitBltLin

;----------------------------------------------------------------------------
; void ACCEL_srcTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,color_t transparent)
;----------------------------------------------------------------------------
; Routine to perform a screen to screen transBlt operation for an accelerated
; device.
;----------------------------------------------------------------------------
procstart	_ACCEL_srcTransBltOff

		ARG     dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT, 	\
				bottom:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		add		eax,[AF.OffscreenStartY]
		mov		[top],eax
		lea     esi,[left]
		call	[AF.SrcTransBlt]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_srcTransBltOff

;----------------------------------------------------------------------------
; void ACCEL_dstTransBltOff(MGLDC *dst,MGLDC *src,int left,int top,int right,
;	int bottom,int dstLeft,int dstTop,int op,color_t transparent)
;----------------------------------------------------------------------------
; Routine to perform a screen to screen transBlt operation for an accelerated
; device.
;----------------------------------------------------------------------------
procstart	_ACCEL_dstTransBltOff

		ARG     dst:DPTR, src:DPTR, left:UINT, top:UINT, right:UINT, 	\
				bottom:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[left]
		sub		[right],eax
		mov		eax,[top]
		sub		[bottom],eax
		add		eax,[AF.OffscreenStartY]
		mov		[top],eax
		lea     esi,[left]
		call	[AF.DstTransBlt]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_dstTransBltOff

;----------------------------------------------------------------------------
; void ACCEL_srcTransBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,
;	int dstLeft,int dstTop,int dstRight,int dstBottom,int op,
;	color_t transparent);
;----------------------------------------------------------------------------
; Routine to perform a screen to screen transBlt operation for an accelerated
; device between a linear offscreen DC and the display DC.
;----------------------------------------------------------------------------
procstart	_ACCEL_srcTransBltLin

		ARG     dst:DPTR, src:DPTR, srcOfs:UINT, dstLeft:UINT, 			\
				dstTop:UINT, dstRight:UINT, dstBottom:UINT,	transparent:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		pushad

		mov		ebx,[__MGL_afPtr]
		mov		eax,[AF.OffscreenOffset]
		add		[srcOfs],eax
		mov		eax,[dstLeft]
		sub		[dstRight],eax
		mov		eax,[dstTop]
		sub		[dstBottom],eax
		lea     esi,[srcOfs]
		call	[AF.SrcTransBltLin]

		popad

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_srcTransBltLin

;----------------------------------------------------------------------------
; void ACCEL_dstTransBltLin(MGLDC *dst,MGLDC *src,ulong srcOfs,
;	int dstLeft,int dstTop,int dstRight,int dstBottom,int op,
;	color_t transparent);
;----------------------------------------------------------------------------
; Routine to perform a screen to screen transBlt operation for an accelerated
; device between a linear offscreen DC and the display DC.
;----------------------------------------------------------------------------
procstart	_ACCEL_dstTransBltLin

		ARG     dst:DPTR, src:DPTR, srcOfs:UINT, dstLeft:UINT, 			\
				dstTop:UINT, dstRight:UINT, dstBottom:UINT,	transparent:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ebx,[__MGL_afPtr]
		mov		eax,[AF.OffscreenOffset]
		add		[srcOfs],eax
		mov		eax,[dstLeft]
		sub		[dstRight],eax
		mov		eax,[dstTop]
		sub		[dstBottom],eax
		lea     esi,[srcOfs]
		call	[AF.DstTransBltLin]

		pop		esi
		pop		ebx
		pop		ebp
		ret

procend		_ACCEL_dstTransBltLin

endif	; !MGL_LITE

;----------------------------------------------------------------------------
; void ACCEL_blastPalette(palette_t *pal,int num,int index,int maxProgram,
;	int waitVRT)
;----------------------------------------------------------------------------
; Program the palette. The values passed to this routine are always in
; 8 bits per primary format, which is what the ACCEL.DRV palette programming
; routines expect.
;
; In order to avoid snow when setting the palette, we only ever set the
; entries during vertical retrace periods.
;
; Entry:	pal			- Palette of values to program
;           num			- Number of bytes to program
;			index		- Index to start programming at
;			maxProgram	- Number of entries to program during retrace
;			waitVRT		- True if vertical retrace syncing should be used
;----------------------------------------------------------------------------
procstart	_ACCEL_blastPalette

		ARG		pal:DPTR, num:UINT, index:UINT, maxProgram:UINT, waitVRT:UINT

		enter_c	0

		mov		ebx,[__MGL_afPtr]	; EBX -> AF_devCtx
		mov     ecx,[num]			; ECX := entries to program
		mov		edx,[index]			; EDX := starting index
		mov    	esi,[pal]			; ESI -> buffer to restore from
		mov     edi,[maxprogram]	; EDI := maxprogram value

; Determine how many colors can be programmed before the onset of snow.

		xor		eax,eax
		cmp		[waitVRT],0
		je		@@SkipWait			; Skip VRT syncing

		mov		eax,ecx				; EAX := number of colors to set

; Work out how many registers we can program this time around.

@@OuterLoop:
		mov		ecx,edi
		cmp		eax,ecx
		jge		@@Larger
		mov		ecx,eax				; Set count to what is left in AX
@@Larger:
		sub		eax,ecx				; AX := number of registers left to
									;  program after this loop
@@SkipWait:
		pushad
		mov		eax,[waitVRT]
		call	[AF.SetPaletteData]
		popad
		add     edx,ecx				; Increment starting palette index
		shl		ecx,2
		add		esi,ecx				; Increment to next position in buffer
		or		eax,eax
		jnz		@@OuterLoop			; We are all done

		leave_c_nolocal
		ret

procend		_ACCEL_blastPalette

endcodeseg	_gaccel

endif

		END							; End of module
