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
* Description:  Implementation for the SPR_Bitmap class hierarchy.
*
*
****************************************************************************/

#include "sprmgr.hpp"
#pragma hdrstop

/*------------------------- Implementation --------------------------------*/

SPR_Bitmap::~SPR_Bitmap()
{
    MGL_unloadBitmap(bmp);
}

void SPR_Bitmap::downloadToHW() {}

SPR_SolidBitmap::SPR_SolidBitmap(MGLDC *dc,color_t color,int width,int height)
    : SPR_Bitmap(dc,NULL), color(color), width(width), height(height)
/****************************************************************************
*
* Function:     SPR_SolidBitmap::SPR_SolidBitmap
* Parameters:   dc  - Device context bitmaps will be drawn to
*               bmp - Source bitmap in host memory
*
* Description:  This is a special type of bitmap that is known to be of
*               a single solid color so can be drawn fast just as a single
*               rectangle.
*
****************************************************************************/
{
}

void SPR_SolidBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_SolidBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the solid bitmap as a rectangle.
*
****************************************************************************/
{
    MGL_setColor(color);
    MGL_fillRectCoord(x,y,x+width,y+height);
}

SPR_HostBitmap::SPR_HostBitmap(MGLDC *dc,bitmap_t *bmp) : SPR_Bitmap(dc,bmp)
/****************************************************************************
*
* Function:     SPR_HostBitmap::SPR_HostBitmap
* Parameters:   dc  - Device context bitmaps will be drawn to
*               bmp - Source bitmap in host memory
*
****************************************************************************/
{
}

void SPR_HostBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_HostBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the host bitmap to the main device context.
*
****************************************************************************/
{
    MGL_putBitmap(dc,x,y,bmp,MGL_REPLACE_MODE);
}

SPR_OffscreenBitmap::SPR_OffscreenBitmap(MGLDC *dc,bitmap_t *bmp,
    MGLDC *offdc,const MGLRect& bounds)
    : SPR_Bitmap(dc,bmp), offdc(offdc), bounds(bounds)
/****************************************************************************
*
* Function:     SPR_OffscreenBitmap::SPR_OffscreenBitmap
* Parameters:   dc      - Device context bitmaps will be drawn to
*               bmp     - Source bitmap in host memory
*               offdc   - Offscreen DC for caching bitmap
*               bounds  - Bounding rectangle for bitmap in offscreen DC
*
****************************************************************************/
{
}

void SPR_OffscreenBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_OffscreenBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the offscreen bitmap to the main device context.
*
****************************************************************************/
{
	MGL_bitBlt(dc,offdc,bounds.r(),x,y,MGL_REPLACE_MODE);
}

void SPR_OffscreenBitmap::downloadToHW()
/****************************************************************************
*
* Function:     SPR_OffscreenBitmap::downloadToHw
*
* Description:  Downloads the bitmap to the offscreen display device
*
****************************************************************************/
{
    MGL_putBitmap(offdc,bounds.left(),bounds.top(),bmp,MGL_REPLACE_MODE);
}

SPR_LinOffscreenBitmap::SPR_LinOffscreenBitmap(MGLDC *dc,bitmap_t *bmp,
    MGLDC *offdc,ulong startOff)
    : SPR_Bitmap(dc,bmp), offdc(offdc), startOff(startOff)
/****************************************************************************
*
* Function:     SPR_LinOffscreenBitmap::SPR_LinOffscreenBitmap
* Parameters:   dc          - Device context bitmaps will be drawn to
*               bmp         - Source bitmap in host memory
*               offdc       - Linear offscreen DC for caching bitmap
*               startOff    - Starting offset in offscreen device for bitmap
*
****************************************************************************/
{
    width = bmp->width;
	height = bmp->height;
}

void SPR_LinOffscreenBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_LinOffscreenBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the Linear offscreen bitmap to the main device
*               context.
*
****************************************************************************/
{
    MGL_bitBltLinCoord(dc,offdc,startOff,x,y,x+width,y+height,
        MGL_REPLACE_MODE);
}

