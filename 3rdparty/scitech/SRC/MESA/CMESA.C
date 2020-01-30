
/*
 * Linux clgd547x/Mesa interface.
 *
 * v0.1 - Peter McDermott (pmcdermo@cs.utexas.edu) 
 *
 * notes:
 *   TODO: texture mapping, stenciling, stippling, fog.  (and many more)
 *
 * bugs:
 *   see TODOs.  Try grepping the source tree for -PFM or TODO to get an
 *   idea of things that have changed
 */

#ifdef MONDELLO

#include <stdio.h>
#include <assert.h>
#include "mondello/clgd547x.h"
#include "mondello/graphics.h"
#include "mondello/cmesa.h"
#include "context.h"
#include "types.h"
#include "xform.h"

#include "macros.h"
#include "vb.h"
#include "texture.h"

/* #define DEBUG_CMESA */


struct clgd547x_mesa_context {

   struct gl_context *glctx;	/* the core Mesa context */

   GLint width, height;		/* size of color buffer */
   GLint depth;			/* bits per pixel (8,16,24 or 32) */  

   GLboolean rgb_flag;          /* rgb or ci mode */
   GLboolean db_flag;           /* double buffering avail */

   GLuint index;		/* current color index */
   GLint red,green,blue,alpha;  /* current rgb color */
   GLuint rmult, gmult, bmult, amult;

   GLuint pixel;                /* 8bpp: white */
   GLuint clearpixel;           /* 8bpp: black */

   GLuint clearr;               /* 24bpp: r,g,b triple used to clear buff */
   GLuint clearg;
   GLuint clearb;
   GLuint cleara;            
      
   unsigned long color_table[256];  /* used for dithering in 8pp -TODO: fix */
   GLubyte red_table[65536];   
   GLubyte blue_table[65536];
   GLubyte green_table[65536];
};

static clgd547xMesaContext clgd547xMesa = NULL;    /* the current context */

/* taken from xmesaP.h */

#define _R	5
#define _G	9
#define _B	5
#define _DX     4
#define _DY     4
#define _D      (_DX*_DY)

#define _MIX(r,g,b)	(((r)*_G+(g))*_B+(b)) 
#define _DITH0(C,c)	(((unsigned)((_D*(C-1)+1)*c)) >> 12) 
#define LOOKUP( R, G, B )				\
	 clgd547xMesa->color_table[_MIX(_DITH0(_R, (R)),	\
				 _DITH0(_G, (G)),	\
				 _DITH0(_B, (B)))]




static points_func choose_points_function( void );
static void NULL_points( GLuint first, GLuint last);
static void size1_rgba_points( GLuint first, GLuint last);
static void mono_size1_rgba_points( GLuint first, GLuint last);
static void size1_ci_points( GLuint first, GLuint last);
static void mono_size1_ci_points( GLuint first, GLuint last);
static void sizen_rgba_points( GLuint first, GLuint last);
static void mono_sizen_rgba_points( GLuint first, GLuint last);
static void sizen_ci_points( GLuint first, GLuint last);
static void mono_sizen_ci_points( GLuint first, GLuint last);

static line_func choose_line_function( void );
static void NULL_line( GLuint v1, GLuint v2, GLuint pv);
static void size1_rgba_smooth_line(GLuint v1, GLuint v2, GLuint pv);
static void size1_ci_smooth_line( GLuint v1, GLuint v2, GLuint pv );
static void size1_rgba_flat_line( GLuint v1, GLuint v2, GLuint pv);
static void mono_size1_rgba_flat_line( GLuint v1, GLuint v2, GLuint pv);
static void size1_ci_flat_line( GLuint v1, GLuint v2, GLuint pv );
static void mono_size1_ci_flat_line( GLuint v1, GLuint v2, GLuint pv );
static void sizen_line( GLuint v1, GLuint v2, GLuint pv );
static void mono_sizen_line( GLuint v1, GLuint v2, GLuint pv );

static polygon_func choose_polygon_function( void );
static void NULL_polygon( GLuint n, GLuint vlist[], GLuint pv);
static void rgba_smooth_polygon( GLuint n, GLuint vlist[], GLuint pv);
static void dith_smooth_polygon( GLuint n, GLuint vlist[], GLuint pv);
static void ci_smooth_polygon( GLuint n, GLuint vlist[], GLuint pv );
static void rgba_flat_polygon( GLuint n, GLuint vlist[], GLuint pv);
static void mono_rgba_flat_polygon( GLuint n, GLuint vlist[], GLuint pv);
static void dith_flat_polygon( GLuint n, GLuint vlist[], GLuint pv);
static void ci_flat_polygon( GLuint n, GLuint vlist[], GLuint pv );
static void mono_ci_flat_polygon( GLuint n, GLuint vlist[], GLuint pv );

void alloc_depth_buffer(void);
void clear_depth_buffer(void);
void read_depth_span( GLuint n, GLint x, GLint y, GLfloat depth[] );

void write_index_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                         GLuint index[], GLenum primitive );
void write_monoindex_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                             GLuint index, GLenum primitive );                             
void write_color_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                         GLubyte red[], GLubyte green[],
                         GLubyte blue[], GLubyte alpha[],
                         GLenum primitive );
void write_monocolor_span_z( GLuint n, GLint x, GLint y, GLdepth z[],                                                                                                      
                             GLint r, GLint g, GLint b, GLint a,
                             GLenum primitive );
void write_texture_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                           GLfloat s[], GLfloat t[],
                           GLubyte red[], GLubyte green[],
                           GLubyte blue[], GLubyte alpha[],
                           GLenum primitive );
 
