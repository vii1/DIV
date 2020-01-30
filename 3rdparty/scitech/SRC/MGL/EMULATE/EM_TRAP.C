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

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

#define	BEGIN_X1X2()	int ix1,ix2,y = DC.tr.y
#define	BEGIN_Z16()		zbuf16_t z,z0,_HUGE *zbuf
#define	BEGIN_Z32()		zbuf32_t z,_HUGE *zbuf
#define	BEGIN_C()		int	dx;	fix32_t ic1,ic2,cSlope
#define	BEGIN_RGB()		fix32_t dx,ir1,ir2,rSlope,ig1,ig2,gSlope,ib1,ib2,bSlope

#define END_X1X2()		DC.tr.y = y

void _ASMAPI __EMU__trap(void)
/****************************************************************************
*
* Function:		__EMU__trap
*
* Description:	Renders a flat shaded trapezoid. The parameters for the
*				trapezoid are found in the DC.tr structure.
*
****************************************************************************/
{
	BEGIN_X1X2();

	while (DC.tr.count--) {
		if (y >= DC.intClipRect.top && y < DC.intClipRect.bottom) {
			ix1 = MGL_FIXROUND(DC.tr.x1);
			ix2 = MGL_FIXROUND(DC.tr.x2);
			ix1 = MAX(ix1,DC.intClipRect.left);
			ix2 = MIN(ix2,DC.intClipRect.right);
			DC.r.cur.scanLine(y,ix1,ix2);
			}
		DC.tr.x1 += DC.tr.slope1;	DC.tr.x2 += DC.tr.slope2;
		y++;
		}
	END_X1X2();
}

#endif	/* !MGL_LITE */
