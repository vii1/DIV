/* $Id: api2.c,v 1.9 1998/02/05 00:17:23 brianp Exp $ */

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
 * $Log: api2.c,v $
 * Revision 1.9  1998/02/05 00:17:23  brianp
 * removed #ifdef/#endif around glPolygonOffsetEXT()
 *
 * Revision 1.8  1998/01/16 03:46:07  brianp
 * fixed a few Windows compilation warnings (Theodore Jump)
 *
 * Revision 1.7  1998/01/06 02:03:54  brianp
 * added a few missing APIENTRY keywords
 *
 * Revision 1.6  1997/11/25 03:20:09  brianp
 * simple clean-ups for multi-threading (John Stone)
 *
 * Revision 1.5  1997/11/02 20:19:04  brianp
 * small changes to gl_TexSubImage[123]D()
 *
 * Revision 1.4  1997/10/29 02:24:29  brianp
 * added glTexImage2D() hack for GLQuake (David Bucciarelli v20 3dfx)
 *
 * Revision 1.3  1997/10/29 01:29:09  brianp
 * added GL_EXT_point_parameters extension from Daniel Barrero
 *
 * Revision 1.2  1997/09/27 00:15:05  brianp
 * added GL_EXT_paletted_texture extension
 *
 * Revision 1.1  1997/08/22 01:42:18  brianp
 * Initial revision
 *
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include "api.h"
#include "bitmap.h"
#include "context.h"
#include "eval.h"
#include "image.h"
#include "macros.h"
#include "matrix.h"
#include "teximage.h"
#include "types.h"
#include "vb.h"
#endif



void APIENTRY glOrtho( GLdouble left, GLdouble right,
                       GLdouble bottom, GLdouble top,
                       GLdouble nearval, GLdouble farval )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Ortho)(CC, left, right, bottom, top, nearval, farval);
}


void APIENTRY glPassThrough( GLfloat token )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PassThrough)(CC, token);
}


void APIENTRY glPixelMapfv( GLenum map, GLint mapsize, const GLfloat *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelMapfv)( CC, map, mapsize, values );
}


void APIENTRY glPixelMapuiv( GLenum map, GLint mapsize, const GLuint *values )
{
   GLfloat fvalues[MAX_PIXEL_MAP_TABLE];
   GLuint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = UINT_TO_FLOAT( values[i] );
      }
   }
   (*CC->API.PixelMapfv)( CC, map, mapsize, fvalues );
}



void APIENTRY glPixelMapusv( GLenum map, GLint mapsize, const GLushort *values )
{
   GLfloat fvalues[MAX_PIXEL_MAP_TABLE];
   GLuint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   if (map==GL_PIXEL_MAP_I_TO_I || map==GL_PIXEL_MAP_S_TO_S) {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = (GLfloat) values[i];
      }
   }
   else {
      for (i=0;i<mapsize;i++) {
         fvalues[i] = USHORT_TO_FLOAT( values[i] );
      }
   }
   (*CC->API.PixelMapfv)( CC, map, mapsize, fvalues );
}


void APIENTRY glPixelStoref( GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelStorei)( CC, pname, (GLint) param );
}


void APIENTRY glPixelStorei( GLenum pname, GLint param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelStorei)( CC, pname, param );
}


void APIENTRY glPixelTransferf( GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelTransferf)(CC, pname, param);
}


void APIENTRY glPixelTransferi( GLenum pname, GLint param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelTransferf)(CC, pname, (GLfloat) param);
}


void APIENTRY glPixelZoom( GLfloat xfactor, GLfloat yfactor )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PixelZoom)(CC, xfactor, yfactor);
}


void APIENTRY glPointSize( GLfloat size )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PointSize)(CC, size);
}


void APIENTRY glPolygonMode( GLenum face, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PolygonMode)(CC, face, mode);
}


void APIENTRY glPolygonOffset( GLfloat factor, GLfloat units )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PolygonOffset)( CC, factor, units );
}


/* GL_EXT_polygon_offset */
void APIENTRY glPolygonOffsetEXT( GLfloat factor, GLfloat bias )
{
   glPolygonOffset( factor, bias * DEPTH_SCALE );
}


void APIENTRY glPolygonStipple( const GLubyte *mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PolygonStipple)(CC, mask);
}


void APIENTRY glPopAttrib( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopAttrib)(CC);
}


