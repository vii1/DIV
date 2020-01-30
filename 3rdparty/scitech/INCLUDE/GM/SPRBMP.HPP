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
* Description:  Header for the internal SPR_Bitmap class hierarchy.
*
*
****************************************************************************/

#ifndef __SPRBMP_HPP
#define __SPRBMP_HPP

#include "mgraph.hpp"
#include "gm\rlebmp.h"
#include "gm\dlist.hpp"

/*-------------------------- Class Definitions ----------------------------*/

//---------------------------------------------------------------------------
// This is the abstract base class for all bitmap objects. All bitmaps
// have an associated bitmap stored in host memory, which may optionally
// be destroyed when the bitmap is downloaded to the hardware.
//---------------------------------------------------------------------------

class SPR_Bitmap : public FX_DListNode {
protected:
    MGLDC       *dc;        // Main device context for rendering to
    bitmap_t    *bmp;       // Source bitmap in host memory

public:
            // Constructor
            SPR_Bitmap(MGLDC *dc,bitmap_t *bmp) : dc(dc), bmp(bmp) {};

            // Destructor
    virtual ~SPR_Bitmap();

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y) = 0;

            // Download data to hardware if necessary
    virtual void downloadToHW();

            // Determine if the bitmap was constructed properly
    virtual ibool isValid()  { return bmp != NULL; };
    };

typedef FX_DList<SPR_Bitmap>          SPR_BitmapList;
typedef FX_DListIterator<SPR_Bitmap>  SPR_BitmapListIter;

//---------------------------------------------------------------------------
// Solid bitmap optimised as a rectangle fill
//---------------------------------------------------------------------------

class SPR_SolidBitmap : public SPR_Bitmap {
protected:
    color_t color;              // Solid color for bitmap
    int     width,height;       // Width and height for rectangle

public:
            // Constructor
            SPR_SolidBitmap(MGLDC *dc,color_t color,int width,int height);

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);

            // Determine if the bitmap was constructed properly
    virtual ibool isValid()      { return true; };
    };

//---------------------------------------------------------------------------
// Bitmap stored in host memory.
//---------------------------------------------------------------------------

class SPR_HostBitmap : public SPR_Bitmap {
public:
            // Constructor
            SPR_HostBitmap(MGLDC *dc,bitmap_t *bmp);

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);
    };

//---------------------------------------------------------------------------
// Bitmap cached in an offscreen display device context.
//---------------------------------------------------------------------------

class SPR_OffscreenBitmap : public SPR_Bitmap {
protected:
    MGLDC   *offdc;         // Offscreen device context holding bitmap
    MGLRect bounds;         // Bounding rectangle in offscreen display DC

public:
            // Constructor
            SPR_OffscreenBitmap(MGLDC *dc,bitmap_t *bmp,MGLDC *offdc,
                const MGLRect& bounds);

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);

            // Download data to hardware if necessary
    virtual void downloadToHW();
    };

//---------------------------------------------------------------------------
// Bitmap cached in a linear offscreen display device context.
//---------------------------------------------------------------------------

class SPR_LinOffscreenBitmap : public SPR_Bitmap {
protected:
    MGLDC   *offdc;         // Offscreen device context holding bitmap
    ulong   startOff;       // Offset to start of bitmap in offscreen DC
    int     width,height;   // Width and height (in case host bitmap deleted)

public:
            // Constructor
            SPR_LinOffscreenBitmap(MGLDC *dc,bitmap_t *bmp,MGLDC *offdc,
                ulong startOff);

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);

            // Download data to hardware if necessary
	virtual void downloadToHW();
	};

//---------------------------------------------------------------------------
// Transparent bitmaps stored in host memory. For host transparent bitmaps,
// we actually compile the bitmaps into a run length encoded bitmap that
// we can draw directly to the destination device context surface as fast
// as possible. Note that the low level code is virtual linear buffer
// friendly, so you can render directly to virtual linear buffers as well.
//---------------------------------------------------------------------------

