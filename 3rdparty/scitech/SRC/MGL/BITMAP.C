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
* Description:	Bitmap resource loading/unloading routines. Bitmaps are
*				stored on disk in standard Windows .BMP Device Independant
*				Bitmap format.
*
*
****************************************************************************/

#include "mgl.h"

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

static ibool openBitmapFileExt(winBITMAPFILEHEADER *h,winBITMAPINFOHEADER *b,
	FILE *f,ulong dwOffset)
/****************************************************************************
*
* Function:		openBitmapFileExt
* Parameters:	h			- Bitmap header info to fill in
*				b			- Bitmap info structure to fill info
*				f			- Open binary file to read bitmap from
*				dwOffset	- Offset to start of bitmap in file
* Returns:		True on success, false on error.
*
* Description:	Attempts to open the specified bitmap and read the
*				bitmap header information. If the header information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	size_t	size;

	/* Read in the header record and verify the bitmap file */
	_MGL_result = grOK;
	_MGL_fseek(f,dwOffset,SEEK_SET);
	size = _MGL_fread(h,1,sizeof(winBITMAPFILEHEADER),f);
	if (size != sizeof(winBITMAPFILEHEADER) || h->bfType[0] != 'B'
			|| h->bfType[1] != 'M') {
		_MGL_result = grInvalidBitmap;
		return false;
		}

	/* Read in the bitmap info header and verify it */
	size = _MGL_fread(b,1,sizeof(winBITMAPINFOHEADER),f);
	if (size != sizeof(winBITMAPINFOHEADER) || b->biSize[0] != 0x28
			|| b->biPlanes[0] != 1 ||
			(b->biCompression[0] != winBI_RGB && b->biBitCount[0] <= 8)) {
		_MGL_result = grInvalidBitmap;
		return false;
		}
	return true;
}

static FILE *openBitmapFile(winBITMAPFILEHEADER *h,winBITMAPINFOHEADER *b,
	const char *bitmapName)
/****************************************************************************
*
* Function:		openBitmapFile
* Parameters:	h			- Bitmap header info to fill in
*				b			- Bitmap info structure to fill info
*				bitmapName	- Name of the bitmap file to load
* Returns:		Handle to the opened file, NULL on error.
*
* Description:	Attempts to open the specified bitmap file and read the
*				bitmap header information. If the header information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	FILE	*f;

	_MGL_result = grOK;
	f = _MGL_openFile(MGL_BITMAPS, bitmapName, "rb");
	if (f == NULL) {
		_MGL_result = grBitmapNotFound;
		return NULL;
		}
	if (!openBitmapFileExt(h,b,f,0)) {
		_MGL_fclose(f);
		return NULL;
		}
	return f;
}

static ibool readBitmapHeaderExt(bitmap_t *bitmap,palette_t *pal,
	int *palSize,pixel_format_t *pf,FILE *f,ulong dwOffset,ibool loadPalette)
/****************************************************************************
*
* Function:		readBitmapHeaderExt
* Returns:		Pointer to file for reading bitmap bits
*
* Description:	Locates the specified bitmap file and loads it into memory.
*				If 'loadPalette' is true, the palette values for the bitmap
*				will be loaded into the structure as well (if there is no
*				palette, it will not be loaded), otherwise the palette
*				entry for the bitmap will be NULL.
*
****************************************************************************/
{
	winBITMAPFILEHEADER	header;
	winBITMAPINFOHEADER	bmInfo;
	long				bytesPerLine,clrUsed;
    ulong				masks[3];

	_MGL_result = grOK;
	if (!openBitmapFileExt(&header,&bmInfo,f,dwOffset))
		return false;

	/* Load the palette values into the local palette if present, or
	 * the pixel format information for RGB images
	 */
	switch (bmInfo.biBitCount[0]) {
		case 1:		*palSize = 2 * sizeof(palette_t);	break;
		case 4:		*palSize = 16 * sizeof(palette_t);	break;
		case 8:		*palSize = 256 * sizeof(palette_t);	break;
		default:	*palSize = sizeof(pixel_format_t);	break;
		}
	if (bmInfo.biBitCount[0] <= 8) {
		if ((clrUsed = getLELong(bmInfo.biClrUsed) * sizeof(palette_t)) == 0)
			clrUsed = *palSize;
		if (loadPalette)
			_MGL_fread(pal,1,clrUsed,f);
		else
			_MGL_fseek(f,clrUsed,SEEK_CUR);
		}
	else if (bmInfo.biCompression[0] == winBI_BITFIELDS) {
		/* Load the BI_BITFIELDS info from the first three palette
		 * entries for new style bitmap formats.
		 */
		_MGL_fread(masks,1,3 * sizeof(ulong),f);
		}

	/* Load the header information into the bitmap header. Note that
	 * we DWORD align the bitmap data, as DIB's are always DWORD aligned
	 */
	bitmap->width = getLEShort(bmInfo.biWidth);
	bitmap->height = getLEShort(bmInfo.biHeight);
	bitmap->bitsPerPixel = bmInfo.biBitCount[0];
	switch (bmInfo.biBitCount[0]) {
		case 1:		bytesPerLine = (bitmap->width+7)/8;	break;
		case 4:		bytesPerLine = (bitmap->width+1)/2;	break;
		case 8:		bytesPerLine = bitmap->width;		break;
		case 16:	bytesPerLine = bitmap->width * 2;	break;
		case 24:	bytesPerLine = bitmap->width * 3;	break;
		case 32:	bytesPerLine = bitmap->width * 4;	break;
		default:
			_MGL_result = grInvalidBitmap;
			return false;
		}
	bitmap->bytesPerLine = (bytesPerLine + 3) & ~3;

	/* Create a pixel format block for 15 bit and higher bitmaps */
	if (bitmap->bitsPerPixel > 8) {
		switch (bitmap->bitsPerPixel) {
			case 16:
				if (bmInfo.biCompression[0] == winBI_BITFIELDS && masks[1] == 0x7E0UL)
					*pf = _MGL_pixelFormats[pfRGB565];
				else {
					*pf = _MGL_pixelFormats[pfRGB555];
					bitmap->bitsPerPixel = 15;
					}
				break;
			case 24:
				if (bmInfo.biCompression[0] == winBI_BITFIELDS && masks[0] == 0xFFUL)
					*pf = _MGL_pixelFormats[pfBGR24];
				else *pf = _MGL_pixelFormats[pfRGB24];
				break;
			case 32:
				if (bmInfo.biCompression[0] == winBI_BITFIELDS) {
					if (masks[0] == 0xFFUL)
						*pf = _MGL_pixelFormats[pfABGR32];
					else if (masks[0] == 0xFF0000UL)
						*pf = _MGL_pixelFormats[pfARGB32];
					else if (masks[0] == 0xFF00UL)
						*pf = _MGL_pixelFormats[pfBGRA32];
					else *pf = _MGL_pixelFormats[pfRGBA32];
					}
				else *pf = _MGL_pixelFormats[pfARGB32];
				break;
			}
		}
	return true;
}

