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
* Description:	C based emulation routines for transBlt.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__srcTransBlt(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
/****************************************************************************
*
* Function:		__EMU__srcTransBlt
* Parameters:	dst			- Destination Device context
*				src			- Source device context
* 				left		- Left coordinate of are to Blt from
*				top			- Top coordinate of image to Blt from
*				right		- Right coordinate of image to Blt from
*				bottom		- Bottom coordinate of image to Blt from
*				dstLeft		- Left coordinate to Blt to
*				dstTop		- Right coordinate to Blt to
*				transparent	- Transparent color
*
****************************************************************************/
{
	int		i,count,dstRight = (dstLeft + (right - left));
	int		bytesPerLine = src->mi.bytesPerLine;
	uchar	_HUGE *p;

	p = _MGL_pixelAddr2(src->surface,left,top,bytesPerLine,
		src->mi.bitsPerPixel);

	/* Display data by calling device driver to dump scanlines	*/
	dst->r.beginPixel(dst);
	i = dstTop;
	count = bottom - top;
	while (count--) {
		dst->r.putSrcTransparent(dst,i++,dstLeft,dstRight,p,transparent);
		p += bytesPerLine;
		NORMALISE_PTR(p);
		}
	dst->r.endPixel(dst);
	op = op;
}

void _ASMAPI __EMU__dstTransBlt(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,int op,color_t transparent)
/****************************************************************************
*
* Function:		__EMU__dstTransBlt
* Parameters:	dst			- Destination Device context
*				src			- Source device context
* 				left		- Left coordinate of are to Blt from
*				top			- Top coordinate of image to Blt from
*				right		- Right coordinate of image to Blt from
*				bottom		- Bottom coordinate of image to Blt from
*				dstLeft		- Left coordinate to Blt to
*				dstTop		- Right coordinate to Blt to
*				transparent	- Transparent color
*
****************************************************************************/
{
	int		i,count,dstRight = (dstLeft + (right - left));
	int		bytesPerLine = src->mi.bytesPerLine;
	uchar	_HUGE *p;

	p = _MGL_pixelAddr2(src->surface,left,top,bytesPerLine,
		src->mi.bitsPerPixel);

	/* Display data by calling device driver to dump scanlines	*/
	dst->r.beginPixel(dst);
	i = dstTop;
	count = bottom - top;
	while (count--) {
		dst->r.putDstTransparent(dst,i++,dstLeft,dstRight,p,transparent);
		p += bytesPerLine;
		NORMALISE_PTR(p);
		}
	dst->r.endPixel(dst);
	op = op;
}

#endif	/* !MGL_LITE */
