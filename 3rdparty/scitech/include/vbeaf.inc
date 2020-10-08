;****************************************************************************
;*
;*				 VESA BIOS Extensions/Accelerator Functions
;*								Version 1.0
;*
;*                   Copyright (C) 1996 SciTech Software.
;*                           All rights reserved.
;*
;* Filename:     $Workfile:   vbeaf.inc  $
;* Developed by: SciTech Software
;*
;* Language:     80386 Assembler (TASM ideal mode)
;* Environment:  IBM PC 32 bit Protected Mode.
;*
;* Description:  Macros and type definitions for VBE/AF
;*
;* $Date:   21 Feb 1996 18:35:42  $ $Author:   KendallB  $
;*
;****************************************************************************

;----------------------------------------------------------------------------
; Fundamental types
;----------------------------------------------------------------------------

typedef AF_int8		byte
typedef AF_int16	word
typedef	AF_int32	dword
typedef AF_uint8	byte
typedef AF_uint16	word
typedef	AF_uint32	dword
typedef AF_fix32	dword

typedef	AF_pattern	AF_uint8
typedef	AF_stipple	AF_uint16
typedef AF_color	AF_uint32

true				=	1
false				=	0

;----------------------------------------------------------------------------
; Attribute flags
;----------------------------------------------------------------------------

afDontClear			=	8000h
afLinearBuffer		=	4000h
afMultiBuffer       =   2000h
afVirtualScroll		=	1000h

afHaveMultiBuffer   =   0001h
afHaveVirtualScroll	=	0002h
afHaveBankedBuffer	=	0004h
afHaveLinearBuffer	=	0008h
afHaveAccel2D		=	0010h
afHaveDualBuffers	=   0020h
afHaveHWCursor		=	0040h
afHave8BitDAC		=	0080h
afNonVGAMode		=	0100h

enum AF_mixmodes {
	AF_REPLACE_MIX
	AF_AND_MIX
	AF_OR_MIX
	AF_XOR_MIX
	AF_NOP_MIX
	}

;----------------------------------------------------------------------------
; Public device context structure
;----------------------------------------------------------------------------

struc	AF_devCtx_s
Signature				uchar 12 dup (?)
AFVersion				AF_uint32	?
DriverRev           	AF_uint32	?
OemVendorName			uchar 80 dup (?)
OemCopyright			uchar 80 dup (?)
AvailableModes			dptr		?
TotalMemory         	AF_uint32	?
Attributes          	AF_uint32	?
BankSize            	AF_uint32	?
BankedBasePtr			AF_uint32	?
LinearSize				AF_uint32	?
LinearBasePtr       	AF_uint32	?
LinearGranularity		AF_uint32	?
IOPortsTable        	dptr		?
IOMemoryBase        	AF_uint32 4 dup (?)
IOMemoryLen         	AF_uint32 4 dup (?)
res1					AF_uint32 10 dup (?)
IOMemMaps				dptr 4 dup (?)
BankedMem				dptr		?
LinearMem				dptr		?
Sel0000h				AF_uint16	?
pad1					AF_uint16	?
Sel0040h				AF_uint16	?
pad2					AF_uint16	?
SelA000h				AF_uint16	?
pad3					AF_uint16	?
SelB000h				AF_uint16	?
pad4					AF_uint16	?
SelC000h				AF_uint16	?
pad5					AF_uint16	?
BufferEndX           	AF_uint32	?
BufferEndY           	AF_uint32	?
OriginOffset            AF_uint32	?
OffscreenOffset         AF_uint32	?
OffscreenStartY         AF_uint32	?
OffscreenEndY           AF_uint32	?
res2					AF_uint32 10 dup (?)
SetBank32Len            AF_uint32	?
SetBank32				cptr		?
Int86                   cptr		?
CallRealMode            cptr		?
InitDriver              cptr		?
GetVideoModeInfo        cptr		?
SetVideoMode            cptr		?
RestoreTextMode         cptr		?
SetBank                 cptr		?
SetDisplayStart         cptr		?
SetActiveBuffer         cptr		?
SetVisibleBuffer        cptr		?
SetPaletteData          cptr		?
SetGammaCorrectData     cptr		?
WaitTillIdle            cptr		?
EnableDirectAccess      cptr		?
DisableDirectAccess     cptr		?
SetCursor               cptr		?
SetCursorPos            cptr		?
SetCursorColor          cptr		?
ShowCursor              cptr		?
SetMix        			cptr		?
Set8x8MonoPattern		cptr		?
Set8x8ColorPattern		cptr		?
SetLineStipple          cptr		?
SetClipRect             cptr		?
DrawScan                cptr		?
DrawPattScan            cptr		?
DrawColorPattScan		cptr		?
DrawScanList			cptr		?
DrawRect                cptr		?
DrawPattRect            cptr		?
DrawColorPattRect		cptr		?
DrawLine                cptr		?
DrawStippleLine         cptr		?
DrawTrap				cptr		?
DrawTri                 cptr		?
DrawQuad                cptr		?
PutMonoImage            cptr		?
BitBlt                  cptr		?
BitBltLin               cptr		?
SrcTransBlt             cptr		?
SrcTransBltLin          cptr		?
DstTransBlt             cptr		?
DstTransBltLin          cptr		?
ends    AF_devCtx_s

