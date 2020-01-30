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
* Description:	C based emulation routines for Blt'ing between a memory
*				device context and another device context.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__getImage(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,void *surface,int bytesPerLine)
/****************************************************************************
*
* Function:		__EMU__getImage
* Parameters:	dc				- Device context to Blt from
* 				left			- Left coordinate of are to Blt from
*				top				- Top coordinate of image to Blt from
*				right			- Right coordinate of image to Blt from
*				bottom			- Bottom coordinate of image to Blt from
*				dstLeft			- Left coordinate to Blt to
*				dstTop			- Right coordinate to Blt to
*				op				- Write mode to use during Blt
*				surface			- Surface to blt image to
*               bytesPerLine	- bytesPerLine value for surface
*
* Description:  Blt an image from the device context into the specified
*				memory device context. The memory device context will be
*				the same pixel depth as the display device in packed pixel
*				format. This routine may be required to perform translation
*				from planar to packed pixel format internally.
*
****************************************************************************/
{
	int		i;
	uchar	_HUGE *p = _MGL_pixelAddr2(surface,dstLeft,dstTop,bytesPerLine,dc->mi.bitsPerPixel);

	/* Read data by calling device driver to read scanlines	*/
	dc->r.beginPixel(dc);
	for (i = top; i < bottom; i++, p += bytesPerLine) {
		NORMALISE_PTR(p);
		dc->r.getScanLine(dc,i,left,right,p,dstLeft,op);
		}
	dc->r.endPixel(dc);
}

void _ASMAPI __EMU__putImage(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,void *surface,int bytesPerLine, MGLDC *src)
/****************************************************************************
*
* Function:		__EMU__putImage
* Parameters:	dc				- Device context to Blt to
* 				left			- Left coordinate of are to Blt from
*				top				- Top coordinate of image to Blt from
*				right			- Right coordinate of image to Blt from
*				bottom			- Bottom coordinate of image to Blt from
*				dstLeft			- Left coordinate to Blt to
*				dstTop			- Right coordinate to Blt to
*				op				- Write mode to use during Blt
*				surface			- Surface to blt image from
*               bytesPerLine	- bytesPerLine value for surface
*
* Description:	Blt an image from a memory buffer onto the specified
*				device context. The information in the memory buffer is in
*				packed pixel format, so the underlying device driver may
*				need to do some translation if the mode is in a planar
*				format.
*
****************************************************************************/
{
	int		i,dstRight = (dstLeft + (right - left));
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *p = _MGL_pixelAddr2(surface,left,top,bytesPerLine,dc->mi.bitsPerPixel);

	/* Display data by calling device driver to dump scanlines	*/
	dc->r.beginPixel(dc);
	for (i = dstTop; i < dstBottom; i++, p += bytesPerLine) {
		NORMALISE_PTR(p);
		dc->r.putScanLine(dc,i,dstLeft,dstRight,p,left,op);
		}
	dc->r.endPixel(dc);
	src = src;
}
