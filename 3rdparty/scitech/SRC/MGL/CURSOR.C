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
* Description:  Mouse cursor resource loading/unloading routines. Cursors
*				are stored on disk in standard Windows .CUR cursor files.
*
*
****************************************************************************/

#include "mgl.h"

/*--------------------------- Global Variables ----------------------------*/

cursor_t _MGL_def_cursor = {	/* Standard arrow cursor				*/
   {0x00000000, 0x00000040, 0x00000060, 0x00000070,
	0x00000078, 0x0000007C, 0x0000007E, 0x0000007F,
	0x0000807F, 0x0000C07F, 0x0000007E, 0x00000076,
	0x00000066, 0x00000043, 0x00000003, 0x00008001,
	0x00008001, 0x0000C000, 0x0000C000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000},

   {0x000000C0, 0x000000E0, 0x000000F0, 0x000000F8,
	0x000000FC, 0x000000FE, 0x000000FF, 0x000080FF,
	0x0000C0FF, 0x0000E0FF, 0x0000F0FF, 0x000000FF,
	0x000000FF, 0x000080EF, 0x000080C7, 0x0000C083,
	0x0000C003, 0x0000E001, 0x0000E001, 0x0000C000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000},
	1,1
	};

/*------------------------- Implementation --------------------------------*/

cursor_t * MGLAPI _MGL_getDefCursor(void)  { return &_MGL_def_cursor; }

static ibool openCursorFileExt(winCURSORHEADER *h,FILE *f,ulong dwOffset)
/****************************************************************************
*
* Function:		openCursorFileExt
* Parameters:	h			- Cursor h info to fill in
*				f			- Open binary file to read cursor from
*				dwOffset	- Offset to start of info in binary file
* Returns:		True on success, false on failure
*
* Description:	Attempts to open the specified cursor file and read the
*				cursor h information. If the h information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	size_t				size;
	winCURSORDIRENTRY	*e = &h->chEntries[0];

	/* Read in the h record and verify the cursor file. We only accept
	 * valid files containing a single 32x32 monochrome cursor.
	 */
	_MGL_result = grOK;
	_MGL_fseek(f,dwOffset,SEEK_SET);
	size = _MGL_fread(h,1,sizeof(winCURSORHEADER),f);
	if (size != sizeof(winCURSORHEADER) || getLEShort(h->chReserved) != 0
			|| getLEShort(h->chType) != 2 || getLEShort(h->chCount) != 1
			|| e->bWidth != 32 || e->bHeight != 32) {
		_MGL_result = grInvalidCursor;
		return false;
		}
	return true;
}

static FILE *openCursorFile(winCURSORHEADER *h,const char *cursorName)
/****************************************************************************
*
* Function:		openCursorFile
* Parameters:	h			- Cursor h info to fill in
*				cursorName	- Name of the cursor file to load
* Returns:		Handle to the opened file, NULL on error.
*
* Description:	Attempts to open the specified cursor file and read the
*				cursor h information. If the h information is
*				invalid, we close the file and return NULL.
*
****************************************************************************/
{
	FILE	*f;

	_MGL_result = grOK;
	f = _MGL_openFile(MGL_CURSORS, cursorName, "rb");
	if (f == NULL) {
		_MGL_result = grCursorNotFound;
		return NULL;
		}
	if (!openCursorFileExt(h,f,0)) {
		_MGL_fclose(f);
		return NULL;
		}
	return f;
}

static cursor_t * LoadCursor(winCURSORHEADER *h,FILE *f,ulong dwOffset)
/****************************************************************************
*
* Function:		LoadCursor
* Returns:		Pointer to the loaded cursor, NULL on error.
*
* Description:	Routine to do the actual loading of the cursor file from
*				disk.
*
****************************************************************************/
{
	cursor_t	*cursor;
	int			i;

	/* Allocate memory and read in the cursor */
	if ((cursor = MGL_malloc(sizeof(cursor_t))) == NULL) {
		_MGL_result = grNoMem;
		return NULL;
		}

	/* Save the hotspot values */
	cursor->xHotSpot = getLEShort(h->chEntries[0].wXHotSpot);
	cursor->yHotSpot = getLEShort(h->chEntries[0].wYHotSpot);

	/* Skip past the BITMAPINFOHEADER and color table */
	_MGL_fseek(f,48,SEEK_CUR);
	_MGL_fread(cursor,1,256,f);

	/* Swap the ordering of the bitmaps, since DIB's start in the lower
	 * left corner, while we want them to start in the upper right corner
	 */
	for (i = 0; i < 16; i++) {
		SWAP(cursor->andMask[i],cursor->andMask[31-i]);
		SWAP(cursor->xorMask[i],cursor->xorMask[31-i]);
		}

	/* Invert the AND mask so that we can simply draw to punch a hole */
	for (i = 0; i < 32; i++)
		cursor->andMask[i] ^= 0xFFFFFFFFL;
	return cursor;
}

