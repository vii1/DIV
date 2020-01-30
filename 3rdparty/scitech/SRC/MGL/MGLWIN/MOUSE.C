/****************************************************************************
*
*						MegaGraph Graphics Library
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
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Mouse support routines. Most of these routines are interal,
*				as the MGL only publicly supports a minimal mouse interface
*				(for portability reasons).
*
*
****************************************************************************/

#include "mglwin\internal.h"

/*------------------------- Global Variables ------------------------------*/

static cursor_t cursor = {		/* Currently active mouse cursor	*/
   {0x00000000, 0x00000040, 0x00000060, 0x00000070,
	0x00000078, 0x0000007C, 0x0000007E, 0x0000007F,
	0x0000807F, 0x0000C07F, 0x0000007E, 0x00000076,
	0x00000066, 0x00000043, 0x00000003, 0x00008001,
	0x00008001, 0x0000C000, 0x0000C000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000},

   {0x000000C0, 0x000000E0, 0x000000F0, 0x000000F8,
	0x000000FC, 0x000000FE, 0x000000FF, 0x000080FF,
	0x0000C0FF, 0x0000E0FF, 0x0000F0FF, 0x000000FF,
	0x000000FF, 0x000080EF, 0x000080C7, 0x0000C083,
	0x0000C003, 0x0000E001, 0x0000E001, 0x0000C000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000},
	1,1
    };

static int		mx,my,screenmx,screenmy;
static	rect_t	msRect;
static	int		visible = -1;
static	MGLDC	*DispDC = NULL;
static	color_t	cursorColor;
static	int		numDivots = 0;
static	void	**msDivot = NULL;
static	int		oldVisible;

/*------------------------- Implementation --------------------------------*/

#ifdef __BORLANDC__
#pragma warn -par
#endif
/* {secret} */
void _MS_sizeScreen(int xres,int yres) {}

/* {secret} */
void _MS_setDisplayDC(MGLDC *dc)
/****************************************************************************
*
* Function:		_MS_setDisplayDC
* Parameters:	dc	- Current display device context
*
* Description:	Saves the current display device context, and allocates
*				the memory we require for saving the image behind the mouse
*				cursor. We also save the internal color representation to
*				draw the cursor in white.
*
****************************************************************************/
{
	int	i;

	if ((DispDC = dc) != NULL) {
		visible = -1;
		if (!dc->v->d.hardwareCursor) {
			if (numDivots) {
				for (i = 0; i < numDivots; i++)
					MGL_free(msDivot[i]);
				MGL_free(msDivot);
				}
			/* Determine the size of the largest divot block required to save
			 * the section of video memory. Internally the MGL will extend
			 * the coordinates to fit to possibly 8 pixel boundaries, so we
			 * need to determine the largest of all possible combinations.
			 */
			numDivots = dc->mi.maxPage+1;
			msDivot = MGL_malloc(sizeof(void*) * numDivots);
			for (i = 0; i < numDivots; i++)
				msDivot[i] = MGL_malloc(dc->r.divotSize(dc,1,1,33,33));
			}
		MS_setCursorColor(MGL_realColor(dc,MGL_WHITE));
		if (_MGL_winDirectMode == 1)
			WD_setMouseCallback(_MS_moveCursor);
		}
	else {
		if (_MGL_winDirectMode == 1)
			WD_setMouseCallback(NULL);
		}
}

/****************************************************************************
DESCRIPTION:
Draws the mouse cursor at the current location.

HEADER:
mgraph.h

REMARKS:
This function draws the mouse cursor at the current location regardless of it's
hidden status. This function does not save the video memory below the mouse
cursor, and is primarily intended to implement mouse cursors when performing
double buffered or multibuffered animation. You simply call this function to draw
the mouse cursor on the active display page after you have rasterized the frame.
Note that when performing animation and drawing the mouse cursor, you should
leave the normal mouse cursor hidden with a call to MS_hide and use this function
to draw the cursor on the rasterized frame when necessary.

Note that this is different to the normal automatic mouse cursor drawing functions
which always draw the mouse cursor onto the currently visible display page, which
wont work with double or multibuffered animation techniques.

SEE ALSO:
MS_show, MS_hide
****************************************************************************/
void MGLAPI MS_drawCursor(void)
{
	DispDC->r.putMouseImage(DispDC,msRect.left,msRect.top,4,32,
		(uchar*)cursor.andMask,(uchar*)cursor.xorMask,cursorColor);
}

