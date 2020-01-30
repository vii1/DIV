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
* Description:	Fast triangle filling routines. Polygon clipping is
*				done on a scissoring basis, where each scan line is
*				clipped individually, rather than using a routine such as
*				the Sutherland Hodgeman polygon clipping algorithm. This
*				approach has been taken since we don't expect to clip
*				polygons all that often, and those that are clipped will
*				only require a small amount of clipping. It also allows
*				us to easily take into account proper clipping of shaded
*				and zbuffered polygons. We do however do a quick rejection
*				test to skip the clip step altogether if the polygon can
*				be drawn unclipped.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_FIX3D

/*------------------------- Implementation --------------------------------*/

#if	defined(__16BIT__) || defined(NO_ASSEMBLER) || !defined(__INTEL__)

/****************************************************************************
DESCRIPTION:
Scan converts a filled, flat shaded triangle.

HEADER:
mgraph.h

PARAMETERS:
v1	- Vertex in the triangle
v2	- Vertex in the triangle
v3	- Vertex in the triangle
xOffset	- Offset of x coordinates
yOffset	- Offset of y coordinates

REMARKS:
All vertices are offset by (xOffset,yOffset).
****************************************************************************/
/* {secret} */
void ASMAPI MGL_tri(
	fxpoint_t *v1,
	fxpoint_t *v2,
	fxpoint_t *v3,
	fix32_t xOffset,
	fix32_t yOffset)
{
	DC.doClip = FALSE;
	if (DC.a.clip && (DC.doClip = _MGL_preClipTri(v1,v2,v3,
			xOffset,yOffset)) == -1)
		return;

	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		DC.r.ditherTri(v1,v2,v3,
			xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top);
	else
		DC.r.tri(v1,v2,v3,
			xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top);
}

/****************************************************************************
DESCRIPTION:
Rasterizes a filled, smooth color index shaded triangle.

HEADER:
mgraph.h

PARAMETERS:
v1	- Vertices in the triangle
v2	- Vertices in the triangle
v3	- Vertices in the triangle
xOffset	- Offset of all vertices in the x direction
yOffset	- Offset of all vertices in the y direction


REMARKS:
All vertices are offset by (xOffset,yOffset), which can be used to achieve fast
viewport transformations on the vertex data. This routine takes the x and y
coordinates in 16.16 fixed point format, and the color coordinates in 8.16
fixed point format. See the fix32_t and fxcolor_t types for more information
on these formats. The color values for the vertices in the triangle are
linearly interpolated across the face of the triangle between each of the
triangle vertices, producing a smooth shaded effect.

As with all MGL triangle rasterizing routines, this routine does not rasterize
the pixels down the right hand side or the bottom edges of the triangle. This
ensures that pixels along shared edges of triangles are not rasterized twice,
which can cause annoying pixel flashes in animation code. Note also that the
edges in the triangle will always be rasterized from top to bottom, to ensure
that all shared edges will actually generate the same set of vertices,
eliminating the possibility of pixel dropouts between shared edges in triangles.

SEE ALSO:
MGL_cQuad
****************************************************************************/
/* {secret} */
void ASMAPI MGL_cTri(
	fxpointc_t *v1,
	fxpointc_t *v2,
	fxpointc_t *v3,
	fix32_t xOffset,
	fix32_t yOffset)
{
	DC.doClip = FALSE;
	if (DC.a.clip && (DC.doClip = _MGL_preClipTri(&v1->p,&v2->p,&v3->p,
			xOffset,yOffset)) == -1)
		return;
	DC.r.cTri(v1,v2,v3,xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top);
}

/****************************************************************************
DESCRIPTION:
Scan converts a filled, smooth shaded RGB triangle.

HEADER:
mgraph.h

PARAMETERS:
v1	- Vertices in the triangle
v2	- Vertices in the triangle
v3	- Vertices in the triangle
xOffset	- Offset of x coordinates
yOffset	- Offset of y coordinates

REMARKS:
All vertices are offset by (xOffset,yOffset).
****************************************************************************/
/* {secret} */
void ASMAPI MGL_rgbTri(
	fxpointrgb_t *v1,
	fxpointrgb_t *v2,
	fxpointrgb_t *v3,
	fix32_t xOffset,
	fix32_t yOffset)
{
	DC.doClip = FALSE;
	if (DC.a.clip && (DC.doClip = _MGL_preClipTri(&v1->p,&v2->p,&v3->p,
			xOffset,yOffset)) == -1)
		return;
	DC.r.rgbTri(v1,v2,v3,xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top);
}

