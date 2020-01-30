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
* Description:  Zbuffered line drawing routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

#ifdef	MGL_FIX3D
/****************************************************************************
DESCRIPTION:
Rasterize a smooth, zbuffered RGB shaded line.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate of first endpoint (syntax 1)
y1	- y coordinate of first endpoint (syntax 1)
z1	- z coordinate of first endpoint (syntax 1)
r1	- Red component of first endpoint (syntax 1)
g1	- Green component of first endpoint (syntax 1)
b1	- Blue component of first endpoint (syntax 1)
x2	- x coordinate of second endpoint (syntax 1)
y2	- y coordinate of second endpoint (syntax 1)
z2	- z coordinate of second endpoint (syntax 1)
r1	- Red component of second endpoint (syntax 1)
g1	- Green component of second endpoint (syntax 1)
b1	- Blue component of second endpoint (syntax 1)
p1	- First endpoint (syntax 2)
p2	- Second endpoint (syntax 2)

REMARKS:
Draws a zbuffered, RGB shaded line in the current device context. The color
values will be linearly interpolated between the color at the first endpoint
and the color at the second endpoint, producing a smooth shaded line. Note that
each RGB color channel is interpolated individually, so this function will
produce lines with properly blended RGB colors.
****************************************************************************/
/* {secret} */
void ASMAPI MGL_rgbzLineCoord(
	fix32_t x1,
	fix32_t y1,
	zfix32_t z1,
	fix32_t r1,
	fix32_t g1,
	fix32_t b1,
	fix32_t x2,
	fix32_t y2,
	zfix32_t z2,
	fix32_t r2,
	fix32_t g2,
	fix32_t b2)
{
	/* Ensure that we are always scan converting the line from right to
	 * left.
	 */
	if ((x2 - x1) < 0) {
		fix32_t	t;
		SWAPT(x2,x1,t);
		SWAPT(y2,y1,t);
		SWAPT(z2,z1,t);
		SWAPT(r2,r1,t);
		SWAPT(g2,g1,t);
		SWAPT(b2,b1,t);
		}
	if (DC.a.clip && !_MGL_clipRGBZLineFX(
			&x1,&y1,&z1,&r1,&g1,&b1,
			&x2,&y2,&z2,&r2,&g2,&b2,
			DC.clipRectFX.left,DC.clipRectFX.top,
			DC.clipRectFX.right,DC.clipRectFX.bottom))
		return;
	DC.r.z.rgbzLine(
		x1 + DC.viewPortFX.left,y1 + DC.viewPortFX.top,z1,r1,g1,b1,
		x2 + DC.viewPortFX.left,y2 + DC.viewPortFX.top,z2,r2,g2,b2);
}
#endif	/* !MGL_FIX3D */
