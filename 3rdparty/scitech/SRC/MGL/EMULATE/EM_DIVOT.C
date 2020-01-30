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
* Environment:	IBM PC (MS DOS)
*
* Description:	C based emulation routines for fast image save & restores.
*
* 				In cases where we can directly access the entire device
*				contexts surface through a single linear aperture, we will
*				use simple and efficient memory copies. This will be true
*				for 320x200x256, all memory device contexts and all modes
* 				with > 8 bits per pixel in 32 bit memory models and video
* 				hardware that supports a linear frame buffer.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

long _ASMAPI __EMU__divotSize(MGLDC *dc,int left,int top,int right,int bottom)
/****************************************************************************
*
* Function:		__EMU__divotSize
* Parameters:	dc		- Device context
* 				left	- Left coordinate of image
*				top		- Top coordinate of image
*				right	- Right coordinate of image
*				bottom	- Bottom coordinate of image
* Returns:		Number of bytes required to read divot.
*
* Description:  Calculates the number of bytes required to hold the image
*				divot in system memory. This is dependant on the type of
*				device context used, and we correctly calculate the value
*				depending on whether the device is packed pixel or planar.
*
*				NOTE: These coordinate CANNOT be viewport relative.
*
****************************************************************************/
{
	long	size;

	if (dc->mi.bitsPerPixel >= 8) {
		/* All packed pixel formats with greater than 8 bits per pixel
		 * are always byte aligned, so the size is simple.
		 */
		size = (long)(right - left) * (long)(bottom - top) *
			((dc->mi.bitsPerPixel+7) / 8);
		}
	else {
		if (dc->mi.bitsPerPixel == 4 && dc->mi.numberOfPlanes == 1) {
			/* 4 bits packed pixel modes */
			left /= 2;
			right = ((right-1) / 2) + 1;
			size = (long)(right - left) * (long)(bottom - top);
			}
		else {
			/* 1 bit mono and 4 bits planar modes */
			left /= 8;
			right = ((right-1) / 8) + 1;
			size = (long)(right - left) * (long)(bottom - top)
				* dc->mi.bitsPerPixel;
			}
		}

	return size + sizeof(divotHeader);
}

void _ASMAPI __EMU__getDivot(MGLDC *dc,int left,int top,int right,int bottom,
	void *divot)
/****************************************************************************
*
* Function:		__EMU__getDivot
* Parameters:	dc		- Device context
* 				left	- Left coordinate of image
*				top		- Top coordinate of image
*				right	- Right coordinate of image
*				bottom	- Bottom coordinate of image
*               divot	- Place to store the image data
*
* Description:	Gets a block of video data from the current video page into
*				system RAM. 'divot' MUST have been allocated with enough
*				room to store the image. Use MGL_divotSize() to compute
*				the size of the image. This routine saves the entire
*				byte boundary block of video data.
*
****************************************************************************/
{
	int			i;
	uchar		_HUGE *p = divot;
	divotHeader	hdr;

	hdr.left = left;	hdr.top = top;
	hdr.right = right;	hdr.bottom = bottom;
	hdr.bytesPerLine = _MGL_bytesPerLine(dc,left,right);
	memcpy(p,&hdr,sizeof(hdr));		/* Store header information			*/
	p += sizeof(hdr);

	/* Read data by calling device driver to read scanlines			*/
	dc->r.beginPixel(dc);
	for (i = top; i < bottom; i++, p += hdr.bytesPerLine) {
		NORMALISE_PTR(p);
		dc->r.getScanLine(dc,i,left,right,p,left,MGL_REPLACE_MODE);
		}
	dc->r.endPixel(dc);
}

void _ASMAPI __EMU__putDivot(MGLDC *dc,void *divot)
/****************************************************************************
*
* Function:		__EMU__putDivot
* Parameters:	dc		- Device context
* 				divot	- Buffer containing image block to use
*
* Description:	Moves a block of video data from system RAM onto the current
*				display page. The image is restored to exactly the same
*				position that it was saved from.
*
*				NOTE: These coordinate CANNOT be viewport relative.
*
****************************************************************************/
{
	int			i;
	uchar		_HUGE *p = divot;
	divotHeader	hdr;

	memcpy(&hdr,p,sizeof(hdr));		/* Extract header information		*/
	p += sizeof(hdr);

	/* Write data by calling device driver to write scanlines		*/
	dc->r.beginPixel(dc);
	for (i = hdr.top; i < hdr.bottom; i++, p += hdr.bytesPerLine) {
		NORMALISE_PTR(p);
		dc->r.putScanLine(dc,i,hdr.left,hdr.right,p,hdr.left,MGL_REPLACE_MODE);
		}
	dc->r.endPixel(dc);
}
