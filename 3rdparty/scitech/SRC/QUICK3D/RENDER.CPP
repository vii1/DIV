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
* Description:  3d polygon rendering pipeline routines. The color of the
*				polygon is determined depending on the current shading type.
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick3d::beginPoly()
/****************************************************************************
*
* Function:		Quick3d::beginPoly
*
* Description:	Begins rendering of a polygon with the specified maximum
*				number of vertices. A polygon is a leaf element in the
*				DSObject hierarchy for depth sorting.
*
****************************************************************************/
{
	meshMode = PolyMode;
	count = 0;
}

void Quick3d::beginTStrip()
/****************************************************************************
*
* Function:		Quick3d::beginTStrip
*
* Description:	Begins rendering of a triangluar strip, with the specified
*				maximum number of triangles. The maximum number of triangles
*				in a strip is always numVerts - 2.
*
****************************************************************************/
{
	save = 0;
	meshMode = TStripMode;
}

void Quick3d::beginTFan()
/****************************************************************************
*
* Function:		Quick3d::beginTFan
*
* Description:	Begins rendering of a triangluar fan, with the specified
*				maximum number of triangles. The maximum number of triangles
*				in a fan is always numVerts - 2.
*
****************************************************************************/
{
	save = 0;
	meshMode = TFanMode;
}

void Quick3d::beginQStrip()
/****************************************************************************
*
* Function:		Quick3d::beginQStrip
*
* Description:	Begins rendering of a quadrilateral strip, with the specified
*				maximum number of quads. The maximum number of quads
*				in a strip is always (numVerts - 2) / 2.
*
****************************************************************************/
{
	save = 0;
	meshMode = QStripMode;
}

void Quick3d::beginScene()
/****************************************************************************
*
* Function:		Quick3d::beginScene
*
* Description:	Begins the rendering of a new scene, allocating the first
*				level DSAggregate object to hold all the objects in the
*				scene.
*
*				Note that for efficient memory management for the DSObject
*				hierarchy, we use a fast MemStack as our memory pool and
*				delete the entire chunk of memory used for the previous
*				scene in a single swoop.
*
****************************************************************************/
{
	if (hsrMode == DepthSortHSR) {
		// Free all allocated storage for the previous scene, and mark the
		// current state to begin memory allocation from local MemStack again.
		delete memStart;
		memStart = new TCMemMarker(mem);

		// Begin the scene by creating the first aggregate object
		cntObject = NULL;
		beginObject();
		}
}

void Quick3d::beginObject()
/****************************************************************************
*
* Function:		Quick3d::beginObject
*
* Description:	Creates a new aggregate object in the scene ready to accept
*				new child objects. It is beneficial to break the scene into
*				localised objects, as each local objects is depth sorted
*				internally which significantly speeds up the overall depth
*				sorting process.
*
****************************************************************************/
{
	if (hsrMode == DepthSortHSR) {
		// Save current aggregate object on stack
		objStack.push(cntObject);

		// Create a new aggregate object for the specified number of objects
		cntObject = new(mem) DSAggregate();
		}
}

void Quick3d::beginConvexObject()
/****************************************************************************
*
* Function:		Quick3d::beginConvexObject
*
* Description:	Creates a new aggregate object in the scene ready to accept
*				new child objects. The entire object is known to be convex
*				in nature and can be correctly rendered with hidden surface
*				removal by backface culling only. Hence a DSConvexAggregate
*				object does a NOP when requested to depth sort itself, and
*				we turn on backface culling during the creation of this
*				object.
*
*				Although you can create sub-objects of a ConvexObject, it
*				is only advisable to add polygons to a ConvexObject.
*
****************************************************************************/
{
	if (hsrMode == DepthSortHSR) {
		// Save current aggregate object on stack
		objStack.push(cntObject);

		// Create a new aggregate object for the specified number of objects
		cntObject = new(mem) DSConvexAggregate();

		prevCulling = culling;
		culling = BackfaceCull;
		}
}

