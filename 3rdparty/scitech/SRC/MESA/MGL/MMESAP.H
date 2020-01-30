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
* Description:	Internal header file for the Mesa/OpenGL interface bindings
*				for the SciTech MGL graphics library. 
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

#ifndef	__MMESAP_H
#define	__MMESAP_H

#include "context.h"
#include "mgl_int.h"
#include "matrix.h"
#include "types.h"
#include "vb.h"
#include "mgl/halftone.h"

/*---------------------- Macros and type definitions ----------------------*/

/* Internal Mesa rendering context */

struct MGLMesaContext {
	GLcontext		*gl_ctx;			/* the core Mesa context 		*/
	GLvisual		*gl_vis;			/* describes the color buffer 	*/
	GLframebuffer	*gl_buffer;			/* the ancillary buffers 		*/
	MGLDC			*dc;				/* Rendering DC					*/
	MGLDC			*dispdc;			/* Display DC					*/
	MGLDC			*memdc;				/* Memory DC back buffer		*/
	color_t			clearColor;			/* Color to clear device with	*/
	color_t			color;				/* Current color				*/
	uchar			red,green,blue;		/* RGB components for color		*/
	int				bottom;				/* Bottom coordinate for buffer	*/
	int				bufferMode;			/* Current buffer access mode	*/
	int				frontbuffer;		/* Page index of front buffer	*/
	int				backbuffer;			/* Page index of back buffer	*/
	uint 			pixelformat;		/* Current pixel format 		*/
	uint 			undithered_pf;		/* Undithered pixel format		*/
	uint 			dithered_pf;		/* Dithered pixel format		*/
	};

/* Macros for MGL API callbacks when running in a DLL for Windows */

#ifdef	__WINDOWS__
#define MGL_makeCurrentDC(dc)												_MGL_callbacks.makeCurrentDC(dc)
#define	MGL_clearCurrentDC()												_MGL_callbacks.makeCurrentDC(NULL)
#define MGL_setActivePage(dc,page)											_MGL_callbacks.setActivePage(dc,page)
#define MGL_setVisualPage(dc,page,waitVRT)									_MGL_callbacks.setVisualPage(dc,page,waitVRT)
#define MGL_surfaceAccessType(dc)											_MGL_callbacks.surfaceAccessType(dc)
#define MGL_isDisplayDC(dc)                                                 _MGL_callbacks.isDisplayDC(dc)
#define MGL_isWindowedDC(dc)                                                _MGL_callbacks.isWindowedDC(dc)
#define MGL_isMemoryDC(dc)                                                  _MGL_callbacks.isMemoryDC(dc)
#define MGL_createMemoryDC(xSize,ySize,bitsPerPixel,pf)                     _MGL_callbacks.createMemoryDC(xSize,ySize,bitsPerPixel,pf)
#define MGL_destroyDC(dc)                                                   _MGL_callbacks.destroyDC(dc)
#define MGL_bitBltCoord(dst,src,left,top,right,bottom,dstLeft,dstTop,op)    _MGL_callbacks.bitBltCoord(dst,src,left,top,right,bottom,dstLeft,dstTop,op)
#define MGL_setPaletteEntry(dc,entry,red,green,blue)                        _MGL_callbacks.setPaletteEntry(dc,entry,red,green,blue)
#define MGL_setPalette(dc,pal,numColors,startIndex)                         _MGL_callbacks.setPalette(dc,pal,numColors,startIndex)
#define MGL_getPalette(dc,pal,numColors,startIndex)                         _MGL_callbacks.getPalette(dc,pal,numColors,startIndex)
#define MGL_realizePalette(dc,numColors,startIndex,waitVRT)                 _MGL_callbacks.realizePalette(dc,numColors,startIndex,waitVRT)
#else
#define	MGL_clearCurrentDC()												MGL_makeCurrentDC(NULL);
#endif

/* Macro to pack a color value for use by the MGL rendering functions. Since
 * Mesa pre-scales our color values for us to the correct range we simply
 * need to shift and combine the color values into the final packed color
 * value used by the MGL.
 */

#define	PACK_COLOR(R,G,B)					\
	((ulong)(R) << PF.redPos)			\
	| ((ulong)(G) << PF.greenPos)		\
	| ((ulong)(B) << PF.bluePos)

#define	UNPACK_COLOR(c,R,G,B)										\
{																	\
 (R) = (uchar)(((ulong)(c) >> PF.redPos) & PF.redMask);		\
 (G) = (uchar)(((ulong)(c) >> PF.greenPos) & PF.greenMask);	\
 (B) = (uchar)(((ulong)(c) >> PF.bluePos) & PF.blueMask);		\
}

/* Converts a GL window Y coord to an X window Y coord */

#define FLIP(Y)  (RC.bottom - (Y))

/* Values for RC.pixelformat */

