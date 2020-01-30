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
* Description:  Line drawing routines (including code for the
*				Cohen-Sutherland line clipping algorithm).
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

#ifdef	MGL_FIX3D

/****************************************************************************
DESCRIPTION:
Renders a zbuffered line to the current display context.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate of first endpoint 
y1	- y coordinate of first endpoint 
c1	- color of first endpoint 
x2	- x coordinate of second endpoint 
y2	- y coordinate of second endpoint 
c2	- color of the second endpoint 
p1	- First endpoint 
p2	- Second endpoint

REMARKS:
Draws a color index shaded line in the current device context. The color values
will be linearly interpolated between the color at the first endpoint and the
color at the second endpoint, producing a smooth shaded line. This function also
works in RGB modes and the color indexes will be translated via the currently
active shade table to find the actual color used to display each pixel in the
line. The function MGL_setShadeTable is used to change the currently active
shade table.
****************************************************************************/
/* {secret} */
void ASMAPI MGL_cLineCoord(
	fix32_t x1,
	fix32_t y1,
	fix32_t c1,
	fix32_t x2,
	fix32_t y2,
	fix32_t c2)
{
	/* Ensure that we are always scan converting the line from right to
	 * left.
	 */
	if ((x2 - x1) < 0) {
		fix32_t	t;
		SWAPT(x2,x1,t);
		SWAPT(y2,y1,t);
		SWAPT(c2,c1,t);
		}
	if (DC.a.clip && !_MGL_clipCLineFX(&x1,&y1,&c1,&x2,&y2,&c2,
			DC.clipRectFX.left,DC.clipRectFX.top,
			DC.clipRectFX.right,DC.clipRectFX.bottom))
		return;
	DC.r.cLine(
		x1 + DC.viewPortFX.left,y1 + DC.viewPortFX.top,c1,
		x2 + DC.viewPortFX.left,y2 + DC.viewPortFX.top,c2);
}
#endif	/* !MGL_FIX3D */
