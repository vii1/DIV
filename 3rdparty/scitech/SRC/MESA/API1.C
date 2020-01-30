/* $Id: api1.c,v 1.3 1997/11/05 03:25:52 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.5
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
 * $Log: api1.c,v $
 * Revision 1.3  1997/11/05 03:25:52  brianp
 * fixed null bitmap problem in glBitmap
 *
 * Revision 1.2  1997/09/27 00:15:05  brianp
 * added GL_EXT_paletted_texture extension
 *
 * Revision 1.1  1997/08/22 01:42:11  brianp
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

#include "drawpix.h"

#include "eval.h"
#include "image.h"
#include "macros.h"
#include "matrix.h"
#include "teximage.h"
#include "types.h"
#include "vb.h"
#endif




void APIENTRY glAccum( GLenum op, GLfloat value )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Accum)(CC, op, value);
}


void APIENTRY glAlphaFunc( GLenum func, GLclampf ref )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.AlphaFunc)(CC, func, ref);
}


GLboolean APIENTRY glAreTexturesResident( GLsizei n, const GLuint *textures,
								 GLboolean *residences )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.AreTexturesResident)(CC, n, textures, residences);
}


void APIENTRY glArrayElement( GLint i )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ArrayElement)(CC, i);
}


void APIENTRY glBegin( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Begin)( CC, mode );
}


void APIENTRY glBindTexture( GLenum target, GLuint texture )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BindTexture)(CC, target, texture);
}


void APIENTRY glBitmap( GLsizei width, GLsizei height,
               GLfloat xorig, GLfloat yorig,
               GLfloat xmove, GLfloat ymove,
               const GLubyte *bitmap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   if (!CC->CompileFlag) {
      /* execute only, try optimized case where no unpacking needed */
      if (   CC->Unpack.LsbFirst==GL_FALSE
          && CC->Unpack.Alignment==1
          && CC->Unpack.RowLength==0
          && CC->Unpack.SkipPixels==0
          && CC->Unpack.SkipRows==0) {
         /* Special case: no unpacking needed */
         struct gl_image image;
         image.Width = width;
         image.Height = height;
         image.Components = 0;
         image.Type = GL_BITMAP;
         image.Format = GL_COLOR_INDEX;
         image.Data = (GLvoid *) bitmap;
         (*CC->Exec.Bitmap)( CC, width, height, xorig, yorig,
                             xmove, ymove, &image );
      }
      else {
         struct gl_image *image;
         image = gl_unpack_bitmap( CC, width, height, bitmap );
         (*CC->Exec.Bitmap)( CC, width, height, xorig, yorig,
                             xmove, ymove, image );
         if (image) {
            gl_free_image( image );
         }
      }
   }
   else {
      /* compile and maybe execute */
      struct gl_image *image;
      image = gl_unpack_bitmap( CC, width, height, bitmap );
      (*CC->API.Bitmap)(CC, width, height, xorig, yorig, xmove, ymove, image );
   }
}


void APIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.BlendFunc)(CC, sfactor, dfactor);
}


void APIENTRY glCallList( GLuint list )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CallList)(CC, list);
}


void APIENTRY glCallLists( GLsizei n, GLenum type, const GLvoid *lists )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CallLists)(CC, n, type, lists);
}


void APIENTRY glClear( GLbitfield mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Clear)(CC, mask);
}


void APIENTRY glClearAccum( GLfloat red, GLfloat green,
			  GLfloat blue, GLfloat alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearAccum)(CC, red, green, blue, alpha);
}



void APIENTRY glClearIndex( GLfloat c )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearIndex)(CC, c);
}


void APIENTRY glClearColor( GLclampf red,
			  GLclampf green,
			  GLclampf blue,
			  GLclampf alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearColor)(CC, red, green, blue, alpha);
}


void APIENTRY glClearDepth( GLclampd depth )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearDepth)( CC, depth );
}


void APIENTRY glClearStencil( GLint s )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ClearStencil)(CC, s);
}


void APIENTRY glClipPlane( GLenum plane, const GLdouble *equation )
{
   GLfloat eq[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   eq[0] = (GLfloat) equation[0];
   eq[1] = (GLfloat) equation[1];
   eq[2] = (GLfloat) equation[2];
   eq[3] = (GLfloat) equation[3];
   (*CC->API.ClipPlane)(CC, plane, eq );
}


void APIENTRY glColor3b( GLbyte red, GLbyte green, GLbyte blue )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, BYTE_TO_FLOAT(red), BYTE_TO_FLOAT(green),
                       BYTE_TO_FLOAT(blue) );
}


