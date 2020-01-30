/* -*- mode: C; tab-width:8;  -*-

             fxmesa5.c - 3Dfx VooDoo Texture mapping functions
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
 * Feb 5, 1998  brianp
 *      fixed errors in setting the texture min filter parameters
 *
 */

#if defined(FX)

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "fxdrv.h"

/************************************************************************/
/*************************** Texture Mapping ****************************/
/************************************************************************/

static texinfo *alloctexobjdata(void)
{
  texinfo *ti;

  if(!(ti=malloc(sizeof(texinfo)))) {
    fprintf(stderr,"fx Driver: out of memory !\n");
    fxCloseHardware();
    exit(-1);
  }

  ti->valid=GL_FALSE;
  ti->tmi.isintexturememory=GL_FALSE;

  return ti;
}

void texbind(GLcontext *ctx, GLenum target, struct gl_texture_object *tObj)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  texinfo *ti;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: texbind(%d,%x)\n",tObj->Name,tObj->DriverData);
#endif

  if(target!=GL_TEXTURE_2D) {
#ifndef FX_SILENT
    fprintf(stderr,"fx Driver: unsupported texture in texbind()\n");
#endif
    return;
  }

  if(!tObj->DriverData)
    tObj->DriverData=alloctexobjdata();

  ti=(texinfo *)tObj->DriverData;

  if(!ti->valid)
    return;

  fxMesa->texbindnumber++;
  ti->tmi.lasttimeused=fxMesa->texbindnumber;

  moveintexturememory(fxMesa,tObj);

  if((ti->info.format==GR_TEXFMT_P_8) && (!fxMesa->globalpalette_texture)) {
#if defined(DEBUG_FXMESA)
    fprintf(stderr,"fxmesa: uploading texture palette\n");
#endif
    grTexDownloadTable(GR_TMU0,GR_TEXTABLE_PALETTE,&(ti->palette));
  }

  grTexClampMode(GR_TMU0,ti->sclamp,ti->tclamp);
  grTexFilterMode(GR_TMU0,ti->minfilt,ti->maxfilt);
  grTexMipMapMode(GR_TMU0,ti->mmmode,FXFALSE);
  grTexSource(GR_TMU0,ti->tmi.tm->startadr,GR_MIPMAPLEVELMASK_BOTH,&(ti->info));
}

void texenv(GLcontext *ctx, GLenum pname, const GLfloat *param)
{
#if defined(DEBUG_FXMESA)
  if(param)
    fprintf(stderr,"fxmesa: texenv(%x,%x)\n",pname,(GLint)(*param));
  else
    fprintf(stderr,"fxmesa: texenv(%x)\n",pname);
#endif

  ctx->NewState |= NEW_RASTER_OPS;
}

void texparam(GLcontext *ctx, GLenum target, struct gl_texture_object *tObj,
	      GLenum pname, const GLfloat *params)
{
  GLenum param=(GLenum)(GLint)params[0];
  texinfo *ti;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: texparam(%d,%x)\n",tObj->Name,tObj->DriverData);
#endif

  if(target!=GL_TEXTURE_2D) {
#ifndef FX_SILENT
    fprintf(stderr,"fx Driver: unsupported texture in texparam()\n");
#endif
    return;
  }

  if(!tObj->DriverData)
    tObj->DriverData=alloctexobjdata();

  ti=(texinfo *)tObj->DriverData;

  switch(pname) {

  case GL_TEXTURE_MIN_FILTER:
    switch(param) {
    case GL_NEAREST:
      ti->mmmode=GR_MIPMAP_DISABLE;
      ti->minfilt=GR_TEXTUREFILTER_POINT_SAMPLED;
      break;
    case GL_LINEAR:
      ti->mmmode=GR_MIPMAP_DISABLE;
      ti->minfilt=GR_TEXTUREFILTER_BILINEAR;
      break;
    case GL_NEAREST_MIPMAP_NEAREST:
      ti->mmmode=GR_MIPMAP_NEAREST;
      ti->minfilt=GR_TEXTUREFILTER_POINT_SAMPLED;
      break;
    case GL_LINEAR_MIPMAP_NEAREST:
      ti->mmmode=GR_MIPMAP_NEAREST;
      ti->minfilt=GR_TEXTUREFILTER_BILINEAR;
      break;
    case GL_NEAREST_MIPMAP_LINEAR:
      ti->mmmode=GR_MIPMAP_NEAREST_DITHER;
      ti->minfilt=GR_TEXTUREFILTER_POINT_SAMPLED;
      break;
    case GL_LINEAR_MIPMAP_LINEAR:
      ti->mmmode=GR_MIPMAP_NEAREST_DITHER;
      ti->minfilt=GR_TEXTUREFILTER_BILINEAR;
      break;
    default:
      break;
    }
    break;

  case GL_TEXTURE_MAG_FILTER:
    switch(param) {
    case GL_NEAREST:
      ti->maxfilt=GR_TEXTUREFILTER_POINT_SAMPLED;
      break;
    case GL_LINEAR:
      ti->maxfilt=GR_TEXTUREFILTER_BILINEAR;
      break;
    default:
      break;
    }
    break;

  case GL_TEXTURE_WRAP_S:
    switch(param) {
    case GL_CLAMP:
      ti->sclamp=GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      ti->sclamp=GR_TEXTURECLAMP_WRAP;
      break;
    default:
      break;
    }
    break;

  case GL_TEXTURE_WRAP_T:
    switch(param) {
    case GL_CLAMP:
      ti->tclamp=GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      ti->tclamp=GR_TEXTURECLAMP_WRAP;
      break;
    default:
      break;
    }
    break;

  case GL_TEXTURE_BORDER_COLOR:
    /* TO DO */
    break;
  default:
    break;
  }

  texbind(ctx,target,tObj);
}

