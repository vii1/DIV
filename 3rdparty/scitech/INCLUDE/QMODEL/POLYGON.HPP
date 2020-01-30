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
* Description:	Header file for the QMFlatPolygon primitive.
*
*
****************************************************************************/

#ifndef	__QMODEL_POLYGON_HPP
#define	__QMODEL_POLYGON_HPP

#ifndef	__QMODEL_PRIMITIV_HPP
#include "qmodel/primitiv.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Abstract base class for all polygon primitives
//---------------------------------------------------------------------------

class QMPolygon : public QMPrimitive {
protected:
	int			count;			// Number of vertices in polygon
	FXPoint3d	*vertices;		// Array of polygon vertices

public:
			// Constructor
			QMPolygon(int count,const FXPoint3d *vertices);

			// Destructor
	virtual	~QMPolygon();

			// Methods to render the object
	virtual void renderWireframe(Quick3d& dc);

			// Method to return the bounds of a primitive
	virtual	void getBounds(FXPoint3d& min,FXPoint3d& max);
	};

//---------------------------------------------------------------------------
// Primitive object representing a single flat shaded polygon
//---------------------------------------------------------------------------

class QMFlatPolygon : public QMPolygon {
protected:
	FXVec3d		normal;			// Polygon normal

public:
			// Constructor
			QMFlatPolygon(int count,const FXPoint3d *vertices,
				const FXVec3d& normal)
				: QMPolygon(count,vertices),normal(normal) {};

			// Destructor
	virtual	~QMFlatPolygon() {};

			// Method to render the object
	virtual void renderShaded(Quick3d& dc);
	};

//---------------------------------------------------------------------------
// Primitive object representing a single smooth shaded polygon
//---------------------------------------------------------------------------

class QMSmoothPolygon : public QMPolygon {
protected:
	FXVec3d		*normals;		// Polygon vertex normals

public:
			// Constructor
			QMSmoothPolygon(int count,const FXPoint3d *vertices,
				const FXVec3d *normals);

			// Destructor
	virtual	~QMSmoothPolygon();

			// Method to render the object
	virtual void renderShaded(Quick3d& dc);
	};

#endif	// __QMODEL_POLYGON_HPP
