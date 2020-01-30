/* $Id: fakeglx.h,v 1.2 1997/12/08 04:04:23 brianp Exp $ */

/*
 * Mesa 3-D graphics library
 * Version:  2.6
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
 * $Log: fakeglx.h,v $
 * Revision 1.2  1997/12/08 04:04:23  brianp
 * added Fake_glXCopySubBufferMESA() function
 *
 * Revision 1.1  1997/05/24 12:09:40  brianp
 * Initial revision
 *
 */


/*
 * GLX API functions which either call fake or real GLX implementations
 */



#ifndef FAKEGLX_H
#define FAKEGLX_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "GL/glx.h"


extern XVisualInfo *Fake_glXChooseVisual( Display *dpy,
                                          int screen, int *list );


extern int Fake_glXGetConfig( Display *dpy, XVisualInfo *visinfo,
                              int attrib, int *value );


extern GLXContext Fake_glXCreateContext( Display *dpy, XVisualInfo *visinfo,
                                         GLXContext shareList, Bool direct );


extern void Fake_glXDestroyContext( Display *dpy, GLXContext ctx );


extern void Fake_glXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
                                 GLuint mask );


extern Bool Fake_glXMakeCurrent( Display *dpy, GLXDrawable drawable,
                                 GLXContext ctx );


extern GLXContext Fake_glXGetCurrentContext( void );


extern GLXDrawable Fake_glXGetCurrentDrawable( void );


extern GLXPixmap Fake_glXCreateGLXPixmap( Display *dpy, XVisualInfo *visinfo,
                                          Pixmap pixmap );


extern void Fake_glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap );


extern Bool Fake_glXQueryExtension( Display *dpy, int *errorb, int *event );


extern Bool Fake_glXIsDirect( Display *dpy, GLXContext ctx );


extern void Fake_glXSwapBuffers( Display *dpy, GLXDrawable drawable );


extern void Fake_glXCopySubBufferMESA( Display *dpy, GLXDrawable drawable,
                                       int x, int y, int width, int height );


extern Bool Fake_glXQueryVersion( Display *dpy, int *maj, int *min );


extern void Fake_glXUseXFont( Font font, int first, int count, int listBase );


extern void Fake_glXWaitGL( void );


extern void Fake_glXWaitX( void );


/* GLX 1.1 and later */
extern const char *Fake_glXQueryExtensionsString( Display *dpy, int screen );


/* GLX 1.1 and later */
extern const char *Fake_glXQueryServerString( Display *dpy, int screen,
                                              int name );


/* GLX 1.1 and later */
extern const char *Fake_glXGetClientString( Display *dpy, int name );


#ifdef GLX_MESA_release_buffers
extern Bool Fake_glXReleaseBuffersMESA( Display *dpy, Window w );
#endif


#ifdef GLX_MESA_pixmap_colormap
extern GLXPixmap Fake_glXCreateGLXPixmapMESA( Display *dpy,
                                              XVisualInfo *visinfo,
                                              Pixmap pixmap, Colormap cmap );
#endif


#endif
