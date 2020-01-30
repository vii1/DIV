/* $Id: draw.c,v 1.18 1997/03/08 02:02:52 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.2
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
 * $Log:   S:/scitech/src/mesa/draw.c_v  $
 * 
 *    Rev 1.1   01 Apr 1997 22:48:08   KendallB
 *  
 * 
 *    Rev 1.0   28 Mar 1997 17:18:28   KendallB
 *  
 * Revision 1.18  1997/03/08 02:02:52  brianp
 * vertex_feedback() didn't save the current normal
 *
 * Revision 1.17  1997/02/15 19:00:02  brianp
 * more robust, but slower, test for zero-area polygons
 *
 * Revision 1.16  1997/02/04 19:40:16  brianp
 * changed size of vlist[] array to VB_SIZE per Randy Frank
 *
 * Revision 1.15  1997/01/28 22:16:08  brianp
 * glBegin() now checks if already inside Begin/End pair
 *
 * Revision 1.14  1996/12/18 20:00:57  brianp
 * gl_set_material() now takes a bitmask instead of face and pname
 *
 * Revision 1.13  1996/12/07 10:20:44  brianp
 * call gl_set_material() instead of gl_Materialfv()
 *
 * Revision 1.12  1996/12/03 20:35:21  brianp
 * copy_vertex() didn't copy clip coordinates, caused line strip clipping bug
 *
 * Revision 1.11  1996/11/09 03:13:12  brianp
 * gl_transform_vb_part2() no longer calls render_vb()
 *
 * Revision 1.10  1996/11/08 02:20:25  brianp
 * new texture coordinate transformation code
 *
 * Revision 1.9  1996/11/04 02:31:19  brianp
 * Viewport.Sx and Viewport.Tx are now multiplied by DEPTH_SCALE
 *
 * Revision 1.8  1996/10/31 01:11:46  brianp
 * minor clean ups
 *
 * Revision 1.7  1996/10/12 18:23:06  brianp
 * window Z coord was incorrectly computed in render_clipped_line()
 *
 * Revision 1.6  1996/10/11 03:44:34  brianp
 * cleaned up the polygon offset code
 *
 * Revision 1.5  1996/09/27 01:26:16  brianp
 * removed unused variables
 *
 * Revision 1.4  1996/09/20 02:55:17  brianp
 * updated profiling code, removed old polygon offset code
 *
 * Revision 1.3  1996/09/15 14:17:30  brianp
 * now use GLframebuffer and GLvisual
 *
 * Revision 1.2  1996/09/15 01:48:58  brianp
 * removed #define NULL 0
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


/*
 * Draw points, lines, and polygons.
 */


#include "mesap.h"



#ifdef DEBUG
#  define ASSERT(X)   assert(X)
#else
#  define ASSERT(X)
#endif



#ifdef PROFILE
#  define START_PROFILE				\
	{					\
	   GLdouble t0 = gl_time();

#  define END_PROFILE( TIMER, COUNTER, INCR )	\
	   TIMER += (gl_time() - t0);		\
	   COUNTER += INCR;			\
	}
#else
#  define START_PROFILE
#  define END_PROFILE( TIMER, COUNTER, INCR )
#endif



/*
 * Check if the global material has to be updated with info that was
 * associated with a vertex via glMaterial.
 */
static void update_material( GLcontext *ctx, GLuint i )
{
   struct vertex_buffer *VB = ctx->VB;

   if (VB->MaterialMask[i]) {
      if (VB->MaterialMask[i] & FRONT_AMBIENT_BIT) {
         COPY_4V( ctx->Light.Material[0].Ambient, VB->Material[i][0].Ambient );
      }
      if (VB->MaterialMask[i] & BACK_AMBIENT_BIT) {
         COPY_4V( ctx->Light.Material[1].Ambient, VB->Material[i][1].Ambient );
      }
      if (VB->MaterialMask[i] & FRONT_DIFFUSE_BIT) {
         COPY_4V( ctx->Light.Material[0].Diffuse, VB->Material[i][0].Diffuse );
      }
      if (VB->MaterialMask[i] & BACK_DIFFUSE_BIT) {
         COPY_4V( ctx->Light.Material[1].Diffuse, VB->Material[i][1].Diffuse );
      }
      if (VB->MaterialMask[i] & FRONT_SPECULAR_BIT) {
         COPY_4V( ctx->Light.Material[0].Specular, VB->Material[i][0].Specular );
      }
      if (VB->MaterialMask[i] & BACK_SPECULAR_BIT) {
         COPY_4V( ctx->Light.Material[1].Specular, VB->Material[i][1].Specular );
      }
      if (VB->MaterialMask[i] & FRONT_EMISSION_BIT) {
         COPY_4V( ctx->Light.Material[0].Emission, VB->Material[i][0].Emission );
      }
      if (VB->MaterialMask[i] & BACK_EMISSION_BIT) {
         COPY_4V( ctx->Light.Material[1].Emission, VB->Material[i][1].Emission );
      }
      if (VB->MaterialMask[i] & FRONT_SHININESS_BIT) {
         ctx->Light.Material[0].Shininess = VB->Material[i][0].Shininess;
      }
      if (VB->MaterialMask[i] & BACK_SHININESS_BIT) {
         ctx->Light.Material[1].Shininess = VB->Material[i][1].Shininess;
      }
      if (VB->MaterialMask[i] & FRONT_INDEXES_BIT) {
         ctx->Light.Material[0].AmbientIndex = VB->Material[i][0].AmbientIndex;
         ctx->Light.Material[0].DiffuseIndex = VB->Material[i][0].DiffuseIndex;
         ctx->Light.Material[0].SpecularIndex = VB->Material[i][0].SpecularIndex;
      }
      if (VB->MaterialMask[i] & BACK_INDEXES_BIT) {
         ctx->Light.Material[1].AmbientIndex = VB->Material[i][1].AmbientIndex;
         ctx->Light.Material[1].DiffuseIndex = VB->Material[i][1].DiffuseIndex;
         ctx->Light.Material[1].SpecularIndex = VB->Material[i][1].SpecularIndex;
      }
      VB->MaterialMask[i] = 0;  /* reset now */
   }
}



/*
 * Render a line segment from VB[v1] to VB[v2] when either one or both
 * endpoints must be clipped.
 */
static void render_clipped_line( GLcontext *ctx, GLuint v1, GLuint v2 )
{
   GLfloat d;
   GLfloat ndc_x, ndc_y, ndc_z;
   GLuint provoking_vertex;
   struct vertex_buffer *VB = ctx->VB;

   /* which vertex dictates the color when flat shading: */
   provoking_vertex = v2;

   /*
    * Clipping may introduce new vertices.  New vertices will be stored
    * in the vertex buffer arrays starting with location VB->Free.  After
    * we've rendered the line, these extra vertices can be overwritten.
    */
   VB->Free = VB_MAX;

   /* Clip against user clipping planes */
   if (ctx->Transform.AnyClip) {
      GLuint orig_v1 = v1, orig_v2 = v2;
      if (gl_userclip_line( ctx, &v1, &v2 )==0)
	return;
      /* Apply projection matrix:  clip = Proj * eye */
      if (v1!=orig_v1) {
         TRANSFORM_POINT( VB->Clip[v1], ctx->ProjectionMatrix, VB->Eye[v1] );
      }
      if (v2!=orig_v2) {
         TRANSFORM_POINT( VB->Clip[v2], ctx->ProjectionMatrix, VB->Eye[v2] );
      }
   }

   /* Clip against view volume */
   if (gl_viewclip_line( ctx, &v1, &v2 )==0)
      return;

   /* Transform from clip coords to ndc:  ndc = clip / W */
   ASSERT( VB->Clip[v1][3] != 0.0 );
   d = 1.0F / VB->Clip[v1][3];
   ndc_x = VB->Clip[v1][0] * d;
   ndc_y = VB->Clip[v1][1] * d;
   ndc_z = VB->Clip[v1][2] * d;

   /* Map ndc coord to window coords. */
   VB->Win[v1][0] = ndc_x * ctx->Viewport.Sx + ctx->Viewport.Tx;
   VB->Win[v1][1] = ndc_y * ctx->Viewport.Sy + ctx->Viewport.Ty;
   VB->Win[v1][2] = ndc_z * ctx->Viewport.Sz + ctx->Viewport.Tz;

   /* Transform from clip coords to ndc:  ndc = clip / W */
   ASSERT( VB->Clip[v2][3] != 0.0 );
   d = 1.0F / VB->Clip[v2][3];
   ndc_x = VB->Clip[v2][0] * d;
   ndc_y = VB->Clip[v2][1] * d;
   ndc_z = VB->Clip[v2][2] * d;

   /* Map ndc coord to window coords. */
   VB->Win[v2][0] = ndc_x * ctx->Viewport.Sx + ctx->Viewport.Tx;
   VB->Win[v2][1] = ndc_y * ctx->Viewport.Sy + ctx->Viewport.Ty;
   VB->Win[v2][2] = ndc_z * ctx->Viewport.Sz + ctx->Viewport.Tz;

   START_PROFILE
   (*ctx->LineFunc)( ctx, v1, v2, provoking_vertex );
   END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
}