void SPR_LinOffscreenBitmap::downloadToHW()
/****************************************************************************
*
* Function:     SPR_LinOffscreenBitmap::downloadToHw
*
* Description:  Downloads the bitmap to the linear offscreen display
*               device. We can do this with a simply memory copy, however
*               the display surface may be virtualised, so we use the
*               virtual buffer safe memory copy.
*
****************************************************************************/
{
    uchar   *dst = (uchar*)offdc->surface+startOff;
    uchar   *src = (uchar*)bmp->surface;
    int     i,dstDeltaScan;

    if (bmp->bitsPerPixel == 8)
        dstDeltaScan = bmp->width;
    else if (bmp->bitsPerPixel == 15 || bmp->bitsPerPixel == 16)
        dstDeltaScan = bmp->width * 2;
    else if (bmp->bitsPerPixel == 24)
        dstDeltaScan = bmp->width * 3;
    else dstDeltaScan = bmp->width * 4;

    // Download data a scanline at a time directly to display memory. Note
    // that host bitmaps have the scanlines DWORD aligned for speed, but
    // linear offscreen bitmaps and packed to pixel boundaries so we cant
	// just copy the data directly.
    for (i = 0; i < bmp->height; i++) {
        MGL_memcpyVIRTDST(dst,src,dstDeltaScan);
        dst += dstDeltaScan;
        src += bmp->bytesPerLine;
        }
}

SPR_HostSrcTransparentBitmap::SPR_HostSrcTransparentBitmap(MGLDC *dc,
    bitmap_t *_bmp,color_t transparent,ibool useRLE)
    : SPR_Bitmap(dc,_bmp), transparent(transparent)
/****************************************************************************
*
* Function:     SPR_HostSrcTransparentBitmap::SPR_HostSrcTransparentBitmap
* Parameters:   dc          - Device context bitmaps will be drawn to
*               bmp         - Source bitmap in host memory
*               transparent - Transparent color for bitmap
*               sourceTrans - True if source transparent
*
****************************************************************************/
{
	rle = NULL;
	if (!SPR_Bitmap::isValid())
		return;
	virtualDst = (MGL_surfaceAccessType(dc) == MGL_VIRTUAL_ACCESS);
    hardwareDC = (MGL_getHardwareFlags(dc) != MGL_HW_NONE);
	if ((MGL_getBitsPerPixel(dc) <= 16) && useRLE) {
    	if ((rle = SPR_buildRLEBitmap(bmp,transparent)) != NULL) {
	        MGL_unloadBitmap(bmp);
	        bmp = NULL;
	        }
		}
}

SPR_HostSrcTransparentBitmap::~SPR_HostSrcTransparentBitmap()
{
	if (rle) 
	    SPR_unloadRLEBitmap(rle);
}

void SPR_HostSrcTransparentBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_HostSrcTransparentBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the Linear offscreen bitmap to the main device
*               context.
*
****************************************************************************/
{
    // Draw the bitmap directly to the rendering device surface using
    // our pre-compiled run length encoded transparent bitmap. Note
    // that if we have a hardware accelerated rendering device, we
    // must call MGL_beginDirectAccess and MGL_endDirectAccess before
    // and after peforming any direct surface access.

	if (rle) {
	    if (hardwareDC) {
	        MGL_beginDirectAccess();
	        SPR_putRLEBitmap(dc,x,y,rle,virtualDst);
	        MGL_endDirectAccess();
	        }
	    else
	        SPR_putRLEBitmap(dc,x,y,rle,virtualDst);
		}
	else
	    MGL_putBitmapTransparent(dc,x,y,bmp,transparent,true);
}

ibool SPR_HostSrcTransparentBitmap::isValid()
/****************************************************************************
*
* Function:     SPR_HostSrcTransparentBitmap::isValid
* Returns:      True if all is well
*
****************************************************************************/
{
    return (rle != NULL);
}

