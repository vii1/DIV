/* $Id: vertex.c,v 1.4 1996/12/18 20:00:57 brianp Exp $ */

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
 * $Log:   S:/scitech/src/mesa/vertex.c_v  $
 * 
 *    Rev 1.1   01 Apr 1997 22:48:44   KendallB
 *  
 * 
 *    Rev 1.0   28 Mar 1997 17:19:04   KendallB
 *  
 * Revision 1.4  1996/12/18 20:00:57  brianp
 * gl_set_material() now takes a bitmask instead of face and pname
 *
 * Revision 1.3  1996/12/07 10:21:28  brianp
 * call gl_set_material() instead of gl_Materialfv()
 *
 * Revision 1.2  1996/09/15 14:19:16  brianp
 * now use GLframebuffer and GLvisual
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


#include "mesap.h"



#ifdef DEBUG
#  define ASSERT(X)  assert(X)
#else
#  define ASSERT(X)
#endif



void gl_Normal3f( GLcontext *ctx, GLfloat nx, GLfloat ny, GLfloat nz )
{
   ctx->Current.Normal[0] = nx;
   ctx->Current.Normal[1] = ny;
   ctx->Current.Normal[2] = nz;
}



void gl_Normal3fv( GLcontext *ctx, const GLfloat *n )
{
   ctx->Current.Normal[0] = n[0];
   ctx->Current.Normal[1] = n[1];
   ctx->Current.Normal[2] = n[2];
}




void gl_Indexf( GLcontext *ctx, GLfloat c )
{
   ctx->Current.Index = (GLuint) (GLint) c;
   ctx->VB->MonoColor = GL_FALSE;
}


void gl_Indexi( GLcontext *ctx, GLint c )
{
   ctx->Current.Index = (GLuint) c;
   ctx->VB->MonoColor = GL_FALSE;
}




void gl_Color4f( GLcontext *ctx,
                 GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
   ctx->Current.IntColor[0] = red   * ctx->Visual->RedScale;
   ctx->Current.IntColor[1] = green * ctx->Visual->GreenScale;
   ctx->Current.IntColor[2] = blue  * ctx->Visual->BlueScale;
   ctx->Current.IntColor[3] = alpha * ctx->Visual->AlphaScale;
   ASSERT( !ctx->Light.ColorMaterialEnabled );
   ctx->VB->MonoColor = GL_FALSE;
}



/* glColor() which modifies (a) material(s) */
void gl_ColorMat4f( GLcontext *ctx,
                    GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
   GLfloat color[4];
   ctx->Current.IntColor[0] = red   * ctx->Visual->RedScale;
   ctx->Current.IntColor[1] = green * ctx->Visual->GreenScale;
   ctx->Current.IntColor[2] = blue  * ctx->Visual->BlueScale;
   ctx->Current.IntColor[3] = alpha * ctx->Visual->AlphaScale;
   /* update material */
   ASSERT( ctx->Light.ColorMaterialEnabled );
   ASSIGN_4V( color, red, green, blue, alpha );
   gl_set_material( ctx, ctx->Light.ColorMaterialBitmask, color );
   ctx->VB->MonoColor = GL_FALSE;
}



/*
 * Used when colors are not scaled to [0,255]
 */
void gl_Color4ub( GLcontext *ctx,
                  GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
   ctx->Current.IntColor[0] = red   * ctx->Visual->RedScale   * (1.0F/255.0F);
   ctx->Current.IntColor[1] = green * ctx->Visual->GreenScale * (1.0F/255.0F);
   ctx->Current.IntColor[2] = blue  * ctx->Visual->BlueScale  * (1.0F/255.0F);
   ctx->Current.IntColor[3] = alpha * ctx->Visual->AlphaScale * (1.0F/255.0F);
   ASSERT( !ctx->Light.ColorMaterialEnabled );
   ctx->VB->MonoColor = GL_FALSE;
}


/*
 * Used when colors are scaled to [0,255].
 */
void gl_Color4ub8bit( GLcontext *ctx,
                      GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
   ASSIGN_4V( ctx->Current.IntColor, red, green, blue, alpha );
   ASSERT( !ctx->Light.ColorMaterialEnabled );
   ctx->VB->MonoColor = GL_FALSE;
}



/* glColor() which modifies (a) material(s) */
void gl_ColorMat4ub( GLcontext *ctx,
                     GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
   GLfloat color[4];
   if (ctx->Visual->EightBitColor) {
      ASSIGN_4V( ctx->Current.IntColor, red, green, blue, alpha );
   }
   else {
      ctx->Current.IntColor[0] = red   * ctx->Visual->RedScale   * (1.0F/255.0F);
      ctx->Current.IntColor[1] = green * ctx->Visual->GreenScale * (1.0F/255.0F);
      ctx->Current.IntColor[2] = blue  * ctx->Visual->BlueScale  * (1.0F/255.0F);
      ctx->Current.IntColor[3] = alpha * ctx->Visual->AlphaScale * (1.0F/255.0F);
   }
   /* update material */
   ASSERT( ctx->Light.ColorMaterialEnabled );
   color[0] = red   * (1.0F/255.0F);
   color[1] = green * (1.0F/255.0F);
   color[2] = blue  * (1.0F/255.0F);
   color[3] = alpha * (1.0F/255.0F);
   gl_set_material( ctx, ctx->Light.ColorMaterialBitmask, color );
   ctx->VB->MonoColor = GL_FALSE;
}




void gl_TexCoord4f( GLcontext *ctx,
                    GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
   ctx->Current.TexCoord[0] = s;
   ctx->Current.TexCoord[1] = t;
   ctx->Current.TexCoord[2] = r;
   ctx->Current.TexCoord[3] = q;
}



void gl_EdgeFlag( GLcontext *ctx, GLboolean flag )
{
   ctx->Current.EdgeFlag = flag;
}

