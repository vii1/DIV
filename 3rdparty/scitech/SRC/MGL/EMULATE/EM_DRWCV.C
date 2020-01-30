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
* Description:	C based emulation routine for drawing single vector font
*				glyph.
*
*
****************************************************************************/

#include "mgl.h"

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__drawCharVec(int x,int y,vector *vec,int sz_nx,
		int sz_ny,int sz_dx,int sz_dy)
/****************************************************************************
*
* Function:		__EMU__drawCharVec
* Parameters:	x,y		- Coordinate to start drawing text at
*				vec		- Vector font glyph to draw
*				sz_nx	- X scale numerator
*				sz_ny	- Y scale numrtator
*				sz_dx	- X scale denominator
*				sz_dy	- Y scale denominator
*
* Description:	Emulated C routine to draw a single vector glyph by calling
*				the device context line drawing routine.
*
****************************************************************************/
{
	int		oldx,oldy,newx,newy;
	fix32_t fx1,fy1,fx2,fy2;

	/* Perform the sequence of operations to draw the character */

	newx = newy = 0;
	while (vec->op.op1) {
		oldx = newx;
		oldy = newy;
		newx = (sz_nx * vec->op.dx) / sz_dx;
		newy = (sz_ny * vec->op.dy) / sz_dy;
		if (!vec->op.op2) {
			fx1 = MGL_TOFIX(x+oldx);
			fy1 = MGL_TOFIX(y-oldy);
			fx2 = MGL_TOFIX(x+newx);
			fy2 = MGL_TOFIX(y-newy);
			if ((fx2 - fx1) < 0) {	/* Ensure X1 < X2 for line routine	*/
				SWAP(fx2,fx1);
				SWAP(fy2,fy1);
				}
			DC.r.cur.line(fx1,fy1,fx2,fy2);
			}
		vec++;
		}
}

#endif	/* !MGL_LITE */