SPR_HostDstTransparentBitmap::SPR_HostDstTransparentBitmap(MGLDC *dc,
    bitmap_t *bmp,color_t transparent)
    : SPR_Bitmap(dc,bmp), transparent(transparent)
/****************************************************************************
*
* Function:     SPR_HostDstTransparentBitmap::SPR_HostDstTransparentBitmap
* Parameters:   dc          - Device context bitmaps will be drawn to
*               bmp         - Source bitmap in host memory
*               transparent - Transparent color for bitmap
*
****************************************************************************/
{
}

void SPR_HostDstTransparentBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_HostDstTransparentBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the Linear offscreen bitmap to the main device
*               context.
*
****************************************************************************/
{
    MGL_putBitmapTransparent(dc,x,y,bmp,transparent,false);
	
}

SPR_OffscreenTransparentBitmap::SPR_OffscreenTransparentBitmap(MGLDC *dc,
    bitmap_t *bmp,MGLDC *offdc,const MGLRect& bounds,color_t transparent,
    ibool sourceTrans)
    : SPR_OffscreenBitmap(dc,bmp,offdc,bounds), transparent(transparent),
      sourceTrans(sourceTrans)
/****************************************************************************
*
* Function:     SPR_OffscreenTransparentBitmap::SPR_OffscreenTransparentBitmap
* Parameters:   dc          - Device context bitmaps will be drawn to
*               bmp         - Source bitmap in host memory
*               offdc       - OffscreenTransparent DC for caching bitmap
*               bounds      - Bounding rectangle for bitmap in offscreen DC
*               transparent - Transparent color for bitmap
*               sourceTrans - True if source transparent
*
****************************************************************************/
{
}

void SPR_OffscreenTransparentBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_OffscreenTransparentBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the offscreen bitmap to the main device context.
*
****************************************************************************/
{
	MGL_transBlt(dc,offdc,bounds.r(),x,y,transparent,sourceTrans);
}

SPR_LinOffscreenTransparentBitmap::SPR_LinOffscreenTransparentBitmap(MGLDC *dc,
    bitmap_t *bmp,MGLDC *offdc,ulong startOff,color_t transparent,
    ibool sourceTrans)
    : SPR_LinOffscreenBitmap(dc,bmp,offdc,startOff),
      transparent(transparent), sourceTrans(sourceTrans)
/****************************************************************************
*
* Function:     SPR_LinOffscreenTransparentBitmap::SPR_LinOffscreenTransparentBitmap
* Parameters:   dc          - Device context bitmaps will be drawn to
*               bmp         - Source bitmap in host memory
*               offdc       - Linear OffscreenTransparent DC for caching bitmap
*               startOff    - Starting offset in OffscreenTransparent device for bitmap
*
****************************************************************************/
{
}

void SPR_LinOffscreenTransparentBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_LinOffscreenTransparentBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the Linear OffscreenTransparent bitmap to the main device
*               context.
*
****************************************************************************/
{
    MGL_transBltLinCoord(dc,offdc,startOff,x,y,x+width,y+height,
        transparent,sourceTrans);
}

SPR_OffscreenMaskedBitmap::SPR_OffscreenMaskedBitmap(MGLDC *dc,
    bitmap_t *bmp,MGLDC *offdc,const MGLRect& bounds,color_t transparent)
    : SPR_OffscreenBitmap(dc,bmp,offdc,bounds)
