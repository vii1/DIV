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
* Description:	Header file for the QMObject base class.
*
*
****************************************************************************/

#ifndef	__QMODEL_OBJECT_HPP
#define	__QMODEL_OBJECT_HPP

#ifndef	__QUICK3D_QUICK3D_HPP
#include "quick3d/quick3d.hpp"
#endif

#ifndef	__TCL_LIST_HPP
#include "tcl/list.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Base class for all objects in the scene. All QMObject's are reference
// counted, so you can create instances of objects to save space (up to
// 65,536 instances). Objects must *always* be allocated with new and free
// before being inserted into a scene.
//---------------------------------------------------------------------------

class QMObject : public TCListNode {
public:
			// Constructor
			QMObject() 	{};

			// Destructor
	virtual	~QMObject()	{};

			// Methods to render the object
	virtual void renderShaded(Quick3d& dc) = 0;
	virtual void renderWireframe(Quick3d& dc) = 0;

			// Method to determine if a node is a primitive
	virtual	ibool isPrimitive() const { return false; };

			// Method to return the bounds of the object
	virtual	void getBounds(FXPoint3d& min,FXPoint3d& max) = 0;

			// Method to compute the dirty rectangle for the object
	virtual	void computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool viewChanged = false) = 0;

			// Method to pre-process the object prior to rendering
	virtual	void preProcess() {};
	};

typedef TCList<QMObject> 			QMObjList;
typedef TCListIterator<QMObject>	QMObjListIter;

/*--------------------------- Global Variables ----------------------------*/

extern	ibool 	QMrenderBounds;
extern	color_t	QMboundsColor;

/*-------------------------- Function Prototypes --------------------------*/

void 	QMcomputeNormal(FXVec3d& normal,int count,FXPoint3d *v,int orientation);
void 	QMcomputeNormal(FXVec3d& normal,int count,FXPoint3d *v,short *index,int orientation);
void	QMdrawWireframeBox(Quick3d& dc,const FXPoint3d& min,const FXPoint3d& max);

#endif	// __QMODEL_OBJECT_HPP