/*
 * Convert Mesa window Y coordinate to VGA screen Y coordinate:
 */
#define FLIP(Y)  (clgd547xMesa->height-(Y)-1)

/**********************************************************************/
/*****                 Miscellaneous functions                    *****/
/**********************************************************************/

/* Taken from XMesa1.c 
 * Setup RGB rending for a window with a PseudoColor, StaticColor,
 * GrayScale or StaticGray visual.  We try to allocate a palette of 225
 * colors (5 red, 9 green, 5 blue) and dither to approximate a 24-bit RGB
 * color.  While this function was originally designed just for 8-bit
 * visuals, it has also proven to work from 4-bit up to 16-bit visuals.
 * The fact that this method works for gray scale displays depends on
 * XAllocColor allocating a good gray to approximate an RGB color.
 * Dithering code contributed by Bob Mercier.
 */
static int setup_dithered_color( clgd547xMesaContext c )
{
   int r, g, b, i,index;
   GLuint red, blue, green;

   index=0;   

   if (c->depth<4 || c->depth>16) return;
   
   assert( c->depth>=4 && c->depth<=16 );

   /* This is a hack for GLUT: pre-allocate the gray needed for pop-up menus */
   
   /* Allocate X colors and initialize color_table[], red_table[], etc */
   for (r = 0; r < _R; r++) {
      for (g = 0; g < _G; g++) {
	 for (b = 0; b < _B; b++) {
	    red   = r * 65535 / (_R-1);
	    green = g * 65535 / (_G-1);
	    blue  = b * 65535 / (_B-1);

	    i = _MIX( r, g, b );

            clgd547x_setcolorindex(index,red,green,blue);

            c->color_table[i]=index;           
	    c->red_table[index]   = r * 255 / (_R-1);
	    c->green_table[index] = g * 255 / (_G-1);
	    c->blue_table[index]  = b * 255 / (_B-1);
	    
	    index++;
	 }
      }
   }

   c->rmult = 255;
   c->gmult = 255;
   c->bmult = 255;
   c->amult = 255;
/*   c->dithered_pf = PF_DITHER;
   c->undithered_pf = PF_LOOKUP; */
   c->pixel = c->color_table[_MIX(_R-1,_G-1,_B-1)];  /* white */
   c->clearpixel = c->color_table[_MIX(0,0,0)];      /* black */

   return 1;
}

/*
 * return buffer size information 
 */
static void buffer_size( GLuint *width, GLuint *height, GLuint *depth )
{
   int colors;

#ifdef DEBUG_CMESA
   printf("cmesa: buffer_size(...)\n");
#endif

   *width = clgd547xMesa->width = clgd547x_getxdim();
   *height = clgd547xMesa->height = clgd547x_getydim();
   *depth = clgd547xMesa->depth;
}

/* Set current color index */
static void set_index( GLuint index )
{
#ifdef DEBUG_CMESA
   printf("cmesa: set_index(%d)\n",index);
#endif

   clgd547xMesa->index = index;
}


