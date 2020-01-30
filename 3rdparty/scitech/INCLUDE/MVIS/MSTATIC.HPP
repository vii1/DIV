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
* Description:	Header file for the static window classes.
*
*
****************************************************************************/

#ifndef	__MVIS_MSTATIC_HPP
#define	__MVIS_MSTATIC_HPP

#ifndef	__MVIS_MVIEW_HPP
#include "mvis/mview.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

ushort const

// Static bitmap creation flags

	sfStretchBitmap	= 0x0001,		// Bitmap is stretched to bounding box
	sfFitBitmap		= 0x0002;		// Bounds are fitted to bitmap

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The MVStaticIcon class is used to display an icon in a Window. The icon
// is displayed at a set location and will expand to fit the icon.
//---------------------------------------------------------------------------

class MVStaticIcon : public MVView {
protected:
	MGLIcon		icon;			// Icon to display in the window

public:
			// Constructor
			MVStaticIcon(MGLDevCtx& dc,const MVPoint& loc,
				const char *iconName);
			MVStaticIcon(MGLDevCtx& dc,const MVPoint& loc,
				const MGLIcon& icon);
			MVStaticIcon(MGLDevCtx& dc,const MVPoint& loc,
				icon_t *icon);

			// Method to draw the static icon item
	virtual	void draw(const MVRect& dirty);
	};

//---------------------------------------------------------------------------
// The MVStaticBitmap class is used to display a bitmap in a Window. The
// bitmap is displayed at a set location and can either be stretched to fit
// the bounding box, clipped to the bounding box or have the bounding box
// expand to fit the bitmap.
//---------------------------------------------------------------------------

class MVStaticBitmap : public MVView {
protected:
	MGLBitmap	bitmap;			// Bitmap to display in the window
	ushort		flags;			// Creation flags

public:
			// Constructor
			MVStaticBitmap(MGLDevCtx& dc,const MVRect& bounds,
				const char *bitmapName,ushort flags = 0);
			MVStaticBitmap(MGLDevCtx& dc,const MVRect& bounds,
				const MGLBitmap& bitmap,ushort flags = 0);
			MVStaticBitmap(MGLDevCtx& dc,const MVRect& bounds,
				bitmap_t *bitmap,ushort flags = 0);

			// Method to draw the static bitmap item
	virtual	void draw(const MVRect& dirty);
	};

#endif	// __MVIS_MSTATIC_HPP
