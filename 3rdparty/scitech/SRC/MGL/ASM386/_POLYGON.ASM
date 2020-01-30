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
;* Description:	Optimised assembly language routines for the polygon
;*				rendering module. This module contains fast assembler
;*				code entry points for triangle and quad filling, that
;*				does pre-clipping inline and provides a fast path directly
;*				into the device driver routines for code that does not
;*				need the polygons to be clipped.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

ifdef	MGL_FIX3D

header	_polygon				; Setup for MGL hybrid 16/32 bit segment

if flatmodel

begdataseg	_polygon

		$EXTRN	__MGL_dc,devCtx_s

left	dd	?
right	dd	?
top		dd	?
bottom	dd	?

enddataseg	_polygon

begcodeseg	_polygon			; Start of hybrid 16/32 code segment

;----------------------------------------------------------------------------
; MGL_preClipAndViewMapTri
;----------------------------------------------------------------------------
; Macro to pre-clip the triangle if needed, and to viewport map the
; xoffset and yoffset values directly. This macro will return directly from
; the function for trivial rejection cases, or will fall through with
; the appropriate clip flag set in GDC.doClip ready to draw the trigangle.
;----------------------------------------------------------------------------
MACRO   MGL_preClipAndViewMapTri point
		local	Draw

		ARG     v1:DPTR, v2:DPTR, v3:DPTR, xOffset:FIX32_T, yOffset:FIX32_T

		mov		[GDC.doClip],false
		cmp     [GDC.a.clip],false
		je		Draw

		enter_c	0

; Find the bounding rectangle for the polygon

		mov		ebx,[v1]
		MGL_initialMinMax <point>
		mov		ebx,[v2]
		MGL_findMinMax <point>
		mov		ebx,[v3]
		MGL_findMinMax <point>
		MGL_offsetRect	[xOffset],[yOffset]

; Compute intersection between bounding rectangle and clipping rectangle
; and determine cases for trivial reject, trivial accept and clip.

		mov		[left],ecx
		mov		[right],edx
		mov		[top],esi
		mov		[bottom],edi
		MGL_sectRect    GDC.clipRectFX
		MGL_checkEmpty	[left],[top],[right],[bottom]
		mov		[GDC.doClip],eax
		or		eax,eax
		leave_c_nolocal
		jns		Draw
		ret

Draw:	mov     eax,[GDC.viewPortFX.left]
		mov     ecx,[GDC.viewPortFX.top]
		add		[esp+16],eax
		add		[esp+20],ecx
ENDM

;----------------------------------------------------------------------------
; MGL_preClipAndViewMapQuad
;----------------------------------------------------------------------------
; Macro to pre-clip the triangle if needed, and to viewport map the
; xoffset and yoffset values directly. This macro will return directly from
; the function for trivial rejection cases, or will fall through with
; the appropriate clip flag set in GDC.doClip ready to draw the trigangle.
;----------------------------------------------------------------------------
MACRO   MGL_preClipAndViewMapQuad point
		local	Draw

		ARG     v1:DPTR, v2:DPTR, v3:DPTR, v4:DPTR, xOffset:FIX32_T,	\
				yOffset:FIX32_T

		mov		[GDC.doClip],false
		cmp     [GDC.a.clip],false
		je		Draw

		enter_c	0

; Find the bounding rectangle for the polygon

		mov		ebx,[v1]
		MGL_initialMinMax <point>
		mov		ebx,[v2]
		MGL_findMinMax <point>
		mov		ebx,[v3]
		MGL_findMinMax <point>
		mov		ebx,[v4]
		MGL_findMinMax <point>
		MGL_offsetRect	[xOffset],[yOffset]

; Compute intersection between bounding rectangle and clipping rectangle
; and determine cases for trivial reject, trivial accept and clip.

		mov		[left],ecx
		mov		[right],edx
		mov		[top],esi
		mov		[bottom],edi
		MGL_sectRect    GDC.clipRectFX
		MGL_checkEmpty	[left],[top],[right],[bottom]
		mov		[GDC.doClip],eax
		or		eax,eax
		leave_c_nolocal
		jns		Draw
		ret