void texdel(GLcontext *ctx, struct gl_texture_object *tObj)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  texinfo *ti=(texinfo *)tObj->DriverData;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: texdel(%d,%x)\n",tObj->Name,ti);
#endif

  if(!ti)
    return;

  freetexture(fxMesa,tObj);

  free(ti);
  tObj->DriverData=NULL;
}


void texpalette(GLcontext *ctx, struct gl_texture_object *tObj)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  int i;
  FxU32 r,g,b,a;
  texinfo *ti;

  if(tObj) {  
#if defined(DEBUG_FXMESA)
    fprintf(stderr,"fxmesa: texpalette(%d,%x)\n",tObj->Name,tObj->DriverData);
#endif

    if(tObj->PaletteFormat!=GL_RGBA) {
#ifndef FX_SILENT
      fprintf(stderr,"fxmesa: unsupported palette format in texpalette()\n");
#endif
      return;
    }

    if(tObj->PaletteSize>256) {
#ifndef FX_SILENT
      fprintf(stderr,"fxmesa: unsupported palette size in texpalette()\n");
#endif
      return;
    }

    if(!tObj->DriverData)
      tObj->DriverData=alloctexobjdata();
  
    ti=(texinfo *)tObj->DriverData;

    for(i=0;i<tObj->PaletteSize;i++) {
      r=tObj->Palette[i*4];
      g=tObj->Palette[i*4+1];
      b=tObj->Palette[i*4+2];
      a=tObj->Palette[i*4+3];
      ti->palette.data[i]=(a<<24)|(r<<16)|(g<<8)|b;
    }
  
    texbind(ctx,GL_TEXTURE_2D,tObj);
  } else {
#if defined(DEBUG_FXMESA)
    fprintf(stderr,"fxmesa: texpalette(global)\n");
#endif
    if(ctx->Texture.PaletteFormat!=GL_RGBA) {
#ifndef FX_SILENT
      fprintf(stderr,"fxmesa: unsupported palette format in texpalette()\n");
#endif
      return;
    }

    if(ctx->Texture.PaletteSize>256) {
#ifndef FX_SILENT
      fprintf(stderr,"fxmesa: unsupported palette size in texpalette()\n");
#endif
      return;
    }

    for(i=0;i<ctx->Texture.PaletteSize;i++) {
      r=ctx->Texture.Palette[i*4];
      g=ctx->Texture.Palette[i*4+1];
      b=ctx->Texture.Palette[i*4+2];
      a=ctx->Texture.Palette[i*4+3];
      fxMesa->glbpalette.data[i]=(a<<24)|(r<<16)|(g<<8)|b;
    }

    grTexDownloadTable(GR_TMU0,GR_TEXTABLE_PALETTE,&(fxMesa->glbpalette));
  }
}

void useglbtexpalette(GLcontext *ctx, GLboolean state)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: useglbtexpalette(%d)\n",state);
#endif

  if(state) {
    fxMesa->globalpalette_texture=1;

    grTexDownloadTable(GR_TMU0,GR_TEXTABLE_PALETTE,&(fxMesa->glbpalette));
  } else {
    fxMesa->globalpalette_texture=0;

    if((ctx->Texture.Current==ctx->Texture.Current2D) &&
       (ctx->Texture.Current2D!=NULL))
      texbind(ctx,GL_TEXTURE_2D,ctx->Texture.Current);
  }
}

