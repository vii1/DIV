/* $Id: winpos.h,v 1.1 1996/09/13 01:38:16 brianp Exp $ */

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
 * $Log: winpos.h,v $
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */



#ifndef WINPOS_H
#define WINPOS_H


#include "types.h"


#ifndef GL_MESA_window_pos

extern void glWindowPos2iMESA( GLint x, GLint y );
extern void glWindowPos2fMESA( GLfloat x, GLfloat y );
extern void glWindowPos2dMESA( GLdouble x, GLdouble y );

extern void glWindowPos2ivMESA( const GLint *p );
extern void glWindowPos2fvMESA( const GLfloat *p );
extern void glWindowPos2dvMESA( const GLdouble *p );

extern void glWindowPos3iMESA( GLint x, GLint y, GLint z );
extern void glWindowPos3fMESA( GLfloat x, GLfloat y, GLfloat z );
extern void glWindowPos3dMESA( GLdouble x, GLdouble y, GLdouble z );

extern void glWindowPos3ivMESA( const GLint *p );
extern void glWindowPos3fvMESA( const GLfloat *p );
extern void glWindowPos3dvMESA( const GLdouble *p );

extern void glWindowPos4iMESA( GLint x, GLint y, GLint z, GLint w );
extern void glWindowPos4fMESA( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void glWindowPos4dMESA( GLdouble x, GLdouble y, GLdouble z, GLdouble w);

extern void glWindowPos4ivMESA( const GLint *p );
extern void glWindowPos4fvMESA( const GLfloat *p );
extern void glWindowPos4dvMESA( const GLdouble *p );

#endif



extern void gl_WindowPos4fMESA( GLcontext *ctx,
                                GLfloat x, GLfloat y, GLfloat z, GLfloat w );


#endif
