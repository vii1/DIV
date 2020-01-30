/* $Id: vertex.h,v 1.1 1996/09/13 01:38:16 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.0
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
 * $Log:   S:/scitech/src/mesa/vertex.h_v  $
 * 
 *    Rev 1.0   28 Mar 1997 17:19:04   KendallB
 *  
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


#ifndef VERTEX_H
#define VERTEX_H


#include "types.h"


extern void gl_Normal3f( GLcontext *ctx, GLfloat nx, GLfloat ny, GLfloat nz );

extern void gl_Normal3fv( GLcontext *ctx, const GLfloat *normal );

extern void gl_Indexf( GLcontext *ctx, GLfloat c );

extern void gl_Indexi( GLcontext *ctx, GLint c );

extern void gl_Color4f( GLcontext *ctx,
                        GLfloat red, GLfloat green,
                        GLfloat blue, GLfloat alpha );

extern void gl_ColorMat4f( GLcontext *ctx,
                           GLfloat red, GLfloat green,
                           GLfloat blue, GLfloat alpha );


extern void gl_Color4ub( GLcontext *ctx,
                         GLubyte red, GLubyte green,
                         GLubyte blue, GLubyte alpha );

extern void gl_Color4ub8bit( GLcontext *ctx,
                             GLubyte red, GLubyte green,
                             GLubyte blue, GLubyte alpha );

extern void gl_ColorMat4ub( GLcontext *ctx,
                            GLubyte red, GLubyte green,
                            GLubyte blue, GLubyte alpha );


extern void gl_TexCoord4f( GLcontext *ctx,
                           GLfloat s, GLfloat t, GLfloat r, GLfloat q );


extern void gl_EdgeFlag( GLcontext *ctx, GLboolean flag );


#endif

