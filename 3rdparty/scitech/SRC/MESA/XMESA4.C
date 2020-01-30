/* $Id: xmesa4.c,v 1.4 1997/06/03 02:00:23 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.3
 * Copyright (C) 1995-1997  Brian Paul
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
 */


/*
 * $Log: xmesa4.c,v $
 * Revision 1.4  1997/06/03 02:00:23  brianp
 * fixed problem with polygon stippling
 *
 * Revision 1.3  1997/05/26 20:34:57  brianp
 * renamed PACK_RGB to PACK_TRUECOLOR
 *
 * Revision 1.2  1997/03/16 02:16:32  brianp
 * moved PACK_8B8G8R macro to xmesaP.h
 *
 * Revision 1.1  1997/03/16 02:14:45  brianp
 * Initial revision
 *
 */


/*
 * Mesa/X11 interface, part 4.
 *
 * This file contains "accelerated" triangle functions.  It should be
 * fairly easy to write new special-purpose triangle functions and hook
 * them into this module.
 */


#include <sys/time.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "depth.h"
#include "macros.h"
#include "vb.h"
#include "types.h"
#include "xmesaP.h"




/**********************************************************************/
/***                   Triangle rendering                           ***/
/**********************************************************************/

/*
 * Render a triangle into a pixmap, any pixel format, flat shaded and
 * no raster ops.
 */
void flat_pixmap_triangle( GLcontext *ctx,
 	                   GLuint v0, GLuint v1, GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   XPoint p[3];
   GC gc;
   if (VB->MonoColor) {
      gc = xmesa->xm_buffer->gc1;  /* use current color */
   }
   else {
      unsigned long pixel;
      if (xmesa->xm_visual->gl_visual->RGBAflag) {
         pixel = xmesa_color_to_pixel( xmesa,
                                       VB->Color[pv][0], VB->Color[pv][1],
                                       VB->Color[pv][2], VB->Color[pv][3] );
      }
      else {
         pixel = VB->Index[pv];
      }
      gc = xmesa->xm_buffer->gc2;
      XSetForeground( xmesa->display, gc, pixel );
   }
   p[0].x =       (GLint) (VB->Win[v0][0] + 0.5f);
   p[0].y = FLIP( (GLint) (VB->Win[v0][1] - 0.5f) );
   p[1].x =       (GLint) (VB->Win[v1][0] + 0.5f);
   p[1].y = FLIP( (GLint) (VB->Win[v1][1] - 0.5f) );
   p[2].x =       (GLint) (VB->Win[v2][0] + 0.5f);
   p[2].y = FLIP( (GLint) (VB->Win[v2][1] - 0.5f) );
   XFillPolygon( xmesa->display, xmesa->xm_buffer->buffer, gc,
		 p, 3, Convex, CoordModeOrigin );
}



/*
 * XImage, smooth, depth-buffered, PF_TRUECOLOR triangle.
 */