static int logbase2(int n)
{
  GLint i = 1;
  GLint log2 = 0;

  if (n<0) {
    return -1;
  }

  while (n > i) {
    i *= 2;
    log2++;
  }
  if (i != n) {
    return -1;
  }
  else {
    return log2;
  }
}

int texgetinfo(int w, int h, GrLOD_t *lodlevel, GrAspectRatio_t *ar,
	       float *sscale, float *tscale,
	       int *wscale, int *hscale)
{
  static GrLOD_t lod[9]={GR_LOD_256,GR_LOD_128,GR_LOD_64,GR_LOD_32,
                         GR_LOD_16,GR_LOD_8,GR_LOD_4,GR_LOD_2,GR_LOD_1};
  int logw,logh,ws,hs;
  GrLOD_t l;
  GrAspectRatio_t aspectratio;
  float s,t;

  logw=logbase2(w);
  logh=logbase2(h);

  switch(logw-logh) {
  case 0:
    aspectratio=GR_ASPECT_1x1;
    l=lod[8-logw];
    s=t=256.0f;
    ws=hs=1;
    break;
  case 1:
    aspectratio=GR_ASPECT_2x1;
    l=lod[8-logw];
    s=256.0f;
    t=128.0f;
    ws=1;
    hs=1;
    break;
  case 2:
    aspectratio=GR_ASPECT_4x1;
    l=lod[8-logw];
    s=256.0f;
    t=64.0f;
    ws=1;
    hs=1;
    break;
  case 3:
    aspectratio=GR_ASPECT_8x1;
    l=lod[8-logw];
    s=256.0f;
    t=32.0f;
    ws=1;
    hs=1;
    break;
  case 4:
    aspectratio=GR_ASPECT_8x1;
    l=lod[8-logw];
    s=256.0f;
    t=32.0f;
    ws=1;
    hs=2;
    break;
  case 5:
    aspectratio=GR_ASPECT_8x1;
    l=lod[8-logw];
    s=256.0f;
    t=32.0f;
    ws=1;
    hs=4;
    break;
  case 6:
    aspectratio=GR_ASPECT_8x1;
    l=lod[8-logw];
    s=256.0f;
    t=32.0f;
    ws=1;
    hs=8;
    break;
  case 7:
    aspectratio=GR_ASPECT_8x1;
    l=lod[8-logw];
    s=256.0f;
    t=32.0f;
    ws=1;
    hs=16;
    break;
  case 8:
    aspectratio=GR_ASPECT_8x1;
    l=lod[8-logw];
    s=256.0f;
    t=32.0f;
    ws=1;
    hs=32;
    break;
  case -1:
    aspectratio=GR_ASPECT_1x2;
    l=lod[8-logh];
    s=128.0f;
    t=256.0f;
    ws=1;
    hs=1;
    break;
  case -2:
    aspectratio=GR_ASPECT_1x4;
    l=lod[8-logh];
    s=64.0f;
    t=256.0f;
    ws=1;
    hs=1;
    break;
  case -3:
    aspectratio=GR_ASPECT_1x8;
    l=lod[8-logh];
    s=32.0f;
    t=256.0f;
    ws=1;
    hs=1;
    break;
  case -4:
    aspectratio=GR_ASPECT_1x8;
    l=lod[8-logh];
    s=32.0f;
    t=256.0f;
    ws=2;
    hs=1;
    break;
  case -5:
    aspectratio=GR_ASPECT_1x8;
    l=lod[8-logh];
    s=32.0f;
    t=256.0f;
    ws=4;
    hs=1;
    break;
  case -6:
    aspectratio=GR_ASPECT_1x8;
    l=lod[8-logh];
    s=32.0f;
    t=256.0f;
    ws=8;
    hs=1;
    break;
  case -7:
    aspectratio=GR_ASPECT_1x8;
    l=lod[8-logh];
    s=32.0f;
    t=256.0f;
    ws=16;
    hs=1;
    break;
  case -8:
    aspectratio=GR_ASPECT_1x8;
    l=lod[8-logh];
    s=32.0f;
    t=256.0f;
    ws=32;
    hs=1;
    break;
  default:
    return 0;
    break;
  }

  if(lodlevel)
    (*lodlevel)=l;

  if(ar)
    (*ar)=aspectratio;

  if(sscale)
    (*sscale)=s;

  if(tscale)
    (*tscale)=t;

  if(wscale)
    (*wscale)=ws;

  if(hscale)
    (*hscale)=hs;

  return 1;
}

