/* $Id: xmesa2.c,v 1.11 1997/05/26 20:34:40 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.3
 * Copyright (C) 1995-1996  Brian Paul
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
 * $Log: xmesa2.c,v $
 * Revision 1.11  1997/05/26 20:34:40  brianp
 * added PF_TRUEDITHER pixel format
 *
 * Revision 1.10  1997/04/20 20:29:11  brianp
 * replaced abort() with gl_problem()
 *
 * Revision 1.9  1997/04/12 16:24:07  brianp
 * removed several xmesa_setup_DD_pointers calls, removed die()
 *
 * Revision 1.8  1997/04/04 05:23:57  brianp
 * fixed problem with black/white being reversed on monochrome displays
 *
 * Revision 1.7  1997/03/21 01:58:32  brianp
 * added RendererString() function
 *
 * Revision 1.6  1996/10/22 02:46:29  brianp
 * now use DITHER_SETUP and XDITHER macros
 * use array indexing instead of pointer dereferencing in inner loops
 *
 * Revision 1.5  1996/10/11 00:23:58  brianp
 * fixed dithering bug in write_span_DITHER_pixmap() again!
 *
 * Revision 1.4  1996/10/09 23:09:56  brianp
 * fixed dithering bug in write_span_DITHER_pixmap()
 *
 * Revision 1.3  1996/09/27 01:31:42  brianp
 * removed unused variables
 *
 * Revision 1.2  1996/09/19 03:16:04  brianp
 * new X/Mesa interface with XMesaContext, XMesaVisual, and XMesaBuffer types
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


/*
 * Mesa/X11 interface, part 2.
 *
 * This file contains the implementations of all the device driver functions.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "GL/xmesa.h"
#include "context.h"
#include "macros.h"
#include "types.h"
#include "xmesaP.h"




/*
 * The following functions are used to trap XGetImage() calls which
 * generate BadMatch errors if the drawable isn't mapped.
 */

static int caught_xgetimage_error = 0;
static int (*old_xerror_handler)( Display *dpy, XErrorEvent *ev );
static unsigned long xgetimage_serial;

/*
 * This is the error handler which will be called if XGetImage fails.
 */
static int xgetimage_error_handler( Display *dpy, XErrorEvent *ev )
{
   if (ev->serial==xgetimage_serial && ev->error_code==BadMatch) {
      /* caught the expected error */
      caught_xgetimage_error = 0;
   }
   else {
      /* call the original X error handler, if any.  otherwise ignore */
      if (old_xerror_handler) {
         (*old_xerror_handler)( dpy, ev );
      }
   }
   return 0;
}


/*
 * Call this right before XGetImage to setup error trap.
 */
static void catch_xgetimage_errors( Display *dpy )
{
   xgetimage_serial = NextRequest( dpy );
   old_xerror_handler = XSetErrorHandler( xgetimage_error_handler );
   caught_xgetimage_error = 0;
}


/*
 * Call this right after XGetImage to check if an error occured.
 */
static int check_xgetimage_errors( void )
{
   /* restore old handler */
   (void) XSetErrorHandler( old_xerror_handler );
   /* return 0=no error, 1=error caught */
   return caught_xgetimage_error;
}


/*
 * Read a pixel from an X drawable.
 */
static unsigned long read_pixel( Display *dpy, Drawable d, int x, int y )
{
   XImage *pixel;
   unsigned long p;
   int error;
   catch_xgetimage_errors( dpy );
   pixel = XGetImage( dpy, d, x, y, 1, 1, AllPlanes, ZPixmap );
   error = check_xgetimage_errors();
   if (pixel && !error) {
      p = XGetPixel( pixel, 0, 0 );
   }
   else {
      p = 0;
   }
   if (pixel) {
      XDestroyImage( pixel );
   }
   return p;
}




/*
 * Return the size (width,height of the current color buffer.
 * This function should be called by the glViewport function because
 * glViewport is often called when the window gets resized.  We need to
 * update some X/Mesa stuff when that happens.
 * Output:  width - width of buffer in pixels.
 *          height - height of buffer in pixels.
 */
static void get_buffer_size( GLcontext *ctx, GLuint *width, GLuint *height )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Window root;
   int winx, winy;
   unsigned int winwidth, winheight;
   unsigned int bw, d;

   XGetGeometry( xmesa->display, xmesa->xm_buffer->frontbuffer, &root,
		 &winx, &winy, &winwidth, &winheight, &bw, &d );

   *width = winwidth;
   *height = winheight;

   if (   winwidth!=xmesa->xm_buffer->width
       || winheight!=xmesa->xm_buffer->height) {
      xmesa->xm_buffer->width = winwidth;
      xmesa->xm_buffer->height = winheight;
      xmesa_alloc_back_buffer( xmesa->xm_buffer );
   }

   /* Needed by FLIP macro */
   xmesa->xm_buffer->bottom = (int) winheight - 1;

   if (xmesa->xm_buffer->backimage) {
      /* Needed by PIXELADDR1 macro */
      xmesa->xm_buffer->ximage_width1
                  = xmesa->xm_buffer->backimage->bytes_per_line;
      xmesa->xm_buffer->ximage_origin1
                  = (GLubyte *) xmesa->xm_buffer->backimage->data
                    + xmesa->xm_buffer->ximage_width1 * (winheight-1);

      /* Needed by PIXELADDR2 macro */
      xmesa->xm_buffer->ximage_width2
                  = xmesa->xm_buffer->backimage->bytes_per_line / 2;
      xmesa->xm_buffer->ximage_origin2
                  = (GLushort *) xmesa->xm_buffer->backimage->data
                    + xmesa->xm_buffer->ximage_width2 * (winheight-1);

      /* Needed by PIXELADDR4 macro */
      xmesa->xm_buffer->ximage_width4 = xmesa->xm_buffer->backimage->width;
      xmesa->xm_buffer->ximage_origin4
                  = (GLuint *) xmesa->xm_buffer->backimage->data
                    + xmesa->xm_buffer->ximage_width4 * (winheight-1);
   }
}


static void finish( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (xmesa) {
      XSync( xmesa->display, False );
   }
}


static void flush( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (xmesa) {
      XFlush( xmesa->display );
   }
}



static GLboolean set_buffer( GLcontext *ctx, GLenum mode )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (mode==GL_FRONT) {
      /* read/write front buffer */
      xmesa->xm_buffer->buffer = xmesa->xm_buffer->frontbuffer;
      ctx->NewState |= NEW_RASTER_OPS;
      gl_update_state(ctx);
      /*xmesa_setup_DD_pointers( ctx );*/
      return GL_TRUE;
   }
   else if (mode==GL_BACK && xmesa->xm_buffer->db_state) {
      /* read/write back buffer */
      if (xmesa->xm_buffer->backpixmap) {
         xmesa->xm_buffer->buffer = xmesa->xm_buffer->backpixmap;
      }
      else if (xmesa->xm_buffer->backimage) {
         xmesa->xm_buffer->buffer = None;
      }
      else {
         /* just in case there wasn't enough memory for back buffer */
         xmesa->xm_buffer->buffer = xmesa->xm_buffer->frontbuffer;
      }
      ctx->NewState |= NEW_RASTER_OPS;
      gl_update_state(ctx);
      /*xmesa_setup_DD_pointers( ctx );*/
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}



static void clear_index( GLcontext *ctx, GLuint index )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->clearpixel = (unsigned long) index;
   XSetForeground( xmesa->display, xmesa->xm_buffer->cleargc,
                   (unsigned long) index );
}


static void clear_color( GLcontext *ctx,
                         GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->clearcolor[0] = r;
   xmesa->clearcolor[1] = g;
   xmesa->clearcolor[2] = b;
   xmesa->clearcolor[3] = a;
   xmesa->clearpixel = xmesa_color_to_pixel( xmesa, r, g, b, a );
   XSetForeground( xmesa->display, xmesa->xm_buffer->cleargc,
                   xmesa->clearpixel );
}


/* Set current color index */
static void set_index( GLcontext *ctx, GLuint index )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   unsigned long p = (unsigned long) index;
   xmesa->pixel = p;
   XSetForeground( xmesa->display, xmesa->xm_buffer->gc1, p );
}


/* Set current drawing color */
static void set_color( GLcontext *ctx,
                       GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   xmesa->red   = r;
   xmesa->green = g;
   xmesa->blue  = b;
   xmesa->alpha = a;
   xmesa->pixel = xmesa_color_to_pixel( xmesa, r, g, b, a );;
   XSetForeground( xmesa->display, xmesa->xm_buffer->gc1, xmesa->pixel );
}



