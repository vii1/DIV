
/*
 * 3Dfx VooDoo/Mesa interface.
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
 *
 * V0.16 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         written the quadfunc support (no performance improvement)
 *         written the support for the new Mesa 2.3beta1 driver interface (Wow ! It is faaaster)
 *         rewritten the glBitmap support for the Glide 2.3 (~35% slower !)
 *         written the glBitmap support for the most common case (fonts)
 *
 *         Jack Palevich
 *         Glide 2.3 porting
 *
 *         Diego Picciani (d.picciani@novacomp.it) Nova Computer s.r.l.
 *         extended the fxMesaCreateContext() and fxMesaCreateBestContext() functions in order to support also the Voodoo Rush
 *         tested with the Hercules Stingray 128/3D (The rendering in a window works !)
 *
 * V0.15 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         written the GL_LUMINANCE_ALPHA support
 *         written the GL_ALPHA support
 *         written the GL_LUMINANCE support
 *         now SETUP correctly set color for mono color sequences
 *         written the 9x1,10x1,...,1x9,1x10,... texture map ratio support
 *         written the no square texture map support
 *         the fog table is no more rebuilt inside setup_fx_units() each time
 *
 *         Henri Fousse (arnaud@pobox.oleane.com) Thomson Training & Simulation
 *         written (not yet finished: no texture mapping) support for glOrtho
 *         some change to setup functions
 *         the fog support is now fully compatible with the standard OpenGL
 *         rewritten several parts of the driver in order to take advantage of meshes (40% faster !!!)
 *
 * V0.14 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         now glAlphaFunc() works
 *         now glDepthMask() works
 *         solved a mipmap problem when using more than one texture
 *         moved ti, texid and wscale inside the fxMesaContext (now we can easy support more ctx and more boards)
 *         the management of the fxMesaContext was completly broken !
 *         solved several problems about Alpha and texture Alpha
 *         4 (RGBA) texture channels supported
 *         setting the default color to white
 *
 *         Henri Fousse (arnaud@pobox.oleane.com) Thomson Training & Simulation
 *         small change to fxMesaCreateContext() and fxMesaMakeCurrent()
 *         written the fog support
 *         setting the default clear color to black
 *         written cleangraphics() for the onexit() function
 *         written fxMesaCreateBestContext()
 *
 * V0.13 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         now glBlendFunc() works for all glBlendFunc without DST_ALPHA (because the alpha buffer is not yet implemented) 
 *         now fxMesaCreateContext() accept resolution and refresh rate
 *         fixed a bug for texture mapping: the w (alias z) must be set also without depth buffer
 *         fixed a bug for texture image with width!=256
 *         written texparam()
 *         written all point, line and triangle functions for all possible supported contexts and
 *         the driver is slight faster with points, lines and small triangles
 *         fixed a small bug in fx/fxmesa.h (glOrtho)
 *
 * V0.12 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         glDepthFunc supported
 *         introduced a trick to discover the far plane distance (see fxMesaSetFar and fx/fxmesa.h)
 *         now the wbuffer works with homogeneous coordinate (and it doesn't work with a glOrtho projection :)
 *         solved several problems with homogeneous coordinate and texture mapping
 *         fixed a bug in all line functions
 *         fixed a clear framebuffer bug
 *         solved a display list/teximg problem (but use glBindTexture: it is several times faster)
 *
 * V0.11 - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         introduced texture mapping support (not yet finished !)
 *         tested with Mesa2.2b6
 *         the driver is faster 
 *         written glFlush/glFinish
 *         the driver print a lot of info about the Glide lib
 *
 * v0.1  - David Bucciarelli (tech.hmw@plus.it) Humanware s.r.l.
 *         Initial revision
 *
 */

#ifdef FX


#include <stdio.h>
#include <math.h>

#include "context.h"
#include "types.h"
#include "xform.h"

#include "macros.h"
#include "vb.h"
#include "texture.h"

#include "fxmesa.h"

#define MAXNUM_TEX		128

#define FXCOLOR(r,g,b,a) (( ((unsigned int)(a))<<24 )|( ((unsigned int)(b))<<16 )|( ((unsigned int)(g))<<8 )|(r))

#define FUNC_DEPTH			0x01
#define FUNC_SMOOTH			0x02
#define FUNC_TEX_DECAL	0x04
#define FUNC_TEX_MOD		0x08

typedef struct {
	GrLOD_t smallLOD; 
	GrLOD_t largeLOD;
	float sscale,tscale;
	int levelsdefined;
} texinfo;

struct fx_mesa_context {
  GLcontext *gl_ctx;		    /* the core Mesa context */
  GLvisual *gl_vis;		    /* describes the color buffer */
  GLframebuffer *gl_buffer;	/* the ancillary buffers */

  GLint width, height;		    /* size of color buffer */

	GrBuffer_t currentfb;

  GrColor_t color;
  GrColor_t clearc;
  GrAlpha_t cleara;

	GrMipMapId_t texid[MAXNUM_TEX];
	texinfo ti[MAXNUM_TEX];
	int currenttex;

	float wscale,nearval,farval;

	GLenum fogtablemode;
	GLfloat fogdensity;
	GrFog_t fogtable[64];

	GrVertex gwin[VB_SIZE];
};

fxMesaContext CurrentfxMesaCtx=NULL;

/**********************************************************************/
/*****                 Miscellaneous functions                    *****/
/**********************************************************************/

/* return buffer size information */
static void buffer_size(GLcontext *ctx, GLuint *width, GLuint *height)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  int colors;

#if defined(DEBUG_FXMESA)
  printf("fxmesa: buffer_size(...)\n");
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
  printf("fxmesa: set_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
#endif

  fxMesa->color=FXCOLOR(red,green,blue,alpha);
}


/* implements glClearColor() */
static void clear_color(GLcontext *ctx, GLubyte red, GLubyte green,
                         GLubyte blue, GLubyte alpha )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
   printf("fxmesa: clear_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
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
	printf("fxmesa: clear(%d,%d,%d,%d)\n",x,y,width,height);
#endif

	grDepthMask(FXFALSE);

 	grBufferClear(fxMesa->clearc,fxMesa->cleara,GR_WDEPTHVALUE_FARTHEST);

	if(ctx->Depth.Test && ctx->Depth.Mask)
		grDepthMask(FXTRUE);
}


