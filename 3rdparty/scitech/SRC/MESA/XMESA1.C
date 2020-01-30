/* $Id: xmesa1.c,v 1.44 1998/01/29 02:42:40 brianp Exp $ */

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
 * $Log: xmesa1.c,v $
 * Revision 1.44  1998/01/29 02:42:40  brianp
 * XMesaSetFXmode() now returns GLboolean
 *
 * Revision 1.43  1998/01/27 03:28:25  brianp
 * added XMesaSetFXmode()
 *
 * Revision 1.42  1997/12/11 01:38:26  brianp
 * fixed unitialized xm_buffer problem for fxmesa driver (Gary E Sanders)
 *
 * Revision 1.41  1997/12/09 00:42:07  brianp
 * added XMesaCopySubBuffer()
 *
 * Revision 1.40  1997/12/05 04:39:27  brianp
 * fixed color table reuse bug (Randy Frank)
 *
 * Revision 1.39  1997/11/07 03:48:03  brianp
 * added basic stencil buffer and accumulation buffer support for 3Dfx/Glide
 *
 * Revision 1.38  1997/11/02 20:21:07  brianp
 * if a window doesn't have a colormap, allocate one (xlockmore)
 *
 * Revision 1.37  1997/08/26 02:08:42  brianp
 * replaced MESA_FX_WINDOW/FULLSCREEN env vars with MESA_GLX_FX
 *
 * Revision 1.36  1997/08/14 01:22:18  brianp
 * renamed environment vars: MESA_FX_WINDOW and MESA_FX_FULLSCREEN
 *
 * Revision 1.35  1997/08/13 02:06:03  brianp
 * incorporated David Bucciarelli's v0.18 changes
 *
 * Revision 1.34  1997/08/11 01:23:41  brianp
 * added a pointer cast
 *
 * Revision 1.33  1997/08/02 16:40:13  brianp
 * removed unused FXhack variable
 *
 * Revision 1.32  1997/07/24 00:59:10  brianp
 * implemented FXHACK (3Dfx rendering copied into X window)
 *
 * Revision 1.31  1997/07/18 03:15:44  brianp
 * added some new assertions and error checking
 *
 * Revision 1.30  1997/07/11 23:11:40  brianp
 * added NULL testing to XMesaFlush per Matthew Rees.  Bumped version to 2.4
 *
 * Revision 1.29  1997/07/11 23:07:55  brianp
 * added XMesaGetDepthBuffer() function (Randy Frank)
 *
 * Revision 1.28  1997/06/20 02:54:37  brianp
 * hopefully fixed a bug in XMesaMakeCurrent() involving reallocated memory
 *
 * Revision 1.27  1997/05/26 21:15:37  brianp
 * now pass red/green/blue/alpha bits to gl_create_visual()
 *
 * Revision 1.26  1997/05/26 20:34:40  brianp
 * added PF_TRUEDITHER pixel format
 *
 * Revision 1.25  1997/05/17 03:43:21  brianp
 * prev pointer wasn't updated in free_xmesa_buffer() (Stephane Rehel)
 *
 * Revision 1.24  1997/05/14 03:27:48  brianp
 * added XMesaDitherColor()
 *
 * Revision 1.23  1997/04/27 03:20:13  brianp
 * added XMesaFindBuffer() and XMesaGarbageCollect
 *
 * Revision 1.22  1997/04/27 02:42:52  brianp
 * Optimized color table setup.  Free X colormap entries when finished.
 *
 * Revision 1.21  1997/04/26 04:34:12  brianp
 * new comments for XMesaCreateVisual()
 *
 * Revision 1.20  1997/04/20 20:29:11  brianp
 * replaced abort() with gl_problem()
 *
 * Revision 1.19  1997/04/12 17:13:33  brianp
 * added optimization to XMesaMakeCurrent() so rebinding current ctx is faster
 *
 * Revision 1.18  1997/04/12 16:23:23  brianp
 * made XMesa static, removed call to xmesa_setup_DD_pointers() in MakeCurrent
 *
 * Revision 1.17  1997/04/04 05:23:57  brianp
 * fixed problem with black/white being reversed on monochrome displays
 *
 * Revision 1.16  1997/01/31 20:40:46  brianp
 * in XMesaCreateVisual copy viinfo to vishandle
 *
 * Revision 1.15  1997/01/29 19:21:39  brianp
 * save a copy of the visinfo passed to XMesaCreateVisual() per Wolfram Gloger
 *
 * Revision 1.14  1997/01/03 23:56:13  brianp
 * pick a better dithered color in xmesa_color_to_pixel() per Michael Pichler
 *
 * Revision 1.13  1996/12/20 20:40:55  brianp
 * added .30 : .59 : .11 color weighting to noFaultXAllocColor()
 *
 * Revision 1.12  1996/12/03 21:40:24  brianp
 * remove the shared memory ID if shmat() fails in alloc_shm_back_buffer()
 *
 * Revision 1.11  1996/12/03 21:12:30  brianp
 * setup for 8-bit True/DirectColor no longer needs a colormap
 *
 * Revision 1.10  1996/11/21 03:42:44  brianp
 * fixed 24/32 bit per pixel bug, added check for MESA_INFO env var
 *
 * Revision 1.9  1996/11/09 01:44:27  brianp
 * noFaultXAllocColor() was reading from free()'d space!
 *
 * Revision 1.8  1996/10/22 02:44:41  brianp
 * now uses DITHER_SETUP and LOOKUP_SETUP macros
 *
 * Revision 1.7  1996/09/27 17:10:38  brianp
 * index_bits now stored in the XMesaVisual struct
 *
 * Revision 1.6  1996/09/27 01:31:29  brianp
 * removed unused variables
 *
 * Revision 1.5  1996/09/20 02:55:39  brianp
 * updated profiling code
 *
 * Revision 1.4  1996/09/19 03:42:05  brianp
 * added XMesaGetCurrentBuffer
 *
 * Revision 1.3  1996/09/19 03:16:04  brianp
 * new X/Mesa interface with XMesaContext, XMesaVisual, and XMesaBuffer types
 *
 * Revision 1.2  1996/09/15 14:21:27  brianp
 * use new GLframebuffer and GLvisual types and functions
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


/*
 * Mesa/X11 interface, part 1.
 *
 * This file contains the implementations of all the XMesa* functions.
 *
 *
 * NOTES:
 *
 * The window coordinate system origin (0,0) is in the lower-left corner
 * of the window.  X11's window coordinate origin is in the upper-left
 * corner of the window.  Therefore, most drawing functions in this
 * file have to flip Y coordinates.
 *
 * Define SHM in the Makefile with -DSHM if you want to compile in support
 * for the MIT Shared Memory extension.  If enabled, when you use an Ximage
 * for the back buffer in double buffered mode, the "swap" operation will
 * be faster.  You must also link with -lXext.
 *
 * Byte swapping:  If the Mesa host and the X display use a different
 * byte order then there's some trickiness to be aware of when using
 * XImages.  The byte ordering used for the XImage is that of the X
 * display, not the Mesa host.
 * The color-to-pixel encoding for True/DirectColor must be done
 * according to the display's visual red_mask, green_mask, and blue_mask.
 * If XPutPixel is used to put a pixel into an XImage then XPutPixel will
 * do byte swapping if needed.  If one wants to directly "poke" the pixel
 * into the XImage's buffer then the pixel must be byte swapped first.  In
 * Mesa, when byte swapping is needed we use the PF_TRUECOLOR pixel format
 * and use XPutPixel everywhere except in the implementation of
 * glClear(GL_COLOR_BUFFER_BIT).  We want this function to be fast so
 * instead of using XPutPixel we "poke" our values after byte-swapping
 * the clear pixel value if needed.
 *
 */


#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef SHM
#  include <sys/ipc.h>
#  include <sys/shm.h>
#  include <X11/extensions/XShm.h>
#endif
#include "GL/xmesa.h"
#include "xmesaP.h"
#include "context.h"
#include "macros.h"
#include "matrix.h"
#include "types.h"

#ifdef FX
#include "GL/fxmesa.h"
#define FX_WIDTH 640
#define FX_HEIGHT 480
#endif


/*
 * Current X/Mesa context pointer:
 */
static XMesaContext XMesa = NULL;


/*
 * Lookup tables for HPCR pixel format:
 */
static short hpcr_rTbl[256] = {
 16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  23,
 24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31,
 32,  32,  33,  33,  34,  34,  35,  35,  36,  36,  37,  37,  38,  38,  39,  39,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239
};

static short hpcr_gTbl[256] = {
 16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  23,
 24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31,
 32,  32,  33,  33,  34,  34,  35,  35,  36,  36,  37,  37,  38,  38,  39,  39,
 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239
};

