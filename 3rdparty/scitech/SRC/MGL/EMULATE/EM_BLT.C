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
* Description:	C based emulation routines for BitBlt.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__bitBlt(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,int incx,int incy)
/****************************************************************************
*
* Function:		__EMU__bitBlt
* Parameters:	dc		- Device context
* 				left	- Left coordinate of are to Blt from
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*				incx	- Do we increment or decrement in x
*				incy	- Do we increment or decrement in y
*				op		- Write mode to use during Blt
*
* Description:	Blts a block of image data from one place on the device
*				context to another place on the device context. The values
*				of 'incx' and 'incy' will have been set by the high level
*				code to ensure that the transfer will take place correctly
*				for overlapping regions.
*
*               This routine performs the blt by reading a scanline into
*				the scratch buffer and writing it to the destination
*				location. Even though this may seem slow, it is actually
*				the _fastest_ way of moving information from one location
*				to another on the display surface. The reason for this
*				is that the scanline read and write routines are optimised
*				to transfer data 32 bits at a time, and the system memory
*				buffer is always DWORD aligned. Hence the writes to the
*				system memory buffer run at full speed, and reads are
*				basically free because the scanline of information will be
*				sitting directly on the main CPU internal cache. Thus we
*				have a fast 32 bit bitBlt routine, while an inplace routine
*				would have to be written using byte copies (to correctly
*				account for overlapping regions).
*
****************************************************************************/
{
	rect_t	c,d;

	c.left = left;
	c.right = right;
	d.left = dstLeft;
	d.right = dstLeft + (right - left);
	if (incy) {
		c.top = top;
		c.bottom = top + 1;
		d.top = dstTop;
		d.bottom = dstTop + 1;
		}
	else {
		c.bottom = bottom;
		c.top = bottom - 1;
		d.bottom = dstTop + (bottom - top);
		d.top = d.bottom - 1;
		}

	dc->r.beginPixel(dc);
	if (incy) {
		while (1) {
			dc->r.getScanLine(dc,c.top,c.left,c.right,_MGL_buf,c.left,MGL_REPLACE_MODE);
			dc->r.putScanLine(dc,d.top,d.left,d.right,_MGL_buf,c.left,op);
			c.top++;	c.bottom++;
			d.top++;	d.bottom++;
			if (c.bottom > bottom)
				break;
			}
		}
	else {
		while (1) {
			dc->r.getScanLine(dc,c.top,c.left,c.right,_MGL_buf,c.left,MGL_REPLACE_MODE);
			dc->r.putScanLine(dc,d.top,d.left,d.right,_MGL_buf,c.left,op);
			c.top--;	c.bottom--;
			d.top--;	d.bottom--;
			if (c.top < top)
				break;
			}
		}
	dc->r.endPixel(dc);
	incx = incx;
}

#endif	/* !MGL_LITE */
