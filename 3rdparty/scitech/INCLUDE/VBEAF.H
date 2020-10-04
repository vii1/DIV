/****************************************************************************
*
*				 VESA BIOS Extensions/Accelerator Functions
*								Version 1.0
*
*                   Copyright (C) 1996 SciTech Software.
*                           All rights reserved.
*
* Filename:     $Workfile:   vbeaf.h  $
* Developed by:	SciTech Software
*
* Language:     ANSI C
* Environment:  IBM PC 32 bit Protected Mode.
*
* Description:  Header file for the VBE/AF Graphics Acclerator Driver API.
*
*				When this code is used to load the VBEAF.DRV driver file,
*				it will look for it in the following standard locations
*				in the following order:
*
*					1. C:\VBEAF.DRV	for DOS, Windows, OS/2
*					     /VBEAF.DRV for Unix
*                   2. VBEAF_PATH environment variable
*					3. Path passed to AF_loadDriver
*
*				The last location searched is to allow specific versions
*				of a driver file to be stored with a applications in case
*				an application needs a specific version for some obscure
*				reason.
*
* $Date:   22 Apr 1996 23:49:40  $ $Author:   KendallB  $
*
****************************************************************************/

#ifndef __VBEAF_H
#define __VBEAF_H

#ifndef	__DEBUG_H
#include "debug.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

/* Define the calling conventions for the code in this module */

#define	AFAPI  _ASMAPI			/* 'C' calling conventions always		*/

#pragma pack(1)

/* Type definitions for fundamental types */

typedef	char			AF_int8;	/* 8 bit signed value				*/
typedef short			AF_int16;	/* 16 bit signed value				*/
typedef long			AF_int32;	/* 32 bit signed value				*/
typedef	unsigned char	AF_uint8;	/* 8 bit unsigned value				*/
typedef unsigned short	AF_uint16;	/* 16 bit unsigned value			*/
typedef unsigned long	AF_uint32;	/* 32 bit unsigned value			*/
typedef	long			AF_fix32;	/* 16.16 signed fixed point format	*/

typedef AF_uint8 	AF_pattern;		/* Pattern array elements			*/
typedef AF_uint16 	AF_stipple;		/* 16 bit line stipple pattern 		*/
typedef AF_uint32 	AF_color;		/* Packed color values 				*/

/* VBE/AF Graphics Accelerator Driver structure.
 *
 * Internally in the structure there are members represented as pointers.
 * However when the driver file is first loaded, these values will actually
 * be offsets from the start of the loaded driver file, but the intial
 * call to InitDriver will 'fix-up' the pointers and turn them into
 * real pointers.
 */

