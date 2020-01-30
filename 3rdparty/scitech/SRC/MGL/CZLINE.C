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

PARAMETERS:
p1	- The first point on the line.
p2	- The second point on the line.
x1	- x coordinate for first endpoint in line
y1	- y coordinate for first endpoint in line
z1	- z coordinate for first endpoint in line
c1	- c coordinate for first endpoint in line
x2	- x coordinate for second endpoint in line
y2	- y coordinate for second endpoint in line
z2	- z coordinate for second endpoint in line
c2	- c coordinate for third endpoint in line

REMARKS:
****************************************************************************/
/* {secret} */
void ASMAPI MGL_czLineCoord(
	fix32_t x1,
	fix32_t y1,
	zfix32_t z1,
	fix32_t c1,
	fix32_t x2,
	fix32_t y2,
	zfix32_t z2,
	fix32_t c2)
{
	/* Ensure that we are always scan converting the line from right to
	 * left.
	 */
	if ((x2 - x1) < 0) {
		fix32_t	t;
		SWAPT(x2,x1,t);
		SWAPT(y2,y1,t);
		SWAPT(z2,z1,t);
		SWAPT(c2,c1,t);
		}
	if (DC.a.clip && !_MGL_clipCZLineFX(&x1,&y1,&z1,&c1,&x2,&y2,&z2,&c2,
			DC.clipRectFX.left,DC.clipRectFX.top,
			DC.clipRectFX.right,DC.clipRectFX.bottom))
		return;
	DC.r.z.czLine(
		x1 + DC.viewPortFX.left,y1 + DC.viewPortFX.top,z1,c1,
		x2 + DC.viewPortFX.left,y2 + DC.viewPortFX.top,z2,c2);
}
#endif	/* !MGL_FIX3D */
