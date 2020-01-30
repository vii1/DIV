/* -*- mode: C; tab-width:8;  -*-

             fxmesa2.c - 3Dfx VooDoo vertices setup functions 
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

#define GOURAUD_ENABLED 0x1
#define TEXTURE_ENABLED 0x2
#define FOG_ENABLED     0x4
#define ZBUFFER_ENABLED 0x8

#ifdef __WIN32__

/************************************************************************/
/********* The Win32 Version of the asm GrVertex Setup code *************/
/************************************************************************/

#define RDTSC	__asm _emit 0x0f __asm _emit 0x31

#if 0
#define DATA_PROFILING char *format="Number of Cycles for the GrVertex setup: %u (%u number of GrVertex)\n"; unsigned long numcycl

#define START_PROFILING __asm { \
  RDTSC                         \							
  _asm mov numcycl,eax	        \
}

#define STOP_PROFILING __asm { \
  RDTSC		               \
  __asm mov ebx,vend	       \
  __asm sub ebx,vstart	       \
  __asm push ebx	       \
  __asm sub eax,numcycl	       \
  __asm push eax	       \
  __asm mov ecx,format	       \
  __asm push ecx	       \
  __asm call printf	       \
  __asm add esp,12             \
}
#else
#define DATA_PROFILING
#define START_PROFILING
#define STOP_PROFILING
#endif

#define NOSNAP(v) __asm {                       \
  __asm mov eax,dword ptr [edx]	                \
  __asm mov ebx,dword ptr [edx+4]               \
  __asm mov dword ptr [ecx]GrVertex.x,eax       \
  __asm mov dword ptr [ecx]GrVertex.y,ebx       \
}

#define SNAP(v) __asm {                         \
  /* Snap verticies */	                        \
  __asm fld dword ptr [edx]			\
  __asm fadd snapconst				\
  __asm fld dword ptr [edx+4]			\
  __asm fadd snapconst				\
  __asm fxch					\
  __asm fstp dword ptr [ecx]GrVertex.x		\
  __asm fstp dword ptr [ecx]GrVertex.y		\
}

#define GOURAUD(v) __asm {              \
  __asm mov ebx,vcol			\
  __asm mov eax,dword ptr [ebx]		\
  __asm add ebx,4			\
  __asm mov vcol,ebx			\
                                        \
  __asm mov byte ptr stmp1,al           \
  __asm fild stmp1		        \
  __asm mov byte ptr stmp1,ah           \
  __asm fild stmp1		        \
  __asm bswap eax                       \
  __asm mov byte ptr stmp1,ah           \
  __asm fild stmp1		        \
  __asm mov byte ptr stmp1,al           \
  __asm fild stmp1		        \
  __asm fstp dword ptr [ecx]GrVertex.a 		\
  __asm fstp dword ptr [ecx]GrVertex.b 		\
  __asm fstp dword ptr [ecx]GrVertex.g 		\
  __asm fstp dword ptr [ecx]GrVertex.r 		\
}

#define TEXTURE(v) __asm {              \
  __asm mov eax,vtex		        \
  __asm fld sscale			\
  __asm fmul dword ptr [eax]				\
  __asm fmul dword ptr [ecx]GrVertex.oow		\
  __asm fstp dword ptr [ecx]GrVertex.tmuvtx[0].sow	\
  __asm fld tscale			\
  __asm fmul dword ptr [eax+4]	        \
  __asm add eax,16			\
  __asm fmul dword ptr [ecx]GrVertex.oow		\
  __asm fstp dword ptr [ecx]GrVertex.tmuvtx[0].tow	\
  __asm mov vtex,eax		        \
}

#define WBUFFER(v) __asm {      \
  __asm mov ebx,vclip		\
  __asm fld wscale		\
  __asm fdiv dword ptr [ebx]	\
  __asm add ebx,16		\
  __asm mov vclip,ebx		\
  __asm fstp dword ptr [ecx]GrVertex.oow	\
}

