;****************************************************************************
;*
;*          The SuperVGA Kit - UniVBE Software Development Kit
;*
;*                  Copyright (C) 1996 SciTech Software
;*                          All rights reserved.
;*
;* Filename:    $Workfile:   svga.inc  $
;* Version:     $Revision:   1.1  $
;*
;* Language:    80386 Assembler (TASM ideal mode)
;* Environment: IBM PC 32 bit Protected Mode.
;*
;* Description: Macros and type definitions for the SuperVGA Kit
;*
;* $Date:   17 Feb 1996 19:34:44  $ $Author:   KendallB  $
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
bytesperline		ulong	?
bitsperpixel		uint	?
bytesperpixel		uint	?
memory				uint	?
linearAddr			ulong	?
modeList			cptr	?
OEMString			cptr	?
curBank				uint	?
haveVirtualBuffer	bool	?
haveDoubleBuffer	bool	?
haveVirtualScroll	bool	?
haveWideDAC			bool	?
haveAccel2D			bool	?
haveHWCursor		bool	?
virtualBuffer		bool	?
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
AFDC				cptr	?
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
