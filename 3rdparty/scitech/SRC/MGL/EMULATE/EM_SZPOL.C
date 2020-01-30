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
* Description:	Zbuffered, shaded polygon drawing routines with clipping. Polygon
*				clipping is done on a scissoring basis, where each scan
*				line is clipped individually, rather than using a routine
*				such as the Sutherland Hodgeman polygon clipping algorithm.
*				This approach has been taken since we don't expect to clip
*				polygons all that often, and those that are clipped will
*				only require a small amount of clipping.
*
* 				Convex gouraud shaded polygon routine. This routine draws a
*				convex polygon using gouraud shading. Colors are provided
*				for each of the vertices in the polygon, and the color is
*				linearly interpolated across the entire polygon to give a
*				smooth shading effect. Note that in color mapped modes, the
*				color index in interpolated, so the palette must be set up
*				accordingly.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_FIX3D

/*------------------------- Implementation --------------------------------*/

#ifndef	__16BIT__

PRIVATE void rgbzShadePoly(int count,fxpointrgbz_t *lArray,
	fxpointrgbz_t *rArray,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
/****************************************************************************
*
* Function:		rgbzShadePoly
* Parameters:	count		- Number of vertices in lists
*				lArray		- Array of vertices for first edge
*				rArray		- Array of vertices for second edge
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a pre-setup shaded zbuffered polygon with the
*				vertices passed in the left and right list arrays.
*
****************************************************************************/
{
	int				i;
	fix32_t			y,endy,minx,maxx;
	fxpointrgbz_t	*v;
	trapFunc		trap;

	/* If we are rendering in non-REPLACE then we need to use the C version
	 * of the trapezoid filling routine that will correctly handle this.
	 */
	if (DC.a.writeMode != MGL_REPLACE_MODE)
		DC.doClip = true;
	trap = DC.doClip ? __EMU__rgbzTrap : DC.r.z.rgbzTrap;

	/* Scan the list of vertices to determine polygon x extents */
	minx = maxx = lArray->p.x;
	for (i = 1, v = lArray+1; i < count; i++, v++) {
		if (v->p.x < minx)
			minx = v->p.x;
		if (v->p.x > maxx)
			maxx = v->p.x;
		}

#define	V(v)	((fxpointrgb_t *)(v))
#define	VZ(v)	((fxpointz_t *)(&(v)->p))

	/* Setup for rendering the first edge in left list */
	while (_MGL_rgbComputeSlope(V(lArray),V(lArray+1),&DC.tr.slope1,&DC.tr.rSlope1,&DC.tr.gSlope1,&DC.tr.bSlope1) <= 0) {
		lArray++;
		if ((--count) == 0)
			return;					/* Bail out for zero height polys	*/
		}

	/* Setup for rendering the first edge in right list */
	while (_MGL_rgbComputeSlope(V(rArray),V(rArray+1),&DC.tr.slope2,&DC.tr.rSlope2,&DC.tr.gSlope2,&DC.tr.bSlope2) <= 0)
		rArray++;

	/* Compute the zbuffer slopes for the entire polygon face */
	_MGL_zComputeSlope(VZ(lArray),VZ(lArray+1),VZ(rArray+1));

	/* Now render the polygon as a series of trapezoidal slices			*/
	DC.tr.x1 = lArray->p.x+xOffset;
	DC.tr.x2 = rArray->p.x+xOffset;
	DC.tr.z0 = lArray->z+zOffset;
	DC.tr.r1 = lArray->c.r;
	DC.tr.r2 = rArray->c.r;
	DC.tr.g1 = lArray->c.g;
	DC.tr.g2 = rArray->c.g;
	DC.tr.b1 = lArray->c.b;
	DC.tr.b2 = rArray->c.b;
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

		/* Shade the trapezoid */
		DC.tr.count = MGL_FIXROUND(endy) - MGL_FIXROUND(y);
		trap();

		/* Advance the left and right edges */
		if (lArray[1].p.y == endy) {/* Has left edge ended?				*/
			lArray++;
			while ((count = _MGL_rgbComputeSlope(V(lArray),V(lArray+1),&DC.tr.slope1,&DC.tr.rSlope1,&DC.tr.gSlope1,&DC.tr.bSlope1)) <= 0) {
				lArray++;
				if (count < 0)
					return;			/* No more left edges				*/
				}
			DC.tr.x1 = lArray->p.x+xOffset;
			DC.tr.r1 = lArray->c.r;
			DC.tr.g1 = lArray->c.g;
			DC.tr.b1 = lArray->c.b;
			}
		if (rArray[1].p.y == endy) {/* Has right edge ended?			*/
			rArray++;
			while ((count = _MGL_rgbComputeSlope(V(rArray),V(rArray+1),&DC.tr.slope2,&DC.tr.rSlope2,&DC.tr.gSlope2,&DC.tr.bSlope2)) <= 0) {
				rArray++;
				if (count < 0)
					return;			/* No more right edges				*/
				}
			DC.tr.x2 = rArray->p.x+xOffset;
			DC.tr.r2 = lArray->c.r;
			DC.tr.g2 = lArray->c.g;
			DC.tr.b2 = lArray->c.b;
			}
		y = endy;
		}
#undef	V
#undef	VZ
}

