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
* Description:	Member functions for the MVBitmapButton classes.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mbitbtn.hpp"

/*----------------------------- Implementation ----------------------------*/

MVSysBitmapButton::MVSysBitmapButton(MGLDevCtx& dc,const MVRect& bounds,
	ushort command,int *indexes,ushort flags,ibool selected)
	: MVButtonBase(dc,bounds,command,flags,selected)
/****************************************************************************
*
* Function:		MVSysBitmapButton::MVSysBitmapButton
* Parameters:	bounds		- Bounding rectangle for the button
*				command		- Command code to send when activated
*				flags		- Flags for the button
*
****************************************************************************/
{
	for (int i = 0; i < 32; i++)
		index[i] = indexes[i];
}

void MVSysBitmapButton::drawBody(const MVRect& clip,ushort bodyState)
/****************************************************************************
*
* Function:		MVSysBitmapButton::drawBody
* Parameters:	clip		- Clipping rectangle to draw the button with
*				depressed	- True if button is depressed
*
* Description:	Internal routine for the button class to draw the button
*				body in the current state. This can be called by all
*				subclasses of button to draw a default button body.
*
*				Note that this routine draws the button in the context
*				of the owner's viewport.
*
****************************************************************************/
{
	MS_obscure();
	setClipRect(clip);
	MV_putSysBitmap(dc,bounds.left(),bounds.top(),index[bodyState]);
	MS_show();
}

MVBitmapButton::MVBitmapButton(MGLDevCtx& dc,const MVRect& bounds,
	ushort command,bitmap_t *bmps[],ushort flags,ibool selected)
	: MVButtonBase(dc,bounds,command,flags,selected)
/****************************************************************************
*
* Function:		MVBitmapButton::MVBitmapButton
* Parameters:	bounds		- Bounding rectangle for the button
*				command		- Command code to send when activated
*				flags		- Flags for the button
*
****************************************************************************/
{
	for (int i = 0; i < 32; i++)
		bmp[i] = bmps[i];
}

void MVBitmapButton::drawBody(const MVRect& clip,ushort bodyState)
/****************************************************************************
*
* Function:		MVBitmapButton::drawBody
* Parameters:	clip		- Clipping rectangle to draw the button with
*				depressed	- True if button is depressed
*
* Description:	Internal routine for the button class to draw the button
*				body in the current state. This can be called by all
*				subclasses of button to draw a default button body.
*
*				Note that this routine draws the button in the context
*				of the owner's viewport.
*
****************************************************************************/
{
	MS_obscure();
	setClipRect(clip);
	MGL_putBitmap(dc,bounds.left(),bounds.top(),bmp[bodyState],MGL_REPLACE_MODE);
	MS_show();
}