void APIENTRY glColor3d( GLdouble red, GLdouble green, GLdouble blue )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, (GLfloat) red, (GLfloat) green, (GLfloat) blue );
}


void APIENTRY glColor3f( GLfloat red, GLfloat green, GLfloat blue )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, red, green, blue );
}


void APIENTRY glColor3i( GLint red, GLint green, GLint blue )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, INT_TO_FLOAT(red), INT_TO_FLOAT(green),
                       INT_TO_FLOAT(blue) );
}


void APIENTRY glColor3s( GLshort red, GLshort green, GLshort blue )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, SHORT_TO_FLOAT(red), SHORT_TO_FLOAT(green),
                       SHORT_TO_FLOAT(blue) );
}


void APIENTRY glColor3ub( GLubyte red, GLubyte green, GLubyte blue )
{
   GET_CONTEXT;
   (*CC->API.Color4ub)( CC, red, green, blue, 255 );
}


void APIENTRY glColor3ui( GLuint red, GLuint green, GLuint blue )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, UINT_TO_FLOAT(red), UINT_TO_FLOAT(green),
                       UINT_TO_FLOAT(blue) );
}


void APIENTRY glColor3us( GLushort red, GLushort green, GLushort blue )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, USHORT_TO_FLOAT(red), USHORT_TO_FLOAT(green),
                       USHORT_TO_FLOAT(blue) );
}


void APIENTRY glColor4b( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, BYTE_TO_FLOAT(red), BYTE_TO_FLOAT(green),
                       BYTE_TO_FLOAT(blue), BYTE_TO_FLOAT(alpha) );
}


void APIENTRY glColor4d( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, (GLfloat) red, (GLfloat) green,
                       (GLfloat) blue, (GLfloat) alpha );
}


void APIENTRY glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, red, green, blue, alpha );
}

void APIENTRY glColor4i( GLint red, GLint green, GLint blue, GLint alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, INT_TO_FLOAT(red), INT_TO_FLOAT(green),
                       INT_TO_FLOAT(blue), INT_TO_FLOAT(alpha) );
}


void APIENTRY glColor4s( GLshort red, GLshort green, GLshort blue, GLshort alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, SHORT_TO_FLOAT(red), SHORT_TO_FLOAT(green),
                       SHORT_TO_FLOAT(blue), SHORT_TO_FLOAT(alpha) );
}

void APIENTRY glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4ub)( CC, red, green, blue, alpha );
}

void APIENTRY glColor4ui( GLuint red, GLuint green, GLuint blue, GLuint alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, UINT_TO_FLOAT(red), UINT_TO_FLOAT(green),
                       UINT_TO_FLOAT(blue), UINT_TO_FLOAT(alpha) );
}

void APIENTRY glColor4us( GLushort red, GLushort green, GLushort blue, GLushort alpha )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, USHORT_TO_FLOAT(red), USHORT_TO_FLOAT(green),
                       USHORT_TO_FLOAT(blue), USHORT_TO_FLOAT(alpha) );
}


void APIENTRY glColor3bv( const GLbyte *v )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]),
                       BYTE_TO_FLOAT(v[2]) );
}


void APIENTRY glColor3dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, (GLdouble) v[0], (GLdouble) v[1], (GLdouble) v[2] );
}


void APIENTRY glColor3fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.Color3fv)( CC, v );
}


void APIENTRY glColor3iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]),
                       INT_TO_FLOAT(v[2]) );
}


void APIENTRY glColor3sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]),
                       SHORT_TO_FLOAT(v[2]) );
}


void APIENTRY glColor3ubv( const GLubyte *v )
{
   GET_CONTEXT;
   (*CC->API.Color4ub)( CC, v[0], v[1], v[2], 255 );
}


void APIENTRY glColor3uiv( const GLuint *v )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, UINT_TO_FLOAT(v[0]), UINT_TO_FLOAT(v[1]),
                       UINT_TO_FLOAT(v[2]) );
}


void APIENTRY glColor3usv( const GLushort *v )
{
   GET_CONTEXT;
   (*CC->API.Color3f)( CC, USHORT_TO_FLOAT(v[0]), USHORT_TO_FLOAT(v[1]),
                       USHORT_TO_FLOAT(v[2]) );

}


void APIENTRY glColor4bv( const GLbyte *v )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, BYTE_TO_FLOAT(v[0]), BYTE_TO_FLOAT(v[1]),
                       BYTE_TO_FLOAT(v[2]), BYTE_TO_FLOAT(v[3]) );
}


void APIENTRY glColor4dv( const GLdouble *v )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, (GLdouble) v[0], (GLdouble) v[1],
                       (GLdouble) v[2], (GLdouble) v[3] );
}


