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
* Environment:  32 bit flat mode only!
*
* Description:  Header for the SPR_BitmapManager class.
*
*
****************************************************************************/

#ifndef __BMPMGR_HPP
#define __BMPMGR_HPP

#include "gm/sprbmp.hpp"

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// Class to manage a list of bitmaps used for a particular level. This class
// can create either opaque or transparent bitmap objects for the level.
// If the hardware supports accelerated bitmap rendering it will cache as
// many of those in an MGL offscreen memory DC as possible. If there is
// no hardware support (or we have run out of offscreen display memory)
// bitmaps will be created and rendered in software. For source transparent
// software bitmaps, we actually compile them into RLE encoded bitmaps
// and render them directly to the display surface for maximum speed.
// Note also that you can make your main device context a system memory
// device context, in which case no hardware rendering will be used.
//
// When managing rectangular offscreen memory device contexts, we use the
// MGL complex region manipulation routines to keep track of what has been
// allocated so that we can tuck small bitmaps into any gaps that may
// arise. We always allocate bitmaps in a left to right and top to bottom
// fashion, so you should experiment to find the best method of loading
// your bitmaps to minimise the amount of wasted space. If we are using
// a linear offscreen DC we dont have any wasted space.
//
// This class also manages all the bitmaps in the level, and will
// automatically delete all the bitmaps when the bitmap manager is emptied.
//---------------------------------------------------------------------------

class SPR_BitmapManager {
protected:
    MGLDC           *dc;            // Main MGL display DC to draw onto
    MGLDC           *offdc;         // Offscreen DC for caching bitmaps
    SPR_BitmapList	bitmaps;        // List of all loaded bitmaps for level
    long            flags;          // Hardware access flags for the DC
    ibool            linearDC;       // True if using linear offscreen DC
	ibool            useBitBlt;      // Use hardware opaque bitBlt
    ibool            useSrcTrans;    // Use hardware source transparency
	ibool            useMskTrans;    // Use hardware masked transparency
	ibool            offdcFull;      // True if offscreen DC is full
    ibool			useRLE;			// True for RLE encoding of host bitmaps
    ulong           linearCur;      // Cursor into linear offscreen memory
    ulong           linearMax;      // Maximum linear offscreen memory
    MGLRegion		freeSpace;		// Region of free offscreen memory

            // Function to allocate linear offscreen memory
            ibool allocateLinearMem(bitmap_t *bmp,ulong& startOff);

            // Function to allocate rectangular offscreen memory
            ibool allocateRectMem(bitmap_t *bmp,MGLRect& bounds);

public:
            // Constructor
            SPR_BitmapManager(MGLDC *dc,ibool useRLE);

			// Destructor
    virtual ~SPR_BitmapManager();

            // Destroy all bitmaps and start again
            void empty();

            // Add a new opaque bitmap object to the bitmap manager
            SPR_Bitmap *addOpaqueBitmap(bitmap_t *bmp);

            // Add a new source transparent bitmap object to the bitmap manager
			SPR_Bitmap *addTransparentBitmap(bitmap_t *bmp,color_t transparent);

			// Attempt to reload the hardware using cached host bitmaps
            ibool reloadHW();

            // Return true if using an offscreen device for caching bitmaps
            ibool usingOffscreenDC() { return (offdc != NULL); };

            // Return true if offscreen device context is full
            ibool offscreenCacheFull()   { return offdcFull; };
    };

#endif  // __BMPMGR_HPP