typedef struct {
	/*------------------------------------------------------------------*/
	/* Device driver header block                                       */
	/*------------------------------------------------------------------*/

	char		Signature[12];		/* 'VBEAF.DRV\0' 12 byte signature	*/
	AF_uint32	Version;            /* Driver Interface Version (1.0)	*/
	AF_uint32	DriverRev;			/* Driver revision number			*/
	char		OemVendorName[80];	/* Vendor Name string				*/
	char		OemCopyright[80];	/* Vendor Copyright string			*/
	AF_int16	*AvailableModes;	/* Offset to supported mode table	*/
	AF_uint32	TotalMemory;		/* Amount of memory in Kb detected	*/
	AF_uint32	Attributes;			/* Driver attributes				*/
	AF_uint32   BankSize;			/* Bank size in Kb (4Kb or 64Kb)	*/
	AF_uint32   BankedBasePtr;		/* Physical addr of banked buffer	*/
	AF_uint32	LinearSize;			/* Linear buffer size in Kb			*/
	AF_uint32	LinearBasePtr;		/* Physical addr of linear buffer	*/
	AF_uint32	LinearGranularity;	/* Linear blt granularity in bytes	*/
	AF_uint16	*IOPortsTable;		/* Offset of I/O ports table		*/
	AF_uint32	IOMemoryBase[4];	/* Base address of I/O memory maps	*/
	AF_uint32	IOMemoryLen[4];		/* Length of I/O memory maps		*/
	AF_uint32   res1[10];			/* Reserved for future expansion	*/

	/*------------------------------------------------------------------*/
	/* Near pointers mapped by application for driver                   */
	/*------------------------------------------------------------------*/

	void		*IOMemMaps[4];		/* Pointers to mapped I/O memory 	*/
	void		*BankedMem;			/* Ptr to mapped banked video mem	*/
	void		*LinearMem;			/* Ptr to mapped linear video mem	*/

	/*------------------------------------------------------------------*/
	/* Important selectors allocated by application for driver 			*/
	/*------------------------------------------------------------------*/

	AF_uint32	Sel0000h;			/* 1Mb selector to entire first Mb	*/
	AF_uint32	Sel0040h;			/* Selector to segment at 0x0040:0	*/
	AF_uint32	SelA000h;			/* Selector to segment at 0xA000:0	*/
	AF_uint32	SelB000h;			/* Selector to segment at 0xB000:0	*/
	AF_uint32	SelC000h;			/* Selector to segment at 0xC000:0	*/

	/*------------------------------------------------------------------*/
	/* Device driver state variables 									*/
	/*------------------------------------------------------------------*/

	AF_uint32	BufferEndX;			/* Last X coord of each buffer		*/
	AF_uint32	BufferEndY;			/* Last Y coord of each buffer		*/
	AF_uint32	OriginOffset;		/* Current start of active page		*/
	AF_uint32   OffscreenOffset;	/* Start of offscreen memory area	*/
	AF_uint32   OffscreenStartY;	/* First Y coord of offscreen mem	*/
	AF_uint32	OffscreenEndY;		/* Last Y coord of offscreen mem	*/
	AF_uint32   res2[10];			/* Reserved for future expansion	*/

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
	void		*SetBank32;				/* 32 bit relocateable code		*/

	/*------------------------------------------------------------------*/
	/* REQUIRED callback functions provided by application 				*/
	/*------------------------------------------------------------------*/

	void		*Int86;					/* Issue real mode interrupt	*/
	void		*CallRealMode;			/* Call a real mode function	*/

	/*------------------------------------------------------------------*/
	/* Device driver functions 											*/
	/*------------------------------------------------------------------*/

	void		*InitDriver;			/* Initialise driver			*/
	void		*GetVideoModeInfo;		/* Get video mode information	*/
	void		*SetVideoMode;			/* Set a video mode				*/
	void		*RestoreTextMode;		/* Restore text mode operation	*/
	void		*SetBank;				/* Set framebuffer bank			*/
	void		*SetDisplayStart;		/* Set virtual display start	*/
	void		*SetActiveBuffer;		/* Set active output buffer		*/
	void		*SetVisibleBuffer;		/* Set Visible display buffer	*/
	void		*SetPaletteData;		/* Program palette data			*/
	void		*SetGammaCorrectData;	/* Program gamma correct'n data	*/
	void		*WaitTillIdle;			/* Wait till engine is idle		*/
	void		*EnableDirectAccess;	/* Enable direct mem access 	*/
	void		*DisableDirectAccess;	/* Disable direct mem access	*/
	void		*SetCursor;				/* Download hardware cursor		*/
	void		*SetCursorPos;			/* Set cursor position			*/
	void		*SetCursorColor;		/* Set cursor color				*/
	void		*ShowCursor;			/* Show/hide cursor				*/
	void		*SetMix;				/* Set ALU mix operations		*/
	void		*Set8x8MonoPattern;		/* Set 8x8 mono bitmap pattern	*/
	void		*Set8x8ColorPattern;	/* Set 8x8 color bitmap pattern	*/
	void		*SetLineStipple;		/* Set 16 bit line stipple		*/
	void		*SetClipRect;			/* Set clipping rectangle		*/
	void		*DrawScan;				/* Draw a solid scanline		*/
	void		*DrawPattScan;			/* Draw a patterned scanline	*/
	void		*DrawColorPattScan;		/* Draw color pattern scanline	*/
	void		*DrawScanList;			/* Draw list of solid scanlines	*/
	void		*DrawRect;				/* Draw a solid rectangle		*/
	void		*DrawPattRect;			/* Draw a patterned rectangle	*/
	void		*DrawColorPattRect;		/* Draw color pattern rectangle	*/
	void		*DrawLine;				/* Draw a solid line			*/
	void		*DrawStippleLine;		/* Draw a stippled line			*/
	void		*DrawTrap;				/* Draw a solid trapezoid		*/
	void		*DrawTri;				/* Draw a solid triangle		*/
	void		*DrawQuad;				/* Draw a solid quad			*/
	void		*PutMonoImage;			/* Display a monochrome bitmap	*/
	void		*BitBlt;				/* Blt screen to screen			*/
	void		*BitBltLin;				/* Linear source BitBlt			*/
	void		*SrcTransBlt;			/* Source transparent BitBlt	*/
	void		*SrcTransBltLin;		/* Linear source SrcTransBlt	*/
	void		*DstTransBlt;			/* Dest. transparent BitBlt 	*/
	void		*DstTransBltLin;		/* Linear source DstTransBlt	*/
	} AF_devCtx;

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
	AF_uint8    reserved[118];		/* Pad to 128 byte block size       */
	} AF_modeInfo;