static short hpcr_bTbl[256] = {
 32,  32,  33,  33,  34,  34,  35,  35,  36,  36,  37,  37,  38,  38,  39,  39,
 40,  40,  41,  41,  42,  42,  43,  43,  44,  44,  45,  45,  46,  46,  47,  47,
 48,  48,  49,  49,  50,  50,  51,  51,  52,  52,  53,  53,  54,  54,  55,  55,
 56,  56,  57,  57,  58,  58,  59,  59,  60,  60,  61,  61,  62,  62,  63,  63,
 64,  64,  65,  65,  66,  66,  67,  67,  68,  68,  69,  69,  70,  70,  71,  71,
 72,  72,  73,  73,  74,  74,  75,  75,  76,  76,  77,  77,  78,  78,  79,  79,
 80,  80,  81,  81,  82,  82,  83,  83,  84,  84,  85,  85,  86,  86,  87,  87,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223
};



/**********************************************************************/
/*****                     X Utility Functions                    *****/
/**********************************************************************/


/*
 * X/Mesa error reporting function:
 */
static void error( const char *msg )
{
   if (getenv("MESA_DEBUG"))
      fprintf( stderr, "X/Mesa error: %s\n", msg );
}


/*
 * Return the host's byte order as LSBFirst or MSBFirst ala X.
 */
static int host_byte_order( void )
{
   int i = 1;
   char *cptr = (char *) &i;
   return (*cptr==1) ? LSBFirst : MSBFirst;
}



/*
 * Error handling.
 */
static int mesaXErrorFlag = 0;

static int mesaHandleXError( Display *dpy, XErrorEvent *event )
{
    mesaXErrorFlag = 1;
    return 0;
}


/*
 * Check if the X Shared Memory extension is available.
 * Return:  0 = not available
 *          1 = shared XImage support available
 *          2 = shared Pixmap support available also
 */
static int check_for_xshm( Display *display )
{
#ifdef SHM
   int major, minor, ignore;
   Bool pixmaps;

   if (XQueryExtension( display, "MIT-SHM", &ignore, &ignore, &ignore )) {
      if (XShmQueryVersion( display, &major, &minor, &pixmaps )==True) {
	 return (pixmaps==True) ? 2 : 1;
      }
      else {
	 return 0;
      }
   }
   else {
      return 0;
   }
#else
   /* Can't compile XSHM support */
   return 0;
#endif
}


/*
 * Return the width and height of the given drawable.
 */
static void get_drawable_size( Display *dpy, Drawable d,
			       unsigned int *width, unsigned int *height )
{
   Window root;
   int x, y;
   unsigned int bw, depth;

   XGetGeometry( dpy, d, &root, &x, &y, width, height, &bw, &depth );
}



/*
 * Apply gamma correction to an intensity value in [0..max].  Return the
 * new intensity value.
 */
static GLint gamma_adjust( GLfloat gamma, GLint value, GLint max )
{
   double x = (double) value / (double) max;
   return (GLint) ((GLfloat) max * pow( x, 1.0F/gamma ) );
}



/*
 * Return the true number of bits per pixel for XImages.
 * For example, if we request a 24-bit deep visual we may actually
 * get a 32-bit per pixel XImage.  This function gives us this information.
 * Input:  dpy - the X display
 *         visinfo - desribes the visual to be used for XImages
 * Return:  true number of bits per pixel for XImages
 */
static int bits_per_pixel( Display *dpy, XVisualInfo *visinfo )
{
   XImage *img;
   int bitsPerPixel;
   /* Create a temporary XImage */
   img = XCreateImage( dpy, visinfo->visual, visinfo->depth,
                       ZPixmap, 0,           /*format, offset*/
                       (char*) malloc(8),    /*data*/
                       1, 1,                 /*width, height*/
                       32,                   /*bitmap_pad*/
                       0                     /*bytes_per_line*/
                     );
   assert(img);
   /* grab the bits/pixel value */
   bitsPerPixel = img->bits_per_pixel;
   /* free the XImage */
   free( img->data );
   img->data = NULL;
   XDestroyImage( img );
   return bitsPerPixel;
}



/*
 * Determine if a given X window ID is valid (window exists).
 * Do this by calling XGetWindowAttributes() for the window and
 * checking if we catch an X error.
 * Input:  dpy - the display
 *         win - the window to check for existance
 * Return:  GL_TRUE - window exists
 *          GL_FALSE - window doesn't exist
 */
static GLboolean WindowExistsFlag;

static int window_exists_err_handler( Display* dpy, XErrorEvent* xerr )
{
   if (xerr->error_code == BadWindow) {
      WindowExistsFlag = GL_FALSE;
   }
   return 0;
}

static GLboolean window_exists( Display *dpy, Window win )
{
   XWindowAttributes wa;
   int (*old_handler)( Display*, XErrorEvent* );
   WindowExistsFlag = GL_TRUE;
   old_handler = XSetErrorHandler(window_exists_err_handler);
   XGetWindowAttributes( dpy, win, &wa ); /* dummy request */
   XSetErrorHandler(old_handler);
   return WindowExistsFlag;
}



/**********************************************************************/
/*****                Linked list of XMesaBuffers                 *****/
/**********************************************************************/

static XMesaBuffer XMesaBufferList = NULL;


/* Allocate a new XMesaBuffer, add to linked list */
static XMesaBuffer alloc_xmesa_buffer(void)
{
   XMesaBuffer b = (XMesaBuffer) calloc( 1, sizeof(struct xmesa_buffer) );
   if (b) {
      b->Next = XMesaBufferList;
      XMesaBufferList = b;
   }
   return b;
}


/*
 * Find an XMesaBuffer by matching X display and colormap but NOT matching
 * the notThis buffer.
 */
static XMesaBuffer find_xmesa_buffer(Display *dpy, Colormap cmap,
                                     XMesaBuffer notThis)
{
   XMesaBuffer b;
   for (b=XMesaBufferList; b; b=b->Next) {
      if (b->display==dpy && b->cmap==cmap && b!=notThis) {
         return b;
      }
   }
   return NULL;
}


/*
 * Free an XMesaBuffer, remove from linked list, perhaps free X colormap
 * entries.
 */
static void free_xmesa_buffer(XMesaBuffer buffer)
{
   XMesaBuffer prev = NULL, b;
   for (b=XMesaBufferList; b; b=b->Next) {
      if (b==buffer) {
         /* unlink bufer from list */
         if (prev)
            prev->Next = buffer->Next;
         else
            XMesaBufferList = buffer->Next;
         /* Check to free X colors */
         if (buffer->num_alloced>0) {
            /* If no other buffer uses this X colormap then free the colors. */
            if (!find_xmesa_buffer(buffer->display, buffer->cmap, buffer)) {
               XFreeColors(buffer->display, buffer->cmap,
                           buffer->alloced_colors, buffer->num_alloced, 0);
            }
         }
         free(buffer);
         return;
      }
      /* continue search */
      prev = b;
   }
   /* buffer not found in XMesaBufferList */
   gl_problem(NULL,"free_xmesa_buffer() - buffer not found\n");
}


/* Copy X color table stuff from on XMesaBuffer to another. */
static void copy_colortable_info(XMesaBuffer dst, const XMesaBuffer src)
{
   MEMCPY(dst->color_table, src->color_table, sizeof(src->color_table));
   MEMCPY(dst->pixel_to_r, src->pixel_to_r, sizeof(src->pixel_to_r));
   MEMCPY(dst->pixel_to_g, src->pixel_to_g, sizeof(src->pixel_to_g));
   MEMCPY(dst->pixel_to_b, src->pixel_to_b, sizeof(src->pixel_to_b));
   dst->num_alloced = src->num_alloced;
   MEMCPY(dst->alloced_colors, src->alloced_colors,
          sizeof(src->alloced_colors));
}



/**********************************************************************/
/*****                   Misc Private Functions                   *****/
/**********************************************************************/


/*
 * Return number of bits set in n.
 */
static int bitcount( unsigned long n )
{
   int bits;
   for (bits=0; n>0; n=n>>1) {
      if (n&1) {
         bits++;
      }
   }
   return bits;
}



/*
 * Allocate a shared memory XImage back buffer for the given XMesaBuffer.
 * Return:  GL_TRUE if success, GL_FALSE if error
 */