/* Set index mask ala glIndexMask */
static GLboolean index_mask( GLcontext *ctx, GLuint mask )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (xmesa->xm_buffer->buffer==XIMAGE) {
      return GL_FALSE;
   }
   else {
      unsigned long m;
      if (mask==0xffffffff) {
         m = AllPlanes;
      }
      else {
         m = (unsigned long) mask;
      }
      XSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc1, m );
      XSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc2, m );
      XSetPlaneMask( xmesa->display, xmesa->xm_buffer->cleargc, m );
      return GL_TRUE;
   }
}


/* Implements glColorMask() */
static GLboolean color_mask( GLcontext *ctx,
                             GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
#if defined(__cplusplus) || defined(c_plusplus)
   int xclass = xmesa->xm_visual->visinfo->c_class;
#else
   int xclass = xmesa->xm_visual->visinfo->class;
#endif

   if (xmesa->xm_buffer->buffer!=XIMAGE
       && (xclass==TrueColor || xclass==DirectColor)) {
      unsigned long m;
      if (rmask && gmask && bmask) {
         m = AllPlanes;
      }
      else {
         m = 0;
         if (rmask)   m |= xmesa->xm_visual->visinfo->red_mask;
         if (gmask)   m |= xmesa->xm_visual->visinfo->green_mask;
         if (bmask)   m |= xmesa->xm_visual->visinfo->blue_mask;
      }
      XSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc1, m );
      XSetPlaneMask( xmesa->display, xmesa->xm_buffer->gc2, m );
      XSetPlaneMask( xmesa->display, xmesa->xm_buffer->cleargc, m );
      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}


/*
 * Set the pixel logic operation.  Return GL_TRUE if the device driver
 * can perform the operation, otherwise return GL_FALSE.  GL_COPY _must_
 * be operational, obviously.
 */
static GLboolean logicop( GLcontext *ctx, GLenum op )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int func;
   if ((xmesa->xm_buffer->buffer==XIMAGE) && op!=GL_COPY) {
      /* X can't do logic ops in Ximages, except for GL_COPY */
      return GL_FALSE;
   }
   switch (op) {
      case GL_CLEAR:		func = GXclear;		break;
      case GL_SET:		func = GXset;		break;
      case GL_COPY:		func = GXcopy;		break;
      case GL_COPY_INVERTED:	func = GXcopyInverted;	break;
      case GL_NOOP:		func = GXnoop;		break;
      case GL_INVERT:		func = GXinvert;	break;
      case GL_AND:		func = GXand;		break;
      case GL_NAND:		func = GXnand;		break;
      case GL_OR:		func = GXor;		break;
      case GL_NOR:		func = GXnor;		break;
      case GL_XOR:		func = GXxor;		break;
      case GL_EQUIV:		func = GXequiv;		break;
      case GL_AND_REVERSE:	func = GXandReverse;	break;
      case GL_AND_INVERTED:	func = GXandInverted;	break;
      case GL_OR_REVERSE:	func = GXorReverse;	break;
      case GL_OR_INVERTED:	func = GXorInverted;	break;
      default:  return GL_FALSE;
   }
   XSetFunction( xmesa->display, xmesa->xm_buffer->gc1, func );
   XSetFunction( xmesa->display, xmesa->xm_buffer->gc2, func );
   return GL_TRUE;
}


/*
 * Enable/disable dithering
 */
static void dither( GLcontext *ctx, GLboolean enable )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (enable) {
      xmesa->pixelformat = xmesa->xm_visual->dithered_pf;
   }
   else {
      xmesa->pixelformat = xmesa->xm_visual->undithered_pf;
   }
}



/**********************************************************************/
/*** glClear implementations                                        ***/
/**********************************************************************/

static void clear_pixmap( GLcontext *ctx, GLboolean all,
                          GLint x, GLint y, GLint width, GLint height )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      XFillRectangle( xmesa->display, xmesa->xm_buffer->buffer,
                      xmesa->xm_buffer->cleargc,
                      0, 0,
                      xmesa->xm_buffer->width+1, xmesa->xm_buffer->height+1 );
   }
   else {
      XFillRectangle( xmesa->display, xmesa->xm_buffer->buffer,
                      xmesa->xm_buffer->cleargc,
                      x, xmesa->xm_buffer->height - y - height,
                      width, height );
   }
}


static void clear_8bit_ximage( GLcontext *ctx, GLboolean all,
                               GLint x, GLint y, GLint width, GLint height )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      size_t n = xmesa->xm_buffer->backimage->bytes_per_line
               * xmesa->xm_buffer->backimage->height;
      MEMSET( xmesa->xm_buffer->backimage->data, xmesa->clearpixel, n );
   }
   else {
      GLint i;
      for (i=0;i<height;i++) {
         GLubyte *ptr = PIXELADDR1( x, y+i );
         MEMSET( ptr, xmesa->clearpixel, width );
      }
   }
}


static void clear_16bit_ximage( GLcontext *ctx, GLboolean all,
                                GLint x, GLint y, GLint width, GLint height )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      register GLuint n;
      register GLushort *ptr2 =(GLushort *) xmesa->xm_buffer->backimage->data;
      register GLushort pixel = (GLushort) xmesa->clearpixel;
      if (xmesa->swapbytes) {
         pixel = ((pixel >> 8) & 0x00ff)
               | ((pixel << 8) & 0xff00);
      }
      if ((pixel & 0xff) == (pixel >> 8)) {
         /* low and high bytes are equal so use memset() */
         n = xmesa->xm_buffer->backimage->bytes_per_line
             * xmesa->xm_buffer->height;
         MEMSET( ptr2, pixel & 0xff, n );
      }
      else {
         n = xmesa->xm_buffer->backimage->bytes_per_line / 2
             * xmesa->xm_buffer->height;
	 do {
	    *ptr2++ = pixel;
	    n--;
	 } while (n!=0);
      }
   }
   else {
      register int i, j;
      register GLushort pixel = (GLushort) xmesa->clearpixel;
      for (j=0;j<height;j++) {
	 register GLushort *ptr2 = PIXELADDR2( x, y+j );
         for (i=0;i<width;i++) {
            *ptr2++ = pixel;
         }
      }
   }
}


static void clear_32bit_ximage( GLcontext *ctx, GLboolean all,
                                GLint x, GLint y, GLint width, GLint height )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   if (all) {
      register GLint n = xmesa->xm_buffer->width * xmesa->xm_buffer->height;
      register GLuint *ptr4 = (GLuint *) xmesa->xm_buffer->backimage->data;
      register GLuint pixel = (GLuint) xmesa->clearpixel;
      if (xmesa->swapbytes) {
         pixel = ((pixel >> 24) & 0x000000ff)
               | ((pixel >> 8)  & 0x0000ff00)
               | ((pixel << 8)  & 0x00ff0000)
               | ((pixel << 24) & 0xff000000);
      }
      if (pixel==0) {
         MEMSET( ptr4, pixel, 4*n );
      }
      else {
         do {
            *ptr4++ = pixel;
            n--;
         } while (n!=0);
      }
   }
   else {
      register int i, j;
      register GLuint pixel = (GLuint) xmesa->clearpixel;
      for (j=0;j<height;j++) {
         register GLuint *ptr4 = PIXELADDR4( x, y+j );
         for (i=0;i<width;i++) {
            *ptr4++ = pixel;
         }
      }
   }
}


static void clear_nbit_ximage( GLcontext *ctx, GLboolean all,
                               GLint x, GLint y, GLint width, GLint height )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   if (all) {
      register int i, j;
      width = xmesa->xm_buffer->width;
      height = xmesa->xm_buffer->height;
      for (j=0;j<height;j++) {
         for (i=0;i<width;i++) {
            XPutPixel( img, i, j, xmesa->clearpixel );
         }
      }
   }
   else {
      /* TODO: optimize this */
      register int i, j;
      y = FLIP(y);
      for (j=0;j<height;j++) {
         for (i=0;i<width;i++) {
            XPutPixel( img, x+i, y-j, xmesa->clearpixel );
         }
      }
   }
}




/*
 * The Mesa library needs to be able to draw pixels in a number of ways:
 *   1. RGB vs Color Index
 *   2. as horizontal spans (polygons, images) vs random locations (points,
 *      lines)
 *   3. different color per-pixel or same color for all pixels
 *
 * Furthermore, the X driver needs to support rendering to 3 possible
 * "buffers", usually one, but sometimes two at a time:
 *   1. The front buffer as an X window
 *   2. The back buffer as a Pixmap
 *   3. The back buffer as an XImage
 *
 * Finally, if the back buffer is an XImage, we can avoid using XPutPixel and
 * optimize common cases such as 24-bit and 8-bit modes.
 *
 * By multiplication, there's at least 48 possible combinations of the above.
 *
 * Below are implementations of the most commonly used combinations.  They are
 * accessed through function pointers which get initialized here and are used
 * directly from the Mesa library.  The 8 function pointers directly correspond
 * to the first 3 cases listed above.
 *
 *
 * The function naming convention is:
 *
 *   write_[span|pixels]_[mono]_[format]_[pixmap|ximage]
 *
 * New functions optimized for specific cases can be added without too much
 * trouble.  An example might be the 24-bit TrueColor mode 8A8R8G8B which is
 * found on IBM RS/6000 X servers.
 */




