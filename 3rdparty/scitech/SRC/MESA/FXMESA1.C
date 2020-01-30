/* -*- mode: C; tab-width:8;  -*-

             fxmesa1.c - 3Dfx VooDoo/Mesa interface
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
 *
 * Jan 19, 1998 - brianp
 *         replaced the clear_color_depth() func with David's latest one
 *
 * V0.22 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - written the support for the MESA_WGL_FX env. var. but
 *           not tested because I have only Voodoo Graphics boards
 *         - fixed a bug in the backface culling code
 *           (thanks to David Farrell for an example of the problem)
 *         - fixed the "Quake2 elevator" bug
 *         - GL_POLYGONS with 3/4 vertices are now drawn as
 *           GL_TRIANLGES/GL_QUADS (a small optimization for GLQuake)
 *         - fixed a bug in fxmesa6.h for GL_LINE_LOOP
 *         - fixed a NearFarStack bug in the Mesa when applications
 *           directly call glLoadMatrix to load a projection matrix 
 *         - done some cleanup in the fxmesa2.c file
 *         - the driver no longer translates the texture maps
 *           when the Mesa internal format and the Voodoo
 *           format are the some (usefull for 1 byte texture maps
 *           where the driver can directly use the Mesa texture
 *           map). Also the amount of used memory is halfed
 *         - fixed a bug for GL_DECAL and GL_RGBA
 *         - fixed a bug in the clear_color_depth()
 *         - tested the v0.22 with the Mesa-2.6beta2. Impressive
 *           performances improvement thanks to the new Josh's
 *           asm code (+10fps in the isosurf demo, +5fps in GLQuake
 *           TIMEREFRESH)
 *         - written a optimized version of the RenderVB Mesa driver
 *           function. The Voodoo driver is now able to upload polygons
 *           in the most common cases at a very good speed. Good
 *           performance improvement for large set of small polygons
 *         - optimized the asm code for setting up the color informations
 *           in the Glide grVertex structure
 *         - fixed a bug in the fxmesa2.c asm code (the ClipMask[]
 *           wasn't working)
 *
 *         Josh Vanderhoof (joshv@planet.net)
 *         - removed the flush() function because it isn't required
 *         - limited the maximum number of swapbuffers in the Voodoo
 *         commands FIFO (controlled by the env. var. MESA_FX_SWAP_PENDING)
 *
 *         Holger Kleemiss (holger.kleemiss@metronet.de) STN Atlas Elektronik GmbH
 *         - applied some patch for the Voodoo Rush
 *
 * V0.21 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - the driver is now able to take advantage of the ClipMask[],
 *           ClipOrMask and ClipAndMask informations also under Windows
 *         - introduced a new function in the Mesa driver interface
 *           ClearColorAndDepth(). Now the glClear() function is
 *           2 times faster (!) when you have to clear the color buffer
 *           and the depth buffer at some time
 *         - written the first version of the fxRenderVB() driver
 *           function
 *         - optimized the glTexImage() path
 *         - removed the fxMesaTextureUsePalette() support
 *         - fixed a bug in the points support (thanks to David Farrell
 *           for an example of the problem)
 *         - written the optimized path for glSubTexImage(),
 *           introduced a new function in the Mesa driver interface
 *           TexSubImage(...)
 *         - fixed a bug for glColorMask and glDepthMask
 *         - the wbuffer is not more used. The Voodoo driver uses
 *           a standard 16bit zbuffer in all cases. It is more consistent
 *           and now GLQuake and GLQuake2test work also with a GL_ZTRICK 0
 *         - the driver is now able to take advantage of the ClipMask[],
 *           ClipOrMask and ClipAndMask informations (under Linux);
 *         - rewritten the setup_fx_units() function, now the texture
 *           mapping support is compliant to the OpenGL specs (GL_BLEND
 *           support is still missing). The LinuxGLQuake console correctly
 *           fade in/out and transparent water of GLQuake2test works fine
 *         - written the support for the env. var. FX_GLIDE_SWAPINTERVAL
 *         - found a bug in the Mesa core. There is a roundup problem for
 *           color values out of the [0.0,1.0] range
 *
 *         Wonko <matt@khm.de>
 *         - fixed a Voodoo Rush related problem in the fxwgl.c
 *
 *         Daryll Strauss <daryll@harlot.rb.ca.us>
 *         - written the scissor test support
 *
 * V0.20 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - written the closetmmanger() function in order to free all the memory
 *           allocated by the Texture Memory Manager (it will be useful
 *           when the support for multiple contexts/boards will be ready)
 *         - now the Voodoo driver runs without printing any information,
 *           define the env. var. MESA_FX_INFO if you want to read some
 *           information about the hardware and some statistic
 *         - written a small workaround for the "GLQuake multiplayer white box bug"
 *           in the setup_fx_units() funxtions. I'm already rewriting
 *           this function because it is the source of nearly all the current
 *           Voodoo driver problems
 *         - fixed the GLQuake texture misalignment problem (the texture
 *           coordinates must be scaled between 0.0 and 256.0 and not
 *           between 0.0 and 255.0)
 *         - written the support for the GL_EXT_shared_texture_palette
 *         - some small change for supporting the automatic bulding of the
 *           OpenGL32.dll under the Windows platform
 *         - the redefinition of a mipmap level is now a LOT faster. This path
 *           is used by GLQuake for dynamic lighting with some call to glTexSubImage2D()
 *         - the texture memory is now managed a set of 2MB blocks so
 *           texture maps can't be allocated on a 2MB boundary. The new Pure3D
 *           needs this kind of support (and probably any other Voodoo Graphics
 *           board with more than 2MB of texture memory)
 *
 *         Brian Paul (brianp@elastic.avid.com) Avid Technology
 *         - added write_monocolor_span(), fixed bug in write_color_span()
 *         - added test for stenciling in choosepoint/line/triangle functions
 *
 *         Joe Waters (falc@attila.aegistech.com) Aegis
 *         - written the support for the env. var. SST_SCREENREFRESH
 *
 * V0.19 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - written the 3Dfx Global Palette extension for GLQuake
 *         - written the support for the GL_EXT_paletted_texture (it works only with GL_RGBA
 *           palettes and the alpha value is ignored ... this is a limitation of the
 *           the current Glide version and Voodoo hardware)
 *         - fixed the amount of memory allocated for 8bit textures
 *         - merged the under construction v0.19 driver with the Mesa 2.5
 *         - finally written the support for deleting textures
 *         - introduced a new powerful texture memory manager: the texture memory
 *           is used as a cache of the set of all defined texture maps. You can
 *           now define several MB of texture maps also with a 2MB of texture memory
 *           (the texture memory manager will do automatically all the swap out/swap in
 *           work). The new texture memory manager has also
 *           solved a lot of other bugs/no specs compliance/problems
 *           related to the texture memory usage. The texture
 *           manager code is inside the new fxmesa3.c file
 *         - broken the fxmesa.c file in two files (fxmesa1.c and fxmesa2.c)
 *           and done some code cleanup
 *         - now is possible to redefine texture mipmap levels already defined
 *         - fixed a problem with the amount of texture memory allocated for textures
 *           with not all mipmap levels defined
 *         - fixed a small problem with single buffer rendering
 *
 *         Brian Paul (brianp@elastic.avid.com) Avid Technology
 *         - read/write_color_span() now use front/back buffer correctly
 *         - create GLvisual with 5,6,5 bits per pixel, not 8,8,8
 *         - removed a few ^M characters from fxmesa2.c file
 *
 * V0.18 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - the Mesa-2.4beta3 is finally using the driver quads support (the
 *           previous Mesa versions have never taken any advantage from the quads support !)
 *         - tested with the Glide 2.4 for Win
 *         - ported all asm code to Linux
 *         - ported the v0.18 to Linux (without asm code)
 *         - back to Linux !!!
 *         - optimized the SETUP macro (no more vertex snap for points and lines)
 *         - optimized the SETUP macro (added one argument)
 *         - the Mesa/Voodoo is now 20/30% for points, lines and small triangles !
 *         - performance improvement setting VBSIZE to 72 
 *         - the GrVertex texture code is now written in asm
 *         - the GrVertex zbuffer code is now written in asm
 *         - the GrVertex wbuffer code is now written in asm
 *         - the GrVertex gouraud code is now written in asm
 *         - the GrVertex snap code is now written in asm
 *         - changed the 8bit compressed texture maps in 8bit palette texture maps
 *           support (it has the some advantage of compressed texture maps without the
 *           problem of a fixed NCC table for all mipmap levels)
 *         - written the support for 8bit compressed texture maps (but texture maps with
 *           more than one mipmap level aren't working fine)
 *         - finnaly everthing is working fine in MesaQuake !
 *         - fixed a bug in the computation of texture mapping coordinates (I have found
 *           the bug thanks to MesaQuake !)
 *         - written the GL_REPLACE support (mainly for MesaQuake)
 *         - written the support for textures with not all mipmap levels defined
 *         - rewritten all the Texture memory stuff
 *         - written the MesaQuake support (define MESAQUAKE)
 *         - working with a ZBuffer if glOrtho or not int the default glDepthRange,
 *           otherwise working with the WBuffer
 *         written the glDepthRange support
 *
 *         Diego Picciani (d.picciani@novacomp.it) Nova Computer s.r.l.
 *         - written the fxCloseHardware() and the fxQuaryHardware() (mainly
 *           for the VoodooWGL emulator)
 *
 *         Brian Paul (brianp@elastic.avid.com) Avid Technology
 *         - implemented read/write_color_span() so glRead/DrawPixels() works
 *         - now needs Glide 2.3 or later.  Removed GLIDE_FULL_SCREEN and call to grSstOpen()
 *
 * V0.17 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - optimized the bitmap support (66% faster)
 *         - tested with the Mesa 2.3beta2
 *
 *         Diego Picciani (d.picciani@novacomp.it) Nova Computer s.r.l.
 *         - solved a problem with the drawbitmap() and the Voodoo Rush
 *           (GR_ORIGIN_LOWER_LEFT did not work with the Stingray)
 *
 *         Brian Paul (brianp@elastic.avid.com) Avid Technology
 *         - linux stuff
 *         - general code clean-up
 *         - added attribList parameter to fxMesaCreateContext()
 *         - single buffering works now
 *         - VB colors are now GLubytes, removed ColorShift stuff
 *
 *         Paul Metzger
 *         - linux stuff
 *
 * V0.16 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - written the quadfunc support (no performance improvement)
 *         - written the support for the new Mesa 2.3beta1 driver interface (Wow ! It is faaaster)
 *         - rewritten the glBitmap support for the Glide 2.3 (~35% slower !)
 *         - written the glBitmap support for the most common case (fonts)
 *
 *         Jack Palevich
 *         - Glide 2.3 porting
 *
 *         Diego Picciani (d.picciani@novacomp.it) Nova Computer s.r.l.
 *         - extended the fxMesaCreateContext() and fxMesaCreateBestContext()
 *           functions in order to support also the Voodoo Rush
 *         - tested with the Hercules Stingray 128/3D (The rendering in a window works !)
 *
 * V0.15 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - written the GL_LUMINANCE_ALPHA support
 *         - written the GL_ALPHA support
 *         - written the GL_LUMINANCE support
 *         - now SETUP correctly set color for mono color sequences
 *         - written the 9x1,10x1,...,1x9,1x10,... texture map ratio support
 *         - written the no square texture map support
 *         - the fog table is no more rebuilt inside setup_fx_units() each time
 *
 *         Henri Fousse (arnaud@pobox.oleane.com) Thomson Training & Simulation
 *         - written (not yet finished: no texture mapping) support for glOrtho
 *         - some change to setup functions
 *         - the fog support is now fully compatible with the standard OpenGL
 *         - rewritten several parts of the driver in order to take
 *           advantage of meshes (40% faster !!!)
 *
 * V0.14 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - now glAlphaFunc() works
 *         - now glDepthMask() works
 *         - solved a mipmap problem when using more than one texture
 *         - moved ti, texid and wscale inside the fxMesaContext (now we can
 *           easy support more ctx and more boards)
 *         - the management of the fxMesaContext was completly broken !
 *         - solved several problems about Alpha and texture Alpha
 *         - 4 (RGBA) texture channels supported
 *         - setting the default color to white
 *
 *         Henri Fousse (arnaud@pobox.oleane.com) Thomson Training & Simulation
 *         - small change to fxMesaCreateContext() and fxMesaMakeCurrent()
 *         - written the fog support
 *         - setting the default clear color to black
 *         - written cleangraphics() for the onexit() function
 *         - written fxMesaCreateBestContext()
 *
 * V0.13 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - now glBlendFunc() works for all glBlendFunc without DST_ALPHA
 *           (because the alpha buffer is not yet implemented) 
 *         - now fxMesaCreateContext() accept resolution and refresh rate
 *         - fixed a bug for texture mapping: the w (alias z) must be set
 *           also without depth buffer
 *         - fixed a bug for texture image with width!=256
 *         - written texparam()
 *         - written all point, line and triangle functions for all possible supported
 *           contexts and the driver is slight faster with points, lines and small triangles
 *         - fixed a small bug in fx/fxmesa.h (glOrtho)
 *
 * V0.12 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - glDepthFunc supported
 *         - introduced a trick to discover the far plane distance
 *           (see fxMesaSetFar and fx/fxmesa.h)
 *         - now the wbuffer works with homogeneous coordinate (and it
 *           doesn't work with a glOrtho projection :)
 *         - solved several problems with homogeneous coordinate and texture mapping
 *         - fixed a bug in all line functions
 *         - fixed a clear framebuffer bug
 *         - solved a display list/teximg problem (but use
 *           glBindTexture: it is several times faster)
 *
 * V0.11 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - introduced texture mapping support (not yet finished !)
 *         - tested with Mesa2.2b6
 *         - the driver is faster 
 *         - written glFlush/glFinish
 *         - the driver print a lot of info about the Glide lib
 *
 * v0.1  - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         - Initial revision
 *
 */

