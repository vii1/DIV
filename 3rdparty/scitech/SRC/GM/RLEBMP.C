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
* Language:     ANSI C
* Environment:  32 bit flat model only!
*
* Description:  Code to compile run length encoded transparent bitmaps used
*               to implement fast transparent sprite rendering in software.
*               We also provide a function to build a monochrome bitmap
*               mask for a bitmap in any format.
*
*
****************************************************************************/

#include "gm/rlebmp.h"
#include <stdlib.h>

/*------------------------- Implementation --------------------------------*/

#define NEWLINE(len)    ((ushort)(0x0000U | (ushort)(len)))
#define SKIPRUN(len)    ((ushort)(0x4000U | (ushort)(len)))
#define COPYRUN(len)    ((ushort)(0x8000U | (ushort)(len)))
#define TOKEN_SIZE      2
#define SCAN_OVERHEAD   (TOKEN_SIZE * 2)
#define InSkipRun       0
#define InCopyRun       1

PRIVATE long compile8(rlebmp_t *rle,bitmap_t *bitmap,color_t transparent)
/****************************************************************************
*
* Function:     compile8
* Parameters:   rle         - RLE bitmap to encode
*               bitmap      - Bitmap to build RLE bitmap from
*               transparent - Transparent color to mask out
*
* Description:  Performs the run length encoding for 8 bit bitmap data.
*
****************************************************************************/
{
    int         bytesPerLine = bitmap->bytesPerLine,width = bitmap->width;
    uchar       *source = bitmap->surface;
    int         x,y,state;
    ushort      *rec;
    uchar       *data;
    long        totalSize;

    /* Compile the bitmap */
    rec = (ushort*)rle->surface;
    totalSize = sizeof(rlebmp_t);
    for (y = 0; y < bitmap->height; y++, source += bytesPerLine) {
        uchar   *p = source;
        ushort  *lineStartRec = rec++;
        int     lineLength = TOKEN_SIZE,runLength = 1;

        /* Start encoding the first record */
        data = (uchar*)(rec + 1);
        if (*p == (uchar)transparent) {
            state = InSkipRun;          /* We're starting a skip run    */
            lineLength += TOKEN_SIZE;
            }
        else {
            state = InCopyRun;          /* We're starting a copy run    */
            *data++ = *p;
            lineLength += TOKEN_SIZE + 1;
            }
        p++;

        /* Now encode the rest of the scanline */
        for (x = 1; x < width; x++, p++) {
            if (*p == (uchar)transparent) {
                if (state == InSkipRun)         /* Still in skip run    */
                    runLength++;
                else {                          /* Changing to skip run */
                    *rec = COPYRUN(runLength);  /* Write copy record    */
                    rec = (ushort*)data;
                    runLength = 1;
                    state = InSkipRun;
                    lineLength += TOKEN_SIZE;
                    }
                }
            else {
                if (state == InCopyRun) {       /* Still in copy run    */
                    *data++ = *p;
                    runLength++;
                    lineLength++;
                    }
                else {                          /* Changing to copy run */
                    *rec = SKIPRUN(runLength);  /* Write skip record    */
                    rec++;
                    data = (uchar*)(rec + 1);
                    runLength = 1;
                    state = InCopyRun;
                    *data++ = *p;
                    lineLength += TOKEN_SIZE + 1;
                    }
                }
            }

        /* Finish off current record */
        if (state == InSkipRun) {
            *rec = SKIPRUN(runLength);
            rec++;
            }
        else {
            *rec = COPYRUN(runLength);
            rec = (ushort*)data;
            }
        *lineStartRec = NEWLINE(lineLength);
        totalSize += lineLength;
        }
    return totalSize;
}