void APIENTRY glPopClientAttrib( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopClientAttrib)(CC);
}


void APIENTRY glPopMatrix( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopMatrix)( CC );
}


void APIENTRY glPopName( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PopName)(CC);
}


void APIENTRY glPrioritizeTextures( GLsizei n, const GLuint *textures,
                                    const GLclampf *priorities )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PrioritizeTextures)(CC, n, textures, priorities);
}


void APIENTRY glPushMatrix( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushMatrix)( CC );
}


void APIENTRY glRasterPos2d( GLdouble x, GLdouble y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


void APIENTRY glRasterPos2f( GLfloat x, GLfloat y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


void APIENTRY glRasterPos2i( GLint x, GLint y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


void APIENTRY glRasterPos2s( GLshort x, GLshort y )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}


void APIENTRY glRasterPos3d( GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


void APIENTRY glRasterPos3f( GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


void APIENTRY glRasterPos3i( GLint x, GLint y, GLint z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


void APIENTRY glRasterPos3s( GLshort x, GLshort y, GLshort z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}


void APIENTRY glRasterPos4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y,
							   (GLfloat) z, (GLfloat) w );
}


void APIENTRY glRasterPos4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, x, y, z, w );
}


void APIENTRY glRasterPos4i( GLint x, GLint y, GLint z, GLint w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y,
                           (GLfloat) z, (GLfloat) w );
}


void APIENTRY glRasterPos4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) x, (GLfloat) y,
                           (GLfloat) z, (GLfloat) w );
}


void APIENTRY glRasterPos2dv( const GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


void APIENTRY glRasterPos2fv( const GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


void APIENTRY glRasterPos2iv( const GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


void APIENTRY glRasterPos2sv( const GLshort *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1], 0.0F, 1.0F );
}


/*** 3 element vector ***/

void APIENTRY glRasterPos3dv( const GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], 1.0F );
}


void APIENTRY glRasterPos3fv( const GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                               (GLfloat) v[2], 1.0F );
}


void APIENTRY glRasterPos3iv( const GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], 1.0F );
}


void APIENTRY glRasterPos3sv( const GLshort *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], 1.0F );
}


void APIENTRY glRasterPos4dv( const GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glRasterPos4fv( const GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, v[0], v[1], v[2], v[3] );
}


void APIENTRY glRasterPos4iv( const GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glRasterPos4sv( const GLshort *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.RasterPos4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                           (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glReadBuffer( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ReadBuffer)( CC, mode );
}


void APIENTRY glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height,
		   GLenum format, GLenum type, GLvoid *pixels )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ReadPixels)( CC, x, y, width, height, format, type, pixels );
}


void APIENTRY glRectd( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) x1, (GLfloat) y1,
                     (GLfloat) x2, (GLfloat) y2 );
}


void APIENTRY glRectf( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, x1, y1, x2, y2 );
}


void APIENTRY glRecti( GLint x1, GLint y1, GLint x2, GLint y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) x1, (GLfloat) y1,
                         (GLfloat) x2, (GLfloat) y2 );
}


void APIENTRY glRects( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) x1, (GLfloat) y1,
                     (GLfloat) x2, (GLfloat) y2 );
}


void APIENTRY glRectdv( const GLdouble *v1, const GLdouble *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)(CC, (GLfloat) v1[0], (GLfloat) v1[1],
                    (GLfloat) v2[0], (GLfloat) v2[1]);
}


void APIENTRY glRectfv( const GLfloat *v1, const GLfloat *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)(CC, v1[0], v1[1], v2[0], v2[1]);
}


void APIENTRY glRectiv( const GLint *v1, const GLint *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)( CC, (GLfloat) v1[0], (GLfloat) v1[1],
                     (GLfloat) v2[0], (GLfloat) v2[1] );
}


void APIENTRY glRectsv( const GLshort *v1, const GLshort *v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rectf)(CC, (GLfloat) v1[0], (GLfloat) v1[1],
        (GLfloat) v2[0], (GLfloat) v2[1]);
}


void APIENTRY glScissor( GLint x, GLint y, GLsizei width, GLsizei height)
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Scissor)(CC, x, y, width, height);
}


GLboolean APIENTRY glIsEnabled( GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.IsEnabled)( CC, cap );
}



void APIENTRY glPushAttrib( GLbitfield mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushAttrib)(CC, mask);
}


