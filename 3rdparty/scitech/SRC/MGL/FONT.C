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
* Description:	Font resource loading/unloading routines. Font files are
*				stored on disk in either the old MGL 1.x font file format
*				or the standard Windows 2.x bitmap font file format (not
*				Windows 3.x format).
*
*
****************************************************************************/

#include "mgl.h"

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

static ibool openWinFontFileExt(winFONTINFO *fi,FILE *f,ulong dwOffset)
/****************************************************************************
*
* Function:		openWinFontFileExt
* Parameters:   fi			- Pointer to FONTINFO structure to fill
*				fontname	- Name of the font file to load
* Returns:		True on success, false on error.
*
* Description:	Attempts to open the specified font file and read the
*				font header information. If the header information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	size_t	size;

	/* Read in the header record and verify the font file */
	_MGL_result = grOK;
	_MGL_fseek(f,dwOffset,SEEK_SET);
	size = _MGL_fread(fi,1,sizeof(winFONTINFO),f);
	if (size != sizeof(winFONTINFO) || fi->fh.dfType != 0) {
		_MGL_result = grBadFontFile;
		return false;
		}
	if (fi->fh.dfVersion != 0x200) {
		_MGL_result = grNewFontFile;
		return false;
		}
	return true;
}

static FILE *openWinFontFile(winFONTINFO *fi,const char *fontname)
/****************************************************************************
*
* Function:		openWinFontFile
* Parameters:   fi			- Pointer to FONTINFO structure to fill
*				fontname	- Name of the font file to load
* Returns:		Handle to the opened file, NULL on error.
*
* Description:	Attempts to open the specified font file and read the
*				font header information. If the header information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	FILE	*f;

	_MGL_result = grOK;
	f = _MGL_openFile(MGL_FONTS, fontname, "rb");
	if (f == NULL) {
		_MGL_result = grFontNotFound;
		return NULL;
		}
	if (!openWinFontFileExt(fi,f,0)) {
		_MGL_fclose(f);
		return NULL;
		}
	return f;
}

static ibool openFontFileExt(font_header *h,FILE *f,ulong dwOffset)
/****************************************************************************
*
* Function:		openFontFileExt
* Parameters:	f			- Open binary file to read font data from
*				dwOffset	- Offset to start of font in file
* Returns:		True on success, false on error.
*
* Description:	Attempts to open the specified font file and read the
*				font header information. If the header information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	/* Read in the header record and verify the font file */
	_MGL_result = grOK;
	_MGL_fseek(f,dwOffset,SEEK_SET);
	_MGL_fread(h,1,sizeof(font_header),f);
	if (strcmp(h->ident,FONT_IDENT)) {
		_MGL_result = grBadFontFile;
		return false;
		}
	return true;
}

static FILE *openFontFile(font_header *h,const char *fontname)
/****************************************************************************
*
* Function:		openFontFile
* Parameters:	fontname	- Name of the font file to load
* Returns:		Handle to the opened file, NULL on error.
*
* Description:	Attempts to open the specified font file and read the
*				font header information. If the header information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	FILE	*f;

	_MGL_result = grOK;
	f = _MGL_openFile(MGL_FONTS, fontname, "rb");
	if (f == NULL) {
		_MGL_result = grFontNotFound;
		return NULL;
		}
	if (!openFontFileExt(h,f,0)) {
		_MGL_fclose(f);
		return NULL;
		}
	return f;
}

#define	TEMP_BUF_SIZE	512

static void swapGlyph(uchar *bits,int height,int width)
/****************************************************************************
*
* Function:		swapGlyph
* Parameters:	bits	- Bitmaps definition for the glyph
*               height	- Height of the bitmap
*				width	- Width of the bitmap
*
* Description:	Converts the glyph definition from the Windows 2.x font
*				file format (color major) to the MGL monochrome bitmap
*				format (row major).
*
****************************************************************************/
{
	uchar	*p,buf[TEMP_BUF_SIZE];
	int		i,j,size = height * width;

	if (width == 1)
		return;
	if (size > TEMP_BUF_SIZE)
		MGL_fatalError("Temporary glyph buffer is too small!!");
	memcpy(buf,bits,size);
	for (j = 0, p = buf; j < width; j++)
		for (i = 0; i < height; i++) {
			bits[i*width+j] = *p++;
			}
}