/****************************************************************************
DESCRIPTION:
Locates an open bitmap and loads it into memory from an open file.

HEADER:
mgraph.h

PARAMETERS:
f			- Open binary file to read data from
dwOffset	- Offset to start of bitmap in file
dwSize		-	Size of the file
loadPalette	- Should we load the palette values as well?

RETURNS:
Pointer to the loaded bitmap file

REMARKS:
This function is the same as MGL_loadBitmap, however it loads the file from a
previously open file. This allows you to create your own large files with
multiple files embedded in them.

SEE ALSO:
MGL_loadBitmap, MGL_loadBitmapIntoDC
****************************************************************************/
bitmap_t * MGLAPI MGL_loadBitmapExt(
	FILE *f,
	ulong dwOffset,
	ulong dwSize,
	ibool loadPalette)
{
	bitmap_t			bmh,*bitmap;
	palette_t			pal[256];		/* Temporary space for palette	*/
	pixel_format_t		pf;
	long				size;
	int					i,palSize;
	uchar				_HUGE *p;

    /* Open the bitmap header */
	if (!readBitmapHeaderExt(&bmh,pal,&palSize,&pf,f,dwOffset,loadPalette))
		return NULL;

	/* Allocate memory for the bitmap */
	if (!loadPalette)
		palSize = 0;
	size = (long)bmh.bytesPerLine * bmh.height + palSize;
	if ((bitmap = MGL_malloc(sizeof(bitmap_t)+size)) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}
	*bitmap = bmh;

	size = sizeof(bitmap_t);
	if (bitmap->bitsPerPixel <= 8) {
		if (palSize) {
			bitmap->pal = (palette_t*)((uchar*)bitmap + size);
			memcpy(bitmap->pal,pal,palSize);
			}
		else
			bitmap->pal = NULL;
		bitmap->pf = NULL;
		}
	else {
		bitmap->pf = (pixel_format_t*)((uchar*)bitmap + size);
		*bitmap->pf = pf;
		bitmap->pal = NULL;
		}
	size += palSize;
	bitmap->surface = (uchar*)bitmap + size;

	/* Now read in the bits in the bitmap. We need to handle both cases
	 * of bottom up and top down DIB's.
	 */
	if (bitmap->height < 0) {
		bitmap->height = -bitmap->height;
		p = bitmap->surface;
		for (i = 0; i < bitmap->height; i++, p += bitmap->bytesPerLine) {
			NORMALISE_PTR(p);
			_MGL_fread(p,1,bitmap->bytesPerLine,f);
			}
		}
	else {
		p = (uchar _HUGE *)bitmap->surface + (long)bitmap->bytesPerLine * (bitmap->height-1);
		for (i = 0; i < bitmap->height; i++, p -= bitmap->bytesPerLine) {
			NORMALISE_PTR(p);
			_MGL_fread(p,1,bitmap->bytesPerLine,f);
			}
		}
	return bitmap;
}

