/****************************************************************************
*
*						MegaGraph Graphics Library
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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MegaGraph graphics library, containing
*				the declarations for internal stuctures used by the library.
*
*				You define one of the following #defines to build the
*				library for each different platform:
*
*					MGLDOS	- Build for MSDOS
*					MGLWIN	- Build for Windows with WinG and DISPDIB
*					MGLPM	- Build for OS/2 PM with DIVE and fullscreen
*					MGLX	- Build for X11
*
****************************************************************************/

#ifndef	__MGL_H
#define	__MGL_H

#define	VBEAF_10_COMPAT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "mgl_int.h"
#include "list.h"
#include "resource.h"
#include "halftone.h"
#include "vbeaf.h"
#include "fgl.h"
#include "pmpro.h"

/*---------------------- Macro and type definitions -----------------------*/

#ifdef __ALPHA__
#define	NO_ASSEMBLER		/* Compile as C only for DEC Alpha			*/
#endif

#ifdef	__INTEL__
#pragma pack(1)				/* Pack structures to byte granularity		*/
#endif

#if	defined(__INTEL__) && defined(__WINDOWS__)
#define	GET_DEFAULT_CW()					\
{                                           \
	if (_MGL_cw_default == 0)               \
		_MGL_cw_default = _control87(0,0);  \
}
#define	RESET_DEFAULT_CW()					\
	_control87(_MGL_cw_default,0xFFFFFFFF)
#else
#define	GET_DEFAULT_CW()
#define	RESET_DEFAULT_CW()
#endif

#ifndef	_MAX_PATH
#define	_MAX_PATH	255
#endif

/* Define the default size of the MGL scratch buffer	*/

#ifdef	PM386
#define	DEF_MGL_BUFSIZE	32*1024
#else
#define	DEF_MGL_BUFSIZE	6*1024
#endif

/* Indices of memory device drivers in driver table 			*/

#define	PACKED1INDEX	0
#define	PACKED4INDEX	1
#define	PACKED8INDEX	2
#define	PACKED16INDEX	3
#define	PACKED24INDEX	4
#define	PACKED32INDEX	5

/* Indices of windowed device drivers in driver table 			*/

#define WDCI8INDEX		6
#define WDCI16INDEX		7
#define WDCI24INDEX		8
#define WDCI32INDEX		9
#define WDDRAW8INDEX	10
#define WDDRAW16INDEX	11
#define WDDRAW24INDEX	12
#define WDDRAW32INDEX	13

/* Name of environment variable pointing to MGL root directory	*/

#define MGL_ROOT	"MGL_ROOT"

/* Names of MGL data file directories, located below this root	*/

#define	MGL_DRIVERS	"DRIVERS\\"
#define	MGL_FONTS	"FONTS\\"
#define	MGL_BITMAPS	"BITMAPS\\"
#define	MGL_ICONS	"ICONS\\"
#define	MGL_CURSORS	"CURSORS\\"

/* The following defines the version number of the device driver files
 * that this version of the MGL must be used with. Older driver files
 * will not be compatible with this version of the MGL and an error will
 * be returned if an attempt is made to use them.
 */

#define	DRIVER_MAJOR_VERSION	2
#define	DRIVER_MINOR_VERSION	0
#define	DRIVER_VERSION_STR		"2.0"

#define	VBE_VIRTUAL_LINEAR		0x0800	/* Virtual linear framebuffer	*/

/* Macro to normalise a huge pointer */

#define	NORMALISE_PTR(p)

/* Table of available modes, driver to use and mode information */

typedef struct {
	uchar	driver;				/* Id of driver supporting this mode	*/
	uchar	pages;				/* Number of available hardware pages	*/
	ulong	flags;				/* Mode flags for mode					*/
	} modeent;

typedef modeent modetab[grMAXMODE];

/* Device driver header structure */