static font_t *loadWinFontFileExt(FILE *f,ulong dwOffset)
/****************************************************************************
*
* Function:		loadWinFontFileExt
* Parameters:	f			- Open binary file to read font data from
*				dwOffset	- Offset to start of font in file
* Returns:		Pointer to the font data, or NULL on error.
*
* Description:	Attempts to locate and load in a Windows 3.x bitmap font
*				file and convert it to the structure used internally in
*				the MGL for rendering MGL style bitmap fonts. If the font
*				is invalid or we cant find it we return NULL.
*
****************************************************************************/
{
	int			i,j,numChars,glyphSize,offset;
	bitmap_font	*font = NULL;
	winGLYPH	g[MAXVEC];
	winFONTINFO	fi;

    /* Open the font file header */
	if (!openWinFontFileExt(&fi,f,dwOffset)) {
		// TODO: Add support for loading fonts from a font file library if
		//		 we cant make sense of the header information here. We would
		//		 need to know which font to load from the library to make this
        //		 work (perhaps the name of the font should be passed in?).
		return NULL;
		}

	/* Allocate memory for the font file */
	glyphSize = fi.fm.dfFace - fi.fm.dfBitsOffset;
	if ((font = MGL_malloc(sizeof(bitmap_font) + glyphSize)) == NULL) {
		_MGL_result = grLoadMem;
		return NULL;
		}

	/* Read the face name of the font */
	_MGL_fseek(f,fi.fm.dfFace + dwOffset,SEEK_SET);
	_MGL_fread(font->name,1,_MGL_FNAMESIZE,f);

	/* Build the MGL font header information */
	if (fi.ft.dfPixWidth != 0) {
		font->fontType =  MGL_FIXEDFONT;
		font->fontWidth = fi.ft.dfPixWidth;
		}
	else {
		font->fontType =  MGL_PROPFONT;
		font->fontWidth = fi.ft.dfMaxWidth;
		}
	font->fontHeight = fi.ft.dfPixHeight;
	font->maxWidth = fi.ft.dfMaxWidth;
	font->maxKern = 0;
	font->ascent = fi.fs.dfAscent;
	font->descent = fi.fs.dfAscent - fi.ft.dfPixHeight;
	font->leading = fi.fs.dfExternalLeading;

	/* Read the glyph table from disk */
	numChars = fi.fc.dfLastChar - fi.fc.dfFirstChar+1;
	_MGL_fseek(f,sizeof(winFONTINFO) + dwOffset,SEEK_SET);
	_MGL_fread(g,sizeof(winGLYPH),numChars,f);

	/* Read the glyph definitions from disk */
	_MGL_fseek(f,fi.fm.dfBitsOffset + dwOffset,SEEK_SET);
	_MGL_fread(&font->def,1,glyphSize,f);

	for (i = offset = 0; i < MAXVEC; i++) {
		if (i < fi.fc.dfFirstChar || i > fi.fc.dfLastChar)
			font->offset[i] = -1;
		else {
			j = i - fi.fc.dfFirstChar;
			font->width[i] = g[j].gWidth;
			font->iwidth[i] = (g[j].gWidth + 0x7) & ~0x7;
			font->offset[i] = offset;
			font->loc[i] = 0;	/* Fonts dont appear to support kerning	*/
			swapGlyph(&font->def[offset],font->fontHeight,font->iwidth[i] >> 3);
			offset += (g[j+1].gOffset - g[j].gOffset);
			}
		}

	/* Create the missing symbol definition */
	i = fi.fc.dfFirstChar + fi.fc.dfDefaultChar;
	font->width[MISSINGSYMBOL] = font->width[i];
	font->iwidth[MISSINGSYMBOL] = font->iwidth[i];
	font->offset[MISSINGSYMBOL] = font->offset[i];
	font->loc[MISSINGSYMBOL] = font->loc[i];
	return (font_t*)font;
}