void APIENTRY glColor4fv( const GLfloat *v )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, v[0], v[1], v[2], v[3] );
}


void APIENTRY glColor4iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, INT_TO_FLOAT(v[0]), INT_TO_FLOAT(v[1]),
                       INT_TO_FLOAT(v[2]), INT_TO_FLOAT(v[3]) );
}


void APIENTRY glColor4sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, SHORT_TO_FLOAT(v[0]), SHORT_TO_FLOAT(v[1]),
                       SHORT_TO_FLOAT(v[2]), SHORT_TO_FLOAT(v[3]) );
}


void APIENTRY glColor4ubv( const GLubyte *v )
{
   GET_CONTEXT;
   (*CC->API.Color4ubv)( CC, v );
}


void APIENTRY glColor4uiv( const GLuint *v )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, UINT_TO_FLOAT(v[0]), UINT_TO_FLOAT(v[1]),
                       UINT_TO_FLOAT(v[2]), UINT_TO_FLOAT(v[3]) );
}


void APIENTRY glColor4usv( const GLushort *v )
{
   GET_CONTEXT;
   (*CC->API.Color4f)( CC, USHORT_TO_FLOAT(v[0]), USHORT_TO_FLOAT(v[1]),
                       USHORT_TO_FLOAT(v[2]), USHORT_TO_FLOAT(v[3]) );
}


void APIENTRY glColorMask( GLboolean red, GLboolean green,
			 GLboolean blue, GLboolean alpha )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ColorMask)(CC, red, green, blue, alpha);
}


void APIENTRY glColorMaterial( GLenum face, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ColorMaterial)(CC, face, mode);
}


void APIENTRY glColorPointer( GLint size, GLenum type, GLsizei stride,
					 const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ColorPointer)(CC, size, type, stride, ptr);
}


void APIENTRY glColorTableEXT( GLenum target, GLenum internalFormat,
                               GLsizei width, GLenum format, GLenum type,
                               const GLvoid *table )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_image( CC, width, 1, format, type, table );
   (*CC->API.ColorTable)( CC, target, internalFormat, image );
   if (image->RefCount == 0)
      gl_free_image(image);
}


void APIENTRY glColorSubTableEXT( GLenum target, GLsizei start, GLsizei count, 
                                  GLenum format, GLenum type,
                                  const GLvoid *data )
{
   struct gl_image *image;
   GET_CONTEXT;
   CHECK_CONTEXT;
   image = gl_unpack_image( CC, count, 1, format, type, data );
   (*CC->API.ColorSubTable)( CC, target, start, image );
   if (image->RefCount == 0)
      gl_free_image(image);
}



void APIENTRY glCopyPixels( GLint x, GLint y, GLsizei width, GLsizei height,
			  GLenum type )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyPixels)(CC, x, y, width, height, type);
}


void APIENTRY glCopyTexImage1D( GLenum target, GLint level,
                                GLenum internalformat,
                                GLint x, GLint y,
                                GLsizei width, GLint border )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexImage1D)( CC, target, level, internalformat,
								 x, y, width, border );
}


void APIENTRY glCopyTexImage2D( GLenum target, GLint level,
                                GLenum internalformat,
                                GLint x, GLint y,
                                GLsizei width, GLsizei height, GLint border )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexImage2D)( CC, target, level, internalformat,
                              x, y, width, height, border );
}


void APIENTRY glCopyTexSubImage1D( GLenum target, GLint level,
                                   GLint xoffset, GLint x, GLint y,
                                   GLsizei width )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexSubImage1D)( CC, target, level, xoffset, x, y, width );
}


void APIENTRY glCopyTexSubImage2D( GLenum target, GLint level,
                                   GLint xoffset, GLint yoffset,
                                   GLint x, GLint y,
                                   GLsizei width, GLsizei height )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CopyTexSubImage2D)( CC, target, level, xoffset, yoffset,
                                 x, y, width, height );
}



void APIENTRY glCullFace( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.CullFace)(CC, mode);
}


void APIENTRY glDepthFunc( GLenum func )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DepthFunc)( CC, func );
}


void APIENTRY glDepthMask( GLboolean flag )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DepthMask)( CC, flag );
}


void APIENTRY glDepthRange( GLclampd near_val, GLclampd far_val )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DepthRange)( CC, near_val, far_val );
}


void APIENTRY glDeleteLists( GLuint list, GLsizei range )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DeleteLists)(CC, list, range);
}


void APIENTRY glDeleteTextures( GLsizei n, const GLuint *textures)
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DeleteTextures)(CC, n, textures);
}


void APIENTRY glDisable( GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Disable)( CC, cap );
}


void APIENTRY glDisableClientState( GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DisableClientState)( CC, cap );
}


