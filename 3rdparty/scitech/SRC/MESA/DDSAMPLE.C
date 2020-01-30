/* $Id: ddsample.c,v 1.5 1997/06/04 00:30:40 brianp Exp $ */

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
 * $Log: ddsample.c,v $
 * Revision 1.5  1997/06/04 00:30:40  brianp
 * replaced VB->Unclipped with VB->ClipMask
 *
 * Revision 1.4  1997/05/26 21:15:37  brianp
 * now pass red/green/blue/alpha bits to gl_create_visual()
 *
 * Revision 1.3  1996/11/13 03:52:12  brianp
 * updated comments
 *
 * Revision 1.2  1996/09/15 14:30:27  brianp
 * updated for new GLframebuffer and GLvisual datatypes and functions
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */



/*
 * This is a sample template for writing new Mesa device drivers.
 * You'll have to rewrite all the pseudo code below.
 *
 * Let's say you're interfacing Mesa to a window/operating system
 * called FOO.  Replace all occurances of FOOMesa with the real name
 * you select for your interface  (i.e. XMesa, WMesa, AmigaMesa).
 *
 * You'll have to design an API for clients to use, defined in a
 * header called Mesa/include/GL/FooMesa.h  Use the sample as an
 * example.  The API should at least have functions for creating
 * rendering contexts, binding rendering contexts to windows/frame
 * buffers, etc.
 * 
 * Next, you'll have to write implementations for the device driver
 * functions described in dd.h
 *
 * Note that you'll usually have to flip Y coordinates since Mesa's
 * window coordinates start at the bottom and increase upward.  Most
 * window system's Y-axis increases downward
 *
 * Functions marked OPTIONAL may be completely omitted by your driver.
 *
 * Your Makefile should compile this module along with the rest of
 * the core Mesa library.
 */


#include <stdlib.h>
#include "GL/FooMesa.h"
#include "context.h"
#include "types.h"
#include "vb.h"



/*
 * This struct describes the attributes (number of channels, video mode,
 * bits per pixel, etc) for a drawable (window, screen, or frame buffer).
 */
struct foo_mesa_visual {
   GLvisual *gl_visual;
   GLboolean db_flag;		/* double buffered? */
   GLboolean rgb_flag;		/* RGB mode? */
   GLuint depth;		/* bits per pixel (1, 8, 24, etc) */
};


/*
 * This struct is a wrapper for your system's notion of a window or
 * frame buffer.  It's needed because most window systems don't have
 * any notion of the Z buffers, stencil buffers, etc needed by Mesa.
 */
struct foo_mesa_buffer {
   GLframebuffer *gl_buffer;	/* The depth, stencil, accum, etc buffers */
   WindowType *the_window;	/* your window handle, etc */
};



/*
 * This struct contains all device-driver state information.  Think of it
 * as an extension of the core GLcontext from types.h.
 */
struct foo_mesa_context {
   GLcontext *gl_ctx;		/* the core library context */
   FooMesaVisual visual;
   FooMesaBuffer buffer;
   unsigned long pixel;		/* current color index or RGBA pixel value */
   unsigned long clearpixel;	/* pixel for clearing the color buffers */
   /* etc... */
};




/**********************************************************************/
/*****              Miscellaneous device driver funcs             *****/
/**********************************************************************/


static void finish( GLcontext *ctx )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* OPTIONAL FUNCTION: implements glFinish if possible */
}



static void flush( GLcontext *ctx )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* OPTIONAL FUNCTION: implements glFlush if possible */
}



static void clear_index( GLcontext *ctx, GLuint index )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* implement glClearIndex */
   /* usually just save the color index value in the foo struct */
}



static void clear_color( GLcontext *ctx, GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* implement glClearColor */
   /* color components are floats in [0,1] */
   /* usually just save the value in the foo struct */
}



static void clear( GLcontext *ctx,
		   GLboolean all, GLint x, GLint y, GLint width, GLint height )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
/*
 * Clear the specified region of the current color buffer using the clear
 * color or index as specified by one of the two functions above.
 * If all==GL_TRUE, clear whole buffer, else just clear region defined
 * by x,y,width,height
 */
}



static void set_index( GLcontext *ctx, GLuint index )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* Set the current color index. */
}



static void set_color( GLcontext *ctx, GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* Set the current RGBA color. */
   /* r is in [0,ctx->Visual->RedScale]   */
   /* g is in [0,ctx->Visual->GreenScale] */
   /* b is in [0,ctx->Visual->BlueScale]  */
   /* a is in [0,ctx->Visual->AlphaScale] */
}