#define ZBUFFER(v) __asm {                      \
  __asm	mov eax,dword ptr [edx+8]		\
  __asm mov dword ptr [ecx]GrVertex.ooz,eax	\
}

#define NOP(v) 

#define SETUP(label, snap, gouraud, texture, wdepth, zdepth) {	\
  struct vertex_buffer *VB = ctx->VB;				\
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;		\
  GrVertex *GVB = &fxMesa->gwin[vstart];			\
  GrVertex *GVBlast = &fxMesa->gwin[vend];			\
  float wscale = fxMesa->wscale; 				\
  float sscale;                                                 \
  float tscale;                                                 \
  float snapconst=(float)(3L<<18);				\
  float *vwin=VB->Win[vstart];					\
  unsigned short stmp1=0;                                       \
  unsigned char *vcol=VB->Color[vstart];		        \
  float *vclip=&VB->Clip[vstart][3];			        \
  float *vtex=VB->TexCoord[vstart];			        \
  unsigned char *vclipmask=&VB->ClipMask[vstart];               \
  DATA_PROFILING;						\
								\
  if(ctx->Texture.Current && ctx->Texture.Current->DriverData) {    \
    sscale=((texinfo *)(ctx->Texture.Current->DriverData))->sscale; \
    tscale=((texinfo *)(ctx->Texture.Current->DriverData))->tscale; \
  }                                                             \
                                                                \
  if(!VB->ClipOrMask) {                                         \
    __asm	{						\
      START_PROFILING			\
      /* eax: tmp */			\
      /* ebx: tmp */			\
      /* ecx: GVB */			\
      /* edx: vwin */			\
      __asm mov ecx,GVB			\
      __asm mov edx,vwin		\
    			                \
      __asm align 16			\
      __asm label :                     \
      __asm mov eax,vclipmask           \
      __asm mov bl,byte ptr [eax]       \
      __asm inc eax                     \
      __asm mov vclipmask,eax           \
      __asm test bl,bl                  \
      __asm jne pre_##label             \
                                        \
      snap(GVB)				\
      gouraud(GVB)			\
      wdepth(GVB)			\
      zdepth(GVB)			\
      texture(GVB)			\
    				        \
      __asm pre_##label :               \
      __asm add ecx,SIZE GrVertex	\
      __asm add edx,12			\
      __asm cmp ecx,GVBlast	        \
      __asm jne label                   \
    				        \
      STOP_PROFILING			\
    }					\
  } else {                              \
    __asm	{			\
      START_PROFILING			\
      /* eax: tmp */			\
      /* ebx: tmp */			\
      /* ecx: GVB */			\
      /* edx: vwin */			\
      __asm mov ecx,GVB			\
      __asm mov edx,vwin		\
    			                \
      __asm align 16			\
      __asm nc##label :                 \
                                        \
      snap(GVB)				\
      gouraud(GVB)			\
      wdepth(GVB)			\
      zdepth(GVB)			\
      texture(GVB)			\
    				        \
      __asm add ecx,SIZE GrVertex	\
      __asm add edx,12			\
      __asm cmp ecx,GVBlast	        \
      __asm jne nc##label               \
    				        \
      STOP_PROFILING			\
    }					\
  }                                     \
}


#else /******************************************************************/


#if defined(__linux__) && defined(__i386__)


/************************************************************************/
/*********** The Linux Version of the asm GrVertex Setup code ***********/
/************************************************************************/


#define NOSNAP(v) "      \
  movl (%%edx),%%eax;    \
  movl 4(%%edx),%%ebx;   \
  movl %%eax,(%%ecx);    \
  movl %%ebx,4(%%ecx);"

#define SNAP(v) "               \
  flds (%%edx);	        	\
  fadds %0;			\
  flds 4(%%edx);		\
  fadds %0;			\
  fxch;				\
  fstps (%%ecx);	        \
  fstps 4(%%ecx);" 

