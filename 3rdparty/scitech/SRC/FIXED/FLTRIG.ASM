;****************************************************************************
;*
;*				  High Speed Fixed/Floating Point Library
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
;* Language:	80387 Assembler
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	This module contains routines for high performance
;*				realing point trig operations by directly programming the
;*				80387 FPU (parameters passed as either floats or doubles
;*				depending on which compilation option is selected).
;*
;*                  FXsin		- floating point sine (degrees)
;*					FXcos		- floating point cosine (degrees)
;*					FXsincos	- floating point sine and cosine (degrees)
;*					FXtan		- floating point tangent (degrees)
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros
INCLUDE	"flmacs.mac"

		P387					; Turn on i387 instructions

header	fltrig					; Set up memory model

begdataseg	fltrig

		$EXTRN	_FXpi_180,REAL		; Real value for PI/180

tmp_real	real	?

enddataseg	fltrig

begcodeseg	fltrig

;----------------------------------------------------------------------------
; real FXsin(real angle)
;----------------------------------------------------------------------------
procstart	_FXsin

		ARG		angle:REAL

if flatmodel
angle	=		REAL esp+4		; Access directly without stack frame
else
		push	_bp
		mov		_bp,_sp
endif
		fld		[angle]			; ST(0) := angle in degrees
		fmul    [_FXpi_180]		; ST(0) := angle in radians
		fsin					; ST(0) := sine of angle
		RETURN	tmp_real
ife flatmodel
		pop		_bp
endif
		ret

procend		_FXsin

;----------------------------------------------------------------------------
; real FXcos(real angle)
;----------------------------------------------------------------------------
procstart	_FXcos

		ARG		angle:REAL

if flatmodel
angle	=		REAL esp+4		; Access directly without stack frame
else
		push	_bp
		mov		_bp,_sp
endif
		fld		[angle]			; ST(0) := angle in degrees
		fmul    [_FXpi_180]		; ST(0) := angle in radians
		fcos					; ST(0) := cosine of angle
		RETURN	tmp_real
ife flatmodel
		pop		_bp
endif
		ret

procend		_FXcos

;----------------------------------------------------------------------------
; void FXsincos(real angle,real& Sin,real& Cos)
;----------------------------------------------------------------------------
; Computes both the sine and cosine of an angle as fast as possible.
;----------------------------------------------------------------------------
procstart	_FXsincos

		ARG		angle:REAL, Sin:DPTR, Cos:DPTR

		push	_bp
		mov		_bp,_sp
		push	_bx

		fld		[angle]			; ST(0) := angle in degrees
		fmul    [_FXpi_180]		; ST(0) := angle in radians
		fsincos					; ST(1) := sine of angle
								; ST(0) := cosine of angle
		wait					; Avoid bugs in fsincos instruction ;-(
		_les	_bx,[Cos]		; ES:_BX -> place for cosine
		fstp	[REAL _ES _bx]	; Store cosine value
		_les	_bx,[Sin]		; ES:_BX -> place for sine
		fstp	[REAL _ES _bx]	; Store sine value

		pop		_bx
		pop		_bp
		ret

procend		_FXsincos

;----------------------------------------------------------------------------
; real FXtan(real angle)
;----------------------------------------------------------------------------
procstart	_FXtan

		ARG		angle:REAL

if flatmodel
angle	=		REAL esp+4		; Access directly without stack frame
else
		push	_bp
		mov		_bp,_sp
endif
		fld		[angle]			; ST(0) := angle in degrees
		fmul    [_FXpi_180]		; ST(0) := angle in radians
		fptan					; ST(0) := 1.0
								; ST(1) := tangent
		wait					; Avoid bugs with FPTAN
		nop
		fstp	st(0)			; Clean up FPU stack
		RETURN	tmp_real
ife flatmodel
		pop		_bp
endif
		ret

procend		_FXtan

endcodeseg	fltrig

		END						; End of module