/****************************************************************************
DESCRIPTION:
Load a lightweight bitmap file from disk.

HEADER:
mgraph.h

PARAMETERS:
bitmapName	- Name of bitmap file to load
loadPalette	- True if the palette should also be loaded

RETURNS:
Pointer to the loaded bitmap file, NULL on error.

REMARKS:
Locates the specified bitmap file and loads it into a lightweight bitmap structure.
MGL can load any Windows 3.x style bitmap files, including new format bitmap
files with colors depths of 15/16 and 32 bits per pixel. Consult the Windows SDK
documentation for the format of Windows bitmap files.

If loadPalette is true, the palette values for the bitmap will be loaded into the
structure as well (if there is no palette, it will not be loaded), otherwise the palette
entry for the bitmap will be NULL. For small bitmaps you can save space by not
loading the palette for the bitmap.

When MGL is searching for bitmap files it will first attempt to find the files just be
using the filename itself. Hence if you wish to look for a specific bitmap file, you
should pass the full pathname to the file that you are interested in. If the filename is
a simple relative filename (i.e. "MYBMP.BMP"), MGL will then search in the
BITMAPS directory relative to the path specified in mglpath variable that was
passed to MGL_init. As a final resort MGL will also look for the files in the
BITMAPS directory relative to the MGL_ROOT environment variable.

If the bitmap file was not found, or an error occurred while reading the bitmap file,
this function will return NULL. You can check the MGL_result error code to
determine the cause.

The routine allocates a lightweight bitmap structure for holding the bitmap, which
loads the bitmap with the minimum memory overheads. You can draw the bitmap
on any device context surface using the MGL_putBitmap function, but you don't
have the full flexibility of using a full memory device context for the bitmap
surface. If you need more control over the bitmap, you can allocate a memory
device context to hold the bitmap surface and load the bitmap with the
MGL_loadBitmapIntoDC function.

SEE ALSO:
MGL_unloadBitmap, MGL_availableBitmap, MGL_getBitmapSize,
MGL_loadBitmapIntoDC, MGL_saveBitmapFromDC, MGL_putBitmap, MGL_loadBitmapExt
****************************************************************************/
bitmap_t * MGLAPI MGL_loadBitmap(
	const char *bitmapName,
	ibool loadPalette)
{
	FILE		*f;
	bitmap_t	*bitmap;

	_MGL_result = grOK;
	if ((f = _MGL_openFile(MGL_BITMAPS, bitmapName, "rb")) == NULL) {
		_MGL_result = grBitmapNotFound;
		return NULL;
		}
	bitmap = MGL_loadBitmapExt(f,0,_MGL_fileSize(f),loadPalette);
	_MGL_fclose(f);
	return bitmap;
}

/****************************************************************************
DESCRIPTION:
Obtain the dimensions of a bitmap from an open file.

HEADER:
mgraph.h

PARAMETERS:
f				- Pointer to opened bitmap file
dwSize			- Size of the bitmap file
dwOffset		- Offset into the file
width			- Place to store the bitmap width
height			- Place to store the bitmap height
bitsPerPixel	- Place to store the bitmap pixel depth
pf				- Place to store the bitmap pixel format

RETURNS:
True if the bitmap was found, otherwise false.

REMARKS:
This function is the same as MGL_getBitmapSize, however it works with a
previously opened file. This allows you to create your own large files with
multiple files embedded in them.

SEE ALSO:
MGL_getBitmapSize
****************************************************************************/
ibool MGLAPI MGL_getBitmapSizeExt(
	FILE *f,
	ulong dwOffset,
	ulong dwSize,
	int *width,
	int *height,
	int *bitsPerPixel,
	pixel_format_t *pf)
{
	palette_t	pal[256];
	bitmap_t	bmh;
	int			palSize;

	if (!readBitmapHeaderExt(&bmh,pal,&palSize,pf,f,dwOffset,false))
		return false;
	*width = bmh.width;
	*height = ABS(bmh.height);
	*bitsPerPixel = bmh.bitsPerPixel;
	return true;
}

/****************************************************************************
DESCRIPTION:
Obtain the dimensions of a bitmap file from disk.

HEADER:
mgraph.h

PARAMETERS:
bitmapName	- Name of the bitmap file to load header for
width	- Place to store the bitmap width
height	- Place to store the bitmap height
bitsPerPixel	- Place to store the bitmap pixel depth
pf	- Place to store the bitmap pixel format information

RETURNS:
True if the bitmap was found, false if not.

REMARKS:
This functions loads all the header information for a bitmap file from disk, without
actually loading the bits for the bitmap surface. This is useful to determine the
dimensions and pixel format for the bitmap before it is loaded, so you can create an
appropriate memory device context that you can load the bitmap into with the
MGL_loadBitmapIntoDC function.

SEE ALSO:
MGL_loadBitmap, MGL_loadBitmapExt
****************************************************************************/
ibool MGLAPI MGL_getBitmapSize(
	const char *bitmapName,
	int *width,
	int *height,
	int *bitsPerPixel,
	pixel_format_t *pf)
{
	FILE	*f;
	ibool	ret;

	_MGL_result = grOK;
	if ((f = _MGL_openFile(MGL_BITMAPS, bitmapName, "rb")) == NULL) {
		_MGL_result = grBitmapNotFound;
		return false;
		}
	ret = MGL_getBitmapSizeExt(f,0,_MGL_fileSize(f),width,height,bitsPerPixel,pf);
	_MGL_fclose(f);
	return ret;
}

