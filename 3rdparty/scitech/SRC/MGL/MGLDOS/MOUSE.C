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

#include "mgl.h"
#pragma hdrstop
#include "mgldos\internal.h"

/*------------------------- Global Variables ------------------------------*/

PRIVATE cursor_t cursor = {		/* Currently active mouse cursor	*/
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

PRIVATE int		mx,my,screenmx,screenmy,frozen = 0,buttons = 0;
PRIVATE	rect_t	msRect;
PRIVATE	int		visible = -1;
PRIVATE	MGLDC	*DispDC = NULL;
PRIVATE	color_t	cursorColor;
PRIVATE	int		numDivots = 0;
PRIVATE	void	**msDivot = NULL;

/*------------------------- Implementation --------------------------------*/

/* The following two routines fool the mouse driver into working correctly
 * in SuperVGA video modes, by making it think it is always in EGA mode 10h,
 * which means we always get coordinates returned in virtual coordinates.
 */

int _MS_foolDriver(void)
{
	int oldmode = PM_getByte(_MGL_biosSel,0x49);
	if (oldmode > 0x13)
		PM_setByte(_MGL_biosSel,0x49,0x10);
	return oldmode;
}

void _MS_unfoolDriver(int oldmode)
{
	PM_setByte(_MGL_biosSel,0x49,oldmode);

	/* Some mouse drivers reset the plane mask register for VGA plane 4
	 * modes, which screws up the display on some VGA compatible controllers
	 * in SuperVGA modes. We reset the value back again in here to solve
	 * the problem.
	 */
	UV_wrinx(0x3C4,0x2,0x0F);
}

PRIVATE void callMouse(RMREGS *r)
/****************************************************************************
*
* Function:		callMouse
* Parameters:	r	- Pointer to register values to load
*
* Description:	Calls the mouse driver with the appropriate values.
*
****************************************************************************/
{
	int oldval = _MS_foolDriver();
	PM_int86(0x33,r,r);
	_MS_unfoolDriver(oldval);
}

PRIVATE ibool MS_installed(void)
/****************************************************************************
*
* Function:		MS_installed
* Returns:		True if a mouse driver is currently installed in the system
*
* Description:	Determines if a software mouse driver is currently installed
*				in the system.
*
****************************************************************************/
{
	RMREGS	regs;
	RMSREGS	sregs;
	uint    sel,off;
	ibool	retval;

	regs.x.ax = 0x3533;					/* Get interrupt vector 0x33	*/
	PM_int86x(0x21,&regs,&regs,&sregs);

	/* Check that interrupt vector 0x33 is not a zero, and that the first
	 * instruction in the interrupt vector is not an IRET instruction
	 */
	retval = true;
	PM_mapRealPointer(&sel,&off,sregs.es, regs.x.bx);
	retval = (sregs.es != 0) && (regs.x.bx != 0) && (PM_getByte(sel,off) != 207);
	return retval;
}

void _MS_init(void)
/****************************************************************************
*
* Function:		_MS_init
*
* Description:	Initialises the mouse module by doing a software reset on
*				the mouse driver (if it is installed).
*
****************************************************************************/
{
	RMREGS	regs;

	if (MS_installed()) {
		regs.x.ax = 33;		/* Mouse function 33 - Software reset		*/
		callMouse(&regs);
		if (regs.x.bx != 0)
			buttons = regs.x.bx;	/* Buttons returned in BX			*/
		visible = -1;
		frozen = 0;
		}
}

void _MS_sizeScreen(int xres,int yres)
/****************************************************************************
*
* Function:		_MS_sizeScreen
* Parameters:	xres	- Screen X resolution
*				yres	- Screen Y resolution
*
* Description:	Calls the mouse driver to re-size the virtual screen
*				coordinates for the current video mode.
*
****************************************************************************/
{
	RMREGS	regs;

	if (buttons) {
		uchar mode = PM_getByte(_MGL_biosSel,0x49);
		if (mode == 0x0D || mode == 0x13)
			xres *= 2;
		regs.x.ax = 7;	/* Mouse function 7 - Set horizontal min and max */
		regs.x.cx = 0;
		regs.x.dx = xres;
		callMouse(&regs);
		regs.x.ax = 8;	/* Mouse function 8 - Set vertical min and max	 */
		regs.x.cx = 0;
		regs.x.dx = yres;
		callMouse(&regs);
		}
}

void _MS_setMouseHandler(int mask,PM_mouseHandler mh)
/****************************************************************************
*
* Function:		_MS_setMouseHandler
* Parameters:	mask	- Mouse handler mask
*				mh		- Mouse handler routine to install
*
* Description:	Simply calls the PMODE library to install the mouse
*				handler, but first we must fool the mouse driver to ensure
*				it will work correctly in SuperVGA video modes.
*
****************************************************************************/
{
	if (buttons) {
		int oldmode = _MS_foolDriver();
		PM_setMouseHandler(mask,mh);
		_MS_unfoolDriver(oldmode);
		}
}

void _MS_restoreMouseHandler(void)
/****************************************************************************
*
* Function:		_MS_restoreMouseHandler
*
* Description:	Calls the PM/Pro library to restore the mouse handler.
*
****************************************************************************/
{
	if (buttons) {
		int oldmode = _MS_foolDriver();
		PM_restoreMouseHandler();
		_MS_unfoolDriver(oldmode);
		}
}

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
	/* Do a hardware reset on the mouse driver, to ensure that our
	 * fooling code works correctly, and the mouse cursor moves correctly
	 * in graphics mode.
	 */
	if ((DispDC = dc) != NULL) {
		int	i,oldmode = _MS_foolDriver();
		PM_resetMouseDriver(1);
		_MS_unfoolDriver(oldmode);
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
		}
}

