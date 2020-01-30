;****************************************************************************
;*
;*				 VESA BIOS Extensions/Accelerator Functions
;*								Version 2.0 DRAFT
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
;* Developed by: SciTech Software
;*
;* Language:     80386 Assembler (TASM ideal mode)
;* Environment:  IBM PC 32 bit Protected Mode.
;*
;* Description:  Macros and type definitions for VBE/AF
;*
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
typedef AF_flt32	dword

typedef	AF_pattern	AF_uint8
typedef	AF_stipple	AF_uint16
typedef AF_color	AF_uint32

ifndef	__VXD__
true				=	1
false				=	0
endif

;----------------------------------------------------------------------------
; Attribute flags
;----------------------------------------------------------------------------

; Flags for combining with video modes during mode set

afDontClear			=	8000h
afLinearBuffer		=	4000h
afMultiBuffer       =   2000h
afVirtualScroll		=	1000h
afRefreshCtrl		=	0800h
afStereo			=	0400h
afModeMask			=   03FFh

; Flags for the mode attributes returned by GetModeInfo

afHaveMultiBuffer   =   0001h
afHaveVirtualScroll	=	0002h
afHaveBankedBuffer	=	0004h
afHaveLinearBuffer	=	0008h
afHaveAccel2D		=	0010h
afHaveDualBuffers	=   0020h
afHaveHWCursor		=	0040h
afHave8BitDAC		=	0080h
afNonVGAMode		=	0100h
afHaveDoubleScan	=	0200h
afHaveInterlaced	=	0400h
afHaveTripleBuffer	=	0800h
afHaveStereo		=	1000h
afHaveROP2			=	2000h

; Extra flags for the controller Attributes field (all the above flags
; are valid also)

afHaveHWStereoSync	=	4000h
afHaveEVCStereoSync	=	8000h

; Flags for hardware video capabilities returned by GetModeInfo

afVideoXInterp		=	00000001h
afVideoYInterp		=	00000002h
afVideoRGB332		=	00000004h
afVideoRGB555		=	00000008h
afVideoRGB565		=	00000010h
afVideoRGB888		=	00000020h
afVideoARGB8888		=	00000040h
afVideoYUV9			=	00000080h
afVideoYUV12		=	00000100h
afVideoYUV411		=	00000200h
afVideoYUV422		=	00000400h
afVideoYUV444		=	00000800h
afVideoYCrCb422		=	00001000h
afVideoYUYV			=	10000000h
afVideoYVYU			=	20000000h
afVideoUYVY			=	40000000h
afVideoVYUY			=	80000000h

; Flags for CRTC information block for refresh rate control

afDoubleScan		=	0001h
afInterlaced		=	0002h
afHSyncNeg			=	0004h
afVSyncNeg			=	0008h

; Defines for left and right buffers for SetActiveBuffer

afLeftBuffer		=	00000h		; Default is for left buffer
afRightBuffer		=	08000h		; Flag to make right buffer active

; Mix codes for rendering functions

enum AF_mixmodes {
	AF_FORE_MIX		=	00h
	AF_REPLACE_MIX	=	00h
	AF_AND_MIX
	AF_OR_MIX
	AF_XOR_MIX
	AF_NOP_MIX
	AF_R2_BLACK 	= 	10h
	AF_R2_NOTMERGESRC
	AF_R2_MASKNOTSRC
	AF_R2_NOTCOPYSRC
	AF_R2_MASKSRCNOT
	AF_R2_NOT
	AF_R2_XORSRC
	AF_R2_NOTMASKSRC
	AF_R2_MASKSRC
	AF_R2_NOTXORSRC
	AF_R2_NOP
	AF_R2_MERGENOTSRC
	AF_R2_COPYSRC
	AF_R2_MERGESRCNOT
	AF_R2_MERGESRC
	AF_R2_WHITE
	}

; Flags for DPMS states supported by the graphics card

afDPMS_standbyFlag	= 01h
afDPMS_suspendFlag	= 02h
afDPMS_offFlag		= 04h

; States values to pass for VBE/PM set state function call

afDPMS_on			= 0
afDPMS_standby		= 1
afDPMS_suspend		= 2
afDPMS_off			= 4

; Flags for level of DDC supported by the hardware