/****************************************************************************
DESCRIPTION:
Locates the specified bitmap file and loads it into a device context.

HEADER:
mgraph.h

PARAMETERS:
dc			- Device context to load bitmap into
f			- Pointer to opened bitmap file
dwSize		- Size of the bitmap file
dwOffset	- Offset into the file
dstLeft		- Left coordinate to place bitmap at
dstTop		- Top coordinate to place bitmap at
loadPalette	- Should we load the palette values as well?

RETURNS:
True if the bitmap was successfully loaded.

REMARKS:
This function is the same as MGL_loadBitmapIntoDC, however it works with a
previously opened file. This allows you to create your own large files with
multiple files embedded in them.

SEE ALSO:
MGL_loadBitmapIntoDC
****************************************************************************/
ibool MGLAPI MGL_loadBitmapIntoDCExt(
	MGLDC *dc,
	FILE *f,
	ulong dwOffset,
	ulong dwSize,
	int dstLeft,
	int dstTop,
	ibool loadPalette)
{
	bitmap_t			bmh;
	MGLDC				*memDC;
	palette_t			pal[256];		/* Temporary space for palette	*/
	pixel_format_t		pf;
	int					i,palSize;
	ibool				oldCheckId;

    /* Read bitmap header */
	if (!readBitmapHeaderExt(&bmh,pal,&palSize,&pf,f,dwOffset,loadPalette))
		return false;

	/* Allocate a temporary memory DC for the bitmap scanlines */
	if ((memDC = MGL_createMemoryDC(bmh.width,1,bmh.bitsPerPixel,&pf)) == NULL)
		return false;
	oldCheckId = MGL_checkIdentityPalette(FALSE);

	/* Store the palette in the destination DC */
	if (loadPalette && (bmh.bitsPerPixel == 4 || bmh.bitsPerPixel == 8)) {
		MGL_setPalette(memDC,pal,palSize / sizeof(palette_t),0);
		MGL_setPalette(dc,pal,palSize / sizeof(palette_t),0);
		if (MGL_getVisualPage(dc) == MGL_getActivePage(dc))
			MGL_realizePalette(dc,palSize / sizeof(palette_t),0,false);
		}
	else if (loadPalette && (dc->mi.bitsPerPixel == 8 && bmh.bitsPerPixel > 8)) {
		MGL_getHalfTonePalette(pal);
		MGL_setPalette(memDC,pal,palSize / sizeof(palette_t),0);
		MGL_setPalette(dc,pal,MGL_getPaletteSize(dc),0);
		if (MGL_getVisualPage(dc) == MGL_getActivePage(dc))
			MGL_realizePalette(dc,MGL_getPaletteSize(dc),0,false);
		}

	/* Now read in the bits in the bitmap, by reading the data a scanline
	 * at a time into our temporary memory DC, and then blting this to
	 * the destination DC. We need to handle both cases of bottom up and
	 * top down DIB's.
	 */
	if (bmh.height < 0) {
		/* Top down DIB */
		bmh.height = -bmh.height;
		for (i = 0; i < bmh.height; i++) {
			_MGL_fread(memDC->surface,1,memDC->mi.bytesPerLine,f);
			MGL_bitBltCoord(dc,memDC,0,0,bmh.width,1,dstLeft,dstTop+i,
				MGL_REPLACE_MODE);
			}
		}
	else {
		/* Bottom up DIB */
		for (i = bmh.height-1; i >= 0; i--) {
			_MGL_fread(memDC->surface,1,memDC->mi.bytesPerLine,f);
			MGL_bitBltCoord(dc,memDC,0,0,bmh.width,1,dstLeft,dstTop+i,
				MGL_REPLACE_MODE);
			}
		}
	MGL_destroyDC(memDC);
	MGL_checkIdentityPalette(oldCheckId);
	return true;
}

/****************************************************************************
DESCRIPTION:
Loads a bitmap file directly into an existing device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to load bitmap into
bitmapName	- Name of bitmap file to load
dstLeft	- Left coordinate to load bitmap at
dstTop	- Top coordinate to load bitmap at
loadPalette	- True if the palette should also be loaded

RETURNS:
True if the bitmap was loaded, false on error.

REMARKS:
Locates the specified bitmap file and loads it into the specified device context at the
specified destination coordinates. If the bitmap is of a different pixel depth then the
device context that it is being loaded into, the bitmap will be converted as it is
loaded to the pixel format of the device context it is being loaded into. MGL can
load any Windows 3.x style bitmap files, including new format bitmap files with
colors depths of 15/16 and 32 bits per pixel. Consult the Windows SDK
documentation for the format of Windows bitmap files.

If loadPalette is true, the palette values for the bitmap will be loaded and stored in
the device context's palette. If the device context being loaded into is the currently
active display device, the palette will also be realized before the bits in the bitmap
are loaded.

When MGL is searching for bitmap files it will first attempt to find the files just by
using the filename itself. Hence if you wish to look for a specific bitmap file, you
should pass the full pathname to the file that you are interested in. If the filename is
a simple relative filename (i.e. "MYBMP.BMP"), MGL will then search in the
BITMAPS directory relative to the path specified in mglpath variable that was
passed to MGL_init. As a final resort MGL will also look for the files in the
BITMAPS directory relative to the MGL_ROOT environment variable.

If the bitmap file was not found, or an error occurred while reading the bitmap file,
this function will return false. You can check the MGL_result error code to
determine the cause.

SEE ALSO:
MGL_availableBitmap, MGL_getBitmapSize, MGL_loadBitmap,
MGL_saveBitmapFromDC, MGL_loadBitmapIntoDCExt
****************************************************************************/
ibool MGLAPI MGL_loadBitmapIntoDC(
	MGLDC *dc,
	const char *bitmapName,
	int dstLeft,
	int dstTop,
	ibool loadPalette)
{
	FILE	*f;
	ibool	ret;

	_MGL_result = grOK;
	if ((f = _MGL_openFile(MGL_BITMAPS, bitmapName, "rb")) == NULL) {
		_MGL_result = grBitmapNotFound;
		return false;
		}
	ret = MGL_loadBitmapIntoDCExt(dc,f,0,_MGL_fileSize(f),dstLeft,dstTop,loadPalette);
	_MGL_fclose(f);
	return ret;
}