void APIENTRY glPushClientAttrib( GLbitfield mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushClientAttrib)(CC, mask);
}


void APIENTRY glPushName( GLuint name )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PushName)(CC, name);
}


GLint APIENTRY glRenderMode( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(0);
   return (*CC->API.RenderMode)(CC, mode);
}


void APIENTRY glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rotatef)( CC, (GLfloat) angle,
                       (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void APIENTRY glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Rotatef)( CC, angle, x, y, z );
}


void APIENTRY glSelectBuffer( GLsizei size, GLuint *buffer )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.SelectBuffer)(CC, size, buffer);
}


void APIENTRY glScaled( GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Scalef)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void APIENTRY glScalef( GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Scalef)( CC, x, y, z );
}


void APIENTRY glShadeModel( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ShadeModel)(CC, mode);
}


void APIENTRY glStencilFunc( GLenum func, GLint ref, GLuint mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.StencilFunc)(CC, func, ref, mask);
}


void APIENTRY glStencilMask( GLuint mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.StencilMask)(CC, mask);
}


void APIENTRY glStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.StencilOp)(CC, fail, zfail, zpass);
}


void APIENTRY glTexCoord1d( GLdouble s )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord1f( GLfloat s )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, s, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord1i( GLint s )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord1s( GLshort s )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord2d( GLdouble s, GLdouble t )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, (GLfloat) s, (GLfloat) t );
}


void APIENTRY glTexCoord2f( GLfloat s, GLfloat t )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, s, t );
}


void APIENTRY glTexCoord2i( GLint s, GLint t )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, (GLfloat) s, (GLfloat) t );
}


void APIENTRY glTexCoord2s( GLshort s, GLshort t )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, (GLfloat) s, (GLfloat) t );
}


void APIENTRY glTexCoord3d( GLdouble s, GLdouble t, GLdouble r )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, (GLfloat) t, (GLfloat) r, 1.0 );
}


void APIENTRY glTexCoord3f( GLfloat s, GLfloat t, GLfloat r )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, s, t, r, 1.0 );
}


void APIENTRY glTexCoord3i( GLint s, GLint t, GLint r )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, (GLfloat) t,
                               (GLfloat) r, 1.0 );
}


void APIENTRY glTexCoord3s( GLshort s, GLshort t, GLshort r )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, (GLfloat) t,
                               (GLfloat) r, 1.0 );
}


void APIENTRY glTexCoord4d( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, (GLfloat) t,
                               (GLfloat) r, (GLfloat) q );
}


void APIENTRY glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, s, t, r, q );
}


void APIENTRY glTexCoord4i( GLint s, GLint t, GLint r, GLint q )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, (GLfloat) t,
                               (GLfloat) r, (GLfloat) q );
}


void APIENTRY glTexCoord4s( GLshort s, GLshort t, GLshort r, GLshort q )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) s, (GLfloat) t,
                               (GLfloat) r, (GLfloat) q );
}


void APIENTRY glTexCoord1dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) *v, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord1fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, *v, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord1iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, *v, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord1sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) *v, 0.0, 0.0, 1.0 );
}


void APIENTRY glTexCoord2dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, (GLfloat) v[0], (GLfloat) v[1] );
}


void APIENTRY glTexCoord2fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, v[0], v[1] );
}


void APIENTRY glTexCoord2iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, (GLfloat) v[0], (GLfloat) v[1] );
}


void APIENTRY glTexCoord2sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord2f)( CC, (GLfloat) v[0], (GLfloat) v[1] );
}


void APIENTRY glTexCoord3dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                               (GLfloat) v[2], 1.0 );
}


void APIENTRY glTexCoord3fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, v[0], v[1], v[2], 1.0 );
}


void APIENTRY glTexCoord3iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                          (GLfloat) v[2], 1.0 );
}


void APIENTRY glTexCoord3sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                               (GLfloat) v[2], 1.0 );
}


void APIENTRY glTexCoord4dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                               (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glTexCoord4fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, v[0], v[1], v[2], v[3] );
}


void APIENTRY glTexCoord4iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                               (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glTexCoord4sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.TexCoord4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                               (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glTexCoordPointer( GLint size, GLenum type, GLsizei stride,
                        const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexCoordPointer)(CC, size, type, stride, ptr);
}


void APIENTRY glTexGend( GLenum coord, GLenum pname, GLdouble param )
{
   GLfloat p = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, &p );
}