/****************************************************************************
	see MS_drawCursor in MGLWIN\Mouse.c
****************************************************************************/
void MGLAPI MS_drawCursor(void)
{
	DispDC->r.putMouseImage(DispDC,msRect.left,msRect.top,4,32,
		(uchar*)cursor.andMask,(uchar*)cursor.xorMask,cursorColor);
}

PRIVATE void saveCursor(int page)
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

PRIVATE void restoreCursor(int page)
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

	if (!frozen) {
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

void _MS_displayStartChanged(void)
{ _MS_moveCursor(mx,my); }

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
	if (visible == 0 && !DispDC->v->d.hardwareCursor) {
		int apage = DispDC->v->d.activePage;
		DispDC->v->d.setActivePage(DispDC,newPage);
		saveCursor(newPage);
		MS_drawCursor();
		DispDC->v->d.setActivePage(DispDC,apage);
		}
}

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
	if (visible == 0 && !DispDC->v->d.hardwareCursor) {
		int apage = DispDC->v->d.activePage;
		DispDC->v->d.setActivePage(DispDC,oldPage);
		restoreCursor(oldPage);
		DispDC->v->d.setActivePage(DispDC,apage);
		}
}

/*------------------------ Public interface routines ----------------------*/
/****************************************************************************
	See MS_available in MGLWIN\mouse.c
****************************************************************************/
ibool MGLAPI MS_available(void)
{
	return buttons;
}

void MGLAPI MS_show(void)
/****************************************************************************
*
* Function:		MS_show
* Description:	Makes the mouse cursor visible
*
****************************************************************************/
{
	int	apage,vpage;

	if (!buttons)
		return;
	if (visible < 0) {
		UV_disable();
		visible++;
		frozen = true;
		UV_enable();
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
		UV_disable();
		frozen = false;
		UV_enable();
		}
}