static GLboolean index_mask( GLcontext *ctx, GLuint mask )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* OPTIONAL FUNCTION: implement glIndexMask if possible, else
    * return GL_FALSE
    */
}



static GLboolean color_mask( GLcontext *ctx, 
                             GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask)
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* OPTIONAL FUNCTION: implement glColorMask if possible, else
    * return GL_FALSE
    */
}



static GLboolean logicop( GLcontext *ctx, GLenum op )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /*
    * OPTIONAL FUNCTION: 
    * Implements glLogicOp if possible.  Return GL_TRUE if the device driver
    * can perform the operation, otherwise return GL_FALSE.  If GL_FALSE
    * is returned, the logic op will be done in software by Mesa.
    */
}


static void dither( GLcontext *ctx, GLboolean enable )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* OPTIONAL FUNCTION: enable/disable dithering if applicable */
}



static GLboolean set_buffer( GLcontext *ctx, GLenum mode )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* set the current drawing/reading buffer, return GL_TRUE or GL_FALSE */
   /* for success/failure */
   setup_DD_pointers( ctx );
}



static void get_buffer_size( GLcontext *ctx, GLuint *width, GLuint *height )
{
   struct foo_mesa_context *foo = (struct foo_mesa_context *) ctx->DriverCtx;
   /* return the width and height of the current buffer */
   /* if anything special has to been done when the buffer/window is */
   /* resized, do it now */
}



/**********************************************************************/
/*****           Accelerated point, line, triangle rendering      *****/
/**********************************************************************/


/* There may several functions like this, for different screen modes, etc */
static void fast_points_function( GLcontext *ctx, GLuint first, GLuint last )
{
   struct vertex_buffer *VB = ctx->VB;

   /* Render a number of points by some hardware/OS accerated method */
   if (VB->MonoColor) {
      /* draw all points using the current color (set_color) */
      for (i=first;i<=last;i++) {
         if (VB->ClipMask[i]==0) {
            /* compute window coordinate */
            int x, y;
            x =       (GLint) VB->Win[i][0];
            y = FLIP( (GLint) VB->Win[i][1] );
            PLOT_PIXEL( x, y, currentcolor );
         }
      }
   }
   else {
      /* each point is a different color */
      for (i=first;i<=last;i++) {
         if (VB->ClipMask[i]==0) {
            int x, y;
            x =       (GLint) VB->Win[i][0];
            y = FLIP( (GLint) VB->Win[i][1] );
            PLOT_PIXEL( x, y, VB->Color[i] );
         }
      }
   }
}




/* There may several functions like this, for different screen modes, etc */
static void fast_line_function( GLcontext *ctx, GLuint v0, GLuint v1, GLuint pv )
{
   /* Render a line by some hardware/OS accerated method */
   struct vertex_buffer *VB = ctx->VB;
   int x0, y0, x1, y1;
   unsigned long pixel;

   if (VB->MonoColor) {
      pixel = current color;
   }
   else {
      pixel = VB->Color[pv];
   }

   x0 =       (int) VB->Win[v0][0];
   y0 = FLIP( (int) VB->Win[v0][1] );
   x1 =       (int) VB->Win[v1][0];
   y1 = FLIP( (int) VB->Win[v1][1] );

   DRAW_LINE( x0,y0, x1,y1, pixel );
}




/* There may several functions like this, for different screen modes, etc */
static void fast_triangle_function( GLcontext *ctx, GLuint v0,
                                    GLuint v1, GLuint v2, GLuint pv )
{
   /* Render a triangle by some hardware/OS accerated method */
   struct vertex_buffer *VB = ctx->VB;
   int i;

   if (VB->MonoColor) {
      pixel = current color or index;
   }
   else {
      pixel = VB->Color[pv] or VB->Index[pv];
   }

   DRAW_TRIANGLE( v0, v1, v2 );
}






/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/


static void write_index_span( GLcontext *ctx,
                              GLuint n, GLint x, GLint y,
                              const GLuint index[],
                              const GLubyte mask[] )
{
   int i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         /* draw pixel (x[i],y[i]) using index[i] */
      }
   }
}



static void write_monoindex_span( GLcontext *ctx,
                                  GLuint n,GLint x,GLint y,const GLubyte mask[] )
{
   int i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         /* draw pixel (x[i],y[i]) using current color index */
      }
   }
}



