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
* Description:  C based emulation routine for fat line line drawing.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__fatPenLine(fix32_t fx1,fix32_t fy1,fix32_t fx2,
	fix32_t fy2)
/****************************************************************************
*
* Function:		__EMU__fatPenLine
* Parameters:	fx1,fy1	- First endpoint of line to draw (fixed point)
*				fx2,fy2	- Second endpoint of line to draw (fixed point)
*
* Description:  Scan converts the image of a fat line into a buffer, and
*				displays it in the current viewport. Note that by the
*				time we get here, the line has already been clipped and
*				mapped to the current viewport.
*
*				NOTE:	This routine assumes that we are always scan
*						converting the line from right to left.
*
****************************************************************************/
{
	int			maxx;
	scanlist	scanList;

	maxx = _MGL_scanFatLine(&scanList,fx1,fy1,fx2,fy2,DC.a.penHeight,
		DC.a.penWidth);

	/* The line has been fully scan converted into the buffer, so draw all
	 * the scanlines in the list.
	 */
	if (DC.a.clip) {
		if (!MGL_ptInRectCoord(maxx,scanList.top+scanList.length,
				DC.intClipRect)) {
			_MGL_drawClippedScanList(scanList.top,scanList.length,&scanList.scans->left);
			return;
			}
		}
	DC.r.cur.drawScanList(scanList.top,scanList.length,&scanList.scans->left);
}

#endif	/* !MGL_LITE */