void APIENTRY glDrawArrays( GLenum mode, GLint first, GLsizei count )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DrawArrays)(CC, mode, first, count);
}


void APIENTRY glDrawBuffer( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DrawBuffer)(CC, mode);
}


void APIENTRY glDrawElements( GLenum mode, GLsizei count,
                              GLenum type, const GLvoid *indices )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DrawElements)( CC, mode, count, type, indices );
}


void APIENTRY glDrawPixels( GLsizei width, GLsizei height,
                            GLenum format, GLenum type, const GLvoid *pixels )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.DrawPixels)( CC, width, height, format, type, pixels );
}


void APIENTRY glEnable( GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Enable)( CC, cap );
}


void APIENTRY glEnableClientState( GLenum cap )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EnableClientState)( CC, cap );
}


void APIENTRY glEnd( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.End)( CC );
}


void APIENTRY glEndList( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EndList)(CC);
}




void APIENTRY glEvalCoord1d( GLdouble u )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord1f)( CC, (GLfloat) u );
}


void APIENTRY glEvalCoord1f( GLfloat u )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord1f)( CC, u );
}


void APIENTRY glEvalCoord1dv( const GLdouble *u )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord1f)( CC, (GLfloat) *u );
}


void APIENTRY glEvalCoord1fv( const GLfloat *u )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord1f)( CC, (GLfloat) *u );
}


void APIENTRY glEvalCoord2d( GLdouble u, GLdouble v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord2f)( CC, (GLfloat) u, (GLfloat) v );
}


void APIENTRY glEvalCoord2f( GLfloat u, GLfloat v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord2f)( CC, u, v );
}


void APIENTRY glEvalCoord2dv( const GLdouble *u )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord2f)( CC, (GLfloat) u[0], (GLfloat) u[1] );
}


void APIENTRY glEvalCoord2fv( const GLfloat *u )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalCoord2f)( CC, u[0], u[1] );
}


void APIENTRY glEvalPoint1( GLint i )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalPoint1)( CC, i );
}


void APIENTRY glEvalPoint2( GLint i, GLint j )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalPoint2)( CC, i, j );
}


void APIENTRY glEvalMesh1( GLenum mode, GLint i1, GLint i2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalMesh1)( CC, mode, i1, i2 );
}


void APIENTRY glEdgeFlag( GLboolean flag )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EdgeFlag)(CC, flag);
}


void APIENTRY glEdgeFlagv( const GLboolean *flag )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EdgeFlag)(CC, *flag);
}


void APIENTRY glEdgeFlagPointer( GLsizei stride, const GLboolean *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EdgeFlagPointer)(CC, stride, ptr);
}


void APIENTRY glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.EvalMesh2)( CC, mode, i1, i2, j1, j2 );
}


void APIENTRY glFeedbackBuffer( GLsizei size, GLenum type, GLfloat *buffer )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.FeedbackBuffer)(CC, size, type, buffer);
}


void APIENTRY glFinish( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Finish)(CC);
}


void APIENTRY glFlush( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Flush)(CC);
}


void APIENTRY glFogf( GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Fogfv)(CC, pname, &param);
}


void APIENTRY glFogi( GLenum pname, GLint param )
{
   GLfloat fparam = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Fogfv)(CC, pname, &fparam);
}


void APIENTRY glFogfv( GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Fogfv)(CC, pname, params);
}


void APIENTRY glFogiv( GLenum pname, const GLint *params )
{
   GLfloat p[4];
   GET_CONTEXT;
   CHECK_CONTEXT;

   switch (pname) {
      case GL_FOG_MODE:
      case GL_FOG_DENSITY:
      case GL_FOG_START:
      case GL_FOG_END:
      case GL_FOG_INDEX:
	 p[0] = (GLfloat) *params;
	 break;
      case GL_FOG_COLOR:
	 p[0] = INT_TO_FLOAT( params[0] );
	 p[1] = INT_TO_FLOAT( params[1] );
	 p[2] = INT_TO_FLOAT( params[2] );
	 p[3] = INT_TO_FLOAT( params[3] );
	 break;
      default:
         /* Error will be caught later in gl_Fogfv */
         ;
   }
   (*CC->API.Fogfv)( CC, pname, p );
}



void APIENTRY glFrontFace( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.FrontFace)(CC, mode);
}


void APIENTRY glFrustum( GLdouble left, GLdouble right,
                         GLdouble bottom, GLdouble top,
                         GLdouble nearval, GLdouble farval )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Frustum)(CC, left, right, bottom, top, nearval, farval);
}


GLuint APIENTRY glGenLists( GLsizei range )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(0);
   return (*CC->API.GenLists)(CC, range);
}