typedef struct {
	char	ident[18];		/* Driver file identification string		*/
	char	name[13];		/* Driver's name							*/
	char	realname[13];	/* Driver's real name to display to user	*/
	char	copyright[95];	/* Copyright notice etc						*/
	char	majorversion;	/* Major version number						*/
	char	minorversion;	/* Minor version number						*/
    char	driverId;		/* Driver ID number							*/

	/* Pointers to detection and initialisation routines */

	ibool	(_ASMAPI *detect)(int id,int force,int *driver,int *mode,modetab availableModes);
	ibool	(_ASMAPI *initDriver)(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
	} drivertype;

/* Device driver table entry structure */

typedef	struct {
	char		name[13];		/* Name of driver file (with .DRV)		*/
	ibool		registered;		/* True when driver has been registered	*/
	drivertype	*driver;		/* Address of start of driver			*/
	} driverent;

typedef	ushort	zbuf16_t;		/* Type used for 16 bit zbuffering		*/

typedef	struct {
	uchar		z[3];
	} zbuf24_t;					/* Type used for 24 bit zbuffering		*/

typedef	ulong	zbuf32_t;		/* Type used for 32 bit zbuffering		*/

/* Macro to convert from 4.28 z value to zbuffer value */

#define	MGL_TOZBUF16(z)	(zbuf16_t)(z >> 15)
#define	MGL_TOZBUF24(z)	(z >> 7)
#define	MGL_TOZBUF32(z)	(zbuf32_t)z

/* Structures used to scan convert filled primitives */

typedef struct {
	short	left;				/* left coordinate of scan line			*/
	short	right;				/* right coordinate of scan line		*/
	} scanline;

typedef struct {
	int		length;			/* Number of scan lines in list			*/
	int		top;			/* Y coordinate of top scan line		*/
	scanline	*scans;			/* Pointer to list of scan lines		*/
	} scanlist;

typedef struct {
	short	leftL;				/* left coordinate of left scan line	*/
	short	leftR;				/* right coordinate of left scan line	*/
	short	rightL;				/* left coordinate of right scan line	*/
	short	rightR;				/* right coordinate of right scan line	*/
	} scanline2;

typedef struct {
	int		length;			/* Number of scan lines in list			*/
	int		top;			/* Y coordinate of top scan line		*/
	scanline2	*scans;			/* Pointer to list of scan lines		*/
	} scanlist2;

/* Header used for divots. The scratch values are driver dependant.		*/

typedef struct {
	int 	left,top,right,bottom;
	short	bytesPerLine;
	} divotHeader;

#define	SENTINEL	32767		/* Very large sentinel value			*/
#define	MAXVEC		256			/* Number of characters in font defn	*/

/* Internal structures for region manipulation */

#define DEF_NUM_SEGMENTS    500
#define	DEF_NUM_SPANS		500
#define	DEF_NUM_REGIONS		100
#define	FREELIST_NEXT(p)   	(*(void**)(p))

typedef struct segment_t {
	struct segment_t	*next;	/* Next segment in span					*/
	short				x;		/* X coordinates of this segment		*/
	} segment_t;

struct _span_t {
	struct _span_t	*next;		/* Next span in region					*/
	segment_t		*seg;		/* Index of first segment in span		*/
	short			y;			/* Y coordinate of this span			*/
    };

typedef struct block_t {
	struct block_t	*next;		/* Pointer to next block in list		*/
	} block_t;

typedef struct {
	block_t	*curBlock;			/* List of memory pool blocks			*/
	int	blockCount;			/* Number of pool blocks allocated		*/
	int	max;				/* Maximum objects in block				*/
	int	objectSize;			/* Size of objects in block				*/
	} blockList_t;

typedef struct {
	blockList_t bl;				/* Memory pool containg objects			*/
	segment_t	*freeList;		/* Start of free list for pool			*/
	} segmentList_t;

typedef struct {
	blockList_t bl;				/* Memory pool containg objects			*/
	span_t		*freeList;		/* Start of free list for pool			*/
	} spanList_t;

typedef struct {
	blockList_t bl;				/* Memory pool containg objects			*/
	region_t	*freeList;		/* Start of free list for pool			*/
	} regionList_t;

/* Vector Font structure */

// TODO: Need to convert the following structures to unpacked format for
//		 the DEC alpha for better performance. To do this we will need to
//		 read them from disk in the packed format and then convert them
//		 to this internal unpacked structure internally when loaded
//		 using the GetLEShort style functions.

#pragma pack(1)
typedef struct {
	char			name[_MGL_FNAMESIZE];/* Name of the font			*/
	short			fontType;		/* Type of font						*/
	short			maxWidth;		/* Maximum character width			*/
	short			maxKern;		/* Maximum character kern			*/
	short			fontWidth;		/* Font width						*/
	short			fontHeight;		/* Font height						*/
	short			ascent;			/* Font ascent value				*/
	short			descent;		/* Font descent value				*/
	short			leading;		/* Font leading value				*/

	uchar			width[MAXVEC];	/* Character width table			*/
	short			offset[MAXVEC];	/* Offset table into vector defn.	*/
	uchar			def[1];			/* Vector definitions for font		*/
	} vec_font;

/* Bitmap font structure */

typedef struct {
	char			name[_MGL_FNAMESIZE];/* Name of the font			*/
	short			fontType;		/* Type of font						*/
	short			maxWidth;		/* Maximum character width			*/
	short			maxKern;		/* Maximum character kern			*/
	short			fontWidth;		/* Font width						*/
	short			fontHeight;		/* Font height						*/
	short			ascent;			/* Font ascent value				*/
	short			descent;		/* Font descent value				*/
	short			leading;		/* Font leading value				*/

	uchar			width[MAXVEC];	/* Character width table			*/
	short			offset[MAXVEC];	/* Offset table into character defn.*/
	uchar			iwidth[MAXVEC];	/* Character image width table		*/
	uchar			loc[MAXVEC];	/* Character location table			*/
	uchar			def[1];			/* Bitmap definitions for font		*/
	} bitmap_font;

/* Missing symbol character */

#define	MISSINGSYMBOL	0

/* Macro to convert to a vector font pointer */

#define	VECFONT(fnt)	((vec_font *)fnt)

/* Macro to convert to a bitmap font pointer */

#define	BITFONT(fnt)	((bitmap_font *)fnt)

/* Macros to test for the missing symbol */

#define	VEC_ISMISSING(fnt,ch)	(VECFONT(fnt)->offset[(uchar)(ch)] == -1)
#define	BIT_ISMISSING(fnt,ch)	(BITFONT(fnt)->offset[(uchar)(ch)] == -1)

/* Font file header structure */

typedef struct {
	char	ident[21];			/* Font file identification string		*/
	char	name[_MGL_FNAMESIZE];	/* Font's name						*/
	char	copyright[175];		/* Copyright notice etc					*/
	char	majorversion;		/* Major version number					*/
	char	minorversion;		/* Minor version number					*/
	} font_header;
#pragma pack()

/* Identification flag for graphics device drivers */

#define	IDENT_FLAG	"MegaGraph Driver:"

/* Identification flag for font files */

#define	FONT_IDENT	"MegaGraph Font File:"

/* Macros to swap two values */

#define SWAP(a,b)	{ a^=b; b^=a; a^=b; }

#define	SWAPT(v1,v2,t) { 	\
	t = v1;					\
	v1 = v2;				\
	v2 = t;					\
	}