static void texgetformat(GLcontext *ctx, GLenum glformat,
			 GrTextureFormat_t *tfmt, GLint *ifmt)
{
  switch(glformat) {
  case 1:
  case GL_LUMINANCE:
  case GL_LUMINANCE4:
  case GL_LUMINANCE8:
  case GL_LUMINANCE12:
  case GL_LUMINANCE16:
    if(tfmt)
      (*tfmt)=GR_TEXFMT_INTENSITY_8;
    if(ifmt)
      (*ifmt)=GL_LUMINANCE;
    break;
  case 2:
  case GL_LUMINANCE_ALPHA:
  case GL_LUMINANCE4_ALPHA4:
  case GL_LUMINANCE6_ALPHA2:
  case GL_LUMINANCE8_ALPHA8:
  case GL_LUMINANCE12_ALPHA4:
  case GL_LUMINANCE12_ALPHA12:
  case GL_LUMINANCE16_ALPHA16:
    if(tfmt)
      (*tfmt)=GR_TEXFMT_ALPHA_INTENSITY_88;
    if(ifmt)
      (*ifmt)=GL_LUMINANCE_ALPHA;
    break;
  case GL_INTENSITY:
  case GL_INTENSITY4:
  case GL_INTENSITY8:
  case GL_INTENSITY12:
  case GL_INTENSITY16:
    if(tfmt)
      (*tfmt)=GR_TEXFMT_ALPHA_8;
    if(ifmt)
      (*ifmt)=GL_INTENSITY;
    break;
  case GL_ALPHA:
  case GL_ALPHA4:
  case GL_ALPHA8:
  case GL_ALPHA12:
  case GL_ALPHA16:
    if(tfmt)
      (*tfmt)=GR_TEXFMT_ALPHA_8;
    if(ifmt)
      (*ifmt)=GL_ALPHA;
    break;
  case 3:
  case GL_RGB:
  case GL_R3_G3_B2:
  case GL_RGB4:
  case GL_RGB5:
  case GL_RGB8:
  case GL_RGB10:
  case GL_RGB12:
  case GL_RGB16:
    if(tfmt)
      (*tfmt)=GR_TEXFMT_RGB_565;
    if(ifmt)
      (*ifmt)=GL_RGB;
    break;
  case 4:
  case GL_RGBA:
  case GL_RGBA2:
  case GL_RGBA4:
  case GL_RGB5_A1:
  case GL_RGBA8:
  case GL_RGB10_A2:
  case GL_RGBA12:
  case GL_RGBA16:
    if(tfmt)
      (*tfmt)=GR_TEXFMT_ARGB_4444;
    if(ifmt)
      (*ifmt)=GL_RGBA;
    break;
  case GL_COLOR_INDEX:
  case GL_COLOR_INDEX1_EXT:
  case GL_COLOR_INDEX2_EXT:
  case GL_COLOR_INDEX4_EXT:
  case GL_COLOR_INDEX8_EXT:
  case GL_COLOR_INDEX12_EXT:
  case GL_COLOR_INDEX16_EXT:
    if(tfmt)
      (*tfmt)=GR_TEXFMT_P_8;
    if(ifmt)
      (*ifmt)=GL_RGB;
    break;
  default:
    fprintf(stderr,"fx Driver: unsupported internalFormat in texgetformat()\n");
    fxCloseHardware();
    exit(-1);
    break;
  }
}

static void texalloc(GLcontext *ctx, struct gl_texture_object *tObj,
		     GLenum glformat, int w, int h)
{
  GrTextureFormat_t format;
  GrLOD_t l;
  GrAspectRatio_t aspectratio;
  texinfo *ti=(texinfo *)tObj->DriverData;
  int wscale,hscale;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: texalloc()\n");
#endif

  assert(ti);

  texgetinfo(w,h,&l,&aspectratio,&(ti->sscale),&(ti->tscale),&wscale,&hscale);

  texgetformat(ctx,glformat,&format,&(ti->internalformat));

  ti->width=w;
  ti->height=h;
  ti->info.smallLod=l;
  ti->info.largeLod=l;
  ti->info.aspectRatio=aspectratio;
  ti->info.format=format;
  ti->info.data=NULL;

  ti->minfilt=GR_TEXTUREFILTER_BILINEAR;
  ti->maxfilt=GR_TEXTUREFILTER_BILINEAR;

  ti->sclamp=GR_TEXTURECLAMP_WRAP;
  ti->tclamp=GR_TEXTURECLAMP_WRAP;

  ti->mmmode=GR_MIPMAP_NEAREST;

  ti->levelsdefined=0;
}

