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
* Description:	Header file for the QMMaterialRegistry class.
*
*
****************************************************************************/

#ifndef	__QMODEL_MATREG_HPP
#define	__QMODEL_MATREG_HPP

#ifndef	__QUICK3D_QUICK3D_HPP
#include "quick3d/quick3d.hpp"
#endif

#ifndef	__TCL_LIST_HPP
#include "tcl/list.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Class to implement a registry of materials used in the scene. For
// color map modes this allows the registry to allocate all the color
// map indices as required to allow all the desired colors in the scene
// to be mapped in. In RGB modes this simply manages all the materials
// allocated for the scene.
//---------------------------------------------------------------------------

class QMMaterialRegistry {
protected:
	// Nested class to manage the list of materials in the registry
	struct Node : public TCListNode {
		MGLMaterial	*material;
		Node(MGLMaterial* material) : material(material) {};
		~Node() { delete material; };
		};

	typedef TCList<Node> 			NodeList;
	typedef TCListIterator<Node>	NodeListIter;

	NodeList	materials;			// List of materials in the scene
	real		diffuseRange;		// Range of diffuse values (0-1)
	int			startIndex;			// Starting index in palette
	int			endIndex;			// Ending index in palette

public:
			// Constructor
			QMMaterialRegistry(int startIndex,int endIndex,
				real diffuseRange = REAL(0.75));

			// Destructor
			~QMMaterialRegistry() {};

			// Method to add a new material to the registry
			void add(MGLMaterial *m) { materials.addToHead(new Node(m)); };

			// Method to build the palette required by the registry
			void buildPalette(palette_t *pal);

			// Method to build the material shade tables
			void buildShadeTables(MGLDevCtx& dc,int entries);
	};

#endif	// __QMODEL_MATREG_HPP