void APIENTRY glTexGenf( GLenum coord, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, &param );
}


void APIENTRY glTexGeni( GLenum coord, GLenum pname, GLint param )
{
   GLfloat p = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, &p );
}


void APIENTRY glTexGendv( GLenum coord, GLenum pname, const GLdouble *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   p[0] = params[0];
   p[1] = params[1];
   p[2] = params[2];
   p[3] = params[3];
   (*CC->API.TexGenfv)( CC, coord, pname, p );
}


void APIENTRY glTexGeniv( GLenum coord, GLenum pname, const GLint *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   p[0] = params[0];
   p[1] = params[1];
   p[2] = params[2];
   p[3] = params[3];
   (*CC->API.TexGenfv)( CC, coord, pname, p );
}


void APIENTRY glTexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexGenfv)( CC, coord, pname, params );
}




void APIENTRY glTexEnvf( GLenum target, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, &param );
}



void APIENTRY glTexEnvi( GLenum target, GLenum pname, GLint param )
{
   GLfloat p[4];
   GET_CONTEXT;
   p[0] = (GLfloat) param;
   p[1] = p[2] = p[3] = 0.0;
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, p );
}



void APIENTRY glTexEnvfv( GLenum target, GLenum pname, const GLfloat *param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, param );
}



void APIENTRY glTexEnviv( GLenum target, GLenum pname, const GLint *param )
{
   GLfloat p[4];
   GET_CONTEXT;
   p[0] = INT_TO_FLOAT( param[0] );
   p[1] = INT_TO_FLOAT( param[1] );
   p[2] = INT_TO_FLOAT( param[2] );
   p[3] = INT_TO_FLOAT( param[3] );
   CHECK_CONTEXT;
   (*CC->API.TexEnvfv)( CC, target, pname, p );
}


void APIENTRY glTexImage1D( GLenum target, GLint level, GLint internalformat,
                            GLsizei width, GLint border,
                            GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_image *teximage;
   GET_CONTEXT;
   CHECK_CONTEXT;
   teximage = gl_unpack_image( CC, width, 1, format, type, pixels );
   (*CC->API.TexImage1D)( CC, target, level, internalformat,
                          width, border, format, type, teximage );
}



void APIENTRY glTexImage2D( GLenum target, GLint level, GLint internalformat,
                            GLsizei width, GLsizei height, GLint border,
                            GLenum format, GLenum type, const GLvoid *pixels )
{
  struct gl_image *teximage;
#if defined(FX) && defined(__WIN32__)
  GLvoid *newpixels=NULL;
  GLsizei newwidth,newheight;
  GLint x,y;
  static GLint leveldif=0;
  static GLuint lasttexobj=0xffffff;
#endif
  GET_CONTEXT;
  CHECK_CONTEXT;

#if defined(FX) && defined(__WIN32__)
  newpixels=NULL;
  
  /* AN HACK for WinGLQuake*/
  
  if(CC->Texture.Current2D->Name!=lasttexobj) {
    lasttexobj=CC->Texture.Current2D->Name;
    leveldif=0;
  }
  
  if((format==GL_COLOR_INDEX) && (internalformat==1))
    internalformat=GL_COLOR_INDEX8_EXT;
  
  if(width>256 || height >256) {
    while(width>256 || height >256) {
      newwidth=width/2;
      newheight=height/2;
      leveldif++;
      
      fprintf(stderr,"Scaling: %dx%d -> %dx%d\n",width,height,newwidth,newheight);
      
      newpixels=malloc((newwidth+4)*newheight*4);
      
      for(y=0;y<newheight;y++)
	for(x=0;x<newwidth;x++)
	  ((GLubyte *)newpixels)[x+y*newwidth]=((GLubyte *)pixels)[x*2+y*width*2];
      
      if(newpixels)
	free((void*)pixels);
      
      pixels=newpixels;
      width=newwidth;
      height=newheight;
    }
    
    level=0;
  } else
    level-=leveldif;
#endif
  teximage = gl_unpack_image( CC, width, height, format, type, pixels );
  (*CC->API.TexImage2D)( CC, target, level, internalformat,
			 width, height, border, format, type, teximage );
#if defined(FX) && defined(__WIN32__)
  if(newpixels)
    free(newpixels);
#endif
}


void APIENTRY glTexParameterf( GLenum target, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexParameterfv)( CC, target, pname, &param );
}


