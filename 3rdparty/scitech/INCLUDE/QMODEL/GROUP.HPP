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
* Description:	Header file for the QMGroup class.
*
*
****************************************************************************/

#ifndef	__QMODEL_GROUP_HPP
#define	__QMODEL_GROUP_HPP

#ifndef	__QMODEL_PRIMITIV_HPP
#include "qmodel/primitiv.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Class to represent a group of objects in the scene.
//---------------------------------------------------------------------------

class QMGroup : public QMObject {
protected:
	QMObjList	children;

public:
			// Constructor
			QMGroup() : QMObject() {};

			// Destructor
	virtual	~QMGroup() {};

			// Method to add an object to the group
			void add(QMObject* o) { children.addToHead(o); };

			// Method to detach a child from the group
			void detach(QMObject *o);

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

#endif	// __QMODEL_GROUP_HPP