static GLboolean alloc_shm_back_buffer( XMesaBuffer b )
{
#ifdef SHM
   /*
    * We have to do a _lot_ of error checking here to be sure we can
    * really use the XSHM extension.  It seems different servers trigger
    * errors at different points if the extension won't work.  Therefore
    * we have to be very careful...
    */
   GC gc;
   int (*old_handler)( Display *, XErrorEvent * );

   b->backimage = XShmCreateImage( b->xm_visual->display,
                                   b->xm_visual->visinfo->visual,
                                   b->xm_visual->visinfo->depth,
				   ZPixmap, NULL, &b->shminfo,
				   b->width, b->height );
   if (b->backimage == NULL) {
      error("alloc_back_buffer: Shared memory error (XShmCreateImage), disabling.");
      b->shm = 0;
      return GL_FALSE;
   }

   b->shminfo.shmid = shmget( IPC_PRIVATE, b->backimage->bytes_per_line
			     * b->backimage->height, IPC_CREAT|0777 );
   if (b->shminfo.shmid < 0) {
      if (getenv("MESA_DEBUG"))
          perror("alloc_back_buffer");
      XDestroyImage( b->backimage );
      b->backimage = NULL;
      error("alloc_back_buffer: Shared memory error (shmget), disabling.");
      b->shm = 0;
      return GL_FALSE;
   }

   b->shminfo.shmaddr = b->backimage->data
                      = (char*)shmat( b->shminfo.shmid, 0, 0 );
   if (b->shminfo.shmaddr == (char *) -1) {
      if (getenv("MESA_DEBUG"))
          perror("alloc_back_buffer");
      XDestroyImage( b->backimage );
      shmctl( b->shminfo.shmid, IPC_RMID, 0 );
      b->backimage = NULL;
      error("alloc_back_buffer: Shared memory error (shmat), disabling.");
      b->shm = 0;
      return GL_FALSE;
   }

   b->shminfo.readOnly = False;
   mesaXErrorFlag = 0;
   old_handler = XSetErrorHandler( mesaHandleXError );
   /* This may trigger the X protocol error we're ready to catch: */
   XShmAttach( b->xm_visual->display, &b->shminfo );
   XSync( b->xm_visual->display, False );

   if (mesaXErrorFlag) {
      /* we are on a remote display, this error is normal, don't print it */
      XFlush( b->xm_visual->display );
      mesaXErrorFlag = 0;
      XDestroyImage( b->backimage );
      shmdt( b->shminfo.shmaddr );
      shmctl( b->shminfo.shmid, IPC_RMID, 0 );
      b->backimage = NULL;
      b->shm = 0;
      (void) XSetErrorHandler( old_handler );
      return GL_FALSE;
   }

   shmctl( b->shminfo.shmid, IPC_RMID, 0 ); /* nobody else needs it */

   /* Finally, try an XShmPutImage to be really sure the extension works */
   gc = XCreateGC( b->xm_visual->display, b->frontbuffer, 0, NULL );
   XShmPutImage( b->xm_visual->display, b->frontbuffer, gc,
		 b->backimage, 0, 0, 0, 0, 1, 1 /*one pixel*/, False );
   XSync( b->xm_visual->display, False );
   XFreeGC( b->xm_visual->display, gc );
   (void) XSetErrorHandler( old_handler );
   if (mesaXErrorFlag) {
      XFlush( b->xm_visual->display );
      mesaXErrorFlag = 0;
      XDestroyImage( b->backimage );
      shmdt( b->shminfo.shmaddr );
      shmctl( b->shminfo.shmid, IPC_RMID, 0 );
      b->backimage = NULL;
      b->shm = 0;
      return GL_FALSE;
   }

   return GL_TRUE;
#else
   /* Can't compile XSHM support */
   return GL_FALSE;
#endif
}



/*
 * Setup an off-screen pixmap or Ximage to use as the back buffer.
 * Input:  b - the X/Mesa buffer
 */
void xmesa_alloc_back_buffer( XMesaBuffer b )
{
   if (b->db_state==BACK_XIMAGE) {
      /* Deallocate the old backimage, if any */
      if (b->backimage) {
#ifdef SHM
	 if (b->shm) {
	    XShmDetach( b->xm_visual->display, &b->shminfo );
	    XDestroyImage( b->backimage );
	    shmdt( b->shminfo.shmaddr );
	 }
	 else
#endif
	   XDestroyImage( b->backimage );
	 b->backimage = NULL;
      }

      /* Allocate new back buffer */
      if (b->shm==0 || alloc_shm_back_buffer(b)==GL_FALSE) {
	 /* Allocate a regular XImage for the back buffer. */
	 b->backimage = XCreateImage( b->xm_visual->display,
                                      b->xm_visual->visinfo->visual,
                                      b->xm_visual->visinfo->depth,
				      ZPixmap, 0,   /* format, offset */
				      NULL, b->width, b->height,
				      8, 0 );  /* pad, bytes_per_line */
	 if (!b->backimage) {
	    error("alloc_back_buffer: XCreateImage failed.");
	 }
         b->backimage->data = (char *) malloc( b->backimage->height
                                             * b->backimage->bytes_per_line );
         if (!b->backimage->data) {
            error("alloc_back_buffer: malloc failed.");
            XDestroyImage( b->backimage );
            b->backimage = NULL;
         }
      }
      b->backpixmap = None;
   }
   else if (b->db_state==BACK_PIXMAP) {
      Pixmap old_pixmap = b->backpixmap;
      /* Free the old back pixmap */
      if (b->backpixmap) {
	 XFreePixmap( b->xm_visual->display, b->backpixmap );
      }
      /* Allocate new back pixmap */
      b->backpixmap = XCreatePixmap( b->xm_visual->display, b->frontbuffer,
				     b->width, b->height,
                                     b->xm_visual->visinfo->depth );
      b->backimage = NULL;
      /* update other references to backpixmap */
      if (b->buffer==old_pixmap) {
	 b->buffer = b->backpixmap;
      }
   }
}



/*
 * A replacement for XAllocColor.  This function should never
 * fail to allocate a color.  When XAllocColor fails, we return
 * the nearest matching color.  If we have to allocate many colors
 * this function isn't too efficient; the XQueryColors() could be
 * done just once.
 * Written by Michael Pichler, Brian Paul, Mark Kilgard
 * Input:  dpy - X display
 *         cmap - X colormap
 *         cmapSize - size of colormap
 * In/Out: color - the XColor struct
 * Output:  exact - 1=exact color match, 0=closest match
 *          alloced - 1=XAlloc worked, 0=XAlloc failed
 */
static void
noFaultXAllocColor( Display *dpy, Colormap cmap, int cmapSize,
                    XColor *color, int *exact, int *alloced )
{
   XColor *ctable, subColor;
   int i, bestmatch;
   double mindist;       /* 3*2^16^2 exceeds long int precision. */

   /* First try just using XAllocColor. */
   if (XAllocColor(dpy, cmap, color)) {
      *exact = 1;
      *alloced = 1;
      return;
   }

   /* Alloc failed, search for closest match */

   /* Retrieve color table entries. */
   /* XXX alloca candidate. */
   ctable = (XColor *) malloc(cmapSize * sizeof(XColor));
   for (i = 0; i < cmapSize; i++) {
      ctable[i].pixel = i;
   }
   XQueryColors(dpy, cmap, ctable, cmapSize);

   /* Find best match. */
   bestmatch = -1;
   mindist = 0.0;
   for (i = 0; i < cmapSize; i++) {
      double dr = 0.30 * ((double) color->red - (double) ctable[i].red);
      double dg = 0.59 * ((double) color->green - (double) ctable[i].green);
      double db = 0.11 * ((double) color->blue - (double) ctable[i].blue);
      double dist = dr * dr + dg * dg + db * db;
      if (bestmatch < 0 || dist < mindist) {
         bestmatch = i;
         mindist = dist;
      }
   }

   /* Return result. */
   subColor.red   = ctable[bestmatch].red;
   subColor.green = ctable[bestmatch].green;
   subColor.blue  = ctable[bestmatch].blue;
   /* Try to allocate the closest match color.  This should only
    * fail if the cell is read/write.  Otherwise, we're incrementing
    * the cell's reference count.
    */
   if (XAllocColor(dpy, cmap, &subColor)) {
      *alloced = 1;
   }
   else {
      /* do this to work around a problem reported by Frank Ortega */
      subColor.pixel = (unsigned long) bestmatch;
      subColor.red   = ctable[bestmatch].red;
      subColor.green = ctable[bestmatch].green;
      subColor.blue  = ctable[bestmatch].blue;   
      subColor.flags = DoRed | DoGreen | DoBlue;
      *alloced = 0;
   }
   free(ctable);
   *color = subColor;
   *exact = 0;
}



/*
 * Do setup for PF_GRAYSCALE pixel format.
 * Note that buffer may be NULL.
 */
