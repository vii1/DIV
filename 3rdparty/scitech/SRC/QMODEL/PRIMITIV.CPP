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

#include "qmodel/primitiv.hpp"

/*---------------------------- Implementation -----------------------------*/

void QMPrimitive::computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool)
/****************************************************************************
*
* Function:		QMPrimitive::computeDirtyRect
* Parameters:   dc		- 3D device context
*				dirty	- Place to store dirty rectangle
*
* Description:	Computes the dirty rectangle by first computing the
*				bounds for the objects and then using these bounds to
*				get the dirty rectangle. This is the default and can be
*				sped up at the expense of speed by binding a QMBoundingBox
*				to the object.
*
****************************************************************************/
{
	FXPoint3d min,max;
	getBounds(min,max);
	dc.computeDirtyRect(min,max,dirty);
}
