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
* Description:	Icon resource loading/unloading routines. Icons are
*				stored on disk in standard Windows .ICO file format.
*
*
****************************************************************************/

#include "mgl.h"

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

static ibool openIconFileExt(winICONHEADER *h,winBITMAPINFOHEADER *b,
	FILE *f,ulong dwOffset)
/****************************************************************************
*
* Function:		openIconFileExt
* Parameters:	h			- Icon h info to fill in
*				b			- Bitmap info structure to fill info
*				f			- Open binary file to read icon from
*				dwOffset	- Offset to start of icon in file
* Returns:		True on success, false on error.
*
* Description:	Attempts to read the icon header information. If the h
*				information is invalid, we close the file and return NULL.
*
****************************************************************************/
{
	size_t	size;

	/* Read in the h record and verify the icon file */
	_MGL_result = grOK;
	_MGL_fseek(f,dwOffset,SEEK_SET);
	size = _MGL_fread(h,1,sizeof(winICONHEADER),f);
	if (size != sizeof(winICONHEADER) || h->ihRes[0] != 0 || h->ihRes[1] != 0
			|| h->ihType[0] != 1 || h->ihType[1] != 0 || h->ihCount[0] != 1) {
		_MGL_result = grInvalidIcon;
		return false;
		}

	/* Read in the bitmap info h and verify it */
	size = _MGL_fread(b,1,sizeof(winBITMAPINFOHEADER),f);
	if (size != sizeof(winBITMAPINFOHEADER) || b->biSize[0] != 0x28
			|| b->biPlanes[0] != 1 || b->biCompression[0] != 0) {
		_MGL_result = grInvalidIcon;
		return false;
		}
	return true;
}

static FILE *openIconFile(winICONHEADER *h,winBITMAPINFOHEADER *b,
	const char *iconName)
/****************************************************************************
*
* Function:		openIconFile
* Parameters:	h			- Icon h info to fill in
*				b			- Bitmap info structure to fill info
*				iconName	- Name of the icon file to load
* Returns:		Handle to the opened file, NULL on error.
*
* Description:	Attempts to open the specified icon file and read the
*				icon header information. If the header information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	FILE	*f;

	_MGL_result = grOK;
	f = _MGL_openFile(MGL_ICONS, iconName, "rb");
	if (f == NULL) {
		_MGL_result = grIconNotFound;
		return NULL;
		}
	if (!openIconFileExt(h,b,f,0)) {
		_MGL_fclose(f);
		return NULL;
		}
	return f;
}

static icon_t * LoadIcon(winICONHEADER *h,winBITMAPINFOHEADER *b,FILE *f,
	ulong dwOffset,ibool loadPalette)
/****************************************************************************
*
* Function:		LoadIcon
* Parameters:	h,b			- Headers read for icon file
*				f			- Open binary file to read data from
*				dwOffset	- Offset to start of icon in binary file
*				loadPalette	- Should we load the palette values as well?
* Returns:		Pointer to the loaded icon 
*
* Description:	Locates the specified icon and loads it into memory.
*				If 'loadPalette' is true, the palette values for the icon
*				will be loaded into the structure as well (if there is no
*				palette, it will not be loaded), otherwise the palette
*				entry for the icon will be NULL.
*
****************************************************************************/
{
	icon_t				*icon;
	palette_t			pal[256];		/* Temporary space for palette	*/
	int					i,j,palSize,width,height,lineAdjust;
	int					bytesPerLine,byteWidth,size;
	uchar				*p;

	/* Load the palette values into the local palette if present */
	_MGL_result = grOK;
	palSize = h->ihEntries[0].bColorCount;
	if (palSize == 0) {
		switch (b->biBitCount[0]) {
			case 1:	palSize = 2;	break;
			case 4:	palSize = 16;	break;
			case 8:	palSize = 256;	break;
			}
		}
	if (loadPalette)
		_MGL_fread(pal,1,palSize * sizeof(palette_t),f);
	else
		_MGL_fseek(f,palSize * sizeof(palette_t),SEEK_CUR);

	/* Allocate memory for the icon structure */
	width = h->ihEntries[0].bWidth;
	height = h->ihEntries[0].bHeight;
	switch (palSize) {
		case 2:		bytesPerLine = (width+7)/8;	break;
		case 16:	bytesPerLine = (width+1)/2;	break;
		case 256:	bytesPerLine = width;		break;
		default:
			_MGL_result = grInvalidIcon;
			return NULL;
		}
	bytesPerLine = (bytesPerLine + 3) & ~3;
	byteWidth = (width+7)/8;
	size = sizeof(icon_t) + byteWidth * height + bytesPerLine * height +
		palSize * sizeof(palette_t);
	if ((icon = MGL_malloc(size)) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}

	/* Compute the offsets into the icon structure for the AND mask and
	 * the XOR mask
	 */
	icon->byteWidth = byteWidth;
	icon->andMask = (uchar*)icon + sizeof(icon_t);

	/* Fill in the icon bitmap h info, and copy the palette into the
	 * icon if present
	 */
	size = byteWidth * height;
	if (palSize) {
		icon->xorMask.pal = (palette_t*)((uchar*)icon->andMask + size);
		memcpy(icon->xorMask.pal,pal,palSize * sizeof(palette_t));
		size += palSize * sizeof(palette_t);
		}
	else
		icon->xorMask.pal = NULL;
	icon->xorMask.width = width;
	icon->xorMask.height = height;
	icon->xorMask.bitsPerPixel = b->biBitCount[0];
	icon->xorMask.bytesPerLine = bytesPerLine;
	icon->xorMask.surface = (uchar*)icon->andMask + size;

	/* Now read in the XOR mask bits in the icon, inverting the icon
	 * vertically as Windows DIB's are stored with the bottom line first
	 */
	lineAdjust = ((bytesPerLine + 0x3) & ~0x3) - bytesPerLine;
	p = icon->xorMask.surface + bytesPerLine * (height-1);
	for (i = 0; i < height; i++, p -= bytesPerLine) {
		_MGL_fread(p,1,bytesPerLine,f);
		_MGL_fseek(f,lineAdjust,SEEK_CUR);	/* Align to 32 bits	*/
		}

	/* Now read in the AND mask bits in the icon - note that we have to
	 * invert the AND mask bits so that we can display it correctly
	 * internally.
	 */
	lineAdjust = ((byteWidth + 0x3) & ~0x3) - byteWidth;
	p = icon->andMask + byteWidth * (height-1);
	for (i = 0; i < height; i++, p -= byteWidth) {
		_MGL_fread(p,1,byteWidth,f);
		_MGL_fseek(f,lineAdjust,SEEK_CUR);	/* Align to 32 bits	*/
		for (j = 0; j < byteWidth; j++)
			p[j] ^= 0xFF;
		}
	return (icon_t*)icon;
}