/****************************************************************************
DESCRIPTION:
Load a font file for use from an opened file.

HEADER:
mgraph.h

PARAMETERS:
f			- Open binary file to read font data from
dwOffset	- Offset to start of font in file
dwSize		- Size of the file in bytes

RETURNS:
Pointer to the font data, or NULL on error.

REMARKS:
This function is the same as MGL_loadFont, however it works with a
previously opened file. This allows you to create your own large files with
multiple files embedded in them.

SEE ALSO:
MGL_loadFont
****************************************************************************/
font_t * MGLAPI MGL_loadFontExt(
	FILE *f,
	ulong dwOffset,
	ulong dwSize)
{
	size_t		size;
	font_t		*font;
	font_header	header;

	/* Try and load a Windows font file */
	if (!openFontFileExt(&header,f,dwOffset))
		return loadWinFontFileExt(f,dwOffset);

	/* Allocate memory and read in the entire MGL 1.x font file */
	size = dwSize - sizeof(font_header);
	if ((font = MGL_malloc(size + _MGL_FNAMESIZE)) == NULL) {
		_MGL_result = grLoadMem;
		return NULL;
		}

	strcpy(font->name,header.name);
	_MGL_fread(&font->fontType,1,size,f);
	return font;
}

/****************************************************************************
DESCRIPTION:
Load a font file for use.

HEADER:
mgraph.h

PARAMETERS:
fontname 	- Name of the font file to load

RETURNS:
Pointer to the loaded font file, NULL on error.

REMARKS:
Locates the specified font file and loads it into memory. MGL can load any
Windows 2.x style font files (Windows 3.x font files are not supported, but
Windows 2.x font files are the standard files even for Windows 3.1. Most resource
editors can only output 2.x style font files). Consult the Windows SDK
documentation for the format of Windows font files.

When MGL is searching for font files it will first attempt to find the files just by
using the filename itself. Hence if you wish to look for a specific font file, you
should pass the full pathname to the file that you are interested in. If the filename is
a simple relative filename (i.e. "MYFONT.FNT"), MGL will then search in the
FONTS directory relative to the path specified in mglpath variable that was passed
to MGL_init. As a final resort MGL will also look for the files in the FONTS
directory relative to the MGL_ROOT environment variable.

If the font file was not found, or an error occurred while reading the font file, this
function will return NULL. You can check the MGL_result error code to determine
the cause.

SEE ALSO:
MGL_unloadFont, MGL_useFont, MGL_availableFont.
****************************************************************************/
font_t * MGLAPI MGL_loadFont(
	const char *fontname)
{
	FILE	*f;
	font_t	*font;

	_MGL_result = grOK;
	if ((f = _MGL_openFile(MGL_FONTS, fontname, "rb")) == NULL) {
		_MGL_result = grFontNotFound;
		return NULL;
		}
	font = MGL_loadFontExt(f,0,_MGL_fileSize(f));
	_MGL_fclose(f);
	return font;
}

/****************************************************************************
DESCRIPTION:
Determines if a specific font file is available for use.

HEADER:
mgraph.h

PARAMETERS:
fontname 	- Relative filename of the required font file

RETURNS:
True if font file is available, false if not.

REMARKS:
Attempt to locate the specified font file, and verify that it is available for use. See
MGL_loadFont for more information on the algorithm that MGL uses when
searching for font files on disk.

SEE ALSO:
MGL_loadFont
****************************************************************************/
ibool MGLAPI MGL_availableFont(
	const char *fontname)
{
	FILE		*f;
	font_header	header;
	winFONTINFO	fi;

	_MGL_result = grOK;
	if ((f = openFontFile(&header,fontname)) == NULL)
		if ((f = openWinFontFile(&fi,fontname)) == NULL)
			return false;
	_MGL_fclose(f);
	return true;
}

/****************************************************************************
DESCRIPTION:
Unloads a font file from memory.

HEADER:
mgraph.h

PARAMETERS:
font	- Pointer to font to unload

REMARKS:
Unloads the specified font file from memory, and frees up all the system resources
associated with this font.

SEE ALSO:
MGL_loadCursorExt
****************************************************************************/
void MGLAPI MGL_unloadFont(
	font_t *font)
{
	if (font)
		MGL_free(font);
}

#endif	/* !MGL_LITE */

