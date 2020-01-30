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
* Description:	Ellipse region generation routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*--------------------------- Global Variables ----------------------------*/

static	region_t		*rgn;
static const region_t	*pen;
static int				left,top,right,bot;

/*------------------------- Implementation --------------------------------*/

static void _ASMAPI pen_setup(int _topY,int _botY,int _left,int _right)
{ top = _topY;	bot = _botY; left = _left;	right = _right; }

static void _ASMAPI pen_set4pixels(ibool inc_x,ibool inc_y,ibool region1)
/****************************************************************************
*
* Function:		fill_set4pixels
* Parameters:	inc_x	- Increment x coordinates of current scan lines
*				inc_y	- Increment y coordinates of current scan lines
*
* Description:	Plots each of the four pixels for the ellipse by rendering
*				the pen at each of the four current locations.
*
****************************************************************************/
{
	region1 = region1;

	/* Draw the pen at all four locations */
	MGL_unionRegionOfs(rgn,pen,left,top);
	MGL_unionRegionOfs(rgn,pen,left,bot);
	MGL_unionRegionOfs(rgn,pen,right,top);
	MGL_unionRegionOfs(rgn,pen,right,bot);

	/* Update coordinates */
	if (inc_y) {
		top++;	bot--;
		}
	if (inc_x) {
		left--;	right++;
		}
}

static void _ASMAPI pen_finished(void) {}

/* {secret} */
region_t * _MGL_rgnEllipse(int left,int top,int A,int B,const region_t *_pen)
{
	pen = _pen;
	_MGL_ellipseEngine(left,top,A,B,pen_setup,pen_set4pixels,pen_finished);
	return rgn;
}

/****************************************************************************
DESCRIPTION:
Generate an ellipse outline as a region.

HEADER:
mgraph.h

PARAMETERS:
extentRect	- Bounding rectangle for the ellipse
pen	- Region to use as the pen when drawing the ellipse

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function generates the outline of an ellipse as a complex region by dragging
the specified pen region around the perimeter of the ellipse. The pen used can be
any arbitrary shape, however rectangular pens are special cased to provide fast
region generation.

SEE ALSO:
MGL_rgnEllipseArc
****************************************************************************/
region_t * MGLAPI MGL_rgnEllipse(
	rect_t extentRect,
	const region_t *_pen)
{
	int			A,B,i,j,maxIndex,penHeight,minx,maxx,l,lr,rl,r;
	scanlist2	scanList;
	scanline2	*scanPtr;
	span_t		**sp,*head,*st1,*st2,*tail;

	rgn = MGL_newRegion();
	A = extentRect.right - extentRect.left - 1;	/* Major Axis		*/
	B = extentRect.bottom - extentRect.top - 1;	/* Minor Axis		*/
	if (A == 0 || B == 0 || MGL_emptyRect(extentRect))
		return rgn;			/* Ignore degenerate ellipses			*/

	if (_pen->spans) {
		pen = _pen;
		_MGL_ellipseEngine(extentRect.left,extentRect.top,A,B,pen_setup,
			pen_set4pixels,pen_finished);
		}
	else {
		/* Scan the outline of a fat ellipse drawn with a rectangular pen
		 * into the scratch buffer
		 */
		_MGL_scanFatEllipse(&scanList,extentRect.left,extentRect.top,A,B,
			_pen->rect.right-_pen->rect.left-1,
			penHeight = _pen->rect.bottom-_pen->rect.top-1);

		/* Initialise the span list */
		head = NULL;			sp = &head;
		tail = _MGL_newSpan();	tail->y = scanList.top+B+penHeight+1;
		tail->seg = NULL;		tail->next = NULL;
		minx = 32767;			maxx = -32767;

		maxIndex = scanList.top + scanList.length;
		scanPtr = scanList.scans;
		for (i = scanList.top,j = tail->y-1; i < maxIndex; i++,j--,scanPtr++) {
			if (scanPtr->leftR < scanPtr->rightL) {
				*sp = st1 = _MGL_createSpan2(i,l = scanPtr->leftL,
					lr = scanPtr->leftR,rl = scanPtr->rightL,
					r = scanPtr->rightR);
				st2 = _MGL_createSpan2(j,l,lr,rl,r);
				minx = MIN(minx,l);		maxx = MAX(maxx,r);
				}
			else {
				*sp = st1 = _MGL_createSpan(i,l = scanPtr->leftL,r = scanPtr->rightR);
				st2 = _MGL_createSpan(j,l,r);
				minx = MIN(minx,l);		maxx = MAX(maxx,r);
				}
			sp = &st1->next;
			st2->next = tail;
			tail = st2;
			}

		/* Terminate the ellipse by joining top and bottom halves */
		if ((B+penHeight+1) & 1) {
			if (scanPtr->leftR < scanPtr->rightL) {
				*sp = st1 = _MGL_createSpan2(i,scanPtr->leftL,scanPtr->leftR,
					scanPtr->rightL,scanPtr->rightR);
				}
			else {
				*sp = st1 = _MGL_createSpan(i,scanPtr->leftL,scanPtr->rightR);
				}
			st1->next = tail;
			}
		else {
			*sp = tail;
			}

		/* Fill in the bounding rectangle for the region */
		rgn->rect.left = minx;
		rgn->rect.right = maxx;
		rgn->rect.top = scanList.top;
		rgn->rect.bottom = maxIndex;
		rgn->spans = head;
		}
	return rgn;
}

