/* $Id: project.c,v 1.5 1997/07/24 01:28:44 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.4
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
 * $Log: project.c,v $
 * Revision 1.5  1997/07/24 01:28:44  brianp
 * changed precompiled header symbol from PCH to PC_HEADER
 *
 * Revision 1.4  1997/05/28 02:29:38  brianp
 * added support for precompiled headers (PCH), inserted APIENTRY keyword
 *
 * Revision 1.3  1997/04/11 23:22:42  brianp
 * added divide by zero checks to gluProject() and gluUnproject()
 *
 * Revision 1.2  1997/01/29 19:05:29  brianp
 * faster invert_matrix() function from Stephane Rehel
 *
 * Revision 1.1  1996/09/27 01:19:39  brianp
 * Initial revision
 *
 */


#ifdef PC_HEADER
#include "all.h"
#else
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gluP.h"
#endif


/*
 * This code was contributed by Marc Buffat (buffat@mecaflu.ec-lyon.fr).
 * Thanks Marc!!!
 */



/* implementation de gluProject et gluUnproject */
/* M. Buffat 17/2/95 */



/*
 * Transform a point (column vector) by a 4x4 matrix.  I.e.  out = m * in
 * Input:  m - the 4x4 matrix
 *         in - the 4x1 vector
 * Output:  out - the resulting 4x1 vector.
 */
static void transform_point( GLdouble out[4], const GLdouble m[16],
			     const GLdouble in[4] )
{
#define M(row,col)  m[col*4+row]
   out[0] = M(0,0) * in[0] + M(0,1) * in[1] + M(0,2) * in[2] + M(0,3) * in[3];
   out[1] = M(1,0) * in[0] + M(1,1) * in[1] + M(1,2) * in[2] + M(1,3) * in[3];
   out[2] = M(2,0) * in[0] + M(2,1) * in[1] + M(2,2) * in[2] + M(2,3) * in[3];
   out[3] = M(3,0) * in[0] + M(3,1) * in[1] + M(3,2) * in[2] + M(3,3) * in[3];
#undef M
}




/*
 * Perform a 4x4 matrix multiplication  (product = a x b).
 * Input:  a, b - matrices to multiply
 * Output:  product - product of a and b
 */
static void matmul( GLdouble *product, const GLdouble *a, const GLdouble *b )
{
   /* This matmul was contributed by Thomas Malik */
   GLdouble temp[16];
   GLint i;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define T(row,col)  temp[(col<<2)+row]

   /* i-te Zeile */
   for (i = 0; i < 4; i++)
     {
	T(i, 0) = A(i, 0) * B(0, 0) + A(i, 1) * B(1, 0) + A(i, 2) * B(2, 0) + A(i, 3) * B(3, 0);
	T(i, 1) = A(i, 0) * B(0, 1) + A(i, 1) * B(1, 1) + A(i, 2) * B(2, 1) + A(i, 3) * B(3, 1);
	T(i, 2) = A(i, 0) * B(0, 2) + A(i, 1) * B(1, 2) + A(i, 2) * B(2, 2) + A(i, 3) * B(3, 2);
	T(i, 3) = A(i, 0) * B(0, 3) + A(i, 1) * B(1, 3) + A(i, 2) * B(2, 3) + A(i, 3) * B(3, 3);
     }

#undef A
#undef B
#undef T
   MEMCPY( product, temp, 16*sizeof(GLdouble) );
}


static GLdouble Identity[16] = {
   1.0, 0.0, 0.0, 0.0,
   0.0, 1.0, 0.0, 0.0,
   0.0, 0.0, 1.0, 0.0,
   0.0, 0.0, 0.0, 1.0
};



/*
 * Compute the inverse of a 4x4 matrix.  Contributed by scotter@lafn.org
 */
static void invert_matrix_general( const GLdouble *m, GLdouble *out )
{

/* NB. OpenGL Matrices are COLUMN major. */
#define MAT(m,r,c) (m)[(c)*4+(r)]

/* Here's some shorthand converting standard (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

   GLdouble det;
   GLdouble d12, d13, d23, d24, d34, d41;
   GLdouble tmp[16]; /* Allow out == in. */

   /* Inverse = adjoint / det. (See linear algebra texts.)*/

   /* pre-compute 2x2 dets for last two rows when computing */
   /* cofactors of first two rows. */
   d12 = (m31*m42-m41*m32);
   d13 = (m31*m43-m41*m33);
   d23 = (m32*m43-m42*m33);
   d24 = (m32*m44-m42*m34);
   d34 = (m33*m44-m43*m34);
   d41 = (m34*m41-m44*m31);

   tmp[0] =  (m22 * d34 - m23 * d24 + m24 * d23);
   tmp[1] = -(m21 * d34 + m23 * d41 + m24 * d13);
   tmp[2] =  (m21 * d24 + m22 * d41 + m24 * d12);
   tmp[3] = -(m21 * d23 - m22 * d13 + m23 * d12);

   /* Compute determinant as early as possible using these cofactors. */
   det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2] + m14 * tmp[3];

   /* Run singularity test. */
   if (det == 0.0) {
      /* printf("invert_matrix: Warning: Singular matrix.\n"); */
      MEMCPY( out, Identity, 16*sizeof(GLdouble) );
   }
   else {
      GLdouble invDet = 1.0 / det;
      /* Compute rest of inverse. */
      tmp[0] *= invDet;
      tmp[1] *= invDet;
      tmp[2] *= invDet;
      tmp[3] *= invDet;

      tmp[4] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
      tmp[5] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
      tmp[6] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
      tmp[7] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12 = m11*m22-m21*m12;
      d13 = m11*m23-m21*m13;
      d23 = m12*m23-m22*m13;
      d24 = m12*m24-m22*m14;
      d34 = m13*m24-m23*m14;
      d41 = m14*m21-m24*m11;

      tmp[8] =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
      tmp[9] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
      tmp[10] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
      tmp[11] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
      tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
      tmp[13] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
      tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
      tmp[15] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;

      MEMCPY(out, tmp, 16*sizeof(GLdouble));
   }