/*
 * Compute Z offsets for a polygon with plane defined by (A,B,C,D)
 * D is not needed.
 */
static void offset_polygon( GLcontext *ctx, GLfloat a, GLfloat b, GLfloat c )
{
   GLfloat ac, bc, m;
   GLfloat offset;

   if (c<0.001F && c>-0.001F) {
      /* to prevent underflow problems */
      offset = 0.0F;
   }
   else {
      ac = a / c;
      bc = b / c;
      if (ac<0.0F)  ac = -ac;
      if (bc<0.0F)  bc = -bc;
      m = MAX2( ac, bc );
      /* m = sqrt( ac*ac + bc*bc ); */

      offset = m * ctx->Polygon.OffsetFactor + ctx->Polygon.OffsetUnits;
   }

   ctx->PointZoffset   = ctx->Polygon.OffsetPoint ? offset : 0.0F;
   ctx->LineZoffset    = ctx->Polygon.OffsetLine  ? offset : 0.0F;
   ctx->PolygonZoffset = ctx->Polygon.OffsetFill  ? offset : 0.0F;
}



/*
 * When glPolygonMode() is used to specify that the front/back rendering
 * mode for polygons is not GL_FILL we end up calling this function.
 */
static void unfilled_polygon( GLcontext *ctx,
                              GLuint n, GLuint vlist[],
                              GLuint pv, GLuint facing )
{
   GLenum mode = facing ? ctx->Polygon.BackMode : ctx->Polygon.FrontMode;
   struct vertex_buffer *VB = ctx->VB;

   if (mode==GL_POINT) {
      GLint i, j;
      GLboolean edge;

      if (   ctx->Primitive==GL_TRIANGLES
          || ctx->Primitive==GL_QUADS
          || ctx->Primitive==GL_POLYGON) {
         edge = GL_FALSE;
      }
      else {
         edge = GL_TRUE;
      }

      for (i=0;i<n;i++) {
         j = vlist[i];
         if (edge || VB->Edgeflag[j]) {
            (*ctx->PointsFunc)( ctx, j, j );
         }
      }
   }
   else if (mode==GL_LINE) {
      GLuint i, j0, j1;
      GLboolean edge;

      ctx->StippleCounter = 0;

      if (   ctx->Primitive==GL_TRIANGLES
          || ctx->Primitive==GL_QUADS
          || ctx->Primitive==GL_POLYGON) {
         edge = GL_FALSE;
      }
      else {
         edge = GL_TRUE;
      }

      /* draw the edges */
      for (i=0;i<n;i++) {
         j0 = (i==0) ? vlist[n-1] : vlist[i-1];
         j1 = vlist[i];
         if (edge || VB->Edgeflag[j0]) {
            START_PROFILE
            (*ctx->LineFunc)( ctx, j0, j1, pv );
            END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
         }
      }
   }
   else {
      /* Fill the polygon */
      GLuint j0, i;
      j0 = vlist[0];
      for (i=2;i<n;i++) {
         START_PROFILE
         (*ctx->TriangleFunc)( ctx, j0, vlist[i-1], vlist[i], pv );
         END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
      }
   }
}


/*
 * Compute signed area of the n-sided polgyon specified by vertices vb->Win[]
 * and vertex list vlist[].
 * A clockwise polygon will return a negative area.
 * A counter-clockwise polygon will return a positive area.
 */
static GLfloat polygon_area( const struct vertex_buffer *vb,
                             GLuint n, const GLuint vlist[] )
{
   GLfloat area = 0.0F;
   GLint i;
   for (i=0;i<n;i++) {
      /* area = sum of trapezoids */
      GLuint j0 = vlist[i];
      GLuint j1 = vlist[(i+1)%n];
      GLfloat x0 = vb->Win[j0][0];
      GLfloat y0 = vb->Win[j0][1];
      GLfloat x1 = vb->Win[j1][0];
      GLfloat y1 = vb->Win[j1][1];
      GLfloat trapArea = (x0-x1)*(y0+y1);  /* Note: no divide by two here! */
      area += trapArea;
   }
   return area * 0.5F;     /* divide by two now! */
}


/*
 * Render a polygon in which doesn't have to be clipped.
 * Input:  n - number of vertices
 *         vlist - list of vertices in the polygon.
 *         odd_flag - if non-zero, reverse the orientation of the polygon
 */
static void render_polygon( GLcontext *ctx,
                            GLuint n, GLuint vlist[], GLuint odd_flag )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint pv;

   /* which vertex dictates the color when flat shading: */
   pv = (ctx->Primitive==GL_POLYGON) ? vlist[0] : vlist[n-1];

   /* Compute orientation of polygon, do cull test, offset, etc */
   {
      GLuint facing;   /* 0=front, 1=back */
      GLfloat area = polygon_area( VB, n, vlist );

      if (area==0.0F) {
         /* polygon has zero area, don't draw it */
         return;
      }

      facing = (area<0.0F) ^ odd_flag ^ (ctx->Polygon.FrontFace==GL_CW);

      if ((facing+1) & ctx->Polygon.CullBits) {
         return;   /* culled */
      }

      if (ctx->Polygon.OffsetAny) {
         /* compute plane equation of polygon, apply offset */
         GLuint j0 = vlist[0];
         GLuint j1 = vlist[1];
         GLuint j2 = vlist[2];
         GLuint j3 = vlist[ (n==3) ? 0 : 3 ];
         GLfloat ex = VB->Win[j1][0] - VB->Win[j3][0];
         GLfloat ey = VB->Win[j1][1] - VB->Win[j3][1];
         GLfloat ez = VB->Win[j1][2] - VB->Win[j3][2];
         GLfloat fx = VB->Win[j2][0] - VB->Win[j0][0];
         GLfloat fy = VB->Win[j2][1] - VB->Win[j0][1];
         GLfloat fz = VB->Win[j2][2] - VB->Win[j0][2];
         GLfloat a = ey*fz-ez*fy;
         GLfloat b = ez*fx-ex*fz;
         GLfloat c = ex*fy-ey*fx;
         offset_polygon( ctx, a, b, c );
      }

      if (ctx->Light.Model.TwoSide) {
         if (facing==1 && ctx->Light.Enabled) {
            /* use back color or index */
            VB->Color = VB->Bcolor;
            VB->Index = VB->Bindex;
         }
         else {
            /* use front color or index */
            VB->Color = VB->Fcolor;
            VB->Index = VB->Findex;
         }
      }

      /* Render the polygon! */
      if (ctx->Polygon.Unfilled) {
         unfilled_polygon( ctx, n, vlist, pv, facing );
      }
      else {
         /* Draw filled polygon as a triangle fan */
         GLint i;
         GLuint j0 = vlist[0];
         for (i=2;i<n;i++) {
            START_PROFILE
            (*ctx->TriangleFunc)( ctx, j0, vlist[i-1], vlist[i], pv );
            END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
         }
      }
   }
}



/*
 * Render a polygon in which at least one vertex has to be clipped.
 * Input:  n - number of vertices
 *         vlist - list of vertices in the polygon.
 *         odd_flag - if non-zero, reverse the orientation of the polygon
 */