PRIVATE long compile16(rlebmp_t *rle,bitmap_t *bitmap,color_t transparent)
/****************************************************************************
*
* Function:     compile16
* Parameters:   rle         - RLE bitmap to encode
*               bitmap      - Bitmap to build RLE bitmap from
*               transparent - Transparent color to mask out
*
* Description:  Performs the run length encoding for 16 bit bitmap data.
*
****************************************************************************/
{
    int         bytesPerLine = bitmap->bytesPerLine,width = bitmap->width;
    uchar       *source = (uchar*)bitmap->surface;
    int         x,y,state;
    ushort      *rec;
    ushort      *data;
    long        totalSize;

    /* Compile the bitmap */
    rec = (ushort*)rle->surface;
    totalSize = sizeof(rlebmp_t);
    for (y = 0; y < bitmap->height; y++, source += bytesPerLine) {
        ushort  *p = (ushort*)source;
        ushort  *lineStartRec = rec++;
        int     lineLength = TOKEN_SIZE,runLength = 1;

        /* Start encoding the first record */
        data = (ushort*)(rec + 1);
        if (*p == (ushort)transparent) {
            state = InSkipRun;          /* We're starting a skip run    */
            lineLength += TOKEN_SIZE;
            }
        else {
            state = InCopyRun;          /* We're starting a copy run    */
            *data++ = *p;
            lineLength += TOKEN_SIZE + 2;
            }
        p++;

        /* Now encode the rest of the scanline */
        for (x = 1; x < width; x++, p++) {
            if (*p == (ushort)transparent) {
                if (state == InSkipRun)         /* Still in skip run    */
                    runLength++;
                else {                          /* Changing to skip run */
                    *rec = COPYRUN(runLength);  /* Write copy record    */
                    rec = (ushort*)data;
                    runLength = 1;
                    state = InSkipRun;
                    lineLength += TOKEN_SIZE;
                    }
                }
            else {
                if (state == InCopyRun) {       /* Still in copy run    */
                    *data++ = *p;
                    runLength++;
                    lineLength += 2;
                    }
                else {                          /* Changing to copy run */
                    *rec = SKIPRUN(runLength);  /* Write skip record    */
                    rec++;
                    data = (ushort*)(rec + 1);
                    runLength = 1;
                    state = InCopyRun;
                    *data++ = *p;
                    lineLength += TOKEN_SIZE + 2;
                    }
                }
            }

        /* Finish off current record */
        if (state == InSkipRun) {
            *rec = SKIPRUN(runLength);
            rec++;
            }
        else {
            *rec = COPYRUN(runLength);
            rec = (ushort*)data;
            }
        *lineStartRec = NEWLINE(lineLength);
        totalSize += lineLength;
        }
    return totalSize;
}

rlebmp_t * SPR_buildRLEBitmap(bitmap_t *bitmap,
    color_t transparent)
/****************************************************************************
*
* Function:     SPR_buildRLEBitmap
* Parameters:   bitmap      - Bitmap to build RLE bitmap from
*               transparent - Transparent color to mask out
* Returns:      Pointer to allocated RLE bitmap, or NULL on error.
*
* Description:  Attempts to compile a run length encoded transparent
*               bitmap that can be used to render transparent bitmaps
*               extremely fast for lack of hardware acceleration.
*
*               Note that we only support 8,15 and 16 bits per pixel
*               bitmaps.
*
****************************************************************************/
{
    long        size;
    rlebmp_t    *rle;

    /* We currently only support 8/15/16 bits per pixel bitmaps */
    if (bitmap->bitsPerPixel < 8 || bitmap->bitsPerPixel > 16)
        return NULL;

    /* Allocate memory and build bitmap header information. Note that
     * we pre-allocate the bitmap with a 10% overhead in case it grows
     * in size (most unlikely).
     */
    size = sizeof(rlebmp_t) + bitmap->bytesPerLine * bitmap->height;
    size = ((size + bitmap->height * SCAN_OVERHEAD) * 120) / 100;
    if ((rle = malloc(size)) == NULL)
        return NULL;
    rle->width = bitmap->width;
    rle->height = bitmap->height;
    rle->bitsPerPixel = bitmap->bitsPerPixel;
    rle->bytesPerLine = bitmap->bytesPerLine;
    rle->surface = (uchar*)rle + sizeof(rlebmp_t);

    if (bitmap->bitsPerPixel == 8)
        size = compile8(rle,bitmap,transparent);
    else size = compile16(rle,bitmap,transparent);
    rle = realloc(rle,size);
    return rle;
}

void SPR_unloadRLEBitmap(rlebmp_t *bitmap)
{ free(bitmap); }