void _ASMAPI __EMU__rgbzTri(fxpointrgbz_t *v1,fxpointrgbz_t *v2,
	fxpointrgbz_t *v3,fix32_t xOffset,fix32_t yOffset,zfix32_t zOffset)
/****************************************************************************
*
* Function:		__EMU__rgbzTri
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a smooth shaded zbuffered triangle. All
*				vertices are offset by (xOffset,yOffset).
*
****************************************************************************/
{
	fxpointrgbz_t	*t,*lArray,*rArray;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpointrgbz_t*)((uchar*)_MGL_buf + 3 * sizeof(fxpointrgbz_t));

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

	rgbzShadePoly(3,lArray,rArray,xOffset,yOffset,zOffset);
}

void _ASMAPI __EMU__rgbzQuad(fxpointrgbz_t *v1,fxpointrgbz_t *v2,
	fxpointrgbz_t *v3,fxpointrgbz_t *v4,fix32_t xOffset,fix32_t yOffset,
	zfix32_t zOffset)
/****************************************************************************
*
* Function:		__EMU__rgbzQuad
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a smooth shaded zbuffered quad. All vertices
*				offset by (xOffset,yOffset). In order to make sure that
*				the linear interpolation is handled correctly across the
*				face of the quadrilateral, we actually have to decompose
*				the quad into two triangles.
*
****************************************************************************/
{
	fxpointrgbz_t	*t,*lArray,*rArray,*vt1,*vt2,*vt3;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpointrgbz_t*)((uchar*)_MGL_buf + 4 * sizeof(fxpointrgbz_t));

	/* Render the first triangle for the quad for vertices (v1,v2,v3) */
	vt1 = v1; vt2 = v2; vt3 = v3;

	/* Sort vertices in y order */
	if (vt3->p.y < vt2->p.y)
		SWAPT(vt2,vt3,t);
	if (vt2->p.y < vt1->p.y)
		SWAPT(vt1,vt2,t);
	if (vt3->p.y < vt2->p.y)
		SWAPT(vt2,vt3,t);

	/* Copy all vertices from lowest to highest into left list */
	lArray[0] = *vt1;
	lArray[1] = *vt2;
	lArray[2] = *vt3;
	lArray[3].p.y = -1;				/* Terminate the list 				*/

	/* Copy the right list as the left list in reverse order */
	rArray[0] = *vt1;
	rArray[1] = *vt3;
	rArray[2] = *vt2;
	rArray[3].p.y = -1;				/* Terminate the list 				*/

	rgbzShadePoly(3,lArray,rArray,xOffset,yOffset,zOffset);

	/* Render the second triangle for the quad for vertices (v1,v3,v4) */
	vt1 = v1; vt2 = v3; vt3 = v4;

	/* Sort vertices in y order */
	if (vt3->p.y < vt2->p.y)
		SWAPT(vt2,vt3,t);
	if (vt2->p.y < vt1->p.y)
		SWAPT(vt1,vt2,t);
	if (vt3->p.y < vt2->p.y)
		SWAPT(vt2,vt3,t);

	/* Copy all vertices from lowest to highest into left list */
	lArray[0] = *vt1;
	lArray[1] = *vt2;
	lArray[2] = *vt3;
	lArray[3].p.y = -1;				/* Terminate the list 				*/

	/* Copy the right list as the left list in reverse order */
	rArray[0] = *vt1;
	rArray[1] = *vt3;
	rArray[2] = *vt2;
	rArray[3].p.y = -1;				/* Terminate the list 				*/

	rgbzShadePoly(3,lArray,rArray,xOffset,yOffset,zOffset);
}

#endif

#endif	/* MGL_FIX3D */
