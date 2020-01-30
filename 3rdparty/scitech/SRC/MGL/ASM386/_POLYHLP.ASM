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
;* Language:	80386 Assembler
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	32 bit optimised assembly language routines for the polygon
;*				rendering helper functions.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

header	_polyhlp				; Setup for MGL hybrid 16/32 bit segment

begdataseg	_polyhlp

	$EXTRN	__MGL_dc,devCtx_s

enddataseg	_polyhlp

begcodeseg	_polyhlp

if flatmodel

ifndef	MGL_LITE

;----------------------------------------------------------------------------
; int _MGL_computeSlope(fxpoint_t *v1,fxpoint_t *v2,fix32_t *slope)
;----------------------------------------------------------------------------
; Compute the slope of the edge and return the sign of the slope.
;
; Entry:		v1		- First vertex in edge
;				v2		- Second vertex in edge
;               slope	- Pointer to place to store slope
;
; Exit:      	_AX		- Sign of slope (1 = +ve, 0, -1 = i-ve)
;
;----------------------------------------------------------------------------
procstart	__MGL_computeSlope

		ARG 	v1:DPTR, v2:DPTR, slope:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi

		mov		ecx,[v2]
		mov		ebx,[v1]
		mov		esi,[(fxpoint PTR ecx).y]
		sub		esi,[(fxpoint PTR ebx).y]	; ESI := v2.y - v1.y
		jle		@@InvalidEdge	; Check for negative/0 edges
		cmp		esi,MGL_FIX_1
		jle		@@QuickSlope	; Handle divide overflow

		mov		eax,[(fxpoint PTR ecx).x]
		sub		eax,[(fxpoint PTR ebx).x]	; EAX := v2.x - v1.x
		mov		edx,eax
		xor		eax,eax
		mov		ebx,[slope]
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	esi
		mov		[ebx],eax		; Store the resulting slope
		mov		eax,1			; Positive edge

@@Exit:
		pop		esi
		pop		ebx
		pop		ebp
		ret

@@QuickSlope:
		mov		eax,[(fxpoint PTR ecx).x]
		sub		eax,[(fxpoint PTR ebx).x]	; EAX := v2.x - v1.x
		mov		ebx,[slope]
		mov		[ebx],eax		; Store the resulting slope
		mov		eax,1			; Positive edge
		jmp		@@Exit

@@InvalidEdge:
		mov     eax,-1			; Negative edge
		jl		@@Exit
		xor		eax,eax			; Zero height edge
		jmp		@@Exit

procend		__MGL_computeSlope

ifdef	MGL_FIX3D

;----------------------------------------------------------------------------
; int _MGL_cComputeSlope(fxpointc_t *v1,fxpointc_t *v2,fix32_t *slope,
;	fix32_t *cSlope)
;----------------------------------------------------------------------------
; Compute the coordinate slope and color slope of the edge and return the
; sign of the coordinate slope.
;
; Entry:		v1		- First vertex in edge
;				v2		- Second vertex in edge
;               slope	- Pointer to place to store slope
;               cSlope	- Pointer to place to store color slope
;
; Exit:      	_AX		- Sign of slope (1 = +ve, 0, -1 = i-ve)
;
;----------------------------------------------------------------------------
procstart	__MGL_cComputeSlope

		ARG 	v1:DPTR, v2:DPTR, slope:DPTR, cSlope:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi
		push	edi

		mov		ecx,[v2]
		mov		ebx,[v1]
		mov		esi,[(fxpointc PTR ecx).p.y]
		sub		esi,[(fxpointc PTR ebx).p.y]; ESI := v2.p.y - v1.p.y
		jle		@@InvalidEdge	; Check for negative/0 edges
		cmp		esi,MGL_FIX_1
		jle		@@QuickSlope	; Handle divide overflow

		mov		edx,[(fxpointc PTR ecx).p.x]
		sub		edx,[(fxpointc PTR ebx).p.x]; EDX := v2.p.x - v1.p.x
		xor		eax,eax
		mov		edi,[slope]
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	esi
		mov		[edi],eax		; Store the res}lting slope

		mov		edx,[(fxpointc PTR ecx).c]
		sub		edx,[(fxpointc PTR ebx).c]	; EDX := v2.c - v1.c
		xor		eax,eax
		mov		edi,[cSlope]
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	esi
		mov		[edi],eax		; Store the resulting slope

		mov		eax,1			; Positive edge

