/****************************************************************************
*
*			The SuperVGA Kit - UniVBE Software Development Kit
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
*
* Language:     ANSI C
* Environment:  IBM PC (MSDOS) Real Mode and 16/32 bit Protected Mode.
*
* Description:  Header file declaring the structures used to communicate
*               with a VESA VBE 1.x, 2.x or 3.x compliant BIOS.
*
*				Note that this module does automatic translation of all
*				information in the VBE_vgaInfo and VBE_modeInfo blocks so
*				that all pointers are converted to the native format,
*				insulating the application from dealing with a real
*				mode VBE 2.0+ implementation.
*
*               MUST be compiled in the LARGE or FLAT models.
*
*
****************************************************************************/

#ifndef __VESAVBE_H
#define __VESAVBE_H

#ifndef __PMODE_H
#include "pmode.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

/* Define the calling conventions for the code in this module */

#if		defined(SVGA_DLL) || defined(BUILD_DLL)

#ifdef	__WINDOWS32__
#define	VBEAPI	_DLLASM			/* 'C' calling conventions for Win32	*/
#elif	defined(__WINDOWS16__)
#define	VBEAPI	_DLLAPI			/* Pascal calling conventions for Win16	*/
#else
#define	VBEAPI  _ASMAPI			/* 'C' calling conventions for DOS		*/
#endif

#else

#define	VBEAPI  _ASMAPI			/* 'C' calling conventions for libs		*/

#endif

#pragma pack(1)

/* SuperVGA information block */

typedef struct {
	char    VESASignature[4];       /* 'VESA' 4 byte signature          */
    short   VESAVersion;            /* VBE version number               */
	char	*OemStringPtr;          /* Pointer to OEM string            */
	ulong	Capabilities;           /* Capabilities of video card       */
	ushort	*VideoModePtr;          /* Pointer to supported modes       */
	short   TotalMemory;            /* Number of 64kb memory blocks     */

	/* VBE 2.0 extension information */

	short	OemSoftwareRev;			/* OEM Software revision number		*/
	char	*OemVendorNamePtr;		/* Pointer to Vendor Name string	*/
	char	*OemProductNamePtr;		/* Pointer to Product Name string	*/
	char	*OemProductRevPtr;		/* Pointer to Product Revision str	*/
	char    reserved[222];          /* Pad to 256 byte block size       */
	char	OemDATA[256];			/* Scratch pad for OEM data			*/
	} VBE_vgaInfo;

/* Flags for the Capabilities field */

#define	vbe8BitDAC			0x0001	/* DAC width is switchable to 8 bit	*/
#define	vbeNonVGA			0x0002	/* Controller is non-VGA			*/
#define	vbeBlankRAMDAC		0x0004	/* Programmed DAC with blank bit	*/
#define vbeHWStereoSync		0x0008	/* Hardware stereo signalling		*/
#define	vbeEVCStereoSync	0x0010	/* HW stereo sync via EVC connector	*/

/* SuperVGA mode information block */

typedef struct {
	short   ModeAttributes;         /* Mode attributes                  */
	char    WinAAttributes;         /* Window A attributes              */
	char    WinBAttributes;         /* Window B attributes              */
	short   WinGranularity;         /* Window granularity in k          */
	short   WinSize;                /* Window size in k                 */
	ushort  WinASegment;            /* Window A segment                 */
	ushort  WinBSegment;            /* Window B segment                 */
	void	*WinFuncPtr;			/* Pointer to window function       */
	short   BytesPerScanLine;       /* Bytes per scanline               */
	short   XResolution;            /* Horizontal resolution            */
	short   YResolution;            /* Vertical resolution              */
	char    XCharSize;              /* Character cell width             */
	char    YCharSize;              /* Character cell height            */
	char    NumberOfPlanes;         /* Number of memory planes          */
	char    BitsPerPixel;           /* Bits per pixel                   */
	char    NumberOfBanks;          /* Number of CGA style banks        */
	char    MemoryModel;            /* Memory model type                */
	char    BankSize;               /* Size of CGA style banks          */
	char    NumberOfImagePages;     /* Number of images pages (Max)     */
	char    res1;                   /* Reserved                         */
	char    RedMaskSize;            /* Size of direct color red mask    */
	char    RedFieldPosition;       /* Bit posn of lsb of red mask      */
	char    GreenMaskSize;          /* Size of direct color green mask  */
	char    GreenFieldPosition;     /* Bit posn of lsb of green mask    */
	char    BlueMaskSize;           /* Size of direct color blue mask   */
	char    BlueFieldPosition;      /* Bit posn of lsb of blue mask     */
	char    RsvdMaskSize;           /* Size of direct color res mask    */
	char    RsvdFieldPosition;      /* Bit posn of lsb of res mask      */
	char    DirectColorModeInfo;    /* Direct color mode attributes     */

	/* VBE 2.0 extension information */
	ulong	PhysBasePtr;			/* Physical address for linear buf	*/
	ulong	res2;					/* Reserved							*/
	short   res3;					/* Reserved							*/

	/* VBE 3.0 extension information */
	short   LinBytesPerScanLine;    /* Bytes per scanline               */
	char    BnkNumberOfImagePages;  /* Number of images pages (banked)  */
	char    LinNumberOfImagePages;  /* Number of images pages (linear)  */
	char    LinRedMaskSize;         /* Size of direct color red mask    */
	char    LinRedFieldPosition;    /* Bit posn of lsb of red mask      */
	char    LinGreenMaskSize;       /* Size of direct color green mask  */
	char    LinGreenFieldPosition;  /* Bit posn of lsb of green mask    */
	char    LinBlueMaskSize;        /* Size of direct color blue mask   */
	char    LinBlueFieldPosition;   /* Bit posn of lsb of blue mask     */
	char    LinRsvdMaskSize;        /* Size of direct color res mask    */
	char    LinRsvdFieldPosition; 	/* Bit posn of lsb of res mask      */
	ulong	MaxPixelClock;			/* Maximum pixel clock				*/

	char    res4[190];        		/* Pad to 256 byte block size       */
	} VBE_modeInfo;

