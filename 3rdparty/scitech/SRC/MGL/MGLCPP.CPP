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
* Language:		C++ 3.0
* Environment:	any
*
* Description:	Implementation for the C++ API wrapper classes for the
*				MGL. This is really simple stuff, but makes using the MGL
*				from C++ a breeze.
*
*
****************************************************************************/

#include "mgraph.hpp"

/*---------------------------- Implementation -----------------------------*/

MGLCursor::MGLCursor(const char *cursorname)
/****************************************************************************
*
* Function:		MGLCursor::MGLCursor
* Parameters:	cursorname	- Name of cursor file to load
*
* Description:  Constructs and MGL cursor given the name of the cursor file
*				to load. If the cursor could not be loaded, we set 'cursor' to
*				NULL and save the error code returned by the MGL in the
*				owner flag.
*
****************************************************************************/
{
	if ((cursor = MGL_loadCursor(cursorname)) == NULL)
		owner = MGL_result();
}

int MGLCursor::result()
/****************************************************************************
*
* Function:		MGLCursor::result
* Returns:		Result code if an error occurred loading the MGL
*
****************************************************************************/
{
	if (cursor)
		return grOK;
	else return owner;
}

MGLIcon::MGLIcon(const char *iconname,ibool loadPalette)
/****************************************************************************
*
* Function:		MGLIcon::MGLIcon
* Parameters:	iconname	- Name of icon file to load
*
* Description:  Constructs and MGL icon given the name of the icon file
*				to load. If the icon could not be loaded, we set 'icon' to
*				NULL and save the error code returned by the MGL in the
*				owner flag.
*
****************************************************************************/
{
	if ((icon = MGL_loadIcon(iconname,loadPalette)) == NULL)
		owner = MGL_result();
}

int MGLIcon::result()
/****************************************************************************
*
* Function:		MGLIcon::result
* Returns:		Result code if an error occurred loading the MGL
*
****************************************************************************/
{
	if (icon)
		return grOK;
	else return owner;
}

MGLBitmap::MGLBitmap(const char *bitmapname,ibool loadPalette)
/****************************************************************************
*
* Function:		MGLBitmap::MGLBitmap
* Parameters:	bitmapname	- Name of bitmap file to load
*
* Description:  Constructs and MGL bitmap given the name of the bitmap file
*				to load. If the bitmap could not be loaded, we set 'bitmap' to
*				NULL and save the error code returned by the MGL in the
*				owner flag.
*
****************************************************************************/
{
	if ((bitmap = MGL_loadBitmap(bitmapname,loadPalette)) == NULL)
		owner = MGL_result();
}

int MGLBitmap::result()
/****************************************************************************
*
* Function:		MGLBitmap::result
* Returns:		Result code if an error occurred loading the MGL
*
****************************************************************************/
{
	if (bitmap)
		return grOK;
	else return owner;
}