static void render_clipped_polygon( GLcontext *ctx,
                                    GLuint n, GLuint vlist[], GLuint odd_flag )
{
   GLuint pv;
   struct vertex_buffer *VB = ctx->VB;
   GLfloat (*win)[3] = VB->Win;

   /* which vertex dictates the color when flat shading: */
   pv = (ctx->Primitive==GL_POLYGON) ? vlist[0] : vlist[n-1];

   /*
    * Clipping may introduce new vertices.  New vertices will be stored
    * in the vertex buffer arrays starting with location VB->Free.  After
    * we've rendered the polygon, these extra vertices can be overwritten.
    */
   VB->Free = VB_MAX;

   /* Clip against user clipping planes in eye coord space. */
   if (ctx->Transform.AnyClip) {
      GLfloat *proj = ctx->ProjectionMatrix;
      GLuint i;
      n = gl_userclip_polygon( ctx, n, vlist );
      if (n<3)
         return;
      /* Transform vertices from eye to clip coordinates:  clip = Proj * eye */
      for (i=0;i<n;i++) {
         GLuint j = vlist[i];
         TRANSFORM_POINT( VB->Clip[j], proj, VB->Eye[j] );
      }
   }

   /* Clip against view volume in clip coord space */
   n = gl_viewclip_polygon( ctx, n, vlist );
   if (n<3)
      return;

   /* Transform vertices from clip to ndc to window coords.        */
   /* ndc = clip / W    window = viewport_mapping(ndc)             */
   /* Note that window Z values are scaled to the range of integer */
   /* depth buffer values.                                         */
   {
      GLfloat sx = ctx->Viewport.Sx;
      GLfloat tx = ctx->Viewport.Tx;
      GLfloat sy = ctx->Viewport.Sy;
      GLfloat ty = ctx->Viewport.Ty;
      GLfloat sz = ctx->Viewport.Sz;
      GLfloat tz = ctx->Viewport.Tz;
      GLuint i;
      for (i=0;i<n;i++) {
         GLuint j = vlist[i];
         GLfloat d;
         ASSERT( VB->Clip[j][3] != 0.0 );
         d = 1.0F / VB->Clip[j][3];
         win[j][0] = VB->Clip[j][0] * d * sx + tx;
         win[j][1] = VB->Clip[j][1] * d * sy + ty;
         win[j][2] = VB->Clip[j][2] * d * sz + tz;
      }
   }

   /* Compute orientation of polygon, do cull test, offset, etc */
   {
      GLuint facing;   /* 0=front, 1=back */
      GLfloat area = polygon_area( VB, n, vlist );

      if (area==0.0F) {
         /* polygon has zero area, don't draw it */
         return;
      }

      facing = (area<0.0F) ^ odd_flag ^ (ctx->Polygon.FrontFace==GL_CW);

      if ((facing+1) & ctx->Polygon.CullBits) {
         return;   /* culled */
      }

      if (ctx->Polygon.OffsetAny) {
         /* compute plane equation of polygon, apply offset */
         GLuint j0 = vlist[0];
         GLuint j1 = vlist[1];
         GLuint j2 = vlist[2];
         GLuint j3 = vlist[ (n==3) ? 0 : 3 ];
         GLfloat ex = win[j1][0] - win[j3][0];
         GLfloat ey = win[j1][1] - win[j3][1];
         GLfloat ez = win[j1][2] - win[j3][2];
         GLfloat fx = win[j2][0] - win[j0][0];
         GLfloat fy = win[j2][1] - win[j0][1];
         GLfloat fz = win[j2][2] - win[j0][2];
         GLfloat a = ey*fz-ez*fy;
         GLfloat b = ez*fx-ex*fz;
         GLfloat c = ex*fy-ey*fx;
         offset_polygon( ctx, a, b, c );
      }

      if (ctx->Light.Model.TwoSide) {
         if (facing==1 && ctx->Light.Enabled) {
            /* use back color or index */
            VB->Color = VB->Bcolor;
            VB->Index = VB->Bindex;
         }
         else {
            /* use front color or index */
            VB->Color = VB->Fcolor;
            VB->Index = VB->Findex;
         }
      }

      /* Render the polygon! */
      if (ctx->Polygon.Unfilled) {
         unfilled_polygon( ctx, n, vlist, pv, facing );
      }
      else {
         /* Draw filled polygon as a triangle fan */
         GLint i;
         GLuint j0 = vlist[0];
         for (i=2;i<n;i++) {
            START_PROFILE
            (*ctx->TriangleFunc)( ctx, j0, vlist[i-1], vlist[i], pv );
            END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
         }
      }
   }
}



/*
 * Render an un-clipped triangle.
 */
static void render_triangle( GLcontext *ctx,
                             GLuint v0, GLuint v1, GLuint v2, GLuint pv,
                             GLuint odd_flag )
{
   struct vertex_buffer *VB = ctx->VB;
   GLfloat ex, ey, fx, fy, c;
   GLuint facing;  /* 0=front, 1=back */

   /* Compute orientation of triangle */
   ex = VB->Win[v1][0] - VB->Win[v0][0];
   ey = VB->Win[v1][1] - VB->Win[v0][1];
   fx = VB->Win[v2][0] - VB->Win[v0][0];
   fy = VB->Win[v2][1] - VB->Win[v0][1];
   c = ex*fy-ey*fx;

   if (c==0.0F) {
      /* polygon is perpindicular to view plane, don't draw it */
      return;
   }

   facing = (c<0.0F) ^ odd_flag ^ (ctx->Polygon.FrontFace==GL_CW);

   if ((facing+1) & ctx->Polygon.CullBits) {
      return;   /* culled */
   }

   if (ctx->Polygon.OffsetAny) {
      /* finish computing plane equation of polygon, compute offset */
      GLfloat fz = VB->Win[v2][2] - VB->Win[v0][2];
      GLfloat ez = VB->Win[v1][2] - VB->Win[v0][2];
      GLfloat a = ey*fz-ez*fy;
      GLfloat b = ez*fx-ex*fz;
      offset_polygon( ctx, a, b, c );
   }

   if (ctx->Light.Model.TwoSide) {
      if (facing==1 && ctx->Light.Enabled) {
         /* use back color or index */
         VB->Color = VB->Bcolor;
         VB->Index = VB->Bindex;
      }
      else {
         /* use front color or index */
         VB->Color = VB->Fcolor;
         VB->Index = VB->Findex;
      }
   }

   if (ctx->Polygon.Unfilled) {
      GLuint vlist[3];
      vlist[0] = v0;
      vlist[1] = v1;
      vlist[2] = v2;
      unfilled_polygon( ctx, 3, vlist, pv, facing );
   }
   else {
      START_PROFILE
      (*ctx->TriangleFunc)( ctx, v0, v1, v2, pv );
      END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
   }
}



/*
 * Render an un-clipped quadrilateral.
 */
static void render_quad( GLcontext *ctx,
                         GLuint v0, GLuint v1, GLuint v2, GLuint v3,
                         GLuint pv, GLuint odd_flag )
{
   struct vertex_buffer *VB = ctx->VB;
   GLfloat ex, ey, fx, fy, c;
   GLuint facing;  /* 0=front, 1=back */

   /* Compute polygon orientation */
   ex = VB->Win[v2][0] - VB->Win[v0][0];
   ey = VB->Win[v2][1] - VB->Win[v0][1];
   fx = VB->Win[v3][0] - VB->Win[v1][0];
   fy = VB->Win[v3][1] - VB->Win[v1][1];
   c = ex*fy-ey*fx;

   if (c==0.0F) {
      /* polygon is perpindicular to view plane, don't draw it */
      return;
   }

   facing = (c<0.0F) ^ odd_flag ^ (ctx->Polygon.FrontFace==GL_CW);

   if ((facing+1) & ctx->Polygon.CullBits) {
      return;   /* culled */
   }

   if (ctx->Polygon.OffsetAny) {
      /* finish computing plane equation of polygon, compute offset */
      GLfloat ez = VB->Win[v2][2] - VB->Win[v0][2];
      GLfloat fz = VB->Win[v3][2] - VB->Win[v1][2];
      GLfloat a = ey*fz-ez*fy;
      GLfloat b = ez*fx-ex*fz;
      offset_polygon( ctx, a, b, c );
   }

   if (ctx->Light.Model.TwoSide) {
      if (facing==1 && ctx->Light.Enabled) {
         /* use back color or index */
         VB->Color = VB->Bcolor;
         VB->Index = VB->Bindex;
      }
      else {
         /* use front color or index */
         VB->Color = VB->Fcolor;
         VB->Index = VB->Findex;
      }
   }

   /* Render the quad! */
   if (ctx->Polygon.Unfilled) {
      GLuint vlist[4];
      vlist[0] = v0;
      vlist[1] = v1;
      vlist[2] = v2;
      vlist[3] = v3;
      unfilled_polygon( ctx, 4, vlist, pv, facing );
   }
   else {
      START_PROFILE
      (*ctx->TriangleFunc)( ctx, v0, v1, v3, pv );
      (*ctx->TriangleFunc)( ctx, v1, v2, v3, pv );
      END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 2 )
   }
}