@@Exit:
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

@@QuickSlope:
		mov		eax,[(fxpointc PTR ecx).p.x]
		sub		eax,[(fxpointc PTR ebx).p.x]; EAX := v2.x - v1.x
		mov		edx,[(fxpointc PTR ecx).c]
		sub		edx,[(fxpointc PTR ebx).c]	; EDX := v2.c - v1.c
		mov		esi,[slope]
		mov		edi,[cSlope]
		mov		[esi],eax
		mov		[edi],edx
		mov		eax,1			; Positive edge
		jmp		@@Exit

@@InvalidEdge:
		mov     eax,-1			; Negative edge
		jl		@@Exit
		xor		eax,eax			; Zero height edge
		jmp		@@Exit

procend		__MGL_cComputeSlope

;----------------------------------------------------------------------------
; int _MGL_rgbComputeSlope(fxpointrgb_t *v1,fxpointrgb_t *v2,
;	fix32_t *slope,fix32_t *rSlope,fix32_t *gSlope,fix32_t *bSlope)
;----------------------------------------------------------------------------
; Compute the coordinate slope and color slope of the edge and return the
; sign of the coordinate slope.
;
; Entry:		v1		- First vertex in edge
;				v2		- Second vertex in edge
;               slope	- Pointer to place to store slope
;				rSlope	- Pointer to place to store red channel slope
;				gSlope	- Pointer to place to store green channel slope
;				bSlope	- Pointer to place to store blue channel slope
;
; Exit:      	_AX		- Sign of slope (1 = +ve, 0, -1 = i-ve)
;
;----------------------------------------------------------------------------
procstart	__MGL_rgbComputeSlope

		ARG 	v1:DPTR, v2:DPTR, slope:DPTR, rSlope:DPTR, gSlope:DPTR,	\
				bSlope:DPTR

		push	ebp
		mov		ebp,esp
		push	ebx
		push	esi
		push	edi

		mov		ecx,[v2]
		mov		ebx,[v1]
		mov		esi,[(fxpointrgb PTR ecx).p.y]
		sub		esi,[(fxpointrgb PTR ebx).p.y]; ESI := v2.p.y - v1.p.y
		jle		@@InvalidEdge	; Check for negative/0 edges
		cmp		esi,MGL_FIX_1
		jle		@@QuickSlope	; Handle divide overflow

		mov		edx,[(fxpointrgb PTR ecx).p.x]
		sub		edx,[(fxpointrgb PTR ebx).p.x]; EDX := v2.p.x - v1.p.x
		xor		eax,eax
		mov		edi,[slope]
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	esi
		mov		[edi],eax		; Store the resulting slope

		mov		edx,[(fxpointrgb PTR ecx).c.r]
		sub		edx,[(fxpointrgb PTR ebx).c.r]; EDX := v2.c.r - v1.c.r
		xor		eax,eax
		mov		edi,[rSlope]
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	esi
		mov		[edi],eax		; Store the resulting slope

		mov		edx,[(fxpointrgb PTR ecx).c.g]
		sub		edx,[(fxpointrgb PTR ebx).c.g]; EDX := v2.c.g - v1.c.g
		xor		eax,eax
		mov		edi,[gSlope]
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	esi
		mov		[edi],eax		; Store the resulting slope

		mov		edx,[(fxpointrgb PTR ecx).c.b]
		sub		edx,[(fxpointrgb PTR ebx).c.b]; EDX := v2.c.b - v1.c.b
		xor		eax,eax
		mov		edi,[bSlope]
		shrd	eax,edx,16		; position so that result ends up
		sar		edx,16			; in EAX
		idiv	esi
		mov		[edi],eax		; Store the resulting slope

		mov		eax,1			; Positive edge