/**********************************************************************/
/*** Write COLOR SPAN functions                                     ***/
/**********************************************************************/


#define COLOR_SPAN_ARGS	GLcontext *ctx,					\
			GLuint n, GLint x, GLint y,			\
			const GLubyte red[], const GLubyte green[],	\
			const GLubyte blue[], const GLubyte alpha[],	\
			const GLubyte mask[]

/* NOTE: if mask==NULL, draw all pixels */


/*
 * Write a span of PF_TRUECOLOR pixels to a pixmap.
 */
static void write_span_TRUECOLOR_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUECOLOR( p, red[i], green[i], blue[i] );
            XSetForeground( dpy, gc, p );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         unsigned long p;
         PACK_TRUECOLOR( p, red[i], green[i], blue[i] );
         XPutPixel( rowimg, i, 0, p );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to a pixmap.
 */
static void write_span_TRUEDITHER_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, red[i], green[i], blue[i]);
            XSetForeground( dpy, gc, p );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         unsigned long p;
         PACK_TRUEDITHER(p, x+i, y, red[i], green[i], blue[i]);
         XPutPixel( rowimg, i, 0, p );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_8A8B8G8R pixels to a pixmap.
 */
static void write_span_8A8B8G8R_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc,
                         PACK_8A8B8G8R(red[i], green[i], blue[i], alpha[i]) );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_8R8G8B pixels to a pixmap.
 */
static void write_span_8R8G8B_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc, PACK_8R8G8B( red[i], green[i], blue[i] ));
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLuint *ptr4 = (GLuint *) rowimg->data;
      for (i=0;i<n;i++) {
         *ptr4++ = PACK_8R8G8B( red[i], green[i], blue[i] );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_5R6G5B pixels to a pixmap.
 */
static void write_span_5R6G5B_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc, PACK_5R6G5B( red[i], green[i], blue[i] ));
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLushort *ptr2 = (GLushort *) rowimg->data;
      for (i=0;i<n;i++) {
         ptr2[i] = PACK_5R6G5B( red[i], green[i], blue[i] );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_DITHER pixels to a pixmap.
 */
static void write_span_DITHER_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   XDITHER_SETUP(y);
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc, XDITHER(x, red[i], green[i], blue[i]) );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XPutPixel( rowimg, i, 0, XDITHER(x+i, red[i], green[i], blue[i]) );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_1BIT pixels to a pixmap.
 */
static void write_span_1BIT_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   SETUP_1BIT;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc,
                            DITHER_1BIT( x, y, red[i], green[i], blue[i] ) );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      /* draw all pixels */
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XPutPixel( rowimg, i, 0,
                    DITHER_1BIT( x+i, y, red[i], green[i], blue[i] ) );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_HPCR pixels to a pixmap.
 */
static void write_span_HPCR_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc,
                            DITHER_HPCR( x, y, red[i], green[i], blue[i] ) );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      register GLubyte *ptr = (GLubyte *) xmesa->xm_buffer->rowimage->data;
      for (i=0;i<n;i++) {
         ptr[i] = DITHER_HPCR( (x+i), y, red[i], green[i], blue[i] );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_LOOKUP pixels to a pixmap.
 */
static void write_span_LOOKUP_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   LOOKUP_SETUP;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc, LOOKUP( red[i], green[i], blue[i] ) );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XPutPixel( rowimg, i, 0, LOOKUP(red[i],green[i],blue[i]) );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}



/*
 * Write a span of PF_GRAYSCALE pixels to a pixmap.
 */
static void write_span_GRAYSCALE_pixmap( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP( y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XSetForeground( dpy, gc, GRAY_RGB( red[i], green[i], blue[i] ) );
            XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
         }
      }
   }
   else {
      XImage *rowimg = xmesa->xm_buffer->rowimage;
      for (i=0;i<n;i++) {
         XPutPixel( rowimg, i, 0, GRAY_RGB(red[i],green[i],blue[i]) );
      }
      XPutImage( dpy, buffer, gc, rowimg, 0, 0, x, y, n, 1 );
   }
}


/*
 * Write a span of PF_TRUECOLOR pixels to an XImage.
 */
static void write_span_TRUECOLOR_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUECOLOR( p, red[i], green[i], blue[i] );
            XPutPixel( img, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         unsigned long p;
         PACK_TRUECOLOR( p, red[i], green[i], blue[i] );
         XPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to an XImage.
 */
static void write_span_TRUEDITHER_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, red[i], green[i], blue[i]);
            XPutPixel( img, x, y, p );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         unsigned long p;
         PACK_TRUEDITHER(p, x, y, red[i], green[i], blue[i]);
         XPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of PF_8A8B8G8R-format pixels to an ximage.
 */
static void write_span_8A8B8G8R_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
      }
   }
}


/*
 * Write a span of PF_8R8G8B-format pixels to an ximage.
 */
static void write_span_8R8G8B_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint *ptr = PIXELADDR4( x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_8R8G8B( red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = PACK_8R8G8B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write a span of PF_5R6G5B-format pixels to an ximage.
 */
static void write_span_5R6G5B_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLushort *ptr = PIXELADDR2( x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = PACK_5R6G5B( red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = PACK_5R6G5B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write a span of PF_DITHER pixels to an XImage.
 */
static void write_span_DITHER_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   int yy = FLIP(y);
   XDITHER_SETUP(yy);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XPutPixel( img, x, yy, XDITHER( x, red[i], green[i], blue[i] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XPutPixel( img, x, yy, XDITHER( x, red[i], green[i], blue[i] ) );
      }
   }
}



/*
 * Write a span of 8-bit PF_DITHER pixels to an XImage.
 */
static void write_span_DITHER8_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( x, y );
   XDITHER_SETUP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = XDITHER( x, red[i], green[i], blue[i] );
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         ptr[i] = XDITHER( x, red[i], green[i], blue[i] );
      }
   }
}



/*
 * Write a span of PF_1BIT pixels to an XImage.
 */
static void write_span_1BIT_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   SETUP_1BIT;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XPutPixel(img, x, y, DITHER_1BIT(x, y, red[i], green[i], blue[i]));
         }
      }
   }
   else {
      for (i=0;i<n;i++,x++) {
         XPutPixel( img, x, y, DITHER_1BIT(x, y, red[i], green[i], blue[i]) );
      }
   }
}


/*
 * Write a span of PF_HPCR pixels to an XImage.
 */
static void write_span_HPCR_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( x, y );
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = DITHER_HPCR( x, y, red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         ptr[i] = DITHER_HPCR( x, y, red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write a span of PF_LOOKUP pixels to an XImage.
 */
static void write_span_LOOKUP_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   LOOKUP_SETUP;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XPutPixel( img, x, y, LOOKUP( red[i], green[i], blue[i] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XPutPixel( img, x, y, LOOKUP( red[i], green[i], blue[i] ) );
      }
   }
}


/*
 * Write a span of 8-bit PF_LOOKUP pixels to an XImage.
 */
static void write_span_LOOKUP8_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( x, y );
   LOOKUP_SETUP;
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            ptr[i] = LOOKUP( red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         ptr[i] = LOOKUP( red[i], green[i], blue[i] );
      }
   }
}




/*
 * Write a span of PF_GRAYSCALE pixels to an XImage.
 */
static void write_span_GRAYSCALE_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(y);
   if (mask) {
      for (i=0;i<n;i++,x++) {
         if (mask[i]) {
            XPutPixel( img, x, y, GRAY_RGB( red[i], green[i], blue[i] ) );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++,x++) {
         XPutPixel( img, x, y, GRAY_RGB( red[i], green[i], blue[i] ) );
      }
   }
}



/*
 * Write a span of 8-bit PF_GRAYSCALE pixels to an XImage.
 */
static void write_span_GRAYSCALE8_ximage( COLOR_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1( x, y );
   if (mask) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            ptr[i] = GRAY_RGB( red[i], green[i], blue[i] );
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0;i<n;i++) {
         ptr[i] = GRAY_RGB( red[i], green[i], blue[i] );
      }
   }
}




/**********************************************************************/
/*** Write COLOR PIXEL functions                                    ***/
/**********************************************************************/


