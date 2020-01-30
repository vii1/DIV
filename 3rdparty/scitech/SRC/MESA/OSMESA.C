/* $Id: osmesa.c,v 1.20 1998/02/05 00:35:12 brianp Exp $ */

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
 * $Log: osmesa.c,v $
 * Revision 1.20  1998/02/05 00:35:12  brianp
 * added John Stone's initial thread modifications
 *
 * Revision 1.19  1997/07/24 01:24:11  brianp
 * changed precompiled header symbol from PCH to PC_HEADER
 *
 * Revision 1.18  1997/07/16 03:07:28  brianp
 * added optimized blended lines (Randy Frank)
 *
 * Revision 1.17  1997/07/11 23:08:15  brianp
 * added OSMesaGetDepthBuffer() function (Randy Frank)
 *
 * Revision 1.16  1997/06/20 02:48:54  brianp
 * changed color components from GLfixed to GLubyte
 * fixed bug involving rowlength in OSMesaMakeCurrent()
 *
 * Revision 1.15  1997/05/28 03:25:43  brianp
 * added precompiled header (PCH) support
 *
 * Revision 1.14  1997/05/26 21:15:37  brianp
 * now pass red/green/blue/alpha bits to gl_create_visual()
 *
 * Revision 1.13  1997/03/21 01:57:49  brianp
 * added RendererString() function
 *
 * Revision 1.12  1997/03/16 02:41:20  brianp
 * did some clean-up in osmesa_setup_DD_pointers()
 *
 * Revision 1.11  1997/03/16 02:37:05  brianp
 * changed line functions to use linetemp.h
 *
 * Revision 1.10  1997/03/06 01:10:29  brianp
 * added Randy Frank's optimized line drawing code
 *
 * Revision 1.9  1997/02/10 20:34:33  brianp
 * added OSMESA_RGB and OSMESA_BGR, per Randy Frank
 *
 * Revision 1.8  1996/10/25 00:09:45  brianp
 * pass DEPTH_BITS, STENCIL_BITS, and ACCUM_BITS to gl_create_visual()
 *
 * Revision 1.7  1996/10/01 03:30:48  brianp
 * use new FixedToDepth() macro
 *
 * Revision 1.6  1996/10/01 01:43:21  brianp
 * added extra braces to the INNER_LOOP triangle macros
 *
 * Revision 1.5  1996/09/27 01:32:37  brianp
 * removed unused variables
 *
 * Revision 1.4  1996/09/19 03:17:28  brianp
 * now just one parameter to gl_create_framebuffer()
 *
 * Revision 1.3  1996/09/15 14:28:16  brianp
 * now use GLframebuffer and GLvisual
 *
 * Revision 1.2  1996/09/14 20:20:11  brianp
 * misc bug fixes from Randy Frank
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */



/*
 * Off-Screen Mesa rendering / Rendering into client memory space
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include <stdlib.h>
#include <string.h>
#include "GL/osmesa.h"
#include "context.h"
#include "depth.h"
#include "macros.h"
#include "matrix.h"
#include "types.h"
#include "vb.h"
#endif


struct osmesa_context {
   GLcontext *gl_ctx;		/* The core GL/Mesa context */
   GLvisual *gl_visual;		/* Describes the buffers */
   GLframebuffer *gl_buffer;	/* Depth, stencil, accum, etc buffers */
   GLenum format;		/* either GL_RGBA or GL_COLOR_INDEX */
   void *buffer;		/* the image buffer */
   GLint width, height;		/* size of image buffer */
   GLuint pixel;		/* current color index or RGBA pixel value */
   GLuint clearpixel;		/* pixel for clearing the color buffer */
   GLint rowlength;		/* number of pixels per row */
   GLint userRowLength;		/* user-specified number of pixels per row */
   GLint rshift, gshift;	/* bit shifts for RGBA formats */
   GLint bshift, ashift;
   GLint rind, gind, bind;	/* index offsets for RGBA formats */
   void *rowaddr[MAX_HEIGHT];	/* address of first pixel in each image row */
   GLboolean yup;		/* TRUE  -> Y increases upward */
				/* FALSE -> Y increases downward */
};



#ifdef THREADS

#include "mthreads.h" /* Mesa platform independent threads interface */

static MesaTSD osmesa_ctx_tsd;

static void osmesa_ctx_thread_init() {
  MesaInitTSD(&osmesa_ctx_tsd);
}

static OSMesaContext osmesa_get_thread_context( void ) {
  return (OSMesaContext) MesaGetTSD(&osmesa_ctx_tsd);
}

static void osmesa_set_thread_context( OSMesaContext ctx ) {
  MesaSetTSD(&osmesa_ctx_tsd, ctx, osmesa_ctx_thread_init);
}


#else
   /* One current context for address space, all threads */
   static OSMesaContext Current = NULL;
#endif



/* A forward declaration: */
static void osmesa_setup_DD_pointers( GLcontext *ctx );



/**********************************************************************/
/*****                    Public Functions                        *****/
/**********************************************************************/


/*
 * Create an Off-Screen Mesa rendering context.  The only attribute needed is
 * an RGBA vs Color-Index mode flag.
 *
 * Input:  format - either GL_RGBA or GL_COLOR_INDEX
 *         sharelist - specifies another OSMesaContext with which to share
 *                     display lists.  NULL indicates no sharing.
 * Return:  an OSMesaContext or 0 if error
 */
