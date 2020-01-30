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
* Description:	Viewport control routines
*
*
****************************************************************************/

#include "mgl.h"

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
DESCRIPTION:
Sets the currently active viewport.

HEADER:
mgraph.h

PARAMETERS:
view	- New global viewport bounding rectangle

REMARKS:
Sets the dimensions of the currently active viewport. These dimensions are global to
the entire display area used by the currently active video device driver. Note that
when the viewport is changing, the viewport origin is always reset back to (0,0).

All output in MGL is relative to the current viewport, so by changing the viewport
to a new value you can make all output appear in a different rectangular portion of
the video display.

SEE ALSO:
MGL_getViewport, MGL_setRelViewport, MGL_clearViewport,
MGL_setClipRect, MGL_setViewportOrg
****************************************************************************/
void MGLAPI MGL_setViewport(
	rect_t view)
{
	DC.a.viewPort.left 		= (view.left += DC.size.left);
	DC.a.viewPort.top 		= (view.top += DC.size.top);
	DC.a.viewPort.right 	= (view.right += DC.size.left);
	DC.a.viewPort.bottom	= (view.bottom += DC.size.top);
	DC.viewPortFX.left 		= MGL_TOFIX(view.left);
	DC.viewPortFX.top 		= MGL_TOFIX(view.top);
	DC.viewPortFX.right 	= MGL_TOFIX(view.right);
	DC.viewPortFX.bottom 	= MGL_TOFIX(view.bottom);
	DC.r.setClipRect(&DC,0,0,view.right - view.left,view.bottom - view.top);
	DC.a.CP.x = DC.a.CP.y = 0;
	DC.a.viewPortOrg.x = DC.a.viewPortOrg.y = 0;
}

/****************************************************************************
DESCRIPTION:
Sets the currently active viewport for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc		- Device context to change viewport for
view	- New global viewport bounding rectangle

REMARKS:
This function is the same as MGL_setViewport, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_setViewport, MGL_getViewport, MGL_setRelViewport, MGL_clearViewport,
MGL_setClipRect, MGL_setViewportOrg
****************************************************************************/
void MGLAPI MGL_setViewportDC(
	MGLDC *dc,
	rect_t view)
{
	if (dc == _MGL_dcPtr) {
		MGL_setViewport(view);
		dc->a.viewPort = DC.a.viewPort;
		dc->a.clipRect = DC.a.clipRect;
		}
	else {
		dc->a.viewPort.left 	= (view.left += dc->size.left);
		dc->a.viewPort.top 		= (view.top += dc->size.top);
		dc->a.viewPort.right 	= (view.right += dc->size.left);
		dc->a.viewPort.bottom	= (view.bottom += dc->size.top);
		dc->viewPortFX.left 	= MGL_TOFIX(view.left);
		dc->viewPortFX.top 		= MGL_TOFIX(view.top);
		dc->viewPortFX.right 	= MGL_TOFIX(view.right);
		dc->viewPortFX.bottom 	= MGL_TOFIX(view.bottom);
		dc->r.setClipRect(dc,0,0,view.right - view.left,view.bottom - view.top);
		dc->a.CP.x = dc->a.CP.y = 0;
		dc->a.viewPortOrg.x = dc->a.viewPortOrg.y =0;
		}
}

/****************************************************************************
DESCRIPTION:
Sets a viewport relative to the current viewport.

HEADER:
mgraph.h

PARAMETERS:
view	- Bounding rectangle for the new viewport

REMARKS:
Sets the current viewport to the viewport specified by view, relative to the currently
active viewport. The new viewport is restricted to fall within the bounds of the
currently active viewport. Note that when the viewport is changing, the viewport
origin is always reset back to (0,0).

All output in MGL is relative to the current viewport, so by changing the viewport
to a new value you can make all output appear in a different rectangular portion of
the video display.

SEE ALSO:
MGL_getViewport, MGL_setViewport, MGL_clearViewport, MGL_setClipRect,
MGL_setViewportOrg
****************************************************************************/
void MGLAPI MGL_setRelViewport(
	rect_t view)
{
	MGL_offsetRect(view,DC.a.viewPort.left,DC.a.viewPort.top);
	MGL_setViewport(view);
}

