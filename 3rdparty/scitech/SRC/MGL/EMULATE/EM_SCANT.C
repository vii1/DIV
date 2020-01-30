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
* Description:	C based emulation routines for texture mapped scanline
*				rendering
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__textureScanLine(MGLDC *mgldc,int y,int x1,int x2)
/****************************************************************************
*
* Function:		__EMU__textureScanLine
* Parameters:	dc		- Device context
*				y		- Y coordinate to draw scanline at
*				x1,x2	- X coordinate extents for the scanline
*
* Description:	Draws a texture mapped scanline from (y,x1) to (y,x2)
*
****************************************************************************/
{
	mgldc = mgldc;
	y = y;
	x1 = x1;
	x2 = x2;
}
