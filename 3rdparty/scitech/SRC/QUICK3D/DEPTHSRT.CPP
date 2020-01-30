/****************************************************************************
*
*			  Quick3D - A 3D C++ rendering pipeline for the MGL
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
* Language:		C++ 3.0
* Environment:	any
*
* Description:  3d polygon depth sorting implementation.
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

int DSObjectList::depthCmp(DSObject *o1,DSObject *o2)
/****************************************************************************
*
* Function:		DSObjectList::depthCmp
* Parameters:	o1	- First object to compare
*				o2	- Second object to compare
*
* Description:	Sorting function used by the List mergesort routine to
*				sort the object list. We simply compare the average Z
*				values of the objects to get the sorted order (impossible
*				to solve correctly otherwise).
*
****************************************************************************/
{
	if (o1->zAvg < o2->zAvg)
		return -1;
	return 1;
}

void DSAggregate::depthSort()
/****************************************************************************
*
* Function:		DSAggregate::depthSort
*
* Description:	Depths sorts all of the objects in the aggregate. We do
*				this by first getting each child object to sort itself,
*				and then depth sorting all of the child objects.
*
****************************************************************************/
{
	if (objList.isEmpty())
		return;

	// Depth sort all child objects and compute Z extents
	DSObject *o = objList.peekHead();
	o->depthSort();
	zfix32_t z_min = o->zAvg;
	zfix32_t z_max = o->zAvg;

	for (o = objList.next(o); o; o = objList.next(o)) {
		o->depthSort();
		if (o->zAvg < z_min) z_min = o->zAvg;
		if (o->zAvg > z_max) z_max = o->zAvg;
		}
	zAvg = (z_min + z_max)/2;

	// Now depth sort all of the objects in our list
	objList.depthSort();
}

void DSConvexAggregate::depthSort()
/****************************************************************************
*
* Function:		DSConvexAggregate::depthSort
*
* Description:	A NOP of depth sorting of a Convex Aggregate object. We
*				assume only primitive objects are stored in this aggregate
*				(ie: no sub-objects) so we simply compute the objects
*				extents and we are ready to render.
*
****************************************************************************/
{
	if (objList.isEmpty())
		return;

	// Compute the averag Z extent for the aggregate
	DSObject *o = objList.peekHead();
	zfix32_t z_min = o->zAvg;
	zfix32_t z_max = o->zAvg;

	for (o = objList.next(o); o; o = objList.next(o)) {
		if (o->zAvg < z_min) z_min = o->zAvg;
		if (o->zAvg > z_max) z_max = o->zAvg;
		}
	zAvg = (z_min + z_max)/2;
}

void DSAggregate::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSAggregate::render
*
* Description:	Renders all of the child objects in the scene in depth
*				first order.
*
****************************************************************************/
{
	for (DSObject *o = objList.peekHead(); o; o = objList.next(o))
		o->render(dc);
}

DSFlatTriangle::DSFlatTriangle(const fxpointz_t& vz1,const fxpointz_t& vz2,
	const fxpointz_t& vz3,color_t color)
	: color(color), v1(vz1.p), v2(vz2.p), v3(vz3.p)
/****************************************************************************
*
* Function:		DSFlatTriangle::DSFlatTriangle
* Parameters:	v1,v2,v3	- Vertices in the triangle
*				color		- Color of the triangle
*
* Description:	Constructor for the DSFlatTriangle class that copies the
*				triangle vertex data and determines the triangle X,Y and
*				Z extents.
*
****************************************************************************/
{
	zfix32_t	z_min,z_max;

	// Find the min/max Z and copy coordinate values
	z_min = z_max = vz1.z;
	if (vz2.z < z_min) z_min = vz2.z;
	else if (vz2.z > z_max) z_max = vz2.z;
	if (vz3.z < z_min) z_min = vz3.z;
	else if (vz3.z > z_max) z_max = vz3.z;
	zAvg = (z_min + z_max)/2;
}

void DSFlatTriangle::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSFlatTriangle::render
* Parameters:	dc	- Device context to render into
*
****************************************************************************/
{
	dc.setColor(color);
	dc.tri(&v1,&v2,&v3,0,0);
}

DSOutlineFlatTriangle::DSOutlineFlatTriangle(const fxpointz_t& v1,
	const fxpointz_t& v2,const fxpointz_t& v3,color_t color,
	color_t outlineColor)
	: DSFlatTriangle(v1,v2,v3,color), outlineColor(outlineColor)