/* Set current drawing color */
static void set_color( GLubyte red, GLubyte green,
                       GLubyte blue, GLubyte alpha )
{
#ifdef DEBUG_CMESA
   printf("cmesa: set_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
#endif

   clgd547xMesa->red = red;
   clgd547xMesa->green = green;
   clgd547xMesa->blue = blue;
}

/* implements glClearIndex() */
static void clear_index( GLuint index )
{
#ifdef DEBUG_CMESA
   printf("cmesa: clear_index(%d)\n",index);
#endif
}


/* implements glClearColor() */
static void clear_color( GLubyte red, GLubyte green,
                         GLubyte blue, GLubyte alpha )
{
#ifdef DEBUG_CMESA
   printf("cmesa: clear_color(%d,%d,%d,%d)\n",red,green,blue,alpha);
#endif
 
   clgd547xMesa->clearr=red;
   clgd547xMesa->clearg=green;
   clgd547xMesa->clearb=blue;
   clgd547xMesa->cleara=alpha;
   /* note: clgd5470 doesn't maintain a separate alpha buffer */
}


static void clear( GLboolean all,
                   GLint x, GLint y, GLint width, GLint height )
{

#ifdef DEBUG_CMESA
   printf("cmesa: clear(%d,%d,%d,%d)\n",x,y,width,height);
#endif
   
   if (clgd547xMesa->depth==8)  {
     clear8c(clgd547xMesa->clearpixel);
   }
   else {
     clear24c(clgd547xMesa->clearr,clgd547xMesa->clearg,
                      clgd547xMesa->clearb,clgd547xMesa->cleara);
   }
}


static GLboolean index_mask( GLuint mask )
{
  /* TODO */
#ifdef DEBUG_CMESA
   printf("cmesa: index_mask(%d)\n",mask);
#endif

   return GL_FALSE;
}


static GLboolean color_mask( GLboolean rmask, GLboolean gmask,
                             GLboolean bmask, GLboolean amask)
{
  /* TODO */
#ifdef DEBUG_CMESA
   printf("cmesa: color_mask(%d,%d,%d,%d)\n",rmask,gmask,bmask,amask);
#endif

   return GL_FALSE;
}

/*
 * note: logical operations are enabled via glEnable(GL_LOGIC_OP) and
 *       are disabled via glDisable(GL_LOGIC_OP);
 */  
static GLboolean logicop( GLenum op )
{
   uint mfield;

#ifdef DEBUG_CMESA
   printf("cmesa: logicop(%d)\n",op);
#endif

#ifdef FALSE_X
   switch (op) {
      case GL_CLEAR:		mfield = ALL_ZEROS;	break; /* 0 */
      case GL_SET:		mfield = ALL_ONES;	break; /* 1 */
      case GL_COPY:		mfield = A_ONLY;	break; /* s */
      case GL_COPY_INVERTED:	mfield = ABAR;		break; /* !s */
      case GL_NOOP:		mfield = B_ONLY;	break; /* d */
      case GL_INVERT:		mfield = BBAR;		break; /* !d */
      case GL_AND:		mfield = A_AND_B;	break; /* s&d */
      case GL_NAND:		mfield = A_NAND_B;	break; /* !(s&d) */
      case GL_OR:		mfield = A_OR_B;	break; /* s|d */
      case GL_NOR:		mfield = A_NOR_B;	break; /* !(s|d) */
      case GL_XOR:		mfield = A_XOR_B;	break; /* s^d */
      case GL_EQUIV:		mfield = A_XNOR_B;	break; /* !(s^d) */
      case GL_AND_REVERSE:	mfield = A_AND_BBAR;	break; /* s&!d */
      case GL_AND_INVERTED:	mfield = ABAR_AND_B;	break; /* !s&d */
      case GL_OR_REVERSE:	mfield = A_OR_BBAR;	break; /* s|!d */
      case GL_OR_INVERTED:	mfield = ABAR_OR_B;	break; /* !s|d */
      default:  return GL_FALSE;
   }

   clgd547x_logicop(mfield);
#endif

   return GL_TRUE;
}


static void dither( GLboolean enable )
{
  /* TODO */

#ifdef DEBUG_CMESA
   printf("cmesa: dither\n");
#endif

   /* no-op */
}

void set_depth_func(GLenum func) 
{
  /* Mondello's depth buffer is broken, and wasn't fixed for the revision 
   * I (PFM) have.  It's possible to work around it, but not fun.  The 
   * default mode works fine.  See mondello/graphics.c, #define FN(z) for
   * an idea of what's taken to fix it 
   */
  
#ifdef DEBUG_CMESA
   printf("cmesa: set_depth_func\n");
#endif
 
  switch (func) {
  case GL_NEVER:
  case GL_LESS:    /* (default) pass if incoming z < stored z */
  case GL_GEQUAL:
  case GL_LEQUAL:
  case GL_GREATER:
  case GL_NOTEQUAL:
  case GL_EQUAL:
  case GL_ALWAYS:     
     CC.NewState |= NEW_RASTER_OPS;
     break;
  default:
     gl_error( GL_INVALID_ENUM, "glDepth.Func" );
  }
}

/* 
 * set the buffer used in double buffering 
 */
static GLboolean set_buffer( GLenum mode )
{
#ifdef DEBUG_CMESA
   printf("cmesa: set_buffer(%d)\n",mode);
#endif

   if (!clgd547xMesa->db_flag) {
     return GL_FALSE;
   }
   
   if (mode==GL_FRONT) {
     clgd547x_setBuffer(0);
   }
   else if (mode==GL_BACK) {
       clgd547x_setBuffer(1);
   }
   else {
     return GL_FALSE;
   }
   
   return GL_TRUE;
}


/**********************************************************************/
/*****               Rasterization                                *****/
/**********************************************************************/

/*********************  points  ***************************************/
/* 
   pv is the "provoking vertex".  This is the vertex that determines
   the color of flat lines/polygons 
*/

#define POINT_CLIP(x,y) if (x<0 || x>>CC.BufferWidth ||         \
                            y<0 || y>>CC.BufferWidth) return;

static points_func choose_points_function( void )
{
   
#ifdef DEBUG_CMESA
   printf("cmesa: choose_points_function()\n");
#endif

   if (CC.RenderMode!=GL_RENDER) {  
     /* either in selection or feedback mode */
     return NULL;
   }

   if (CC.Texture.Enabled) {
     return NULL_points;			/* no texturing yet */
   }
   
   if (CC.Point.Size==1.0) {
      /* non-texture mapped, size=1 */
      if (CC.RGBAflag) 
        return CC.MonoPixels ? mono_size1_rgba_points : size1_rgba_points;
      else 
        return CC.MonoPixels ? mono_size1_ci_points : size1_ci_points;
   }
   else {
      /* non-texture mapped, any size */
      if (CC.RGBAflag)
        return CC.MonoPixels ? mono_sizen_rgba_points : sizen_rgba_points;
      else
        return CC.MonoPixels ? mono_sizen_ci_points : sizen_ci_points;
   }     
}

/*
 * This function is called to skip drawing points the driver
 * doesn't know how to handle (i.e. texture mapped points)
 */ 
static void NULL_points( GLuint first, GLuint last)
{
  return;
}

static void mono_size1_rgba_points( GLuint first, GLuint last)
{
   GLuint i;
   GLint shift = CC.ColorShift;
   
#ifdef DEBUG_CMESA
   printf("cmesa: mono_size1_rgba_points(...)\n");
#endif

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint red, green, blue, alpha;

         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         point24_3c( x, FLIP(y), z, 
         	     clgd547xMesa->red>>shift,clgd547xMesa->green>>shift,
         	     clgd547xMesa->blue>>shift,clgd547xMesa->alpha>>shift);
      }
   }
}

static void size1_rgba_points( GLuint first, GLuint last)
{
   GLuint i;
   GLint shift = CC.ColorShift;

#ifdef DEBUG_CMESA
   printf("cmesa: size1_rgba_points(...)\n");
#endif

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint red, green, blue, alpha;

         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         red   = VB.Color[i][0] >> shift;
         green = VB.Color[i][1] >> shift;
         blue  = VB.Color[i][2] >> shift;
         alpha = VB.Color[i][3] >> shift;

         point24_3c( x, FLIP(y), z, red, green, blue, alpha );
      }
   }
}

