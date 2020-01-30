/****************************************************************************
*
*                       MegaGraph Graphics Library
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
* Language:     C++ 3.0
* Environment:  32 bit flat model only!
*
* Description:  C API bindings for the Sprite Manager.
*
*
****************************************************************************/

#include "sprmgr.hpp"
#pragma hdrstop
#include "gm/sprite.h"

/*---------------------------- Global Variables ---------------------------*/

PRIVATE	SPR_BitmapManager	*bmpMgr = NULL;

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
DESCRIPTION:
Initializes the Game Framework Sprite Manager

PARAMETERS:
dc		- MGL device context to use
useRLE	- True if Run Length Encoding should be used for system bitmaps

HEADER:
gm/sprite.h

RETURNS:
True on success, false on failure to initialize.

REMARKS:
This function initializes the Sprite Manager library and sets up for storing
bitmaps with the sprite manager. The device context you pass in can be any
MGL device context, but usually it should be an MGL display device context
or an MGL memory device context. The Sprite Manager will automatically
integrogate the capabilities of the device context, and if the device
context supports the creation of an offscreen device context for storing
sprites in video memory, an offscreen device context will be created
and managed by the sprite manager.

The sprite manager automatically does all the work to keep track of which
bitmaps are loaded in video memory and which are loaded in system memory. If
there is not enough video memory left to store a sprite when you add it to
the Sprite Manager, that sprite is stored in system memory automatically.
When the sprites are loaded in system memory, if you pass in true to the
useRLE parameter the Sprite Manager will compress all the system memory
sprites using a Run Length Encoding algorithm which can lead to faster
performance when blitting from system memory to video memory. If you pass
in false to this parameter, the Sprite Manager will simply store the bitmaps
in system memory and display them using MGL_putBitmap and
MGL_putTransparentBitmap.

Note:	When you add bitmaps to the Sprite Manager, those bitmaps are then
 		/owned/ by the Sprite Manager and will be automatically destroyed
        with MGL_unloadBitmap when the Sprite Manager is emtpied of all
        bitmaps.

Note:	If you switch fullscreen graphics modes or you switch from fullscreen
		modes to windowed modes, you /must/ call SPR_mgrExit to destroy
        the bitmap manager and re-initialize it with the newly created
        device contexts. This is necessary because the capabilities of the
        new device context and the amount of available offscreen video memory
        (if any) will be different in the new graphics mode and all the
        sprites will have to be re-loaded into the Sprite Manager.

SEE ALSO:
SPR_mgrExit
***************************************************************************/
ibool MGLAPI SPR_mgrInit(
	MGLDC *dc,
    ibool useRLE)
{
	bmpMgr = new SPR_BitmapManager(dc,useRLE);
	return (bmpMgr != NULL);
}

/****************************************************************************
DESCRIPTION:
Shuts down the Game Framework Sprite Manager

HEADER:
gm/sprite.h

REMARKS:
This function shuts down the Sprite Manager, and destroys all bitmaps
currently own by the Sprite Manager.

SEE ALSO:
SPR_mgrInit, SPR_mgrEmpty
***************************************************************************/
void MGLAPI SPR_mgrExit(void)
{ delete bmpMgr; }

/****************************************************************************
DESCRIPTION:
Empties Sprite Manager of all loaded bitmaps

HEADER:
gm/sprite.h

REMARKS:
This function empties the Sprite Manager of all currently loaded bitmaps
and deallocates the memory owned by those bitmaps. This function is most
useful to clearing the Sprite Manager of all bitmaps when moving from one
level to another in your game.

SEE ALSO:
SPR_mgrInit, SPR_mgrEmpty
***************************************************************************/
void MGLAPI SPR_mgrEmpty(void)
{ bmpMgr->empty(); }

/****************************************************************************
DESCRIPTION:
Reloads all offscreen sprites to the hardware

HEADER:
gm/sprite.h

REMARKS:
This function re-downloads all sprites that are stored in offscreen video
memory to the offscreen device context. This function is usually called
in the Game Framework suspend application callback, since on re-activation
and switching back to GDI mode all the offscreen memory has been lost and
needs to be reloaded.
***************************************************************************/
void MGLAPI SPR_mgrReloadHW(void)
{ bmpMgr->reloadHW(); }

/****************************************************************************
DESCRIPTION:
Determines if the Sprite Manager is using a hardware offscreen DC.

HEADER:
gm/sprite.h

RETURNS:
True if Sprite Manager is using a hardware offscreen device context.

SEE ALSO:
SPR_mgrOffscreenCacheFull
***************************************************************************/
ibool MGLAPI SPR_mgrUsingOffscreenDC(void)
{ return bmpMgr->usingOffscreenDC(); }