#define GOURAUD(v) "            \
  movl %2,%%ebx;                \
  movl (%%ebx),%%eax;		\
  addl $4,%%ebx;  		\
  movl %%ebx,%2;                \
                                \
  movb %%al,%3;	                \
  fildw %3;		        \
  movb %%ah,%3;	                \
  fildw %3;		        \
  bswap %%eax;		        \
  movb %%ah,%3;	                \
  fildw %3;		        \
  movb %%al,%3;	                \
  fildw %3;		        \
  fstps 28(%%ecx);		\
  fstps 20(%%ecx);		\
  fstps 16(%%ecx);		\
  fstps 12(%%ecx);"

#define TEXTURE(v) "            \
  movl %5,%%eax;	        \
  flds %6;			\
  fmuls (%%eax);		\
  fmuls 32(%%ecx);		\
  fstps 36(%%ecx);       	\
  flds %7;			\
  fmuls 4(%%eax);	        \
  addl $16,%%eax;		\
  fmuls 32(%%ecx);		\
  fstps 40(%%ecx);       	\
  mov %%eax,%5;"

#define WBUFFER(v) "            \
  movl %9,%%ebx;		\
  flds %8;		        \
  fdivs (%%ebx);                \
  addl $16,%%ebx;		\
  movl %%ebx,%9;		\
  fstps 32(%%ecx);"

#define ZBUFFER(v) "            \
  movl 8(%%edx),%%eax;		\
  movl %%eax,24(%%ecx);"

#define NOP(v) ""

#define SETUP_ASMLOOP(c)               \
  __asm__ volatile (c                  \
  : /* not correctly filled */         \
  : "m" (GVB), "m" (GVBlast), "m" (vwin), "i" (sizeof(GrVertex)) \
  : "eax", "ebx", "ecx", "edx", "cc", "memory")

#define SETUP_ASM(c)                   \
  __asm__ volatile (c                  \
  : /* not correctly filled */         \
  : "m" (snapconst), "m" (vwin),       \
    "m" (vcol), "m" (stmp1), "m" (stmp1), "m" (vtex), "m" (sscale), "m" (tscale),  \
    "m" (wscale), "m" (vclip)                                                      \
  : "eax", "ebx", "ecx", "edx", "cc", "memory")

#define SETUP_ASMCLIPTEST(c)           \
  __asm__ volatile (c                  \
  : /* not correctly filled */         \
  : "m" (vclipmask)                    \
  : "eax", "ebx", "ecx", "edx", "cc", "memory")

