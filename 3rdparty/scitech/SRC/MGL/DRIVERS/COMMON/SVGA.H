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
* Description:	Common header file for VESA VBE SuperVGA routines used by
*				all SuperVGA device drivers.
*
*
****************************************************************************/

#ifndef	__DRIVERS_COMMON_SVGA_H
#define	__DRIVERS_COMMON_SVGA_H

#ifndef	__DRIVERS_COMMON_VGA_H
#include "drivers\common\vga.h"
#endif

#ifndef	__VESAVBE_H
#include "vesavbe.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

#pragma pack(1)				/* Pack structures to byte granularity		*/

/* The following defines the internal SVGA device driver state buffer. A
 * copy of this is allocated for us in the device context created for
 * the device driver. This state buffer is used to maintain device specific
 * state information that is device context specific.
 */

typedef struct {
	display_vec	v;				/* Display device vectors and info		*/
	short		oldBIOSMode;	/* Old video mode before graphics		*/
	ibool		old50Lines;		/* 1 if old mode was 43/50 line mode	*/
	} SVGA_state;

#pragma pack()

/*------------------------- Function Prototypes ---------------------------*/

ibool	MGLAPI SVGA_detect(int *driver);
void 	MGLAPI SVGA_useMode(modetab modes,int mode,int id,gmode_t *modeInfo);
void 	MGLAPI SVGALIN_useMode(modetab modes,int mode,int id,gmode_t *modeInfo,ibool useVirtual);
ibool 	MGLAPI SVGA_setMode(MGLDC *dc,int AX,int BX,short *oldBIOSMode,ibool *old43Lines,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
void	_ASMAPI SVGA_startStereo(MGLDC *dc);
void	_ASMAPI SVGA_stopStereo(MGLDC *dc);
void 	_ASMAPI SVGA_restoreTextMode(MGLDC *dc);
ibool 	_ASMAPI SVGA_restoreGraphMode(MGLDC *dc);
void 	_ASMAPI SVGA_setActivePage(MGLDC *dc,int page);
void 	_ASMAPI SVGALIN_setActivePage(MGLDC *dc,int page);
void 	_ASMAPI SVGA_setVisualPage(MGLDC *dc,int page,int waitVRT);
void 	_ASMAPI SVGA_setDisplayStart(MGLDC *dc,int x,int y,int waitFlag);
void 	_ASMAPI SVGA_realizePalette(MGLDC *dc,palette_t *pal,int num,int index,int waitVRT);
void 	_ASMAPI SVGA_vSync(MGLDC *dc);
void	_ASMAPI SVGA_setBankC(int bank);
void 	SVGA_saveVideoMemory(int mode);
void 	SVGA_restoreVideoMemory(void);
void 	* _ASMAPI SVGA_getDefaultPalette(MGLDC *dc);

/* VESA GTF functions to generate CRTC timings */

ibool	GTF_useDoubleScan(MGLDC *dc,int refreshRate);
int 	GTF_getMaxRefreshRate(MGLDC *dc,ulong maxPixelClock,ibool interlaced);
ibool 	GTF_computeCRTCTimings(MGLDC *dc,int mode,int refreshRate,ibool interlaced,AF_CRTCInfo *crtc,ulong (_ASMAPI *getClosestPixelClock)(ushort mode,ulong pixelClock));

#endif	/* __DRIVERS_COMMON_SVGA_H */