/****************************************************************************
DESCRIPTION:
Determines if the Sprite Manager offscreen device context is full.

HEADER:
gm/sprite.h

RETURNS:
True if Sprite Manager offscreen device context is full.

REMARKS:
This function is used to determine if the Sprite Manager's offscreen
device context (if it is using one) is full and cannot accept any more
bitmaps. This function is most useful for determining if the offscreen
video memory is full and that the Sprite Manager has started overflowing
the bitmaps into system memory. In this case you can expect the performance
of your application to drop when the offscreen video memory is full.    

SEE ALSO:
SPR_mgrUsingOffscreenDC
***************************************************************************/
ibool MGLAPI SPR_mgrOffscreenCacheFull(void)
{ return bmpMgr->offscreenCacheFull(); }

/****************************************************************************
DESCRIPTION:
Adds an opaque or non-transparent bitmap to the Sprite Manager

PARAMETERS:
bmp		- MGL bitmap to add to the Sprite Manager

HEADER:
gm/sprite.h

RETURNS:
Pointer to the loaded sprite object

REMARKS:
This function adds a new opaque or non-transparent bitmap to the Sprite
Manager. When you add the bitmap, the Sprite Manager takes over ownership
of the memory allocated to the bitmap and you /must/ not call
MGL_unloadBitmap on the bitmap to free the memory. Once the bitmap has
been added, you can then draw the bitmap by calling SPR_draw and pass in
the pointer to the bitmap returned by this function.

Note:	The bitmap added to the Sprite Manager /must/ be in the same
    	format at the device context that the bitmaps will be copied to.
        Hence you should add code to your program to do any necessary
        conversions to the destination pixel format for the bitmaps (see
        the Fox & Bear sample program which contains code to do this).

Note:	The Sprite Manager also maintains ownership of the SPR_bitmap object
		that is returned by this function, and this object will be destroyed
        automatically when you empty or exit the Sprite Manager.

SEE ALSO:
SPR_mgrAddTransparentBitmap, SPR_draw, SPR_mgrEmpty
***************************************************************************/
SPR_bitmap * MGLAPI SPR_mgrAddOpaqueBitmap(
	bitmap_t *bmp)
{ return bmpMgr->addOpaqueBitmap(bmp); }

/****************************************************************************
DESCRIPTION:
Adds a source transparent bitmap to the Sprite Manager

PARAMETERS:
bmp			- MGL bitmap to add to the Sprite Manager
transparent	- Transparent color for the bitmap

HEADER:
gm/sprite.h

RETURNS:
Pointer to the loaded sprite object

REMARKS:
This function adds a new transparent bitmap to the Sprite Manager. When you
add the bitmap, the Sprite Manager takes over ownership of the memory allocated
to the bitmap and you /must/ not call MGL_unloadBitmap on the bitmap to free
the memory. Once the bitmap has been added, you can then draw the bitmap by
calling SPR_draw and pass in the pointer to the bitmap returned by this
function.

Note that the transparent color you pass in is a /source transparent/ color,
which means that pixels in the source bitmap that match the transparent
color will not be drawn when you call SPR_draw for the returned sprite
object (ie: they are transparent).

Note:	The bitmap added to the Sprite Manager /must/ be in the same
    	format at the device context that the bitmaps will be copied to.
        Hence you should add code to your program to do any necessary
        conversions to the destination pixel format for the bitmaps (see
        the Fox & Bear sample program which contains code to do this).

Note:	The Sprite Manager also maintains ownership of the SPR_bitmap object
		that is returned by this function, and this object will be destroyed
        automatically when you empty or exit the Sprite Manager.

SEE ALSO:
SPR_mgrAddOpaqueBitmap, SPR_draw, SPR_mgrEmpty
***************************************************************************/
SPR_bitmap * MGLAPI SPR_mgrAddTransparentBitmap(
	bitmap_t *bmp,
    color_t transparent)
{ return bmpMgr->addTransparentBitmap(bmp,transparent); }

/****************************************************************************
DESCRIPTION:
Draws the sprite object at the specified location.

PARAMETERS:
bmp		- Sprite object to draw
x		- X coordinate to draw the sprite at
y		- Y coordinate to draw the sprite at

HEADER:
gm/sprite.h

REMARKS:
This function draws the sprite object at the specified (x,y) location on
the device context currently bound to the Sprite Manager (ie: the device
context you used when you initialized it with SPR_mgrInit). The sprite
is drawn using the attributes of the sprite when you added it (ie: opaque
or source transparent).

SEE ALSO:
SPR_mgrAddOpaqueBitmap, SPR_mgrAddTransparentBitmap, 
***************************************************************************/
void MGLAPI SPR_draw(
	SPR_bitmap *bmp,
    int x,
    int y)
{ bmp->draw(x,y); }