OSMesaContext OSMesaCreateContext( GLenum format, OSMesaContext sharelist )
{
   OSMesaContext osmesa;
   GLfloat rscale, gscale, bscale, ascale;
   GLint rshift, gshift, bshift, ashift;
   GLint rind, gind, bind;
   GLint index_bits;
   GLboolean rgbmode;
   GLboolean swalpha;
   GLuint i4 = 1;
   GLubyte *i1 = (GLubyte *) &i4;
   GLint little_endian = *i1;

   swalpha = GL_FALSE;
   rind = gind = bind = 0;
   if (format==OSMESA_COLOR_INDEX) {
      rscale = gscale = bscale = ascale = 0.0;
      index_bits = 8;
      rshift = gshift = bshift = ashift = 0;
      rgbmode = GL_FALSE;
   }
   else if (format==OSMESA_RGBA) {
      rscale = gscale = bscale = ascale = 255.0;
      index_bits = 0;
      if (little_endian) {
         rshift = 0;
         gshift = 8;
         bshift = 16;
         ashift = 24;
      }
      else {
         rshift = 24;
         gshift = 16;
         bshift = 8;
         ashift = 0;
      }
      rgbmode = GL_TRUE;
   }
   else if (format==OSMESA_BGRA) {
      rscale = gscale = bscale = ascale = 255.0;
      index_bits = 0;
      if (little_endian) {
         ashift = 0;
         rshift = 8;
         gshift = 16;
         bshift = 24;
      }
      else {
         bshift = 24;
         gshift = 16;
         rshift = 8;
         ashift = 0;
      }
      rgbmode = GL_TRUE;
   }
   else if (format==OSMESA_ARGB) {
      rscale = gscale = bscale = ascale = 255.0;
      index_bits = 0;
      if (little_endian) {
         bshift = 0;
         gshift = 8;
         rshift = 16;
         ashift = 24;
      }
      else {
         ashift = 24;
         rshift = 16;
         gshift = 8;
         bshift = 0;
      }
      rgbmode = GL_TRUE;
   }
   else if (format==OSMESA_RGB) {
      rscale = gscale = bscale = ascale = 255.0;
      index_bits = 0;
      bshift = 0;
      gshift = 8;
      rshift = 16;
      ashift = 24;
      bind = 2;
      gind = 1;
      rind = 0;
      rgbmode = GL_TRUE;
      swalpha = GL_TRUE;
   }
   else if (format==OSMESA_BGR) {
      rscale = gscale = bscale = ascale = 255.0;
      index_bits = 0;
      bshift = 0;
      gshift = 8;
      rshift = 16;
      ashift = 24;
      bind = 0;
      gind = 1;
      rind = 2;
      rgbmode = GL_TRUE;
      swalpha = GL_TRUE;
   }
   else {
      return NULL;
   }


   osmesa = (OSMesaContext) calloc( 1, sizeof(struct osmesa_context) );
   if (osmesa) {
      osmesa->gl_visual = gl_create_visual( rgbmode,
					    swalpha,    /* software alpha */
                                            GL_FALSE,	/* db_flag */
                                            DEPTH_BITS,
                                            STENCIL_BITS,
                                            ACCUM_BITS,
                                            index_bits,
                                            rscale, gscale, bscale, ascale,
                                            8, 8, 8, 0 );
      if (!osmesa->gl_visual) {
         return NULL;
      }

      osmesa->gl_ctx = gl_create_context( osmesa->gl_visual,
                                          sharelist ? sharelist->gl_ctx : NULL,
                                          (void *) osmesa );
      if (!osmesa->gl_ctx) {
         gl_destroy_visual( osmesa->gl_visual );
         free(osmesa);
         return NULL;
      }
      osmesa->gl_buffer = gl_create_framebuffer( osmesa->gl_visual );
      if (!osmesa->gl_buffer) {
         gl_destroy_visual( osmesa->gl_visual );
         gl_destroy_context( osmesa->gl_ctx );
         free(osmesa);
         return NULL;
      }
      osmesa->format = format;
      osmesa->buffer = NULL;
      osmesa->width = 0;
      osmesa->height = 0;
      osmesa->pixel = 0;
      osmesa->clearpixel = 0;
      osmesa->userRowLength = 0;
      osmesa->rowlength = 0;
      osmesa->yup = GL_TRUE;
      osmesa->rshift = rshift;
      osmesa->gshift = gshift;
      osmesa->bshift = bshift;
      osmesa->ashift = ashift;
      osmesa->rind = rind;
      osmesa->gind = gind;
      osmesa->bind = bind;
   }
   return osmesa;
}



/*
 * Destroy an Off-Screen Mesa rendering context.
 *
 * Input:  ctx - the context to destroy
 */
void OSMesaDestroyContext( OSMesaContext ctx )
{
   if (ctx) {
      gl_destroy_visual( ctx->gl_visual );
      gl_destroy_framebuffer( ctx->gl_buffer );
      gl_destroy_context( ctx->gl_ctx );
      free( ctx );
   }
}



/*
 * Recompute the values of the context's rowaddr array.
 */
