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
* Description:	C based emulation routines for stretchBlt. Contains
*				code for generic stretching and for bitmap format
*				translation.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

#ifndef	MGL_LITE

PRIVATE void translateScanLine(MGLDC *dst,MGLDC *src,int y,int x1,int count,
	void *buffer,int *repCounts)
/****************************************************************************
*
* Function:		translateScanLine
* Parameters:	dst  		- Device context to dump scanline to
*				src			- Source DC that buffer was read from
*				y			- Y coordinate of scanline
*               x1			- Initial x coordinate
*				count		- Number of pixels in source
*               buffer		- Pointer to buffer to translate
*           	repCounts	- Buffer containing pixel count repetitions
*
* Description:	Translates a scanline of a different pixel depth or
*				pixel format with stretching. This is not particularly
*				fast, but it does work.
*
****************************************************************************/
{
	int		i,bytesPerPixel,right = x1;
	void	*tempBuf = (uchar*)_MGL_buf + count * sizeof(color_t);
	color_t	c,*colorTab = dst->colorTab;
	uchar	*s = buffer,R,G,B;

	if (src->mi.bitsPerPixel == 8) {
		/* Stretch 8 bit images to RGB formats */
		switch (dst->mi.bitsPerPixel) {
			case 8: {
				uchar *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					c = _MGL_translate[*s++];
					while (i--)
						*p++ = c;
					}
				} break;
			case 15:
			case 16: {
				ushort *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					c = colorTab[*s++];
					while (i--)
						*p++ = c;
					}
				} break;
			case 24: {
				uchar *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					c = colorTab[*s++];
					while (i--) {
						*p++ = (uchar)c;
						*p++ = (uchar)(c >> 8);
						*p++ = (uchar)(c >> 16);
						}
					}
				} break;
			case 32: {
				ulong *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					c = colorTab[*s++];
					while (i--)
						*p++ = c;
					}
				} break;
			}
		}
	else {
		/* Handle stretching of RGB images to the device context */
		switch (src->mi.bitsPerPixel) {
			case 15:
			case 16:	bytesPerPixel = 2; break;
			case 24: 	bytesPerPixel = 3; break;
			case 32: 	bytesPerPixel = 4; break;
			default:
				return;
			}

		switch (dst->mi.bitsPerPixel) {
			case 8: {
				uchar *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					MGL_unpackColorFast(&src->pf,(*(color_t*)s),R,G,B);
					while (i--)
						MGL_halfTonePixelFast(*p++,(int)p,y,R,G,B);
					s += bytesPerPixel;
					}
				} break;
			case 15:
			case 16: {
				ushort *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					MGL_unpackColorFast(&src->pf,(*(color_t*)s),R,G,B);
					c = MGL_packColorFast(&dst->pf,R,G,B);
					while (i--)
						*p++ = c;
					s += bytesPerPixel;
					}
				} break;
			case 24: {
				uchar *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					MGL_unpackColorFast(&src->pf,(*(color_t*)s),R,G,B);
					c = MGL_packColorFast(&dst->pf,R,G,B);
					while (i--) {
						*p++ = (uchar)c;
						*p++ = (uchar)(c >> 8);
						*p++ = (uchar)(c >> 16);
						}
					s += bytesPerPixel;
					}
				} break;
			case 32: {
				ulong *p = tempBuf;

				while (count--) {
					right += (i = *repCounts++);
					MGL_unpackColorFast(&src->pf,(*(color_t*)s),R,G,B);
					c = MGL_packColorFast(&dst->pf,R,G,B);
					while (i--)
						*p++ = c;
					s += bytesPerPixel;
					}
				} break;
			default:
				return;
			}
		}
	dst->r.putScanLine(dst,y,x1,right,tempBuf,0,MGL_REPLACE_MODE);
}

#endif

void _ASMAPI __EMU__stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		__EMU__stretchBlt1x2
* Parameters:	dst		- Destination Device context
*				src		- Source device context
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*
****************************************************************************/
{
	int		i,count,dstRight = dstLeft + (right - left);
	int		bytesPerLine = src->mi.bytesPerLine;
	uchar	_HUGE *p;

#ifndef	MGL_LITE
	if (!idPal || NEED_TRANSLATE_DC(src,dst)) {
		/* Handle translation using the generic stretchBlt code */
		__EMU__stretchBlt(dst,src,left,top,right,bottom,dstLeft,dstTop,
			MGL_FIX_1,MGL_FIX_2,pal,idPal);
		return;
		}
#endif

	p = _MGL_pixelAddr2(src->surface,left,top,bytesPerLine,
		src->mi.bitsPerPixel);

	/* Display data by calling device driver to dump scanlines	*/
	dst->r.beginPixel(dst);
	i = dstTop;
	count = bottom - top;
	while (count--) {
		dst->r.putScanLine(dst,i++,dstLeft,dstRight,p,0,MGL_REPLACE_MODE);
		dst->r.putScanLine(dst,i++,dstLeft,dstRight,p,0,MGL_REPLACE_MODE);
		p += bytesPerLine;
		NORMALISE_PTR(p);
		}
	dst->r.endPixel(dst);
}