#define COLOR_PIXEL_ARGS   GLcontext *ctx,				\
			   GLuint n, const GLint x[], const GLint y[],	\
			   const GLubyte red[], const GLubyte green[],	\
			   const GLubyte blue[], const GLubyte alpha[],	\
			   const GLubyte mask[]


/*
 * Write an array of PF_TRUECOLOR pixels to a pixmap.
 */
static void write_pixels_TRUECOLOR_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUECOLOR( p, red[i], green[i], blue[i] );
	 XSetForeground( dpy, gc, p );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_TRUEDITHER pixels to a pixmap.
 */
static void write_pixels_TRUEDITHER_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], y[i], red[i], green[i], blue[i]);
         XSetForeground( dpy, gc, p );
         XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_8A8B8G8R pixels to a pixmap.
 */
static void write_pixels_8A8B8G8R_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XSetForeground( dpy, gc,
                         PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] ));
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_8R8G8B pixels to a pixmap.
 */
static void write_pixels_8R8G8B_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XSetForeground( dpy, gc, PACK_8R8G8B( red[i], green[i], blue[i] ) );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_5R6G5B pixels to a pixmap.
 */
static void write_pixels_5R6G5B_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XSetForeground( dpy, gc, PACK_5R6G5B( red[i], green[i], blue[i] ) );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to a pixmap.
 */
static void write_pixels_DITHER_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   DITHER_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XSetForeground( dpy, gc,
                         DITHER(x[i], y[i], red[i], green[i], blue[i]) );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to a pixmap.
 */
static void write_pixels_1BIT_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   SETUP_1BIT;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XSetForeground( dpy, gc,
                         DITHER_1BIT( x[i], y[i], red[i], green[i], blue[i] ));
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_HPCR pixels to a pixmap.
 */
static void write_pixels_HPCR_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XSetForeground( dpy, gc,
                         DITHER_HPCR( x[i], y[i], red[i], green[i], blue[i] ));
         XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_LOOKUP pixels to a pixmap.
 */
static void write_pixels_LOOKUP_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   LOOKUP_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XSetForeground( dpy, gc, LOOKUP( red[i], green[i], blue[i] ) );
         XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_GRAYSCALE pixels to a pixmap.
 */
static void write_pixels_GRAYSCALE_pixmap( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XSetForeground( dpy, gc, GRAY_RGB( red[i], green[i], blue[i] ) );
         XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_TRUECOLOR pixels to an ximage.
 */
static void write_pixels_TRUECOLOR_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUECOLOR( p, red[i], green[i], blue[i] );
	 XPutPixel( img, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of PF_TRUEDITHER pixels to an XImage.
 */
static void write_pixels_TRUEDITHER_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], FLIP(y[i]), red[i], green[i], blue[i]);
	 XPutPixel( img, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of PF_8A8B8G8R pixels to an ximage.
 */
static void write_pixels_8A8B8G8R_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
         *ptr = PACK_8A8B8G8R( red[i], green[i], blue[i], alpha[i] );
      }
   }
}


/*
 * Write an array of PF_8R8G8B pixels to an ximage.
 */
static void write_pixels_8R8G8B_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
         *ptr = PACK_8R8G8B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_5R6G5B pixels to an ximage.
 */
static void write_pixels_5R6G5B_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLushort *ptr = PIXELADDR2( x[i], y[i] );
         *ptr = PACK_5R6G5B( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to an XImage.
 */
static void write_pixels_DITHER_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   DITHER_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]),
                    DITHER( x[i], y[i], red[i], green[i], blue[i] ) );
      }
   }
}


/*
 * Write an array of 8-bit PF_DITHER pixels to an XImage.
 */
static void write_pixels_DITHER8_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   DITHER_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = DITHER( x[i], y[i], red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to an XImage.
 */
static void write_pixels_1BIT_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   SETUP_1BIT;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]),
                    DITHER_1BIT( x[i], y[i], red[i], green[i], blue[i] ));
      }
   }
}


/*
 * Write an array of PF_HPCR pixels to an XImage.
 */
static void write_pixels_HPCR_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr = PIXELADDR1(x[i],y[i]);
         *ptr = DITHER_HPCR( x[i], y[i], red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_LOOKUP pixels to an XImage.
 */
static void write_pixels_LOOKUP_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   LOOKUP_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]), LOOKUP(red[i], green[i], blue[i]) );
      }
   }
}


/*
 * Write an array of 8-bit PF_LOOKUP pixels to an XImage.
 */
static void write_pixels_LOOKUP8_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   LOOKUP_SETUP;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = LOOKUP( red[i], green[i], blue[i] );
      }
   }
}


/*
 * Write an array of PF_GRAYSCALE pixels to an XImage.
 */
static void write_pixels_GRAYSCALE_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]),
                    GRAY_RGB( red[i], green[i], blue[i] ) );
      }
   }
}


/*
 * Write an array of 8-bit PF_GRAYSCALE pixels to an XImage.
 */
static void write_pixels_GRAYSCALE8_ximage( COLOR_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1( x[i], y[i] );
	 *ptr = GRAY_RGB( red[i], green[i], blue[i] );
      }
   }
}




/**********************************************************************/
/*** Write MONO COLOR SPAN functions                                ***/
/**********************************************************************/

#define MONO_SPAN_ARGS	GLcontext *ctx,	\
		 	GLuint n, GLint x, GLint y, const GLubyte mask[]


/*
 * Write a span of identical pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_pixmap( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc1;
   register GLuint i;
   register GLboolean write_all;
   y = FLIP( y );
   write_all = GL_TRUE;
   for (i=0;i<n;i++) {
      if (!mask[i]) {
	 write_all = GL_FALSE;
	 break;
      }
   }
   if (write_all) {
      XFillRectangle( dpy, buffer, gc, (int) x, (int) y, n, 1 );
   }
   else {
      for (i=0;i<n;i++,x++) {
	 if (mask[i]) {
	    XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
	 }
      }
   }
}


/*
 * Write a span of PF_TRUEDITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_TRUEDITHER_pixmap( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   int yy = FLIP( y );
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x, yy, r, g, b);
         XSetForeground( dpy, gc, p );
         XDrawPoint( dpy, buffer, gc, (int) x, (int) yy );
      }
   }
}


/*
 * Write a span of PF_DITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_DITHER_pixmap( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   int yy = FLIP( y );
   XDITHER_SETUP(yy);
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         XSetForeground( dpy, gc, XDITHER( x, r, g, b ) );
         XDrawPoint( dpy, buffer, gc, (int) x, (int) yy );
      }
   }
}


/*
 * Write a span of PF_1BIT pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_1BIT_pixmap( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   y = FLIP( y );
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         XSetForeground( dpy, gc, DITHER_1BIT( x, y, r, g, b ) );
         XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
      }
   }
}


/*
 * Write a span of identical pixels to an XImage.  The pixel value is
 * the one set by DD.color() or DD.index().
 */
static void write_span_mono_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register unsigned long p = xmesa->pixel;
   y = FLIP( y );
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XPutPixel( img, x, y, p );
      }
   }
}


/*
 * Write a span of identical PF_TRUEDITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_TRUEDITHER_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   GLuint i;
   GLint r = xmesa->red;
   GLint g = xmesa->green;
   GLint b = xmesa->blue;
   y = FLIP( y );
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER( p, x+i, y, r, g, b);
	 XPutPixel( img, x+i, y, p );
      }
   }
}


/*
 * Write a span of identical 8A8B8G8R pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_8A8B8G8R_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLuint i, p, *ptr;
   p = (GLuint) xmesa->pixel;
   ptr = PIXELADDR4( x, y );
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = p;
      }
   }
}


/*
 * Write a span of identical 8R8G8B pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_8R8G8B_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLuint i, p, *ptr;
   p = (GLuint) xmesa->pixel;
   ptr = PIXELADDR4( x, y );
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = p;
      }
   }
}


/*
 * Write a span of identical DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_DITHER_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   int yy = FLIP(y);
   XDITHER_SETUP(yy);
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XPutPixel( img, x, yy, XDITHER( x, r, g, b ) );
      }
   }
}


/*
 * Write a span of identical 8-bit DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_DITHER8_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1(x,y);
   register GLubyte r, g, b;
   XDITHER_SETUP(y);
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 ptr[i] = XDITHER( x, r, g, b );
      }
   }
}


/*
 * Write a span of identical 8-bit LOOKUP pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_LOOKUP8_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1(x,y);
   register GLubyte pixel = xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = pixel;
      }
   }
}


/*
 * Write a span of identical PF_1BIT pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_1BIT_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XPutPixel( img, x, y, DITHER_1BIT( x, y, r, g, b ) );
      }
   }
}


/*
 * Write a span of identical HPCR pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_HPCR_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte *ptr = PIXELADDR1(x,y);
   register GLubyte r, g, b;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
         ptr[i] = DITHER_HPCR( x, y, r, g, b );
      }
   }
}



/*
 * Write a span of identical 8-bit GRAYSCALE pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_span_mono_GRAYSCALE8_ximage( MONO_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   GLuint i;
   unsigned long p = xmesa->pixel;
   GLubyte *ptr = PIXELADDR1(x,y);
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 ptr[i] = p;
      }
   }
}



/**********************************************************************/
/*** Write MONO COLOR PIXELS functions                              ***/
/**********************************************************************/

