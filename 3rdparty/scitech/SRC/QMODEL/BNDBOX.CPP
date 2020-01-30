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
* Description:	Implementation for the bounding box class
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/bndbox.hpp"

/*---------------------------- Implementation -----------------------------*/

QMBoundingBox::~QMBoundingBox()
/****************************************************************************
*
* Function:		QMBoundingBox::~QMBoundingBox
*
* Description:	Deletes the child that the bounding box is bound to.
*
****************************************************************************/
{
	delete child;
}

void QMBoundingBox::renderShaded(Quick3d& dc)
/****************************************************************************
*
* Function:		QMBoundingBox::renderShaded
* Parameters:	dc	- Device context
*
****************************************************************************/
{
	drawBounds(dc);
	child->renderShaded(dc);
}

void QMBoundingBox::renderWireframe(Quick3d& dc)
/****************************************************************************
*
* Function:		QMBoundingBox::renderWireframe
* Parameters:	dc	- Device context
*
****************************************************************************/
{
	drawBounds(dc);
	child->renderWireframe(dc);
}

void QMBoundingBox::preProcess()
/****************************************************************************
*
* Function:		QMBoundingBox::preProcess
*
* Description:  Pre-process the bounding box class. First we pre-process
*				the child object and then cache it's bounding box.
*
****************************************************************************/
{
	child->preProcess();
	child->getBounds(min,max);
	preProcessed = true;
}

void QMBoundingBox::getBounds(FXPoint3d& _min,FXPoint3d& _max)
/****************************************************************************
*
* Function:		QMBoundingBox::getBounds
* Parameters:   min	- Minimum coordinate for bounds
*				max	- Maximum coordinate for bounds
*
****************************************************************************/
{
	if (preProcessed) {
		_min = min;
		_max = max;
		}
	else {
		child->getBounds(_min,_max);
		}
}

void QMBoundingBox::computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool)
/****************************************************************************
*
* Function:		QMBoundingBox::computeDirtyRect
* Parameters:   dc		- 3D device context
*				dirty	- Place to store dirty rectangle
*
* Description:	Computes the dirty rectangle using the cached bounding
*				box for the object. The results of this will be invalid
*				if the scene has not been preprocessed since the last
*				time the scene or viewing parameters changed.
*
****************************************************************************/
{
	dc.computeDirtyRect(min,max,dirty);
}