static GLboolean setup_grayscale( XMesaVisual v, XMesaBuffer buffer,
                                  Colormap cmap )
{
   if (v->visinfo->depth<4 || v->visinfo->depth>16) {
      return GL_FALSE;
   }

   if (buffer) {
      XMesaBuffer prevBuffer;

      if (!cmap) {
         return GL_FALSE;
      }

      prevBuffer = find_xmesa_buffer(v->display, cmap, buffer);
      if (prevBuffer &&
          (buffer->xm_visual->gl_visual->RGBAflag == 
           prevBuffer->xm_visual->gl_visual->RGBAflag)) {
         /* Copy colormap stuff from previous XMesaBuffer which uses same
          * X colormap.  Do this to avoid time spent in noFaultXAllocColor.
          */
         copy_colortable_info(buffer, prevBuffer);
      }
      else {
         /* Allocate 256 shades of gray */
         int gray;
         int colorsfailed = 0;
         for (gray=0;gray<256;gray++) {
            GLint r = gamma_adjust( v->RedGamma,   gray, 255 );
            GLint g = gamma_adjust( v->GreenGamma, gray, 255 );
            GLint b = gamma_adjust( v->BlueGamma,  gray, 255 );
            int exact, alloced;
            XColor xcol;
            xcol.red   = (r << 8) | r;
            xcol.green = (g << 8) | g;
            xcol.blue  = (b << 8) | b;
            noFaultXAllocColor( v->display, cmap, v->visinfo->colormap_size,
                                &xcol, &exact, &alloced );
            if (!exact) {
               colorsfailed++;
            }
            if (alloced) {
               assert(buffer->num_alloced<256);
               buffer->alloced_colors[buffer->num_alloced] = xcol.pixel;
               buffer->num_alloced++;
            }

            assert(gray < 576);
            buffer->color_table[gray] = xcol.pixel;
            assert(xcol.pixel < 65536);
            buffer->pixel_to_r[xcol.pixel] = gray * 30 / 100;
            buffer->pixel_to_g[xcol.pixel] = gray * 59 / 100;
            buffer->pixel_to_b[xcol.pixel] = gray * 11 / 100;
         }

         if (colorsfailed && getenv("MESA_DEBUG")) {
            fprintf( stderr,
                  "Note: %d out of 256 needed colors do not match exactly.\n",
                  colorsfailed );
         }
      }
   }

#define WEIGHT
#ifdef WEIGHT
   v->rmult = 30 * 255 / 100;
   v->gmult = 59 * 255 / 100;
   v->bmult = 11 * 255 / 100;
#else
   v->rmult = 255/3;
   v->gmult = 255/3;
   v->bmult = 255/3;
#endif
   v->dithered_pf = PF_GRAYSCALE;
   v->undithered_pf = PF_GRAYSCALE;
   return GL_TRUE;
}



/*
 * Setup RGB rendering for a window with a PseudoColor, StaticColor,
 * or 8-bit TrueColor visual visual.  We try to allocate a palette of 225
 * colors (5 red, 9 green, 5 blue) and dither to approximate a 24-bit RGB
 * color.  While this function was originally designed just for 8-bit
 * visuals, it has also proven to work from 4-bit up to 16-bit visuals.
 * Dithering code contributed by Bob Mercier.
 */
static GLboolean setup_dithered_color( XMesaVisual v, XMesaBuffer buffer,
                                       Window window, Colormap cmap )
{
   if (v->visinfo->depth<4 || v->visinfo->depth>16) {
      return GL_FALSE;
   }

   if (buffer) {
      XMesaBuffer prevBuffer;

      if (!cmap) {
         return GL_FALSE;
      }

      prevBuffer = find_xmesa_buffer(v->display, cmap, buffer);
      if (prevBuffer &&
          (buffer->xm_visual->gl_visual->RGBAflag == 
           prevBuffer->xm_visual->gl_visual->RGBAflag)) {
         /* Copy colormap stuff from previous, matching XMesaBuffer.
          * Do this to avoid time spent in noFaultXAllocColor.
          */
         copy_colortable_info(buffer, prevBuffer);
      }
      else {
         /* Allocate X colors and initialize color_table[], red_table[], etc */
         int r, g, b, i;
         int colorsfailed = 0;
         for (r = 0; r < _R; r++) {
            for (g = 0; g < _G; g++) {
               for (b = 0; b < _B; b++) {
                  XColor xcol;
                  int exact, alloced;
                  xcol.red  =gamma_adjust(v->RedGamma,   r*65535/(_R-1),65535);
                  xcol.green=gamma_adjust(v->GreenGamma, g*65535/(_G-1),65535);
                  xcol.blue =gamma_adjust(v->BlueGamma,  b*65535/(_B-1),65535);
                  noFaultXAllocColor(v->display, cmap,
                                     v->visinfo->colormap_size, &xcol, &exact,
                                     &alloced);
                  if (!exact) {
                     colorsfailed++;
                  }
                  if (alloced) {
                     assert(buffer->num_alloced<256);
                     buffer->alloced_colors[buffer->num_alloced] = xcol.pixel;
                     buffer->num_alloced++;
                  }
                  i = _MIX( r, g, b );
                  assert(i < 576);
                  buffer->color_table[i] = xcol.pixel;
                  assert(xcol.pixel < 65536);
                  buffer->pixel_to_r[xcol.pixel] = r * 255 / (_R-1);
                  buffer->pixel_to_g[xcol.pixel] = g * 255 / (_G-1);
                  buffer->pixel_to_b[xcol.pixel] = b * 255 / (_B-1);
               }
            }
         }

         if (colorsfailed && getenv("MESA_DEBUG")) {
            fprintf( stderr,
                  "Note: %d out of %d needed colors do not match exactly.\n",
                  colorsfailed, _R*_G*_B );
         }
      }
   }

   v->rmult = 255;
   v->gmult = 255;
   v->bmult = 255;
   v->dithered_pf = PF_DITHER;
   v->undithered_pf = PF_LOOKUP;
   return GL_TRUE;
}


/*
 * Setup for Hewlett Packard Color Recovery 8-bit TrueColor mode.
 * HPCR simulates 24-bit color fidelity with an 8-bit frame buffer.
 * Special dithering tables have to be initialized.
 */
static void setup_8bit_hpcr( XMesaVisual v )
{
   /* HP Color Recovery contributed by:  Alex De Bruyn (ad@lms.be)
    * To work properly, the atom _HP_RGB_SMOOTH_MAP_LIST must be defined
    * on the root window AND the colormap obtainable by XGetRGBColormaps
    * for that atom must be set on the window.  (see also tkInitWindow)
    * If that colormap is not set, the output will look stripy.
    */

   /* Setup color tables with gamma correction */
   int i;
   double g;

   g = 1.0 / v->RedGamma; 
   for (i=0; i<256; i++) { 
      GLint red = 255.0 * pow( hpcr_rTbl[i]/255.0, g ) + 0.5;
      v->hpcr_rTbl[i] = CLAMP( red, 16, 239 );
   }

   g = 1.0 / v->GreenGamma;
   for (i=0; i<256; i++) {
      GLint green = 255.0 * pow( hpcr_gTbl[i]/255.0, g ) + 0.5;
      v->hpcr_gTbl[i] = CLAMP( green, 16, 239 );
   }

   g = 1.0 / v->BlueGamma;
   for (i=0; i<256; i++) {
      GLint blue = 255.0 * pow( hpcr_bTbl[i]/255.0, g ) + 0.5;
      v->hpcr_bTbl[i] = CLAMP( blue, 32, 223 );
   }
   v->rmult = 255;
   v->gmult = 255;
   v->bmult = 255;
   v->undithered_pf = PF_HPCR;  /* can't really disable dithering for now */
   v->dithered_pf = PF_HPCR;
}



/*
 * Setup RGB rendering for a window with a True/DirectColor visual.
 */
