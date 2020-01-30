/****************************************************************************
*
*				 VESA BIOS Extensions/Accelerator Functions
*                               Version 2.0 DRAFT
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Developed by:	SciTech Software
*
* Language:     ANSI C
* Environment:  IBM PC 32 bit Protected Mode.
*
* Description:  Header file for the VBE/AF Graphics Acclerator Driver API.
*
*				When this code is used to load a valid VBE/AF driver file,
*				it will look for all available driver files in the
*				following standard locations:
*
*					MS-DOS:    	c:\vbeaf
*					Windows:	x:\windows\system\vbeaf
*					OS/2:		x:\os2\drivers\vbeaf
*					Unix:       /usr/lib/vbeaf
*
*				where x: is the drive where the operating system is
*				installed, and the 'windows' or 'os2' directories are the
*				standard names for these operating systems (the OS functions
*				are used to determine the exact location).
*
*				In the case where the drivers are not located in the
*				standard location, the user or the install program can
*				set the VBEAF_PATH environment variable to point to the
*				directory where the driver files are located. Hence a user
*				with multiple operating system's installed may put a single
*				copy of the drivers for their system in the c:\vbeaf
*				directory and tell the loader to look for the files in that
*				directory rather than the standard location.
*
*				Note that VBE/AF 2.0 provides support for multiple
*				independant controllers and full plug and play support, and
*				hence allows for multiple VBE/AF drivers to be present in
*				a single system, and the proper driver will be matched to
*				the card's found in the system given the PCI configuration
*				ID's for the cards. In the case of VLBus systems, only a
*				single driver should be present in the VBE/AF driver
*				directory and it should be named VBEAF.DRV.
*
*
****************************************************************************/

#ifndef __VBEAF_H
#define __VBEAF_H

#ifndef	__SCITECH_H
#include "scitech.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

/* Define the calling conventions for the code in this module */

#ifdef	__16BIT__
#define	_FAR_	far
#else
#define	_FAR_
#endif
#define	AFAPI _FAR_ _ASMAPI			/* 'C' calling conventions always	*/

#pragma pack(1)

/* Type definitions for fundamental types */

typedef	char			AF_int8;	/* 8 bit signed value				*/
typedef short			AF_int16;	/* 16 bit signed value				*/
typedef long			AF_int32;	/* 32 bit signed value				*/
typedef	unsigned char	AF_uint8;	/* 8 bit unsigned value				*/
typedef unsigned short	AF_uint16;	/* 16 bit unsigned value			*/
typedef unsigned long	AF_uint32;	/* 32 bit unsigned value			*/
typedef	long			AF_fix32;	/* 16.16 signed fixed point format	*/
typedef	long			AF_zfix32;	/* 4.28 signed fixed point format	*/
typedef	float			AF_flt32;	/* 32 bit floating point format		*/

typedef AF_uint8 		AF_pattern;	/* Pattern array elements			*/
typedef AF_uint32 		AF_stipple;	/* 16 bit line stipple pattern 		*/
typedef AF_uint32 		AF_color;	/* Packed color values 				*/

/* Default locations to find the driver for different operating systems */

#define	VBEAF_DRV		"VBEAF.DRV"	/* Name of driver file on disk 		*/
#define	VBEAF_PATH		"VBEAF_PATH"/* Name of environment variable		*/
#define	AF_DRIVERDIR_DOS	"c:\\"	/* DOS, Windows and OS/2			*/
#define	AF_DRIVERDIR_UNIX	"/"		/* Unix								*/

/* Flags for combining with video modes during mode set */

#define	afDontClear		0x8000		/* Dont clear display memory		*/
#define	afLinearBuffer	0x4000		/* Enable linear framebuffer mode	*/
#define afMultiBuffer   0x2000      /* Enable multi buffered mode       */
#define	afVirtualScroll	0x1000		/* Enable virtual scrolling			*/
#define	afRefreshCtrl	0x0800		/* Use refresh rate control			*/
#define	afStereo		0x0400		/* Use hardware stereo				*/
#define afModeMask		0x03FF		/* Mask for VBE mode numbers		*/

/* Types of mix operations supported. The foreground mix is the mix code
 * used for all normal operations, and for all monochrome blit and pattern
 * fill operations where the pattern bit is set to a 1, and can be any of
 * the mix codes. The background mix is the mix code used only for
 * monochrome blit and pattern fill operations where the pattern bit is
 * set to a 0, and can only be AF_FORE_MIX or AF_NOP_MIX. Essentially
 * setting the background mix to AF_NOP_MIX enables a 'transparent'
 * monochrome pattern fill, and setting the mix to AF_FORE_MIX enables
 * an 'opaque' monochrome pattern fill.
 */

typedef enum {						/* Write mode operators				*/
	AF_FORE_MIX = 0,				/* Use the foreground mix value		*/
	AF_REPLACE_MIX = 0,				/* Replace mode						*/
	AF_AND_MIX,						/* AND mode							*/
	AF_OR_MIX,						/* OR mode							*/
	AF_XOR_MIX,						/* XOR mode							*/
	AF_NOP_MIX,						/* Destination pixel unchanged		*/

	/* ROP2 foreground mix codes. We define these codes as being between
	 * the source and destination pixels for blt's, between the foreground
	 * or background color and the destination pixels for solid and
	 * mono pattern fills and between the pattern pixels and the destination
	 * pixels for color pattern fills. It is up to the driver to do any
	 * translation between these generic ROP codes and each
	 * different type of hardware operation internally. Next to each code
	 * is the equivalent Microsoft defined ROP3 code between source and
	 * destination pixels.
	 */
	AF_R2_BLACK = 0x10,        		/* 0       							*/
	AF_R2_NOTMERGESRC,      		/* DSon     						*/
	AF_R2_MASKNOTSRC,       		/* DSna     						*/
	AF_R2_NOTCOPYSRC,       		/* Sn       						*/
	AF_R2_MASKSRCNOT,       		/* SDna     						*/
	AF_R2_NOT,              		/* Dn       						*/
	AF_R2_XORSRC,           		/* DSx      						*/
	AF_R2_NOTMASKSRC,       		/* DSan     						*/
	AF_R2_MASKSRC,          		/* DSa      						*/
	AF_R2_NOTXORSRC,        		/* DSxn     						*/
	AF_R2_NOP,						/* D        						*/
	AF_R2_MERGENOTSRC,      		/* DSno     						*/
	AF_R2_COPYSRC,          		/* S        						*/
	AF_R2_MERGESRCNOT,      		/* SDno     						*/
	AF_R2_MERGESRC,         		/* DSo      						*/
	AF_R2_WHITE,            		/* 1        						*/
	} AF_mixModes;

/* Flags for DPMS states supported by the VBE/PM interface */

#define	afDPMS_standbyFlag	0x1
#define	afDPMS_suspendFlag	0x2
#define	afDPMS_offFlag		0x4

/* DPMS state values to pass for AF_PMsetState() function call */

typedef enum {
	afDPMS_on			= 0,
	afDPMS_standby		= 1,
	afDPMS_suspend		= 2,
	afDPMS_off			= 4,
	} UV_DPMSStatesType;

/* Flags for level of DDC supported by the hardware */

#define	afDDC_DDC1Flag		0x01   	/* DDC1 is supported				*/
#define	afDDC_DDC2Flag		0x02    /* DDC2 is supported				*/
#define	afDDC_blankFlag		0x04    /* Screen blaned for transfer		*/
#define	afDDC_DDC2A2Flag	0x08    /* DDC2 at address A2h is supported	*/
#define	afDDC_DDC2A6Flag	0x10    /* DDC2 at address A6h is supported	*/
#define afDDC_multiplePorts	0x20	/* Multiple monitor ports supported	*/

/* Flags for the level of SCI supported by the hardware */