#define MONO_PIXEL_ARGS	GLcontext *ctx,					\
			GLuint n, const GLint x[], const GLint y[],	\
			const GLubyte mask[]

/*
 * Write an array of identical pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_pixmap( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc1;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_TRUEDITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_TRUEDITHER_pixmap( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], y[i], r, g, b);
         XSetForeground( dpy, gc, p );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_DITHER pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_DITHER_pixmap( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   DITHER_SETUP;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XSetForeground( dpy, gc, DITHER( x[i], y[i], r, g, b ) );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of PF_1BIT pixels to a pixmap.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_1BIT_pixmap( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         XSetForeground( dpy, gc, DITHER_1BIT( x[i], y[i], r, g, b ) );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of identical pixels to an XImage.  The pixel value is
 * the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register unsigned long p = xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]), p );
      }
   }
}


/*
 * Write an array of identical TRUEDITHER pixels to an XImage.
 * The pixel value is the one set by DD.color() or DD.index.
 */
static void write_pixels_mono_TRUEDITHER_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   int r = xmesa->red;
   int g = xmesa->green;
   int b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         unsigned long p;
         PACK_TRUEDITHER(p, x[i], FLIP(y[i]), r, g, b);
	 XPutPixel( img, x[i], FLIP(y[i]), p );
      }
   }
}



/*
 * Write an array of identical 8A8B8G8R pixels to an XImage.  The pixel value
 * is the one set by DD.color().
 */
static void write_pixels_mono_8A8B8G8R_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint p = (GLuint) xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
	 *ptr = p;
      }
   }
}


/*
 * Write an array of identical 8R8G8B pixels to an XImage.  The pixel value
 * is the one set by DD.color().
 */
static void write_pixels_mono_8R8G8B_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLuint p = (GLuint) xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLuint *ptr = PIXELADDR4( x[i], y[i] );
	 *ptr = p;
      }
   }
}


/*
 * Write an array of identical PF_DITHER pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_pixels_mono_DITHER_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   DITHER_SETUP;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]), DITHER( x[i], y[i], r, g, b ) );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_DITHER pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_DITHER8_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte r, g, b;
   DITHER_SETUP;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = DITHER( x[i], y[i], r, g, b );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_LOOKUP pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_LOOKUP8_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte pixel = xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = pixel;
      }
   }
}



/*
 * Write an array of identical PF_1BIT pixels to an XImage.  The pixel
 * value is the one set by DD.color().
 */
static void write_pixels_mono_1BIT_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   register GLubyte r, g, b;
   SETUP_1BIT;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]), DITHER_1BIT( x[i], y[i], r, g, b ));
      }
   }
}


/*
 * Write an array of identical PF_HPCR pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_HPCR_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register GLubyte r, g, b;
   r = xmesa->red;
   g = xmesa->green;
   b = xmesa->blue;
   for (i=0;i<n;i++) {
      if (mask[i]) {
         GLubyte *ptr = PIXELADDR1(x[i],y[i]);
         *ptr = DITHER_HPCR( x[i], y[i], r, g, b );
      }
   }
}


/*
 * Write an array of identical 8-bit PF_GRAYSCALE pixels to an XImage.  The
 * pixel value is the one set by DD.color().
 */
static void write_pixels_mono_GRAYSCALE8_ximage( MONO_PIXEL_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   register unsigned long p = xmesa->pixel;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 GLubyte *ptr = PIXELADDR1(x[i],y[i]);
	 *ptr = p;
      }
   }
}




/**********************************************************************/
/*** Write INDEX SPAN functions                                     ***/
/**********************************************************************/

#define INDEX_SPAN_ARGS	GLcontext *ctx,					\
			GLuint n, GLint x, GLint y, const GLuint index[], \
			const GLubyte mask[]


/*
 * Write a span of CI pixels to a Pixmap.
 */
static void write_span_index_pixmap( INDEX_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XSetForeground( dpy, gc, (unsigned long) index[i] );
	 XDrawPoint( dpy, buffer, gc, (int) x, (int) y );
      }
   }
}


/*
 * Write a span of CI pixels to an XImage.
 */
static void write_span_index_ximage( INDEX_SPAN_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
	 XPutPixel( img, x, y, (unsigned long) index[i] );
      }
   }
}



/**********************************************************************/
/*** Write INDEX PIXELS functions                                   ***/
/**********************************************************************/

#define INDEX_PIXELS_ARGS	GLcontext *ctx,				\
				GLuint n, const GLint x[], const GLint y[], \
				const GLuint index[], const GLubyte mask[]


/*
 * Write an array of CI pixels to a Pixmap.
 */
static void write_pixels_index_pixmap( INDEX_PIXELS_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   GC gc = xmesa->xm_buffer->gc2;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XSetForeground( dpy, gc, (unsigned long) index[i] );
	 XDrawPoint( dpy, buffer, gc, (int) x[i], (int) FLIP(y[i]) );
      }
   }
}


/*
 * Write an array of CI pixels to an XImage.
 */
static void write_pixels_index_ximage( INDEX_PIXELS_ARGS )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   XImage *img = xmesa->xm_buffer->backimage;
   register GLuint i;
   for (i=0;i<n;i++) {
      if (mask[i]) {
	 XPutPixel( img, x[i], FLIP(y[i]), (unsigned long) index[i] );
      }
   }
}




/**********************************************************************/
/*****                      Pixel reading                         *****/
/**********************************************************************/



/*
 * Read a horizontal span of color-index pixels.
 */
static void read_index_span( GLcontext *ctx,
			     GLuint n, GLint x, GLint y, GLuint index[] )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   int i;

   y = FLIP(y);

   if (xmesa->xm_buffer->buffer) {
      XImage *span;
      int error;
      catch_xgetimage_errors( xmesa->display );
      span = XGetImage( xmesa->display, xmesa->xm_buffer->buffer,
		        x, y, n, 1, AllPlanes, ZPixmap );
      error = check_xgetimage_errors();
      if (span && !error) {
	 for (i=0;i<n;i++) {
	    index[i] = (GLuint) XGetPixel( span, i, 0 );
	 }
      }
      else {
	 /* return 0 pixels */
	 for (i=0;i<n;i++) {
	    index[i] = 0;
	 }
      }
      if (span) {
	 XDestroyImage( span );
      }
   }
   else if (xmesa->xm_buffer->backimage) {
      XImage *img = xmesa->xm_buffer->backimage;
      for (i=0;i<n;i++,x++) {
	 index[i] = (GLuint) XGetPixel( img, x, y );
      }
   }
}



/*
 * Read a horizontal span of color pixels.
 */