FIRST_AF_VEC	EQU	InitDriver
LAST_AF_VEC		EQU	DstTransBltLin

AF_devCtx = (AF_devCtx_s PTR DS:EBX)

;----------------------------------------------------------------------------
; Mode information block structure
;----------------------------------------------------------------------------

struc	AF_modeInfo_s
Attributes              AF_uint16	?
XResolution             AF_uint16	?
YResolution             AF_uint16	?
BytesPerScanLine        AF_uint16	?
BitsPerPixel            AF_uint16	?
MaxBuffers              AF_uint16   ?
RedMaskSize             AF_uint8    ?
RedFieldPosition        AF_uint8	?
GreenMaskSize           AF_uint8	?
GreenFieldPosition      AF_uint8	?
BlueMaskSize            AF_uint8	?
BlueFieldPosition       AF_uint8	?
RsvdMaskSize            AF_uint8	?
RsvdFieldPosition       AF_uint8	?
MaxBytesPerScanLine		AF_uint16	?
MaxScanLineWidth		AF_uint16	?
reserved				uchar 118 dup (?)
ends    AF_modeInfo_s

AF_modeInfo = (AF_modeInfo_s PTR DS:EDI)

;----------------------------------------------------------------------------
; DPMI register structure for passing to Int86 and CallRealMode
;----------------------------------------------------------------------------

struc 	AF_DPMI_regs_s
edi						AF_uint32	?
esi						AF_uint32	?
ebp						AF_uint32	?
reserved				AF_uint32	?
ebx						AF_uint32	?
edx						AF_uint32	?
ecx						AF_uint32	?
eax						AF_uint32	?
flags					AF_uint16	?
es						AF_uint16	?
ds						AF_uint16	?
fs						AF_uint16	?
gs						AF_uint16	?
ip						AF_uint16	?
cs						AF_uint16	?
sp						AF_uint16	?
ss						AF_uint16	?
ends    AF_DPMI_regs_s

;----------------------------------------------------------------------------
; Palette entry structure
;----------------------------------------------------------------------------

struc 	AF_palette_s
blue					AF_uint8	?
green					AF_uint8	?
red						AF_uint8	?
alpha					AF_uint8	?
ends  	AF_palette_s