static void saveCursor(int page)
/****************************************************************************
*
* Function:		saveCursor
*
* Description:	Saves the image under the mouse cursor, so that we can
*				display it again at a later date.
*
****************************************************************************/
{
	rect_t	d = msRect;

	if (d.left < 0)						d.left = 0;
	if (d.right > DispDC->mi.xRes+1)	d.right = DispDC->mi.xRes+1;
	if (d.top < 0)                      d.top = 0;
	if (d.bottom > DispDC->mi.yRes+1)	d.bottom = DispDC->mi.yRes+1;
	DispDC->r.getDivot(DispDC,d.left,d.top,d.right,d.bottom,msDivot[page]);
}

static void restoreCursor(int page)
/****************************************************************************
*
* Function:		restoreCursor
*
* Description:	Restores the image under the mouse cusor, by displaying
*				the divot that we save previously.
*
****************************************************************************/
{
	DispDC->r.putDivot(DispDC,msDivot[page]);
}

/* {secret} */
void ASMAPI _MS_moveCursor(int x,int y)
/****************************************************************************
*
* Function:		_MS_moveCursor
* Parameters:	x,y	- New location for mouse cursor
*
* Description:  Interrupt routine called to move the mouse cursor to the
*				new location.
*
*				This routine is written to be able to be called from
*				within an interrupt handler. The old MGL used to do it
*				this way, but the new MGL handler only moves the mouse
*				when the user application is processing events.
*
****************************************************************************/
{
	int	apage,vpage;
	int	screenx,screeny;

	if (DispDC) {
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		screenx = (mx = x) + DispDC->startX;
		screeny = (my = y) + DispDC->startY;
		if (screenx != screenmx || screeny != screenmy) {
			screenmx = screenx;
			screenmy = screeny;
			msRect.left = screenmx - cursor.xHotSpot;
			msRect.right = msRect.left + 32;
			msRect.top = screenmy - cursor.yHotSpot;
			msRect.bottom = msRect.top + 32;
			if (visible == 0) {
				if (DispDC->v->d.hardwareCursor)
					DispDC->v->d.setCursorPos(mx,my);
				else {
					apage = DispDC->v->d.activePage;
					vpage = DispDC->v->d.visualPage;
					DispDC->v->d.setActivePage(DispDC,vpage);
					restoreCursor(vpage);
					saveCursor(vpage);
					MS_drawCursor();
					DispDC->v->d.setActivePage(DispDC,apage);
					}
				}
			}
		}
}

/* {secret} */
void _MS_displayStartChanged(void)
{ _MS_moveCursor(mx,my); }

/* {secret} */
void _MS_preChangeVisualPage(int newPage)
/****************************************************************************
*
* Function:		_MS_preChangeVisualPage
* Parameters:	newPage	- New page about to become visible
*
* Description:  This routine is called by the MGL_setVisualPage just before
*				a page flip is about to occur. We save the memory
*				under the cursor on the new page, draw it and then return.
*
****************************************************************************/
{
	if (_MGL_fullScreen && visible == 0 && !DispDC->v->d.hardwareCursor) {
		int apage = DispDC->v->d.activePage;
		DispDC->v->d.setActivePage(DispDC,newPage);
		saveCursor(newPage);
		MS_drawCursor();
		DispDC->v->d.setActivePage(DispDC,apage);
		}
}

/* {secret} */
void _MS_postChangeVisualPage(int oldPage)
/****************************************************************************
*
* Function:		_MS_postChangeVisualPage
* Parameters:	oldPage	- Old visual page before flip
*
* Description:  This routine is called by the MGL_setVisualPage just before
*				a page flip is about to occur. We then remove the mouse
*				cursor from the currently visible page, save the memory
*				under the cursor on the new page, draw it and then return.
*				This way we can properly support double buffered mouse
*				cursors when the mouse is visible.
*
****************************************************************************/
{
	if (_MGL_fullScreen && visible == 0 && !DispDC->v->d.hardwareCursor) {
		int apage = DispDC->v->d.activePage;
		DispDC->v->d.setActivePage(DispDC,oldPage);
		restoreCursor(oldPage);
		DispDC->v->d.setActivePage(DispDC,apage);
		}
}