static void read_color_span( GLcontext *ctx,
			     GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;

   if (xmesa->xm_buffer->buffer) {
      XImage *span;
      int error;
      catch_xgetimage_errors( xmesa->display );
      span = XGetImage( xmesa->display, xmesa->xm_buffer->buffer,
		        x, FLIP(y), n, 1, AllPlanes, ZPixmap );
      error = check_xgetimage_errors();
      if (span && !error) {
	 switch (xmesa->pixelformat) {
	    case PF_TRUECOLOR:
	    case PF_TRUEDITHER:
            case PF_5R6G5B:
               {
                  unsigned long rMask = xmesa->xm_visual->visinfo->red_mask;
                  unsigned long gMask = xmesa->xm_visual->visinfo->green_mask;
                  unsigned long bMask = xmesa->xm_visual->visinfo->blue_mask;
                  GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
                  GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
                  GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
                  GLint rShift = xmesa->xm_visual->rshift;
                  GLint gShift = xmesa->xm_visual->gshift;
                  GLint bShift = xmesa->xm_visual->bshift;
                  for (i=0;i<n;i++) {
                     unsigned long p = XGetPixel( span, i, 0 );
                     red[i]   = pixelToR[(p & rMask) >> rShift];
                     green[i] = pixelToG[(p & gMask) >> gShift];
                     blue[i]  = pixelToB[(p & bMask) >> bShift];
                     alpha[i] = 255;
                  }
               }
	       break;
	    case PF_8A8B8G8R:
               {
                  GLuint *ptr4 = (GLuint *) span->data;
                  for (i=0;i<n;i++) {
                     GLuint p4 = *ptr4++;
                     red[i]   = (GLubyte) ( p4        & 0xff);
                     green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                     blue[i]  = (GLubyte) ((p4 >> 16) & 0xff);
                     alpha[i] = (GLubyte) ((p4 >> 24) & 0xff);
                  }
	       }
	       break;
            case PF_8R8G8B:
               {
                  GLuint *ptr4 = (GLuint *) span->data;
                  for (i=0;i<n;i++) {
                     GLuint p4 = *ptr4++;
                     red[i]   = (GLubyte) ((p4 >> 16) & 0xff);
                     green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                     blue[i]  = (GLubyte) ( p4        & 0xff);
                     alpha[i] = 255;
                  }
	       }
	       break;
            case PF_HPCR:
               {
                  GLubyte *ptr1 = (GLubyte *) span->data;
                  for (i=0;i<n;i++) {
                     GLubyte p = *ptr1++;
                     red[i]   =  p & 0xE0;
                     green[i] = (p & 0x1C) << 3;
                     blue[i]  = (p & 0x03) << 6;
                     alpha[i] = 255;
                  }
               }
               break;
	    case PF_DITHER:
	    case PF_LOOKUP:
	    case PF_GRAYSCALE:
               {
                  GLubyte *red_table   = xmesa->xm_buffer->pixel_to_r;
                  GLubyte *green_table = xmesa->xm_buffer->pixel_to_g;
                  GLubyte *blue_table  = xmesa->xm_buffer->pixel_to_b;
                  if (xmesa->xm_visual->visinfo->depth==8) {
                     GLubyte *ptr1 = (GLubyte *) span->data;
                     for (i=0;i<n;i++) {
                        unsigned long p = *ptr1++;
                        red[i]   = red_table[p];
                        green[i] = green_table[p];
                        blue[i]  = blue_table[p];
                        alpha[i] = 255;
                     }
                  }
                  else {
                     for (i=0;i<n;i++) {
                        unsigned long p = XGetPixel( span, i, 0 );
                        red[i]   = red_table[p];
                        green[i] = green_table[p];
                        blue[i]  = blue_table[p];
                        alpha[i] = 255;
                     }
                  }
               }
	       break;
	    case PF_1BIT:
               {
                  int bitFlip = xmesa->xm_visual->bitFlip;
                  for (i=0;i<n;i++) {
                     unsigned long p = XGetPixel( span, i, 0 ) ^ bitFlip;
                     red[i]   = (GLubyte) (p * 255);
                     green[i] = (GLubyte) (p * 255);
                     blue[i]  = (GLubyte) (p * 255);
                     alpha[i] = 255;
                  }
               }
	       break;
	    default:
	       gl_problem(NULL,"Problem in DD.read_color_span (1)");
               return;
	 }
      }
      else {
	 /* return black pixels */
	 for (i=0;i<n;i++) {
	    red[i] = green[i] = blue[i] = alpha[i] = 0;
	 }
      }
      if (span) {
	 XDestroyImage( span );
      }
   }
   else if (xmesa->xm_buffer->backimage) {
      switch (xmesa->pixelformat) {
         case PF_TRUECOLOR:
         case PF_TRUEDITHER:
         case PF_5R6G5B:
            {
               unsigned long rMask = xmesa->xm_visual->visinfo->red_mask;
               unsigned long gMask = xmesa->xm_visual->visinfo->green_mask;
               unsigned long bMask = xmesa->xm_visual->visinfo->blue_mask;
               GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
               GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
               GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
               GLint rShift = xmesa->xm_visual->rshift;
               GLint gShift = xmesa->xm_visual->gshift;
               GLint bShift = xmesa->xm_visual->bshift;
               XImage *img = xmesa->xm_buffer->backimage;
               y = FLIP(y);
               for (i=0;i<n;i++) {
                  unsigned long p = XGetPixel( img, x+i, y );
                  red[i]   = pixelToR[(p & rMask) >> rShift];
                  green[i] = pixelToG[(p & gMask) >> gShift];
                  blue[i]  = pixelToB[(p & bMask) >> bShift];
                  alpha[i] = 255;
               }
            }
            break;
	 case PF_8A8B8G8R:
            {
               GLuint *ptr4 = PIXELADDR4( x, y );
               for (i=0;i<n;i++) {
                  GLuint p4 = *ptr4++;
                  red[i]   = (GLubyte) ( p4        & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ((p4 >> 16) & 0xff);
                  alpha[i] = (GLint)   ((p4 >> 24) & 0xff);
               }
            }
	    break;
	 case PF_8R8G8B:
            {
               GLuint *ptr4 = PIXELADDR4( x, y );
               for (i=0;i<n;i++) {
                  GLuint p4 = *ptr4++;
                  red[i]   = (GLubyte) ((p4 >> 16) & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ( p4        & 0xff);
                  alpha[i] = 255;
               }
            }
	    break;
         case PF_HPCR:
            {
               GLubyte *ptr1 = PIXELADDR1( x, y );
               for (i=0;i<n;i++) {
                  GLubyte p = *ptr1++;
                  red[i]   =  p & 0xE0;
                  green[i] = (p & 0x1C) << 3;
                  blue[i]  = (p & 0x03) << 6;
                  alpha[i] = 255;
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *red_table   = xmesa->xm_buffer->pixel_to_r;
               GLubyte *green_table = xmesa->xm_buffer->pixel_to_g;
               GLubyte *blue_table  = xmesa->xm_buffer->pixel_to_b;
               if (xmesa->xm_visual->visinfo->depth==8) {
                  GLubyte *ptr1 = PIXELADDR1(x,y);
                  for (i=0;i<n;i++) {
                     unsigned long p = *ptr1++;
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
               else {
                  XImage *img = xmesa->xm_buffer->backimage;
                  y = FLIP(y);
                  for (i=0;i<n;i++,x++) {
                     unsigned long p = XGetPixel( img, x, y );
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
            }
	    break;
	 case PF_1BIT:
            {
               XImage *img = xmesa->xm_buffer->backimage;
               int bitFlip = xmesa->xm_visual->bitFlip;
               y = FLIP(y);
               for (i=0;i<n;i++,x++) {
                  unsigned long p = XGetPixel( img, x, y ) ^ bitFlip;
                  red[i]   = (GLubyte) (p * 255);
                  green[i] = (GLubyte) (p * 255);
                  blue[i]  = (GLubyte) (p * 255);
                  alpha[i] = 255;
               }
	    }
	    break;
	 default:
	    gl_problem(NULL,"Problem in DD.read_color_span (2)");
            return;
      }
   }
}



/*
 * Read an array of color index pixels.
 */
static void read_index_pixels( GLcontext *ctx,
			       GLuint n, const GLint x[], const GLint y[],
                               GLuint indx[], const GLubyte mask[] )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   register GLuint i;
   if (xmesa->xm_buffer->buffer) {
      for (i=0;i<n;i++) {
         if (mask[i]) {
            indx[i] = (GLuint) read_pixel( xmesa->display,
                                           xmesa->xm_buffer->buffer,
                                           x[i], FLIP(y[i]) );
         }
      }
   }
   else if (xmesa->xm_buffer->backimage) {
      XImage *img = xmesa->xm_buffer->backimage;
      for (i=0;i<n;i++) {
         if (mask[i]) {
            indx[i] = (GLuint) XGetPixel( img, x[i], FLIP(y[i]) );
         }
      }
   }
}



static void read_color_pixels( GLcontext *ctx,
			       GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   Display *dpy = xmesa->xm_visual->display;
   Drawable buffer = xmesa->xm_buffer->buffer;
   register GLuint i;

   if (xmesa->xm_buffer->buffer) {
      switch (xmesa->pixelformat) {
	 case PF_TRUECOLOR:
         case PF_TRUEDITHER:
         case PF_5R6G5B:
            {
               unsigned long rMask = xmesa->xm_visual->visinfo->red_mask;
               unsigned long gMask = xmesa->xm_visual->visinfo->green_mask;
               unsigned long bMask = xmesa->xm_visual->visinfo->blue_mask;
               GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
               GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
               GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
               GLint rShift = xmesa->xm_visual->rshift;
               GLint gShift = xmesa->xm_visual->gshift;
               GLint bShift = xmesa->xm_visual->bshift;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                                   x[i], FLIP(y[i]) );
                     red[i]   = pixelToR[(p & rMask) >> rShift];
                     green[i] = pixelToG[(p & gMask) >> gShift];
                     blue[i]  = pixelToB[(p & bMask) >> bShift];
                     alpha[i] = 255;
                  }
               }
            }
            break;
	 case PF_8A8B8G8R:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( dpy, buffer,
                                                x[i], FLIP(y[i]) );
                  red[i]   = (GLubyte) ( p        & 0xff);
                  green[i] = (GLubyte) ((p >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ((p >> 16) & 0xff);
                  alpha[i] = (GLubyte) ((p >> 24) & 0xff);
               }
	    }
	    break;
	 case PF_8R8G8B:
	    for (i=0;i<n;i++) {
               if (mask[i]) {
                  unsigned long p = read_pixel( dpy, buffer,
                                                x[i], FLIP(y[i]) );
                  red[i]   = (GLubyte) ((p >> 16) & 0xff);
                  green[i] = (GLubyte) ((p >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ( p        & 0xff);
                  alpha[i] = 255;
               }
	    }
	    break;
         case PF_HPCR:
            {
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                                   x[i], FLIP(y[i]) );
                     red[i]   =  p & 0xE0;
                     green[i] = (p & 0x1C) << 3;
                     blue[i]  = (p & 0x03) << 6;
                     alpha[i] = 255;
                  }
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *red_table   = xmesa->xm_buffer->pixel_to_r;
               GLubyte *green_table = xmesa->xm_buffer->pixel_to_g;
               GLubyte *blue_table  = xmesa->xm_buffer->pixel_to_b;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                                   x[i], FLIP(y[i]) );
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 case PF_1BIT:
            {
               int bitFlip = xmesa->xm_visual->bitFlip;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = read_pixel( dpy, buffer,
                                                   x[i], FLIP(y[i])) ^ bitFlip;
                     red[i]   = (GLubyte) (p * 255);
                     green[i] = (GLubyte) (p * 255);
                     blue[i]  = (GLubyte) (p * 255);
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 default:
	    gl_problem(NULL,"Problem in DD.read_color_pixels (1)");
            return;
      }
   }
   else if (xmesa->xm_buffer->backimage) {
      switch (xmesa->pixelformat) {
	 case PF_TRUECOLOR:
         case PF_TRUEDITHER:
         case PF_5R6G5B:
            {
               unsigned long rMask = xmesa->xm_visual->visinfo->red_mask;
               unsigned long gMask = xmesa->xm_visual->visinfo->green_mask;
               unsigned long bMask = xmesa->xm_visual->visinfo->blue_mask;
               GLubyte *pixelToR = xmesa->xm_visual->PixelToR;
               GLubyte *pixelToG = xmesa->xm_visual->PixelToG;
               GLubyte *pixelToB = xmesa->xm_visual->PixelToB;
               GLint rShift = xmesa->xm_visual->rshift;
               GLint gShift = xmesa->xm_visual->gshift;
               GLint bShift = xmesa->xm_visual->bshift;
               XImage *img = xmesa->xm_buffer->backimage;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p = XGetPixel( img, x[i], FLIP(y[i]) );
                     red[i]   = pixelToR[(p & rMask) >> rShift];
                     green[i] = pixelToG[(p & gMask) >> gShift];
                     blue[i]  = pixelToB[(p & bMask) >> bShift];
                     alpha[i] = 255;
                  }
               }
            }
            break;
	 case PF_8A8B8G8R:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLuint *ptr4 = PIXELADDR4( x[i], y[i] );
                  GLuint p4 = *ptr4;
                  red[i]   = (GLubyte) ( p4        & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ((p4 >> 16) & 0xff);
                  alpha[i] = (GLubyte) ((p4 >> 24) & 0xff);
               }
	    }
	    break;
	 case PF_8R8G8B:
	    for (i=0;i<n;i++) {
	       if (mask[i]) {
                  GLuint *ptr4 = PIXELADDR4( x[i], y[i] );
                  GLuint p4 = *ptr4;
                  red[i]   = (GLubyte) ((p4 >> 16) & 0xff);
                  green[i] = (GLubyte) ((p4 >> 8)  & 0xff);
                  blue[i]  = (GLubyte) ( p4        & 0xff);
                  alpha[i] = 255;
               }
	    }
	    break;
         case PF_HPCR:
            for (i=0;i<n;i++) {
               if (mask[i]) {
                  GLubyte *ptr1 = PIXELADDR1( x[i], y[i] );
                  GLubyte p = *ptr1;
                  red[i]   =  p & 0xE0;
                  green[i] = (p & 0x1C) << 3;
                  blue[i]  = (p & 0x03) << 6;
                  alpha[i] = 255;
               }
            }
            break;
	 case PF_DITHER:
	 case PF_LOOKUP:
	 case PF_GRAYSCALE:
            {
               GLubyte *red_table   = xmesa->xm_buffer->pixel_to_r;
               GLubyte *green_table = xmesa->xm_buffer->pixel_to_g;
               GLubyte *blue_table  = xmesa->xm_buffer->pixel_to_b;
               XImage *img = xmesa->xm_buffer->backimage;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p;
                     p = XGetPixel( img, x[i], FLIP(y[i]) );
                     red[i]   = red_table[p];
                     green[i] = green_table[p];
                     blue[i]  = blue_table[p];
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 case PF_1BIT:
            {
               XImage *img = xmesa->xm_buffer->backimage;
               int bitFlip = xmesa->xm_visual->bitFlip;
               for (i=0;i<n;i++) {
                  if (mask[i]) {
                     unsigned long p;
                     p = XGetPixel( img, x[i], FLIP(y[i]) ) ^ bitFlip;
                     red[i]   = (GLubyte) (p * 255);
                     green[i] = (GLubyte) (p * 255);
                     blue[i]  = (GLubyte) (p * 255);
                     alpha[i] = 255;
                  }
               }
	    }
	    break;
	 default:
	    gl_problem(NULL,"Problem in DD.read_color_pixels (1)");
            return;
      }
   }
}


static const char *renderer_string(void)
{
   return "X11";
}


/*
 * Initialize all the DD.* function pointers depending on the color
 * buffer configuration.  This is mainly called by XMesaMakeCurrent.
 */
void xmesa_setup_DD_pointers( GLcontext *ctx )
{
   XMesaContext xmesa = (XMesaContext) ctx->DriverCtx;
   /*
    * Always the same:
    */
   ctx->Driver.RendererString = renderer_string;
   ctx->Driver.UpdateState = xmesa_setup_DD_pointers;
   ctx->Driver.GetBufferSize = get_buffer_size;
   ctx->Driver.Flush = flush;
   ctx->Driver.Finish = finish;

   ctx->Driver.SetBuffer = set_buffer;

   ctx->Driver.Index = set_index;
   ctx->Driver.Color = set_color;
   ctx->Driver.ClearIndex = clear_index;
   ctx->Driver.ClearColor = clear_color;
   ctx->Driver.IndexMask = index_mask;
   ctx->Driver.ColorMask = color_mask;
   ctx->Driver.LogicOp = logicop;
   ctx->Driver.Dither = dither;

   if (!ctx->Driver.PointsFunc)
      ctx->Driver.PointsFunc = xmesa_get_points_func( ctx );
   if (!ctx->Driver.LineFunc)
      ctx->Driver.LineFunc = xmesa_get_line_func( ctx );
   if (!ctx->Driver.TriangleFunc)
      ctx->Driver.TriangleFunc = xmesa_get_triangle_func( ctx );

   /*
    * These drawing functions depend on color buffer config:
    */
   if (xmesa->xm_buffer->buffer!=XIMAGE) {
      /* Writing to window or back pixmap */
      ctx->Driver.Clear = clear_pixmap;
      switch (xmesa->pixelformat) {
	 case PF_INDEX:
	    ctx->Driver.WriteIndexSpan       = write_span_index_pixmap;
	    ctx->Driver.WriteMonoindexSpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteIndexPixels     = write_pixels_index_pixmap;
	    ctx->Driver.WriteMonoindexPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_TRUECOLOR:
	    ctx->Driver.WriteColorSpan       = write_span_TRUECOLOR_pixmap;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteColorPixels     = write_pixels_TRUECOLOR_pixmap;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_TRUEDITHER:
	    ctx->Driver.WriteColorSpan       = write_span_TRUEDITHER_pixmap;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_TRUEDITHER_pixmap;
	    ctx->Driver.WriteColorPixels     = write_pixels_TRUEDITHER_pixmap;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_TRUEDITHER_pixmap;
	    break;
	 case PF_8A8B8G8R:
	    ctx->Driver.WriteColorSpan       = write_span_8A8B8G8R_pixmap;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteColorPixels     = write_pixels_8A8B8G8R_pixmap;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_8R8G8B:
	    ctx->Driver.WriteColorSpan       = write_span_8R8G8B_pixmap;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteColorPixels     = write_pixels_8R8G8B_pixmap;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_5R6G5B:
	    ctx->Driver.WriteColorSpan       = write_span_5R6G5B_pixmap;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_pixmap;
	    ctx->Driver.WriteColorPixels     = write_pixels_5R6G5B_pixmap;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_pixmap;
	    break;
	 case PF_DITHER:
	    ctx->Driver.WriteColorSpan       = write_span_DITHER_pixmap;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_DITHER_pixmap;
	    ctx->Driver.WriteColorPixels     = write_pixels_DITHER_pixmap;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_DITHER_pixmap;
	    break;
	 case PF_1BIT:
	    ctx->Driver.WriteColorSpan       = write_span_1BIT_pixmap;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_1BIT_pixmap;
	    ctx->Driver.WriteColorPixels     = write_pixels_1BIT_pixmap;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_1BIT_pixmap;
	    break;
         case PF_HPCR:
            ctx->Driver.WriteColorSpan       = write_span_HPCR_pixmap;
            ctx->Driver.WriteMonocolorSpan   = write_span_mono_pixmap;
            ctx->Driver.WriteColorPixels     = write_pixels_HPCR_pixmap;
            ctx->Driver.WriteMonocolorPixels = write_pixels_mono_pixmap;
            break;
         case PF_LOOKUP:
            ctx->Driver.WriteColorSpan       = write_span_LOOKUP_pixmap;
            ctx->Driver.WriteMonocolorSpan   = write_span_mono_pixmap;
            ctx->Driver.WriteColorPixels     = write_pixels_LOOKUP_pixmap;
            ctx->Driver.WriteMonocolorPixels = write_pixels_mono_pixmap;
            break;
         case PF_GRAYSCALE:
            ctx->Driver.WriteColorSpan       = write_span_GRAYSCALE_pixmap;
            ctx->Driver.WriteMonocolorSpan   = write_span_mono_pixmap;
            ctx->Driver.WriteColorPixels     = write_pixels_GRAYSCALE_pixmap;
            ctx->Driver.WriteMonocolorPixels = write_pixels_mono_pixmap;
            break;
	 default:
	    gl_problem(NULL,"Bad pixel format in xmesa_setup_DD_pointers (1)");
            return;
      }
   }
   else if (xmesa->xm_buffer->buffer==XIMAGE) {
      /* Writing to back XImage */
      if (sizeof(GLushort)!=2 || sizeof(GLuint)!=4) {
         /* Do this on Crays */
         ctx->Driver.Clear = clear_nbit_ximage;
      }
      else {
         /* Do this on most machines */
         switch (xmesa->xm_buffer->backimage->bits_per_pixel) {
            case 8:
               ctx->Driver.Clear = clear_8bit_ximage;
               break;
            case 16:
               ctx->Driver.Clear = clear_16bit_ximage;
               break;
            case 32:
               ctx->Driver.Clear = clear_32bit_ximage;
               break;
            default:
               ctx->Driver.Clear = clear_nbit_ximage;
               break;
         }
      }
      switch (xmesa->pixelformat) {
	 case PF_INDEX:
	    ctx->Driver.WriteIndexSpan       = write_span_index_ximage;
	    ctx->Driver.WriteMonoindexSpan   = write_span_mono_ximage;
	    ctx->Driver.WriteIndexPixels     = write_pixels_index_ximage;
	    ctx->Driver.WriteMonoindexPixels = write_pixels_mono_ximage;
	    break;
	 case PF_TRUECOLOR:
	    /* Generic RGB */
	    ctx->Driver.WriteColorSpan       = write_span_TRUECOLOR_ximage;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_ximage;
	    ctx->Driver.WriteColorPixels     = write_pixels_TRUECOLOR_ximage;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_ximage;
	    break;
	 case PF_TRUEDITHER:
	    ctx->Driver.WriteColorSpan       = write_span_TRUEDITHER_ximage;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_TRUEDITHER_ximage;
	    ctx->Driver.WriteColorPixels     = write_pixels_TRUEDITHER_ximage;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_TRUEDITHER_ximage;
	    break;
	 case PF_8A8B8G8R:
	    ctx->Driver.WriteColorSpan       = write_span_8A8B8G8R_ximage;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_8A8B8G8R_ximage;
	    ctx->Driver.WriteColorPixels     = write_pixels_8A8B8G8R_ximage;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_8A8B8G8R_ximage;
	    break;
	 case PF_8R8G8B:
	    ctx->Driver.WriteColorSpan       = write_span_8R8G8B_ximage;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_8R8G8B_ximage;
	    ctx->Driver.WriteColorPixels     = write_pixels_8R8G8B_ximage;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_8R8G8B_ximage;
	    break;
	 case PF_5R6G5B:
	    ctx->Driver.WriteColorSpan       = write_span_5R6G5B_ximage;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_ximage;
	    ctx->Driver.WriteColorPixels     = write_pixels_5R6G5B_ximage;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_ximage;
	    break;
	 case PF_DITHER:
	    if (xmesa->xm_visual->visinfo->depth==8) {
	       ctx->Driver.WriteColorSpan       = write_span_DITHER8_ximage;
	       ctx->Driver.WriteMonocolorSpan   = write_span_mono_DITHER8_ximage;
	       ctx->Driver.WriteColorPixels     = write_pixels_DITHER8_ximage;
	       ctx->Driver.WriteMonocolorPixels = write_pixels_mono_DITHER8_ximage;
	    }
	    else {
	       ctx->Driver.WriteColorSpan       = write_span_DITHER_ximage;
	       ctx->Driver.WriteMonocolorSpan   = write_span_mono_DITHER_ximage;
	       ctx->Driver.WriteColorPixels     = write_pixels_DITHER_ximage;
	       ctx->Driver.WriteMonocolorPixels = write_pixels_mono_DITHER_ximage;
	    }
	    break;
	 case PF_1BIT:
	    ctx->Driver.WriteColorSpan       = write_span_1BIT_ximage;
	    ctx->Driver.WriteMonocolorSpan   = write_span_mono_1BIT_ximage;
	    ctx->Driver.WriteColorPixels     = write_pixels_1BIT_ximage;
	    ctx->Driver.WriteMonocolorPixels = write_pixels_mono_1BIT_ximage;
	    break;
         case PF_HPCR:
            ctx->Driver.WriteColorSpan       = write_span_HPCR_ximage;
            ctx->Driver.WriteMonocolorSpan   = write_span_mono_HPCR_ximage;
            ctx->Driver.WriteColorPixels     = write_pixels_HPCR_ximage;
            ctx->Driver.WriteMonocolorPixels = write_pixels_mono_HPCR_ximage;
            break;
         case PF_LOOKUP:
	    if (xmesa->xm_visual->visinfo->depth==8) {
               ctx->Driver.WriteColorSpan       = write_span_LOOKUP8_ximage;
               ctx->Driver.WriteMonocolorSpan   = write_span_mono_LOOKUP8_ximage;
               ctx->Driver.WriteColorPixels     = write_pixels_LOOKUP8_ximage;
               ctx->Driver.WriteMonocolorPixels = write_pixels_mono_LOOKUP8_ximage;
            }
            else {
               ctx->Driver.WriteColorSpan       = write_span_LOOKUP_ximage;
               ctx->Driver.WriteMonocolorSpan   = write_span_mono_ximage;
               ctx->Driver.WriteColorPixels     = write_pixels_LOOKUP_ximage;
               ctx->Driver.WriteMonocolorPixels = write_pixels_mono_ximage;
            }
            break;
         case PF_GRAYSCALE:
	    if (xmesa->xm_visual->visinfo->depth==8) {
	       ctx->Driver.WriteColorSpan       = write_span_GRAYSCALE8_ximage;
	       ctx->Driver.WriteMonocolorSpan   = write_span_mono_GRAYSCALE8_ximage;
	       ctx->Driver.WriteColorPixels     = write_pixels_GRAYSCALE8_ximage;
	       ctx->Driver.WriteMonocolorPixels = write_pixels_mono_GRAYSCALE8_ximage;
	    }
	    else {
	       ctx->Driver.WriteColorSpan       = write_span_GRAYSCALE_ximage;
	       ctx->Driver.WriteMonocolorSpan   = write_span_mono_ximage;
	       ctx->Driver.WriteColorPixels     = write_pixels_GRAYSCALE_ximage;
	       ctx->Driver.WriteMonocolorPixels = write_pixels_mono_ximage;
	    }
	    break;
	 default:
	    gl_problem(NULL,"Bad pixel format in xmesa_setup_DD_pointers (2)");
            return;
      }
   }

   /* Pixel/span reading functions: */
   ctx->Driver.ReadIndexSpan = read_index_span;
   ctx->Driver.ReadColorSpan = read_color_span;
   ctx->Driver.ReadIndexPixels = read_index_pixels;
   ctx->Driver.ReadColorPixels = read_color_pixels;
}