#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}


/*
 * Invert matrix m.  This algorithm contributed by Stephane Rehel
 * <rehel@worldnet.fr>
 */
static void invert_matrix( const GLdouble *m, GLdouble *out )
{
/* NB. OpenGL Matrices are COLUMN major. */
#define MAT(m,r,c) (m)[(c)*4+(r)]

/* Here's some shorthand converting standard (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

   register GLdouble det;
   GLdouble tmp[16]; /* Allow out == in. */

   if( m41 != 0. || m42 != 0. || m43 != 0. || m44 != 1. ) {
      invert_matrix_general(m, out);
      return;
   }

   /* Inverse = adjoint / det. (See linear algebra texts.)*/

   tmp[0]= m22 * m33 - m23 * m32;
   tmp[1]= m23 * m31 - m21 * m33;
   tmp[2]= m21 * m32 - m22 * m31;

   /* Compute determinant as early as possible using these cofactors. */
   det= m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2];

   /* Run singularity test. */
   if (det == 0.0) {
      /* printf("invert_matrix: Warning: Singular matrix.\n"); */
      MEMCPY( out, Identity, 16*sizeof(GLdouble) );
   }
   else {
      GLdouble d12, d13, d23, d24, d34, d41;
      register GLdouble im11, im12, im13, im14;

      det= 1. / det;

      /* Compute rest of inverse. */
      tmp[0] *= det;
      tmp[1] *= det;
      tmp[2] *= det;
      tmp[3]  = 0.;

      im11= m11 * det;
      im12= m12 * det;
      im13= m13 * det;
      im14= m14 * det;
      tmp[4] = im13 * m32 - im12 * m33;
      tmp[5] = im11 * m33 - im13 * m31;
      tmp[6] = im12 * m31 - im11 * m32;
      tmp[7] = 0.;

      /* Pre-compute 2x2 dets for first two rows when computing */
      /* cofactors of last two rows. */
      d12 = im11*m22 - m21*im12;
      d13 = im11*m23 - m21*im13;
      d23 = im12*m23 - m22*im13;
      d24 = im12*m24 - m22*im14;
      d34 = im13*m24 - m23*im14;
      d41 = im14*m21 - m24*im11;

      tmp[8] =  d23;
      tmp[9] = -d13;
      tmp[10] = d12;
      tmp[11] = 0.;

      tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23);
      tmp[13] =  (m31 * d34 + m33 * d41 + m34 * d13);
      tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12);
      tmp[15] =  1.;

      MEMCPY(out, tmp, 16*sizeof(GLdouble));
  }

#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}



/* projection du point (objx,objy,obz) sur l'ecran (winx,winy,winz) */
GLint APIENTRY gluProject(GLdouble objx,GLdouble objy,GLdouble objz,
                          const GLdouble model[16],const GLdouble proj[16],
                          const GLint viewport[4],
                          GLdouble *winx,GLdouble *winy,GLdouble *winz)
{
    /* matrice de transformation */
    GLdouble in[4],out[4];

    /* initilise la matrice et le vecteur a transformer */
    in[0]=objx; in[1]=objy; in[2]=objz; in[3]=1.0;
    transform_point(out,model,in);
    transform_point(in,proj,out);

    /* d'ou le resultat normalise entre -1 et 1*/
    if (in[3]==0.0)
       return GL_FALSE;

    in[0]/=in[3]; in[1]/=in[3]; in[2]/=in[3];

    /* en coordonnees ecran */
    *winx = viewport[0]+(1+in[0])*viewport[2]/2;
    *winy = viewport[1]+(1+in[1])*viewport[3]/2;
    /* entre 0 et 1 suivant z */
    *winz = (1+in[2])/2;
    return GL_TRUE;
}



/* transformation du point ecran (winx,winy,winz) en point objet */
GLint APIENTRY gluUnProject(GLdouble winx,GLdouble winy,GLdouble winz,
                            const GLdouble model[16],const GLdouble proj[16],
                            const GLint viewport[4],
                            GLdouble *objx,GLdouble *objy,GLdouble *objz)
{
    /* matrice de transformation */
    GLdouble m[16], A[16];
    GLdouble in[4],out[4];

    /* transformation coordonnees normalisees entre -1 et 1 */
    in[0]=(winx-viewport[0])*2/viewport[2] - 1.0;
    in[1]=(winy-viewport[1])*2/viewport[3] - 1.0;
    in[2]=2*winz - 1.0;
    in[3]=1.0;

    /* calcul transformation inverse */
    matmul(A,proj,model);
    invert_matrix(A,m);

    /* d'ou les coordonnees objets */
    transform_point(out,m,in);
    if (out[3]==0.0)
       return GL_FALSE;
    *objx=out[0]/out[3];
    *objy=out[1]/out[3];
    *objz=out[2]/out[3];
    return GL_TRUE;
}