static void smooth_TRUECOLOR_z_triangle( GLcontext *ctx,
                                         GLuint v0, GLuint v1, GLuint v2,
                                         GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define INTERP_Z 1
#define INTERP_RGB 1
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, xx = LEFT, yy = FLIP(Y), len = RIGHT-LEFT;			\
   for (i=0;i<len;i++,xx++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         unsigned long p;						\
         PACK_TRUECOLOR(p, FixedToInt(ffr), FixedToInt(ffg), FixedToInt(ffb));\
         XPutPixel( img, xx, yy, p );					\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}



/*
 * XImage, smooth, depth-buffered, PF_8A8B8G8R triangle.
 */
static void smooth_8A8B8G8R_z_triangle( GLcontext *ctx,
                                         GLuint v0, GLuint v1, GLuint v2,
                                         GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, len = RIGHT-LEFT;						\
   for (i=0;i<len;i++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         pRow[i] = PACK_8B8G8R( FixedToInt(ffr), FixedToInt(ffg),	\
				 FixedToInt(ffb) );			\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, depth-buffered, PF_8R8G8B triangle.
 */
static void smooth_8R8G8B_z_triangle( GLcontext *ctx,
                                         GLuint v0, GLuint v1, GLuint v2,
                                         GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, len = RIGHT-LEFT;						\
   for (i=0;i<len;i++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         pRow[i] = PACK_8R8G8B( FixedToInt(ffr), FixedToInt(ffg),	\
				 FixedToInt(ffb) );			\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}



/*
 * XImage, smooth, depth-buffered, PF_5R6G5B triangle.
 */
static void smooth_5R6G5B_z_triangle( GLcontext *ctx,
                                         GLuint v0, GLuint v1, GLuint v2,
                                         GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, len = RIGHT-LEFT;						\
   for (i=0;i<len;i++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         pRow[i] = PACK_5R6G5B( FixedToInt(ffr), FixedToInt(ffg),	\
				 FixedToInt(ffb) );			\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, depth-buffered, 8-bit, PF_DITHER8 triangle.
 */
static void smooth_DITHER8_z_triangle( GLcontext *ctx,
                                       GLuint v0, GLuint v1, GLuint v2,
                                       GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, xx = LEFT, yy = FLIP(Y), len = RIGHT-LEFT;			\
   XDITHER_SETUP(yy);							\
   for (i=0;i<len;i++,xx++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         pRow[i] = XDITHER( xx, FixedToInt(ffr), FixedToInt(ffg),	\
				FixedToInt(ffb) );			\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}



/*
 * XImage, smooth, depth-buffered, PF_DITHER triangle.
 */
static void smooth_DITHER_z_triangle( GLcontext *ctx,
                                       GLuint v0, GLuint v1, GLuint v2,
                                       GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, xx = LEFT, yy = FLIP(Y), len = RIGHT-LEFT;			\
   XDITHER_SETUP(yy);							\
   for (i=0;i<len;i++,xx++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
	 unsigned long p = XDITHER( xx, FixedToInt(ffr),		\
				 FixedToInt(ffg), FixedToInt(ffb) );	\
	 XPutPixel( img, xx, yy, p );					\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, depth-buffered, 8-bit PF_LOOKUP triangle.
 */
static void smooth_LOOKUP8_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                       GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, len = RIGHT-LEFT;						\
   LOOKUP_SETUP;							\
   for (i=0;i<len;i++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         pRow[i] = LOOKUP( FixedToInt(ffr), FixedToInt(ffg),		\
				 FixedToInt(ffb) );			\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}



/*
 * XImage, smooth, depth-buffered, 8-bit PF_HPCR triangle.
 */
static void smooth_HPCR_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                    GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, xx = LEFT, yy = FLIP(Y), len = RIGHT-LEFT;			\
   for (i=0;i<len;i++,xx++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         pRow[i] = DITHER_HPCR( xx, yy, FixedToInt(ffr),		\
				 FixedToInt(ffg), FixedToInt(ffb) );	\
         zRow[i] = z;							\
      }									\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}



/*
 * XImage, flat, depth-buffered, PF_TRUECOLOR triangle.
 */
static void flat_TRUECOLOR_z_triangle( GLcontext *ctx,
                        	       GLuint v0, GLuint v1, GLuint v2,
                                       GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define INTERP_Z 1
#define SETUP_CODE						\
   unsigned long pixel;						\
   PACK_TRUECOLOR(pixel, VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2]);

#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, xx = LEFT, yy = FLIP(Y), len = RIGHT-LEFT;			\
   for (i=0;i<len;i++,xx++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         XPutPixel( img, xx, yy, pixel );				\
         zRow[i] = z;							\
      }									\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, flat, depth-buffered, PF_8A8B8G8R triangle.
 */
static void flat_8A8B8G8R_z_triangle( GLcontext *ctx, GLuint v0,
                        	      GLuint v1, GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE					\
   unsigned long p = PACK_8B8G8R( VB->Color[pv][0],	\
		 VB->Color[pv][1], VB->Color[pv][2] );
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, len = RIGHT-LEFT;						\
   for (i=0;i<len;i++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
	 pRow[i] = p;							\
         zRow[i] = z;							\
      }									\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, flat, depth-buffered, PF_8R8G8B triangle.
 */
static void flat_8R8G8B_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                    GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE					\
   unsigned long p = PACK_8R8G8B( VB->Color[pv][0],	\
		 VB->Color[pv][1], VB->Color[pv][2] );
#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint i, len = RIGHT-LEFT;				\
   for (i=0;i<len;i++) {				\
      GLdepth z = FixedToDepth(ffz);			\
      if (z < zRow[i]) {				\
	 pRow[i] = p;					\
         zRow[i] = z;					\
      }							\
      ffz += fdzdx;					\
   }							\
}
#include "tritemp.h"
}


/*
 * XImage, flat, depth-buffered, PF_5R6G5B triangle.
 */
static void flat_5R6G5B_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                    GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE					\
   unsigned long p = PACK_5R6G5B( VB->Color[pv][0],	\
		 VB->Color[pv][1], VB->Color[pv][2] );
#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint i, len = RIGHT-LEFT;				\
   for (i=0;i<len;i++) {				\
      GLdepth z = FixedToDepth(ffz);			\
      if (z < zRow[i]) {				\
	 pRow[i] = p;					\
         zRow[i] = z;					\
      }							\
      ffz += fdzdx;					\
   }							\
}
#include "tritemp.h"
}


/*
 * XImage, flat, depth-buffered, 8-bit PF_DITHER triangle.
 */
static void flat_DITHER8_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                     GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE	\
   FLAT_DITHER_SETUP( VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2] );

#define INNER_LOOP( LEFT, RIGHT, Y )				\
{								\
   GLint i, xx = LEFT, len = RIGHT-LEFT;			\
   FLAT_DITHER_ROW_SETUP(FLIP(Y));				\
   for (i=0;i<len;i++,xx++) {					\
      GLdepth z = FixedToDepth(ffz);				\
      if (z < zRow[i]) {					\
	 pRow[i] = FLAT_DITHER(xx);				\
         zRow[i] = z;						\
      }								\
      ffz += fdzdx;						\
   }								\
}
#include "tritemp.h"
}


/*
 * XImage, flat, depth-buffered, PF_DITHER triangle.
 */
static void flat_DITHER_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                    GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define INTERP_Z 1
#define SETUP_CODE	\
   FLAT_DITHER_SETUP( VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2] );

#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint i, xx = LEFT, yy = FLIP(Y), len = RIGHT-LEFT;			\
   FLAT_DITHER_ROW_SETUP(yy);						\
   for (i=0;i<len;i++,xx++) {						\
      GLdepth z = FixedToDepth(ffz);					\
      if (z < zRow[i]) {						\
         unsigned long p = FLAT_DITHER(xx);				\
	 XPutPixel( img, xx, yy, p );					\
         zRow[i] = z;							\
      }									\
      ffz += fdzdx;							\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, flat, depth-buffered, 8-bit PF_HPCR triangle.
 */
static void flat_HPCR_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                  GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE				\
   GLubyte r = VB->Color[pv][0];		\
   GLubyte g = VB->Color[pv][1];		\
   GLubyte b = VB->Color[pv][2];
#define INNER_LOOP( LEFT, RIGHT, Y )				\
{								\
   GLint i, xx = LEFT, yy = FLIP(Y), len = RIGHT-LEFT;		\
   for (i=0;i<len;i++,xx++) {					\
      GLdepth z = FixedToDepth(ffz);				\
      if (z < zRow[i]) {					\
	 pRow[i] = DITHER_HPCR( xx, yy, r, g, b );		\
         zRow[i] = z;						\
      }								\
      ffz += fdzdx;						\
   }								\
}
#include "tritemp.h"
}



/*
 * XImage, flat, depth-buffered, 8-bit PF_LOOKUP triangle.
 */
static void flat_LOOKUP8_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                        	     GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE				\
   LOOKUP_SETUP;				\
   GLubyte r = VB->Color[pv][0];		\
   GLubyte g = VB->Color[pv][1];		\
   GLubyte b = VB->Color[pv][2];		\
   GLubyte p = LOOKUP(r,g,b);
#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint i, len = RIGHT-LEFT;				\
   for (i=0;i<len;i++) {				\
      GLdepth z = FixedToDepth(ffz);			\
      if (z < zRow[i]) {				\
	 pRow[i] = p;					\
         zRow[i] = z;					\
      }							\
      ffz += fdzdx;					\
   }							\
}
#include "tritemp.h"
}