static void compute_row_addresses( OSMesaContext ctx )
{
   GLint i;

   if (ctx->yup) {
      /* Y=0 is bottom line of window */
      if (ctx->format==OSMESA_COLOR_INDEX) {
         /* 1-byte CI mode */
         GLubyte *origin = (GLubyte *) ctx->buffer;
         for (i=0;i<MAX_HEIGHT;i++) {
            ctx->rowaddr[i] = origin + i * ctx->rowlength;
         }
      }
      else {
         if ((ctx->format==OSMESA_RGB) || (ctx->format==OSMESA_BGR)) {
            /* 3-byte RGB mode */
            GLubyte *origin = (GLubyte *) ctx->buffer;
            for (i=0;i<MAX_HEIGHT;i++) {
               ctx->rowaddr[i] = origin + (i * (ctx->rowlength*3));
            }
         } else {
            /* 4-byte RGBA mode */
            GLuint *origin = (GLuint *) ctx->buffer;
            for (i=0;i<MAX_HEIGHT;i++) {
               ctx->rowaddr[i] = origin + i * ctx->rowlength;
            }
         }
      }
   }
   else {
      /* Y=0 is top line of window */
      if (ctx->format==OSMESA_COLOR_INDEX) {
         /* 1-byte CI mode */
         GLubyte *origin = (GLubyte *) ctx->buffer;
         for (i=0;i<MAX_HEIGHT;i++) {
            ctx->rowaddr[i] = origin + (ctx->height-i-1) * ctx->rowlength;
         }
      }
      else {
         if ((ctx->format==OSMESA_RGB) || (ctx->format==OSMESA_BGR)) {
            /* 3-byte RGB mode */
            GLubyte *origin = (GLubyte *) ctx->buffer;
            for (i=0;i<MAX_HEIGHT;i++) {
               ctx->rowaddr[i] = origin + ((ctx->height-i-1) * (ctx->rowlength*3));
            }
         } else {
            /* 4-byte RGBA mode */
            GLuint *origin = (GLuint *) ctx->buffer;
            for (i=0;i<MAX_HEIGHT;i++) {
               ctx->rowaddr[i] = origin + (ctx->height-i-1) * ctx->rowlength;
            }
         }
      }
   }
}


/*
 * Bind an OSMesaContext to an image buffer.  The image buffer is just a
 * block of memory which the client provides.  Its size must be at least
 * as large as width*height*sizeof(type).  Its address should be a multiple
 * of 4 if using RGBA mode.
 *
 * Image data is stored in the order of glDrawPixels:  row-major order
 * with the lower-left image pixel stored in the first array position
 * (ie. bottom-to-top).
 *
 * Since the only type initially supported is GL_UNSIGNED_BYTE, if the
 * context is in RGBA mode, each pixel will be stored as a 4-byte RGBA
 * value.  If the context is in color indexed mode, each pixel will be
 * stored as a 1-byte value.
 *
 * If the context's viewport hasn't been initialized yet, it will now be
 * initialized to (0,0,width,height).
 *
 * Input:  ctx - the rendering context
 *         buffer - the image buffer memory
 *         type - data type for pixel components, only GL_UNSIGNED_BYTE
 *                supported now
 *         width, height - size of image buffer in pixels, at least 1
 * Return:  GL_TRUE if success, GL_FALSE if error because of invalid ctx,
 *          invalid buffer address, type!=GL_UNSIGNED_BYTE, width<1, height<1,
 *          width>internal limit or height>internal limit.
 */
GLboolean OSMesaMakeCurrent( OSMesaContext ctx, void *buffer, GLenum type,
                             GLsizei width, GLsizei height )
{
   if (!ctx || !buffer || type!=GL_UNSIGNED_BYTE
       || width<1 || height<1 || width>MAX_WIDTH || height>MAX_HEIGHT) {
      return GL_FALSE;
   }

   gl_make_current( ctx->gl_ctx, ctx->gl_buffer );

   ctx->buffer = buffer;
   ctx->width = width;
   ctx->height = height;
   if (ctx->userRowLength)
      ctx->rowlength = ctx->userRowLength;
   else
      ctx->rowlength = width;

   osmesa_setup_DD_pointers( ctx->gl_ctx );

#ifdef THREADS
   /* Set current context for the calling thread */
   osmesa_set_thread_context(ctx);
#else
   /* Set current context for the address space, all threads */
   Current = ctx;
#endif

   compute_row_addresses( ctx );

   /* init viewport */
   if (ctx->gl_ctx->Viewport.Width==0) {
      /* initialize viewport and scissor box to buffer size */
      gl_Viewport( ctx->gl_ctx, 0, 0, width, height );
      ctx->gl_ctx->Scissor.Width = width;
      ctx->gl_ctx->Scissor.Height = height;
   }

   return GL_TRUE;
}




OSMesaContext OSMesaGetCurrentContext( void )
{
#ifdef THREADS
   /* Return current handle for the calling thread */
   return osmesa_get_thread_context();
#else
   /* Return current handle for the address space, all threads */
   return Current;
#endif
}



void OSMesaPixelStore( GLint pname, GLint value )
{
   OSMesaContext ctx = OSMesaGetCurrentContext();

   switch (pname) {
      case OSMESA_ROW_LENGTH:
         if (value<0) {
            gl_error( ctx->gl_ctx, GL_INVALID_VALUE,
                      "OSMesaPixelStore(value)" );
            return;
         }
         ctx->userRowLength = value;
         ctx->rowlength = value;
         break;
      case OSMESA_Y_UP:
         ctx->yup = value ? GL_TRUE : GL_FALSE;
         break;
      default:
         gl_error( ctx->gl_ctx, GL_INVALID_ENUM, "OSMesaPixelStore(pname)" );
         return;
   }

   compute_row_addresses( ctx );
}


void OSMesaGetIntegerv( GLint pname, GLint *value )
{
   OSMesaContext ctx = OSMesaGetCurrentContext();

   switch (pname) {
      case OSMESA_WIDTH:
         *value = ctx->width;
         return;
      case OSMESA_HEIGHT:
         *value = ctx->height;
         return;
      case OSMESA_FORMAT:
         *value = ctx->format;
         return;
      case OSMESA_TYPE:
         *value = GL_UNSIGNED_BYTE;
         return;
      case OSMESA_ROW_LENGTH:
         *value = ctx->rowlength;
         return;
      case OSMESA_Y_UP:
         *value = ctx->yup;
         return;
      default:
         gl_error( ctx->gl_ctx, GL_INVALID_ENUM, "OSMesaGetIntergerv(pname)" );
         return;
   }
}



