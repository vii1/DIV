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
* Description:  SPR_BitmapManager class, which manages all bitmaps in
*               host memory and offscreen display memory.
*
*
****************************************************************************/

#include "sprmgr.hpp"
#pragma hdrstop

/*------------------------- Implementation --------------------------------*/

SPR_BitmapManager::SPR_BitmapManager(MGLDC *dc,ibool useRLE)
    : dc(dc), useRLE(useRLE)
/****************************************************************************
*
* Function:     SPR_BitmapManager::SPR_BitmapManager
* Parameters:   dc          - Device context for drawing all bitmaps to
*               quickReload - True if to allow quick reloaded to hardware
*
* Description:  Constructor for the bitmap manager class
*
****************************************************************************/
{
    linearDC = false;
	useBitBlt = false;
    useSrcTrans = false;
    useMskTrans = false;
    offdc = NULL;
    offdcFull = false;
    flags = MGL_getHardwareFlags(dc);
    if (!MGL_isDisplayDC(dc) || (flags & MGL_HW_SCR_BLT) == MGL_HW_NONE)
		return;

	// We are drawing to a hardware display DC which has at least
    // hardware blitting capabilities, so try to create an offscreen
    // display device context for caching bitmaps.
    if ((offdc = MGL_createLinearOffscreenDC()) != NULL)
        linearDC = true;
    else
        offdc = MGL_createOffscreenDC();
    if (!offdc)
        return;

    // Determine the hardware capabilities
    useBitBlt = true;           // We have hardware bitBlt capabilities
    if (flags & MGL_HW_SRCTRANS_BLT)
        useSrcTrans = true;     // We have hardware source transparency
    else if (flags & MGL_HW_MONO_BLT)
        useMskTrans = true;     // We have hardware masked transparency
    empty();                    // Empty bitmap manager and offscreen mem
}

SPR_BitmapManager::~SPR_BitmapManager()
{
    if (offdc)
        MGL_destroyDC(offdc);
}

void SPR_BitmapManager::empty()
/****************************************************************************
*
* Function:     SPR_BitmapManager::empty
*
* Description:  Empties the bitmap manager and destroys all bitmaps,
*               allowing a new set of bitmaps to be loaded.
*
****************************************************************************/
{
    bitmaps.empty();
    offdcFull = false;
	if (offdc) {
        if (linearDC) {
            linearCur = 0;
            linearMax = offdc->mi.bytesPerLine * (offdc->mi.yRes+1) - 1;
            }
        else {
            freeSpace = MGLRect(0,0,offdc->mi.xRes+1,offdc->mi.yRes+1);
            }
        }
}

ibool SPR_BitmapManager::allocateLinearMem(bitmap_t *bmp,ulong& startOff)
/****************************************************************************
*
* Function:     SPR_BitmapManager::allocateLinearMem
* Parameters:   bmp         - Bitmap to get size info from
* Returns:      startOff    - Place to store starting offset in display mem
*
* Description:  Locates a free space in offscreen display memory to cache
*               the bitmap.
*
****************************************************************************/
{
    long size;

    if (bmp->bitsPerPixel == 8)
        size = bmp->width * bmp->height;
    else if (bmp->bitsPerPixel == 15 || bmp->bitsPerPixel == 16)
        size = bmp->width * bmp->height * 2;
    else if (bmp->bitsPerPixel == 24)
        size = bmp->width * bmp->height * 3;
    else size = bmp->width * bmp->height * 4;
    if ((linearCur + size) > linearMax)
		return false;
	startOff = linearCur;

	// Note that we must align the data in video memory to a 64 bit
	// boundary.
	linearCur = (linearCur + size + 7) & ~7;
	return true;
}

PRIVATE int         bmpWidth;       // Height of bitmap to allocate
PRIVATE int         bmpHeight;      // Width of bitmap to allocate
PRIVATE ibool        foundSpace;     // True if we found a spot
PRIVATE MGLRect     bestBounds;     // Bounds for the best fit rectangle
PRIVATE MGLRect     testRect;       // Rectangle we are testing
PRIVATE MGLRegion   *rgn;           // Region represting the allocated mem

PRIVATE void ASMAPI expandVert(const rect_t *r)
/****************************************************************************
*
* Function:     expandVert
* Parameters:   r   - Rectangle to test
*
* Description:  Tries to expand the testRect rectangle vertically with
*               all rectangles vertically adjacent to it.
*
****************************************************************************/
{
	if (r->top == testRect.bottom() && r->left <= testRect.left() &&
			r->right >= testRect.left() + bmpWidth) {
		testRect.bottom() = r->bottom;
        }
}

PRIVATE void ASMAPI findFreeRect(const rect_t *r)
/****************************************************************************
*
* Function:     findFreeRect
* Parameters:   r   - Rectangle to test
*
* Description:  Checks the test rectangle against the size of our bitmap,
*               and if it is larger, then we have found some free space.
*               We also check t find the first rectangle with the best
*               fit for the bitmap. Note that the bestBounds returned is
*               the bounds of the free space that we found, and will most
*               likely be larger than the size of our bitmap!
*
****************************************************************************/
{
	if ((r->right - r->left) >= bmpWidth) {
		testRect = *r;
        if (testRect.height() < bmpHeight) {
            // We have found a rectangle big enough in the width to hold
			// our bitmap, so we need to search for and coallesce all
            // rectangles below this rectangle that are also wide enough
            // to hold our bitmap.
            rgn->traverse(expandVert);
            }
        if (testRect.height() >= bmpHeight) {
            // We have found a rectangle big enough to hold our bitmap
			if (foundSpace) {
                if (testRect.width() < bestBounds.width()
                    && testRect.height() < bestBounds.height())
                    bestBounds = testRect;
                }
            else {
                bestBounds = testRect;
                foundSpace = true;
                }
            }
        }
}

