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
* Description:  C based emulation routine for Zbuffered line drawing. We
*				only currently have support for 16 bit zbuffer rendering
*				in software. This code is never used for 32 bit versions
*				of the library, so we have not bothered to write 24 and
*				32 bit versions.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_FIX3D

/*------------------------- Implementation --------------------------------*/

#if defined(NO_ASSEMBLER) || !defined(__INTEL__)

void _ASMAPI __EMU__zLine16(fix32_t x1,fix32_t y1,zfix32_t z1,fix32_t x2,
	fix32_t y2,zfix32_t z2)
/****************************************************************************
*
* Function:		__EMU__zLine16
* Parameters:	x1,y1,z1	- First endpoint for line
*				x2,y2,z2	- Second endpoint for line
*
* Description:  Scan convert a 3D line segment using the MidPoint Digital
*				Differential Analyser algorithm, including Zbuffer
*				coordinates. We call the DC.putPixel code directly to
*				render each pixel.
*
*				Lines must ALWAYS be passed with the X1 value less than the
*				X2 value!!
*
*				This routine expects the endpoints to be in 32 bit fixed
*				point 16.16 format and Z coordinates to be in 4.28 fixed
*				point format.
*
****************************************************************************/
{
	fix32_t		d;					/* Decision variable				*/
	fix32_t		dx,dy;				/* Dx and Dy values for the line	*/
	fix32_t		Eincr,NEincr;		/* Decision variable increments		*/
	zfix32_t	dz;					/* Current Z coordinate slope		*/
	zbuf16_t	_HUGE *zbuf;		/* Current position in Z buffer		*/
	zbuf16_t	z,zSlope;			/* Current position in Z buffer		*/
	int			x,y,count;			/* Current (x,y) and count value	*/
	int			yincr;				/* Increment for values				*/
	int			zincr;				/* Increment to get to next z scan	*/
	ibool		lowslope = true;	/* True if slope <= 1				*/
	pixelFunc	putPixel;

	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		putPixel = DC.r.ditherPixel;
	else
		putPixel = DC.r.putPixel;

	yincr = 1;
	zincr = DC.zwidth;
	dz = z2 - z1;
	if ((dy = y2 - y1) < 0) {
		dy = -dy;
		yincr = -1;
		zincr = -zincr;
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
	zbuf = _MGL_zbufAddr16(x,y);

	if (lowslope) {
		/* Adjust initial decision variable depending on the difference
		 * between the real line and the initial starting point we have
		 * selected
		 */
		d += MGL_FixMul(2*dy,MGL_TOFIX(x)-x1)
		   - MGL_FixMul(2*dx,yincr*(MGL_TOFIX(y)-y1));

		/* Compute Z buffer coordinates and slope */
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				putPixel(x,y);
				}
			x++;					/* Increment x coordinate			*/
			zbuf++;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				y += yincr;			/* Increment y						*/
				zbuf += zincr;		/* Increment zbuffer pointer		*/
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

		/* Compute Z buffer coordinates and slope */
		zSlope = MGL_TOZBUF16(MGL_ZFixDiv(dz,dx));
		z = MGL_TOZBUF16(z1);

		while (count--) {
			if (z > *zbuf) {
				*zbuf = z;
				putPixel(x,y);
				}
			y += yincr;				/* Increment y coordinate			*/
			zbuf += zincr;
			z += zSlope;

			if (d <= 0) {
				d += Eincr;			/* Choose the Eastern Pixel			*/
				}
			else {
				d += NEincr;		/* Choose the North Eastern Pixel	*/
				x++;
				zbuf++;
				}
			}
		}
}

#endif

#endif	/* MGL_FIX3D */