static int istexsupported(GLenum target, GLint internalFormat,
                          const struct gl_texture_image *image)
{
  if(target!=GL_TEXTURE_2D)
    return GL_FALSE;

  switch(internalFormat) {
  case GL_INTENSITY:
  case GL_INTENSITY4:
  case GL_INTENSITY8:
  case GL_INTENSITY12:
  case GL_INTENSITY16:
  case 1:
  case GL_LUMINANCE:
  case GL_LUMINANCE4:
  case GL_LUMINANCE8:
  case GL_LUMINANCE12:
  case GL_LUMINANCE16:
  case 2:
  case GL_LUMINANCE_ALPHA:
  case GL_LUMINANCE4_ALPHA4:
  case GL_LUMINANCE6_ALPHA2:
  case GL_LUMINANCE8_ALPHA8:
  case GL_LUMINANCE12_ALPHA4:
  case GL_LUMINANCE12_ALPHA12:
  case GL_LUMINANCE16_ALPHA16:
  case GL_ALPHA:
  case GL_ALPHA4:
  case GL_ALPHA8:
  case GL_ALPHA12:
  case GL_ALPHA16:
  case 3:
  case GL_RGB:
  case GL_R3_G3_B2:
  case GL_RGB4:
  case GL_RGB5:
  case GL_RGB8:
  case GL_RGB10:
  case GL_RGB12:
  case GL_RGB16:
  case 4:
  case GL_RGBA:
  case GL_RGBA2:
  case GL_RGBA4:
  case GL_RGB5_A1:
  case GL_RGBA8:
  case GL_RGB10_A2:
  case GL_RGBA12:
  case GL_RGBA16:
  case GL_COLOR_INDEX:
  case GL_COLOR_INDEX1_EXT:
  case GL_COLOR_INDEX2_EXT:
  case GL_COLOR_INDEX4_EXT:
  case GL_COLOR_INDEX8_EXT:
  case GL_COLOR_INDEX12_EXT:
  case GL_COLOR_INDEX16_EXT:
    break;
  default:
    return GL_FALSE;
  }

  if(image->Width>256)
    return GL_FALSE;

  if(image->Height>256)
    return GL_FALSE;

  if(!texgetinfo(image->Width,image->Height,NULL,NULL,NULL,NULL,NULL,NULL))
    return GL_FALSE;

  return GL_TRUE;
}