/*  set the buffer used in double buffering */
static GLboolean set_buffer(GLcontext *ctx, GLenum mode )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
#if defined(DEBUG_FXMESA)
  printf("fxmesa: set_buffer(%d)\n",mode);
#endif

  if(mode==GL_FRONT)
		fxMesa->currentfb=GR_BUFFER_FRONTBUFFER;
	else {
		if(mode==GL_BACK)
			fxMesa->currentfb=GR_BUFFER_BACKBUFFER;
		else 
			return GL_FALSE;
	}

	grRenderBuffer(fxMesa->currentfb);
   
  return GL_TRUE;
}

static GLboolean drawbitmap(GLcontext *ctx, GLsizei width, GLsizei height,
														GLfloat xorig, GLfloat yorig,
														GLfloat xmove, GLfloat ymove,
														const struct gl_image *bitmap )
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
	FxU16 *p;
	GrLfbInfo_t info;
  GLubyte *pb;
	int x,y;
  GLint r,g,b,a,px,py;
	short color;

#define ISCLIPPED(rx,ry) (((rx)<0) || ((rx)>fxMesa->width) || ((ry)<0) || ((ry)>fxMesa->height))
#define DRAWBIT(i) { \
	if(!ISCLIPPED(x+px,y+py)) \
		if( (*pb) & (1<<(i)) ) \
			(*p)=color; \
	p++; \
	x++; \
	if(x>=width) { \
		pb++; \
		break; \
	} \
}

	info.size=sizeof(info);
  if(!grLfbLock(GR_LFB_WRITE_ONLY,
		fxMesa->currentfb,
		GR_LFBWRITEMODE_565,
		GR_ORIGIN_LOWER_LEFT,
		FXFALSE,
		&info)) {
#ifndef FX_SILENT
		fprintf(stderr,"3Dfx Driver: error locking the linear frame buffer\n");
#endif
		return GL_TRUE;
	}

	r=(GLint) (ctx->Current.RasterColor[0] * ctx->Visual->RedScale);
	g=(GLint) (ctx->Current.RasterColor[1] * ctx->Visual->GreenScale);
	b=(GLint) (ctx->Current.RasterColor[2] * ctx->Visual->BlueScale);
	a=(GLint) (ctx->Current.RasterColor[3] * ctx->Visual->AlphaScale);
	color=(FxU16)
						( ((FxU16)0xf8 & r) <<(11-3))  |
						( ((FxU16)0xfc & g) <<(5-3+1)) |
						( ((FxU16)0xf8 & b) >> 3);

	px=(GLint)( (ctx->Current.RasterPos[0] - xorig) + 0.0F );
  py=(GLint)( (ctx->Current.RasterPos[1] - yorig) + 0.0F );
  pb=(GLubyte *)bitmap->Data;

	/* This code is a bit slow... */

	for(y=0;y<height;y++) {
		p=((FxU16 *)info.lfbPtr)+px+((y+py)*(info.strideInBytes>>1));

		if(!ISCLIPPED(0,y+py))
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
/************************************************************************/
/************************************************************************/

#define GOURAUD_ENABLED 0x1
#define TEXTURE_ENABLED 0x2
#define FOG_ENABLED 0x4
#define WBUFFER_ENABLED 0x8
#define ZBUFFER_ENABLED 0x10

#define GOURAUD(v) {\
	(v)->r = (float) (VB->Color[i][0]>>shift); \
	(v)->g = (float) (VB->Color[i][1]>>shift); \
	(v)->b = (float) (VB->Color[i][2]>>shift); \
	(v)->a = (float) (VB->Color[i][3]>>shift); }

#define WBUFFER(v) { (v)->oow = wscale/VB->Clip[i][3]; }

#define TEXTURE(v)  { \
	(v)->tmuvtx[0].sow = sscale*VB->TexCoord[i][0]*(v)->oow; \
	(v)->tmuvtx[0].tow = tscale*VB->TexCoord[i][1]*(v)->oow; \
}

#define ZBUFFER(v) { (v)->oow = 1.0/VB->Win[i][2]; }

#define NOP(v) 

#ifdef WIN32
#define SETUP( gouraud, texture, depth ) {	\
	register unsigned int i;							\
	register struct vertex_buffer *VB = ctx->VB;		\
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx; \
	register GrVertex *GVB = &fxMesa->gwin[vstart];			\
	register unsigned int shift = ctx->ColorShift;		\
	register float wscale = fxMesa->wscale; \
	register float sscale = fxMesa->ti[fxMesa->currenttex].sscale;	\
	register float tscale = fxMesa->ti[fxMesa->currenttex].tscale;	\
	unsigned int cw,ocw;								\
														\
	__asm												\
	{													\
		__asm fstcw ocw								\
		__asm fwait										\
		__asm mov eax, ocw							\
		__asm and eax, 0fffffcffh						\
		__asm mov cw, eax							\
		__asm fldcw cw								\
		__asm fwait										\
	}													\
	for ( i = vstart; i < vend ; i++, GVB++ )			\
	{													\
		GVB->x = (VB->Win[i][0] + 524288.0f) - 524288.0f;	\
		GVB->y = (VB->Win[i][1] + 524288.0f) - 524288.0f;	\
	}													\
	__asm												\
	{													\
		__asm fldcw ocw								\
		__asm fwait										\
	}													\
	GVB = &fxMesa->gwin[vstart];							\
	for ( i = vstart; i < vend ; i++, GVB++ )			\
	{													\
		gouraud(GVB);									\
		depth(GVB);										\
		texture(GVB);									\
	} \
}
#else
#define SETUP( gouraud, texture, depth )	{ \
	register unsigned int i;							\
	register struct vertex_buffer *VB = ctx->VB;		\
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx; \
	register GrVertex *GVB = &fxMesa->gwin[vstart];			\
	register unsigned int shift = ctx->ColorShift;		\
	register float wscale = fxMesa->wscale; \
	register float sscale = fxMesa->ti[fxMesa->currenttex].sscale;	\
	register float tscale = fxMesa->ti[fxMesa->currenttex].tscale;	\
									\
	for ( i = vstart; i < vend ; i++, GVB++ )			\
	{													\
		GVB->x = ((int)(VB->Win[i][0]*16.0f))/16.0f;	\
		GVB->y = ((int)(VB->Win[i][1]*16.0f))/16.0f;	\
		gouraud(GVB);									\
		depth(GVB);										\
		texture(GVB);									\
	} \
}
#endif