static void createBitmapHeader(MGLDC *dc,int *left,int *top,int *right,
	int *bottom,bitmap_t *bitmap,int *palSize)
/****************************************************************************
*
* Function:		createBitmapHeader
* Parameters:	dc		- Device context to create bitmap from
*               left	- Left coordinate of rectangle to save
*				top		- Top coordinate of rectangle to save
*				right	- Right coordinate of rectangle to save
*				bottom	- Bottom coordinate of rectangle to save
*				bitmap	- Bitmap header to fill in
*				palSize	- Place to store size of palette
*
* Description:	Fills in the bitmap header for the sub-portion of the
*				specified device context.
*
****************************************************************************/
{
	int		bytesPerLine;

	/* Clip the source rectangle to the device context bounds */
	*left += dc->a.viewPort.left;
	*right += dc->a.viewPort.left;
	*top += dc->a.viewPort.top;
	*bottom += dc->a.viewPort.top;
	if (*left < 0) *left = 0;
	if (*right > (dc->mi.xRes+1)) *right = dc->mi.xRes+1;
	if (*top < 0) *top = 0;
	if (*bottom > (dc->mi.yRes+1)) *bottom = dc->mi.yRes+1;

	/* Create the header information into the bitmap header. Note that
	 * we DWORD align the bitmap data, as DIB's are always DWORD aligned
	 */
	bitmap->width = (*right-*left);
	bitmap->height = (*bottom-*top);
	bitmap->bitsPerPixel = dc->mi.bitsPerPixel;
	switch (dc->mi.bitsPerPixel) {
		case 1:		bytesPerLine = (bitmap->width+7)/8;	break;
		case 4:		bytesPerLine = (bitmap->width+1)/2;	break;
		case 8:		bytesPerLine = bitmap->width;		break;
		case 15:
		case 16:	bytesPerLine = bitmap->width * 2;	break;
		case 24:	bytesPerLine = bitmap->width * 3;	break;
		default:	bytesPerLine = bitmap->width * 4;	break;
		}
	bitmap->bytesPerLine = (bytesPerLine + 3) & ~3;

	/* Determine the size of the palette data */
	switch (bitmap->bitsPerPixel) {
		case 1:		*palSize = 2 * sizeof(palette_t);	break;
		case 4:		*palSize = 16 * sizeof(palette_t);	break;
		case 8:		*palSize = 256 * sizeof(palette_t);	break;
		default:	*palSize = sizeof(pixel_format_t);	break;
		}
}