static void setup_truecolor( XMesaVisual v, XMesaBuffer buffer,
                             Window window, Colormap cmap )
{
   unsigned long rmask, gmask, bmask;
   int bitsPerPixel;

   /* Compute red multiplier (mask) and bit shift */
   v->rshift = 0;
   rmask = v->visinfo->red_mask;
   while ((rmask & 1)==0) {
      v->rshift++;
      rmask = rmask >> 1;
   }

   /* Compute green multiplier (mask) and bit shift */
   v->gshift = 0;
   gmask = v->visinfo->green_mask;
   while ((gmask & 1)==0) {
      v->gshift++;
      gmask = gmask >> 1;
   }

   /* Compute blue multiplier (mask) and bit shift */
   v->bshift = 0;
   bmask = v->visinfo->blue_mask;
   while ((bmask & 1)==0) {
      v->bshift++;
      bmask = bmask >> 1;
   }

   /*
    * Compute component-to-pixel lookup tables and dithering kernel
    */
   {
      static GLubyte kernel[16] = {
          0*16,  8*16,  2*16, 10*16,
         12*16,  4*16, 14*16,  6*16,
          3*16, 11*16,  1*16,  9*16,
         15*16,  7*16, 13*16,  5*16,
      };
      GLint rBits = bitcount(rmask);
      GLint gBits = bitcount(gmask);
      GLint bBits = bitcount(bmask);
      GLint minBits;
      GLint i;

      /* convert pixel components in [0,_mask] to RGB values in [0,255] */
      for (i=0; i<=rmask; i++)
         v->PixelToR[i] = (i * 255) / rmask;
      for (i=0; i<=gmask; i++)
         v->PixelToG[i] = (i * 255) / gmask;
      for (i=0; i<=bmask; i++)
         v->PixelToB[i] = (i * 255) / bmask;

      /* convert RGB values from [0,255] to pixel components */
      for (i=0;i<256;i++) {
         GLint r = gamma_adjust(v->RedGamma,   i, 255);
         GLint g = gamma_adjust(v->GreenGamma, i, 255);
         GLint b = gamma_adjust(v->BlueGamma,  i, 255);
         v->RtoPixel[i] = (r >> (8-rBits)) << v->rshift;
         v->GtoPixel[i] = (g >> (8-gBits)) << v->gshift;
         v->BtoPixel[i] = (b >> (8-bBits)) << v->bshift;
      }
      /* overflow protection */
      for (i=256;i<512;i++) {
         v->RtoPixel[i] = v->RtoPixel[255];
         v->GtoPixel[i] = v->GtoPixel[255];
         v->BtoPixel[i] = v->BtoPixel[255];
      }

      /* setup dithering kernel */
      minBits = rBits;
      if (gBits < minBits)  minBits = gBits;
      if (bBits < minBits)  minBits = bBits;
      for (i=0;i<16;i++) {
         v->Kernel[i] = kernel[i] >> minBits;
      }

      v->rmult = v->gmult = v->bmult = 255;
      v->undithered_pf = PF_TRUECOLOR;
      v->dithered_pf = (v->visinfo->depth<24) ? PF_TRUEDITHER : PF_TRUECOLOR;
   }

   /*
    * Now check for TrueColor visuals which we can optimize.
    */

   bitsPerPixel = bits_per_pixel( v->display, v->visinfo );

   if (   v->visinfo->red_mask  ==0x0000ff
       && v->visinfo->green_mask==0x00ff00
       && v->visinfo->blue_mask ==0xff0000
       && host_byte_order()==ImageByteOrder(v->display)
       && bitsPerPixel==32
       && sizeof(GLuint)==4
       && v->RedGamma==1.0 && v->GreenGamma==1.0 && v->BlueGamma==1.0) {
      /* common 24-bit config used on SGI, Sun */
      v->undithered_pf = v->dithered_pf = PF_8A8B8G8R;
   }
   else if (v->visinfo->red_mask  ==0xff0000
       &&   v->visinfo->green_mask==0x00ff00
       &&   v->visinfo->blue_mask ==0x0000ff
       && host_byte_order()==ImageByteOrder(v->display)
       && bitsPerPixel==32
       && sizeof(GLuint)==4
       && v->RedGamma==1.0 && v->GreenGamma==1.0 && v->BlueGamma==1.0) {
      /* common 24-bit config used on Linux, HP, IBM */
      v->undithered_pf = v->dithered_pf = PF_8R8G8B;
   }
   else if (v->visinfo->red_mask  ==0xf800
       &&   v->visinfo->green_mask==0x07e0
       &&   v->visinfo->blue_mask ==0x001f
       && host_byte_order()==ImageByteOrder(v->display)
       && bitsPerPixel==16
       && sizeof(GLushort)==2
       && v->RedGamma==1.0 && v->GreenGamma==1.0 && v->BlueGamma==1.0) {
      /* 5-6-5 color weight on common PC VGA boards */
      v->undithered_pf = PF_5R6G5B;
      v->dithered_pf = PF_TRUEDITHER;
   }
   else if (v->visinfo->red_mask  ==0xe0
       &&   v->visinfo->green_mask==0x1c
       &&   v->visinfo->blue_mask ==0x03
       && XInternAtom(v->display, "_HP_RGB_SMOOTH_MAP_LIST", True)) {
      setup_8bit_hpcr( v );
   }
}



/*
 * Setup RGB rendering for a window with a monochrome visual.
 */
static void setup_monochrome( XMesaVisual v )
{
   v->rmult = 255;
   v->gmult = 255;
   v->bmult = 255;
   v->dithered_pf = v->undithered_pf = PF_1BIT;
   /* if black=1 then we must flip pixel values */
   v->bitFlip = (BlackPixel(v->display, v->visinfo->screen) != 0);
}



/*
 * When a context is "made current" for the first time, we can finally
 * finish initializing the context's visual and buffer information.
 * Input:  v - the XMesaVisual to initialize
 *         b - the XMesaBuffer to initialize (may be NULL)
 *         rgb_flag - TRUE = RGBA mode, FALSE = color index mode
 *         window - the window/pixmap we're rendering into
 *         cmap - the colormap associated with the window/pixmap
 * Return:  GL_TRUE=success, GL_FALSE=failure
 */
static GLboolean initialize_visual_and_buffer( XMesaVisual v,
                                               XMesaBuffer b,
                                               GLboolean rgb_flag,
                                               Window window,
                                               Colormap cmap )
{
   XGCValues gcvalues;

   if (b) {
      assert(b->xm_visual == v);
   }

   if (rgb_flag==GL_FALSE) {
      /* COLOR-INDEXED WINDOW:
       * Even if the visual is TrueColor or DirectColor we treat it as
       * being color indexed.  This is weird but might be useful to someone.
       */
      v->dithered_pf = v->undithered_pf = PF_INDEX;
      v->rmult = v->gmult = v->bmult = 0;
      v->index_bits = v->visinfo->depth;
   }
   else {
      /* RGB WINDOW:
       * We support RGB rendering into almost any kind of visual.
       */
      int xclass;
#if defined(__cplusplus) || defined(c_plusplus)
      xclass = v->visinfo->c_class;
#else
      xclass = v->visinfo->class;
#endif
      if (xclass==TrueColor || xclass==DirectColor) {
	 setup_truecolor( v, b, window, cmap );
      }
      else if (xclass==StaticGray && v->visinfo->depth==1) {
	 setup_monochrome( v );
      }
      else if (xclass==GrayScale || xclass==StaticGray) {
         if (!setup_grayscale( v, b, cmap )) {
            return GL_FALSE;
         }
      }
      else if ((xclass==PseudoColor || xclass==StaticColor)
               && v->visinfo->depth>=4 && v->visinfo->depth<=16) {
	 if (!setup_dithered_color( v, b, window, cmap )) {
            return GL_FALSE;
         }
      }
      else {
	 error("XMesa: RGB mode rendering not supported in given visual.");
	 return GL_FALSE;
      }
      v->index_bits = 0;
   }

   /*
    * If MESA_INFO env var is set print out some debugging info
    * which can help Brian figure out what's going on when a user
    * reports bugs.
    */
   if (getenv("MESA_INFO")) {
      printf("v = %p\n", v);
      printf("dithered pf = %d\n", v->dithered_pf);
      printf("undithered pf = %d\n", v->undithered_pf);
      printf("level = %d\n", v->level);
      printf("depth = %d\n", v->visinfo->depth);
      printf("bits per pixel = %d\n", bits_per_pixel(v->display, v->visinfo));
   }

   if (b && window) {
      /* Do window-specific initializations */

      /* Window dimensions */
      unsigned int w, h;
      get_drawable_size( v->display, window, &w, &h );
      b->width = w;
      b->height = h;

      b->frontbuffer = window;

      assert( v->gl_visual );

      /* Setup for single/double buffering */
      if (v->gl_visual->DBflag) {
         /* Double buffered */
         b->shm = check_for_xshm( v->display );
         xmesa_alloc_back_buffer( b );
         if (b->db_state==BACK_PIXMAP) {
            b->buffer = b->backpixmap;
         }
         else {
            b->buffer = XIMAGE;
         }
      }
      else {
         /* Single Buffered */
         b->buffer = b->frontbuffer;
      }

      /* X11 graphics contexts */
      b->gc1 = XCreateGC( v->display, window, 0, NULL );
      XSetFunction( v->display, b->gc1, GXcopy );
      b->gc2 = XCreateGC( v->display, window, 0, NULL );
      XSetFunction( v->display, b->gc2, GXcopy );
      /*
       * Don't generate Graphics Expose/NoExpose events in swapbuffers().
       * Patch contributed by Michael Pichler May 15, 1995.
       */
      gcvalues.graphics_exposures = False;
      b->cleargc = XCreateGC( v->display, window,
                              GCGraphicsExposures, &gcvalues);
      XSetFunction( v->display, b->cleargc, GXcopy );

      /* Initialize the row buffer XImage for use in write_color_span() */
      b->rowimage = XCreateImage( v->display,
                                  v->visinfo->visual,
                                  v->visinfo->depth,
                                  ZPixmap, 0,           /*format, offset*/
                                  (char*) malloc(MAX_WIDTH*4),  /*data*/
                                  MAX_WIDTH, 1,         /*width, height*/
                                  32,                   /*bitmap_pad*/
                                  0                     /*bytes_per_line*/ );
   }

   return GL_TRUE;
}