static void setup( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, NOP, NOP );
}
	
static void setupG( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, NOP, NOP );
}

static void setupT( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, TEXTURE, WBUFFER );
}
	
static void setupGT( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, TEXTURE, WBUFFER );
}
	
static void setupW( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, NOP, WBUFFER );
}
	
static void setupGW( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, NOP, WBUFFER );
}

static void setupZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, NOP, ZBUFFER );
}
	
static void setupGZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, NOP, ZBUFFER );
}

static void setupTZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( NOP, TEXTURE, ZBUFFER );
}
	
static void setupGTZ( GLcontext *ctx, GLuint vstart, GLuint vend )
{
	SETUP( GOURAUD, TEXTURE, ZBUFFER );
}
	
setup_func fxSetupFuncs[] = {
	setup,
	setupG,
	setupT,
	setupGT,
	setupW,
	setupGW,
	setupT,
	setupGT,
	setupW,
	setupGW,
	setupT,
	setupGT,
	setupW,
	setupGW,
	setupT,
	setupGT,
	setupZ,
	setupGZ,
	setupTZ,
	setupGTZ,
	setupZ,
	setupGZ,
	setupTZ,
	setupGTZ
};

setup_func 
choose_setup_function( GLcontext *ctx )
{
	unsigned int setupIndex = 0;

	if ( ctx->Light.ShadeModel == GL_SMOOTH && !ctx->Light.Model.TwoSide )
		setupIndex |= GOURAUD_ENABLED;
	if ( ctx->Texture.Enabled )
		setupIndex |= TEXTURE_ENABLED;
	if ( ctx->Fog.Enabled )
		setupIndex |= FOG_ENABLED;
	if ( ctx->Depth.Test ) {
	  if ( ctx->ProjectionMatrix[15] == 0.0f )
	    setupIndex |= WBUFFER_ENABLED;
	  else
	    setupIndex |= ZBUFFER_ENABLED;
	}

	return fxSetupFuncs[setupIndex];
}

#undef GOURAUD
#define GOURAUD(v) { \
	fxMesa->gwin[(v)].r = (float) (VB->Color[(v)][0]>>shift); \
	fxMesa->gwin[(v)].g = (float) (VB->Color[(v)][1]>>shift); \
	fxMesa->gwin[(v)].b = (float) (VB->Color[(v)][2]>>shift); \
	fxMesa->gwin[(v)].a = (float) (VB->Color[(v)][3]>>shift); }

static void 
fxPoint(GLcontext *ctx, GLuint first, GLuint last)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	unsigned int i;

	if(ctx->MonoPixels)
		grConstantColorValue(fxMesa->color);

	for ( i = first; i < last ; i++ )
		grDrawPoint( &fxMesa->gwin[i] );
}


points_func 
choose_points_function( GLcontext *ctx )
{
	return fxPoint;
}

static void 
fxLineSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	
	grDrawLine( &fxMesa->gwin[v1], &fxMesa->gwin[v2] );
}

static void 
fxLineSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	unsigned int shift = ctx->ColorShift;
	struct vertex_buffer *VB = ctx->VB;

	GOURAUD(v1); 
	GOURAUD(v2); 
	grDrawLine( &fxMesa->gwin[v1], &fxMesa->gwin[v2] );
}

static void 
fxLineFlat(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	GLfixed *Color = ctx->VB->Color[pv];
	unsigned int shift = ctx->ColorShift;

	grConstantColorValue(FXCOLOR( Color[0]>>shift, Color[1]>>shift,
	   Color[2]>>shift, Color[3]>>shift));
	grDrawLine( &fxMesa->gwin[v1], &fxMesa->gwin[v2] );
}

line_func 
choose_line_function( GLcontext *ctx )
{
	if ( ctx->Light.ShadeModel == GL_SMOOTH )
		if ( ctx->Light.Model.TwoSide )
			return fxLineSmoothTwoSide;
		else
			return fxLineSmooth;
	else
		return fxLineFlat;
}

/************************************************************************/
/*********************** Triangle functions *****************************/
/************************************************************************/

static void 
fxTriangleSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;

	grDrawTriangle( &fxMesa->gwin[v1], &fxMesa->gwin[v2], &fxMesa->gwin[v3] );
}

static void 
fxTriangleSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	unsigned int shift = ctx->ColorShift;
	struct vertex_buffer *VB = ctx->VB;
	
	GOURAUD(v1); 
	GOURAUD(v2); 
	GOURAUD(v3); 
	grDrawTriangle( &fxMesa->gwin[v1], &fxMesa->gwin[v2], &fxMesa->gwin[v3] );
}

static void 
fxTriangleFlat( GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	GLfixed *Color = ctx->VB->Color[pv];
	unsigned int shift = ctx->ColorShift;

	grConstantColorValue(FXCOLOR( Color[0]>>shift, Color[1]>>shift,
	   Color[2]>>shift, Color[3]>>shift));
	grDrawTriangle( &fxMesa->gwin[v1], &fxMesa->gwin[v2], &fxMesa->gwin[v3] );
}

triangle_func 
choose_triangle_function( GLcontext *ctx )
{
	if ( ctx->Light.ShadeModel == GL_SMOOTH )
		if ( ctx->Light.Model.TwoSide )
			return fxTriangleSmoothTwoSide;
		else
			return fxTriangleSmooth;
	else
		return fxTriangleFlat;
}

/************************************************************************/
/************************* Quads functions ******************************/
/************************************************************************/

static void 
fxQuadSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint v4, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	GrVertex *grv2,*grv3;

	grv2=&fxMesa->gwin[v2];
	grv3=&fxMesa->gwin[v3];

	grDrawTriangle(&fxMesa->gwin[v1], grv2, grv3);
	grDrawTriangle(grv3, grv2, &fxMesa->gwin[v4]);
}

