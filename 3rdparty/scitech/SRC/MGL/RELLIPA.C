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
* Description:  Elliptical arc region generation routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*--------------------------- Global Variables ----------------------------*/

static	arc_coords_t	ac;		/* Place to store arc coordinates		*/

/*------------------------- Implementation --------------------------------*/

void _MGL_scanRectArc(scanlist2 *sl,arc_coords_t *ac,int left,int top,
	int A,int B,int startAngle,int endAngle,int penWidth,int penHeight);

static region_t * buildSolidRegion(scanlist2 *sl)
/****************************************************************************
*
* Function:		buildSolidRegion
* Parameters:	extentRect	- Bounding rectangle for ellipse
*				startAngle	- Starting angle for arc (in degrees)
*				endAngle	- Ending angle for arc (in degress)
* Returns:		Pointers to the generated elliptical arc
*
* Description:  Generates a region representing a solid elliptical arc.
*
****************************************************************************/
{
	int			i,maxIndex,minx,maxx,l,r,leftL,leftR,rightL,rightR;
	scanline2	*scanPtr;
	span_t		**sp,*head,*st;
	region_t	*rgn = MGL_newRegion();

	/* Initialise the span list */
	head = NULL;
	sp = &head;
	minx = 32767;			maxx = -32767;

	/* Skip past empty part at top of region */
	scanPtr = sl->scans;
	maxIndex = sl->top + sl->length;
	for (i = sl->top; i < maxIndex; i++,scanPtr++) {
		if (scanPtr->leftL != SENTINEL || scanPtr->rightR != SENTINEL)
			break;
		}

	/* Now build the spans from the scratch buffer */
	rgn->rect.top = i;
	for (; i < maxIndex; i++,scanPtr++) {
		leftL = scanPtr->leftL;	rightR = scanPtr->rightR;
		if (leftL == SENTINEL && rightR == SENTINEL)
			break;
		leftR = scanPtr->leftR;	rightL = scanPtr->rightL;

		if (leftR < rightL) {
			if (rightL == SENTINEL)
				*sp = st = _MGL_createSpan(i,l = leftL,r = leftR);
			else *sp = st = _MGL_createSpan2(i,l = leftL,leftR,rightL,r = rightR);
			}
		else {
			if (leftL == SENTINEL)
				*sp = st = _MGL_createSpan(i,l = rightL,r = rightR);
			else *sp = st = _MGL_createSpan(i,l = leftL,r = rightR);
			}
		minx = MIN(minx,l);		maxx = MAX(maxx,r);
		sp = &st->next;
		}
	rgn->rect.bottom = i;
	rgn->rect.left = minx;
	rgn->rect.right = maxx;

	/* Terminate the span list */
	*sp = st = _MGL_newSpan();
	st->y = i;
	st->seg = NULL;
	st->next = NULL;

	rgn->spans = head;
	return rgn;
}

/****************************************************************************
DESCRIPTION:
Generate an elliptical arc outline as a region.

HEADER:
mgraph.h

PARAMETERS:
extentRect	- Bounding rectangle for the ellipse
startAngle	- Starting angle for the elliptical arc
endAngle	- Ending angle for the elliptical arc
pen	- Region to use as the pen when drawing the ellipse

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function generates the outline of an elliptical arc as a complex region by 
dragging the specified pen region around the perimeter of the ellipse. The pen used 
can be any arbitrary shape, however rectangular pens are special cased to provide 
fast region generation.

SEE ALSO:
MGL_rgnEllipse, MGL_rgnGetArcCoords

****************************************************************************/
region_t * MGLAPI MGL_rgnEllipseArc(
	rect_t extentRect,
	int startAngle,
	int endAngle,
	const region_t *_pen)
{
	int			A,B;
	scanlist2	scanList;
	region_t	*rgn,*wedge;

	A = extentRect.right - extentRect.left - 1;	/* Major Axis		*/
	B = extentRect.bottom - extentRect.top - 1;	/* Minor Axis		*/
	if (A <= 5 || B <= 5 || B < A/5 || A < B/5 || MGL_emptyRect(extentRect))
		return MGL_newRegion();	/* Ignore degenerate ellipses		*/

	/* Scan an elliptical arc */
	rgn = MGL_rgnEllipse(extentRect,_pen);

	/* Now chop out the elliptical wedge */
#if 0
	// TODO: This needs to be fixed
	_MGL_scanRectArc(&scanList,&ac,extentRect.left,extentRect.top,A,B,
		endAngle,startAngle,
		_pen->rect.right-_pen->rect.left,
		_pen->rect.bottom-_pen->rect.top);
	wedge = buildSolidRegion(&scanList);
	MGL_diffRegion(rgn,wedge);
	MGL_freeRegion(wedge);
#endif

	return rgn;
}

/****************************************************************************
DESCRIPTION:
Generates a solid elliptical arc as a region.

HEADER:
mgraph.h

PARAMETERS:
extentRect	- Bounding rectangle for the ellipse
startAngle	- Starting angle for the elliptical arc
endAngle	- Ending angle for the elliptical arc

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function generates a solid elliptical arc as a region.

SEE ALSO:
MGL_rgnEllipse, MGL_rgnGetArcCoords
****************************************************************************/
region_t * MGLAPI MGL_rgnSolidEllipseArc(
	rect_t extentRect,
	int startAngle,
	int endAngle)
{
	int			A,B;
	scanlist2	scanList;

	A = extentRect.right - extentRect.left - 1;	/* Major Axis		*/
	B = extentRect.bottom - extentRect.top - 1;	/* Minor Axis		*/
	if (A <= 5 || B <= 5 || B < A/5 || A < B/5 || MGL_emptyRect(extentRect))
		return MGL_newRegion();	/* Ignore degenerate ellipses		*/

	/* Scan the outline of the ellipse into scratch buffer */
	_MGL_scanEllipseArc(&scanList,&ac,extentRect.left,extentRect.top,A,B,
		startAngle,endAngle);

	/* Build the span list */
	return buildSolidRegion(&scanList);
}

/****************************************************************************
DESCRIPTION:
Returns the real arc coordinates for an elliptical arc region.


HEADER:
mgraph.h

PARAMETERS:
coords	- Pointer to structure to store coordinates

REMARKS:
This function returns the center coordinate, and starting and ending points on the
ellipse that defines the last elliptical arc region that was generated. You can then use
these coordinates to draw a line from the center of the ellipse to the starting and
ending points to complete the outline of an elliptical wedge.

Note that you must call this routine immediately after calling the
MGL_rgnEllipseArc routine.

SEE ALSO:
MGL_rgnEllipseArc

****************************************************************************/
void MGLAPI MGL_rgnGetArcCoords(
	arc_coords_t *coords)
{ *coords = ac; }

#endif	/* !MGL_LITE */