static void mono_size1_ci_points( GLuint first, GLuint last)
{
   GLuint i;
   
#ifdef DEBUG_CMESA
   printf("cmesa: mono_size1_ci_points(...)\n");
#endif

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLfloat r,g,b;
         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);
         point8_3c( x, FLIP(y), z, clgd547xMesa->index );
      }
   }
}

static void size1_ci_points( GLuint first, GLuint last)
{
   GLuint i;
   
#ifdef DEBUG_CMESA
   printf("cmesa: size1_ci_points(...)\n");
#endif

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLfloat r,g,b;
         x = (GLint)  VB.Win[i][0];
         y = (GLint)  VB.Win[i][1];
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);
         r = VB.Color[i][0];
         g = VB.Color[i][1];
         b = VB.Color[i][2];
         point8_3c( x, FLIP(y), z, LOOKUP(r,g,b) );
      }
   }
}

static void mono_sizen_rgba_points( GLuint first, GLuint last)
{
   GLuint i;
   GLint isize;

#ifdef DEBUG_CMESA
   printf("cmesa: mono_sizen_rgba_points(...)\n");
#endif

   isize = (GLint) (CLAMP(CC.Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB.Win[i][0];
         y = FLIP((GLint)  VB.Win[i][1]);
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }
         
         clearArea24_3c( x0, y0, x1, y1, z,
                         clgd547xMesa->red,
                         clgd547xMesa->green,
                         clgd547xMesa->blue,
                         clgd547xMesa->alpha);
      }
   }
}

static void sizen_rgba_points( GLuint first, GLuint last)
{
   GLuint i;
   GLint isize;
   GLint shift = CC.ColorShift;

#ifdef DEBUG_CMESA
   printf("cmesa: sizen_rgba_points(...)\n");
#endif

   isize = (GLint) (CLAMP(CC.Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB.Win[i][0];
         y = FLIP((GLint)  VB.Win[i][1]);
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }
         
         clearArea24_3c( x0, y0, x1, y1, z,
                        VB.Color[i][0] >> shift,
                        VB.Color[i][1] >> shift,
                        VB.Color[i][2] >> shift,
                        VB.Color[i][3] >> shift );
      }
   }
}

static void mono_sizen_ci_points( GLuint first, GLuint last)
{
   GLuint i;
   GLint isize;

#ifdef DEBUG_CMESA
   printf("cmesa: mono_sizen_ci_points(...)\n");
#endif

   isize = (GLint) (CLAMP(CC.Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB.Win[i][0];
         y = FLIP((GLint)  VB.Win[i][1]);
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }
         
         clearArea8_3c( x0, y0, x1, y1, z, clgd547xMesa->index);
      }
   }
}

static void sizen_ci_points( GLuint first, GLuint last)
{
   GLuint i;
   GLint isize;
   GLint shift = CC.ColorShift;

#ifdef DEBUG_CMESA
   printf("cmesa: sizen_ci_points(...)\n");
#endif

   isize = (GLint) (CLAMP(CC.Point.Size,MIN_POINT_SIZE,MAX_POINT_SIZE) + 0.5F);

   for (i=first;i<=last;i++) {
      if (VB.Unclipped[i]) {
         GLint x, y, z;
         GLint x0, x1, y0, y1;
         GLint ix, iy;

         x = (GLint)  VB.Win[i][0];
         y = FLIP((GLint)  VB.Win[i][1]);
         z = (GLint) (VB.Win[i][2] * DEPTH_SCALE);

         if (isize&1) {
            /* odd size */
            x0 = x - isize/2;
            x1 = x + isize/2;
            y0 = y - isize/2;
            y1 = y + isize/2;
         }
         else {
            /* even size */
            x0 = (GLint) (x + 0.5F) - isize/2;
            x1 = x0 + isize-1;
            y0 = (GLint) (y + 0.5F) - isize/2;
            y1 = y0 + isize-1;
         }
         
         clearArea8_3c( x0, y0, x1, y1, z, VB.Index[i]);
      }
   }
}

/*********************  lines  ***************************************/
static line_func choose_line_function( void )
{
   line_func t;
   
#ifdef DEBUG_CMESA
   printf("cmesa: choose_line_function()\n");
#endif

   if (CC.RenderMode!=GL_RENDER) {  
     /* either in selection or feedback mode */
     return NULL;
   }

   if (CC.Texture.Enabled) {
     return NULL_line;			/* texturing isn't supported */
   }

   /* non-texture mapped lines */
   if (CC.Line.SmoothFlag) {
       /* smooth shaded */ 
       t=CC.RGBAflag ? size1_rgba_smooth_line : size1_ci_smooth_line;
    }
    else {
       /* flat shaded */
       if (CC.RGBAflag) 
         t=CC.MonoPixels ? mono_size1_rgba_flat_line : size1_rgba_flat_line;
       else
         t=CC.MonoPixels ? mono_size1_ci_flat_line : size1_ci_flat_line;
    }
      
    if (CC.Line.Width!=1.0) {
       CC.LineFunc2=t;  /* flat/smooth line func called inside sizen_line */
       return sizen_line;
    }
    return t;
}

/* 
 * this function is called for lines that aren't supported by the
 * driver (i.e. texturing)
 */
static void NULL_line(GLuint v1, GLuint v2, GLuint pv)
{
  return;
}