/*
 * Return the depth buffer associated with an OSMesa context.
 * Input:  c - the OSMesa context
 * Output:  width, height - size of buffer in pixels
 *          bytesPerValue - bytes per depth value (2 or 4)
 *          buffer - pointer to depth buffer values
 * Return:  GL_TRUE or GL_FALSE to indicate success or failure.
 */
GLboolean OSMesaGetDepthBuffer( OSMesaContext c, GLint *width, GLint *height,
                                GLint *bytesPerValue, void **buffer )
{
   if ((!c->gl_buffer) || (!c->gl_buffer->Depth)) {
      *width = 0;
      *height = 0;
      *bytesPerValue = 0;
      *buffer = 0;
      return GL_FALSE;
   }
   else {
      *width = c->gl_buffer->Width;
      *height = c->gl_buffer->Height;
      *bytesPerValue = sizeof(GLdepth);
      *buffer = c->gl_buffer->Depth;
      return GL_TRUE;
   }
}




/**********************************************************************/
/*** Device Driver Functions                                        ***/
/**********************************************************************/


/*
 * Useful macros:
 */
#define PACK_RGBA(R,G,B,A)  (  ((R) << osmesa->rshift) \
                             | ((G) << osmesa->gshift) \
                             | ((B) << osmesa->bshift) \
                             | ((A) << osmesa->ashift) )

#define PACK_RGBA2(R,G,B,A)  (  ((R) << rshift) \
                              | ((G) << gshift) \
                              | ((B) << bshift) \
                              | ((A) << ashift) )

#define UNPACK_RED(P)      (((P) >> osmesa->rshift) & 0xff)
#define UNPACK_GREEN(P)    (((P) >> osmesa->gshift) & 0xff)
#define UNPACK_BLUE(P)     (((P) >> osmesa->bshift) & 0xff)
#define UNPACK_ALPHA(P)    (((P) >> osmesa->ashift) & 0xff)

#define PIXELADDR1(X,Y)  ((GLubyte *) osmesa->rowaddr[Y] + (X))
#define PIXELADDR3(X,Y)  ((GLubyte *) osmesa->rowaddr[Y] + ((X)*3))
#define PIXELADDR4(X,Y)  ((GLuint *)  osmesa->rowaddr[Y] + (X))




static GLboolean set_buffer( GLcontext *ctx, GLenum mode )
{
   if (mode==GL_FRONT) {
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}


static void clear_index( GLcontext *ctx, GLuint index )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   osmesa->clearpixel = index;
}



static void clear_color( GLcontext *ctx,
                         GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   osmesa->clearpixel = PACK_RGBA( r, g, b, a );
}



static void clear( GLcontext *ctx,
                   GLboolean all, GLint x, GLint y, GLint width, GLint height )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   if (osmesa->format==OSMESA_COLOR_INDEX) {
      if (all) {
         /* Clear whole CI buffer */
         MEMSET(osmesa->buffer, osmesa->clearpixel, osmesa->rowlength*osmesa->height);
      }
      else {
         /* Clear part of CI buffer */
         GLuint i, j;
         for (i=0;i<height;i++) {
            GLubyte *ptr1 = PIXELADDR1( x, (y+i) );
            for (j=0;j<width;j++) {
               *ptr1++ = osmesa->clearpixel;
            }
         }
      }
   }
   else if ((osmesa->format==OSMESA_RGB)||(osmesa->format==OSMESA_BGR)) {
      GLubyte rval = UNPACK_RED(osmesa->clearpixel);
      GLubyte gval = UNPACK_GREEN(osmesa->clearpixel);
      GLubyte bval = UNPACK_BLUE(osmesa->clearpixel);
      GLint   rind = osmesa->rind;
      GLint   gind = osmesa->gind;
      GLint   bind = osmesa->bind;
      if (all) {
         GLuint  i, n; 
         GLubyte *ptr3 = (GLubyte *) osmesa->buffer;
         /* Clear whole RGB buffer */
         n = osmesa->rowlength * osmesa->height;
         for (i=0;i<n;i++) {
            ptr3[rind] = rval;
            ptr3[gind] = gval;
            ptr3[bind] = bval;
            ptr3 += 3;
         }
      }
      else {
         /* Clear part of RGB buffer */
         GLuint i, j;
         for (i=0;i<height;i++) {
            GLubyte *ptr3 = PIXELADDR3( x, (y+i) );
            for (j=0;j<width;j++) {
               ptr3[rind] = rval;
               ptr3[gind] = gval;
               ptr3[bind] = bval;
               ptr3 += 3;
            }
         }
      }
   }
   else {
      if (all) {
         /* Clear whole RGBA buffer */
         GLuint i, n, *ptr4;
         n = osmesa->rowlength * osmesa->height;
         ptr4 = (GLuint *) osmesa->buffer;
         for (i=0;i<n;i++) {
            *ptr4++ = osmesa->clearpixel;
         }
      }
      else {
         /* Clear part of RGBA buffer */
         GLuint i, j;
         for (i=0;i<height;i++) {
            GLuint *ptr4 = PIXELADDR4( x, (y+i) );
            for (j=0;j<width;j++) {
               *ptr4++ = osmesa->clearpixel;
            }
         }
      }
   }
}



static void set_index( GLcontext *ctx, GLuint index )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   osmesa->pixel = index;
}



static void set_color( GLcontext *ctx,
                       GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   osmesa->pixel = PACK_RGBA( r, g, b, a );
}



static void buffer_size( GLcontext *ctx, GLuint *width, GLuint *height )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   *width = osmesa->width;
   *height = osmesa->height;
}


/**********************************************************************/
/*****        4 byte RGB and 1 byte CI pixel support funcs        *****/
/**********************************************************************/