#define SETUP(label, snap, gouraud, texture, wdepth, zdepth) {  \
  struct vertex_buffer *VB = ctx->VB;				\
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx;		\
  GrVertex *GVB = &fxMesa->gwin[vstart];			\
  GrVertex *GVBlast = &fxMesa->gwin[vend];			\
  float wscale = fxMesa->wscale; 				\
  float sscale;                                                 \
  float tscale;                                                 \
  float snapconst=(float)(3L<<18);				\
  float *vwin=VB->Win[vstart];					\
  unsigned short stmp1=0;					\
  unsigned char *vcol=VB->Color[vstart];		        \
  float *vclip=&VB->Clip[vstart][3];			        \
  float *vtex=VB->TexCoord[vstart];			        \
  unsigned char *vclipmask=&VB->ClipMask[vstart];               \
                                                                \
  if(ctx->Texture.Current && ctx->Texture.Current->DriverData) {    \
    sscale=((texinfo *)(ctx->Texture.Current->DriverData))->sscale; \
    tscale=((texinfo *)(ctx->Texture.Current->DriverData))->tscale; \
  }                                                             \
  								\
  if(!VB->ClipOrMask) {                                         \
    /* eax: tmp */			\
    /* ebx: tmp */			\
    /* ecx: GVB */			\
    /* edx: vwin */			\
    SETUP_ASMLOOP("                     \
      movl %0,%%ecx;			\
      movl %2,%%edx;		        \
                                        \
      .align 4,0x90;		        \
      "#label"1:;");	                \
                                        \
    SETUP_ASMCLIPTEST("                 \
      movl %0,%%eax;                    \
      movb (%%eax),%%bl;                \
      inc %%eax;                        \
      movl %%eax,%0;                    \
      test %%bl,%%bl;                   \
      jne "#label"pre;");               \
                                        \
    SETUP_ASM(snap(GVB));		\
    SETUP_ASM(gouraud(GVB));		\
    SETUP_ASM(wdepth(GVB));		\
    SETUP_ASM(zdepth(GVB));		\
    SETUP_ASM(texture(GVB));		\
                                        \
    SETUP_ASMLOOP("                     \
      "#label"pre:;                     \
      addl %3,%%ecx;                    \
      addl $12,%%edx;                   \
      cmpl %1,%%ecx;                    \
      jne "#label"1;                    \
    ");                                 \
  } else {                              \
    /* eax: tmp */			\
    /* ebx: tmp */			\
    /* ecx: GVB */			\
    /* edx: vwin */			\
    SETUP_ASMLOOP("                     \
      movl %0,%%ecx;			\
      movl %2,%%edx;		        \
                                        \
      .align 4,0x90;		        \
      "#label"2:;");	                \
                                        \
    SETUP_ASM(snap(GVB));		\
    SETUP_ASM(gouraud(GVB));		\
    SETUP_ASM(wdepth(GVB));		\
    SETUP_ASM(zdepth(GVB));		\
    SETUP_ASM(texture(GVB));		\
                                        \
    SETUP_ASMLOOP("                     \
      addl %3,%%ecx;                    \
      addl $12,%%edx;                   \
      cmpl %1,%%ecx;                    \
      jne "#label"2;                    \
    ");                                 \
  }                                     \
}


#else /******************************************************************/


/************************************************************************/
/********* The generic Version of the asm GrVertex Setup code ***********/
/************************************************************************/

#define NOSNAP(v) { \
  (v)->x=VB->Win[i][0];	         \
  (v)->y=VB->Win[i][1];	         \
}

#define SNAP(v) { \
  /* trunc (x,y) to multiple of 1/16 */		        \
  (v)->x=((int)(VB->Win[i][0]*16.0f))*(1.0f/16.0f);	\
  (v)->y=((int)(VB->Win[i][1]*16.0f))*(1.0f/16.0f);	\
}


#define GOURAUD(v) { \
  (v)->r=(float) (VB->Color[i][0]); \
  (v)->g=(float) (VB->Color[i][1]); \
  (v)->b=(float) (VB->Color[i][2]); \
  (v)->a=(float) (VB->Color[i][3]); \
}

#define TEXTURE(v)  { \
  (v)->tmuvtx[0].sow=sscale*VB->TexCoord[i][0]*(v)->oow; \
  (v)->tmuvtx[0].tow=tscale*VB->TexCoord[i][1]*(v)->oow; \
}

#define WBUFFER(v) { (v)->oow=wscale/VB->Clip[i][3]; }

#define ZBUFFER(v) { (v)->ooz=VB->Win[i][2]; }

#define NOP(v) 

