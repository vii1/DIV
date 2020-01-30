/* $Id: osmesa.h,v 1.8 1997/12/07 17:28:03 brianp Exp $ */

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
 * $Log: osmesa.h,v $
 * Revision 1.8  1997/12/07 17:28:03  brianp
 * updated version to 2.6
 *
 * Revision 1.7  1997/09/29 23:22:16  brianp
 * updated version to 2.5
 *
 * Revision 1.6  1997/07/11 23:07:16  brianp
 * added OSMesaGetDepthBuffer function (Randy Frank)
 *
 * Revision 1.5  1997/02/19 10:13:54  brianp
 * now test for __QUICKDRAW__ like for __BEOS__ (Randy Frank)
 *
 * Revision 1.4  1997/02/10 20:31:41  brianp
 * added OSMESA_RGB and OSMESA_BGR, per Randy Frank
 *
 * Revision 1.3  1997/02/03 20:07:40  brianp
 * patches for BeOS
 *
 * Revision 1.2  1996/10/30 03:12:30  brianp
 * incremented version to 2.1
 *
 * Revision 1.1  1996/09/13 01:26:41  brianp
 * Initial revision
 *
 */


/*
 * Mesa Off-Screen rendering interface.
 *
 * This is an operating system and window system independent interface to
 * Mesa which allows one to render images into a client-supplied buffer in
 * main memory.  Such images may manipulated or saved in whatever way the
 * client wants.
 *
 * These are the API functions:
 *   OSMesaCreateContext - create a new Off-Screen Mesa rendering context
 *   OSMesaMakeCurrent - bind an OSMesaContext to a client's image buffer
 *                       and make the specified context the current one.
 *   OSMesaDestroyContext - destroy an OSMesaContext
 *   OSMesaGetCurrentContext - return thread's current context ID
 *   OSMesaPixelStore - controls how pixels are stored in image buffer
 *   OSMesaGetIntegerv - return OSMesa state parameters
 *
 *
 * The limits on the width and height of an image buffer are MAX_WIDTH and
 * MAX_HEIGHT as defined in Mesa/src/config.h.  Defaults are 1280 and 1024.
 * You can increase them as needed but beware that many temporary arrays in
 * Mesa are dimensioned by MAX_WIDTH or MAX_WIDTH.
 */



#ifndef OSMESA_H
#define OSMESA_H



#ifdef __cplusplus
extern "C" {
#endif


#include "GL/gl.h"



#define OSMESA_MAJOR_VERSION 2
#define OSMESA_MINOR_VERSION 6



/*
 * Values for the format parameter of OSMesaCreateContext()
 * New in version 2.0.
 */
#define OSMESA_COLOR_INDEX	GL_COLOR_INDEX
#define OSMESA_RGBA		GL_RGBA
#define OSMESA_BGRA		0x1
#define OSMESA_ARGB		0x2
#define OSMESA_RGB		GL_RGB
#define OSMESA_BGR		0x4


/*
 * OSMesaPixelStore() parameters:
 * New in version 2.0.
 */
#define OSMESA_ROW_LENGTH	0x10
#define OSMESA_Y_UP		0x11


/*
 * Accepted by OSMesaGetIntegerv:
 */
#define OSMESA_WIDTH		0x20
#define OSMESA_HEIGHT		0x21
#define OSMESA_FORMAT		0x22
#define OSMESA_TYPE		0x23



typedef struct osmesa_context *OSMesaContext;


#if defined(__BEOS__) || defined(__QUICKDRAW__)
#pragma export on
#endif


/*
 * Create an Off-Screen Mesa rendering context.  The only attribute needed is
 * an RGBA vs Color-Index mode flag.
 *
 * Input:  format - one of OSMESA_COLOR_INDEX, OSMESA_RGBA, OSMESA_BGRA,
 *                  OSMESA_ARGB, OSMESA_RGB, or OSMESA_BGR.
 *         sharelist - specifies another OSMesaContext with which to share
 *                     display lists.  NULL indicates no sharing.
 * Return:  an OSMesaContext or 0 if error
 */
extern OSMesaContext OSMesaCreateContext( GLenum format,
                                          OSMesaContext sharelist );




/*
 * Destroy an Off-Screen Mesa rendering context.
 *
 * Input:  ctx - the context to destroy
 */
extern void OSMesaDestroyContext( OSMesaContext ctx );



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
extern GLboolean OSMesaMakeCurrent( OSMesaContext ctx,
                                    void *buffer, GLenum type,
                                    GLsizei width, GLsizei height );




/*
 * Return the current Off-Screen Mesa rendering context handle.
 */
extern OSMesaContext OSMesaGetCurrentContext( void );



/*
 * Set pixel store/packing parameters for the current context.
 * This is similar to glPixelStore.
 * Input:  pname - OSMESA_ROW_LENGTH
 *                    specify actual pixels per row in image buffer
 *                    0 = same as image width (default)
 *                 OSMESA_Y_UP
 *                    zero = Y coordinates increase downward
 *                    non-zero = Y coordinates increase upward (default)
 *         value - the value for the parameter pname
 *
 * New in version 2.0.
 */
extern void OSMesaPixelStore( GLint pname, GLint value );



/*
 * Return context info.  This is like glGetIntegerv.
 * Input:  pname -
 *                 OSMESA_WIDTH  return current image width
 *                 OSMESA_HEIGHT  return current image height
 *                 OSMESA_FORMAT  return image format
 *                 OSMESA_TYPE  return color component data type
 *                 OSMESA_ROW_LENGTH return row length in pixels
 *                 OSMESA_Y_UP returns 1 or 0 to indicate Y axis direction
 *         value - pointer to integer in which to return result.
 */
extern void OSMesaGetIntegerv( GLint pname, GLint *value );



/*
 * Return the depth buffer associated with an OSMesa context.
 * Input:  c - the OSMesa context
 * Output:  width, height - size of buffer in pixels
 *          bytesPerValue - bytes per depth value (2 or 4)
 *          buffer - pointer to depth buffer values
 * Return:  GL_TRUE or GL_FALSE to indicate success or failure.
 *
 * New in Mesa 2.4.
 */
extern GLboolean OSMesaGetDepthBuffer( OSMesaContext c,
                                       GLint *width, GLint *height,
                                       GLint *bytesPerValue, void **buffer );




#if defined(__BEOS__) || defined(__QUICKDRAW__)
#pragma export off
#endif


#ifdef __cplusplus
}
#endif


#endif
