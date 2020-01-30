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
;*				fixed point power operations using table lookup and
;*				linear interpolation:
;*
;*                  F386_pow	- 16.16 rase x to power of y
;*
;*
;****************************************************************************

		IDEAL

INCLUDE "model.mac"				; Memory model macros

header	fxpow					; Set up memory model

begdataseg	fxpow

;		$EXTRN	_FXpow_table,USHORT

enddataseg	fxpow

begcodeseg	fxpow

endcodeseg	fxpow

		END						; End of module