#define	afSCI_writeSCL		0x01
#define	afSCI_writeSDA		0x02
#define	afSCI_readSCL		0x04
#define	afSCI_readSDA		0x08

/* Video mode information block */

typedef struct {
	AF_uint16	Attributes;			/* Mode attributes					*/
	AF_uint16	XResolution;		/* Horizontal resolution in pixels	*/
	AF_uint16	YResolution;		/* Vertical resolution in pixels	*/
	AF_uint16   BytesPerScanLine;	/* Bytes per horizontal scan line	*/
	AF_uint16	BitsPerPixel;		/* Bits per pixel					*/
	AF_uint16   MaxBuffers;         /* Maximum num. of display buffers  */

	/* RGB pixel format info */
	AF_uint8    RedMaskSize;        /* Size of direct color red mask    */
	AF_uint8    RedFieldPosition;   /* Bit posn of lsb of red mask      */
	AF_uint8    GreenMaskSize;      /* Size of direct color green mask  */
	AF_uint8    GreenFieldPosition; /* Bit posn of lsb of green mask    */
	AF_uint8    BlueMaskSize;       /* Size of direct color blue mask   */
	AF_uint8    BlueFieldPosition;  /* Bit posn of lsb of blue mask     */
	AF_uint8    RsvdMaskSize;       /* Size of direct color res mask    */
	AF_uint8    RsvdFieldPosition;	/* Bit posn of lsb of res mask      */

	/* Virtual buffer dimensions */
	AF_uint16	MaxBytesPerScanLine;/* Maximum bytes per scan line		*/
	AF_uint16	MaxScanLineWidth;	/* Maximum pixels per scan line		*/

	/* VBE/AF 2.0 extension information */
	AF_uint16	LinBytesPerScanLine;/* Bytes per scanline               */
	AF_uint8   	BnkMaxBuffers; 		/* Maximum num. of buffers (banked)	*/
	AF_uint8    LinMaxBuffers; 		/* Maximum num. of buffers (linear)	*/
	AF_uint8    LinRedMaskSize;     /* Size of direct color red mask    */
	AF_uint8    LinRedFieldPosition;/* Bit posn of lsb of red mask      */
	AF_uint8    LinGreenMaskSize;   /* Size of direct color green mask  */
	AF_uint8    LinGreenFieldPosition;/* Bit posn of lsb of green mask	*/
	AF_uint8    LinBlueMaskSize;    /* Size of direct color blue mask   */
	AF_uint8    LinBlueFieldPosition;/* Bit posn of lsb of blue mask    */
	AF_uint8    LinRsvdMaskSize;    /* Size of direct color res mask    */
	AF_uint8    LinRsvdFieldPosition;/* Bit posn of lsb of res mask   	*/
	AF_uint32	MaxPixelClock;		/* Maximum pixel clock for mode		*/

	/* VBE/AF 2.0 hardware video information */
	AF_uint32	VideoCapabilities;	/* Hardware video capabilities flags*/
	AF_uint16	VideoMinXScale;		/* Minimum X scale factor (1/value)	*/
	AF_uint16	VideoMinYScale;		/* Minimum Y scale factor (1/value)	*/
	AF_uint16	VideoMaxXScale;		/* Maximum X scale factor 			*/
	AF_uint16	VideoMaxYScale;		/* Maximum Y scale factor 			*/

	AF_uint8    reserved[76];		/* Pad to 128 byte block size       */
	} AF_modeInfo;

/* Flags for the mode attributes returned by GetModeInfo */

#define afHaveMultiBuffer   0x0001  /* Mode supports multi buffering    */
#define	afHaveVirtualScroll	0x0002	/* Mode supports virtual scrolling	*/
#define	afHaveBankedBuffer	0x0004	/* Mode supports banked framebuffer	*/
#define	afHaveLinearBuffer	0x0008	/* Mode supports linear framebuffer	*/
#define	afHaveAccel2D		0x0010	/* Mode supports 2D acceleration	*/
#define	afHaveDualBuffers	0x0020	/* Mode uses dual buffers			*/
#define	afHaveHWCursor		0x0040	/* Mode supports a hardware cursor	*/
#define	afHave8BitDAC		0x0080	/* Mode uses an 8 bit palette DAC	*/
#define	afNonVGAMode		0x0100	/* Mode is a NonVGA mode			*/
#define	afHaveDoubleScan	0x0200	/* Mode supports double scanning	*/
#define	afHaveInterlaced	0x0400	/* Mode supports interlacing		*/
#define	afHaveTripleBuffer	0x0800	/* Mode supports triple buffering	*/
#define	afHaveStereo		0x1000	/* Mode supports stereo LCD glasses	*/
#define	afHaveROP2			0x2000	/* Hardware supports ROP2 mix codes	*/

/* Extra flags for the controller Attributes field (all the above flags
 * are valid also).
 */

#define afHaveHWStereoSync	0x4000	/* Hardware stereo signalling 		*/
#define	afHaveEVCStereoSync	0x8000	/* HW stereo sync via EVC connector	*/

/* Flags for hardware video capabilities returned by GetModeInfo */

#define	afVideoXInterp	0x00000001	/* Supports X interpolation			*/
#define	afVideoYInterp	0x00000002	/* Supports Y interpolation			*/
#define afVideoRGB332	0x00000004	/* Supports RGB 3:3:2 input format	*/
#define afVideoRGB555	0x00000008	/* Supports RGB 5:5:5 input format	*/
#define afVideoRGB565	0x00000010	/* Supports RGB 5:6:5 input format	*/
#define afVideoRGB888	0x00000020	/* Supports RGB 8:8:8 input format	*/
#define afVideoARGB8888	0x00000040	/* Supports RGB 8:8:8:8 input format*/
#define	afVideoYUV9		0x00000080	/* Supports YUV9 input format		*/
#define	afVideoYUV12	0x00000100	/* Supports YUV12 input format		*/
#define	afVideoYUV411	0x00000200	/* Supports YUV411 input format		*/
#define	afVideoYUV422	0x00000400  /* Supports YUV422 input format		*/
#define	afVideoYUV444	0x00000800  /* Supports YUV444 input format		*/
#define	afVideoYCrCb422	0x00001000  /* Supports YCrCb422 input format	*/
#define	afVideoYUYV		0x10000000	/* Specifies the YUV pixel format	*/
#define	afVideoYVYU		0x20000000
#define	afVideoUYVY		0x40000000
#define	afVideoVYUY		0x80000000

/* Flags for hardware stretch blitting */

#define	afStretchXInterp	0x0001	/* Interpolate along the X axis		*/
#define	afStretchYInterp	0x0002	/* Interpolate along the Y axis		*/

/* CRTC information block for refresh rate control */

typedef struct {
	AF_uint16	HorizontalTotal;	/* Horizontal total (pixels)		*/
	AF_uint16   HorizontalSyncStart;/* Horizontal sync start position	*/
	AF_uint16	HorizontalSyncEnd;	/* Horizontal sync end position		*/
	AF_uint16	VerticalTotal;		/* Vertical Total (lines)			*/
	AF_uint16	VerticalSyncStart;	/* Vertical sync start position		*/
	AF_uint16	VerticalSyncEnd;	/* Vertical sync end position		*/
	AF_uint8	Flags;				/* Initialisation flags for mode	*/
	AF_uint32	PixelClock;			/* Pixel clock in units of Hz		*/
	AF_uint16	RefreshRate;		/* Expected refresh rate in .01Hz 	*/
	AF_uint16	NumBuffers;			/* Number of display buffers		*/
	} AF_CRTCInfo;

/* Definitions for CRTC information block flags */

