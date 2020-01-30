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
* Description:	Common header file to all DirectDraw display drivers.
*
*               This module interfaces with DirectDraw and handles all of
*				the common DirectDraw specific code for the DirectDraw
*				drivers.
*
*
****************************************************************************/

#ifndef	__DRIVERS_COMMON_GFULLDIB_H
#define	__DRIVERS_COMMON_GFULLDIB_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)				/* Pack structures to byte granularity		*/

/* The following defines the internal FULLDIB device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	display_vec	v;				/* Display device vectors and info		*/
	} FULLDIB_state;

/* The following defines the internal WINDC device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	windowed_vec	v;			/* Windowed device vectors and info		*/
	} WINDC_state;

#pragma pack()

/*------------------------- Function Prototypes ---------------------------*/

/* Fullscreen DIB interfacing functions */
ibool	MGLAPI FULLDIB_detect(void);
void 	MGLAPI FULLDIB_useMode(modetab modes,int mode,int id,gmode_t *modeInfo);
ibool 	MGLAPI FULLDIB_setMode(MGLDC *dc);
void 	_ASMAPI FULLDIB_restoreTextMode(MGLDC *dc);

/* Fullscreen OpenGL functions */
ibool 	MGLAPI FDIBOPENGL_detect(void);
void 	MGLAPI FDIBOPENGL_useMode(modetab modes,int mode,int id,gmode_t *mi);
ibool 	MGLAPI FDIBOPENGL_setMode(MGLDC *dc);

/* Windowed interfacing functions */
int 	WIN_getBitsPerPixel(HDC hdc,DWORD *maskRed);
void 	WIN_findPixelFormat(MGLDC *dc,HDC hdc);
ulong 	_ASMAPI WIN_getWinDC(MGLDC *dc);
ibool 	_ASMAPI WIN_noZbuf(MGLDC *dc);
void * 	_ASMAPI WIN_getDefaultPalette(MGLDC *dc);
void 	_ASMAPI WIN_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);
void	_ASMAPI WIN_putImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine, MGLDC *src);
void	_ASMAPI WIN_translateImage(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op,void *surface,int bytesPerLine,int bitsPerPixel,palette_t *pal,pixel_format_t *pf,MGLDC *src);
void 	_ASMAPI WIN_stretchBlt1x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI WIN_stretchBlt2x2(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,palette_t *pal,ibool idPal);
void 	_ASMAPI WIN_stretchBlt(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,fix32_t zoomx,fix32_t zoomy,palette_t *pal,ibool idPal);

#endif	/* __DRIVERS_COMMON_GFULLDIB_H */