void APIENTRY glTexParameteri( GLenum target, GLenum pname, GLint param )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   fparam[0] = (GLfloat) param;
   fparam[1] = fparam[2] = fparam[3] = 0.0;
   CHECK_CONTEXT;
   (*CC->API.TexParameterfv)( CC, target, pname, fparam );
}


void APIENTRY glTexParameterfv( GLenum target, GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexParameterfv)( CC, target, pname, params );
}


void APIENTRY glTexParameteriv( GLenum target, GLenum pname, const GLint *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   if (pname==GL_TEXTURE_BORDER_COLOR) {
      p[0] = INT_TO_FLOAT( params[0] );
      p[1] = INT_TO_FLOAT( params[1] );
      p[2] = INT_TO_FLOAT( params[2] );
      p[3] = INT_TO_FLOAT( params[3] );
   }
   else {
      p[0] = (GLfloat) params[0];
      p[1] = (GLfloat) params[1];
      p[2] = (GLfloat) params[2];
      p[3] = (GLfloat) params[3];
   }
   (*CC->API.TexParameterfv)( CC, target, pname, p );
}


void APIENTRY glTexSubImage1D( GLenum target, GLint level, GLint xoffset,
                               GLsizei width, GLenum format,
                               GLenum type, const GLvoid *pixels )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_texsubimage( CC, width, 1, 1, format, type, pixels );
   (*CC->API.TexSubImage1D)( CC, target, level, xoffset, width,
                             format, type, image );
}


void APIENTRY glTexSubImage2D( GLenum target, GLint level,
                               GLint xoffset, GLint yoffset,
                               GLsizei width, GLsizei height,
                               GLenum format, GLenum type,
                               const GLvoid *pixels )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_texsubimage( CC, width, height, 1, format, type, pixels );
   (*CC->API.TexSubImage2D)( CC, target, level, xoffset, yoffset,
                             width, height, format, type, image );
}


void APIENTRY glTranslated( GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Translatef)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void APIENTRY glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Translatef)( CC, x, y, z );
}


void APIENTRY glVertex2d( GLdouble x, GLdouble y )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, (GLfloat) x, (GLfloat) y );
}


void APIENTRY glVertex2f( GLfloat x, GLfloat y )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, x, y );
}


void APIENTRY glVertex2i( GLint x, GLint y )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, (GLfloat) x, (GLfloat) y );
}


void APIENTRY glVertex2s( GLshort x, GLshort y )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, (GLfloat) x, (GLfloat) y );
}


void APIENTRY glVertex3d( GLdouble x, GLdouble y, GLdouble z )
{
   GET_CONTEXT;
   (*CC->API.Vertex3f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void APIENTRY glVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
   GET_CONTEXT;
   (*CC->API.Vertex3f)( CC, x, y, z );
}


void APIENTRY glVertex3i( GLint x, GLint y, GLint z )
{
   GET_CONTEXT;
   (*CC->API.Vertex3f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void APIENTRY glVertex3s( GLshort x, GLshort y, GLshort z )
{
   GET_CONTEXT;
   (*CC->API.Vertex3f)( CC, (GLfloat) x, (GLfloat) y, (GLfloat) z );
}


void APIENTRY glVertex4d( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, (GLfloat) x, (GLfloat) y,
                            (GLfloat) z, (GLfloat) w );
}


void APIENTRY glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, x, y, z, w );
}


void APIENTRY glVertex4i( GLint x, GLint y, GLint z, GLint w )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, (GLfloat) x, (GLfloat) y,
                            (GLfloat) z, (GLfloat) w );
}


void APIENTRY glVertex4s( GLshort x, GLshort y, GLshort z, GLshort w )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, (GLfloat) x, (GLfloat) y,
                            (GLfloat) z, (GLfloat) w );
}


void APIENTRY glVertex2dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, (GLfloat) v[0], (GLfloat) v[1] );
}


void APIENTRY glVertex2fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, v[0], v[1] );
}


void APIENTRY glVertex2iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, (GLfloat) v[0], (GLfloat) v[1] );
}


void APIENTRY glVertex2sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex2f)( CC, (GLfloat) v[0], (GLfloat) v[1] );
}


void APIENTRY glVertex3dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex3f)( CC, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


void APIENTRY glVertex3fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex3fv)( CC, v );
}


