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
;*				floating point logarithms using the 80387.
;*
;*					FXlog10	- floating point base 10 logarithm
;*                  FXlog	- floating point natural logarithm
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros
INCLUDE "flmacs.mac"

header	fllog					; Set up memory model

begdataseg	fllog

tmp_real	real	?

enddataseg	fllog

begcodeseg	fllog

;----------------------------------------------------------------------------
; real FXlog10(real f);
;----------------------------------------------------------------------------
procstart	_FXlog10

		ARG		f:REAL

if flatmodel
f		=		REAL esp+4		; Access directly without stack frame
else
		push	_bp
		mov		_bp,_sp
endif
		fldlg2					; ST(0) := log10(2)
		fld		[f]				; ST(0) := f
		fyl2x					; ST(0) := log10(f)
		RETURN	tmp_real
ife flatmodel
		pop		_bp
endif
		ret

procend		_FXlog10

;----------------------------------------------------------------------------
; real FXlog(real f);
;----------------------------------------------------------------------------
procstart	_FXlog

		ARG		f:REAL

if flatmodel
f		=		REAL esp+4		; Access directly without stack frame
else
		push	_bp
		mov		_bp,_sp
endif
		fldln2					; ST(0) := loge(2)
		fld		[f]				; ST(0) := f
		fyl2x					; ST(0) := loge(f)
		RETURN	tmp_real
ife flatmodel
		pop		_bp
endif
		ret

procend		_FXlog

endcodeseg	fllog

		END						; End of module