/****************************************************************************
DESCRIPTION:
Load an icon file from disk.

HEADER:
mgraph.h

PARAMETERS:
iconName 	- Name of icon file to load
loadPalette	- True if the palette should also be loaded

RETURNS:
Pointer to the loaded icon file, NULL on error.

REMARKS:
Locates the specified icon file and loads it into memory. MGL can load any
Windows 3.x style icon files of any dimensions. Generally icon files will be either
32x32 or 64x64 in size. Consult the Windows SDK documentation for the format of
Windows font files.

If loadPalette is true, the palette values for the icon will be loaded into the structure
as well (if there is no palette, it will not be loaded), otherwise the palette entry for
the bitmap will be NULL.

When MGL is searching for icon files it will first attempt to find the files just by
using the filename itself. Hence if you wish to look for a specific icon file, you
should pass the full pathname to the file that you are interested in. If the filename is
a simple relative filename (i.e. "MYICON.ICO"), MGL will then search in the
ICONS directory relative to the path specified in mglpath variable that was passed
to MGL_init. As a final resort MGL will also look for the files in the ICONS
directory relative to the MGL_ROOT environment variable.

If the icon file was not found, or an error occurred while reading the icon file, this
function will return NULL. You can check the MGL_result error code to determine
the cause.

SEE ALSO:
MGL_unloadIcon, MGL_availableIcon, MGL_putIcon, MGL_loadIconExt
****************************************************************************/
icon_t * MGLAPI MGL_loadIcon(
	const char *iconName,
	ibool loadPalette)
{
	icon_t				*icon;
	FILE				*f;
	winICONHEADER		h;
	winBITMAPINFOHEADER	b;

	if ((f = openIconFile(&h,&b,iconName)) == NULL)
		return NULL;
	if ((icon = LoadIcon(&h,&b,f,0,loadPalette)) == NULL) {
		fclose(f);
		return NULL;
		}
	return icon;
}

/****************************************************************************
DESCRIPTION:
Load an icon file from disk from an open file. 

HEADER:
mgraph.h

PARAMETERS:
f			- Pointer to open icon file
dwOffset	- Offset into the icon file
dwSize		- Size of the icon file
loadPalette	- If true, the palette is loaded as well

RETURNS:
Pointer to the loaded icon file.

REMARKS:
This function is the same as MGL_loadIcon, however it works with a
previously opened file. This allows you to create your own large files with
multiple files embedded in them.

SEE ALSO:
MGL_loadIcon
****************************************************************************/
icon_t * MGLAPI MGL_loadIconExt(
	FILE *f,
	ulong dwOffset,
	ulong dwSize,
	ibool loadPalette)
{
	winICONHEADER		h;
	winBITMAPINFOHEADER	b;

	if (!openIconFileExt(&h,&b,f,dwOffset))
		return NULL;
	return LoadIcon(&h,&b,f,dwOffset,loadPalette);
}

/****************************************************************************
DESCRIPTION:
Determines if the specified icon file is available for use.

HEADER:
mgraph.h

PARAMETERS:
iconName 	- Name of icon file to check for

RETURNS:
True if the icon file exists, False if not.

REMARKS:
Attempt to locate the specified icon file, and verify that it is available for use.

SEE ALSO:
MGL_loadIcon

****************************************************************************/
ibool MGLAPI MGL_availableIcon(
	const char *iconName)
{
	FILE				*f;
	winICONHEADER		h;
	winBITMAPINFOHEADER	b;

	_MGL_result = grOK;
	if ((f = openIconFile(&h,&b,iconName)) == NULL)
		return false;
	_MGL_fclose(f);
	return true;
}

/****************************************************************************
DESCRIPTION:
Unloads an icon file from memory.

HEADER:
mgraph.h

PARAMETERS:
icon	- Pointer to icon to unload

REMARKS:
Unloads the specified icon file from memory, and frees up all the system resources
associated with this icon.

SEE ALSO:
MGL_loadIcon
****************************************************************************/
void MGLAPI MGL_unloadIcon(
	icon_t *icon)
{
	if (icon)
		MGL_free(icon);
}

#endif	/* !MGL_LITE */