/****************************************************************************
DESCRIPTION:
Save a portion of a device context to bitmap file on disk.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to save
bitmapName	- Name of bitmap file to save
left	- Left coordinate of bitmap to save
top	- Top coordinate of bitmap to save
right	- Right coordinate of bitmap to save
bottom	- Bottom coordinate of bitmap to save

RETURNS:
True on success, false on error.

REMARKS:
This function saves a portion of a device context as a bitmap file to disk. If this
function fails for some reason, it will return false and you can get the error code
from the MGL_result function.

Note that the source rectangle for the bitmap to be saved is not clipped to the
current clipping rectangle for the device context, but it is mapped to the current
viewport. If you specify dimensions that are larger than the current device context,
you will get garbage in the bitmap file as a result.

SEE ALSO:
MGL_loadBitmap,MGL_loadBitmapIntoDC
****************************************************************************/
ibool MGLAPI MGL_saveBitmapFromDC(
	MGLDC *dc,
	const char *bitmapName,
	int left,
	int top,
	int right,
	int bottom)
{
	FILE				*f;
	bitmap_t			bmh;
	winBITMAPFILEHEADER	hdr;
	winBITMAPINFOHEADER	bmInfo;
	long				size;
	int					i,palSize;
	uchar				_HUGE *p;
	ulong				masks[3];

	/* Attempt to open the file for writing */
	if ((f = _MGL_fopen(bitmapName,"wb")) == NULL)
		return false;

	/* Build the bitmap file header and write to disk */
	createBitmapHeader(dc,&left,&top,&right,&bottom,&bmh,&palSize);
	memset(&hdr,0,sizeof(hdr));
	hdr.bfType[0] = 'B';
	hdr.bfType[1] = 'M';

	/* Build bitmap info header and write to disk */
	memset(&bmInfo,0,sizeof(bmInfo));
	putLELong(bmInfo.biSize,sizeof(winBITMAPINFOHEADER));
	putLELong(bmInfo.biWidth,bmh.width);
	putLELong(bmInfo.biHeight,bmh.height);
	putLELong(bmInfo.biPlanes,1);
	switch (bmh.bitsPerPixel) {
		case 1:
			putLELong(bmInfo.biBitCount,1);
			putLELong(bmInfo.biCompression,winBI_RGB);
			putLELong(bmInfo.biClrUsed,2);
			break;
		case 4:
			putLELong(bmInfo.biBitCount,4);
			putLELong(bmInfo.biCompression,winBI_RGB);
			putLELong(bmInfo.biClrUsed,16);
			break;
		case 8:
			putLELong(bmInfo.biBitCount,8);
			putLELong(bmInfo.biCompression,winBI_RGB);
			putLELong(bmInfo.biClrUsed,256);
			break;
		case 15:
			putLELong(bmInfo.biBitCount,16);
			putLELong(bmInfo.biCompression,winBI_RGB);
			putLELong(bmInfo.biClrUsed,0);
			break;
		case 16:
			putLELong(bmInfo.biBitCount,16);
			putLELong(bmInfo.biClrUsed,0);
			putLELong(bmInfo.biCompression,winBI_BITFIELDS);
			masks[0] = 0x0000F800UL;
			masks[1] = 0x000007E0UL;
			masks[2] = 0x0000001FUL;
			break;
		case 24:
			putLELong(bmInfo.biBitCount,24);
			putLELong(bmInfo.biClrUsed,0);
			if (dc->pf.redPos == 0) {
				/* Bitmap is in the extended 24 bit BGR format */
				putLELong(bmInfo.biCompression,winBI_BITFIELDS);
				masks[0] = 0x0000FFUL;
				masks[1] = 0x00FF00UL;
				masks[2] = 0xFF0000UL;
				}
			else putLELong(bmInfo.biCompression,winBI_RGB);
			break;
		case 32:
			putLELong(bmInfo.biBitCount,32);
			putLELong(bmInfo.biClrUsed,0);
			if (dc->pf.rsvdPos != 0) {
				if (dc->pf.redPos != 0)
					putLELong(bmInfo.biCompression,winBI_RGB);
				else {
					/* Bitmap is in the extended 32 bit ABGR format */
					putLELong(bmInfo.biCompression,winBI_BITFIELDS);
					masks[0] = 0x000000FFUL;
					masks[1] = 0x0000FF00UL;
					masks[2] = 0x00FF0000UL;
					}
				}
			else {
				if (dc->pf.redPos != 8) {
					putLELong(bmInfo.biCompression,winBI_BITFIELDS);
					masks[0] = 0xFF000000UL;
					masks[1] = 0x00FF0000UL;
					masks[2] = 0x0000FF00UL;
					}
				else {
					putLELong(bmInfo.biCompression,winBI_BITFIELDS);
					masks[0] = 0x0000FF00UL;
					masks[1] = 0x00FF0000UL;
					masks[2] = 0xFF000000UL;
					}
				}
			break;
		}
	size = (long)bmh.bytesPerLine * bmh.height;
	putLELong(bmInfo.biSizeImage,size);
	putLELong(bmInfo.biClrImportant,0);

	/* Write header and palette data to disk */
	if (bmh.bitsPerPixel <= 8)
		size += sizeof(hdr) + sizeof(bmInfo) + palSize;
	else if (getLELong(bmInfo.biCompression) == winBI_BITFIELDS)
		size += sizeof(hdr) + sizeof(bmInfo) + sizeof(masks);
	else
		size += sizeof(hdr) + sizeof(bmInfo);
	putLELong(hdr.bfSize,size);
	putLELong(hdr.bfOffBits,size - getLELong(bmInfo.biSizeImage));
	_MGL_fwrite(&hdr,1,sizeof(hdr),f);
	_MGL_fwrite(&bmInfo,1,sizeof(bmInfo),f);
	if (bmh.bitsPerPixel <= 8)
		_MGL_fwrite(dc->colorTab,1,palSize,f);
	else if (getLELong(bmInfo.biCompression) == winBI_BITFIELDS)
		_MGL_fwrite(masks,1,sizeof(masks),f);

	/* Now write the bits from the device context to disk. Note that we
	 * write the data in bottom up DIB format, which is the standard DIB
	 * format for files saved to disk
	 */
	dc->r.beginPixel(dc);
	p = _MGL_buf;
	for (i = bottom-1; i >= top; i--) {
		NORMALISE_PTR(p);
		dc->r.getScanLine(dc,i,left,right,p,left,MGL_REPLACE_MODE);
		_MGL_fwrite(p,1,bmh.bytesPerLine,f);
		}
	dc->r.endPixel(dc);

	_MGL_fclose(f);
	return true;
}

/****************************************************************************
DESCRIPTION:
Copy a portion of a device context as a lightweight memory bitmap.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to save from
left	- Left coordinate of bitmap to save
top	- Top coordinate of bitmap to save
right	- Right coordinate of bitmap to save
bottom	- Bottom coordinate of bitmap to save
savePalette	- Save palette with bitmap.

RETURNS:
Pointer to allocated bitmap, NULL on error.

REMARKS:
This function copies a portion of a device context as a lightweight memory bitmap.
If this function fails (for instance if out of memory), it will return NULL and you
can get the error code from the MGL_result function. Otherwise this function will
return a pointer to a new lightweight bitmap containing the bitmap data.

Note that the source rectangle for the bitmap to be saved is not clipped to the
current clipping rectangle for the device context, but it is mapped to the current
viewport. If you specify dimensions that are larger than the current device context,
you will get garbage in the bitmap file as a result.

SEE ALSO:
MGL_loadBitmap, MGL_saveBitmapFromDC
****************************************************************************/
bitmap_t * MGLAPI MGL_getBitmapFromDC(
	MGLDC *dc,
	int left,
	int top,
	int right,
	int bottom,
	ibool savePalette)
{
	bitmap_t		*bitmap,bmh;
	long			size;
	int				i,palSize;
	uchar			_HUGE *p;

	/* Build the bitmap header and allocate memory for it */
	createBitmapHeader(dc,&left,&top,&right,&bottom,&bmh,&palSize);
	if (!savePalette && dc->mi.bitsPerPixel <= 8)
		palSize = 0;
	size = (long)bmh.bytesPerLine * bmh.height + palSize;
	if ((bitmap = MGL_malloc(sizeof(bitmap_t)+size)) == NULL)
		return NULL;
	*bitmap = bmh;

	size = sizeof(bitmap_t);
	if (bitmap->bitsPerPixel <= 8) {
		if (savePalette) {
			bitmap->pal = (palette_t*)((uchar*)bitmap + size);
			memcpy(bitmap->pal,dc->colorTab,palSize);
			}
		else
			bitmap->pal = NULL;
		bitmap->pf = NULL;
		}
	else {
		bitmap->pf = (pixel_format_t*)((uchar*)bitmap + size);
		*bitmap->pf = dc->pf;
		bitmap->pal = NULL;
		}
	size += palSize;
	bitmap->surface = (uchar*)bitmap + size;

	/* Now copy the bits from the device context into the bitmap */
	dc->r.beginPixel(dc);
	p = bitmap->surface;
	for (i = top; i < bottom; i++, p += bitmap->bytesPerLine) {
		NORMALISE_PTR(p);
		dc->r.getScanLine(dc,i,left,right,p,left,MGL_REPLACE_MODE);
		}
	dc->r.endPixel(dc);

	return bitmap;
}

