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
;* Description:	Generic DirectDraw device context stub routines.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"

if	flatmodel

header	_stubs

begdataseg	_stubs

	$EXTRN  __PACKED_vecs,vecs_s
	$EXTRN	__MGL_surfLock,UINT

enddataseg	_stubs

begcodeseg	_stubs

		EXTRN	_WDCI_lock:FPTR
		EXTRN	_WDCI_unlock:FPTR

; Macros to build the stub functions

MACRO   stubstart name,orgname
__org_name EQU orgname
PROC	_WDCISTUB_&name&
		PUBLIC	_WDCISTUB_&name&

		push	ebp
		mov		ebp,esp
		inc		[__MGL_surfLock]
		jnz		@@1
		call	_WDCI_lock
@@1:
ENDM

MACRO	stubend name,parmcount
		call	[__PACKED_vecs.__org_name]
		add     esp,parmcount * 4
		dec		[__MGL_surfLock]
		jns		@@2
		call	_WDCI_unlock
@@2:	pop		ebp
		ret
ENDP	_WDCISTUB_&name&
ENDM

include "drivers\common\_stubs.asm"

endcodeseg	_stubs

endif	; flatmodel

		END							; End of module

