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
* Description:	C based emulation routine for rendering a complex region.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__drawRegion(int x,int y,const region_t *r)
/****************************************************************************
*
* Function:		__EMU__drawRegion
* Parameters:	x,y	- Location to draw region at
*				r	- Pointer to region to draw
*
* Description:	Draws the representation of the region at the specified
*				location a scanline at a time. The region is rendered in
*				the current bitmap pattern and write mode, and rendered
*				without any clipping. When this routine gets called, it is
*				assumed that the region has already been clipped by the
*				high level code and will be rendered directly in screen
*				space. This routine also assumes that the region definition
*				is valid and is not a special cased rectangle.
*
****************************************************************************/
{
	segment_t	*seg;
	span_t		*s = r->spans;
	int			sy;

	y += (sy = s->y);
	while (s) {
		for (seg = s->seg; seg; seg = seg->next->next)
			DC.r.cur.scanLine(y,x+seg->x,x+seg->next->x);
		if (!s->next || (++sy == s->next->y))
			s = s->next;
		y++;
		}
}

#endif	/* !MGL_LITE */
