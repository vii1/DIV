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
* Description:	Header file for the QMPrimitive base class.
*
*
****************************************************************************/

#ifndef	__QMODEL_PRIMITIV_HPP
#define	__QMODEL_PRIMITIV_HPP

#ifndef	__QMODEL_OBJECT_HPP
#include "qmodel/object.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Base class for all primitive objects which can be rendered in the scene
//---------------------------------------------------------------------------

class QMPrimitive : public QMObject {
public:
			// Constructor
			QMPrimitive() : QMObject() {};

			// Destructor
	virtual	~QMPrimitive() {};

			// Method to determine if a node is a primitive
	virtual	ibool isPrimitive() const { return true; };

			// Default method to compute the dirty rectangle for primitives
	virtual	void computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool viewChanged);
	};

#endif	// __QMODEL_PRIMITIV_HPP
