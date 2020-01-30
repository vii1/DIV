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
* Description:	Header file for the QMMaterial class.
*
*
****************************************************************************/

#ifndef	__QMODEL_MATERIAL_HPP
#define	__QMODEL_MATERIAL_HPP

#ifndef	__QMODEL_OBJECT_HPP
#include "qmodel/object.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Class to represent bound front and back material properties. Note that
// the material binding owns the child object and will delete it, but does
// not own the materials since materials will most likely be bound to
// multiple objects in the scene.
//---------------------------------------------------------------------------

class QMMaterial : public QMObject {
protected:
	QMObject	*child;		// Child material is bound to
	MGLMaterial	*front;		// Front face material
	MGLMaterial	*back;		// Back face material
	char		ownChild;	// True if we own the child

public:
			// Constructor
			QMMaterial(QMObject *child,MGLMaterial *front,MGLMaterial *back,
				ibool ownChild = true);

			// Destructor
	virtual	~QMMaterial() { if (ownChild) delete child; };

			// Methods to render the object
	virtual void renderShaded(Quick3d& dc);
	virtual void renderWireframe(Quick3d& dc);

			// Method to pre-process the object prior to rendering
	virtual	void preProcess();

			// Method to return the bounds of the object
	virtual	void getBounds(FXPoint3d& min,FXPoint3d& max);

			// Method to compute the dirty rectangle for the object
	virtual	void computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool viewChanged);
	};

#endif	// __QMODEL_MATERIAL_HPP