/*
 * When the vertex buffer is full, we transform/render it.  Sometimes we
 * have to copy the last vertex (or two) to the front of the vertex list
 * to "continue" the primitive.  For example:  line or triangle strips.
 * This function is a helper for that.
 */
static void copy_vertex( struct vertex_buffer *vb, GLuint dst, GLuint src )
{
   COPY_4V( vb->Clip[dst], vb->Clip[src] );
   COPY_4V( vb->Eye[dst], vb->Eye[src] );
   COPY_3V( vb->Win[dst], vb->Win[src] );
   COPY_4V( vb->Fcolor[dst], vb->Fcolor[src] );
   COPY_4V( vb->Bcolor[dst], vb->Bcolor[src] );
   COPY_4V( vb->TexCoord[dst], vb->TexCoord[src] );
   vb->Findex[dst] = vb->Findex[src];
   vb->Bindex[dst] = vb->Bindex[src];
   vb->Edgeflag[dst] = vb->Edgeflag[src];
   vb->Unclipped[dst] = vb->Unclipped[src];
}




/*
 * Either the vertex buffer is full (VB->Count==VB_MAX) or glEnd() has been
 * called.  Render the primitives defined by the vertices and reset the
 * buffer.
 * Input:  alldone - GL_TRUE = caller is glEnd()
 *                   GL_FALSE = calling because buffer is full.
 */
void gl_render_vb( GLcontext *ctx, GLboolean alldone )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint vlist[VB_SIZE];

   switch (ctx->Primitive) {
      case GL_POINTS:
         START_PROFILE
         (*ctx->PointsFunc)( ctx, 0, VB->Count-1 );
         END_PROFILE( ctx->PointTime, ctx->PointCount, VB->Count )
	 VB->Count = 0;
         VB->AnyClipped = GL_FALSE;
	 break;

      case GL_LINES:
         if (VB->AnyClipped) {
            GLuint i;
            for (i=1;i<VB->Count;i+=2) {
               if (VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  START_PROFILE
                  (*ctx->LineFunc)( ctx, i-1, i, i );
                  END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
               }
               else {
                  render_clipped_line( ctx, i-1, i );
               }
               ctx->StippleCounter = 0;
            }
         }
         else {
            GLuint i;
            for (i=1;i<VB->Count;i+=2) {
               START_PROFILE
               (*ctx->LineFunc)( ctx, i-1, i, i );
               END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
               ctx->StippleCounter = 0;
            }
         }
	 VB->Count = 0;
         VB->AnyClipped = GL_FALSE;
	 break;

      case GL_LINE_STRIP:
         if (VB->AnyClipped) {
            GLuint i;
	    for (i=1;i<VB->Count;i++) {
               if (VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  START_PROFILE
                  (*ctx->LineFunc)( ctx, i-1, i, i );
                  END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
               }
               else {
                  render_clipped_line( ctx, i-1, i );
               }
	    }
         }
         else {
            /* no clipping needed */
            GLuint i;
	    for (i=1;i<VB->Count;i++) {
               START_PROFILE
               (*ctx->LineFunc)( ctx, i-1, i, i );
               END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
            }
         }
         if (!alldone) {
            copy_vertex( VB, 0, VB->Count-1 );  /* copy last vertex to front */
            VB->Count = 1;
            VB->AnyClipped = VB->Unclipped[0] ? GL_FALSE : GL_TRUE;
	 }
         break;

      case GL_LINE_LOOP:
         {
            GLuint i;
            if (VB->Start==0) {
               i = 1;  /* start at 0th vertex */
            }
            else {
               i = 2;  /* skip first vertex, we're saving it until glEnd */
            }
            while (i<VB->Count) {
               if (VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  START_PROFILE
                  (*ctx->LineFunc)( ctx, i-1, i, i );
                  END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
               }
               else {
                  render_clipped_line( ctx, i-1, i );
               }
               i++;
            }
         }
	 if (alldone) {
            if (VB->Unclipped[VB->Count-1] & VB->Unclipped[0]) {
               START_PROFILE
               (*ctx->LineFunc)( ctx, VB->Count-1, 0, 0 );
               END_PROFILE( ctx->LineTime, ctx->LineCount, 1 )
            }
            else {
               render_clipped_line( ctx, VB->Count-1, 0 );
            }
	 }
	 else {
	    ASSERT(VB->Count==VB_MAX);
	    /* recycle the vertex list */
            copy_vertex( VB, 1, VB_MAX-1 );
	    VB->Count = 2;
            VB->AnyClipped = !VB->Unclipped[0] || !VB->Unclipped[1];
	 }
         break;

      case GL_TRIANGLES:
         if (VB->AnyClipped) {
            GLuint i;
            for (i=2;i<VB->Count;i+=3) {
               if (VB->Unclipped[i-2] & VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  if (ctx->DirectTriangles) {
                     START_PROFILE
                     (*ctx->TriangleFunc)( ctx, i-2, i-1, i, i );
                     END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
                  }
                  else {
                     render_triangle( ctx, i-2, i-1, i, i, 0 );
                  }
               }
               else {
                  vlist[0] = i-2;
                  vlist[1] = i-1;
                  vlist[2] = i-0;
                  render_clipped_polygon( ctx, 3, vlist, 0 );
               }
            }
         }
         else {
            /* no clipping needed */
            GLuint i;
            if (ctx->DirectTriangles) {
               for (i=2;i<VB->Count;i+=3) {
                  START_PROFILE
                  (*ctx->TriangleFunc)( ctx, i-2, i-1, i, i );
                  END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
               }
            }
            else {
               for (i=2;i<VB->Count;i+=3) {
                  render_triangle( ctx, i-2, i-1, i, i, 0 );
               }
            }
         }
	 VB->Count = 0;
         VB->AnyClipped = GL_FALSE;
	 break;

      case GL_TRIANGLE_STRIP:
         if (VB->AnyClipped) {
            GLuint i;
            for (i=2;i<VB->Count;i++) {
               if (VB->Unclipped[i-2] & VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  /* TODO: if direct triangle... */
                  if (ctx->DirectTriangles) {
                     START_PROFILE
                     (*ctx->TriangleFunc)( ctx, i-2, i-1, i, i );
                     END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
                  }
                  else {
                     render_triangle( ctx, i-2, i-1, i, i, i&1 );
                  }
               }
               else {
                  vlist[0] = i-2;
                  vlist[1] = i-1;
                  vlist[2] = i-0;
                  render_clipped_polygon( ctx, 3, vlist, i&1 );
               }
            }
         }
         else {
            /* no vertices were clipped */
            GLuint i;
            if (ctx->DirectTriangles) {
               for (i=2;i<VB->Count;i++) {
                  START_PROFILE
                  (*ctx->TriangleFunc)( ctx, i-2, i-1, i, i );
                  END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
               }
            }
            else {
               for (i=2;i<VB->Count;i++) {
                  render_triangle( ctx, i-2, i-1, i, i, i&1 );
               }
            }
         }
         if (!alldone) {
            /* get ready for more vertices in this triangle strip */
            copy_vertex( VB, 0, VB_MAX-2 );
            copy_vertex( VB, 1, VB_MAX-1 );
            VB->Count = 2;
            VB->AnyClipped = !VB->Unclipped[0] || !VB->Unclipped[1];
         }
	 break;

      case GL_TRIANGLE_FAN:
         if (VB->AnyClipped) {
            GLuint i;
            for (i=2;i<VB->Count;i++) {
               if (VB->Unclipped[0] & VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  if (ctx->DirectTriangles) {
                     START_PROFILE
                     (*ctx->TriangleFunc)( ctx, 0, i-1, i, i );
                     END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
                  }
                  else {
                     render_triangle( ctx, 0, i-1, i, i, 0 );
                  }
               }
               else {
                  vlist[0] = 0;
                  vlist[1] = i-1;
                  vlist[2] = i;
                  render_clipped_polygon( ctx, 3, vlist, 0 );
               }
            }
         }
         else {
            /* no clipping needed */
            GLuint i;
            if (ctx->DirectTriangles) {
               for (i=2;i<VB->Count;i++) {
                  START_PROFILE
                  (*ctx->TriangleFunc)( ctx, 0, i-1, i, i );
                  END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 1 )
               }
            }
            else {
               for (i=2;i<VB->Count;i++) {
                  render_triangle( ctx, 0, i-1, i, i, 0 );
               }
            }
         }
         if (!alldone) {
            /* get ready for more vertices in this triangle fan */
            copy_vertex( VB, 1, VB_MAX-1 );
            VB->Count = 2;
            VB->AnyClipped = !VB->Unclipped[0] || !VB->Unclipped[1];
	 }
	 break;

      case GL_QUADS:
         if (VB->AnyClipped) {
            GLuint i;
            for (i=3;i<VB->Count;i+=4) {
               if (  VB->Unclipped[i-3] & VB->Unclipped[i-2]
                   & VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  if (ctx->DirectTriangles) {
                     START_PROFILE
                     (*ctx->TriangleFunc)( ctx, i-3, i-2, i, i );
                     (*ctx->TriangleFunc)( ctx, i-2, i-1, i, i );
                     END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 2 )
                  }
                  else {
                     render_quad( ctx, i-3, i-2, i-1, i, i, 0 );
                  }
               }
               else {
                  vlist[0] = i-3;
                  vlist[1] = i-2;
                  vlist[2] = i-1;
                  vlist[3] = i-0;
                  render_clipped_polygon( ctx, 4, vlist, 0 );
               }
            }
         }
         else {
            /* no vertices were clipped */
            GLuint i;
            if (ctx->DirectTriangles) {
               for (i=3;i<VB->Count;i+=4) {
                  START_PROFILE
                  (*ctx->TriangleFunc)( ctx, i-3, i-2, i, i );
                  (*ctx->TriangleFunc)( ctx, i-2, i-1, i, i );
                  END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 2 )
               }
            }
            else {
               for (i=3;i<VB->Count;i+=4) {
                  render_quad( ctx, i-3, i-2, i-1, i, i, 0 );
               }
            }
         }
	 VB->Count = 0;
         VB->AnyClipped = GL_FALSE;
	 break;

      case GL_QUAD_STRIP:
         if (VB->AnyClipped) {
            GLuint i;
            for (i=3;i<VB->Count;i+=2) {
               if (  VB->Unclipped[i-2] & VB->Unclipped[i-3]
                   & VB->Unclipped[i-1] & VB->Unclipped[i]) {
                  if (ctx->DirectTriangles) {
                     START_PROFILE
                     (*ctx->TriangleFunc)( ctx, i-2, i-3, i, i );
                     (*ctx->TriangleFunc)( ctx, i-3, i-1, i, i );
                     END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 2 )
                  }
                  else {
                     render_quad( ctx, i-2, i-3, i-1, i, i, 1 );
                  }
               }
               else {
                  vlist[0] = i-2;
                  vlist[1] = i-3;
                  vlist[2] = i-1;
                  vlist[3] = i-0;
                  render_clipped_polygon( ctx, 4, vlist, 1 );
               }
            }
         }
         else {
            /* no clipping needed */
            GLuint i;
            if (ctx->DirectTriangles) {
               for (i=3;i<VB->Count;i+=2) {
                  START_PROFILE
                  (*ctx->TriangleFunc)( ctx, i-2, i-3, i, i );
                  (*ctx->TriangleFunc)( ctx, i-3, i-1, i, i );
                  END_PROFILE( ctx->PolygonTime, ctx->PolygonCount, 2 )
               }
            }
            else {
               for (i=3;i<VB->Count;i+=2) {
                  render_quad( ctx, i-2, i-3, i-1, i, i, 1 );
               }
            }
         }
         if (!alldone) {
            /* get ready for more vertices in this quad strip */
            copy_vertex( VB, 0, VB_MAX-2 );
            copy_vertex( VB, 1, VB_MAX-1 );
            VB->Count = 2;
            VB->AnyClipped = !VB->Unclipped[0] || !VB->Unclipped[1];
         }
	 break;

      case GL_POLYGON:
         if (VB->Count>2) {
            GLuint i;
            for (i=0;i<VB->Count;i++) {
               vlist[i] = i;
            }
            if (VB->AnyClipped) {
               render_clipped_polygon( ctx, VB->Count, vlist, 0 );
            }
            else {
               render_polygon( ctx, VB->Count, vlist, 0 );
            }
         }
	 if (!alldone) {
            /* get ready for more vertices just like a triangle fan */
            copy_vertex( VB, 1, VB_MAX-1 );
            VB->Count = 2;
            VB->AnyClipped = !VB->Unclipped[0] || !VB->Unclipped[1];
	 }
	 break;

      default:
         /* should never get here */
	 abort();
   }

   /* Start = first vertex which hasn't been transformed yet */
   VB->Start = VB->Count;
}