static void 
fxQuadSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint v4, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	unsigned int shift = ctx->ColorShift;
	struct vertex_buffer *VB = ctx->VB;
	GrVertex *grv2,*grv3;

	GOURAUD(v1); 
	GOURAUD(v2); 
	GOURAUD(v3); 
	GOURAUD(v4); 

	grv2=&fxMesa->gwin[v2];
	grv3=&fxMesa->gwin[v3];
	
	grDrawTriangle(&fxMesa->gwin[v1], grv2, grv3);
	grDrawTriangle(grv3, grv2, &fxMesa->gwin[v4]);
}

static void 
fxQuadFlat(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3,GLuint v4, GLuint pv)
{
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;
	GLfixed *Color = ctx->VB->Color[pv];
	unsigned int shift = ctx->ColorShift;
	GrVertex *grv2,*grv3;

	grConstantColorValue(FXCOLOR( Color[0]>>shift, Color[1]>>shift,
	   Color[2]>>shift, Color[3]>>shift));

	grv2=&fxMesa->gwin[v2];
	grv3=&fxMesa->gwin[v3];
	
	grDrawTriangle(&fxMesa->gwin[v1], grv2, grv3);
	grDrawTriangle(grv3, grv2, &fxMesa->gwin[v4]);
}

quad_func 
choose_quad_function(GLcontext *ctx)
{
	if(ctx->Light.ShadeModel == GL_SMOOTH)
		if(ctx->Light.Model.TwoSide)
			return fxQuadSmoothTwoSide;
		else
			return fxQuadSmooth;
	else
		return fxQuadFlat;
}

/************************************************************************/
/**************** 3D depth buffer functions *****************************/
/************************************************************************/

/* this is a no-op, since the z-buffer is in hardware */
static void alloc_depth_buffer(GLcontext *ctx)
{
#if defined(DEBUG_FXMESA)
   printf("fxmesa: alloc_depth_buffer()\n");
#endif
}

static void clear_depth_buffer(GLcontext *ctx)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
	printf("fxmesa: clear_depth_buffer()\n");
#endif

	if(ctx->Depth.Test && ctx->Depth.Mask) {
		grDepthMask(FXTRUE);
		grColorMask(FXFALSE,FXFALSE);

  	/* I don't know how to convert ctx->Depth.Clear */
		grBufferClear(fxMesa->clearc,fxMesa->cleara,GR_WDEPTHVALUE_FARTHEST);

		grColorMask(FXTRUE,FXFALSE);
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
/*************************** Texture Mapping ****************************/
/************************************************************************/

static void texenv(GLcontext *ctx, GLenum pname, const GLfloat *param)
{
	/* ;) */
}

static void texparam(GLcontext *ctx, GLenum target, GLuint texObject,
					 GLenum pname, const GLfloat *params)
{
	GLenum param=(GLenum)(GLint)params[0];
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
	GrMipMapId_t *texid=fxMesa->texid;

#if defined(DEBUG_FXMESA)
	printf("fxmesa: texparam(%d)\n",texObject);
#endif

	if(target!=GL_TEXTURE_2D) {
#ifndef FX_SILENT
		fprintf(stderr,"3Dfx Driver: unsupported texture in texparam()\n");
#endif
		return;
	}

	switch(pname) {

		case GL_TEXTURE_MIN_FILTER:
			switch(param) {
			case GL_NEAREST:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,GR_MIPMAP_DISABLE,-1,-1,-1,-1,-1,GR_TEXTUREFILTER_POINT_SAMPLED,-1);
				break;
			case GL_LINEAR:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,GR_MIPMAP_DISABLE,-1,-1,-1,-1,-1,GR_TEXTUREFILTER_BILINEAR,-1);
				break;
			case GL_NEAREST_MIPMAP_NEAREST:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,GR_MIPMAP_NEAREST,-1,-1,-1,-1,-1,GR_TEXTUREFILTER_POINT_SAMPLED,-1);
				break;
			case GL_NEAREST_MIPMAP_LINEAR:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,GR_MIPMAP_NEAREST,-1,-1,-1,-1,-1,GR_TEXTUREFILTER_BILINEAR,-1);
				break;
			case GL_LINEAR_MIPMAP_LINEAR:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,GR_MIPMAP_NEAREST_DITHER,-1,-1,-1,-1,-1,GR_TEXTUREFILTER_BILINEAR,-1);
				break;
			}
			break;

		case GL_TEXTURE_MAG_FILTER:
			switch(param) {
			case GL_NEAREST:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,GR_TEXTUREFILTER_POINT_SAMPLED);
				break;
			case GL_LINEAR:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,GR_TEXTUREFILTER_BILINEAR);
				break;
			}
			break;

		case GL_TEXTURE_WRAP_S:
			switch(param) {
			case GL_CLAMP:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,-1,-1,-1,GR_TEXTURECLAMP_CLAMP,-1,-1,-1);
				break;
			case GL_REPEAT:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,-1,-1,-1,GR_TEXTURECLAMP_WRAP,-1,-1,-1);
				break;
			}
			break;

		case GL_TEXTURE_WRAP_T:
			switch(param) {
			case GL_CLAMP:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,-1,-1,-1,-1,GR_TEXTURECLAMP_CLAMP,-1,-1);
				break;
			case GL_REPEAT:
				guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,-1,-1,-1,-1,GR_TEXTURECLAMP_WRAP,-1,-1);
				break;
			}
			break;

		case GL_TEXTURE_BORDER_COLOR:
			/* TO DO */
			break;
	}

	guTexSource(texid[texObject]);
	fxMesa->currenttex=texObject;
}

static void texdel(GLcontext *ctx, GLuint texObject)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

	fxMesa->texid[texObject]=GR_NULL_MIPMAP_HANDLE;
	fxMesa->ti[texObject].levelsdefined=0;

	/* TO DO: Free the texture memory */
}

static void texbind(GLcontext *ctx, GLenum target, GLuint texObject)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

#if defined(DEBUG_FXMESA)
	printf("fxmesa: texbind(%d)\n",texObject);
#endif

	guTexSource(fxMesa->texid[texObject]);
	fxMesa->currenttex=texObject;
}

