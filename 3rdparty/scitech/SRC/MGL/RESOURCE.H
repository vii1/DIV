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
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the disk based resource file structures
*				used by the MGL.
*
*
****************************************************************************/

#ifndef	__RESOURCE_H
#define	__RESOURCE_H

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)				/* Pack structures to byte granularity		*/

/* Windows .CUR header and file structures */

typedef struct {
	uchar	bWidth;
	uchar	bHeight;
	uchar	bColorCount;
	uchar	bReserved;
	uchar	wXHotSpot[2];
	uchar	wYHotSpot[2];
	uchar	dwBytesInRes[4];
	uchar	dwImageOffset[4];
	} winCURSORDIRENTRY;

typedef struct {
	uchar				chReserved[2];
	uchar				chType[2];
	uchar				chCount[2];
	winCURSORDIRENTRY	chEntries[1];
	} winCURSORHEADER;

/* Windows .BMP header and file structures */

typedef struct {
	uchar   bfType[2];
	uchar	bfSize[4];
	uchar   bfRes[4];
	uchar	bfOffBits[4];
	} winBITMAPFILEHEADER;

typedef struct {
	uchar	biSize[4];
	uchar	biWidth[4];
	uchar	biHeight[4];
	uchar   biPlanes[2];
	uchar	biBitCount[2];
	uchar	biCompression[4];
	uchar	biSizeImage[4];
	uchar	biXPelsPerMeter[4];
	uchar	biYPelsPerMeter[4];
	uchar	biClrUsed[4];
	uchar	biClrImportant[4];
	} winBITMAPINFOHEADER;

typedef struct {
	ulong   biSize;
	long    biWidth;
	long    biHeight;
	ushort  biPlanes;
	ushort  biBitCount;
	ulong   biCompression;
	ulong   biSizeImage;
	long    biXPelsPerMeter;
	long    biYPelsPerMeter;
	ulong   biClrUsed;
	ulong	biClrImportant;
	} winBITMAPINFOHEADER_full;

typedef struct {
	uchar	rgbBlue;
	uchar   rgbGreen;
	uchar   rgbRed;
	uchar  	rgbReserved;
	} winRGBQUAD;

typedef struct {
	winBITMAPINFOHEADER_full	header;
	winRGBQUAD					colors[256];
	} winBITMAPINFO;

typedef struct {
	winBITMAPINFOHEADER_full	header;
	ulong						maskRed;
	ulong						maskGreen;
	ulong						maskBlue;
	} winBITMAPINFOEXT;

/* Constants for the biCompression field */

#define winBI_RGB      	0L
#define winBI_RLE8     	1L
#define winBI_RLE4     	2L
#define	winBI_BITFIELDS	3L

/* Windows .ICO header and file structures */

typedef struct {
	uchar	bWidth;
	uchar	bHeight;
	uchar	bColorCount;
	uchar	bRes;
	uchar	wRes1[2];
	uchar	wRes2[2];
	uchar	dwBytesInRes[4];
	uchar	dwImageOffset[4];
	} winICONDIRECTORY;

typedef struct {
	uchar				ihRes[2];
	uchar				ihType[2];
	uchar				ihCount[2];
	winICONDIRECTORY	ihEntries[1];
	} winICONHEADER;

/* Windows 2.x .FNT header and file structures */

typedef struct {
	ushort  dfVersion;
	ulong	dfSize;
	uchar	dfCopyright[60];
	ushort	dfType;
	} winFONTHEADER;

typedef struct {
	ushort	dfPoints;
	ushort	dfVertRes;
	ushort  dfHorizRes;
	ushort  dfAscent;
	ushort  dfInternalLeading;
	ushort	dfExternalLeading;
	} winFONTSPACING;

typedef struct {
	uchar	dfItalic;
	uchar   dfUnderline;
	uchar   dfStrikeOut;
	ushort  dfWeight;
	uchar   dfCharSet;
	ushort	dfPixWidth;
	ushort	dfPixHeight;
	uchar	dfPitchAndFamily;
	ushort	dfAvgWidth;
	ushort	dfMaxWidth;
	} winFONTSTYLE;

typedef struct {
	uchar	dfFirstChar;
	uchar	dfLastChar;
	uchar	dfDefaultChar;
	uchar	dfBreakChar;
	} winFONTCHAR;

typedef struct {
	ushort  dfWidthBytes;
	ulong	dfDevice;
	ulong	dfFace;
	ulong	dfBitsPointer;
	ulong	dfBitsOffset;
	uchar  	dfRes1;
	/* Variable length font info resides in here */
	} winFONTMISC;

typedef struct {
	winFONTHEADER	fh;
	winFONTSPACING	fs;
	winFONTSTYLE	ft;
	winFONTCHAR		fc;
	winFONTMISC		fm;
	} winFONTINFO;

typedef struct {
	ushort	gWidth;
	ushort	gOffset;
	} winGLYPH;

/* PCX file format header information */

typedef struct {
	char	format;						/* Always 10 for PCX		*/
	char	version;					/* Version info				*/
	char    rleflag;					/* Set to 1					*/
	char	bitpx;						/* Bits per pixel			*/
	uchar	x1[2],y1[2],x2[2],y2[2];	/* Image bounds in pixels	*/
	uchar	hres[2],vres[2];			/* Image resolution in dpi	*/
	uchar	colors[48];					/* Palette					*/
	char	vmode;						/* (ignored)				*/
	char	nplanes;					/* Plane count (v2.5=0)		*/
	uchar  	bplin[2];					/* Bytes per scanline		*/
	uchar	paltype;					/* 1 for color, 2 for gray	*/
	uchar	res1;						/* Reserved (0)				*/
	uchar 	scrnw[2],scrnh[2];			/* Screen size in pixels	*/
	char	extra[54];					/* Pad to 128 bytes			*/
	} PCXHEADER;

#define	PCX_RUN_MASK	0xC0			/* Mask top 2 bits			*/
#define	PCX_CNT_MASK	0x3F			/* Mask out pixel count		*/
#define	PCX_MAX_RUN		0x3F			/* Maxium repeat count		*/

/* Macros for extracting values from resource file structures in little
 * endian or big endian format. We define the following macros:
 *
 *	getLEShort()	- Get a short in little endian format
 *	getLELong()		- Get a long in little endian format
 *	getBEShort()	- Get a short in big endian format
 *	getBESHory()	- Get a long in big endian format
 *
 * Note that all of these macros expect the input argument to be an
 * array of characters.
 */

#ifdef	BIG_ENDIAN
#define	getBEShort(v)	(*((ushort*)&v[0]))
#define	getBELong(v)	(*((ulong*)&v[0]))
#define	getLEShort(v)	(((ushort)v[1] << 8) | (ushort)v[0])
#define	getLELong(v)    (((ulong)v[3] << 24) | ((ulong)v[2] << 16)	\
							| ((ulong)v[1] << 8) | (ulong)v[0])
#else
#define	getLEShort(v)	(*((ushort*)&v[0]))
#define	getLELong(v)	(*((ulong*)&v[0]))
#define	putLEShort(p,v)	(*((ushort*)&p[0])) = (v)
#define	putLELong(p,v)	(*((ulong*)&p[0])) = (v)
#define	getBEShort(v)	(((ushort)v[0] << 8) | (ushort)v[1])
#define	getBELong(v)    (((ulong)v[0] << 24) | ((ulong)v[1] << 16)	\
							| ((ulong)v[2] << 8) | (ulong)v[3])
#endif

#pragma pack()				/* Return to default packing				*/

#endif	/* __RESOURCE_H */

