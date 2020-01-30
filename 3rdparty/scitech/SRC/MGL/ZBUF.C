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
* Description:	Zbuffer routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_3D

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
DESCRIPTION:
Clears a specified portion of the Z-buffer.

HEADER:
mgraph.h

PARAMETERS:
left	- Left coordinate of rectangle to clear (syntax 1)
top	- Top coordinate of rectangle to clear (syntax 1)
right	- Right coordinate of rectangle to clear (syntax 1)
bottom	- Bottom coordinate of rectangle to clear (syntax 1)
r	- Rectangle to clear (syntax 2)
leftTop	- Top left corner of rectangle to clear (syntax 3)
rightBot	- Bottom right corner of rectangle to clear (syntax 3)
clearVal	- Value to clear Z	- buffer with

REMARKS:
This function clears the specified rectangle in the Z-buffer to a set value, which is
usually done once per frame to initialize the Z-buffer to a standard value before
rasterizing primitives. Normally you should clear the Z-buffer to a value of 0 which
is used for pixels that are the farthest away.

SEE ALSO:
MGL_zBegin
****************************************************************************/
/* {secret} */
void MGLAPI MGL_zClearCoord(
	int left,
	int top,
	int right,
	int bottom,
	zfix32_t clearVal)
{
	rect_t	d;

	d.left = left;		d.top = top;
	d.right = right;	d.bottom = bottom;

	if (DC.a.clip && !MGL_sectRect(DC.a.clipRect,d,&d))
		return;
	MGL_offsetRect(d,DC.a.viewPort.left,DC.a.viewPort.top);
	DC.r.z.zClear(d.left,d.top,d.right,d.bottom,clearVal);
}

#endif	/* MGL_3D */