/* Device driver type id's */

#define	MGL_DISPLAY_DEVICE			0
#define	MGL_FULLDIB_DEVICE			1
#define	MGL_OPENGL_DEVICE			2
#define	MGL_FS_OPENGL_DEVICE		3
#define	MGL_OFFSCREEN_DEVICE		4
#define	MGL_LINOFFSCREEN_DEVICE     5
#define	MGL_WINDOWED_DEVICE 		6
#define	MGL_MEMORY_DEVICE			7

#define	_MGL_isFullscreenDevice(dc)						\
	(dc->deviceType == MGL_DISPLAY_DEVICE ||            \
	 dc->deviceType == MGL_FULLDIB_DEVICE ||            \
	 dc->deviceType == MGL_OPENGL_DEVICE ||             \
	 dc->deviceType == MGL_FS_OPENGL_DEVICE)

/* Internal flags stored in the DC	*/

#define	MGL_SHARED_ZBUFFER 0x80000000UL	/* Zbuffer is not owned by DC	*/

/* Function pointer prototypes for rendering code */
typedef	void (_ASMAPI *pixelFunc)(int x,int y);
typedef	void (_ASMAPI *lineFunc)(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
typedef	void (_ASMAPI *scanLineFunc)(int y,int x1,int x2);
typedef	void (_ASMAPI *fillRectFunc)(int x1,int y1,int x2,int y2);
typedef	void (_ASMAPI *drawScanListFunc)(int y,int length,short *scans);
typedef	void (_ASMAPI *trapFunc)(void);
typedef	void (_ASMAPI *drawRegionFunc)(int x,int y,const region_t *r);
typedef	void (_ASMAPI *ellipseFunc)(int left,int top,int A,int B,int clip);
typedef	void (_ASMAPI *fillEllipseFunc)(int left,int top, int A, int B,int clip);
typedef	void (_ASMAPI *ellipseArcFunc)(int left,int top,int A,int B,int startangle,int endangle,int clip);
typedef	void (_ASMAPI *fillEllipseArcFunc)(int left,int top,int A,int B,int startangle,int endangle,int clip);

/* Macro versions of routines in 'STUBS.C' that simply invoke the appropriate
 * routine within the device context. Both the macro and non-macro versions
 * of the routines exist, so you can take the address of one of these
 * routines.
 */

#define MGL_setColor(c) 					DC.r.setColor(c)
#define MGL_setBackColor(c) 				DC.r.setBackColor(c)
#define MGL_beginPixel()					DC.r.beginPixel(&DC)
#define MGL_endPixel()						DC.r.endPixel(&DC)
#define	MGL_getArcCoords(c)					DC.r.getArcCoords(c)
#define MGL_scanRightForColor(x,y,c)		DC.r.scanRightForColor(x,y,c)
#define MGL_scanLeftForColor(x,y,c)			DC.r.scanLeftForColor(x,y,c)
#define MGL_scanRightWhileColor(x,y,c)		DC.r.scanRightWhileColor(x,y,c)
#define MGL_scanLeftWhileColor(x,y,c)		DC.r.scanLeftWhileColor(x,y,c)

/* Macro to compute the address of a pixel with a device surface. This
 * works for all packed pixel modes (the only ones that can be directly
 * accessed through the device context surface anyway).
 */

#define	_MGL_pixelAddr2(s,x,y,b,p)									\
	((uchar _HUGE *)(s) + ((y) * (b)) + ((x) * ((p+7)/8)))

#define	_MGL_pixelAddr(x,y)											\
	_MGL_pixelAddr2(DC.surface,x,y,DC.mi.bytesPerLine,DC.mi.bitsPerPixel)

#define	_MGL_zbufAddr16(x,y)										\
	((zbuf16_t _HUGE*)(DC.zbuffer)									\
		+ (((y) - DC.size.top) * (long)DC.zwidth)					\
		+ (x) - DC.size.left)

#define	_MGL_zbufAddr24(x,y)										\
	((zbuf24_t _HUGE*)(DC.zbuffer)									\
		+ (((y) - DC.size.top) * (long)DC.zwidth)					\
		+ (x) - DC.size.left)

#define	_MGL_zbufAddr32(x,y)										\
	((zbuf32_t _HUGE*)(DC.zbuffer)									\
		+ (((y) - DC.size.top) * (long)DC.zwidth)					\
		+ (x) - DC.size.left)

/* Macros to halfTone dither an RGB pixel */

#define	MGL_halfTonePixelFast(p,x,y,R,G,B)						\
{																\
	uchar dither = _MGL_dither8x8[((x & 7) << 3) + (y & 7)];	\
	(p) = 20 + 													\
		_MGL_div51[R] + (_MGL_mod51[R] > dither) +				\
		_MGL_mul6[_MGL_div51[G] + (_MGL_mod51[G] > dither)] +	\
		_MGL_mul36[_MGL_div51[B] + (_MGL_mod51[B] > dither)];	\
}

#define	MGL_halfTonePixelFast2(p,x,y,c)							\
{																\
	uchar dither = _MGL_dither8x8[((x & 7) << 3) + (y & 7)];	\
	(p) = 20 +													\
		_MGL_div51[MGL_rgbRed(c)] +								\
		(_MGL_mod51[MGL_rgbRed(c)] > dither) +					\
		_MGL_mul6[_MGL_div51[MGL_rgbGreen(c)] +                 \
		(_MGL_mod51[MGL_rgbGreen(c)] > dither)] +				\
		_MGL_mul36[_MGL_div51[MGL_rgbBlue(c)] +                 \
		(_MGL_mod51[MGL_rgbBlue(c)] > dither)];					\
}

#define	MGL_halfTonePixelSetup(y,c,dither,Rdiv51,Rmod51,		\
	Gdiv51,Gmod51,Bdiv51,Bmod51)								\
{																\
	dither = &_MGL_dither8x8[(y & 7)];							\
	Rdiv51 = _MGL_div51[MGL_rgbRed(c)];							\
	Rmod51 = _MGL_mod51[MGL_rgbRed(c)];							\
	Gdiv51 = _MGL_div51[MGL_rgbGreen(c)];						\
	Gmod51 = _MGL_mod51[MGL_rgbGreen(c)];						\
	Bdiv51 = _MGL_div51[MGL_rgbBlue(c)];						\
	Bmod51 = _MGL_mod51[MGL_rgbBlue(c)];						\
}

