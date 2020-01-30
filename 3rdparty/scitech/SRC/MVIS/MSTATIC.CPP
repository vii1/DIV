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
* Description:	Member functions for the MVStaticIcon class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mstatic.hpp"

/*----------------------------- Implementation ----------------------------*/

MVStaticIcon::MVStaticIcon(MGLDevCtx& dc,const MVPoint& loc,
	const char *iconName)
	: MVView(dc,MVRect(0,0,0,0)), icon(iconName)
/****************************************************************************
*
* Function:		MVStaticIcon::MVStaticIcon
* Parameters:   dc			- Device context to draw to
*				loc			- Location to display the icon at
*				iconName	- Name of the icon to load
*
* Description:	Constructor for the MVStaticIcon class.
*
****************************************************************************/
{
	if (icon.valid()) {
		setBounds(MVRect(loc.x,loc.y,
			loc.x + icon.getIcon()->xorMask.width,
			loc.y + icon.getIcon()->xorMask.height));
		}
}

MVStaticIcon::MVStaticIcon(MGLDevCtx& dc,const MVPoint& loc,
	const MGLIcon& i)
	: MVView(dc,MVRect(0,0,0,0)), icon(i)
/****************************************************************************
*
* Function:		MVStaticIcon::MVStaticIcon
* Parameters:   dc			- Device context to draw to
*				loc			- Location to display the icon at
*				iconName	- Name of the icon to load
*
* Description:	Constructor for the MVStaticIcon class.
*
****************************************************************************/
{
	if (icon.valid()) {
		setBounds(MVRect(loc.x,loc.y,
			loc.x + icon.getIcon()->xorMask.width,
			loc.y + icon.getIcon()->xorMask.height));
		}
}

MVStaticIcon::MVStaticIcon(MGLDevCtx& dc,const MVPoint& loc,
	icon_t *i)
	: MVView(dc,MVRect(0,0,0,0)), icon(i)
/****************************************************************************
*
* Function:		MVStaticIcon::MVStaticIcon
* Parameters:   dc			- Device context to draw to
*				loc			- Location to display the icon at
*				iconName	- Name of the icon to load
*
* Description:	Constructor for the MVStaticIcon class.
*
****************************************************************************/
{
	if (icon.valid()) {
		setBounds(MVRect(loc.x,loc.y,
			loc.x + icon.getIcon()->xorMask.width,
			loc.y + icon.getIcon()->xorMask.height));
		}
}

void MVStaticIcon::draw(const MVRect&)
/****************************************************************************
*
* Function:		MVStaticIcon::draw
* Parameters:	clip	- Clipping rectangle for the view.
*
* Description:	Draw's the static text item.
*
****************************************************************************/
{
	if (icon.valid()) {
		// Clear the icon background
		dc.setColor(getColor(scBackground));
		dc.fillRect(bounds);
		dc.putIcon(bounds.left(),bounds.top(),icon);
		}
}

MVStaticBitmap::MVStaticBitmap(MGLDevCtx& dc,const MVRect& bounds,
	const char *bitmapName,ushort flags)
	: MVView(dc,bounds), bitmap(bitmapName), flags(flags)
/****************************************************************************
*
* Function:		MVStaticBitmap::MVStaticBitmap
* Parameters:   dc			- Device context to draw to
*				loc			- Location to display the bitmap at
*				bitmapName	- Name of the bitmap to load
*
* Description:	Constructor for the MVStaticBitmap class.
*
****************************************************************************/
{
	if (bitmap.valid()) {
		if (flags & sfFitBitmap) {
			setBounds(MVRect(bounds.left(),bounds.top(),
				bounds.left() + bitmap.getBitmap()->width,
				bounds.top() + bitmap.getBitmap()->height));
			}
		}
}

MVStaticBitmap::MVStaticBitmap(MGLDevCtx& dc,const MVRect& bounds,
	const MGLBitmap& bmp,ushort flags)
	: MVView(dc,bounds), bitmap(bmp), flags(flags)
/****************************************************************************
*
* Function:		MVStaticBitmap::MVStaticBitmap
* Parameters:   dc			- Device context to draw to
*				loc			- Location to display the bitmap at
*				bitmapName	- Name of the bitmap to load
*
* Description:	Constructor for the MVStaticBitmap class.
*
****************************************************************************/
{
	if (bitmap.valid()) {
		if (flags & sfFitBitmap) {
			setBounds(MVRect(bounds.left(),bounds.top(),
				bounds.left() + bitmap.getBitmap()->width,
				bounds.top() + bitmap.getBitmap()->height));
			}
		}
}

MVStaticBitmap::MVStaticBitmap(MGLDevCtx& dc,const MVRect& bounds,
	bitmap_t *bmp,ushort flags)
	: MVView(dc,bounds), bitmap(bmp), flags(flags)
/****************************************************************************
*
* Function:		MVStaticBitmap::MVStaticBitmap
* Parameters:   dc			- Device context to draw to
*				loc			- Location to display the bitmap at
*				bitmapName	- Name of the bitmap to load
*
* Description:	Constructor for the MVStaticBitmap class.
*
****************************************************************************/
{
	if (bitmap.valid()) {
		if (flags & sfFitBitmap) {
			setBounds(MVRect(bounds.left(),bounds.top(),
				bounds.left() + bitmap.getBitmap()->width,
				bounds.top() + bitmap.getBitmap()->height));
			}
		}
}

void MVStaticBitmap::draw(const MVRect&)
/****************************************************************************
*
* Function:		MVStaticBitmap::draw
* Parameters:	clip	- Clipping rectangle for the view.
*
* Description:	Draw's the static text item.
*
****************************************************************************/
{
	if (bitmap.valid()) {
		if (flags & sfStretchBitmap)
			dc.stretchBitmap(bounds,bitmap);
		else {
			if (!(flags & sfFitBitmap)) {
				// Clear the bitmap background
				dc.setColor(getColor(scBackground));
				dc.fillRect(bounds);
				}
			dc.putBitmap(bounds.left(),bounds.top(),bitmap,MGL_REPLACE_MODE);
			}
		}
}