/****************************************************************************
*
* Function:		DSFlatTriangle::DSFlatTriangle
* Parameters:	v1,v2,v3		- Vertices in the triangle
*				color			- Color of the triangle
*				outlineColor	- Color for triangle outline
*
****************************************************************************/
{
}

void DSOutlineFlatTriangle::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSOutlineFlatTriangle::render
* Parameters:	dc	- Device context to render into
*
* Description:	Renders an outlined flat shaded polygon to the specified
*				device context.
*
****************************************************************************/
{
	dc.setColor(color);
	dc.tri(&v1,&v2,&v3,0,0);
	dc.setColor(outlineColor);
	dc.line(v1,v2);
	dc.line(v2,v3);
	dc.line(v3,v1);
}

DSFlatQuad::DSFlatQuad(const fxpointz_t& vz1,const fxpointz_t& vz2,
	const fxpointz_t& vz3,const fxpointz_t& vz4,color_t color)
	: color(color), v1(vz1.p), v2(vz2.p), v3(vz3.p), v4(vz4.p)
/****************************************************************************
*
* Function:		DSFlatQuad::DSFlatQuad
* Parameters:	v1,v2,v3,v4	- Vertices in the quad
*				color		- Color of the quad
*
* Description:	Constructor for the DSFlatQuad class that copies the
*				quad vertex data and determines the quad X,Y and
*				Z extents.
*
****************************************************************************/
{
	zfix32_t	z_min,z_max;

	// Find the min/max Z and copy coordinate values
	z_min = z_max = vz1.z;
	if (vz2.z < z_min) z_min = vz2.z;
	else if (vz2.z > z_max) z_max = vz2.z;
	if (vz3.z < z_min) z_min = vz3.z;
	else if (vz3.z > z_max) z_max = vz3.z;
	if (vz4.z < z_min) z_min = vz4.z;
	else if (vz4.z > z_max) z_max = vz4.z;
	zAvg = (z_min + z_max)/2;
}

void DSFlatQuad::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSFlatQuad::render
* Parameters:	dc	- Device context to render into
*
****************************************************************************/
{
	dc.setColor(color);
	dc.quad(&v1,&v2,&v3,&v4,0,0);
}

DSOutlineFlatQuad::DSOutlineFlatQuad(const fxpointz_t& v1,
	const fxpointz_t& v2,const fxpointz_t& v3,const fxpointz_t& v4,
	color_t color,color_t outlineColor)
	: DSFlatQuad(v1,v2,v3,v4,color), outlineColor(outlineColor)
/****************************************************************************
*
* Function:		DSFlatQuad::DSFlatQuad
* Parameters:	v1,v2,v3,v4		- Vertices in the quad
*				color			- Color of the quad
*				outlineColor	- Color for quad outline
*
****************************************************************************/
{
}

void DSOutlineFlatQuad::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSOutlineFlatQuad::render
* Parameters:	dc	- Device context to render into
*
* Description:	Renders an outlined flat shaded polygon to the specified
*				device context.
*
****************************************************************************/
{
	dc.setColor(color);
	dc.quad(&v1,&v2,&v3,&v4,0,0);
	dc.setColor(outlineColor);
	dc.line(v1,v2);
	dc.line(v2,v3);
	dc.line(v3,v4);
	dc.line(v4,v1);
}

DSShadedCITriangle::DSShadedCITriangle(const fxpointcz_t& vz1,
	const fxpointcz_t& vz2,const fxpointcz_t& vz3)
	: v1(TOC2(vz1)), v2(TOC2(vz2)), v3(TOC2(vz3))
/****************************************************************************
*
* Function:		DSShadedCITriangle::DSShadedCITriangle
* Parameters:	v1,v2,v3	- Vertices in the triangle
*
* Description:	Constructor for the DSShadedCITriangle class that copies the
*				triangle vertex data and determines the triangle X,Y and
*				Z extents.
*
****************************************************************************/
{
	zfix32_t	z_min,z_max;

	// Find the min/max Z and copy coordinate values
	z_min = z_max = vz1.z;
	if (vz2.z < z_min) z_min = vz2.z;
	else if (vz2.z > z_max) z_max = vz2.z;
	if (vz3.z < z_min) z_min = vz3.z;
	else if (vz3.z > z_max) z_max = vz3.z;
	zAvg = (z_min + z_max)/2;
}

