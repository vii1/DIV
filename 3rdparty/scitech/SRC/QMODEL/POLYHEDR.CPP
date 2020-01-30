/****************************************************************************
*
*			  	QuickModeler - A Real-Time 3D Modeling System
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
* Description:	Implementation for the QMPolyhedron class
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/polyhedr.hpp"

/*---------------------------- Implementation -----------------------------*/

#define	MAX_SMOOTH	50

QMPolyhedron::QMPolyhedron(ibool isConvex,int backfaceMethod,int orientation)
	: QMPrimitive(), isConvex(isConvex),
		backfaceMethod(backfaceMethod), orientation(orientation)
/****************************************************************************
*
* Function:		QMPolyhedron::QMPolyhedron
* Parameters:	isConvex		- Is the polyhedron convex?
*               backfaceMethod	- Default backface handling method
*               orientation		- Default polygon orientation setting
*
* Description:	Constructor for the QMPolyhedron class that is empty. The
*				subclass of this class should load in the vertices for
*				the object and create either a default flat or smooth
*				object.
*
****************************************************************************/
{
	vertices = NULL;
	normals = NULL;
	polys = NULL;
	edgeData = NULL;
	isSmooth = false;
	shadingType = Quick3d::LitFlatShading;
}

QMPolyhedron::~QMPolyhedron()
/****************************************************************************
*
* Function:		QMPolyhedron::~QMPolyhedron
*
* Description:	Destructor for the QMPolyhedron class. Simply kills off all
*				memory owned by the object.
*
****************************************************************************/
{
	delete [] vertices;
	delete [] normals;
	delete [] polys;
	delete [] edgeData;
}

void QMPolyhedron::transform(const FXForm3d& xform)
/****************************************************************************
*
* Function:		QMPolyhedron::transform
* Parameters:	xform	- Transformation to transform vertices by
*
* Description:	Transforms all of the vertices by the specified
*				transformation. The vertices are transformed in place.
*
****************************************************************************/
{
	int			i;
	FXPoint3d	*v,result;

	for (i = numVertices, v = vertices; i; i--,v++) {
		xform.map(result,*v);
		*v = result;
		}
}

void QMPolyhedron::renderWireframe(Quick3d& dc)
/****************************************************************************
*
* Function:		QMPolyhedron::renderWireframe
* Parameters:   dc	- Device context to render into
*
* Description:	Draws a wireframe representation of the polyhedron.
*
****************************************************************************/
{
	int			i,j,k;
	FXPoint3d	*v;
	short		*e;
	Poly		*p;

	v = vertices;
	e = edgeData;
	for (i = numPolys, p = polys; i; i--,p++) {
		k = p->index;
		dc.moveTo(v[e[k++]]);
		for (j = p->numEdges-1; j; j--)
			dc.lineTo(v[e[k++]]);;
		dc.lineTo(v[e[p->index]]);
		}
}

void QMPolyhedron::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMPolyhedron::renderShaded
* Parameters:   dc	- Device context to render into
*
* Description:	Renders the OFF object as a set of flat shaded polygons. We
*				do this by calling Quick3D. This rendering code is
*				*horribly* inefficient, not to mention that Quick3D itself
*				is pretty inefficient internally.
*
****************************************************************************/
{
	int			i,j,k;
	FXPoint3d	*v;
	FXVec3d		*n;
	short		*e;
	Poly		*p;


	v = vertices;
	e = edgeData;
	dc.setShadingType(this->shadingType);	// GCC bug requires this->
	int oldBackface = dc.setBackfaceMethod(backfaceMethod);
	int oldOrient = dc.setPolygonOrientation(orientation);
	if (isConvex)
		dc.beginConvexObject();
	else dc.beginObject();
	if (isSmooth) {
		for (i = numPolys, p = polys, n = normals; i; i--,p++) {
			k = p->index;
			dc.beginPoly();
			for (j = p->numEdges; j; j--) {
				dc.normal(*n++);
				dc.vertex(v[e[k++]]);
				}
			dc.endPoly();
			}
		}
	else {
		for (i = numPolys, p = polys, n = normals; i; i--,p++,n++) {
			k = p->index;
			dc.normal(*n);
			dc.beginPoly();
			for (j = p->numEdges; j; j--)
				dc.vertex(v[e[k++]]);
			dc.endPoly();
			}
		}
	dc.endObject();
	dc.setBackfaceMethod(oldBackface);
	dc.setPolygonOrientation(oldOrient);
}

