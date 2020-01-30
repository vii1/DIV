/* $Id: xmesa3.c,v 1.19 1998/01/16 03:57:40 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.6
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
 * $Log: xmesa3.c,v $
 * Revision 1.19  1998/01/16 03:57:40  brianp
 * added optimized blended line functions (Randy Frank)
 *
 * Revision 1.18  1997/06/20 02:56:52  brianp
 * changed color components from GLfixed to GLubyte
 *
 * Revision 1.17  1997/05/26 20:34:57  brianp
 * renamed PACK_RGB to PACK_TRUECOLOR
 *
 * Revision 1.16  1997/04/02 03:15:30  brianp
 * replaced VB->Unclipped with VB->ClipMask
 *
 * Revision 1.15  1997/03/16 02:16:32  brianp
 * moved PACK_8B8G8R macro to xmesaP.h
 *
 * Revision 1.14  1997/03/16 02:14:05  brianp
 * moved triangle code to xmesa4.c
 *
 * Revision 1.13  1997/03/16 02:08:31  brianp
 * now use linetemp.h in line drawing functions
 *
 * Revision 1.12  1997/01/31 23:45:19  brianp
 * faster flat-shaded dithered triangles from code by Martin Schenk (schenkm@ping.at)
 *
 * Revision 1.11  1996/11/30 15:13:26  brianp
 * added some parenthesis to WINCLIP macros
 *
 * Revision 1.10  1996/11/02 06:17:02  brianp
 * removed some unused local vars
 *
 * Revision 1.9  1996/10/22 02:59:12  brianp
 * incorporated Micheal Pichler's X line stipple patches
 *
 * Revision 1.8  1996/10/22 02:48:18  brianp
 * now use DITHER_SETUP and XDITHER macros
 * use array indexing instead of pointer dereferencing in inner loops
 *
 * Revision 1.7  1996/10/11 03:43:03  brianp
 * add LineZoffset factor to window Z coords
 *
 * Revision 1.6  1996/10/01 03:31:30  brianp
 * use new FixedToDepth() macro
 *
 * Revision 1.5  1996/09/27 01:31:54  brianp
 * removed unused variables
 *
 * Revision 1.4  1996/09/25 02:02:59  brianp
 * coordinates were incorrectly biased in flat_pixmap_triangle()
 *
 * Revision 1.3  1996/09/19 03:16:04  brianp
 * new X/Mesa interface with XMesaContext, XMesaVisual, and XMesaBuffer types
 *
 * Revision 1.2  1996/09/15 14:21:43  brianp
 * now use GLframebuffer and GLvisual
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


/*
 * Mesa/X11 interface, part 3.
 *
 * This file contains "accelerated" point, line, and triangle functions.
 * It should be fairly easy to write new special-purpose point, line or
 * triangle functions and hook them into this module.
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
/***                    Point rendering                             ***/
/**********************************************************************/


/*
 * Render an array of points into a pixmap, any pixel format.
 */
static void draw_points_ANY_pixmap( GLcontext *ctx, GLuint first, GLuint last )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   struct vertex_buffer *VB = ctx->VB;
   register GLuint i;

   if (VB->MonoColor) {
      /* all same color */
      XPoint p[VB_SIZE];
      int n = 0;
      for (i=first;i<=last;i++) {
         if (VB->ClipMask[i]==0) {
            p[n].x =       (GLint) VB->Win[i][0];
            p[n].y = FLIP( (GLint) VB->Win[i][1] );
            n++;
         }
      }
      XDrawPoints( dpy, buffer, xmesa->xm_buffer->gc1, p, n, CoordModeOrigin );
   }
   else {
      /* all different colors */
      if (xmesa->xm_visual->gl_visual->RGBAflag) {
         /* RGB mode */
         for (i=first;i<=last;i++) {
            if (VB->ClipMask[i]==0) {
               register int x, y;
               unsigned long pixel = xmesa_color_to_pixel( xmesa,
                                                VB->Color[i][0],
                                                VB->Color[i][1],
                                                VB->Color[i][2],
                                                VB->Color[i][3] );
               XSetForeground( dpy, gc, pixel );
               x =       (GLint) VB->Win[i][0];
               y = FLIP( (GLint) VB->Win[i][1] );
               XDrawPoint( dpy, buffer, gc, x, y);
            }
         }
      }
      else {
         /* Color index mode */
         for (i=first;i<=last;i++) {
            if (VB->ClipMask[i]==0) {
               register int x, y;
               XSetForeground( dpy, gc, VB->Index[i] );
               x =       (GLint) VB->Win[i][0];
               y = FLIP( (GLint) VB->Win[i][1] );
               XDrawPoint( dpy, buffer, gc, x, y);
            }
         }
      }
   }
}



