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
* Description:	Header file for the MVTBButton classes for button to go
*				on toolbars.
*
*
****************************************************************************/

#ifndef	__MVIS_MTBBTN_HPP
#define	__MVIS_MTBBTN_HPP

#ifndef	__MVIS_MBUTTON_HPP
#include "mvis/mbutton.hpp"
#endif

/*------------------------------ Constants --------------------------------*/

/*-------------------------- Class definitions ----------------------------*/

//---------------------------------------------------------------------------
// The following is the base class for all toolbar button types. It basically
// overrides the default drawing for the button and handles drawing the
// proper border around the button body. It is subclassed by either the
// glyph or bitmap button toolbar button classes to create real classes that
// can be used in practice.
//---------------------------------------------------------------------------

class MVTBButton : public MVButtonBase {
protected:
			// Internal method to draw the button body in specified state
	virtual	void drawBody(const MVRect& clip,ushort bodyState);

public:
			// Constructor
			MVTBButton(MGLDevCtx& dc,const MVRect& bounds,ushort command,
				ushort flags = bfNormal,ibool selected = false)
				: MVButtonBase(dc,bounds,command,flags,selected) {};
	};

//---------------------------------------------------------------------------
// The following is a class for representing toolbar buttons that have the
// face drawn as a monochrome glyph from the current system glyph font.
//---------------------------------------------------------------------------

class MVTBGlyphButton : public MVTBButton {
protected:
	char        glyph;			// System glyph for face of button

			// Internal method to draw the button body in specified state
	virtual	void drawBody(const MVRect& clip,ushort bodyState);

public:
			// Constructor
			MVTBGlyphButton(MGLDevCtx& dc,const MVRect& bounds,ushort command,
				char glyph,ushort flags = bfNormal,ibool selected = false)
				: MVTBButton(dc,bounds,command,flags,selected),
				  glyph(glyph) {};

			// Set or get the current glyph
			char getGlyph()			{ return glyph; };
			void setGlyph(char g);
	};

//---------------------------------------------------------------------------
// The following is a class for representing toolbar buttons that have the
// face drawn as a color bitmap. The button bounds shrink to fit the
// bitmap that it holds.
//---------------------------------------------------------------------------

class MVTBBitmapButton : public MVTBButton {
protected:
	char        index;			// System bitmap index (-1 for user bitmap)
	MGLBitmap	bmp;			// Optional user supplied bitmap

			// Internal method to draw the button body in specified state
	virtual	void drawBody(const MVRect& clip,ushort bodyState);

public:
			// Constructor
			MVTBBitmapButton(MGLDevCtx& dc,const MVRect& bounds,
				ushort command,char index,ushort flags = bfNormal,
				ibool selected = false);
			MVTBBitmapButton(MGLDevCtx& dc,const MVRect& bounds,
				ushort command,const char *bitmapName,ushort flags = bfNormal,
				ibool selected = false);
			MVTBBitmapButton(MGLDevCtx& dc,const MVRect& bounds,
				ushort command,const MGLBitmap& bitmap,ushort flags = bfNormal,
				ibool selected = false);
			MVTBBitmapButton(MGLDevCtx& dc,const MVRect& bounds,
				ushort command,bitmap_t *bitmap,ushort flags = bfNormal,
				ibool selected = false);
	};

#endif	// __MVIS_MTBBTN_HPP