void APIENTRY glGenTextures( GLsizei n, GLuint *textures )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GenTextures)(CC, n, textures);
}


void APIENTRY glGetBooleanv( GLenum pname, GLboolean *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetBooleanv)(CC, pname, params);
}


void APIENTRY glGetClipPlane( GLenum plane, GLdouble *equation )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetClipPlane)(CC, plane, equation);
}


void APIENTRY glGetColorTableEXT( GLenum target, GLenum format,
                                  GLenum type, GLvoid *table )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetColorTable)(CC, target, format, type, table);
}


void APIENTRY glGetColorTableParameterivEXT( GLenum target, GLenum pname,
                                             GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetColorTableParameteriv)(CC, target, pname, params);
}


void APIENTRY glGetColorTableParameterfvEXT( GLenum target, GLenum pname,
                                             GLfloat *params )
{
   GLint iparams;
   glGetColorTableParameterivEXT( target, pname, &iparams );
   *params = (GLfloat) iparams;
}


void APIENTRY glGetDoublev( GLenum pname, GLdouble *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetDoublev)(CC, pname, params);
}


GLenum APIENTRY glGetError( void )
{
   GET_CONTEXT;
   if (!CC) {
      /* No current context */
      return GL_NO_ERROR;
   }
   return (*CC->API.GetError)(CC);
}


void APIENTRY glGetFloatv( GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetFloatv)(CC, pname, params);
}


void APIENTRY glGetIntegerv( GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetIntegerv)(CC, pname, params);
}


void APIENTRY glGetLightfv( GLenum light, GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetLightfv)(CC, light, pname, params);
}


void APIENTRY glGetLightiv( GLenum light, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetLightiv)(CC, light, pname, params);
}


void APIENTRY glGetMapdv( GLenum target, GLenum query, GLdouble *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMapdv)( CC, target, query, v );
}


void APIENTRY glGetMapfv( GLenum target, GLenum query, GLfloat *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMapfv)( CC, target, query, v );
}


void APIENTRY glGetMapiv( GLenum target, GLenum query, GLint *v )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMapiv)( CC, target, query, v );
}


void APIENTRY glGetMaterialfv( GLenum face, GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMaterialfv)(CC, face, pname, params);
}


void APIENTRY glGetMaterialiv( GLenum face, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetMaterialiv)(CC, face, pname, params);
}


void APIENTRY glGetPixelMapfv( GLenum map, GLfloat *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPixelMapfv)(CC, map, values);
}


void APIENTRY glGetPixelMapuiv( GLenum map, GLuint *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPixelMapuiv)(CC, map, values);
}


void APIENTRY glGetPixelMapusv( GLenum map, GLushort *values )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPixelMapusv)(CC, map, values);
}


void APIENTRY glGetPointerv( GLenum pname, GLvoid **params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPointerv)(CC, pname, params);
}


void APIENTRY glGetPolygonStipple( GLubyte *mask )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetPolygonStipple)(CC, mask);
}


const GLubyte * APIENTRY glGetString( GLenum name )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(NULL);
   return (*CC->API.GetString)(CC, name);
}



void APIENTRY glGetTexEnvfv( GLenum target, GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexEnvfv)(CC, target, pname, params);
}


void APIENTRY glGetTexEnviv( GLenum target, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexEnviv)(CC, target, pname, params);
}


void APIENTRY glGetTexGeniv( GLenum coord, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexGeniv)(CC, coord, pname, params);
}


void APIENTRY glGetTexGendv( GLenum coord, GLenum pname, GLdouble *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexGendv)(CC, coord, pname, params);
}


void APIENTRY glGetTexGenfv( GLenum coord, GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexGenfv)(CC, coord, pname, params);
}



void APIENTRY glGetTexImage( GLenum target, GLint level, GLenum format,
                             GLenum type, GLvoid *pixels )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexImage)(CC, target, level, format, type, pixels);
}


void APIENTRY glGetTexLevelParameterfv( GLenum target, GLint level,
                                        GLenum pname, GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexLevelParameterfv)(CC, target, level, pname, params);
}


void APIENTRY glGetTexLevelParameteriv( GLenum target, GLint level,
                                        GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexLevelParameteriv)(CC, target, level, pname, params);
}




void APIENTRY glGetTexParameterfv( GLenum target, GLenum pname, GLfloat *params)
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexParameterfv)(CC, target, pname, params);
}


void APIENTRY glGetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.GetTexParameteriv)(CC, target, pname, params);
}


void APIENTRY glHint( GLenum target, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Hint)(CC, target, mode);
}


