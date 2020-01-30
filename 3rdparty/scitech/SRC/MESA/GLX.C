/* $Id: glx.c,v 1.13 1997/03/11 01:31:39 brianp Exp $ */

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
 * $Log:   S:/scitech/src/mesa/glx.c_v  $
 * 
 *    Rev 1.0   28 Mar 1997 17:18:36   KendallB
 *  
 * Revision 1.13  1997/03/11 01:31:39  brianp
 * changed version strings to 2.2
 *
 * Revision 1.12  1997/02/27 20:02:40  brianp
 * better visual creation error checking (Alex Harper)
 *
 * Revision 1.11  1997/01/31 20:38:49  brianp
 * in find_glx_visual(), try to match new vishandle instead of visinfo
 *
 * Revision 1.10  1997/01/29 19:34:47  brianp
 * replaced is_overlay_visual() with level_of_visual()
 *
 * Revision 1.9  1997/01/29 19:23:31  brianp
 * fixed memory leaks per Wolfram Gloger
 *
 * Revision 1.8  1996/12/09 22:33:53  brianp
 * faster window_exists() function from Stephane Rehel
 *
 * Revision 1.7  1996/11/21 03:40:56  brianp
 * 32-bit per pixel visuals now work
 *
 * Revision 1.6  1996/10/31 01:01:43  brianp
 * added ancillary buffer garbage collection
 * incremented version to 2.1
 *
 * Revision 1.5  1996/09/27 17:09:15  brianp
 * fixed a few warnings about unreachable breaks and returns
 *
 * Revision 1.4  1996/09/27 01:27:01  brianp
 * removed unused variables
 *
 * Revision 1.3  1996/09/19 03:42:38  brianp
 * fixed bug in glXGetCurrentDrawable()
 *
 * Revision 1.2  1996/09/19 03:16:50  brianp
 * re-implemented for new X/Mesa interface, added glXReleaseBuffersMESA()
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */


/*
 * A pseudo-GLX implementation to allow OpenGL/GLX programs to work with Mesa.
 *
 * Thanks to the contributors:
 *
 * Initial version:  Philip Brown (philb@CSUA.Berkeley.EDU)
 * Better glXGetConfig() support: Armin Liebchen (liebchen@asylum.cs.utah.edu)
 * Further visual-handling refinements: Wolfram Gloger
 *    (wmglo@Dent.MED.Uni-Muenchen.DE).
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "GL/gl.h"
#include "GL/glx.h"
#include "GL/xmesa.h"
#include "context.h"
#include "config.h"
#include "macros.h"
#include "types.h"
#include "xmesaP.h"



#define DONT_CARE -1



#define MAX_VISUALS 100
static XMesaVisual VisualTable[MAX_VISUALS];
static int NumVisuals = 0;


#define MAX_BUFFERS 1000
static XMesaBuffer BufferList[MAX_BUFFERS];
static int NumBuffers = 0;



/*
 * This struct and some code fragments borrowed
 * from Mark Kilgard's GLUT library.
 */
typedef struct _OverlayInfo {
  /* Avoid 64-bit portability problems by being careful to use
     longs due to the way XGetWindowProperty is specified. Note
     that these parameters are passed as CARD32s over X
     protocol. */
  long overlay_visual;
  long transparent_type;
  long value;
  long layer;
} OverlayInfo;



/* Macro to handle c_class vs class field name in XVisualInfo struct */
#if defined(__cplusplus) || defined(c_plusplus)
#define CLASS c_class
#else
#define CLASS class
#endif




/*
 * Test if the given XVisualInfo is usable for Mesa rendering.
 */
static GLboolean is_usable_visual( XVisualInfo *vinfo )
{
   switch (vinfo->CLASS) {
      case StaticGray:
      case GrayScale:
         /* Any StaticGray/GrayScale visual works in RGB or CI mode */
         return GL_TRUE;
      case StaticColor:
      case PseudoColor:
	 /* Any StaticColor/PseudoColor visual of at least 4 bits */
	 if (vinfo->depth>=4) {
	    return GL_TRUE;
	 }
	 else {
	    return GL_FALSE;
	 }
      case TrueColor:
      case DirectColor:
	 /* Any depth of TrueColor or DirectColor works in RGB mode */
	 return GL_TRUE;
      default:
	 /* This should never happen */
	 return GL_FALSE;
   }
}



/*
 * Return the level (overlay, normal, underlay) of a given XVisualInfo.
 * Input:  dpy - the X display
 *         vinfo - the XVisualInfo to test
 * Return:  level of the visual:
 *             0 = normal planes
 *            >0 = overlay planes
 *            <0 = underlay planes
 */
static int level_of_visual( Display *dpy, XVisualInfo *vinfo )
{
   Atom overlayVisualsAtom;
   OverlayInfo *overlay_info = NULL;
   int numOverlaysPerScreen;
   Status status;
   Atom actualType;
   int actualFormat;
   unsigned long sizeData, bytesLeft;
   int i;

   /*
    * The SERVER_OVERLAY_VISUALS property on the root window contains
    * a list of overlay visuals.  Get that list now.
    */
   overlayVisualsAtom = XInternAtom(dpy,"SERVER_OVERLAY_VISUALS", True);
   if (overlayVisualsAtom == None) {
      return 0;
   }

   status = XGetWindowProperty(dpy, RootWindow( dpy, vinfo->screen ),
                               overlayVisualsAtom, 0L, (long) 10000, False,
                               overlayVisualsAtom, &actualType, &actualFormat,
                               &sizeData, &bytesLeft,
                               (unsigned char **) &overlay_info );

   if (status != Success || actualType != overlayVisualsAtom ||
       actualFormat != 32 || sizeData < 4) {
      /* something went wrong */
      XFree(overlay_info);
      return 0;
   }

   /* search the overlay visual list for the visual ID of interest */
   numOverlaysPerScreen = sizeData / 4;
   for (i=0;i<numOverlaysPerScreen;i++) {
      OverlayInfo *ov;
      ov = overlay_info + i;
      if (ov->overlay_visual==vinfo->visualid) {
         /* found the visual */
         if (/*ov->transparent_type==1 &&*/ ov->layer!=0) {
            int level = ov->layer;
            XFree(overlay_info);
            return level;
         }
         else {
            XFree(overlay_info);
            return 0;
         }
      }
   }

   /* The visual ID was not found in the overlay list. */
   XFree(overlay_info);
   return 0;
}




