/* -*- mode: C; tab-width:8;  -*-

             fxdrv.h - 3Dfx VooDoo vertices setup functions 
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

#ifndef FXDRV_H
#define FXDRV_H

#include <assert.h>

#include "context.h"
#include "macros.h"
#include "matrix.h"
#include "texture.h"
#include "types.h"
#include "vb.h"
#include "xform.h"

#include "GL/fxmesa.h"

#if defined(FX) && defined(__WIN32__)
#include <windows.h>
#pragma warning (disable:4273)
#else
#define APIENTRY
#define CALLBACK
#define WINGDIAPI extern
#endif

#define MAXNUM_MIPMAPLEVELS 9


#define FXCOLOR(r,g,b,a) (( ((unsigned int)(a))<<24 )|( ((unsigned int)(b))<<16 )|( ((unsigned int)(g))<<8 )|((unsigned int)r))

typedef GLboolean (*rendervb_func)(GLcontext *, GLboolean);

typedef struct freetexmemlistnode {
  struct freetexmemlistnode *next;
  FxU32 startadr, endadr;
} texmemfreenode;

typedef struct alloctexmemlistnode {
  struct alloctexmemlistnode *next;
  FxU32 startadr, endadr;
  struct gl_texture_object *tobj;
} texmemallocnode;

typedef struct {
  GLuint lasttimeused;

  texmemallocnode *tm;

  unsigned short *mipmaplevel[MAXNUM_MIPMAPLEVELS];
  GLboolean translate_mipmaplevel[MAXNUM_MIPMAPLEVELS];
  GLboolean isintexturememory;
} texmeminfo;

typedef struct {
  texmeminfo tmi;

  GLsizei width,height;
  GLint internalformat;

  GrTexInfo info;

  GrTextureFilterMode_t minfilt;
  GrTextureFilterMode_t maxfilt;

  GrTextureClampMode_t sclamp;
  GrTextureClampMode_t tclamp;

  GrMipMapMode_t mmmode;

  GLfloat sscale,tscale;
  GLint levelsdefined;

  GuTexPalette palette;

  GLboolean fixedpalette;
  GLboolean valid;
} texinfo;

typedef struct {
  GLuint swapbuffer;
  GLuint reqtexupload;
  GLuint texupload;
  GLuint memtexupload;
} fxStats;

struct fx_mesa_context {
  GrVertex gwin[VB_SIZE];

  GuTexPalette glbpalette;

  GLcontext *gl_ctx;             /* the core Mesa context */
  GLvisual *gl_vis;              /* describes the color buffer */
  GLframebuffer *gl_buffer;      /* the ancillary buffers */

  GLint width, height;           /* size of color buffer */
  GLint swapinterval;
  GLint maxpending_swapbuffers;

  GrBuffer_t currentfb;

  GrColor_t color;
  GrColor_t clearc;
  GrAlpha_t cleara;

  GLuint texbindnumber;
  texmemfreenode *tmfree;
  texmemallocnode *tmalloc;

  GLfloat wscale,nearval,farval;

  GLenum fogtablemode;
  GLfloat fogdensity;
  GrFog_t fogtable[64];

  fxStats stats;

  GLboolean double_buffer;
  GLboolean verbose;
  GLboolean globalpalette_texture;

  GrHwConfiguration hwconf;
};

typedef void (*setup_func)(GLcontext *, GLuint, GLuint);

extern void setup_fx_units(GLcontext *);

extern setup_func choose_setup_function(GLcontext *);
extern points_func choose_points_function(GLcontext *);
extern line_func choose_line_function(GLcontext *);
extern triangle_func choose_triangle_function(GLcontext *);
extern quad_func choose_quad_function(GLcontext *);
extern rendervb_func choose_rendervb_function(GLcontext *);

extern void inittmmanager(fxMesaContext);
extern void closetmmanager(fxMesaContext fxMesa);
extern void moveintexturememory(fxMesaContext, struct gl_texture_object *);
extern void moveouttexturememory(fxMesaContext, struct gl_texture_object *);
extern void freetexture(fxMesaContext, struct gl_texture_object *);
extern void reloadmipmaplevel(fxMesaContext, struct gl_texture_object *, GLint);
extern void reloadsubmipmaplevel(fxMesaContext, struct gl_texture_object *, GLint, GLint, GLint);

extern void texenv(GLcontext *, GLenum, const GLfloat *);
extern void teximg(GLcontext *, GLenum, struct gl_texture_object *,
		   GLint, GLint, const struct gl_texture_image *);
extern void texparam(GLcontext *, GLenum, struct gl_texture_object *,
                     GLenum, const GLfloat *);
extern void texbind(GLcontext *, GLenum, struct gl_texture_object *);
extern void texdel(GLcontext *, struct gl_texture_object *);
extern void texpalette(GLcontext *, struct gl_texture_object *);
extern void useglbtexpalette(GLcontext *, GLboolean);
extern void texsubimg(GLcontext *, GLenum, struct gl_texture_object *, GLint,
		      GLint, GLint, GLint, GLint, GLint, const struct gl_texture_image *);

extern int texgetinfo(int, int, GrLOD_t *, GrAspectRatio_t *, float *, float *, int *, int *);

#endif
