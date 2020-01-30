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
* Description:  Line clipping routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/
#include "clipline.h"

/*------------------------- Implementation --------------------------------*/


/****************************************************************************
DESCRIPTION:
Clips a line to a specified fixed point clipping rectangle.

HEADER:
mgraph.h

PARAMETERS:
fx1	- Pointer to x coordinate of first endpoint to clip
fy1	- Pointer to y coordinate of first endpoint to clip
fx2	- Pointer to x coordinate of second endpoint to clip
fy2	- Pointer to y coordinate of second endpoint to clip
left	- Left coordinate of clip rectangle
top	- Top coordinate of clip rectangle
right	- Right coordinate of clip rectangle
bottom	- Bottom coordinate of clip rectangle

RETURNS:
True if clipped line is inside the clip rectangle, false if completely outside.

REMARKS:
This function will clip the line defined by the endpoints (x1,y1) and (x2,y2) to the
specified clipping rectangle. If the line is accepted (i.e. it intersects with the
clipping rectangle) then the routine will return true and the line endpoints will be
updated to reflect the clipped line's new endpoints. If the line is completely outside
of the clipping rectangle, the routine will return false.

Note that all coordinates are passed in 16.16 fixed point format, as is the clipping
rectangle, to provide for maximum precision in the clipping calculations.
****************************************************************************/
ibool MGLAPI MGL_clipLineFX(
	fix32_t *fx1,
	fix32_t *fy1,
	fix32_t *fx2,
	fix32_t *fy2,
	fix32_t left,
	fix32_t top,
	fix32_t right,
	fix32_t bottom)
{
	int		outcode1,outcode2;		/* Outcodes for the two endpoints	*/
	int		outcodeOut;				/* Outcode of endpoint outside		*/
	fix32_t	x,y,dx,dy;

	/* Set up the initial 4 bit outcodes */

	SETOUTCODES(outcode1,x1,y1);
	SETOUTCODES(outcode2,x2,y2);

	if (outcode1 & outcode2)			/* Trivial rejection			*/
		return REJECT;
	if (!(outcode1 | outcode2))			/* Trivial acceptance			*/
		return ACCEPT;

	dx = x2 - x1;						/* Compute dx and dy once only	*/
	dy = y2 - y1;

	while (true) {
		/* Determine which endpoint is currently outside of clip rectangle
		 * and pick it to be clipped.
		 */

		if (outcode2)
			outcodeOut = outcode2;
		else
			outcodeOut = outcode1;

		/* Clip the endpoint to one of the appropriate boundaries...	*/

		if (outcodeOut & 1) {				/* Clip to bottom boundary	*/
			x = x1 + MGL_FixMulDiv(dx,bottom-MGL_FIX_1-y1,dy);
			y = bottom-MGL_FIX_1;
			}
		else if (outcodeOut & 2) {			/* Clip to top boundary		*/
			x = x1 + MGL_FixMulDiv(dx,top-y1,dy);
			y = top;
			}
		else if (outcodeOut & 4) {			/* Clip to right boundary	*/
			y = y1 + MGL_FixMulDiv(dy,right-MGL_FIX_1-x1,dx);
			x = right-MGL_FIX_1;
			}
		else /*if (outcodeOut & 8)*/ {		/* Clip to left boundary	*/
			y = y1 + MGL_FixMulDiv(dy,left-x1,dx);
			x = left;
			}

		if (outcodeOut == outcode1) {
			x1 = x;	y1 = y;
			SETOUTCODES(outcode1,x,y);
			}
		else {
			x2 = x; y2 = y;
			SETOUTCODES(outcode2,x,y);
			}

		if (outcode1 & outcode2)		/* Trivial rejection			*/
			return REJECT;
		if (!(outcode1 | outcode2))		/* Trivial acceptance			*/
			return ACCEPT;
		}
}
