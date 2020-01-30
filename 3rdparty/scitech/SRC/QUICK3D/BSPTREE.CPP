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
* Description:	Implementation of the BSP Tree hidden surface algorithm
*				for the MGL3D extensions. Rendering is done with hidden
*				surface removal turned off for the MGL, and all computations
*				are done in view space.
*
*
****************************************************************************/

#include "quick3di.hpp"
#include "quick3d/bsptree.hpp"
#include <iostream.h>

/*---------------------------- Global Variables ---------------------------*/

PUBLIC	TCMemBlocks	BSPTreeNode::mem(sizeof(BSPTreeNode),300);
PUBLIC	int			BSPTree::numSplits;
PUBLIC	int			BSPTree::numPolys;
PRIVATE	BSPPolygon	*p,*frontPoly,*backPoly;
PRIVATE	int			backface;
PRIVATE	Quick3d		*gdc;
PRIVATE FXPoint3d	gpos;

/*---------------------------- Implementation -----------------------------*/

BSPTreeNode::~BSPTreeNode()
/****************************************************************************
*
* Function:		BSPTreeNode::~BSPTreeNode
*
* Description:	Destructor for the BSPTreeNode class. We simply recursively
*				delete the front, then the back subtree.
*
****************************************************************************/
{
	delete poly;
	delete front;
	delete back;
}

BSPTreeNode *BSP_makeTree(TCSimpleList<BSPPolygon>& polyList)
/****************************************************************************
*
* Function:		BSP_makeTree
* Parameters:	polyList	- List of polygons to build the tree from.
* Returns:		Pointer to the BSPTreeNode rooting the subtree
*
* Description:	Builds the BSP tree structure. This takes a list of polygons
*				to be inserted into the tree, and recursively builds the BSP
*				tree from these polygons.
*
****************************************************************************/
{
	TCSimpleList<BSPPolygon>	frontList,backList;
	BSPTreeNode					*node = new BSPTreeNode;

	if (node == NULL)
		goto OutOfMemory;

	node->poly = polyList.removeFromHead();

	while ((p = polyList.removeFromHead()) != NULL) {
#ifdef	PRINT_INFO
		cerr << "root: " << node->poly << endl;
		cerr << "p: " << p;
#endif
		switch (node->poly->split(p,frontPoly,backPoly)) {
			case BSPPolygon::FRONT:
				// The polygon p is in front of the root polygon
				// for this node, so we add it to the front polygon list

				frontList.addToHead(p);
#ifdef	PRINT_INFO
				cerr << " FRONT" << endl;
#endif
				break;
			case BSPPolygon::BACK:
				// The polygon p is behind the root polygon for this
				// node, so we add it to the back polygon list.

				backList.addToHead(p);
#ifdef	PRINT_INFO
				cerr << " BACK" << endl;
#endif
				break;
			case BSPPolygon::INPLANE:
				// The polygon p was intersected by the root polygon's
				// partitioning plane, so we kill the old polygon p,
				// and add the front and back parts to the correct
				// lists.

				frontList.addToHead(frontPoly);
				backList.addToHead(backPoly);
#ifdef	PRINT_INFO
				cerr << " SPLIT" << endl;
				cerr << "frontPoly: " << frontPoly;
				cerr << "backPoly: " << backPoly;
#endif
				delete p;
				BSPTree::numSplits++;
				break;
			case BSPPolygon::OUTOFMEMORY:
				delete p;
				goto OutOfMemory;
			}
		}

	// Now recursively build the front and back subtree's for the BSP tree.

	if (!frontList.isEmpty())
		if ((node->front = BSP_makeTree(frontList)) == NULL)
			goto OutOfMemory;
	if (!backList.isEmpty())
		if ((node->back = BSP_makeTree(backList)) == NULL)
			goto OutOfMemory;
	return node;

	// Ran out of memory while processing data.

OutOfMemory:
	delete node;
	return NULL;
}

void BSPTreeNode::render()
/****************************************************************************
*
* Function:		BSPTreeNode::render
*
* Description:	Renders the tree in back to front order given the specified
*				reference point. The reference point is normally the
*				eye position, but it may be transformed if you wish to
*				spin the scene rather than move the eye position.
*
*				We get the 3D DC and eye position for rendering from
*				global variables to save on processor stack space since
*				this rendering routine is recursive.
*
****************************************************************************/
{
	if (poly->evalPlaneEquation(gpos) > 0) {
		// The reference position is in front of the root polygon, so
		// render all the polygons behind first, then those in front

		if (back)	back->render();
		gdc->forceBackface(false);
		poly->render(*gdc);
		if (front)	front->render();
		}
	else {
		// The reference position is behind the root polygon, so render
		// the front polygons first, then those behind.

		if (front)	front->render();
		if (backface != Quick3d::BackfaceCull) {
			gdc->forceBackface(true);
			poly->render(*gdc);
			}
		if (back)	back->render();
		}
}

ibool BSPTree::preprocess()
/****************************************************************************
*
* Function:		BSPTree::preprocess
* Returns:		True if the BSPTree was completely built
*
* Description:	Pre-processes the stored polygons and builds BSP tree
*				tree from them for hidden surface removal.
*
****************************************************************************/
{
	BSPTree::numSplits = 0;
	numPolys = polyList.numberOfItems();
	if (!polyList.isEmpty())
		root = BSP_makeTree(polyList);
	return (root != NULL);
}

void BSPTree::render(Quick3d& dc,const FXPoint3d& pos)
/****************************************************************************
*
* Function:		BSPTree::render
* Parameters:	dc	- 3D Device context to render into
*				pos	- Reference position used to determine visible surfaces
*
* Description:	Renders the BSP tree via the current rendering pipeline.
*				Note that the polygons in the BSP tree may have had the
*				ordering of vertices changed during splitting, so we
*				_must_ turn off low level backface culling in the MGL3D
*				extensions and do it ourselve from the knowledge we
*				have in the tree.
*
****************************************************************************/
{
	if (root) {
		backface = dc.setBackfaceMethod(Quick3d::BackfaceShade);
		gpos = pos;
		gdc = &dc;
		root->render();
		dc.setBackfaceMethod(backface);
		}
}
