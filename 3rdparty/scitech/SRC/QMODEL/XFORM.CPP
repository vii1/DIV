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
* Description:	Implementation for the QMTransform class.
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/xform.hpp"

/*---------------------------- Implementation -----------------------------*/

QMTransform::QMTransform(QMObject* child,ibool ownChild) : QMObject(),
	child(child), ownChild(ownChild)
/****************************************************************************
*
* Function:		QMTransform::QMTransform
* Parameters:	child	- Child object to bind the transform to
*
****************************************************************************/
{
	xform.identity();
}

void QMTransform::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMTransform::renderShaded
* Parameters:	dc	- Device context to render to
*
* Description:	Render all children in group.
*
****************************************************************************/
{
	beginRender(dc);
	child->renderShaded(dc);
	endRender(dc);
}

void QMTransform::renderWireframe(Quick3d& dc)
/****************************************************************************
*
* Function:		QMTransform::renderWireframe
* Parameters:	dc	- Device context to render to
*
* Description:	Render all children in group.
*
****************************************************************************/
{
	beginRender(dc);
	child->renderWireframe(dc);
	endRender(dc);
}

void QMTransform::preProcess()
/****************************************************************************
*
* Function:		QMTransform::preProcess
*
* Description:	Pre-process all children in the group
*
****************************************************************************/
{
	child->preProcess();
	changed = true;
	childIsPrim = child->isPrimitive();
}

void QMTransform::getBounds(FXPoint3d& min,FXPoint3d& max)
{
	child->getBounds(min,max);
	xform.map(min);
	xform.map(max);
}

void QMTransform::computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool viewChanged)
/****************************************************************************
*
* Function:		QMTransform::computeDirtyRect
* Parameters:	dc		- Device context to use
*				dirty	- Place to store dirty rectangle
*
* Description:	Transform the current view into the new object space
*				coordinates and compute the dirty rectangle for the child.
*
****************************************************************************/
{
	if (changed || viewChanged || !childIsPrim) {
		dc.push();
		dc.concat(xform);
		child->computeDirtyRect(dc,dirty,viewChanged);
		dc.pop();
		}
	else
		dirty.empty();
}