/****************************************************************************
DESCRIPTION:
Sets a viewport relative to the current viewport for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc		- Device context to change viewport for
view	- Bounding rectangle for the new viewport

REMARKS:
This function is the same as MGL_setRelViewport, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_setRelViewport, MGL_getViewport, MGL_setViewport, MGL_clearViewport,
MGL_setClipRect, MGL_setViewportOrg
****************************************************************************/
void MGLAPI MGL_setRelViewportDC(
	MGLDC *dc,
	rect_t view)
{
	MGL_offsetRect(view,dc->a.viewPort.left,dc->a.viewPort.top);
	MGL_setViewportDC(dc,view);
}

/****************************************************************************
DESCRIPTION:
Sets the logical viewport origin.

HEADER:
mgraph.h

PARAMETERS:
org	- New logical viewport origin.

REMARKS:
This function sets the currently active viewport origin. When a new viewport is set
with the MGL_setViewport function, the viewport origin is reset to (0,0), which
means that any primitives drawn at pixel location (0,0) will appear at the top left
hand corner of the viewport.

You can change the logical coordinate of the viewport origin to any value you
please, which will effectively offset all drawing within the currently active
viewport. Hence if you set the viewport origin to (10,10), drawing a pixel at (10,10)
would make it appear at the top left hand corner of the viewport.

SEE ALSO:
MGL_setViewportOrgDC, MGL_getViewportOrg, MGL_setViewport
****************************************************************************/
void  MGLAPI MGL_setViewportOrg(
	point_t org)
{
	DC.a.viewPortOrg = org;
	MGL_offsetRect(DC.a.viewPort,-org.x,-org.y);
}

/****************************************************************************
DESCRIPTION:
Sets the logical viewport origin for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to change viewport for
org	- New logical viewport origin.

REMARKS:
This function is the same as MGL_setViewportOrg, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_setViewportOrg, MGL_getViewportOrg, MGL_setViewport
****************************************************************************/
void  MGLAPI MGL_setViewportOrgDC(
	MGLDC *dc,
	point_t org)
{
	if (dc == _MGL_dcPtr) {
		dc->a.viewPortOrg = DC.a.viewPortOrg = org;
		MGL_offsetRect(DC.a.viewPort,-org.x,-org.y);
		dc->a.viewPort = DC.a.viewPort;
		}
	else {
		dc->a.viewPortOrg = org;
		MGL_offsetRect(dc->a.viewPort,-org.x,-org.y);
		}
}

/****************************************************************************
DESCRIPTION:
Returns the current viewport origin.

HEADER:
mgraph.h

PARAMETERS:
org	- Place to store the viewport origin

REMARKS:
This function returns a copy of the currently active viewport origin. When a new
viewport is set with the MGL_setViewport function, the viewport origin is reset to
(0,0), which means that any primitives drawn at pixel location (0,0) will appear at
the top left hand corner of the viewport.

You can change the logical coordinate of the viewport origin to any value you
please, which will effectively offset all drawing within the currently active
viewport. Hence if you set the viewport origin to (10,10), drawing a pixel at (10,10)
would make it appear at the top left hand corner of the viewport.

SEE ALSO:
MGL_getViewportOrgDC, MGL_setViewport, MGL_setViewportOrg
****************************************************************************/
void MGLAPI MGL_getViewportOrg(
	point_t *org)
{
	*org = DC.a.viewPortOrg;
}

/****************************************************************************
DESCRIPTION:
Returns the current viewport origin for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to change viewport for
org	- Place to store the viewport origin

REMARKS:
This function is the same as MGL_getViewportOrg, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_getViewportOrg, MGL_setViewport, MGL_setViewportOrg
****************************************************************************/
void MGLAPI MGL_getViewportOrgDC(
	MGLDC *dc,
	point_t *org)
{
	if (dc == _MGL_dcPtr)
		*org = DC.a.viewPortOrg;
	else
		*org = dc->a.viewPortOrg;
}

