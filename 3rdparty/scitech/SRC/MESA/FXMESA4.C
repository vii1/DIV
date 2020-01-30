/* -*- mode: C; tab-width:8;  -*-

             fxmesa4.c - 3Dfx VooDoo rendering mode setup functions
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

/************************************************************************/
/************************* Rendering Mode SetUp *************************/
/************************************************************************/

static void setup_texture(GLcontext *ctx)
{
  GLint ifmt;
  GrCombineLocal_t localc;

  /* Texture Combine, Color Combine and Alpha Combine */

  if(ctx->Light.ShadeModel==GL_SMOOTH)
    localc=GR_COMBINE_LOCAL_ITERATED;
  else
    localc=GR_COMBINE_LOCAL_CONSTANT;

  if(ctx->Texture.Enabled) {
    if((ctx->Texture.Current2D) && (ctx->Texture.Current2D->DriverData) &&
       (((texinfo *)ctx->Texture.Current2D->DriverData)->valid))
      ifmt=((texinfo *)ctx->Texture.Current2D->DriverData)->internalformat;
    else {
#if defined(DEBUG_FXMESA)
      if(ctx->Texture.Current) {
	fprintf(stderr,"fxmesa:   ifmt failed 1\n");
	if(ctx->Texture.Current2D->DriverData) {
	  fprintf(stderr,"fxmesa:   ifmt failed 2\n");
	  if(((texinfo *)ctx->Texture.Current2D->DriverData)->internalformat)
	    fprintf(stderr,"fxmesa:   ifmt failed 3\n");
	}
      }
#endif
      ifmt=-1; /* TO DO: check */
    }

#if defined(DEBUG_FXMESA)
    if(ifmt!=-1)
      fprintf(stderr,"fxmesa:  texobj=%d envmode=%x ifmt=%x\n",
	      ctx->Texture.Current2D->Name,ctx->Texture.EnvMode,ifmt);
    else
      fprintf(stderr,"fxmesa:  envmode=%x ifmt=%x\n",ctx->Texture.EnvMode,ifmt);
#endif

    grTexCombine(GR_TMU0,GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		 GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		 FXFALSE,FXFALSE);

    switch(ctx->Texture.EnvMode) {
    case GL_DECAL:
      grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     localc,
		     GR_COMBINE_OTHER_NONE,
		     FXFALSE);

      grColorCombine(GR_COMBINE_FUNCTION_BLEND,
		     GR_COMBINE_FACTOR_TEXTURE_ALPHA,
		     localc,
		     GR_COMBINE_OTHER_TEXTURE,
		     FXFALSE);
      break;
    case GL_MODULATE:
      grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		     GR_COMBINE_FACTOR_LOCAL,
		     localc,
		     GR_COMBINE_OTHER_TEXTURE,
		     FXFALSE);

      if(ifmt==GL_ALPHA)
	grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
		       GR_COMBINE_FACTOR_NONE,
		       localc,
		       GR_COMBINE_OTHER_NONE,
		       FXFALSE);
      else
	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_LOCAL,
		       localc,
		       GR_COMBINE_OTHER_TEXTURE,
		       FXFALSE);
      break;
    case GL_BLEND:
#ifndef FX_SILENT
      fprintf(stderr,"fxmesa: GL_BLEND not yet supported\n");
#endif
      /* TO DO (I think that the Voodoo Graphics isn't able to support GL_BLEND) */
      break;
    case GL_REPLACE:
      if((ifmt==GL_RGB) || (ifmt==GL_LUMINANCE))
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		       GR_COMBINE_FACTOR_NONE,
		       localc,
		       GR_COMBINE_OTHER_NONE,
		       FXFALSE);
      else
	grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_ONE,
		       localc,
		       GR_COMBINE_OTHER_TEXTURE,
		       FXFALSE);

      if(ifmt==GL_ALPHA)
	grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
		       GR_COMBINE_FACTOR_NONE,
		       localc,
		       GR_COMBINE_OTHER_NONE,
		       FXFALSE);
      else
	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_ONE,
		       localc,
		       GR_COMBINE_OTHER_TEXTURE,
		       FXFALSE);
      break;
    default:
