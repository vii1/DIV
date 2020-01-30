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
*				device context and another device context with a different
*				pixel format. This routine does on the fly translation to
*				the destination device context pixel format. We can handle
*				the following format translations (others will be flagged
*				with an error):
*
*                   4 bit  -> 4 bit, 8 bit, 15 bit, 16 bit, 24 bit, 32 bit
*                   8 bit  -> 4 bit, 8 bit, 15 bit, 16 bit, 24 bit, 32 bit
*                   15 bit -> 4 bit, 8 bit, 15 bit, 16 bit, 24 bit, 32 bit
*                   16 bit -> 4 bit, 8 bit, 15 bit, 16 bit, 24 bit, 32 bit
*					24 bit -> 4 bit, 8 bit, 15 bit, 16 bit, 24 bit, 32 bit
*					32 bit -> 4 bit, 8 bit, 15 bit, 16 bit, 24 bit, 32 bit
*
*				Note that when this routine gets called to translate
*				HiColor and TrueColor images of the same pixel depth, it
*				is assumed that the pixel formats of the two DC's differ
*				and need to be translated. Similarly when called for 4->4
*				or 8->8 translattions it is assumed that the palettes
*				different and the pixels need to be translated.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI _MGL_translate4to8(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1,*p,*buf = _MGL_buf,*endp = buf + count;

	/* Build translation vector to map between the palettes */
	_MGL_buildTranslateVector(_MGL_translate,16,pal,256,(palette_t*)dc->colorTab);

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left/2);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		if (left & 1)
			*p++ = _MGL_translate[*s1++ & 0xF];	// Handle first odd pixel
		while (p != endp) {
			*p++ = _MGL_translate[*s1 >> 4];
			if (p == endp) break;
			*p++ = _MGL_translate[*s1++ & 0xF];
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate4to16(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1;
	ushort	*p,*buf = _MGL_buf,*endp = buf + count;
	color_t	*cp,colorTab[16];

	/* Build a color translation table from the source palette */
	for (cp = colorTab,i = 0; i < 16; i++,pal++,cp++)
		*cp = MGL_packColorFast(&dc->pf,pal->red,pal->green,pal->blue);

	/* Translate the pixels in the bitmap */
	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left/2);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		if (left & 1)
			*p++ = colorTab[*s1++ & 0xF];
		while (p != endp) {
			*p++ = colorTab[*s1 >> 4];
			if (p == endp) break;
			*p++ = colorTab[*s1++ & 0xF];
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate4to24(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1,*p,*buf = _MGL_buf,*endp = buf + count*3;
	color_t	c,*cp,colorTab[16];

	/* Build a color translation table from the source palette */
	for (cp = colorTab,i = 0; i < 16; i++,pal++,cp++)
		*cp = MGL_packColorFast(&dc->pf,pal->red,pal->green,pal->blue);

	/* Translate the pixels in the bitmap */
	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left/2);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		if (left & 1) {
			c = colorTab[*s1++ & 0xF];		// Handle first odd pixel
			*p++ = (uchar)c;
			*p++ = (uchar)(c >> 8);
			*p++ = (uchar)(c >> 16);
			}
		while (p != endp) {
			c = colorTab[*s1 >> 4];
			*p++ = (uchar)c;
			*p++ = (uchar)(c >> 8);
			*p++ = (uchar)(c >> 16);
			if (p == endp) break;
			c = colorTab[*s1++ & 0xF];
			*p++ = (uchar)c;
			*p++ = (uchar)(c >> 8);
			*p++ = (uchar)(c >> 16);
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate4to32(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1;
	ulong	*p,*buf = _MGL_buf,*endp = buf + count;
	color_t	*cp,colorTab[16];

	/* Build a color translation table from the source palette */
	for (cp = colorTab,i = 0; i < 16; i++,pal++,cp++)
		*cp = MGL_packColorFast(&dc->pf,pal->red,pal->green,pal->blue);

	/* Translate the pixels in the bitmap */
	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left/2);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		if (left & 1)
			*p++ = colorTab[*s1++ & 0xF];
		while (p != endp) {
			*p++ = colorTab[*s1 >> 4];
			if (p == endp) break;
			*p++ = colorTab[*s1++ & 0xF];
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate8to4(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int			i,count = (right-left);
	int			dstRight = (dstLeft + count);
	int			dstBottom = (dstTop + (bottom - top));
	uchar		_HUGE *s,*s1;
	uchar		*p,*buf = _MGL_buf,*endp = buf + count;

	/* Build translation vector to map between the palettes */
	_MGL_buildTranslateVector(_MGL_translate,256,pal,16,(palette_t*)dc->colorTab);

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		while (p != endp) {
			*p++ = (_MGL_translate[*s1] << 4) | _MGL_translate[*(s1+1)];
			s1 += 2;
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate8to8(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int			i,count = (right-left);
	int			dstRight = (dstLeft + count);
	int			dstBottom = (dstTop + (bottom - top));
	uchar		_HUGE *s,*s1;
	uchar		*p,*buf = _MGL_buf,*endp = buf + count;

	/* Build translation vector to map between the palettes */
	_MGL_buildTranslateVector(_MGL_translate,256,pal,256,(palette_t*)dc->colorTab);

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		while (p != endp)
			*p++ = _MGL_translate[*s1++];
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate8to16(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1;
	ushort	*p,*buf = _MGL_buf,*endp = buf + count;
	color_t	*cp,colorTab[256];

	/* Build a color translation table from the source palette */
	for (cp = colorTab,i = 0; i < 256; i++,pal++,cp++)
		*cp = MGL_packColorFast(&dc->pf,pal->red,pal->green,pal->blue);

	/* Translate the pixels in the bitmap */
	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		while (p != endp)
			*p++ = colorTab[*s1++];
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate8to24(MGLDC *dc,int left,int top,int right,int bottom,
	int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1,*p,*buf = _MGL_buf,*endp = buf + count*3;
	color_t	c,*cp,colorTab[256];

	/* Build a color translation table from the source palette */
	for (cp = colorTab,i = 0; i < 256; i++,pal++,cp++)
		*cp = MGL_packColorFast(&dc->pf,pal->red,pal->green,pal->blue);

	/* Translate the pixels in the bitmap */
	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		while (p != endp) {
			c = colorTab[*s1++];
			*p++ = (uchar)c;
			*p++ = (uchar)(c >> 8);
			*p++ = (uchar)(c >> 16);
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translate8to32(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	palette_t *pal)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1;
	ulong	*p,*buf = _MGL_buf,*endp = buf + count;
	color_t	*cp,colorTab[256];

	/* Build a color translation table from the source palette */
	for (cp = colorTab,i = 0; i < 256; i++,pal++,cp++)
		*cp = MGL_packColorFast(&dc->pf,pal->red,pal->green,pal->blue);

	/* Translate the pixels in the bitmap */
	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		while (p != endp)
			*p++ = colorTab[*s1++];
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

/* Halftone dither an RGB image down to a 4 bit DC */

void _ASMAPI _MGL_translateRGBto4(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bytesPerPixel,pixel_format_t *pf)
{
	int			x,y,count = (right-left);
	int			dstRight = (dstLeft + count);
	int			dstBottom = (dstTop + (bottom - top));
	uchar		_HUGE *s,*s1,R,G,B;
	uchar		c,*p,*buf = _MGL_buf;
	palette_t	*devPal = (palette_t*)dc->colorTab;

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left*bytesPerPixel);
	for (y = dstTop; y < dstBottom; y++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		for (x = 0; x < count; x++) {
			MGL_unpackColorFast(pf,(*(color_t*)s1),R,G,B);
			s1 += bytesPerPixel;
			MGL_halfTonePixelFast(c,x,y,R,G,B);
			*p = _MGL_closestColor(&_MGL_halftonePal[c],devPal,16) << 4;
			x++;
			MGL_unpackColorFast(pf,(*(color_t*)s1),R,G,B);
			s1 += bytesPerPixel;
			MGL_halfTonePixelFast(c,x,y,R,G,B);
			*p++ |= _MGL_closestColor(&_MGL_halftonePal[c],devPal,16);
			}
		dc->r.putScanLine(dc,y,dstLeft,dstRight,buf,0,op);
		}
}

/* Halftone dither an RGB image down to an 8 bit DC */

void _ASMAPI _MGL_translateRGBto8(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bytesPerPixel,pixel_format_t *pf)
{
	int		x,y,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1,R,G,B;
	uchar	*p,*buf = _MGL_buf;

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left*bytesPerPixel);
	for (y = dstTop; y < dstBottom; y++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		for (x = 0; x < count; x++) {
			MGL_unpackColorFast(pf,(*(color_t*)s1),R,G,B);
			MGL_halfTonePixelFast(*p++,x,y,R,G,B);
			s1 += bytesPerPixel;
			}
		dc->r.putScanLine(dc,y,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translateRGBto16(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bytesPerPixel,pixel_format_t *pf)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1,R,G,B;
	ushort	*p,*buf = _MGL_buf,*endp = buf + count;

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left*bytesPerPixel);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		while (p != endp) {
			MGL_unpackColorFast(pf,(*(color_t*)s1),R,G,B);
			*p++ = MGL_packColorFast(&dc->pf,R,G,B);
			s1 += bytesPerPixel;
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translateRGBto24(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bytesPerPixel,pixel_format_t *pf)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1,R,G,B;
	uchar	*p,*buf = _MGL_buf,*endp = buf + count*3;

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left*bytesPerPixel);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		if (dc->pf.redPos == 0) {
			while (p != endp) {
				MGL_unpackColorFast(pf,(*(color_t*)s1),R,G,B);
				*p++ = R;
				*p++ = G;
				*p++ = B;
				s1 += bytesPerPixel;
				}
			}
		else {
			while (p != endp) {
				MGL_unpackColorFast(pf,(*(color_t*)s1),R,G,B);
				*p++ = B;
				*p++ = G;
				*p++ = R;
				s1 += bytesPerPixel;
				}
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

void _ASMAPI _MGL_translateRGBto32(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bytesPerPixel,pixel_format_t *pf)
{
	int		i,count = (right-left);
	int		dstRight = (dstLeft + count);
	int		dstBottom = (dstTop + (bottom - top));
	uchar	_HUGE *s,*s1,R,G,B;
	ulong	*p,*buf = _MGL_buf,*endp = buf + count;

	s = ((uchar _HUGE*)surface + ((long)top * bytesPerLine) + left*bytesPerPixel);
	for (i = dstTop; i < dstBottom; i++, s += bytesPerLine) {
		NORMALISE_PTR(s);
		s1 = (uchar*)s;	p = buf;
		while (p != endp) {
			MGL_unpackColorFast(pf,(*(color_t*)s1),R,G,B);
			*p++ = MGL_packColorFast(&dc->pf,R,G,B);
			s1 += bytesPerPixel;
			}
		dc->r.putScanLine(dc,i,dstLeft,dstRight,buf,0,op);
		}
}

#if defined(__INTEL__) && !defined(__16BIT__) && !defined(NO_ASSEMBLER)
void 	_ASMAPI _MGL_translateRGB24to8(MGLDC *dc,int left,int top,int right,
		int bottom,int dstLeft,int dstTop,int op,void *surface,
		int bytesPerLine);
void 	_ASMAPI _MGL_translateBGR24to8(MGLDC *dc,int left,int top,int right,
		int bottom,int dstLeft,int dstTop,int op,void *surface,
		int bytesPerLine);
void 	_ASMAPI _MGL_translateRGB15to8(MGLDC *dc,int left,int top,int right,
		int bottom,int dstLeft,int dstTop,int op,void *surface,
		int bytesPerLine);
void 	_ASMAPI _MGL_translateRGB16to8(MGLDC *dc,int left,int top,int right,
		int bottom,int dstLeft,int dstTop,int op,void *surface,
		int bytesPerLine);
#endif

void _ASMAPI __EMU__translateImage(MGLDC *dc,int left,int top,int right,
	int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,
	int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src)
/****************************************************************************
*
* Function:		__EMU__translateImage
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
*				bitsPerPixel	- Pixel depth for source buffer
*				pal				- Palette for the source image
*				pf				- Pixel format for the image
*
* Description:	Blt and image from a memory buffer onto the specified
*				device context, translating the pixel format on the fly.
*				The information in the memory buffer is always in packed
*				pixel format.
*
*				NOTE: We only ever get to this routine if the pixel formats
*					  for the source and destination contexts are different.
*					  If they are the same it will be handled by directly
*					  copying the pixel information.
*
****************************************************************************/
{
	int	bytesPerPixel = 0;

	/* Must have a source palette for translations */
	if (bitsPerPixel == 8 && !pal)
		return;

	dc->r.beginPixel(dc);
	switch (bitsPerPixel) {
		case 4:
			switch (dc->mi.bitsPerPixel) {
				case 4:
					MGL_fatalError("Do not support palette translations in 4 bit modes");
					break;
				case 8:
					_MGL_translate4to8(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				case 15:
				case 16:
					_MGL_translate4to16(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				case 24:
					_MGL_translate4to24(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				case 32:
					_MGL_translate4to32(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				}
			break;
		case 8:
			switch (dc->mi.bitsPerPixel) {
				case 4:
					_MGL_translate8to4(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				case 8:
					_MGL_translate8to8(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				case 15:
				case 16:
					_MGL_translate8to16(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				case 24:
					_MGL_translate8to24(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				case 32:
					_MGL_translate8to32(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine,pal);
					break;
				}
			break;
		case 15:
#if defined(__INTEL__) && !defined(__16BIT__) && !defined(NO_ASSEMBLER)
			if (dc->mi.bitsPerPixel == 8) {
				_MGL_translateRGB15to8(dc,left,top,right,bottom,dstLeft,
					dstTop,op,surface,bytesPerLine);
				}
			else
#endif
				bytesPerPixel = 2;
			break;
		case 16:
#if defined(__INTEL__) && !defined(__16BIT__) && !defined(NO_ASSEMBLER)
			if (dc->mi.bitsPerPixel == 8) {
				_MGL_translateRGB16to8(dc,left,top,right,bottom,dstLeft,
					dstTop,op,surface,bytesPerLine);
				}
			else
#endif
				bytesPerPixel = 2;
			break;
		case 24:
#if defined(__INTEL__) && !defined(__16BIT__) && !defined(NO_ASSEMBLER)
			if (dc->mi.bitsPerPixel == 8) {
				if (pf->redPos == 0)
					_MGL_translateBGR24to8(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine);
				else
					_MGL_translateRGB24to8(dc,left,top,right,bottom,dstLeft,
						dstTop,op,surface,bytesPerLine);
				}
			else
#endif
				bytesPerPixel = 3;
			break;
		case 32:
#if defined(__INTEL__) && !defined(__16BIT__) && !defined(NO_ASSEMBLER)
// TODO: Add fast 32 bit conversion routines in assembler
#endif
				bytesPerPixel = 4;
			break;
		}

	/* Handle translation of RGB images to the display device context */
	if (bytesPerPixel) {
		switch (dc->mi.bitsPerPixel) {
			case 4:
				_MGL_translateRGBto4(dc,left,top,right,bottom,dstLeft,dstTop,
					op,surface,bytesPerLine,bytesPerPixel,pf);
				break;
			case 8:
				_MGL_translateRGBto8(dc,left,top,right,bottom,dstLeft,dstTop,
					op,surface,bytesPerLine,bytesPerPixel,pf);
				break;
			case 15:
			case 16:
				_MGL_translateRGBto16(dc,left,top,right,bottom,dstLeft,dstTop,
					op,surface,bytesPerLine,bytesPerPixel,pf);
				break;
			case 24:
				_MGL_translateRGBto24(dc,left,top,right,bottom,dstLeft,dstTop,
					op,surface,bytesPerLine,bytesPerPixel,pf);
				break;
			case 32:
				_MGL_translateRGBto32(dc,left,top,right,bottom,dstLeft,dstTop,
					op,surface,bytesPerLine,bytesPerPixel,pf);
				break;
			}
		}
	dc->r.endPixel(dc);
	src = src;
}

#endif	/* !MGL_LITE */
