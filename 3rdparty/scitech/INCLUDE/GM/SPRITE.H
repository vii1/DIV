/****************************************************************************
*
*						MegaGraph Graphics Library
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
* Language:		ANSI C
* Environment:  MSDOS/Win32 full screen
*
* Description:  Game Framework Sprite Manager for SciTech MGL. This library
* 				provides a simple yet powerful bitmap sprite manager for
*				automatically managing both software and hardware sprites
*				within the MGL and Game Framework. The sprite manager
*				is completely stand-alone and can be used independanly of
*				the main Game Framework functions.
*
*
****************************************************************************/

#ifndef	__SPRITE_H
#define	__SPRITE_H

#ifndef __MGRAPH_H
#include "mgraph.h"
#endif

/*------------------------- Function Prototypes ---------------------------*/

#ifdef	__cplusplus
#include "gm\sprbmp.hpp"
extern "C" {			/* Use "C" linkage when in C++ mode	*/
/* {secret} */
typedef SPR_Bitmap			SPR_bitmap;
#else
/* {secret} */
typedef struct SPR_Bitmap	SPR_bitmap;
#endif

/*---------------------------------------------------------------------------
 * Initialize the sprite manager. The useRLE flag is used to enable support
 * for run length encoding system memory bitmaps for faster transparent
 * blitting. However it may be faster on some architectures (like the DEC
 * Alpha) to do it with the MGL's internal functions rather than using
 * run length encoding.
 *-------------------------------------------------------------------------*/

ibool	MGLAPI SPR_mgrInit(MGLDC *dc,ibool useRLE);

/*---------------------------------------------------------------------------
 * Destroys the sprite manager. You should call this on cleanup.
 *-------------------------------------------------------------------------*/

void	MGLAPI SPR_mgrExit(void);

/*---------------------------------------------------------------------------
 * Empty the sprite manager of all bitmaps. Useful to destroy all bitmaps
 * so you can reload a new set such as when changing levels.
 *-------------------------------------------------------------------------*/

void	MGLAPI SPR_mgrEmpty(void);

/*---------------------------------------------------------------------------
 * Re-download all the bitmaps to the hardware. This function should be
 * called when your application is switched away from and have just been
 * re-activated. You should call this in your suspend application callback
 * if you are using the Game Framework.
 *-------------------------------------------------------------------------*/

void	MGLAPI SPR_mgrReloadHW(void);

/*---------------------------------------------------------------------------
 * Returns true if hardware offscreen memory is available and being used.
 *-------------------------------------------------------------------------*/

ibool 	MGLAPI SPR_mgrUsingOffscreenDC(void);

/*---------------------------------------------------------------------------
 * Returns true if the hardware offscreen memory cache is full and bitmaps
 * have started to overflow into system memory.
 *-------------------------------------------------------------------------*/

ibool 	MGLAPI SPR_mgrOffscreenCacheFull(void);

/*---------------------------------------------------------------------------
 * Add a new opaque bitmap to the sprite manager and download it to the
 * hardware. This function returns an SPR_bitmap pointer than you can use
 * to draw the bitmap at a later date.
 *-------------------------------------------------------------------------*/

SPR_bitmap * MGLAPI SPR_mgrAddOpaqueBitmap(bitmap_t *bmp);

/*---------------------------------------------------------------------------
 * Add a new transparent bitmap to the sprite manager and download it to the
 * hardware. This function returns an SPR_bitmap pointer than you can use
 * to draw the bitmap at a later date. This function automatically
 * determines the most efficient way to store and draw the bitmap depending
 * on the underlying hardware configuration.
 *-------------------------------------------------------------------------*/

SPR_bitmap * MGLAPI SPR_mgrAddTransparentBitmap(bitmap_t *bmp,color_t transparent);

/*---------------------------------------------------------------------------
 * Draw the bitmap at the specified location. Since the SPR_bitmap is
 * actually a C++ class, C++ programs can simply call the C++ member
 * functions for the bitmap class directly to draw it.   
 *-------------------------------------------------------------------------*/

void	MGLAPI SPR_draw(SPR_bitmap *bmp,int x,int y);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif	/* __cplusplus */

#endif	/* __SPRITE_H */

