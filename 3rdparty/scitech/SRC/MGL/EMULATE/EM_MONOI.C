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
* Description:	C based emulation routine for monochrome image rendering.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__putMonoImage(MGLDC *dc,int x,int y,int byteWidth,
	int height,uchar *image)
/****************************************************************************
*
* Function:		__EMU__putMonoImage
* Parameters:	dc		- Device context
* Parameters:	x			- X coordinate to place image at
*				y			- Y coordinate to place image at
*				byteWidth	- Width of the image in bytes (8 pixels)
*				height		- Height of the image
*				image		- Pointer to the image data
*
* Description:	Draws a monochromatic image at the position (x,y) using
*				the definition provided by the data pointed to by 'image'.
*				The bitmap image definition should be width*height bytes in
*				size. Where a bit is a 1 in the image defintion, a pixel
*				will be plotted in the current foreground color. Where a bit
*				is a zero, the data is left unchanged.
*
*				We emulate this by repeatedly calling the MGL_pixelCoord
*				routine (to ensure correct clipping).
*
*				The image will be drawn in the current drawing color and
*				in the current write mode.
*
****************************************************************************/
{
	rect_t	r,d,clipRect;
	int		i,j,k,clip,oldWriteMode;
	color_t	oldColor;
	uchar	byte;
	MGLDC	*oldDC = _MGL_dcPtr;

	/* Check to see whether we need to clip the image or not. If we dont,
	 * then we can use the faster unclipped putpixel routines in the
	 * device driver. If the image is entirely off the display, we clip
	 * it entirely.
	 */

	MGL_makeCurrentDC(dc);
	if (dc != &DC)
		clipRect 	= dc->intClipRect;
	else
		clipRect 	= DC.intClipRect;
	r.left 		= x;
	r.top 		= y;
	r.right 	= x + (byteWidth << 3);
	r.bottom 	= y + height;
	if (MGL_disjointRect(r,clipRect))
		return;									/* Trivial reject 	*/

	MGL_unionRect(r,clipRect,&d);
	clip = !MGL_equalRect(r,d);					/* Should we clip?	*/

	if (dc != &DC) {
		/* This is probably coming from the mouse driver code in a 24 bit
		 * banked framebuffer mode.
		 */
		oldWriteMode = DC.a.writeMode;
		oldColor = DC.a.color;
		DC.r.setWriteMode(dc->a.writeMode);
		DC.r.setColor(dc->intColor);
		}
	MGL_beginPixel();
	for (j = y; j < y + height; j++) {
		for (i = x; i < x + byteWidth*8;) {
			byte = *image++;
			for (k = 0; k < 8; k++,i++) {
				if (byte & 0x80) {
					if (!clip || MGL_ptInRectCoord(i,j,clipRect))
						DC.r.putPixel(i,j);
					}
				byte <<= 1;
				}
			}
		}
	MGL_endPixel();
	MGL_makeCurrentDC(oldDC);
	if (dc != &DC) {
		/* This is probably coming from the mouse driver code in a 24 bit
		 * banked framebuffer mode.
		 */
		DC.r.setWriteMode(oldWriteMode);
		DC.r.setColor(oldColor);
		}
}

void _ASMAPI __EMU__putMouseImage(MGLDC *dc,int x,int y,int byteWidth,
	int height,uchar *andMask,uchar *xorMask,color_t color)
/****************************************************************************
*
* Function:		__EMU__putMouseImage
* Parameters:	dc		- Device context
* Parameters:	x			- X coordinate to place image at
*				y			- Y coordinate to place image at
*				byteWidth	- Width of the image in bytes (8 pixels)
*				height		- Height of the image
*				andMask		- Pointer to the AND mask data
*				xorMask		- Pointer to the XOR mask data
*				color		- Color to draw in
*
* Description:	Draws a mouse image at the specified position (x,y).
*
****************************************************************************/
{
	int		oldop = dc->a.writeMode;
	color_t	oldcolor = dc->a.color;
	rect_t	oldintclip = dc->intClipRect;

	dc->intClipRect.left = 0;
	dc->intClipRect.right = dc->mi.xRes+1;
	dc->intClipRect.top = 0;
	dc->intClipRect.bottom = dc->mi.yRes+1;

	dc->r.setWriteMode(MGL_REPLACE_MODE);
	dc->r.setColor(0);
	dc->a.writeMode = DC.a.writeMode;
	dc->a.color = DC.a.color;
	dc->intColor = DC.intColor;
	dc->r.putMonoImage(dc,x,y,byteWidth,height,andMask);
	dc->r.setWriteMode(MGL_XOR_MODE);
	dc->r.setColor(color);
	dc->a.writeMode = DC.a.writeMode;
	dc->a.color = DC.a.color;
	dc->intColor = DC.intColor;
	dc->r.putMonoImage(dc,x,y,byteWidth,height,xorMask);

	dc->r.setWriteMode(oldop);
	dc->r.setColor(oldcolor);
	dc->a.writeMode = DC.a.writeMode;
	dc->a.color = DC.a.color;
	dc->intColor = DC.intColor;
	dc->intClipRect = oldintclip;
}

