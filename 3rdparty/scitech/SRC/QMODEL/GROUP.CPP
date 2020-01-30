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
* Description:	Implementation for the QMGroup class.
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/group.hpp"

/*---------------------------- Implementation -----------------------------*/

void QMGroup::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMGroup::renderShaded
* Parameters:	dc	- Device context to render to
*
* Description:	Render all children in group.
*
****************************************************************************/
{
	dc.beginObject();
	for (QMObjListIter i(children); i; i++)
		i.node()->renderShaded(dc);
	dc.endObject();
}

void QMGroup::renderWireframe(Quick3d& dc)
/****************************************************************************
*
* Function:		QMGroup::renderWireframe
* Parameters:	dc	- Device context to render to
*
* Description:	Render all children in group.
*
****************************************************************************/
{
	dc.beginObject();
	for (QMObjListIter i(children); i; i++)
		i.node()->renderWireframe(dc);
	dc.endObject();
}

void QMGroup::preProcess()
/****************************************************************************
*
* Function:		QMGroup::preProcess
*
* Description:	Pre-process all children in the group
*
****************************************************************************/
{
	for (QMObjListIter i(children); i; i++)
		i.node()->preProcess();
}

void QMGroup::getBounds(FXPoint3d& min,FXPoint3d& max)
/****************************************************************************
*
* Function:		QMGroup::getBounds
* Parameters:	min	- Place to store minimum coordinate
*				max	- Place to store maximum coordinate
*
* Description:	Finds the bounding box for the polyhedron.
*
****************************************************************************/
{
	FXPoint3d		mn,mx;
	QMObjListIter	i(children);

	if (i)
		i.node()->getBounds(min,max);
	while (++i) {
		i.node()->getBounds(mn,mx);
		min.x = MIN(min.x,mn.x);
		min.y = MIN(min.y,mn.y);
		min.z = MIN(min.z,mn.z);
		max.x = MAX(max.x,mx.x);
		max.y = MAX(max.y,mx.y);
		max.z = MAX(max.z,mx.z);
		}
}

void QMGroup::computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool viewChanged)
/****************************************************************************
*
* Function:		QMGroup::computeDirtyRect
* Parameters:	dc		- Device context to use
*				dirty	- Place to store dirty rectangle
*
* Description:	Iterates through each child in the list determining the
*				union of all dirty rectangle regions.
*
****************************************************************************/
{
	MGLRect	d;

	for (QMObjListIter i(children); i; i++) {
		i.node()->computeDirtyRect(dc,d,viewChanged);
		dirty += d;
		}
}