/* {secret} */
void _MS_saveCursor(void)
/****************************************************************************
*
* Function:		_MS_saveCursor
*
* Description:	Save the mouse cursor visible flag and hide it from view.
*
****************************************************************************/
{
	oldVisible = visible;
	MS_hide();
}

/* {secret} */
void _MS_restoreCursor(void)
/****************************************************************************
*
* Function:		_MS_restoreCursor
*
* Description:	Restore the current mouse cursor shape by re-downloading it
*				to the hardware and resetting the hardware cursor location.
*				We assume that the mouse cursor is hidden
*
****************************************************************************/
{
	if (DispDC->v->d.hardwareCursor) {
		DispDC->v->d.setCursor(&cursor);
		DispDC->v->d.setCursorColor(DispDC,cursorColor);
		MS_moveTo(screenmx,screenmy);
		}
	if (oldVisible == 0) {
		visible = -1;
		MS_show();
		}
	visible = oldVisible;
}

/* {secret} */
void _MS_saveState(void)
{
	visible--;
	oldVisible = visible;
}

/* {secret} */
void _MS_restoreState(void)
{
	visible = oldVisible;
	MS_show();
}

/*------------------------ Public interface routines ----------------------*/

/****************************************************************************
DESCRIPTION:
Determines if a mouse is attached and functioning.

HEADER:
mgraph.h

RETURNS:
True if a mouse is attached and can be used, false if mouse is not available.

REMARKS:
This function can be used to determine if a mouse pointer is installed and
functioning, so that application software can determine if a mouse can be used or
not. If this function returns false, it is up to the application software to provide full
control via the keyboard or inform users that they will need to install a mouse.
****************************************************************************/
ibool MGLAPI MS_available(void)
{
	return true;
}

/****************************************************************************
DESCRIPTION:
Displays the mouse cursor.

HEADER:
mgraph.h

REMARKS:
Increments the internal mouse cursor display counter, and displays the cursor when
the counter gets to zero. Calls to MS_hide decrement the counter, and this call
effectively cancels a single MS_hide call, allowing the MS_show and MS_hide
calls to be nested.

If the mouse was obscured with the MS_obscure function, this reverses the effect
and will redisplay the mouse cursor again. On systems with a hardware mouse
cursor, the MS_obscure function effectively does nothing, while on systems using a
software mouse cursor, the MS_obscure function simply calls MS_hide.

Note that the mouse cursor display counter is reset to -1 by default when an MGL
fullscreen mode is started, so a single MS_show will display the mouse cursor after
the mode has been started.

SEE ALSO:
MS_hide, MS_obscure
****************************************************************************/
void MGLAPI MS_show(void)
{
	int	apage,vpage;

	if (_MGL_fullScreen) {
		if (visible < 0) {
			visible++;
			if (visible == 0) {
				// Show the mouse cursor
				if (DispDC->v->d.hardwareCursor) {
					DispDC->v->d.setCursorPos(mx,my);
					DispDC->v->d.showCursor(true);
					}
				else {
					msRect.left = screenmx - cursor.xHotSpot;
					msRect.right = msRect.left + 32;
					msRect.top = screenmy - cursor.yHotSpot;
					msRect.bottom = msRect.top + 32;
					apage = DispDC->v->d.activePage;
					vpage = DispDC->v->d.visualPage;
					DispDC->v->d.setActivePage(DispDC,vpage);
					saveCursor(vpage);
					MS_drawCursor();
					DispDC->v->d.setActivePage(DispDC,apage);
					}
				}
			}
		}
	else
		ShowCursor(TRUE);
}
/****************************************************************************
DESCRIPTION:
Hides the mouse cursor.


HEADER:
mgraph.h

REMARKS:
Decrements the internal mouse cursor display counter, and hides the cursor if the
counter was previously set to zero. Calls to MS_show increment the counter,
allowing the MS_show and MS_hide calls to be nested.

SEE ALSO:
MS_show, MS_obscure
***************************************************************************/
void MGLAPI MS_hide(void)
{
	int	apage,vpage;

	if (_MGL_fullScreen) {
		visible--;
		if (visible == -1) {
			if (DispDC->v->d.hardwareCursor)
				DispDC->v->d.showCursor(false);
			else {
				apage = DispDC->v->d.activePage;
				vpage = DispDC->v->d.visualPage;
				DispDC->v->d.setActivePage(DispDC,vpage);
				restoreCursor(vpage);
				DispDC->v->d.setActivePage(DispDC,apage);
				}
			}
		}
	else
		ShowCursor(FALSE);
}

