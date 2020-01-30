/****************************************************************************
*
*			  Quick3D - A 3D C++ rendering pipeline for the MGL
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
* Description:  3d polygon convenience functions.
*
*				NOTE! Should try to re-write these routines to bypass
*					  the vertex passing routines and transform directly
*					  into the pipeline to speed up the rendering
*					  process. This would make them the fastest possible
*					  methods for rendering polygons (for OpenGL
*					  implementations we would simply use code similar to
*					  the current code).
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick3d::polyLine(int count,const FXPoint3d *vArray)
/****************************************************************************
*
* Function:		Quick3d::polyLine
* Parameters:	count	- Number of points in array
*				vArray	- Vertex array of points
*
* Description:	Draws a polyline in the current drawing attributes.
*
****************************************************************************/
{
	moveTo(*vArray++);
	while (--count)
		lineTo(*vArray++);
}

void Quick3d::fillPolygon(int count,const FXPoint3d *vArray)
/****************************************************************************
*
* Function:		Quick3d::fillPolygon
* Parameters:	count	- Number of points in array
*				vArray	- Vertex array of points
*
* Description:	Draws a polygon in the current drawing attributes.
*
****************************************************************************/
{
	beginPoly();
	while (count--)
		vertex(*vArray++);
	endPoly();
}

void Quick3d::fillPolygon(int count,const FXPoint3d *vArray,
	const FXVec3d *nArray)
/****************************************************************************
*
* Function:		Quick3d::fillPolygon
* Parameters:	count	- Number of points in array
*				vArray	- Vertex array of points
*				nArray	- Vertex normal array of vectors
*
* Description:	Draws a polygon in the current drawing attributes, given
*				a list of vertices and vertex normals.
*
****************************************************************************/
{
	beginPoly();
	while (count--) {
		normal(*nArray++);
		vertex(*vArray++);
		}
	endPoly();
}
