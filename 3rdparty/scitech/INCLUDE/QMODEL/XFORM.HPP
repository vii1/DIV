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
* Description:	Header file for the QMTransform class.
*
*
****************************************************************************/

#ifndef	__QMODEL_XFORM_HPP
#define	__QMODEL_XFORM_HPP

#ifndef	__QMODEL_OBJECT_HPP
#include "qmodel/object.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Class to represent a set of transformations bound to a primitive.
//---------------------------------------------------------------------------

class QMTransform : public QMObject {
protected:
	QMObject	*child;		// Child transform is bound to
	FXForm3d	xform;		// Tranformation to apply
	char		changed;	// True if transform changed (reset on render)
	char		childIsPrim;// Cache for the isPrimtive() function
	char        ownChild;	// Do we own the child object?

			// Begin and end rendering
			void beginRender(Quick3d& dc) { dc.push(); dc.concat(xform); };
			void endRender(Quick3d& dc) { dc.pop(); changed = false; };

public:

			// Constructor
			QMTransform(QMObject *child,ibool ownChild = true);

			// Destructor
	virtual	~QMTransform() { if (ownChild) delete child; };

			// Create an identity transformation
			void identity(void);

			// Concatenate a scale transformation about the origin
			void scale(real xscale,real yscale,real zscale);

			// Concatenate a translation transformation
			void translate(real xtrans,real ytrans,real ztrans);
			void translate(const FXPoint3d& p);

			// Concatenate a rotation transformation about the x,y,z axes
			void rotatex(real angle);
			void rotatey(real angle);
			void rotatez(real angle);

			// Concatenate a rotation about an arbitrary axis through origin
			void rotate(real angle,real x,real y,real z);
			void rotate(real angle,const FXVec3d& axis);

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

/*------------------------- Inline member functions -----------------------*/

inline void QMTransform::identity()
{
	xform.identity();
	changed = true;
}

inline void QMTransform::scale(real xscale,real yscale,real zscale)
{
	xform.scale(xscale,yscale,zscale);
	changed = true;
}

inline void QMTransform::translate(real xtrans,real ytrans,real ztrans)
{
	xform.translate(xtrans,ytrans,ztrans);
	changed = true;
}

inline void QMTransform::translate(const FXPoint3d& p)
{
	xform.translate(p.x,p.y,p.z);
	changed = true;
}

inline void QMTransform::rotatex(real angle)
{
	xform.rotatex(angle);
	changed = true;
}

inline void QMTransform::rotatey(real angle)
{
	xform.rotatey(angle);
	changed = true;
}

inline void QMTransform::rotatez(real angle)
{
	xform.rotatez(angle);
	changed = true;
}

inline void QMTransform::rotate(real angle,real x,real y,real z)
{
	xform.rotate(angle,x,y,z);
	changed = true;
}

inline void QMTransform::rotate(real angle,const FXVec3d& axis)
{
	xform.rotate(angle,axis.x,axis.y,axis.z);
	changed = true;
}

#endif	// __QMODEL_XFORM_HPP