ibool SPR_BitmapManager::allocateRectMem(bitmap_t *bmp,MGLRect& bounds)
/****************************************************************************
*
* Function:     SPR_BitmapManager::allocateRectMem
* Parameters:   filename    - Name of the bitmap to load
* Returns:      Pointer to loaded bitmap, or NULL on error.
*
* Description:  Locates a free space in offscreen display memory to cache
*               the bitmap.
*
****************************************************************************/
{
    // Traverse the region of empty space (always traverses left to right,
    // top to bottom) looking for the best fit rectangle for our bitmap.
    // If we find some free space, allocate the space and remove it from
    // the freespace region.
    bmpWidth = bmp->width;
    bmpHeight = bmp->height;
    foundSpace = false;
    rgn = &freeSpace;
    freeSpace.traverse(findFreeRect);
    if (foundSpace) {
        bounds.left() = bestBounds.left();
        bounds.top() = bestBounds.top();
        bounds.right() = bounds.left() + bmp->width;
		bounds.bottom() = bounds.top() + bmp->height;
        freeSpace -= bounds;
        freeSpace.optimize();
        return true;
        }
    return false;
}

SPR_Bitmap *SPR_BitmapManager::addOpaqueBitmap(bitmap_t *bmp)
/****************************************************************************
*
* Function:     SPR_BitmapManager::addOpaqueBitmap
* Parameters:   bmp	- Bitmap to add to bitmap manager
* Returns:      Pointer to loaded bitmap, or NULL on error.
*
* Description:  Adds the opaque bitmap to the bitmap manager for later
*				use. If the bitmap is cached in offscreen memory it is
*				downloaded to the hardware.
*
****************************************************************************/
{
    ulong       startOff;
    MGLRect     bounds;
	SPR_Bitmap	*bitmap = NULL;

	if (useBitBlt && linearDC) {
		if (allocateLinearMem(bmp,startOff))
			bitmap = new SPR_LinOffscreenBitmap(dc,bmp,offdc,startOff);
		}
	else if (useBitBlt) {
		if (allocateRectMem(bmp,bounds))
			bitmap = new SPR_OffscreenBitmap(dc,bmp,offdc,bounds);
		}
	if (!bitmap)
		bitmap = new SPR_HostBitmap(dc,bmp);
	if (bitmap && bitmap->isValid()) {
        bitmap->downloadToHW();
        bitmaps.addToTail(bitmap);
        }
    return bitmap;
}

SPR_Bitmap *SPR_BitmapManager::addTransparentBitmap(bitmap_t *bmp,
	color_t transparent)
/****************************************************************************
*
* Function:     SPR_BitmapManager::loadTransparentBitmap
* Parameters:   filename    - Name of the bitmap to load
*               transparent - Transparent color for the bitmap
* Returns:      Pointer to loaded bitmap, or NULL on error.
*
* Description:  Attempts to load the specified bitmap, and will create
*               the appropriate bitmap given the current hardware
*               characteristics. This function creates a bitmap that will
*               be drawn as transparent.
*
****************************************************************************/
{
    ulong       startOff;
	MGLRect     bounds;
	SPR_Bitmap	*bitmap = NULL;

	if (useSrcTrans || useMskTrans) {
		if (linearDC) {
			if (allocateLinearMem(bmp,startOff)) {
				if (useSrcTrans)
					bitmap = new SPR_LinOffscreenTransparentBitmap(dc,bmp,
						offdc,startOff,transparent,useSrcTrans);
				else
					bitmap = new SPR_LinOffscreenMaskedBitmap(dc,bmp,
						offdc,startOff,transparent);
				}
			else
				offdcFull = true;
			}
		else {
			if (allocateRectMem(bmp,bounds)) {
				if (useSrcTrans)
					bitmap = new SPR_OffscreenTransparentBitmap(dc,bmp,offdc,
						bounds,transparent,useSrcTrans);
				else
					bitmap = new SPR_OffscreenMaskedBitmap(dc,bmp,offdc,
						bounds,transparent);
				}
			else
				offdcFull = true;
			}
		}
	if (!bitmap)
		bitmap = new SPR_HostSrcTransparentBitmap(dc,bmp,transparent,useRLE);
	if (bitmap && bitmap->isValid()) {
		bitmap->downloadToHW();
		bitmaps.addToTail(bitmap);
		}
	return bitmap;
}

ibool SPR_BitmapManager::reloadHW()
/****************************************************************************
*
* Function:     SPR_BitmapManager::reloadHW
* Returns:      True if hardware reloaded, false if bitmaps no fast reload
*
* Description:  Attempts to reload all the bitmaps to the hardware. If the
*               fast reload option is on all source bitmaps will still be
*               cached in system memory so we can do a quick reload.
*               Otherwise it is up to the application to reload all the
*               bitmaps for the level from disk.
*
*               Keeping the source bitmaps around is useful when running
*               under fullscreen modes under Windows where you will need
*               to re-load the hardware when the user Alt-Tabs back to your
*               application. This will take up a bit more memory, but will
*               provide the fastest response.
*
****************************************************************************/
{
	for (SPR_BitmapListIter i(bitmaps); i; i++)
        i.node()->downloadToHW();
    return true;
}