/****************************************************************************
DESCRIPTION:
Create a monochrome bitmap mask given a transparent color.

HEADER:
mgraph.h

PARAMETERS:
bitmap	- bitmap to build monochrome mask from
transparent	- transparent color to mask out

RETURNS:
Pointer to allocated bitmap mask, or NULL on error.

REMARKS:
Attempts to build a monochrome bitmap mask that can be used to rasterize
transparent bitmaps. This is useful for devices that have hardware BitBlt
capabilities but lack hardware transparent BitBlt support. Everywhere the
transparent color is found, the mask will be 0, and everywhere else it will be 1.
Once you have created a monochrome bitmap mask, you can rasterize a transparent
bitmap by replacing all the transparent pixels in the original bitmap with 0's, and
then doing the following:

1. Set the foreground color to black and drawing the bitmap mask with
MGL_putBitmapMask. Everywhere that the bitmap has data a black
pixel will be used to 'punch' a hole in the display.

2. Use the MGL_bitBlt function to draw the
original bitmap on the display using the MGL_OR_MODE  write
mode. Everywhere that a pixel is transparent in the original source
bitmap, the destination will remain the same. Everywhere that a pixel is
non-transparent the pixel on the destination will be replaced with the
pixel in the source bitmap.

Note that only 8+ bits per pixel bitmaps are supported, and this function will fail on
lower color depths.

SEE ALSO:
MGL_transBltLin, MGL_putBitmapMask, MGL_bitBlt
****************************************************************************/
bitmap_t * MGLAPI MGL_buildMonoMask(
	bitmap_t *bitmap,
	color_t transparent)
{
	bitmap_t	*mask;
	int			i,bytes,last;
	long		bytesPerLine,size;
	uchar		_HUGE *ps,_HUGE *ms,*m,b;

	if (bitmap->bitsPerPixel < 8)
		return NULL;

	/* Allocate memory and build bitmap header information */
	bytesPerLine = (bitmap->width+7)/8;
	size = bytesPerLine * bitmap->height;
	if ((mask = MGL_malloc(sizeof(bitmap_t)+size)) == NULL)
		return NULL;
	mask->width = bitmap->width;
	mask->height = bitmap->height;
	mask->bitsPerPixel = 1;
	mask->bytesPerLine = bytesPerLine;
	mask->pf = NULL;
	mask->pal = NULL;
	mask->surface = (uchar*)mask + sizeof(bitmap_t);

	/* Now read the bits from the bitmap and build the mask */
	ps = bitmap->surface;
	ms = mask->surface;
	for (i = 0; i < bitmap->height; i++, ps += bitmap->bytesPerLine, ms += bytesPerLine) {
		NORMALISE_PTR(ps);
		NORMALISE_PTR(ms);
		bytes = bitmap->width / 8;
		last = bitmap->width - (bytes * 8);
		m = (uchar*)ms;
		if (bitmap->bitsPerPixel == 8) {
			uchar *p = (uchar*)ps;
			while (bytes--) {
				b = 0;
				if (p[0] != (uchar)transparent)	b |= 0x80;
				if (p[1] != (uchar)transparent) b |= 0x40;
				if (p[2] != (uchar)transparent)	b |= 0x20;
				if (p[3] != (uchar)transparent)	b |= 0x10;
				if (p[4] != (uchar)transparent) b |= 0x08;
				if (p[5] != (uchar)transparent) b |= 0x04;
				if (p[6] != (uchar)transparent) b |= 0x02;
				if (p[7] != (uchar)transparent) b |= 0x01;
				*m++ = b;
				p += 8;
				}
			if (last) {
				b = 0;
				if (last >= 1 && p[0] != (uchar)transparent) b |= 0x80;
				if (last >= 2 && p[1] != (uchar)transparent) b |= 0x40;
				if (last >= 3 && p[2] != (uchar)transparent) b |= 0x20;
				if (last >= 4 && p[3] != (uchar)transparent) b |= 0x10;
				if (last >= 5 && p[4] != (uchar)transparent) b |= 0x08;
				if (last >= 6 && p[5] != (uchar)transparent) b |= 0x04;
				if (last >= 7 && p[6] != (uchar)transparent) b |= 0x02;
				*m++ = b;
				}
			}
		else if (bitmap->bitsPerPixel == 15 || bitmap->bitsPerPixel == 16) {
			ushort *p = (ushort*)ps;
			while (bytes--) {
				b = 0;
				if (p[0] != (ushort)transparent) b |= 0x80;
				if (p[1] != (ushort)transparent) b |= 0x40;
				if (p[2] != (ushort)transparent) b |= 0x20;
				if (p[3] != (ushort)transparent) b |= 0x10;
				if (p[4] != (ushort)transparent) b |= 0x08;
				if (p[5] != (ushort)transparent) b |= 0x04;
				if (p[6] != (ushort)transparent) b |= 0x02;
				if (p[7] != (ushort)transparent) b |= 0x01;
				*m++ = b;
				p += 8;
				}
			if (last) {
				b = 0;
				if (last >= 1 && p[0] != (ushort)transparent) b |= 0x80;
				if (last >= 2 && p[1] != (ushort)transparent) b |= 0x40;
				if (last >= 3 && p[2] != (ushort)transparent) b |= 0x20;
				if (last >= 4 && p[3] != (ushort)transparent) b |= 0x10;
				if (last >= 5 && p[4] != (ushort)transparent) b |= 0x08;
				if (last >= 6 && p[5] != (ushort)transparent) b |= 0x04;
				if (last >= 7 && p[6] != (ushort)transparent) b |= 0x02;
				*m++ = b;
				}
			}
		else if (bitmap->bitsPerPixel == 24) {
			uchar *p = (uchar*)ps;
			while (bytes--) {
				b = 0;
#define	CHECK24(i)								\
   (p[i+0] != (uchar)transparent ||				\
	p[i+1] != (uchar)(transparent >> 8) ||		\
	p[i+2] != (uchar)(transparent >> 16))
				if (CHECK24(0))	 b |= 0x80;
				if (CHECK24(3))  b |= 0x40;
				if (CHECK24(6))	 b |= 0x20;
				if (CHECK24(9))	 b |= 0x10;
				if (CHECK24(12)) b |= 0x08;
				if (CHECK24(15)) b |= 0x04;
				if (CHECK24(18)) b |= 0x02;
				if (CHECK24(23)) b |= 0x01;
				*m++ = b;
				p += 24;
				}
			if (last) {
				b = 0;
				if (last >= 1 && CHECK24(0)) b |= 0x80;
				if (last >= 2 && CHECK24(1)) b |= 0x40;
				if (last >= 3 && CHECK24(2)) b |= 0x20;
				if (last >= 4 && CHECK24(3)) b |= 0x10;
				if (last >= 5 && CHECK24(4)) b |= 0x08;
				if (last >= 6 && CHECK24(5)) b |= 0x04;
				if (last >= 7 && CHECK24(6)) b |= 0x02;
				*m++ = b;
				}
			}
		else {
			ulong *p = (ulong*)ps;
			while (bytes--) {
				b = 0;
				if (p[0] != (ulong)transparent) b |= 0x80;
				if (p[1] != (ulong)transparent) b |= 0x40;
				if (p[2] != (ulong)transparent) b |= 0x20;
				if (p[3] != (ulong)transparent) b |= 0x10;
				if (p[4] != (ulong)transparent) b |= 0x08;
				if (p[5] != (ulong)transparent) b |= 0x04;
				if (p[6] != (ulong)transparent) b |= 0x02;
				if (p[7] != (ulong)transparent) b |= 0x01;
				*m++ = b;
				p += 8;
				}
			if (last) {
				b = 0;
				if (last >= 1 && p[0] != (ulong)transparent) b |= 0x80;
				if (last >= 2 && p[1] != (ulong)transparent) b |= 0x40;
				if (last >= 3 && p[2] != (ulong)transparent) b |= 0x20;
				if (last >= 4 && p[3] != (ulong)transparent) b |= 0x10;
				if (last >= 5 && p[4] != (ulong)transparent) b |= 0x08;
				if (last >= 6 && p[5] != (ulong)transparent) b |= 0x04;
				if (last >= 7 && p[6] != (ulong)transparent) b |= 0x02;
				*m++ = b;
				}
			}
		}

	return mask;
}