#define SETUP(label, snap, gouraud, texture, wdepth, zdepth) {          \
  register unsigned int i;					        \
  register struct vertex_buffer *VB = ctx->VB;			        \
  fxMesaContext fxMesa = (fxMesaContext)ctx->DriverCtx; 		\
  register GrVertex *GVB = &fxMesa->gwin[vstart];			\
  register float wscale = fxMesa->wscale; 			        \
  register float sscale;                                                \
  register float tscale;                                                \
								    \
  if(ctx->Texture.Current && ctx->Texture.Current->DriverData) {    \
    sscale=((texinfo *)(ctx->Texture.Current->DriverData))->sscale; \
    tscale=((texinfo *)(ctx->Texture.Current->DriverData))->tscale; \
  }                                                             \
                                                                \
  if(!VB->ClipOrMask) {                                         \
    for(i=vstart;i<vend;i++,GVB++) {                            \
      if(VB->ClipMask[i]==0) {                                  \
	snap(GVB);                                              \
        gouraud(GVB);                                           \
        wdepth(GVB);                                            \
        zdepth(GVB);                                            \
        texture(GVB);                                           \
     }                                                          \
   }                                                            \
  } else {                                                      \
    for(i=vstart;i<vend;i++,GVB++) {                            \
      snap(GVB);                                                \
      gouraud(GVB);                                             \
      wdepth(GVB);                                              \
      zdepth(GVB);                                              \
      texture(GVB);                                             \
   }                                                            \
 }                                                              \
}


#endif /********************************************************/
#endif


static void setup(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l1, NOSNAP, NOP, NOP, NOP, NOP);
  } else {
    SETUP(l2, SNAP, NOP, NOP, NOP, NOP);
  }
}
        
static void setupG(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l3, NOSNAP, GOURAUD, NOP, NOP, NOP);
  } else {
    SETUP(l4, SNAP, GOURAUD, NOP, NOP, NOP);
  }
}

static void setupT(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l5, NOSNAP, NOP, TEXTURE, WBUFFER, NOP);
  } else {
    SETUP(l6, SNAP, NOP, TEXTURE, WBUFFER, NOP);
  }
}

static void setupGT(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l7, NOSNAP, GOURAUD, TEXTURE, WBUFFER, NOP);
  } else {
    SETUP(l8, SNAP, GOURAUD, TEXTURE, WBUFFER, NOP);
  }
}
        
static void setupW(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l9, NOSNAP, NOP, NOP, WBUFFER, NOP);
  } else {
    SETUP(l10, SNAP, NOP, NOP, WBUFFER, NOP);
  }
}
        
static void setupGW(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l11, NOSNAP, GOURAUD, NOP, WBUFFER, NOP);
  } else {
    SETUP(l12, SNAP, GOURAUD, NOP, WBUFFER, NOP);
  }
}

static void setupZ(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l13, NOSNAP, NOP, NOP, NOP, ZBUFFER);
  } else {
    SETUP(l14, SNAP, NOP, NOP, NOP, ZBUFFER);
  }
}
  
static void setupWZ(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l13bis, NOSNAP, NOP, NOP, WBUFFER, ZBUFFER);
  } else {
    SETUP(l14bis, SNAP, NOP, NOP, WBUFFER, ZBUFFER);
  }
}
      
static void setupGZ(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l15, NOSNAP, GOURAUD, NOP, NOP, ZBUFFER);
  } else {
    SETUP(l16, SNAP, GOURAUD, NOP, NOP, ZBUFFER);
  }
}

static void setupGWZ(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l15bis, NOSNAP, GOURAUD, NOP, WBUFFER, ZBUFFER);
  } else {
    SETUP(l16bis, SNAP, GOURAUD, NOP, WBUFFER, ZBUFFER);
  }
}

static void setupTZ(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l17, NOSNAP, NOP, TEXTURE, WBUFFER, ZBUFFER);
  } else {
    SETUP(l18, SNAP, NOP, TEXTURE, WBUFFER, ZBUFFER);
  }
}
        
static void setupGTZ(GLcontext *ctx, GLuint vstart, GLuint vend)
{
  if(ctx->Primitive==GL_POINTS) {
    SETUP(l19, NOSNAP, GOURAUD, TEXTURE, WBUFFER, ZBUFFER);
  } else {
    SETUP(l20, SNAP, GOURAUD, TEXTURE, WBUFFER, ZBUFFER);
  }
}

setup_func fxSetupFuncs[]={
  setup,
  setupG,
  setupT,
  setupGT,
  setupW, /* 4 */
  setupGW,
  setupT,
  setupGT,
  setupZ, /* 8 */
  setupGZ,
  setupTZ,
  setupGTZ,
  setupWZ, /* 12 */
  setupGWZ,
  setupTZ,
  setupGTZ,
};