#define	VBEAF_DRV		"VBEAF.DRV"	/* Name of driver file on disk 		*/
#define	VBEAF_PATH		"VBEAF_PATH"/* Name of environment variable		*/
#define	VBEAF_VERSION	0x100		/* Lowest version we can work with	*/

/* Flags for combining with video modes during mode set */

#define	afDontClear		0x8000		/* Dont clear display memory		*/
#define	afLinearBuffer	0x4000		/* Enable linear framebuffer mode	*/
#define afMultiBuffer   0x2000      /* Enable multi buffered mode       */
#define	afVirtualScroll	0x1000		/* Enable virtual scrolling			*/

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

/* Types of mix operations supported */

typedef enum {						/* Write mode operators				*/
	AF_REPLACE_MIX,					/* Replace mode						*/
	AF_AND_MIX,						/* AND mode							*/
	AF_OR_MIX,						/* OR mode							*/
	AF_XOR_MIX,						/* XOR mode							*/
	AF_NOP_MIX,						/* Destination pixel unchanged		*/
	} AF_mixModes;

/* Palette entry structure, always in 8 bits per primary format */

typedef struct {
	AF_uint8	blue;				/* Blue component of color			*/
	AF_uint8	green;				/* Green component of color			*/
	AF_uint8	red;				/* Blue component of color			*/
	AF_uint8	alpha;				/* Alpha or alignment byte			*/
	} AF_palette;

/* Hardware cursor structure */

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

/* Default locations to find the driver for different operating systems */

#define	AF_DRIVERDIR_DOS	"c:\\"		/* DOS, Windows and OS/2		*/
#define	AF_DRIVERDIR_UNIX	"/"			/* Unix							*/

#pragma pack()

/*-------------------------- Function Prototypes --------------------------*/