#if defined(FX)

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "fxdrv.h"

fxMesaContext CurrentfxMesaCtx=NULL;

/*
 * Status of 3Dfx hardware initialization
 */

static int glideInitialized = 0;
static int _3dfxPresent = 0;
static GrHwConfiguration hwconfig;

static void setup_dd_pointers(GLcontext *ctx);

/**********************************************************************/
/*****                 Miscellaneous functions                    *****/
/**********************************************************************/

/* return buffer size information */
static void buffer_size(GLcontext *ctx, GLuint *width, GLuint *height)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: buffer_size(...)\n");
#endif

  *width=fxMesa->width;
  *height=fxMesa->height;
}


/* Set current drawing color */
static void set_color(GLcontext *ctx, GLubyte red, GLubyte green,
		      GLubyte blue, GLubyte alpha )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: set_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
#endif

  fxMesa->color=FXCOLOR(red,green,blue,alpha);
}


/* implements glClearColor() */
static void clear_color(GLcontext *ctx, GLubyte red, GLubyte green,
			GLubyte blue, GLubyte alpha )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: clear_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
#endif
 
  fxMesa->clearc=FXCOLOR(red,green,blue,255);
  fxMesa->cleara=alpha;
}


/* clear the frame buffer */
static void clear(GLcontext *ctx, GLboolean all,
		  GLint x, GLint y, GLint width, GLint height )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: clear(%d,%d,%d,%d)\n",x,y,width,height);