void DSShadedCITriangle::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSShadedCITriangle::render
* Parameters:	dc	- Device context to render into
*
****************************************************************************/
{
	dc.cTri(&v1,&v2,&v3,0,0);
}

DSOutlineShadedCITriangle::DSOutlineShadedCITriangle(const fxpointcz_t& v1,
	const fxpointcz_t& v2,const fxpointcz_t& v3,color_t outlineColor)
	: DSShadedCITriangle(v1,v2,v3), outlineColor(outlineColor)
/****************************************************************************
*
* Function:		DSShadedCITriangle::DSShadedCITriangle
* Parameters:	v1,v2,v3		- Vertices in the triangle
*				outlineColor	- Color for triangle outline
*
****************************************************************************/
{
}

void DSOutlineShadedCITriangle::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSOutlineShadedCITriangle::render
* Parameters:	dc	- Device context to render into
*
* Description:	Renders an outlined flat shaded polygon to the specified
*				device context.
*
****************************************************************************/
{
	dc.cTri(&v1,&v2,&v3,0,0);
	dc.setColor(outlineColor);
	dc.line(v1.p,v2.p);
	dc.line(v2.p,v3.p);
	dc.line(v3.p,v1.p);
}

DSShadedCIQuad::DSShadedCIQuad(const fxpointcz_t& vz1,const fxpointcz_t& vz2,
	const fxpointcz_t& vz3,const fxpointcz_t& vz4)
	: v1(TOC2(vz1)), v2(TOC2(vz2)), v3(TOC2(vz3)), v4(TOC2(vz4))
/****************************************************************************
*
* Function:		DSShadedCIQuad::DSShadedCIQuad
* Parameters:	v1,v2,v3,v4	- Vertices in the quad
*
* Description:	Constructor for the DSShadedCIQuad class that copies the
*				quad vertex data and determines the quad X,Y and
*				Z extents.
*
****************************************************************************/
{
	zfix32_t	z_min,z_max;

	// Find the min/max Z and copy coordinate values
	z_min = z_max = vz1.z;
	if (vz2.z < z_min) z_min = vz2.z;
	else if (vz2.z > z_max) z_max = vz2.z;
	if (vz3.z < z_min) z_min = vz3.z;
	else if (vz3.z > z_max) z_max = vz3.z;
	if (vz4.z < z_min) z_min = vz4.z;
	else if (vz4.z > z_max) z_max = vz4.z;
	zAvg = (z_min + z_max)/2;
}

void DSShadedCIQuad::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSShadedCIQuad::render
* Parameters:	dc	- Device context to render into
*
****************************************************************************/
{
	dc.cQuad(&v1,&v2,&v3,&v4,0,0);
}

DSOutlineShadedCIQuad::DSOutlineShadedCIQuad(const fxpointcz_t& v1,
	const fxpointcz_t& v2,const fxpointcz_t& v3,const fxpointcz_t& v4,
	color_t outlineColor)
	: DSShadedCIQuad(v1,v2,v3,v4), outlineColor(outlineColor)
/****************************************************************************
*
* Function:		DSShadedCIQuad::DSShadedCIQuad
* Parameters:	v1,v2,v3,v4		- Vertices in the quad
*				outlineColor	- Color for quad outline
*
****************************************************************************/
{
}

void DSOutlineShadedCIQuad::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSOutlineShadedCIQuad::render
* Parameters:	dc	- Device context to render into
*
* Description:	Renders an outlined flat shaded polygon to the specified
*				device context.
*
****************************************************************************/
{
	dc.cQuad(&v1,&v2,&v3,&v4,0,0);
	dc.setColor(outlineColor);
	dc.line(v1.p,v2.p);
	dc.line(v2.p,v3.p);
	dc.line(v3.p,v4.p);
	dc.line(v4.p,v1.p);
}

DSShadedRGBTriangle::DSShadedRGBTriangle(const fxpointrgbz_t& vz1,
	const fxpointrgbz_t& vz2,const fxpointrgbz_t& vz3)
	: v1(TORGB(vz1)), v2(TORGB(vz2)), v3(TORGB(vz3))
