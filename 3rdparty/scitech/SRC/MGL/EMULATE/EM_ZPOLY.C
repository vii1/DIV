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
* Description:	Zbuffered polygon drawing routines with clipping. Polygon
*				clipping is done on a scissoring basis, where each scan
*				line is clipped individually, rather than using a routine
*				such as the Sutherland Hodgeman polygon clipping algorithm.
*				This approach has been taken since we don't expect to clip
*				polygons all that often, and those that are clipped will
*				only require a small amount of clipping. Also, any 3D
*				library will most likely turn off the 2D clipping and do
*				all clipping in 3D space.
*
* 				Zbuffered polygon routine. This routine draws a convex
*				polygon with hidden surface removal. Z values are provided
*				for each of the vertices in the polygon, and the Z coord is
*				linearly interpolated across the entire polygon to perform
*				the HSR.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_FIX3D

/*------------------------- Implementation --------------------------------*/

#ifndef	__16BIT__

PRIVATE void zRenderPoly(int count,fxpointz_t *lArray,fxpointz_t *rArray,
	fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
/****************************************************************************
*
* Function:		zRenderPoly
* Parameters:	count		- Number of vertices in lists
*				lArray		- Array of vertices for first edge
*				rArray		- Array of vertices for second edge
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*				zOffset		- Offset of z coordinates
*
* Description:	Scan converts a pre-setup zbuffered polygon with the
*				vertices passed in the left and right list arrays.
*
****************************************************************************/
{
	int			i;
	fix32_t		y,endy,minx,maxx;
	fxpointz_t	*v;
	trapFunc	trap;

	/* If we are rendering in non-REPLACE then we need to use the C version
	 * of the trapezoid filling routine that will correctly handle this.
	 */
	if (DC.a.writeMode != MGL_REPLACE_MODE)
		DC.doClip = true;
	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		trap = DC.doClip ? __EMU__zTrap : DC.r.z.zDitherTrap;
	else
		trap = DC.doClip ? __EMU__zTrap : DC.r.z.zTrap;

	/* Scan the list of vertices to determine polygon x extents */
	minx = maxx = lArray->p.x;
	for (i = 1, v = lArray+1; i < count; i++, v++) {
		if (v->p.x < minx)
			minx = v->p.x;
		if (v->p.x > maxx)
			maxx = v->p.x;
		}

	/* Setup for rendering the first edge in left list */
	while (_MGL_computeSlope(&lArray->p,&lArray[1].p,&DC.tr.slope1) <= 0) {
		lArray++;
		if ((--count) == 0)
			return;					/* Bail out for zero height polys	*/
		}

	/* Setup for rendering the first edge in right list */
	while (_MGL_computeSlope(&rArray->p,&rArray[1].p,&DC.tr.slope2) <= 0)
		rArray++;

	/* Compute the zbuffer slopes for the entire polygon face */
	_MGL_zComputeSlope(lArray,lArray+1,rArray+1);

	/* Now render the polygon as a series of trapezoidal slices			*/
	DC.tr.x1 = lArray->p.x+xOffset;
	DC.tr.x2 = rArray->p.x+xOffset;
	DC.tr.z0 = lArray->z+zOffset;
	y = lArray->p.y;
	DC.tr.y = MGL_FIXROUND(y+yOffset);

	/* Pre-render the pre-combined z buffer offsets into local buffer */
	if (DC.zbits == 16)
		_MGL_preRenderScanLine16(MGL_FIXROUND(DC.tr.x1),
			MGL_FIXROUND(minx+xOffset),MGL_FIXROUND(maxx+xOffset));
	else if (DC.zbits == 32)
		_MGL_preRenderScanLine32(MGL_FIXROUND(DC.tr.x1),
			MGL_FIXROUND(minx+xOffset),MGL_FIXROUND(maxx+xOffset));
	else
		_MGL_preRenderScanLine24(MGL_FIXROUND(DC.tr.x1),
			MGL_FIXROUND(minx+xOffset),MGL_FIXROUND(maxx+xOffset));

	while (1) {
		endy = lArray[1].p.y;
		if (endy > rArray[1].p.y)
			endy = rArray[1].p.y;

		/* Draw the trapezoid */
		DC.tr.count = MGL_FIXROUND(endy) - MGL_FIXROUND(y);
		trap();

		/* Advance the left and right edges */
		if (lArray[1].p.y == endy) {/* Has left edge ended?				*/
			lArray++;
			while ((count = _MGL_computeSlope(&lArray->p,&lArray[1].p,&DC.tr.slope1)) <= 0) {
				lArray++;
				if (count < 0)
					return;			/* No more left edges				*/
				}
			DC.tr.x1 = lArray->p.x+xOffset;
			}
		if (rArray[1].p.y == endy) {/* Has right edge ended?			*/
			rArray++;
			while ((count = _MGL_computeSlope(&rArray->p,&rArray[1].p,&DC.tr.slope2)) <= 0) {
				rArray++;
				if (count < 0)
					return;			/* No more right edges				*/
				}
			DC.tr.x2 = rArray->p.x+xOffset;
			}
		y = endy;
		}
}

void _ASMAPI __EMU__zTri(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
	fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
/****************************************************************************
*
* Function:		__EMU__zTri
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*				zOffset		- Offset of z coordinates
*
* Description:	Scan converts a flat shaded triangle. All vertices are
*				offset by (xOffset,yOffset).
*
****************************************************************************/
{
	fxpointz_t	*t,*lArray,*rArray;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpointz_t*)((uchar*)_MGL_buf + 3 * sizeof(fxpointz_t));

	/* Sort vertices in y order */
	if (v3->p.y < v2->p.y)
		SWAPT(v2,v3,t);
	if (v2->p.y < v1->p.y)
		SWAPT(v1,v2,t);
	if (v3->p.y < v2->p.y)
		SWAPT(v2,v3,t);

	/* Copy all vertices from lowest to highest into left list */
	lArray[0] = *v1;
	lArray[1] = *v2;
	lArray[2] = *v3;
	lArray[3].p.y = -1;				/* Terminate the list 				*/

	/* Copy the right list as the left list in reverse order */
	rArray[0] = *v1;
	rArray[1] = *v3;
	rArray[2] = *v2;
	rArray[3].p.y = -1;				/* Terminate the list 				*/

	zRenderPoly(3,lArray,rArray,xOffset,yOffset,zOffset);
}

void _ASMAPI __EMU__zQuad(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3,
	fxpointz_t *v4,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
/****************************************************************************
*
* Function:		__EMU__zQuad
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*				zOffset		- Offset of z coordinates
*
* Description:	Scan converts a flat shaded zbuffered quad. All vertices are
*				offset by (xOffset,yOffset).
*
****************************************************************************/
{
	fxpointz_t	*lArray,*rArray,*sorted[4],**v,*p;
	int			minIndex;
	fix32_t		y;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpointz_t*)((uchar*)_MGL_buf + 5 * sizeof(fxpointz_t));

	/* Scan the list of vertices to determine the lowest vertex 		*/
	sorted[0] = v1;
	minIndex = 0;
	y = v1->p.y;
	sorted[1] = v2;
	if (v2->p.y < y) {
		y = v2->p.y;
		minIndex = 1;
		}
	sorted[2] = v3;
	if (v3->p.y < y) {
		y = v3->p.y;
		minIndex = 2;
		}
	sorted[3] = v4;
	if (v4->p.y < y)
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
	p->p.y = -1;                    	/* Terminate the list				*/

	/* Copy the right list as the left list in reverse order */
	rArray[0] = lArray[0];
	rArray[1] = lArray[3];
	rArray[2] = lArray[2];
	rArray[3] = lArray[1];
	rArray[4].p.y = -1;				/* Terminate the list 				*/

	zRenderPoly(4,lArray,rArray,xOffset,yOffset,zOffset);
}

#endif

#endif	/* MGL_FIX3D */