#define	MGL_halfTonePixelFast3(p,x,dp,Rdiv51,Rmod51,			\
	Gdiv51,Gmod51,Bdiv51,Bmod51)								\
{																\
	uchar dither = dp[((x & 7) << 3)];							\
	(p) = 20 +													\
		Rdiv51 + (Rmod51 > dither) +							\
		_MGL_mul6[Gdiv51 + (Gmod51 > dither)] +					\
		_MGL_mul36[Bdiv51 + (Bmod51 > dither)];					\
}

/* Macro to check for an identity palette */

#define	ID_PAL(m1,p1,m2,p2)	(memcmp(p1,p2,MIN(m1,m2) * sizeof(color_t)) == 0)

#define	ID_PAL_DC(src,dst)												\
	ID_PAL((src)->mi.maxColor+1,(src)->colorTab,						\
		   (dst)->mi.maxColor+1,(dst)->colorTab)

/* Macro to compare two pixel formats strcutures */

#define	CMP_PF(pf1,pf2)													\
   (((pf1).redPos == (pf2).redPos) &&                                 	\
	((pf1).greenPos == (pf2).greenPos) &&                             	\
	((pf1).bluePos == (pf2).bluePos))

/* Macros to check to see if the bitmap needs to be translated, which
 * is required if the pixel depths are different, the pixel format is
 * different or we dont have an identity palette mapping in color
 * index modes.
 */

#define NEED_TRANSLATE(sBits,sPal,sPF,sMax,dBits,dPal,dPF,dMax)			\
   (((sBits) != (dBits))												\
	||                                                      			\
	(_MGL_checkIdentityPal && (sPal) && (dBits) <= 8 &&					\
		!ID_PAL((sMax),(sPal),(dMax),(dPal)))							\
	||																	\
	((dBits) > 8 && !CMP_PF(dPF,sPF)))

#define	NEED_TRANSLATE_DC(src,dst)                            			\
	NEED_TRANSLATE((src)->mi.bitsPerPixel,(src)->colorTab,(src)->pf,	\
		(src)->mi.maxColor+1,											\
		(dst)->mi.bitsPerPixel,(dst)->colorTab,(dst)->pf,				\
		(dst)->mi.maxColor+1)

#define	NEED_TRANSLATE_BM(bm,dst)                            			\
	NEED_TRANSLATE((bm)->bitsPerPixel,(bm)->pal,*((bm)->pf),			\
		(((bm)->bitsPerPixel == 8) ? 16 : 256),							\
		(dst)->mi.bitsPerPixel,(dst)->colorTab,(dst)->pf,				\
		(dst)->mi.maxColor+1)

/* Macro to build a translation vector between two palettes, mapping
 * colors from one palette to the closest colors in the other palette.
 */

#define	_MGL_buildTranslateVector(t,sSize,sPal,dSize,dPal)				\
{                                                                       \
	int		i;                                                      \
	uchar		*p = t;                                                 \
	palette_t	*sp = sPal;                                             \
																		\
	for (i = 0; i < sSize; i++,sp++) {                                  \
		int		j,closest = 0,d,distance = 0x7FFFL;                 \
		palette_t	*dp = dPal;                                         \
																		\
		for (j = 0; j < dSize; j++,dp++) {                              \
			if ((d = ABS((int)sp->red - (int)dp->red)               \
				   + ABS((int)sp->green - (int)dp->green)           \
				   + ABS((int)sp->blue - (int)dp->blue)) < distance) { \
				distance = d;                                           \
				closest = j;                                            \
				}                                                       \
			}                                                           \
		*p++ = closest;                                                 \
		}                                                               \
}

/* Macros to index into polygon vertex and color arrays */

#define	VTX(vArray,vinc,i)	((fxpoint_t*)((char*)(vArray) + (vinc)*(i)))
#define	INCVTX(vArray,vinc) *((char**)&(vArray)) += (vinc)
#define	VTX3(vArray,vinc,i)	((fxpoint3d_t*)((char*)(vArray) + (vinc)*(i)))
#define	INCVTX3(vArray,vinc) *((char**)&(vArray)) += (vinc)
#define	CLR(cArray,cinc,i)	((color_t*)((char*)(cArray) + (cinc)*(i)))
#define	INCCLR(cArray,cinc) *((char**)&(cArray)) += (cinc)

/* Macro to increment any pointer by a number of bytes */

#define	BINC(v,inc) *((char**)&(v)) += (inc)

/* Define the indexes for the pre-defined pixel format types */

