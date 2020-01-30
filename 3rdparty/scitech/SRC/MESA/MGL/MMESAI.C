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
* Description:	Include file defining macros to implement many of the
*				optimized rendering functions for the MGL Mesa driver.
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

/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/

#define	IMPLEMENT_WRITE_SPAN(DEPTH,FMT,TYPE)                             	\
void _mmesa_write_span_##DEPTH##_##FMT (GLcontext *ctx,GLuint n, GLint x,   \
	GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],  \
	GLubyte mask[])                                                         \
{                                                                           \
	TYPE *d = PACKED##DEPTH##_pixelAddr(x,FLIP(y));                         \
	if (mask) {                                                             \
		while (n >= 8) {                                                    \
			if (mask[0]) d[0] = PACK_COLOR_##FMT(red[0],green[0],blue[0]);  \
			if (mask[1]) d[1] = PACK_COLOR_##FMT(red[1],green[1],blue[1]);  \
			if (mask[2]) d[2] = PACK_COLOR_##FMT(red[2],green[2],blue[2]);  \
			if (mask[3]) d[3] = PACK_COLOR_##FMT(red[3],green[3],blue[3]);  \
			if (mask[4]) d[4] = PACK_COLOR_##FMT(red[4],green[4],blue[4]);  \
			if (mask[5]) d[5] = PACK_COLOR_##FMT(red[5],green[5],blue[5]);  \
			if (mask[6]) d[6] = PACK_COLOR_##FMT(red[6],green[6],blue[6]);  \
			if (mask[7]) d[7] = PACK_COLOR_##FMT(red[7],green[7],blue[7]);  \
			d += 8; red += 8; green += 8; blue += 8; mask += 8; n -= 8;     \
			}                                                               \
		while (n--) {                                                       \
			if (mask[0]) d[0] = PACK_COLOR_##FMT(red[0],green[0],blue[0]);  \
			d++; red++; green++; blue++; mask++;                            \
			}                                                               \
		}                                                                   \
	else {                                                                  \
		while (n >= 8) {                                                    \
			d[0] = PACK_COLOR_##FMT(red[0],green[0],blue[0]);               \
			d[1] = PACK_COLOR_##FMT(red[1],green[1],blue[1]);               \
			d[2] = PACK_COLOR_##FMT(red[2],green[2],blue[2]);               \
			d[3] = PACK_COLOR_##FMT(red[3],green[3],blue[3]);               \
			d[4] = PACK_COLOR_##FMT(red[4],green[4],blue[4]);               \
			d[5] = PACK_COLOR_##FMT(red[5],green[5],blue[5]);               \
			d[6] = PACK_COLOR_##FMT(red[6],green[6],blue[6]);               \
			d[7] = PACK_COLOR_##FMT(red[7],green[7],blue[7]);               \
			d += 8; red += 8; green += 8; blue += 8; n -= 8;                \
			}                                                               \
		while (n--) {                                                       \
			d[0] = PACK_COLOR_##FMT(red[0],green[0],blue[0]);               \
			d++; red++; green++; blue++;                                    \
			}                                                               \
		}                                                                   \
}

#define	IMPLEMENT_WRITE_SPAN_DITHER(DEPTH,FMT,TYPE)                     			\
void _mmesa_write_span_##DEPTH##_##FMT(GLcontext *ctx,GLuint n, GLint x,			\
	GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],          \
	GLubyte mask[])                                                                 \
{                                                                                   \
	TYPE *d;                                                                        \
	y = FLIP(y);                                                                    \
	d = PACKED##DEPTH##_pixelAddr(x,y);                                             \
	if (mask) {                                                                     \
		while (n >= 4) {                                                            \
			if (mask[0]) PACK_COLOR_##FMT(d[0],x,y,red[0],green[0],blue[0]);        \
			if (mask[1]) PACK_COLOR_##FMT(d[1],x+1,y,red[1],green[1],blue[1]);      \
			if (mask[2]) PACK_COLOR_##FMT(d[2],x+2,y,red[2],green[2],blue[2]);      \
			if (mask[3]) PACK_COLOR_##FMT(d[3],x+3,y,red[3],green[3],blue[3]);      \
			d += 4; red += 4; green += 4; blue += 4; mask += 4; x += 4; n -= 4;     \
			}                                                                       \
		while (n--) {                                                               \
			if (mask[0]) PACK_COLOR_##FMT(d[0],x,y,red[0],green[0],blue[0]);        \
			d++; red++; green++; blue++; mask++; x++;                               \
			}                                                                       \
		}                                                                           \
	else {                                                                          \
		while (n >= 4) {                                                            \
			PACK_COLOR_##FMT(d[0],x,y,red[0],green[0],blue[0]);                     \
			PACK_COLOR_##FMT(d[1],x+1,y,red[1],green[1],blue[1]);                   \
			PACK_COLOR_##FMT(d[2],x+2,y,red[2],green[2],blue[2]);                   \
			PACK_COLOR_##FMT(d[3],x+3,y,red[3],green[3],blue[3]);                   \
			d += 4; red += 4; green += 4; blue += 4; x += 4; n -= 4;                \
			}                                                                       \
		while (n--) {                                                               \
			PACK_COLOR_##FMT(d[0],x,y,red[0],green[0],blue[0]);                     \
			d++; red++; green++; blue++; x++;                                       \
			}                                                                       \
		}                                                                           \
}

