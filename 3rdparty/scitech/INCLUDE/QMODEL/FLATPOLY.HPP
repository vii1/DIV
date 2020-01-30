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
// Base class for all objects in the OBJECT.
//---------------------------------------------------------------------------

class QMObject : public TCListNode {
public:
			// Constructor
			QMObject() {};

			// Destructor
	virtual	~QMObject() {};

			// Methods to render the object
	virtual void renderShaded(Quick3d& dc) = 0;
	virtual void renderWireframe(Quick3d& dc) = 0;
	virtual void renderSIPP(void) = 0;

			// Method to compute the dirty rectangle for the object
	virtual	ibool computeDirtyRect(MGLRect& dirty,ibool viewChanged) = 0;

			// Method to pre-process the object prior to rendering
	virtual	void preProcess();
	};

typedef TCList<QMObject> 			QMObjList;
typedef TCListIterator<QMObject>	QMObjListIter;

//---------------------------------------------------------------------------
// Base class for all primitive objects which can be rendered in the OBJECT
//---------------------------------------------------------------------------

class QMPrimitive : public QMObject {
public:
			// Constructor
			QMPrimitive() {};

			// Destructor
	virtual	~QMPrimitive() {};

			// Method to return the bounds of a primitive
	virtual	void getBounds(FXPoint3d& min,FXPoint3d& max) = 0;
	};

//---------------------------------------------------------------------------
// Class to wrap a bounding box around an object or set of objects for
// efficient dirty rectangle calculations.
//---------------------------------------------------------------------------

class QMBoundingBox : public QMPrimitive {
protected:
	FXPoint3d	min,max;		// Cached bounding box for child
	QMObject	*child;			// Child object to wrap box around

public:
			// Constructor
			QMBoundingBox(QMObject *child) : child(child) {};

			// Destructor
	virtual	~QMBoundingBox();

			// Method to pre-process the object prior to rendering
	virtual	void preProcess();

			// Method to return the bounds of a primitive
	virtual	void getBounds(FXPoint3d& min,FXPoint3d& max);

			// Method to compute the dirty rectangle for the object
	virtual	ibool computeDirtyRect(MGLRect& dirty,ibool viewChanged);
	};

//---------------------------------------------------------------------------
// Primitive object representing a single flat shaded polygon
//---------------------------------------------------------------------------

class QMFlatPolygon : public QMPrimitive {
protected:
	int			count;			// Number of vertices in polygon
	FXPoint3d	*vertices;		// Array of polygon vertices
	FXVec3d		normal;			// Polygon normal

public:
			// Constructor
			QMFlatPolygon(int count,const FXPoint3d *vertices);
			QMFlatPolygon(int count,const FXPoint3d *vertices,
				const FXVec3d& normal);

			// Destructor
	virtual	~QMFlatPolygon();

			// Method to render the object
	virtual void renderShaded(Quick3d& dc);
	virtual void renderWireframe(Quick3d& dc) = 0;
	virtual void renderSIPP(void) = 0;
	};

//---------------------------------------------------------------------------
// Primitive object representing a single smooth shaded polygon
//---------------------------------------------------------------------------

class QMSmoothPolygon : public QMPrimitive {
protected:
	int			count;			// Number of vertices in polygon
	FXPoint3d	*vertices;		// Array of polygon vertices
	FXVec3d		*normals;		// Polygon vertex normals

public:
			// Constructor
			QMSmoothPolygon(int count,const FXPoint3d *vertices,
				const FXVec3d *normals);

			// Destructor
	virtual	~QMSmoothPolygon();

			// Method to render the object
	virtual void renderShaded(Quick3d& dc);
	virtual void renderWireframe(Quick3d& dc) = 0;
	virtual void renderSIPP(void) = 0;
	};

//---------------------------------------------------------------------------
// Primitive object representing a cube
//---------------------------------------------------------------------------

class QMCube : public QMPrimitive {
protected:
	FXPoint3d	vertices[8];	// Face vertices
	FXVec3d		normals[6];		// Face normal

public:
			// Constructor
			QMCube(const FXPoint3d vertices[8]);

			// Destructor
	virtual	~QMCube() {};

			// Method to render the object
	virtual void renderShaded(Quick3d& dc);
	virtual void renderWireframe(Quick3d& dc) = 0;
	virtual void renderSIPP(void) = 0;
	};

/*------------------------- Inline member functions -----------------------*/

#endif	// __QMODEL_OBJECT_HPP