#endif

  grDepthMask(FXFALSE);
  grColorMask(FXTRUE,FXFALSE);

  grBufferClear(fxMesa->clearc,fxMesa->cleara,0);

  if(ctx->Depth.Mask)
    grDepthMask(FXTRUE);

  if(ctx->Color.ColorMask)
    grColorMask(FXTRUE,FXFALSE);
  else
    grColorMask(FXFALSE,FXFALSE);
}

/* clear the frame buffer and the depth buffer at the some time */
static void clear_color_depth(GLcontext *ctx, GLboolean all,
                              GLint x, GLint y, GLint width, GLint height )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa:clear_color_depth(%d,%d,%d,%d)\n",x,y,width,height);
#endif

  grDepthMask(FXTRUE);
  grColorMask(FXTRUE,FXFALSE);


grBufferClear(fxMesa->clearc,fxMesa->cleara,(FxU16)(ctx->Depth.Clear*0xffff));

  /* The problem was in the line below */ 
  if(!ctx->Depth.Mask)
    grDepthMask(FXFALSE);

  if(ctx->Color.ColorMask)
    grColorMask(FXTRUE,FXFALSE);
  else
    grColorMask(FXFALSE,FXFALSE);
}


/*  set the buffer used in double buffering */
static GLboolean set_buffer(GLcontext *ctx, GLenum mode )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: set_buffer(%x)\n",mode);
#endif

  if (fxMesa->double_buffer) {
    if(mode==GL_FRONT)
      fxMesa->currentfb = GR_BUFFER_FRONTBUFFER;
    else if(mode==GL_BACK)
      fxMesa->currentfb = GR_BUFFER_BACKBUFFER;
    else
      return GL_FALSE;
  } else {
    if(mode==GL_FRONT)
      fxMesa->currentfb = GR_BUFFER_FRONTBUFFER;
    else
      return GL_FALSE;
  }

  grRenderBuffer(fxMesa->currentfb);

  setup_dd_pointers(ctx);

  return GL_TRUE;
}