setup_func 
choose_setup_function(GLcontext *ctx)
{
  unsigned int setupIndex=0;

  if (ctx->Light.ShadeModel == GL_SMOOTH && !ctx->Light.Model.TwoSide)
    setupIndex |= GOURAUD_ENABLED;
  if (ctx->Texture.Enabled)
    setupIndex |= TEXTURE_ENABLED;
  if (ctx->Fog.Enabled)
    setupIndex |= FOG_ENABLED;
  if (ctx->Depth.Test)
    setupIndex |= ZBUFFER_ENABLED;

#if defined(DEBUG_FXMESA)
    fprintf(stderr,"fxmesa: vertex setup function %d (%d,%d)\n",
	    setupIndex,setupIndex & GOURAUD_ENABLED, setupIndex & TEXTURE_ENABLED);
#endif

  return fxSetupFuncs[setupIndex];
}

/************************************************************************/
/************************* Pints functions ******************************/
/************************************************************************/

#undef GOURAUD
#define GOURAUD(v) { \
  fxMesa->gwin[(v)].r=(float) VB->Color[(v)][0]; \
  fxMesa->gwin[(v)].g=(float) VB->Color[(v)][1]; \
  fxMesa->gwin[(v)].b=(float) VB->Color[(v)][2]; \
  fxMesa->gwin[(v)].a=(float) VB->Color[(v)][3]; \
}

static void 
fxPoint(GLcontext *ctx, GLuint first, GLuint last)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  struct vertex_buffer *VB=ctx->VB;
  GLuint i;

  if(ctx->VB->MonoColor) {
    if(ctx->Light.ShadeModel!=GL_SMOOTH)
      grConstantColorValue(fxMesa->color);

    if(!VB->ClipOrMask)
      for(i=first;i<=last;i++)
	grDrawPoint(&fxMesa->gwin[i]);
    else
      for(i=first;i<=last;i++)
	if(VB->ClipMask[i]==0)
	  grDrawPoint(&fxMesa->gwin[i]);
  } else {
    if(ctx->Light.ShadeModel==GL_SMOOTH) {
      if(!VB->ClipOrMask)
	for(i=first;i<=last;i++)
	  grDrawPoint(&fxMesa->gwin[i]);
      else
	for(i=first;i<=last;i++)
	  if(VB->ClipMask[i]==0)
	    grDrawPoint(&fxMesa->gwin[i]);
    } else {
      if(!VB->ClipOrMask) {
	for(i=first;i<=last;i++) {
	  grConstantColorValue(FXCOLOR(VB->Color[i][0],VB->Color[i][1],
				       VB->Color[i][2],VB->Color[i][3]));
	  grDrawPoint(&fxMesa->gwin[i]);
	}
      } else {
	for(i=first;i<=last;i++) {
	  if(VB->ClipMask[i]==0) {
	    grConstantColorValue(FXCOLOR(VB->Color[i][0],VB->Color[i][1],
					 VB->Color[i][2],VB->Color[i][3]));
	    grDrawPoint(&fxMesa->gwin[i]);
	  }
	}
      }
    }
  }
}

points_func 
choose_points_function(GLcontext *ctx)
{
  if (ctx->RasterMask & STENCIL_BIT)
    return NULL;

  return fxPoint;
}

/************************************************************************/
/************************* Lines functions ******************************/
/************************************************************************/

static void 
fxLineSmooth(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

  grDrawLine(&fxMesa->gwin[v1], &fxMesa->gwin[v2]);
}

static void 
fxLineSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  struct vertex_buffer *VB=ctx->VB;

  GOURAUD(v1); 
  GOURAUD(v2); 

  grDrawLine(&fxMesa->gwin[v1], &fxMesa->gwin[v2]);
}