#define	IMPLEMENT_WRITE_SPAN_MONO(DEPTH,TYPE)								\
void _mmesa_write_span_mono_##DEPTH (GLcontext *ctx,GLuint n,GLint x,		\
	GLint y,GLubyte mask[])                                                 \
{                                                                           \
	TYPE *d = PACKED##DEPTH##_pixelAddr(x,FLIP(y));                         \
	while (n >= 8) {                                                        \
		if (mask[0]) d[0] = (TYPE)RC.color;                                 \
		if (mask[1]) d[1] = (TYPE)RC.color;                                 \
		if (mask[2]) d[2] = (TYPE)RC.color;                                 \
		if (mask[3]) d[3] = (TYPE)RC.color;                                 \
		if (mask[4]) d[4] = (TYPE)RC.color;                                 \
		if (mask[5]) d[5] = (TYPE)RC.color;                                 \
		if (mask[6]) d[6] = (TYPE)RC.color;                                 \
		if (mask[7]) d[7] = (TYPE)RC.color;                                 \
		d += 8; mask += 8; n -= 8;                                          \
		}                                                                   \
	while (n--) {                                                           \
		if (mask[0]) d[0] = (TYPE)RC.color;                                 \
		d++; mask++;                                                        \
		}                                                                   \
}

#define	IMPLEMENT_WRITE_SPAN_MONO_DITHER(DEPTH,FMT,TYPE)						\
void _mmesa_write_span_mono_##DEPTH##_##FMT (GLcontext *ctx,GLuint n,GLint x,	\
	GLint y,GLubyte mask[])                                                 	\
{                                                                           	\
	HALFTONE_VARS_##DEPTH;                                                      \
	TYPE *d;                                                                    \
	y = FLIP(y);                                                                \
	d = PACKED##DEPTH##_pixelAddr(x,y);                                         \
	SETUP_##FMT(y,RC.red,RC.green,RC.blue);                                     \
	while (n >= 4) {                                                            \
		if (mask[0]) PACK_COLOR2_##FMT(d[0],x);                                 \
		if (mask[1]) PACK_COLOR2_##FMT(d[1],x+1);                               \
		if (mask[2]) PACK_COLOR2_##FMT(d[2],x+2);                               \
		if (mask[3]) PACK_COLOR2_##FMT(d[3],x+3);                               \
		d += 4; mask += 4; x += 4; n -= 4;                                      \
		}                                                                       \
	while (n--) {                                                               \
		if (mask[0]) PACK_COLOR2_##FMT(d[0],x);                                 \
		d++; mask++; x++;                                                       \
		}                                                                       \
}

/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/

#define	IMPLEMENT_WRITE_PIXELS(DEPTH,FMT,TYPE)																	\
void _mmesa_write_pixels_##DEPTH##_##FMT (GLcontext *ctx,GLuint n, GLint x[],                                   \
	GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],                                                             \
	GLubyte a[],GLubyte mask[])                                                                                 \
{                                                                                                               \
	while (n >= 4) {                                                                                            \
		if (mask[0]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]))) = PACK_COLOR_##FMT(r[0],g[0],b[0]);   \
		if (mask[1]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[1],FLIP(y[1]))) = PACK_COLOR_##FMT(r[1],g[1],b[1]);   \
		if (mask[2]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[2],FLIP(y[2]))) = PACK_COLOR_##FMT(r[2],g[2],b[2]);   \
		if (mask[3]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[3],FLIP(y[3]))) = PACK_COLOR_##FMT(r[3],g[3],b[3]);   \
		r += 4; g += 4; b += 4; mask += 4; x+= 4; y += 4; n -= 4;                                               \
		}                                                                                                       \
	while (n--) {                                                                                               \
		if (mask[0]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]))) = PACK_COLOR_##FMT(r[0],g[0],b[0]);   \
		r++; g++; b++; mask++; x++; y++;                                                                        \
		}                                                                                                       \
}                                                                                                               

#define IMPLEMENT_WRITE_PIXELS_DITHER(DEPTH,FMT,TYPE)							\
void _mmesa_write_pixels_##DEPTH##_##FMT(GLcontext *ctx,GLuint n, GLint x[],    \
	GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],                     		\
	GLubyte a[],GLubyte mask[])                                                 \
{                                                                               \
	while (n--) {                                                               \
		if (mask[0]) {                                                          \
			TYPE *d = PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]));               \
			PACK_COLOR_##FMT(d[0],x[0],FLIP(y[0]),r[0],g[0],b[0]);              \
			}                                                                   \
		r++; g++; b++; mask++; x++; y++;                                        \
		}                                                                       \
}                                                                               

