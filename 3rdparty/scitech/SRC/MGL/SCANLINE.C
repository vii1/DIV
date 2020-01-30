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
* Description:  Scanline drawing routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
* Internal routines to clip and draw a scanlines in screen coordinate space
* rather than viewport coordinate space. The scan line is not drawn unless
* it is visible, and only the visible portion is scan converted.
****************************************************************************/

/* {secret} */
void _ASMAPI _MGL_clipScanLine(int y,int x1,int x2)
{
	if (y >= DC.intClipRect.top && y < DC.intClipRect.bottom) {
		if (x2 < x1)
			SWAP(x1,x2);
		x1 = MAX(x1,DC.intClipRect.left);
		x2 = MIN(x2,DC.intClipRect.right);
		if (x1 < x2)
			DC.r.cur.scanLine(y,x1,x2);
		}
}

/* {secret} */
void _ASMAPI _MGL_clipScanLineV(int top,int left,int right)
{
	if (!DC.a.clip || (top >= DC.a.clipRect.top && top < DC.a.clipRect.bottom)) {
		if (DC.a.clip) {
			if (right < left)
				SWAP(left,right);
			left = MAX(left,DC.a.clipRect.left);
			right = MIN(right,DC.a.clipRect.right);
			}
		if (left < right)
			DC.r.cur.scanLine(top+DC.a.viewPort.top,
			  left+DC.a.viewPort.left,right+DC.a.viewPort.left);
		}
}

#ifndef	MGL_LITE

/* {secret} */
void _ASMAPI _MGL_drawClippedScanList(int y,int length,
	short *scans)
/****************************************************************************
*
* Function:		_MGL_drawClippedScanList
* Parameters:	y		- Y coordinate to begin rendering
*				length	- Number of scanlines in the list
*				scans	- Pointer to array of scanlines to render
*
* Description:	Renders the passed in list of scanlines beginning at
*				the specified y coordinate in the current drawing
*				attributes. This routine performs clipping on a scanline
*				by scanline scissoring basis, and will be slower than the
*				above routine. The high level code will ensure that the
*				correct routine is called when clipping is required.
*
****************************************************************************/
{
	int		i,diff,xstart,xend,left,top,right,bottom;
	short	*p;

	/* Move clipping rectangle coordinates into locals */

	left = DC.intClipRect.left;
	top = DC.intClipRect.top;
	right = DC.intClipRect.right;
	bottom = DC.intClipRect.bottom;

	/* Clip the y extents first. We will trivially reject the scanline list
	 * if the y extents are outside of the clipping region
	 */
	if ((diff = (top - y)) > 0) {
		y = top;
		if ((length -= diff) <= 0)
			return;
		scans += diff * 2;
		}
	if ((diff = (y + length - bottom)) > 0) {
		if ((length -= diff) <= 0)
			return;
		}

	/* Now go through each scanline in turn, clipping them to the left and
	 * right clipping coordinates.
	 */
	for (i = 0, p = scans; i < length; i++) {
		xstart = *p;
		xend = *(p+1);
		if (xstart != xend) {
			if (xstart > xend)
				SWAP(xstart,xend);
			if (xstart >= right || xend <= left) {
				xstart = xend = 0;				/* Clip entire line		*/
				}
			else {
				if (xstart < left)
					xstart = left;				/* Clip to left edge	*/
				if (xend > right)
					xend = right;				/* Clip to right edge	*/
				}
			*p++ = xstart;
			*p++ = xend;
			}
		else p += 2;
		}
	DC.r.cur.drawScanList(y,length,scans);
}

#endif /* !MGL_LITE */

