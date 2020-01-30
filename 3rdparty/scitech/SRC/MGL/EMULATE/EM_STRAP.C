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
* Description:	C based emulation routines for flat top/bottom trapezoid
*				rendering. All these routines should be written entirely
*				in assembler or at least with optimised C code for the
*				target environment. Implementing this routines will provide
*				the single largest speedup for polygon rendering code. None
*				of these routines work with any write mode except REPLACE.
*
*				Note also that these routines perform clipping of
*				trapezoids on a scissoring basis. Clipping is not performed
*				by the low level driver routines, so we get here if a
*				trapezoid needs to be clipped. Doing the clipping here is
*				*very* slow for non-solid traps, but efficient clipping
*				at this level is pointless. By the time the rendering code
*				gets here, the polygons should already have been clipped by
*				a high level 3D API so the low level code will simply
*				run at full speed.
*
*				We only currently have support for 16 bit zbuffer rendering
*				in software. This code is never used for 32 bit versions
*				of the library, so we have not bothered to write 24 and
*				32 bit versions.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_FIX3D

/*------------------------- Implementation --------------------------------*/

#define	BEGIN_X1X2()	int ix1,ix2,y = DC.tr.y
#define	BEGIN_Z16()		zbuf16_t z,z0,_HUGE *zbuf
#define	BEGIN_Z32()		zbuf32_t z,_HUGE *zbuf
#define	BEGIN_C()		int	dx;	fix32_t ic1,ic2,cSlope
#define	BEGIN_RGB()		fix32_t dx,ir1,ir2,rSlope,ig1,ig2,gSlope,ib1,ib2,bSlope

#define END_X1X2()		DC.tr.y = y

void _ASMAPI __EMU__rgbTrap(void)
/****************************************************************************
*
* Function:		__EMU__rgbTrap
*
* Description:	Renders a smooth shaded RGB trapezoid. The
*				parameters for the trapezoid are found in the DC.tr
*				structure.
*
****************************************************************************/
{
	pixelFunc   putPixel;
	BEGIN_X1X2();
	BEGIN_RGB();

	if (DC.mi.bitsPerPixel > 8)
		putPixel = DC.r.putPixel;
	else
		putPixel = DC.r.ditherPixel;

	while (DC.tr.count--) {
		if (y >= DC.intClipRect.top && y < DC.intClipRect.bottom) {
			ix1 = MGL_FIXROUND(DC.tr.x1);
			ix2 = MGL_FIXROUND(DC.tr.x2);
			ir1 = DC.tr.r1;	ir2 = DC.tr.r2;
			ig1 = DC.tr.g1;	ig2 = DC.tr.g2;
			ib1 = DC.tr.b1;	ib2 = DC.tr.b2;
			if (ix2 < ix1) {
				SWAP(ix1,ix2);
				SWAP(ir1,ir2);
				SWAP(ig1,ig2);
				SWAP(ib1,ib2);
				}
			/* Compute color slope */
			dx = MGL_TOFIX(ix2 - ix1 + 1);
			if (dx <= MGL_FIX_1) {
				rSlope = ir2 - ir1;
				gSlope = ig2 - ig1;
				bSlope = ib2 - ib1;
				}
			else {
				rSlope = MGL_FixDiv(ir2 - ir1,dx);
				gSlope = MGL_FixDiv(ig2 - ig1,dx);
				bSlope = MGL_FixDiv(ib2 - ib1,dx);
				}
			while (ix1 < ix2) {
				if (ix1 >= DC.intClipRect.left && ix1 < DC.intClipRect.right) {
					DC.intColor = MGL_packColorFast(&DC.pf,MGL_FIXTOINT(ir1),
						MGL_FIXTOINT(ig1),MGL_FIXTOINT(ib1));
					putPixel(ix1,y);
					}
				ix1++;
				ir1 += rSlope;	ig1 += gSlope;	ib1 += bSlope;
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.r1 += DC.tr.rSlope1;	DC.tr.r2 += DC.tr.rSlope2;
		DC.tr.g1 += DC.tr.gSlope1;	DC.tr.g2 += DC.tr.gSlope2;
		DC.tr.b1 += DC.tr.bSlope1;	DC.tr.b2 += DC.tr.bSlope2;
		y++;
		}
	END_X1X2();
}

#endif	/* MGL_FIX3D */