/*
 * Analyze context state to see if we can provide a fast points drawing
 * function, like those in points.c.  Otherwise, return NULL.
 */
points_func xmesa_get_points_func( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;

   if (ctx->Point.Size==1.0F && !ctx->Point.SmoothFlag && ctx->RasterMask==0
       && !ctx->Texture.Enabled) {
      if (xmesa->xm_buffer->buffer==XIMAGE) {
         return NULL; /*draw_points_ximage;*/
      }
      else {
         return draw_points_ANY_pixmap;
      }
   }
   else {
      return NULL;
   }
}



/**********************************************************************/
/***                      Line rendering                            ***/
/**********************************************************************/

/*
 * Render a line into a pixmap, any pixel format.
 */
static void flat_pixmap_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   register int x0, y0, x1, y1;
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
   x0 =       (GLint) VB->Win[vert0][0];
   y0 = FLIP( (GLint) VB->Win[vert0][1] );
   x1 =       (GLint) VB->Win[vert1][0];
   y1 = FLIP( (GLint) VB->Win[vert1][1] );
   XDrawLine( xmesa->display, xmesa->xm_buffer->buffer, gc, x0, y0, x1, y1 );
}



/*
 * Draw a flat-shaded, PF_TRUECOLOR line into an XImage.
 */
static void flat_TRUECOLOR_line( GLcontext *ctx,
                                 GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   XImage *img = xmesa->xm_buffer->backimage;
   unsigned long pixel;
   PACK_TRUECOLOR( pixel, color[0], color[1], color[2] );

#define INTERP_XY 1
#define CLIP_HACK 1
#define PLOT(X,Y) XPutPixel( img, X, FLIP(Y), pixel );

#include "linetemp.h"
}



/*
 * Draw a flat-shaded, PF_8A8B8G8R line into an XImage.
 */
static void flat_8A8B8G8R_line( GLcontext *ctx,
                                GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLuint pixel = PACK_8B8G8R( color[0], color[1], color[2] );

#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = pixel;

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, PF_8R8G8B line into an XImage.
 */
static void flat_8R8G8B_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLuint pixel = PACK_8R8G8B( color[0], color[1], color[2] );

#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = pixel;

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, PF_5R6G5B line into an XImage.
 */
static void flat_5R6G5B_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLushort pixel = PACK_5R6G5B( color[0], color[1], color[2] );

#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = pixel;

#include "linetemp.h"
}



/*
 * Draw a flat-shaded, PF_DITHER 8-bit line into an XImage.
 */
