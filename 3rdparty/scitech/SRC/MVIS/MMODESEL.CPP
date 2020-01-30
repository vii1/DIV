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
* Description:	Member functions for the MVModeSelector class.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

#include "mvis/mmodesel.hpp"
#include "mvis/mfontmgr.hpp"
#include "mvis/msttext.hpp"
#include "mvis/mbutton.hpp"
#include "mvis/mkeys.hpp"
#include <stdio.h>

/*----------------------------- Implementation ----------------------------*/

#define	LINESPERLIST	6

MVModeSelector::MVModeSelector(MGLDevCtx& dc,const char *title,int driver,
	int mode,uchar *validModes)
	: MVDialog(dc,MVRect(0,0,100,100),title), driver(driver), mode(mode),
	  modenames(5,5),
	  MVWindowInit(MVWindow::initFrame,MVWindow::initTitleBar)
/****************************************************************************
*
* Function:		MVModeSelector::MVModeSelector
* Parameters:	title		- Title for the dialog box
*				driver		- Video device driver number
*				mode		- Video mode number
*				validModes	- List of valid video modes (terminate with -1)
*
* Description:	Constructor for the MVModeSelector dialog box. We build the
*				list of all names to be loaded into the dialog box.
*
****************************************************************************/
{
	MVLabel	*label;

	options |= ofCentered;

	// Find the height of the system font, and compute the location of
	// all the elements of the input dialog.

	useFont(fmSystemFont);
	metrics_t m;
	dc.getFontMetrics(m);
	int height = dc.textHeight();
	int cellHeight = m.ascent - m.descent + 1 + 4;
	int buttonHeight = height + 12;
	int dialogWidth = (dc.sizex()*2)/3;

	MVRect r(clientRect);
	int	adjustBottom = bounds.bottom() - r.bottom();

	// Create the label and list box for video mode names

	r.top() += height;
	r.bottom() = r.top() + height;
	r.left() = 15;
	r.right() = 15 + dc.textWidth(modeListText);
	insert(label = new MVLabel(dc,r,modeListText,NULL));

	r.top() = r.bottom();
	r.bottom() = r.top() + cellHeight*LINESPERLIST + 4;
	r.right() = dialogWidth - 15 - _MV_sysScrollBarWidth;
	modeList = new MVList(dc,r,MVPoint(r.right()-r.left()-4,cellHeight));
	label->setAttached(modeList);

	// Now create the scroll bars for the list and attach them.

	MVRect	r2(r);
	r2.left() = r2.right();
	r2.right() = r2.left() + _MV_sysScrollBarWidth;
	MVScrollBar *vScroll = new MVScrollBar(dc,r2);

	r.top() = r.bottom();
	r.bottom() = r.top() + _MV_sysScrollBarWidth;
	MVScrollBar *hScroll = new MVScrollBar(dc,r);

	modeList->setHScroll(hScroll);
	modeList->setVScroll(vScroll);

	// Adjust the bounds of the box to fit, including the buttons

	bounds.right() = dialogWidth;
	bounds.bottom() = r.bottom() + adjustBottom + buttonHeight + 20;
	setBounds(bounds);

	// Now add the buttons to the dialog box

	int width = dc.textWidth(setText) + 16;
	int sizex = width + 15;
	r.top() = bounds.bottom() - buttonHeight - 10;
	r.bottom() = r.top() + buttonHeight;
	r.right() =  r.left() + width;
	MVButton *setBut = new MVButton(dc,r,setText,cmOk,bfDefault);

	r.left() += sizex;	r.right() +=  sizex;
	MVButton *cancelBut = new MVButton(dc,r,cancelText,cmCancel,bfNormal);

	// Now insert all of the items into the dialog box in the correct
	// tab-ing order

	insert(cancelBut);
	insert(setBut);
	insert(modeList);

	// Now insert the scroll bars on top of everything else
	insert(vScroll);
	insert(hScroll);

	// Build the list of valid video mode names, and put the highlight on
	// the current video mode.

	uint	i;
	int		selection;
	uchar	*modes = MGL_availableModes();
	char	buf[40];

	for (i = 0; *modes != 0xFF; modes++) {
		for (uchar *p = validModes; *p != 0xFF; p++)
			if (*p == *modes) {
				if (*modes == mode)
					selection = i;
				TCDynStr	*s = new TCDynStr(MGL_modeName(*modes));
				sprintf(buf," %d page (%s)",
					MGL_availablePages(*modes),
					MGL_modeDriverName(*modes));
				*s += buf;
				modenames.add(s);
				i++;
				break;
				}
		}

	modeList->setDataBounds(MVRect(0,0,1,modenames.numberOfItems()));
	for (i = 0; i < modenames.numberOfItems(); i++)
		modeList->setCell(0,i,*modenames[i]);
	modeList->setHRange(0,0);
	modeList->setVRange(0,modenames.numberOfItems()-1);
	modeList->clearSelection();
	modeList->selectCell(0,selection);
	modeList->setCurrentCell(0,selection);

	modeList->select();
}

void MVModeSelector::handleEvent(MVEvent& event,phaseType phase)
/****************************************************************************
*
* Function:		MVModeSelector::handleEvent
* Parameters:	event	- Event to handle
*				phase	- Current phase for the event (pre,focus,post)
*
* Description:	Event handling routine for the MVModeSelector class.
*
****************************************************************************/
{
#ifdef	DUMP_IMAGE
	rect_t r;

	switch (event.what) {
		case evCommand:
			switch (event.message.command) {
				case cmOk:
					r.left() = 0;
					r.top() = 0;
					r.right() = MGL_sizex(dc)+1;
					r.bottom() = MGL_sizey(dc)+1;
					MGL_setViewportDC(dc,r);
					MGL_saveBitmapFromDC(dc,"dump00.bmp",0,0,r.right(),r.bottom());
					break;
				}
			break;
		}
#endif

	MVDialog::handleEvent(event,phase);

	switch (event.what) {
		case evBroadcast:
			switch (event.message.command) {
				case cmListItemSelected:
					if (event.message.infoPtr == modeList) {
						endModal(cmOk);
						clearEvent(event);
						}
					break;
				}
			break;
		}
}

int MVModeSelector::getMode()
/****************************************************************************
*
* Function:		MVModeSelector::getMode
* Returns:		Selected video mode number.
*
****************************************************************************/
{
	MVPoint		cursor;
	const char	*name;

	modeList->getCurrentCell(cursor);
	modeList->getCell(cursor.x,cursor.y,name);

	for (int i = 0; i <= grMAXMODE; i++) {
		if (strncmp(MGL_modeName(i),name,strlen(MGL_modeName(i))) == 0)
			return i;
		}
	return -1;
}
