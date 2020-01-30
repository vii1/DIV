;****************************************************************************
;*
;*          The SuperVGA Kit - UniVBE Software Development Kit
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
;* Language:    80386 Assembler (TASM ideal mode)
;* Environment: IBM PC 32 bit Protected Mode.
;*
;* Description: Macros and type definitions for the SuperVGA Kit
;*
;*
;****************************************************************************

;----------------------------------------------------------------------------
; Public device context structure
;----------------------------------------------------------------------------

struc	SV_devCtx_s

VBEVersion			uint	?
maxx				uint	?
maxy				uint	?
maxcolor			ulong	?
defcolor			ulong	?
maxpage				uint	?
activePage			uint	?
visualPage          uint	?
bytesperline		ulong	?
bitsperpixel		uint	?
bytesperpixel		uint	?
memory				uint	?
linearAddr			ulong	?
modeList			cptr	?
OEMString			cptr	?
curBank				uint	?
haveNonVGA			uint	?
haveVirtualBuffer	uint	?
haveDoubleBuffer	uint	?
haveVirtualScroll	uint	?
haveWideDAC			uint	?
haveAccel2D			uint	?
haveRefreshCtrl		uint	?
haveHWCursor		uint	?
haveTripleBuffer	uint	?
haveStereo			uint	?
haveHWStereoSync 	uint	?
haveEVCStereoSync	uint	?
virtualBuffer		uint	?
isNonVGA			uint	?
isStereo			uint	?
stereoRunning		uint	?
useBlueCode			uint	?
blueCodeIndex		uint	?
videoMem			cptr	?
originOffset		ulong	?
bankOffset			ushort	?
redMask				uchar	?
greenMask			uchar	?
blueMask			uchar	?
redPos				uint	?
redAdjust			uint	?
greenPos            uint	?
greenAdjust         uint	?
bluePos             uint	?
blueAdjust          uint	?
AFDC				dptr	?
loadedAF			dptr	?
putPixel            cptr	?
beginPixel          cptr	?
putPixelFast        cptr	?
endPixel            cptr	?
clear               cptr	?
line                cptr	?
beginLine           cptr	?
lineFast            cptr	?
endLine             cptr	?
setActivePage       cptr	?
setVisualPage       cptr	?
beginDirectAccess   cptr	?
endDirectAccess     cptr	?

ends    SV_devCtx_s

; Values stored in the DC.isStereo field

svNoStereo			= 0
svSoftStereo		= 1
svHWStereo			= 2
svHWStereoDual		= 3

; Defines for waitVRT flag for SV_setVisualPage()

svTripleBuffer  	= 0
svWaitVRT			= 1
svDontWait			= 2