static void size1_rgba_smooth_line(GLuint v1, GLuint v2, GLuint pv)
{

#ifdef DEBUG_CMESA
   printf("cmesa: size1_rgba_smooth_line(...)\n");
#endif

  line24_3s((uint)VB.Win[v1][0],FLIP((uint)VB.Win[v1][1]),(uint)(VB.Win[v1][2]*DEPTH_SCALE),
            (uint)VB.Color[v1][0],(uint)VB.Color[v1][1],(uint)VB.Color[v1][2],(uint)VB.Color[v1][3],
            (uint)VB.Win[v2][0],FLIP((uint)VB.Win[v2][1]),(uint)(VB.Win[v2][2]*DEPTH_SCALE),
            (uint)VB.Color[v2][0],(uint)VB.Color[v2][1],(uint)VB.Color[v2][2],(uint)VB.Color[v2][3]);
  
}

static void size1_ci_smooth_line( GLuint v1, GLuint v2, GLuint pv )
{

#ifdef DEBUG_CMESA
   printf("cmesa: size1_ci_smooth_line(...)\n");
#endif

  line8_3s((uint)VB.Win[v1][0],FLIP((uint)VB.Win[v1][1]),
           (uint)(VB.Win[v1][2]*DEPTH_SCALE),(uint)VB.Index[v1],
           (uint)VB.Win[v2][0],FLIP((uint)VB.Win[v2][1]),
           (uint)(VB.Win[v2][2]*DEPTH_SCALE),(uint)VB.Index[v2]);

}

static void mono_size1_rgba_flat_line( GLuint v1, GLuint v2, GLuint pv)
{

#ifdef DEBUG_CMESA
   printf("cmesa: mono_size1_rgba_flat_line(...)\n");
#endif

  line24_3fc((uint)VB.Win[v1][0],FLIP((uint)VB.Win[v1][1]),(uint)(VB.Win[v1][2]*DEPTH_SCALE),
             (uint)VB.Win[v2][0],FLIP((uint)VB.Win[v2][1]),(uint)(VB.Win[v2][2]*DEPTH_SCALE),
             clgd547xMesa->red,clgd547xMesa->green,
             clgd547xMesa->blue,clgd547xMesa->alpha);

}

static void size1_rgba_flat_line( GLuint v1, GLuint v2, GLuint pv)
{

#ifdef DEBUG_CMESA
   printf("cmesa: size1_rgba_flat_line(...)\n");
#endif

  line24_3fc((uint)VB.Win[v1][0],FLIP((uint)VB.Win[v1][1]),(uint)(VB.Win[v1][2]*DEPTH_SCALE),
             (uint)VB.Win[v2][0],FLIP((uint)VB.Win[v2][1]),(uint)(VB.Win[v2][2]*DEPTH_SCALE),
             (uint)VB.Color[pv][0],(uint)VB.Color[pv][1],
             (uint)VB.Color[pv][2],(uint)VB.Color[pv][3]);

}

static void mono_size1_ci_flat_line( GLuint v1, GLuint v2, GLuint pv )
{

#ifdef DEBUG_CMESA
   printf("cmesa: mono_size1_ci_flat_line(...)\n");
#endif

  line8_3fc((uint)VB.Win[v1][0],FLIP((uint)VB.Win[v1][1]),(uint)(VB.Win[v1][2]*DEPTH_SCALE),
            (uint)VB.Win[v2][0],FLIP((uint)VB.Win[v2][1]),(uint)(VB.Win[v2][2]*DEPTH_SCALE),
            clgd547xMesa->index);

}

static void size1_ci_flat_line( GLuint v1, GLuint v2, GLuint pv )
{

#ifdef DEBUG_CMESA
   printf("cmesa: size1_ci_flat_line(...)\n");
#endif

  line8_3fc((uint)VB.Win[v1][0],FLIP((uint)VB.Win[v1][1]),(uint)(VB.Win[v1][2]*DEPTH_SCALE),
            (uint)VB.Win[v2][0],FLIP((uint)VB.Win[v2][1]),(uint)(VB.Win[v2][2]*DEPTH_SCALE),
            (uint)VB.Index[pv]);

}

static void sizen_line(GLuint v1, GLuint v2, GLuint pv)
{
   GLint width, w0, wi, s1, s2;
   GLint x1, y1, x2, y2;
   
#ifdef DEBUG_CMESA
   printf("cmesa: sizen_line(...)\n");
#endif

   x1=VB.Win[v1][0];
   y1=VB.Win[v1][1];
   x2=VB.Win[v2][0];
   y2=VB.Win[v2][1];

   /* calculate width */
   width = (GLint) CLAMP( CC.Line.Width, MIN_LINE_WIDTH, MAX_LINE_WIDTH );
   w0 = -width / 2;

   if ( ABSI(y2-y1) < ABSI(x2-x1) ) {  /* dx < dy */
      /* Y-major: duplicate pixels in X direction */

     /* should the vertices be invariant? -PFM */
     VB.Win[v1][0]-=w0;
     VB.Win[v2][0]-=w0;
     for (wi=0;wi<width;wi++) {
        CC.LineFunc2(v1,v2,pv); 
        VB.Win[v1][0]++;        
        VB.Win[v2][0]++;
     }
     
     VB.Win[v1][0]=x1;
     VB.Win[v2][0]=x2;
   }
   else { /* dx > dy */
     /* X-major: duplicate pixels in Y direction */
     /* should the verices be invariant? -PFM */

     VB.Win[v1][1]-=w0;
     VB.Win[v2][1]-=w0;
     for (wi=0;wi<width;wi++) {
        CC.LineFunc2(v1,v2,pv); 
        VB.Win[v1][1]++;        
        VB.Win[v2][1]++;
     }
     
     VB.Win[v1][1]=y1;
     VB.Win[v2][1]=y2;
   }
}

