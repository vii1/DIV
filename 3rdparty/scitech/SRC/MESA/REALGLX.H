/* $Id: realglx.h,v 1.1 1997/05/24 12:10:47 brianp Exp $ */

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
 * $Log: realglx.h,v $
 * Revision 1.1  1997/05/24 12:10:47  brianp
 * Initial revision
 *
 */


#ifndef REALGLX_H
#define REALGLX_H


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "GL/glx.h"



extern XVisualInfo *Real_glXChooseVisual( Display *dpy,
                                          int screen, int *list );


extern int Real_glXGetConfig( Display *dpy, XVisualInfo *visinfo,
                              int attrib, int *value );


extern GLXContext Real_glXCreateContext( Display *dpy, XVisualInfo *visinfo,
                                         GLXContext shareList, Bool direct );


extern void Real_glXDestroyContext( Display *dpy, GLXContext ctx );


extern void Real_glXCopyContext( Display *dpy, GLXContext src, GLXContext dst,
                                 GLuint mask );


extern Bool Real_glXMakeCurrent( Display *dpy, GLXDrawable drawable,
                                 GLXContext ctx );


extern GLXContext Real_glXGetCurrentContext( void );


extern GLXDrawable Real_glXGetCurrentDrawable( void );


extern GLXPixmap Real_glXCreateGLXPixmap( Display *dpy, XVisualInfo *visinfo,
                                          Pixmap pixmap );


extern void Real_glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap );


extern Bool Real_glXQueryExtension( Display *dpy, int *errorb, int *event );


extern Bool Real_glXIsDirect( Display *dpy, GLXContext ctx );


extern void Real_glXSwapBuffers( Display *dpy, GLXDrawable drawable );


extern Bool Real_glXQueryVersion( Display *dpy, int *maj, int *min );


extern void Real_glXUseXFont( Font font, int first, int count, int listBase );


extern void Real_glXWaitGL( void );


extern void Real_glXWaitX( void );


/* GLX 1.1 and later */
extern const char *Real_glXQueryExtensionsString( Display *dpy, int screen );


/* GLX 1.1 and later */
extern const char *Real_glXQueryServerString( Display *dpy, int screen,
                                              int name );


/* GLX 1.1 and later */
extern const char *Real_glXGetClientString( Display *dpy, int name );


#endif