void QMPolyhedron::getBounds(FXPoint3d& min,FXPoint3d& max)
/****************************************************************************
*
* Function:		QMPolyhedron::getBounds
* Parameters:	min	- Place to store minimum coordinate
*				max	- Place to store maximum coordinate
*
* Description:	Finds the bounding box for the polyhedron.
*
****************************************************************************/
{
	int			i = numVertices;
	FXPoint3d   *v = vertices;

	min = max = *v++;
	while (--i) {
		min.x = MIN(min.x,v->x);
		min.y = MIN(min.y,v->y);
		min.z = MIN(min.z,v->z);
		max.x = MAX(max.x,v->x);
		max.y = MAX(max.y,v->y);
		max.z = MAX(max.z,v->z);
		v++;
		}
}

ibool QMPolyhedron::makeFlat(QMPolyhedronCallback cb)
/****************************************************************************
*
* Function:		QMPolyhedron::makeFlat
*
* Description:	Makes the polyhedron flat shaded if already smooth shaded.
*				Does nothing if the polyhedron already has flat shaded
*				normals.
*
****************************************************************************/
{
	if (!normals || isSmooth) {
		delete [] normals;
		if ((normals = new FXVec3d[numPolys]) == NULL)
			return false;
		callback = cb;
		buildFlatNormals();
		isSmooth = false;
		}
	return true;
}

ibool QMPolyhedron::makeSmooth(real smoothAngle,QMPolyhedronCallback cb)
/****************************************************************************
*
* Function:		QMPolyhedron::makeFlat
* Parameters:	smoothAngle	- Maximum angle to consider a smooth edge
*
* Description:	Makes the polyhedron smooth shaded if already flat shaded.
*				Does nothing if the polyhedron already has smooth shaded
*				normals.
*
****************************************************************************/
{
	if (!normals || !isSmooth) {
		delete [] normals;
		if ((normals = new FXVec3d[numEdges]) == NULL)
			return false;
		callback = cb;
		buildSmoothNormals(cntSmoothAngle = smoothAngle);
		isSmooth = true;
		}
	return true;
}

void QMPolyhedron::buildFlatNormals()
/****************************************************************************
*
* Function:		QMPolyhedron::buildFlatNormals
* Parameters:	clockwise	- True if polygon vertices are clockwise
*
* Description:	Build all the polygon normals for the object. We simply
*				compute a single face normal for the object from the first
*				three vertices in each polygon.
*
****************************************************************************/
{
	FXPoint3d	*v;
	FXVec3d		*n;
	short		*e;
	Poly		*p;
	int			i,k;

	// Compute the polygon normals for the object
	v = vertices;
	e = edgeData;
	for (i = 0, p = polys, n = normals; i < numPolys; i++,p++,n++) {
		k = p->index;
		QMcomputeNormal(*n,p->numEdges,v,&e[k],orientation);
        callback((i * 100L) / numPolys);
		}
}

int QMPolyhedron::findPolyNormals(FXVec3d *toSmooth,
	FXVec3d *polyNormals,int polyIndex,int vertexIndex,real minDot)
/****************************************************************************
*
* Function:		QMSmoothPolyhedron::findPolyNormals
* Parameters:	toSmooth	- Array to hold normals to smooth
*               polyNormals	- Polygon face normal data for object
*				polyIndex	- Index of this polygon in array
*				vertexIndex	- Index of this vertex in the polyhedron
*               minDot		- Minium dot product for smooth faces
*
* Description:	Finds all the adjacent face normals for the specified
*				vertex, and adds them to the array of normals to be
*				smoothed.
*
*****************************************************************************/
{
	FXVec3d		*pn;
	Poly		*p;
	short		*e;
	int			i,j,k,numToSmooth = 0;

	// First add the initial polygon normal
	toSmooth[numToSmooth++] = polyNormals[polyIndex];

	// Run through each polygon looking for shared vertices
	e = edgeData;
	for (i = 0, p = polys, pn = polyNormals; i < numPolys; i++,p++,pn++) {
		if (i == polyIndex)
			continue;
		k = p->index;
		for (j = 0; j < p->numEdges; j++,k++) {
			if (e[k] == vertexIndex) {
				if ((toSmooth[0] % *pn) >= minDot) {
					toSmooth[numToSmooth++] = *pn;
					if (numToSmooth > MAX_SMOOTH)
						MGL_fatalError("Too many shared vertices to smooth!");
					}
				}
			}
		}

	return numToSmooth;
}