AF_palette = (AF_palette_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Hardware cursor definition structure
;----------------------------------------------------------------------------

struc 	AF_cursor_s
xorMask					AF_int32 32 dup (?)
andMask					AF_int32 32 dup (?)
hotx					AF_int32	?
hoty					AF_int32	?
ends  	AF_cursor_s

AF_cursor = (AF_cursor_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for SetClipRect
;----------------------------------------------------------------------------

struc 	AF_clipRect_s
minX					AF_int32	?
minY                    AF_int32	?
maxX                    AF_int32	?
maxY                    AF_int32	?
ends  	AF_clipRect_s

AF_clipRect = (AF_clipRect_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for DrawRect and DrawPattRect
;----------------------------------------------------------------------------

struc 	AF_rect_s
left                    AF_int32	?
top                     AF_int32	?
width                   AF_int32	?
height                  AF_int32	?
ends  	AF_rect_s

AF_rect = (AF_rect_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for DrawLine and DrawStippleLine
;----------------------------------------------------------------------------

struc 	AF_line_s
x1		               	AF_fix32	?
y1                     	AF_fix32	?
x2                  	AF_fix32	?
y2                  	AF_fix32	?
ends  	AF_line_s

AF_line = (AF_line_s PTR DS:ESI)

;----------------------------------------------------------------------------
; 2D fixed point vertex structure
;----------------------------------------------------------------------------

struc 	AF_fxpoint
x		               	AF_fix32	?
y                     	AF_fix32	?
ends  	AF_fxpoint

;----------------------------------------------------------------------------
; Parameter block for DrawTrap
;----------------------------------------------------------------------------

struc 	AF_trap_s
y						AF_uint32	?
count					AF_uint32	?
x1						AF_fix32	?
x2						AF_fix32	?
slope1					AF_fix32	?
slope2					AF_fix32	?
ends  	AF_trap_s

AF_trap = (AF_trap_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for DrawTri
;----------------------------------------------------------------------------

struc 	AF_tri_s
v1		               	dptr		?
v2		               	dptr		?
v3		               	dptr		?
xOffset		            AF_fix32	?
yOffset		            AF_fix32	?
ends  	AF_tri_s

AF_tri = (AF_tri_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for DrawQuad
;----------------------------------------------------------------------------

struc 	AF_quad_s
v1		               	dptr		?
v2		               	dptr		?
v3		               	dptr		?
v4		               	dptr		?
xOffset		            AF_fix32	?
yOffset		            AF_fix32	?
ends  	AF_quad_s

AF_quad = (AF_quad_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for PutMonoImage
;----------------------------------------------------------------------------

struc 	AF_monoImage_s
x		               	AF_int32	?
y                       AF_int32	?
byteWidth               AF_int32	?
height                  AF_int32	?
image					dptr		?
ends  	AF_monoImage_s

AF_monoImage = (AF_monoImage_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for BitBlt
;----------------------------------------------------------------------------

struc 	AF_bitBlt_s
left                    AF_int32	?
top                     AF_int32	?
width                   AF_int32	?
height                  AF_int32	?
dstLeft                 AF_int32	?
dstTop                  AF_int32	?
op						AF_int32	?
ends  	AF_bitBlt_s

AF_bitBlt = (AF_bitBlt_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for BitBltLin
;----------------------------------------------------------------------------

struc 	AF_bitBltLin_s
srcOfs                  AF_int32	?
dstLeft                 AF_int32	?
dstTop                  AF_int32	?
width                   AF_int32	?
height                  AF_int32	?
op						AF_int32	?
ends  	AF_bitBltLin_s

AF_bitBltLin = (AF_bitBltLin_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for TransBlt
;----------------------------------------------------------------------------

struc 	AF_transBlt_s
left                    AF_int32	?
top                     AF_int32	?
width                   AF_int32	?
height                  AF_int32	?
dstLeft                 AF_int32	?
dstTop                  AF_int32	?
op						AF_int32	?
transparent				AF_int32	?
ends  	AF_transBlt_s

AF_transBlt = (AF_transBlt_s PTR DS:ESI)

;----------------------------------------------------------------------------
; Parameter block for TransBltLin
;----------------------------------------------------------------------------

struc 	AF_transBltLin_s
srcOfs                  AF_int32	?
dstLeft                 AF_int32	?
dstTop                  AF_int32	?
width                   AF_int32	?
height                  AF_int32	?
op						AF_int32	?
transparent				AF_int32	?
ends  	AF_transBltLin_s

AF_transBltLin = (AF_transBltLin_s PTR DS:ESI)