@@Exit:
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		ret

@@QuickSlope:
		mov		eax,[(fxpointrgb PTR ecx).p.x]
		sub		eax,[(fxpointrgb PTR ebx).p.x]; EAX := v2.x - v1.x
		mov		edx,[(fxpointrgb PTR ecx).c.r]
		sub		edx,[(fxpointrgb PTR ebx).c.r]
		mov		esi,[slope]
		mov		edi,[rSlope]
		mov		[esi],eax
		mov		[edi],edx
		mov		eax,[(fxpointrgb PTR ecx).c.g]
		sub		eax,[(fxpointrgb PTR ebx).c.g]
		mov		edx,[(fxpointrgb PTR ecx).c.b]
		sub		edx,[(fxpointrgb PTR ebx).c.b]
		mov		esi,[gSlope]
		mov		edi,[bSlope]
		mov		[esi],eax
		mov		[edi],edx
		mov		eax,1			; Positive edge
		jmp		@@Exit

@@InvalidEdge:
		mov     eax,-1			; Negative edge
		jl		@@Exit
		xor		eax,eax			; Zero height edge
		jmp		@@Exit

procend		__MGL_rgbComputeSlope

;----------------------------------------------------------------------------
; void _MGL_zComputeSlope(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3)
;----------------------------------------------------------------------------
; Compute the dZdX and dZdY slopes for the entire polygon face for fast
; zbuffer
;
; Compute the z slope across the entire face of the polygon from the
; following equations (where v1,v2 and v3 are three vertices in the polygon
; with components x?,y?,z?):
;
;			dZ   (y2-y1)(z3-z1) - (y3-y1)(z2-z1)
;			-- = -------------------------------
;			dY   (y2-y1)(x3-x1) - (y3-y1)(x2-x1)
;
;		and
;
;			dZ   (z2-z1)(x3-x1) - (z3-z1)(x2-x1)
;			-- = -------------------------------
;			dX   (y2-y1)(x3-x1) - (y3-y1)(x2-x1)
;
; Note that we store the dZdX and dZdY values into the current DC.tr
; structure directly.
;
; Entry:		v1,v2,v3	- Three vertices in the polygon
;
;----------------------------------------------------------------------------
procstart	__MGL_zComputeSlope

		ARG 	v1:DPTR, v2:DPTR, v3:DPTR

		LOCAL   dX1:UINT, dX2:UINT, dY1:UINT, dY2:UINT, dZ1:UINT,	\
				dZ2:UINT = LocalSize

		enter_c	LocalSize

		mov		ebx,[v1]
		mov		ecx,[v2]
		mov		edx,[v3]

		mov		eax,[(fxpointz PTR ecx).p.x]; EAX := x2
		mov		esi,[(fxpointz PTR ebx).p.x]; ESI := x1
		mov		edi,[(fxpointz PTR edx).p.x]; EDI := x3
		sub		eax,esi
		sub		edi,esi
		mov		[dX1],eax		; dX1 = x2 - x1
		mov		[dX2],edi		; dX2 = x3 - x1

		mov		eax,[(fxpointz PTR ecx).p.y]; EAX := y2
		mov		esi,[(fxpointz PTR ebx).p.y]; ESI := y1
		mov		edi,[(fxpointz PTR edx).p.y]; EDI := y3
		sub		eax,esi
		sub		edi,esi
		mov		[dY1],eax		; dY1 = y2 - y1
		mov		[dY2],edi		; dY2 = y3 - y1

		mov		eax,[(fxpointz PTR ecx).z]; EAX := z2
		mov		esi,[(fxpointz PTR ebx).z]; ESI := z1
		shr		eax,12
		mov		edi,[(fxpointz PTR edx).z]; EDI := z3
		shr		esi,12
		sub		eax,esi
		shr		edi,12
		mov		[dZ1],eax		; dZ1 = z2 - z1
		sub		edi,esi
		mov		[dZ2],edi		; dZ2 = z3 - z1