static int logbase2(int n)
{
   GLint i = 1;
   GLint log2 = 0;

   if (n<0) {
      return -1;
   }

   while ( n > i ) {
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

static int texgetinfo(int w, int h, GrLOD_t *lodlevel, GrAspectRatio_t *ar, float *sscale, float *tscale,
											int *wscale, int *hscale)
{
	static GrLOD_t lod[9]={GR_LOD_256,GR_LOD_128,GR_LOD_64,GR_LOD_32,GR_LOD_16,GR_LOD_8,GR_LOD_4,GR_LOD_2,GR_LOD_1};
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
			s=t=255.0;
			ws=hs=1;
			break;
		case 1:
			aspectratio=GR_ASPECT_2x1;
			l=lod[8-logw];
			s=255.0;
			t=127.0;
			ws=1;
			hs=1;
			break;
		case 2:
			aspectratio=GR_ASPECT_4x1;
			l=lod[8-logw];
			s=255.0;
			t=63.0;
			ws=1;
			hs=1;
			break;
		case 3:
			aspectratio=GR_ASPECT_8x1;
			l=lod[8-logw];
			s=255.0;
			t=31.0;
			ws=1;
			hs=1;
			break;
		case 4:
			aspectratio=GR_ASPECT_8x1;
			l=lod[8-logw];
			s=255.0;
			t=31.0;
			ws=1;
			hs=2;
			break;
		case 5:
			aspectratio=GR_ASPECT_8x1;
			l=lod[8-logw];
			s=255.0;
			t=31.0;
			ws=1;
			hs=4;
			break;
		case 6:
			aspectratio=GR_ASPECT_8x1;
			l=lod[8-logw];
			s=255.0;
			t=31.0;
			ws=1;
			hs=8;
			break;
		case 7:
			aspectratio=GR_ASPECT_8x1;
			l=lod[8-logw];
			s=255.0;
			t=31.0;
			ws=1;
			hs=16;
			break;
		case 8:
			aspectratio=GR_ASPECT_8x1;
			l=lod[8-logw];
			s=255.0;
			t=31.0;
			ws=1;
			hs=32;
			break;
		case -1:
			aspectratio=GR_ASPECT_1x2;
			l=lod[8-logh];
			s=127.0;
			t=255.0;
			ws=1;
			hs=1;
			break;
		case -2:
			aspectratio=GR_ASPECT_1x4;
			l=lod[8-logh];
			s=63.0;
			t=255.0;
			ws=1;
			hs=1;
			break;
		case -3:
			aspectratio=GR_ASPECT_1x8;
			l=lod[8-logh];
			s=31.0;
			t=255.0;
			ws=1;
			hs=1;
			break;
		case -4:
			aspectratio=GR_ASPECT_1x8;
			l=lod[8-logh];
			s=31.0;
			t=255.0;
			ws=2;
			hs=1;
			break;
		case -5:
			aspectratio=GR_ASPECT_1x8;
			l=lod[8-logh];
			s=31.0;
			t=255.0;
			ws=4;
			hs=1;
			break;
		case -6:
			aspectratio=GR_ASPECT_1x8;
			l=lod[8-logh];
			s=31.0;
			t=255.0;
			ws=8;
			hs=1;
			break;
		case -7:
			aspectratio=GR_ASPECT_1x8;
			l=lod[8-logh];
			s=31.0;
			t=255.0;
			ws=16;
			hs=1;
			break;
		case -8:
			aspectratio=GR_ASPECT_1x8;
			l=lod[8-logh];
			s=31.0;
			t=255.0;
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

static void texalloc(GLcontext *ctx, GLuint texObject, GLenum glformat, int w, int h)
{
	GrTextureFormat_t format;
	GrLOD_t l;
	GrAspectRatio_t aspectratio;
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
	GrMipMapId_t *texid=fxMesa->texid;
	texinfo *ti=fxMesa->ti;
	int wscale,hscale;

	texgetinfo(w,h,&l,&aspectratio,&(ti[texObject].sscale),&(ti[texObject].tscale),&wscale,&hscale);

	switch(glformat) {
		case GL_LUMINANCE:
    case GL_LUMINANCE8:
		case 1:
			format=GR_TEXFMT_INTENSITY_8;
			break;
		case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE8_ALPHA8:
		case 2:
			format=GR_TEXFMT_ALPHA_INTENSITY_88;
			break;
    case GL_ALPHA8:
		case GL_ALPHA:
			format=GR_TEXFMT_ALPHA_8;
			break;
		case GL_RGB:
    case GL_RGB8:
		case 3:
			format=GR_TEXFMT_RGB_565;
			break;
		case GL_RGBA:
		case GL_RGBA8:
		case 4:
			format=GR_TEXFMT_ARGB_4444;
			break;
		default:
			fprintf(stderr,"3Dfx Driver: wrong internalFormat in texalloc()\n");
			grGlideShutdown();
			exit(-1);
			break;
	}

	texid[texObject]=guTexAllocateMemory(GR_TMU0,GR_MIPMAPLEVELMASK_BOTH,w*wscale,h*hscale,
										format,GR_MIPMAP_NEAREST,GR_LOD_1,l,aspectratio,
										GR_TEXTURECLAMP_WRAP,GR_TEXTURECLAMP_WRAP,GR_TEXTUREFILTER_BILINEAR,
										GR_TEXTUREFILTER_BILINEAR,0.0,FXFALSE);

	if(texid[texObject]==GR_NULL_MIPMAP_HANDLE) {
		fprintf(stderr,"3Dfx Driver: out of texture memory !\n");
		grGlideShutdown();
		exit(-1);
	}

	ti[texObject].smallLOD=l; 
	ti[texObject].largeLOD=l;
	ti[texObject].levelsdefined=0;
}

static int istexsupported(GLenum target, GLint internalFormat, const struct gl_texture_image *image)
{
	int logw,logh;

	if(target!=GL_TEXTURE_2D)
		return GL_FALSE;

	switch(internalFormat) {
		case GL_LUMINANCE:
    case GL_LUMINANCE8:
		case 1:
		case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE8_ALPHA8:
		case 2:
    case GL_ALPHA8:
		case GL_ALPHA:
		case GL_RGB:
    case GL_RGB8:
		case 3:
		case GL_RGBA:
		case GL_RGBA8:
		case 4:
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

static unsigned short *texbuildimagemap(const struct gl_texture_image *image, GLint internalFormat)
{
	unsigned short *src,*srccpy;
	unsigned char r,g,b,a,l,*data,*srcb;
	int x,y,w,h,wscale,hscale,idx;

	texgetinfo(image->Width,image->Height,NULL,NULL,NULL,NULL,&wscale,&hscale);
	w=image->Width*wscale;
	h=image->Height*hscale;

	if(!(srccpy=src=(unsigned short *)malloc(sizeof(unsigned short)*w*h))) {
		fprintf(stderr,"3Dfx Driver: out of memory !\n");
		grGlideShutdown();
		exit(-1);
	}

	data=image->Data;
	switch(internalFormat) {
		case GL_ALPHA:
    case GL_ALPHA8:
    case GL_LUMINANCE:
    case GL_LUMINANCE8:
		case 1:
			srcb=(unsigned char *)src;
			for(y=0;y<h;y++)
				for(x=0;x<w;x++) {
					idx=(x/wscale+(y/hscale)*(w/wscale));
					srcb[x+y*w]=data[idx];
				}
			break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE8_ALPHA8:
		case 2:
			for(y=0;y<h;y++)
				for(x=0;x<w;x++) {
					idx=(x/wscale+(y/hscale)*(w/wscale))*2;
					l=data[idx];
					a=data[idx+1];

					src[x+y*w]=(unsigned short)
						( ((unsigned short) a) << 8) |
						( ((unsigned short) l));
				}
			break;
    case GL_RGB:
    case GL_RGB8:
		case 3:
			for(y=0;y<h;y++)
				for(x=0;x<w;x++) {
					idx=(x/wscale+(y/hscale)*(w/wscale))*3;
					r=data[idx];
					g=data[idx+1];
					b=data[idx+2];

					src[x+y*w]=(unsigned short)
						( ((unsigned short)0xf8 & r) <<(11-3))  |
						( ((unsigned short)0xfc & g) <<(5-3+1)) |
						( ((unsigned short)0xf8 & b) >> 3); 
				}
			break;
    case GL_RGBA:
    case GL_RGBA8:
		case 4:
			for(y=0;y<h;y++)
				for(x=0;x<w;x++) {
					idx=(x/wscale+(y/hscale)*(w/wscale))*4;
					r=data[idx];
					g=data[idx+1];
					b=data[idx+2];
					a=data[idx+3];

					src[x+y*w]=(unsigned short)
						( ((unsigned short)0xf0 & a) << 8) |
						( ((unsigned short)0xf0 & r) << 4) |
						  ((unsigned short)0xf0 & g)       |
						( ((unsigned short)0xf0 & b) >> 4); 
				}
			break;
		default:
			fprintf(stderr,"3Dfx Driver: wrong internalFormat in texbuildimagemap()\n");
			grGlideShutdown();
			exit(-1);
			break;
	}

	return srccpy;
}

static void teximg(GLcontext *ctx, GLenum target,
				   GLuint texObject, GLint level, GLint internalFormat,
				   const struct gl_texture_image *image)
{
	unsigned short *src,*srccpy;
	GrLOD_t lodlev;
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
	GrMipMapId_t *texid=fxMesa->texid;
	texinfo *ti=fxMesa->ti;

#if defined(DEBUG_FXMESA)
	printf("fxmesa: teximg(...,%d,%d,%d,%d...)\n",target,internalFormat,image->Width,image->Height);
#endif

	if(istexsupported(target,internalFormat,image)) {
		if(texid[texObject]==GR_NULL_MIPMAP_HANDLE)
			texalloc(ctx,texObject,image->Format,image->Width,image->Height);
		else {
			if(ti[texObject].levelsdefined & (1<<level)) {
				texdel(ctx,texObject);
				texalloc(ctx,texObject,image->Format,image->Width,image->Height);
			}
		}

		srccpy=src=texbuildimagemap(image,internalFormat);
		
		texgetinfo(image->Width,image->Height,&lodlev,NULL,NULL,NULL,NULL,NULL);

		if(lodlev>ti[texObject].smallLOD) {
			guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,lodlev,-1,-1,-1,-1,-1,-1);
			ti[texObject].smallLOD=lodlev;
		}

		if(lodlev<ti[texObject].largeLOD) {
			guTexChangeAttributes(texid[texObject],-1,-1,-1,-1,-1,lodlev,-1,-1,-1,-1,-1);
			ti[texObject].largeLOD=lodlev;
		}

		guTexDownloadMipMapLevel(texid[texObject],lodlev,&src);

		free(srccpy);

		ti[texObject].levelsdefined|=(1<<level);

		guTexSource(texid[texObject]);
		fxMesa->currenttex=texObject;
	}
#ifndef FX_SILENT
	else
		fprintf(stderr,"3Dfx Driver: unsupported texture in teximg()\n");
#endif

}

/************************************************************************/
/************************************************************************/
/************************************************************************/

static void setup_fx_units(GLcontext *ctx)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

	if(ctx->Color.BlendEnabled) {
		GrAlphaBlendFnc_t sfact,dfact;

		if(ctx->Light.ShadeModel==GL_SMOOTH)
			guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
		else
			guAlphaSource(GR_ALPHASOURCE_CC_ALPHA);

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
			sfact=GR_BLEND_ZERO;
			break;
		}

		grAlphaBlendFunction(sfact,dfact,GR_BLEND_ONE,GR_BLEND_ZERO);

		grColorMask(FXTRUE,FXFALSE);
	} else {
		grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ZERO,GR_BLEND_ONE,GR_BLEND_ZERO);
		grColorMask(FXTRUE,FXFALSE);
	}

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
		}
		grAlphaTestReferenceValue(ctx->Color.AlphaRefUbyte);
	} else
		grAlphaTestFunction(GR_CMP_ALWAYS);

	if(ctx->Texture.Enabled) {
		switch(ctx->Texture.EnvMode) {
		case GL_DECAL:
			guColorCombineFunction(GR_COLORCOMBINE_DECAL_TEXTURE);
			guTexCombineFunction(GR_TMU0,GR_TEXTURECOMBINE_DECAL);
			break;
		case GL_MODULATE:
			if(ctx->Color.BlendEnabled)
				guAlphaSource(GR_ALPHASOURCE_TEXTURE_ALPHA);

			if(ctx->Light.ShadeModel==GL_SMOOTH)
				guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
			else
				guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB);

			guTexCombineFunction(GR_TMU0,GR_TEXTURECOMBINE_DECAL);
			break;
		case GL_BLEND:
			/* TO DO */
			break;
		}
	} else {
		if(ctx->Light.ShadeModel==GL_SMOOTH)
			guColorCombineFunction(GR_COLORCOMBINE_ITRGB);
		else
			guColorCombineFunction(GR_COLORCOMBINE_CCRGB);
				 
		grTexCombineFunction(GR_TMU0,GR_TEXTURECOMBINE_ZERO);
	}

	if(ctx->Depth.Test) {
		grDepthBufferMode(GR_DEPTHBUFFER_WBUFFER); 

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
		}

		if(ctx->Depth.Mask)
			grDepthMask(FXTRUE);
		else
			grDepthMask(FXFALSE);
	} else {
		grDepthBufferFunction(GR_CMP_ALWAYS);
		grDepthMask(FXFALSE);
	}

	if(ctx->Fog.Enabled && ctx->Hint.Fog==GL_NICEST) {
		grFogMode(GR_FOG_WITH_TABLE);
		grFogColorValue FXCOLOR((unsigned int)(255 * ctx->Fog.Color[0]), 
			(unsigned int)(255*ctx->Fog.Color[1]), (unsigned int)(255*ctx->Fog.Color[2]), 
				(unsigned int)(255*ctx->Fog.Color[3]));

		if((fxMesa->fogtablemode!=ctx->Fog.Mode) ||
			(fxMesa->fogdensity!=ctx->Fog.Density)) {
                        float wscale = ((fxMesaContext)ctx->DriverCtx)->wscale;
			switch(ctx->Fog.Mode)	{
				case GL_LINEAR:
					guFogGenerateLinear(fxMesa->fogtable,
						ctx->Fog.Start / wscale,
						ctx->Fog.End / wscale);
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
	}	else
		grFogMode(GR_FOG_DISABLE);
}


static const char *renderer_string(void)
{
	return "David Bucciarelli's Voodoo driver v0.16";
}


static void setup_dd_pointers(GLcontext *ctx)
{
#if defined(DEBUG_FXMESA)
   printf("fxmesa: fx_mesa_setup_dd_pointers()\n");
#endif

   ctx->Driver.UpdateState = setup_dd_pointers;
	 
	 ctx->Driver.RendererString = renderer_string;

   ctx->Driver.NearFar = fxMesaSetNearFar;

   ctx->Driver.ClearIndex = NULL;
   ctx->Driver.ClearColor = clear_color;
   ctx->Driver.Clear = clear;

   ctx->Driver.Index = NULL;
   ctx->Driver.Color = set_color;

   ctx->Driver.SetBuffer = set_buffer;
   ctx->Driver.GetBufferSize = buffer_size;

   ctx->Driver.AllocDepthBuffer = alloc_depth_buffer;
   ctx->Driver.ClearDepthBuffer = clear_depth_buffer;

   /* acc. functions*/

	 setup_fx_units(ctx);

   ctx->Driver.PointsFunc = choose_points_function(ctx);
   ctx->Driver.LineFunc = choose_line_function(ctx);
   ctx->Driver.TriangleFunc = choose_triangle_function(ctx);
	 ctx->Driver.QuadFunc = choose_quad_function(ctx);
	 ctx->Driver.RectFunc = NULL;

	 ctx->Driver.Bitmap=drawbitmap;
	 ctx->Driver.DrawPixels=NULL;

   ctx->Driver.RasterSetup = choose_setup_function(ctx);
	 ctx->Driver.RenderVB = NULL;

   ctx->Driver.Finish=ctx->Driver.Flush=finish;

   ctx->Driver.TexEnv=texenv;
   ctx->Driver.TexImage=teximg;
   ctx->Driver.TexParameter=texparam;
   ctx->Driver.BindTexture=texbind;
   ctx->Driver.DeleteTexture=texdel;

   ctx->Driver.WriteColorSpan       = NULL;
   ctx->Driver.WriteMonocolorSpan   = NULL;
   ctx->Driver.WriteColorPixels     = NULL;
   ctx->Driver.WriteMonocolorPixels = NULL;
   ctx->Driver.WriteIndexSpan       = NULL;
   ctx->Driver.WriteMonoindexSpan   = NULL;
   ctx->Driver.WriteIndexPixels     = NULL;
   ctx->Driver.WriteMonoindexPixels = NULL;

   ctx->Driver.ReadIndexSpan = NULL;
   ctx->Driver.ReadColorSpan = NULL;
   ctx->Driver.ReadIndexPixels = NULL;
   ctx->Driver.ReadColorPixels = NULL;
}


void fxMesaSetNearFar( GLcontext *ctx, GLfloat n, GLfloat f)
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
			switch(ctx->Fog.Mode)	{
				case GL_LINEAR:
					guFogGenerateLinear(fxMesa->fogtable,
						ctx->Fog.Start/fxMesa->wscale,
						ctx->Fog.End /fxMesa->wscale); /* works ? */
					break;
				case GL_EXP:
					guFogGenerateExp(fxMesa->fogtable,ctx->Fog.Density*fxMesa->wscale);
					break;
				case GL_EXP2:
					guFogGenerateExp2(fxMesa->fogtable,ctx->Fog.Density*fxMesa->wscale);
					break;
				default: /* That should never happen */
					break; 
			}
		}
	}
}

