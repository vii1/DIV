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
* Description:	Member functions for the MVTitleBar class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mwindow.hpp"
#include "mvis/mfontmgr.hpp"

/*----------------------------- Implementation ----------------------------*/

MVTitleBar::MVTitleBar(MGLDevCtx& dc,const MVRect& bounds,const char *title,
	MVWindow *owner,ushort flags)
	: MVGroup(dc,bounds), title(MV_newStr(title)), owner(owner)
/****************************************************************************
*
* Function:		MVTitleBar::MVTitleBar
* Parameters:	bounds	- Bounding rectangle for the title bar
*				title	- Title text for the title bar
*				flags	- Window creation flags
*
* Description:	Constructor for the title bar.
*
****************************************************************************/
{
	int hjust = (flags & wfLeftJust ? MGL_LEFT_TEXT :
				(flags & wfRightJust ? MGL_RIGHT_TEXT : MGL_CENTER_TEXT));
	int vjust = (flags & wfTopJust ? MGL_TOP_TEXT :
				(flags & wfBottomJust ? MGL_BOTTOM_TEXT : MGL_CENTER_TEXT));
	tjust.setJustification(hjust,vjust);
	options &= ~ofClipToClient;	// Dont clip to client rect

	// Add close button
	closeBox = new MVTBGlyphButton(dc,MVRect(),cmClose,sgClose,bfBroadcast);
	if (!(flags & wfClose))
		closeBox->disable();
	insert(closeBox);

	// Add maximise/minimise buttons
	maximiseBox = NULL;
	minimiseBox = NULL;
	if (flags & wfResizeable) {
		if (flags & wfMaximise) {
			maximiseBox = new MVTBGlyphButton(dc,MVRect(),cmMaximise,
				owner && owner->isMaximised() ? sgRestore : sgMaximise,
				bfBroadcast);
			insert(maximiseBox);
			}
		if (flags & wfMinimise) {
			minimiseBox = new MVTBGlyphButton(dc,MVRect(),cmMinimise,
				sgMinimise,bfBroadcast);
			insert(minimiseBox);
			}
		}

	// Update the bounds for the titlebar
	setBounds(bounds);
}

void MVTitleBar::handleEvent(MVEvent& event,phaseType phase)
/****************************************************************************
*
* Function:		MVTitleBar::handleEvent
* Parameters:	event	- Event to handle
*				phase	- Current phase for the event (pre,focus,post)
*
* Description:	Event handling routine for the MVTitleBar class. Determine's
*				if the title bar has been hit, and handles the interaction
*				of the title bar icons.
*
****************************************************************************/
{
	MVGroup::handleEvent(event,phase);

	if (event.what == evBroadcast && event.which == this) {
		// Reflect menu bar button commands onto owner window
		MV_postMessage(owner,evCommand,event.message.command,event.message.infoPtr);
		}
	else if (event.what == evMouseDown && (event.mouse.buttons & mbLeftButton)) {
		// The title bar has been clicked, which indicates the user
		// wishes to move the window. A double click toggles between
		// a maximised and normal window.
		if (event.mouse.doubleClick)
			MV_postMessage(owner,evCommand,cmMaximise);
		else
			MV_postMessage(owner,evCommand,cmMove,MV_PASS_POINT(event.where));
/*		if (maximiseBox.command) {
			int newGlyph;
			if (owner && owner->isMaximised())
				newGlyph = sgRestore;
			else
				newGlyph = sgMaximise;
			if (maximiseBox.glyph != newGlyph) {
				maximiseBox.glyph = newGlyph;
				repaint(maximiseBox.bounds);
				}
			}*/
		clearEvent(event);
		}
}

void MVTitleBar::setBounds(const MVRect& bounds)
/****************************************************************************
*
* Function:		TTitlebar::setBounds
* Parameters:	bounds	- New bounding rectangle for the view
*
* Description:	Sets the bounding rectangle for the title bar. The bounding
*				rectangle will be set to the entire inner bounds for the
*				window, and we must adjust the size of it to the size of
*				the current title bar.
*
****************************************************************************/
{
	// Compute the height and width of the title bar text, and adjust the
	// bounding box for the title based on it's size.
	metrics_t	m;
	MVRect 		r(bounds);

	useFont(fmSystemFont);
	dc.getFontMetrics(m);
	r.bottom() = r.top() + (m.ascent-m.descent+1) + 10;
	MVGroup::setBounds(r);

	// Compute the location to draw the text at
	r.moveTo(0,0);
	r.inset(2,2);
	start.x = (tjust.hjust() == MGL_LEFT_TEXT ? 	r.left()+3 :
			  (tjust.hjust() == MGL_CENTER_TEXT ?  (r.left() + r.right())/2 :
													r.right()-4));
	start.y = (tjust.vjust() == MGL_TOP_TEXT ? 		r.top() :
			  (tjust.vjust() == MGL_CENTER_TEXT ?  (r.top() + r.bottom())/2 :
													r.bottom()-1 + m.descent));

	// Create the minimise/maximise boxes
	r.inset(1,1);
	r.left() = r.right()-r.height();
	closeBox->setBounds(r);
	if (maximiseBox)
		maximiseBox->setBounds(r.offset(-r.width()-3,0));
	if (minimiseBox)
		minimiseBox->setBounds(r.offset(-r.width(),0));
}

void MVTitleBar::drawBackground(const MVRect&)
/****************************************************************************
*
* Function:		MVTitleBar::drawBackground
* Parameters:	clip	- Clipping rectangle for the view
*
* Description:	Draws the representation of the title bar.
*
****************************************************************************/
{
	ibool active = state & sfActive;

	// Fill the background
	dc.setColor(getColor(active ? scActiveTitleBar : scPassiveTitleBar));
	dc.fillRect(0,0,bounds.width(),bounds.height());

	// Draw the title bar text
	MVTextJust old;
	old.save(dc);
	tjust.use(dc);
	dc.setColor(getColor(active ? scActiveTitleText : scPassiveTitleText));
	useFont(MVFontRecord(_MV_systemFont,-1,_MV_systemFontSize,fmBold));
	dc.drawStr(start.x,start.y,title);
	old.use(dc);
}