/*
 * Part 2 of Vertex Buffer transformation:  compute lighting, clipflags,
 * fog, texture coords, etc.
 * The function is called either by xform_vb_part1() or by glDrawArraysEXT().
 */
void gl_transform_vb_part2( GLcontext *ctx, GLboolean alldone )
{
   struct vertex_buffer *VB = ctx->VB;
#ifdef PROFILE
   GLdouble t0 = gl_time();
#endif

   ASSERT( VB->Count>0 );

   /* Lighting */
   if (ctx->Light.Enabled) {
      if (ctx->Visual->RGBAflag) {
         if (VB->MaterialChanges) {
            GLuint i;
            /* NOTE the <= here.  This is needed in case glColor/glMaterial
             * is called after the last glVertex inside a glBegin/glEnd pair.
             */
	    for (i=VB->Start;i<=VB->Count;i++) {
               update_material( ctx, i );
	       gl_color_shade_vertices( ctx, 1, &VB->Eye[i], &VB->Normal[i],
                                        ctx->LightTwoSide,
                                        &VB->Fcolor[i], &VB->Bcolor[i] );
	    }
	 }
         else {
            if (ctx->Light.Fast) {
               /* call optimized shader */
               gl_color_shade_vertices_fast( ctx, VB->Count-VB->Start,
                                             VB->Eye + VB->Start,
                                             VB->Normal + VB->Start,
                                             ctx->LightTwoSide,
                                             VB->Fcolor + VB->Start,
                                             VB->Bcolor + VB->Start );
            }
            else {
               /* call full-featured shader */
               gl_color_shade_vertices( ctx, VB->Count-VB->Start,
                                        VB->Eye + VB->Start,
                                        VB->Normal + VB->Start,
                                        ctx->LightTwoSide,
                                        VB->Fcolor + VB->Start,
                                        VB->Bcolor + VB->Start );
            }
	 }
      }
      else {
         if (VB->MaterialChanges) {
            GLuint i;
            /* NOTE the <= here.  This is needed in case glColor/glMaterial
             * is called after the last glVertex inside a glBegin/glEnd pair.
             */
            for (i=VB->Start;i<=VB->Count;i++) {
               update_material( ctx, i );
               gl_index_shade_vertices( ctx, 1, &VB->Eye[i], &VB->Normal[i],
                                        ctx->LightTwoSide,
                                        &VB->Findex[i], &VB->Bindex[i] );
            }
         }
         else {
            gl_index_shade_vertices( ctx, VB->Count-VB->Start,
                                     VB->Eye + VB->Start, VB->Normal + VB->Start,
                                     ctx->LightTwoSide,
                                     VB->Findex + VB->Start, VB->Bindex + VB->Start );
         }
      }
   }

   /* Per-vertex fog */
   if (ctx->Fog.Enabled && ctx->Hint.Fog!=GL_NICEST) {
      if (ctx->Visual->RGBAflag) {
         /* Fog RGB colors */
         gl_fog_color_vertices( ctx, VB->Count - VB->Start,
                                VB->Eye + VB->Start,
                                VB->Fcolor + VB->Start );
         if (ctx->LightTwoSide) {
            gl_fog_color_vertices( ctx, VB->Count - VB->Start,
                                   VB->Eye + VB->Start,
                                   VB->Bcolor + VB->Start );
         }
      }
      else {
         /* Fog color indexes */
         gl_fog_index_vertices( ctx, VB->Count - VB->Start,
                                VB->Eye + VB->Start,
                                VB->Findex + VB->Start );
         if (ctx->LightTwoSide) {
            gl_fog_index_vertices( ctx, VB->Count - VB->Start,
                                   VB->Eye + VB->Start,
                                   VB->Bindex + VB->Start );
         }
      }
   }

   /* Generate/transform texture coords */
   if (ctx->Texture.Enabled || ctx->RenderMode==GL_FEEDBACK) {
      if (ctx->Texture.TexGenEnabled) {
         gl_texgen( ctx, VB->Count - VB->Start,
                    VB->Obj + VB->Start,
                    VB->Eye + VB->Start,
                    VB->Normal + VB->Start,
                    VB->TexCoord + VB->Start );
      }
      if (!ctx->IdentityTexMat) {
         gl_xform_texcoords_4fv( VB->Count - VB->Start,
                                 VB->TexCoord + VB->Start,
                                 ctx->TextureMatrix );
      }
   }

   /* Initialize clip flags */
   MEMSET( VB->Unclipped+VB->Start, GL_TRUE, VB->Count-VB->Start );

   if (ctx->Transform.AnyClip) {
      /* Clip against user-defined clip planes */
      GLuint p;
      for (p=0;p<MAX_CLIP_PLANES;p++) {
         if (ctx->Transform.ClipEnabled[p]) {
            GLuint i;
            GLfloat a = ctx->Transform.ClipEquation[p][0];
            GLfloat b = ctx->Transform.ClipEquation[p][1];
            GLfloat c = ctx->Transform.ClipEquation[p][2];
            GLfloat d = ctx->Transform.ClipEquation[p][3];
            for (i=VB->Start;i<VB->Count;i++) {
               GLfloat dot = VB->Eye[i][0] * a + VB->Eye[i][1] * b
                           + VB->Eye[i][2] * c + VB->Eye[i][3] * d;
               if (dot < 0.0F) {
                  VB->Unclipped[i] = GL_FALSE;
                  VB->AnyClipped = GL_TRUE;
               }
            }
         }
      }
   }

   /* Transform vertices from eye to clip coords */
   /* Even transform clipped vertices because it's usually faster. */
   gl_xform_points_4fv( VB->Count-VB->Start, VB->Clip+VB->Start,
                        ctx->ProjectionMatrix, VB->Eye+VB->Start );

   /*
    * Combined clip testing with clip-to-window coordinate mapping.
    */
   {
      GLfloat sx = ctx->Viewport.Sx;
      GLfloat tx = ctx->Viewport.Tx;
      GLfloat sy = ctx->Viewport.Sy;
      GLfloat ty = ctx->Viewport.Ty;
      GLfloat sz = ctx->Viewport.Sz;
      GLfloat tz = ctx->Viewport.Tz;
      GLuint i, start = VB->Start, n = VB->Count;
      for (i=start;i<n;i++) {
         GLfloat clipx = VB->Clip[i][0], clipy = VB->Clip[i][1];
         GLfloat clipz = VB->Clip[i][2], clipw = VB->Clip[i][3];
         if (clipx > clipw || clipx < -clipw ||
             clipy > clipw || clipy < -clipw ||
             clipz > clipw || clipz < -clipw ) {
            /* vertex is clipped */
            VB->Unclipped[i] = GL_FALSE;
            VB->AnyClipped = GL_TRUE;
         }
         else {
            /* vertex not clipped */
            GLfloat d = 1.0F / clipw;
            VB->Win[i][0] = clipx * d * sx + tx;
            VB->Win[i][1] = clipy * d * sy + ty;
            VB->Win[i][2] = clipz * d * sz + tz;
         }
      }
   }

#ifdef PROFILE
   ctx->VertexTime += gl_time() - t0;
   ctx->VertexCount += VB->Count - VB->Start;
#endif
}