#ifdef  __cplusplus
extern "C" {            			/* Use "C" linkage when in C++ mode */
#endif

/* Function to load the VBEAF.DRV driver file and initialise it */

AF_devCtx	* AFAPI AF_loadDriver(const char *driverDir);
void		AFAPI AF_unloadDriver(AF_devCtx *drv);
AF_int32	AFAPI AF_status(void);
const char	* AFAPI AF_errorMsg(int status);

/* Function to return a 32 bit bank function for virtual linear buffering */

bool 		AFAPI AF_getBankFunc32(AF_devCtx *dc,int *codeLen,void **bankFunc);

/* The following provides a high level C based API to the accelerated
 * rendering functions. For maximum performance, you should make direct
 * calls to the accelerated rendering functions in assembler from your own
 * rendering routines.
 */

AF_int32	AFAPI AF_getVideoModeInfo(AF_devCtx *dc,AF_int16 mode,AF_modeInfo *modeInfo);
AF_int32	AFAPI AF_setVideoMode(AF_devCtx *dc,AF_int16 mode,AF_int32 *bytesPerLine,int numBuffers);
AF_int32	AFAPI AF_setVirtualVideoMode(AF_devCtx *dc,AF_int16 mode,AF_int32 virtualX,AF_int32 virtualY,AF_int32 *bytesPerLine,int numBuffers);
void		AFAPI AF_restoreTextMode(AF_devCtx *dc);
void		AFAPI AF_setDisplayStart(AF_devCtx *dc,AF_int32 x,AF_int32 y,AF_int32 waitVRT);
void		AFAPI AF_setActiveBuffer(AF_devCtx *dc,AF_int32 index);
void		AFAPI AF_setVisibleBuffer(AF_devCtx *dc,AF_int32 index,AF_int32 waitVRT);
void		AFAPI AF_setPaletteData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,AF_int32 index,AF_int32 waitVRT);
void		AFAPI AF_setGammaCorrectData(AF_devCtx *dc,AF_palette *pal,AF_int32 num,AF_int32 index);
void		AFAPI AF_setBank(AF_devCtx *dc,AF_int32 bank);
void		AFAPI AF_waitTillIdle(AF_devCtx *dc);
void		AFAPI AF_enableDirectAccess(AF_devCtx *dc);
void		AFAPI AF_disableDirectAccess(AF_devCtx *dc);
void		AFAPI AF_setCursor(AF_devCtx *dc,AF_cursor *cursor);
void		AFAPI AF_setCursorPos(AF_devCtx *dc,AF_int32 x,AF_int32 y);
void		AFAPI AF_setCursorColor(AF_devCtx *dc,AF_uint8 red,AF_uint8 green,AF_uint8 blue);
void		AFAPI AF_showCursor(AF_devCtx *dc,AF_int32 visible);
void		AFAPI AF_setMix(AF_devCtx *dc,AF_int32 foreMix,AF_int32 backMix);
void		AFAPI AF_set8x8MonoPattern(AF_devCtx *dc,AF_pattern *pattern);
void		AFAPI AF_setLineStipple(AF_devCtx *dc,AF_stipple stipple);
void		AFAPI AF_setClipRect(AF_devCtx *dc,AF_int32 minx,AF_int32 miny,AF_int32 maxx,AF_int32 maxy);
void		AFAPI AF_drawScan(AF_devCtx *dc,AF_int32 color,AF_int32 y,AF_int32 x1,AF_int32 x2);
void		AFAPI AF_drawPattScan(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 y,AF_int32 x1,AF_int32 x2);
void		AFAPI AF_drawScanList(AF_devCtx *dc,AF_color color,AF_int32 y,AF_int32 length,AF_int16 *scans);
void		AFAPI AF_drawRect(AF_devCtx *dc,AF_color color,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
void		AFAPI AF_drawPattRect(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height);
void		AFAPI AF_drawLine(AF_devCtx *dc,AF_color color,AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2);
void		AFAPI AF_drawStippleLine(AF_devCtx *dc,AF_color foreColor,AF_color backColor,AF_fix32 x1,AF_fix32 y1,AF_fix32 x2,AF_fix32 y2);
void		AFAPI AF_drawTrap(AF_devCtx *dc,AF_color color,AF_int32 y,AF_int32 count,AF_fix32 x1,AF_fix32 x2,AF_fix32 slope1,AF_fix32 slope2);
void		AFAPI AF_drawTri(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,AF_fxpoint *v3,AF_fix32 xOffset,AF_fix32 yOffset);
void		AFAPI AF_drawQuad(AF_devCtx *dc,AF_color color,AF_fxpoint *v1,AF_fxpoint *v2,AF_fxpoint *v3,AF_fxpoint *v4,AF_fix32 xOffset,AF_fix32 yOffset);
void		AFAPI AF_putMonoImage(AF_devCtx *dc,AF_int32 foreColor,AF_int32 backColor,AF_int32 x,AF_int32 y,AF_int32 byteWidth,AF_int32 height,AF_uint8 *image);
void		AFAPI AF_bitBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op);
void		AFAPI AF_bitBltLin(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 width,AF_int32 height,AF_int32 op);
void		AFAPI AF_srcTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
void		AFAPI AF_srcTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 width,AF_int32 height,AF_int32 op,AF_color transparent);
void		AFAPI AF_dstTransBlt(AF_devCtx *dc,AF_int32 left,AF_int32 top,AF_int32 width,AF_int32 height,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 op,AF_color transparent);
void		AFAPI AF_dstTransBltLin(AF_devCtx *dc,AF_int32 srcOfs,AF_int32 dstLeft,AF_int32 dstTop,AF_int32 width,AF_int32 height,AF_int32 op,AF_color transparent);

#ifdef  __cplusplus
}                       			/* End of "C" linkage for C++   	*/
#endif

#endif  /* __VBEAF_H */
