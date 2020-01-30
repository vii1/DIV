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

#ifdef	MGL_FIX3D
/* {secret} */
ibool _MGL_clipZLineFX(fix32_t *fx1,fix32_t *fy1,zfix32_t *fz1,
	fix32_t *fx2,fix32_t *fy2,zfix32_t *fz2,fix32_t left,fix32_t top,
	fix32_t right,fix32_t bottom)
/****************************************************************************
*
* Function:		_MGL_clipZLineFX
* Parameters:	x1,y1,z1	- First endpoint of line to clip
*				x2,y2,z2	- Second endpoint of line to clip
*				left		- Left coordinate to clip to
*				top			- Top coordinate to clip to
*				right		- Right coordinate to clip to
*				bottom		- Bottom coordinate to clip to
* Returns:		True if line should be drawn (ie: clipped and accepted).
*
* Description:	Clips the line from p1(x,y) - p2(x,y) to the clip rectangle
*				r. The clipping is done using the Cohen Sutherland line
*				clipping algorithm in 32 bit fixed point.
*
****************************************************************************/
{
	int			outcode1,outcode2;	/* Outcodes for the two endpoints	*/
	int			outcodeOut;			/* Outcode of endpoint outside		*/
	fix32_t		x,y,dx,dy,dz;
	zfix32_t	z;

	/* Set up the initial 4 bit outcodes */
	SETOUTCODES(outcode1,x1,y1);
	SETOUTCODES(outcode2,x2,y2);

	if (outcode1 & outcode2)			/* Trivial rejection			*/
		return REJECT;
	if (!(outcode1 | outcode2))			/* Trivial acceptance			*/
		return ACCEPT;

	dx = x2 - x1;						/* Compute dx,dy,dz once only	*/
	dy = y2 - y1;
	dz = MGL_ZTOFIX(z2 - z1);

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
			x = x1 + MGL_FixMulDiv(dx,y = bottom-MGL_FIX_1-y1,dy);
			z = z1 + MGL_FIXTOZ(MGL_FixMulDiv(dz,y,dy));
			y = bottom-MGL_FIX_1;
			}
		else if (outcodeOut & 2) {			/* Clip to top boundary		*/
			x = x1 + MGL_FixMulDiv(dx,y = top-y1,dy);
			z = z1 + MGL_FIXTOZ(MGL_FixMulDiv(dz,y,dy));
			y = top;
			}
		else if (outcodeOut & 4) {			/* Clip to right boundary	*/
			y = y1 + MGL_FixMulDiv(dy,x = right-MGL_FIX_1-x1,dx);
			z = z1 + MGL_FIXTOZ(MGL_FixMulDiv(dz,x,dx));
			x = right-MGL_FIX_1;
			}
		else /* if (outcodeOut & 8) */ {	/* Clip to left boundary	*/
			y = y1 + MGL_FixMulDiv(dy,x = left-x1,dx);
			z = z1 + MGL_FIXTOZ(MGL_FixMulDiv(dz,x,dx));
			x = left;
			}

		if (outcodeOut == outcode1) {
			x1 = x;	y1 = y; z1 = z;
			SETOUTCODES(outcode1,x,y);
			}
		else {
			x2 = x; y2 = y; z2 = z;
			SETOUTCODES(outcode2,x,y);
			}

		if (outcode1 & outcode2)		/* Trivial rejection			*/
			return REJECT;
		if (!(outcode1 | outcode2))		/* Trivial acceptance			*/
			return ACCEPT;
		}
}
#endif	/* !MGL_FIX3D */