; Compute 32 bit integerized dZ value. Because this value is an unormalised
; cross product in screen space, the value is going to be very large, so
; we can afford to drop the bottom 16 fractional bits for the final divide
; for speed.
;
;	dZ = dY1*dX2 - dY2*dX1;

		mov		eax,[dY1]
		imul    [dX2]
		mov     esi,edx
		mov		ebx,eax			; ESI:EBX := dY1*dX2
		mov		eax,[dY2]
		imul	[dX1]
		xor		edi,edi			; Set sign of result to zero
		sub     ebx,eax
		sbb		esi,edx			; ESI := dY1*dX2 - dY2*dX1
		jns		@@Pos1
		mov		edi,3			; Flag both results as negative
		neg		esi				; Make divisor positive

;	DC.tr.dZdX = (dY1*dZ2 - dY2*dZ1)/dZ;

@@Pos1:
		mov     eax,[dY1]
		imul    [dZ2]
		mov		ecx,edx
		mov		ebx,eax			; ECX:EBX := dY1*dZ2
		mov		eax,[dY2]
		imul	[dZ1]
		sub		ebx,eax
		sbb		ecx,edx			; ECX:EBX := dY1*dZ2 - dY2*dZ1
		jns		@@Pos2
		xor     edi,1			; Change sign of result
		neg		ecx				; Make dividend positive
		neg		ebx
		sbb		ecx,0

@@Pos2:	mov		eax,ecx
		test	esi,esi
		jz		@@Skp1			; Avoid divide by 0
		push	edi
		mov		edi,ebx			; Save fractional part of dividend
		xor		edx,edx			; EDX:EAX := integer part of dividend
		div		esi				; EAX := top half of 64/32 divide
		mov		ecx,eax			; Set aside integer part of result
		mov		eax,edi			; EDX:EAX := concatenation of fractional
								;	part and integer remainder
		pop		edi
		div		esi
		shrd	eax,ecx,4		; EAX := (dY1*dZ2 - dY2*dZ1)/dZ
@@Skp1:	test	edi,1
		jz		@@Pos3
		neg		eax				; Result is negative
@@Pos3:	mov		[GDC.tr.dZdX],eax

;	DC.tr.dZdY = (dZ1*dX2 - dZ2*dX1)/dZ;

		mov     eax,[dZ1]
		imul    [dX2]
		mov		ecx,edx
		mov		ebx,eax			; ECX:EBX := dZ1*dX2
		mov		eax,[dZ2]
		imul	[dX1]
		sub		ebx,eax
		sbb		ecx,edx			; ECX:EBX := dY1*dZ2 - dY2*dZ1
		jns		@@Pos4
		xor     edi,2			; Change sign of result
		neg		ecx				; Make dividend positive
		neg		ebx
		sbb		ecx,0

@@Pos4: mov		eax,ecx
		test	esi,esi
		jz		@@Skp2			; Avoid divide by 0
		push	edi
		mov		edi,ebx			; Save fractional part of dividend
		xor		edx,edx			; EDX:EAX := integer part of dividend
		div		esi				; EAX := top half of 64/32 divide
		mov		ecx,eax			; Set aside integer part of result
		mov		eax,edi			; EDX:EAX := concatenation of fractional
								;	part and integer remainder
		pop		edi
		div		esi
		shrd	eax,ecx,4		; EAX := (dZ1*dX2 - dZ2*dX1)/dZ
@@Skp2:	test	edi,2
		jz		@@Pos5
		neg		eax				; Result is negative
@@Pos5:	mov		[GDC.tr.dZdY],eax

		leave_c
		ret

procend		__MGL_zComputeSlope

