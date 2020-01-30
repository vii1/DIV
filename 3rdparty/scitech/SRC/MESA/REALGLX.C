/* $Id: realglx.c,v 1.1 1997/05/24 12:10:47 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.3
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
 * $Log: realglx.c,v $
 * Revision 1.1  1997/05/24 12:10:47  brianp
 * Initial revision
 *
 */


/*
 * Real GLX-encoder functions.  Called from glxapi.c
 *
 * Steven Parker's code for the GLX client API functions should be
 * put in this file.
 *
 * Also, the main API functions in api.c should somehow hook into the
 * GLX-encoding functions...
 */



#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "realglx.h"



XVisualInfo *Real_glXChooseVisual( Display *dpy, int screen, int *list )
{
   return 0;
}



int Real_glXGetConfig( Display *dpy, XVisualInfo *visinfo,
                       int attrib, int *value )
{
   return 0;
}



GLXContext Real_glXCreateContext( Display *dpy, XVisualInfo *visinfo,
                                  GLXContext shareList, Bool direct )
{
   return 0;
}



void Real_glXDestroyContext( Display *dpy, GLXContext ctx )
{
}



void Real_glXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
                          GLuint mask )
{
}



Bool Real_glXMakeCurrent( Display *dpy, GLXDrawable drawable, GLXContext ctx )
{
   return 0;
}



GLXContext Real_glXGetCurrentContext( void )
{
   return 0;
}



GLXDrawable Real_glXGetCurrentDrawable( void )
{
   return 0;
}



GLXPixmap Real_glXCreateGLXPixmap( Display *dpy, XVisualInfo *visinfo,
                              Pixmap pixmap )
{
   return 0;
}


void Real_glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap )
{
}



Bool Real_glXQueryExtension( Display *dpy, int *errorb, int *event )
{
   return 0;
}



Bool Real_glXIsDirect( Display *dpy, GLXContext ctx )
{
   return 0;
}



void Real_glXSwapBuffers( Display *dpy, GLXDrawable drawable )
{
}



Bool Real_glXQueryVersion( Display *dpy, int *maj, int *min )
{
   return 0;
}



void Real_glXUseXFont( Font font, int first, int count, int listBase )
{
}


typedef struct {
   struct {
      int major_opcode;
   } codes;



} XExtDisplayInfo;


void Real_glXWaitGL( void )
{
}



void Real_glXWaitX( void )
{
}



/* GLX 1.1 and later */
const char *Real_glXQueryExtensionsString( Display *dpy, int screen )
{
   return 0;
}



/* GLX 1.1 and later */
const char *Real_glXQueryServerString( Display *dpy, int screen, int name )
{
   return 0;
}



/* GLX 1.1 and later */
const char *Real_glXGetClientString( Display *dpy, int name )
{
   return 0;
}
