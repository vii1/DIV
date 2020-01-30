/* $Id: xfonts.c,v 1.8 1997/05/24 12:11:25 brianp Exp $ */

/* xfonts.c -- glXUseXFont() for Mesa
   Copyright (C) 1995 Thorsten.Ohl @ Physik.TH-Darmstadt.de

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * $Log: xfonts.c,v $
 * Revision 1.8  1997/05/24 12:11:25  brianp
 * renamed glXUseXFont() to Fake_glXUseXFont()
 *
 * Revision 1.7  1997/03/13 03:14:34  brianp
 * removed unused variable 'c' from dump_font_struct()
 *
 * Revision 1.6  1996/11/20 03:05:14  brianp
 * added #include "GL/glx.h"
 *
 * Revision 1.5  1996/10/22 01:02:46  brianp
 * call XFreeFontInfo instead of XFreeFont (again)
 *
 * Revision 1.4  1996/10/09 03:07:42  brianp
 * replaced XFreeFontInfo with XFreeFont
 *
 * Revision 1.3  1996/09/19 03:41:48  brianp
 * updated for new X/Mesa changes
 *
 * Revision 1.2  1996/09/15 01:50:05  brianp
 * fixed #includes
 *
 * Revision 1.1  1996/09/13 01:38:16  brianp
 * Initial revision
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "GL/gl.h"
#include "GL/glx.h"
#include "GL/xmesa.h"
#include "macros.h"


/* Some debugging info.  */

#ifdef DEBUG
#include <ctype.h>

int debug_xfonts = 0;

static void
dump_char_struct (XCharStruct *ch, char *prefix)
{
  printf ("%slbearing = %d, rbearing = %d, width = %d\n",
	  prefix, ch->lbearing, ch->rbearing, ch->width);
  printf ("%sascent = %d, descent = %d, attributes = %u\n",
	  prefix, ch->ascent, ch->descent, ch->attributes);
}

static void
dump_font_struct (XFontStruct *font)
{
  printf ("ascent = %d, descent = %d\n", font->ascent, font->descent);
  printf ("char_or_byte2 = (%d,%d)\n",
	  font->min_char_or_byte2, font->max_char_or_byte2);
  printf ("byte1 = (%d,%d)\n", font->min_byte1, font->max_byte1);
  printf ("all_chars_exist = %s\n", font->all_chars_exist ? "True" : "False");
  printf ("default_char = %c (\\%03o)\n",
	  isprint (font->default_char) ? font->default_char : ' ',
	  font->default_char);
  dump_char_struct (&font->min_bounds, "min> ");
  dump_char_struct (&font->max_bounds, "max> ");
#if 0
  for (c = font->min_char_or_byte2; c <= font->max_char_or_byte2; c++)
    {
      char prefix[8];
      sprintf (prefix, "%d> ", c);
      dump_char_struct (&font->per_char[c], prefix);
    }
#endif
}

static void
dump_bitmap (unsigned int width, unsigned int height, GLubyte *bitmap)
{
  int x, y;

  printf ("    ");
  for (x = 0; x < 8*width; x++)
    printf ("%o", 7 - (x % 8));
  putchar ('\n');
  for (y = 0; y < height; y++)
    {
      printf ("%3o:", y);
      for (x = 0; x < 8*width; x++)
        putchar ((bitmap[width*(height - y - 1) + x/8] & (1 << (7 - (x % 8))))
		 ? '*' : '.');
      printf ("   ");
      for (x = 0; x < width; x++)
	printf ("0x%02x, ", bitmap[width*(height - y - 1) + x]);
      putchar ('\n');
    }
}
#endif /* DEBUG */


/* Implementation.  */

/* Fill a BITMAP with a character C from thew current font
   in the graphics context GC.  WIDTH is the width in bytes
   and HEIGHT is the height in bits.

   Note that the generated bitmaps must be used with

	glPixelStorei (GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei (GL_UNPACK_LSB_FIRST, GL_FALSE);
	glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei (GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

   Possible optimizations:

     * use only one reusable pixmap with the maximum dimensions.
     * draw the entire font into a single pixmap (careful with
       proportional fonts!).
*/

static void
fill_bitmap (Display *dpy, Window win, GC gc,
	     unsigned int width, unsigned int height,
	     int x0, int y0, char c, GLubyte *bitmap)
{
  XImage *image;
  int x, y;
  Pixmap pixmap;

  pixmap = XCreatePixmap (dpy, win, 8*width, height, 1);
  XSetForeground(dpy, gc, 0);
  XFillRectangle (dpy, pixmap, gc, 0, 0, 8*width, height);
  XSetForeground(dpy, gc, 1);
  XDrawString (dpy, pixmap, gc, x0, y0, &c, 1);

  image = XGetImage (dpy, pixmap, 0, 0, 8*width, height, 1, XYPixmap);

  /* Fill the bitmap (X11 and OpenGL are upside down wrt each other).  */
  for (y = 0; y < height; y++)
    for (x = 0; x < 8*width; x++)
      if (XGetPixel (image, x, y))
	bitmap[width*(height - y - 1) + x/8] |= (1 << (7 - (x % 8)));
  
  XFreePixmap (dpy, pixmap);
  XDestroyImage (image);
}

#include "xmesaP.h"
#include "context.h"
#include "fakeglx.h"