/****************************************************************************
DESCRIPTION:
Returns the currently active viewport.

HEADER:
mgraph.h

PARAMETERS:
view	- Place to store the current viewport

REMARKS:
This function returns a copy of the currently active viewport. These dimensions are
global to the entire device context surface. When the viewport is changed with this
function, the viewport origin is reset to (0,0).

All output in MGL is relative to the current viewport, so by changing the viewport
to a new value you can make all output appear in a different rectangular portion of
the device surface.

SEE ALSO:
MGL_getViewportDC, MGL_setViewport, MGL_setRelViewport, MGL_setViewportOrg,
MGL_clearViewport, MGL_setClipRect
****************************************************************************/
void MGLAPI MGL_getViewport(
	rect_t *view)
{ *view = DC.a.viewPort; }

/****************************************************************************
DESCRIPTION:
Returns the currently active viewport for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc		- Device context to change viewport for
view	- Place to store the current viewport

REMARKS:
This function is the same as MGL_getViewport, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_getViewport, MGL_setViewport, MGL_setRelViewport, MGL_setViewportOrg,
MGL_clearViewport, MGL_setClipRect
****************************************************************************/
void MGLAPI MGL_getViewportDC(
	MGLDC *dc,
	rect_t *view)
{
	if (dc == _MGL_dcPtr)
		*view = DC.a.viewPort;
	else
		*view = dc->a.viewPort;
}

/****************************************************************************
DESCRIPTION:
Converts a point from global to local coordinates.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context in which the point is defined
p	- Pointer to point to be converted

REMARKS:
This function converts a coordinate from global coordinates to local coordinates.
Global coordinates are defined relative to the entire output device context surface,
while local coordinates are relative to the currently active viewport.

This routine is usually used to convert mouse coordinate values from global screen
coordinates to the local coordinate system of the currently active viewport.

SEE ALSO:
MGL_globalToLocalDC, MGL_localToGlobal
****************************************************************************/
void MGLAPI MGL_globalToLocal(
	point_t *p)
{
	p->x -= DC.a.viewPort.left;
	p->y -= DC.a.viewPort.top;
}

/****************************************************************************
DESCRIPTION:
Converts a point from global to local coordinates.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context in which the point is defined
p	- Pointer to point to be converted

REMARKS:
This function is the same as MGL_globalToLocal, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_globalToLocal, MGL_localToGlobal
****************************************************************************/
void MGLAPI MGL_globalToLocalDC(
	MGLDC *dc,
	point_t *p)
{
	if (dc == _MGL_dcPtr) {
		p->x -= DC.a.viewPort.left;
		p->y -= DC.a.viewPort.top;
		}
	else {
		p->x -= dc->a.viewPort.left;
		p->y -= dc->a.viewPort.top;
		}
}

/****************************************************************************
DESCRIPTION:
Converts a point from local to global coordinates.

HEADER:
mgraph.h

PARAMETERS:
p	- Pointer to point to be converted

REMARKS:
This function converts a coordinate from local coordinates to global coordinates.
Global coordinates are defined relative to the entire output device display, while
local coordinates are relative to the currently active viewport.

SEE ALSO:
MGL_localToGlobalDC, MGL_globalToLocal
****************************************************************************/
void MGLAPI MGL_localToGlobal(
	point_t *p)
{
	p->x += DC.a.viewPort.left;
	p->y += DC.a.viewPort.top;
}

/****************************************************************************
DESCRIPTION:
Converts a point from local to global coordinates for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context in which the point is defined
p	- Pointer to point to be converted

REMARKS:
This function is the same as MGL_localToGlobal, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_localToGlobal, MGL_globalToLocal
****************************************************************************/
void MGLAPI MGL_localToGlobalDC(
	MGLDC *dc,
	point_t *p)
{
	if (dc == _MGL_dcPtr) {
		p->x += DC.a.viewPort.left;
		p->y += DC.a.viewPort.top;
		}
	else {
		p->x += dc->a.viewPort.left;
		p->y += dc->a.viewPort.top;
		}
}

/****************************************************************************
DESCRIPTION:
Returns the total device x coordinate dimensions.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context

RETURNS:
Number of pixels in x direction for entire device - 1

REMARKS:
Returns the total number of pixels available along the x coordinate axis for the
currently active device context. This is different than the MGL_maxx routine which
returns the dimensions of the currently active viewport.

SEE ALSO:
MGL_sizey, MGL_maxx, MGL_maxy
****************************************************************************/
int MGLAPI MGL_sizex(
	MGLDC *dc)
{
	return (dc->size.right - dc->size.left)-1;
}

