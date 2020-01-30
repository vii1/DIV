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
* Description:	Implementation of the BSPPolygon class representing all
*				the polygons present in the BSP tree. Because of the
*				numerical accuracy needed by the splitting process, all
*				splitting must be done in floating point, but the final
*				rendering can be done in fixed point.
*
*
****************************************************************************/

#include "quick3di.hpp"
#include "quick3d/bsptree.hpp"
#include <iostream.h>

/*---------------------------- Global Variables ---------------------------*/

PUBLIC	TCMemBlocks	BSPPolygon::mem(sizeof(BSPPolygon),300);
PUBLIC	real 		BSPPolygon::quality = REAL(0.01);

/*---------------------------- Implementation -----------------------------*/

BSPPolygon::BSPPolygon(int numVertices)
/****************************************************************************
*
* Function:		BSPPolygon::BSPPolygon
* Parameters:	numVertices	 - Number of vertices in the polygon
*
* Description:	Constructor for the BSPPolygon class. This allocates the
*				space for the specified number of vertices.
*
****************************************************************************/
{
PRE(maxCount = numVertices;)
	vertex = new FXPoint3d[numVertices];
	count = 0;
}

void BSPPolygon::computeNormal(ibool clockwise)
/****************************************************************************
*
* Function:		BSPPolygon::computeNormal
* Parameters:	clockwise	- True backfacing polygons are clockwise defined
*
* Description:	Computes the normal for the polygon by finding the cross
*				product of the vectors from vertices 0,1 and 2.
*
****************************************************************************/
{
	FXVec3d	t1,t2;

	t1 = vertex[2];	t1 -= vertex[1];
	t2 = vertex[1]; t2 -= vertex[0];
	normal.cross(t1,t2);
	normal.normalise();
	if (!clockwise)
		normal.negate();

	// The plane equation for the polygon is Ax + By + Cz + D = 0. The
	// vector <A,B,C> is the normal vector to the plane, so we need
	// to calculate D which is D = -(Ax + By + Cz) where <x,y,z> is
	// a point on the plane (any point in the polygon), or the dot
	// product between this and the vector.

	planeD = -(normal % (FXVec3d&)vertex[0]);
}

void BSPPolygon::render(Quick3d& dc)
/****************************************************************************
*
* Function:		BSPPolygon::render
* Parameters:	dc	- 3D device context to render into
*
* Description:	Renders the polygon via the MGL. By the time this routine
*				is called, the polygon is being displayed in the correct
*				order for hidden surface removal.
*
****************************************************************************/
{
	dc.beginPoly();
		dc.normal(normal);
		for (int i = 0; i < count; i++)
			dc.vertex(vertex[i]);
	dc.endPoly();
}

void BSPPolygon::intersect(const FXPoint3d& p1,const FXPoint3d& p2,
	FXPoint3d& theInt)
/****************************************************************************
*
* Function:		BSPPolygon::intersect
* Parameters:	p1		- First point on the line
*				p2		- Second point on the line
*				theInt	- Place to store the computed intersection
*
* Description:	Computes the point of intersection between an edge of the
*				polygon and the partitioning plane. Note that we do a
*				fuzzy check to see if the intersection point is close
*				enough to either p1 or p2 to be called the same as that
*				point.
*
****************************************************************************/
{
	FXVec3d	dir(p2);	dir -= p1;
	real	t = dir % normal;

	PRECONDITION(ABS(t) > _FUDGE);
	t = FXdiv(-evalPlaneEquation(p1),t);
	scaleAndAdd((FXVec3d&)theInt,p1,dir,t);		// theInt = p1 + dir*t
}

#define	front_add(x)			(front[frontIndex++] = x)
#define	back_add(x)				(back[backIndex++] = x)
#define	extras_add(x)			(extras[extrasIndex++] = x)
#define	front_isEmpty()			(frontIndex == 0)
#define	back_isEmpty()			(backIndex == 0)
#define	MAXSPLIT				30

int BSPPolygon::split(BSPPolygon *p,BSPPolygon*& frontPoly,
	BSPPolygon*& backPoly)
