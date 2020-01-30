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
;* Description:	Optimised assembly language stubs for calls directly into
;*				the device driver. We simply extract the address of the
;*				routine being called and jump directly to it.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"asm386\mgl.equ"		; Assembly language equates

header	_stubs					; Set up memory model

begdataseg	_stubs

		$EXTRN	__MGL_dc,devCtx_s

enddataseg	_stubs

begcodeseg	_stubs				; Start of code segment

; The following macro defines the stub routine that extract the address of
; the function to call, and jumps directly to it. This way there is no
; need to re-push all of the parameters onto the stack again in order to
; call the function within the current device context.

MACRO   defineStub	funcName, dcName
funcName:
ifdef   BUILD_DLL
		PUBLICDLL	funcName
else
		PUBLIC		funcName
endif
		jmp		[GDC.r.dcName]	; Jump directly to the routine
ENDM

defineStub  _MGL_setColor, 				setColor
defineStub	_MGL_setBackColor,			setBackColor
defineStub	_MGL_beginDirectAccess,		beginDirectAccess
defineStub	_MGL_endDirectAccess,		endDirectAccess
defineStub	_MGL_scanLine,				cur.scanLine

ifndef	MGL_LITE
defineStub  _MGL_setWriteMode, 			setWriteMode
defineStub  _MGL_setPenStyle, 			setPenStyle
defineStub  _MGL_setLineStipple,		setLineStipple
defineStub  _MGL_setLineStippleCount,	setLineStippleCount
defineStub  _MGL_setPenBitmapPattern,	setPenBitmapPattern
defineStub  _MGL_setPenPixmapPattern,	setPenPixmapPattern
defineStub	_MGL_getArcCoords,			getArcCoords
defineStub	_MGL_scanRightForColor,		scanRightForColor
defineStub	_MGL_scanLeftForColor,		scanLeftForColor
defineStub	_MGL_scanRightWhileColor,	scanRightWhileColor
defineStub	_MGL_scanLeftWhileColor,	scanLeftWhileColor

ifdef	MGL_3D
defineStub	_MGL_zBegin,				zBegin
endif

endif	; !MGL_LITE

endcodeseg	_stubs

		END						; End of module