static void write_color_span( GLcontext *ctx, 
                              GLuint n, GLint x, GLint y,
                              const GLubyte red[], const GLubyte green[],
                              const GLubyte blue[], const GLubyte alpha[],
                              const GLubyte mask[] )
{
   int i;
   y=FLIP(y);
   if (mask) {
      /* draw some pixels */
      for (i=0; i<n; i++, x++) {
         if (mask[i]) {
            /* draw pixel x,y using color red[i]/green[i]/blue[i]/alpha[i] */
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++, x++) {
         /* draw pixel x,y using color red[i]/green[i]/blue[i]/alpha[i] */
      }
   }
}



static void write_monocolor_span( GLcontext *ctx,
                                  GLuint n, GLint x, GLint y,
                                  const GLubyte mask[])
{
   int i;
   y=FLIP(y);
   for (i=0; i<n; i++, x++) {
      if (mask[i]) {
         plot pixel (x,y) using current color
      }
   }
}



/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/


static void read_index_span( GLcontext *ctx,
                             GLuint n, GLint x, GLint y, GLuint index[])
{
   int i;
   for (i=0; i<n; i++) {
      index[i] = read_pixel(x[i],y[i]);
   }
}



static void read_color_span( GLcontext *ctx,
                             GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
   int i;
   for (i=0; i<n; i++, x++) {
      red[i] = read_red( x, y );
      green[i] = read_green( x, y );
      /* etc */
   }
}



/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/


static void write_index_pixels( GLcontext *ctx,
                                GLuint n, const GLint x[], const GLint y[],
                                const GLuint index[], const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         plot pixel x[i], y[i] using index[i]
      }
   }
}



static void write_monoindex_pixels( GLcontext *ctx,
                                    GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         write pixel x[i], y[i] using current index
      }
   }
}



static void write_color_pixels( GLcontext *ctx,
                                GLuint n, const GLint x[], const GLint y[],
                                const GLubyte r[], const GLubyte g[],
                                const GLubyte b[], const GLubyte a[],
                                const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         write pixel x[i], y[i] using red[i],green[i],blue[i],alpha[i]
      }
   }
}



static void write_monocolor_pixels( GLcontext *ctx,
                                    GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         write pixel x[i], y[i] using current color
      }
   }
}




/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

/* Read an array of color index pixels. */
static void read_index_pixels( GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
                               GLuint indx[], const GLubyte mask[] )
{
  int i;
  for (i=0; i<n; i++) {
     if (mask[i]) {
        index[i] = read_pixel( x[i], y[i] );
     }
  }
}



static void read_color_pixels( GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   int i;
   for (i=0; i<n; i++) {
      if (mask[i]) {
         red[i] = read_red( x[i], y[i] );
         green[i] = read_green( x[i], y[i] );
         /* etc */
      }
   }
}




/**********************************************************************/
/**********************************************************************/


static void setup_DD_pointers( GLcontext *ctx )
{
   /* Initialize all the pointers in the DD struct.  Do this whenever */
   /* a new context is made current or we change buffers via set_buffer! */

   ctx->Driver.UpdateState = setup_DD_pointers;

   ctx->Driver.ClearIndex = clear_index;
   ctx->Driver.ClearColor = clear_color;
   ctx->Driver.Clear = clear;

   ctx->Driver.Index = set_index;
   ctx->Driver.Color = set_color;

   ctx->Driver.SetBuffer = set_buffer;
   ctx->Driver.GetBufferSize = get_buffer_size;

   ctx->Driver.PointsFunc = fast_points_function;
   ctx->Driver.LineFunc = fast_line_function;
   ctx->Driver.TriangleFunc = fast_triangle_function;

   /* Pixel/span writing functions: */
   ctx->Driver.WriteColorSpan       = write_color_span;
   ctx->Driver.WriteMonocolorSpan   = write_monocolor_span;
   ctx->Driver.WriteColorPixels     = write_color_pixels;
   ctx->Driver.WriteMonocolorPixels = write_monocolor_pixels;
   ctx->Driver.WriteIndexSpan       = write_index_span;
   ctx->Driver.WriteMonoindexSpan   = write_monoindex_span;
   ctx->Driver.WriteIndexPixels     = write_index_pixels;
   ctx->Driver.WriteMonoindexPixels = write_monoindex_pixels;

   /* Pixel/span reading functions: */
   ctx->Driver.ReadIndexSpan = read_index_span;
   ctx->Driver.ReadColorSpan = read_color_span;
   ctx->Driver.ReadIndexPixels = read_index_pixels;
   ctx->Driver.ReadColorPixels = read_color_pixels;


   /*
    * OPTIONAL FUNCTIONS:  these may be left uninitialized if the device
    * driver can't/needn't implement them.
    */
#if 0
   ctx->Driver.Finish = finish;
   ctx->Driver.Flush = flush;
   ctx->Driver.IndexMask = index_mask;
   ctx->Driver.ColorMask = color_mask;
   ctx->Driver.LogicOp = logicop;
   ctx->Driver.Dither = dither;
#endif
}