/****************************************************************************
*
* Function:		DSShadedRGBTriangle::DSShadedRGBTriangle
* Parameters:	v1,v2,v3	- Vertices in the triangle
*
* Description:	Constructor for the DSShadedRGBTriangle class that copies the
*				triangle vertex data and determines the triangle X,Y and
*				Z extents.
*
****************************************************************************/
{
	zfix32_t	z_min,z_max;

	// Find the min/max Z and copy coordinate values
	z_min = z_max = vz1.z;
	if (vz2.z < z_min) z_min = vz2.z;
	else if (vz2.z > z_max) z_max = vz2.z;
	if (vz3.z < z_min) z_min = vz3.z;
	else if (vz3.z > z_max) z_max = vz3.z;
	zAvg = (z_min + z_max)/2;
}

void DSShadedRGBTriangle::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSShadedRGBTriangle::render
* Parameters:	dc	- Device context to render into
*
****************************************************************************/
{
	dc.rgbTri(&v1,&v2,&v3,0,0);
}

DSOutlineShadedRGBTriangle::DSOutlineShadedRGBTriangle(const fxpointrgbz_t& v1,
	const fxpointrgbz_t& v2,const fxpointrgbz_t& v3,color_t outlineColor)
	: DSShadedRGBTriangle(v1,v2,v3), outlineColor(outlineColor)
/****************************************************************************
*
* Function:		DSShadedRGBTriangle::DSShadedRGBTriangle
* Parameters:	v1,v2,v3		- Vertices in the triangle
*				outlineColor	- Color for triangle outline
*
****************************************************************************/
{
}

void DSOutlineShadedRGBTriangle::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSOutlineShadedRGBTriangle::render
* Parameters:	dc	- Device context to render into
*
* Description:	Renders an outlined flat shaded polygon to the specified
*				device context.
*
****************************************************************************/
{
	dc.rgbTri(&v1,&v2,&v3,0,0);
	dc.setColor(outlineColor);
	dc.line(v1.p,v2.p);
	dc.line(v2.p,v3.p);
	dc.line(v3.p,v1.p);
}

DSShadedRGBQuad::DSShadedRGBQuad(const fxpointrgbz_t& vz1,const fxpointrgbz_t& vz2,
	const fxpointrgbz_t& vz3,const fxpointrgbz_t& vz4)
	: v1(TORGB(vz1)), v2(TORGB(vz2)), v3(TORGB(vz3)), v4(TORGB(vz4))
/****************************************************************************
*
* Function:		DSShadedRGBQuad::DSShadedRGBQuad
* Parameters:	v1,v2,v3,v4	- Vertices in the quad
*
* Description:	Constructor for the DSShadedRGBQuad class that copies the
*				quad vertex data and determines the quad X,Y and
*				Z extents.
*
****************************************************************************/
{
	zfix32_t	z_min,z_max;

	// Find the min/max Z and copy coordinate values
	z_min = z_max = vz1.z;
	if (vz2.z < z_min) z_min = vz2.z;
	else if (vz2.z > z_max) z_max = vz2.z;
	if (vz3.z < z_min) z_min = vz3.z;
	else if (vz3.z > z_max) z_max = vz3.z;
	if (vz4.z < z_min) z_min = vz4.z;
	else if (vz4.z > z_max) z_max = vz4.z;
	zAvg = (z_min + z_max)/2;
}

void DSShadedRGBQuad::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSShadedRGBQuad::render
* Parameters:	dc	- Device context to render into
*
****************************************************************************/
{
	dc.rgbQuad(&v1,&v2,&v3,&v4,0,0);
}

DSOutlineShadedRGBQuad::DSOutlineShadedRGBQuad(const fxpointrgbz_t& v1,
	const fxpointrgbz_t& v2,const fxpointrgbz_t& v3,const fxpointrgbz_t& v4,
	color_t outlineColor)
	: DSShadedRGBQuad(v1,v2,v3,v4), outlineColor(outlineColor)
/****************************************************************************
*
* Function:		DSShadedRGBQuad::DSShadedRGBQuad
* Parameters:	v1,v2,v3,v4		- Vertices in the quad
*				outlineColor	- Color for quad outline
*
****************************************************************************/
{
}

void DSOutlineShadedRGBQuad::render(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		DSOutlineShadedRGBQuad::render
* Parameters:	dc	- Device context to render into
*
* Description:	Renders an outlined flat shaded polygon to the specified
*				device context.
*
****************************************************************************/
{
	dc.rgbQuad(&v1,&v2,&v3,&v4,0,0);
	dc.setColor(outlineColor);
	dc.line(v1.p,v2.p);
	dc.line(v2.p,v3.p);
	dc.line(v3.p,v4.p);
	dc.line(v4.p,v1.p);
}
