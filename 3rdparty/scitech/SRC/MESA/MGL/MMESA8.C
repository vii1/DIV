/****************************************************************************
*
*				 	   Mesa bindings for SciTech MGL
*
*               Copyright (C) 1996-1998 SciTech Software, Inc.
*                            All rights reserved.
*
* Language:     ANSI C
* Environment:	Any
*
* Description:	Optimized 8bpp rendering functions.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
****************************************************************************/

#include "mgl/mmesap.h"

/*------------------------- Implementation --------------------------------*/

#pragma warn -par
#include "mgl/mmesai.c"

/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/

void _mmesa_write_span_ci(GLcontext *ctx,GLuint n, GLint x, GLint y,
	GLuint index[],GLubyte mask[])
{
	uchar *d = PACKED8_pixelAddr(x,FLIP(y));
	while (n >= 8) {
		if (mask[0]) d[0] = (uchar)index[0];
		if (mask[1]) d[1] = (uchar)index[1];
		if (mask[2]) d[2] = (uchar)index[2];
		if (mask[3]) d[3] = (uchar)index[3];
		if (mask[4]) d[4] = (uchar)index[4];
		if (mask[5]) d[5] = (uchar)index[5];
		if (mask[6]) d[6] = (uchar)index[6];
		if (mask[7]) d[7] = (uchar)index[7];
		d += 8; index += 8; mask += 8; n -= 8;
		}
	while (n--) {
		if (mask[0]) d[0] = (uchar)index[0];
		d++; index++; mask++;
		}
}

void _mmesa_write_span_mono_ci(GLcontext *ctx,GLuint n,GLint x,GLint y,
	GLubyte mask[])
{
	uchar *d = PACKED8_pixelAddr(x,FLIP(y));
	while (n >= 8) {
		if (mask[0]) d[0] = (uchar)RC.color;
		if (mask[1]) d[1] = (uchar)RC.color;
		if (mask[2]) d[2] = (uchar)RC.color;
		if (mask[3]) d[3] = (uchar)RC.color;
		if (mask[4]) d[4] = (uchar)RC.color;
		if (mask[5]) d[5] = (uchar)RC.color;
		if (mask[6]) d[6] = (uchar)RC.color;
		if (mask[7]) d[7] = (uchar)RC.color;
		d += 8; mask += 8; n -= 8;
		}
	while (n--) {
		if (mask[0]) d[0] = (uchar)RC.color;
		d++; mask++;
		}
}

IMPLEMENT_WRITE_SPAN(8,8,uchar);
IMPLEMENT_WRITE_SPAN_DITHER(8,DITHER8,uchar);
IMPLEMENT_WRITE_SPAN_MONO(8,uchar);
IMPLEMENT_WRITE_SPAN_MONO_DITHER(8,DITHER8,uchar);

/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/

void _mmesa_write_pixels_ci(GLcontext *ctx,GLuint n, GLint x[],
	GLint y[],GLuint index[], GLubyte mask[])
{
	while (n >= 4) {
		if (mask[0]) *((uchar*)PACKED8_pixelAddr(x[0],FLIP(y[0]))) = (uchar)index[0];
		if (mask[1]) *((uchar*)PACKED8_pixelAddr(x[1],FLIP(y[1]))) = (uchar)index[1];
		if (mask[2]) *((uchar*)PACKED8_pixelAddr(x[2],FLIP(y[2]))) = (uchar)index[2];
		if (mask[3]) *((uchar*)PACKED8_pixelAddr(x[3],FLIP(y[3]))) = (uchar)index[3];
		index += 4; mask += 4; x += 4; y += 4; n -= 4;
		}
	while (n--) {
		if (mask[0]) *((uchar*)PACKED8_pixelAddr(x[0],FLIP(y[0]))) = (uchar)index[0];
		index++; mask++; x++; y++;
		}
}

void _mmesa_write_pixels_mono_ci(GLcontext *ctx,GLuint n,GLint x[],
	GLint y[],GLubyte mask[])
{
	while (n >= 4) {
		if (mask[0]) *((uchar*)PACKED8_pixelAddr(x[0],FLIP(y[0]))) = (uchar)RC.color;
		if (mask[1]) *((uchar*)PACKED8_pixelAddr(x[1],FLIP(y[1]))) = (uchar)RC.color;
		if (mask[2]) *((uchar*)PACKED8_pixelAddr(x[2],FLIP(y[2]))) = (uchar)RC.color;
		if (mask[3]) *((uchar*)PACKED8_pixelAddr(x[3],FLIP(y[3]))) = (uchar)RC.color;
		mask += 4; x += 4; y += 4; n -= 4;
		}
	while (n--) {
		if (mask[0]) *((uchar*)PACKED8_pixelAddr(x[0],FLIP(y[0]))) = (uchar)RC.color;
		mask++; x++; y++;
		}
}

IMPLEMENT_WRITE_PIXELS(8,8,uchar);
IMPLEMENT_WRITE_PIXELS_DITHER(8,DITHER8,uchar);
IMPLEMENT_WRITE_PIXELS_MONO(8,uchar);
IMPLEMENT_WRITE_PIXELS_MONO_DITHER(8,DITHER8,uchar);

/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/

void _mmesa_read_span_ci(GLcontext *ctx,GLuint n, GLint x, GLint y,
	GLuint index[])
{
	uchar *d = PACKED8_pixelAddr(x,FLIP(y));
	while (n >= 8) {
		index[0] = d[0];
		index[1] = d[1];
		index[2] = d[2];
		index[3] = d[3];
		index[4] = d[4];
		index[5] = d[5];
		index[6] = d[6];
		index[7] = d[7];
		d += 8; index += 8; n -= 8;
		}
	while (n--) {
		index[0] = d[0];
		d++; index++;
		}
}

void _mmesa_read_span_8(GLcontext *ctx,GLuint n, GLint x, GLint y,
	GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[])
{
	color_t	color,*colorTab = RC.dc->colorTab;
	uchar *d = PACKED8_pixelAddr(x,FLIP(y));
	while (n--) {
		color = colorTab[d[0]];
		UNPACK_COLOR_RGB(color,red[0],green[0],blue[0]);
		red++; green++; blue++; d++;
		}
}

/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

void _mmesa_read_pixels_ci(GLcontext *ctx,GLuint n, GLint x[],
	GLint y[],GLuint index[], GLubyte mask[])
{
	while (n--) {
		if (mask[0]) {
			uchar *d = PACKED8_pixelAddr(x[0],FLIP(y[0]));
			index[0] = d[0];
			}	
		index++; mask++; x++; y++;
		}
}

void _mmesa_read_pixels_8(GLcontext *ctx,GLuint n, GLint x[],
	GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],
	GLubyte alpha[],GLubyte mask[])
{
	color_t *colorTab = RC.dc->colorTab;
	while (n--) {
		if (mask[0]) {
			uchar *d = PACKED8_pixelAddr(x[0],FLIP(y[0]));
			color_t color = colorTab[d[0]];
			UNPACK_COLOR(color,red[0],green[0],blue[0]);
			}
		red++; green++; blue++; mask++; x++; y++;
		}
}