/****************************************************************************
DESCRIPTION:
Returns the total device y coordinate dimensions.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context

RETURNS:
Number of pixels in y direction for entire device - 1

REMARKS:
Returns the total number of pixels available along the y coordinate axis for the
currently active device context. This is different than the MGL_maxy routine
which returns the dimensions of the currently active viewport.

SEE ALSO:
MGL_sizex, MGL_maxx
****************************************************************************/
int MGLAPI MGL_sizey(
	MGLDC *dc)
{
	return (dc->size.bottom - dc->size.top - 1);
}

/****************************************************************************
DESCRIPTION:
Returns the current maximum x coordinate.

HEADER:
mgraph.h

RETURNS:
Maximum x coordinate in current viewport.

REMARKS:
Returns the maximum x coordinate available in the currently active viewport. This
value will change if you change the dimensions of the current viewport.

Use the MGL_sizex routine to determine the dimensions of the physical display
area available to the application.

SEE ALSO:
MGL_maxxDC, MGL_maxy, MGL_sizex, MGL_sizey
****************************************************************************/
int MGLAPI MGL_maxx(void)
{
	return (DC.a.viewPort.right - DC.a.viewPort.left - 1);
}

/****************************************************************************
DESCRIPTION:
Returns the current maximum x coordinate for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of the target x coordinate

RETURNS:
Maximum x coordinate in current viewport.

REMARKS:
This function is the same as MGL_maxx, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_maxx, MGL_maxy, MGL_sizex, MGL_sizey
****************************************************************************/
int MGLAPI MGL_maxxDC(
	MGLDC *dc)
{
	if (dc == _MGL_dcPtr)
		return (DC.a.viewPort.right - DC.a.viewPort.left - 1);
	else
		return (dc->a.viewPort.right - dc->a.viewPort.left - 1);
}

/****************************************************************************
DESCRIPTION:
Returns the current maximum y coordinate.

HEADER:
mgraph.h

RETURNS:
Maximum y coordinate in current viewport.

REMARKS:
Returns the maximum y coordinate available in the currently active viewport. This
value will change if you change the dimensions of the current viewport.

Use the MGL_sizey routine to determine the dimensions of the physical display
area available to the program.

SEE ALSO:
MGL_maxyDC, MGL_maxx, MGL_sizex, MGL_sizey
****************************************************************************/
int MGLAPI MGL_maxy(void)
{
	return (DC.a.viewPort.bottom - DC.a.viewPort.top - 1);
}

/****************************************************************************
DESCRIPTION:
Returns the current maximum y coordinate for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of the target y coordinate

RETURNS:
Maximum y coordinate in current viewport.

REMARKS:
This function is the same as MGL_maxy, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_maxy, MGL_maxx, MGL_sizex, MGL_sizey
****************************************************************************/
int MGLAPI MGL_maxyDC(
	MGLDC *dc)
{
	if (dc == _MGL_dcPtr)
		return (DC.a.viewPort.bottom - DC.a.viewPort.top - 1);
	else
		return (dc->a.viewPort.bottom - dc->a.viewPort.top - 1);
}

/****************************************************************************
DESCRIPTION:
Clears the currently active display page.

HEADER:
mgraph.h

REMARKS:
This function will clear the entire currently active display page in the current
background color. This is the fastest way to clear an entire display page, but if you
wish to only clear a portion of the page, use the MGL_clearViewport routine
instead.

SEE ALSO:
MGL_clearViewport
****************************************************************************/
void ASMAPI MGL_clearDevice(void)
{
	/* Save the current pen attributes */
	int 	svStyle = DC.a.penStyle;
	int 	svColor = DC.intColor;
	color_t	svMode = DC.a.writeMode;

	/* Set the current pen attributes to those required for the
	 * background color.
	 */
	DC.a.penStyle = MGL_BITMAP_SOLID;
	DC.intColor = DC.intBackColor;
	DC.a.writeMode = MGL_REPLACE_MODE;
	DC.r.cur.fillRect(DC.size.left,DC.size.top,DC.size.right,DC.size.bottom);

	/* Restore the pen attributes */
	DC.a.penStyle = svStyle;
	DC.intColor = svColor;
	DC.a.writeMode = svMode;
}

