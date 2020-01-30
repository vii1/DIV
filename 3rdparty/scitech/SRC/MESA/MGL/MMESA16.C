/****************************************************************************
*
*				 	   Mesa bindings for SciTech MGL
*
*               Copyright (C) 1996-1998 SciTech Software, Inc.
*                            All rights reserved.
*
* Language:     ANSI C
* Environment:	Any
*
* Description:	Optimized 16bpp rendering functions.
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
****************************************************************************/

#include "mgl/mmesap.h"

/*------------------------- Implementation --------------------------------*/

#pragma warn -par
#include "mgl/mmesai.c"

/**********************************************************************/
/*****            Write spans of pixels                           *****/
/**********************************************************************/

IMPLEMENT_WRITE_SPAN(16,555,ushort);
IMPLEMENT_WRITE_SPAN(16,565,ushort);
IMPLEMENT_WRITE_SPAN_DITHER(16,DITHER555,ushort);
IMPLEMENT_WRITE_SPAN_DITHER(16,DITHER565,ushort);
IMPLEMENT_WRITE_SPAN_MONO(16,ushort);
IMPLEMENT_WRITE_SPAN_MONO_DITHER(16,DITHER555,ushort);
IMPLEMENT_WRITE_SPAN_MONO_DITHER(16,DITHER565,ushort);

/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/

IMPLEMENT_WRITE_PIXELS(16,555,ushort);
IMPLEMENT_WRITE_PIXELS(16,565,ushort);
IMPLEMENT_WRITE_PIXELS_DITHER(16,DITHER555,ushort);
IMPLEMENT_WRITE_PIXELS_DITHER(16,DITHER565,ushort);
IMPLEMENT_WRITE_PIXELS_MONO(16,ushort);
IMPLEMENT_WRITE_PIXELS_MONO_DITHER(16,DITHER555,ushort);
IMPLEMENT_WRITE_PIXELS_MONO_DITHER(16,DITHER565,ushort);

/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/

IMPLEMENT_READ_SPAN(16,555,ushort);
IMPLEMENT_READ_SPAN(16,565,ushort);

/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

IMPLEMENT_READ_PIXELS(16,555,ushort);
IMPLEMENT_READ_PIXELS(16,565,ushort);