/*
 * Given an XVisualInfo and RGB, Double, and Depth buffer flags, save the
 * configuration in our list of GLX visuals.
 */
static XMesaVisual
save_glx_visual( Display *dpy, XVisualInfo *vinfo,
                 GLboolean rgb, GLboolean alpha, GLboolean dbl,
                 GLint depth_size, GLint stencil_size,
                 GLint accum_size, GLint level )
{
   GLboolean ximage_flag = GL_TRUE;
   XMesaVisual xmvis;

   if (NumVisuals>=MAX_VISUALS) {
      fprintf( stderr, "GLX Error: maximum number of visuals exceeded\n");
      return NULL;
   }

   if (dbl) {
      /* Check if the MESA_BACK_BUFFER env var is set */
      char *backbuffer = getenv("MESA_BACK_BUFFER");
      if (backbuffer) {
         if (backbuffer[0]=='p' || backbuffer[0]=='P') {
            ximage_flag = GL_FALSE;
         }
         else if (backbuffer[0]=='x' || backbuffer[0]=='X') {
            ximage_flag = GL_TRUE;
         }
         else {
            fprintf(stderr, "Mesa: invalid value for MESA_BACK_BUFFER ");
            fprintf(stderr, "environment variable, using an XImage.\n");
         }
      }
   }

   xmvis = XMesaCreateVisual( dpy, vinfo, rgb, alpha, dbl, ximage_flag,
                              depth_size, stencil_size, accum_size, level );
   if (xmvis) {
      VisualTable[NumVisuals] = xmvis;
      NumVisuals++;
   }
   return xmvis;
}



/*
 * Create a GLX visual from a regular XVisualInfo.
 */
static XMesaVisual
create_glx_visual( Display *dpy, XVisualInfo *visinfo )
{
   int vislevel;

   vislevel = level_of_visual( dpy, visinfo );
   if (vislevel) {
      /* Configure this visual as a CI, single-buffered overlay */
      return save_glx_visual( dpy, visinfo,
                              GL_FALSE,  /* rgb */
                              GL_FALSE,  /* alpha */
                              GL_FALSE,  /* double */
                              0,         /* depth bits */
                              0,         /* stencil bits */
                              0,         /* accum bits */
                              vislevel   /* level */
                            );
   }
   else if (is_usable_visual( visinfo )) {
      /* Configure this visual as RGB, double-buffered, depth-buffered. */
      /* This is surely wrong for some people's needs but what else */
      /* can be done?  They should use glXChooseVisual(). */
      return save_glx_visual( dpy, visinfo,
                              GL_TRUE,   /* rgb */
                              GL_FALSE,  /* alpha */
                              GL_TRUE,   /* double */
                              8*sizeof(GLdepth),
                              8*sizeof(GLstencil),
                              8*sizeof(GLaccum),
                              0          /* level */
                            );
   }
   else {
      fprintf(stderr,"Mesa: error in glXCreateContext: bad visual\n");
      return NULL;
   }
}



/*
 * Find the GLX visual associated with an XVisualInfo.
 */
static XMesaVisual
find_glx_visual( Display *dpy, XVisualInfo *vinfo )
{
   int i;

   /* First try to match pointers */
   for (i=0;i<NumVisuals;i++) {
      if (VisualTable[i]->display==dpy && VisualTable[i]->vishandle==vinfo) {
         return VisualTable[i];
      }
   }
   /* try to match visual id */
   for (i=0;i<NumVisuals;i++) {
      if (VisualTable[i]->display==dpy
          && VisualTable[i]->visinfo->visualid == vinfo->visualid) {
         return VisualTable[i];
      }
   }
   return NULL;
}




/*
 * Determine if a given X window ID is valid (window exists).
 * Do this by calling XGetWindowAttributes() for the window and
 * checking if we catch an X error.
 * Input:  dpy - the display
 *         win - the window to check for existance
 * Return:  GL_TRUE - window exists
 *          GL_FALSE - window doesn't exist
 */
static GLboolean WindowExistsFlag;

static int window_exists_err_handler( Display* dpy, XErrorEvent* xerr )
{
   if (xerr->error_code == BadWindow) {
      WindowExistsFlag = GL_FALSE;
   }
   return 0;
}

static GLboolean window_exists( Display *dpy, Window win )
{
   XWindowAttributes wa;
   int (*old_handler)( Display*, XErrorEvent* );
   WindowExistsFlag = GL_TRUE;
   old_handler = XSetErrorHandler(window_exists_err_handler);
   XGetWindowAttributes( dpy, win, &wa ); /* dummy request */
   XSetErrorHandler(old_handler);
   return WindowExistsFlag;
}



/*
 * We occasionally call this function to check if any windows in the
 * BufferList have been destroyed.  If so, we can then deallocate
 * the corresponding Mesa ancillary buffers (depth, stencil, accum)
 * We need to do this because there's no way for Xlib to notify us
 * when a window gets destroyed.
 */