static void 
fxLineFlat(GLcontext *ctx, GLuint v1, GLuint v2, GLuint pv)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLubyte *Color=ctx->VB->Color[pv];

  grConstantColorValue(FXCOLOR(Color[0], Color[1], Color[2], Color[3]));

  grDrawLine(&fxMesa->gwin[v1], &fxMesa->gwin[v2]);
}

line_func 
choose_line_function(GLcontext *ctx)
{
  if (ctx->RasterMask & STENCIL_BIT)
    return NULL;

  if (ctx->Light.ShadeModel == GL_SMOOTH)
    if (ctx->Light.Model.TwoSide)
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
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;

  grDrawTriangle(&fxMesa->gwin[v1], &fxMesa->gwin[v2], &fxMesa->gwin[v3]);
}

static void 
fxTriangleSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  struct vertex_buffer *VB=ctx->VB;

  GOURAUD(v1); 
  GOURAUD(v2); 
  GOURAUD(v3); 

  grDrawTriangle(&fxMesa->gwin[v1], &fxMesa->gwin[v2], &fxMesa->gwin[v3]);
}

static void 
fxTriangleFlat(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint pv)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLubyte *Color=ctx->VB->Color[pv];

  grConstantColorValue(FXCOLOR(Color[0], Color[1], Color[2], Color[3]));

  grDrawTriangle(&fxMesa->gwin[v1], &fxMesa->gwin[v2], &fxMesa->gwin[v3]);
}

triangle_func 
choose_triangle_function(GLcontext *ctx)
{
  if (ctx->RasterMask & STENCIL_BIT)
    return NULL;

  if (ctx->Light.ShadeModel == GL_SMOOTH)
    if (ctx->Light.Model.TwoSide)
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
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GrVertex *grv2,*grv4;

  grv2=&fxMesa->gwin[v2];
  grv4=&fxMesa->gwin[v4];

  grDrawTriangle(&fxMesa->gwin[v1], grv2, grv4);
  grDrawTriangle(grv2, &fxMesa->gwin[v3], grv4);
}

static void 
fxQuadSmoothTwoSide(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3, GLuint v4, GLuint pv)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  struct vertex_buffer *VB=ctx->VB;
  GrVertex *grv2,*grv4;

  GOURAUD(v1); 
  GOURAUD(v2); 
  GOURAUD(v3); 
  GOURAUD(v4); 

  grv2=&fxMesa->gwin[v2];
  grv4=&fxMesa->gwin[v4];

  grDrawTriangle(&fxMesa->gwin[v1], grv2, grv4);
  grDrawTriangle(grv2, &fxMesa->gwin[v3], grv4);
}

static void 
fxQuadFlat(GLcontext *ctx, GLuint v1, GLuint v2, GLuint v3,GLuint v4, GLuint pv)
{
  fxMesaContext fxMesa=(fxMesaContext)ctx->DriverCtx;
  GLubyte *Color=ctx->VB->Color[pv];
  GrVertex *grv2,*grv4;

  grConstantColorValue(FXCOLOR(Color[0], Color[1], Color[2], Color[3]));

  grv2=&fxMesa->gwin[v2];
  grv4=&fxMesa->gwin[v4];

  grDrawTriangle(&fxMesa->gwin[v1], grv2, grv4);
  grDrawTriangle(grv2, &fxMesa->gwin[v3], grv4);
}

quad_func 
choose_quad_function(GLcontext *ctx)
{
  if (ctx->RasterMask & STENCIL_BIT)
    return NULL;

  if(ctx->Light.ShadeModel == GL_SMOOTH)
    if(ctx->Light.Model.TwoSide)
      return fxQuadSmoothTwoSide;
    else
      return fxQuadSmooth;
  else
    return fxQuadFlat;
}


#else


/*
 * Need this to provide at least one external definition.
 */

int gl_fx_dummy_function2(void)
{
  return 0;
}

#endif  /* FX */