class SPR_HostSrcTransparentBitmap : public SPR_Bitmap {
protected:
    color_t     transparent;    // Transparent color
    rlebmp_t    *rle;           // Run length encoded bitmap
    ibool        virtualDst;     // True if drawing to virtualised buffer
    ibool        hardwareDC;     // True if we have hardware acceleration

public:
            // Constructor
            SPR_HostSrcTransparentBitmap(MGLDC *dc,bitmap_t *bmp,
                color_t transparent,ibool useRLE);

            // Destructor
    virtual ~SPR_HostSrcTransparentBitmap();

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);

            // Determine if the bitmap was constructed properly
    virtual ibool isValid();
    };

class SPR_HostDstTransparentBitmap : public SPR_Bitmap {
protected:
    color_t     transparent;    // Transparent color

public:
            // Constructor
            SPR_HostDstTransparentBitmap(MGLDC *dc,bitmap_t *bmp,
                color_t transparent);

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);
    };

//---------------------------------------------------------------------------
// Transparent bitmap cached in an offscreen display device context. This
// is used on hardware devices that have hardware tranparent bitblt support,
// and can be either source transparent or destination transparent.
//---------------------------------------------------------------------------

class SPR_OffscreenTransparentBitmap : public SPR_OffscreenBitmap {
protected:
    color_t transparent;    // Transparent color
    ibool    sourceTrans;    // True if source transparent

public:
            // Constructor
            SPR_OffscreenTransparentBitmap(MGLDC *dc,bitmap_t *bmp,
                MGLDC *offdc,const MGLRect& bounds,color_t transparent,
                ibool sourceTrans);

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);
    };

//---------------------------------------------------------------------------
// Transparent bitmap cached in a linear offscreen display device context.
//---------------------------------------------------------------------------

class SPR_LinOffscreenTransparentBitmap : public SPR_LinOffscreenBitmap {
protected:
    color_t transparent;    // Transparent color
    ibool    sourceTrans;    // True if source transparent

public:
            // Constructor
            SPR_LinOffscreenTransparentBitmap(MGLDC *dc,bitmap_t *bmp,
                MGLDC *offdc,ulong startOff,color_t transparent,
                ibool sourceTrans);

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);
    };

//---------------------------------------------------------------------------
// Masked bitmap cached in an offscreen display device context. This is a
// transparent bitmap that is drawn in combination with a monochrome bitmap
// mask stored in host memory and a normal bitmap stored in offscreen
// memory, and will provide fast transparent bitmaps for devices that do
// not have hardware transparent bitmaps.
//---------------------------------------------------------------------------

class SPR_OffscreenMaskedBitmap : public SPR_OffscreenBitmap {
protected:
    bitmap_t    *mask;      // Monochrome host bitmap mask

public:
            // Constructor
            SPR_OffscreenMaskedBitmap(MGLDC *dc,bitmap_t *bmp,MGLDC *offdc,
                const MGLRect& bounds,color_t transparent);

            // Destructor
    virtual ~SPR_OffscreenMaskedBitmap();

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);

            // Determine if the bitmap was constructed properly
    virtual ibool isValid();
    };

//---------------------------------------------------------------------------
// Masked bitmap cached in a linear offscreen display device context.
//---------------------------------------------------------------------------

class SPR_LinOffscreenMaskedBitmap : public SPR_LinOffscreenBitmap {
protected:
    bitmap_t    *mask;      // Monochrome host bitmap mask

public:
            // Constructor
            SPR_LinOffscreenMaskedBitmap(MGLDC *dc,bitmap_t *bmp,MGLDC *offdc,
                ulong startOff,color_t transparent);

            // Destructor
	virtual ~SPR_LinOffscreenMaskedBitmap();

            // Draw the bitmap at the specified location
    virtual void draw(int x,int y);

            // Determine if the bitmap was constructed properly
    virtual ibool isValid();
    };

#endif  // __SPRBMP_HPP

