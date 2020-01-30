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
* Description:	C based emulation routine for rectangle filling
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__fillRect(int x1,int y1,int x2,int y2)
/****************************************************************************
*
* Function:		__EMU__fillRect
* Parameters:	x1,y1	- Top left coordinate of rectangle
*				x2,y2	- Bottom right coordinate of rectangle
*
* Description:	Fills the specified rectangle by calling the device
*				driver scanline filling routine.
*
****************************************************************************/
{
	int	i;

	for (i = y1; i < y2; i++)
		DC.r.cur.scanLine(i,x1,x2);
}