static void free_zombie_buffers( void )
{
   GLint i, j;

   for (i=0;i<NumBuffers;i++) {
      if (!BufferList[i]->pixmap_flag) {
         Display *dpy = BufferList[i]->xm_visual->display;
         Window win   = BufferList[i]->frontbuffer;
         int exists;

         XSync(dpy,False);
         exists = window_exists( dpy, win );

         if (!exists) {
            /* found a dead window, free the ancillary info */
            XMesaDestroyBuffer( BufferList[i] );
            /* move remaining buffer entries up one slot */
            for (j=i+1;j<NumBuffers;j++) {
               BufferList[j-1] = BufferList[j];
            }
            NumBuffers--;
            i--;            /* this cancels out the i++ */
         }
      }
   }
}



/*
 * Return the XMesaBuffer which corresponds to the given X drawable.
 * Return NULL if no corresponding XMesaBuffer.
 */
static XMesaBuffer find_buffer( Display *dpy, Drawable d )
{
   GLint i;

   for (i=0;i<NumBuffers;i++) {
      if (BufferList[i]->frontbuffer==d) {
         return BufferList[i];
      }
   }
   return NULL;
}



/*
 * Create a new XMesaBuffer from an X drawable.
 * Input:  v - the XMesaVisual
 *         d - the Window or Pixmap
 *         pixmap_flag - GL_TRUE if d is a pixmap, GL_FALSE if d is a window
 *         cmap - the colormap if d is a pixmap, ignored otherwise.
 */
static XMesaBuffer save_buffer( XMesaVisual v, Drawable d,
                                GLboolean pixmap_flag, Colormap cmap )
{
   XMesaBuffer b;

   if (NumBuffers>=MAX_BUFFERS) {
      fprintf( stderr, "GLX Error: maximum number of buffers exceeded\n");
      return NULL;
   }
   if (pixmap_flag) {
      b = XMesaCreatePixmapBuffer( v, d, cmap );
   }
   else {
      b = XMesaCreateWindowBuffer( v, d );
   }
   if (!b) {
      return NULL;
   }
   BufferList[NumBuffers] = b;
   NumBuffers++;
   return b;
}



/*
 * Return the transparent pixel value for a GLX visual.
 * Input:  glxvis - the glx_visual
 * Return:  a pixel value or -1 if no transparent pixel
 */
static int transparent_pixel( XMesaVisual glxvis )
{
   Display *dpy = glxvis->display;
   XVisualInfo *vinfo = glxvis->visinfo;
   Atom overlayVisualsAtom;
   OverlayInfo *overlay_info = NULL;
   int numOverlaysPerScreen;
   Status status;
   Atom actualType;
   int actualFormat;
   unsigned long sizeData, bytesLeft;
   int i;

   /*
    * The SERVER_OVERLAY_VISUALS property on the root window contains
    * a list of overlay visuals.  Get that list now.
    */
   overlayVisualsAtom = XInternAtom(dpy,"SERVER_OVERLAY_VISUALS", True);
   if (overlayVisualsAtom == None) {
      return -1;
   }

   status = XGetWindowProperty(dpy, RootWindow( dpy, vinfo->screen ),
                               overlayVisualsAtom, 0L, (long) 10000, False,
                               overlayVisualsAtom, &actualType, &actualFormat,
                               &sizeData, &bytesLeft,
                               (unsigned char **) &overlay_info );

   if (status != Success || actualType != overlayVisualsAtom ||
       actualFormat != 32 || sizeData < 4) {
      /* something went wrong */
      XFree(overlay_info);
      return -1;
   }

   /* search the overlay visual list for the visual ID of interest */
   numOverlaysPerScreen = sizeData / 4;
   for (i=0;i<numOverlaysPerScreen;i++) {
      OverlayInfo *ov;
      ov = overlay_info + i;
      if (ov->overlay_visual==vinfo->visualid) {
         /* found it! */
         if (ov->transparent_type==0) {
            /* type 0 indicates no transparency */
            XFree(overlay_info);
            return -1;
         }
         else {
            /* ov->value is the transparent pixel */
            XFree(overlay_info);
            return ov->value;
         }
      }
   }

   /* The visual ID was not found in the overlay list. */
   XFree(overlay_info);
   return -1;
}



/*
 * Try to get an X visual which matches the given arguments.
 */
static XVisualInfo *get_visual( Display *dpy, int scr,
			        unsigned int depth, int xclass )
{
   XVisualInfo temp;
   long mask;
   int n;
   int default_depth;
   int default_class;

   mask = VisualScreenMask | VisualDepthMask | VisualClassMask;
   temp.screen = scr;
   temp.depth = depth;
   temp.CLASS = xclass;

   default_depth = DefaultDepth(dpy,scr);
   default_class = DefaultVisual(dpy,scr)->CLASS;

   if (depth==default_depth && xclass==default_class) {
      /* try to get root window's visual */
      temp.visualid = DefaultVisual(dpy,scr)->visualid;
      mask |= VisualIDMask;
   }

   return XGetVisualInfo( dpy, mask, &temp, &n );
}



/*
 * Retrieve the value of the given environment variable and find
 * the X visual which matches it.
 * Input:  dpy - the display
 *         screen - the screen number
 *         varname - the name of the environment variable
 * Return:  an XVisualInfo pointer to NULL if error.
 */