void APIENTRY glIndexd( GLdouble c )
{
   GET_CONTEXT;
   (*CC->API.Indexf)( CC, (GLfloat) c );
}


void APIENTRY glIndexf( GLfloat c )
{
   GET_CONTEXT;
   (*CC->API.Indexf)( CC, c );
}


void APIENTRY glIndexi( GLint c )
{
   GET_CONTEXT;
   (*CC->API.Indexi)( CC, c );
}


void APIENTRY glIndexs( GLshort c )
{
   GET_CONTEXT;
   (*CC->API.Indexi)( CC, (GLint) c );
}


#ifdef GL_VERSION_1_1
void APIENTRY glIndexub( GLubyte c )
{
   GET_CONTEXT;
   (*CC->API.Indexi)( CC, (GLint) c );
}
#endif


void APIENTRY glIndexdv( const GLdouble *c )
{
   GET_CONTEXT;
   (*CC->API.Indexf)( CC, (GLfloat) *c );
}


void APIENTRY glIndexfv( const GLfloat *c )
{
   GET_CONTEXT;
   (*CC->API.Indexf)( CC, *c );
}


void APIENTRY glIndexiv( const GLint *c )
{
   GET_CONTEXT;
   (*CC->API.Indexi)( CC, *c );
}


void APIENTRY glIndexsv( const GLshort *c )
{
   GET_CONTEXT;
   (*CC->API.Indexi)( CC, (GLint) *c );
}


#ifdef GL_VERSION_1_1
void APIENTRY glIndexubv( const GLubyte *c )
{
   GET_CONTEXT;
   (*CC->API.Indexi)( CC, (GLint) *c );
}
#endif


void APIENTRY glIndexMask( GLuint mask )
{
   GET_CONTEXT;
   (*CC->API.IndexMask)(CC, mask);
}


void APIENTRY glIndexPointer( GLenum type, GLsizei stride, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.IndexPointer)(CC, type, stride, ptr);
}


void APIENTRY glInterleavedArrays( GLenum format, GLsizei stride,
                                   const GLvoid *pointer )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.InterleavedArrays)( CC, format, stride, pointer );
}


void APIENTRY glInitNames( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.InitNames)(CC);
}


GLboolean APIENTRY glIsList( GLuint list )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.IsList)(CC, list);
}


GLboolean APIENTRY glIsTexture( GLuint texture )
{
   GET_CONTEXT;
   CHECK_CONTEXT_RETURN(GL_FALSE);
   return (*CC->API.IsTexture)(CC, texture);
}


void APIENTRY glLightf( GLenum light, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Lightfv)( CC, light, pname, &param, 1 );
}



void APIENTRY glLighti( GLenum light, GLenum pname, GLint param )
{
   GLfloat fparam = (GLfloat) param;
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Lightfv)( CC, light, pname, &fparam, 1 );
}



void APIENTRY glLightfv( GLenum light, GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Lightfv)( CC, light, pname, params, 4 );
}



void APIENTRY glLightiv( GLenum light, GLenum pname, const GLint *params )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;

   switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_POSITION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         fparam[3] = (GLfloat) params[3];
         break;
      case GL_SPOT_DIRECTION:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         break;
      case GL_SPOT_EXPONENT:
	  case GL_SPOT_CUTOFF:
      case GL_CONSTANT_ATTENUATION:
      case GL_LINEAR_ATTENUATION:
      case GL_QUADRATIC_ATTENUATION:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
		 /* error will be caught later in gl_Lightfv */
		 ;
   }
   (*CC->API.Lightfv)( CC, light, pname, fparam, 4 );
}



void APIENTRY glLightModelf( GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LightModelfv)( CC, pname, &param );
}


void APIENTRY glLightModeli( GLenum pname, GLint param )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   fparam[0] = (GLfloat) param;
   (*CC->API.LightModelfv)( CC, pname, fparam );
}


void APIENTRY glLightModelfv( GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LightModelfv)( CC, pname, params );
}


void APIENTRY glLightModeliv( GLenum pname, const GLint *params )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;

   switch (pname) {
      case GL_LIGHT_MODEL_AMBIENT:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
      case GL_LIGHT_MODEL_TWO_SIDE:
         fparam[0] = (GLfloat) params[0];
         break;
      default:
         /* Error will be caught later in gl_LightModelfv */
         ;
   }
   (*CC->API.LightModelfv)( CC, pname, fparam );
}


void APIENTRY glLineWidth( GLfloat width )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LineWidth)(CC, width);
}


void APIENTRY glLineStipple( GLint factor, GLushort pattern )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LineStipple)(CC, factor, pattern);
}


void APIENTRY glListBase( GLuint base )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.ListBase)(CC, base);
}