void Quick3d::vertex(const FXPoint3d& v)
/****************************************************************************
*
* Function:		Quick3d::vertex
* Parameters:	v	- Vertex to send to mesh drawing routines.
*
* Description:	Registers the vertex 'v' as the next vertex in the mesh
*				structure or polygon structure.
*
****************************************************************************/
{
	validateTotalMapping();
	if (meshMode == PolyMode) {
		// Save the transformed vertex for the polygon for later.
		if (count > maxVertices)
			MGL_fatalError("Too many vertices in polygon");
		storeVertex(count++,v);
		}
	else if (meshMode == QStripMode) {
		// Draws a quadrilateral strip structure

		if (save < 3) {
			// For the first three calls to this routine, we simply save the
			// transformed vertex coordinates in a temporary array and leave.
			if (save == 2)
				storeVertex(3,v);
			else storeVertex(save,v);
			}
		else {
			// For all other calls to this routine, we transform the new
			// vertex to screen space, draw the resulting quadrilateral on
			// the screen (if one is currenly defined), and save the new
			// coordinate in the appropriate spot.
			if (save % 2) {
				storeVertex(2,v);
				count = 4;
				endPoly();					// Draw the polygon

				// Save the two newest points over the two oldest
				recycleVertex(0,3);
				recycleVertex(1,2);
				}
			else {
				// This is the third point for a new quadrilateral, so save
				// it in the last register (the next point goes before it
				// in the polygon definition).
				storeVertex(3,v);
				}
			}
		save++;
		}
	else {
		// Draws a triangular fan/strip structure

		if (save < 2) {
			// For the first two calls to this routine, we simply save the
			// transformed vertex coordinates in a temporary array and leave.
			storeVertex(save,v);
			}
		else {
			// For all other calls to this routine, we transform the new
			// vertex to screen space, draw the resulting triangle on the
			// screen, and save the new coordinate in the appropriate spot.
			storeVertex(2,v);
			count = 3;
			endPoly();					// Draw the polygon

			// Save the new point so that the resulting polygon will always
			// be defined in clockwise order.
			if (meshMode == TFanMode)
				recycleVertex(1,2);
			else recycleVertex(save % 2,2);
			}
		save++;
		}
}

void Quick3d::normal(const FXVec3d& n)
/****************************************************************************
*
* Function:		Quick3d::normal
* Parameters:	n	- Normal to bind.
*
* Description:	Normal to bind to the next vertex specified or the next
*				polygon to be specified depending on the type of
*				shading being performed. This normal will be transformed
*				by the current transformation matrix into view space,
*				but it will not be put through the perspective
*				transformation.
*
****************************************************************************/
{
	modelView.map(cntNormal,n);			// Map the normal to world coords
	cntNormal.normalise();				// Normalise to unit normal
}

void Quick3d::renderPolyNoHSR(ibool backfacing)
/****************************************************************************
*
* Function:		Quick3d::renderPolyNoHSR
*
* Description:	Renders the current polygon without any hidden surface
*				removal.
*
****************************************************************************/
{
	if ((shading & SmoothShading)
			&& (!backfacing || culling != BackfacePaint)) {
		if (count == 3) {
			if (rgbFlag) {
				rgbTri(&TORGB(pipe[0].win),&TORGB(pipe[1].win),
					&TORGB(pipe[2].win),0,0);
				}
			else {
				cTri(&TOC(pipe[0].win),&TOC(pipe[1].win),
					&TOC(pipe[2].win),0,0);
				}
			}
		else if (count == 4) {
			if (rgbFlag) {
				rgbQuad(&TORGB(pipe[0].win),&TORGB(pipe[1].win),
					&TORGB(pipe[2].win),&TORGB(pipe[3].win),0,0);
				}
			else {
				cQuad(&TOC(pipe[0].win),&TOC(pipe[1].win),
					&TOC(pipe[2].win),&TOC(pipe[3].win),0,0);
				}
			}
		else {
			// Break the polygon (assuming it is convex) down into
			// single triangles. This is just a simple niave decomposition
			if (rgbFlag) {
				for (int i = 2; i < count; i++) {
					rgbTri(&TORGB(pipe[0].win),&TORGB(pipe[i-1].win),
						&TORGB(pipe[i].win),0,0);
					}
				}
			else {
				for (int i = 2; i < count; i++) {
					cTri(&TOC(pipe[0].win),&TOC(pipe[i-1].win),
						&TOC(pipe[i].win),0,0);
					}
				}
			}
		}
	else {
		if (count == 3) {
			tri(&pipe[0].win.p,&pipe[1].win.p,
				&pipe[2].win.p,0,0);
			}
		else if (count == 4) {
			quad(&pipe[0].win.p,&pipe[1].win.p,
				&pipe[2].win.p,&pipe[3].win.p,0,0);
			}
		else {
			MGLDevCtx::fillPolygon(count,&pipe[0].win.p,
				sizeof(pipeEntry),0,0);
			}
		}
	if (outline == DrawOutline) {
		setColor(outlineColor);
		pipeEntry *p = pipe,*prev;
		while (--count) {
			prev = p++;
			MGLDevCtx::line(prev->win.p,p->win.p);
			}
		MGLDevCtx::line(p->win.p,pipe[0].win.p);
		}
}