static GLboolean drawbitmap(GLcontext *ctx, GLsizei width, GLsizei height,
                            GLfloat xorig, GLfloat yorig,
                            GLfloat xmove, GLfloat ymove,
                            const struct gl_image *bitmap)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  FxU16 *p;
  GrLfbInfo_t info;
  GLubyte *pb;
  int x,y;
  GLint r,g,b,a,px,py,scrwidth,scrheight,stride;
  FxU16 color;

#define ISCLIPPED(rx) ( ((rx)<0) || ((rx)>=scrwidth) )
#define DRAWBIT(i) {	   \
  if(!ISCLIPPED(x+px))	   \
    if( (*pb) & (1<<(i)) ) \
      (*p)=color;	   \
  p++;                     \
  x++;			   \
  if(x>=width) {           \
    pb++;                  \
    break;                 \
  }                        \
}

  scrwidth=fxMesa->width;
  scrheight=fxMesa->height;

  px=(GLint)((ctx->Current.RasterPos[0]-xorig)+0.0F);
  py=(GLint)((ctx->Current.RasterPos[1]-yorig)+0.0F);

  if((px>=scrwidth) || (px+width<=0) || (py>=scrheight) || (py+height<=0))
    return GL_TRUE;

  pb=(GLubyte *)bitmap->Data;

  if(py<0) {
    pb+=(bitmap->Height*(-py)) >> (3+1);
    height+=py;
    py=0;
  }

  if(py+height>=scrheight)
    height-=(py+height)-scrheight;

  info.size=sizeof(info);
  if(!grLfbLock(GR_LFB_WRITE_ONLY,
		fxMesa->currentfb,
		GR_LFBWRITEMODE_565,
		GR_ORIGIN_UPPER_LEFT,
		FXFALSE,
		&info)) {
#ifndef FX_SILENT
    fprintf(stderr,"fx Driver: error locking the linear frame buffer\n");
#endif
    return GL_TRUE;
  }

  r=(GLint)(ctx->Current.RasterColor[0]*ctx->Visual->RedScale);
  g=(GLint)(ctx->Current.RasterColor[1]*ctx->Visual->GreenScale);
  b=(GLint)(ctx->Current.RasterColor[2]*ctx->Visual->BlueScale);
  a=(GLint)(ctx->Current.RasterColor[3]*ctx->Visual->AlphaScale);
  color=(FxU16)
    ( ((FxU16)0xf8 & r) <<(11-3))  |
    ( ((FxU16)0xfc & g) <<(5-3+1)) |
    ( ((FxU16)0xf8 & b) >> 3);

  stride=info.strideInBytes>>1;

  /* This code is a bit slow... */

  for(y=0;y<height;y++) {
    p=((FxU16 *)info.lfbPtr)+px+((scrheight-(y+py))*stride);

    for(x=0;;) {
      DRAWBIT(7);	DRAWBIT(6);	DRAWBIT(5);	DRAWBIT(4);
      DRAWBIT(3);	DRAWBIT(2);	DRAWBIT(1);	DRAWBIT(0);
      pb++;
    }
  }

  grLfbUnlock(GR_LFB_WRITE_ONLY,fxMesa->currentfb);

#undef ISCLIPPED
#undef DRAWBIT

  return GL_TRUE;
}

/************************************************************************/
/**************** 3D depth buffer functions *****************************/
/************************************************************************/

/* this is a no-op, since the z-buffer is in hardware */
static void alloc_depth_buffer(GLcontext *ctx)
{
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: alloc_depth_buffer()\n");
#endif
}

static void clear_depth_buffer(GLcontext *ctx)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: clear_depth_buffer(%f)\n",ctx->Depth.Clear);
#endif

  grDepthMask(FXTRUE);
  grColorMask(FXFALSE,FXFALSE);

  grBufferClear(fxMesa->clearc,fxMesa->cleara,(FxU16)(ctx->Depth.Clear*0xffff));
  
  if(!ctx->Depth.Mask)
    grDepthMask(FXFALSE);

  if(ctx->Color.ColorMask)
    grColorMask(FXTRUE,FXFALSE);
}