void QMPolyhedron::smoothNormal(FXVec3d *n,FXVec3d *toSmooth,
	int numToSmooth)
/****************************************************************************
*
* Function:		QMPolyhedron::smoothNormal
* Parameters:   n			- Place to store normal
*               toSmooth	- Array of normals to smooth
*				numToSmooth	- Number of normals to smooth
*
* Description:	Computes the smoothed vertex normal from the list of
*				normals for that vertex.
*
****************************************************************************/
{
	if (numToSmooth <= 1) {
		*n = *toSmooth;
		return;
		}

	real scale = FXoneOver(FXintToReal(numToSmooth));
	n->x = n->y = n->z = 0;
	for (int i = 0; i < numToSmooth; i++, toSmooth++) {
		n->x += FXmul(toSmooth->x,scale);
		n->y += FXmul(toSmooth->y,scale);
		n->z += FXmul(toSmooth->z,scale);
		}
	n->normalise();
}

void QMPolyhedron::buildSmoothNormals(real smoothAngle)
/****************************************************************************
*
* Function:		QMPolyhedron::buildSmoothNormals
* Parameters:	smoothAngle	- Mimimum angle for facets to smooth
*
* Description:	Build all the vertex normals for the object. First we
*				compute all the polygon normals for each polygon and make
*				each vertex the same. Then we run through each polygon
*				finding all adjacent vertex normals and finding the average
*               of the normals for that vertex. If the angle between any two
*				normals is greater than 'smoothAngle' we dont average those
*				normals (so that we can still have sharp edges in the
*				objects).
*
*****************************************************************************/
{
	FXPoint3d	*v;
	FXVec3d		*n,*np,*polyNormals,*toSmooth;
    short       *e;
	Poly		*p;
	int			i,j,k,numToSmooth;

	// Compute the polygon normals for the object into a temporary array
	if ((toSmooth = new FXVec3d[numPolys + MAX_SMOOTH]) == NULL)
		MGL_fatalError("Out of memory smoothing polyhedron normals!\n");
	polyNormals = toSmooth + MAX_SMOOTH;
	v = vertices;
	e = edgeData;
    for (i = numPolys, p = polys, np = polyNormals; i; i--,p++,np++) {
		k = p->index;
		QMcomputeNormal(*np,p->numEdges,v,&e[k],orientation);
        callback(0);
		}

	// Now run through each polygon smoothing out those vertex normals that
	// are less than our smoothAngle value.
	real minDot = FXcos(smoothAngle);
	for (i = 0, p = polys, n = normals; i < numPolys; i++,p++) {
		k = p->index;
		for (j = 0; j < p->numEdges; j++,k++) {
			numToSmooth = findPolyNormals(toSmooth,polyNormals,i,e[k],minDot);
			smoothNormal(n++,toSmooth,numToSmooth);
			callback((i * 100L) / numPolys);
			}
		}

	delete [] toSmooth;
}

void QMPolyhedron::setShading(ibool shading)
/****************************************************************************
*
* Function:		QMPolyhedron::setShading
* Parameters:	New shading type
*
****************************************************************************/
{
	if (shading) {
		if (isSmooth)
			shadingType = Quick3d::LitSmoothShading;
		else shadingType = Quick3d::LitFlatShading;
		}
	else shadingType = Quick3d::NoShading;
}

void QMPolyhedron::setOrientation(int o)
/****************************************************************************
*
* Function:		QMPolyhedron::setOrientation
* Parameters:	o	- New facet orientation
*
****************************************************************************/
{
	orientation = o;
	if (isSmooth)
		buildSmoothNormals(cntSmoothAngle);
	else buildFlatNormals();
}
