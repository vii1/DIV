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
* Description:	Implementation for the QMPrimitive class.
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/object.hpp"

/*---------------------------- Implementation -----------------------------*/

ibool 	QMrenderBounds = false;
color_t	QMboundsColor = MGL_BLACK;

void QMcomputeNormal(FXVec3d& normal,int count,FXPoint3d *v,int orientation)
/****************************************************************************
*
* Function:		QMcomputeNormal
* Parameters:	normal		- Place to store the normal
*				count		- Number of vertices in polygon
*               v			- Array of polygon vertices
*				clockwise	- True if vertices are clockwise for front face
*
* Description:  Computes the normal for the polygon from the list of
*				vertices in the polygon. We use Martin Newell's method,
*				which is more robust than the 3 point method. See
*				Newman & Sproull page 499.
*
****************************************************************************/
{
	int	i,j;

	normal.x = 0; normal.y = 0; normal.z = 0;
	for (i = 0, j = i+1; i < count; i++,j++) {
		if (j == count)
			j = 0;
		normal.x -= (v[i].y - v[j].y) * (v[i].z + v[j].z);
		normal.y -= (v[i].z - v[j].z) * (v[i].x + v[j].x);
		normal.z -= (v[i].x - v[j].x) * (v[i].y + v[j].y);
		}
	normal.normalise();
	if (orientation != Quick3d::ClockWise)
		normal.negate();
}

void QMcomputeNormal(FXVec3d& normal,int count,FXPoint3d *v,short *ia,
	int orientation)
/****************************************************************************
*
* Function:		QMcomputeNormal
* Parameters:	normal		- Place to store the normal
*				count		- Number of vertices in polygon
*               v			- Array of polygon vertices
*				ia			- Index array into polygon
*				clockwise	- True if vertices are clockwise for front face
*
* Description:  Computes the normal for the polygon from the list of
*				vertices in the polygon. We use Martin Newell's method,
*				which is more robust than the 3 point method. See
*				Newman & Sproull page 499.
*
****************************************************************************/
{
	int	i,j;

	normal.x = normal.y = normal.z = 0;
	for (i = 0, j = i+1; i < count; i++,j++) {
		if (j == count)
			j = 0;
		normal.x -= (v[ia[i]].y - v[ia[j]].y) * (v[ia[i]].z + v[ia[j]].z);
		normal.y -= (v[ia[i]].z - v[ia[j]].z) * (v[ia[i]].x + v[ia[j]].x);
		normal.z -= (v[ia[i]].x - v[ia[j]].x) * (v[ia[i]].y + v[ia[j]].y);
		}
	normal.normalise();
	if (orientation != Quick3d::ClockWise)
		normal.negate();
}

void QMdrawWireframeBox(Quick3d& dc,const FXPoint3d& min,
	const FXPoint3d& max)
/****************************************************************************
*
* Function:		QMdrawWireframeBox
* Parameters:	dc			- Device context to render to
*               v0			- First vertex in polygon
*				v1			- Second vertex in polygon
*				v2			- Third vertex in polygon
*				clockwise	- True if vertices are clockwise for front face
*
* Description:  Computes the normal for the polygon from any three
*				vertices. If any of the three vertices are collinear
*				or cooincident, then this routine will crap out.
*
****************************************************************************/
{
	 dc.moveTo(min);
	 dc.lineTo(FXPoint3d(min.x,min.y,max.z));
	 dc.lineTo(FXPoint3d(min.x,max.y,max.z));
	 dc.lineTo(FXPoint3d(min.x,max.y,min.z));
	 dc.lineTo(min);
	 dc.lineTo(FXPoint3d(max.x,min.y,min.z));
	 dc.lineTo(FXPoint3d(max.x,min.y,max.z));
	 dc.lineTo(FXPoint3d(max.x,max.y,max.z));
	 dc.lineTo(FXPoint3d(max.x,max.y,min.z));
	 dc.lineTo(FXPoint3d(max.x,min.y,min.z));

	 dc.moveTo(FXPoint3d(min.x,max.y,min.z));
	 dc.lineTo(FXPoint3d(max.x,max.y,min.z));

	 dc.moveTo(FXPoint3d(min.x,max.y,max.z));
	 dc.lineTo(FXPoint3d(max.x,max.y,max.z));

	 dc.moveTo(FXPoint3d(min.x,min.y,max.z));
	 dc.lineTo(FXPoint3d(max.x,min.y,max.z));
}