static XVisualInfo *get_env_visual( Display *dpy, int scr, char *varname )
{
   char *value;
   char type[100];
   int depth, xclass = -1;
   XVisualInfo *vis;

   value = getenv( varname );
   if (!value) {
      return NULL;
   }

   sscanf( value, "%s %d", type, &depth );

   if (strcmp(type,"TrueColor")==0)          xclass = TrueColor;
   else if (strcmp(type,"DirectColor")==0)   xclass = DirectColor;
   else if (strcmp(type,"PseudoColor")==0)   xclass = PseudoColor;
   else if (strcmp(type,"StaticColor")==0)   xclass = StaticColor;
   else if (strcmp(type,"GrayScale")==0)     xclass = GrayScale;
   else if (strcmp(type,"StaticGray")==0)    xclass = StaticGray;

   if (xclass>-1 && depth>0) {
      vis = get_visual( dpy, scr, depth, xclass );
      if (vis) {
	 return vis;
      }
   }

   fprintf( stderr, "Mesa: GLX unable to find visual class=%s, depth=%d.\n",
	    type, depth );
   return NULL;
}



/*
 * Select an X visual which satisfies the RGBA/CI flag and minimum depth.
 * Input:  dpy, screen - X display and screen number
 *         rgba - GL_TRUE = RGBA mode, GL_FALSE = CI mode
 *         min_depth - minimum visual depth
 *         preferred_class - preferred GLX visual class or DONT_CARE
 * Return:  pointer to an XVisualInfo or NULL.
 */
