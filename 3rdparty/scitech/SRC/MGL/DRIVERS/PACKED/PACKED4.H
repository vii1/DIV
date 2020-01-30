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
* Description:	4 bit packed pixel memory device driver header file.
*
*
****************************************************************************/

#ifndef	__DRIVERS_PACKED_PACKED4_H
#define	__DRIVERS_PACKED_PACKED4_H

#ifndef	__DRIVERS_COMMON_COMMON_H
#include "drivers\common\common.h"
#endif

/*---------------------- Macro and type definitions -----------------------*/

typedef struct {
	mem_vec	v;					/* Memory device vectors and info		*/
	} PACKED4_state;

/*------------------------- Function Prototypes ---------------------------*/

/* 1 bit mono C based routines */

ibool 	_ASMAPI PACKED4_initDriver(MGLDC *dc,int driverId,int modeId,ulong hwnd,int virtualX,int virtualY,int numBuffers,ibool stereo,int refreshRate);
color_t _ASMAPI PACKED4_getPixel(int x,int y);
void 	_ASMAPI PACKED4_putPixel(int x,int y);
void	_ASMAPI PACKED4_getScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);
void	_ASMAPI PACKED4_putScanLine(MGLDC *dc,int y,int x1,int x2,void *buffer,int bufx,int op);

#endif	/* __DRIVERS_PACKED_PACKED4_H */