static void texbuildimagemap(const struct gl_texture_image *image,
			     GLint internalFormat, unsigned short **dest,
			     GLboolean *istranslate)
{
  unsigned short *src;
  unsigned char *data;
  int x,y,w,h,wscale,hscale,idx;

  texgetinfo(image->Width,image->Height,NULL,NULL,NULL,NULL,&wscale,&hscale);
  w=image->Width*wscale;
  h=image->Height*hscale;

  data=image->Data;
  switch(internalFormat) {
  case GL_INTENSITY:
  case GL_INTENSITY4:
  case GL_INTENSITY8:
  case GL_INTENSITY12:
  case GL_INTENSITY16:
  case 1:
  case GL_LUMINANCE:
  case GL_LUMINANCE4:
  case GL_LUMINANCE8:
  case GL_LUMINANCE12:
  case GL_LUMINANCE16:
  case GL_ALPHA:
  case GL_ALPHA4:
  case GL_ALPHA8:
  case GL_ALPHA12:
  case GL_ALPHA16:
  case GL_COLOR_INDEX:
  case GL_COLOR_INDEX1_EXT:
  case GL_COLOR_INDEX2_EXT:
  case GL_COLOR_INDEX4_EXT:
  case GL_COLOR_INDEX8_EXT:
  case GL_COLOR_INDEX12_EXT:
  case GL_COLOR_INDEX16_EXT:
    /* Optimized for GLQuake */

    if(wscale==hscale==1) {
      (*istranslate)=GL_FALSE;

      (*dest)=(unsigned short *)data;
    } else {
      unsigned char *srcb;

      (*istranslate)=GL_TRUE;

      if(!(*dest)) {
	if(!((*dest)=src=(unsigned short *)malloc(sizeof(unsigned char)*w*h))) {
	  fprintf(stderr,"fx Driver: out of memory !\n");
	  fxCloseHardware();
	  exit(-1);
	}
      } else
	src=(*dest);

      srcb=(unsigned char *)src;

      for(y=0;y<h;y++)
        for(x=0;x<w;x++) {
          idx=(x/wscale+(y/hscale)*(w/wscale));
          srcb[x+y*w]=data[idx];
        }
    }
    break;
  case 2:
  case GL_LUMINANCE_ALPHA:
  case GL_LUMINANCE4_ALPHA4:
  case GL_LUMINANCE6_ALPHA2:
  case GL_LUMINANCE8_ALPHA8:
  case GL_LUMINANCE12_ALPHA4:
  case GL_LUMINANCE12_ALPHA12:
  case GL_LUMINANCE16_ALPHA16:
    (*istranslate)=GL_TRUE;

    if(!(*dest)) {
      if(!((*dest)=src=(unsigned short *)malloc(sizeof(unsigned short)*w*h))) {
        fprintf(stderr,"fx Driver: out of memory !\n");
        fxCloseHardware();
        exit(-1);
      }
    } else
      src=(*dest);

    if(wscale==hscale==1) {
      int i=0;
      int lenght=h*w;
      unsigned short a,l;

      while(i++<lenght) {
	l=*data++;
	a=*data++;

	*src++=(a << 8) | l;
      }
    } else {
      unsigned short a,l;

      for(y=0;y<h;y++)
	for(x=0;x<w;x++) {
	  idx=(x/wscale+(y/hscale)*(w/wscale))*2;
	  l=data[idx];
	  a=data[idx+1];

	  src[x+y*w]=(a << 8) | l;
	}
    }
    break;
  case 3:
  case GL_RGB:
  case GL_R3_G3_B2:
  case GL_RGB4:
  case GL_RGB5:
  case GL_RGB8:
  case GL_RGB10:
  case GL_RGB12:
  case GL_RGB16:
    (*istranslate)=GL_TRUE;

    if(!(*dest)) {
      if(!((*dest)=src=(unsigned short *)malloc(sizeof(unsigned short)*w*h))) {
        fprintf(stderr,"fx Driver: out of memory !\n");
        fxCloseHardware();
        exit(-1);
      }
    } else
      src=(*dest);

    if(wscale==hscale==1) {
      int i=0;
      int lenght=h*w;
      unsigned short r,g,b;

      while(i++<lenght) {
	r=*data++;
	g=*data++;
	b=*data++;

	*src++=((0xf8 & r) << (11-3))  |
	  ((0xfc & g) << (5-3+1))      |
	  ((0xf8 & b) >> 3); 
      }
    } else {
      unsigned short r,g,b;

      for(y=0;y<h;y++)
	for(x=0;x<w;x++) {
	  idx=(x/wscale+(y/hscale)*(w/wscale))*3;
	  r=data[idx];
	  g=data[idx+1];
	  b=data[idx+2];

	  src[x+y*w]=((0xf8 & r) << (11-3))  |
	  ((0xfc & g) << (5-3+1))      |
	  ((0xf8 & b) >> 3); 
	}
    }
    break;
  case 4:
  case GL_RGBA:
  case GL_RGBA2:
  case GL_RGBA4:
  case GL_RGB5_A1:
  case GL_RGBA8:
  case GL_RGB10_A2:
  case GL_RGBA12:
  case GL_RGBA16:
    (*istranslate)=GL_TRUE;

    if(!(*dest)) {
      if(!((*dest)=src=(unsigned short *)malloc(sizeof(unsigned short)*w*h))) {
        fprintf(stderr,"fx Driver: out of memory !\n");
        fxCloseHardware();
        exit(-1);
      }
    } else
      src=(*dest);

    if(wscale==hscale==1) {
      int i=0;
      int lenght=h*w;
      unsigned short r,g,b,a;

      while(i++<lenght) {
	r=*data++;
	g=*data++;
	b=*data++;
	a=*data++;

	*src++=((0xf0 & a) << 8) |
          ((0xf0 & r) << 4)      |
          (0xf0 & g)             |
          ((0xf0 & b) >> 4);
      }
    } else {
      unsigned short r,g,b,a;

      for(y=0;y<h;y++)
	for(x=0;x<w;x++) {
	  idx=(x/wscale+(y/hscale)*(w/wscale))*4;
	  r=data[idx];
	  g=data[idx+1];
	  b=data[idx+2];
	  a=data[idx+3];

        src[x+y*w]=((0xf0 & a) << 8) |
          ((0xf0 & r) << 4)      |
          (0xf0 & g)             |
          ((0xf0 & b) >> 4);
	}
    }
    break;
  default:
    fprintf(stderr,"fx Driver: wrong internalFormat in texbuildimagemap()\n");
    fxCloseHardware();
    exit(-1);
    break;
  }
}