static void write_color_span( GLcontext *ctx,
                              GLuint n, GLint x, GLint y,
                              const GLubyte red[], const GLubyte green[],
			      const GLubyte blue[], const GLubyte alpha[],
			      const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint *ptr4 = PIXELADDR4( x, y );
   GLuint i;
   GLint rshift = osmesa->rshift;
   GLint gshift = osmesa->gshift;
   GLint bshift = osmesa->bshift;
   GLint ashift = osmesa->ashift;
   if (mask) {
      for (i=0;i<n;i++,ptr4++) {
         if (mask[i]) {
            *ptr4 = PACK_RGBA2( red[i], green[i], blue[i], alpha[i] );
         }
      }
   }
   else {
      for (i=0;i<n;i++,ptr4++) {
         *ptr4 = PACK_RGBA2( red[i], green[i], blue[i], alpha[i] );
      }
   }
}



static void write_monocolor_span( GLcontext *ctx,
                                  GLuint n, GLint x, GLint y,
				  const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint *ptr4 = PIXELADDR4(x,y);
   GLuint i;
   for (i=0;i<n;i++,ptr4++) {
      if (mask[i]) {
         *ptr4 = osmesa->pixel;
      }
   }
}



static void write_color_pixels( GLcontext *ctx,
                                GLuint n, const GLint x[], const GLint y[],
                                const GLubyte red[], const GLubyte green[],
			        const GLubyte blue[], const GLubyte alpha[],
			        const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   GLint rshift = osmesa->rshift;
   GLint gshift = osmesa->gshift;
   GLint bshift = osmesa->bshift;
   GLint ashift = osmesa->ashift;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLuint *ptr4 = PIXELADDR4(x[i],y[i]);
         *ptr4 = PACK_RGBA2( red[i], green[i], blue[i], alpha[i] );
      }
   }
}



static void write_monocolor_pixels( GLcontext *ctx,
                                    GLuint n, const GLint x[], const GLint y[],
				    const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLuint *ptr4 = PIXELADDR4(x[i],y[i]);
         *ptr4 = osmesa->pixel;
      }
   }
}



static void write_index_span( GLcontext *ctx,
                              GLuint n, GLint x, GLint y, const GLuint index[],
			      const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLubyte *ptr1 = PIXELADDR1(x,y);
   GLuint i;
   for (i=0;i<n;i++,ptr1++) {
      if (mask[i]) {
         *ptr1 = (GLubyte) index[i];
      }
   }
}



static void write_monoindex_span( GLcontext *ctx,
                                  GLuint n, GLint x, GLint y,
				  const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLubyte *ptr1 = PIXELADDR1(x,y);
   GLuint i;
   for (i=0;i<n;i++,ptr1++) {
      if (mask[i]) {
         *ptr1 = (GLubyte) osmesa->pixel;
      }
   }
}



static void write_index_pixels( GLcontext *ctx,
                                GLuint n, const GLint x[], const GLint y[],
			        const GLuint index[], const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr1 = PIXELADDR1(x[i],y[i]);
         *ptr1 = (GLubyte) index[i];
      }
   }
}



static void write_monoindex_pixels( GLcontext *ctx,
                                    GLuint n, const GLint x[], const GLint y[],
				    const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr1 = PIXELADDR1(x[i],y[i]);
         *ptr1 = (GLubyte) osmesa->pixel;
      }
   }
}



static void read_index_span( GLcontext *ctx,
                             GLuint n, GLint x, GLint y, GLuint index[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   GLubyte *ptr1 = PIXELADDR1(x,y);
   for (i=0;i<n;i++,ptr1++) {
      index[i] = (GLuint) *ptr1;
   }
}


static void read_color_span( GLcontext *ctx,
                             GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
			     GLubyte blue[], GLubyte alpha[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   GLuint *ptr4 = PIXELADDR4(x,y);
   for (i=0;i<n;i++) {
      GLuint pixel = *ptr4++;
      red[i]   = UNPACK_RED(pixel);
      green[i] = UNPACK_GREEN(pixel);
      blue[i]  = UNPACK_BLUE(pixel);
      alpha[i] = UNPACK_ALPHA(pixel);
   }
}


static void read_index_pixels( GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
			       GLuint index[], const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i] ) {
         GLubyte *ptr1 = PIXELADDR1(x[i],y[i]);
         index[i] = (GLuint) *ptr1;
      }
   }
}


static void read_color_pixels( GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
			       GLubyte red[], GLubyte green[],
			       GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLuint *ptr4 = PIXELADDR4(x[i],y[i]);
         GLuint pixel = *ptr4;
         red[i]   = UNPACK_RED(pixel);
         green[i] = UNPACK_GREEN(pixel);
         blue[i]  = UNPACK_BLUE(pixel);
         alpha[i] = UNPACK_ALPHA(pixel);
      }
   }
}

/**********************************************************************/
/*****                3 byte RGB pixel support funcs              *****/
/**********************************************************************/

static void write_color_span3( GLcontext *ctx,
                              GLuint n, GLint x, GLint y,
                              const GLubyte red[], const GLubyte green[],
			      const GLubyte blue[], const GLubyte alpha[],
			      const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLubyte *ptr3 = PIXELADDR3( x, y);
   GLuint i;
   GLint rind = osmesa->rind;
   GLint gind = osmesa->gind;
   GLint bind = osmesa->bind;
   if (mask) {
      for (i=0;i<n;i++,ptr3+=3) {
         if (mask[i]) {
            ptr3[rind] = red[i];
            ptr3[gind] = green[i];
            ptr3[bind] = blue[i];
         }
      }
   }
   else {
      for (i=0;i<n;i++,ptr3+=3) {
         ptr3[rind] = red[i];
         ptr3[gind] = green[i];
         ptr3[bind] = blue[i];
      }
   }
}

