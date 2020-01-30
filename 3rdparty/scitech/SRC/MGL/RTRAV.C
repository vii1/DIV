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
* Description:	Region traversal routines
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

#undef	MGL_ptInRegion

/****************************************************************************
DESCRIPTION:
Traverses a region for all rectangles in definition.

HEADER:
mgraph.h

PARAMETERS:
rgn	- Region to traverse
doRect	- Callback function to call for every rectangle processed

REMARKS:
This function traverses the definition of the region, calling the supplied callback
function once for every rectangle in union of rectangles that make up the complex
region.

SEE ALSO:
MGL_diffRegion, MGL_unionRegion, MGL_sectRegion

****************************************************************************/
void MGLAPI MGL_traverseRegion(
	region_t *rgn,
	rgncallback_t doRect)
{
	segment_t	*seg;
	span_t		*s = rgn->spans;
	rect_t		r;

	if (!s)
		doRect(&rgn->rect);
	else {
		while (s->next) {
			r.top = s->y;
			r.bottom = s->next->y;
			for (seg = s->seg; seg; seg = seg->next->next) {
				r.left = seg->x;
				r.right = seg->next->x;
				doRect(&r);
				}
			s = s->next;
			}
		}
}

#ifndef	MGL_LITE
/****************************************************************************
DESCRIPTION:
Determines if a point is contained in a specified region.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to test for inclusion
y	- y coordinate to test for inclusion
rgn	- Region to test


RETURNS:
True if the point is contained in the region, false if not.

REMARKS:
This function determines if a specified point is contained within a particular region.
Note that if the region has a hole it in, and the point lies within the hole, then the
point is classified as not being included in the region.

SEE ALSO:
MGL_ptInRegion
****************************************************************************/
ibool MGLAPI MGL_ptInRegionCoord(
	int x,
	int y,
	const region_t *rgn)
{
	int			topy,boty;
	segment_t	*seg;
	span_t		*s = rgn->spans;

	if (!s)
		return MGL_ptInRectCoord(x,y,rgn->rect);
	else {
		while (s->next) {
			topy = s->y;
			boty = s->next->y;
			for (seg = s->seg; seg; seg = seg->next->next)
				if (topy <= y && y < boty && seg->x <= x && x < seg->next->x)
					return true;
			s = s->next;
			}
		}
	return false;
}

/****************************************************************************
DESCRIPTION:
Determines if a point is contained in a specified region.

HEADER:
mgraph.h

PARAMETERS:
p	- point structure containing coordinate to test
rgn	- Region to test


RETURNS:
True if the point is contained in the region, false if not.

REMARKS:
This function is the same as MGL_ptInRegionCoord, however it takes the
coordinate of the point to test as a point not two coordinates.

SEE ALSO:
MGL_ptInRegion
****************************************************************************/
ibool MGL_ptInRegion(
	point_t p,
	region_t r);
/* Implemented as a macro */

#endif	/* !MGL_LITE */