/****************************************************************************
*
* Function:		BSPTreeNode::split
* Parameters:	p			- Polygon to split if necessary
*				frontPoly	- Place to store front polygon if needed
*				backPoly	- Place to store back polygon if needed
* Returns:		flag indicating whether split was performed
*
* Description:	Determines if the polygon 'p' lies in front, behind or
*				intersects with the partitioning plane defined by the
*				this polygon. If the polygon lies in front or behind,
*				we simply return the appropriate flag. If the polygon
*				intersects the partitioning plane, it is split into
*				two new polygons, and the old polygon is deleted.
*
*				NOTE: This presently will _ONLY_ work for polys with up to
*					  30 sides per split poly. It will fail vocally with
*					  more however.
*
****************************************************************************/
{
	short		front[MAXSPLIT],back[MAXSPLIT];
	FXPoint3d	theInt,extras[MAXSPLIT];
	int			i,j,prev,frontIndex,backIndex,extrasIndex,inplane;
	real		val,firstVal;

	// Determine where the first point in the polygon lies in relation to
	// the partitioning plane.

	frontIndex = backIndex = extrasIndex = inplane = 0;
	firstVal = val = evalPlaneEquation(p->vertex[0]);
	if (_FZERO(val,quality)) {
		prev = INPLANE;			// First point is in the plane
		inplane++;
		}
	else if (val > 0) {
		front_add(0);			// First point is in front
		prev = FRONT;
		}
	else {
		back_add(0);			// First point is behind
		prev = BACK;
		}

	// Now run through every point in the polygon, building up a list
	// of points that need to be included in the split polygons if the
	// polygon is to be split. Note that the quality factor is used to
	// determine how close a point needs to be to the plane to be
	// considered 'on' it.

	for (i = 1; i < p->count; i++) {
		val = evalPlaneEquation(p->vertex[i]);
		if (_FZERO(val,quality)) {
			// The point is on the partitioning plane, so continue
			// adding the point to the same side the previous one was
			// on.

			if (prev == FRONT) {
				front_add(i);
				inplane = -1;
				}
			else if (prev == BACK) {
				back_add(i);
				inplane = -1;
				}
			else inplane++;		// Still in the plane from first point
			}
		else if (val > 0) {
			// The point is in front of the partitioning plane.

			if (prev == BACK) {
				// The previous point was on the other side, so we have
				// found an intersection between the plane and the
				// polygon. Find the point of intersection and insert
				// this into the polygon.

				if (inplane == -1) {
					// The previous point was actually in the plane and
					// was actually a dividing vertex, so add to the
					// front polygon (it has already been added to the
					// back one).

					front_add(i-1);
					}
				else {
					intersect(p->vertex[i-1],p->vertex[i],theInt);
					extras_add(theInt);
					back_add(-extrasIndex);
					front_add(-extrasIndex);
					}
				front_add(i);
				}
			else if (prev == FRONT)
				front_add(i);	// Add to front list
			else {
				// All previous points were in the plane.

				for (j = 0; j < inplane; j++)
					front_add(j);
				front_add(i);
				}
			inplane = 0;
			prev = FRONT;
			}
		else {
			// The point is behind the partitioning plane.

			if (prev == FRONT) {
				// The previous point was on the other side, so we have
				// found and intersection between the plane and the
				// polygon. Find the point of intersection and insert
				// this into the polygon.

				if (inplane == -1) {
					// The previous point was actually in the plane and
					// was actually a dividing vertex, so add to the
					// back polygon (it has already been added to the
					// front one).

					back_add(i-1);
					}
				else {
					intersect(p->vertex[i-1],p->vertex[i],theInt);
					extras_add(theInt);
					front_add(-extrasIndex);
					back_add(-extrasIndex);
					}
				back_add(i);
				}
			else if (prev == BACK)
				back_add(i);	// Add to back list
			else {
				// All previous points were in the plane.

				for (j = 0; j < inplane; j++)
					back_add(j);
				back_add(i);
				}
			inplane = 0;
			prev = BACK;
			}
		}

	// Take a quick exit if the polygon does not require splitting.

	if (inplane > 0)
		return (val > 0) ? FRONT : BACK;

	if (front_isEmpty())
		return BACK;
	if (back_isEmpty())
		return FRONT;

	// Close off both polygons if the first and last points are on
	// different sides of the partitioning plane, otherwise find the
	// correct plane to store the first and last points.

	if (!_FZERO(val,quality) && !_FZERO(firstVal,quality) &&
			(SIGN(val) != SIGN(firstVal))) {
		intersect(p->vertex[0],p->vertex[p->count-1],theInt);
		extras_add(theInt);
		back_add(-extrasIndex);
		front_add(-extrasIndex);
		}
	else {
		if (_FZERO(firstVal,quality)) {
			// The first point in the polygon was on the plane, so close
			// the front or back polygon using the point depending on
			// which one recieved the first point originally.

			if (back[0] == 0)
				front_add(0);
			else back_add(0);
			}
		else if (_FZERO(val,quality)) {
			// The last point in the polygon was on the plane, so close
			// the front or back polygon using the last point depending
			// on which one received the last point.

			if (prev == BACK)
				front_add(p->count-1);
			else back_add(p->count-1);
			}
		}

	PRECONDITION(frontIndex >= 3);
	PRECONDITION(backIndex >= 3);
	PRECONDITION(frontIndex <= MAXSPLIT);
	PRECONDITION(backIndex <= MAXSPLIT);
	PRECONDITION(extrasIndex <= MAXSPLIT);

	// Build the new polygons from the polygon vertex information that
	// we built during the splitting process. Note that the plane
	// equations for the polygons will be the same as the original polygon.

	BSPPolygon *fp = frontPoly = new BSPPolygon(frontIndex);
	BSPPolygon *bp = backPoly = new BSPPolygon(backIndex);

	if (fp == NULL || bp == NULL)
		goto OutOfMemory;
	if (!fp->valid() || !bp->valid())
		goto OutOfMemory;

	for (i = 0; i < frontIndex; i++) {
		if (front[i] >= 0)
			fp->addVertex(p->vertex[front[i]]);
		else
			fp->addVertex(extras[(-front[i])-1]);
		}
	fp->normal = p->normal;
	fp->planeD = p->planeD;

	for (i = 0; i < backIndex; i++) {
		if (back[i] >= 0)
			bp->addVertex(p->vertex[back[i]]);
		else
			bp->addVertex(extras[(-back[i])-1]);
		}
	bp->normal = p->normal;
	bp->planeD = p->planeD;

	return INPLANE;

OutOfMemory:
	delete fp;
	delete bp;
	return OUTOFMEMORY;
}

ostream& operator << (ostream& o,const BSPPolygon *p)
/****************************************************************************
*
* Function:		operator <<
* Parameters:	p	- Pointer to polygon to display
* Returns:		Stream poly was dumped to.
*
****************************************************************************/
{
	o << p->count << " vertices" << endl;
	for (int i = 0; i < p->count; i++)
		o << "    " << p->vertex[i] << endl;
	o << "    normal = " << p->normal << ", D = ";
	o << (float)p->planeD << endl;
	return o;
}