typedef enum {
	pfRGB555,			/* 15 bits per pixel 5:5:5 format				*/
	pfRGB565,			/* 16 bits per pixel 5:6:5 format				*/
	pfRGB24,			/* 24 bits per pixel RGB format					*/
	pfBGR24,			/* 24 bits per pixel BGR format					*/
	pfARGB32,			/* 32 bits per pixel ARGB format				*/
	pfABGR32,			/* 32 bits per pixel ABGR format				*/
	pfRGBA32,			/* 32 bits per pixel RGBA format				*/
    pfBGRA32,			/* 32 bits per pixel BGRA format				*/
	} _MGL_pixelFormatIdx;

/*--------------------------- Global Variables ----------------------------*/

extern int			_MGL_driverId;
extern int			_MGL_modeId;
extern LIST			*_MGL_dispDCList;
#ifndef	MGL_LITE
extern LIST			*_MGL_offDCList;
#endif
extern LIST			*_MGL_memDCList;
extern uint			_MGL_cw_default;
extern ibool			_VARAPI _MGL_fullScreen;
extern int	 		_VARAPI _MGL_result;
extern char	 		_VARAPI _MGL_path[_MAX_PATH];
extern uchar	 	_VARAPI _MGL_modeTable[grMAXMODE];
extern modetab 		_VARAPI _MGL_availableModes;
extern driverent	_VARAPI _MGL_driverTable[];
extern MGLDC		_VARAPI _MGL_dc;
extern MGLDC		* _VARAPI _MGL_dcPtr;
extern void			* _VARAPI _MGL_buf;
extern int			_VARAPI _MGL_bufSize;
extern segment_t	_VARAPI _MGL_tmpX1;
extern segment_t	_VARAPI _MGL_tmpX2;
extern span_t		_VARAPI _MGL_tmpY1;
extern span_t		_VARAPI _MGL_tmpY2;
extern region_t		_VARAPI _MGL_rtmp;
extern region_t 	_VARAPI _MGL_rtmp2;
extern pixel_format_t _VARAPI _MGL_pixelFormats[];
extern ibool			_VARAPI _MGL_checkIdentityPal;
extern uchar 		_VARAPI _MGL_translate[256];
extern AF_devCtx   	* _VARAPI _MGL_wdDriver;
extern int			_VARAPI _MGL_surfLock;
extern void 		* (*_MGL_malloc)(long size);
extern void 		(*_MGL_free)(void *p);
extern FILE *		(*_MGL_fopen)(const char *filename,const char *mode);
extern int 			(*_MGL_fclose)(FILE *f);
extern int 			(*_MGL_fseek)(FILE *f,long offset,int whence);
extern long 		(*_MGL_ftell)(FILE *f);
extern size_t		(*_MGL_fread)(void *ptr,size_t size,size_t n,FILE *f);
extern size_t		(*_MGL_fwrite)(const void *ptr,size_t size,size_t n,FILE *f);
extern vecs			_PACKED_vecs;
extern int			_MGL_whichOpenGL;
extern int			_MGL_cntOpenGL;
extern int			_VARAPI _MGL_stereoSyncType;
extern int			_VARAPI _MGL_blueCodeIndex;

/*------------------------- Function Prototypes ---------------------------*/

/* Private platform independant routines */