#ifndef FX_SILENT
      fprintf(stderr,"fxmesa: %x Texture.EnvMode not yet supported\n",ctx->Texture.EnvMode);
#endif
      break;
    }
  } else {
    grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		   GR_COMBINE_FACTOR_NONE,
		   localc,
		   GR_COMBINE_OTHER_NONE,
		   FXFALSE);

    grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
		   GR_COMBINE_FACTOR_NONE,
		   localc,
		   GR_COMBINE_OTHER_NONE,
		   FXFALSE);
  }
}

static void setup_blend(GLcontext *ctx)
{
  if(ctx->Color.BlendEnabled) {
    GrAlphaBlendFnc_t sfact,dfact;

    switch(ctx->Color.BlendSrc) {
    case GL_ZERO:
      sfact=GR_BLEND_ZERO;
      break;
    case GL_ONE:
      sfact=GR_BLEND_ONE;
      break;
    case GL_DST_COLOR:
      sfact=GR_BLEND_DST_COLOR;
      break;
    case GL_ONE_MINUS_DST_COLOR:
      sfact=GR_BLEND_ONE_MINUS_DST_COLOR;
      break;
    case GL_SRC_ALPHA:
      sfact=GR_BLEND_SRC_ALPHA;
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      sfact=GR_BLEND_ONE_MINUS_SRC_ALPHA;
      break;
    case GL_SRC_ALPHA_SATURATE:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
      /* USELESS or TO DO */
      sfact=GR_BLEND_ONE;
      break;
    default:
      sfact=GR_BLEND_ONE;
      break;
    }

    switch(ctx->Color.BlendDst) {
    case GL_ZERO:
      dfact=GR_BLEND_ZERO;
      break;
    case GL_ONE:
      dfact=GR_BLEND_ONE;
      break;
    case GL_SRC_COLOR:
      dfact=GR_BLEND_SRC_COLOR;
      break;
    case GL_ONE_MINUS_SRC_COLOR:
      dfact=GR_BLEND_ONE_MINUS_SRC_COLOR;
      break;
    case GL_SRC_ALPHA:
      dfact=GR_BLEND_SRC_ALPHA;
      break;
    case GL_ONE_MINUS_SRC_ALPHA:
      dfact=GR_BLEND_ONE_MINUS_SRC_ALPHA;
      break;
    case GL_SRC_ALPHA_SATURATE:
    case GL_DST_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
      /* USELESS or TO DO */
      dfact=GR_BLEND_ZERO;
      break;
    default:
      dfact=GR_BLEND_ZERO;
      break;
    }

    grAlphaBlendFunction(sfact,dfact,GR_BLEND_ONE,GR_BLEND_ZERO);
  } else
    grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ZERO,GR_BLEND_ONE,GR_BLEND_ZERO);

}
  
static void setup_alphatest(GLcontext *ctx)
{
  if(ctx->Color.AlphaEnabled) {
    switch(ctx->Color.AlphaFunc) {
    case GL_NEVER:
      grAlphaTestFunction(GR_CMP_NEVER);
      break;
    case GL_LESS:
      grAlphaTestFunction(GR_CMP_LESS);
      break;
    case GL_EQUAL:
      grAlphaTestFunction(GR_CMP_EQUAL);
      break;
    case GL_LEQUAL:
      grAlphaTestFunction(GR_CMP_LEQUAL);
      break;
    case GL_GREATER:
      grAlphaTestFunction(GR_CMP_GREATER);
      break;
    case GL_NOTEQUAL:
      grAlphaTestFunction(GR_CMP_NOTEQUAL);
      break;
    case GL_GEQUAL:
      grAlphaTestFunction(GR_CMP_GEQUAL);
      break;
    case GL_ALWAYS:
      grAlphaTestFunction(GR_CMP_ALWAYS);
      break;
    default:
      break;
    }
    grAlphaTestReferenceValue(ctx->Color.AlphaRefUbyte);
  } else
    grAlphaTestFunction(GR_CMP_ALWAYS);
}
  