/*********************  polygons  ***************************************/
static polygon_func choose_polygon_function( void )
{

#ifdef DEBUG_CMESA
   printf("cmesa: choose_polygon_function(...)\n");
#endif

   if (CC.RenderMode!=GL_RENDER) {  
     /* either in selection or feedback mode */
     return NULL;
   }
   
   if (CC.Texture.Enabled) {
     return NULL_polygon;		/* texturing isn't supported */
   }

   if (CC.Light.ShadeModel==GL_SMOOTH) {
      /* smooth shaded, no texturing */
     if (clgd547xMesa->depth==8) 
       return CC.RGBAflag ? dith_smooth_polygon : ci_smooth_polygon;
     else
       return rgba_smooth_polygon;
   }
   else {
     /* flat shaded, no texturing */
     if (clgd547xMesa->depth==8) {
       if (CC.RGBAflag)               /* 8bpp dithered */
         return dith_flat_polygon;
       else
         return CC.MonoPixels ? mono_ci_flat_polygon : ci_flat_polygon;
     }
     else {
       return CC.MonoPixels ? mono_rgba_flat_polygon : rgba_flat_polygon;
     }
   }
}

/* 
 * this function is called for polygons we don't support (i.e. textures)
 */
static void NULL_polygon( GLuint n, GLuint vlist[], GLuint pv)
{
  return;
}

static void rgba_smooth_polygon( GLuint n, GLuint vlist[], GLuint pv)
{
   int v;

#ifdef DEBUG_CMESA
   printf("cmesa: rgba_smooth_polygon(...)\n");
#endif

   if (n<3) 
     return;
     
   for(v=0; v <= n-3; v++) {
      #define v0 vlist[0]
      #define v1 vlist[v+1]
      #define v2 vlist[v+2]
      
      triangle24_3sv(v0,v1,v2);
   }
}

static void dith_smooth_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
  int v;
  
  #define SHIFT CC.ColorShift
  
#ifdef DEBUG_CMESA
   printf("cmesa: dith_smooth_polygon()\n");
#endif

   if (n<3) 
     return;
     
   for(v=0; v <= n-3; v++) {      
      triangle8_3sv(v0,v1,v2);
   }
}

static void ci_smooth_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
  int v;

#ifdef DEBUG_CMESA
   printf("cmesa: ci_smooth_polygon(%d,%d,%d)\n",
          VB.Index[v0],VB.Index[v1],VB.Index[v2]);
#endif

   if (n<3) 
     return;
     
   for(v=0; v <= n-3; v++) {      
      triangle8_3sv(v0,v1,v2);
   }
}

static void mono_rgba_flat_polygon( GLuint n, GLuint vlist[], GLuint pv)
{
  int v;
  
#ifdef DEBUG_CMESA
   printf("cmesa: mono_rgba_flat_polygon(...)\n");
#endif

   for(v=0; v <= n-3; v++) {      
      triangle24_3fcv(v0,v1,v2,pv);
   }
}

static void rgba_flat_polygon( GLuint n, GLuint vlist[], GLuint pv)
{
  int v;
  
#ifdef DEBUG_CMESA
   printf("cmesa: rgba_flat_polygon(...)\n");
#endif

   for(v=0; v <= n-3; v++) {      
      triangle24_3fcv(v0,v1,v2,pv);      
   }
}

static void dith_flat_polygon( GLuint n, GLuint vlist[], GLuint pv)
{
  int v;
  
#ifdef DEBUG_CMESA
   printf("cmesa: dith_flat_polygon(...)\n");
#endif

   for(v=0; v <= n-3; v++) {      
      triangle8_3fcv(v0,v1,v2,pv);
   }
}

static void mono_ci_flat_polygon(GLuint n, GLuint vlist[], GLuint pv )
{
   int v;

#ifdef DEBUG_CMESA
   printf("cmesa: mono_ci_flat_polygon(%d,%d,%d)\n",VB.Color[pv][0],VB.Color[pv][1],VB.Color[pv][2]);
#endif   

   for(v=0; v <= n-3; v++) {      
      triangle8_3fcv(v0,v1,v2,pv);
   }
}


static void ci_flat_polygon( GLuint n, GLuint vlist[], GLuint pv )
{
   int v;

#ifdef DEBUG_CMESA
   printf("cmesa: ci_flat_polygon(%d,%d,%d)\n",VB.Color[pv][0],VB.Color[pv][1],VB.Color[pv][2]);
#endif   

   for(v=0; v <= n-3; v++) {      
      triangle8_3fcv(v0,v1,v2,pv);
   }
}

/**************** 3D depth buffer functions *******************************/

/* this is a no-op, since the z-buffer is in hardware */
void alloc_depth_buffer( void )
{
#ifdef DEBUG_CMESA
   printf("cmesa: alloc_depth_buffer()\n");
#endif
}

void read_depth_span( GLuint n, GLint x, GLint y, GLfloat depth[] )
{
#ifdef DEBUG_CMESA
   printf("cmesa: read_depth_span()\n");
#endif
}

/***************** 3D span functions **************************************/ 

void write_index_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                         GLuint index[], GLenum primitive )
{
#ifdef DEBUG_CMESA
   printf("cmesa: write_index_span_z()\n");
#endif
}

void write_monoindex_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                             GLuint index, GLenum primitive )
{
#ifdef DEBUG_CMESA
   printf("cmesa: write_monoindex_span_z()\n");
#endif
}
                             