ibool 	_MGL_initDC(MGLDC *dc,drivertype *driver,MGL_HWND hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
MGLDC * _MGL_createOffscreenDC(ibool linear);
ibool 	_MGL_destroyDisplayDC(MGLDC *dc);
ibool 	_MGL_destroyOffscreenDC(MGLDC *dc);
ibool 	_MGL_destroyMemoryDC(MGLDC *dc);
void	_MGL_setCommonVectors(MGLDC *dc);
void	ASMAPI _MGL_setRenderingVectors(void);
void	_MGL_disableAutoPlay(void);
void 	_MGL_restoreAutoPlay(void);
long	_MGL_fileSize(FILE *f);
FILE *	_MGL_openFile(const char *dir, const char *name, const char *mode);
void 	_MGL_initMalloc(void);
void	_MGL_scratchTooSmall(void);
void	_MGL_updateCurrentDC(MGLDC *dc);
int 	_MGL_bytesPerLine(MGLDC *dc,int left,int right);
int 	_ASMAPI _MGL_preClipPoly(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
int 	_ASMAPI _MGL_preClipTri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
int 	_ASMAPI _MGL_preClipQuad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void 	_ASMAPI _MGL_drawClippedScanList(int y,int length,short *scans);
void 	_ASMAPI _MGL_clipScanLine(int top,int left,int right);
void 	_ASMAPI _MGL_clipScanLineV(int top,int left,int right);
void 	_ASMAPI _MGL_clipPixel(int x,int y);
void 	_ASMAPI _MGL_clipDitherPixel(int x,int y);
ibool	_MGL_clipCLineFX(fix32_t *x1,fix32_t *y1,fix32_t *c1,fix32_t *x2,fix32_t *y2,fix32_t *c2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);
ibool	_MGL_clipRGBLineFX(fix32_t *x1,fix32_t *y1,fix32_t *r1,fix32_t *g1,fix32_t *b1,fix32_t *x2,fix32_t *y2,fix32_t *r2,fix32_t *g2,fix32_t *b2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);
ibool	_MGL_clipZLineFX(fix32_t *x1,fix32_t *y1,zfix32_t *z1,fix32_t *x2,fix32_t *y2,zfix32_t *z2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);
ibool	_MGL_clipCZLineFX(fix32_t *x1,fix32_t *y1,zfix32_t *z1,fix32_t *c1,fix32_t *x2,fix32_t *y2,zfix32_t *z2,fix32_t *c2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);
ibool	_MGL_clipRGBZLineFX(fix32_t *x1,fix32_t *y1,zfix32_t *z1,fix32_t *r1,fix32_t *g1,fix32_t *b1,fix32_t *x2,fix32_t *y2,zfix32_t *z2,fix32_t *r2,fix32_t *g2,fix32_t *b2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);
void 	_ASMAPI _MGL_ellipseEngine(int left,int top,int A,int B,void (_ASMAPI *setup)(int topY,int botY,int left,int right),void (_ASMAPI *set4pixels)(ibool inc_x,ibool inc_y,ibool region1),void (_ASMAPI *finished)(void));
void 	_ASMAPI _MGL_drawCharVecSlow(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy);
void 	_ASMAPI _MGL_rotateVector(vector *vec,int dir);
void 	_ASMAPI _MGL_rotateFull8x8AClockwise(uchar *dst,int dstStep,uchar *src,int srcStep);
void 	_ASMAPI _MGL_rotatePartial8x8AClockwise(uchar *dst,int dstStep,uchar *src,int srcStep,int height);
void 	_ASMAPI _MGL_rotateFull8x8Clockwise(uchar *dst,int dstStep,uchar *src,int srcStep);
void 	_ASMAPI _MGL_rotatePartial8x8Clockwise(uchar *dst,int dstStep,uchar *src,int srcStep,int height);
void 	_ASMAPI _MGL_rotateBitmap180(uchar *dst,uchar *src,int byteWidth,int height);
void	_MGL_scanFatEllipse(scanlist2 *scanList,int left,int top,int A,int _B,int _penWidth,int _penHeight);
void	_MGL_scanEllipse(scanlist *scanList,int left,int top, int A, int _B);
void 	_MGL_computeCoords(int A,int B,int cx,int cy,int angle,int *rx,int *ry,int *quadrant);
int 	_MGL_fixAngle(int angle);
void	_MGL_scanEllipseArc(scanlist2 *sl,arc_coords_t *ac,int left,int top,int A,int _B,int startAngle,int endAngle);
void	_ASMAPI _MGL_ellipseArcEngine(int left,int top,int A,int B,int startAngle,int endAngle,arc_coords_t *ac,void (_ASMAPI *plotPoint)(int x,int y));
int 	_MGL_scanFatLine(scanlist *scanList,fix32_t fx1,fix32_t fy1,fix32_t fx2,fix32_t fy2,int _penHeight,int _penWidth);
void 	_MGL_computePF(MGLDC *dc);
int 	_MGL_closestColor(palette_t *color,palette_t *pal,int palSize);
ibool 	MGL_glLoadOpenGL(MGLDC *dc);
void 	MGL_glUnloadOpenGL(void);
ibool 	_MGL_isOpenGLDC(MGLDC *dc);
ibool 	_MGL_isFSOpenGLDC(MGLDC *dc);
ibool	_MGL_haveFSOpenGL(int *numModes,FGL_modeInfo *modeList);
#ifdef	__WINDOWS__
void 	__stdcall gl_WindowPos4fMESA( float x, float y, float z, float w );
#endif

/* Bitmap allocation routines */

long _MGL_allocateDIB(MGLDC *dc,winBITMAPINFO *info);
void _MGL_freeDIB(MGLDC *dc);

/* Initialise the low level PC hardware routines */

void MGLPC_init(void);

/* Enable and disable full screen operation */

#ifdef	__WINDOWS__
void MGLPC_startFullScreen(int xRes,int yRes);
void MGLPC_disableFullScreen(void);
void MGLPC_destroyFullScreenMode(void);
#else
#define	MGLPC_startFullScreen(x,y)
#define	MGLPC_disableFullScreen()
#define MGLPC_destroyFullScreenMode()
#endif

/* Internal mouse handling routines */

int _MS_foolDriver(void);
void _MS_unfoolDriver(int oldval);
void _MS_init(void);
void _MS_setMouseHandler(int mask,PM_mouseHandler mh);
void _MS_restoreMouseHandler(void);
void _MS_setDisplayDC(MGLDC *dc);
void ASMAPI _MS_moveCursor(int x,int y);
void _MS_saveCursor(void);
void _MS_restoreCursor(void);
void _MS_displayStartChanged(void);
void _MS_preChangeVisualPage(int newPage);
void _MS_postChangeVisualPage(int oldPage);
void _MS_sizeScreen(int xres,int yres);
void _DD_setMousePos(int x,int y);

/* Routines for doing simplified I/O port manipulation from C */

void    _ASMAPI UV_disable(void);
void    _ASMAPI UV_enable(void);
ushort  _ASMAPI UV_inp(ushort port);
ushort  _ASMAPI UV_inpw(ushort port);
ulong   _ASMAPI UV_inpd(ushort port);
void    _ASMAPI UV_outp(ushort port,ushort value);
void    _ASMAPI UV_outpw(ushort port,ushort value);
void    _ASMAPI UV_outpd(ushort port,ulong value);
ushort	UV_rdinx(ushort port,ushort index);
void	UV_wrinx(ushort port,ushort index,ushort value);
void	UV_modinx(ushort port,ushort index,ushort mask,ushort value);
void	UV_setinx(ushort port,ushort index,ushort mask);
void	UV_clrinx(ushort port,ushort index,ushort mask);

/* Polygon rendering helper functions */

int 	_ASMAPI _MGL_computeSlope(fxpoint_t *v1,fxpoint_t *v2,fix32_t *slope);
int 	_ASMAPI _MGL_cComputeSlope(fxpointc_t *v1,fxpointc_t *v2,fix32_t *slope,fix32_t *cSlope);
int 	_ASMAPI _MGL_rgbComputeSlope(fxpointrgb_t *v1,fxpointrgb_t *v2,fix32_t *slope,fix32_t *rSlope,fix32_t *gSlope,fix32_t *bSlope);
void 	_ASMAPI _MGL_preRenderScanLine16(int x0,int minx,int maxx);
void 	_ASMAPI _MGL_preRenderScanLine24(int x0,int minx,int maxx);
void 	_ASMAPI _MGL_preRenderScanLine32(int x0,int minx,int maxx);
void 	_ASMAPI _MGL_zComputeSlope(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3);

/* 4 bit color mapped bitmap translation routines */

void 	_ASMAPI _MGL_translate4to8(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);
void 	_ASMAPI _MGL_translate4to16(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);
void 	_ASMAPI _MGL_translate4to24(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);
void 	_ASMAPI _MGL_translate4to32(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);

/* 8 bit color mapped bitmap translation routines */

void 	_ASMAPI _MGL_translate8to4(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);
void 	_ASMAPI _MGL_translate8to8(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);
void 	_ASMAPI _MGL_translate8to16(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);
void 	_ASMAPI _MGL_translate8to24(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);
void 	_ASMAPI _MGL_translate8to32(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,palette_t *pal);

/* RGB direct color bitmap translation routines */

void 	_ASMAPI _MGL_translateRGBto4(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bytesPerPixel,pixel_format_t *pf);
void 	_ASMAPI _MGL_translateRGBto8(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bytesPerPixel,pixel_format_t *pf);
void 	_ASMAPI _MGL_translateRGBto16(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bytesPerPixel,pixel_format_t *pf);
void 	_ASMAPI _MGL_translateRGBto24(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bytesPerPixel,pixel_format_t *pf);
void 	_ASMAPI _MGL_translateRGBto32(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bytesPerPixel,pixel_format_t *pf);

/* Internal region management routines */

void 	_MGL_createBlockList(blockList_t *l,int max,int objectSize);
void 	_MGL_resizeBlockList(blockList_t *l);
void 	_MGL_freeBlockList(blockList_t *l);
void 	_MGL_createSegmentList(void);
void 	_MGL_freeSegmentList(void);
segment_t * _MGL_newSegment(void);
segment_t * _MGL_copySegment(segment_t *s);
segment_t * _MGL_copySegmentOfs(segment_t *s,int xOffset);
void 	_MGL_freeSegment(segment_t *s);
void 	_MGL_createSpanList(void);
void 	_MGL_freeSpanList(void);
span_t 	* _MGL_newSpan(void);
span_t	* _MGL_createSpan(int y,int x1,int x2);
span_t	* _MGL_createSpan2(int y,int x1,int x2,int x3,int x4);
span_t 	* _MGL_copySpan(span_t *s);
span_t 	* _MGL_copySpanOfs(span_t *s,int xOffset,int yOffset);
span_t	* _MGL_copySpans(span_t *s);
span_t 	* _MGL_copySpansOfs(span_t *s,int xOffset,int yOffset);
void  	_MGL_freeSpan(span_t *s);
void 	_MGL_createRegionList(void);
void 	_MGL_freeRegionList(void);
span_t	* _MGL_rectRegion(region_t *r);
region_t * _MGL_createRegionFromBuf(scanlist *scanList);
region_t * _MGL_rgnEllipse(int left,int top,int A,int B,const region_t *_pen);
region_t * _MGL_rgnEllipseArc(int left,int top,int A,int B,int startAngle,int endAngle,const region_t *pen);

/* Region manipulation macros for maximum speed */

#define	_MGL_tmpRectRegion(r)				\
{											\
	_MGL_tmpY1.y = (r)->rect.top;			\
	_MGL_tmpY2.y = (r)->rect.bottom;		\
	_MGL_tmpX1.x = (r)->rect.left;			\
	_MGL_tmpX2.x = (r)->rect.right;			\
	_MGL_rtmp.rect = (r)->rect;				\
	(r) = &_MGL_rtmp;						\
}

#define	_MGL_tmpRectRegion2(r,rt)			\
{											\
	_MGL_rtmp2.rect = rt;					\
	(r) = &_MGL_rtmp2;						\
}

/* Private emulation routines */

void 	_ASMAPI __EMU__clearDevice(void);
void 	_ASMAPI __EMU__line(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI __EMU__stippleLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI __EMU__fatPenLine(fix32_t x1,fix32_t y1,fix32_t x2,fix32_t y2);
void 	_ASMAPI __EMU__drawScanList(int y,int length,short *scans);
void	_ASMAPI __EMU__drawRegion(int x,int y,const region_t *r);
void 	_ASMAPI __EMU__fillRect(int x1,int y1,int x2,int y2);
void	_ASMAPI __EMU__ellipse(int left,int top,int A,int B,int clip);
void	_ASMAPI __EMU__fatPenEllipse(int left,int top,int A,int B,int clip);
void	_ASMAPI __EMU__fillEllipse(int left,int top, int A, int B,int clip);
void	_ASMAPI __EMU__ellipseArc(int left,int top,int A,int B,int startAngle,int endAngle,int clip);
void	_ASMAPI __EMU__fatPenEllipseArc(int left,int top,int A,int B,int startangle,int endangle,int clip);
void	_ASMAPI __EMU__fillEllipseArc(int left,int top,int A,int B,int startAngle,int endAngle,int clip);
void 	_ASMAPI __EMU__getArcCoords(arc_coords_t *coords);
void 	_ASMAPI __EMU__drawStrBitmap(int x,int y,const char *str);
void 	_ASMAPI __EMU__drawCharVec(int x,int y,vector *vec,int sz_nx,int sz_ny,int sz_dx,int sz_dy);
void	_ASMAPI __EMU__complexPolygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI __EMU__polygon(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void 	_ASMAPI __EMU__putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *image);
void    _ASMAPI __EMU__putMouseImage(MGLDC *dc,int x,int y,int byteWidth,int height,uchar *andMask,uchar *xorMask,color_t color);
long 	_ASMAPI __EMU__divotSize(MGLDC *dc,int left,int top,int right,int bottom);
void 	_ASMAPI __EMU__getDivot(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	_ASMAPI __EMU__putDivot(MGLDC *dc,void *divot);
void	_ASMAPI __EMU__getImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine);
void	_ASMAPI __EMU__putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine, MGLDC *src);
void	_ASMAPI __EMU__translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
void 	_ASMAPI __EMU__bitBlt(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,int incx,int incy);
void 	_ASMAPI __EMU__stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI __EMU__stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI __EMU__stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool idPal);
void 	_ASMAPI __EMU__srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
void 	_ASMAPI __EMU__dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent);
int  	_ASMAPI __EMU__scanRightForColor(int x,int y,color_t color);
int  	_ASMAPI __EMU__scanLeftForColor(int x,int y,color_t color);
int  	_ASMAPI __EMU__scanRightWhileColor(int x,int y,color_t color);
int  	_ASMAPI __EMU__scanLeftWhileColor(int x,int y,color_t color);
void 	_ASMAPI __EMU__scanLine(int y,int x1,int x2);