#define	IMPLEMENT_WRITE_PIXELS_MONO(DEPTH,TYPE)												\
void _mmesa_write_pixels_mono_##DEPTH (GLcontext *ctx,GLuint n,GLint x[],                   \
	GLint y[],GLubyte mask[])                                                               \
{                                                                                           \
	while (n >= 4) {                                                                        \
		if (mask[0]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]))) = (TYPE)RC.color; \
		if (mask[1]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[1],FLIP(y[1]))) = (TYPE)RC.color; \
		if (mask[2]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[2],FLIP(y[2]))) = (TYPE)RC.color; \
		if (mask[3]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[3],FLIP(y[3]))) = (TYPE)RC.color; \
		mask += 4; x+= 4; y += 4; n -= 4;                                                   \
		}                                                                                   \
	while (n--) {                                                                           \
		if (mask[0]) *((TYPE*)PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]))) = (TYPE)RC.color; \
		mask++; x++; y++;                                                                   \
		}                                                                                   \
}

#define IMPLEMENT_WRITE_PIXELS_MONO_DITHER(DEPTH,FMT,TYPE)							\
void _mmesa_write_pixels_mono_##DEPTH##_##FMT(GLcontext *ctx,GLuint n,GLint x[],    \
	GLint y[],GLubyte mask[])                                                       \
{                                                                                   \
	while (n--) {                                                                   \
		if (mask[0]) {                                                              \
			TYPE *d = PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]));                   \
			PACK_COLOR_##FMT(d[0],x[0],FLIP(y[0]),RC.red,RC.green,RC.blue);         \
			}                                                                       \
		mask++; x++; y++;                                                           \
		}                                                                           \
}

/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/

#define	IMPLEMENT_READ_SPAN(DEPTH,FMT,TYPE)											\
void _mmesa_read_span_##DEPTH##_##FMT(GLcontext *ctx,GLuint n, GLint x, GLint y,    \
	GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[])                 \
{                                                                                   \
	TYPE *d = PACKED##DEPTH##_pixelAddr(x,FLIP(y));                                 \
	while (n >= 8) {                                                                \
		UNPACK_COLOR_##FMT(d[0],red[0],green[0],blue[0]);                           \
		UNPACK_COLOR_##FMT(d[1],red[1],green[1],blue[1]);                           \
		UNPACK_COLOR_##FMT(d[2],red[2],green[2],blue[2]);                           \
		UNPACK_COLOR_##FMT(d[3],red[3],green[3],blue[3]);                           \
		UNPACK_COLOR_##FMT(d[4],red[4],green[4],blue[4]);                           \
		UNPACK_COLOR_##FMT(d[5],red[5],green[5],blue[5]);                           \
		UNPACK_COLOR_##FMT(d[6],red[6],green[6],blue[6]);                           \
		UNPACK_COLOR_##FMT(d[7],red[7],green[7],blue[7]);                           \
		d += 8; red += 8; green += 8; blue += 8; n -= 8;                            \
		}                                                                           \
	while (n--) {                                                                   \
		UNPACK_COLOR_##FMT(d[0],red[0],green[0],blue[0]);                           \
		d++; red++; green++; blue++;                                                \
		}                                                                           \
}

/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

#define	IMPLEMENT_READ_PIXELS(DEPTH,FMT,TYPE)									\
void _mmesa_read_pixels_##DEPTH##_##FMT (GLcontext *ctx,GLuint n, GLint x[],    \
	GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],                    \
	GLubyte alpha[],GLubyte mask[])                                             \
{                                                                               \
	while (n >= 4) {                                                            \
		if (mask[0]) {                                                          \
			TYPE *d = PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]));               \
			UNPACK_COLOR_##FMT(d[0],red[0],green[0],blue[0]);                   \
			}                                                                   \
		if (mask[1]) {                                                          \
			TYPE *d = PACKED##DEPTH##_pixelAddr(x[1],FLIP(y[1]));               \
			UNPACK_COLOR_##FMT(d[1],red[1],green[1],blue[1]);                   \
			}                                                                   \
		if (mask[2]) {                                                          \
			TYPE *d = PACKED##DEPTH##_pixelAddr(x[2],FLIP(y[2]));               \
			UNPACK_COLOR_##FMT(d[2],red[2],green[2],blue[2]);                   \
			}                                                                   \
		if (mask[3]) {                                                          \
			TYPE *d = PACKED##DEPTH##_pixelAddr(x[3],FLIP(y[3]));               \
			UNPACK_COLOR_##FMT(d[3],red[3],green[3],blue[3]);                   \
			}                                                                   \
		red += 4; green += 4; blue += 4; mask += 4; x += 4; y += 4; n -= 4;     \
		}                                                                       \
	while (n--) {                                                               \
		if (mask[0]) {                                                          \
			TYPE *d = PACKED##DEPTH##_pixelAddr(x[0],FLIP(y[0]));               \
			UNPACK_COLOR_##FMT(d[0],red[0],green[0],blue[0]);                   \
			}                                                                   \
		red++; green++; blue++; mask++; x++; y++;                               \
		}                                                                       \
}