void APIENTRY glVertex3iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex3f)( CC, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


void APIENTRY glVertex3sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex3f)( CC, (GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2] );
}


void APIENTRY glVertex4dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                            (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glVertex4fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, v[0], v[1], v[2], v[3] );
}


void APIENTRY glVertex4iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                            (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glVertex4sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.Vertex4f)( CC, (GLfloat) v[0], (GLfloat) v[1],
                            (GLfloat) v[2], (GLfloat) v[3] );
}


void APIENTRY glVertexPointer( GLint size, GLenum type, GLsizei stride,
                               const GLvoid *ptr )
{
   GET_CONTEXT;
   (*CC->API.VertexPointer)(CC, size, type, stride, ptr);
}


void APIENTRY glViewport( GLint x, GLint y, GLsizei width, GLsizei height )
{
   GET_CONTEXT;
   (*CC->API.Viewport)( CC, x, y, width, height );
}



/**
 ** Extensions
 **
 ** Some of these are incorporated into the 1.1 API.  They also remain as
 ** extensions for backward compatibility.  May be removed in the future.
 **/


/* GL_EXT_blend_minmax */

void APIENTRY glBlendEquationEXT( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BlendEquation)(CC, mode);
}


/* GL_EXT_blend_color */

void APIENTRY glBlendColorEXT( GLclampf red, GLclampf green,
                               GLclampf blue, GLclampf alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BlendColor)(CC, red, green, blue, alpha);
}


/* GL_EXT_vertex_array */

void APIENTRY glVertexPointerEXT( GLint size, GLenum type, GLsizei stride,
                                  GLsizei count, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.VertexPointer)(CC, size, type, stride, ptr);
}


void APIENTRY glNormalPointerEXT( GLenum type, GLsizei stride, GLsizei count,
                                  const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.NormalPointer)(CC, type, stride, ptr);
}


void APIENTRY glColorPointerEXT( GLint size, GLenum type, GLsizei stride,
                                 GLsizei count, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ColorPointer)(CC, size, type, stride, ptr);
}


void APIENTRY glIndexPointerEXT( GLenum type, GLsizei stride,
                                 GLsizei count, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.IndexPointer)(CC, type, stride, ptr);
}


void APIENTRY glTexCoordPointerEXT( GLint size, GLenum type, GLsizei stride,
                                    GLsizei count, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.TexCoordPointer)(CC, size, type, stride, ptr);
}


void APIENTRY glEdgeFlagPointerEXT( GLsizei stride, GLsizei count,
                                    const GLboolean *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EdgeFlagPointer)(CC, stride, ptr);
}


void APIENTRY glGetPointervEXT( GLenum pname, GLvoid **params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPointerv)(CC, pname, params);
}


void APIENTRY glArrayElementEXT( GLint i )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ArrayElement)(CC, i);
}


void APIENTRY glDrawArraysEXT( GLenum mode, GLint first, GLsizei count )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DrawArrays)(CC, mode, first, count);
}


/* GL_EXT_texture_object */

GLboolean APIENTRY glAreTexturesResidentEXT( GLsizei n, const GLuint *textures,
                                             GLboolean *residences )
{
   return glAreTexturesResident( n, textures, residences );
}


void APIENTRY glBindTextureEXT( GLenum target, GLuint texture )
{
   glBindTexture( target, texture );
}


void APIENTRY glDeleteTexturesEXT( GLsizei n, const GLuint *textures)
{
   glDeleteTextures( n, textures );
}


void APIENTRY glGenTexturesEXT( GLsizei n, GLuint *textures )
{
   glGenTextures( n, textures );
}


GLboolean APIENTRY glIsTextureEXT( GLuint texture )
{
   return glIsTexture( texture );
}


void APIENTRY glPrioritizeTexturesEXT( GLsizei n, const GLuint *textures,
                                       const GLclampf *priorities )
{
   glPrioritizeTextures( n, textures, priorities );
}



/* GL_EXT_texture3D */

void APIENTRY glCopyTexSubImage3DEXT( GLenum target, GLint level, GLint xoffset,
                                      GLint yoffset, GLint zoffset,
                                      GLint x, GLint y, GLsizei width,
                                      GLsizei height )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexSubImage3DEXT)( CC, target, level, xoffset, yoffset,
                                    zoffset, x, y, width, height );
}