/****************************************************************************
DESCRIPTION:
Load a cursor file from disk.

HEADER:
mgraph.h

PARAMETERS:
cursorName	- Name of cursor file to load

RETURNS:
Pointer to loaded cursor file, NULL on error.

REMARKS:
Locates the specified cursor file and loads it into memory.  MGL can load any
Windows 3.x style cursor files. Consult the Windows SDK documentation for the
format of Windows cursor files.

When MGL is searching for cursor files it will first attempt to find the files just by
using the filename itself. Hence if you wish to look for a specific cursor file, you
should pass the full pathname to the file that you are interested in. If the filename is
a simple relative filename (i.e. "MYCURS.CUR"), MGL will then search in the
CURSORS directory relative to the path specified in mglpath variable that was
passed to MGL_init. As a final resort MGL will also look for the files in the
CURSORS directory relative to the MGL_ROOT environment variable.

If the cursor file was not found, or an error occurred while reading the cursor file,
this function will return NULL. You can check the MGL_result error code to
determine the cause.

SEE ALSO:
MGL_unloadCursor, MGL_availableCursor, MS_setCursor, MGL_loadCursorExt
****************************************************************************/
cursor_t * MGLAPI MGL_loadCursor(
	const char *cursorName)
{
	cursor_t		*cursor;
	FILE			*f;
	winCURSORHEADER	h;

	if ((f = openCursorFile(&h,cursorName)) == NULL)
		return NULL;
	if ((cursor = LoadCursor(&h,f,0)) == NULL) {
    	fclose(f);
		return NULL;
		}
	return cursor;
}

/****************************************************************************
DESCRIPTION:
Load a cursor file from disk from an opened file.

HEADER:
mgraph.h

PARAMETERS:
f			- Open file to read cursor from (binary mode)
dwOffset	- Offset to the start of the cursor file
dwSize		- Size of the file

RETURNS:
Pointer to the loaded cursor file

REMARKS:
This function is the same as MGL_loadCursor, however it works with a
previously opened file. This allows you to create your own large files with
multiple files embedded in them.

SEE ALSO:
MGL_loadCursor
****************************************************************************/
cursor_t * MGLAPI MGL_loadCursorExt(
	FILE *f,
	ulong dwOffset,
	ulong dwSize)
{
	winCURSORHEADER	h;

	if (!openCursorFileExt(&h,f,dwOffset))
		return NULL;
	return LoadCursor(&h,f,dwOffset);
}

/****************************************************************************
DESCRIPTION:
Determines if the specified cursor file is available for use.

HEADER:
mgraph.h

PARAMETERS:
cursorName	- Name of cursor file to check for

RETURNS:
True if the cursor file exists, false if not.

REMARKS:
Attempt to locate the specified mouse cursor, and verify that it is available for use.
See MGL_loadCursor for more information on the algorithm that MGL uses when
searching for mouse cursor files on disk.

SEE ALSO:
MGL_loadCursor

****************************************************************************/
ibool MGLAPI MGL_availableCursor(
	const char *cursorName)
{
	FILE			*f;
	winCURSORHEADER	h;

	_MGL_result = grOK;
	if ((f = openCursorFile(&h,cursorName)) == NULL)
		return false;
	_MGL_fclose(f);
	return true;
}

/****************************************************************************
DESCRIPTION:
Unloads a cursor file from memory.

HEADER:
mgraph.h

PARAMETERS:
cursor	- Pointer to cursor to unload

REMARKS:
Unloads the specified cursor file from memory, and frees up all the system
resources associated with this cursor.

SEE ALSO:
MGL_loadCursor

****************************************************************************/
void MGLAPI MGL_unloadCursor(
	cursor_t *cursor)
{
	if (cursor != MGL_DEF_CURSOR)
		MGL_free(cursor);
}

