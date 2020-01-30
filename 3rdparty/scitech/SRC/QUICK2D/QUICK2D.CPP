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
* Description:  Main routines for the Quick2d class.
*
*
****************************************************************************/

#include "quick2d/quick2d.hpp"

/*---------------------------- Implementation -----------------------------*/

Quick2d::Quick2d(MGLDC *mgldc,int viewStackSize,uint bufSize)
	: MGLDevCtx(mgldc), viewStack(viewStackSize), bufSize(bufSize),
	  buf(new char[bufSize])
/****************************************************************************
*
* Function:		Quick2d::Quick2d
* Parameters:	dc				- MGL device context to use
*				viewStackSize	- Size of the viewport stack to allocate
*               bufSize			- Size of internal buffer to allocate
*
****************************************************************************/
{
	// Create the default viewing transformation, to map onto the currently
	// active viewport.
	worldToView(FXintToReal(maxx()),FXintToReal(maxy()),
		REAL(-1),REAL(1),REAL(-1),REAL(1));

	// Default to 40 segments in a circle
	csegs = 40;
}

Quick2d::~Quick2d()
{
	delete [] (char*)buf;
}

void Quick2d::buildTotalMapping()
/****************************************************************************
*
* Function:		Quick2d::buildTotalMapping
*
* Description:	Updates the value for the total mapping matrix, by
*				combining the values in the modelView and worldView
*				transformations. This will be done only once before a
*				set of points is transformed, to speed up the transformation
*				process. Note that we also add in the viewPort origin
*				value as a simple scalar translation of the final resulting
*               matrix.
*
****************************************************************************/
{
	m.concat(worldView,modelView);
	m(0,2) += viewPortOrg.x;
	m(1,2) += viewPortOrg.y;
	validMapping = true;
}

void Quick2d::setUserModelView(const FXForm2d& m)
/****************************************************************************
*
* Function:		Quick2d::setUserModelView
* Parameters:	m	- New modelView transform
*
****************************************************************************/
{
	modelView = m;
	validMapping = false;
}

void Quick2d::setUserWorldView(const FXForm2d& m)
/****************************************************************************
*
* Function:		Quick2d::setUserWorldView
* Parameters:	m	- New worldView transform
*
****************************************************************************/
{
	worldView = m;
	viewPortOrg.x = viewPortOrg.y = 0;
	validMapping = false;
}

void Quick2d::concat(const FXForm2d& map)
/****************************************************************************
*
* Function:		Quick2d::concat
* Parameters:	map	- Matrix to concatenate
*
* Description:	Pre-multiplies the current modelView matrix with a specified
*				3x3 transformation matrix. This means that you must specify
*				the transformations to be applied in REVERSE order.
*
*				ie: To do a scale then a rotate, do the following:
*
*					dc.rotate(...)
*					dc.scale(...)
*
****************************************************************************/
{
	FXForm2d	result;
	result.concat(modelView,map);
	modelView = result;
	validMapping = false;
}

void Quick2d::worldToView(real NDC_maxx,real NDC_maxy,real Xmin,real Xmax,
	real Ymin,real Ymax,ibool keepSquare)
/****************************************************************************
*
* Function:		Quick2d::worldToView
* Parameters:	NDC_maxx	- Maximum viewport x coordinate
*				NDC_maxy	- Maximum viewport y coordinate
*				Xmin		- Minimum world x coordinate
*				Xmax		- Maximum world x coordinate
*				Ymin		- Minimum world y coordinate
*				Ymax		- Maximum world y coordinate
*				keepSquare	- True if transform should be kept square
*
* Description:	Builds a matrix to map points from viewport coordinates
*				to world coordinates on the top of the viewing stack. It
*				assumes that the minimum viewport coordinate is always
*				(0,0), and that points are mapped into the viewport with
*				(0,0) being the top left hand corner.
*
*				If keepSquare is true, we scale the windows x coordinates
*				to fit the size of the viewport, taking into account
*				the given output device's pixel aspect ratio to ensure that
*				squares appear as true squares.
*
****************************************************************************/
{
	if (keepSquare) {
		// Find the centre of the viewing window, and scale coordinates
		// about this point in the x axis.

		real centre_x = FXmul((Xmax + Ymin),REAL(0.5));
		real factor = FXmul(FXdiv(NDC_maxx,NDC_maxy),
			FXdiv(REAL(1000),FXintToReal(getAspectRatio())));

		Xmin = centre_x + FXmul(Xmin - centre_x, factor);
		Xmax = centre_x + FXmul(Xmax - centre_x, factor);
		}
	worldView.worldToView(NDC_maxx,NDC_maxy,Xmin,Xmax,Ymin,Ymax);
	modelView.identity();
	viewPortOrg.x = viewPortOrg.y = 0;
	validMapping = false;
}

void Quick2d::setViewportOrg(real x,real y)
/****************************************************************************
*
* Function:		Quick2d::setViewportOrg
* Parameters:	x,y	- New viewport origin
*
* Description:	Sets the viewport origin. The viewport origin is a specific
*               viewport pixel coordinate offset that can be used to change
*				the logical viewport origin. It is applied after all
*				transformations, so is measure in units of physical device
*				pixels.
*
****************************************************************************/
{
	viewPortOrg.x = x;
	viewPortOrg.y = y;
	validMapping = false;
}

void Quick2d::push()
/****************************************************************************
*
* Function:		Quick2d::push
*
* Description:	Pushes the current view mapping matrix onto the top of the
*				view stack. If the viewing stack overflows, we bomb
*				out with an error message.
*
****************************************************************************/
{
	if (viewStack.isFull())
		MGL_fatalError("2D viewing stack overflow...");
	viewStack.pushq(modelView);
}

void Quick2d::pop()
/****************************************************************************
*
* Function:		Quick2d::pop
*
* Description:	Pops the current view mapping matrix off the top of the
*				stack and makes it the new mapping matrix. If the viewing
*				stack underflows, we bomb out with an error message.
*
****************************************************************************/
{
	if (viewStack.isEmpty())
		MGL_fatalError("2D viewing stack underflow...");
	modelView = viewStack.popq();
	validMapping = false;
}
