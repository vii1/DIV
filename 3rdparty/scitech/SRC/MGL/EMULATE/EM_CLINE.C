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
* Description:  C based emulation routine for line drawing.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_FIX3D

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__cLine(fix32_t x1,fix32_t y1,fix32_t c1,fix32_t x2,
	fix32_t y2,fix32_t c2)
/****************************************************************************
*
* Function:		__EMU__cLine
* Parameters:	x1,y1,c1	- First endpoint for line
*				x2,y2,c2	- Second endpoint for line
*
* Description:  Renders a shaded line (non clipped).
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		dc,cSlope;			/* Color slopes						*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr;				/* Increment for values				*/
	ibool		lowslope = true;	/* True if slope <= 1				*/

	yincr = 1;
	dc = c2 - c1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		}
	if (dy > (dx = x2 - x1)) {
		SWAP(dx,dy);
		lowslope = false;
		}

	Eincr = 2*dy;					/* Increment to move to E pixel		*/
	d = Eincr - dx;					/* Decision var = 2 * dy - dx		*/
	NEincr = d - dx;				/* NEincr = 2 * (dy - dx)			*/
	x = MGL_FIXROUND(x1);
	y = MGL_FIXROUND(y1);
	count = MGL_FIXTOINT(dx)+1;

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);

		while (count--) {
			if (DC.mi.bitsPerPixel > 8)
				DC.intColor = DC.shadeTab[MGL_FIXTOINT(c1)];
			else DC.intColor = MGL_FIXTOINT(c1);
			DC.r.putPixel(x,y);
			x++;					/* Increment x coordinate			*/
			c1 += cSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				}
			}
		}
	else {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,yincr*(MGL_TOFIX(y)-y1))
		   - MGL_FixMul(2*dx,MGL_TOFIX(x)-x1);

		/* Compute color coordinate slope */
		cSlope = MGL_FixDiv(dc,dx);

		while (count--) {
			if (DC.mi.bitsPerPixel > 8)
				DC.intColor = DC.shadeTab[MGL_FIXTOINT(c1)];
			else DC.intColor = MGL_FIXTOINT(c1);
			DC.r.putPixel(x,y);
			y += yincr;				/* Increment y coordinate			*/
			c1 += cSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				x++;
				}
			}
		}
}

#endif	/* MGL_FIX3D */
