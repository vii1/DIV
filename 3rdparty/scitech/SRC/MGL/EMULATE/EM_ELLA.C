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
* Description:	C based emulation routine for elliptical arc drawing.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__ellipseArc(int left,int top,int A,int B,
	int startAngle,int endAngle,int clip)
/****************************************************************************
*
* Function:		__EMU__ellipseArc
* Parameters:   left		- Left coorindate of ellipse rectangle
*				top			- Top coodinate of ellipse rectangle
*				A			- Major axis
*				B			- Minor axis
*				startAngle	- Starting angle for arc (in degrees)
*				endAngle	- Ending angle for arc (in degress)
*				clip		- True if ellipse should be clipped
*
* Description:	Draw the frame of an elliptical arc just inside the
*				mathematical boundary of extentRect.
*
****************************************************************************/
{
	arc_coords_t    ac;
	void        	(_ASMAPI *putPixel)(int x,int y);

	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		putPixel = clip ? _MGL_clipDitherPixel : DC.r.ditherPixel;
	else
		putPixel = clip ? _MGL_clipPixel : DC.r.putPixel;
	MGL_beginPixel();
	_MGL_ellipseArcEngine(left,top,A,B,startAngle,endAngle,&ac,putPixel);
	MGL_endPixel();

	/* Save the arc coordinate information in the device context */
	DC.ac.x = ac.x - DC.a.viewPort.left;
	DC.ac.y = ac.y - DC.a.viewPort.top;
	DC.ac.startX = ac.startX - DC.a.viewPort.left;
	DC.ac.startY = ac.startY - DC.a.viewPort.top;
	DC.ac.endX = ac.endX - DC.a.viewPort.left;
	DC.ac.endY = ac.endY - DC.a.viewPort.top;
}

#define	_MGL_tmpRectRegion3(rgn,l,t,r,b)	\
{											\
	_MGL_rtmp2.rect.left = l;				\
	_MGL_rtmp2.rect.top = t;				\
	_MGL_rtmp2.rect.right = r;				\
	_MGL_rtmp2.rect.bottom = b;				\
	(rgn) = &_MGL_rtmp2;					\
}

void _ASMAPI __EMU__fatPenEllipseArc(int left,int top,int A,int B,
	int startAngle,int endAngle,int clip)
/****************************************************************************
*
* Function:		__EMU__fatPenEllipseArc
* Parameters:   left		- Left coorindate of ellipse rectangle
*				top			- Top coodinate of ellipse rectangle
*				A			- Major axis
*				B			- Minor axis
*				startAngle	- Starting angle for arc (in degrees)
*				endAngle	- Ending angle for arc (in degress)
*				clip		- True if ellipse should be clipped
*
* Description:	Draw the frame of an elliptical arc just inside the
*				mathematical boundary of extentRect using a fat pen.
*
****************************************************************************/
{
#if 0
	/* TODO: This needs to be fixed */
	arc_coords_t    ac;
	rect_t			extentRect;
	region_t		*rgn,*pen;

	_MGL_tmpRectRegion3(pen,0,0,DC.a.penWidth+1,DC.a.penHeight+1);
	extentRect.left = left;
	extentRect.top = top;
	extentRect.right = left + A + 1;
	extentRect.bottom = top + B + 1;
	rgn = MGL_rgnEllipseArc(extentRect,startAngle,endAngle,pen);
	MGL_drawRegion(0,0,rgn);
	MGL_freeRegion(rgn);

	/* Save the arc coordinate information in the device context */
	MGL_rgnGetArcCoords(&ac);
	DC.ac.x = ac.x - DC.a.viewPort.left;
	DC.ac.y = ac.y - DC.a.viewPort.top;
	DC.ac.startX = ac.startX - DC.a.viewPort.left;
	DC.ac.startY = ac.startY - DC.a.viewPort.top;
	DC.ac.endX = ac.endX - DC.a.viewPort.left;
	DC.ac.endY = ac.endY - DC.a.viewPort.top;
	clip = clip;
#endif
	__EMU__ellipseArc(left,top,A,B,startAngle,endAngle,clip);
}

void _ASMAPI __EMU__fillEllipseArc(int left,int top,int A,int B,
	int startAngle,int endAngle,int clip)
/****************************************************************************
*
* Function:		__EMU__fillEllipseArc
* Parameters:	left		- Left coorindate of ellipse rectangle
*				top			- Top coodinate of ellipse rectangle
*				A			- Major axis
*				B			- Minor axis
*				startAngle	- Starting angle for arc (in degrees)
*				endAngle	- Ending angle for arc (in degress)
*				clip		- True if ellipse should be clipped
*
* Description:	Fills an elliptical arc (or wedge) just inside the
*				mathematical boundary of extentRect.
*
****************************************************************************/
{
	void			(_ASMAPI *drawScanLine)(int y,int x1,int x2);
	int				i,lastIndex,leftL,leftR,rightL,rightR;
	arc_coords_t	ac;
	scanlist2		scanList;
	scanline2		*s;

	/* Scan the outline of the ellipse into the memory buffer */
	_MGL_scanEllipseArc(&scanList,&ac,left,top,A,B,startAngle,endAngle);

	/* Draw the list of scanlines to be drawn. The following code looks
	 * complicated, but it needs to take into account all the different
	 * types of arrangements of the scanlines that can occur.
	 */
	s = scanList.scans;
	lastIndex = scanList.top + scanList.length;
	drawScanLine = clip ? _MGL_clipScanLine : DC.r.cur.scanLine;

	for (i = scanList.top; i < lastIndex; i++,s++) {
		leftL = s->leftL;	rightR = s->rightR;
		if (leftL == SENTINEL && rightR == SENTINEL)
			continue;
		leftR = s->leftR;	rightL = s->rightL;

		if (leftR < rightL) {
			drawScanLine(i,leftL,leftR);
			if (rightL != SENTINEL)
				drawScanLine(i,rightL,rightR);
			}
		else {
			if (leftL == SENTINEL)
				drawScanLine(i,rightL,rightR);
			else
				drawScanLine(i,leftL,rightR);
			}
		}

	/* Save the arc coordinate information in the device context */
	DC.ac.x = ac.x - DC.a.viewPort.left;
	DC.ac.y = ac.y - DC.a.viewPort.top;
	DC.ac.startX = ac.startX - DC.a.viewPort.left;
	DC.ac.startY = ac.startY - DC.a.viewPort.top;
	DC.ac.endX = ac.endX - DC.a.viewPort.left;
	DC.ac.endY = ac.endY - DC.a.viewPort.top;
}

void _ASMAPI __EMU__getArcCoords(arc_coords_t *coords)
{ *coords = DC.ac; }

#endif	/* !MGL_LITE */