#define	afDoubleScan		0x0001	/* Enable double scanned mode		*/
#define	afInterlaced		0x0002	/* Enable interlaced mode			*/
#define	afHSyncNeg			0x0004	/* Horizontal sync is negative		*/
#define	afVSyncNeg			0x0008	/* Vertical sync is negative		*/

/* Defines for left and right buffers for SetActiveBuffer */

#define	afLeftBuffer		0x0000	/* Default is for left buffer		*/
#define	afRightBuffer		0x8000	/* Flag to make right buffer active	*/

/* Palette entry structure, always in 8 bits per primary format */

typedef struct {
	AF_uint8	blue;				/* Blue component of color			*/
	AF_uint8	green;				/* Green component of color			*/
	AF_uint8	red;				/* Blue component of color			*/
	AF_uint8	alpha;				/* Alpha or alignment byte			*/
	} AF_palette;

/* Hardware cursor structure.
 *
 * The definition of the AND mask, XOR mask and the pixels that will
 * appear on the screen is as follows:
 *
 *		AND XOR
 *  	 0   0	= transparent (color from screen memory)
 *		 0   1 	= invert (complement of color from screen memory)
 *		 1   0 	= color 0 (black or background color)
 *		 1   1 	= color 1 (white or foreground color)
 */

typedef struct {
	AF_uint32   xorMask[32];		/* Cursor XOR mask					*/
	AF_uint32	andMask[32];		/* Cursor AND mask					*/
	AF_uint32	hotx;				/* Cursor X coordinate hot spot		*/
	AF_uint32	hoty;				/* Cursor Y coordinate hot spot		*/
	} AF_cursor;

/* Integer coordinates passed to DrawLineList */

typedef struct {
	int	x;
	int	y;
	} AF_point;

/* 16.16 fixed point coordinates passed for triangle and quad fills */

typedef struct {
	AF_fix32	x;
	AF_fix32	y;
	} AF_fxpoint;

/* Parameter block for DrawTrap */

typedef struct {
	AF_uint32	y;
	AF_uint32	count;
	AF_fix32	x1;
	AF_fix32	x2;
	AF_fix32	slope1;
	AF_fix32	slope2;
	} AF_trap;

/* Macros to convert between integer and 32 bit fixed point format */

#define	AF_FIX_1			0x10000L
#define	AF_FIX_2			0x20000L
#define	AF_FIX_HALF			0x08000L
#define	AF_TOFIX(i)			((long)(i) << 16)
#define AF_FIXTOINT(f)		((int)((f) >> 16))
#define	AF_FIXROUND(f) 		((int)(((f) + MGL_FIX_HALF) >> 16))

/* DPMI register structure used in calls to Int86 and CallRealMode */

typedef struct {
	long	edi;
	long	esi;
	long	ebp;
	long	reserved;
	long	ebx;
	long	edx;
	long	ecx;
	long	eax;
	short	flags;
	short	es,ds,fs,gs,ip,cs,sp,ss;
	} AF_DPMI_regs;

/* Flags returned by AF_status to indicate driver load status */

typedef enum {
	afOK,				/* No error										*/
	afNotDetected,		/* Graphics hardware not detected				*/
	afDriverNotFound,	/* Driver file not found						*/
	afCorruptDriver,	/* File loaded not a driver file				*/
	afLoadMem,			/* Not enough memory to load driver				*/
	afOldVersion,		/* Driver file is an older version				*/
	afMemMapError,		/* Could not map physical memory areas			*/
	afMaxError,
	} AF_errorType;

/****************************************************************************
REMARKS:
Structure returned by GetConfigInfo, which contains configuration
information about the installed graphics hardware.

Note:	The dwSize member is intended for future compatibility, and
		should be set to the size of the structure as defined in this
		header file. Future drivers will be compatible with older software
		by examiming this value.

HEADER:
inertia/graphics.h

MEMBERS:
dwSize			- Set to size of structure in bytes
ChipsetFamily	- Name of graphics chipset family
ChipsetName		- Name of graphics chipset name
DACName			- Name of DAC on graphics card
ClockName		- Name of clock on graphics card
****************************************************************************/
typedef struct {
	AF_int32	dwSize;
	char		ChipsetFamily[80];
	char		ChipsetName[80];
	char    	DACName[80];
	char		ClockName[80];
	} GA_configInfo;

/****************************************************************************
REMARKS:
Generic graphics buffer parameter block. This structure defines a generic
buffer in offscreen video memory, and is passed to the driver to allocate
such buffers for rendering operations. The Offset member is the offset
of the start of the buffer in video memory, local depth buffer or stencil
buffer memory depending on the buffer type (some hardware has separate
buffer regions for framebuffer, depth buffer and texture memory etc). The
Stride member defines the stride of the buffer in bytes, while the Width and
Height members define the dimensions of the buffer in logical pixel units.

Note:	All buffers are in packed pixel format, and the values of the Offset
		and Stride members must adhere to the format restrictions defined in
		the GA_modeInfo structure for the buffer type being enabled.

Note:	The dwSize member is intended for future compatibility, and
		should be set to the size of the structure as defined in this
		header file. Future drivers will be compatible with older software
		by examiming this value.

HEADER:
inertia/graphics.h

MEMBERS:
dwSize     	- Set to size of structure in bytes
Offset    	- Buffer starting address
Stride		- Stride of the buffer in bytes (bytes for a line of data)
Width      	- Width of buffer in pixels
Height     	- Height of the buffer in pixels
****************************************************************************/
typedef struct {
	AF_int32   	dwSize;
	AF_int32    Offset;
	AF_int32    Stride;
	AF_int32    Width;
	AF_int32    Height;
	} GA_buffer;

/* Forward reference to AF_devCtx */

typedef struct _AF_devCtx	AF_devCtx;

typedef void _FAR_ *AF_monoCursor;
typedef void _FAR_ *AF_colorCursor;
typedef void _FAR_ *AF_bltFx;

/* VBE/AF Graphics Accelerator C functions structure (VBE/AF 2.0+)
 *
 * This structure contains C callable versions of all VBE/AF functions,
 * and is the preferred method of calling the VBE/AF driver in new code.
 */