void Quick3d::renderPolyDepthSortHSR(ibool backfacing)
/****************************************************************************
*
* Function:		Quick3d::renderPolyDepthSortHSR
*
* Description:	Renders the current polygon with depth sorting
*
****************************************************************************/
{
	PRECONDITION(cntObject != NULL);

	// Add the new DSObject to the currently active aggregate
	if ((shading & SmoothShading)
			&& (!backfacing || culling != BackfacePaint)) {
		if (outline == DrawOutline) {
			if (count == 3) {
				if (rgbFlag) {
					cntObject->add(new(mem) DSOutlineShadedRGBTriangle(
						TORGBZ(pipe[0].win),TORGBZ(pipe[1].win),
						TORGBZ(pipe[2].win),outlineColor));
					}
				else {
					cntObject->add(new(mem) DSOutlineShadedCITriangle(
						TOCZ(pipe[0].win),TOCZ(pipe[1].win),
						TOCZ(pipe[2].win),outlineColor));
					}
				}
			else if (count == 4) {
				if (rgbFlag) {
					cntObject->add(new(mem) DSOutlineShadedRGBQuad(
						TORGBZ(pipe[0].win),TORGBZ(pipe[1].win),
						TORGBZ(pipe[2].win),TORGBZ(pipe[3].win),outlineColor));
					}
				else {
					cntObject->add(new(mem) DSOutlineShadedCIQuad(
						TOCZ(pipe[0].win),TOCZ(pipe[1].win),
						TOCZ(pipe[2].win),TOCZ(pipe[3].win),outlineColor));
					}
				}
			else {
				// Break the polygon (assuming it is convex) down into
				// single triangles.
				if (rgbFlag) {
					for (int i = 2; i < count; i++) {
						cntObject->add(new(mem) DSOutlineShadedRGBTriangle(
							TORGBZ(pipe[0].win),TORGBZ(pipe[i-1].win),
							TORGBZ(pipe[i].win),outlineColor));
						}
					}
				else {
					for (int i = 2; i < count; i++) {
						cntObject->add(new(mem) DSOutlineShadedCITriangle(
							TOCZ(pipe[0].win),TOCZ(pipe[i-1].win),
							TOCZ(pipe[i].win),outlineColor));
						}
					}
				}
			}
		else {
			if (count == 3) {
				if (rgbFlag) {
					cntObject->add(new(mem) DSShadedRGBTriangle(
						TORGBZ(pipe[0].win),TORGBZ(pipe[1].win),
						TORGBZ(pipe[2].win)));
					}
				else {
					cntObject->add(new(mem) DSShadedCITriangle(
						TOCZ(pipe[0].win),TOCZ(pipe[1].win),
						TOCZ(pipe[2].win)));
					}
				}
			else if (count == 4) {
				if (rgbFlag) {
					cntObject->add(new(mem) DSShadedRGBQuad(
						TORGBZ(pipe[0].win),TORGBZ(pipe[1].win),
						TORGBZ(pipe[2].win),TORGBZ(pipe[3].win)));
					}
				else {
					cntObject->add(new(mem) DSShadedCIQuad(
						TOCZ(pipe[0].win),TOCZ(pipe[1].win),
						TOCZ(pipe[2].win),TOCZ(pipe[3].win)));
					}
				}
			else {
				// Break the polygon (assuming it is convex) down into
				// single triangles.
				if (rgbFlag) {
					for (int i = 2; i < count; i++) {
						cntObject->add(new(mem) DSShadedRGBTriangle(
							TORGBZ(pipe[0].win),TORGBZ(pipe[i-1].win),
							TORGBZ(pipe[i].win)));
						}
					}
				else {
					for (int i = 2; i < count; i++) {
						cntObject->add(new(mem) DSShadedCITriangle(
							TOCZ(pipe[0].win),TOCZ(pipe[i-1].win),
							TOCZ(pipe[i].win)));
						}
					}
				}
			}
		}
	else {
		color_t color = getColor();

		if (outline == DrawOutline) {
			if (count == 3) {
				cntObject->add(new(mem) DSOutlineFlatTriangle(
					TOZ(pipe[0].win),TOZ(pipe[1].win),
					TOZ(pipe[2].win),color,outlineColor));
				}
			else if (count == 4) {
				cntObject->add(new(mem) DSOutlineFlatQuad(
					TOZ(pipe[0].win),TOZ(pipe[1].win),
					TOZ(pipe[2].win),TOZ(pipe[3].win),color,outlineColor));
				}
			else {
				// Break the polygon (assuming it is convex) down into
				// single triangles.
				for (int i = 2; i < count; i++) {
					cntObject->add(new(mem) DSOutlineFlatTriangle(
						TOZ(pipe[0].win),TOZ(pipe[i-1].win),
						TOZ(pipe[i].win),color,outlineColor));
					}
				}
			}
		else {
			if (count == 3) {
				cntObject->add(new(mem) DSFlatTriangle(
					TOZ(pipe[0].win),TOZ(pipe[1].win),
					TOZ(pipe[2].win),color));
				}
			else if (count == 4) {
				cntObject->add(new(mem) DSFlatQuad(
					TOZ(pipe[0].win),TOZ(pipe[1].win),
					TOZ(pipe[2].win),TOZ(pipe[3].win),color));
				}
			else {
				// Break the polygon (assuming it is convex) down into
				// single triangles.
				for (int i = 2; i < count; i++) {
					cntObject->add(new(mem) DSFlatTriangle(
						TOZ(pipe[0].win),TOZ(pipe[i-1].win),
						TOZ(pipe[i].win),color));
					}
				}
			}
		}
}