#define NUM_RESOLUTIONS 3

static resolutions[NUM_RESOLUTIONS][3] = { 
/*	{ 320, 200, GR_RESOLUTION_320x200 },*
	{ 320, 240, GR_RESOLUTION_320x240 },
	{ 400, 256, GR_RESOLUTION_400x256 },
	{ 512, 256, GR_RESOLUTION_512x256 },*/
	{ 512, 384, GR_RESOLUTION_512x384 },
/*	{ 640, 200, GR_RESOLUTION_640x200 },
	{ 640, 350, GR_RESOLUTION_640x350 },*/
	{ 640, 400, GR_RESOLUTION_640x400 },
	{ 640, 480, GR_RESOLUTION_640x480 } /*,
	{ 800, 600, GR_RESOLUTION_800x600 },
	{ 856, 480, GR_RESOLUTION_856x480 },
	{ 960, 720, GR_RESOLUTION_960x720 }*/ };

GrScreenResolution_t bestResolution(int width, int height)
{
	int i;

	for(i=0;i<NUM_RESOLUTIONS;i++)
		if((width<=resolutions[i][0]) && (height<=resolutions[i][1]))
			return resolutions[i][2];
	
	return GR_RESOLUTION_640x480;
}

fxMesaContext fxMesaCreateBestContext(GLuint win,GLint width, GLint height)
{
	return fxMesaCreateContext(win,bestResolution(width,height),GR_REFRESH_75Hz);
}