#define PF_INDEX		1		/* Color Index mode 					*/
#define	PF_RGB8			2		/* 8bpp RGB without dithering			*/
#define	PF_DITHER8		3		/* 8bpp RGB dithered					*/
#define	PF_RGB555		4		/* 15bpp RGB without dithering			*/
#define	PF_DITHER555	5		/* 15bpp RGB dithered					*/
#define	PF_RGB565		6       /* 16bpp RGB without dithering			*/
#define	PF_DITHER565	7		/* 16bpp RGB dithered					*/
#define	PF_RGB888		8		/* 24bpp RGB TrueColor					*/
#define	PF_BGR888		9       /* 24bpp BGR TrueColor					*/
#define	PF_ARGB8888		10		/* 32bpp ARGB TrueColor					*/
#define	PF_ABGR8888		11     	/* 32bpp ABGR TrueColor					*/
#define	PF_RGBA8888		12		/* 32bpp RGBA TrueColor					*/
#define	PF_BGRA8888		13		/* 32bpp BGRA TrueColor					*/

/* Macros to directly access the software z-buffer */

#define	ZBUF_ADDR(x,y)											\
	((GLdepth _HUGE*)(RC.dc->zbuffer)							\
		+ (((y) - RC.dc->size.top) * (long)RC.dc->zwidth)		\
		+ (x) - RC.dc->size.left)

/* Macros for 8bpp direct surface access */

#define	PACKED8_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)RC.dc->surface + ((long)y * MI.bytesPerLine) + x))

#define	PACK_COLOR_8(R,G,B)										\
	(uchar)(20 + _MGL_div51[R] + _MGL_mul6[_MGL_div51[G]] +		\
		  _MGL_mul36[_MGL_div51[B]])

/* Macros for 16bpp direct surface access */

#define	PACKED16_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)RC.dc->surface + ((long)y * MI.bytesPerLine) + x*2))

#define	PACK_COLOR_555(R,G,B)				\
	(ushort)(((ulong)(R) << 10)				\
			| ((ulong)(G) << 5)				\
			| ((ulong)(B) << 0))

#define	PACK_COLOR_565(R,G,B)				\
	(ushort)(((ulong)(R) << 11)				\
			| ((ulong)(G) << 5)				\
			| ((ulong)(B) << 0))

#define	UNPACK_COLOR_555(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 10) & 0x3);			\
 (G) = (uchar)(((ulong)(c) >> 5) & 0x3);			\
 (B) = (uchar)(((ulong)(c) >> 0) & 0x3);			\
}

#define	UNPACK_COLOR_565(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 11) & 0x3);			\
 (G) = (uchar)(((ulong)(c) >> 5) & 0x7);			\
 (B) = (uchar)(((ulong)(c) >> 0) & 0x3);			\
}

/* Macros for 24bpp direct surface access */

#define	PACKED24_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)RC.dc->surface + ((long)y * MI.bytesPerLine) + x*3))

#define	PACK_COLOR_RGB(R,G,B)		\
	((ulong)(R) << 16)				\
	| ((ulong)(G) << 8)				\
	| ((ulong)(B) << 0)

#define	PACK_COLOR_BGR(R,G,B)		\
	((ulong)(R) << 0)				\
	| ((ulong)(G) << 8)				\
	| ((ulong)(B) << 16)

#define	UNPACK_COLOR_RGB(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 16) & 0xFF);			\
 (G) = (uchar)(((ulong)(c) >> 8) & 0xFF);			\
 (B) = (uchar)(((ulong)(c) >> 0) & 0xFF);			\
}

#define	UNPACK_COLOR_BGR(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 0) & 0xFF);			\
 (G) = (uchar)(((ulong)(c) >> 8) & 0xFF);			\
 (B) = (uchar)(((ulong)(c) >> 16) & 0xFF);			\
}

/* Macros for 32bpp direct surface access */

#define	PACKED32_pixelAddr(x,y)	\
	 ((void*)((uchar _HUGE *)RC.dc->surface + ((long)y * MI.bytesPerLine) + x*4))

#define	PACK_COLOR_ARGB(R,G,B)		\
	((ulong)(R) << 16)				\
	| ((ulong)(G) << 8)				\
	| ((ulong)(B) << 0)

#define	PACK_COLOR_ABGR(R,G,B)		\
	((ulong)(R) << 0)				\
	| ((ulong)(G) << 8)				\
	| ((ulong)(B) << 16)

#define	PACK_COLOR_RGBA(R,G,B)		\
	((ulong)(R) << 24)				\
	| ((ulong)(G) << 16)			\
	| ((ulong)(B) << 8)

#define	PACK_COLOR_BGRA(R,G,B)		\
	((ulong)(R) << 8)				\
	| ((ulong)(G) << 16)			\
	| ((ulong)(B) << 24)

#define	UNPACK_COLOR_ARGB(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 16) & 0xFF);			\
 (G) = (uchar)(((ulong)(c) >> 8) & 0xFF);			\
 (B) = (uchar)(((ulong)(c) >> 0) & 0xFF);			\
}