/**********************************************************************/
/*****               FOO/Mesa API Functions                       *****/
/**********************************************************************/



/*
 * The exact arguments to this function will depend on your window system
 */
FooMesaVisual FooMesaCreateVisual( GLboolean rgb_mode, GLboolean db_flag,
                                   /* etc... */ )
{
   FooMesaVisual v;
   GLfloat redscale, greenscale, bluescale, alphascale;

   v = (FooMesaVisual) calloc( 1, sizeof(struct foo_mesa_visual) );
   if (!v) {
      return NULL;
   }

   if (rgb_mode) {
      /* RGB(A) mode */
      redscale = ??;
      greedscale = ??;
      bluescale = ??;
      alphascale = ??;
      redbits = ??;
      greenbits = ??;
      bluebits = ??;
      alphabits = ??;
      index_bits = 0;
   }
   else {
      /* color index mode */
      redscale = 0.0;
      greedscale = 0.0;
      bluescale = 0.0;
      alphascale = 0.0;
      redbits = 0;
      greenbits = 0;
      bluebits = 0;
      alphabits = 0;
      index_bits = ??;
   }

   /* Create core visual */
   v->gl_visual = gl_create_visual( rgb_mode, 
                                    alpha_flag,
                                    db_flag,
                                    depth_size,
                                    stencil_size,
                                    accum_size,
                                    index_bits,
                                    redscale, greenscale,
                                    bluescale, alphascale,
                                    redbits, greenbits, bluebits, alphabits;

   return v;
}



void FooMesaDestroyVisual( FooMesaVisual v )
{
   gl_destroy_visual( v->gl_visual );
   free( v );
}




FooMesaBuffer FooMesaCreateBuffer( FooMesaVisual visual,
                                   int /* your window id */ )
{
   FooMesaBuffer b;

   b = (FooMesaBuffer) calloc( 1, sizeof(struct foo_mesa_buffer) );
   if (!b) {
      return NULL;
   }

   b->gl_buffer = gl_create_buffer( visual->gl_visual, windowid, 0, 0 );

   /* other stuff */

   return b;
}



void FooMesaDestroyBuffer( FooMesaBuffer b )
{
   gl_destroy_buffer( b->gl_buffer );
   free( b );
}




FooMesaContext FooMesaCreateContext( FooMesaVisual visual,
                                     FooMesaContext share )
{
   FooMesaContext c;

   c = (FooMesaContext) calloc( 1, sizeof(struct foo_mesa_context) );
   if (!c) {
      return NULL;
   }

   c->gl_ctx = gl_create_context( visual->gl_visual,
                                  share ? share->gl_ctx : NULL,
                                  (void *) c );


   /* you probably have to do a bunch of other initializations here. */

   return c;
}



void FooMesaDestroyContext( FooMesaContext c )
{
   gl_destroy_context( c->gl_ctx );
   free( c );
}



/*
 * Make the specified context the current one */
 * Might also want to specify the window/drawable here, like for GLX.
 */
void FooMesaMakeCurrent( FooMesaContext c, FooMesaBuffer b )
{
   if (c && b) {
      gl_make_current( c->gl_ctx, b->gl_buffer );
      Current_context = c;
      if (c->gl_ctx->Viewport.Width==0) {
         /* initialize viewport to window size */
         gl_Viewport( c->gl_ctx, 0, 0, c->width, c->height );
      }
   }
   else {
      /* Detach */
      gl_make_current( NULL, NULL );
      Current = NULL;
   }
}



void FooMesaSwapBuffers( FooMesaBuffer b )
{
   /* copy/swap back buffer to front if applicable */
}



/* you may need to add other FOO/Mesa functions too... */