void teximg(GLcontext *ctx, GLenum target,
	    struct gl_texture_object *tObj, GLint level, GLint internalFormat,
	    const struct gl_texture_image *image)
{
  GrLOD_t lodlev;
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  texinfo *ti;
  GrTextureFormat_t format;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: (%d) teximg(...,%d,%x,%d,%d...)\n",tObj->Name,
	  target,internalFormat,image->Width,image->Height);
#endif

  if(target!=GL_TEXTURE_2D) {
#ifndef FX_SILENT
    fprintf(stderr,"fx Driver: unsupported texture in teximg()\n");
#endif
    return;
  }

  if(!tObj->DriverData)
    tObj->DriverData=alloctexobjdata();

  ti=(texinfo *)tObj->DriverData;

  if(istexsupported(target,internalFormat,image)) {
    /* The following line isn't really right ... */

    texgetformat(ctx,internalFormat,&format,NULL);

    if((!ti->valid) || (ti->info.format!=format))
      texalloc(ctx,tObj,image->Format,image->Width,image->Height);

    if(ti->levelsdefined & (1<<level)) {
      texbuildimagemap(image,internalFormat,&(ti->tmi.mipmaplevel[level]),
		       &(ti->tmi.translate_mipmaplevel[level]));

      reloadmipmaplevel(fxMesa,tObj,level);
    } else {
      moveouttexturememory(fxMesa,tObj); /* TO DO: SLOW but easy to write */

      ti->tmi.mipmaplevel[level]=NULL;

      texbuildimagemap(image,internalFormat,&(ti->tmi.mipmaplevel[level]),
		       &(ti->tmi.translate_mipmaplevel[level]));

      texgetinfo(image->Width,image->Height,&lodlev,NULL,NULL,NULL,NULL,NULL);

      if(lodlev<ti->info.largeLod) /* A new larger mip map level */
        ti->info.largeLod=lodlev;
      else if(lodlev>ti->info.smallLod) /* A new smaller mip map level */
        ti->info.smallLod=lodlev;

      ti->levelsdefined|=(1<<level);
      ti->valid=GL_TRUE;

      texbind(ctx,target,tObj);
    }
  }
#ifndef FX_SILENT
  else
    fprintf(stderr,"fx Driver: unsupported texture in teximg()\n");
#endif
}