static int cleangraphics(void)
{
	fxMesaDestroyContext(CurrentfxMesaCtx);

	return 0;
}

/*
 * Create a new FX/Mesa context and return a handle to it.
 */
fxMesaContext fxMesaCreateContext(GLuint win,GrScreenResolution_t res, GrScreenRefresh_t ref)
{
	fxMesaContext ctx;
	GrHwConfiguration hwconfig;
	int i;

#if defined(DEBUG_FXMESA)
	printf("fxmesa: fxMesaCreateContext()\n");
#endif

#ifndef FX_SILENT
	fprintf(stderr,"Mesa 3Dfx Voodoo Device Driver V0.16\nWritten by David Bucciarelli (tech.hmw@plus.it)\n");
#endif

	grGlideInit();
	if (grSstQueryHardware(&hwconfig)) {
#ifndef FX_SILENT
		char buf[80];
		grGlideGetVersion(buf);
#endif
		grSstSelect(0);

		if(hwconfig.SSTs[0].type==GR_SSTTYPE_VOODOO)
			win = 0;
		if(!grSstWinOpen((FxU32)win,res,ref,GR_COLORFORMAT_ABGR,GR_ORIGIN_LOWER_LEFT,2,1))
			return NULL;

#ifndef FX_SILENT
		fprintf(stderr,"Using Glide V%s\nNumber of boards: %d\nGlide screen size: %dx%d\n",buf,hwconfig.num_sst,grSstScreenWidth(),grSstScreenHeight());
#endif
	} else {
		printf("ERROR: no Voodoo Graphics or Voodoo Rush !\n");

		return NULL;
	}

  ctx=(fxMesaContext)malloc(sizeof(struct fx_mesa_context));
  if(!ctx)
		return NULL;

  ctx->width=grSstScreenWidth();
  ctx->height=grSstScreenHeight();
	ctx->color=FXCOLOR(255,255,255,255);
	ctx->clearc=0;
	ctx->cleara=0;
	ctx->wscale=65535.0/100.0;
	ctx->fogdensity=-HUGE_VAL;
	ctx->currenttex=0;
	ctx->currentfb=GR_BUFFER_BACKBUFFER;

	grColorMask(FXTRUE,FXFALSE);

	for(i=0;i<MAXNUM_TEX;i++)
		ctx->texid[i]=GR_NULL_MIPMAP_HANDLE;

	ctx->gl_vis = gl_create_visual(GL_TRUE,
																 GL_FALSE,
																 GL_TRUE,
																 16,   /* depth_size */
																 8,    /* stencil_size */
																 16,   /* accum_size */
																 0,
																 255.0,
																 255.0,
																 255.0,
																 255.0);

	ctx->gl_ctx = gl_create_context(ctx->gl_vis,
																	NULL,  /* share list context */
																	(void *) ctx);

	ctx->gl_buffer = gl_create_framebuffer( ctx->gl_vis );

	setup_dd_pointers(ctx->gl_ctx);
#ifdef WIN32
  onexit(cleangraphics);
#endif

	return ctx;
}


