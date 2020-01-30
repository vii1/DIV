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
* Description:	Smooth shaded polygon drawing routines with clipping. Polygon
*				clipping is done on a scissoring basis, where each scan
*				line is clipped individually, rather than using a routine
*				such as the Sutherland Hodgeman polygon clipping algorithm.
*				This approach has been taken since we don't expect to clip
*				polygons all that often, and those that are clipped will
*				only require a small amount of clipping.
*
* 				Convex linear shaded polygon routine. This routine draws a
*				convex polygon using linear shading. Colors are provided
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

PRIVATE void cShadePoly(int count,fxpointc_t *lArray,fxpointc_t *rArray,
	fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		cShadePoly
* Parameters:	count		- Number of vertices in lists
*				lArray		- Array of vertices for first edge
*				rArray		- Array of vertices for second edge
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a pre-setup shaded polygon with the vertices
*				passed in the left and right list arrays.
*
****************************************************************************/
{
	fix32_t		y,endy;
	trapFunc	trap;

	/* If we are rendering in non-REPLACE then we need to use the C version
	 * of the trapezoid filling routine that will correctly handle this.
	 */
	if (DC.a.writeMode != MGL_REPLACE_MODE)
		DC.doClip = true;
	trap = DC.doClip ? __EMU__cTrap : DC.r.cTrap;

	/* Setup for rendering the first edge in left list */
	while (_MGL_cComputeSlope(lArray,lArray+1,&DC.tr.slope1,&DC.tr.cSlope1) <= 0) {
		lArray++;
		if ((--count) == 0)
			return;					/* Bail out for zero height polys	*/
		}

	/* Setup for rendering the first edge in right list */
	while (_MGL_cComputeSlope(rArray,rArray+1,&DC.tr.slope2,&DC.tr.cSlope2) <= 0)
		rArray++;

	/* Now render the polygon as a series of trapezoidal slices			*/
	DC.tr.x1 = lArray->p.x+xOffset;
	DC.tr.x2 = rArray->p.x+xOffset;
	DC.tr.c1 = lArray->c;
	DC.tr.c2 = rArray->c;
	y = lArray->p.y;
	DC.tr.y = MGL_FIXROUND(y+yOffset);

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
			while ((count = _MGL_cComputeSlope(lArray,lArray+1,&DC.tr.slope1,&DC.tr.cSlope1)) <= 0) {
				lArray++;
				if (count < 0)
					return;			/* No more left edges				*/
				}
			DC.tr.x1 = lArray->p.x+xOffset;
			DC.tr.c1 = lArray->c;
			}
		if (rArray[1].p.y == endy) {/* Has right edge ended?			*/
			rArray++;
			while ((count = _MGL_cComputeSlope(rArray,rArray+1,&DC.tr.slope2,&DC.tr.cSlope2)) <= 0) {
				rArray++;
				if (count < 0)
					return;			/* No more right edges				*/
				}
			DC.tr.x2 = rArray->p.x+xOffset;
			DC.tr.c2 = rArray->c;
			}
		y = endy;
		}
}

void _ASMAPI __EMU__cTri(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
	fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		__EMU__cTri
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a smooth shaded triangle. All vertices are
*				offset by (xOffset,yOffset).
*
****************************************************************************/
{
	fxpointc_t	*t,*lArray,*rArray;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpointc_t*)((uchar*)_MGL_buf + 4 * sizeof(fxpointc_t));

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

	cShadePoly(3,lArray,rArray,xOffset,yOffset);
}

void _ASMAPI __EMU__cQuad(fxpointc_t *v1,fxpointc_t *v2,fxpointc_t *v3,
	fxpointc_t *v4,fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		__EMU__cQuad
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				xOffset		- Offset of x coordinates
*				yOffset		- Offset of y coordinates
*
* Description:	Scan converts a flat shaded quad. All vertices are
*				offset by (xOffset,yOffset). In order to make sure that
*				the linear interpolation is handled correctly across the
*				face of the quadrilateral, we actually have to decompose
*				the quad into two triangles.
*
****************************************************************************/
{
	fxpointc_t	*t,*lArray,*rArray,*vt1,*vt2,*vt3;

	/* Get memory in which to store left and right edge arrays			*/
	lArray = _MGL_buf;
	rArray = (fxpointc_t*)((uchar*)_MGL_buf + 4 * sizeof(fxpointc_t));

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

	cShadePoly(3,lArray,rArray,xOffset,yOffset);

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

	cShadePoly(3,lArray,rArray,xOffset,yOffset);
}

#endif	/* MGL_FIX3D */