/****************************************************************************
DESCRIPTION:
Determines if the specified bitmap file is available for use.

HEADER:
mgraph.h

PARAMETERS:
bitmapName	- Name of bitmap file to check for

RETURNS:
True if the bitmap file exists, false if not.

REMARKS:
Attempt to locate the specified bitmap file, and verify that it is available for use. See
MGL_loadBitmap for more information on the algorithm that MGL uses when
searching for bitmap files on disk.

SEE ALSO:
MGL_loadBitmap
****************************************************************************/
ibool MGLAPI MGL_availableBitmap(
	const char *bitmapName)
{
	FILE				*f;
	winBITMAPFILEHEADER	header;
	winBITMAPINFOHEADER	bmInfo;

	_MGL_result = grOK;
	if ((f = openBitmapFile(&header,&bmInfo,bitmapName)) == NULL)
		return false;
	_MGL_fclose(f);
	return true;
}

/****************************************************************************
DESCRIPTION:
Unloads a bitmap file from memory.

HEADER:
mgraph.h

PARAMETERS:
bitmap	- Pointer to bitmap to unload

REMARKS:
Unloads the specified bitmap file from memory, and frees up all the system
resources associated with this bitmap.

SEE ALSO:
MGL_loadBitmap
****************************************************************************/
void MGLAPI MGL_unloadBitmap(
	bitmap_t *bitmap)
{
	if (bitmap)
		MGL_free(bitmap);
}

#endif	/* !MGL_LITE */