/*
 * Convert an RGBA color to a pixel value.
 */
unsigned long xmesa_color_to_pixel( XMesaContext xmesa,
                                    GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
   switch (xmesa->pixelformat) {
      case PF_INDEX:
         return 0;
      case PF_TRUECOLOR:
         {
            unsigned long p;
            PACK_TRUECOLOR( p, r, g, b );
            return p;
         }
      case PF_8A8B8G8R:
         return PACK_8A8B8G8R( r, g, b, a );
      case PF_8R8G8B:
         return PACK_8R8G8B( r, g, b );
      case PF_5R6G5B:
         return PACK_5R6G5B( r, g, b );
      case PF_DITHER:
         {
            DITHER_SETUP;
            return DITHER( 1, 0, r, g, b );
         }
      case PF_1BIT:
         /* 382 = (3*255)/2 */
         return ((r+g+b) > 382U) ^ xmesa->xm_visual->bitFlip;
      case PF_HPCR:
         return DITHER_HPCR(1, 1, r, g, b);
      case PF_LOOKUP:
         {
            LOOKUP_SETUP;
            return LOOKUP( r, g, b );
         }
      case PF_GRAYSCALE:
         return GRAY_RGB( r, g, b );
      case PF_TRUEDITHER:
         {
            unsigned long p;
            PACK_TRUEDITHER(p, 1, 0, r, g, b);
            return p;
         }
      default:
         gl_problem(NULL, "Bad pixel format in xmesa_color_to_pixel");
         return 0;
   }
   return 0;  /*never get here*/
}


/**********************************************************************/
/*****                       Public Functions                     *****/
/**********************************************************************/


/*
 * Create a new X/Mesa visual.
 * Input:  display - the X display
 *         visinfo - the XVisualInfo
 *         rgb_flag - TRUE=RGB(A) mode, FALSE=CI mode
 *         alpha_flag - need alpha planes?
 *         db_flag - TRUE=double bufferd, FALSE=single
 *         ximage_flag - TRUE=use XImage for back buffer, FALSE=use Pixmap
 *         depth_size - requested min bits per depth buffer value
 *         stencil_size - requested min bits per stencil buffer value
 *         accum_size - requested min bits per accum buffer value (per channel)
 *         level - 0=normal, 1=overaly, -1=underlay, etc.
 * Return:  New XMesaVisual or NULL if something goes wrong
 */
XMesaVisual XMesaCreateVisual( Display *display,
                               XVisualInfo *visinfo,
                               GLboolean rgb_flag,
                               GLboolean alpha_flag,
                               GLboolean db_flag,
                               GLboolean ximage_flag,
                               GLint depth_size,
                               GLint stencil_size,
                               GLint accum_size,
                               GLint level )
{
   char *gamma;
   XMesaVisual v;
   GLfloat red_scale, green_scale, blue_scale, alpha_scale;
   GLint red_bits, green_bits, blue_bits, alpha_bits;

   /* For debugging only */
   if (getenv("MESA_XSYNC")) {
      XSynchronize( display, 1 );    /* This makes debugging X easier */
   }

   v = (XMesaVisual) calloc( 1, sizeof(struct xmesa_visual) );
   if (!v) {
      return NULL;
   }

   v->display = display;

   /* Save a copy of the XVisualInfo struct because the user may XFree()
    * the struct but we may need some of the information contained in it
    * at a later time.
    */
   v->visinfo = (XVisualInfo *) malloc(sizeof(*visinfo));
   if(!v->visinfo) {
      free(v);
      return NULL;
   }
   MEMCPY(v->visinfo, visinfo, sizeof(*visinfo));

   /* Save a copy of the pointer now so we can find this visual again
    * if we need to search for it in find_glx_visual().
    */
   v->vishandle = visinfo;

   /* check for MESA_GAMMA environment variable */
   gamma = getenv("MESA_GAMMA");
   if (gamma) {
      v->RedGamma = v->GreenGamma = v->BlueGamma = 0.0;
      sscanf( gamma, "%f %f %f", &v->RedGamma, &v->GreenGamma, &v->BlueGamma );
      if (v->RedGamma<=0.0)    v->RedGamma = 1.0;
      if (v->GreenGamma<=0.0)  v->GreenGamma = v->RedGamma;
      if (v->BlueGamma<=0.0)   v->BlueGamma = v->RedGamma;
   }
   else {
      v->RedGamma = v->GreenGamma = v->BlueGamma = 1.0;
   }

   v->ximage_flag = ximage_flag;
   v->level = level;

   (void) initialize_visual_and_buffer( v, NULL, rgb_flag, 0, 0 );

   red_scale   = (GLfloat) v->rmult;
   green_scale = (GLfloat) v->gmult;
   blue_scale  = (GLfloat) v->bmult;
   alpha_scale = 255.0;

   {
      int xclass;
#if defined(__cplusplus) || defined(c_plusplus)
      xclass = v->visinfo->c_class;
#else
      xclass = v->visinfo->class;
#endif
      if (xclass==TrueColor || xclass==DirectColor) {
         red_bits   = bitcount(v->visinfo->red_mask);
         green_bits = bitcount(v->visinfo->green_mask);
         blue_bits  = bitcount(v->visinfo->blue_mask);
         alpha_bits = 0;
      }
      else {
         /* this is an approximation */
         int depth = v->visinfo->depth;
         red_bits = depth / 3;
         depth -= red_bits;
         green_bits = depth / 2;
         depth -= green_bits;
         blue_bits = depth;
         alpha_bits = 0;
         assert( red_bits + green_bits + blue_bits == v->visinfo->depth );
      }
   }

   v->gl_visual = gl_create_visual( rgb_flag, alpha_flag, db_flag,
                                    depth_size, stencil_size, accum_size,
                                    v->index_bits,
                                    red_scale, green_scale,
                                    blue_scale, alpha_scale,
                                    red_bits, green_bits,
                                    blue_bits, alpha_bits );
   if (!v->gl_visual) {
      free(v->visinfo);
      free(v);
      return NULL;
   }

   return v;
}



void XMesaDestroyVisual( XMesaVisual v )
{
   gl_destroy_visual( v->gl_visual );
   free(v->visinfo);
   free(v);
}



/*
 * Create a new XMesaContext.
 * Input:  v - XMesaVisual
 *         share_list - another XMesaContext with which to share display
 *                      lists or NULL if no sharing is wanted.
 * Return:  an XMesaContext or NULL if error.
 */
XMesaContext XMesaCreateContext( XMesaVisual v, XMesaContext share_list )
{
   XMesaContext c;
   char *fxEnvVar;

   c = (XMesaContext) calloc( 1, sizeof(struct xmesa_context) );
   if (!c) {
      return NULL;
   }

   c->gl_ctx = gl_create_context( v->gl_visual,
                                  share_list ? share_list->gl_ctx : NULL,
                                  (void *) c );
   if (!c->gl_ctx) {
      free(c);
      return NULL;
   }

   if (host_byte_order()==ImageByteOrder(v->display)) {
      c->swapbytes = GL_FALSE;
   }
   else {
      c->swapbytes = GL_TRUE;
   }

   c->xm_visual = v;
   c->xm_buffer = NULL;   /* set later by XMesaMakeCurrent */
   c->display = v->display;
   c->pixelformat = v->dithered_pf;      /* Dithering is enabled by default */

   c->gl_ctx->Driver.UpdateState = xmesa_setup_DD_pointers;

#ifdef FX
   fxEnvVar = getenv("MESA_GLX_FX");
   if (fxEnvVar) {
      int attribs[100];
      int numAttribs = 0;
      if (v->gl_visual->DepthBits > 0) {
         attribs[numAttribs++] = FXMESA_DEPTH_SIZE;
         attribs[numAttribs++] = 1;
      }
      if (v->gl_visual->DBflag) {
         attribs[numAttribs++] = FXMESA_DOUBLEBUFFER;
      }
      if (v->gl_visual->AccumBits > 0) {
         attribs[numAttribs++] = FXMESA_ACCUM_SIZE;
         attribs[numAttribs++] = v->gl_visual->AccumBits;
      }
      if (v->gl_visual->StencilBits > 0) {
         attribs[numAttribs++] = FXMESA_STENCIL_SIZE;
         attribs[numAttribs++] = v->gl_visual->StencilBits;
      }
      if (v->gl_visual->AlphaBits > 0) {
         attribs[numAttribs++] = FXMESA_ALPHA_SIZE;
         attribs[numAttribs++] = 1;
      }
      attribs[numAttribs++] = FXMESA_NONE;
      c->FXctx = fxMesaCreateBestContext(0, FX_WIDTH, FX_HEIGHT, attribs);

      if (v->undithered_pf==PF_5R6G5B) {
         c->FXisHackUsable = c->FXctx ? GL_TRUE : GL_FALSE;

         if (tolower(fxEnvVar[0])=='w')
            c->FXwindowHack = c->FXctx ? GL_TRUE : GL_FALSE;
         else
            c->FXwindowHack = GL_FALSE;
      }
      else {
         c->FXisHackUsable = GL_FALSE;
         c->FXwindowHack = GL_FALSE;
      }
   }
#endif

   return c;
}