#define ISSKIPRUN(rec) (int)((((ushort)(rec)) & 0xC000) == 0x4000U)
#define ISCOPYRUN(rec) (int)((((ushort)(rec)) & 0xC000) == 0x8000U)
#define RUNLENGTH(rec) (int)(((ushort)(rec)) & 0x3FFF)

PRIVATE void draw8(MGLDC *dc,int srcX,int srcY,int dstX,int dstY,
    int width,int height,rlebmp_t *bitmap)
/****************************************************************************
*
* Function:     draw8
* Parameters:   dc          - Device context to render to
*               srcX,srcY   - Starting pixel in source bitmap
*               dstX,dstY   - Location to begin drawing at
*               bitmap      - Bitmap to draw
*
* Description:  Draws the RLE bitmap in 8 bits per pixel modes.
*
****************************************************************************/
{
    int     x,y,destDeltaScan = dc->mi.bytesPerLine;
    ushort  *scan = (ushort*)bitmap->surface;
    uchar   *dest = (uchar*)dc->surface + (dstY * destDeltaScan) + dstX;

    /* Prestep to starting source scanline */
    for (y = 0; y < srcY; y++, dest += destDeltaScan) {
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        dstY++;
        }

    /* Now process all scanlines within clipped area */
    destDeltaScan -= width;
    for (y = 0; y < height; y++, dest += destDeltaScan){
        ushort  *rec = scan + 1;

        /* Prestep to starting source X coordinate */
        for (x = 0; x < srcX; rec++) {
            x += RUNLENGTH(*rec);

            if (x > srcX) {
                /* We need to partially process the current record */
                int overlap = x - srcX;
                int activeOverlap = (overlap > width) ? width : overlap;

                if (ISCOPYRUN(*rec)) {
                    /* Copy overlap pixels to destination */
                    uchar *p = (uchar*)(rec + 1);
                    p += RUNLENGTH(*rec) - overlap;
                    MGL_memcpy(dest,p,activeOverlap);
                    }
                dest += activeOverlap;
                }

            /* Skip to next record */
            if (ISCOPYRUN(*rec))
                rec = (ushort*)((uchar*)rec + RUNLENGTH(*rec));
            }
        for (x = x - srcX; x < width; rec++) {
            int runLength = RUNLENGTH(*rec);
            int remainingWidth = width - x;
            int activePixels = (runLength > remainingWidth) ?
                                    remainingWidth : runLength;

            if (ISCOPYRUN(*rec)){
                /* Copy pixels to destination */
                uchar *p = (uchar*)(rec + 1);
                MGL_memcpy(dest,p,activePixels);
                rec = (ushort*)((uchar*)rec + runLength);
                }
            dest += activePixels;
            x += runLength;
            }
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        }
}

PRIVATE void draw8VIRT(MGLDC *dc,int srcX,int srcY,int dstX,int dstY,
    int width,int height,rlebmp_t *bitmap)
/****************************************************************************
*
* Function:     draw8VIRT
* Parameters:   dc          - Device context to render to
*               srcX,srcY   - Starting pixel in source bitmap
*               dstX,dstY   - Location to begin drawing at
*               bitmap      - Bitmap to draw
*
* Description:  Draws the RLE bitmap in 8 bits per pixel modes. This
*               version properly handlers drawing to virtual framebuffer
*               devices.
*
****************************************************************************/
{
    int     x,y,destDeltaScan = dc->mi.bytesPerLine;
    ushort  *scan = (ushort*)bitmap->surface;
    uchar   *dest = (uchar*)dc->surface + (dstY * destDeltaScan) + dstX;

    /* Prestep to starting source scanline */
    for (y = 0; y < srcY; y++, dest += destDeltaScan) {
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        dstY++;
        }

    /* Now process all scanlines within clipped area */
    destDeltaScan -= width;
    for (y = 0; y < height; y++, dest += destDeltaScan){
        ushort  *rec = scan + 1;

        /* Prestep to starting source X coordinate */
        for (x = 0; x < srcX; rec++) {
            x += RUNLENGTH(*rec);

            if (x > srcX) {
                /* We need to partially process the current record */
                int overlap = x - srcX;
                int activeOverlap = (overlap > width) ? width : overlap;

                if (ISCOPYRUN(*rec)) {
                    /* Copy overlap pixels to destination */
                    uchar *p = (uchar*)(rec + 1);
                    p += RUNLENGTH(*rec) - overlap;
                    MGL_memcpyVIRTDST(dest,p,activeOverlap);
                    }
                dest += activeOverlap;
                }

            /* Skip to next record */
            if (ISCOPYRUN(*rec))
                rec = (ushort*)((uchar*)rec + RUNLENGTH(*rec));
            }
        for (x = x - srcX; x < width; rec++) {
            int runLength = RUNLENGTH(*rec);
            int remainingWidth = width - x;
            int activePixels = (runLength > remainingWidth) ?
                                    remainingWidth : runLength;

            if (ISCOPYRUN(*rec)){
                /* Copy pixels to destination */
                uchar *p = (uchar*)(rec + 1);
                MGL_memcpyVIRTDST(dest,p,activePixels);
                rec = (ushort*)((uchar*)rec + runLength);
                }
            dest += activePixels;
            x += runLength;
            }
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        }
}

