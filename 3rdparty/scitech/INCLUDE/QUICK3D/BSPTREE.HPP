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
* Description:	Header file for the BSPTree class. This class is used
*				to implement hidden surface removal without using the
*               MGL3D depth sorting or Zbuffering hidden surface removal
*				techniques. The BSPTree class is very fast for displaying
*				static scene information, where the only thing moving is
*				the viewpoint within the scene. Building a BSPTree is
*				however very time and memory consuming, due to the need
*				to split polygons to correctlty build the tree.
*
*				For fast and efficient memory consumption (and also to
*				provide a fast way to free all memory when an out of
*				memory error occurs) we use efficient MemBlock's for
*				local memory management.
*
*
****************************************************************************/

#ifndef	__BSPTREE_HPP
#define	__BSPTREE_HPP

#ifndef	__FX_VEC3D_H
#include "fx/vec3d.h"
#endif

#ifndef	__TCL_LIST_HPP
#include "tcl/list.hpp"
#endif

#ifndef	__TCL_MEMMGR_HPP
#include "tcl/memmgr.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

class BSPTreeNode;				// Forward reference

//---------------------------------------------------------------------------
// The following defines the BSPPolygon class used internally by the BSP
// tree algorithm. BSPPolygon's contain the actual polygon data used
// during rendering, and are linked together on a linked list.
//
// You need to create each individual BSPPolygon that needs to be added
// to the BSP tree and then add them.
//
// Note that we use a local memory allocator pools for allocating the
// BSPolygon's for efficiencty, but we use the normal new and delete
// memory allocators for the vertex arrays since these need to be arbitrary
// in size.
//---------------------------------------------------------------------------

class BSPPolygon : public TCListNode {
protected:
	FXPoint3d	*vertex;	// Array of vertices in the polygon
	short		count;		// Number of vertices in the polygon
	FXVec3d		normal;		// Polygon normal
	real		planeD;		// D value of plane equation
PRE(short		maxCount;)	// Maximum number of vertices in polygon

			// Method to compute the point of intersection
			void intersect(const FXPoint3d& p1,const FXPoint3d& p2,
				FXPoint3d& theInt);

public:
			// Constructor
			BSPPolygon(int numVectices);

			// Destructor
			~BSPPolygon() { delete [] vertex; };

			// Return whether the polygon is valid
			ibool valid() { return vertex != NULL; };

			// Method to add a vertex to the polygon
			void addVertex(const FXPoint3d& v)
			{
				PRECONDITION(count < maxCount);
				vertex[count++] = v;
			};

			// Method to compute the polygon normal
			void computeNormal(ibool clockwise = true);

			// Method to render the polygon to the display
			void render(Quick3d& dc);

			// Method to split the polygon if required
			int split(BSPPolygon *p,BSPPolygon*& frontPoly,
				BSPPolygon*& backPoly);

			// Method to evaluate the plane equation for a specified point
			real evalPlaneEquation(const FXPoint3d& p)
				{ return (normal % (FXVec3d&)p) + planeD; };

	friend	ostream& operator << (ostream& o,const BSPPolygon* p);

	static TCMemBlocks	mem;	// Local memory allocator

			// Overloaded memory allocation
			void *operator new(size_t size) { return mem.allocate(size); };
			void operator delete(void *p)	{ mem.free(p); };

	enum { FRONT, BACK, INPLANE, OUTOFMEMORY };

			// Quality factor used during splitting process. This is used
			// to determine whether a point lies in the partitioning plane
			// and is used to lower the total number of splits performed
			// and the possibility of co-incident vertices created by
			// the splitting process (if the value is too small). This
			// should be tuned for each application to trade rendering
			// speed with correct hidden surface operation.
	static	real quality;
	};

//---------------------------------------------------------------------------
// The following defines the BSPTreeNode class. The actual BSPTree internal
// structure is built from BSPTreeNode's. Note that there is no destructor
// for the class as memory is allocated using a local memory allocator
// so that it can be freed all at once.
//---------------------------------------------------------------------------

class BSPTreeNode {
protected:
	BSPPolygon			*poly;	// Root polygon for this node
	BSPTreeNode			*front;	// Nodes in front of dividing plane
	BSPTreeNode			*back;	// Nodes at back of dividing plane

public:
			// Constructor
			BSPTreeNode()	{ poly = NULL; front = back = NULL; };

			// Destructor
			~BSPTreeNode();

			// Method to render the tree
			void render();

			// Method to build the tree
	friend	BSPTreeNode *BSP_makeTree(TCSimpleList<BSPPolygon>& polyList);

	static TCMemBlocks	mem;	// Local memory allocator

			// Overloaded memory allocation
			void *operator new(size_t size) { return mem.allocate(size); };
			void operator delete(void *p)	{ mem.free(p); };
	};

//---------------------------------------------------------------------------
// The following defines the BSPTree class. This class is used to
// implement BSP tree hidden surface removal.
//---------------------------------------------------------------------------

class BSPTree {
protected:
	TCSimpleList<BSPPolygon>	polyList;	// List of initial BSP polygons
	BSPTreeNode					*root;		// Root node in the BSP Tree.

public:
			// Constructor
			BSPTree() : root(NULL) {};

			// Destructor
			~BSPTree()	{ delete root; };

			// Method to add a polygon to the tree
			void addPolygon(BSPPolygon *p)	{ polyList.addToHead(p); };

			// Method to pre-process the BSP tree before rendering
			ibool preprocess();

			// Method to render the BSP tree using the MGL
			void render(Quick3d& dc,const FXPoint3d& pos);

	static	numPolys;			// Number of polygon in tree
	static	numSplits;			// Number of polygon splits required
	};

#endif	// __BSPTREE_HPP