void Quick3d::renderPolyZBufHSR(ibool backfacing,zfix32_t zofs)
/****************************************************************************
*
* Function:		Quick3d::renderPolyZBufHSR
*
* Description:	Renders the current polygon with zbuffering
*
****************************************************************************/
{
	if ((shading & SmoothShading)
			&& (!backfacing || culling != BackfacePaint)) {
		if (count == 3) {
			if (rgbFlag) {
				rgbzTri(&TORGBZ(pipe[0].win),&TORGBZ(pipe[1].win),
					&TORGBZ(pipe[2].win),0,0,zofs);
				}
			else {
				czTri(&TOCZ(pipe[0].win),&TOCZ(pipe[1].win),
					&TOCZ(pipe[2].win),0,0,zofs);
				}
			}
		else if (count == 4) {
			if (rgbFlag) {
				rgbzQuad(&TORGBZ(pipe[0].win),&TORGBZ(pipe[1].win),
					&TORGBZ(pipe[2].win),&TORGBZ(pipe[3].win),0,0,zofs);
				}
			else {
				czQuad(&TOCZ(pipe[0].win),&TOCZ(pipe[1].win),
					&TOCZ(pipe[2].win),&TOCZ(pipe[3].win),0,0,zofs);
				}
			}
		else {
			// Break the polygon (assuming it is convex) down into
			// single triangles. This is just a simple niave decomposition
			if (rgbFlag) {
				for (int i = 2; i < count; i++) {
					rgbzTri(&TORGBZ(pipe[0].win),&TORGBZ(pipe[i-1].win),
						&TORGBZ(pipe[i].win),0,0,zofs);
					}
				}
			else {
				for (int i = 2; i < count; i++) {
					czTri(&TOCZ(pipe[0].win),&TOCZ(pipe[i-1].win),
						&TOCZ(pipe[i].win),0,0,zofs);
					}
				}
			}
		}
	else {
		if (count == 3) {
			zTri(&TOZ(pipe[0].win),&TOZ(pipe[1].win),
				&TOZ(pipe[2].win),0,0,zofs);
			}
		else if (count == 4) {
			zQuad(&TOZ(pipe[0].win),&TOZ(pipe[1].win),
				&TOZ(pipe[2].win),&TOZ(pipe[3].win),0,0,zofs);
			}
		else {
			// Break the polygon (assuming it is convex) down into
			// single triangles. This is just a simple niave decomposition
			for (int i = 2; i < count; i++) {
				zTri(&TOZ(pipe[0].win),&TOZ(pipe[i-1].win),
					&TOZ(pipe[i].win),0,0,zofs);
				}
			}
		}
	if (outline == DrawOutline) {
		setColor(outlineColor);
		pipeEntry *p = pipe,*prev;

		// In order to avoid problems with zbuffer aliasing causing
		// the outline lines to disappear, we offset the Z
		// coordinates a fraction to ensure they are in front of
		// the polygon.
		while (--count) {
			prev = p++;
			MGLDevCtx::zLine(
				prev->win.p.x,prev->win.p.y,prev->win.z+Z_LINEOFS,
				p->win.p.x,p->win.p.y,p->win.z+Z_OFS);
			}
		MGLDevCtx::zLine(
			p->win.p.x,p->win.p.y,p->win.z+Z_OFS,
			pipe[0].win.p.x,pipe[0].win.p.y,pipe[0].win.z+Z_LINEOFS);
		}
}