PRIVATE void draw16(MGLDC *dc,int srcX,int srcY,int dstX,int dstY,
    int width,int height,rlebmp_t *bitmap)
/****************************************************************************
*
* Function:     draw16
* Parameters:   dc          - Device context to render to
*               srcX,srcY   - Starting pixel in source bitmap
*               dstX,dstY   - Location to begin drawing at
*               bitmap      - Bitmap to draw
*
* Description:  Draws the RLE bitmap in 8 bits per pixel modes.
*
****************************************************************************/
{
    int     x,y,destDeltaScan = dc->mi.bytesPerLine;
    ushort  *scan = (ushort*)bitmap->surface;
    uchar   *dest = (uchar*)dc->surface + (dstY * destDeltaScan) + dstX*2;

    /* Prestep to starting source scanline */
    for (y = 0; y < srcY; y++, dest += destDeltaScan) {
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        dstY++;
        }

    /* Now process all scanlines within clipped area */
    destDeltaScan -= width*2;
    for (y = 0; y < height; y++, dest += destDeltaScan){
        ushort  *rec = scan + 1;

        /* Prestep to starting source X coordinate */
        for (x = 0; x < srcX; rec++) {
            x += RUNLENGTH(*rec);

            if (x > srcX) {
                /* We need to partially process the current record */
                int overlap = x - srcX;
                int activeOverlap = (overlap > width) ? width : overlap;

                if (ISCOPYRUN(*rec)) {
                    /* Copy overlap pixels to destination */
                    ushort *p = (ushort*)(rec + 1);
                    p += RUNLENGTH(*rec) - overlap;
                    MGL_memcpy(dest,p,activeOverlap*2);
                    }
                dest += activeOverlap*2;
                }

            /* Skip to next record */
            if (ISCOPYRUN(*rec))
                rec = (ushort*)((ushort*)rec + RUNLENGTH(*rec));
            }
        for (x = x - srcX; x < width; rec++) {
            int runLength = RUNLENGTH(*rec);
            int remainingWidth = width - x;
            int activePixels = (runLength > remainingWidth) ?
                                    remainingWidth : runLength;

            if (ISCOPYRUN(*rec)){
                /* Copy pixels to destination */
                ushort *p = (ushort*)(rec + 1);
                MGL_memcpy(dest,p,activePixels*2);
                rec = (ushort*)((ushort*)rec + runLength);
                }
            dest += activePixels*2;
            x += runLength;
            }
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        }
}

PRIVATE void draw16VIRT(MGLDC *dc,int srcX,int srcY,int dstX,int dstY,
    int width,int height,rlebmp_t *bitmap)