typedef struct {
	/*------------------------------------------------------------------*/
	/* C callable device driver functions (VBE/AF 2.0+)       			*/
	/*------------------------------------------------------------------*/

	AF_int32	(AFAPI *GetVideoModeInfo)(AF_devCtx *dc,AF_int32 mode,AF_modeInfo *modeInfo);
	AF_int32	(AFAPI *SetVideoMode)(AF_devCtx *dc,AF_int32 mode,AF_int32 virtualX,AF_int32 virtualY,AF_int32 *bytesPerLine,int numBuffers,AF_CRTCInfo *crtc);
	void		(AFAPI *RestoreTextMode)(AF_devCtx *dc);
	AF_uint32	(AFAPI *GetClosestPixelClock)(AF_devCtx *dc,AF_int32 mode,AF_uint32 pixelClock);
	void		(AFAPI *SaveRestoreState)(AF_devCtx *dc,int subfunc,void *saveBuf);
	void		(AFAPI *SetDisplayStart)(AF_devCtx *dc,AF_int32 x,AF_int32 y,AF_int32 waitVRT);
	void		(AFAPI *SetActiveBuffer)(AF_devCtx *dc,AF_int32 index);
	void		(AFAPI *SetVisibleBuffer)(AF_devCtx *dc,AF_int32 index,AF_int32 waitVRT);
	ibool		(AFAPI *GetDisplayStartStatus)(AF_devCtx *dc);
	void		(AFAPI *EnableStereoMode)(AF_devCtx *dc,ibool enable);
	void		(AFAPI *SetPaletteData)(AF_devCtx *dc,AF_palette *pal,AF_int32 num,AF_int32 index,AF_int32 waitVRT);
	void		(AFAPI *SetGammaCorrectData)(AF_devCtx *dc,AF_palette *pal,AF_int32 num,AF_int32 index);

	/*------------------------------------------------------------------*/
	/* Hardware cursor functions 										*/
	/*------------------------------------------------------------------*/

	void		(AFAPI *SetBank)(AF_devCtx *dc,AF_int32 bank);
	void		(AFAPI *SetCursor)(AF_devCtx *dc,AF_cursor *cursor);
	void		(AFAPI *SetCursorPos)(AF_devCtx *dc,AF_int32 x,AF_int32 y);
	void		(AFAPI *SetCursorColor)(AF_devCtx *dc,AF_uint32 red,AF_uint32 green,AF_uint32 blue);
	void		(AFAPI *ShowCursor)(AF_devCtx *dc,AF_int32 visible);

	/*------------------------------------------------------------------*/
	/* 2D rendering functions 											*/
	/*------------------------------------------------------------------*/

	void		(AFAPI *WaitTillIdle)(AF_devCtx *dc);
	void		(AFAPI *EnableDirectAccess)(AF_devCtx *dc);
	void		(AFAPI *DisableDirectAccess)(AF_devCtx *dc);
	void		(AFAPI *SetMix)(AF_devCtx *dc,AF_int32 foreMix,AF_int32 backMix);
	void		(AFAPI *Set8x8MonoPattern)(AF_devCtx *dc,AF_pattern *pattern);
	void		(AFAPI *Set8x8ColorPattern)(AF_devCtx *dc,int index,AF_color *pattern);
	void		(AFAPI *Use8x8ColorPattern)(AF_devCtx *dc,int index);
	void		(AFAPI *SetLineStipple)(AF_devCtx *dc,AF_stipple stipple);
	void		(AFAPI *SetLineStippleCount)(AF_devCtx *dc,AF_uint32 count);
	void		(AFAPI *SetClipRect)(AF_devCtx *dc,AF_int32 minx,AF_int32 miny,AF_int32 maxx,AF_int32 maxy);
	void		(AFAPI *DrawScan)(AF_devCtx *dc,AF_int32 color,AF_int32 y,AF_int32 x1,AF_int32 x2);
	void		(AFAPI *DrawPattScan)(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 y,AF_int32 x1,AF_int32 x2);
	void		(AFAPI *DrawColorPattScan)(AF_devCtx *dc,AF_int32 y,AF_int32 x1,AF_int32 x2);
	void		(AFAPI *DrawScanList)(AF_devCtx *dc,AF_color color,AF_int32 y,AF_int32 length,AF_int16 *scans);
	void		(AFAPI *DrawPattScanList)(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_int32 y,AF_int32 length,AF_int16 *scans);
	void		(AFAPI *DrawColorPattScanList)(AF_devCtx *dc,AF_int32 y,AF_int32 length,AF_int16 *scans);
	void		(AFAPI *DrawRect)(AF_devCtx *dc,AF_color color,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
	void		(AFAPI *DrawPattRect)(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
	void		(AFAPI *DrawColorPattRect)(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
	void		(AFAPI *DrawLine)(AF_devCtx *dc,AF_color color,AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2);
	void		(AFAPI *DrawStippleLine)(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2);
	void		(AFAPI *DrawTrap)(AF_devCtx *dc,AF_color color,AF_trap *trap);
	void		(AFAPI *DrawTri)(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,AF_fxpoint *v3,AF_fix32 xOffset,AF_fix32 yOffset);
	void		(AFAPI *DrawQuad)(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,AF_fxpoint *v3,AF_fxpoint *v4,AF_fix32 xOffset,AF_fix32 yOffset);
	void		(AFAPI *PutMonoImage)(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 dstX,AF_int32 dstY,AF_int32 byteWidth,AF_int32 srcX,AF_int32 srcY,AF_int32 width,AF_int32 height,AF_uint8 *image);
	void		(AFAPI *PutMonoImageLin)(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 dstX,AF_int32 dstY,AF_int32 byteWidth,AF_int32 srcX,AF_int32 srcY,AF_int32 width,AF_int32 height,AF_int32 imageOfs);
	void		(AFAPI *PutMonoImageBM)(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 dstX,AF_int32 dstY,AF_int32 byteWidth,AF_int32 srcX,AF_int32 srcY,AF_int32 width,AF_int32 height,AF_int32 imagePhysAddr);
	void		(AFAPI *BitBlt)(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op);
	void		(AFAPI *BitBltSys)(AF_devCtx *dc,void *srcAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op);
	void		(AFAPI *BitBltLin)(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op);
	void		(AFAPI *BitBltBM)(AF_devCtx *dc,AF_int32 srcPhysAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op);
	void		(AFAPI *SrcTransBlt)(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *SrcTransBltSys)(AF_devCtx *dc,void *srcAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *SrcTransBltLin)(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *SrcTransBltBM)(AF_devCtx *dc,AF_int32 srcPhysAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransBlt)(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransBltSys)(AF_devCtx *dc,void *srcAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransBltLin)(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransBltBM)(AF_devCtx *dc,AF_int32 srcPhysAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
	void		(AFAPI *StretchBlt)(AF_devCtx *dc,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op);
	void		(AFAPI *StretchBltSys)(AF_devCtx *dc,void *srcAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op);
	void		(AFAPI *StretchBltLin)(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op);
	void		(AFAPI *StretchBltBM)(AF_devCtx *dc,AF_int32 srcPhysAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op);
	void		(AFAPI *SrcTransStretchBlt)(AF_devCtx *dc,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);
	void		(AFAPI *SrcTransStretchBltSys)(AF_devCtx *dc,void *srcAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);
	void		(AFAPI *SrcTransStretchBltLin)(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);
	void		(AFAPI *SrcTransStretchBltBM)(AF_devCtx *dc,AF_int32 srcPhysAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransStretchBlt)(AF_devCtx *dc,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransStretchBltSys)(AF_devCtx *dc,void *srcAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransStretchBltLin)(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);
	void		(AFAPI *DstTransStretchBltBM)(AF_devCtx *dc,AF_int32 srcPhysAddr,AF_int32 srcPitch,AF_int32 srcLeft,AF_int32 srcTop,AF_int32 srcWidth,AF_int32 srcHeight,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 dstWidth,AF_int32 dstHeight,AF_int32 flags,AF_int32 op,AF_color transparent);

	/*------------------------------------------------------------------*/
	/* Hardware video functions				              				*/
	/*------------------------------------------------------------------*/

	void		(AFAPI *SetVideoInput)(AF_devCtx *dc,AF_int32 width,AF_int32 height,AF_int32 format);
	void *		(AFAPI *SetVideoOutput)(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
	void		(AFAPI *StartVideoFrame)(AF_devCtx *dc);
	void		(AFAPI *EndVideoFrame)(AF_devCtx *dc);

	/* Special 16-bit callable direct access functions for 16-bit Windows
	 * display drivers.
	 */
	void		_FAR_ *WaitTillIdle16;
	void		_FAR_ *EnableDirectAccess16;
	void		_FAR_ *DisableDirectAccess16;
	void		_FAR_ *SetCursorPos16;

	/* New generic device driver functions from the forthcoming Interia
	 * specification.
	 */
	void		(AFAPI *GetConfigInfo)(AF_devCtx *dc,GA_configInfo *info);
	AF_int32	(AFAPI *GetCurrentMode)(AF_devCtx *dc);
	void		(AFAPI *SetVSyncWidth)(AF_devCtx *dc,AF_int32 width);
	AF_int32	(AFAPI *GetVSyncWidth)(AF_devCtx *dc);
	AF_int32	(AFAPI *GetBank)(AF_devCtx *dc);
	AF_int32	(AFAPI *GetVisibleBuffer)(AF_devCtx *dc);
	void		(AFAPI *GetDisplayStart)(AF_devCtx *dc,AF_int32 *x,AF_int32 *y);
	void		(AFAPI *SetDisplayStartAddr)(AF_devCtx *dc,AF_int32 offset,AF_int32 waitVRT);
	void		(AFAPI *SetStereoDisplayStartAddr)(AF_devCtx *dc,AF_int32 leftOffset,AF_int32 rightOffset,AF_int32 waitVRT);
	AF_int32	(AFAPI *GetDisplayStartAddr)(AF_devCtx *dc);
	AF_int32	(AFAPI *IsVSync)(AF_devCtx *dc);
	void		(AFAPI *WaitVSync)(AF_devCtx *dc);
	AF_int32	(AFAPI *GetActiveBuffer)(AF_devCtx *dc);
	void		(AFAPI *SetDrawBuffer)(AF_devCtx *dc,GA_buffer *drawBuf);
	void		(AFAPI *GetPaletteData)(AF_devCtx *dc,AF_palette *pal);
	void		(AFAPI *GetGammaCorrectData)(AF_devCtx *dc,AF_palette *pal);
	AF_int32	(AFAPI *IsIdle)(AF_devCtx *dc);
	AF_int32	(AFAPI *IsDMAIdle)(AF_devCtx *dc);
	void		(AFAPI *WaitTillDMAIdle)(AF_devCtx *dc);

	/* New 2D drawing functions from the forthcoming Inertia specification.
	 */
	void		(AFAPI *DrawPattTrap)(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_trap *trap);
	void		(AFAPI *DrawColorPattTrap)(AF_devCtx *dc,AF_trap *trap);
	void		(AFAPI *DrawLineInt)(AF_devCtx *dc,AF_color color,AF_int32 x1,AF_int32 y1,AF_int32 x2,AF_int32 y2,AF_int32 drawLast);
	void		(AFAPI *DrawStippleLineInt)(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_int32 x1,AF_int32 y1,AF_int32 x2,AF_int32 y2,AF_int32 drawLast);
	void		(AFAPI *PutMonoImageSys)(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 dstX,AF_int32 dstY,AF_int32 width,AF_int32 height,AF_int32 byteWidth,AF_uint8 *image);

	/* Lengths of special Windows driver functions */
	ulong		WaitTillIdle16Len;
	ulong		EnableDirectAccess16Len;
	ulong		DisableDirectAccess16Len;
	ulong		SetCursorPos16Len;
	} AF_cFuncs;

/* VBE/AF Graphics Accelerator Driver structure.
 *
 * Internally in the structure there are members represented as pointers.
 * However when the driver file is first loaded, these values will actually
 * be offsets from the start of the loaded driver file, but the intial
 * call to InitDriver will 'fix-up' the pointers and turn them into
 * real pointers.
 */

struct _AF_devCtx {
	/*------------------------------------------------------------------*/
	/* Device driver header block                                       */
	/*------------------------------------------------------------------*/

	char		Signature[12];		/* 'VBEAF.DRV\0' 12 byte signature	*/
	AF_uint32   Version;            /* Driver Interface Version (2.0)   */
	AF_uint32	DriverRev;			/* Driver revision number			*/
	char		OemVendorName[80];	/* Vendor Name string				*/
	char		OemCopyright[80];	/* Vendor Copyright string			*/
	AF_int16	_FAR_ *AvailableModes;/* Offset to supported mode table	*/
	AF_uint32	TotalMemory;		/* Amount of memory in Kb detected	*/
	AF_uint32	Attributes;			/* Driver attributes				*/
	AF_uint32   BankSize;			/* Bank size in Kb (4Kb or 64Kb)	*/
	AF_uint32   BankedBasePtr;		/* Physical addr of banked buffer	*/
	AF_uint32	LinearSize;			/* Linear buffer size in Kb			*/
	AF_uint32	LinearBasePtr;		/* Physical addr of linear buffer	*/
	AF_uint32	LinearGranularity;	/* Linear blt granularity in bytes	*/
	AF_uint16	_FAR_ *IOPortsTable;	/* Offset of I/O ports table		*/
	AF_uint32	IOMemoryBase[4];	/* Base address of I/O memory maps	*/
	AF_uint32	IOMemoryLen[4];		/* Length of I/O memory maps		*/
	AF_uint32	LinearStridePad;	/* Linear blt stride pad in bytes	*/

	/*------------------------------------------------------------------*/
	/* PCI device ID's for plug and play initialisation                 */
	/*------------------------------------------------------------------*/

	AF_uint16   PCIVendorID;        /* Vendor ID (-1 for VLB)           */
	AF_uint16   PCIDeviceID;        /* Device ID (-1 for VLB)           */
	AF_uint16   PCISubSysVendorID;  /* Subsystem Vendor ID (-1 for all) */
	AF_uint16   PCISubSysID;        /* Subsystem ID (-1 for all)        */
	AF_uint32   Checksum;           /* Checksum for header block        */
	AF_uint32   res2[6];            /* Reserved for future expansion    */

	/*------------------------------------------------------------------*/
	/* Near pointers mapped by application for driver                   */
	/*------------------------------------------------------------------*/

	void		_FAR_ *IOMemMaps[4];	/* Pointers to mapped I/O memory 	*/
	void		_FAR_ *BankedMem;		/* Ptr to mapped banked video mem	*/
	void		_FAR_ *LinearMem;		/* Ptr to mapped linear video mem	*/
	AF_uint32	res3[5];			/* Reserved for future expansion	*/

	/*------------------------------------------------------------------*/
	/* Device driver state variables 									*/
	/*------------------------------------------------------------------*/

	AF_uint32	BufferEndX;			/* Last X coord of each buffer		*/
	AF_uint32	BufferEndY;			/* Last Y coord of each buffer		*/
	AF_uint32	OriginOffset;		/* Current start of active page		*/
	AF_uint32   OffscreenOffset;	/* Start of offscreen memory area	*/
	AF_uint32   OffscreenStartY;	/* First Y coord of offscreen mem	*/
	AF_uint32	OffscreenEndY;		/* Last Y coord of offscreen mem	*/
	AF_uint32   res4[10];			/* Reserved for future expansion	*/

	/*------------------------------------------------------------------*/
	/* Relocateable 32 bit bank switch routine, needed for framebuffer	*/
	/* virtualisation under Windows with DVA.386/VFLATD.386. This 		*/
	/* function *MUST* program the bank with IO mapped registers, as 	*/
	/* when the function is called there is no way to provide access to	*/
	/* the devices memory mapped registers (because there is no way to	*/
	/* for it to gain access to a copy of this AF_devCtx block). For 	*/
	/* devices that only have memory mapped registers, this vector 		*/
	/* *MUST* be NULL indicating that this is not supported. However	*/
	/* all these devices all have a real linear framebuffer anyway, 	*/
	/* so the virtualisation services will not be needed.				*/
	/*------------------------------------------------------------------*/

	AF_uint32	SetBank32Len;			/* Length of 32 bit code		*/
	void		_FAR_ *SetBank32;			/* 32 bit relocateable code		*/

	/*------------------------------------------------------------------*/
	/* Callback functions provided by application for the driver. All	*/
	/* new drivers should be developed to not require access to the 	*/
	/* BIOS in order to fully support multiple display adapters in a	*/
	/* single system. If the operating system cannot provide these 		*/
	/* services these entry points should be set to NULL for VBE/AF 	*/
	/* 2.0 drivers, and the driver initialisation function will fail	*/
	/* if the driver requires access to the real mode VBE.				*/
	/*------------------------------------------------------------------*/

	void		_FAR_ *Int86;				/* Issue real mode interrupt	*/
	void		_FAR_ *CallRealMode;		/* Call a real mode function	*/

	/*------------------------------------------------------------------*/
	/* Assembler callable device driver functions (VBE/AF 1.0+)       	*/
	/* These function entry points are obsolete with VBE/AF 2.0 and 	*/
	/* later drivers, and are provided for backwards compatibility. 	*/
	/* All new code should be written to use the new C based entry 		*/
	/* points for maximum performance.									*/
	/*------------------------------------------------------------------*/

	void		_FAR_ *InitDriver;			/* Initialise driver			*/
	void		_FAR_ *GetVideoModeInfo;	/* Get video mode information	*/
	void		_FAR_ *SetVideoMode;		/* Set a video mode				*/
	void		_FAR_ *RestoreTextMode;		/* Restore text mode operation	*/
	void		_FAR_ *SetBank;				/* Set framebuffer bank			*/
	void		_FAR_ *SetDisplayStart;		/* Set virtual display start	*/
	void		_FAR_ *SetActiveBuffer;		/* Set active output buffer		*/
	void		_FAR_ *SetVisibleBuffer;	/* Set Visible display buffer	*/
	void		_FAR_ *SetPaletteData;		/* Program palette data			*/
	void		_FAR_ *SetGammaCorrectData;	/* Program gamma correct'n data	*/
	void		_FAR_ *WaitTillIdle;		/* Wait till engine is idle		*/
	void		_FAR_ *EnableDirectAccess;	/* Enable direct mem access 	*/
	void		_FAR_ *DisableDirectAccess;	/* Disable direct mem access	*/
	void		_FAR_ *SetCursor;			/* Download hardware cursor		*/
	void		_FAR_ *SetCursorPos;		/* Set cursor position			*/
	void		_FAR_ *SetCursorColor;		/* Set cursor color				*/
	void		_FAR_ *ShowCursor;			/* Show/hide cursor				*/
	void		_FAR_ *SetMix;				/* Set ALU mix operations		*/
	void		_FAR_ *Set8x8MonoPattern;	/* Set 8x8 mono bitmap pattern	*/
	void		_FAR_ *Set8x8ColorPattern;	/* Set 8x8 color bitmap pattern	*/
	void		_FAR_ *SetLineStipple;		/* Set 16 bit line stipple		*/
	void		_FAR_ *SetClipRect;			/* Set clipping rectangle		*/
	void		_FAR_ *DrawScan;			/* Draw a solid scanline		*/
	void		_FAR_ *DrawPattScan;		/* Draw a patterned scanline	*/
	void		_FAR_ *DrawColorPattScan;	/* Draw color pattern scanline	*/
	void		_FAR_ *DrawScanList;		/* Draw list of solid scanlines	*/
	void		_FAR_ *DrawRect;			/* Draw a solid rectangle		*/
	void		_FAR_ *DrawPattRect;		/* Draw a patterned rectangle	*/
	void		_FAR_ *DrawColorPattRect;	/* Draw color pattern rectangle	*/
	void		_FAR_ *DrawLine;			/* Draw a solid line			*/
	void		_FAR_ *DrawStippleLine;		/* Draw a stippled line			*/
	void		_FAR_ *DrawTrap;			/* Draw a solid trapezoid		*/
	void		_FAR_ *DrawTri;				/* Draw a solid triangle		*/
	void		_FAR_ *DrawQuad;			/* Draw a solid quad			*/
	void		_FAR_ *PutMonoImage;		/* Display a monochrome bitmap	*/
	void		_FAR_ *BitBlt;				/* Blt screen to screen			*/
	void		_FAR_ *BitBltLin;			/* Linear source BitBlt			*/
	void		_FAR_ *SrcTransBlt;			/* Source transparent BitBlt	*/
	void		_FAR_ *SrcTransBltLin;		/* Linear source SrcTransBlt	*/
	void		_FAR_ *DstTransBlt;			/* Dest. transparent BitBlt 	*/
	void		_FAR_ *DstTransBltLin;		/* Linear source DstTransBlt	*/

	/*------------------------------------------------------------------*/
	/* Assembler callable function for plug and play initialisation		*/
	/*------------------------------------------------------------------*/

	void        _FAR_ *PlugAndPlayInit;	/* Plug and Play init function  */

	/*------------------------------------------------------------------*/
	/* OEM extensions and supplemental functions callouts				*/
	/*------------------------------------------------------------------*/

	void 		_FAR_ *OEMExt;			/* OEM extensions callout		*/
	void		_FAR_ *SupplementalExt;	/* Supplemental functions 		*/

	/*------------------------------------------------------------------*/
	/* C callable rendering functions (VBE/AF 2.0+)						*/
	/*------------------------------------------------------------------*/

	AF_cFuncs	cFuncs;
	};

/* Generic typedef for a VBE/AF internal file handler */

typedef ulong AF_FILE;

#pragma pack()

/*-------------------------- Function Prototypes --------------------------*/

#ifdef  __cplusplus
extern "C" {            			/* Use "C" linkage when in C++ mode */
#endif

/* Generic VBE/AF file handling functions. If you define NO_FILEIO
 * when you compile the VBEAF.C module, the File IO functions will not be
 * compiled and you can provide your own OS specific versions. Generally
 * the VBE/AF module will contain support for as many OS'es as possible.
 */

AF_FILE		AFAPI AF_openFile(char *filename);
long 		AFAPI AF_getFileSize(AF_FILE f);
long 		AFAPI AF_readFile(AF_FILE f,void *buf,long size);
void 		AFAPI AF_closeFile(AF_FILE f);

/* Function to load the VBE/AF driver file and initialise it */

AF_devCtx	* AFAPI AF_loadDriver(const char *driverDir);
ulong		AFAPI AF_getDriverSize(void);
void		AFAPI AF_unloadDriver(AF_devCtx *drv);
AF_int32	AFAPI AF_status(void);
const char	* AFAPI AF_errorMsg(int status);

/* Utility functions to call the OEM and supplemental callouts */

void 		AFAPI AF_OEMExt(AF_devCtx *dc,AF_DPMI_regs *regs);
void 		AFAPI AF_SupplementalExt(AF_devCtx *dc,AF_DPMI_regs *regs);

/* Utility functions for VBE/PM supplemental specification */

int			AFAPI AF_PMdetect(AF_devCtx *dc,AF_int16 *supportedStates);
void		AFAPI AF_PMsetState(AF_devCtx *dc,AF_int16 state);
int			AFAPI AF_PMgetState(AF_devCtx *dc);

/* Utility functions for VBE/DDC supplemental specification */

int			AFAPI AF_DDCdetect(AF_devCtx *dc,AF_uint8 *transferTime,AF_uint8 *attributes);
int			AFAPI AF_DDCreadEDID(AF_devCtx *dc,AF_int16 block,AF_uint8 *edid,AF_int32 monitorPort);
int			AFAPI AF_DDCgetEDID_A2Size(AF_devCtx *dc,AF_int32 monitorPort);
int			AFAPI AF_DDCreadEDID_A2(AF_devCtx *dc,AF_int16 block,AF_uint8 *edid,AF_int32 monitorPort);
int			AFAPI AF_DDCgetEDID_A2Size(AF_devCtx *dc,AF_int32 monitorPort);
int			AFAPI AF_DDCreadEDID_A6(AF_devCtx *dc,AF_int16 block,AF_uint8 *edid,AF_int32 monitorPort);

/* Utility functions for VBE/SCI supplemental specification */

int			AFAPI AF_SCIdetect(AF_devCtx *dc,AF_uint8 *capabilities,AF_int32 *numMonitorPorts);
void		AFAPI AF_SCIbegin(AF_devCtx *dc,AF_int32 monitorPort);
void		AFAPI AF_SCIwriteSCL(AF_devCtx *dc,AF_int32 monitorPort,AF_int32 bit);
void		AFAPI AF_SCIwriteSDA(AF_devCtx *dc,AF_int32 monitorPort,AF_int32 bit);
int			AFAPI AF_SCIreadSCL(AF_devCtx *dc,AF_int32 monitorPort);
int			AFAPI AF_SCIreadSDA(AF_devCtx *dc,AF_int32 monitorPort);
void		AFAPI AF_SCIend(AF_devCtx *dc,AF_int32 monitorPort);

/* Utility function to return a 32 bit bank function for virtual linear buffering */

ibool 		AFAPI AF_getBankFunc32(AF_devCtx *dc,int *codeLen,void **bankFunc);

/* Functions to set a video mode. These must be called rather than using the
 * direct function calls in case we need to call our loaded VxD to set the
 * mode for us (since it needs to make calls to the BIOS). Once NoBIOS is
 * supported in our drivers, this will no longer be necessary.
 */

AF_int32 	AFAPI AF_setVideoMode(AF_devCtx *dc,AF_uint32 mode,AF_int32 virtualX,AF_int32 virtualY,AF_int32 *bytesPerLine,AF_int32 numBuffers,AF_CRTCInfo *crtc);
void 		AFAPI AF_restoreTextMode(AF_devCtx *dc);

/* C macros to VBE/AF 2.0 C callable functions */

#define AF_getVideoModeInfo(dc,mode,modeInfo)																			(dc)->cFuncs.GetVideoModeInfo(dc,mode,modeInfo)
#define AF_getClosestPixelClock(dc,mode,pixelClock) 																	(dc)->cFuncs.GetClosestPixelClock(dc,mode,pixelClock)
#define AF_saveRestoreState(dc,subfunc,saveBuf)																			(dc)->cFuncs.SaveRestoreState(dc,subfunc,saveBuf)
#define AF_setDisplayStart(dc,x,y,waitVRT)																				(dc)->cFuncs.SetDisplayStart(dc,x,y,waitVRT)
#define AF_setActiveBuffer(dc,index) 																					(dc)->cFuncs.SetActiveBuffer(dc,index)
#define	AF_setVisibleBuffer(dc,index,waitVRT) 																			(dc)->cFuncs.SetVisibleBuffer(dc,index,waitVRT)
#define AF_getDisplayStartStatus(dc) 																					(dc)->cFuncs.GetDisplayStartStatus(dc)
#define AF_enableStereoMode(dc,enable) 																					(dc)->cFuncs.EnableStereoMode(dc,enable)
#define AF_setPaletteData(dc,pal,num,index,waitVRT) 																	(dc)->cFuncs.SetPaletteData(dc,pal,num,index,waitVRT)
#define AF_setGammaCorrectData(dc,pal,num,index) 																		(dc)->cFuncs.SetGammaCorrectData(dc,pal,num,index)
#define AF_setBank(dc,bank) 																							(dc)->cFuncs.SetBank(dc,bank)
#define AF_setCursor(dc,cursor) 																						(dc)->cFuncs.SetCursor(dc,cursor)
#define	AF_setCursorPos(dc,x,y) 																						(dc)->cFuncs.SetCursorPos(dc,x,y)
#define AF_setCursorColor(dc,red,green,blue) 																			(dc)->cFuncs.SetCursorColor(dc,red,green,blue)
#define AF_showCursor(dc,visible)																						(dc)->cFuncs.ShowCursor(dc,visible)
#define AF_waitTillIdle(dc)																								(dc)->cFuncs.WaitTillIdle(dc)
#define AF_enableDirectAccess(dc)																						(dc)->cFuncs.EnableDirectAccess(dc)
#define AF_disableDirectAccess(dc)																						(dc)->cFuncs.DisableDirectAccess(dc)
#define AF_setMix(dc,foreMix,backMix)																					(dc)->cFuncs.SetMix(dc,foreMix,backMix)
#define AF_set8x8MonoPattern(dc,pattern)																				(dc)->cFuncs.Set8x8MonoPattern(dc,pattern)
#define AF_set8x8ColorPattern(dc,index,pattern)																			(dc)->cFuncs.Set8x8ColorPattern(dc,index,pattern)
#define AF_use8x8ColorPattern(dc,index)																					(dc)->cFuncs.Use8x8ColorPattern(dc,index)
#define AF_setLineStipple(dc,stipple)																					(dc)->cFuncs.SetLineStipple(dc,stipple)
#define AF_setLineStippleCount(dc,count)																				(dc)->cFuncs.SetLineStippleCount(dc,count)
#define AF_setClipRect(dc,minx,miny,maxx,maxy)																			(dc)->cFuncs.SetClipRect(dc,minx,miny,maxx,maxy)
#define AF_drawScan(dc,color,y,x1,x2)																					(dc)->cFuncs.DrawScan(dc,color,y,x1,x2)
#define AF_drawPattScan(dc,foreColor,backColor,y,x1,x2)																	(dc)->cFuncs.DrawPattScan(dc,foreColor,backColor,y,x1,x2)
#define AF_drawColorPattScan(dc,y,x1,x2)																				(dc)->cFuncs.DrawColorPattScan(dc,y,x1,x2)
#define AF_drawScanList(dc,color,y,length,scans)																		(dc)->cFuncs.DrawScanList(dc,color,y,length,scans)
#define AF_drawPattScanList(dc,foreColor,backColor,y,length,scans)														(dc)->cFuncs.DrawPattScanList(dc,foreColor,backColor,y,length,scans)
#define AF_drawColorPattScanList(dc,y,length,scans)																		(dc)->cFuncs.DrawColorPattScanList(dc,y,length,scans)
#define AF_drawRect(dc,color,left,top,width,height)																		(dc)->cFuncs.DrawRect(dc,color,left,top,width,height)
#define AF_drawPattRect(dc,foreColor,backColor,left,top,width,height)													(dc)->cFuncs.DrawPattRect(dc,foreColor,backColor,left,top,width,height)
#define AF_drawColorPattRect(dc,left,top,width,height)																	(dc)->cFuncs.DrawColorPattRect(dc,left,top,width,height)
#define AF_drawLine(dc,color,x1,y1,x2,y2)																				(dc)->cFuncs.DrawLine(dc,color,x1,y1,x2,y2)
#define AF_drawStippleLine(dc,foreColor,backColor,x1,y1,x2,y2)															(dc)->cFuncs.DrawStippleLine(dc,foreColor,backColor,x1,y1,x2,y2)
#define AF_drawTrap(dc,color,trap)																						(dc)->cFuncs.DrawTrap(dc,color,trap)
#define AF_drawTri(dc,color,v1,v2,v3,xOffset,yOffset)																	(dc)->cFuncs.DrawTri(dc,color,v1,v2,v3,xOffset,yOffset)
#define AF_drawQuad(dc,color,v1,v2,v3,v4,xOffset,yOffset)																(dc)->cFuncs.DrawQuad(dc,color,v1,v2,v3,v4,xOffset,yOffset)
#define AF_putMonoImage(dc,foreColor,backColor,dstX,dstY,byteWidth,srcX,srcY,width,height,image)						(dc)->cFuncs.PutMonoImage(dc,foreColor,backColor,dstX,dstY,byteWidth,srcX,srcY,width,height,image)
#define AF_putMonoImageLin(dc,foreColor,backColor,dstX,dstY,byteWidth,srcX,srcY,width,height,imageOfs)					(dc)->cFuncs.PutMonoImageLin(dc,foreColor,backColor,dstX,dstY,byteWidth,srcX,srcY,width,height,imageOfs)
#define AF_putMonoImageBM(dc,foreColor,backColor,dstX,dstY,byteWidth,srcX,srcY,width,height,imagePhysAddr)				(dc)->cFuncs.PutMonoImageBM(dc,foreColor,backColor,dstX,dstY,byteWidth,srcX,srcY,width,height,imagePhysAddr)
#define AF_bitBlt(dc,left,top,width,height,dstLeft,dstTop,op)															(dc)->cFuncs.BitBlt(dc,left,top,width,height,dstLeft,dstTop,op)
#define AF_bitBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op)									(dc)->cFuncs.BitBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op)
#define AF_bitBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op)									(dc)->cFuncs.BitBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op)
#define AF_bitBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op)								(dc)->cFuncs.BitBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op)
#define AF_srcTransBlt(dc,left,top,width,height,dstLeft,dstTop,op,transparent)											(dc)->cFuncs.SrcTransBlt(dc,left,top,width,height,dstLeft,dstTop,op,transparent)
#define AF_srcTransBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)             	(dc)->cFuncs.SrcTransBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)
#define AF_srcTransBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)					(dc)->cFuncs.SrcTransBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)
#define AF_srcTransBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)             (dc)->cFuncs.SrcTransBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)
#define AF_dstTransBlt(dc,left,top,width,height,dstLeft,dstTop,op,transparent)											(dc)->cFuncs.DstTransBlt(dc,left,top,width,height,dstLeft,dstTop,op,transparent)
#define AF_dstTransBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)             	(dc)->cFuncs.DstTransBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)
#define AF_dstTransBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)                 (dc)->cFuncs.DstTransBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)
#define AF_dstTransBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)             (dc)->cFuncs.DstTransBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,width,height,dstLeft,dstTop,op,transparent)
#define AF_stretchBlt(dc,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)                  		(dc)->cFuncs.StretchBlt(dc,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)
#define AF_stretchBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)		(dc)->cFuncs.StretchBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)
#define AF_stretchBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)		(dc)->cFuncs.StretchBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)
#define AF_stretchBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)	(dc)->cFuncs.StretchBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)
#define AF_srcTransStretchBlt(dc,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)                  		(dc)->cFuncs.SrcTransStretchBlt(dc,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_srcTransStretchBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)		(dc)->cFuncs.SrcTransStretchBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_srcTransStretchBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)		(dc)->cFuncs.SrcTransStretchBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_srcTransStretchBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)	(dc)->cFuncs.SrcTransStretchBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_dstTransStretchBlt(dc,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)                  		(dc)->cFuncs.DstTransStretchBlt(dc,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_dstTransStretchBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)		(dc)->cFuncs.DstTransStretchBltSys(dc,srcAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_dstTransStretchBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)		(dc)->cFuncs.DstTransStretchBltLin(dc,srcOfs,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_dstTransStretchBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op)	(dc)->cFuncs.DstTransStretchBltBM(dc,srcPhysAddr,srcPitch,srcLeft,srcTop,srcWidth,srcHeight,dstLeft,dstTop,dstWidth,dstHeight,flags,op,transparent)
#define AF_setVideoInput(dc,width,height,format)                                                                        (dc)->cFuncs.SetVideoInput(dc,width,height,format)
#define AF_setVideoOutput(dc,left,top,width,height)                                                                     (dc)->cFuncs.SetVideoOutput(dc,left,top,width,height)
#define AF_startVideoFrame(dc)                                                                                          (dc)->cFuncs.StartVideoFrame(dc)
#define AF_endVideoFrame(dc)                                                                                            (dc)->cFuncs.EndVideoFrame(dc)

/* C interface to obsolete VBE/AF 1.0 assembler functions */

AF_int32	AFAPI AF10_getVideoModeInfo(AF_devCtx *dc,AF_int16 mode,AF_modeInfo *modeInfo);
AF_int32	AFAPI AF10_setVideoMode(AF_devCtx *dc,AF_int16 mode,AF_int32 virtualX,AF_int32 virtualY,AF_int32 *bytesPerLine,int numBuffers);
void		AFAPI AF10_restoreTextMode(AF_devCtx *dc);
void		AFAPI AF10_setDisplayStart(AF_devCtx *dc,AF_int32 x,AF_int32 y,AF_int32 waitVRT);
void		AFAPI AF10_setActiveBuffer(AF_devCtx *dc,AF_int32 index);
void		AFAPI AF10_setVisibleBuffer(AF_devCtx *dc,AF_int32 index,AF_int32 waitVRT);
void		AFAPI AF10_setPaletteData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,AF_int32 index,AF_int32 waitVRT);
void		AFAPI AF10_setGammaCorrectData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,AF_int32 index);
void		AFAPI AF10_setBank(AF_devCtx *dc,AF_int32 bank);
void		AFAPI AF10_waitTillIdle(AF_devCtx *dc);
void		AFAPI AF10_enableDirectAccess(AF_devCtx *dc);
void		AFAPI AF10_disableDirectAccess(AF_devCtx *dc);
void		AFAPI AF10_setCursor(AF_devCtx *dc,AF_cursor *cursor);
void		AFAPI AF10_setCursorPos(AF_devCtx *dc,AF_int32 x,AF_int32 y);
void		AFAPI AF10_setCursorColor(AF_devCtx *dc,AF_uint8 red,AF_uint8 green,AF_uint8 blue);
void		AFAPI AF10_showCursor(AF_devCtx *dc,AF_int32 visible);
void		AFAPI AF10_setMix(AF_devCtx *dc,AF_int32 foreMix,AF_int32 backMix);
void		AFAPI AF10_set8x8MonoPattern(AF_devCtx *dc,AF_pattern *pattern);
void		AFAPI AF10_set8x8ColorPattern(AF_devCtx *dc,AF_color *pattern);
void		AFAPI AF10_setLineStipple(AF_devCtx *dc,AF_stipple stipple);
void		AFAPI AF10_setClipRect(AF_devCtx *dc,AF_int32 minx,AF_int32 miny,AF_int32 maxx,AF_int32 maxy);
void		AFAPI AF10_drawScan(AF_devCtx *dc,AF_int32 color,AF_int32 y,AF_int32 x1,AF_int32 x2);
void		AFAPI AF10_drawPattScan(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 y,AF_int32 x1,AF_int32 x2);
void		AFAPI AF10_drawColorPattScan(AF_devCtx *dc,AF_int32 y,AF_int32 x1,AF_int32 x2);
void		AFAPI AF10_drawScanList(AF_devCtx *dc,AF_color color,AF_int32 y,AF_int32 length,AF_int16 *scans);
void		AFAPI AF10_drawRect(AF_devCtx *dc,AF_color color,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
void		AFAPI AF10_drawPattRect(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
void		AFAPI AF10_drawColorPattRect(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
void		AFAPI AF10_drawLine(AF_devCtx *dc,AF_color color,AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2);
void		AFAPI AF10_drawStippleLine(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2);
void		AFAPI AF10_drawTrap(AF_devCtx *dc,AF_color color,AF_trap *trap);
void		AFAPI AF10_drawTri(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,AF_fxpoint *v3,AF_fix32 xOffset,AF_fix32 yOffset);
void		AFAPI AF10_drawQuad(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,AF_fxpoint *v3,AF_fxpoint *v4,AF_fix32 xOffset,AF_fix32 yOffset);
void		AFAPI AF10_putMonoImage(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 x,AF_int32 y,AF_int32 byteWidth,AF_int32 height,AF_uint8 *image);
void		AFAPI AF10_bitBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op);
void		AFAPI AF10_bitBltLin(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 width,AF_int32 height,AF_int32 op);
void		AFAPI AF10_srcTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
void		AFAPI AF10_srcTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 width,AF_int32 height,AF_int32 op,AF_color transparent);
void		AFAPI AF10_dstTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
void		AFAPI AF10_dstTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 width,AF_int32 height,AF_int32 op,AF_color transparent);

#ifdef  __cplusplus
}                       			/* End of "C" linkage for C++   	*/
#endif

#endif  /* __VBEAF_H */
