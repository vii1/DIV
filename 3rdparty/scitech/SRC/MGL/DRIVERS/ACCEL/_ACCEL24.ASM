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
;* Description:	ACCEL24, 24 bit accelerated device context stub routines.
;*
;*
;****************************************************************************

		IDEAL
		JUMPS

include	"model.mac"
include	"asm386\mgl.equ"
include "vbeaf.inc"

		EMU_PUTIMAGE	= 1
		EMU_SCANCOLOR	= 1

; Macros to build the stub functions

MACRO   stubstart_internal name,orgname
		EXTRN	orgname:FPTR
_ACCEL24_&name&:
		PUBLIC	_ACCEL24_&name&
		push	ebx
		mov		ebx,[__MGL_afPtr]
		call	[AF_devCtx.WaitTillIdle]
		pop		ebx
		jmp		orgname

PROC	_ACCEL24_&name&SLOW
		PUBLIC	_ACCEL24_&name&SLOW

		push	ebp
		mov		ebp,esp
		push	ebx
		mov		ebx,[__MGL_afPtr]
		call	[AF_devCtx.EnableDirectAccess]
ENDM

MACRO	stubend_internal name,orgname,parmcount
		call	orgname
		add     esp,parmcount * 4
		call	[AF_devCtx.DisableDirectAccess]
		pop		ebx
		pop		ebp
		ret
ENDP	_ACCEL24_&name&SLOW
ENDM

MACRO   stubstart name
		stubstart_internal name,_PACKED24_&name&
ENDM

MACRO	stubend name,parmcount
		stubend_internal name,_PACKED24_&name&,parmcount
ENDM

MACRO   stubstartEMU name,emuname
		stubstart_internal name,___EMU__&emuname&
ENDM

MACRO	stubendEMU name,emuname,parmcount
		stubend_internal name,___EMU__&emuname&,parmcount
ENDM

include	"drivers\accel\_stubs.asm"

		END							; End of module