static void write_monocolor_span3( GLcontext *ctx,
                                  GLuint n, GLint x, GLint y,
				  const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   
   GLubyte rval = UNPACK_RED(osmesa->pixel);
   GLubyte gval = UNPACK_GREEN(osmesa->pixel);
   GLubyte bval = UNPACK_BLUE(osmesa->pixel);
   GLint   rind = osmesa->rind;
   GLint   gind = osmesa->gind;
   GLint   bind = osmesa->bind;


   GLubyte *ptr3 = PIXELADDR3( x, y);
   GLuint i;
   for (i=0;i<n;i++,ptr3+=3) {
      if (mask[i]) {
         ptr3[rind] = rval;
         ptr3[gind] = gval;
         ptr3[bind] = bval;
      }
   }
}

static void write_color_pixels3( GLcontext *ctx,
                                GLuint n, const GLint x[], const GLint y[],
                                const GLubyte red[], const GLubyte green[],
			        const GLubyte blue[], const GLubyte alpha[],
			        const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   GLint rind = osmesa->rind;
   GLint gind = osmesa->gind;
   GLint bind = osmesa->bind;

   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr3 = PIXELADDR3(x[i],y[i]);
         ptr3[rind] = red[i];
         ptr3[gind] = green[i];
         ptr3[bind] = blue[i];
      }
   }
}

static void write_monocolor_pixels3( GLcontext *ctx,
                                    GLuint n, const GLint x[], const GLint y[],
				    const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   GLint rind = osmesa->rind;
   GLint gind = osmesa->gind;
   GLint bind = osmesa->bind;
   GLubyte rval = UNPACK_RED(osmesa->pixel);
   GLubyte gval = UNPACK_GREEN(osmesa->pixel);
   GLubyte bval = UNPACK_BLUE(osmesa->pixel);
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr3 = PIXELADDR3(x[i],y[i]);
         ptr3[rind] = rval;
         ptr3[gind] = gval;
         ptr3[bind] = bval;
      }
   }
}

static void read_color_span3( GLcontext *ctx,
                             GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
			     GLubyte blue[], GLubyte alpha[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   GLint rind = osmesa->rind;
   GLint gind = osmesa->gind;
   GLint bind = osmesa->bind;
   GLubyte *ptr3 = PIXELADDR3( x, y);
   for (i=0;i<n;i++,ptr3+=3) {
      red[i]   = ptr3[rind];
      green[i] = ptr3[gind];
      blue[i]  = ptr3[bind];
      alpha[i] = 0;
   }
}

static void read_color_pixels3( GLcontext *ctx,
                               GLuint n, const GLint x[], const GLint y[],
			       GLubyte red[], GLubyte green[],
			       GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLuint i;
   GLint rind = osmesa->rind;
   GLint gind = osmesa->gind;
   GLint bind = osmesa->bind;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr3 = PIXELADDR3(x[i],y[i]);
         red[i]   = ptr3[rind];
         green[i] = ptr3[gind];
         blue[i]  = ptr3[bind];
         alpha[i] = 0;
      }
   }
}


/**********************************************************************/
/*****                   Optimized line rendering                 *****/
/**********************************************************************/


/*
 * Draw a flat-shaded, RGB line into an osmesa buffer.
 */
static void flat_color_line( GLcontext *ctx,
                             GLuint vert0, GLuint vert1, GLuint pvert )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pvert];
   unsigned long pixel = PACK_RGBA( color[0], color[1], color[2], color[3] );

#define INTERP_XY 1
#define CLIP_HACK 1
#define PLOT(X,Y) { GLuint *ptr4 = PIXELADDR4(X,Y); *ptr4 = pixel; }

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, Z-less, RGB line into an osmesa buffer.
 */
static void flat_color_z_line( GLcontext *ctx,
                               GLuint vert0, GLuint vert1, GLuint pvert )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLubyte *color = ctx->VB->Color[pvert];
   unsigned long pixel = PACK_RGBA( color[0], color[1], color[2], color[3] );

#define INTERP_XY 1
#define INTERP_Z 1
#define CLIP_HACK 1
#define PLOT(X,Y)				\
	if (Z < *zPtr) {			\
	   GLuint *ptr4 = PIXELADDR4(X,Y);	\
	   *ptr4 = pixel;			\
	   *zPtr = Z;				\
	}

#include "linetemp.h"
}


/*
 * Draw a flat-shaded, alpha-blended, RGB line into an osmesa buffer.
 */
static void flat_blend_color_line( GLcontext *ctx,
                                 GLuint vert0, GLuint vert1, GLuint pvert )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   GLint rshift = osmesa->rshift;
   GLint gshift = osmesa->gshift;
   GLint bshift = osmesa->bshift;
   GLint avalue = VB->Color[pvert][3];
   GLint msavalue = 255 - avalue;
   GLint rvalue = VB->Color[pvert][0]*avalue;
   GLint gvalue = VB->Color[pvert][1]*avalue;
   GLint bvalue = VB->Color[pvert][2]*avalue;

#define INTERP_XY 1
#define CLIP_HACK 1
#define PLOT(X,Y)					\
   { GLuint *ptr4 = PIXELADDR4(X,Y); \
     GLuint  pixel = 0; \
     pixel |=((((((*ptr4) >> rshift) & 0xff)*msavalue+rvalue)>>8) << rshift);\
     pixel |=((((((*ptr4) >> gshift) & 0xff)*msavalue+gvalue)>>8) << gshift);\
     pixel |=((((((*ptr4) >> bshift) & 0xff)*msavalue+bvalue)>>8) << bshift);\
     *ptr4 = pixel; \
   }
   
#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, RGB line into an osmesa buffer.
 */