/****************************************************************************
*
* Function:     draw16VIRT
* Parameters:   dc          - Device context to render to
*               srcX,srcY   - Starting pixel in source bitmap
*               dstX,dstY   - Location to begin drawing at
*               bitmap      - Bitmap to draw
*
* Description:  Draws the RLE bitmap in 8 bits per pixel modes. This
*               version properly handlers drawing to virtual framebuffer
*               devices.
*
****************************************************************************/
{
    int     x,y,destDeltaScan = dc->mi.bytesPerLine;
    ushort  *scan = (ushort*)bitmap->surface;
    uchar   *dest = (uchar*)dc->surface + (dstY * destDeltaScan) + dstX*2;

    /* Prestep to starting source scanline */
    for (y = 0; y < srcY; y++, dest += destDeltaScan) {
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        dstY++;
        }

    /* Now process all scanlines within clipped area */
    destDeltaScan -= width*2;
    for (y = 0; y < height; y++, dest += destDeltaScan){
        ushort  *rec = scan + 1;

        /* Prestep to starting source X coordinate */
        for (x = 0; x < srcX; rec++) {
            x += RUNLENGTH(*rec);

            if (x > srcX) {
                /* We need to partially process the current record */
                int overlap = x - srcX;
                int activeOverlap = (overlap > width) ? width : overlap;

                if (ISCOPYRUN(*rec)) {
                    /* Copy overlap pixels to destination */
                    ushort *p = (ushort*)(rec + 1);
                    p += RUNLENGTH(*rec) - overlap;
                    MGL_memcpyVIRTDST(dest,p,activeOverlap*2);
                    }
                dest += activeOverlap*2;
                }

            /* Skip to next record */
            if (ISCOPYRUN(*rec))
                rec = (ushort*)((ushort*)rec + RUNLENGTH(*rec));
            }
        for (x = x - srcX; x < width; rec++) {
            int runLength = RUNLENGTH(*rec);
            int remainingWidth = width - x;
            int activePixels = (runLength > remainingWidth) ?
                                    remainingWidth : runLength;

            if (ISCOPYRUN(*rec)){
                /* Copy pixels to destination */
                ushort *p = (ushort*)(rec + 1);
                MGL_memcpyVIRTDST(dest,p,activePixels*2);
                rec = (ushort*)((ushort*)rec + runLength);
                }
            dest += activePixels*2;
            x += runLength;
            }
        scan = (ushort*)((uchar*)scan + RUNLENGTH(*scan));
        }
}

void SPR_putRLEBitmap(MGLDC *dc,int dstX,int dstY,rlebmp_t *bitmap,
    ibool virtualDst)
/****************************************************************************
*
* Function:     SPR_putRLEBitmap
* Parameters:   dc          - Device context to render to
*               dstX,dstY   - Location to begin drawing at
*               bitmap      - Bitmap to draw
*               virtualDst  - True if destination is a virtualised surface
*
* Description:  Draws the RLE bitmap.
*
****************************************************************************/
{
    int width = bitmap->width,height = bitmap->height;
    int delta,srcX = 0,srcY = 0;

    if (dc->mi.bitsPerPixel != bitmap->bitsPerPixel)
        return;

    /* Clip source bitmap to left and right edges of clip rectangle */
    if (dstX < dc->a.clipRect.left) {
        width -= (delta = dc->a.clipRect.left - dstX);
        srcX = delta;
        dstX = dc->a.clipRect.left;
        }
    if ((dstX + width) > dc->a.clipRect.right)
        width = dc->a.clipRect.right - dstX;

    /* Clip source bitmap to top and bottom edges of clip rectangle */
    if (dstY < dc->a.clipRect.top) {
        height -= (delta = dc->a.clipRect.top - dstY);
        srcY = delta;
        dstY = dc->a.clipRect.top;
        }
    if ((dstY + height) > dc->a.clipRect.bottom)
        height = dc->a.clipRect.bottom - dstY;

    /* Bail out if totally clipped */
    if (height <= 0 || width <= 0)
        return;

    if (bitmap->bitsPerPixel == 8) {
        if (virtualDst)
            draw8VIRT(dc,srcX,srcY,
                dstX + dc->a.viewPort.left,
                dstY + dc->a.viewPort.top,
                width,height,bitmap);
        else
            draw8(dc,srcX,srcY,
                dstX + dc->a.viewPort.left,
                dstY + dc->a.viewPort.top,
                width,height,bitmap);
        }
    else {
        if (virtualDst)
            draw16VIRT(dc,srcX,srcY,
                dstX + dc->a.viewPort.left,
                dstY + dc->a.viewPort.top,
                width,height,bitmap);
        else
            draw16(dc,srcX,srcY,
                dstX + dc->a.viewPort.left,
                dstY + dc->a.viewPort.top,
                width,height,bitmap);
        }
}
