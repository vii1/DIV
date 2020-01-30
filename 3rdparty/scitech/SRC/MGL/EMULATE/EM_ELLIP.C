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
* Description:	C based emulation routines for ellipse drawing.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*--------------------------- Global Variables ----------------------------*/

PRIVATE int			topY,botY;	/* Current top/bottom scan line			*/
PRIVATE int			left,right;	/* Current scan line extents			*/
PRIVATE void        (_ASMAPI *putPixel)(int x,int y);

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__fatPenEllipse(int left,int top,int A,int B,int clip)
/****************************************************************************
*
* Function:		fatPenEllipse
* Parameters:	left		- Left coorindate of ellipse rectangle
*				top			- Top coodinate of ellipse rectangle
*				A			- Major axis
*				B			- Minor axis
*				clip		- True if ellipse should be clipped
* Returns:		True if ellipse was scan converted, false on error.
*
* Description:	Private function to frame an ellipse with a wide pen in the
*				current color and fill style. The extent rectangle is
*				expected to be in local viewport coordinates. We use the
*				internal buffer to scan convert the ellipse.
*
*				We need a 3k buffer to scan convert an ellipse that is 768
*				scan lines high.
*
****************************************************************************/
{
	void		(_ASMAPI *drawScanLine)(int y,int x1,int x2);
	int			i,j,maxIndex;
	scanlist2	scanList;
	scanline2	*scanPtr;

	/* Scan the outline of the ellipse into the memory buffer */
	_MGL_scanFatEllipse(&scanList,left,top,A,B,DC.a.penWidth,DC.a.penHeight);

	/* Render the outline of the ellipse */
	maxIndex = scanList.top + scanList.length;
	scanPtr = scanList.scans;
	drawScanLine = clip ? _MGL_clipScanLine : DC.r.cur.scanLine;
	for (i = scanList.top,j = i + B+DC.a.penHeight; i < maxIndex; i++,j--,scanPtr++) {
		if (scanPtr->leftR < scanPtr->rightL) {
			drawScanLine(i,scanPtr->leftL,scanPtr->leftR);
			drawScanLine(i,scanPtr->rightL,scanPtr->rightR);
			drawScanLine(j,scanPtr->leftL,scanPtr->leftR);
			drawScanLine(j,scanPtr->rightL,scanPtr->rightR);
			}
		else {
			drawScanLine(i,scanPtr->leftL,scanPtr->rightR);
			drawScanLine(j,scanPtr->leftL,scanPtr->rightR);
			}
		}
	if ((B+DC.a.penHeight+1) & 1) {
		if (scanPtr->leftR < scanPtr->rightL) {
			drawScanLine(i,scanPtr->leftL,scanPtr->leftR);
			drawScanLine(i,scanPtr->rightL,scanPtr->rightR);
			}
		else {
			drawScanLine(i,scanPtr->leftL,scanPtr->rightR);
			}
		}
}

PRIVATE void _ASMAPI setup(int _topY,int _botY,int _left,int _right)
{
	topY = _topY;	botY = _botY;
	left = _left;	right = _right;
	MGL_beginPixel();
}

PRIVATE void _ASMAPI put4pixels(ibool incx,ibool incy,ibool region1)
{
	region1 = region1;
	left -= incx;	right += incx;
	topY += incy;	botY -= incy;
	putPixel(left,topY);
	putPixel(right,topY);
	if (topY != botY) {
		putPixel(left,botY);
		putPixel(right,botY);
		}
}

PRIVATE void _ASMAPI finished(void)
{
	MGL_endPixel();
}

void _ASMAPI __EMU__ellipse(int left,int top,int A,int B,int clip)
{
	if (DC.a.colorMode == MGL_DITHER_RGB_MODE)
		putPixel = clip ? _MGL_clipDitherPixel : DC.r.ditherPixel;
	else
		putPixel = clip ? _MGL_clipPixel : DC.r.putPixel;
	_MGL_ellipseEngine(left,top,A,B,setup,put4pixels,finished);
}

void _ASMAPI __EMU__fillEllipse(int left,int top,int A,int B,int clip)
/****************************************************************************
*
* Function:		__EMU__fillEllipse
* Parameters:	left	- Left coorindate of ellipse rectangle
*				top		- Top coodinate of ellipse rectangle
*				A		- Major axis
*				B		- Minor axis
*				clip	- True if ellipse should be clipped
*
* Description:	Emulates the filling of filled ellipses by rendering into
*				a memory buffer and then rendering each scanline in the
*				ellipse. This is the normal routine used to emulate this
*				for all drivers that cannot do ellipse rendering in hardware.
*
****************************************************************************/
{
	void		(_ASMAPI *drawScanLine)(int y,int x1,int x2);
	int			i,j,maxIndex;
	scanlist	scanList;
	scanline	*scanPtr;

	/* Scan the outline of the ellipse into the memory buffer */
	_MGL_scanEllipse(&scanList,left,top,A,B);

	/* Render the outline of the ellipse */
	maxIndex = scanList.top + scanList.length-1;
	drawScanLine = clip ? _MGL_clipScanLine : DC.r.cur.scanLine;
	scanPtr = scanList.scans;
	for (i = scanList.top; i < maxIndex; i++,scanPtr++) {
		drawScanLine(i,scanPtr->left,scanPtr->right);
		}
	if (!(B & 1))
		drawScanLine(i,scanPtr->left,scanPtr->right);
	scanPtr = scanList.scans;
	for (i = scanList.top,j = i + B; i < maxIndex; i++,j--,scanPtr++) {
		drawScanLine(j,scanPtr->left,scanPtr->right);
		}
}

#endif	/* !MGL_LITE */