/*
 * Destroy the given FX/Mesa context.
 */
void fxMesaDestroyContext(fxMesaContext ctx)
{
#if defined(DEBUG_FXMESA)
	printf("fxmesa: fxMesaDestroyContext()\n");
#endif

	if(ctx) {
		gl_destroy_visual(ctx->gl_vis);
		gl_destroy_context(ctx->gl_ctx);
		gl_destroy_framebuffer(ctx->gl_buffer);

#ifndef FX_SILENT
		{
		GrSstPerfStats_t st;

		grSstPerfStats(&st);

		fprintf(stderr,"# pixels processed (minus buffer clears): %u\n",st.pixelsIn);
		fprintf(stderr,"# pixels not drawn due to chroma key test failure: %u\n",st.chromaFail);
		fprintf(stderr,"# pixels not drawn due to depth test failure: %u\n",st.zFuncFail);
		fprintf(stderr,"# pixels not drawn due to alpha test failure: %u\n",st.aFuncFail);
		fprintf(stderr,"# pixels drawn (including buffer clears and LFB writes): %u\n",st.pixelsOut);
		fprintf(stderr,"Free texture memory: %u bytes\n",guTexMemQueryAvail(GR_TMU0));
		}
#endif

		grGlideShutdown();

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
	printf("fxmesa: fxMesaMakeCurrent()\n");
#endif

	if(!ctx) {
		gl_make_current(NULL,NULL);
		CurrentfxMesaCtx=NULL;

		return;
	}

	CurrentfxMesaCtx=ctx;

	gl_make_current(ctx->gl_ctx,ctx->gl_buffer);

  setup_dd_pointers(ctx->gl_ctx);   
  gl_Viewport(ctx->gl_ctx,0,0,ctx->width,ctx->height);
}


/*
 * Swap front/back buffers for current context if double buffered.
 */
void fxMesaSwapBuffers(void)
{
#if defined(DEBUG_FXMESA)
  printf("fxmesa: fxMesaSwapBuffers()\n");
#endif

	grBufferSwap(1);
}

#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function(void)
{
   return 0;
}

#endif  /* FX */