void write_color_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                         GLubyte red[], GLubyte green[],
                         GLubyte blue[], GLubyte alpha[],
                         GLenum primitive )
{
#ifdef DEBUG_CMESA
   printf("cmesa: write_color_span_z()\n");
#endif
}                         
                         
void write_monocolor_span_z( GLuint n, GLint x, GLint y, GLdepth z[],                                                                                                      
                             GLint r, GLint g, GLint b, GLint a,
                             GLenum primitive )
{
#ifdef DEBUG_CMESA
   printf("cmesa: write_monocolor_span_z()\n");
#endif
}
             
void write_texture_span_z( GLuint n, GLint x, GLint y, GLdepth z[],
                           GLfloat s[], GLfloat t[],
                           GLubyte red[], GLubyte green[],
                           GLubyte blue[], GLubyte alpha[],
                           GLenum primitive )
{
#ifdef DEBUG_CMESA
   printf("cmesa: write_texture_span_z(...)\n");
#endif
}

/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/


static void write_index_span( GLuint n, GLint x, GLint y,
                              const GLuint index[],
                              const GLubyte mask[] )
{
   int i;


#ifdef DEBUG_CMESA
   printf("cmesa: write_index_span(...)\n");
#endif

   y = FLIP(y);
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
/*         clgd547x_setcolor( index[i] );
         clgd547x_drawpixel( x, y );
 */
      }
   }
}

static void write_monoindex_span(GLuint n,GLint x,GLint y,const GLubyte mask[])
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: write_monoindex_span(...)\n");
#endif

   y = FLIP(y);
   /* use current color index */
   for (i=0;i<n;i++,x++) {
      if (mask[i]) {
/*         clgd547x_drawpixel( x, y, clgd547xMesa->index ); */
      }
  
   }
}



static void write_color_span( GLuint n, GLint x, GLint y,
                              const GLubyte red[], const GLubyte green[],
                              const GLubyte blue[], const GLubyte alpha[],
                              const GLubyte mask[] )
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: write_color_span(...)\n");
#endif

   y=FLIP(y);
   if (mask) {
      /* draw some pixels */
      for (i=0; i<n; i++, x++) {
         if (mask[i]) {
/*         clgd547x_drawpixel( x, y, red[i], green[i], blue[i]);
 */
         }
      }
   }
   else {
      /* draw all pixels */
      for (i=0; i<n; i++, x++) {
/*      clgd547x_drawpixel( x, y, red[i], green[i], blue[i]);
 */
      }
   }
}



static void write_monocolor_span( GLuint n, GLint x, GLint y,
                                  const GLubyte mask[])
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: write_monocolor_span(...)\n");
#endif

   y=FLIP(y);
   /* use current rgb color */
   for (i=0; i<n; i++, x++) {
/*      if (mask[i]) {
         clgd547x_drawpixel( x, y, 
           clgd547xMesa->red, clgd547xMesa->green,clgd547xMesa->blue );
      }
 */
   }
}



/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/


static void read_index_span( GLuint n, GLint x, GLint y, GLuint index[])
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: read_index_span()\n");
#endif

   y = FLIP(y);
   for (i=0; i<n; i++,x++) {
/*      index[i] = clgd547x_getpixel( x, y ); */
   }
}



static void read_color_span( GLuint n, GLint x, GLint y,
                             GLubyte red[], GLubyte green[],
                             GLubyte blue[], GLubyte alpha[] )
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: read_color_span()\n");
#endif

   for (i=0; i<n; i++, x++) {
      /* TODO */
   }
}



/**********************************************************************/
/*****                  Write arrays of pixels                    *****/
/**********************************************************************/


static void write_index_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLuint index[], const GLubyte mask[] )
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: write_index_pixel(...)\n");
#endif

   for (i=0; i<n; i++) {
      if (mask[i]) {
/*      clgd547x_drawpixel( x[i], FLIP(y[i]), index[i] );
 */
      }
   }
}



static void write_monoindex_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: write_monoindex_pixels(...)\n");
#endif

   /* use current color index */
   for (i=0; i<n; i++) {
/*      if (mask[i]) {
         clgd547x_drawpixel( x[i], FLIP(y[i]), clgd547xMesa->index );
      }
 */
   }
}



static void write_color_pixels( GLuint n, const GLint x[], const GLint y[],
                                const GLubyte r[], const GLubyte g[],
                                const GLubyte b[], const GLubyte a[],
                                const GLubyte mask[] )
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: write_color_pixels(...)\n");
#endif

   for (i=0; i<n; i++) {
/*      if (mask[i]) {
         clgd547x_drawpixel( x[i], FLIP(y[i]), r[i], g[i], b[i] );
      }
 */
   }
}



static void write_monocolor_pixels( GLuint n,
                                    const GLint x[], const GLint y[],
                                    const GLubyte mask[] )
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: write_monocolor_pixels(...)\n");
#endif

   /* use current rgb color */
   for (i=0; i<n; i++) {
/*      if (mask[i]) {
         clgd547x_drawpixel( x[i], FLIP(y[i]), 
           clgd547xMesa->red, clgd547xMesa->green, clgd547xMesa->blue );
      }
 */
   }
}

/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

/* Read an array of color index pixels. */
static void read_index_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLuint index[], const GLubyte mask[] )
{
   int i;

#ifdef DEBUG_CMESA
   printf("cmesa: read_index_pixels(...)\n");
#endif

   for (i=0; i<n; i++,x++) {
/*      index[i] = clgd547x_getpixel( x[i], FLIP(y[i]) ); */
   }
}