/* CRTC information block for refresh rate control */

typedef struct {
	ushort	HorizontalTotal;		/* Horizontal total (pixels)		*/
	ushort	HorizontalSyncStart;	/* Horizontal sync start position	*/
	ushort	HorizontalSyncEnd;		/* Horizontal sync end position		*/
	ushort	VerticalTotal;			/* Vertical Total (lines)			*/
	ushort	VerticalSyncStart;		/* Vertical sync start position		*/
	ushort	VerticalSyncEnd;		/* Vertical sync end position		*/
	uchar	Flags;					/* Initialisation flags for mode	*/
	ulong	PixelClock;				/* Pixel clock in units of Hz		*/
	ushort	RefreshRate;			/* Refresh rate in units of 0.01 Hz	*/
	} VBE_CRTCInfo;

/* Definitions for CRTC information block flags */

#define	vbeDoubleScan		0x0001	/* Enable double scanned mode		*/
#define	vbeInterlaced		0x0002	/* Enable interlaced mode			*/
#define	vbeHSyncNeg			0x0004	/* Horizontal sync is negative		*/
#define	vbeVSyncNeg			0x0008	/* Vertical sync is negative		*/

/* 32 bit protected mode info block */

typedef struct {
	short   setWindow;				/* Offset of Set Window call		*/
	short	setDisplayStart;		/* Offset of Set Display Start call	*/
	short   setPalette;				/* Offset of Set Primary Palette	*/
	short   IOPrivInfo;				/* Offset of I/O priveledge info	*/

	/* UniVBE extensions */
	long	extensionSig;			/* Identify extension (0xFBADFBAD)	*/
	long	setWindowLen;			/* Used to virtualise framebuffer	*/
	long	setDisplayStartLen;
	long	setPaletteLen;

	/* ... code and tables located in here */
	} VBE_pmInfo;

#define	VBE20_EXT_SIG				0xFBADFBAD

typedef enum {
	vbeMemTXT      = 0,           	/* Text mode memory model           */
	vbeMemCGA      = 1,            	/* CGA style mode                   */
	vbeMemHGC      = 2,            	/* Hercules graphics style mode     */
	vbeMemPL       = 3,            	/* 16 color VGA style planar mode   */
	vbeMemPK       = 4,            	/* Packed pixel mode                */
	vbeMemX        = 5,           	/* Non-chain 4, 256 color (ModeX)   */
	vbeMemRGB      = 6,            	/* Direct color RGB mode        	*/
	vbeMemYUV      = 7,            	/* Direct color YUV mode         	*/
	} VBE_memModels;

/* Flags for combining with video modes during mode set */

#define	vbeDontClear	0x8000		/* Dont clear display memory		*/
#define	vbeLinearBuffer	0x4000		/* Enable linear framebuffer mode	*/
#define	vbeRefreshCtrl	0x0800		/* Use refresh rate control			*/
#define vbeModeMask		0x03FF		/* Mask for VBE mode numbers		*/

/* Flags for the mode attributes returned by VBE_getModeInfo. If
 * vbeMdNonBanked is set to 1 and vbeMdLinear is also set to 1, then only
 * the linear framebuffer mode is available.
 */

#define vbeMdAvailable	0x0001		/* Video mode is available			*/
#define	vbeMdTTYOutput	0x0004		/* TTY BIOS output is supported		*/
#define	vbeMdColorMode	0x0008		/* Mode is a color video mode		*/
#define	vbeMdGraphMode	0x0010		/* Mode is a graphics mode			*/
#define	vbeMdNonVGA		0x0020		/* Mode is not VGA compatible		*/
#define	vbeMdNonBanked	0x0040		/* Banked mode is not supported		*/
#define	vbeMdLinear		0x0080		/* Linear mode supported			*/
#define	vbeMdDoubleScan	0x0100		/* Mode is double scanned			*/
#define vbeMdInterlaced	0x0200		/* Supports interlaced timings		*/
#define	vbeMdTripleBuf 	0x0400		/* Mode supports HW triple buffer	*/
#define vbeMdStereo		0x0800		/* Mode supports hardware stereo	*/
#define	vbeMdDualDisp	0x1000		/* Mode supports dual display start	*/