;----------------------------------------------------------------------------
; void _MGL_preRenderScanLine16(int x0,int minx,int maxx)
;----------------------------------------------------------------------------
; Pre-renders the linearly interpolated z offset values into the z offset
; buffer. We only render z values between minx and maxx inclusive.
;
; Entry:		x0		- Reference or base X coordinate
;               minx	- Minimum X coordinate in polygon
;				maxx	- Maximum X coordinate in polygon
;----------------------------------------------------------------------------
procstart	__MGL_preRenderScanLine16

		ARG     x0:UINT, minx:UINT, maxx:UINT

		push	ebp
		mov		ebp,esp
		push	ebx
		push	edi

		mov		ecx,[minx]
		mov		ebx,[GDC.tr.dZdX]	; EDX -> zbuffer x coordinate slope
		mov		eax,ecx
		sub		eax,[x0]
		shl		eax,16				; EAX := minx - x0
		imul    ebx
		shrd	eax,edx,16			; EAX := dZdX * minx - x0
		mov		edi,eax				; EDI := initial value

		lea		edx,[ecx*2]			; EDX := minx * 2
		sub		ecx,[maxx]
		add		edx,[GDC.zOffset]	; EDX -> zOffset buffer
		neg		ecx					; ECX := number of entries to fill
		jle		@@Exit

num_loops = 8

		cmp		ecx,num_loops
		jl		@@NextEndPixel

@@NextPixel:
off = 0
REPT	num_loops
		mov		eax,edi
		shr		eax,15				; Convert to 16 bit value
		mov		[edx+off],ax		; Store zbuffer offset
		add		edi,ebx				; Increment zbuffer offset
off = off + 2
ENDM
		sub		ecx,num_loops
		jz		@@Exit
		add     edx,off
		cmp		ecx,num_loops
		jge		@@NextPixel

@@NextEndPixel:
		mov		eax,edi
		shr		eax,15
		mov		[edx],ax			; Store zbuffer offset
		add		edi,ebx				; Increment zbuffer offset
		inc		edx
		inc		edx
		dec		ecx					; Count off this pixel
		jnz		@@NextEndPixel

@@Exit: pop		edi
		pop		ebx
		pop		ebp
		ret

procend		__MGL_preRenderScanLine16

;----------------------------------------------------------------------------
; void _MGL_preRenderScanLine32(int x0,int minx,int maxx)
;----------------------------------------------------------------------------
; Pre-renders the linearly interpolated z offset values into the z offset
; buffer. We only render z values between minx and maxx inclusive.
;
; Entry:		x0		- Reference or base X coordinate
;               minx	- Minimum X coordinate in polygon
;				maxx	- Maximum X coordinate in polygon
;----------------------------------------------------------------------------
procstart	__MGL_preRenderScanLine32

		ARG     x0:UINT, minx:UINT, maxx:UINT

		push	ebp
		mov		ebp,esp
		push	ebx

		mov		ecx,[minx]
		mov		ebx,[GDC.tr.dZdX]	; EDX -> zbuffer x coordinate slope
		mov		eax,ecx
		sub		eax,[x0]
		shl		eax,16				; EAX := minx - x0
		imul    ebx
		shrd	eax,edx,16			; EAX := dZdX * minx - x0

		lea		edx,[ecx*4]			; EDX := minx * 4
		sub		ecx,[maxx]
		add		edx,[GDC.zOffset]	; EDX -> zOffset buffer
		neg		ecx					; ECX := number of entries to fill
		jle		@@Exit

num_loops = 8

		cmp		ecx,num_loops
		jl		@@NextEndPixel

@@NextPixel:
off = 0
REPT	num_loops
		mov		[edx+off],eax		; Store zbuffer offset
		add		eax,ebx				; Increment zbuffer offset
off = off + 4
ENDM
		sub		ecx,num_loops
		jz		@@Exit
		add     edx,off
		cmp		ecx,num_loops
		jge		@@NextPixel

@@NextEndPixel:
		mov		[edx],eax			; Store zbuffer offset
		add		eax,ebx				; Increment zbuffer offset
		inc		edx
		inc		edx
		inc		edx
		inc		edx
		dec		ecx					; Count off this pixel
		jnz		@@NextEndPixel

@@Exit:	pop		ebx
		pop		ebp
		ret

procend		__MGL_preRenderScanLine32

endif	; MGL_FIX3D

endif	; !MGL_LITE

endif	; flatmodel

endcodeseg	_polyhlp

		END						; End of module
