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
* Description:	Rectangle region generation routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

#undef MGL_rgnSolidRectPt
#undef MGL_rgnSolidRect

/****************************************************************************
DESCRIPTION:
Generate a solid rectangle as a region.

HEADER:
mgraph.h

PARAMETERS:
left	- Left coordinate of the rectangle
top		- Top coordinate of the rectangle
right	- Right coordinate of the rectangle
bottom	- Bottom coordinate of the rectangle
r	- Rectangle to generate as a region
lt	- Point containing left-top coordinates of the region
rb	- Point containing right-bottom coordinates of the region

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function generates a solid rectangle as a region.

SEE ALSO:
MGL_rgnSolidRect, MGL_rgnSolidRectPt
****************************************************************************/
region_t * MGLAPI MGL_rgnSolidRectCoord(
	int left,
	int top,
	int right,
	int bottom)
{
	region_t *r = MGL_newRegion();
	r->rect.left = left;
	r->rect.top = top;
	r->rect.right = right;
	r->rect.bottom = bottom;
	r->spans = NULL;
	return r;
}

/****************************************************************************
DESCRIPTION:
Generate a solid rectangle as a region from two points.

HEADER:
mgraph.h

PARAMETERS:
lt	- Point containing left-top coordinates of the region
rb	- Point containing right-bottom coordinates of the region

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function generates a solid rectangle as a region.

SEE ALSO:
MGL_rgnSolidRectCoord, MGL_rgnSolidRect
****************************************************************************/
region_t MGL_rgnSolidRectPt(
	point_t lt,
	point_t rb);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Generate a solid rectangle as a region from two points.

HEADER:
mgraph.h

PARAMETERS:
r	- Rectangle the coordinates of the region

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function generates a solid rectangle as a region.

SEE ALSO:
MGL_rgnSolidRectCoord, MGL_rgnSolidRectPt
****************************************************************************/
void MGL_rgnSolidRect(
	rect_t r);
/* Implemented as a macro */

#endif	/* !MGL_LITE */

