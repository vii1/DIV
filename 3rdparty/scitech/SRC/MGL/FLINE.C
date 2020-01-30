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
* Description:  Routines to scan a line drawn with a rectangular pen into
*				the scratch buffer for fast rendering. We special case as
*				much as possible for maximum speed.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*--------------------------- Global Variables ----------------------------*/

static	scanline	*scanPtr;
static	int			penHeight,penWidth;
static	int			oldy;

/*------------------------- Implementation --------------------------------*/

static void _ASMAPI plotPointDown(int x,int y)
/****************************************************************************
*
* Function:		plotPointDown
* Parameters:	x,y	- Position to plot the point at
*
* Description:	Routine to plot a point on a fat line to the buffer for a
*				line that is moving downwards in y. For the left hand side
*				of the line we only update the last bottom-most pixel. This
*				assumes that the scanline array will be set up with the
*				correct values for the first penHeight-1 left entries. We
*				also only update the first right coordinate for every pixel,
*				assuming that the correct values will be placed in the
*				buffer after the line is scan converted.
*
****************************************************************************/
{
	if (y != oldy) {
		scanPtr++;
		scanPtr[penHeight].left = x;
		}
	scanPtr->right = x+penWidth+1;
	oldy = y;
}

static void _ASMAPI plotPointUp(int x,int y)
/****************************************************************************
*
* Function:		plotPointUp
* Parameters:	x,y	- Position to plot the point at
*
* Description:	Routine to plot a point on a fat line to the buffer for a
*				line that is moving upwards in y. For the left hand side
*				of the line we only update the last upper-most pixel. This
*				assumes that the scanline array will be set up with the
*				correct values for the first penHeight-1 left entries. We
*				also only update the first right coordinate for every pixel,
*				assuming that the correct values will be placed in the
*				buffer after the line is scan converted.
*
****************************************************************************/
{
	if (y != oldy) {
		scanPtr--;
		scanPtr[-penHeight].left = x;
		}
	scanPtr->right = x+penWidth+1;
	oldy = y;
}

/* {secret} */
int _MGL_scanFatLine(scanlist *scanList,fix32_t fx1,fix32_t fy1,
	fix32_t fx2,fix32_t fy2,int _penHeight,int _penWidth)
/****************************************************************************
*
* Function:		_MGL_scanFatLine
* Parameters:	scanList	- Scanline list to fill in
*				fx1,fy1		- First endpoint of line to draw (fixed point)
*				fx2,fy2		- Second endpoint of line to draw (fixed point)
*               penHeight	- Height of the pen
*				penWidth	- Width of the pen
* Returns:		Maximum X coordinate
*
* Description:  Scans the image of a fat line into the scratch buffer.
*
*				NOTE:	This routine assumes that we are always scan
*						converting the line from right to left.
*
****************************************************************************/
{
	int		x1 = MGL_FIXROUND(fx1),y1 = MGL_FIXROUND(fy1);
	int		x2 = MGL_FIXROUND(fx2),y2 = MGL_FIXROUND(fy2);
	int		i,height = y2 - y1;
	int		length = ABS(height) + 1 + _penHeight;

	scanList->length = length;
	if (length * sizeof(scanline) > _MGL_bufSize)
		MGL_fatalError("Scratch buffer too small for rendering objects!\n");

	/* Fill in global variables */
	scanPtr = scanList->scans = _MGL_buf;
	oldy = y1;
	penHeight = _penHeight;
	penWidth = _penWidth;

	if ((x2 - x1) == 0) {
		/* We have a vertical line, so special case it */

		x2 += penWidth + 1;
		for (i = 0; i < length; i++,scanPtr++) {
			scanPtr->left = x1;
			scanPtr->right = x2;
			}
		scanPtr = _MGL_buf;
		}
	else if (height == 0) {
		/* We have a horizontal line, so special case it */

		x2 += penWidth + 1;
		for (i = 0; i < length; i++,scanPtr++) {
			scanPtr->left = x1;
			scanPtr->right = x2;
			}
		scanPtr = _MGL_buf;
		}
	else if (height > 0) {
		/* We have a downward moving line. Scan convert most of the line
		 * into the buffer by calling the lineEngine.
		 */

		MGL_lineEngine(fx1,fy1,fx2,fy2,plotPointDown);

		/* Now fix the first penHeight+1 scanline left coordinates and the
		 * last penHeight+1 scanline right coordinates.
		 */

		scanPtr = _MGL_buf;
		x2 += penWidth + 1;
		for (i = penHeight; i >= 0; i--) {
			scanPtr[i].left = x1;
			scanPtr[length-i-1].right = x2;
			}
		}
	else {
		/* We have a downward moving line. Scan convert most of the line
		 * into the buffer by calling the lineEngine.
		 */

		scanPtr += length-1;
		MGL_lineEngine(fx1,fy1,fx2,fy2,plotPointUp);

		/* Now fix the first penHeight+1 scanline left coordinates and the
		 * last penHeight+1 scanline right coordinates.
		 */

		scanPtr = _MGL_buf;
		x2 += penWidth + 1;
		for (i = penHeight; i >= 0; i--) {
			scanPtr[length-i-1].left = x1;
			scanPtr[i].right = x2;
			}
		}

	/* We need to clip the scan converted line now to the clipping
	 * rectangle, since some of the line MAY lie outside.
	 */
	if (height < 0)
		scanList->top = y2;
	else scanList->top = y1;
	return x2;
}
#endif
