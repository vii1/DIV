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
;* Language:	80386 Assembler
;* Environment:	IBM PC (MS DOS)
;*
;* Description:	This module contains routines for high performance 16.16
;*				fixed point inverse trig operations using table lookup and
;*				linear interpolation:
;*
;*                  F386_asin	- 16.16 arc sine
;*					F386_acos	- 16.16 arc cosine
;*					F386_atan	- 16.16 arc tangent
;*					F386_atan2	- 16.16 arc tangent given rise and run
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros

header	fxatrig					; Set up memory model

begdataseg	fxatrig

		$EXTRN	_FXsin_table,ULONG
		$EXTRN	_FXatan_table,ULONG

enddataseg	fxatrig

begcodeseg	fxatrig

endcodeseg	fxatrig

		END						; End of module
