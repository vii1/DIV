/* -*- mode: C; tab-width:8;  -*-

             fxmesa6.h - 3Dfx VooDoo RenderVB driver function support
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

/************************************************************************/
/************************ RenderVB function *****************************/
/************************************************************************/

static GLboolean FXRENDERVB_NAME (GLcontext *ctx, GLboolean allDone)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  struct vertex_buffer *VB=ctx->VB;
  GLuint vlist[VB_SIZE];
  GLenum prim=ctx->Primitive;

  /* A small optimization for GLQuake */

  if(prim==GL_POLYGON) {
    if(VB->Count==3)
      prim=GL_TRIANGLES;
    else if(VB->Count==4)
      prim=GL_QUADS;
  }

  switch (prim) {
  case GL_POINTS:
    (*ctx->Driver.PointsFunc)(ctx,0,VB->Count-1);
    break;

  case GL_LINES:
    if(VB->ClipOrMask) {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for(i=1;i<VB->Count;i+=2,gwin+=2) {
	RVB_COLOR(i);

	if (VB->ClipMask[i-1] | VB->ClipMask[i]) {
	  render_clipped_line(ctx,i-1,i);
	} else
	  grDrawLine(&gwin[0],&gwin[1]);

	ctx->StippleCounter = 0;
      }
    } else {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for(i=1;i<VB->Count;i+=2,gwin+=2) {
	RVB_COLOR(i);
	grDrawLine(&gwin[0],&gwin[1]);

	ctx->StippleCounter=0;
      }
    }
    break;

  case GL_LINE_STRIP:
    if(VB->ClipOrMask) {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for(i=1;i<VB->Count;i++,gwin++) {
	RVB_COLOR(i);

	if(VB->ClipMask[i-1] | VB->ClipMask[i])
	  render_clipped_line(ctx,i-1,i);
	else
	  grDrawLine(&gwin[0],&gwin[1]);
      }
    } else {
      /* no clipping needed */
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;
      
      for(i=1;i<VB->Count;i++,gwin++) {
	RVB_COLOR(i);
	grDrawLine(&gwin[0],&gwin[1]);
      }   
    }
    break;

  case GL_LINE_LOOP:
    {
      GLuint i;
      GrVertex *gwin;

      if(VB->Start==0) {
	i=1;  /* start at 0th vertex */
	gwin=fxMesa->gwin;
      } else {
	i=2;  /* skip first vertex, we're saving it until glEnd */
	gwin=&fxMesa->gwin[1];
      }

      while(i<VB->Count) {
	RVB_COLOR(i);
	
	if(VB->ClipMask[i-1] | VB->ClipMask[i])
	  render_clipped_line(ctx,i-1,i);
	else
	  grDrawLine(&gwin[0],&gwin[1]);

	i++;
	gwin++;
      }
    }
  break;

  case GL_TRIANGLES:
    if(VB->ClipOrMask) {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for(i=2;i<VB->Count;i+=3,gwin+=3) {
	if(VB->ClipMask[i-2] | VB->ClipMask[i-1] | VB->ClipMask[i]) {
	  vlist[0]=i-2;
	  vlist[1]=i-1;
	  vlist[2]=i-0;
	  RVB_COLOR(i);
	  render_clipped_polygon(ctx,3,vlist);
	} else {
	  RVB_COLOR(i);
	  grDrawTriangle(&gwin[0],&gwin[1],&gwin[2]);
	}
      }
    } else {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for(i=2;i<VB->Count;i+=3,gwin+=3) {
	RVB_COLOR(i);
	grDrawTriangle(&gwin[0],&gwin[1],&gwin[2]);
      }
    }
    break;

  case GL_TRIANGLE_STRIP:
    if (VB->ClipOrMask) {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;
      
      for(i=2;i<VB->Count;i++,gwin++) {
	RVB_COLOR(i);

	if(VB->ClipMask[i-2] | VB->ClipMask[i-1] | VB->ClipMask[i]) {
	  if(i&1) {
	    /* reverse vertex order */
	    vlist[0]=i-1;
	    vlist[1]=i-2;
	    vlist[2]=i-0;
	    render_clipped_polygon(ctx,3,vlist);
	  } else {
	    vlist[0]=i-2;
	    vlist[1]=i-1;
	    vlist[2]=i-0;
	    render_clipped_polygon(ctx,3,vlist);
	  }
	} else {
	  if(i&1)
	    grDrawTriangle(&gwin[2],&gwin[1],&gwin[0]);
	  else
	    grDrawTriangle(&gwin[0],&gwin[1],&gwin[2]);
	}
      }
    } else {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;
	
      for(i=2;i<VB->Count;i++,gwin++) {
	RVB_COLOR(i);

	if(i&1)
	  grDrawTriangle(&gwin[2],&gwin[1],&gwin[0]);
	else
	  grDrawTriangle(&gwin[0],&gwin[1],&gwin[2]);
      }
    }
    break;

  case GL_TRIANGLE_FAN:
    if (VB->ClipOrMask) {
      GLuint i;
      GrVertex *gwin,*gstart;

      gstart=fxMesa->gwin;
      gwin=&fxMesa->gwin[1];

      for(i=2;i<VB->Count;i++,gwin++) {
	RVB_COLOR(i);

	if (VB->ClipMask[0] | VB->ClipMask[i-1] | VB->ClipMask[i]) {
	  vlist[0]=0;
	  vlist[1]=i-1;
	  vlist[2]=i;
	  render_clipped_polygon(ctx,3,vlist);
	} else
	  grDrawTriangle(gstart,&gwin[0],&gwin[1]);
      }
    } else {
      /* no clipping needed */
      GLuint i;
      GrVertex *gwin,*gstart;

      gstart=fxMesa->gwin;
      gwin=&fxMesa->gwin[1];

      for(i=2;i<VB->Count;i++,gwin++) {
	RVB_COLOR(i);
	grDrawTriangle(gstart,&gwin[0],&gwin[1]);
      }
    }
    break;

  case GL_QUADS:
    if (VB->ClipOrMask) {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for (i=3;i<VB->Count;i+=4,gwin+=4) {
	RVB_COLOR(i);

	if(VB->ClipMask[i-3] | VB->ClipMask[i-2]
	   | VB->ClipMask[i-1] | VB->ClipMask[i]) {
	  vlist[0]=i-3;
	  vlist[1]=i-2;
	  vlist[2]=i-1;
	  vlist[3]=i-0;
	  render_clipped_polygon(ctx,4,vlist);
	} else {
	  grDrawTriangle(&gwin[0],&gwin[1],&gwin[3]);
	  grDrawTriangle(&gwin[1],&gwin[2],&gwin[3]);
	}
      }
    } else {
      /* no vertices were clipped */
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for(i=3;i<VB->Count;i+=4,gwin+=4) {
	RVB_COLOR(i);
	grDrawTriangle(&gwin[0],&gwin[1],&gwin[3]);
	grDrawTriangle(&gwin[1],&gwin[2],&gwin[3]);
      }
    }
    break;

  case GL_QUAD_STRIP:
    if (VB->ClipOrMask) {
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;

      for(i=3;i<VB->Count;i+=2,gwin+=2) {
	RVB_COLOR(i);

	if(VB->ClipMask[i-2] | VB->ClipMask[i-3]
	   | VB->ClipMask[i-1] | VB->ClipMask[i]) {
	  vlist[0]=i-1;
	  vlist[1]=i-3;
	  vlist[2]=i-2;
	  vlist[3]=i-0;
	  render_clipped_polygon(ctx,4,vlist);
	} else {
	  grDrawTriangle(&gwin[0],&gwin[1],&gwin[2]);
	  grDrawTriangle(&gwin[1],&gwin[3],&gwin[2]);
	}
      }
    } else {
      /* no clipping needed */
      GLuint i;
      GrVertex *gwin=fxMesa->gwin;
      
      for(i=3;i<VB->Count;i+=2,gwin+=2) {
	RVB_COLOR(i);
	grDrawTriangle(&gwin[0],&gwin[1],&gwin[2]);
	grDrawTriangle(&gwin[1],&gwin[3],&gwin[2]);
      }
    }
    break;

  case GL_POLYGON:
    if (VB->Count>2) {
      GLuint i;

      if (VB->ClipAndMask & CLIP_ALL_BITS) {
	/* all points clipped by common plane, draw nothing */
	break;
      }

      for(i=0;i<VB->Count;i++)
	vlist[i]=i;

      RVB_COLOR(0);

      if(VB->ClipOrMask)
	render_clipped_polygon(ctx,VB->Count,vlist);
      else
	render_polygon(ctx,VB->Count,vlist);
    }
    break;

  default:
    /* should never get here */
    gl_problem(ctx,"invalid mode in gl_render_vb");
  }

  gl_reset_vb(ctx,allDone);

  return GL_TRUE;
}