/*
 * When the Vertex Buffer is full, this function transforms all the
 * vertices and normals then calls xform_vb_part2()...
 */
void gl_transform_vb_part1( GLcontext *ctx, GLboolean alldone )
{
   struct vertex_buffer *VB = ctx->VB;
#ifdef PROFILE
   GLdouble t0 = gl_time();
#endif

   ASSERT( VB->Count>0 );

   /* Transform vertexes from object to eye coords */
   gl_xform_points_4fv( VB->Count-VB->Start, VB->Eye+VB->Start,
                        ctx->ModelViewMatrix, VB->Obj+VB->Start );

   /* Transform normals from object to eye coords */
   if (ctx->NeedNormals) {
      gl_xform_normals_3fv( VB->Count-VB->Start,
                            VB->Normal+VB->Start, ctx->ModelViewInv,
                            VB->Normal+VB->Start, ctx->Transform.Normalize );
   }

#ifdef PROFILE
   ctx->VertexTime += gl_time() - t0;
#endif

   /* lighting, project, etc */
   gl_transform_vb_part2( ctx, alldone );

   /* Render the primitives */
   gl_render_vb( ctx, alldone );
}



/**********************************************************************/
/**** Vertex functions.  These are called via ctx->Exec.Vertex4f() ****/
/**********************************************************************/

/*
 * Save a glVertex call into a display list AND execute it.
 */
void gl_save_and_execute_vertex( GLcontext *ctx,
                                 GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;  /* copy to local var to encourage optimization */

   /* Put vertex into display list */
   gl_save_Vertex4f( ctx, x, y, z, w );

   /* Exectue vertex command */
   ASSIGN_4V( VB->Obj[count], x, y, z, w );
   if (ctx->Visual->RGBAflag) {
      if (ctx->Light.Enabled) {
         /* need normal vector, vertex color is computed from material */
         COPY_3V( VB->Normal[count], ctx->Current.Normal );
      }
      else {
         /* not lighting, need vertex color */
         GLint shift = ctx->ColorShift;
         VB->Fcolor[count][0] = ctx->Current.IntColor[0] << shift;
         VB->Fcolor[count][1] = ctx->Current.IntColor[1] << shift;
         VB->Fcolor[count][2] = ctx->Current.IntColor[2] << shift;
         VB->Fcolor[count][3] = ctx->Current.IntColor[3] << shift;
      }
      if (ctx->Texture.Enabled) {
         COPY_4V( VB->TexCoord[count], ctx->Current.TexCoord );
      }
   }
   else {
      if (ctx->Light.Enabled) {
         /* need normal vector, vertex color index computed from material*/
         COPY_3V( VB->Normal[count], ctx->Current.Normal );
      }
      else {
         /* not lighting, new vertex color index */
         VB->Findex[count] = ctx->Current.Index;
      }
   }
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}


/*
 * Used when in feedback mode.
 */
static void vertex_feedback( GLcontext *ctx,
                             GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;
   GLint shift = ctx->ColorShift;

   /* vertex */
   ASSIGN_4V( VB->Obj[count], x, y, z, w );

   /* color */
   VB->Fcolor[count][0] = ctx->Current.IntColor[0] << shift;
   VB->Fcolor[count][1] = ctx->Current.IntColor[1] << shift;
   VB->Fcolor[count][2] = ctx->Current.IntColor[2] << shift;
   VB->Fcolor[count][3] = ctx->Current.IntColor[3] << shift;

   /* index */
   VB->Findex[count] = ctx->Current.Index;

   /* normal */
   COPY_3V( VB->Normal[count], ctx->Current.Normal );

   /* texcoord */
   COPY_4V( VB->TexCoord[count], ctx->Current.TexCoord );

   /* edgeflag */
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}