void APIENTRY glLoadIdentity( void )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LoadIdentity)( CC );
}


void APIENTRY glLoadMatrixd( const GLdouble *m )
{
   GLfloat fm[16];
   GLuint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   for (i=0;i<16;i++) {
	  fm[i] = (GLfloat) m[i];
   }

   (*CC->API.LoadMatrixf)( CC, fm );
}


void APIENTRY glLoadMatrixf( const GLfloat *m )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LoadMatrixf)( CC, m );
}


void APIENTRY glLoadName( GLuint name )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LoadName)(CC, name);
}


void APIENTRY glLogicOp( GLenum opcode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.LogicOp)(CC, opcode);
}



void APIENTRY glMap1d( GLenum target, GLdouble u1, GLdouble u2, GLint stride,
                       GLint order, const GLdouble *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points1d( target, stride, order, points );
   retain = CC->CompileFlag;
   (*CC->API.Map1f)( CC, target, u1, u2, stride, order, pnts, retain );
}


void APIENTRY glMap1f( GLenum target, GLfloat u1, GLfloat u2, GLint stride,
                       GLint order, const GLfloat *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points1f( target, stride, order, points );
   retain = CC->CompileFlag;
   (*CC->API.Map1f)( CC, target, u1, u2, stride, order, pnts, retain );
}


void APIENTRY glMap2d( GLenum target,
                       GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
                       GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
                       const GLdouble *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points2d( target, ustride, uorder,
                                vstride, vorder, points );
   retain = CC->CompileFlag;
   (*CC->API.Map2f)( CC, target, u1, u2, ustride, uorder,
                     v1, v2, vstride, vorder, pnts, retain );
}


void APIENTRY glMap2f( GLenum target,
                       GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
                       GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
                       const GLfloat *points )
{
   GLfloat *pnts;
   GLboolean retain;
   GET_CONTEXT;
   CHECK_CONTEXT;

   pnts = gl_copy_map_points2f( target, ustride, uorder,
                                vstride, vorder, points );
   retain = CC->CompileFlag;
   (*CC->API.Map2f)( CC, target, u1, u2, ustride, uorder,
                     v1, v2, vstride, vorder, pnts, retain );
}


void APIENTRY glMapGrid1d( GLint un, GLdouble u1, GLdouble u2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid1f)( CC, un, (GLfloat) u1, (GLfloat) u2 );
}


void APIENTRY glMapGrid1f( GLint un, GLfloat u1, GLfloat u2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid1f)( CC, un, u1, u2 );
}


void APIENTRY glMapGrid2d( GLint un, GLdouble u1, GLdouble u2,
                           GLint vn, GLdouble v1, GLdouble v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid2f)( CC, un, (GLfloat) u1, (GLfloat) u2,
                         vn, (GLfloat) v1, (GLfloat) v2 );
}


void APIENTRY glMapGrid2f( GLint un, GLfloat u1, GLfloat u2,
                           GLint vn, GLfloat v1, GLfloat v2 )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MapGrid2f)( CC, un, u1, u2, vn, v1, v2 );
}


void APIENTRY glMaterialf( GLenum face, GLenum pname, GLfloat param )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Materialfv)( CC, face, pname, &param );
}



void APIENTRY glMateriali( GLenum face, GLenum pname, GLint param )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   fparam[0] = (GLfloat) param;
   (*CC->API.Materialfv)( CC, face, pname, fparam );
}


void APIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.Materialfv)( CC, face, pname, params );
}


void APIENTRY glMaterialiv( GLenum face, GLenum pname, const GLint *params )
{
   GLfloat fparam[4];
   GET_CONTEXT;
   CHECK_CONTEXT;
   switch (pname) {
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_EMISSION:
      case GL_AMBIENT_AND_DIFFUSE:
         fparam[0] = INT_TO_FLOAT( params[0] );
         fparam[1] = INT_TO_FLOAT( params[1] );
         fparam[2] = INT_TO_FLOAT( params[2] );
         fparam[3] = INT_TO_FLOAT( params[3] );
         break;
      case GL_SHININESS:
         fparam[0] = (GLfloat) params[0];
         break;
      case GL_COLOR_INDEXES:
         fparam[0] = (GLfloat) params[0];
         fparam[1] = (GLfloat) params[1];
         fparam[2] = (GLfloat) params[2];
         break;
      default:
         /* Error will be caught later in gl_Materialfv */
         ;
   }
   (*CC->API.Materialfv)( CC, face, pname, fparam );
}


void APIENTRY glMatrixMode( GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MatrixMode)( CC, mode );
}


