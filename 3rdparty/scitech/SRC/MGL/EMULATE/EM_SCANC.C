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
* Description:	C based emulation routines for scanline scanning.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

int _ASMAPI __EMU__scanRightForColor(int x,int y,
	color_t color)
/****************************************************************************
*
* Function:		__EMU__scanRightForColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to scan for
* Returns:      Coordinate of pixel, or XRes+1 if not found
*
* Description:	Scans right in the device context from the specified
*				location for the specified color value. If a pixel is
*				found, the x coordinate is returned, otherwise a value
*				of XRes+1 will be returned.
*
****************************************************************************/
{
	MGL_beginPixel();
	while (x <= DC.mi.xRes) {
		if (DC.r.getPixel(x,y) == color) {
			MGL_endPixel();
			return x;
			}
		x++;
		}
	MGL_endPixel();
	return x;
}

int _ASMAPI __EMU__scanLeftForColor(int x,int y,
	color_t color)
/****************************************************************************
*
* Function:		__EMU__scanLeftForColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to scan for
* Returns:      Coordinate of pixel, or -1 if not found
*
* Description:	Scans left in the device context from the specified
*				location for the specified color value. If a pixel is
*				found, the x coordinate is returned, otherwise a value
*				of -1 will be returned.
*
****************************************************************************/
{
	MGL_beginPixel();
	while (x >= 0) {
		if (DC.r.getPixel(x,y) == color) {
			MGL_endPixel();
			return x;
			}
		x--;
		}
	MGL_endPixel();
	return -1;
}

int _ASMAPI __EMU__scanRightWhileColor(int x,int y,
	color_t color)
/****************************************************************************
*
* Function:		__EMU__scanRightWhileColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to continue scanning on
* Returns:      Coordinate of pixel, or XRes+1 if not found
*
* Description:	Scans right in the device context from the specified
*				location looking for a change in the pixel color. If a
*				different pixel color is found, the x coordinate is
*				returned, otherwise a value of XRes+1 will be returned.
*
****************************************************************************/
{
	MGL_beginPixel();
	while (x <= DC.mi.xRes && DC.r.getPixel(x,y) == color)
		x++;
	MGL_endPixel();
	return x;
}

int _ASMAPI __EMU__scanLeftWhileColor(int x,int y,
	color_t color)
/****************************************************************************
*
* Function:		__EMU__scanLeftWhileColor
* Parameters:	dc		- Device context
*				x,y		- Coordinate to begin scanning at
*				color	- Color to continue scanning on
* Returns:      Coordinate of pixel, or -1 if not found
*
* Description:	Scans left in the device context from the specified
*				location looking for a change in the pixel color. If a
*				different pixel color is found, the x coordinate is
*				returned, otherwise a value of -1 will be returned.
*
****************************************************************************/
{
	MGL_beginPixel();
	while (x >= 0 && DC.r.getPixel(x,y) == color)
		x--;
	MGL_endPixel();
	return x;
}

#endif	/* !MGL_LITE */