void XMesaDestroyContext( XMesaContext c )
{
#ifdef FX
   if (c->FXctx)
      fxMesaDestroyContext(c->FXctx);
#endif
   if (c->gl_ctx)
      gl_destroy_context( c->gl_ctx );
   free( c );
}



/*
 * Create a new XMesaBuffer from an X window.
 * Input:  v - the XMesaVisual
 *         W - the window
 * Return:  new XMesaBuffer or NULL if error
 */
XMesaBuffer XMesaCreateWindowBuffer( XMesaVisual v, Window w )
{
   XWindowAttributes attr;
   XMesaBuffer b = alloc_xmesa_buffer();
   if (!b) {
      return NULL;
   }

   assert(v);

   XGetWindowAttributes( v->display, w, &attr );

   if (v->visinfo->depth != attr.depth) {
      if (getenv("MESA_DEBUG")) {
         fprintf(stderr, "XMesaCreateWindowBuffer: depth mismatch between visual and window!\n");
      }
      return NULL;
   }

   b->xm_visual = v;
   b->pixmap_flag = GL_FALSE;
   b->display = v->display;
   if (attr.colormap) {
      b->cmap = attr.colormap;
   }
   else {
      if (getenv("MESA_DEBUG")) {
         fprintf(stderr, "Window %ld has no colormap!\n", w);
      }
      /* this is weird, a window w/out a colormap!? */
      /* OK, let's just allocate a new one and hope for the best */
      b->cmap = XCreateColormap(v->display, w, attr.visual, AllocNone);
   }

   /* determine back buffer implementation */
   if (v->gl_visual->DBflag) {
      if (v->ximage_flag) {
	 b->db_state = BACK_XIMAGE;
      }
      else {
	 b->db_state = BACK_PIXMAP;
      }
   }
   else {
      b->db_state = 0;
   }

   b->gl_buffer = gl_create_framebuffer( v->gl_visual );
   if (!b->gl_buffer) {
      free_xmesa_buffer(b);
      return NULL;
   }

   if (!initialize_visual_and_buffer( v, b, v->gl_visual->RGBAflag,
                                      w, b->cmap )) {
      gl_destroy_framebuffer( b->gl_buffer );
      free_xmesa_buffer(b);
      return NULL;
   }

   return b;
}



/*
 * Create a new XMesaBuffer from an X pixmap.
 * Input:  v - the XMesaVisual
 *         p - the pixmap
 *         cmap - the colormap, may be 0 if using a TrueColor or DirectColor
 *                visual for the pixmap
 * Return:  new XMesaBuffer or NULL if error
 */
XMesaBuffer XMesaCreatePixmapBuffer( XMesaVisual v, Pixmap p, Colormap cmap )
{
   XMesaBuffer b = alloc_xmesa_buffer();
   if (!b) {
      return NULL;
   }

   assert(v);

   b->xm_visual = v;
   b->pixmap_flag = GL_TRUE;
   b->display = v->display;
   b->cmap = cmap;

   /* determine back buffer implementation */
   if (v->gl_visual->DBflag) {
      if (v->ximage_flag) {
	 b->db_state = BACK_XIMAGE;
      }
      else {
	 b->db_state = BACK_PIXMAP;
      }
   }
   else {
      b->db_state = 0;
   }

   b->gl_buffer = gl_create_framebuffer( v->gl_visual );
   if (!b->gl_buffer) {
      free_xmesa_buffer(b);
      return NULL;
   }

   if (!initialize_visual_and_buffer(v, b, v->gl_visual->RGBAflag, p, cmap)) {
      gl_destroy_framebuffer( b->gl_buffer );
      free_xmesa_buffer(b);
      return NULL;
   }

   return b;
}



/*
 * Deallocate an XMesaBuffer structure and all related info.
 */
void XMesaDestroyBuffer( XMesaBuffer b )
{
   if (b->gc1)  XFreeGC( b->xm_visual->display, b->gc1 );
   if (b->gc2)  XFreeGC( b->xm_visual->display, b->gc2 );
   if (b->cleargc)  XFreeGC( b->xm_visual->display, b->cleargc );

   if (b->backimage) {
#ifdef SHM
       if (b->shm) {
	   XShmDetach( b->xm_visual->display, &b->shminfo );
	   XDestroyImage( b->backimage );
	   shmdt( b->shminfo.shmaddr );
       }
       else
#endif
	   XDestroyImage( b->backimage );
   }
   if (b->backpixmap) {
      XFreePixmap( b->xm_visual->display, b->backpixmap );
   }
   if (b->rowimage) {
      free( b->rowimage->data );
      b->rowimage->data = NULL;
      XDestroyImage( b->rowimage );
   }

   gl_destroy_framebuffer( b->gl_buffer );
   free_xmesa_buffer(b);
}



/*
 * Bind buffer b to context c and make c the current rendering context.
 */
GLboolean XMesaMakeCurrent( XMesaContext c, XMesaBuffer b )
{
   if ((c && !b) || (!c && b)) {
      return GL_FALSE;
   }

   if (c) {
#ifdef FX
      if (c->FXctx) {
         fxMesaMakeCurrent(c->FXctx);
         XMesa = c;
         c->xm_buffer = b;
         return GL_TRUE;
      }
#endif
      if (c->gl_ctx == gl_get_current_context() && c->xm_buffer == b
          && c->xm_buffer->wasCurrent) {
         /* same context and buffer, do nothing */
         return GL_TRUE;
      }

      c->xm_buffer = b;

      gl_make_current( c->gl_ctx, b->gl_buffer );
      XMesa = c;

      if (c->gl_ctx->Viewport.Width==0) {
	 /* initialize viewport to window size */
	 gl_Viewport( c->gl_ctx, 0, 0, b->width, b->height );
	 c->gl_ctx->Scissor.Width = b->width;
	 c->gl_ctx->Scissor.Height = b->height;
      }

      if (c->xm_visual->gl_visual->RGBAflag) {
         /*
          * Must recompute and set these pixel values because colormap
          * can be different for different windows.
          */
         c->pixel = xmesa_color_to_pixel( c, c->red, c->green,
                                          c->blue, c->alpha );
         XSetForeground( c->display, c->xm_buffer->gc1, c->pixel );
         c->clearpixel = xmesa_color_to_pixel( c,
                                               c->clearcolor[0],
                                               c->clearcolor[1],
                                               c->clearcolor[2],
                                               c->clearcolor[3] );
         XSetForeground( c->display, c->xm_buffer->cleargc, c->clearpixel );
      }

      /* Solution to Stephane Rehel's problem with glXReleaseBuffersMESA(): */
      c->xm_buffer->wasCurrent = GL_TRUE;
   }
   else {
      /* Detach */
      gl_make_current( NULL, NULL );
      XMesa = NULL;
   }
   return GL_TRUE;
}



XMesaContext XMesaGetCurrentContext( void )
{
   return XMesa;
}



XMesaBuffer XMesaGetCurrentBuffer( void )
{
   if (XMesa) {
      return XMesa->xm_buffer;
   }
   else {
      return 0;
   }
}


#ifdef FX
#include <glide.h>
#endif


/*
 * Switch 3Dfx support hack between window and full-screen mode.
 */
GLboolean XMesaSetFXmode( GLint mode )
{
#ifdef FX
   if (XMesa) {
      if (mode == XMESA_FX_WINDOW) {
         if (XMesa->FXisHackUsable) {
            grSstControl(GR_CONTROL_DEACTIVATE);
            XMesa->FXwindowHack = GL_TRUE;
            return GL_TRUE;
         }
      }
      else if (mode == XMESA_FX_FULLSCREEN) {
         grSstControl(GR_CONTROL_ACTIVATE);
         XMesa->FXwindowHack = GL_FALSE;
         return GL_TRUE;
      }
      else {
         /* Error: Bad mode value */
      }
   }
#endif
   return GL_FALSE;
}



#ifdef FX
/*
 * Read image from VooDoo frame buffer into X/Mesa's back XImage.
 */
