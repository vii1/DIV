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
* Description:	Header file for the QMBoundingBox class.
*
*
****************************************************************************/

#ifndef	__QMODEL_BNDBOX_HPP
#define	__QMODEL_BNDBOX_HPP

#ifndef	__QMODEL_PRIMITIV_HPP
#include "qmodel/primitiv.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Class to wrap a bounding box around an object or set of objects for
// efficient dirty rectangle calculations.
//---------------------------------------------------------------------------

class QMBoundingBox : public QMObject {
protected:
	ibool		preProcessed;	// True if been pre-processed
	FXPoint3d	min,max;		// Cached bounding box for child
	QMPrimitive	*child;			// Child primitive to wrap box around

			void drawBounds(Quick3d& dc)
			{
				if (QMrenderBounds) {
					dc.setColor(QMboundsColor);
					QMdrawWireframeBox(dc,min,max);
					}
			};

public:
			// Constructor
			QMBoundingBox(QMPrimitive *child) : QMObject(),
				preProcessed(false), child(child) {};

			// Destructor
	virtual	~QMBoundingBox();

			// Method to determine if a node is a primitive
	virtual	ibool isPrimitive() const { return child->isPrimitive(); };

			// Methods to render the object
	virtual void renderShaded(Quick3d& dc);
	virtual void renderWireframe(Quick3d& dc);

			// Method to pre-process the object prior to rendering
	virtual	void preProcess();

			// Method to return the bounds of the object
	virtual	void getBounds(FXPoint3d& min,FXPoint3d& max);

			// Method to compute the dirty rectangle for the object
	virtual	void computeDirtyRect(Quick3d& dc,MGLRect& dirty,ibool viewChanged);
	};

#endif	// __QMODEL_BNDBOX_HPP
