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
* Description:  C based emulation routine for line drawing.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__line(fix32_t x1,fix32_t y1,fix32_t x2,
	fix32_t y2)
/****************************************************************************
*
* Function:		__EMU__line
* Parameters:	x1,y1	- First endpoint for line
*				x2,y2	- Second endpoint for line
*
* Description:  Renders a line using the line engine and a local putPixel
*				routine (non-clipped).
*
****************************************************************************/
{
	MGL_beginPixel();
#ifndef	MGL_LITE
	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		MGL_lineEngine(x1,y1,x2,y2,DC.r.ditherPixel);
	else
#endif
		MGL_lineEngine(x1,y1,x2,y2,DC.r.putPixel);
	MGL_endPixel();
}