/* Flags for save/restore state calls */

#define	vbeStHardware	0x0001		/* Save the hardware state			*/
#define	vbeStBIOS		0x0002		/* Save the BIOS state				*/
#define	vbeStDAC		0x0004		/* Save the DAC state				*/
#define	vbeStSVGA		0x0008		/* Save the SuperVGA state			*/
#define	vbeStAll		0x000F		/* Save all states					*/

/* Palette entry structure, either 6 or 8 bit format. This is the VBE 2.0
 * palette format and the same as used by Windows (which is NOT the same
 * as the normal VGA BIOS palette format structure).
 */

typedef struct {
	uchar	blue;					/* Blue component of color			*/
	uchar	green;					/* Green component of color			*/
	uchar	red;					/* Blue component of color			*/
	uchar	alpha;					/* Alpha or alignment byte			*/
	} VBE_palette;

#pragma pack()

/*-------------------------- Function Prototypes --------------------------*/

#ifdef  __cplusplus
extern "C" {            			/* Use "C" linkage when in C++ mode */
#endif

/* VBE_init allocates a real mode memory block for use by the internal VESA
 * VBE functions when calling real mode code. VBE_getRMBuf is used by other
 * libraries and modules to gain access to the same real mode memory block
 * so that we only require one real mode memory block per application.
 */

void	VBEAPI VBE_init(void);
void	VBEAPI VBE_getRMBuf(uint *len,uint *sel,uint *off,uint *rseg,uint *roff);

/* Main VBE API routines */

void 	VBEAPI VBE_callESDI(RMREGS *regs, void *buffer, int size);
int     VBEAPI VBE_detect(VBE_vgaInfo *vgaInfo);
ibool    VBEAPI VBE_getModeInfo(int mode,VBE_modeInfo *modeInfo);
long    VBEAPI VBE_getPageSize(VBE_modeInfo *modeInfo);
ibool    VBEAPI VBE_setVideoMode(int mode);
int     VBEAPI VBE_getVideoMode(void);
ibool    VBEAPI VBE_setBank(int window,int bank);
int     VBEAPI VBE_getBank(int window);
ibool    VBEAPI VBE_setPixelsPerLine(int pixelsPerLine,int *newBytes,int *newPixels,
		int *maxScanlines);
ibool    VBEAPI VBE_setBytesPerLine(int bytesPerLine,int *newBytes,int *newPixels,
		int *maxScanlines);
ibool    VBEAPI VBE_getScanlineLength(int *bytesPerLine,int *pixelsPerLine,
		int *maxScanlines);
ibool    VBEAPI VBE_getMaxScanlineLength(int *maxBytes,int *maxPixels);
ibool    VBEAPI VBE_setDisplayStart(int x,int y,ibool waitVRT);
ibool    VBEAPI VBE_getDisplayStart(int *x,int *y);
ibool    VBEAPI VBE_setDACWidth(int width);
int     VBEAPI VBE_getDACWidth(void);
ibool    VBEAPI VBE_setPalette(int start,int num,VBE_palette *pal,ibool waitVRT);
void *	VBEAPI VBE_getBankedPointer(VBE_modeInfo *modeInfo);

/* Functions to return the addresses of the VBE 2.0 32 bit code */

#if	!defined(REALMODE) || defined(TESTING_PM)
ushort  VBEAPI VBE_getLinearSelector(VBE_modeInfo *modeInfo);
void * 	VBEAPI VBE_getLinearPointer(VBE_modeInfo *modeInfo);
void * 	VBEAPI VBE_getSetBank(void);
void * 	VBEAPI VBE_getSetDisplayStart(void);
void * 	VBEAPI VBE_getSetPalette(void);
void    VBEAPI VBE_freePMCode(void);
ibool	VBEAPI VBE_getBankFunc32(int *codeLen,void **bankFunc,
		int dualBanks,int bankAdjust);
#endif

/* New VBE/Core 3.0 functions */

ibool    VBEAPI VBE_setVideoModeExt(int mode,VBE_CRTCInfo *crtc);
ibool 	VBEAPI VBE_setDisplayStartAlt(ulong startAddr,ibool waitVRT);
int		VBEAPI VBE_getDisplayStartStatus(void);
ibool 	VBEAPI VBE_enableStereoMode(void);
ibool 	VBEAPI VBE_disableStereoMode(void);
ibool 	VBEAPI VBE_setStereoDisplayStart(ulong leftAddr,ulong rightAddr,ibool waitVRT);
ulong	VBEAPI VBE_getClosestClock(ushort mode,ulong pixelClock);

#ifdef  __cplusplus
}                       			/* End of "C" linkage for C++   	*/
#endif

#endif  /* __VESAVBE_H */
