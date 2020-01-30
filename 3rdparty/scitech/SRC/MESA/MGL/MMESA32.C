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
* Description:	Optimized 32bpp rendering functions.
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

IMPLEMENT_WRITE_SPAN(32,ARGB,ulong);
IMPLEMENT_WRITE_SPAN(32,ABGR,ulong);
IMPLEMENT_WRITE_SPAN(32,RGBA,ulong);
IMPLEMENT_WRITE_SPAN(32,BGRA,ulong);
IMPLEMENT_WRITE_SPAN_MONO(32,ulong);

/**********************************************************************/
/*****              Write arrays of pixels                        *****/
/**********************************************************************/

IMPLEMENT_WRITE_PIXELS(32,ARGB,ulong);
IMPLEMENT_WRITE_PIXELS(32,ABGR,ulong);
IMPLEMENT_WRITE_PIXELS(32,RGBA,ulong);
IMPLEMENT_WRITE_PIXELS(32,BGRA,ulong);
IMPLEMENT_WRITE_PIXELS_MONO(32,ulong);

/**********************************************************************/
/*****                 Read spans of pixels                       *****/
/**********************************************************************/

IMPLEMENT_READ_SPAN(32,ARGB,ulong);
IMPLEMENT_READ_SPAN(32,ABGR,ulong);
IMPLEMENT_READ_SPAN(32,RGBA,ulong);
IMPLEMENT_READ_SPAN(32,BGRA,ulong);

/**********************************************************************/
/*****                   Read arrays of pixels                    *****/
/**********************************************************************/

IMPLEMENT_READ_PIXELS(32,ARGB,ulong);
IMPLEMENT_READ_PIXELS(32,ABGR,ulong);
IMPLEMENT_READ_PIXELS(32,RGBA,ulong);
IMPLEMENT_READ_PIXELS(32,BGRA,ulong);