void APIENTRY glMultMatrixd( const GLdouble *m )
{
   GLfloat fm[16];
   GLuint i;
   GET_CONTEXT;
   CHECK_CONTEXT;

   for (i=0;i<16;i++) {
	  fm[i] = (GLfloat) m[i];
   }

   (*CC->API.MultMatrixf)( CC, fm );
}


void APIENTRY glMultMatrixf( const GLfloat *m )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.MultMatrixf)( CC, m );
}


void APIENTRY glNewList( GLuint list, GLenum mode )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.NewList)(CC, list, mode);
}

void APIENTRY glNormal3b( GLbyte nx, GLbyte ny, GLbyte nz )
{
   GET_CONTEXT;
   (*CC->API.Normal3f)( CC, BYTE_TO_FLOAT(nx),
                        BYTE_TO_FLOAT(ny), BYTE_TO_FLOAT(nz) );
}


void APIENTRY glNormal3d( GLdouble nx, GLdouble ny, GLdouble nz )
{
   GLfloat fx, fy, fz;
   GET_CONTEXT;
   if (ABSD(nx)<0.00001)   fx = 0.0F;   else  fx = nx;
   if (ABSD(ny)<0.00001)   fy = 0.0F;   else  fy = ny;
   if (ABSD(nz)<0.00001)   fz = 0.0F;   else  fz = nz;
   (*CC->API.Normal3f)( CC, fx, fy, fz );
}


void APIENTRY glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
   GET_CONTEXT;
#ifdef SHORTCUT
   if (CC->CompileFlag) {
      (*CC->Save.Normal3f)( CC, nx, ny, nz );
   }
   else {
      /* Execute */
      CC->Current.Normal[0] = nx;
      CC->Current.Normal[1] = ny;
      CC->Current.Normal[2] = nz;
      CC->VB->MonoNormal = GL_FALSE;
   }
#else
   (*CC->API.Normal3f)( CC, nx, ny, nz );
#endif
}


void APIENTRY glNormal3i( GLint nx, GLint ny, GLint nz )
{
   GET_CONTEXT;
   (*CC->API.Normal3f)( CC, INT_TO_FLOAT(nx),
                        INT_TO_FLOAT(ny), INT_TO_FLOAT(nz) );
}


void APIENTRY glNormal3s( GLshort nx, GLshort ny, GLshort nz )
{
   GET_CONTEXT;
   (*CC->API.Normal3f)( CC, SHORT_TO_FLOAT(nx),
                        SHORT_TO_FLOAT(ny), SHORT_TO_FLOAT(nz) );
}


void APIENTRY glNormal3bv( const GLbyte *v )
{
   GET_CONTEXT;
   (*CC->API.Normal3f)( CC, BYTE_TO_FLOAT(v[0]),
                        BYTE_TO_FLOAT(v[1]), BYTE_TO_FLOAT(v[2]) );
}


void APIENTRY glNormal3dv( const GLdouble *v )
{
   GLfloat fx, fy, fz;
   GET_CONTEXT;
   if (ABSD(v[0])<0.00001)   fx = 0.0F;   else  fx = v[0];
   if (ABSD(v[1])<0.00001)   fy = 0.0F;   else  fy = v[1];
   if (ABSD(v[2])<0.00001)   fz = 0.0F;   else  fz = v[2];
   (*CC->API.Normal3f)( CC, fx, fy, fz );
}


void APIENTRY glNormal3fv( const GLfloat *v )
{
   GET_CONTEXT;
#ifdef SHORTCUT
   if (CC->CompileFlag) {
      (*CC->Save.Normal3fv)( CC, v );
   }
   else {
      /* Execute */
      GLfloat *n = CC->Current.Normal;
      n[0] = v[0];
      n[1] = v[1];
      n[2] = v[2];
      CC->VB->MonoNormal = GL_FALSE;
   }
#else
   (*CC->API.Normal3fv)( CC, v );
#endif
}


void APIENTRY glNormal3iv( const GLint *v )
{
   GET_CONTEXT;
   (*CC->API.Normal3f)( CC, INT_TO_FLOAT(v[0]),
                        INT_TO_FLOAT(v[1]), INT_TO_FLOAT(v[2]) );
}


void APIENTRY glNormal3sv( const GLshort *v )
{
   GET_CONTEXT;
   (*CC->API.Normal3f)( CC, SHORT_TO_FLOAT(v[0]),
                        SHORT_TO_FLOAT(v[1]), SHORT_TO_FLOAT(v[2]) );
}


void APIENTRY glNormalPointer( GLenum type, GLsizei stride, const GLvoid *ptr )
{
   GET_CONTEXT;
   CHECK_CONTEXT;
   (*CC->API.NormalPointer)(CC, type, stride, ptr);
}

