/****************************************************************************
*
*       Sprite Toolkit - A high performance Sprite library for the MGL
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
*
* Language:     ANSI C
* Environment:  32 bit flat mode only!
*
* Description:  Code to compile run length encoded transparent bitmaps used
*               to implement fast transparent sprite rendering in software.
*               We also provide a function to build a monochrome bitmap
*               mask for a bitmap in any format.
*
*
****************************************************************************/

#ifndef __RLEBMP_H
#define __RLEBMP_H

#ifndef __MGRAPH_H
#include "mgraph.h"
#endif

/*---------------------- Macros and type definitions ----------------------*/

/* Transparent RLE bitmap structure */

typedef struct {
    int         width;              /* Width of bitmap in pixels        */
    int         height;             /* Height of bitmap in pixels       */
    int         bitsPerPixel;       /* Pixel width                      */
    int         bytesPerLine;       /* Bytes per line value for surface */
    uchar       *surface;           /* Pointer to bitmap surface        */
    } rlebmp_t;

/*------------------------- Function Prototypes ---------------------------*/

#ifdef  __cplusplus
extern "C" {            /* Use "C" linkage when in C++ mode */
#endif

rlebmp_t *	SPR_buildRLEBitmap(bitmap_t *bitmap,color_t transparent);
void 		SPR_unloadRLEBitmap(rlebmp_t *bitmap);
void 		SPR_putRLEBitmap(MGLDC *dc,int dstX,int dstY,rlebmp_t *bitmap,ibool virtualDst);

#ifdef  __cplusplus
}                       /* End of "C" linkage when in C++ mode  */
#endif

#endif  /* __RLEBMP_H */