static void read_color_pixels( GLuint n, const GLint x[], const GLint y[],
                               GLubyte red[], GLubyte green[],
                               GLubyte blue[], GLubyte alpha[],
                               const GLubyte mask[] )
{
   /* TODO */

#ifdef DEBUG_CMESA
   printf("cmesa: read_color_pixels(...)\n");
#endif

}


/************************************************************************/

static void clgd547x_mesa_setup_dd_pointers( void )
{

#ifdef DEBUG_CMESA
   printf("cmesa: clgd547x_mesa_setup_dd_pointers()\n");
#endif

   /* Initialize all the pointers in the DD struct.  Do this whenever */
   /* a new context is made current or we change buffers via set_buffer! */

   DD.finish = clgd547x_finish;
   DD.flush = clgd547x_flush;

   DD.clear_index = clear_index;
   DD.clear_color = clear_color;
   DD.clear = clear;

   DD.index = set_index;
   DD.color = set_color;
   DD.index_mask = index_mask;
   DD.color_mask = color_mask;

   DD.logicop = logicop;
   DD.dither = dither;

   DD.set_buffer = set_buffer;
   DD.buffer_size = buffer_size;

   DD.alloc_depth_buffer = alloc_depth_buffer;
   DD.clear_depth_buffer = clgd547x_clearDepthBuffer;
/*   DD.set_depth_func = set_depth_func; */
   DD.read_depth_span = read_depth_span;

   DD.get_points_func = choose_points_function;
   DD.get_line_func = choose_line_function;
   DD.get_polygon_func = choose_polygon_function;

/*
   DD.write_index_span_z = write_index_span_z;
   DD.write_monoindex_span_z = write_monoindex_span_z;
   DD.write_color_span_z = write_color_span_z;
   DD.write_monocolor_span_z = write_monocolor_span_z;
   DD.write_texture_span_z = write_texture_span_z;
*/

   /* Pixel/span writing functions: */
   /* TODO: use different funcs for 8, 16, 32-bit depths */
   DD.write_color_span       = write_color_span;
   DD.write_monocolor_span   = write_monocolor_span;
   DD.write_color_pixels     = write_color_pixels;
   DD.write_monocolor_pixels = write_monocolor_pixels;
   DD.write_index_span       = write_index_span;
   DD.write_monoindex_span   = write_monoindex_span;
   DD.write_index_pixels     = write_index_pixels;
   DD.write_monoindex_pixels = write_monoindex_pixels;

   /* Pixel/span reading functions: */
   /* TODO: use different funcs for 8, 16, 32-bit depths */
   DD.read_index_span = read_index_span;
   DD.read_color_span = read_color_span;
   DD.read_index_pixels = read_index_pixels;
   DD.read_color_pixels = read_color_pixels;
}



/*
 * Create a new VGA/Mesa context and return a handle to it.
 */
clgd547xMesaContext clgd547xMesaCreateContext( void )
{
   clgd547xMesaContext ctx;
   GLfloat redscale, greenscale, bluescale, alphascale;

#ifdef DEBUG_CMESA
   printf("cmesa: clgd547xMesaCreateContext()\n");
#endif

   ctx = (clgd547xMesaContext) malloc( sizeof(struct clgd547x_mesa_context) );

   ctx->width = clgd547x_getxdim();
   ctx->height = clgd547x_getydim();
   ctx->db_flag = GL_TRUE;		/* double buffering */
   ctx->depth = clgd547x_getdepth();

   /* RGB mode -- this is what you want for the demos -TODO - PFM*/
   if (GL_TRUE) {
      ctx->rgb_flag = GL_TRUE;
      redscale = greenscale = bluescale = alphascale = 255.0;
      setup_dithered_color(ctx); 
   }
   else {
      ctx->rgb_flag = GL_FALSE;
      redscale = greenscale = bluescale = alphascale = 0.0;
   }

   ctx->glctx = gl_new_context( ctx->rgb_flag,
                                redscale, greenscale, bluescale, alphascale,
                                ctx->db_flag, NULL );
   
   ctx->index = 1;
   ctx->red = ctx->green = ctx->blue = 255;
   
   return ctx;
}


/*
 * Destroy the given VGA/Mesa context.
 */
void clgd547xMesaDestroyContext( clgd547xMesaContext ctx )
{

#ifdef DEBUG_CMESA
   printf("cmesa: clgd547xMesaDestroyContext()\n");
#endif

   if (ctx) {
      gl_destroy_context( ctx->glctx );
      free( ctx );				
      if (ctx==clgd547xMesa) {
         clgd547xMesa = NULL;
      }
   }
}



/*
 * Make the specified VGA/Mesa context the current one.
 */
void clgd547xMesaMakeCurrent( clgd547xMesaContext ctx )
{

#ifdef DEBUG_CMESA
   printf("cmesa: clgd547xMesaMakeCurrent()\n");
#endif

   clgd547xMesa = ctx;
   gl_set_context( ctx->glctx );

   clgd547x_mesa_setup_dd_pointers();   
   gl_viewport( 0, 0, ctx->width, ctx->height );
   setMondelloHeight(ctx->height);    
}


/*
 * Return a handle to the current VGA/Mesa context.
 */
clgd547xMesaContext clgd547xMesaGetCurrentContext( void )
{

#ifdef DEBUG_CMESA
   printf("cmesa: clgd547xMesaContext()\n");
#endif

   return clgd547xMesa;
}


#else

/*
 * Need this to provide at least one external definition.
 */

int gl_clgd547x_dummy_function(void)
{
   return 0;
}

#endif  /* MONDELLO */