#define	UNPACK_COLOR_ABGR(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 0) & 0xFF);			\
 (G) = (uchar)(((ulong)(c) >> 8) & 0xFF);			\
 (B) = (uchar)(((ulong)(c) >> 16) & 0xFF);			\
}

#define	UNPACK_COLOR_RGBA(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 24) & 0xFF);			\
 (G) = (uchar)(((ulong)(c) >> 16) & 0xFF);			\
 (B) = (uchar)(((ulong)(c) >> 8) & 0xFF);			\
}

#define	UNPACK_COLOR_BGRA(c,R,G,B)					\
{													\
 (R) = (uchar)(((ulong)(c) >> 8) & 0xFF);			\
 (G) = (uchar)(((ulong)(c) >> 16) & 0xFF);			\
 (B) = (uchar)(((ulong)(c) >> 24) & 0xFF);			\
}

/*--------------------------- Global Variables ----------------------------*/

#define	RC		_MM_rc
#define	MI		_MGL_mi
#define	PF		_MGL_pf
#define	VECS	_MGL_vecs
extern MGLRC			_MM_rc;			/* Current rendering context	*/
extern MGLRC			*_MM_rcPtr;		/* Pointer to current context	*/
extern gmode_t			_MGL_mi;		/* MGL mode information			*/
extern pixel_format_t	_MGL_pf;		/* MGL mode pixel format		*/
extern vecs				_MGL_vecs;		/* MGL Rendering vectors 		*/

/*------------------------- Function Prototypes ---------------------------*/

/* General functions */

void setup_DD_pointers(GLcontext *ctx);
points_func mmesa_get_points_func(GLcontext *ctx);
line_func mmesa_get_line_func(GLcontext *ctx);
triangle_func mmesa_get_triangle_func(GLcontext *ctx);

/* 8bpp rendering functions */

void _mmesa_write_span_ci(GLcontext *ctx,GLuint n, GLint x, GLint y,GLuint index[],GLubyte mask[]);
void _mmesa_write_span_8_8(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_8_DITHER8(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_mono_ci(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_span_mono_8(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_span_mono_8_DITHER8(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_pixels_ci(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLuint index[], GLubyte mask[]);
void _mmesa_write_pixels_8_8(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_8_DITHER8(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_mono_ci(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_write_pixels_mono_8(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_write_pixels_mono_8_DITHER8(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_read_span_ci(GLcontext *ctx,GLuint n, GLint x, GLint y,GLuint index[]);
void _mmesa_read_span_8(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_pixels_ci(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLuint index[], GLubyte mask[]);
void _mmesa_read_pixels_8(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);

/* 15/16bpp rendering functions */ 

void _mmesa_write_span_16_555(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_16_565(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_16_DITHER555(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_16_DITHER565(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_mono_16(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_span_mono_16_DITHER555(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_span_mono_16_DITHER565(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_pixels_16_555(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_16_565(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_16_DITHER555(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_16_DITHER565(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_mono_16(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_write_pixels_mono_16_DITHER555(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_write_pixels_mono_16_DITHER565(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_read_span_16_555(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_span_16_565(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_pixels_16_555(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_read_pixels_16_565(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);

/* 24bpp rendering functions */

void _mmesa_write_span_24_RGB(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_24_BGR(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_mono_24_RGB(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_span_mono_24_BGR(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_pixels_24_RGB(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_24_BGR(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_mono_24_RGB(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_write_pixels_mono_24_BGR(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_read_span_24_RGB(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_span_24_BGR(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_pixels_24_RGB(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_read_pixels_24_BGR(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);

/* 32bpp rendering functions */

void _mmesa_write_span_32_ARGB(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_32_ABGR(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_32_RGBA(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_32_BGRA(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_write_span_mono_32(GLcontext *ctx,GLuint n,GLint x,GLint y,GLubyte mask[]);
void _mmesa_write_pixels_32_ARGB(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_32_ABGR(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_32_RGBA(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_32_BGRA(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte r[], GLubyte g[],GLubyte b[],GLubyte a[],GLubyte mask[]);
void _mmesa_write_pixels_mono_32(GLcontext *ctx,GLuint n,GLint x[],GLint y[],GLubyte mask[]);
void _mmesa_read_span_32_ARGB(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_span_32_ABGR(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_span_32_RGBA(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_span_32_BGRA(GLcontext *ctx,GLuint n, GLint x, GLint y,GLubyte red[], GLubyte green[],GLubyte blue[], GLubyte alpha[]);
void _mmesa_read_pixels_32_ARGB(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_read_pixels_32_ABGR(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_read_pixels_32_RGBA(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);
void _mmesa_read_pixels_32_BGRA(GLcontext *ctx,GLuint n, GLint x[],GLint y[],GLubyte red[], GLubyte green[],GLubyte blue[],GLubyte alpha[],GLubyte mask[]);

#endif	/* __MMESAP_H */