static XVisualInfo *choose_x_visual( Display *dpy, int screen,
				     GLboolean rgba, int min_depth,
                                     int preferred_class )
{
   XVisualInfo *vis;
   int xclass, visclass;
   int depth;

   if (rgba) {
      Atom hp_cr_maps = XInternAtom(dpy, "_HP_RGB_SMOOTH_MAP_LIST", True);
      /* First see if the MESA_RGB_VISUAL env var is defined */
      vis = get_env_visual( dpy, screen, "MESA_RGB_VISUAL" );
      if (vis) {
	 return vis;
      }
      /* Otherwise, search for a suitable visual */
      if (preferred_class==DONT_CARE) {
         for (xclass=0;xclass<6;xclass++) {
            switch (xclass) {
               case 0:  visclass = TrueColor;    break;
               case 1:  visclass = DirectColor;  break;
               case 2:  visclass = PseudoColor;  break;
               case 3:  visclass = StaticColor;  break;
               case 4:  visclass = GrayScale;    break;
               case 5:  visclass = StaticGray;   break;
            }
            if (min_depth==0) {
               /* start with shallowest */
               for (depth=0;depth<=32;depth++) {
                  if (visclass==TrueColor && depth==8 && !hp_cr_maps) {
                     /* Special case:  try to get 8-bit PseudoColor before */
                     /* 8-bit TrueColor */
                     vis = get_visual( dpy, screen, 8, PseudoColor );
                     if (vis) {
                        return vis;
                     }
                  }
                  vis = get_visual( dpy, screen, depth, visclass );
                  if (vis) {
                     return vis;
                  }
               }
            }
            else {
               /* start with deepest */
               for (depth=32;depth>=min_depth;depth--) {
                  if (visclass==TrueColor && depth==8 && !hp_cr_maps) {
                     /* Special case:  try to get 8-bit PseudoColor before */
                     /* 8-bit TrueColor */
                     vis = get_visual( dpy, screen, 8, PseudoColor );
                     if (vis) {
                        return vis;
                     }
                  }
                  vis = get_visual( dpy, screen, depth, visclass );
                  if (vis) {
                     return vis;
                  }
               }
            }
         }
      }
      else {
         /* search for a specific visual class */
         switch (preferred_class) {
            case GLX_TRUE_COLOR_EXT:    visclass = TrueColor;    break;
            case GLX_DIRECT_COLOR_EXT:  visclass = DirectColor;  break;
            case GLX_PSEUDO_COLOR_EXT:  visclass = PseudoColor;  break;
            case GLX_STATIC_COLOR_EXT:  visclass = StaticColor;  break;
            case GLX_GRAY_SCALE_EXT:    visclass = GrayScale;    break;
            case GLX_STATIC_GRAY_EXT:   visclass = StaticGray;   break;
            default:   return NULL;
         }
         if (min_depth==0) {
            /* start with shallowest */
            for (depth=0;depth<=32;depth++) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
         else {
            /* start with deepest */
            for (depth=32;depth>=min_depth;depth--) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
      }
   }
   else {
      /* First see if the MESA_CI_VISUAL env var is defined */
      vis = get_env_visual( dpy, screen, "MESA_CI_VISUAL" );
      if (vis) {
	 return vis;
      }
      /* Otherwise, search for a suitable visual, starting with shallowest */
      if (preferred_class==DONT_CARE) {
         for (xclass=0;xclass<4;xclass++) {
            switch (xclass) {
               case 0:  visclass = PseudoColor;  break;
               case 1:  visclass = StaticColor;  break;
               case 2:  visclass = GrayScale;    break;
               case 3:  visclass = StaticGray;   break;
            }
            /* try 8-bit up through 16-bit */
            for (depth=8;depth<=16;depth++) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
            /* try min_depth up to 8-bit */
            for (depth=min_depth;depth<8;depth++) {
               vis = get_visual( dpy, screen, depth, visclass );
               if (vis) {
                  return vis;
               }
            }
         }
      }
      else {
         /* search for a specific visual class */
         switch (preferred_class) {
            case GLX_TRUE_COLOR_EXT:    visclass = TrueColor;    break;
            case GLX_DIRECT_COLOR_EXT:  visclass = DirectColor;  break;
            case GLX_PSEUDO_COLOR_EXT:  visclass = PseudoColor;  break;
            case GLX_STATIC_COLOR_EXT:  visclass = StaticColor;  break;
            case GLX_GRAY_SCALE_EXT:    visclass = GrayScale;    break;
            case GLX_STATIC_GRAY_EXT:   visclass = StaticGray;   break;
            default:   return NULL;
         }
         /* try 8-bit up through 16-bit */
         for (depth=8;depth<=16;depth++) {
            vis = get_visual( dpy, screen, depth, visclass );
            if (vis) {
               return vis;
            }
         }
         /* try min_depth up to 8-bit */
         for (depth=min_depth;depth<8;depth++) {
            vis = get_visual( dpy, screen, depth, visclass );
            if (vis) {
               return vis;
            }
         }
      }
   }

   /* didn't find a visual */
   return NULL;
}



/*
 * Find the deepest X over/underlay visual of at least min_depth.
 * Input:  dpy, screen - X display and screen number
 *         level - the over/underlay level
 *         trans_type - transparent pixel type: GLX_NONE_EXT,
 *                      GLX_TRANSPARENT_RGB_EXT, GLX_TRANSPARENT_INDEX_EXT,
 *                      or DONT_CARE
 *         trans_value - transparent pixel value or DONT_CARE
 *         min_depth - minimum visual depth
 *         preferred_class - preferred GLX visual class or DONT_CARE
 * Return:  pointer to an XVisualInfo or NULL.
 */
static XVisualInfo *choose_x_overlay_visual( Display *dpy, int scr,
                                             int level, int trans_type,
                                             int trans_value,
                                             int min_depth,
                                             int preferred_class )
{
   Atom overlayVisualsAtom;
   OverlayInfo *overlay_info;
   int numOverlaysPerScreen;
   Status status;
   Atom actualType;
   int actualFormat;
   unsigned long sizeData, bytesLeft;
   int i;
   XVisualInfo *deepvis;
   int deepest;

   /*TMP*/ int tt, tv;

   switch (preferred_class) {
      case GLX_TRUE_COLOR_EXT:    preferred_class = TrueColor;    break;
      case GLX_DIRECT_COLOR_EXT:  preferred_class = DirectColor;  break;
      case GLX_PSEUDO_COLOR_EXT:  preferred_class = PseudoColor;  break;
      case GLX_STATIC_COLOR_EXT:  preferred_class = StaticColor;  break;
      case GLX_GRAY_SCALE_EXT:    preferred_class = GrayScale;    break;
      case GLX_STATIC_GRAY_EXT:   preferred_class = StaticGray;   break;
      default:                    preferred_class = DONT_CARE;
   }

   /*
    * The SERVER_OVERLAY_VISUALS property on the root window contains
    * a list of overlay visuals.  Get that list now.
    */
   overlayVisualsAtom = XInternAtom(dpy,"SERVER_OVERLAY_VISUALS", True);
   if (overlayVisualsAtom == None) {
      return GL_FALSE;
   }

   status = XGetWindowProperty(dpy, RootWindow( dpy, scr ),
                               overlayVisualsAtom, 0L, (long) 10000, False,
                               overlayVisualsAtom, &actualType, &actualFormat,
                               &sizeData, &bytesLeft,
                               (unsigned char **) &overlay_info );

   if (status != Success || actualType != overlayVisualsAtom ||
       actualFormat != 32 || sizeData < 4) {
      /* something went wrong */
      return GL_FALSE;
   }

   /* Search for the deepest overlay which satisifies all criteria. */
   deepest = min_depth;
   deepvis = NULL;

   numOverlaysPerScreen = sizeData / 4;
   for (i=0;i<numOverlaysPerScreen;i++) {
      XVisualInfo *vislist, vistemplate;
      int count;
      OverlayInfo *ov;
      ov = overlay_info + i;

      if (ov->layer!=level) {
         /* failed overlay level criteria */
         continue;
      }
      if (!(trans_type==DONT_CARE
            || (trans_type==GLX_TRANSPARENT_INDEX_EXT
                && ov->transparent_type>0)
            || (trans_type==GLX_NONE_EXT && ov->transparent_type==0))) {
         /* failed transparent pixel type criteria */
         continue;
      }
      if (trans_value!=DONT_CARE && trans_value!=ov->value) {
         /* failed transparent pixel value criteria */
         continue;
      }

      /* get XVisualInfo and check the depth */
      vistemplate.visualid = ov->overlay_visual;
      vistemplate.screen = scr;
      vislist = XGetVisualInfo( dpy, VisualIDMask | VisualScreenMask,
                                &vistemplate, &count );

      if (count!=1) {
         /* something went wrong */
         continue;
      }
      if (preferred_class!=DONT_CARE && preferred_class!=vislist->CLASS) {
         /* wrong visual class */
         continue;
      }

      if (deepvis==NULL || vislist->depth > deepest) {
         /* YES!  found a satisfactory visual */
         if (deepvis) {
            free( deepvis );
         }
         deepest = vislist->depth;
         deepvis = vislist;
         /* TMP */  tt = ov->transparent_type;
         /* TMP */  tv = ov->value;
      }
   }

/*TMP
   if (deepvis) {
      printf("chose 0x%x:  layer=%d depth=%d trans_type=%d trans_value=%d\n",
             deepvis->visualid, level, deepvis->depth, tt, tv );
   }
*/
   return deepvis;
}



/*
 * Return the number of bits set in n.
 */
static int bitcount( unsigned long n )
{
   int bits;
   for (bits=0; n>0; n=n>>1) {
      if (n&1) {
         bits++;
      }
   }
   return bits;
}



XVisualInfo *glXChooseVisual( Display *dpy, int screen, int *list )
{
   int *parselist;
   XVisualInfo *vis;
   int min_ci = 0;
   int min_red=0, min_green=0, min_blue=0;
   GLboolean rgb_flag = GL_FALSE;
   GLboolean alpha_flag = GL_FALSE;
   GLboolean double_flag = GL_FALSE;
   GLint depth_size = 0;
   GLint stencil_size = 0;
   GLint accum_size = 0;
   int level = 0;
   int visual_type = DONT_CARE;
   int trans_type = DONT_CARE;
   int trans_value = DONT_CARE;

   parselist = list;

   while (*parselist) {

      switch (*parselist) {
	 case GLX_USE_GL:
	    /* ignore */
	    parselist++;
	    break;
	 case GLX_BUFFER_SIZE:
	    parselist++;
	    min_ci = *parselist++;
	    break;
	 case GLX_LEVEL:
	    parselist++;
            level = *parselist++;
	    break;
	 case GLX_RGBA:
	    rgb_flag = GL_TRUE;
	    parselist++;
	    break;
	 case GLX_DOUBLEBUFFER:
	    double_flag = GL_TRUE;
	    parselist++;
	    break;
	 case GLX_STEREO:
	    /* not supported */
            return NULL;
	 case GLX_AUX_BUFFERS:
	    /* ignore */
	    parselist++;
	    parselist++;
	    break;
	 case GLX_RED_SIZE:
	    parselist++;
	    min_red = *parselist++;
	    break;
	 case GLX_GREEN_SIZE:
	    parselist++;
	    min_green = *parselist++;
	    break;
	 case GLX_BLUE_SIZE:
	    parselist++;
	    min_blue = *parselist++;
	    break;
	 case GLX_ALPHA_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               alpha_flag = size>0 ? 1 : 0;
            }
	    break;
	 case GLX_DEPTH_SIZE:
	    parselist++;
	    depth_size = *parselist++;
	    break;
	 case GLX_STENCIL_SIZE:
	    parselist++;
	    stencil_size = *parselist++;
	    break;
	 case GLX_ACCUM_RED_SIZE:
	 case GLX_ACCUM_GREEN_SIZE:
	 case GLX_ACCUM_BLUE_SIZE:
	 case GLX_ACCUM_ALPHA_SIZE:
	    parselist++;
            {
               GLint size = *parselist++;
               accum_size = MAX2( accum_size, size );
            }
	    break;

         /*
          * GLX_EXT_visual_info extension
          */
         case GLX_X_VISUAL_TYPE_EXT:
            parselist++;
            visual_type = *parselist++;
            break;
         case GLX_TRANSPARENT_TYPE_EXT:
            parselist++;
            trans_type = *parselist++;
            break;
         case GLX_TRANSPARENT_INDEX_VALUE_EXT:
            parselist++;
            trans_value = *parselist++;
            break;
         case GLX_TRANSPARENT_RED_VALUE_EXT:
         case GLX_TRANSPARENT_GREEN_VALUE_EXT:
         case GLX_TRANSPARENT_BLUE_VALUE_EXT:
         case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
	    /* ignore */
	    parselist++;
	    parselist++;
	    break;
         
	 case None:
	    break;
	 default:
	    /* undefined attribute */
	    return NULL;
      }
   }

   /*
    * Since we're only simulating the GLX extension this function will never
    * find any real GL visuals.  Instead, all we can do is try to find an RGB
    * or CI visual of appropriate depth.  Other requested attributes such as
    * double buffering, depth buffer, etc. will be associated with the X
    * visual and stored in the VisualTable[].
    */
   if (level==0) {
      /* normal color planes */
      if (rgb_flag) {
         /* Get an RGB visual */
         int min_rgb = min_red + min_green + min_blue;
         if (min_rgb>1 && min_rgb<8) {
            /* a special case to be sure we can get a monochrome visual */
            min_rgb = 1;
         }
         vis = choose_x_visual( dpy, screen, rgb_flag, min_rgb, visual_type );
      }
      else {
         /* Get a color index visual */
         vis = choose_x_visual( dpy, screen, rgb_flag, min_ci, visual_type );
         accum_size = 0;
      }
   }
   else {
      /* over/underlay planes */
      vis = choose_x_overlay_visual( dpy, screen, level, trans_type,
                                     trans_value, min_ci, visual_type );
   }

   if (vis) {
      if (!save_glx_visual( dpy, vis, rgb_flag, alpha_flag, double_flag,
                            depth_size, stencil_size, accum_size, level ))
         return NULL;
   }

   return vis;
}




GLXContext glXCreateContext( Display *dpy, XVisualInfo *visinfo,
			     GLXContext share_list, Bool direct )
{
   XMesaVisual glxvis;

   /* Garbage collection */
   free_zombie_buffers();

   glxvis = find_glx_visual( dpy, visinfo );
   if (!glxvis) {
      /* This visual wasn't found with glXChooseVisual() */
      glxvis = create_glx_visual( dpy, visinfo );
      if (!glxvis) {
         /* unusable visual */
         return NULL;
      }
   }
   return (GLXContext) XMesaCreateContext( glxvis, (XMesaContext) share_list );
}



Bool glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
   if (ctx && drawable) {
      static GLXDrawable prev_drawable = 0;
      static XMesaBuffer prev_buffer = 0;
      XMesaBuffer buffer;

      if (drawable==prev_drawable) {
         buffer = prev_buffer;
      }
      else {
         buffer = find_buffer( dpy, drawable );
      }
      if (!buffer) {
         /* drawable must be a new window! */
         buffer = save_buffer( ctx->xm_visual, drawable, GL_FALSE, 0 );
         if (!buffer) {
            /* If this happens, increase MAX_BUFFERS and recompile */
            fprintf(stderr,"Mesa GLX error: too many windows!\n");
            return False;
         }
      }
      prev_drawable = drawable;
      prev_buffer = buffer;

      /* Now make current! */
      return (Bool) XMesaMakeCurrent( (XMesaContext) ctx, buffer );
   }
   else if (!ctx && !drawable) {
      /* release current context w/out assigning new one. */
      XMesaMakeCurrent( NULL, NULL );
      return True;
   }
   else {
      /* ctx XOR drawable is NULL, this is an error */
      return False;
   }
}