/****************************************************************************
DESCRIPTION:
Clears the currently active viewport.

HEADER:
mgraph.h

REMARKS:
This function will clear the currently active display page viewport to the current
background color. This is the fastest way to clear a rectangular viewport, but you
may also wish to use the MGL_fillRect routine to fill with
an arbitrary pattern instead, as this function always clears the viewport to the solid
background color.

SEE ALSO:
MGL_clearDevice, MGL_fillRect
****************************************************************************/
void MGLAPI MGL_clearViewport(void)
{
	/* Save the current pen attributes */
	int 	svStyle = DC.a.penStyle;
	color_t	svColor = DC.intColor;
#ifndef	MGL_LITE
	int		svMode = DC.a.writeMode;
#endif

	/* Set the current pen attributes to those required for the
	 * background color.
	 */
	DC.a.penStyle = MGL_BITMAP_SOLID;
	DC.intColor = DC.intBackColor;
#ifndef	MGL_LITE
	DC.r.setWriteMode(MGL_REPLACE_MODE);
#endif
	DC.r.cur.fillRect(DC.a.viewPort.left,DC.a.viewPort.top,
		DC.a.viewPort.right,DC.a.viewPort.bottom);

	/* Restore the pen attributes */
	DC.a.penStyle = svStyle;
	DC.intColor = svColor;
#ifndef	MGL_LITE
	DC.r.setWriteMode(svMode);
#endif
}

/****************************************************************************
DESCRIPTION:
Sets the current clipping rectangle.

HEADER:
mgraph.h

PARAMETERS:
clip	- New clipping rectangle to be used

REMARKS:
Sets the current clipping rectangle coordinates. The current clipping rectangle is
used to clip all output, and is always defined as being relative to the currently active
viewport The clipping rectangle can be no larger than the currently active viewport,
and will be truncated if an attempt is made to allow clipping outside of the active
viewport.

SEE ALSO:
MGL_setClipRectDC, MGL_getClipRect, MGL_setViewport, MGL_getViewport
****************************************************************************/
void MGLAPI MGL_setClipRect(
	rect_t clip)
{
	rect_t	view;

	view.left = view.top = 0;
	view.right = DC.a.viewPort.right - DC.a.viewPort.left;
	view.bottom = DC.a.viewPort.bottom - DC.a.viewPort.top;
	MGL_sectRectFast(view,clip,&DC.a.clipRect);
	DC.r.setClipRect(&DC,DC.a.clipRect.left,DC.a.clipRect.top,
				   DC.a.clipRect.right,DC.a.clipRect.bottom);
}

/****************************************************************************
DESCRIPTION:
Sets the current clipping rectangle for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc		- Display device context in which the rectangle is located .
clip	- New clipping rectangle to be used

REMARKS:
This function is the same as MGL_setClipRect, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_setClipRect, MGL_getClipRect, MGL_setViewport, MGL_getViewport
****************************************************************************/
void MGLAPI MGL_setClipRectDC(
	MGLDC *dc,
	rect_t clip)
{
	rect_t	view;

	view.left = view.top = 0;
	view.right = dc->a.viewPort.right - dc->a.viewPort.left;
	view.bottom = dc->a.viewPort.bottom - dc->a.viewPort.top;
	if (dc == _MGL_dcPtr) {
		MGL_sectRectFast(view,clip,&DC.a.clipRect);
		DC.r.setClipRect(&DC,DC.a.clipRect.left,DC.a.clipRect.top,
					   DC.a.clipRect.right,DC.a.clipRect.bottom);
		dc->a.clipRect = DC.a.clipRect;
		}
	else {
		MGL_sectRectFast(view,clip,&dc->a.clipRect);
		dc->r.setClipRect(dc,dc->a.clipRect.left,dc->a.clipRect.top,
						dc->a.clipRect.right,dc->a.clipRect.bottom);
		}
}

/****************************************************************************
DESCRIPTION:
Returns the current clipping rectangle.

HEADER:
mgraph.h

PARAMETERS:
clip	- Place to store the current clipping rectangle

REMARKS:
Returns the current clipping rectangle coordinates. The current clipping rectangle is
used to clip all output, and is always defined as being relative to the currently active
viewport. The clipping rectangle can be no larger than the currently active viewport.

SEE ALSO:
MGL_getClipRectDC, MGL_setClipRect, MGL_getClipMode, MGL_getClipMode.
****************************************************************************/
void MGLAPI MGL_getClipRect(
	rect_t *clip)
{ *clip = DC.a.clipRect; }