afDDC_DDC1Flag		= 01h
afDDC_DDC2Flag		= 02h
afDDC_blankFlag		= 04h

;----------------------------------------------------------------------------
; VBE/AF Graphics Accelerator C functions structure (VBE/AF 2.0+)
;----------------------------------------------------------------------------

struc	AF_cFuncs_s
GetVideoModeInfo                     cptr       ?
SetVideoMode                         cptr       ?
RestoreTextMode                      cptr       ?
GetClosestPixelClock                 cptr       ?
SaveRestoreState                     cptr       ?
SetDisplayStart                      cptr       ?
SetActiveBuffer                      cptr       ?
SetVisibleBuffer                     cptr       ?
GetDisplayStartStatus                cptr       ?
EnableStereoMode                     cptr       ?
SetPaletteData                       cptr       ?
SetGammaCorrectData                  cptr       ?
SetBank                              cptr       ?
SetCursor                            cptr       ?
SetCursorPos                         cptr       ?
SetCursorColor                       cptr       ?
ShowCursor                           cptr       ?
WaitTillIdle                         cptr       ?
EnableDirectAccess                   cptr       ?
DisableDirectAccess                  cptr       ?
SetMix                               cptr       ?
Set8x8MonoPattern                    cptr       ?
Set8x8ColorPattern                   cptr       ?
Use8x8ColorPattern                   cptr       ?
SetLineStipple                       cptr       ?
SetLineStippleCount                  cptr       ?
SetClipRect                          cptr       ?
DrawScan                             cptr       ?
DrawPattScan                         cptr       ?
DrawColorPattScan                    cptr       ?
DrawScanList                         cptr       ?
DrawPattScanList                     cptr       ?
DrawColorPattScanList                cptr       ?
DrawRect                             cptr       ?
DrawPattRect                         cptr       ?
DrawColorPattRect                    cptr       ?
DrawLine                             cptr       ?
DrawStippleLine                      cptr       ?
DrawTrap                             cptr       ?
DrawTri                              cptr       ?
DrawQuad                             cptr       ?
PutMonoImage                         cptr       ?
PutMonoImageLin                      cptr       ?
PutMonoImageBM                       cptr       ?
BitBlt                               cptr       ?
BitBltSys                            cptr       ?
BitBltLin                            cptr       ?
BitBltBM                             cptr       ?
SrcTransBlt                          cptr       ?
SrcTransBltSys                       cptr       ?
SrcTransBltLin                       cptr       ?
SrcTransBltBM                        cptr       ?
DstTransBlt                          cptr       ?
DstTransBltSys                       cptr       ?
DstTransBltLin                       cptr       ?
DstTransBltBM                        cptr       ?
StretchBlt                           cptr       ?
StretchBltSys                        cptr       ?
StretchBltLin                        cptr       ?
StretchBltBM                         cptr       ?
SrcTransStretchBlt                   cptr       ?
SrcTransStretchBltSys                cptr       ?
SrcTransStretchBltLin                cptr       ?
SrcTransStretchBltBM                 cptr       ?
DstTransStretchBlt                   cptr       ?
DstTransStretchBltSys                cptr       ?
DstTransStretchBltLin                cptr       ?
DstTransStretchBltBM                 cptr       ?
SetVideoInput                        cptr       ?
SetVideoOutput                       cptr       ?
StartVideoFrame                      cptr       ?
EndVideoFrame                        cptr       ?

WaitTillIdle16						 cptr     	?
EnableDirectAccess16				 cptr      	?
DisableDirectAccess16 				 cptr      	?
SetCursorPos16						 cptr		?

GetConfigInfo						 cptr		?
GetCurrentMode                       cptr		?
SetVSyncWidth                        cptr		?
GetVSyncWidth                        cptr		?
GetBank                              cptr		?
GetVisibleBuffer                     cptr		?
GetDisplayStart                      cptr		?
SetDisplayStartAddr                  cptr		?
SetStereoDisplayStartAddr            cptr		?
GetDisplayStartAddr                  cptr		?
IsVSync                              cptr		?
WaitVSync                            cptr		?
GetActiveBuffer                      cptr		?
SetDrawBuffer                        cptr		?
GetPaletteData                       cptr		?
GetGammaCorrectData                  cptr		?
IsIdle								 cptr		?
IsDMAIdle                            cptr		?
WaitTillDMAIdle                      cptr		?