/****************************************************************************
	See MS_hide in MGLWIN\mouse.c
****************************************************************************/
void MGLAPI MS_hide(void)
{
	int	apage,vpage;

	if (!buttons)
		return;
	UV_disable();
	visible--;
	UV_enable();
	if (visible == -1) {
		UV_disable();
		frozen = true;
		UV_enable();
		if (DispDC->v->d.hardwareCursor)
			DispDC->v->d.showCursor(false);
		else {
			apage = DispDC->v->d.activePage;
			vpage = DispDC->v->d.visualPage;
			DispDC->v->d.setActivePage(DispDC,vpage);
			restoreCursor(vpage);
			DispDC->v->d.setActivePage(DispDC,apage);
			}
		UV_disable();
		frozen = false;
		UV_enable();
		}
}

void _EVT_moveMouse(int x,int y);

void MGLAPI MS_moveTo(int x,int y)
/****************************************************************************
*
* Function:		MS_moveTo
* Parameters:	x,y	- Coordinate to move to
*
* Description:	Moves to mouse cursor to the specified coordinate.
*
****************************************************************************/
{
	RMREGS	regs;

	if (buttons) {
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x > DispDC->mi.xRes)	x = DispDC->mi.xRes;
		if (y > DispDC->mi.yRes)	y = DispDC->mi.yRes;
		if (_EVT_installed) {
			uchar mode = PM_getByte(_MGL_biosSel,0x49);
			regs.x.ax = 4;		/* Mouse function 4 - Set mouse position	*/
			regs.x.cx = x;		/* New horizontal coordinate				*/
			regs.x.dx = y;		/* New vertical coordinate					*/
			if (mode == 0x0D || mode == 0x13)
				regs.x.cx *= 2;
			callMouse(&regs);
			}
		_MS_moveCursor(x,y);
		_EVT_moveMouse(mx,my);
		}
}

void MGLAPI MS_getPos(int *x,int *y)
/****************************************************************************
*
* Function:		MS_getPos
* Parameters:	x,y	- Place to store the mouse coordinate
*
* Description:	Reads the current mouse cursor location int *screen*
*				coordinates.
*
****************************************************************************/
{
	RMREGS	regs;

	if (buttons) {
		uchar mode = PM_getByte(_MGL_biosSel,0x49);
		regs.x.ax = 3;		/* Mouse function 3 - Get mouse position	*/
		callMouse(&regs);
		*x = regs.x.cx;
		*y = regs.x.dx;
		if (mode == 0x0D || mode == 0x13)
			*x /= 2;
		}
}

void MGLAPI MS_obscure(void)
/****************************************************************************
*
* Function:		MS_obscure
*
* Description:	Obscures the cursor from view if the mouse cursor is within
*				the update area, or moves into the update area. This is to
*				ensure mutual exclusion to display memory between the
*				mouse cursor routines and the user display routines.
*
****************************************************************************/
{
	if (!buttons)
		return;
	if (visible < 0 || !DispDC->v->d.hardwareCursor)
		MS_hide();
}

void MGLAPI MS_setCursor(cursor_t *curs)
/****************************************************************************
*
* Function:		MS_setCursor
* Parameters:	curs	- New mouse cursor to use
*
* Description:	Sets the current mouse cursor to the specified value.
*
****************************************************************************/
{
	if (!buttons)
		return;
	MS_hide();
	cursor = *curs;
	if (DispDC->v->d.hardwareCursor)
		DispDC->v->d.setCursor(curs);
	MS_show();
}

void MGLAPI MS_setCursorColor(color_t color)
/****************************************************************************
*
* Function:		MS_setCursorColor
* Parameters:	color	- New color for mouse cursor
*
* Description:	Sets the color of the mouse cursor. This routine may not
*				work on some systems (such as under Windows), but it
*				provided for full screen sessions where the user application
*				has full control of the palette and needs to change the
*				mouse cursor color (the MegaVision uses this).
*
****************************************************************************/
{
	cursorColor = color;
	if (DispDC->v->d.hardwareCursor)
		DispDC->v->d.setCursorColor(DispDC,color);
}