/* Software 3D rasterization */

void	_ASMAPI __EMU__cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,fix32_t y2,fix32_t c2);
void	_ASMAPI __EMU__rgbLine(fix32_t x1,fix32_t y1,fix32_t r1,fix32_t g1,fix32_t b1,fix32_t x2,fix32_t y2,fix32_t r2,fix32_t g2,fix32_t b2);
void	_ASMAPI __EMU__tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI __EMU__cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI __EMU__rgbTri(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI __EMU__quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI __EMU__cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset);
void	_ASMAPI __EMU__rgbQuad(fxpointrgb_t *v1,fxpointrgb_t *v2,fxpointrgb_t *v3,fxpointrgb_t *v4,fix32_t xOffset,fix32_t yOffset);
void    _ASMAPI __EMU__trap(void);
void	_ASMAPI __EMU__cTrap(void);
void	_ASMAPI __EMU__rgbTrap(void);

/* Software 3D Z-buffer rasterization */

ibool	_ASMAPI __EMU__zBegin(MGLDC *dc,int zbits);
void    _ASMAPI __EMU__zClear16(int left,int top,int right,int bottom,zfix32_t clearVal);
void    _ASMAPI __EMU__zClear32(int left,int top,int right,int bottom,zfix32_t clearVal);
void	_ASMAPI __EMU__zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,fix32_t y2,zfix32_t z2);
void	_ASMAPI __EMU__czLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t c1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t c2);
void	_ASMAPI __EMU__rgbzLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t r1,fix32_t b1,fix32_t g1,fix32_t x2,fix32_t y2,zfix32_t z2,fix32_t r2,fix32_t b2,fix32_t g2);
void	_ASMAPI __EMU__zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI __EMU__czTri(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI __EMU__rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI __EMU__zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI __EMU__czQuad(fxpointcz_t *v1,fxpointcz_t *v2,fxpointcz_t *v3,fxpointcz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI __EMU__rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset);
void	_ASMAPI __EMU__zTrap16(void);
void 	_ASMAPI __EMU__czTrap16(void);
void 	_ASMAPI __EMU__rgbzTrap16(void);
void	_ASMAPI __EMU__zTrap32(void);
void 	_ASMAPI __EMU__czTrap32(void);
void 	_ASMAPI __EMU__rgbzTrap32(void);
#define	__EMU__zTrap	(DC.zbits == 16 ? __EMU__zTrap16 : __EMU__zTrap32)
#define	__EMU__czTrap	(DC.zbits == 16 ? __EMU__czTrap16 : __EMU__czTrap32)
#define	__EMU__rgbzTrap	(DC.zbits == 16 ? __EMU__rgbzTrap16 : __EMU__rgbzTrap32)

#if defined(__WATCOMC__) && defined(__386__) && !defined(NOINLINEASM)

zfix32_t MGL_ZFixDiv(zfix32_t a,fix32_t b);
#pragma aux MGL_ZFixDiv =			\
	"xor	eax,eax"				\
	"shrd	eax,edx,28"				\
	"sar	edx,28"					\
	"idiv	ebx"              		\
	"mov	ecx,eax"				\
	"shl	edx,12"					\
	"mov	eax,edx"				\
	"xor	edx,edx"				\
	"div	ebx"					\
	"shl	eax,20"					\
	"shld	ecx,eax,12"				\
	parm [edx] [ebx] 				\
	value [ecx]             		\
	modify exact [eax edx];

#else

zfix32_t _ASMAPI MGL_ZFixDiv(zfix32_t a,fix32_t b);

#endif

#ifdef	__INTEL__
#pragma pack()				/* Return to default packing				*/
#endif

#endif	/* __MGL_H */