static void texbuildsubimagemap(const struct gl_texture_image *image,
				GLint internalFormat,
				GLint xoffset, GLint yoffset, GLint width, GLint height,
				unsigned short *destimg)
{
  texgetinfo(image->Width,image->Height,NULL,NULL,NULL,NULL,NULL,NULL);

  switch(internalFormat) {
  case GL_INTENSITY:
  case GL_INTENSITY4:
  case GL_INTENSITY8:
  case GL_INTENSITY12:
  case GL_INTENSITY16:
  case 1:
  case GL_LUMINANCE:
  case GL_LUMINANCE4:
  case GL_LUMINANCE8:
  case GL_LUMINANCE12:
  case GL_LUMINANCE16:
  case GL_ALPHA:
  case GL_ALPHA4:
  case GL_ALPHA8:
  case GL_ALPHA12:
  case GL_ALPHA16:
  case GL_COLOR_INDEX:
  case GL_COLOR_INDEX1_EXT:
  case GL_COLOR_INDEX2_EXT:
  case GL_COLOR_INDEX4_EXT:
  case GL_COLOR_INDEX8_EXT:
  case GL_COLOR_INDEX12_EXT:
  case GL_COLOR_INDEX16_EXT:
    {

      int y;
      unsigned char *bsrc,*bdst;

      bsrc=(unsigned char *)(image->Data+(yoffset*image->Width+xoffset));
      bdst=((unsigned char *)destimg)+(yoffset*image->Width+xoffset);
    
      for(y=0;y<height;y++) {
	MEMCPY(bdst,bsrc,width);
	bsrc += image->Width;
	bdst += image->Width;
      }
    }
  break;
  case 2:
  case GL_LUMINANCE_ALPHA:
  case GL_LUMINANCE4_ALPHA4:
  case GL_LUMINANCE6_ALPHA2:
  case GL_LUMINANCE8_ALPHA8:
  case GL_LUMINANCE12_ALPHA4:
  case GL_LUMINANCE12_ALPHA12:
  case GL_LUMINANCE16_ALPHA16:
    {
      int x,y;
      unsigned char *src;
      unsigned short *dst,a,l;
      int simgw,dimgw;

      src=(unsigned char *)(image->Data+(yoffset*image->Width+xoffset)*2);
      dst=destimg+(yoffset*image->Width+xoffset);
    
      simgw=(image->Width-width)*2;
      dimgw=image->Width-width;
      for(y=0;y<height;y++) {
	for(x=0;x<width;x++) {
	  l=*src++;
	  a=*src++;
	  *dst++=(a << 8) | l;
	}

	src += simgw;
	dst += dimgw;
      }
    }
    break;
  case 3:
  case GL_RGB:
  case GL_R3_G3_B2:
  case GL_RGB4:
  case GL_RGB5:
  case GL_RGB8:
  case GL_RGB10:
  case GL_RGB12:
  case GL_RGB16:
    {
      int x,y;
      unsigned char *src;
      unsigned short *dst,r,g,b;
      int simgw,dimgw;

      src=(unsigned char *)(image->Data+(yoffset*image->Width+xoffset)*3);
      dst=destimg+(yoffset*image->Width+xoffset);
    
      simgw=(image->Width-width)*3;
      dimgw=image->Width-width;
      for(y=0;y<height;y++) {
	for(x=0;x<width;x++) {
	  r=*src++;
	  g=*src++;
	  b=*src++;
	  *dst++=((0xf8 & r) << (11-3))  |
            ((0xfc & g) << (5-3+1))      |
            ((0xf8 & b) >> 3); 
	}

	src += simgw;
	dst += dimgw;
      }
    }
    break;
  case 4:
  case GL_RGBA:
  case GL_RGBA2:
  case GL_RGBA4:
  case GL_RGB5_A1:
  case GL_RGBA8:
  case GL_RGB10_A2:
  case GL_RGBA12:
  case GL_RGBA16:
    {
      int x,y;
      unsigned char *src;
      unsigned short *dst,r,g,b,a;
      int simgw,dimgw;

      src=(unsigned char *)(image->Data+(yoffset*image->Width+xoffset)*4);
      dst=destimg+(yoffset*image->Width+xoffset);
    
      simgw=(image->Width-width)*4;
      dimgw=image->Width-width;
      for(y=0;y<height;y++) {
	for(x=0;x<width;x++) {
	  r=*src++;
	  g=*src++;
	  b=*src++;
	  a=*src++;
	  *dst++=((0xf0 & a) << 8) |
	    ((0xf0 & r) << 4)      |
	    (0xf0 & g)             |
	    ((0xf0 & b) >> 4);
	}

	src += simgw;
	dst += dimgw;
      }
    }
    break;
  default:
    fprintf(stderr,"fx Driver: wrong internalFormat in texbuildsubimagemap()\n");
    fxCloseHardware();
    exit(-1);
    break;
  }
}
 

void texsubimg(GLcontext *ctx, GLenum target,
	       struct gl_texture_object *tObj, GLint level,
	       GLint xoffset, GLint yoffset, GLint width, GLint height,
	       GLint internalFormat, const struct gl_texture_image *image)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  texinfo *ti;
  GrTextureFormat_t format;
  int wscale,hscale;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: (%d) texsubimg(...,%d,%x,%d,%d...)\n",tObj->Name,
	  target,internalFormat,image->Width,image->Height);
#endif

  if(target!=GL_TEXTURE_2D) {
#ifndef FX_SILENT
    fprintf(stderr,"fx Driver: unsupported texture in texsubimg()\n");
#endif
    return;
  }

  if(!tObj->DriverData)
    return;

  ti=(texinfo *)tObj->DriverData;

  texgetformat(ctx,internalFormat,&format,NULL);

  if(ti->info.format!=format) {
#if defined(DEBUG_FXMESA)
    fprintf(stderr,"fxmesa:  ti->info.format!=format in texsubimg()\n");
#endif
    teximg(ctx,target,tObj,level,internalFormat,image);

    return;
  }

  texgetinfo(image->Width,image->Height,NULL,NULL,NULL,NULL,&wscale,&hscale);

  if((wscale!=1) || (hscale!=1)) {
#if defined(DEBUG_FXMESA)
    fprintf(stderr,"fxmesa:  (wscale!=1) || (hscale!=1) in texsubimg()\n");
#endif
    teximg(ctx,target,tObj,level,internalFormat,image);

    return;
  }

  if(ti->tmi.translate_mipmaplevel[level])
    texbuildsubimagemap(image,internalFormat,xoffset,yoffset,
			width,height,ti->tmi.mipmaplevel[level]);

  reloadsubmipmaplevel(fxMesa,tObj,level,yoffset,height);
}


#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function5(void)
{
  return 0;
}

#endif  /* FX */