void Quick3d::endPoly(void)
/****************************************************************************
*
* Function:		Quick3d::endPoly
*
* Description:	Completes the drawing of a polygon using the Quick3d::vertex
*				routine to define the vertices. If outlining mode is on,
*				we draw the outline of the polygon over the top. If we
*				are depth sorting, this also adds the created DSPolygon
*				object to the current aggregate object list.
*
****************************************************************************/
{
	color_t 	oldColor = getColor();
	ibool		backfacing;
	zfix32_t	zofs = 0;

	// Determine if the polygon is backfacing, and how we should handle it
	// if it is.
	backfacing = false;
	if (fBackface) {
		backfacing = fBackface-1;
		fBackface = 0;
		}
	else backfacing = Q3D_backfacing(pipe[0].win,pipe[1].win,pipe[2].win,orient);

	if (backfacing && culling == BackfaceCull)
		return;

	// For all backfacing polygons, we shift the z coordinates a fraction
	// further away to solve shared edge aliasing problems between
	// front facing and backfacing polygons.
	if (backfacing)
		zofs = -Z_OFS;
	if (backfacing && culling == BackfacePaint)
		setColor(backfaceColor);
	else if (shading & Lighting)
		lightVertices(backfacing);

	// Render the polygon to the display device using the current HSR method.
	// Note that we only support tris and quads with smooth shading.
	switch (hsrMode) {
		case NoHSR:
			renderPolyNoHSR(backfacing);
			break;
		case DepthSortHSR:
			renderPolyDepthSortHSR(backfacing);
			break;
		case ZBufferHSR:
			renderPolyZBufHSR(backfacing,zofs);
			break;
		}
	setColor(oldColor);
}

void Quick3d::endTStrip(void)
/****************************************************************************
*
* Function:		Quick3d::endTStrip
*
* Description:	Completes rendering of a trianglular strip. When we are
*				depth sorting this closes off the DSAggregate object and
*				adds it to its parent object.
*
****************************************************************************/
{
}