/************************************************************************/
/*****                Span functions                                *****/
/************************************************************************/

static void write_color_span( GLcontext *ctx, 
                              GLuint n, GLint x, GLint y,
                              const GLubyte red[], const GLubyte green[],
                              const GLubyte blue[], const GLubyte alpha[],
                              const GLubyte mask[] )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLubyte data[MAX_WIDTH][4];
  GLint i;
  GLint bottom = fxMesa->height-1;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: write_color_span()\n");
#endif

  if (mask) {
    /* this is really inefficient, but it works */
    grLfbWriteColorFormat(GR_COLORFORMAT_ABGR);
    for (i=0;i<n;i++) {
      if (mask[i]) {
        data[0][0] = red[i];
        data[0][1] = green[i];
        data[0][2] = blue[i];
        data[0][3] = alpha[i];
        grLfbWriteRegion( fxMesa->currentfb, x+i, bottom-y,
                          GR_LFB_SRC_FMT_8888, 1, 1, 0, data );
      }
    }
  }
  else {
    for (i=0;i<n;i++) {
      data[i][0] = red[i];
      data[i][1] = green[i];
      data[i][2] = blue[i];
      data[i][3] = alpha[i];
    }
    grLfbWriteColorFormat(GR_COLORFORMAT_ABGR);
    grLfbWriteRegion( fxMesa->currentfb, x, bottom-y, GR_LFB_SRC_FMT_8888,
                      n, 1, 0, data );
  }
}


static void write_monocolor_span( GLcontext *ctx, 
                                  GLuint n, GLint x, GLint y,
                                  const GLubyte mask[] )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLint i;
  GLint bottom = fxMesa->height-1;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: write_color_span()\n");
#endif
  if (mask) {
    /* this is really inefficient, but it works */
    grLfbWriteColorFormat(GR_COLORFORMAT_ABGR);
    for (i=0;i<n;i++) {
      if (mask[i]) {
        grLfbWriteRegion( fxMesa->currentfb, x+i, bottom-y,
                          GR_LFB_SRC_FMT_8888, 1, 1, 0,
                          (GLubyte *) &fxMesa->color );
      }
    }
  }
  else {
    GLuint data[MAX_WIDTH];
    for (i=0;i<n;i++) {
       data[i] = (GLuint) fxMesa->color;
    }
    grLfbWriteColorFormat(GR_COLORFORMAT_ABGR);
    grLfbWriteRegion( fxMesa->currentfb, x, bottom-y, GR_LFB_SRC_FMT_8888,
                      n, 1, 0, (GLubyte *) data );
  }
}


static void read_color_span( GLcontext *ctx, 
                             GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLushort data[MAX_WIDTH];
  GLint i;
  GLint bottom = fxMesa->height-1;

#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: read_color_span()\n");
#endif

  assert(n < MAX_WIDTH);
  grLfbReadRegion( fxMesa->currentfb, x, bottom-y, n, 1, 0, data);
  for (i=0;i<n;i++) {
    red[i]   = (data[i] & 0x001f) << 3;
    green[i] = (data[i] & 0x07e0) >> 3;
    blue[i]  = (data[i] & 0xf800) >> 8;
    alpha[i] = 255;
  }
}

/************************************************************************/
/************************************************************************/
/************************************************************************/

static void finish(GLcontext *ctx)
{
  grSstIdle();
}

/************************************************************************/
/************************************************************************/
/************************************************************************/

static void SetNearFar(GLcontext *ctx, GLfloat n, GLfloat f)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;

  if(fxMesa) {
    fxMesa->nearval=fabs(n);
    fxMesa->farval=fabs(f);
    fxMesa->wscale=fxMesa->farval/65535.0;

    /*
     * We need to update fog table because it depends on w 
     * and w is adjusted to the maximum range.
     */
    if(ctx->Fog.Enabled && ctx->Hint.Fog==GL_NICEST) {
      switch(ctx->Fog.Mode)     {
      case GL_LINEAR:
        guFogGenerateLinear(fxMesa->fogtable,
                            ctx->Fog.Start/fxMesa->wscale,
                            ctx->Fog.End /fxMesa->wscale); /* works ? */
        break;
      case GL_EXP:
        guFogGenerateExp(fxMesa->fogtable,
                         ctx->Fog.Density*fxMesa->wscale);
        break;
      case GL_EXP2:
        guFogGenerateExp2(fxMesa->fogtable,
                          ctx->Fog.Density*fxMesa->wscale);
        break;
      default: /* That should never happen */
        break; 
      }
    }
  }
}


void fxMesaSetNearFar(GLfloat n, GLfloat f)
{
  if(CurrentfxMesaCtx)
    SetNearFar(CurrentfxMesaCtx->gl_ctx, n, f);
}


static const char *renderer_string(void)
{
  return "Glide v0.22";
}


/*
 * The 3Dfx Global Palette extension for GLQuake.
 * More a trick than a real extesion, use the shared global
 * palette extension. 
 */
void APIENTRY gl3DfxSetPaletteEXT(GLuint *pal)
{
#if defined(DEBUG_FXMESA)
  int i;

  fprintf(stderr,"fxmesa: gl3DfxSetPaletteEXT()\n");

  for(i=0;i<256;i++)
    fprintf(stderr,"%x\n",pal[i]);
#endif

  if(CurrentfxMesaCtx) {
    CurrentfxMesaCtx->globalpalette_texture=1;
    grTexDownloadTable(GR_TMU0,GR_TEXTABLE_PALETTE,(GuTexPalette *)pal); 
  }
}