/****************************************************************************
DESCRIPTION:
Generates a solid ellipse as a region.


HEADER:
mgraph.h

PARAMETERS:
extentRect	- Bounding rectangle for the ellipse

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function generates a solid ellipse as a complex region

SEE ALSO:
MGL_rgnSolidEllipseArc
****************************************************************************/
region_t * MGLAPI MGL_rgnSolidEllipse(
	rect_t extentRect)
{
	int			A,B,i,j,maxIndex,minx,maxx,l,r;
	scanlist	scanList;
	scanline	*scanPtr;
	span_t		**sp,*head,*st,*tail;
	region_t	*rgn = MGL_newRegion();

	A = extentRect.right - extentRect.left - 1;	/* Major Axis		*/
	B = extentRect.bottom - extentRect.top - 1;	/* Minor Axis		*/
	if (A == 0 || B == 0 || MGL_emptyRect(extentRect))
		return rgn;		/* Ignore degenerate ellipses			*/

	/* Scan the outline of the ellipse into scratch buffer */
	_MGL_scanEllipse(&scanList,extentRect.left,extentRect.top,A,B);

	/* Initialise the span list */
	head = NULL;			sp = &head;
	tail = _MGL_newSpan();	tail->y = extentRect.bottom;
	tail->seg = NULL;		tail->next = NULL;
	minx = 32767;			maxx = -32767;

	/* Now build the region spans from the scratch buffer */
	maxIndex = scanList.top + scanList.length-1;
	scanPtr = scanList.scans;
	rgn->rect.top = i = scanList.top;
	rgn->rect.bottom = (j = i + B)+1;
	for (; i < maxIndex; i++,j--,scanPtr++) {
		/* Create a new span at top of ellipse 		*/
		*sp = st = _MGL_createSpan(i,l = scanPtr->left,l = scanPtr->right);
		sp = &st->next;

		/* Create a new span at bottom of ellipse	*/
		st = _MGL_createSpan(j,l = scanPtr->left,r = scanPtr->right);
		minx = MIN(minx,l);		maxx = MAX(maxx,r);
		st->next = tail;
		tail = st;
		}

	/* Terminate the ellipse by joining top and bottom halves */
	if (B & 1)
		*sp = tail;
	else {
		/* Ellipse height is odd, so create a single last span, and link
		 * the top and bottom span lists together
		 */
		*sp = st = _MGL_createSpan(i,l = scanPtr->left,r = scanPtr->right);
		minx = MIN(minx,l);		maxx = MAX(maxx,r);
		st->next = tail;
		}

	/* Fill in the bounding rectangle for the region */
	rgn->rect.left = minx;
	rgn->rect.right = maxx;
	rgn->spans = head;
	return rgn;
}

#endif	/* !MGL_LITE */