void Fake_glXUseXFont( Font font, int first, int count, int listbase )
{
  XMesaContext CC;
  Display *dpy;
  Window win;
  Pixmap pixmap;
  GC gc;
  XGCValues values;
  unsigned long valuemask;

  XFontStruct *fs;

  GLint swapbytes, lsbfirst, rowlength;
  GLint skiprows, skippixels, alignment;

  unsigned int max_width, max_height, max_bm_width, max_bm_height;
  GLubyte *bm;

  int i;

  CC = XMesaGetCurrentContext();  
  dpy = CC->display;
  win = CC->xm_buffer->frontbuffer;

  fs = XQueryFont (dpy, font);  
  if (!fs)
    {
      gl_error (CC->gl_ctx, GL_INVALID_VALUE,
		"Couldn't get font structure information");
      return;
    }

  /* Allocate a bitmap that can fit all characters.  */
  max_width = fs->max_bounds.rbearing - fs->min_bounds.lbearing;
  max_height = fs->max_bounds.ascent + fs->max_bounds.descent;
  max_bm_width = (max_width + 7) / 8;
  max_bm_height = max_height;

  bm = (GLubyte *) malloc ((max_bm_width * max_bm_height) * sizeof (GLubyte));
  if (!bm)
    {
      gl_error (CC->gl_ctx, GL_OUT_OF_MEMORY,
		"Couldn't allocate bitmap in glXUseXFont()");
      return;
    }

  /* Save the current packing mode for bitmaps.  */
  glGetIntegerv	(GL_UNPACK_SWAP_BYTES, &swapbytes);
  glGetIntegerv	(GL_UNPACK_LSB_FIRST, &lsbfirst);
  glGetIntegerv	(GL_UNPACK_ROW_LENGTH, &rowlength);
  glGetIntegerv	(GL_UNPACK_SKIP_ROWS, &skiprows);
  glGetIntegerv	(GL_UNPACK_SKIP_PIXELS, &skippixels);
  glGetIntegerv	(GL_UNPACK_ALIGNMENT, &alignment);

  /* Enforce a standard packing mode which is compatible with
     fill_bitmap() from above.  This is actually the default mode,
     except for the (non)alignment.  */
  glPixelStorei	(GL_UNPACK_SWAP_BYTES, GL_FALSE);
  glPixelStorei	(GL_UNPACK_LSB_FIRST, GL_FALSE);
  glPixelStorei	(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei	(GL_UNPACK_SKIP_ROWS, 0);
  glPixelStorei	(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei	(GL_UNPACK_ALIGNMENT, 1);

  pixmap = XCreatePixmap (dpy, win, 10, 10, 1);
  values.foreground = BlackPixel (dpy, DefaultScreen (dpy));
  values.background = WhitePixel (dpy, DefaultScreen (dpy));
  values.font = fs->fid;
  valuemask = GCForeground | GCBackground | GCFont;
  gc = XCreateGC (dpy, pixmap, valuemask, &values);
  XFreePixmap (dpy, pixmap);

#ifdef DEBUG
  if (debug_xfonts)
    dump_font_struct (fs);
#endif
  
  for (i = 0; i < count; i++)
    {
      unsigned int width, height, bm_width, bm_height;
      GLfloat x0, y0, dx, dy;
      XCharStruct *ch;
      int x, y;
      int c = first + i;
      int list = listbase + i;

      if (fs->per_char
	  && (c >= fs->min_char_or_byte2) && (c <= fs->max_char_or_byte2))
	ch = &fs->per_char[c-fs->min_char_or_byte2];
      else
	ch = &fs->max_bounds;

#ifdef DEBUG
      if (debug_xfonts)
	{
	  char s[7];
	  sprintf (s, isprint (c) ? "%c> " : "\\%03o> ", c);
	  dump_char_struct (ch, s);
	}
#endif
      
      /* glBitmap()' parameters:
	 straight from the glXUseXFont(3) manpage.  */
      width = ch->rbearing - ch->lbearing;
      height = ch->ascent + ch->descent;
      x0 = - ch->lbearing;
      y0 = ch->descent - 1;
      dx = ch->width;
      dy = 0;

      /* X11's starting point.  */
      x = - ch->lbearing;
      y = ch->ascent;
      
      /* Round the width to a multiple of eight.  We will use this also
	 for the pixmap for capturing the X11 font.  This is slightly
	 inefficient, but it makes the OpenGL part real easy.  */
      bm_width = (width + 7) / 8;
      bm_height = height;

      glNewList (list, GL_COMPILE);
        if ((c >= fs->min_char_or_byte2) && (c <= fs->max_char_or_byte2)
	    && (bm_width > 0) && (bm_height > 0))
	  {
	    MEMSET (bm, '\0', bm_width * bm_height);
	    fill_bitmap (dpy, win, gc, bm_width, bm_height, x, y, c, bm);
	    glBitmap (width, height, x0, y0, dx, dy, bm);
#ifdef DEBUG
	    if (debug_xfonts)
	      {
		printf ("width/height = %d/%d\n", width, height);
		printf ("bm_width/bm_height = %d/%d\n", bm_width, bm_height);
		dump_bitmap (bm_width, bm_height, bm);
	      }
#endif
	  }
	else
	  glBitmap (0, 0, 0.0, 0.0, dx, dy, NULL);
      glEndList ();
    }

  free (bm);
  XFreeFontInfo( NULL, fs, 0 );
  XFreeGC (dpy, gc);

  /* Restore saved packing modes.  */    
  glPixelStorei(GL_UNPACK_SWAP_BYTES, swapbytes);
  glPixelStorei(GL_UNPACK_LSB_FIRST, lsbfirst);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, rowlength);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, skiprows);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, skippixels);
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}

/* The End. */
