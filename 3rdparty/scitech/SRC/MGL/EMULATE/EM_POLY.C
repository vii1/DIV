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
* Description:	Convex polygon drawing routines with clipping. Polygon
*				clipping is done on a scissoring basis, where each scan
*				line is clipped individually, rather than using a routine
*				such as the Sutherland Hodgeman polygon clipping algorithm.
*				This approach has been taken since we don't expect to clip
*				polygons all that often, and those that are clipped will
*				only require a small amount of clipping.
*
* 				This algorithm is a lot faster than the more general complex
*				polygon scan conversion algorithm, since we can take
*				advantage of the fact that every scan line in the polygon
*				will only cross a maximum of two actives edges in the
*				polygon at a time.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

PRIVATE void renderPoly(int count,fxpoint_t *lArray,fxpoint_t *rArray,
	fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		renderPoly
* Parameters:	count		- Number of vertices in lists
*				lArray		- Array of vertices for first edge
*				rArray		- Array of vertices for second edge
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a pre-setup polygon with the vertices passed
*				in the left and right list arrays.
*
****************************************************************************/
{
	fix32_t		y,endy;
	trapFunc	trap;

	/* If we are rendering in non-REPLACE or patterned mode, then we
	 * need to use the C version of the trapezoid filling routine that
	 * will correctly handle these cases.
	 */
	if (DC.a.writeMode != MGL_REPLACE_MODE)
		DC.doClip = true;
	trap = DC.doClip ? __EMU__trap : DC.r.cur.trap;

	/* Setup for rendering the first edge in left list */
	while (_MGL_computeSlope(lArray,lArray+1,&DC.tr.slope1) <= 0) {
		lArray++;
		if ((--count) == 0)
			return;					/* Bail out for zero height polys	*/
		}

	/* Setup for rendering the first edge in right list */
	while (_MGL_computeSlope(rArray,rArray+1,&DC.tr.slope2) <= 0)
		rArray++;

	/* Now render the polygon as a series of trapezoidal slices			*/
	DC.tr.x1 = lArray->x+xOffset;
	DC.tr.x2 = rArray->x+xOffset;
	y = lArray->y;
	DC.tr.y = MGL_FIXROUND(y+yOffset);

	while (1) {
		endy = lArray[1].y;
		if (endy > rArray[1].y)
			endy = rArray[1].y;

		/* Draw the trapezoid */
		DC.tr.count = MGL_FIXROUND(endy) - MGL_FIXROUND(y);
		trap();

		/* Advance the left and right edges */
		if (lArray[1].y == endy) {	/* Has left edge ended?				*/
			lArray++;
			while ((count = _MGL_computeSlope(lArray,lArray+1,&DC.tr.slope1)) <= 0) {
				lArray++;
				if (count < 0)
					return;			/* No more left edges				*/
				}
			DC.tr.x1 = lArray->x+xOffset;
			}
		if (rArray[1].y == endy) {	/* Has right edge ended?			*/
			rArray++;
			while ((count = _MGL_computeSlope(rArray,rArray+1,&DC.tr.slope2)) <= 0) {
				rArray++;
				if (count < 0)
					return;			/* No more right edges				*/
				}
			DC.tr.x2 = rArray->x+xOffset;
			}
		y = endy;
		}
}

void _ASMAPI __EMU__polygon(int count,fxpoint_t *vArray,int vinc,
	fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		__EMU__polygon
* Parameters:	count	- Number of vertices to draw
*				vArray	- Array of vertices
*				vinc	- Increment to get to next vertex
*				xOffset	- Offset of x coordinates
*				yOffset	- Offset of y coordinates
*
* Description:	Scan converts a filled convex polygon. A "convex" polygon
*				is defined as a polygon such that eveyr horizontal line
*				drawn through the polygon would cross exactly two active
*				edges (neither horizontal lines nor zero-length edges count
*				as active edges; both are acceptable anywhere in the
*				polygon). Right & left edges may cross (polygons may be
*				nonsimple). Attempting to scan convert a polygon that does
*				non fit this description will produce unpredictable results.
*
*				All vertices are offset by (xOffset,yOffset).
*
****************************************************************************/
{
	int			i,minIndex;
	fxpoint_t	*p,*v,*lArray,*rArray;
	fix32_t		y;

	if (count < 3)
		return;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpoint_t*)((uchar*)_MGL_buf + (count+1) * sizeof(fxpoint_t));

	/* Scan the list of vertices to determine the lowest vertex 		*/
	minIndex = 0;
	y = vArray->y;
	for (i = 1, p = VTX(vArray,vinc,1); i < count; i++, INCVTX(p,vinc)) {
		if (p->y < y) {
			y = p->y;
			minIndex = i;
			}
		}

	/* Copy all vertices from lowest to end into left list 			*/
	v = VTX(vArray,vinc,minIndex);
	for (i = minIndex,p = lArray; i < count; i++,INCVTX(v,vinc))
		*p++ = *v;

	/* Copy all vertices from start to lowest into left list 			*/
	for (i = 0,v = vArray; i < minIndex; i++,INCVTX(v,vinc))
		*p++ = *v;
	p->y = -1;                    	/* Terminate the list				*/

	/* Copy the right list as the left list in reverse order */
	p = rArray;
	*p++ = *lArray;					/* First element the same			*/
	for (i = count-1; i > 0; i--)
		*p++ = lArray[i];
	p->y = -1;						/* Terminate the list				*/

	renderPoly(count,lArray,rArray,xOffset,yOffset);
}

#ifdef	MGL_FIX3D

void _ASMAPI __EMU__tri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
	fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		__EMU__tri
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a flat shaded triangle. All vertices are
*				offset by (xOffset,yOffset).
*
****************************************************************************/
{
	fxpoint_t	*t,*lArray,*rArray;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpoint_t*)((uchar*)_MGL_buf + 4 * sizeof(fxpoint_t));

	/* Sort vertices in y order */
	if (v3->y < v2->y)
		SWAPT(v2,v3,t);
	if (v2->y < v1->y)
		SWAPT(v1,v2,t);
	if (v3->y < v2->y)
		SWAPT(v2,v3,t);

	/* Copy all vertices from lowest to highest into left list */
	lArray[0] = *v1;
	lArray[1] = *v2;
	lArray[2] = *v3;
	lArray[3].y = -1;				/* Terminate the list 				*/

	/* Copy the right list as the left list in reverse order */
	rArray[0] = *v1;
	rArray[1] = *v3;
	rArray[2] = *v2;
	rArray[3].y = -1;				/* Terminate the list 				*/

	renderPoly(3,lArray,rArray,xOffset,yOffset);
}

