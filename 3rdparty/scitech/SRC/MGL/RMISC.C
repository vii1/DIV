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
* Description:	Miscellaneous region manipulation routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*--------------------------- Global Variables ----------------------------*/

segment_t	_VARAPI _MGL_tmpX2 	= { NULL, 0 };
segment_t	_VARAPI _MGL_tmpX1 	= { &_MGL_tmpX2, 0 };
span_t		_VARAPI _MGL_tmpY2	= { NULL, NULL, 0 };
span_t		_VARAPI _MGL_tmpY1	= { &_MGL_tmpY2, &_MGL_tmpX1, 0 };
region_t	_VARAPI _MGL_rtmp	= { {0, 0, 0, 0}, &_MGL_tmpY1 };
region_t	_VARAPI _MGL_rtmp2	= { {0, 0, 0, 0}, NULL };

/*------------------------- Implementation --------------------------------*/

#undef	MGL_isSimpleRegion

/* {secret} */
span_t * _MGL_rectRegion(region_t *r)
/****************************************************************************
*
* Function:		_MGL_rectRegion
* Parameters:	r	- Region to expand
* Returns:		Pointer to the first span in the region
*
* Description:	Expands the regions from a special case rectangle into a
*				proper rectangular region.
*
****************************************************************************/
{
	span_t		*s1,*s2;
	segment_t	*seg;

	r->spans = s1 = _MGL_newSpan();
	s1->y = r->rect.top;
	seg = (s1->seg = _MGL_newSegment());
	seg->x = r->rect.left;
	seg = (seg->next = _MGL_newSegment());
	seg->x = r->rect.right;
	seg->next = NULL;
	s1->next = s2 = _MGL_newSpan();
	s2->y = r->rect.bottom;
	s2->seg = NULL;
	s2->next = NULL;
	return s1;
}

/* {secret} */
span_t * _MGL_createSpan(int y,int x1,int x2)
/****************************************************************************
*
* Function:		_MGL_createSpan
* Parameters:	y	- Y coordinate for span
*				x1	- X1 coordinate for span
*				x2	- X2 coordinate for span
* Returns:		Pointer to the new span
*
* Description:	Function to create a single scanline span. This should be
*				turned into a macro for speed.
*
****************************************************************************/
{
	segment_t	*seg;
	span_t 		*st;

	st = _MGL_newSpan();
	st->y = y;
	seg = (st->seg = _MGL_newSegment());
	seg->x = x1;
	seg = (seg->next = _MGL_newSegment());
	seg->x = x2;
	seg->next = NULL;
	return st;
}

/* {secret} */
span_t * _MGL_createSpan2(int y,int x1,int x2,int x3,int x4)
/****************************************************************************
*
* Function:		_MGL_createSpan2
* Parameters:	y	- Y coordinate for span
*				x1	- X1 coordinate for first segment
*				x2	- X2 coordinate for first segment
*				x3	- X1 coordinate for second segment
*				x4	- X2 coordinate for second segment
* Returns:		Pointer to the new span
*
* Description:	Function to create a double scanline span. This should be
*				turned into a macro for speed.
*
****************************************************************************/
{
	segment_t	*seg;
	span_t 		*st;

	st = _MGL_newSpan();
	st->y = y;
	seg = (st->seg = _MGL_newSegment());
	seg->x = x1;
	seg = (seg->next = _MGL_newSegment());
	seg->x = x2;
	seg = (seg->next = _MGL_newSegment());
	seg->x = x3;
	seg = (seg->next = _MGL_newSegment());
	seg->x = x4;
	seg->next = NULL;
	return st;
}

/* {secret} */
region_t *_MGL_createRegionFromBuf(scanlist *scanList)
/****************************************************************************
*
* Function:		_MGL_createRegionFromBuf
* Parameters:	scanList	- Pointer to scanlist to build region from
* Returns:		Pointer to the newly created region
*
* Description:	Builds a region definition from a shape rendered into the
*				scratch buffer.
*
****************************************************************************/
{
	int			i,maxIndex,l,r,minx,maxx;
	scanline	*scanPtr;
	span_t		**sp,*head,*st;
	region_t	*rgn = MGL_newRegion();

	/* Initialise the span list */
	head = NULL;
	sp = &head;
	minx = 32767;
	maxx = -32767;

	/* Now build the region spans from the scratch buffer */
	maxIndex = scanList->top + scanList->length;
	scanPtr = scanList->scans;
	for (i = scanList->top; i < maxIndex; i++,scanPtr++) {
		*sp = st = _MGL_createSpan(i,l = scanPtr->left,r = scanPtr->right);
		sp = &st->next;
		minx = MIN(minx,l);
		maxx = MAX(maxx,r);
		}

	/* Terminate the span list */
	*sp = st = _MGL_newSpan();
	st->y = i;
	st->seg = NULL;
	st->next = NULL;

	/* Fill in the bounding rectangle for the region */
	rgn->rect.left = minx;
	rgn->rect.right = maxx;
	rgn->rect.top = scanList->top;
	rgn->rect.bottom = maxIndex;

	rgn->spans = head;
	return rgn;
}