DrawPattTrap                         cptr		?
DrawColorPattTrap                    cptr		?
DrawLineInt                          cptr		?
DrawStippleLineInt                   cptr		?
PutMonoImageSys                      cptr		?

WaitTillIdle16Len					 ulong		?
EnableDirectAccess16Len				 ulong		?
DisableDirectAccess16Len			 ulong		?
SetCursorPos16Len					 ulong		?
ends    AF_cFuncs_s

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
LinearStridePad			AF_uint32	?
PCIVendorID             AF_uint16	?
PCIDeviceID				AF_uint16	?
PCISubSysVendorID		AF_uint16	?
PCISubSysID				AF_uint16	?
Checksum				AF_uint32	?
res2					AF_uint32 6 dup (?)
IOMemMaps				dptr 4 dup (?)
BankedMem				dptr		?
LinearMem				dptr		?
res3					AF_uint32 5 dup (?)
BufferEndX           	AF_uint32	?
BufferEndY           	AF_uint32	?
OriginOffset            AF_uint32	?
OffscreenOffset         AF_uint32	?
OffscreenStartY         AF_uint32	?
OffscreenEndY           AF_uint32	?
res4					AF_uint32 10 dup (?)
SetBank32Len            AF_uint32	?
SetBank32               cptr       	?
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
PlugAndPlayInit			cptr		?
OEMExt                  cptr       	?
SupplementalExt         cptr       	?
cFuncs					AF_cFuncs_s <>
ends    AF_devCtx_s

FIRST_AF_VEC    EQU GetVideoModeInfo
LAST_AF_VEC     EQU DstTransBltLin

FIRST_AF_VEC2   EQU OEMExt
LAST_AF_VEC2    EQU cFuncs.PutMonoImageSys

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
MaxBuffers              AF_uint16  	?
RedMaskSize             AF_uint8   	?
RedFieldPosition        AF_uint8	?
GreenMaskSize           AF_uint8	?
GreenFieldPosition      AF_uint8	?
BlueMaskSize            AF_uint8	?
BlueFieldPosition       AF_uint8	?
RsvdMaskSize           	AF_uint8	?
RsvdFieldPosition     	AF_uint8	?
MaxBytesPerScanLine		AF_uint16	?
MaxScanLineWidth		AF_uint16	?
LinBytesPerScanLine		AF_uint16	?
BnkMaxBuffers			AF_uint8	?
LinMaxBuffers           AF_uint8	?
LinRedMaskSize          AF_uint8	?
LinRedFieldPosition     AF_uint8	?
LinGreenMaskSize        AF_uint8	?
LinGreenFieldPosition   AF_uint8	?
LinBlueMaskSize         AF_uint8	?
LinBlueFieldPosition    AF_uint8	?
LinRsvdMaskSize         AF_uint8	?
LinRsvdFieldPosition    AF_uint8	?
MaxPixelClock			AF_uint32	?
VideoCapabilities		AF_uint32	?
VideoMinXScale			AF_uint16	?
VideoMinYScale          AF_uint16	?
VideoMaxXScale          AF_uint16	?
VideoMaxYScale          AF_uint16	?
reserved				uchar 76 dup (?)
ends    AF_modeInfo_s

AF_modeInfo = (AF_modeInfo_s PTR DS:EDI)

;----------------------------------------------------------------------------
; CRTC information block for refresh rate control
;----------------------------------------------------------------------------

struc	AF_CRTCInfo_s
HorizontalTotal			AF_uint16	?
HorizontalSyncStart     AF_uint16	?
HorizontalSyncEnd       AF_uint16	?
VerticalTotal           AF_uint16	?
VerticalSyncStart       AF_uint16	?
VerticalSyncEnd         AF_uint16	?
Flags					AF_uint8	?
PixelClock				AF_uint32	?
RefreshRate				AF_uint16	?
NumBuffers				AF_uint16	?
ends	AF_CRTCInfo_s

AF_CRTCInfo = (AF_CRTCInfo_s PTR DS:EDI)

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
; 16.16 fixed point vertex structure
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