void APIENTRY glTexImage3DEXT( GLenum target, GLint level, GLenum internalformat,
                               GLsizei width, GLsizei height, GLsizei depth,
                               GLint border, GLenum format, GLenum type,
                               const GLvoid *pixels )
{
   struct gl_image *teximage;
   GET_CONTEXT;
   CHECK_CONTEXT;
   teximage = gl_unpack_image3D( CC, width, height, depth, format, type, pixels);
   (*CC->API.TexImage3DEXT)( CC, target, level, internalformat,
                             width, height, depth, border, format, type, 
                             teximage );
}


void APIENTRY glTexSubImage3DEXT( GLenum target, GLint level, GLint xoffset,
                                  GLint yoffset, GLint zoffset, GLsizei width,
                                  GLsizei height, GLsizei depth, GLenum format,
                                  GLenum type, const GLvoid *pixels )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_texsubimage( CC, width, height, depth, format, type, pixels );
   (*CC->API.TexSubImage3DEXT)( CC, target, level, xoffset, yoffset, zoffset,
                                width, height, depth, format, type, image );
}



/* GL_EXT_point_parameters */

void APIENTRY glPointParameterfEXT( GLenum pname, GLfloat param )
{
   GLfloat params[3];
   GET_CONTEXT;
   CHECK_CONTEXT;
   params[0] = param;
   params[1] = 0.0;
   params[2] = 0.0;
   (*CC->API.PointParameterfvEXT)(CC, pname, params);
}


void APIENTRY glPointParameterfvEXT( GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.PointParameterfvEXT)(CC, pname, params);
}



#ifdef GL_MESA_window_pos
/*
 * Mesa implementation of glWindowPos*MESA()
 */
void APIENTRY glWindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.WindowPos4fMESA)( CC, x, y, z, w );
}
#else
/* Implementation in winpos.c is used */
#endif


void APIENTRY glWindowPos2iMESA( GLint x, GLint y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

void APIENTRY glWindowPos2sMESA( GLshort x, GLshort y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

void APIENTRY glWindowPos2fMESA( GLfloat x, GLfloat y )
{
   glWindowPos4fMESA( x, y, 0.0F, 1.0F );
}

void APIENTRY glWindowPos2dMESA( GLdouble x, GLdouble y )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, 0.0F, 1.0F );
}

void APIENTRY glWindowPos2ivMESA( const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

void APIENTRY glWindowPos2svMESA( const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

void APIENTRY glWindowPos2fvMESA( const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], 0.0F, 1.0F );
}

void APIENTRY glWindowPos2dvMESA( const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], 0.0F, 1.0F );
}

void APIENTRY glWindowPos3iMESA( GLint x, GLint y, GLint z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

void APIENTRY glWindowPos3sMESA( GLshort x, GLshort y, GLshort z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

void APIENTRY glWindowPos3fMESA( GLfloat x, GLfloat y, GLfloat z )
{
   glWindowPos4fMESA( x, y, z, 1.0F );
}

void APIENTRY glWindowPos3dMESA( GLdouble x, GLdouble y, GLdouble z )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, 1.0F );
}

void APIENTRY glWindowPos3ivMESA( const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

void APIENTRY glWindowPos3svMESA( const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

void APIENTRY glWindowPos3fvMESA( const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], p[2], 1.0F );
}

void APIENTRY glWindowPos3dvMESA( const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1], (GLfloat) p[2], 1.0F );
}

void APIENTRY glWindowPos4iMESA( GLint x, GLint y, GLint z, GLint w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}

void APIENTRY glWindowPos4sMESA( GLshort x, GLshort y, GLshort z, GLshort w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}

void APIENTRY glWindowPos4dMESA( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
   glWindowPos4fMESA( (GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w );
}


void APIENTRY glWindowPos4ivMESA( const GLint *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}

void APIENTRY glWindowPos4svMESA( const GLshort *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}

void APIENTRY glWindowPos4fvMESA( const GLfloat *p )
{
   glWindowPos4fMESA( p[0], p[1], p[2], p[3] );
}

void APIENTRY glWindowPos4dvMESA( const GLdouble *p )
{
   glWindowPos4fMESA( (GLfloat) p[0], (GLfloat) p[1],
                      (GLfloat) p[2], (GLfloat) p[3] );
}



/* GL_MESA_resize_buffers */

/*
 * Called by user application when window has been resized.
 */
void APIENTRY glResizeBuffersMESA( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ResizeBuffersMESA)( CC );
}

