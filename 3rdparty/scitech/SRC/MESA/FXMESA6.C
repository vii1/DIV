/* -*- mode: C; tab-width:8;  -*-

             fxmesa2.c - 3Dfx VooDoo RenderVB driver function support
*/

/*
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
 *
 * See the file fxmesa1.c for more informations about authors
 *
 */

#if defined(FX)

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "fxdrv.h"

#include "vbrender.h"

/*
 * Render a line segment from VB[v1] to VB[v2] when either one or both
 * endpoints must be clipped.
 */
static void render_clipped_line( GLcontext *ctx, GLuint v1, GLuint v2 )
{
  GLfloat ndc_x,ndc_y,ndc_z;
  struct vertex_buffer *VB=ctx->VB;
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GrVertex *gwin=fxMesa->gwin;

  /*
   * Clipping may introduce new vertices.  New vertices will be stored
   * in the vertex buffer arrays starting with location VB->Free.  After
   * we've rendered the line, these extra vertices can be overwritten.
   */
  VB->Free = VB_MAX;

  /* Clip against view volume */
  if (gl_viewclip_line( ctx, &v1, &v2 )==0)
    return;

  /* Transform from clip coords to ndc:  ndc = clip / W */
  if (VB->Clip[v1][3] != 0.0F) {
    GLfloat wInv = 1.0F / VB->Clip[v1][3];
    ndc_x = VB->Clip[v1][0] * wInv;
    ndc_y = VB->Clip[v1][1] * wInv;
    ndc_z = VB->Clip[v1][2] * wInv;
  } else {
    /* Can't divide by zero, so... */
    ndc_x = ndc_y = ndc_z = 0.0F;
  }

  /* Map ndc coord to window coords. */
  VB->Win[v1][0] = ndc_x * ctx->Viewport.Sx + ctx->Viewport.Tx;
  VB->Win[v1][1] = ndc_y * ctx->Viewport.Sy + ctx->Viewport.Ty;
  VB->Win[v1][2] = ndc_z * ctx->Viewport.Sz + ctx->Viewport.Tz;

  /* Transform from clip coords to ndc:  ndc = clip / W */
  if (VB->Clip[v2][3] != 0.0F) {
    GLfloat wInv = 1.0F / VB->Clip[v2][3];
    ndc_x = VB->Clip[v2][0] * wInv;
    ndc_y = VB->Clip[v2][1] * wInv;
    ndc_z = VB->Clip[v2][2] * wInv;
  } else {
    /* Can't divide by zero, so... */
    ndc_x = ndc_y = ndc_z = 0.0F;
  }

  /* Map ndc coord to window coords. */
  VB->Win[v2][0] = ndc_x * ctx->Viewport.Sx + ctx->Viewport.Tx;
  VB->Win[v2][1] = ndc_y * ctx->Viewport.Sy + ctx->Viewport.Ty;
  VB->Win[v2][2] = ndc_z * ctx->Viewport.Sz + ctx->Viewport.Tz;

  (*ctx->Driver.RasterSetup)(ctx,v1,v1+1);
  (*ctx->Driver.RasterSetup)(ctx,v2,v2+1);

  grDrawLine(&gwin[v1],&gwin[v2]);
}

/*
 * Render a polygon in which doesn't have to be clipped.
 * Input:  n - number of vertices
 *         vlist - list of vertices in the polygon.
 */
static void render_polygon(GLcontext *ctx, GLuint n, GLuint vlist[])
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GrVertex *gwin=fxMesa->gwin;
  GLint i;
  GLuint j0=vlist[0];

  for (i=2;i<n;i++)
    grDrawTriangle(&gwin[j0],&gwin[vlist[i-1]],&gwin[vlist[i]]);
}

/*
 * Render a polygon in which at least one vertex has to be clipped.
 * Input:  n - number of vertices
 *         vlist - list of vertices in the polygon.
 *                 CCW order = front facing.
 */
static void render_clipped_polygon( GLcontext *ctx, GLuint n, GLuint vlist[] )
{
  struct vertex_buffer *VB = ctx->VB;
  GLfloat (*win)[3] = VB->Win;
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GrVertex *gwin=fxMesa->gwin;

  /*
   * Clipping may introduce new vertices.  New vertices will be stored
   * in the vertex buffer arrays starting with location VB->Free.  After
   * we've rendered the polygon, these extra vertices can be overwritten.
   */
  VB->Free = VB_MAX;

  /* Clip against view volume in clip coord space */
  n = gl_viewclip_polygon( ctx, n, vlist );
  if (n<3)
    return;

  /* Transform new vertices from clip to ndc to window coords.    */
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

    /* Only need to compute window coords for new vertices */
    for (i=VB_MAX; i<VB->Free; i++) {
      if (VB->Clip[i][3] != 0.0F) {
	GLfloat wInv = 1.0F / VB->Clip[i][3];
	win[i][0] = VB->Clip[i][0] * wInv * sx + tx;
	win[i][1] = VB->Clip[i][1] * wInv * sy + ty;
	win[i][2] = VB->Clip[i][2] * wInv * sz + tz;
      }
      else {
	/* Can't divide by zero, so... */
	win[i][0] = win[i][1] = win[i][2] = 0.0F;
      }
    }

    if(VB->Free > VB_MAX)
      (*ctx->Driver.RasterSetup)(ctx,VB_MAX,VB->Free);
  }

  {
    GLint i;
    GLuint j0=vlist[0];

    for (i=2;i<n;i++)
      grDrawTriangle(&gwin[j0],&gwin[vlist[i-1]],&gwin[vlist[i]]);
  }
}

/************************************************************************/
/************************ RenderVB functions ****************************/
/************************************************************************/

#undef FXRENDERVB_NAME
#undef RVB_COLOR
#define FXRENDERVB_NAME renderVB_flat
#define RVB_COLOR(pv) grConstantColorValue(FXCOLOR(VB->Color[pv][0], \
						   VB->Color[pv][1], \
						   VB->Color[pv][2], \
						   VB->Color[pv][3]));
#include "fxmesa6.h"

#undef FXRENDERVB_NAME
#undef RVB_COLOR
#define FXRENDERVB_NAME renderVB_smooth
#define RVB_COLOR(pv)
#include "fxmesa6.h"

#undef FXRENDERVB_NAME
#undef RVB_COLOR

rendervb_func choose_rendervb_function(GLcontext *ctx)
{
  if((ctx->RasterMask & STENCIL_BIT) ||
     (ctx->Polygon.OffsetAny) ||
     (ctx->LightTwoSide) ||
     (ctx->Polygon.Unfilled) ||
     (ctx->Transform.AnyClip) ||
     (ctx->RenderMode!=GL_RENDER)) {
    grCullMode(GR_CULL_DISABLE);
    return NULL;
  }

  if(ctx->Polygon.CullFlag) {
    switch(ctx->Polygon.CullFaceMode) {
    case GL_BACK:
      if(ctx->Polygon.FrontFace==GL_CCW)
	grCullMode(GR_CULL_NEGATIVE);
      else
	grCullMode(GR_CULL_POSITIVE);
      break;
    case GL_FRONT:
      if(ctx->Polygon.FrontFace==GL_CCW)
	grCullMode(GR_CULL_POSITIVE);
      else
	grCullMode(GR_CULL_NEGATIVE);
      break;
    case GL_FRONT_AND_BACK:
      return NULL;
    }
  } else
    grCullMode(GR_CULL_DISABLE);

  if(ctx->Light.ShadeModel==GL_SMOOTH)
    return renderVB_smooth;

  return renderVB_flat;
}


#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function6(void)
{
  return 0;
}

#endif  /* FX */