static void FXgetImage( XMesaBuffer b )
{
   int x, y;
   Window root;
   unsigned int bw, depth, width, height;
   assert(XMesa->FXctx);
   XGetGeometry( XMesa->xm_visual->display, b->frontbuffer,
                 &root, &x, &y, &width, &height, &bw, &depth);
   if (b->width != width || b->height != height) {
      b->width = MIN2(width, FX_WIDTH);
      b->height = MIN2(height, FX_HEIGHT);
      if (b->width & 1)
         b->width--;  /* prevent odd width */
      xmesa_alloc_back_buffer( b );
   }

   grLfbReadRegion( GR_BUFFER_FRONTBUFFER,       /* src buffer */
                    0, FX_HEIGHT - b->height,    /* position */
                    b->width, b->height,         /* size */
                    b->width * sizeof(GLushort), /* stride*/
                    b->backimage->data);         /* dest buffer */
}
#endif


/*
 * Copy the back buffer to the front buffer.  If there's no back buffer
 * this is a no-op.
 */
void XMesaSwapBuffers( XMesaBuffer b )
{
#ifdef PROFILE
   GLdouble t0 = gl_time();
#endif

   if (b->db_state) {
#ifdef FX
      if (XMesa->FXctx) {
         fxMesaSwapBuffers();

         if (XMesa->FXwindowHack)
            FXgetImage(b);
         else
            return;
      }
#endif
     if (b->backimage) {
	 /* Copy Ximage from host's memory to server's window */
#ifdef SHM
	 if (b->shm) {
	    XShmPutImage( b->xm_visual->display, b->frontbuffer,
			  b->cleargc,
			  b->backimage, 0, 0,
			  0, 0, b->width, b->height, False );
	    /* wait for finished event??? */
	 }
	 else
#endif
         {
            XPutImage( b->xm_visual->display, b->frontbuffer,
                       b->cleargc,
                       b->backimage, 0, 0,
                       0, 0, b->width, b->height );
         }
      }
      else {
	 /* Copy pixmap to window on server */
	 XCopyArea( b->xm_visual->display,
		    b->backpixmap,   /* source drawable */
		    b->frontbuffer,  /* dest. drawable */
		    b->cleargc,
		    0, 0, b->width, b->height,  /* source region */
		    0, 0                 /* dest region */
		   );
      }
   }
   XSync( b->xm_visual->display, False );
#ifdef PROFILE
   XMesa->gl_ctx->SwapCount++;
   XMesa->gl_ctx->SwapTime += gl_time() - t0;
#endif
}



/*
 * Copy sub-region of back buffer to front buffer
 */
void XMesaCopySubBuffer( XMesaBuffer b, int x, int y, int width, int height )
{
#ifdef PROFILE
   GLdouble t0 = gl_time();
#endif

   if (b->db_state) {
      int yTop = b->bottom - y - height;
#ifdef FX
      if (XMesa->FXctx) {
         fxMesaSwapBuffers();
         if (XMesa->FXwindowHack)
            FXgetImage(b);
         else
            return;
      }
#endif
      if (b->backimage) {
         /* Copy Ximage from host's memory to server's window */
#ifdef SHM
         if (b->shm) {
            /* XXX assuming width and height aren't too large! */
            XShmPutImage( b->xm_visual->display, b->frontbuffer,
                          b->cleargc,
                          b->backimage, x, yTop,
                          x, yTop, width, height, False );
            /* wait for finished event??? */
         }
         else
#endif
         {
            /* XXX assuming width and height aren't too large! */
            XPutImage( b->xm_visual->display, b->frontbuffer,
                       b->cleargc,
                       b->backimage, x, yTop,
                       x, yTop, width, height );
         }
      }
      else {
         /* Copy pixmap to window on server */
         XCopyArea( b->xm_visual->display,
                    b->backpixmap,           /* source drawable */
                    b->frontbuffer,          /* dest. drawable */
                    b->cleargc,
                    x, yTop, width, height,  /* source region */
                    x, yTop                  /* dest region */
                    );
      }
   }
   XSync( b->xm_visual->display, False );
#ifdef PROFILE
   XMesa->gl_ctx->SwapCount++;
   XMesa->gl_ctx->SwapTime += gl_time() - t0;
#endif
}


/*
 * Return a pointer to the XMesa backbuffer Pixmap or XImage.  This function
 * is a way to get "under the hood" of X/Mesa so one can manipulate the
 * back buffer directly.
 * Output:  pixmap - pointer to back buffer's Pixmap, or 0
 *          ximage - pointer to back buffer's XImage, or NULL
 * Return:  GL_TRUE = context is double buffered
 *          GL_FALSE = context is single buffered
 */
GLboolean XMesaGetBackBuffer( XMesaBuffer b, Pixmap *pixmap, XImage **ximage )
{
   if (b->db_state) {
      if (pixmap)  *pixmap = b->backpixmap;
      if (ximage)  *ximage = b->backimage;
      return GL_TRUE;
   }
   else {
      *pixmap = 0;
      *ximage = NULL;
      return GL_FALSE;
   }
}


/*
 * Return the depth buffer associated with an XMesaBuffer.
 * Input:  b - the XMesa buffer handle
 * Output:  width, height - size of buffer in pixels
 *          bytesPerValue - bytes per depth value (2 or 4)
 *          buffer - pointer to depth buffer values
 * Return:  GL_TRUE or GL_FALSE to indicate success or failure.
 */
GLboolean XMesaGetDepthBuffer( XMesaBuffer b, GLint *width, GLint *height,
                               GLint *bytesPerValue, void **buffer )
{
   if ((!b->gl_buffer) || (!b->gl_buffer->Depth)) {
      *width = 0;
      *height = 0;
      *bytesPerValue = 0;
      *buffer = 0;
      return GL_FALSE;
   }
   else {
      *width = b->gl_buffer->Width;
      *height = b->gl_buffer->Height;
      *bytesPerValue = sizeof(GLdepth);
      *buffer = b->gl_buffer->Depth;
      return GL_TRUE;
   }
}


void XMesaFlush( XMesaContext c )
{
   if (c && c->xm_visual) {
      XSync( c->xm_visual->display, False );
   }
}



const char *XMesaGetString( XMesaContext c, int name )
{
   if (name==XMESA_VERSION) {
      return "2.4";
   }
   else if (name==XMESA_EXTENSIONS) {
      return "";
   }
   else {
      return NULL;
   }
}



XMesaBuffer XMesaFindBuffer( Display *dpy, Drawable d )
{
   XMesaBuffer b;
   for (b=XMesaBufferList; b; b=b->Next) {
      if (b->display==dpy && b->frontbuffer==d) {
         return b;
      }
   }
   return NULL;
}



/*
 * Look for XMesaBuffers whose X window has been destroyed.
 * Deallocate any such XMesaBuffers.
 */
void XMesaGarbageCollect( void )
{
   XMesaBuffer b, next;
   for (b=XMesaBufferList; b; b=next) {
      next = b->Next;
      if (!b->pixmap_flag) {
         XSync(b->display, False);
         if (!window_exists( b->display, b->frontbuffer )) {
            /* found a dead window, free the ancillary info */
            XMesaDestroyBuffer( b );
         }
      }
   }
}



unsigned long XMesaDitherColor( XMesaContext xmesa, GLint x, GLint y,
                                GLfloat red, GLfloat green,
                                GLfloat blue, GLfloat alpha )
{
   GLint r = (GLint) (red   * (GLfloat) xmesa->xm_visual->rmult);
   GLint g = (GLint) (green * (GLfloat) xmesa->xm_visual->gmult);
   GLint b = (GLint) (blue  * (GLfloat) xmesa->xm_visual->bmult);
   GLint a = (GLint) (alpha * 255.0F);

   switch (xmesa->pixelformat) {
      case PF_INDEX:
         return 0;
      case PF_TRUECOLOR:
         {
            unsigned long p;
            PACK_TRUECOLOR( p, r, g, b );
            return p;
         }
      case PF_8A8B8G8R:
         return PACK_8A8B8G8R( r, g, b, a );
      case PF_8R8G8B:
         return PACK_8R8G8B( r, g, b );
      case PF_5R6G5B:
         return PACK_5R6G5B( r, g, b );
      case PF_DITHER:
         {
            DITHER_SETUP;
            return DITHER( x, y, r, g, b );
         }
      case PF_1BIT:
         /* 382 = (3*255)/2 */
         return ((r+g+b) > 382U) ^ xmesa->xm_visual->bitFlip;
      case PF_HPCR:
         return DITHER_HPCR(x, y, r, g, b);
      case PF_LOOKUP:
         {
            LOOKUP_SETUP;
            return LOOKUP( r, g, b );
         }
      case PF_GRAYSCALE:
         return GRAY_RGB( r, g, b );
      case PF_TRUEDITHER:
         {
            unsigned long p;
            PACK_TRUEDITHER(p, x, y, r, g, b);
            return p;
         }
      default:
         gl_problem(NULL, "Bad pixel format in XMesaDitherColor");
         return 0;
   }
   return 0;  /*never get here*/
}