/****************************************************************************
*
* Function:     SPR_OffscreenMaskedBitmap::SPR_OffscreenMaskedBitmap
* Parameters:   dc          - Device context bitmaps will be drawn to
*               bmp         - Source bitmap in host memory
*               offdc       - OffscreenTransparent DC for caching bitmap
*               bounds      - Bounding rectangle for bitmap in offscreen DC
*               transparent - Transparent color for bitmap
*               sourceTrans - True if source transparent
*
****************************************************************************/
{
    mask = MGL_buildMonoMask(bmp,transparent);

    // Now that we have made our monochrome mask, we need to translate all
    // the transparent color values in the bitmap into 0's so that the
	// rendering will work properly.

	// TODO: Need to update this to support 24 and 32bpp bitmaps!
	if (bmp->bitsPerPixel == 8) {
        uchar *p = (uchar*)bmp->surface;
        for (int i = bmp->bytesPerLine * bmp->height; i > 0; i--) {
            if (*p == (uchar)transparent)
                *p = 0;
            p++;
            }
        }
    else {
        ushort *p = (ushort*)bmp->surface;
        for (int i = (bmp->bytesPerLine / 2) * bmp->height; i > 0; i--) {
            if (*p == (ushort)transparent)
                *p = 0;
            p++;
            }
        }
}

SPR_OffscreenMaskedBitmap::~SPR_OffscreenMaskedBitmap()
{
    MGL_unloadBitmap(mask);
}

void SPR_OffscreenMaskedBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_OffscreenMaskedBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the offscreen bitmap to the main device context.
*
****************************************************************************/
{
    MGL_putBitmapMask(dc,x,y,mask,0);
	MGL_bitBlt(dc,offdc,bounds.r(),x,y,MGL_OR_MODE);
}

ibool SPR_OffscreenMaskedBitmap::isValid()
/****************************************************************************
*
* Function:     SPR_OffscreenMaskedBitmap::isValid
* Returns:      True if all is well
*
****************************************************************************/
{
    if (!SPR_Bitmap::isValid())
        return false;
    return (mask != NULL);
}

SPR_LinOffscreenMaskedBitmap::SPR_LinOffscreenMaskedBitmap(MGLDC *dc,
    bitmap_t *bmp,MGLDC *offdc,ulong startOff,color_t transparent)
    : SPR_LinOffscreenBitmap(dc,bmp,offdc,startOff)
/****************************************************************************
*
* Function:     SPR_LinOffscreenMaskedBitmap::SPR_LinOffscreenMaskedBitmap
* Parameters:   dc          - Device context bitmaps will be drawn to
*               bmp         - Source bitmap in host memory
*               offdc       - Linear offscreen DC for caching bitmap
*               startOff    - Starting offset in offscreen device for bitmap
*
****************************************************************************/
{
    mask = MGL_buildMonoMask(bmp,transparent);

    // Now that we have made our monochrome mask, we need to translate all
    // the transparent color values in the bitmap into 0's so that the
    // rendering will work properly.
    if (bmp->bitsPerPixel == 8) {
		uchar *p = (uchar*)bmp->surface;
        for (int i = bmp->bytesPerLine * bmp->height; i > 0; i--) {
            if (*p == (uchar)transparent)
                *p = 0;
			p++;
            }
        }
    else {
        ushort *p = (ushort*)bmp->surface;
        for (int i = (bmp->bytesPerLine / 2) * bmp->height; i > 0; i--) {
            if (*p == (ushort)transparent)
                *p = 0;
            p++;
            }
        }
}

SPR_LinOffscreenMaskedBitmap::~SPR_LinOffscreenMaskedBitmap()
{
    MGL_unloadBitmap(mask);
}

void SPR_LinOffscreenMaskedBitmap::draw(int x,int y)
/****************************************************************************
*
* Function:     SPR_LinOffscreenMaskedBitmap::draw
* Parameters:   x,y - Location to draw bitmap at
*
* Description:  Draws the Linear OffscreenTransparent bitmap to the main device
*               context.
*
****************************************************************************/
{
    MGL_putBitmapMask(dc,x,y,mask,0);
    MGL_bitBltLinCoord(dc,offdc,startOff,x,y,x+width,y+height,
        MGL_OR_MODE);
}

ibool SPR_LinOffscreenMaskedBitmap::isValid()
/****************************************************************************
*
* Function:     SPR_LinOffscreenMaskedBitmap::isValid
* Returns:      True if all is well
*
****************************************************************************/
{
	if (!SPR_Bitmap::isValid())
        return false;
    return (mask != NULL);
}