GLXPixmap glXCreateGLXPixmap( Display *dpy, XVisualInfo *visinfo,
                              Pixmap pixmap )
{
   XMesaVisual v;
   XMesaBuffer b;

   v = find_glx_visual( dpy, visinfo );
   if (!v) {
      v = create_glx_visual( dpy, visinfo );
      if (!v) {
         /* unusable visual */
         return 0;
      }
   }

   b = save_buffer( v, pixmap, GL_TRUE, 0 );
   if (!b) {
      return 0;
   }
   return b->frontbuffer;
}


#ifdef GLX_MESA_pixmap_colormap

GLXPixmap glXCreateGLXPixmapMESA( Display *dpy, XVisualInfo *visinfo,
                                  Pixmap pixmap, Colormap cmap )
{
   XMesaVisual v;
   XMesaBuffer b;

   v = find_glx_visual( dpy, visinfo );
   if (!v) {
      v = create_glx_visual( dpy, visinfo );
      if (!v) {
         /* unusable visual */
         return 0;
      }
   }

   b = save_buffer( v, pixmap, GL_TRUE, cmap );
   if (!b) {
      return 0;
   }
   return b->frontbuffer;
}

#endif


void glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap )
{
   int i, j;

   for (i=0;i<NumBuffers;i++) {
      if (BufferList[i]->frontbuffer==pixmap) {
         XMesaDestroyBuffer( BufferList[i] );
	 for (j=i+1;j<NumBuffers;j++) {
	    BufferList[j-1] = BufferList[j];
	 }
	 NumBuffers--;
	 return;
      }
   }
   fprintf( stderr, "Mesa: glXDestroyGLXPixmap: invalid pixmap\n");
}


void glXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
		     GLuint mask )
{
   XMesaContext xm_src, xm_dst;
   xm_src = (XMesaContext) src;
   xm_dst = (XMesaContext) dst;
   gl_copy_context( xm_src->gl_ctx, xm_dst->gl_ctx, mask );
}



Bool glXQueryExtension( Display *dpy, int *errorb, int *event )
{
   /* Mesa's GLX isn't really an X extension but we try to act like one. */
   return True;
}


void glXDestroyContext( Display *dpy, GLXContext ctx )
{
   XMesaDestroyContext( (XMesaContext) ctx );

   /* Garbage collection */
   free_zombie_buffers();
}



Bool glXIsDirect( Display *dpy, GLXContext ctx )
{
   /* This isn't true but... */
   return True;
}



void glXSwapBuffers( Display *dpy, GLXDrawable drawable )
{
   static GLXDrawable prev_drawable = 0;
   static XMesaBuffer prev_buffer = 0;

   if (drawable==prev_drawable) {
      XMesaSwapBuffers( prev_buffer );
      return;
   }

   prev_drawable = drawable;
   prev_buffer = find_buffer( dpy, drawable );
   if (prev_buffer) {
      XMesaSwapBuffers( prev_buffer );
   }
   else {
      fprintf( stderr,
	  "Warning: glXSwapBuffers drawable doesn't match current context\n");
   }
}



Bool glXQueryVersion( Display *dpy, int *maj, int *min )
{
   /* Return GLX version, not Mesa version */
   *maj = 1;
   *min = 1;
   return True;
}



/*
 * Query the GLX attributes of the given XVisualInfo.
 */
