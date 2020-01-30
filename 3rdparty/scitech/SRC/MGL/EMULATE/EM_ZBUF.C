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
* Description:	Software Zbuffering support routines. The Zbuffer that we
*				allocate for software Zbuffering can be 16,24 or 32 bits
*				deep. 24 bit ZBuffers are slow in software, but are
*				supported by some hardware devices.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifdef	MGL_FIX3D

/*------------------------- Implementation --------------------------------*/

ibool _ASMAPI __EMU__zBegin(MGLDC *dc,int zbits)
/****************************************************************************
*
* Function:		__EMU__zBegin
* Parameters:	dc		- Device context
*				zbits	- Bit depth for Z-buffer
* Returns:		True if allocated, false if cannot allocate Zbuffer.
*
* Description:  Attempts to allocate a zbuffer block in main memory for
*				software Zbuffering support, given the full resolution of
*				the dc. If there is not enough memory or is too large,
*				we return false.
*
****************************************************************************/
{
#ifdef	__16BIT__
	dc = dc;
	zbits = zbits;
	return false;					/* Not supported for 16 bit code	*/
#else
	long	size,height = dc->size.bottom - dc->size.top;

	_MGL_result = grOK;
	if (dc->mi.bitsPerPixel < 8 || (dc->flags & MGL_SURFACE_FLAGS) == MGL_NO_ACCESS)
		return false;
	if (dc->zbuffer) {
		if (dc->zbits == zbits)
			return true;
		return false;
		}

	/* Update the current DC if it is the same as this one */
	if (dc == _MGL_dcPtr)
		*_MGL_dcPtr = _MGL_dc;		/* Update this DC copy				*/

	/* Compute the width of the zbuffer and the amount of memory we need to
	 * allocate for it. Note that we allocate one extra scanline which is
	 * used as our temporary z offset buffer during rendering.
	 */
	dc->zbits = zbits;
	dc->zwidth = dc->size.right - dc->size.left;
	switch (zbits) {
		case 16:
			size = (long)dc->zwidth * (height+1) * sizeof(zbuf16_t);
			break;
		case 32:
			size = (long)dc->zwidth * (height+1) * sizeof(zbuf32_t);
			break;
		default:
			return false;
		}
	if ((dc->zbuffer = MGL_calloc(1,size)) == NULL) {
		_MGL_result = grNoMem;
		return false;
		}

	/* Compute the location of our temporary zbuffer offset table */
	switch (zbits) {
		case 16:
			dc->zOffset = (zbuf16_t _HUGE *)dc->zbuffer
				+ height * (long)dc->zwidth;
			dc->r.z = dc->r.z16;
			break;
		case 32:
			dc->zOffset = (zbuf32_t _HUGE *)dc->zbuffer
				+ height * (long)dc->zwidth;
			dc->r.z = dc->r.z32;
			break;
		}

	/* Flag that we own this Zbuffer, so will destroy it */
	dc->flags &= ~MGL_SHARED_ZBUFFER;
	dc->flags |= MGL_LINEAR_ZACCESS;

	/* Flush changes to the cached copy if this is the current DC */
	if (dc == _MGL_dcPtr)
		_MGL_dc = *_MGL_dcPtr;		/* Update the cached copy			*/
	return true;
#endif
}

#if defined(NO_ASSEMBLER) || !defined(__INTEL__)

void _ASMAPI __EMU__zClear16(int left,int top,int right,int bottom,
	zfix32_t clearVal)
/****************************************************************************
*
* Function:		__EMU__zClear16
* Parameters:	left,top,right,bottom	- Rectangle in zbuffer to clear
*
* Description:	Clears the Zbuffer before rendering a new scene into it.
*
****************************************************************************/
{
	int		y;
	long 	size = (long)(right - left);
	zbuf16_t _HUGE *z = _MGL_zbufAddr16(left,top);

	if (!DC.zbuffer)
		MGL_fatalError("Zbuffer not allocated!\n");

	clearVal >>= 15;		// Adjust clear value to 16 bits
	for (y = top; y < bottom; y++,z += DC.zwidth)
		MGL_memsetw(z,clearVal,size);
}

void _ASMAPI __EMU__zClear32(int left,int top,int right,int bottom,
	zfix32_t clearVal)
/****************************************************************************
*
* Function:		__EMU__zClear32
* Parameters:	left,top,right,bottom	- Rectangle in zbuffer to clear
*
* Description:	Clears the Zbuffer before rendering a new scene into it.
*
****************************************************************************/
{
	int		y;
	long 	size = (long)(right - left);
	zbuf32_t _HUGE *z = _MGL_zbufAddr32(left,top);

	if (!DC.zbuffer)
		MGL_fatalError("Zbuffer not allocated!\n");

	for (y = top; y < bottom; y++,z += DC.zwidth)
		MGL_memsetl(z,clearVal,size);
}

#endif

#endif	/* MGL_FIX3D */