static void update_dd_pointers(GLcontext *ctx)
{
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: update_dd_pointers()\n");
#endif

  ctx->Driver.AllocDepthBuffer=alloc_depth_buffer;
  ctx->Driver.ClearDepthBuffer=clear_depth_buffer;
  ctx->Driver.DepthTestSpan=NULL;
  ctx->Driver.DepthTestPixels=NULL;
  ctx->Driver.ReadDepthSpanFloat=NULL;
  ctx->Driver.ReadDepthSpanInt=NULL;

  /* acc. functions*/

  ctx->Driver.PointsFunc=choose_points_function(ctx);
  ctx->Driver.LineFunc=choose_line_function(ctx);
  ctx->Driver.TriangleFunc=choose_triangle_function(ctx);
  ctx->Driver.QuadFunc=choose_quad_function(ctx);
  ctx->Driver.RectFunc=NULL;

  ctx->Driver.RasterSetup=choose_setup_function(ctx);
  ctx->Driver.RenderVB=choose_rendervb_function(ctx);

  setup_fx_units(ctx);
}


static void setup_dd_pointers(GLcontext *ctx)
{
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: setup_dd_pointers()\n");
#endif

  ctx->Driver.UpdateState=update_dd_pointers;
         
  ctx->Driver.RendererString=renderer_string;

  ctx->Driver.NearFar=SetNearFar;

  ctx->Driver.ClearIndex=NULL;
  ctx->Driver.ClearColor=clear_color;
  ctx->Driver.Clear=clear;
  ctx->Driver.ClearColorAndDepth=clear_color_depth;

  ctx->Driver.Index=NULL;
  ctx->Driver.Color=set_color;

  ctx->Driver.SetBuffer=set_buffer;
  ctx->Driver.GetBufferSize=buffer_size;

  ctx->Driver.AllocDepthBuffer=alloc_depth_buffer;
  ctx->Driver.ClearDepthBuffer=clear_depth_buffer;

  ctx->Driver.Bitmap=drawbitmap;
  ctx->Driver.DrawPixels=NULL;

  ctx->Driver.Finish=finish;
  ctx->Driver.Flush=NULL;

  ctx->Driver.TexEnv=texenv;
  ctx->Driver.TexImage=teximg;
  ctx->Driver.TexSubImage=texsubimg;
  ctx->Driver.TexParameter=texparam;
  ctx->Driver.BindTexture=texbind;
  ctx->Driver.DeleteTexture=texdel;
  ctx->Driver.UpdateTexturePalette=texpalette;
  ctx->Driver.UseGlobalTexturePalette=useglbtexpalette;

  ctx->Driver.WriteColorSpan      =write_color_span;
  ctx->Driver.WriteMonocolorSpan  =write_monocolor_span;
  ctx->Driver.WriteColorPixels    =NULL;
  ctx->Driver.WriteMonocolorPixels=NULL;
  ctx->Driver.WriteIndexSpan      =NULL;
  ctx->Driver.WriteMonoindexSpan  =NULL;
  ctx->Driver.WriteIndexPixels    =NULL;
  ctx->Driver.WriteMonoindexPixels=NULL;

  ctx->Driver.ReadIndexSpan   =NULL;
  ctx->Driver.ReadColorSpan   =read_color_span;
  ctx->Driver.ReadIndexPixels =NULL;
  ctx->Driver.ReadColorPixels =NULL;

  update_dd_pointers(ctx);
}


#define NUM_RESOLUTIONS 3

static resolutions[NUM_RESOLUTIONS][3]={ 
  /*
    { 320, 200, GR_RESOLUTION_320x200 }
    { 320, 240, GR_RESOLUTION_320x240 },
    { 400, 256, GR_RESOLUTION_400x256 },
    { 512, 256, GR_RESOLUTION_512x256 },
  */
  { 512, 384, GR_RESOLUTION_512x384 },
  /*
  { 640, 200, GR_RESOLUTION_640x200 },
  { 640, 350, GR_RESOLUTION_640x350 },
  */
  { 640, 400, GR_RESOLUTION_640x400 },
  { 640, 480, GR_RESOLUTION_640x480 }
  /*,
    { 800, 600, GR_RESOLUTION_800x600 },
    { 856, 480, GR_RESOLUTION_856x480 },
    { 960, 720, GR_RESOLUTION_960x720 }
    */
};

GrScreenResolution_t bestResolution(int width, int height)
{
  int i;

  for(i=0;i<NUM_RESOLUTIONS;i++)
    if((width<=resolutions[i][0]) && (height<=resolutions[i][1]))
      return resolutions[i][2];
        
  return GR_RESOLUTION_640x480;
}


fxMesaContext fxMesaCreateBestContext(GLuint win,GLint width, GLint height,
                                      const GLint attribList[])
{
  GrScreenRefresh_t refresh;

  refresh=GR_REFRESH_75Hz;

  if(getenv("SST_SCREENREFRESH")) {
    if(!strcmp(getenv("SST_SCREENREFRESH"),"60"))
      refresh=GR_REFRESH_60Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"70"))
      refresh=GR_REFRESH_70Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"72"))
      refresh=GR_REFRESH_72Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"75"))
      refresh=GR_REFRESH_75Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"80"))
      refresh=GR_REFRESH_80Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"85"))
      refresh=GR_REFRESH_85Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"90"))
      refresh=GR_REFRESH_90Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"100"))
      refresh=GR_REFRESH_100Hz;
    if(!strcmp(getenv("SST_SCREENREFRESH"),"120"))
      refresh=GR_REFRESH_120Hz;
  }

  return fxMesaCreateContext(win,bestResolution(width,height),
                             refresh,attribList);
}

