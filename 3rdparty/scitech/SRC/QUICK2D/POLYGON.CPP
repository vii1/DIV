/****************************************************************************
*
*			 Quick 2D - A 2D C++ tranformation class for the MGL
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
* Description:  2d polygon drawing routines.
*
*
****************************************************************************/

#include "quick2d/quick2d.hpp"

/*---------------------------- Implementation -----------------------------*/

void Quick2d::fillPolygon(int count,const FXPoint2d *vArray)
/****************************************************************************
*
* Function:		Quick2d::fillPolygon
* Parameters:	count	- Number of vertices in the polygon
*				vArray	- Pointer to the array of vertices
* Returns:		True if the polygon was scan converted, false on error.
*
* Description:	Fills the polygon desribed by the array of points on the
*				screen. If the polygon is know to be convex, you can set
*				the polygon type to convex by calling the MGL_setPolyStyle()
*				routine.
*
****************************************************************************/
{
	int			i;
	fxpoint_t	*NDC_array;
	FXPoint2d	r;

	// Get the memory buffer required to transform the polygon's vertices
	if ((i = count) * sizeof(fxpoint_t) > bufSize)
		MGL_fatalError("To many vertices in polygon");
	NDC_array = (fxpoint_t*)buf;

	// Transform all the points in the polygon
	validateTotalMapping();
	while (i--)
		map(*NDC_array++,r,*vArray++);

	MGLDevCtx::fillPolygon(count,(fxpoint_t*)buf,sizeof(fxpoint_t),0,0);
}