static void flat_blend_color_z_line( GLcontext *ctx,
                                   GLuint vert0, GLuint vert1, GLuint pvert )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   GLint rshift = osmesa->rshift;
   GLint gshift = osmesa->gshift;
   GLint bshift = osmesa->bshift;
   GLint avalue = VB->Color[pvert][3];
   GLint msavalue = 256 - avalue;
   GLint rvalue = VB->Color[pvert][0]*avalue;
   GLint gvalue = VB->Color[pvert][1]*avalue;
   GLint bvalue = VB->Color[pvert][2]*avalue;

#define INTERP_XY 1
#define INTERP_Z 1
#define CLIP_HACK 1
#define PLOT(X,Y)				\
	if (Z < *zPtr) {			\
   { GLuint *ptr4 = PIXELADDR4(X,Y); \
     GLuint  pixel = 0; \
     pixel |=((((((*ptr4) >> rshift) & 0xff)*msavalue+rvalue)>>8) << rshift);\
     pixel |=((((((*ptr4) >> gshift) & 0xff)*msavalue+gvalue)>>8) << gshift);\
     pixel |=((((((*ptr4) >> bshift) & 0xff)*msavalue+bvalue)>>8) << bshift);\
     *ptr4 = pixel; \
   } \
	}

#include "linetemp.h"
}

/*
 * Draw a flat-shaded, Z-less, alpha-blended, RGB line into an osmesa buffer.
 */
static void flat_blend_color_z_line_write( GLcontext *ctx,
                                   GLuint vert0, GLuint vert1, GLuint pvert )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   struct vertex_buffer *VB = ctx->VB;
   GLint rshift = osmesa->rshift;
   GLint gshift = osmesa->gshift;
   GLint bshift = osmesa->bshift;
   GLint avalue = VB->Color[pvert][3];
   GLint msavalue = 256 - avalue;
   GLint rvalue = VB->Color[pvert][0]*avalue;
   GLint gvalue = VB->Color[pvert][1]*avalue;
   GLint bvalue = VB->Color[pvert][2]*avalue;

#define INTERP_XY 1
#define INTERP_Z 1
#define CLIP_HACK 1
#define PLOT(X,Y)				\
	if (Z < *zPtr) {			\
   { GLuint *ptr4 = PIXELADDR4(X,Y); \
     GLuint  pixel = 0; \
     pixel |=((((((*ptr4) >> rshift) & 0xff)*msavalue+rvalue)>>8) << rshift);\
     pixel |=((((((*ptr4) >> gshift) & 0xff)*msavalue+gvalue)>>8) << gshift);\
     pixel |=((((((*ptr4) >> bshift) & 0xff)*msavalue+bvalue)>>8) << bshift);\
     *ptr4 = pixel; \
   } \
	   *zPtr = Z;				\
	}

#include "linetemp.h"
}


/*
 * Analyze context state to see if we can provide a fast line drawing
 * function, like those in lines.c.  Otherwise, return NULL.
 */
static line_func choose_line_function( GLcontext *ctx )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   
   if (ctx->Line.SmoothFlag)              return NULL;
   if (ctx->Texture.Enabled)              return NULL;
   if (ctx->Light.ShadeModel!=GL_FLAT)    return NULL;

   if (ctx->Line.Width==1.0F
       && ctx->Line.StippleFlag==GL_FALSE) {

       if (ctx->RasterMask==DEPTH_BIT
           && ctx->Depth.Func==GL_LESS
           && ctx->Depth.Mask==GL_TRUE) {
           switch(osmesa->format) {
       		case OSMESA_RGBA:
       		case OSMESA_BGRA:
       		case OSMESA_ARGB:
       			return flat_color_z_line;
       			break;
       		default:
       			return NULL;
       			break;
           }
       }

       if (ctx->RasterMask==0) {
           switch(osmesa->format) {
       		case OSMESA_RGBA:
       		case OSMESA_BGRA:
       		case OSMESA_ARGB:
       			return flat_color_line;
       			break;
       		default:
       			return NULL;
       			break;
           }
       }

       if (ctx->RasterMask==(DEPTH_BIT|BLEND_BIT)
           && ctx->Depth.Func==GL_LESS
           && ctx->Depth.Mask==GL_TRUE
           && ctx->Color.BlendSrc==GL_SRC_ALPHA
           && ctx->Color.BlendDst==GL_ONE_MINUS_SRC_ALPHA
           && ctx->Color.BlendEquation==GL_FUNC_ADD_EXT) {
           switch(osmesa->format) {
       		case OSMESA_RGBA:
       		case OSMESA_BGRA:
       		case OSMESA_ARGB:
       			return flat_blend_color_z_line_write;
       			break;
       		default:
       			return NULL;
       			break;
           }
       }

       if (ctx->RasterMask==(DEPTH_BIT|BLEND_BIT)
           && ctx->Depth.Func==GL_LESS
           && ctx->Depth.Mask==GL_FALSE
           && ctx->Color.BlendSrc==GL_SRC_ALPHA
           && ctx->Color.BlendDst==GL_ONE_MINUS_SRC_ALPHA
           && ctx->Color.BlendEquation==GL_FUNC_ADD_EXT) {
           switch(osmesa->format) {
       		case OSMESA_RGBA:
       		case OSMESA_BGRA:
       		case OSMESA_ARGB:
       			return flat_blend_color_z_line;
       			break;
       		default:
       			return NULL;
       			break;
           }
       }

       if (ctx->RasterMask==BLEND_BIT
           && ctx->Color.BlendSrc==GL_SRC_ALPHA
           && ctx->Color.BlendDst==GL_ONE_MINUS_SRC_ALPHA
           && ctx->Color.BlendEquation==GL_FUNC_ADD_EXT) {
           switch(osmesa->format) {
       		case OSMESA_RGBA:
       		case OSMESA_BGRA:
       		case OSMESA_ARGB:
       			return flat_blend_color_line;
       			break;
       		default:
       			return NULL;
       			break;
           }
       }

   }
   return NULL;
}


