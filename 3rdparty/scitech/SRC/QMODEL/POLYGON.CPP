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
* Description:	Implementation for the QMPolygon classes
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/polygon.hpp"

/*---------------------------- Implementation -----------------------------*/

QMPolygon::QMPolygon(int count,const FXPoint3d *v)
	: QMPrimitive(), count(count)
/****************************************************************************
*
* Function:		QMPolygon::QMPolygon
* Parameters:	count	- Number of vertices in polygon
*               v		- Array of vertices in polygon
*
* Description:  Creates the base class for polygon primitives
*
****************************************************************************/
{
	vertices = new FXPoint3d[count];
	memcpy(vertices,v,sizeof(FXPoint3d) * count);
}

QMPolygon::~QMPolygon()
{
	delete [] vertices;
}

void QMPolygon::renderWireframe(Quick3d& dc)
/****************************************************************************
*
* Function:		QMPolygon::renderWireframe
* Parameters:	dc	- Device context
*
****************************************************************************/
{
	int			i = count;
	FXPoint3d	*v = vertices;

	dc.moveTo(*v++);
	while (--i)
		dc.lineTo(*v++);
	dc.lineTo(*vertices);
}

void QMPolygon::getBounds(FXPoint3d& min,FXPoint3d& max)
/****************************************************************************
*
* Function:		QMPolygon::getBounds
* Parameters:	min,max	 - Place to store bounds for object
*
****************************************************************************/
{
	int			i = count;
	FXPoint3d   *v = vertices;

	min = max = *v++;
	while (--i) {
		min.x = MIN(min.x,v->x);
		min.y = MIN(min.y,v->y);
		min.z = MIN(min.z,v->z);
		max.x = MAX(max.x,v->x);
		max.y = MAX(max.y,v->y);
		max.z = MAX(max.z,v->z);
		v++;
		}
}

void QMFlatPolygon::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMFlatPolygon::renderShaded
* Parameters:	dc	- Device context
*
****************************************************************************/
{
	int			i = count;
	FXPoint3d	*v = vertices;

	dc.setShadingType(Quick3d::LitFlatShading);
	dc.beginPoly();
	while (i--)
		dc.vertex(*v++);
	dc.endPoly();
}

QMSmoothPolygon::QMSmoothPolygon(int count,const FXPoint3d *v,
	const FXVec3d *n) : QMPolygon(count,v)
/****************************************************************************
*
* Function:		QMSmoothPolygon::QMSmoothPolygon
* Parameters:	count	- Number of vertices in polygon
*               v		- Array of vertices in polygon
*				n		- Array of vertex normals for polygon
*
* Description:  Creates a new smooth shaded polygon
*
****************************************************************************/
{
	normals = new FXVec3d[count];
	memcpy(normals,n,sizeof(FXVec3d) * count);
}

QMSmoothPolygon::~QMSmoothPolygon()
{
	delete [] normals;
}

void QMSmoothPolygon::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMSmoothPolygon::renderShaded
* Parameters:	dc	- Device context
*
****************************************************************************/
{
	int			i = count;
	FXPoint3d	*v = vertices;
	FXVec3d		*n = normals;

	dc.setShadingType(Quick3d::LitSmoothShading);
	dc.beginPoly();
	while (i--) {
		dc.normal(*n++);
		dc.vertex(*v++);
		}
	dc.endPoly();
}
