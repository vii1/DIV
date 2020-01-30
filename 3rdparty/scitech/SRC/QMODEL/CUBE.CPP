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
* Description:	Implementation for the QMCube class.
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/cube.hpp"

/*---------------------------- Implementation -----------------------------*/

PRIVATE real cubeVertices[8][3] = {
	{REAL(0),REAL(0),REAL(1)},
	{REAL(0),REAL(1),REAL(1)},
	{REAL(1),REAL(1),REAL(1)},
	{REAL(1),REAL(0),REAL(1)},
	{REAL(0),REAL(0),REAL(0)},
	{REAL(0),REAL(1),REAL(0)},
	{REAL(1),REAL(1),REAL(0)},
	{REAL(1),REAL(0),REAL(0)},
	};

PRIVATE real cubeNormals[6][3] = {
	{REAL(0),REAL(0),REAL(1)},
	{REAL(0),REAL(1),REAL(0)},
	{REAL(0),REAL(0),REAL(-1)},
	{REAL(1),REAL(0),REAL(0)},
	{REAL(0),REAL(-1),REAL(0)},
	{REAL(-1),REAL(0),REAL(0)},
	};

QMCube::QMCube() : QMPrimitive()
/****************************************************************************
*
* Function:		QMCube::QMCube()
*
* Description:  Creates a unit cube.
*
****************************************************************************/
{
	v = (FXPoint3d*)cubeVertices;
	n = (FXVec3d*)cubeNormals;
}

void QMCube::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMCube::renderShaded
* Parameters:	dc	- Device context
*
****************************************************************************/
{
	// Draw the box as two flat shaded quad strips with 3 faces each.
	dc.setShadingType(Quick3d::LitFlatShading);
	int oldBackface = dc.setBackfaceMethod(Quick3d::BackfaceCull);
	dc.beginConvexObject();

	dc.beginQStrip();
		dc.normal(n[0]);
		dc.vertex(v[3]);
		dc.vertex(v[0]);
		dc.vertex(v[2]);
		dc.vertex(v[1]);

		dc.normal(n[3]);
		dc.vertex(v[6]);
		dc.vertex(v[5]);

		dc.normal(n[2]);
		dc.vertex(v[7]);
		dc.vertex(v[4]);
	dc.endQStrip();

	dc.beginQStrip();
		dc.normal(n[4]);
		dc.vertex(v[2]);
		dc.vertex(v[6]);
		dc.vertex(v[3]);
		dc.vertex(v[7]);

		dc.normal(n[1]);
		dc.vertex(v[0]);
		dc.vertex(v[4]);

		dc.normal(n[5]);
		dc.vertex(v[1]);
		dc.vertex(v[5]);
	dc.endQStrip();

	dc.endObject();
	dc.setBackfaceMethod(oldBackface);
}

void QMCube::renderWireframe(Quick3d& dc)
/****************************************************************************
*
* Function:		QMCube::renderWireframe
* Parameters:	dc	- Device context
*
****************************************************************************/
{
	dc.moveTo(v[0]);
	dc.lineTo(v[1]);
	dc.lineTo(v[2]);
	dc.lineTo(v[3]);
	dc.lineTo(v[0]);
	dc.lineTo(v[4]);
	dc.lineTo(v[5]);
	dc.lineTo(v[6]);
	dc.lineTo(v[7]);
	dc.lineTo(v[4]);

	dc.moveTo(v[1]);
	dc.lineTo(v[5]);

	dc.moveTo(v[2]);
	dc.lineTo(v[6]);

	dc.moveTo(v[3]);
	dc.lineTo(v[7]);
}

void QMCube::getBounds(FXPoint3d& min,FXPoint3d& max)
/****************************************************************************
*
* Function:		QMCube::getBounds
* Parameters:	min,max	 - Place to store bounds for object
*
****************************************************************************/
{
	min = v[4];
	max = v[2];
}

void QMCube::computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool)
/****************************************************************************
*
* Function:		QMCube::computeDirtyRect
* Parameters:   dc		- 3D device context
*				dirty	- Place to store dirty rectangle
* Returns:		True.
*
* Description:	Computes the dirty rectangle using the minimum and maximum
*				coordinates for the cube.
*
****************************************************************************/
{
	dc.computeDirtyRect(v[4],v[2],dirty);
}
