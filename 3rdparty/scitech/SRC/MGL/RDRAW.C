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
* Description:	Scanline coherent shape algebra routines for implementing
*				complex clipping paths in the MGL.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
DESCRIPTION:
Draw a solid complex region.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to draw region at
y	- y coordinate to draw region at
rgn	- region to draw

REMARKS:
Draws the complex region at the specified location in the current pattern and write
mode.

SEE ALSO:
MGL_newRegion, MGL_copyRegion, MGL_freeRegion, MGL_diffRegion,
MGL_unionRegion,  MGL_sectRegion

****************************************************************************/
void MGLAPI MGL_drawRegion(
	int x,
	int y,
	const region_t *rgn)
{
	region_t	*clipped = (region_t*)rgn,*clipRgn;
	rect_t		r;

	if (DC.a.clip) {
		if (MGL_sectRect(DC.a.clipRect,rgn->rect,&r)) {
			if (!MGL_equalRect(rgn->rect,r)) {
				_MGL_tmpRectRegion2(clipRgn,DC.a.clipRect);
				clipped = MGL_sectRegion(rgn,clipRgn);
				if (MGL_emptyRect(clipped->rect))
					goto Done;
				}
			}
		else return;	/* Trivially reject region */
		}
	if (!clipped->spans) {
		/* Region is a special case rectangle, so render it directly */
		DC.r.cur.fillRect(x+clipped->rect.left,y+clipped->rect.top,
			x+clipped->rect.right,y+clipped->rect.bottom);
		}
	else
		DC.r.cur.drawRegion(DC.a.viewPort.left + x, DC.a.viewPort.top + y,
			clipped);

Done:
	if (clipped != rgn)
		MGL_freeRegion(clipped);
}

#endif	/* !MGL_LITE */