/*
 * Create a new FX/Mesa context and return a handle to it.
 */
fxMesaContext fxMesaCreateContext(GLuint win,GrScreenResolution_t res,
                                  GrScreenRefresh_t ref,
                                  const GLint attribList[])
{
  fxMesaContext ctx;
  int i,type;
  GLboolean doubleBuffer=GL_FALSE;
  GLboolean alphaBuffer=GL_FALSE;
  GLboolean verbose=GL_FALSE;
  GLint depthSize=0;
  GLint stencilSize=0;
  GLint accumSize=0;

  if(getenv("MESA_FX_INFO"))
    verbose=GL_TRUE;

  i=0;
  while (attribList[i]!=FXMESA_NONE) {
    switch (attribList[i]) {
    case FXMESA_DOUBLEBUFFER:
      doubleBuffer=GL_TRUE;
      break;
    case FXMESA_ALPHA_SIZE:
      i++;
      alphaBuffer=attribList[i] > 0;
      break;
    case FXMESA_DEPTH_SIZE:
      i++;
      depthSize=attribList[i];
      break;
    case FXMESA_STENCIL_SIZE:
      i++;
      stencilSize=attribList[i];
      break;
    case FXMESA_ACCUM_SIZE:
      i++;
      accumSize=attribList[i];
      break;
    default:
      return NULL;
    }
    i++;
  }


#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: fxMesaCreateContext()\n");
#endif

  if(verbose)
    fprintf(stderr,"Mesa fx Voodoo Device Driver v0.22\nWritten by David Bucciarelli (tech.hmw@plus.it)\n");


  if((type=fxQueryHardware()) >= 0) {
    if(type==GR_SSTTYPE_VOODOO)
      win=0;

    if(!grSstWinOpen((FxU32)win,res,ref,
                     GR_COLORFORMAT_ABGR,GR_ORIGIN_LOWER_LEFT,2,1))
      return NULL;

    if(verbose)
      fprintf(stderr,"Glide screen size: %dx%d\n",
	      (int)grSstScreenWidth(),(int)grSstScreenHeight());
  } else {
    fprintf(stderr,"fxmesa: ERROR no Voodoo Graphics or Voodoo Rush !\n");
    return NULL;
  }

  ctx=(fxMesaContext)malloc(sizeof(struct fx_mesa_context));
  if(!ctx)
    return NULL;

  ctx->width=grSstScreenWidth();
  ctx->height=grSstScreenHeight();
  ctx->double_buffer=doubleBuffer;

  if(getenv("FX_GLIDE_SWAPINTERVAL"))
    ctx->swapinterval=atoi(getenv("FX_GLIDE_SWAPINTERVAL"));
  else
    ctx->swapinterval=1;

  if(getenv("MESA_FX_SWAP_PENDING"))
    ctx->maxpending_swapbuffers=atoi(getenv("MESA_FX_SWAP_PENDING"));
  else
    ctx->maxpending_swapbuffers=2;

  ctx->verbose=verbose;
  ctx->globalpalette_texture=0;

  ctx->color=FXCOLOR(255,255,255,255);
  ctx->clearc=0;
  ctx->cleara=0;

  ctx->stats.swapbuffer=0;
  ctx->stats.reqtexupload=0;
  ctx->stats.texupload=0;
  ctx->stats.memtexupload=0;

  ctx->hwconf=hwconfig;

  inittmmanager(ctx);

  grColorMask(FXTRUE,FXFALSE);
  if(doubleBuffer) {
    ctx->currentfb=GR_BUFFER_BACKBUFFER;
    grRenderBuffer(GR_BUFFER_BACKBUFFER);
  } else {
    ctx->currentfb=GR_BUFFER_FRONTBUFFER;
    grRenderBuffer(GR_BUFFER_FRONTBUFFER);
  }

  if(depthSize!=0)
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER); 

  ctx->gl_vis=gl_create_visual(GL_TRUE,     /* RGB mode */
			       alphaBuffer,
			       doubleBuffer,
			       depthSize,   /* depth_size */
			       stencilSize, /* stencil_size */
			       accumSize,   /* accum_size */
			       0,           /* index bits */
			       255.0,       /* color scales */
			       255.0,
			       255.0,
			       255.0,
			       5,6,5,0);

  ctx->gl_ctx=gl_create_context(ctx->gl_vis,
				NULL,  /* share list context */
				(void *) ctx);

  ctx->gl_buffer=gl_create_framebuffer(ctx->gl_vis);

  setup_dd_pointers(ctx->gl_ctx);

  SetNearFar(ctx->gl_ctx,1.0,100.0);

  return ctx;
}

/*
 * Function to set the new window size in the context (mainly for the Voodoo Rush)
 */

void fxMesaUpdateScreenSize(fxMesaContext ctx)
{
  ctx->width=grSstScreenWidth();
  ctx->height=grSstScreenHeight();
}

/*
 * Destroy the given FX/Mesa context.
 */