/****************************************************************************
DESCRIPTION:
Moves the mouse cursor to a new location.

HEADER:
mgraph.h

PARAMETERS:
x	- New mouse x coordinate (screen coordinates)
y	- New mouse y coordinate (screen coordinates)

REMARKS:
Moves the mouse cursor to the specified location in screen coordinates.

Note that it is not usually a good idea to move the mouse cursor around while the
user is interacting with the application, but this can be used to restore the mouse
cursor to a known location if it has been hidden for a long period of time.

SEE ALSO:
MS_getPos
****************************************************************************/
void MGLAPI MS_moveTo(
	int x,
	int y)
{
	if (_MGL_fullScreen) {
		if (_MGL_winDirectMode == 1)
			WD_setMousePos(x,y);
		else
			_DD_setMousePos(x,y);
		}
	else
		SetCursorPos(x,y);
}

/****************************************************************************
DESCRIPTION:
Returns the current mouse cursor location.

HEADER:
mgraph.h

PARAMETERS:
x	- Place to store value for mouse x coordinate (screen coordinates)
y	- Place to store value for mouse y coordinate (screen coordinates)

REMARKS:
Obtains the current mouse cursor position in screen coordinates. Normally the
mouse cursor location is tracked using the mouse movement events that are posted
to the event queue when the mouse moves, however this routine provides an
alternative method of polling the mouse cursor location.

SEE ALSO:
MS_moveTo
****************************************************************************/
void MGLAPI MS_getPos(
	int *x,
	int *y)
{
	if (_MGL_fullScreen) {
		*x = mx;
		*y = my;
		}
	else {
		POINT p;
		GetCursorPos(&p);
		*x = p.x;
		*y = p.y;
		}
}

/****************************************************************************
DESCRIPTION:
Hides the mouse cursor from view during graphics output.


HEADER:
mgraph.h

REMARKS:
Hides the mouse cursor from view in order to perform graphics output using MGL.
If the graphics device driver supports a hardware cursor, this is handled by the
hardware, otherwise it is removed from the display. You should call this routine
rather than MS_hide in order to temporarily hide the mouse cursor during graphics
output as the MS_hide routine will always hide the cursor, regardless of whether the
system has a hardware mouse cursor or not.

SEE ALSO:
MS_show, MS_hide
****************************************************************************/
void MGLAPI MS_obscure(void)
{
	if (_MGL_fullScreen) {
		if (visible < 0 || !DispDC->v->d.hardwareCursor)
			MS_hide();
		}
}

/****************************************************************************
DESCRIPTION:
Sets the mouse cursor shape.


HEADER:
mgraph.h

PARAMETERS:
curs	- Pointer to new mouse cursor shape

REMARKS:
Sets the graphics mouse cursor shape, passed in the cursor_t structure. The cursor_t
structure contains a mouse cursor and mask and a mouse cursor xor mask that is
used to display the cursor on the screen, along with the mouse cursor hotspot
location. Refer to the cursor_t structure definition for more information.
****************************************************************************/
void MGLAPI MS_setCursor(
	cursor_t *curs)
{
	if (_MGL_fullScreen) {
		MS_hide();
		cursor = *curs;
		if (DispDC->v->d.hardwareCursor)
			DispDC->v->d.setCursor(curs);
		MS_show();
		}
	else {
		/* TODO: Figure out how to create a memory mouse cursor image and
		 * to tell Windows to use that as the mouse cursor.
		 */
		}
}

/****************************************************************************
DESCRIPTION:
Sets the current mouse cursor color.

HEADER:
mgraph.h

PARAMETERS:
color	- New mouse cursor color, in current display mode format.

REMARKS:
Sets the color for the mouse cursor to the specified color, which is passed in as a
packed MGL color in the proper format for the current display mode (either a color
index or a packed RGB color value). By default the mouse cursor is set to white,
which is a color index of 15 by default in MGL. If you re-program the color palette
in 4 or 8 bit modes, you will need to reset the mouse cursor value to the value that
represents white.
****************************************************************************/
void MGLAPI MS_setCursorColor(
	color_t color)
{
	cursorColor = color;
	if (_MGL_fullScreen && DispDC->v->d.hardwareCursor)
		DispDC->v->d.setCursorColor(DispDC,color);
}