Draw:	mov     eax,[GDC.viewPortFX.left]
		mov     ecx,[GDC.viewPortFX.top]
		add		[esp+20],eax
		add		[esp+24],ecx
ENDM

;----------------------------------------------------------------------------
; void MGL_tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
;	fix32_t xOffset,fix32_t yOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_tri

		MGL_preClipAndViewMapTri <(fxpoint PTR ebx)>
		cmp     [GDC.a.colorMode],MGL_DITHER_RGB_MODE
		je		@@1
		jmp		[GDC.r.tri]
@@1:	jmp		[GDC.r.ditherTri]

procend			_MGL_tri

;----------------------------------------------------------------------------
; void MGL_cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
;	fix32_t xOffset,fix32_t yOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_cTri

		MGL_preClipAndViewMapTri <(fxpointc PTR ebx).p>
		jmp		[GDC.r.cTri]

procend			_MGL_cTri

;----------------------------------------------------------------------------
; void MGL_rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,
;	fix32_t xOffset,fix32_t yOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_rgbTri

		MGL_preClipAndViewMapTri <(fxpointrgb PTR ebx).p>
		jmp		[GDC.r.rgbTri]

procend			_MGL_rgbTri

;----------------------------------------------------------------------------
; void MGL_zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
;	fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_zTri

		MGL_preClipAndViewMapTri <(fxpointz PTR ebx).p>
		cmp     [GDC.a.colorMode],MGL_DITHER_RGB_MODE
		je		@@1
		jmp		[GDC.r.z.zTri]
@@1:	jmp		[GDC.r.z.zDitherTri]

procend			_MGL_zTri

;----------------------------------------------------------------------------
; void MGL_czTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
;	fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_czTri

		MGL_preClipAndViewMapTri <(fxpointcz PTR ebx).p>
		jmp		[GDC.r.z.czTri]

procend			_MGL_czTri

;----------------------------------------------------------------------------
; void MGL_rgbzTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
;	fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_rgbzTri

		MGL_preClipAndViewMapTri <(fxpointrgbz PTR ebx).p>
		jmp		[GDC.r.z.rgbzTri]

procend			_MGL_rgbzTri

;----------------------------------------------------------------------------
; void MGL_quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,
;	fix32_t xOffset,fix32_t yOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_quad

		MGL_preClipAndViewMapQuad <(fxpoint PTR ebx)>
		cmp     [GDC.a.colorMode],MGL_DITHER_RGB_MODE
		je		@@1
		jmp		[GDC.r.quad]
@@1:	jmp		[GDC.r.ditherQuad]

procend			_MGL_quad

;----------------------------------------------------------------------------
; void MGL_cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
;	fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_cQuad

		MGL_preClipAndViewMapQuad <(fxpointc PTR ebx).p>
		jmp		[GDC.r.cQuad]

procend			_MGL_cQuad

;----------------------------------------------------------------------------
; void MGL_rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,
;	fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_rgbQuad

		MGL_preClipAndViewMapQuad <(fxpointrgb PTR ebx).p>
		jmp		[GDC.r.rgbQuad]

procend			_MGL_rgbQuad

;----------------------------------------------------------------------------
; void MGL_zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
;	fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_zQuad

		MGL_preClipAndViewMapQuad <(fxpointz PTR ebx).p>
		cmp     [GDC.a.colorMode],MGL_DITHER_RGB_MODE
		je		@@1
		jmp		[GDC.r.z.zQuad]
@@1:	jmp		[GDC.r.z.zDitherQuad]

procend			_MGL_zQuad

;----------------------------------------------------------------------------
; void MGL_czQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
;	fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_czQuad

		MGL_preClipAndViewMapQuad <(fxpointcz PTR ebx).p>
		jmp		[GDC.r.z.czQuad]

procend			_MGL_czQuad

;----------------------------------------------------------------------------
; void MGL_rgbzQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
;	fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
;----------------------------------------------------------------------------
procstartdll	_MGL_rgbzQuad

		MGL_preClipAndViewMapQuad <(fxpointrgbz PTR ebx).p>
		jmp		[GDC.r.z.rgbzQuad]

procend			_MGL_rgbzQuad

endcodeseg	_polygon

endif	; flatmodel

endif	; MGL_FIX3D

		END						; End of module