/**********************************************************************/
/*****                 Optimized triangle rendering               *****/
/**********************************************************************/


/*
 * Smooth-shaded, z-less triangle, RGBA color.
 */
static void smooth_color_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                     GLuint v2, GLuint pv )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   GLint rshift = osmesa->rshift;
   GLint gshift = osmesa->gshift;
   GLint bshift = osmesa->bshift;
   GLint ashift = osmesa->ashift;
#define INTERP_Z 1
#define INTERP_RGB 1
#define INTERP_ALPHA 1
#define INNER_LOOP( LEFT, RIGHT, Y )				\
{								\
   GLint i, len = RIGHT-LEFT;					\
   GLuint *img = PIXELADDR4(LEFT,Y);   				\
   for (i=0;i<len;i++,img++) {					\
      GLdepth z = FixedToDepth(ffz);				\
      if (z < zRow[i]) {					\
         *img = PACK_RGBA2( FixedToInt(ffr), FixedToInt(ffg),	\
		            FixedToInt(ffb), FixedToInt(ffa) );	\
         zRow[i] = z;						\
      }								\
      ffr += fdrdx;  ffg += fdgdx;  ffb += fdbdx;  ffa += fdadx;\
      ffz += fdzdx;						\
   }								\
}
#include "tritemp.h"
}




/*
 * Flat-shaded, z-less triangle, RGBA color.
 */
static void flat_color_z_triangle( GLcontext *ctx, GLuint v0, GLuint v1,
                                   GLuint v2, GLuint pv )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
#define INTERP_Z 1
#define SETUP_CODE			\
   GLubyte r = VB->Color[pv][0];	\
   GLubyte g = VB->Color[pv][1];	\
   GLubyte b = VB->Color[pv][2];	\
   GLubyte a = VB->Color[pv][3];	\
   GLuint pixel = PACK_RGBA(r,g,b,a);

#define INNER_LOOP( LEFT, RIGHT, Y )	\
{					\
   GLint i, len = RIGHT-LEFT;		\
   GLuint *img = PIXELADDR4(LEFT,Y);   	\
   for (i=0;i<len;i++,img++) {		\
      GLdepth z = FixedToDepth(ffz);	\
      if (z < zRow[i]) {		\
         *img = pixel;			\
         zRow[i] = z;			\
      }					\
      ffz += fdzdx;			\
   }					\
}
#include "tritemp.h"
}



/*
 * Return pointer to an accelerated triangle function if possible.
 */
static triangle_func choose_triangle_function( GLcontext *ctx )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;

   if ((osmesa->format==OSMESA_RGB)||(osmesa->format==OSMESA_BGR)) return NULL;
   
   if (ctx->Polygon.SmoothFlag)     return NULL;
   if (ctx->Polygon.StippleFlag)    return NULL;
   if (ctx->Texture.Enabled)        return NULL;

   if (ctx->RasterMask==DEPTH_BIT
       && ctx->Depth.Func==GL_LESS
       && ctx->Depth.Mask==GL_TRUE
       && osmesa->format!=OSMESA_COLOR_INDEX) {
      if (ctx->Light.ShadeModel==GL_SMOOTH) {
         return smooth_color_z_triangle;
      }
      else {
         return flat_color_z_triangle;
      }
   }
   return NULL;
}



static const char *renderer_string(void)
{
   return "OffScreen";
}


static void osmesa_setup_DD_pointers( GLcontext *ctx )
{
   OSMesaContext osmesa = (OSMesaContext) ctx->DriverCtx;
   
   ctx->Driver.RendererString = renderer_string;
   ctx->Driver.UpdateState = osmesa_setup_DD_pointers;

   ctx->Driver.SetBuffer = set_buffer;
   ctx->Driver.Color = set_color;
   ctx->Driver.Index = set_index;
   ctx->Driver.ClearIndex = clear_index;
   ctx->Driver.ClearColor = clear_color;
   ctx->Driver.Clear = clear;

   ctx->Driver.GetBufferSize = buffer_size;

   ctx->Driver.PointsFunc = NULL;
   ctx->Driver.LineFunc = choose_line_function( ctx );
   ctx->Driver.TriangleFunc = choose_triangle_function( ctx );

   /* RGB(A) span/pixel functions */
   if ((osmesa->format==OSMESA_RGB) || (osmesa->format==OSMESA_BGR)) {
      ctx->Driver.WriteColorSpan = write_color_span3;
      ctx->Driver.WriteColorPixels = write_color_pixels3;
      ctx->Driver.WriteMonocolorSpan = write_monocolor_span3;
      ctx->Driver.WriteMonocolorPixels = write_monocolor_pixels3;
      ctx->Driver.ReadColorSpan = read_color_span3;
      ctx->Driver.ReadColorPixels = read_color_pixels3;
   }
   else {
      ctx->Driver.WriteColorSpan = write_color_span;
      ctx->Driver.WriteColorPixels = write_color_pixels;
      ctx->Driver.WriteMonocolorSpan = write_monocolor_span;
      ctx->Driver.WriteMonocolorPixels = write_monocolor_pixels;
      ctx->Driver.ReadColorSpan = read_color_span;
      ctx->Driver.ReadColorPixels = read_color_pixels;
   }

   /* CI span/pixel functions */
   ctx->Driver.WriteIndexSpan = write_index_span;
   ctx->Driver.WriteMonoindexSpan = write_monoindex_span;
   ctx->Driver.WriteIndexPixels = write_index_pixels;
   ctx->Driver.WriteMonoindexPixels = write_monoindex_pixels;
   ctx->Driver.ReadIndexSpan = read_index_span;
   ctx->Driver.ReadIndexPixels = read_index_pixels;
}
