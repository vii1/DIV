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
* Description:	Implementation for the QMMaterial class.
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/material.hpp"

/*---------------------------- Implementation -----------------------------*/

QMMaterial::QMMaterial(QMObject *child,MGLMaterial *front,
	MGLMaterial *back,ibool ownChild)
	: QMObject(), child(child), front(front), back(back), ownChild(ownChild)
/****************************************************************************
*
* Function:		QMMaterial::QMMaterial
* Parameters:	child	- Child object to bind the materials to
*               front	- Front material
*				back	- Back material
*
* Description:	Creates a new material binding. Either of the front or
*				back material properties may be passed as NULL and they
*				will not be used in the binding process.
*
****************************************************************************/
{
}

void QMMaterial::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMMaterial::renderShaded
* Parameters:	dc	- Device context to render to
*
* Description:	Render all children in group.
*
****************************************************************************/
{
	MGLMaterial	*oldFront = dc.getMaterial();
	MGLMaterial *oldBack = dc.getBackMaterial();
	if (front)
		dc.material(front);
	if (back)
		dc.backMaterial(back);
	child->renderShaded(dc);
	if (front)
		dc.material(oldFront);
	if (back)
		dc.backMaterial(oldBack);
}

void QMMaterial::renderWireframe(Quick3d& dc)
/****************************************************************************
*
* Function:		QMMaterial::renderWireframe
* Parameters:	dc	- Device context to render to
*
* Description:	Render all children in group.
*
****************************************************************************/
{
	child->renderWireframe(dc);
}

void QMMaterial::preProcess()
{
	child->preProcess();
}

void QMMaterial::getBounds(FXPoint3d& min,FXPoint3d& max)
{
	child->getBounds(min,max);
}

void QMMaterial::computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool viewChanged)
{
	child->computeDirtyRect(dc,dirty,viewChanged);
}