/*
 * XImage, smooth, NON-depth-buffered, PF_TRUECOLOR triangle.
 */
static void smooth_TRUECOLOR_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                       GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define INTERP_RGB 1
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx, yy = FLIP(Y);						\
   for (xx=LEFT;xx<RIGHT;xx++) {					\
      unsigned long p;							\
      PACK_TRUECOLOR(p, FixedToInt(ffr), FixedToInt(ffg), FixedToInt(ffb));\
      XPutPixel( img, xx, yy, p );					\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, PF_8A8B8G8R triangle.
 */
static void smooth_8A8B8G8R_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
				      GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx;								\
   PIXEL_TYPE *pixel = pRow;						\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {				\
      *pixel = PACK_8B8G8R( FixedToInt(ffr), FixedToInt(ffg),		\
				FixedToInt(ffb) );			\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, PF_8R8G8B triangle.
 */
static void smooth_8R8G8B_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                    GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx;								\
   PIXEL_TYPE *pixel = pRow;						\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {				\
      *pixel = PACK_8R8G8B( FixedToInt(ffr), FixedToInt(ffg),		\
				FixedToInt(ffb) );			\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, PF_5R6G5B triangle.
 */
static void smooth_5R6G5B_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
				    GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx;								\
   PIXEL_TYPE *pixel = pRow;						\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {				\
      *pixel = PACK_5R6G5B( FixedToInt(ffr), FixedToInt(ffg),		\
				FixedToInt(ffb) );			\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, 8-bit PF_DITHER triangle.
 */
static void smooth_DITHER8_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
				     GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx, yy = FLIP(Y);						\
   PIXEL_TYPE *pixel = pRow;						\
   XDITHER_SETUP(yy);							\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {				\
      *pixel = XDITHER( xx, FixedToInt(ffr), FixedToInt(ffg),		\
				FixedToInt(ffb) );			\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, PF_DITHER triangle.
 */
static void smooth_DITHER_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                    GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define INTERP_RGB 1
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx, yy = FLIP(Y);						\
   XDITHER_SETUP(yy);							\
   for (xx=LEFT;xx<RIGHT;xx++) {					\
      unsigned long p = XDITHER( xx, FixedToInt(ffr),			\
				FixedToInt(ffg), FixedToInt(ffb) );	\
      XPutPixel( img, xx, yy, p );					\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, smooth, NON-depth-buffered, 8-bit PF_LOOKUP triangle.
 */
static void smooth_LOOKUP8_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                     GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx;								\
   PIXEL_TYPE *pixel = pRow;						\
   LOOKUP_SETUP;							\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {				\
      *pixel = LOOKUP( FixedToInt(ffr), FixedToInt(ffg),		\
			FixedToInt(ffb) );				\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}



/*
 * XImage, smooth, NON-depth-buffered, 8-bit PF_HPCR triangle.
 */
static void smooth_HPCR_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                  GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define INTERP_RGB 1
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define INNER_LOOP( LEFT, RIGHT, Y )					\
{									\
   GLint xx, yy = FLIP(Y);						\
   PIXEL_TYPE *pixel = pRow;						\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {				\
      *pixel = DITHER_HPCR( xx, yy, FixedToInt(ffr),			\
				FixedToInt(ffg), FixedToInt(ffb) );	\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;			\
   }									\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, PF_TRUECOLOR triangle.
 */
static void flat_TRUECOLOR_triangle( GLcontext *ctx, GLuint v0,
                                     GLuint v1, GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define SETUP_CODE						\
   unsigned long pixel;						\
   PACK_TRUECOLOR(pixel, VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2]);

#define INNER_LOOP( LEFT, RIGHT, Y )				\
{								\
   GLint xx, yy = FLIP(Y);					\
   for (xx=LEFT;xx<RIGHT;xx++) {				\
      XPutPixel( img, xx, yy, pixel );				\
   }								\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, PF_8A8B8G8R triangle.
 */
static void flat_8A8B8G8R_triangle( GLcontext *ctx, GLuint v0,
                        	    GLuint v1, GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE					\
   unsigned long p = PACK_8B8G8R( VB->Color[pv][0],	\
		 VB->Color[pv][1], VB->Color[pv][2] );
#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint xx;						\
   PIXEL_TYPE *pixel = pRow;				\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {		\
      *pixel = p;					\
   }							\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, PF_8R8G8B triangle.
 */
static void flat_8R8G8B_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                  GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE					\
   unsigned long p = PACK_8R8G8B( VB->Color[pv][0],	\
		 VB->Color[pv][1], VB->Color[pv][2] );
#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint xx;						\
   PIXEL_TYPE *pixel = pRow;				\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {		\
      *pixel = p;					\
   }							\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, PF_5R6G5B triangle.
 */
static void flat_5R6G5B_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                  GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define PIXEL_ADDRESS(X,Y) PIXELADDR2(X,Y)
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE					\
   unsigned long p = PACK_5R6G5B( VB->Color[pv][0],	\
		 VB->Color[pv][1], VB->Color[pv][2] );
#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint xx;						\
   PIXEL_TYPE *pixel = pRow;				\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {		\
      *pixel = p;					\
   }							\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, 8-bit PF_DITHER triangle.
 */
static void flat_DITHER8_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                   GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE	\
   FLAT_DITHER_SETUP( VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2] );

#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint xx;						\
   PIXEL_TYPE *pixel = pRow;				\
   FLAT_DITHER_ROW_SETUP(FLIP(Y));			\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {		\
      *pixel = FLAT_DITHER(xx);				\
   }							\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, PF_DITHER triangle.
 */
static void flat_DITHER_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                  GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
#define SETUP_CODE	\
   FLAT_DITHER_SETUP( VB->Color[pv][0], VB->Color[pv][1], VB->Color[pv][2] );

#define INNER_LOOP( LEFT, RIGHT, Y )			\
{							\
   GLint xx, yy = FLIP(Y);				\
   FLAT_DITHER_ROW_SETUP(yy);				\
   for (xx=LEFT;xx<RIGHT;xx++) {			\
      unsigned long p = FLAT_DITHER(xx);		\
      XPutPixel( img, xx, yy, p );			\
   }							\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, 8-bit PF_HPCR triangle.
 */
static void flat_HPCR_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE				\
   GLubyte r = VB->Color[pv][0];		\
   GLubyte g = VB->Color[pv][1];		\
   GLubyte b = VB->Color[pv][2];
#define INNER_LOOP( LEFT, RIGHT, Y )		\
{						\
   GLint xx, yy = FLIP(Y);			\
   PIXEL_TYPE *pixel = pRow;			\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {	\
      *pixel = DITHER_HPCR( xx, yy, r, g, b );	\
   }						\
}
#include "tritemp.h"
}


/*
 * XImage, flat, NON-depth-buffered, 8-bit PF_LOOKUP triangle.
 */
static void flat_LOOKUP8_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                        	   GLuint v2, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define SETUP_CODE				\
   LOOKUP_SETUP;				\
   GLubyte r = VB->Color[pv][0];		\
   GLubyte g = VB->Color[pv][1];		\
   GLubyte b = VB->Color[pv][2];		\
   GLubyte p = LOOKUP(r,g,b);
#define INNER_LOOP( LEFT, RIGHT, Y )		\
{						\
   GLint xx;					\
   PIXEL_TYPE *pixel = pRow;			\
   for (xx=LEFT;xx<RIGHT;xx++,pixel++) {	\
      *pixel = p;				\
   }						\
}
#include "tritemp.h"
}




/*
 * This function is called if we're about to render triangles into an
 * X window/pixmap.  It sets the polygon stipple pattern if enabled.
 */
static void setup_x_polygon_options( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int fill_type;

   if (ctx->Polygon.StippleFlag) {
      if (xmesa->xm_buffer->stipple_pixmap == 0) {
         /* Allocate polygon stippling stuff once for this context. */
         XGCValues values;
         XMesaBuffer b = xmesa->xm_buffer;
         b->stipple_pixmap = XCreatePixmap( xmesa->display,
                                            b->buffer, 32, 32, 1 );
         values.function = GXcopy;
         values.foreground = 1;
         values.background = 0;
         b->stipple_gc = XCreateGC( xmesa->display, b->stipple_pixmap,
                                    (GCFunction|GCForeground|GCBackground),
                                    &values );
      }

      /*
       * NOTE: We don't handle the following here!
       *    GL_UNPACK_SWAP_BYTES
       *    GL_UNPACK_LSB_FIRST
       */
      /* Copy Mesa stipple pattern to an XImage then to Pixmap */
      {
         XImage *stipple_ximage;
         GLuint stipple[32];
         int i;
         int shift = xmesa->xm_buffer->height % 32;
         for (i=0;i<32;i++) {
            stipple[31-i] = ctx->PolygonStipple[(i+shift) % 32];
         }
         stipple_ximage = XCreateImage( xmesa->display,
                                        xmesa->xm_visual->visinfo->visual,
                                        1, ZPixmap, 0,
                                        (char *)stipple,
                                        32, 32, 8, 0 );
         stipple_ximage->byte_order = LSBFirst;
         stipple_ximage->bitmap_bit_order = LSBFirst;
         stipple_ximage->bitmap_unit = 32;
         XPutImage( xmesa->display,
                    xmesa->xm_buffer->stipple_pixmap,
                    xmesa->xm_buffer->stipple_gc,
                    stipple_ximage, 0, 0, 0, 0, 32, 32 );
         stipple_ximage->data = NULL;
         XDestroyImage( stipple_ximage );
      }

      XSetStipple( xmesa->display, xmesa->xm_buffer->gc1,
                   xmesa->xm_buffer->stipple_pixmap );
      XSetStipple( xmesa->display, xmesa->xm_buffer->gc2,
                   xmesa->xm_buffer->stipple_pixmap );
      fill_type = FillStippled;
   }
   else {
      fill_type = FillSolid;
   }

   XSetFillStyle( xmesa->display, xmesa->xm_buffer->gc1, fill_type );
   XSetFillStyle( xmesa->display, xmesa->xm_buffer->gc2, fill_type );
}



triangle_func xmesa_get_triangle_func( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int depth = xmesa->xm_visual->visinfo->depth;

   if (ctx->Polygon.SmoothFlag)     return NULL;
   if (ctx->Texture.Enabled)        return NULL;

   if (xmesa->xm_buffer->buffer==XIMAGE) {
      if (   ctx->Light.ShadeModel==GL_SMOOTH
          && ctx->RasterMask==DEPTH_BIT
          && ctx->Depth.Func==GL_LESS
          && ctx->Depth.Mask==GL_TRUE
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return smooth_TRUECOLOR_z_triangle;
            case PF_8A8B8G8R:
               return smooth_8A8B8G8R_z_triangle;
            case PF_8R8G8B:
               return smooth_8R8G8B_z_triangle;
            case PF_5R6G5B:
               return smooth_5R6G5B_z_triangle;
            case PF_HPCR:
	       return smooth_HPCR_z_triangle;
            case PF_DITHER:
               return (depth==8) ? smooth_DITHER8_z_triangle
                                        : smooth_DITHER_z_triangle;
            case PF_LOOKUP:
               return (depth==8) ? smooth_LOOKUP8_z_triangle : NULL;
            default:
               return NULL;
         }
      }
      if (   ctx->Light.ShadeModel==GL_FLAT
          && ctx->RasterMask==DEPTH_BIT
          && ctx->Depth.Func==GL_LESS
          && ctx->Depth.Mask==GL_TRUE
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return flat_TRUECOLOR_z_triangle;
            case PF_8A8B8G8R:
               return flat_8A8B8G8R_z_triangle;
            case PF_8R8G8B:
               return flat_8R8G8B_z_triangle;
            case PF_5R6G5B:
               return flat_5R6G5B_z_triangle;
            case PF_HPCR:
	       return flat_HPCR_z_triangle;
            case PF_DITHER:
               return (depth==8) ? flat_DITHER8_z_triangle
                                        : flat_DITHER_z_triangle;
            case PF_LOOKUP:
               return (depth==8) ? flat_LOOKUP8_z_triangle : NULL;
            default:
               return NULL;
         }
      }
      if (   ctx->RasterMask==0   /* no depth test */
          && ctx->Light.ShadeModel==GL_SMOOTH
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return smooth_TRUECOLOR_triangle;
            case PF_8A8B8G8R:
               return smooth_8A8B8G8R_triangle;
            case PF_8R8G8B:
               return smooth_8R8G8B_triangle;
            case PF_5R6G5B:
               return smooth_5R6G5B_triangle;
            case PF_HPCR:
	       return smooth_HPCR_triangle;
            case PF_DITHER:
               return (depth==8) ? smooth_DITHER8_triangle
                                        : smooth_DITHER_triangle;
            case PF_LOOKUP:
               return (depth==8) ? smooth_LOOKUP8_triangle : NULL;
            default:
               return NULL;
         }
      }

      if (   ctx->RasterMask==0   /* no depth test */
          && ctx->Light.ShadeModel==GL_FLAT
          && ctx->Polygon.StippleFlag==GL_FALSE) {
         switch (xmesa->pixelformat) {
            case PF_TRUECOLOR:
	       return flat_TRUECOLOR_triangle;
            case PF_8A8B8G8R:
               return flat_8A8B8G8R_triangle;
            case PF_8R8G8B:
               return flat_8R8G8B_triangle;
            case PF_5R6G5B:
               return flat_5R6G5B_triangle;
            case PF_HPCR:
	       return flat_HPCR_triangle;
            case PF_DITHER:
               return (depth==8) ? flat_DITHER8_triangle
                                        : flat_DITHER_triangle;
            case PF_LOOKUP:
               return (depth==8) ? flat_LOOKUP8_triangle : NULL;
            default:
               return NULL;
         }
      }

      return NULL;
   }
   else {
      /* pixmap */
      if (ctx->Light.ShadeModel==GL_FLAT && ctx->RasterMask==0) {
         setup_x_polygon_options( ctx );
         return flat_pixmap_triangle;
      }
      return NULL;
   }
}
