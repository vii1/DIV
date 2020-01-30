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
* Description:	C based emulation routines for scanline rendering.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__scanLine(int y,int x1,int x2)
/****************************************************************************
*
* Function:		__EMU__scanLine
* Parameters:	y		- Scanline to draw line at
*				x1,x2	- Extent of the scanline to draw
*
* Description:	Draws a scanline by plotting each pixel on the scanline
*				separately. Works with patterns fills etc.
*
****************************************************************************/
{
	int				i;
	uchar			pattern;
	color_t			fore,back;
	pixpattern_t	*pixmap;
	void 			(_ASMAPI *putPixel)(int x,int y);

	if (x1 == x2)				/* Ignore lines with xstart = xend	*/
		return;
	if (x2 < x1)
		SWAP(x1,x2);

#ifndef	MGL_LITE
	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		putPixel = DC.r.ditherPixel;
	else
#endif
		putPixel = DC.r.putPixel;

	MGL_beginPixel();
	switch (DC.a.penStyle) {
		case MGL_BITMAP_SOLID:
			for (i = x1; i < x2; i++)
				putPixel(i,y);
			break;
		case MGL_BITMAP_OPAQUE:
			fore = DC.a.color;
			back = DC.a.backColor;
			pattern = DC.a.penPat[y & 7];
			for (i = x1; i < x2; i++) {
				if (pattern & (0x80 >> (i & 7))) {
					DC.intColor = fore;
					putPixel(i,y);
					}
				else {
					DC.intColor = back;
					putPixel(i,y);
					}
				}
			DC.r.setColor(fore);
			break;
		case MGL_BITMAP_TRANSPARENT:
			pattern = DC.a.penPat[y & 7];
			for (i = x1; i < x2; i++)
				if (pattern & (0x80 >> (i & 7)))
					putPixel(i,y);
			break;
		case MGL_PIXMAP:
			fore = DC.a.color;
			pixmap = &DC.a.penPixPat;
			for (i = x1; i < x2; i++) {
				DC.intColor = (*pixmap)[y&7][i&7];
				putPixel(i,y);
				}
			DC.r.setColor(fore);
			break;
		}
	MGL_endPixel();
}