void Quick3d::endTFan(void)
/****************************************************************************
*
* Function:		Quick3d::endTFan
*
* Description:	Completes rendering of a trianglular fan. When we are
*				depth sorting this closes off the DSAggregate object and
*				adds it to its parent object.
*
****************************************************************************/
{
}

void Quick3d::endQStrip(void)
/****************************************************************************
*
* Function:		Quick3d::endQStrip
*
* Description:	Completes rendering of a quadrilateral strip. When we are
*				depth sorting this closes off the DSAggregate object and
*				adds it to its parent object.
*
****************************************************************************/
{
}

void Quick3d::endObject()
/****************************************************************************
*
* Function:		Quick3d::endObject
*
* Description:	Closes off the current aggregate objects and adds this
*				objects to it's original parent object.
*
****************************************************************************/
{
	if (hsrMode == DepthSortHSR) {
		// Pop the top aggregate off the aggregate stack, and add the current
		// aggregate to the TOS aggregate and make the TOS aggregate the new
		// current aggregate.
		DSAggregate	*tos = objStack.pop();
		if (tos) {
			tos->add(cntObject);
			cntObject = tos;
			}
		if (prevCulling != -1) {
			culling = prevCulling;
			prevCulling = -1;
			}
		}
}

void Quick3d::endScene()
/****************************************************************************
*
* Function:		Quick3d::endScene
*
* Description:	Completes the building a scene and renders all of the
*				objects currently held in the display list.
*
****************************************************************************/
{
	if (hsrMode == DepthSortHSR) {
		// End the top level object
		endObject();

		// Depth-sort all objects starting with top level aggregate
		cntObject->depthSort();

		// Render all objects in the scene
		cntObject->render(*this);
		}
}

void Quick3d::computeDirtyRect(const FXPoint3d& min,const FXPoint3d& max,
	MGLRect& dirty)
/****************************************************************************
*
* Function:		Quick3d::computeDirtyRect
* Parameters:	min	- Minimum coordinate values for bounding box
*				max	- Maximum coordinate values for bounding box
*
* Description:	Computes the dirty rectangle for a scene by transforming
*				the bounding box for the scene into screen space and
*				determining the smallest screen space rectangle enclosing
*				the updated region.
*
****************************************************************************/
{
	FXPoint3d   temp,p,NDC_min,NDC_max;

#define	FIND_MINMAX(x1,y1,z1)				\
	p.x = x1;	p.y = y1;	p.z = z1;		\
	m.map(temp,p);							\
	NDC_min.x = MIN(NDC_min.x,temp.x);		\
	NDC_min.y = MIN(NDC_min.y,temp.y);		\
	NDC_min.z = MIN(NDC_min.z,temp.z);		\
	NDC_max.x = MAX(NDC_max.x,temp.x);		\
	NDC_max.y = MAX(NDC_max.y,temp.y);		\
	NDC_max.z = MAX(NDC_max.z,temp.z);		\

	// Find the bounding rectangle for the 6 points of the bounding cube
	validateTotalMapping();
	m.map(NDC_min,min);
	NDC_max = NDC_min;
	FIND_MINMAX(min.x,min.y,max.z)
	FIND_MINMAX(min.x,max.y,max.z)
	FIND_MINMAX(min.x,max.y,min.z)
	FIND_MINMAX(max.x,min.y,min.z)
	FIND_MINMAX(max.x,min.y,max.z)
	FIND_MINMAX(max.x,max.y,min.z)
	FIND_MINMAX(max.x,max.y,max.z)

#undef	FIND_MINMAX

	FXVec2dToInt(dirty.topLeft,NDC_min);
	FXVec2dToInt(dirty.botRight,NDC_max);
	dirty.right()++;
	dirty.bottom()++;
}

void Quick3d::forceBackface(ibool set)
/****************************************************************************
*
* Function:		Quick3d::forceBackface
*
* Description:	Forces the the next polygon to be drawn to be interpreted
*				as backfacing. This is used mainly when rendering with
*				the BSP tree algorithm where the backfacing calculations
*				can be wrong because of the automatic splitting process.
*
****************************************************************************/
{
	fBackface = set ? 2 : 1;
}