/****************************************************************************
DESCRIPTION:
Returns the current clipping rectangle for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc		- Display device context in which the rectangle is defined
clip	- Place to store the current clipping rectangle

REMARKS:
This function is the same as MGL_getClipRect, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_getClipRect, MGL_setClipRect, MGL_getClipMode, MGL_getClipMode.
****************************************************************************/
void MGLAPI MGL_getClipRectDC(
	MGLDC *dc,
	rect_t *clip)
{
	if (dc == _MGL_dcPtr)
		*clip = DC.a.clipRect;
	else
		*clip = dc->a.clipRect;
}

/****************************************************************************
DESCRIPTION:
Sets the clipping mode.

HEADER:
mgraph.h

PARAMETERS:
mode	- True for clipping to be turned on, false for no clipping

REMARKS:
Sets the current clipping mode. You can selectively turn clipping on and off for
MGL, in order to speed up some operations. Clipping is turned on by default, and
generally you will want to leave clipping enabled, however if you are doing your
own rasterizing and perform your own clipping you may want to turn this off for
extra performance from MGL.

SEE ALSO:
MGL_setClipModeDC, MGL_getClipMode
****************************************************************************/
void MGLAPI MGL_setClipMode(
	ibool mode)
{
	if ((DC.a.clip = mode) == true) {
		/* Reset the clipping rectangle to the current clip rect */
		MGL_setClipRect(DC.a.clipRect);
		}
	else {
		/* Set the clipping rectangle to full screen */
		DC.r.setClipRect(&DC,0,0,MGL_sizex(&DC)+1,MGL_sizey(&DC)+1);
		}
}

/****************************************************************************
DESCRIPTION:
Sets the clipping mode for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc		- Display device context of the target clipping mode
mode	- True for clipping to be turned on, false for no clipping

REMARKS:
This function is the same as MGL_setClipMode, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_setClipMode, MGL_getClipMode
****************************************************************************/
void MGLAPI MGL_setClipModeDC(
	MGLDC *dc,
	ibool mode)
{
	if ((dc->a.clip = mode) == true) {
		/* Reset the clipping rectangle to the current clip rect */
		MGL_setClipRectDC(dc,dc->a.clipRect);
		}
	else {
		/* Set the clipping rectangle to full screen */
		if (dc == _MGL_dcPtr) {
			DC.r.setClipRect(&DC,0,0,MGL_sizex(&DC)+1,MGL_sizey(&DC)+1);
			dc->a.clipRect = DC.a.clipRect;
			}
		else
			dc->r.setClipRect(dc,0,0,MGL_sizex(&DC)+1,MGL_sizey(&DC)+1);
		}
}

/****************************************************************************
DESCRIPTION:
Returns the current clipping mode.

HEADER:
mgraph.h

RETURNS:
True if clipping is on, false if not.

REMARKS:
Returns the current clipping mode. You can selectively turn clipping on and off for
MGL, in order to speed up some operations. Clipping is turned on by default, and
generally you will want to leave clipping enabled.

SEE ALSO:
MGL_getClipModeDC, MGL_setClipMode, MGL_getClipRect, MGL_setClipRect
****************************************************************************/
ibool MGLAPI MGL_getClipMode(void)
{ return DC.a.clip; }


/****************************************************************************
DESCRIPTION:
Returns the current clipping mode for a specific DC.

HEADER:
mgraph.h

PARAMETERS:
dc	- Display device context of the target clipping mode

RETURNS:
True if clipping is on, false if not.

REMARKS:
This function is the same as MGL_getClipMode, however the device context
does not have to be the current device context.

SEE ALSO:
MGL_getClipMode, MGL_setClipMode, MGL_getClipRect, MGL_setClipRect
****************************************************************************/
ibool MGLAPI MGL_getClipModeDC(
	MGLDC *dc)
{
	if (dc == _MGL_dcPtr)
		return DC.a.clip;
	else
		return dc->a.clip;
}