/****************************************************************************
DESCRIPTION:
Offsets a region by the specified amount.

HEADER:
mgraph.h

PARAMETERS:
r	- Region to offset
dx	- Amount to offset x coordinates by
dy	- Amount to offset y coordinates by

REMARKS:
This function offsets the specified region by the dx and dy coordinates, by actually
modifying all the coordinate locations for every rectangle in the union of rectangles
that constitutes the region by the specified coordinates.

SEE ALSO:
MGL_unionRegion, MGL_diffRegion, MGL_sectRegion

****************************************************************************/
void MGLAPI MGL_offsetRegion(
	region_t *r,
	int dx,
	int dy)
{
	span_t		*s;
	segment_t	*p;

	/* Bail out for empty regions */
	if (MGL_emptyRegion(r))
		return;

	MGL_offsetRect(r->rect,dx,dy);
	for (s = r->spans; s; s = s->next) {
		s->y += dy;
		for (p = s->seg; p; p = p->next)
			p->x += dx;
		}
}

/****************************************************************************
DESCRIPTION:
Determines if two regions are equal.

HEADER:
mgraph.h

PARAMETERS:
r1	- First region to compare
r2	- Second region to compare

RETURNS:
True if the regions are equal, false if not.

REMARKS:
Determines if two regions are equal, by comparing the bounding rectangles and the
definitions for both of the regions.

SEE ALSO:
****************************************************************************/
ibool MGLAPI MGL_equalRegion(
	const region_t *r1,
	const region_t *r2)

{
	span_t		*s1,*s2;
	segment_t	*seg1,*seg2;

	s1 = r1->spans;
	s2 = r2->spans;
	if (!s1 && !s2)
		return MGL_equalRect(r1->rect,r2->rect);
	if (!s1 || !s2)
		return false;

	while (s1 && s2) {
		if (s1->y != s2->y)
			return false;
		seg1 = s1->seg;
		seg2 = s2->seg;
		while (seg1 && seg2) {
			if (seg1->x != seg2->x)
				return false;
			seg1 = seg1->next;
			seg2 = seg2->next;
			}
		if (!seg1 || !seg2)
			return false;
		s1 = s1->next;
		s2 = s2->next;
		}
	return !s1 && !s2;
}


/****************************************************************************
DESCRIPTION:
Determines if a region is empty.

HEADER:
mgraph.h

PARAMETERS:
r	- region to test

RETURNS:
True if region is empty, false if not.

REMARKS:
Determines if a region is empty or not. A region is defined as being empty if the
bounding rectangle's right coordinate is less than or equal to the left coordinate, or
if the bottom coordinate is less than or equal to the top coordinate.

SEE ALSO:
MGL_equalRegion, MGL_unionRegion, MGL_diffRegion, MGL_sectRegion,
MGL_offsetRegion, MGL_ptInRegion, MGL_ptInRegionCoord
****************************************************************************/
ibool MGLAPI MGL_emptyRegion(
	const region_t *r)
{ return MGL_emptyRect(r->rect); }

/***************************************************************************
DESCRIPTION:
Returns true if a region is a simple region, otherwise false.

HEADER:
mgraph.h

PARAMETERS:
r	- Region to test.

REMARKS:
This function determines if the region is simple or not. A simple region is
one that consists of only a single rectangle. This function will not work
properly if the region has been through a number of region algebra routines
with other non-simple regions, even though the end result may be a single
rectangle.

SEE ALSO:
MGL_unionRegion, MGL_diffRegion, MGL_sectRegion
***************************************************************************/
ibool MGL_isSimpleRegion(
	region_t r);
/* Implemented as a macro */

#endif	/* !MGL_LITE */
