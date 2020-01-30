/****************************************************************************
*
*					 MegaVision Application Framework
*
*			A C++ GUI Toolkit for the MegaGraph Graphics Library
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
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MVRenderArea class. This class is used
*				to represent a special type of view that is used for
*				performing double buffered animation in a window. It is
*				an abstract base class that relies upon a user suppiled
*				render() method to render the actual image in the viewing
*				area. It automatically takes care of setting up and
*				maintaining the double buffered view (if double buffering
*				is available).
*
*
****************************************************************************/

#ifndef	__MVIS_MRENDERA_HPP
#define	__MVIS_MRENDERA_HPP

#ifndef	__MVIS_MVIEW_HPP
#include "mvis/mview.hpp"
#endif

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVRenderArea class is an abstract base class used to represent
// an area for peforming double buffered animation.
//---------------------------------------------------------------------------

class MVRenderArea : public MVView {
protected:
	ibool		doubleBuffered;		// True if double buffering is on

			// Method to swap the display buffers
			void swapBuffers(ibool waitVRT = true)
			{
				if (doubleBuffered)
					dc.swapBuffers(waitVRT);
			};

			// Method to render the display and swap buffers
			void renderAndSwap(ibool waitVRT = true)
			{ render(); swapBuffers(waitVRT); };

			// Method to turn double buffering on
			void doubleBufferOn();

			// Method to turn double buffering off
			void doubleBufferOff();

			// Clip and render against a single rectangle
	static	MVRenderArea *clipAndRenderView;
	static	void _ASMAPI clipAndRender(const rect_t *r);

			// Paint the view clipping to clip region (calls render)
	virtual	void paint();

public:
			// Constructor
			MVRenderArea(MGLDevCtx& dc,const MVRect& bounds);

			// The draw function is not used for the MVRenderArea
	virtual	void draw(const MVRect&) {};

			// Method to render a user defined frame
	virtual	void render() = 0;
	};

#endif	// __MVIS_MRENDERA_HPP