/****************************************************************************
DESCRIPTION:
Scan converts a filled, flat shaded zbuffered triangle.

HEADER:
mgraph.h

PARAMETERS:
v1	- Vertex in the triangle
v2	- Vertex in the triangle
v3	- Vertex in the triangle
xOffset	- Offset of x coordinates
yOffset	- Offset of y coordinates
zOffset	- Offset of z coordinates

REMARKS:
All vertices are offset by (xOffset,yOffset,zOffset).
****************************************************************************/
/* {secret} */
void ASMAPI MGL_zTri(
	fxpointz_t *v1,
	fxpointz_t *v2,
	fxpointz_t *v3,
	fix32_t xOffset,
	fix32_t yOffset,
	zfix32_t zOffset)
{
	DC.doClip = FALSE;
	if (DC.a.clip && (DC.doClip = _MGL_preClipTri(&v1->p,&v2->p,&v3->p,
			xOffset,yOffset)) == -1)
		return;

	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		DC.r.z.zDitherTri(v1,v2,v3,
			xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top,
			zOffset);
	else
		DC.r.z.zTri(v1,v2,v3,
			xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top,
			zOffset);
}


/****************************************************************************
DESCRIPTION:
Scan converts a filled, smooth shaded zbuffered color index
triangle. All vertices are offset by (xOffset,yOffset,zOffset).

HEADER:

PARAMETERS:
v1	- Vertices in the triangle
v2	- Vertices in the triangle
v3	- Vertices in the triangle
xOffset	- Offset of x coordinates
yOffset	- Offset of y coordinates
zOffset	- Offset of z coordinates

REMARKS:
All vertices are offset by (xOffset,yOffset,zOffset).  
****************************************************************************/
/* {secret} */
void ASMAPI MGL_czTri(
	fxpointcz_t *v1,
	fxpointcz_t *v2,
	fxpointcz_t *v3,
	fix32_t xOffset,
	fix32_t yOffset,
	zfix32_t zOffset)
{
	DC.doClip = FALSE;
	if (DC.a.clip && (DC.doClip = _MGL_preClipTri(&v1->p,&v2->p,&v3->p,
			xOffset,yOffset)) == -1)
		return;
	DC.r.z.czTri(v1,v2,v3,
		xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top,zOffset);
}

/****************************************************************************
DESCRIPTION:
Scan converts a filled, smooth shaded zbuffered RGB triangle.
All vertices are offset by (xOffset,yOffset).

HEADER:
mgraph.h

PARAMETERS:
v1	- Vertices in the triangle
v2	- Vertices in the triangle
v3	- Vertices in the triangle
xOffset	- Offset of x coordinates
yOffset	- Offset of y coordinates
zOffset	- Offset of z coordinates

REMARKS:
All vertices are offset by (xOffset,yOffset,zOffset).  
****************************************************************************/
/* {secret} */
void ASMAPI MGL_rgbzTri(
	fxpointrgbz_t *v1,
	fxpointrgbz_t *v2,
	fxpointrgbz_t *v3,
	fix32_t xOffset,
	fix32_t yOffset,
	zfix32_t zOffset)
{
	DC.doClip = FALSE;
	if (DC.a.clip && (DC.doClip = _MGL_preClipTri(&v1->p,&v2->p,&v3->p,
			xOffset,yOffset)) == -1)
		return;
	DC.r.z.rgbzTri(v1,v2,v3,
		xOffset + DC.viewPortFX.left,yOffset + DC.viewPortFX.top,zOffset);
}

/* {secret} */
int ASMAPI _MGL_preClipTri(fxpoint_t *v1,fxpoint_t *v2,fxpoint_t *v3,
	fix32_t xOffset,fix32_t yOffset)
/****************************************************************************
*
* Function:		_MGL_preClipTri
* Parameters:	v1,v2,v3	- Three vertices in the triangle
* 				xOffset		- X coordinate offset for polygon
*				yOffset		- Y coordinate offset for polygon
* Returns:      0 for draw no clip, 1 for clip and draw, -1 for trival reject
*
* Description: 	Determine if the triangle to be scan converted must be
*				clipped, trivially rejected or trivially accepted.
*
****************************************************************************/
{
	fix32_t		left,top,right,bottom;
	fix32_t		cleft,ctop,cright,cbottom;

	left = right = v1->x;
	top = bottom = v1->y;
	if (v2->x < left) left = v2->x;
	else if (v2->x > right)	right = v2->x;
	if (v2->y < top) top = v2->y;
	else if (v2->y > bottom) bottom = v2->y;
	if (v3->x < left) left = v3->x;
	else if (v3->x > right)	right = v3->x;
	if (v3->y < top) top = v3->y;
	else if (v3->y > bottom) bottom = v3->y;

	left += xOffset;
	right += xOffset;
	cleft = MAX(left,DC.clipRectFX.left);
	cright = MIN(right,DC.clipRectFX.right);
	if (cleft >= cright)
		return -1;
	top += yOffset;
	bottom += yOffset;
	ctop = MAX(top,DC.clipRectFX.top);
	cbottom = MIN(bottom,DC.clipRectFX.bottom);
	if (ctop >= cbottom)
		return -1;
	if (cleft == left && ctop == top && cright == right && cbottom == bottom)
		return 0;
	return 1;
}

#endif

#endif	/* MGL_FIX3D */