static void setup_depthtest(GLcontext *ctx)
{
  if(ctx->Depth.Test) {
    switch(ctx->Depth.Func) {
    case GL_NEVER:
      grDepthBufferFunction(GR_CMP_NEVER);
      break;
    case GL_LESS:
      grDepthBufferFunction(GR_CMP_LESS);
      break;
    case GL_GEQUAL:
      grDepthBufferFunction(GR_CMP_GEQUAL);
      break;
    case GL_LEQUAL:
      grDepthBufferFunction(GR_CMP_LEQUAL);
      break;
    case GL_GREATER:
      grDepthBufferFunction(GR_CMP_GREATER);
      break;
    case GL_NOTEQUAL:
      grDepthBufferFunction(GR_CMP_NOTEQUAL);
      break;
    case GL_EQUAL:
      grDepthBufferFunction(GR_CMP_EQUAL);
      break;
    case GL_ALWAYS:
      grDepthBufferFunction(GR_CMP_ALWAYS);
      break;
    default:
      break;
    }

    if(ctx->Depth.Mask)
      grDepthMask(FXTRUE);
    else
      grDepthMask(FXFALSE);
  } else {
    grDepthBufferFunction(GR_CMP_ALWAYS);
    grDepthMask(FXFALSE);
  }
}

static void setup_fog(GLcontext *ctx)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

  if(ctx->Fog.Enabled && ctx->Hint.Fog==GL_NICEST) {
    grFogMode(GR_FOG_WITH_TABLE);
    grFogColorValue FXCOLOR((unsigned int)(255*ctx->Fog.Color[0]), 
                            (unsigned int)(255*ctx->Fog.Color[1]),
                            (unsigned int)(255*ctx->Fog.Color[2]), 
                            (unsigned int)(255*ctx->Fog.Color[3]));

    if((fxMesa->fogtablemode!=ctx->Fog.Mode) ||
       (fxMesa->fogdensity!=ctx->Fog.Density)) {
      float wscale = ((fxMesaContext)ctx->DriverCtx)->wscale;
      switch(ctx->Fog.Mode) {
      case GL_LINEAR:
        guFogGenerateLinear(fxMesa->fogtable,
                            ctx->Fog.Start/wscale,
                            ctx->Fog.End/wscale);
        break;
      case GL_EXP:
        guFogGenerateExp(fxMesa->fogtable,ctx->Fog.Density*wscale);
        break;
      case GL_EXP2:
        guFogGenerateExp2(fxMesa->fogtable,ctx->Fog.Density*wscale);
        break;
      default: /* That should never happen */
        break; 
      }
         
      fxMesa->fogtablemode=ctx->Fog.Mode;
      fxMesa->fogdensity=ctx->Fog.Density;
    }
      
    grFogTable(fxMesa->fogtable);
  } else
    grFogMode(GR_FOG_DISABLE);
}

static void setup_scissor(GLcontext *ctx)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

  if (ctx->Scissor.Enabled) {
    int ymin, ymax;

    ymin=ctx->Scissor.Y;
    ymax=ctx->Scissor.Y+ctx->Scissor.Height;

    if (ymin<0) ymin=0;

    if (ymax>fxMesa->height) ymax=fxMesa->height;

    grClipWindow(ctx->Scissor.X, 
 		 ymin,
 		 ctx->Scissor.X+ctx->Scissor.Width, 
 		 ymax);
  } else
    grClipWindow(0,0,fxMesa->width,fxMesa->height);
}

void setup_fx_units(GLcontext *ctx)
{
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: setup_fx_units()\n");
#endif

  setup_texture(ctx);
  setup_blend(ctx);
  setup_alphatest(ctx);
  setup_depthtest(ctx);
  setup_fog(ctx);
  setup_scissor(ctx);

  if(ctx->Color.ColorMask)
    grColorMask(FXTRUE,FXFALSE);
  else
    grColorMask(FXFALSE,FXFALSE);
}


#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function4(void)
{
  return 0;
}

#endif  /* FX */