int glXGetConfig( Display *dpy, XVisualInfo *visinfo,
		  int attrib, int *value )
{
   int visclass;
   XMesaVisual glxvis;

   glxvis = find_glx_visual( dpy, visinfo );
   if (!glxvis) {
      /* this visual wasn't obtained with glXChooseVisual */
      glxvis = create_glx_visual( dpy, visinfo );
      if (!glxvis) {
	 /* this visual can't be used for GL rendering */
	 if (attrib==GLX_USE_GL) {
	    *value = (int) False;
	    return 0;
	 }
	 else {
	    /*fprintf( stderr, "Mesa: Error in glXGetConfig: bad visual\n");*/
	    return GLX_BAD_VISUAL;
	 }
      }
   }

   /* Get the visual class */
   visclass = visinfo->CLASS;

   switch(attrib) {
      case GLX_USE_GL:
         *value = (int) True;
	 return 0;
      case GLX_BUFFER_SIZE:
	 *value = visinfo->depth;
	 return 0;
      case GLX_LEVEL:
	 *value = glxvis->level;
	 return 0;
      case GLX_RGBA:
	 if (glxvis->gl_visual->RGBAflag) {
	    *value = True;
	 }
	 else {
	    *value = False;
	 }
	 return 0;
      case GLX_DOUBLEBUFFER:
	 *value = (int) glxvis->gl_visual->DBflag;
	 return 0;
      case GLX_STEREO:
	 *value = (int) False;
	 return 0;
      case GLX_AUX_BUFFERS:
	 *value = (int) False;
	 return 0;
      case GLX_RED_SIZE:
         if (visclass==DirectColor || visclass==TrueColor) {
            *value = bitcount( visinfo->visual->red_mask );
         }
         else {
            /* a crude approximation */
            *value = visinfo->depth;
         }
	 return 0;
      case GLX_GREEN_SIZE:
         if (visclass==DirectColor || visclass==TrueColor) {
            *value = bitcount( visinfo->visual->green_mask );
         }
         else {
            *value = visinfo->depth;
         }
	 return 0;
      case GLX_BLUE_SIZE:
         if (visclass==DirectColor || visclass==TrueColor) {
            *value = bitcount( visinfo->visual->blue_mask );
         }
         else {
            *value = visinfo->depth;
         }
	 return 0;
      case GLX_ALPHA_SIZE:
         if (glxvis->gl_visual->FrontAlphaEnabled ||
             glxvis->gl_visual->BackAlphaEnabled) {
            *value = 8;
         }
         else {
            *value = 0;
         }
	 return 0;
      case GLX_DEPTH_SIZE:
         *value = glxvis->gl_visual->DepthBits;
	 return 0;
      case GLX_STENCIL_SIZE:
	 *value = glxvis->gl_visual->StencilBits;
	 return 0;
      case GLX_ACCUM_RED_SIZE:
      case GLX_ACCUM_GREEN_SIZE:
      case GLX_ACCUM_BLUE_SIZE:
      case GLX_ACCUM_ALPHA_SIZE:
	 *value = glxvis->gl_visual->AccumBits;
	 return 0;

      /*
       * GLX_EXT_visual_info extension
       */
      case GLX_X_VISUAL_TYPE_EXT:
         switch (glxvis->visinfo->CLASS) {
            case StaticGray:   *value = GLX_STATIC_GRAY_EXT;   return 0;
            case GrayScale:    *value = GLX_GRAY_SCALE_EXT;    return 0;
            case StaticColor:  *value = GLX_STATIC_GRAY_EXT;   return 0;
            case PseudoColor:  *value = GLX_PSEUDO_COLOR_EXT;  return 0;
            case TrueColor:    *value = GLX_TRUE_COLOR_EXT;    return 0;
            case DirectColor:  *value = GLX_DIRECT_COLOR_EXT;  return 0;
         }
         return 0;
      case GLX_TRANSPARENT_TYPE_EXT:
         if (glxvis->level==0) {
            /* normal planes */
            *value = GLX_NONE_EXT;
         }
         else if (glxvis->level>0) {
            /* overlay */
            if (glxvis->gl_visual->RGBAflag) {
               *value = GLX_TRANSPARENT_RGB_EXT;
            }
            else {
               *value = GLX_TRANSPARENT_INDEX_EXT;
            }
         }
         else if (glxvis->level<0) {
            /* underlay */
            *value = GLX_NONE_EXT;
         }
         return 0;
      case GLX_TRANSPARENT_INDEX_VALUE_EXT:
         {
            int pixel = transparent_pixel( glxvis );
            if (pixel>=0) {
               *value = pixel;
            }
            /* else undefined */
         }
         return 0;
      case GLX_TRANSPARENT_RED_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_GREEN_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_BLUE_VALUE_EXT:
         /* undefined */
         return 0;
      case GLX_TRANSPARENT_ALPHA_VALUE_EXT:
         /* undefined */
         return 0;

      /*
       * Extensions
       */
      default:
	 return GLX_BAD_ATTRIBUTE;
   }
}



GLXContext glXGetCurrentContext( void )
{
   return (GLXContext) XMesaGetCurrentContext();
}



GLXDrawable glXGetCurrentDrawable( void )
{
   XMesaBuffer b = XMesaGetCurrentBuffer();
   if (b) {
      return b->frontbuffer;
   }
   else {
      return 0;
   }
}


void glXWaitGL( void )
{
   XMesaContext xmesa = XMesaGetCurrentContext();
   XMesaFlush( xmesa );
}



void glXWaitX( void )
{
   XMesaContext xmesa = XMesaGetCurrentContext();
   XMesaFlush( xmesa );
}



#define EXTENSIONS "GLX_MESA_pixmap_colormap GLX_EXT_visual_info GLX_MESA_release_buffers"


/* GLX 1.1 and later */
const char *glXQueryExtensionsString( Display *dpy, int screen )
{
   static char *extensions = EXTENSIONS;
   return extensions;
}



/* GLX 1.1 and later */
const char *glXQueryServerString( Display *dpy, int screen, int name )
{
   static char *extensions = EXTENSIONS;
   static char *vendor = "Brian Paul";
   static char *version = "1.1 Mesa 2.2";

   switch (name) {
      case GLX_EXTENSIONS:
         return extensions;
      case GLX_VENDOR:
	 return vendor;
      case GLX_VERSION:
	 return version;
      default:
         return NULL;
   }
}



/* GLX 1.1 and later */
const char *glXGetClientString( Display *dpy, int name )
{
   static char *extensions = EXTENSIONS;
   static char *vendor = "Brian Paul";
   static char *version = "1.1 Mesa 2.2";

   switch (name) {
      case GLX_EXTENSIONS:
         return extensions;
      case GLX_VENDOR:
	 return vendor;
      case GLX_VERSION:
	 return version;
      default:
         return NULL;
   }
}



/*
 * Release the depth, stencil, accum buffers attached to a window prior
 * to destroying the window.
 */
Bool glXReleaseBuffersMESA( Display *dpy, Window w )
{
   GLint i, j;

   for (i=0;i<NumBuffers;i++) {
      if (BufferList[i]->frontbuffer==w) {
         XMesaDestroyBuffer( BufferList[i] );
	 for (j=i+1;j<NumBuffers;j++) {
	    BufferList[j-1] = BufferList[j];
	 }
	 NumBuffers--;
         return True;
      }
   }
   return False;
}