/* RGB, lit, textured vertex */
static void vertex_normal_texture( GLcontext *ctx,
                                   GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;

   ASSIGN_4V( VB->Obj[count], x, y, z, w );
   COPY_3V( VB->Normal[count], ctx->Current.Normal );
   COPY_4V( VB->TexCoord[count], ctx->Current.TexCoord );
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}


/* RGB or CI, lit, untextured vertex */
static void vertex_normal( GLcontext *ctx,
                           GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;

   ASSIGN_4V( VB->Obj[count], x, y, z, w );
   COPY_3V( VB->Normal[count], ctx->Current.Normal );
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}


/* RGB, unlit, textured vertex */
static void vertex_texture( GLcontext *ctx,
                            GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;
   GLint shift = ctx->ColorShift;

   ASSIGN_4V( VB->Obj[count], x, y, z, w );
   VB->Fcolor[count][0] = ctx->Current.IntColor[0] << shift;
   VB->Fcolor[count][1] = ctx->Current.IntColor[1] << shift;
   VB->Fcolor[count][2] = ctx->Current.IntColor[2] << shift;
   VB->Fcolor[count][3] = ctx->Current.IntColor[3] << shift;
   COPY_4V( VB->TexCoord[count], ctx->Current.TexCoord );
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}


/* RGB, unlit, untextured vertex */
static void vertex_color( GLcontext *ctx,
                          GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;
   GLint shift = ctx->ColorShift;

   ASSIGN_4V( VB->Obj[count], x, y, z, w );
   VB->Fcolor[count][0] = ctx->Current.IntColor[0] << shift;
   VB->Fcolor[count][1] = ctx->Current.IntColor[1] << shift;
   VB->Fcolor[count][2] = ctx->Current.IntColor[2] << shift;
   VB->Fcolor[count][3] = ctx->Current.IntColor[3] << shift;
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}


/* CI, unlit vertex */
static void vertex_index( GLcontext *ctx,
                          GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;

   ASSIGN_4V( VB->Obj[count], x, y, z, w );
   VB->Findex[count] = ctx->Current.Index;
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}


/*
 * Called when outside glBegin/glEnd, raises an error.
 */
void gl_nop_vertex( GLcontext *ctx,
                    GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   gl_error( ctx, GL_INVALID_OPERATION, "glVertex" );
}



/*
 * This function examines the current GL state and sets the
 * ctx->Exec.Vertex4f pointer to point at the appropriate vertex function.
 */
void gl_set_vertex_function( GLcontext *ctx )
{
   ASSERT( !INSIDE_BEGIN_END(ctx) );
   if (ctx->RenderMode==GL_FEEDBACK) {
      ctx->Exec.Vertex4f = vertex_feedback;
   }
   else {
      if (ctx->Visual->RGBAflag) {
         if (ctx->Light.Enabled) {
            if (ctx->Texture.Enabled) {
               ctx->Exec.Vertex4f = vertex_normal_texture;
            }
            else {
               ctx->Exec.Vertex4f = vertex_normal;
            }
         }
         else {
            /* not lighting, need vertex color */
            if (ctx->Texture.Enabled) {
               ctx->Exec.Vertex4f = vertex_texture;
            }
            else {
               ctx->Exec.Vertex4f = vertex_color;
            }
         }
      }
      else {
         /* color index mode */
         if (ctx->Light.Enabled) {
            ctx->Exec.Vertex4f = vertex_normal;
         }
         else {
            ctx->Exec.Vertex4f = vertex_index;
         }
      }
   }

   if (!ctx->CompileFlag) {
      ctx->API.Vertex4f = ctx->Exec.Vertex4f;
   }
}



/*
 * Process a vertex produced by an evaluator.
 * Input:  vertex - the X,Y,Z,W vertex
 *         normal - normal vector
 *         color - 4 integer color components
 *         index - color index
 *         texcoord - texture coordinate
 */
void gl_eval_vertex( GLcontext *ctx,
                     const GLfloat vertex[4], const GLfloat normal[3],
		     const GLint color[4], GLuint index,
                     const GLfloat texcoord[4] )
{
   struct vertex_buffer *VB = ctx->VB;
   GLuint count = VB->Count;  /* copy to local var to encourage optimization */
   GLint shift = ctx->ColorShift;

   COPY_4V( VB->Obj[count], vertex );
   COPY_3V( VB->Normal[count], normal );
   VB->Fcolor[count][0] = color[0] << shift;
   VB->Fcolor[count][1] = color[1] << shift;
   VB->Fcolor[count][2] = color[2] << shift;
   VB->Fcolor[count][3] = color[3] << shift;
#ifdef GL_VERSION_1_1
   if (ctx->Light.ColorMaterialEnabled
       && (ctx->Eval.Map1Color4 || ctx->Eval.Map1Color4)) {
      GLfloat fcolor[4];
      fcolor[0] = color[0] * ctx->Visual->InvRedScale;
      fcolor[1] = color[1] * ctx->Visual->InvGreenScale;
      fcolor[2] = color[2] * ctx->Visual->InvBlueScale;
      fcolor[3] = color[3] * ctx->Visual->InvAlphaScale;
      gl_set_material( ctx, ctx->Light.ColorMaterialBitmask, fcolor );
   }
#endif
   VB->Findex[count] = index;
   COPY_4V( VB->TexCoord[count], texcoord );
   VB->Edgeflag[count] = ctx->Current.EdgeFlag;

   count++;
   VB->Count = count;
   if (count==VB_MAX) {
      gl_transform_vb_part1( ctx, GL_FALSE );
   }
}




void gl_RasterPos4f( GLcontext *ctx,
                     GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
   GLfloat v[4], eye[4], clip[4], ndc[3], d;

   v[0] = x;
   v[1] = y;
   v[2] = z;
   v[3] = w;

   /* transform v to eye coords:  eye = ModelView * v */
   TRANSFORM_POINT( eye, ctx->ModelViewMatrix, v );

   /* raster color */
   if (ctx->Light.Enabled) {
      GLfloat eyenorm[3];
      if (!ctx->ModelViewInvValid) {
	 gl_compute_modelview_inverse(ctx);
      }
      TRANSFORM_NORMAL( eyenorm[0], eyenorm[1], eyenorm[2], ctx->Current.Normal,
                        ctx->ModelViewInv );
      ctx->ColorShift = 0;  /* Colors only shifted when smooth shading */
      if (ctx->Visual->RGBAflag) {
         GLfixed color[4];  /* not really fixed point but integers */
         GLfixed bcolor[4]; /* not used, dummy arg */
         gl_color_shade_vertices( ctx, 1, &eye, &eyenorm, 0, &color, &bcolor );
         ctx->Current.RasterColor[0] = color[0] * ctx->Visual->InvRedScale;
         ctx->Current.RasterColor[1] = color[1] * ctx->Visual->InvGreenScale;
         ctx->Current.RasterColor[2] = color[2] * ctx->Visual->InvBlueScale;
         ctx->Current.RasterColor[3] = color[3] * ctx->Visual->InvAlphaScale;
      }
      else {
         GLuint dummy;
	 gl_index_shade_vertices( ctx, 1, &eye, &eyenorm, 0,
                                  &ctx->Current.RasterIndex, &dummy );
      }
   }
   else {
      /* use current color or index */
      if (ctx->Visual->RGBAflag) {
         ctx->Current.RasterColor[0] = ctx->Current.IntColor[0]
                                       * ctx->Visual->InvRedScale;
         ctx->Current.RasterColor[1] = ctx->Current.IntColor[1]
                                       * ctx->Visual->InvGreenScale;
         ctx->Current.RasterColor[2] = ctx->Current.IntColor[2]
                                       * ctx->Visual->InvBlueScale;
         ctx->Current.RasterColor[3] = ctx->Current.IntColor[3]
                                       * ctx->Visual->InvAlphaScale;
      }
      else {
	 ctx->Current.RasterIndex = ctx->Current.Index;
      }
   }

   /* clip to user clipping planes */
   if (gl_userclip_point(ctx, eye)==0) {
      ctx->Current.RasterPosValid = GL_FALSE;
      return;
   }

   /* compute raster distance */
   ctx->Current.RasterDistance = (GLfloat)
                      sqrt( eye[0]*eye[0] + eye[1]*eye[1] + eye[2]*eye[2] );

   /* apply projection matrix:  clip = Proj * eye */
   TRANSFORM_POINT( clip, ctx->ProjectionMatrix, eye );

   /* clip to view volume */
   if (gl_viewclip_point( clip )==0) {
      ctx->Current.RasterPosValid = GL_FALSE;
      return;
   }

   /* ndc = clip / W */
   ASSERT( clip[3]!=0.0 );
   d = 1.0F / clip[3];
   ndc[0] = clip[0] * d;
   ndc[1] = clip[1] * d;
   ndc[2] = clip[2] * d;

   ctx->Current.RasterPos[0] = ndc[0] * ctx->Viewport.Sx + ctx->Viewport.Tx;
   ctx->Current.RasterPos[1] = ndc[1] * ctx->Viewport.Sy + ctx->Viewport.Ty;
   ctx->Current.RasterPos[2] = (ndc[2] * ctx->Viewport.Sz + ctx->Viewport.Tz)
                               / DEPTH_SCALE;
   ctx->Current.RasterPos[3] = clip[3];
   ctx->Current.RasterPosValid = GL_TRUE;

   /* FOG??? */

   if (ctx->Texture.Enabled) {
      COPY_4V( ctx->Current.RasterTexCoord, ctx->Current.TexCoord );
   }

   if (ctx->RenderMode==GL_SELECT) {
      gl_update_hitflag( ctx, ctx->Current.RasterPos[2] );
   }

}