void _ASMAPI __EMU__quad(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
	fxpoint_t *v4,fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		__EMU__quad
* Parameters:	v1,v2,v3,v4	- Vertices in the quad
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a flat shaded quad. All vertices are
*				offset by (xOffset,yOffset).
*
****************************************************************************/
{
	fxpoint_t	*lArray,*rArray,*sorted[4],**v,*p;
	int			minIndex;
	fix32_t		y;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpoint_t*)((uchar*)_MGL_buf + 5 * sizeof(fxpoint_t));

	/* Scan the list of vertices to determine the lowest vertex 		*/
	sorted[0] = v1;
	minIndex = 0;
	y = v1->y;
	sorted[1] = v2;
	if (v2->y < y) {
		y = v2->y;
		minIndex = 1;
		}
	sorted[2] = v3;
	if (v3->y < y) {
		y = v3->y;
		minIndex = 2;
		}
	sorted[3] = v4;
	if (v4->y < y)
		minIndex = 3;

	/* Copy all vertices from lowest to highest into left list */
	v = &sorted[minIndex];
	p = lArray;
	*p++ = **v++;
	if (++minIndex >= 4) {
		v = sorted;
		minIndex = 0;
		}
	*p++ = **v++;
	if (++minIndex >= 4) {
		v = sorted;
		minIndex = 0;
		}
	*p++ = **v++;
	if (++minIndex >= 4) {
		v = sorted;
		minIndex = 0;
		}
	*p++ = **v++;
	p->y = -1;                    	/* Terminate the list				*/

	/* Copy the right list as the left list in reverse order */
	rArray[0] = lArray[0];
	rArray[1] = lArray[3];
	rArray[2] = lArray[2];
	rArray[3] = lArray[1];
	rArray[4].y = -1;				/* Terminate the list 				*/

	renderPoly(4,lArray,rArray,xOffset,yOffset);
}

#endif	/* MGL_FIX3D */

#endif	/* !MGL_LITE */