static void flat_DITHER8_line( GLcontext *ctx,
                               GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLint r = color[0], g = color[1], b = color[2];
   DITHER_SETUP;

#define INTERP_XY 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = DITHER(X,Y,r,g,b);

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, PF_LOOKUP 8-bit line into an XImage.
 */
static void flat_LOOKUP8_line( GLcontext *ctx,
                               GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLubyte pixel;
   LOOKUP_SETUP;
   pixel = LOOKUP( color[0], color[1], color[2] );

#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = pixel;

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, PF_HPCR line into an XImage.
 */
static void flat_HPCR_line( GLcontext *ctx,
                            GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLint r = color[0], g = color[1], b = color[2];

#define INTERP_XY 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y) *pixelPtr = DITHER_HPCR(X,Y,r,g,b);

#include "linetemp.h"
}



/*
 * Draw a flat-shaded, Z-less, PF_TRUECOLOR line into an XImage.
 */
static void flat_TRUECOLOR_z_line( GLcontext *ctx,
                                   GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   XImage *img = xmesa->xm_buffer->backimage;
   unsigned long pixel;
   PACK_TRUECOLOR( pixel, color[0], color[1], color[2] );

#define INTERP_XY 1
#define INTERP_Z 1
#define CLIP_HACK 1
#define PLOT(X,Y)                               \
        if (Z < *zPtr) {                        \
           *zPtr = Z;                           \
           XPutPixel( img, X, FLIP(Y), pixel ); \
        }

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, Z-less, PF_8A8B8G8R line into an XImage.
 */
static void flat_8A8B8G8R_z_line( GLcontext *ctx,
                                  GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLuint pixel = PACK_8B8G8R( color[0], color[1], color[2] );

#define INTERP_Z 1
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)		\
	if (Z < *zPtr) {	\
	   *zPtr = Z;		\
	   *pixelPtr = pixel;	\
	}

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, Z-less, PF_8R8G8B line into an XImage.
 */
static void flat_8R8G8B_z_line( GLcontext *ctx,
                                GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLuint pixel = PACK_8R8G8B( color[0], color[1], color[2] );

#define INTERP_Z 1
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)		\
	if (Z < *zPtr) {	\
	   *zPtr = Z;		\
	   *pixelPtr = pixel;	\
	}

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, Z-less, PF_5R6G5B line into an XImage.
 */
static void flat_5R6G5B_z_line( GLcontext *ctx,
                                GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLushort pixel = PACK_5R6G5B( color[0], color[1], color[2] );

#define INTERP_Z 1
#define PIXEL_TYPE GLushort
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR2(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)		\
	if (Z < *zPtr) {	\
	   *zPtr = Z;		\
	   *pixelPtr = pixel;	\
	}
#include "linetemp.h"
}


/*
 * Draw a flat-shaded, Z-less, PF_DITHER 8-bit line into an XImage.
 */
static void flat_DITHER8_z_line( GLcontext *ctx,
                                 GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLint r = color[0], g = color[1], b = color[2];
   DITHER_SETUP;

#define INTERP_XY 1
#define INTERP_Z 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)				\
	if (Z < *zPtr) {			\
	   *zPtr = Z;				\
	   *pixelPtr = DITHER( X, Y, r, g, b);	\
	}
#include "linetemp.h"
}


/*
 * Draw a flat-shaded, Z-less, PF_LOOKUP 8-bit line into an XImage.
 */
static void flat_LOOKUP8_z_line( GLcontext *ctx,
                                 GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLubyte pixel;
   LOOKUP_SETUP;
   pixel = LOOKUP( color[0], color[1], color[2] );

#define INTERP_Z 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)		\
	if (Z < *zPtr) {	\
	   *zPtr = Z;		\
	   *pixelPtr = pixel;	\
	}

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, Z-less, PF_HPCR line into an XImage.
 */
static void flat_HPCR_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pv];
   GLint r = color[0], g = color[1], b = color[2];

#define INTERP_XY 1
#define INTERP_Z 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)					\
	if (Z < *zPtr) {				\
	   *zPtr = Z;					\
	   *pixelPtr = DITHER_HPCR( X, Y, r, g, b);	\
	}

#include "linetemp.h"
}


/*
 * Routines to draw flat shaded lines into an XImage buffer with Zbuffer
 * testing enabled (Z_LESS), Zbuffer writing disabled and 
 * SRC_ALPHA,1-SRC_ALPHA blending enabled.  While not heavily optimized,
 * they are much faster than the default rendering path.
 */
/*
 * Draw a flat-shaded, Z-less, alpha-blended, PF_TRUECOLOR line into an XImage.
 * No Zbuffer write.
 */
static void flat_blend_TRUECOLOR_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   unsigned long rMask = xmesa->xm_visual->visinfo->red_mask;
   unsigned long gMask = xmesa->xm_visual->visinfo->green_mask;
   unsigned long bMask = xmesa->xm_visual->visinfo->blue_mask;
   GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
   GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
   GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
   GLint rShift = xmesa->xm_visual->rshift;
   GLint gShift = xmesa->xm_visual->gshift;
   GLint bShift = xmesa->xm_visual->bshift;
   GLint avalue = ctx->VB->Color[pv][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = ctx->VB->Color[pv][0]*avalue;
   GLint gvalue = ctx->VB->Color[pv][1]*avalue;
   GLint bvalue = ctx->VB->Color[pv][2]*avalue;
   XImage *img = xmesa->xm_buffer->backimage;

#define INTERP_XY 1
#define INTERP_Z 1
#define CLIP_HACK 1
#define PLOT(X,Y)				 \
	if (Z < *zPtr) {			 \
   	   GLint rtmp, gtmp, btmp;		 \
   	   unsigned long pixel;			 \
	   pixel = XGetPixel( img, X, FLIP(Y) ); \
           rtmp = pixelToR[(pixel & rMask) >> rShift]; \
           gtmp = pixelToG[(pixel & gMask) >> gShift]; \
           btmp = pixelToB[(pixel & bMask) >> bShift]; \
     	   rtmp = ( (rtmp*msavalue + rvalue) >> 8); \
     	   gtmp = ( (gtmp*msavalue + gvalue) >> 8); \
     	   btmp = ( (btmp*msavalue + bvalue) >> 8); \
   	   PACK_TRUECOLOR( pixel, rtmp, gtmp, btmp ); \
           XPutPixel( img, X, FLIP(Y), pixel );	\
	}

#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, PF_8A8B8G8R line into an XImage.
 */
static void flat_blend_8A8B8G8R_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLint avalue = ctx->VB->Color[pv][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = ctx->VB->Color[pv][0]*avalue;
   GLint gvalue = ctx->VB->Color[pv][1]*avalue;
   GLint bvalue = ctx->VB->Color[pv][2]*avalue;

#define INTERP_Z 1
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)				 \
	if (Z < *zPtr) {			 \
   	   GLuint rtmp, gtmp, btmp;		 \
   	   GLuint pixel;			 \
	   pixel = *pixelPtr;                    \
           rtmp = ( pixel        & 0xff);        \
           gtmp = ((pixel >> 8)  & 0xff);        \
           btmp = ((pixel >> 16) & 0xff);        \
     	   rtmp = ( (rtmp*msavalue + rvalue) >> 8); \
     	   gtmp = ( (gtmp*msavalue + gvalue) >> 8); \
     	   btmp = ( (btmp*msavalue + bvalue) >> 8); \
   	   *pixelPtr = PACK_8B8G8R( rtmp, gtmp, btmp ); \
	}

#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, PF_8R8G8B line into an XImage.
 */
static void flat_blend_8R8G8B_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLint avalue = ctx->VB->Color[pv][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = ctx->VB->Color[pv][0]*avalue;
   GLint gvalue = ctx->VB->Color[pv][1]*avalue;
   GLint bvalue = ctx->VB->Color[pv][2]*avalue;

#define INTERP_Z 1
#define PIXEL_TYPE GLuint
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR4(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)				 \
	if (Z < *zPtr) {			 \
   	   GLuint rtmp, gtmp, btmp;		 \
   	   GLuint pixel;			 \
	   pixel = *pixelPtr;                    \
           rtmp = ((pixel >> 16) & 0xff);        \
           gtmp = ((pixel >> 8)  & 0xff);        \
           btmp = ( pixel        & 0xff);        \
     	   rtmp = ( (rtmp*msavalue + rvalue) >> 8); \
     	   gtmp = ( (gtmp*msavalue + gvalue) >> 8); \
     	   btmp = ( (btmp*msavalue + bvalue) >> 8); \
   	   *pixelPtr = PACK_8R8G8B( rtmp, gtmp, btmp ); \
	}

#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, PF_5R6G5B line into an XImage.
 */
static void flat_blend_5R6G5B_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   unsigned long rMask = xmesa->xm_visual->visinfo->red_mask;
   unsigned long gMask = xmesa->xm_visual->visinfo->green_mask;
   unsigned long bMask = xmesa->xm_visual->visinfo->blue_mask;
   GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
   GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
   GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
   GLint rShift = xmesa->xm_visual->rshift;
   GLint gShift = xmesa->xm_visual->gshift;
   GLint bShift = xmesa->xm_visual->bshift;
   GLint avalue = ctx->VB->Color[pv][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = ctx->VB->Color[pv][0]*avalue;
   GLint gvalue = ctx->VB->Color[pv][1]*avalue;
   GLint bvalue = ctx->VB->Color[pv][2]*avalue;
   XImage *img = xmesa->xm_buffer->backimage;

#define INTERP_XY 1
#define INTERP_Z 1
#define CLIP_HACK 1
#define PLOT(X,Y)				 \
	if (Z < *zPtr) {			 \
   	   GLint rtmp, gtmp, btmp;		 \
   	   unsigned long pixel;			 \
	   pixel = XGetPixel( img, X, FLIP(Y) ); \
           rtmp = pixelToR[(pixel & rMask) >> rShift]; \
           gtmp = pixelToG[(pixel & gMask) >> gShift]; \
           btmp = pixelToB[(pixel & bMask) >> bShift]; \
     	   rtmp = ( (rtmp*msavalue + rvalue) >> 8); \
     	   gtmp = ( (gtmp*msavalue + gvalue) >> 8); \
     	   btmp = ( (btmp*msavalue + bvalue) >> 8); \
   	   pixel = PACK_5R6G5B( rtmp, gtmp, btmp ); \
           XPutPixel( img, X, FLIP(Y), pixel );	\
	}

#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, PF_DITHER8 line into an XImage.
 */
static void flat_blend_DITHER8_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLint avalue = ctx->VB->Color[pv][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = ctx->VB->Color[pv][0]*avalue;
   GLint gvalue = ctx->VB->Color[pv][1]*avalue;
   GLint bvalue = ctx->VB->Color[pv][2]*avalue;
   GLubyte *red_table	= xmesa->xm_buffer->pixel_to_r;
   GLubyte *green_table = xmesa->xm_buffer->pixel_to_g;
   GLubyte *blue_table  = xmesa->xm_buffer->pixel_to_b;
   XImage *img = xmesa->xm_buffer->backimage;
   DITHER_SETUP;

#define INTERP_Z 1
#define INTERP_XY 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)				 \
	if (Z < *zPtr) {			 \
   	   GLuint rtmp, gtmp, btmp;		 \
   	   GLubyte pixel;			 \
	   pixel = *pixelPtr;                    \
           rtmp = red_table[pixel];              \
           gtmp = green_table[pixel];            \
           btmp = blue_table[pixel];             \
     	   rtmp = ( (rtmp*msavalue + rvalue) >> 8); \
     	   gtmp = ( (gtmp*msavalue + gvalue) >> 8); \
     	   btmp = ( (btmp*msavalue + bvalue) >> 8); \
   	   *pixelPtr = DITHER( X, Y, rtmp, gtmp, btmp ); \
	}

#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, PF_LOOKUP8 line into an XImage.
 */
static void flat_blend_LOOKUP8_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLint avalue = ctx->VB->Color[pv][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = ctx->VB->Color[pv][0]*avalue;
   GLint gvalue = ctx->VB->Color[pv][1]*avalue;
   GLint bvalue = ctx->VB->Color[pv][2]*avalue;
   GLubyte *red_table	= xmesa->xm_buffer->pixel_to_r;
   GLubyte *green_table = xmesa->xm_buffer->pixel_to_g;
   GLubyte *blue_table  = xmesa->xm_buffer->pixel_to_b;
   XImage *img = xmesa->xm_buffer->backimage;
   LOOKUP_SETUP;

#define INTERP_Z 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)				 \
	if (Z < *zPtr) {			 \
   	   GLuint rtmp, gtmp, btmp;		 \
   	   GLubyte pixel;			 \
	   pixel = *pixelPtr;                    \
           rtmp = red_table[pixel];              \
           gtmp = green_table[pixel];            \
           btmp = blue_table[pixel];             \
     	   rtmp = ( (rtmp*msavalue + rvalue) >> 8); \
     	   gtmp = ( (gtmp*msavalue + gvalue) >> 8); \
     	   btmp = ( (btmp*msavalue + bvalue) >> 8); \
   	   *pixelPtr = LOOKUP( rtmp, gtmp, btmp ); \
	}

#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, PF_HPCR line into an XImage.
 */
static void flat_blend_HPCR_z_line( GLcontext *ctx,
                              GLuint vert0, GLuint vert1, GLuint pv )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLint avalue = ctx->VB->Color[pv][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = ctx->VB->Color[pv][0]*avalue;
   GLint gvalue = ctx->VB->Color[pv][1]*avalue;
   GLint bvalue = ctx->VB->Color[pv][2]*avalue;

#define INTERP_Z 1
#define INTERP_XY 1
#define PIXEL_TYPE GLubyte
#define BYTES_PER_ROW (xmesa->xm_buffer->backimage->bytes_per_line)
#define PIXEL_ADDRESS(X,Y) PIXELADDR1(X,Y)
#define CLIP_HACK 1
#define PLOT(X,Y)				 \
	if (Z < *zPtr) {			 \
   	   GLuint rtmp, gtmp, btmp;		 \
   	   GLuint pixel;			 \
	   pixel = *pixelPtr;                    \
           rtmp = (pixel & 0xE0);                \
           gtmp = (pixel & 0x1C) << 3;           \
           btmp = (pixel & 0x03) << 6;           \
     	   rtmp = ( (rtmp*msavalue + rvalue) >> 8); \
     	   gtmp = ( (gtmp*msavalue + gvalue) >> 8); \
     	   btmp = ( (btmp*msavalue + bvalue) >> 8); \
   	   *pixelPtr = DITHER_HPCR( X, Y, rtmp, gtmp, btmp ); \
	}

#include "linetemp.h"
}

/*
 * Examine ctx->Line attributes and set xmesa->xm_buffer->gc1
 * and xmesa->xm_buffer->gc2 appropriately.
 */
static void setup_x_line_options( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int i, state, state0, new_state, len, offs;
   int tbit;
   char *dptr;
   int n_segments = 0;
   char dashes[20];
   int line_width, line_style;

   /*** Line Stipple ***/
   if (ctx->Line.StippleFlag) {
      const int pattern = ctx->Line.StipplePattern;

      dptr = dashes;
      state0 = state = ((pattern & 1) != 0);

      /* Decompose the pattern */
      for (i=1,tbit=2,len=1;i<16;++i,tbit=(tbit<<1))
	{
	  new_state = ((tbit & pattern) != 0);
	  if (state != new_state)
	    {
	      *dptr++ = ctx->Line.StippleFactor * len;
	      len = 1;
	      state = new_state;
	    }
	  else
	    ++len;
	}
      *dptr = ctx->Line.StippleFactor * len;
      n_segments = 1 + (dptr - dashes);

      /* ensure an even no. of segments, or X may toggle on/off for consecutive patterns */
      /* if (n_segments & 1)  dashes [n_segments++] = 0;  value of 0 not allowed in dash list */

      /* Handle case where line style starts OFF */
      if (state0 == 0)
        offs = dashes[0];
      else
        offs = 0;

#if 0
fprintf (stderr, "input pattern: 0x%04x, offset %d, %d segments:", pattern, offs, n_segments);
for (i = 0;  i < n_segments;  i++)
fprintf (stderr, " %d", dashes[i]);
fprintf (stderr, "\n");
#endif

      XSetDashes( xmesa->display, xmesa->xm_buffer->gc1, offs, dashes, n_segments );
      XSetDashes( xmesa->display, xmesa->xm_buffer->gc2, offs, dashes, n_segments );

      line_style = LineOnOffDash;
   }
   else {
      line_style = LineSolid;
   }

   /*** Line Width ***/
   line_width = (int) (ctx->Line.Width+0.5F);
   if (line_width < 2) {
      /* Use fast lines when possible */
      line_width = 0;
   }

   /*** Set GC attributes ***/
   XSetLineAttributes( xmesa->display, xmesa->xm_buffer->gc1,
                       line_width, line_style, CapButt, JoinBevel);
   XSetLineAttributes( xmesa->display, xmesa->xm_buffer->gc2,
                       line_width, line_style, CapButt, JoinBevel);
   XSetFillStyle( xmesa->display, xmesa->xm_buffer->gc1, FillSolid );
   XSetFillStyle( xmesa->display, xmesa->xm_buffer->gc2, FillSolid );
}



/*
 * Analyze context state to see if we can provide a fast line drawing
 * function, like those in lines.c.  Otherwise, return NULL.
 */
line_func xmesa_get_line_func( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int depth = xmesa->xm_visual->visinfo->depth;

   if (ctx->Line.SmoothFlag)              return NULL;
   if (ctx->Texture.Enabled)              return NULL;
   if (ctx->Light.ShadeModel!=GL_FLAT)    return NULL;

   if (xmesa->xm_buffer->buffer==XIMAGE
       && ctx->RasterMask==DEPTH_BIT
       && ctx->Depth.Func==GL_LESS
       && ctx->Depth.Mask==GL_TRUE
       && ctx->Line.Width==1.0F
       && ctx->Line.StippleFlag==GL_FALSE) {
      switch (xmesa->pixelformat) {
         case PF_TRUECOLOR:
            return flat_TRUECOLOR_z_line;
         case PF_8A8B8G8R:
            return flat_8A8B8G8R_z_line;
         case PF_8R8G8B:
            return flat_8R8G8B_z_line;
         case PF_5R6G5B:
            return flat_5R6G5B_z_line;
         case PF_DITHER:
            return (depth==8) ? flat_DITHER8_z_line : NULL;
         case PF_LOOKUP:
            return (depth==8) ? flat_LOOKUP8_z_line : NULL;
         case PF_HPCR:
            return flat_HPCR_z_line;
         default:
            return NULL;
      }
   }
   if (xmesa->xm_buffer->buffer==XIMAGE
       && ctx->RasterMask==0
       && ctx->Line.Width==1.0F
       && ctx->Line.StippleFlag==GL_FALSE) {
      switch (xmesa->pixelformat) {
         case PF_TRUECOLOR:
            return flat_TRUECOLOR_line;
         case PF_8A8B8G8R:
            return flat_8A8B8G8R_line;
         case PF_8R8G8B:
            return flat_8R8G8B_line;
         case PF_5R6G5B:
            return flat_5R6G5B_line;
         case PF_DITHER:
            return (depth==8) ? flat_DITHER8_line : NULL;
         case PF_LOOKUP:
            return (depth==8) ? flat_LOOKUP8_line : NULL;
         case PF_HPCR:
            return flat_HPCR_line;
	 default:
	    return NULL;
      }
   }	
   if (xmesa->xm_buffer->buffer==XIMAGE
       && ctx->RasterMask==(DEPTH_BIT|BLEND_BIT)
       && ctx->Depth.Func==GL_LESS
       && ctx->Depth.Mask==GL_FALSE
       && ctx->Color.BlendSrc==GL_SRC_ALPHA
       && ctx->Color.BlendDst==GL_ONE_MINUS_SRC_ALPHA
       && ctx->Color.BlendEquation==GL_FUNC_ADD_EXT
       && ctx->Line.Width==1.0F
       && ctx->Line.StippleFlag==GL_FALSE) {
      switch (xmesa->pixelformat) {
         case PF_TRUECOLOR:
            return flat_blend_TRUECOLOR_z_line;
         case PF_8A8B8G8R:
            return flat_blend_8A8B8G8R_z_line;
         case PF_8R8G8B:
            return flat_blend_8R8G8B_z_line;
         case PF_5R6G5B:
            return flat_blend_5R6G5B_z_line;
         case PF_DITHER:
            return (depth==8) ? flat_blend_DITHER8_z_line : NULL;
         case PF_LOOKUP:
            return (depth==8) ? flat_blend_LOOKUP8_z_line : NULL;
         case PF_HPCR:
            return flat_blend_HPCR_z_line;
         default:
            return NULL;
      }
   }

   if (xmesa->xm_buffer->buffer!=XIMAGE && ctx->RasterMask==0) {
      setup_x_line_options( ctx );
      return flat_pixmap_line;
   }
   return NULL;
}
