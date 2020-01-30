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
* Description:	C based emulation routines for scanline list rendering
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__drawScanList(int y,int length,short *scans)
/****************************************************************************
*
* Function:		__EMU__drawScanList
* Parameters:	y		- Y coordinate to begin rendering
*				length	- Number of scanlines in the list
*				scans	- Pointer to array of scanlines to render
*
* Description:	Renders the passed in list of scanlines beginning at
*				the specified y coordinate in the current drawing
*				attributes. This routine performs NO clipping.
*
****************************************************************************/
{
	int			i;

	for (i = 0; i < length; i++,scans += 2)
		DC.r.cur.scanLine(y+i,scans[0],scans[1]);
}

#endif	/* !MGL_LITE */