void fxMesaDestroyContext(fxMesaContext ctx)
{
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: fxMesaDestroyContext()\n");
#endif

  if(ctx) {
    gl_destroy_visual(ctx->gl_vis);
    gl_destroy_context(ctx->gl_ctx);
    gl_destroy_framebuffer(ctx->gl_buffer);

    fxCloseHardware();

    if(ctx->verbose) {
      fprintf(stderr,"Misc Stats:\n");
      fprintf(stderr,"  # swap buffer: %u\n",ctx->stats.swapbuffer);

      if(!ctx->stats.swapbuffer)
	ctx->stats.swapbuffer=1;

      fprintf(stderr,"Textures Stats:\n");
      fprintf(stderr,"  # request to TMM to upload a texture objects: %u\n",
	      ctx->stats.reqtexupload);
      fprintf(stderr,"  # request to TMM to upload a texture objects per swapbuffer: %.2f\n",
	      ctx->stats.reqtexupload/(float)ctx->stats.swapbuffer);
      fprintf(stderr,"  # texture objects uploaded: %u\n",
	      ctx->stats.texupload);
      fprintf(stderr,"  # texture objects uploaded per swapbuffer: %.2f\n",
	      ctx->stats.texupload/(float)ctx->stats.swapbuffer);
      fprintf(stderr,"  # MBs uploaded to texture memory: %.2f\n",
	      ctx->stats.memtexupload/(float)(1<<20));
      fprintf(stderr,"  # MBs uploaded to texture memory per swapbuffer: %.2f\n",
	      (ctx->stats.memtexupload/(float)ctx->stats.swapbuffer)/(float)(1<<20));
    }

    closetmmanager(ctx);

    free(ctx);
  }

  if(ctx==CurrentfxMesaCtx)
    CurrentfxMesaCtx=NULL;
}

/*
 * Make the specified FX/Mesa context the current one.
 */
void fxMesaMakeCurrent(fxMesaContext ctx)
{
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: fxMesaMakeCurrent()\n");
#endif

  if(!ctx) {
    gl_make_current(NULL,NULL);
    CurrentfxMesaCtx=NULL;

    return;
  }

  CurrentfxMesaCtx=ctx;

  gl_make_current(ctx->gl_ctx,ctx->gl_buffer);

  setup_dd_pointers(ctx->gl_ctx);

  /* The first time we call MakeCurrent we set the initial viewport size */
  if(ctx->gl_ctx->Viewport.Width==0)
    gl_Viewport(ctx->gl_ctx,0,0,ctx->width,ctx->height);
}


/*
 * Swap front/back buffers for current context if double buffered.
 */
void fxMesaSwapBuffers(void)
{
#if defined(DEBUG_FXMESA)
  fprintf(stderr,"fxmesa: ------------------------------- fxMesaSwapBuffers() -------------------------------\n");
#endif

  if(CurrentfxMesaCtx)
    if(CurrentfxMesaCtx->double_buffer) {
      grBufferSwap(CurrentfxMesaCtx->swapinterval);

      /*
       * Don't allow swap buffer commands to build up!
       */
      while(grBufferNumPending()>CurrentfxMesaCtx->maxpending_swapbuffers);

      CurrentfxMesaCtx->stats.swapbuffer++;
    }
}


#if defined(__WIN32__)
static int cleangraphics(void)
{
  fxMesaDestroyContext(CurrentfxMesaCtx);

  return 0;
}
#elif defined(__linux__)
static void cleangraphics(void)
{
  fxMesaDestroyContext(CurrentfxMesaCtx);
}
#endif


/*
 * Query 3Dfx hardware presence/kind
 */
int fxQueryHardware(void)
{
  if(!glideInitialized) {
      grGlideInit();
      if(grSstQueryHardware(&hwconfig)) {
	  grSstSelect(0);
	  _3dfxPresent=1;

	  if(getenv("MESA_FX_INFO")) {
	    char buf[80];
			
	    grGlideGetVersion(buf);
	    fprintf(stderr,"Using Glide V%s\nNumber of boards: %d\n",buf,hwconfig.num_sst);
	  }
      } else
	_3dfxPresent=0;

      glideInitialized=1;

#if defined(__WIN32__)
      onexit((_onexit_t)cleangraphics);
#elif defined(__linux__)
      atexit(cleangraphics);
#endif
    }

  if(!_3dfxPresent)
    return(-1);
  else
    return(hwconfig.SSTs[0].type);
}


/*
 * Shutdown Glide library
 */
int fxCloseHardware(void)
{
  if(glideInitialized) {
    if(getenv("MESA_FX_INFO")) {
      GrSstPerfStats_t		st;

      grSstPerfStats(&st);
      fprintf(stderr,"Pixels Stats:\n");
      fprintf(stderr,"  # pixels processed (minus buffer clears): %u\n",(unsigned)st.pixelsIn);
      fprintf(stderr,"  # pixels not drawn due to chroma key test failure: %u\n",(unsigned)st.chromaFail);
      fprintf(stderr,"  # pixels not drawn due to depth test failure: %u\n",(unsigned)st.zFuncFail);
      fprintf(stderr,"  # pixels not drawn due to alpha test failure: %u\n",(unsigned)st.aFuncFail);
      fprintf(stderr,"  # pixels drawn (including buffer clears and LFB writes): %u\n",(unsigned)st.pixelsOut);
    }

    grGlideShutdown();
    glideInitialized=0;
  }

  return 0;
}


#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function1(void)
{
  return 0;
}

#endif  /* FX */
