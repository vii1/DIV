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
* Description:	Bit Block Transfer routines (BitBlt's).
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*--------------------------- Global Variables ----------------------------*/

#ifndef	MGL_LITE
static	publicDevCtx_t  tempDC;	/* Temporary DC for bitmap routines		*/
#endif

/*------------------------- Implementation --------------------------------*/

#undef	MGL_bitBlt
#undef	MGL_bitBltLin
#undef	MGL_transBlt
#undef	MGL_transBltLin
#undef	MGL_getDivot
#undef	MGL_divotSize

#ifndef	MGL_LITE

/****************************************************************************
DESCRIPTION:
Draw a monochrome bitmap at the specified location.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to draw bitmap on
x	- x coordinate to draw bitmap at
y	- y coordinate to draw bitmap at
byteWidth	- Width of the bitmap in bytes
height	- Height of the bitmap in scanlines
image	- Pointer to the buffer holding the bitmap

REMARKS:
This function draws a monochrome bitmap in the current foreground color on the
current device context. Where a bit is a 1 in the bitmap definition, a pixel is plotted
in the foreground color, where a bit is a 0 the original pixels are left alone. This
function can be used to implement fast hardware pixel masking for drawing fast
transparent bitmaps on devices that do not have a native hardware transparent BitBlt
function.
****************************************************************************/
void MGLAPI MGL_putMonoImage(MGLDC *dc,
	int x,
	int y,
	int byteWidth,
	int height,
	void *image)
{
	if (dc == _MGL_dcPtr)
		dc = &DC;
	dc->r.putMonoImage(dc,x + dc->a.viewPort.left,y + dc->a.viewPort.top,
		byteWidth,height,image);
}

/****************************************************************************
DESCRIPTION:
Draw a lightweight bitmap at the specified location.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to display bitmap on
x	- x coordinate to draw bitmap at
y	- y coordinate to draw bitmap at
bitmap	- Bitmap to display
op	- Write mode to use when drawing bitmap

REMARKS:
Draws a lightweight bitmap at the specified location. The bitmap can be in any
color format, and will be translated as necessary to the color format required by the
current device context.

Note that for maximum performance when displaying 8 bit bitmaps, you should
ensure that the color palette for the bitmap is identical to the device context,
otherwise the pixels in the bitmap will be translated during the draw operation.

Supported write modes are enumerated in MGL_writeModeType.



SEE ALSO:
MGL_loadBitmap
****************************************************************************/
void MGLAPI MGL_putBitmap(
	MGLDC *dc,
	int x,
	int y,
	const bitmap_t *bitmap,
	int op)
{
	rect_t	d;

	if (dc == _MGL_dcPtr)
		dc = &DC;

	/* Clip to destination device context */
	d.left = x;						d.top = y;
	d.right = x + bitmap->width;	d.bottom = y + bitmap->height;
	if (dc->a.clip) {
		if (!MGL_sectRect(dc->a.clipRect,d,&d))
			return;
		}

	/* Adjust to bitmap coordinates */
	MGL_offsetRect(d,-x,-y);
	x += d.left;
	y += d.top;

	if (bitmap->bitsPerPixel == 1) {
		/* Bitmap is a monochrome bitmap, so simply draw this using the
		 * normal mono bitmap blitting code, but set the requested write
		 * mode first. Clipping is handled by the device driver rendering.
		 */
		int oldop = dc->a.writeMode;
		dc->r.setWriteMode(op);
		dc->r.putMonoImage(dc,x + dc->a.viewPort.left,y + dc->a.viewPort.top,
			bitmap->bytesPerLine,bitmap->height,bitmap->surface);
		dc->r.setWriteMode(oldop);
		return;
		}

	if (NEED_TRANSLATE_BM(bitmap,dc)) {
		/* Translate the pixel information when doing the Blt */
		dc->r.translateImage(dc,d.left,d.top,d.right,d.bottom,
			x + dc->a.viewPort.left,y + dc->a.viewPort.top,op,
			bitmap->surface,bitmap->bytesPerLine,bitmap->bitsPerPixel,
			bitmap->pal,bitmap->pf,NULL);
		}
	else  {
		dc->r.putImage(dc,d.left,d.top,d.right,d.bottom,
			x + dc->a.viewPort.left,y + dc->a.viewPort.top,op,
			bitmap->surface,bitmap->bytesPerLine,NULL);
		}
}

/****************************************************************************
DESCRIPTION:
Draw a section of a lightweight bitmap at the specified location.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to display bitmap on
left	- Left coordinate of section to draw
top	- Top coordinate of section to draw
right	- Right coordinate of section to draw
bottom	- Bottom coordinate of section to draw
dstLeft	- Left coordinate of destination of bitmap section
dstTop	- Right coordinate for destination of bitmap section
bitmap	- Bitmap to display
op	- Write mode to use when drawing bitmap

REMARKS:
Draws a section of a lightweight bitmap at the specified location. The bitmap can be
in any color format, and will be translated as necessary to the color format required
by the current device context.

Note that for maximum performance when displaying 8 bit bitmaps, you should
ensure that the color palette for the bitmap is identical to the device context,
otherwise the pixels in the bitmap will be translated during the draw operation.

SEE ALSO:
MGL_loadBitmap
****************************************************************************/
void MGLAPI MGL_putBitmapSection(
	MGLDC *dc,
	int left,
	int top,
	int right,
	int bottom,
	int dstLeft,
	int dstTop,
	const bitmap_t *bitmap,
	int op)
{
	rect_t	d;

	if (dc == _MGL_dcPtr)
		dc = &DC;
	if (bitmap->bitsPerPixel == 1) {
		_MGL_result = grInvalidDevice;
		return;
		}

	/* Clip the source rectangle to the bitmap dimensions */
	if (!MGL_sectRectCoord(0,0,bitmap->width,bitmap->height,left,top,right,bottom,&d))
		return;
	dstLeft += (d.left - left);	dstTop += (d.top - top);
	left = d.left;				right = d.right;
	top = d.top;				bottom = d.bottom;

	/* Clip to destination device context */
	if (dc->a.clip) {
		d.left = dstLeft;					d.top = dstTop;
		d.right = dstLeft + (right-left);	d.bottom = dstTop + (bottom-top);
		if (!MGL_sectRect(dc->a.clipRect,d,&d))
			return;
		left += (d.left - dstLeft);	right = left + (d.right - d.left);
		top += (d.top - dstTop);	bottom = top + (d.bottom - d.top);
		dstLeft = d.left;			dstTop = d.top;
		}

	/* Blt the pixels to the device */
	if (NEED_TRANSLATE_BM(bitmap,dc)) {
		/* Translate the pixel information when doing the Blt */
		dc->r.translateImage(dc,left,top,right,bottom,
			dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,op,
			bitmap->surface,bitmap->bytesPerLine,bitmap->bitsPerPixel,
			bitmap->pal,bitmap->pf,NULL);
		}
	else  {
		dc->r.putImage(dc,left,top,right,bottom,
			dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,op,
			bitmap->surface,bitmap->bytesPerLine,NULL);
		}
}

/****************************************************************************
DESCRIPTION:
Draw a lightweight bitmap mask in the specified color.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to display bitmap on
x	- x coordinate to draw bitmap at
y	- y coordinate to draw bitmap at
mask	- Monochrome bitmap mask to display
color	- Color to draw in

REMARKS:
Draws a lightweight monochrome bitmap at the specified location. This is just a
simply utility function that draws the monochrome bitmap in a specified color in
replace mode as fast as possible.

SEE ALSO:
MGL_loadBitmap
****************************************************************************/
void MGLAPI MGL_putBitmapMask(
	MGLDC *dc,
	int x,
	int y,
	const bitmap_t *mask,
	color_t color)
{
	int		oldop = dc->a.writeMode;
	color_t	oldcolor = dc->intColor;

	if (mask->bitsPerPixel != 1)
		return;
	if (dc == _MGL_dcPtr)
		dc = &DC;

	/* Punch a hole in the display with the icon's AND mask */
	dc->r.setWriteMode(MGL_REPLACE_MODE);
	dc->intColor = color;
	dc->r.putMonoImage(dc,x + dc->a.viewPort.left,y + dc->a.viewPort.top,
		mask->bytesPerLine,mask->height,mask->surface);
	dc->r.setWriteMode(oldop);
	dc->intColor = oldcolor;
}

/****************************************************************************
DESCRIPTION:
Draw a transparent lightweight bitmap at the specified location.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to display bitmap on
x	- x coordinate to draw bitmap at
y	- y coordinate to draw bitmap at
bitmap	- Bitmap to display
transparent	- Transparent color for the bitmap
sourceTrans	- True for source transparency, false for destination transparency

REMARKS:
Draws a transparent lightweight bitmap at the specified location with either source
or destination transparency. When transferring the data with source transparency,
pixels in the source image that are equal to the specified transparent color, the
related pixel in the destination buffer will remain untouched. This allows you to
quickly transfer sprites between device contexts with a single color being allocated
as a transparent color. When transferring the data with destination transparency,
pixels in the destination image that are equal to the specified transparent color will
be updated, and those pixels that are not the same will be skipped. This is
effectively the operation performed for 'blueScreen'ing or color keying and can
also be used for drawing transparent sprites. Note however that destination
transparency is very slow in software compared to source transparency!

The pixel depth and pixel format for the source bitmap and the device contexts must
be the same or this routine will simply do nothing. This routine also only works
with pixel depths that are at least 8 bits deep.

SEE ALSO:
MGL_loadBitmap, MGL_putBitmapTransparentSection
****************************************************************************/
void MGLAPI MGL_putBitmapTransparent(
	MGLDC *dc,
	int x,
	int y,
	const bitmap_t *bitmap,
	color_t transparent,
	ibool sourceTrans)
{
	rect_t	d;

	if (dc == _MGL_dcPtr)
		dc = &DC;
	if (bitmap->bitsPerPixel < 8 || (bitmap->bitsPerPixel
			!= dc->mi.bitsPerPixel)) {
		_MGL_result = grInvalidDevice;
		return;
		}
	d.left = x;						d.top = y;
	d.right = x + bitmap->width;	d.bottom = y + bitmap->height;
	if (dc->a.clip) {
		if (!MGL_sectRect(dc->a.clipRect,d,&d))
			return;
		}
	MGL_offsetRect(d,-x,-y);	/* Adjust to bitmap coordinates */
	x += d.left;
	y += d.top;

	tempDC.surface = bitmap->surface;
	tempDC.mi.bytesPerLine = bitmap->bytesPerLine;
	tempDC.mi.bitsPerPixel = bitmap->bitsPerPixel;
	tempDC.colorTab = (color_t*)bitmap->pal;
	if (sourceTrans)
		dc->r.srcTransBlt(dc,(MGLDC*)&tempDC,d.left,d.top,d.right,d.bottom,
			x + dc->a.viewPort.left,y + dc->a.viewPort.top,
			MGL_REPLACE_MODE,transparent);
	else
		dc->r.dstTransBlt(dc,(MGLDC*)&tempDC,d.left,d.top,d.right,d.bottom,
			x + dc->a.viewPort.left,y + dc->a.viewPort.top,
			MGL_REPLACE_MODE,transparent);
}

/****************************************************************************
DESCRIPTION:
Draw a section of a transparent lightweight bitmap at the specified location.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to display bitmap on
left	- Left coordinate of section to draw
top	- Top coordinate of section to draw
right	- Right coordinate of section to draw
bottom	- Bottom coordinate of section to draw
dstLeft	- Left coordinate of destination of bitmap section
dstTop	- Right coordinate for destination of bitmap section
bitmap	- Bitmap to display
transparent	- Transparent color for the bitmap
sourceTrans	- True for source transparency, false for destination transparency

REMARKS:
Draws a section of a transparent lightweight bitmap at the specified location with
either source or destination transparency. When transferring the data with source
transparency, pixels in the source image that are equal to the specified transparent 
color, the related pixel in the destination buffer will remain untouched. This allows
you to quickly transfer sprites between device contexts with a single color being 
allocated as a transparent color. When transferring the data with destination 
transparency, pixels in the destination image that are equal to the specified 
transparent color will be updated, and those pixels that are not the same will be 
skipped. This is effectively the operation performed for 'blueScreen'ing or color 
keying and can also be used for drawing transparent sprites. Note however that 
destination transparency is very slow in software compared to source transparency!

The pixel depth and pixel format for the source bitmap and the device contexts must 
be the same or this routine will simply do nothing. This routine also only works 
with pixel depths that are at least 8 bits deep.

SEE ALSO:
MGL_loadBitmap, MGL_putBitmapTransparent
****************************************************************************/
void MGLAPI MGL_putBitmapTransparentSection(
	MGLDC *dc,
	int left,
	int top,
	int right,
	int bottom,
	int dstLeft,
	int dstTop,
	const bitmap_t *bitmap,
	color_t transparent,
	ibool sourceTrans)
{
	rect_t	d;

	if (dc == _MGL_dcPtr)
		dc = &DC;
	if (bitmap->bitsPerPixel < 8 || (bitmap->bitsPerPixel
			!= dc->mi.bitsPerPixel)) {
		_MGL_result = grInvalidDevice;
		return;
		}

	/* Clip the source rectangle to the bitmap dimensions */
	if (!MGL_sectRectCoord(0,0,bitmap->width,bitmap->height,left,top,right,bottom,&d))
		return;
	dstLeft += (d.left - left);	dstTop += (d.top - top);
	left = d.left;				right = d.right;
	top = d.top;				bottom = d.bottom;

	/* Clip to destination device context */
	if (dc->a.clip) {
		d.left = dstLeft;					d.top = dstTop;
		d.right = dstLeft + (right-left);	d.bottom = dstTop + (bottom-top);
		if (!MGL_sectRect(dc->a.clipRect,d,&d))
			return;
		left += (d.left - dstLeft);	right = left + (d.right - d.left);
		top += (d.top - dstTop);	bottom = top + (d.bottom - d.top);
		dstLeft = d.left;			dstTop = d.top;
		}

	/* Perform the blit of the bitmap section */
	tempDC.surface = bitmap->surface;
	tempDC.mi.bytesPerLine = bitmap->bytesPerLine;
	tempDC.mi.bitsPerPixel = bitmap->bitsPerPixel;
	tempDC.colorTab = (color_t*)bitmap->pal;
	if (sourceTrans)
		dc->r.srcTransBlt(dc,(MGLDC*)&tempDC,left,top,right,bottom,
			dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
			MGL_REPLACE_MODE,transparent);
	else
		dc->r.dstTransBlt(dc,(MGLDC*)&tempDC,left,top,right,bottom,
			dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
			MGL_REPLACE_MODE,transparent);
}

/****************************************************************************
DESCRIPTION:
Stretches a lightweight bitmap to the specified rectangle.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to display bitmap on
dstLeft	- Left coordinate to stretch bitmap to
dstTop	- Top coordinate to stretch bitmap to
dstRight	- Right coordinate to stretch bitmap to
dstBottom	- Bottom coordinate to stretch bitmap to
bitmap	- Bitmap to display

REMARKS:
Stretches a lightweight bitmap to the specified destination rectangle on the device
context. The bitmap can be in any color format, and will be translated as necessary
to the color format required by the current device context.

Note that for maximum performance when displaying 8 bit bitmaps, you should
ensure that the color palette for the bitmap is identical to the device context,
otherwise the pixels in the bitmap will be translated during the draw operation.

SEE ALSO:
MGL_loadBitmap
****************************************************************************/
void MGLAPI MGL_stretchBitmap(
	MGLDC *dc,
	int dstLeft,
	int dstTop,
	int dstRight,
	int dstBottom,
	const bitmap_t *bitmap)
{
	rect_t	d;
	int		left = 0,top = 0,right = bitmap->width,bottom = bitmap->height;
	int		deltaSrc,deltaDst;
	fix32_t	zoomx,zoomy;
	ibool	idPal = true;

	if (dc == _MGL_dcPtr)
		dc = &DC;
	if (bitmap->bitsPerPixel < 8) {
		_MGL_result = grInvalidDevice;
		return;
		}

	/* Calculate the x zoom factor */
	deltaSrc = right - left;
	deltaDst = dstRight - dstLeft;
	if (deltaDst == deltaSrc)
		zoomx = MGL_FIX_1;
	else if (deltaDst == (deltaSrc * 2))
		zoomx = MGL_FIX_2;
	else
		zoomx = MGL_FixDiv(MGL_TOFIX(deltaDst),MGL_TOFIX(deltaSrc));

	/* Calculate the y zoom factor */
	deltaSrc = bottom - top;
	deltaDst = dstBottom - dstTop;
	if (deltaDst == deltaSrc)
		zoomy = MGL_FIX_1;
	else if (deltaDst == (deltaSrc * 2))
		zoomy = MGL_FIX_2;
	else
		zoomy = MGL_FixDiv(MGL_TOFIX(deltaDst),MGL_TOFIX(deltaSrc));

	/* Handle special case of 1:1 stretch */
	if (zoomx == MGL_FIX_1 && zoomy == MGL_FIX_1) {
		MGL_putBitmap(dc,dstLeft,dstTop,bitmap,MGL_REPLACE_MODE);
		return;
		}

	/* Clip to destination device context */
	d.left = dstLeft;
	d.top = dstTop;
	d.right = dstRight;
	d.bottom = dstBottom;
	if (dc->a.clip) {
		if (!MGL_sectRect(dc->a.clipRect,d,&d))
			return;
		}
	left += (d.left - dstLeft);
	top += (d.top - dstTop);
	right = left + MGL_FIXTOINT(MGL_FixDiv(MGL_TOFIX(d.right - d.left),
		zoomx));
	bottom = top + MGL_FIXTOINT(MGL_FixDiv(MGL_TOFIX(d.bottom - d.top),
		zoomy));
	dstLeft = d.left;			dstTop = d.top;

	if (_MGL_checkIdentityPal && bitmap->pal && bitmap->bitsPerPixel == 8
			&& dc->mi.bitsPerPixel == 8
			&& !ID_PAL(256,bitmap->pal,256,dc->colorTab)) {
		idPal = false;
		}

	/* Fill in temporary DC so we can simply call stretchBlt */
	tempDC.surface = bitmap->surface;
	tempDC.mi.bytesPerLine = bitmap->bytesPerLine;
	tempDC.mi.bitsPerPixel = bitmap->bitsPerPixel;
	tempDC.colorTab = (color_t*)bitmap->pal;
	if (zoomy == MGL_FIX_2) {
		if (zoomx == MGL_FIX_1) {
			dc->r.stretchBlt1x2(dc,(MGLDC*)&tempDC,left,top,right,bottom,
				dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
				bitmap->pal,idPal);
			return;
			}
		else if (zoomx == MGL_FIX_2) {
			dc->r.stretchBlt2x2(dc,(MGLDC*)&tempDC,left,top,right,bottom,
				dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
				bitmap->pal,idPal);
			return;
			}
		}

	dc->r.stretchBlt(dc,(MGLDC*)&tempDC,left,top,right,bottom,
		dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
		zoomx,zoomy,bitmap->pal,idPal);
}
/****************************************************************************
DESCRIPTION:
Stretches a section of a lightweight bitmap to the specified device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to display bitmap on
left	- Left coordinate of section to stretch
top	- Top coordinate of section to stretch
right	- Right coordinate of section to stretch
bottom	- Bottom coordinate of section to stretch
dstLeft	- Left coordinate to stretch bitmap to
dstTop	- Top coordinate to stretch bitmap to
dstRight	- Right coordinate to stretch bitmap to
dstBottom	- Bottom coordinate to stretch bitmap to
bitmap	- Bitmap to display

REMARKS:
Stretches a section of a lightweight bitmap to the specified destination rectangle on the 
device context. The bitmap can be in any color format, and will be translated as 
necessary to the color format required by the current device context.
Note:For maximum performance when displaying 8 bit bitmaps, you should ensure 
that the color palette for the bitmap is identical to the device context, otherwise the 
pixels in the bitmap will be translated during the draw operation.

SEE ALSO:
MGL_loadBitmap
****************************************************************************/
void MGLAPI MGL_stretchBitmapSection(
	MGLDC *dc,
	int dstLeft,
	int dstTop,
	int dstRight,
	int dstBottom,
	const bitmap_t *bitmap)
{
	rect_t	d;
	int		left = 0,top = 0,right = bitmap->width,bottom = bitmap->height;
	int		deltaSrc,deltaDst;
	fix32_t	zoomx,zoomy;
	ibool	idPal = true;

	if (dc == _MGL_dcPtr)
		dc = &DC;
	if (bitmap->bitsPerPixel < 8) {
		_MGL_result = grInvalidDevice;
		return;
		}

	/* Calculate the x zoom factor */
	deltaSrc = right - left;
	deltaDst = dstRight - dstLeft;
	if (deltaDst == deltaSrc)
		zoomx = MGL_FIX_1;
	else if (deltaDst == (deltaSrc * 2))
		zoomx = MGL_FIX_2;
	else
		zoomx = MGL_FixDiv(MGL_TOFIX(deltaDst),MGL_TOFIX(deltaSrc));

	/* Calculate the y zoom factor */
	deltaSrc = bottom - top;
	deltaDst = dstBottom - dstTop;
	if (deltaDst == deltaSrc)
		zoomy = MGL_FIX_1;
	else if (deltaDst == (deltaSrc * 2))
		zoomy = MGL_FIX_2;
	else
		zoomy = MGL_FixDiv(MGL_TOFIX(deltaDst),MGL_TOFIX(deltaSrc));

	/* Handle special case of 1:1 stretch */
	if (zoomx == MGL_FIX_1 && zoomy == MGL_FIX_1) {
		MGL_putBitmap(dc,dstLeft,dstTop,bitmap,MGL_REPLACE_MODE);
		return;
		}

	/* Clip the source rectangle to the bitmap dimensions */
	if (!MGL_sectRectCoord(0,0,bitmap->width,bitmap->height,left,top,right,bottom,&d))
		return;
	dstLeft += (d.left - left);	dstTop += (d.top - top);
	left = d.left;				right = d.right;
	top = d.top;				bottom = d.bottom;

	/* Clip to destination device context */
	if (dc->a.clip) {
		d.left = dstLeft;					d.top = dstTop;
		d.right = dstLeft + (right-left);	d.bottom = dstTop + (bottom-top);
		if (!MGL_sectRect(dc->a.clipRect,d,&d))
			return;
		left += (d.left - dstLeft);	right = left + (d.right - d.left);
		top += (d.top - dstTop);	bottom = top + (d.bottom - d.top);
		dstLeft = d.left;			dstTop = d.top;
		}


	if (_MGL_checkIdentityPal && bitmap->pal && bitmap->bitsPerPixel == 8
			&& dc->mi.bitsPerPixel == 8
			&& !ID_PAL(256,bitmap->pal,256,dc->colorTab)) {
		idPal = false;
		}

	/* Fill in temporary DC so we can simply call stretchBlt */
	tempDC.surface = bitmap->surface;
	tempDC.mi.bytesPerLine = bitmap->bytesPerLine;
	tempDC.mi.bitsPerPixel = bitmap->bitsPerPixel;
	tempDC.colorTab = (color_t*)bitmap->pal;
	if (zoomy == MGL_FIX_2) {
		if (zoomx == MGL_FIX_1) {
			dc->r.stretchBlt1x2(dc,(MGLDC*)&tempDC,left,top,right,bottom,
				dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
				bitmap->pal,idPal);
			return;
			}
		else if (zoomx == MGL_FIX_2) {
			dc->r.stretchBlt2x2(dc,(MGLDC*)&tempDC,left,top,right,bottom,
				dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
				bitmap->pal,idPal);
			return;
			}
		}

	dc->r.stretchBlt(dc,(MGLDC*)&tempDC,left,top,right,bottom,
		dstLeft + dc->a.viewPort.left,dstTop + dc->a.viewPort.top,
		zoomx,zoomy,bitmap->pal,idPal);
}

/****************************************************************************
DESCRIPTION:
Draw an icon at the specified location.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to draw icon on
x	- x coordinate to draw icon at
y	- y coordinate to draw icon at
icon	- Icon to display

REMARKS:
Draws an icon at the specified location on the current device context. The icon may
be in any color format, and will be translated as necessary to the color format of the
display device context. The icon is drawn by punching a black hole in the
background with the icon mask, and then OR'ing in the image bitmap for the icon.

SEE ALSO:
MGL_loadIcon
****************************************************************************/
void MGLAPI MGL_putIcon(
	MGLDC *dc,
	int x,
	int y,
	const icon_t *icon)
{
	int		oldop = dc->a.writeMode;
	color_t	oldcolor = dc->intColor;

	if (dc == _MGL_dcPtr)
		dc = &DC;

	/* Punch a hole in the display with the icon's AND mask */
	dc->r.setWriteMode(MGL_REPLACE_MODE);
	dc->intColor = 0;
	dc->r.putMonoImage(dc,x + dc->a.viewPort.left,y + dc->a.viewPort.top,
		icon->byteWidth,icon->xorMask.height,icon->andMask);
	dc->r.setWriteMode(oldop);
	dc->intColor = oldcolor;

	/* Now XOR in the bitmap for the ICON */
	MGL_putBitmap(dc,x,y,&icon->xorMask,MGL_XOR_MODE);
}

/****************************************************************************
DESCRIPTION:
Saves a divot of video memory into system RAM.

HEADER:
mgraph.h

PARAMETERS:
dc		- Device context to save divot from
left	- Left coordinate of area to save
top		- Top coordinate of area to save
right	- Right coordinate of area to save
bottom	- Bottom coordinate of area to save
divot	- Pointer to area to store the video memory in

REMARKS:
This function copies a block of video memory from the active page of the current
device context into a system RAM buffer. A divot is defined as being a rectangular
area of video memory that you wish to save, however the bounding rectangle for the
divot is expanded slightly to properly aligned boundaries for the absolute maximum
performance with the current device context. This function is generally used to store
the video memory behind pull down menus and pop up dialog boxes, and the
memory can only be restored to exactly the same position that it was saved from.

You must pre-allocate enough space to hold the entire divot in system RAM. Use
the MGL_divotSize routine to determine the size of the memory block required to
store the divot. Note also that the memory block for the divot may be larger than
64Kb in size, so you should use the MGL_malloc family of functions to allocate the
memory block.

SEE ALSO:
MGL_getDivot, MGL_putDivot, MGL_divotSize, MGL_malloc
****************************************************************************/
void MGLAPI MGL_getDivotCoord(
	MGLDC *dc,
	int left,
	int top,
	int right,
	int bottom,
	void *divot)
{
	rect_t	d;

	if (dc == _MGL_dcPtr)
		dc = &DC;

	if (dc->a.clip) {
		d.left = left;				d.top = top;
		d.right = right;			d.bottom = bottom;
		if (!MGL_sectRect(dc->a.clipRect,d,&d))
			return;
		}
	dc->r.getDivot(dc,d.left + dc->a.viewPort.left,d.top + dc->a.viewPort.top,
		d.right + dc->a.viewPort.left,d.bottom + dc->a.viewPort.top,divot);
}

/****************************************************************************
DESCRIPTION:
Saves a divot of video memory into system RAM.

HEADER:
mgraph.h

PARAMETERS:
dc		- Device context to save divot from
r		- Rectangle containing coordinates of divot to save
divot	- Pointer to area to store the video memory in

REMARKS:
This function is the same as MGL_getDivotCoord however it takes entire
rectangles as arguments instead of coordinates.

SEE ALSO:
MGL_putDivot, MGL_divotSize, MGL_malloc
****************************************************************************/
void MGL_getDivot(
	MGLDC dc,
	rect_t,
	void *divot);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Replaces a divot of video memory to the location from which it was copied.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to restore the divot to
divot	- Pointer to the divot to replace

REMARKS:
This function replaces a rectangle of video memory that was saved previously with
the MGL_getDivot function. The divot is replaced at the same location that is
was taken from on the current device context.

A divot is defined as being a rectangular area of video memory that you wish to
save, however the bounding rectangle for the divot is expanded slightly to properly
aligned boundaries for the absolute maximum performance with the current device
context. This function is generally used to store the video memory behind pull down
menus and pop up dialog boxes, and the memory can only be restored to exactly the
same position that it was saved from.

SEE ALSO:
MGL_divotSize
****************************************************************************/
void MGLAPI MGL_putDivot(
	MGLDC *dc,
	void *divot)
{
	if (dc == _MGL_dcPtr)
		dc = &DC;
	dc->r.putDivot(dc,divot);
}


/****************************************************************************
DESCRIPTION:
Number of bytes required to store a divot of specified size.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to measure divot size from
left	- Left coordinate of divot area 
top	- Top coordinate of divot area 
right	- Right coordinate of divot area 
bottom	- Bottom coordinate of divot area 

RETURNS:
Size of the specified divot in bytes.

REMARKS:
Determines the number of bytes required to store a divot of the specified size taken
from the current device context. A divot is a portion of video memory that needs to
be temporarily saved and restored, such as implementing pull down menus and pop
up dialog boxes. A divot must always be saved and restored to the same area, and
will extend the dimensions of the area covered to obtain the maximum possible
performance for saving and restoring the memory.

SEE ALSO:
MGL_divotSize, MGL_getDivot, MGL_putDivot
****************************************************************************/
long MGLAPI MGL_divotSizeCoord(
	MGLDC *dc,
	int left,
	int top,
	int right,
	int bottom)
{
	if (dc == _MGL_dcPtr)
		dc = &DC;
	return dc->r.divotSize(dc,left,top,right,bottom);
}

/****************************************************************************
DESCRIPTION:
Number of bytes required to store a divot of specified size.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to measure divot size from
r	- Bounding rectangle of the divot area

RETURNS:
Size of the specified divot in bytes.

REMARKS:
This function is the same as MGL_divotSizeCoord however it takes entire
rectangles as arguments instead of coordinates.

SEE ALSO:
MGL_divotSizeCoord, MGL_getDivot, MGL_putDivot
****************************************************************************/
long MGL_divotSize(
	MGLDC *dc,
	rect_t r);
/* Implemented as a macro */

#endif	/* !MGL_LITE */

/* {secret} */
int _MGL_bytesPerLine(MGLDC *dc,int left,int right)
{
	switch (dc->mi.bitsPerPixel) {
		case 1:
			return ((((right-1) / 8) + 1) - (left/8));
		case 4:
			if (dc->mi.numberOfPlanes == 1)
				return ((((right-1) / 2) + 1) - (left / 2));
			else
				return ((((right-1) / 8) + 1) - (left/8)) * 4;
		default:
			return (right - left) * ((dc->mi.bitsPerPixel+7) / 8);
		}
}

/****************************************************************************
DESCRIPTION:
Blts a block of image data from one device context into another.

HEADER:
mgraph.h

PARAMETERS:
dst		- Destination device context
src		- Source device context
left	- Left coordinate of image to Blt from
top		- Top coordinate of image to Blt from
right	- Right coordinate of image to Blt from
bottom	- Bottom coordinate of image to Blt from
dstLeft	- Left coordinate to Blt to
dstTop	- Right coordinate to Blt to
op		- Write mode to use during Blt

REMARKS:
Copies a block of bitmap data from one device context to another. The source
and destination rectangles may overlap even if the source and destination
device contexts are the same, and MGL will correctly handle the overlapping
regions. This routine has been highly optimized for absolute maximum performance,
so it will provide the fastest method of copying bitmap data between device
contexts. To obtain absolute maximum performance, you should align the source
and destination bitmaps on DWORD boundaries (4 pixels for 8 bit, 2 pixels for
15/16 bit) and the low level device driver code will special case this for
maximum performance.

This function will correctly handle Blt’s across device contexts with
differing pixel depths, and will perform the necessary pixel format translation
to convert from the source device to the destination device. Note that although
the code to implement this is highly optimized, this can be a time consuming
operation so you should attempt to pre-convert all bitmaps to the current
display device pixel format for maximum performance if using this routine for
sprite animation.

MGL does however have special case code to specifically handle translation of
24 bit RGB format bitmaps (the standard RGB DIB format used by Video for
Windows) to all 8 bit and above pixel formats. When converting from 24 bit to
8 bit, MGL will dither bitmaps in real time from 24 bit to the 8 bit halftone
palette. This provides a solid foundation to build real time 24 bit motion video
playback in all supported video modes in MGL.

Note that when MGL_bitBlt is called for 4 and 8 bit source bitmaps MGL first
checks if the color palettes for the source and destination bitmaps are the
same. If they are not, MGL translates the pixel values from the source bitmap
to the destination color palette, looking for the closest match color if an
exact match is not found. In order to obtain maximum performance for blt’ing
bitmaps in color index modes, you should ensure that the color palette in the
source device matches the color palette in the destination device, or you can
turn off all identity palette checking in MGL with the MGL_checkIdentityPalette
function.

This routine can also be used to perform hardware accelerated Blt’s between
offscreen memory devices and the display device when running in fullscreen
modes, providing the hardware accelerator (if present) can support this
operation.

The write mode operation specifies how the source image data should be combined
with the destination image data.  Write modes supported by the SciTech MGL are
enumerated in MGL_writeModeType.

The source and destination rectangles are clipped according to the current
clipping rectangles for the source and destination device contexts respectively.

Note that the MGL/Lite libraries support the MGL_bitBlt functions, but only
from system memory device contexts to display memory device contexts, the write
mode operation is ignored for all calls and no palette translation or color
conversion is supported.

The destination rectangle is clipped according to the current clipping
rectangles for the destination device context.

SEE ALSO:
MGL_bitBlt, MGL_stretchBlt, MGL_transBlt
****************************************************************************/
void MGLAPI MGL_bitBltCoord(
	MGLDC *dst,
	MGLDC *src,
	int left,
	int top,
	int right,
	int bottom,
	int dstLeft,
	int dstTop,
	int op)
{
	rect_t	d;

	/* In order to clip the results, we first clip the source rectangle to
	 * the source device context, and then clip the destination rectangle to
	 * the destination device context.
	 */
	if (src == _MGL_dcPtr)
		src = &DC;
	if (src->mi.bitsPerPixel == 1) {
		_MGL_result = grInvalidDevice;
		return;
		}
	if (dst == _MGL_dcPtr)
		dst = &DC;
	if (src->a.clip) {
		d.left = left;				d.top = top;
		d.right = right;			d.bottom = bottom;
		if (!MGL_sectRect(src->a.clipRect,d,&d))
			return;
		dstLeft += (d.left - left);	dstTop += (d.top - top);
		left = d.left;				right = d.right;
		top = d.top;				bottom = d.bottom;
		}
	if (dst->a.clip) {
		d.left = dstLeft;
		d.top = dstTop;
		d.right = dstLeft + (right-left);
		d.bottom = dstTop + (bottom-top);

		if (!MGL_sectRect(dst->a.clipRect,d,&d))
			return;
		left += (d.left - dstLeft);	right = left + (d.right - d.left);
		top += (d.top - dstTop);	bottom = top + (d.bottom - d.top);
		dstLeft = d.left;			dstTop = d.top;
		}

	if (src == dst) {
#ifndef	MGL_LITE
		/* Blt between areas of the same device context */
		if (src != &DC) {
			/* Current DC is not the same as source, then update the
			 * hardware clip rectangles.
			 */
			src->r.setClipRect(src,
				src->a.clipRect.left,src->a.clipRect.top,
				src->a.clipRect.right,src->a.clipRect.bottom);
			}
		src->r.bitBlt(src,
			left + src->a.viewPort.left,top + src->a.viewPort.top,
			right + src->a.viewPort.left,bottom + src->a.viewPort.top,
			dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
			op,(dstLeft <= left),(dstTop <= top));
		if (src != &DC) {
			/* Reset the clip rectangle to the original value */
			DC.r.setClipRect(&DC,
				DC.a.clipRect.left,DC.a.clipRect.top,
				DC.a.clipRect.right,DC.a.clipRect.bottom);
			}
#endif
		return;
		}
#ifndef	MGL_LITE
	if (src->deviceType == MGL_OFFSCREEN_DEVICE
   			&& _MGL_isFullscreenDevice(dst)) {
		/* Blt between an offscreen DC and a display DC */
		if (dst != &DC) {
			/* Current DC is not the same as destination, so update
			 * hardware clip rectangles.
			 */
			dst->r.setClipRect(dst,
				dst->a.clipRect.left,dst->a.clipRect.top,
				dst->a.clipRect.right,dst->a.clipRect.bottom);
			}
		src->r.bitBltOff(dst,src,
			left + src->a.viewPort.left,top + src->a.viewPort.top,
			right + src->a.viewPort.left,bottom + src->a.viewPort.top,
			dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
			op);
		if (dst != &DC) {
			/* Reset the clip rectangle to the original value */
			DC.r.setClipRect(&DC,
				DC.a.clipRect.left,DC.a.clipRect.top,
				DC.a.clipRect.right,DC.a.clipRect.bottom);
			}
		return;
		}
#endif
	if (src->deviceType == MGL_MEMORY_DEVICE) {
		/* Copy an image from a memory DC to any other DC */
#ifndef	MGL_LITE
		if (NEED_TRANSLATE_DC(src,dst)) {
			dst->r.translateImage(dst,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,op,
				src->surface,src->mi.bytesPerLine,src->mi.bitsPerPixel,
				(palette_t*)src->colorTab,&src->pf,src);
			}
		else
#endif
			{
			dst->r.putImage(dst,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				op,src->surface,src->mi.bytesPerLine, src);
			}
		return;
		}
	if (dst->deviceType == MGL_MEMORY_DEVICE) {
#ifndef	MGL_LITE
		if (NEED_TRANSLATE_DC(src,dst)) {
			dst->r.translateImage(dst,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,op,
				src->surface,src->mi.bytesPerLine,src->mi.bitsPerPixel,
				(palette_t*)src->colorTab,&src->pf,src);
			}
		else
#endif
			{
			src->r.getImage(src,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,op,
				dst->surface,dst->mi.bytesPerLine);
			}
		return;
		}
	_MGL_result = grInvalidDevice;
}

/****************************************************************************
DESCRIPTION:
Blts a block of image data from one device context into another.

HEADER:
mgraph.h

PARAMETERS:
dst		- Destination device context
src		- Source device context
r		- Rectangle defining are to Blt from
dstLeft	- Left coordinate to Blt to
dstTop	- Right coordinate to Blt to
op		- Write mode to use during Blt

REMARKS:
This function is the same as MGL_bitBltCoord, however it takes entire
rectangles as parameters instead of coordinates.

SEE ALSO:
MGL_bitBltCoord, MGL_stretchBlt, MGL_transBlt
****************************************************************************/
void MGL_bitBlt(
	MGLDC *dst,
	MGLDC *src,
	rect_t r,
	int dstLeft,
	int dstTop,
	int op);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:

Stretches a block of image data from one device context to another.

HEADER:
mgraph.h

PARAMETERS:
dst			- Destination device context
src			- Source device context
left		- Left coordinate of source image
top			- Top coordinate of source image
right		- Right coordinate of source image
bottom	   	- Bottom coordinate of source image
dstLeft		- Left coordinate of destination image
dstTop		- Top coordinate of destination image
dstRight	- Right coordinate of destination image 
dstBottom	- Bottom coordinate of destination image 

REMARKS:
Copies a block of bitmap data form one device context to another, stretching or
shrinking the image as necessary to fit the destination rectangle for the destination
device context.

The source and destination device context may not be the same. This routine has
been highly optimized for absolute maximum performance, so it will provide the
fastest method of stretching bitmap data between device contexts, and can also be
used to stretch bitmap data from a memory device context to a windowed device
context.

This function will correctly handle StretchBlt's across device contexts with
differing pixel depths, and will perform the necessary pixel format translation to
convert from the source device to the destination device. Note that although the
code to implement this is highly optimized, this can be a time consuming operation
so you should attempt to pre-convert all bitmaps to the current display device pixel
format for maximum performance if possible.

MGL does however have special case code to specifically handle translation of 24
bit RGB format bitmaps (the standard RGB DIB format used by Video for
Windows) to all 8 bit and above pixel formats. When converting from 24 bit to 8
bit, MGL will dither bitmaps in real time from 24 bit to the 8 bit halftone palette.
This provides a solid foundation to build real time 24 bit motion video playback in
all supported video modes in MGL.

Note that when MGL_bitBlt is called for 4 and 8 bit source
bitmaps MGL first checks if the color palettes for the source and destination
bitmaps are the same. If they are not, MGL translates the pixel values from the
source bitmap to the destination color palette, looking for the closest match color if
an exact match is not found. In order to obtain maximum performance for blt'ing
bitmaps in color index modes, you should ensure that the color palette in the source
device matches the color palette in the destination device, or you can turn off all
identity palette checking in MGL with the MGL_checkIdentityPalette function.

The source and destination rectangles are clipped according to the current clipping
rectangles for the source and destination device contexts respectively, however the
zoom factor is determined using the unclipped source and destination rectangles.

SEE ALSO:
MGL_stretchBlt, MGL_bitBlt, MGL_bitBltCoord
****************************************************************************/
void MGLAPI MGL_stretchBltCoord(
	MGLDC *dst,
	MGLDC *src,
	int left,
	int top,
	int right,
	int bottom,
	int dstLeft,
	int dstTop,
	int dstRight,
	int dstBottom)
{
	rect_t	d;
	int		deltaSrc,deltaDst;
	fix32_t	zoomx,zoomy;
	ibool	idPal = true;

	if ((src == dst) || src->mi.bitsPerPixel < 8) {
		_MGL_result = grInvalidDevice;
		return;
		}

	if (src == _MGL_dcPtr)
		src = &DC;
	if (dst == _MGL_dcPtr)
		dst = &DC;

	/* Calculate the x zoom factor */
	deltaSrc = right - left;
	deltaDst = dstRight - dstLeft;
	if (deltaDst == deltaSrc)
		zoomx = MGL_FIX_1;
	else if (deltaDst == (deltaSrc * 2))
		zoomx = MGL_FIX_2;
	else zoomx = MGL_FixDiv(MGL_TOFIX(deltaDst),MGL_TOFIX(deltaSrc));

	/* Calculate the y zoom factor */
	deltaSrc = bottom - top;
	deltaDst = dstBottom - dstTop;
	if (deltaDst == deltaSrc)
		zoomy = MGL_FIX_1;
	else if (deltaDst == (deltaSrc * 2))
		zoomy = MGL_FIX_2;
	else zoomy = MGL_FixDiv(MGL_TOFIX(deltaDst),MGL_TOFIX(deltaSrc));

	/* Handle special case of 1:1 stretch */
	if (zoomx == MGL_FIX_1 && zoomy == MGL_FIX_1) {
		MGL_bitBltCoord(dst,src,left,top,right,bottom,dstLeft,dstTop,
			MGL_REPLACE_MODE);
		return;
		}

	/* In order to clip the results, we first clip the source rectangle to
	 * the source device context, and then clip the destination rectangle to
	 * the destination device context.
	 */
	if (src->a.clip) {
		d.left = left;				d.top = top;
		d.right = right;			d.bottom = bottom;
		if (!MGL_sectRect(src->a.clipRect,d,&d))
			return;
		dstLeft += (d.left - left);	dstTop += (d.top - top);
		left = d.left;				right = d.right;
		top = d.top;				bottom = d.bottom;
		}
	if (dst->a.clip) {
		d.left = dstLeft;
		d.top = dstTop;
		d.right = dstRight;
		d.bottom = dstBottom;

		if (!MGL_sectRect(dst->a.clipRect,d,&d))
			return;
		left += (d.left - dstLeft);
		top += (d.top - dstTop);
		right = left + MGL_FIXTOINT(MGL_FixDiv(MGL_TOFIX(d.right - d.left),
			zoomx));
		bottom = top + MGL_FIXTOINT(MGL_FixDiv(MGL_TOFIX(d.bottom - d.top),
			zoomy));
		dstLeft = d.left;			dstTop = d.top;
		}

#ifndef	MGL_LITE
	if (_MGL_checkIdentityPal && src->mi.bitsPerPixel == 8
			&& dst->mi.bitsPerPixel == 8 && !ID_PAL_DC(src,dst))
		idPal = false;
#endif

	if (zoomy == MGL_FIX_2) {
		if (zoomx == MGL_FIX_1) {
			dst->r.stretchBlt1x2(dst,src,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				(palette_t*)src->colorTab,idPal);
			return;
			}
		else if (zoomx == MGL_FIX_2) {
			dst->r.stretchBlt2x2(dst,src,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				(palette_t*)src->colorTab,idPal);
			return;
			}
		}

	dst->r.stretchBlt(dst,src,
		left + src->a.viewPort.left,top + src->a.viewPort.top,
		right + src->a.viewPort.left,bottom + src->a.viewPort.top,
		dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
		zoomx,zoomy,(palette_t*)src->colorTab,idPal);
}

/****************************************************************************
DESCRIPTION:
Stretches a block of image data from one device context to another.

HEADER:
mgraph.h

PARAMETERS:
dst	- Destination device context
src	- Source device context
srcRect	- Rectangle defining source image
dstRect	- Rectangle defining destination image 

REMARKS:
This function is the same as MGL_stretchBltCoord, however it takes entire
rectangles as arguments instead of coordinates.

SEE ALSO:
MGL_stretchBltCoord, MGL_bitBlt, MGL_bitBltCoord
****************************************************************************/
void MGL_stretchBLT(
	MGLDC dst,
	MGLDC src,
	rect_t srcRect,
	rect_t destRect);
/* Implemented as a macro */

#ifndef	MGL_LITE

/****************************************************************************
DESCRIPTION:
Copies a block of image data from a linear offscreen device context to a display
device.

HEADER:
mgraph.h

PARAMETERS:
dst	- destination device context (must be a display device)
src	- source device context (must be a linear offscreen device)
srcOfs	- Starting offset of bitmap in the source device surface
dstLeft	- left coordinate of destination image
dstTop	- top coordinate of destination image
dstRight	- right coordinate of destination image
dstBottom	- bottom coordinate of destination image
op	- write mode to use during Blt

REMARKS:
This function is similar to MGL_bitBlt except that the
source device context must be for a linear offscreen device and the destination
device context must be for a display device. The difference is that this function
copies a bitmap as a linear chunk of memory from the offscreen memory to the
display memory using the hardware accelerator. Hence you can store bitmaps in the
offscreen memory device contiguously, which provides for much more efficient
utilization of the hardware video memory for storing sprites and bitmaps.

Note however that not all hardware accelerators can support linear offscreen
memory, in which case you would not be able to create a linear offscreen device
context.

The write mode operation specifies how the source image data should be combined
with the destination image data.  Write modes supported by the SciTech MGL are
enumerated in MGL_writeModeType.

The destination rectangle is clipped according to the current clipping rectangles for
the destination device context.

SEE ALSO:
MGL_bitBltLin, MGL_transBltLin
****************************************************************************/
void MGLAPI MGL_bitBltLinCoord(
	MGLDC *dst,
	MGLDC *src,
	ulong srcOfs,
	int dstLeft,
	int dstTop,
	int dstRight,
	int dstBottom,
	int op)
{
	if (src->deviceType == MGL_LINOFFSCREEN_DEVICE
			&& _MGL_isFullscreenDevice(dst)) {
		/* Blt between an linear offscreen DC and a display DC */
		if (dst != &DC) {
			/* Current DC is not the same as destination, so update
			 * hardware clip rectangles.
			 */
			dst->r.setClipRect(dst,
				dst->a.clipRect.left,dst->a.clipRect.top,
				dst->a.clipRect.right,dst->a.clipRect.bottom);
			}
		dst->r.bitBltLin(dst,src,srcOfs,
			dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
			dstRight + dst->a.viewPort.left,dstBottom + dst->a.viewPort.top,
			op);
		if (dst != &DC) {
			/* Reset the clip rectangle to the original value */
			DC.r.setClipRect(&DC,
				DC.a.clipRect.left,DC.a.clipRect.top,
				DC.a.clipRect.right,DC.a.clipRect.bottom);
			}
		return;
		}
	_MGL_result = grInvalidDevice;
}

/****************************************************************************
DESCRIPTION:
Copies a block of image data from a linear offscreen device context to a display
device.

HEADER:
mgraph.h

PARAMETERS:
dst		- destination device context (must be a display device)
src		- source device context (must be a linear offscreen device)
dstRect	- rectangle defining source image
op		- write mode to use during Blt

REMARKS:
This function is the same as MGL_bitBltLin, however it takes entire
rectangles as arguments instead of coordinates.

SEE ALSO:
MGL_bitBltLinCoord, MGL_transBltLin
****************************************************************************/
void MGL_bitBltLin(
	MGLDC *dst,
	MGLDC *src,
	ulong srcOfs,
	rect_t dstRect,
	int op);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Copies a block of image data with source transparency.

HEADER:
mgraph.h

PARAMETERS:
dst	- Destination device context
src	- Source device context
left	- Left coordinate of source image 
top	- Top coordinate of source image 
right	- Right coordinate of source image 
bottom	- Bottom coordinate of source image 
dstLeft	- Left coordinate of destination
dstTop	- Top coordinate of destination
transparent	- Transparent color to skip in source image
sourceTrans	- True for source transparency, false for destination transparency

REMARKS:
Copies a block of bitmap data form one device context to another with either source
or destination transparency. When transferring the data with source transparency,
pixels in the destination image that are equal to the specified transparent color will
not be updated, and those pixels that are not the same will be updated. This allows
you to quickly transfer sprites between device contexts with a single color being
allocated as a transparent color. When transferring the data with destination
transparency, pixels in the destination image that are equal to the specified
transparent color will be updated, and those pixels that are not the same will be
skipped. This is effectively the operation performed for 'blueScreen'ing or color
keying and can also be used for drawing transparent sprites. Note however that
destination transparency is very slow in software compared to source transparency!

The device contexts must not be the same and must be in the same pixel depth and
the same pixel format, or this routine will simply do nothing. This routine also only
works for device contexts with pixel depths that are at least 8 bits deep.

This routine has been highly optimized for maximum performance in all pixel
depths, so will provide a very fast method for performing transparent sprite
animation. However you may find that if you can use alternative techniques to pre-
compile the sprites (like using run length encoding etc.) you will be able to build
faster software based sprite animation code that can directly access the device
context surface. However this routine can also be used to perform hardware
accelerated Blt's between offscreen memory device's and the display device when
running in fullscreen modes, providing the hardware accelerator (if present) can
support this operation. If you have a hardware accelerator capable of this, this will
provide the ultimate performance for transparent sprite animation.

The source and destination rectangles are clipped according to the current clipping
rectangles for the source and destination device contexts respectively.

SEE ALSO:
MGL_transBlt, MGL_bitBlt
****************************************************************************/
void MGLAPI MGL_transBltCoord(
	MGLDC *dst,
	MGLDC *src,
	int left,
	int top,
	int right,
	int bottom,
	int dstLeft,
	int dstTop,
	color_t transparent,
	ibool sourceTrans)
{
	rect_t	d;

	if ((src == dst) || src->mi.bitsPerPixel < 8 || (src->mi.bitsPerPixel
			!= dst->mi.bitsPerPixel)) {
		_MGL_result = grInvalidDevice;
		return;
		}

	/* In order to clip the results, we first clip the source rectangle to
	 * the source device context, and then clip the destination rectangle to
	 * the destination device context.
	 */
	if (src == _MGL_dcPtr)
		src = &DC;
	if (dst == _MGL_dcPtr)
		dst = &DC;
	if (src->a.clip) {
		d.left = left;				d.top = top;
		d.right = right;			d.bottom = bottom;
		if (!MGL_sectRect(src->a.clipRect,d,&d))
			return;
		dstLeft += (d.left - left);	dstTop += (d.top - top);
		left = d.left;				right = d.right;
		top = d.top;				bottom = d.bottom;
		}
	if (dst->a.clip) {
		d.left = dstLeft;
		d.top = dstTop;
		d.right = dstLeft + (right-left);
		d.bottom = dstTop + (bottom-top);

		if (!MGL_sectRect(dst->a.clipRect,d,&d))
			return;
		left += (d.left - dstLeft);	right = left + (d.right - d.left);
		top += (d.top - dstTop);	bottom = top + (d.bottom - d.top);
		dstLeft = d.left;			dstTop = d.top;
		}

	if (src->deviceType == MGL_OFFSCREEN_DEVICE
			&& _MGL_isFullscreenDevice(dst)) {
		/* Blt between an offscreen DC and a display DC */
		if (dst != &DC) {
			/* Current DC is not the same as destination, so update
			 * hardware clip rectangles.
			 */
			dst->r.setClipRect(dst,
				dst->a.clipRect.left,dst->a.clipRect.top,
				dst->a.clipRect.right,dst->a.clipRect.bottom);
			}
		if (sourceTrans)
			dst->r.srcTransBltOff(dst,src,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				MGL_REPLACE_MODE,transparent);
		else
			dst->r.dstTransBltOff(dst,src,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				MGL_REPLACE_MODE,transparent);
		if (dst != &DC) {
			/* Reset the clip rectangle to the original value */
			DC.r.setClipRect(&DC,
				DC.a.clipRect.left,DC.a.clipRect.top,
				DC.a.clipRect.right,DC.a.clipRect.bottom);
			}
		}
	else {
		if (sourceTrans)
			dst->r.srcTransBlt(dst,src,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				MGL_REPLACE_MODE,transparent);
		else
			dst->r.dstTransBlt(dst,src,
				left + src->a.viewPort.left,top + src->a.viewPort.top,
				right + src->a.viewPort.left,bottom + src->a.viewPort.top,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				MGL_REPLACE_MODE,transparent);
        }
}

/****************************************************************************
DESCRIPTION:
Copies a block of image data with source transparency.

HEADER:
mgraph.h

PARAMETERS:
dst	- Destination device context
src	- Source device context
srcRect	- Rectangle defining source image 
dstLeft	- Left coordinate of destination
dstTop	- Top coordinate of destination
transparent	- Transparent color to skip in source image
sourceTrans	- True for source transparency, false for destination transparency

REMARKS:
This function is the same as MGL_transBltCoord, however it takes a rectangle
as a parameter instead of the four coordinates of a rectangle.

SEE ALSO:
MGL_transBltCoord, MGL_bitBlt
****************************************************************************/
void MGL_transBlt(
	MGLDC *dst,
	MGLDC *src,
	rect_t srcRect,
	int dstLeft,
	int dstTop,
	color_t transparent,
	ibool sourceTrans);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Copies a block of image data with source transparency.

HEADER:
mgraph.h

PARAMETERS:
dst	- Destination device context (must be a display device)
src	- Source device context (must be a linear offscreen device)
srcOfs	- Starting offset of bitmap in the source device surface
dstLeft	- Left coordinate of destination image
dstTop	- Top coordinate of destination image
dstRight	- Right coordinate of destination image
dstBottom	- Bottom coordinate of destination image
transparent	- Transparent color to skip in source image
sourceTrans	- True for source transparency, false for destination transparency

REMARKS:
This function is similar to MGL_transBlt except that the source
device context must be for a linear offscreen device and the destination device
context must be for a display device. The difference is that this function copies a
bitmap as a linear chunk of memory from the offscreen memory to the display
memory using the hardware accelerator. Hence you can store bitmaps in the
offscreen memory device contiguously, which provides for much more efficient
utilization of the hardware video memory for storing sprites and bitmaps.

Note however that not all hardware accelerators can support linear offscreen
memory, in which case you would not be able to create a linear offscreen device
context. Also some hardware accelerators cannot support transparent BitBlt
operations.

The destination rectangle is clipped according to the current clipping rectangles for
the destination device context.

SEE ALSO:
MGL_transBltLin, MGL_bitBltLin
****************************************************************************/
void MGLAPI MGL_transBltLinCoord(
	MGLDC *dst,
	MGLDC *src,
	ulong srcOfs,
	int dstLeft,
	int dstTop,
	int dstRight,
	int dstBottom,
	color_t transparent,
	ibool sourceTrans)
{
	if (src->deviceType == MGL_LINOFFSCREEN_DEVICE
			&& _MGL_isFullscreenDevice(dst)) {
		/* Blt between an linear offscreen DC and a display DC */
		if (dst != &DC) {
			/* Current DC is not the same as destination, so update
			 * hardware clip rectangles.
			 */
			dst->r.setClipRect(dst,
				dst->a.clipRect.left,dst->a.clipRect.top,
				dst->a.clipRect.right,dst->a.clipRect.bottom);
			}
		if (sourceTrans)
			dst->r.srcTransBltLin(dst,src,srcOfs,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				dstRight + dst->a.viewPort.left,dstBottom + dst->a.viewPort.top,
				MGL_REPLACE_MODE,transparent);
		else
			dst->r.dstTransBltLin(dst,src,srcOfs,
				dstLeft + dst->a.viewPort.left,dstTop + dst->a.viewPort.top,
				dstRight + dst->a.viewPort.left,dstBottom + dst->a.viewPort.top,
				MGL_REPLACE_MODE,transparent);
		if (dst != &DC) {
			/* Reset the clip rectangle to the original value */
			DC.r.setClipRect(&DC,
				DC.a.clipRect.left,DC.a.clipRect.top,
				DC.a.clipRect.right,DC.a.clipRect.bottom);
			}
		return;
		}
	_MGL_result = grInvalidDevice;
}

/****************************************************************************
DESCRIPTION:
Copies a block of image data with source transparency.

HEADER:
mgraph.h

PARAMETERS:
dst	- Destination device context (must be a display device)
src	- Source device context (must be a linear offscreen device)
srcOfs	- Starting offset of bitmap in the source device surface
dstRect	- Rectangle defining source image 
transparent	- Transparent color to skip in source image
sourceTrans	- True for source transparency, false for destination transparency

REMARKS:
This function is the same as MGL_transBltLin, however it takes the entire
rectangle as a parameter instead of the individual coordinates.

SEE ALSO:
MGL_transBltLinCoord, MGL_bitBltLin
****************************************************************************/
void MGL_transBltLin(
	MGLDC dst,
	MGLDC src,
	ulong srcOfs,
	rect_t dstRect,
	color_t transparent,
	ibool sourceTrans);
/* Implemented as a macro */

#endif	/* !MGL_LITE */

