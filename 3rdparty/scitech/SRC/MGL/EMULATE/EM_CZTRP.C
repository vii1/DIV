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

#if !defined(__16BIT__) || defined(NO_ASSEMBLER) || !defined(__INTEL__)

void _ASMAPI __EMU__czTrap16(void)
/****************************************************************************
*
* Function:		__EMU__czTrap16
*
* Description:	Renders a smooth shaded color index zbuffered trapezoid. The
*				parameters for the trapezoid are found in the DC.tr
*				structure.
*
****************************************************************************/
{
	BEGIN_X1X2();
	BEGIN_C();
	BEGIN_Z16();

	while (DC.tr.count--) {
		if (y >= DC.intClipRect.top && y < DC.intClipRect.bottom) {
			ix1 = MGL_FIXROUND(DC.tr.x1);
			ix2 = MGL_FIXROUND(DC.tr.x2);
			ic1 = DC.tr.c1;	ic2 = DC.tr.c2;
			if (ix2 < ix1) {
				SWAP(ix1,ix2);
				SWAP(ic1,ic2);
				}
			/* Compute color slope */
			dx = ix2 - ix1 + 1;
			if (dx <= 1)
				cSlope = ic2 - ic1;
			else
				cSlope = MGL_FixDiv(ic2 - ic1,MGL_TOFIX(dx));
			zbuf = _MGL_zbufAddr16(ix1,y);
			z0 = MGL_TOZBUF16(DC.tr.z0);
			if (DC.mi.bitsPerPixel > 8) {
				while (ix1 < ix2) {
					if (ix1 >= DC.intClipRect.left && ix1 < DC.intClipRect.right) {
						z = z0 + ((zbuf16_t*)DC.zOffset)[ix1];
						if (z > *zbuf) {
							*zbuf = z;
							DC.intColor = DC.shadeTab[MGL_FIXTOINT(ic1)];
							DC.r.putPixel(ix1,y);
							}
						}
					zbuf++;
					ix1++;
					ic1 += cSlope;
					}
				}
			else {
				while (ix1 < ix2) {
					if (ix1 >= DC.intClipRect.left && ix1 < DC.intClipRect.right) {
						z = z0 + ((zbuf16_t*)DC.zOffset)[ix1];
						if (z > *zbuf) {
							*zbuf = z;
							DC.intColor = MGL_FIXTOINT(ic1);
							DC.r.putPixel(ix1,y);
							}
						}
					zbuf++;
					ix1++;
					ic1 += cSlope;
					}
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.c1 += DC.tr.cSlope1;	DC.tr.c2 += DC.tr.cSlope2;
		DC.tr.z0 += DC.tr.dZdY;
		y++;
		}
	END_X1X2();
}

void _ASMAPI __EMU__czTrap32(void)
/****************************************************************************
*
* Function:		__EMU__czTrap32
*
* Description:	Renders a smooth shaded color index zbuffered trapezoid. The
*				parameters for the trapezoid are found in the DC.tr
*				structure.
*
****************************************************************************/
{
	BEGIN_X1X2();
	BEGIN_C();
	BEGIN_Z32();

	while (DC.tr.count--) {
		if (y >= DC.intClipRect.top && y < DC.intClipRect.bottom) {
			ix1 = MGL_FIXROUND(DC.tr.x1);
			ix2 = MGL_FIXROUND(DC.tr.x2);
			ic1 = DC.tr.c1;	ic2 = DC.tr.c2;
			if (ix2 < ix1) {
				SWAP(ix1,ix2);
				SWAP(ic1,ic2);
				}
			/* Compute color slope */
			dx = ix2 - ix1 + 1;
			if (dx <= 1)
				cSlope = ic2 - ic1;
			else
				cSlope = MGL_FixDiv(ic2 - ic1,MGL_TOFIX(dx));
			zbuf = _MGL_zbufAddr32(ix1,y);
			if (DC.mi.bitsPerPixel > 8) {
				while (ix1 < ix2) {
					if (ix1 >= DC.intClipRect.left && ix1 < DC.intClipRect.right) {
						z = DC.tr.z0 + ((zbuf32_t*)DC.zOffset)[ix1];
						if (z > *zbuf) {
							*zbuf = z;
							DC.intColor = DC.shadeTab[MGL_FIXTOINT(ic1)];
							DC.r.putPixel(ix1,y);
							}
						}
					zbuf++;
					ix1++;
					ic1 += cSlope;
					}
				}
			else {
				while (ix1 < ix2) {
					if (ix1 >= DC.intClipRect.left && ix1 < DC.intClipRect.right) {
						z = DC.tr.z0 + ((zbuf32_t*)DC.zOffset)[ix1];
						if (z > *zbuf) {
							*zbuf = z;
							DC.intColor = MGL_FIXTOINT(ic1);
							DC.r.putPixel(ix1,y);
							}
						}
					zbuf++;
					ix1++;
					ic1 += cSlope;
					}
				}
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		DC.tr.c1 += DC.tr.cSlope1;	DC.tr.c2 += DC.tr.cSlope2;
		DC.tr.z0 += DC.tr.dZdY;
		y++;
		}
	END_X1X2();
}

#endif

#endif	/* MGL_FIX3D */