void gl_windowpos( GLcontext *ctx, GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{

   /* set raster position */
   ctx->Current.RasterPos[0] = x;
   ctx->Current.RasterPos[1] = y;
   ctx->Current.RasterPos[2] = CLAMP( z, 0.0F, 1.0F );
   ctx->Current.RasterPos[3] = w;

   ctx->Current.RasterPosValid = GL_TRUE;

   /* raster color */
   if (ctx->Light.Enabled) {
      GLfloat eye[4];
      GLfloat eyenorm[3];
      COPY_4V( eye, ctx->Current.RasterPos );
      if (!ctx->ModelViewInvValid) {
	 gl_compute_modelview_inverse(ctx);
      }
      TRANSFORM_NORMAL( eyenorm[0], eyenorm[1], eyenorm[2], ctx->Current.Normal,
                        ctx->ModelViewInv );
      ctx->ColorShift = 0;  /* Colors only shifted when smooth shading */
      if (ctx->Visual->RGBAflag) {
         GLfixed color[4];  /* not really fixed point but integers */
         GLfixed bcolor[4]; /* not used, dummy arg */
         gl_color_shade_vertices( ctx, 1, &eye, &eyenorm, 0, &color, &bcolor );
         ctx->Current.RasterColor[0] = (GLfloat) color[0] * ctx->Visual->InvRedScale;
         ctx->Current.RasterColor[1] = (GLfloat) color[1] * ctx->Visual->InvGreenScale;
         ctx->Current.RasterColor[2] = (GLfloat) color[2] * ctx->Visual->InvBlueScale;
         ctx->Current.RasterColor[3] = (GLfloat) color[3] * ctx->Visual->InvAlphaScale;
      }
      else {
         GLuint dummy;
	 gl_index_shade_vertices( ctx, 1, &eye, &eyenorm, 0,
                                  &ctx->Current.RasterIndex, &dummy );
      }
   }
   else {
      /* use current color or index */
      if (ctx->Visual->RGBAflag) {
         ctx->Current.RasterColor[0] = ctx->Current.IntColor[0] * ctx->Visual->InvRedScale;
         ctx->Current.RasterColor[1] = ctx->Current.IntColor[1] * ctx->Visual->InvGreenScale;
         ctx->Current.RasterColor[2] = ctx->Current.IntColor[2] * ctx->Visual->InvBlueScale;
         ctx->Current.RasterColor[3] = ctx->Current.IntColor[3] * ctx->Visual->InvAlphaScale;
      }
      else {
	 ctx->Current.RasterIndex = ctx->Current.Index;
      }
   }

   ctx->Current.RasterDistance = 0.0;

   if (ctx->Texture.Enabled) {
      COPY_4V( ctx->Current.RasterTexCoord, ctx->Current.TexCoord );
   }

   if (ctx->RenderMode==GL_SELECT) {
      gl_update_hitflag( ctx, ctx->Current.RasterPos[2] );
   }
}




#ifdef PROFILE
static GLdouble begin_time;
#endif


void gl_Begin( GLcontext *ctx, GLenum p )
{
   struct vertex_buffer *VB = ctx->VB;
   struct pixel_buffer *PB = ctx->PB;
#ifdef PROFILE
   begin_time = gl_time();
#endif

   if (INSIDE_BEGIN_END(ctx)) {
      gl_error( ctx, GL_INVALID_OPERATION, "glBegin" );
      return;
   }
   if (!ctx->ModelViewInvValid) {
      gl_compute_modelview_inverse(ctx);
   }
   if (ctx->NewState) {
      gl_update_state(ctx);
   }
   else if (ctx->Exec.Vertex4f==gl_nop_vertex) {
      gl_set_vertex_function(ctx);
   }

   if (ctx->Driver.Begin) {
      (*ctx->Driver.Begin)( ctx, p );
   }

   ctx->Primitive = p;
   VB->Start = VB->Count = 0;
   VB->AnyClipped = GL_FALSE;

   VB->MonoColor = ctx->MonoPixels;
   if (VB->MonoColor) {
      /* All pixels generated are likely to be the same color so have
       * the device driver set the "monocolor" now.
       */
      if (ctx->Visual->RGBAflag) {
         GLubyte r = ctx->Current.IntColor[0];
         GLubyte g = ctx->Current.IntColor[1];
         GLubyte b = ctx->Current.IntColor[2];
         GLubyte a = ctx->Current.IntColor[3];
         (*ctx->Driver.Color)( ctx, r, g, b, a );
      }
      else {
         (*ctx->Driver.Index)( ctx, ctx->Current.Index );
      }
   }

   /*
    * If flat shading, save integer vertex colors
    * else, save fixed-point (scaled) vertex colors
    */
   ctx->ColorShift = (ctx->Light.ShadeModel==GL_FLAT) ? 0 : FIXED_SHIFT;

   /* By default use front color/index.  Two-sided lighting may override. */
   VB->Color = VB->Fcolor;
   VB->Index = VB->Findex;

   switch (ctx->Primitive) {
      case GL_POINTS:
	 ctx->LightTwoSide = 0;
	 PB_INIT( PB, GL_POINT );
	 break;
      case GL_LINES:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
	 ctx->LightTwoSide = 0;
	 ctx->StippleCounter = 0;
	 PB_INIT( PB, GL_LINE );
         break;
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_QUADS:
      case GL_QUAD_STRIP:
      case GL_POLYGON:
	 ctx->LightTwoSide = (GLuint) ctx->Light.Model.TwoSide;
	 PB_INIT( PB, GL_POLYGON );
         break;
      default:
	 gl_error( ctx, GL_INVALID_ENUM, "glBegin" );
	 ctx->Primitive = GL_BITMAP;
   }
}



void gl_End( GLcontext *ctx )
{
   struct pixel_buffer *PB = ctx->PB;
   struct vertex_buffer *VB = ctx->VB;

   if (ctx->Primitive==GL_BITMAP) {
      /* glEnd without glBegin */
      gl_error( ctx, GL_INVALID_OPERATION, "glEnd" );
      return;
   }

   if (VB->Count>VB->Start) {
      gl_transform_vb_part1( ctx, GL_TRUE );
   }
   if (PB->count>0) {
      gl_flush_pb(ctx);
   }
   PB->primitive = ctx->Primitive = GL_BITMAP;  /* Default mode */
   VB->MaterialChanges = GL_FALSE;

   if (ctx->Driver.End) {
      (*ctx->Driver.End)(ctx);
   }

#ifdef PROFILE
   ctx->BeginEndTime += gl_time() - begin_time;
   ctx->BeginEndCount++;
#endif
}