void _ASMAPI __EMU__stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		__EMU__stretchBlt2x2
* Parameters:	dst		- Destination Device context
*				src		- Source device context
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*
****************************************************************************/
{
	int		i,count,xcount = right - left;
	int		bytesPerLine = src->mi.bytesPerLine;
	uchar	_HUGE *p;

#ifndef	MGL_LITE
	if (!idPal || NEED_TRANSLATE_DC(src,dst)) {
		/* Handle translation using the generic stretchBlt code */
		__EMU__stretchBlt(dst,src,left,top,right,bottom,dstLeft,dstTop,
			MGL_FIX_2,MGL_FIX_2,pal,idPal);
		return;
		}
#endif

	p = _MGL_pixelAddr2(src->surface,left,top,bytesPerLine,
		src->mi.bitsPerPixel);

	/* Display data by calling device driver to dump scanlines	*/
	dst->r.beginPixel(dst);
	i = dstTop;
	count = bottom - top;
	while (count--) {
		dst->r.stretchScanLine2x(dst,i++,dstLeft,xcount,p);
		dst->r.stretchScanLine2x(dst,i++,dstLeft,xcount,p);
		p += bytesPerLine;
		NORMALISE_PTR(p);
		}
	dst->r.endPixel(dst);
}

void _ASMAPI __EMU__stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,
	int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,
	palette_t *pal,ibool idPal)
/****************************************************************************
*
* Function:		__EMU__stretchBlt
* Parameters:	dst		- Destination Device context
*				src		- Source device context
* 				left	- Left coordinate of are to Blt from
*				top		- Top coordinate of image to Blt from
*				right	- Right coordinate of image to Blt from
*				bottom	- Bottom coordinate of image to Blt from
*				dstLeft	- Left coordinate to Blt to
*				dstTop	- Right coordinate to Blt to
*				zoomx	- X axis zoom factor
*				zoomy	- Y axis zoom factor
*
****************************************************************************/
{
	int		i,dy,next,prev,count,xcount = right - left;
	int		bytesPerLine = src->mi.bytesPerLine;
	fix32_t	fx;
	uchar	_HUGE *p;
	int		*rc,*repCounts = _MGL_buf;
#ifndef	MGL_LITE
	ibool	translate = !idPal || NEED_TRANSLATE_DC(src,dst);
#endif

	/* Get source address to stretch from */
	p = _MGL_pixelAddr2(src->surface,left,top,bytesPerLine,
		src->mi.bitsPerPixel);

	/* Pre-render the pixel replication counts for each scanline */
	fx = zoomx;
	prev = 0;
	count = xcount;
	rc = repCounts;
	while (count--) {
		*rc++ = (next = MGL_FIXROUND(fx)) - prev;
		prev = next;
		fx += zoomx;
		}

#ifndef MGL_LITE
	/* Build translation vector to map between the palettes. This will
	 * only ever happen for 8 bit surfaces.
	 */
	if (!idPal) {
		_MGL_buildTranslateVector(_MGL_translate,
			256,pal,256,(palette_t*)dst->colorTab);
		}
#endif

	/* Display data by calling device driver to dump scanlines */
	dst->r.beginPixel(dst);
	fx = zoomy;
	prev = 0;
	i = dstTop;
	count = bottom - top;
	while (count--) {
		dy = (next = MGL_FIXROUND(fx)) - prev;
		prev = next;
		fx += zoomy;
#ifndef	MGL_LITE
		if (translate) {
			while (dy--)
				translateScanLine(dst,src,i++,dstLeft,xcount,p,repCounts);
			}
		else
#endif
			{
			while (dy--)
				dst->r.stretchScanLine(dst,i++,dstLeft,xcount,p,repCounts);
			}
		p += bytesPerLine;
		NORMALISE_PTR(p);
		}
	dst->r.endPixel(dst);
}
