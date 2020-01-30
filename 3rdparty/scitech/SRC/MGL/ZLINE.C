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
Renders a zbuffered line to the current device context.

HEADER:
mgraph.h

PARAMETERS:

x1	- x coordinate for first endpoint in line
y1	- y coordinate for first endpoint in line
z1	- z coordinate for first endpoint in line
x2	- x coordinate for second endpoint in line
y2	- y coordinate for second endpoint in line
z2	- z coordinate for second endpoint in line
p1	- Point containing coordinates for first endpoint of line 
p2	- Point containing coordinates for second endpoint of line

REMARKS:
Draws a solid, depth buffered line in the current device context. The depth
values are linearly interpolated between each of the endpoints in the line to
calculate the depth values for each of the pixels along the line. Every pixel
is then tested against the depth values for the pixel at the same location in
the depth buffer. If the pixel in the depth buffer is further away than the new
pixel, the pixel in the buffer is updated, otherwise the pixel is left alone.
****************************************************************************/
/* {secret} */
void ASMAPI MGL_zLineCoord(
	fix32_t x1,
	fix32_t y1,
	zfix32_t z1,
	fix32_t x2,
	fix32_t y2,
	zfix32_t z2)
{
	/* Ensure that we are always scan converting the line from right to
	 * left.
	 */
	if ((x2 - x1) < 0) {
		fix32_t	t;
		SWAPT(x2,x1,t);
		SWAPT(y2,y1,t);
		SWAPT(z2,z1,t);
		}
	if (DC.a.clip && !_MGL_clipZLineFX(&x1,&y1,&z1,&x2,&y2,&z2,
			DC.clipRectFX.left,DC.clipRectFX.top,
			DC.clipRectFX.right,DC.clipRectFX.bottom))
		return;
	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		DC.r.z.zDitherLine(
			x1 + DC.viewPortFX.left,y1 + DC.viewPortFX.top,z1,
			x2 + DC.viewPortFX.left,y2 + DC.viewPortFX.top,z2);
	else DC.r.z.zLine(
			x1 + DC.viewPortFX.left,y1 + DC.viewPortFX.top,z1,
			x2 + DC.viewPortFX.left,y2 + DC.viewPortFX.top,z2);
}
#endif	/* !MGL_FIX3D */
