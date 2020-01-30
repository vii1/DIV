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
* Language:		C++ 3.0
* Environment:	any
*
* Description:	Implementation for an integer rectangle class.
*
*
****************************************************************************/

#include "mgraph.hpp"

/*---------------------------- Implementation -----------------------------*/

inline ibool _isEmpty(int l,int t,int r,int b)
{
	return (b <= t || r <= l);
}

MGLRect& MGLRect::makeUnion(int left,int top,int right,int bottom)
/****************************************************************************
*
* Function:		MGLRect::makeUnion
* Parameters:	left	- Left coordinate of rectangle
*				top		- Top coordinate of rectangle
*				right	- Right coordinate of rectangle
*				bottom	- Bottom coordinate of rectangle
* Returns:      Reference to the current rectangle.
*
* Description:	Computes the union of two rectangles.
*
****************************************************************************/
{
	if (isEmpty()) {
		topLeft.x = left;
		topLeft.y = top;
		botRight.x = right;
		botRight.y = bottom;
		}
	else if (!_isEmpty(left,top,right,bottom)) {
		topLeft.x = MIN(topLeft.x, left);
		topLeft.y = MIN(topLeft.y, top);
		botRight.x = MAX(botRight.x, right);
		botRight.y = MAX(botRight.y, bottom);
		}
	return *this;
}

MGLRect& MGLRect::expand(int x,int y)
/****************************************************************************
*
* Function:		MGLRect::expand
* Parameters:	x,y	- Point to include in rectangle
* Returns:      Reference to the current rectangle.
*
* Description:	Expands the size of a rectangle to include the specified
*				point.
*
****************************************************************************/
{
	if (isEmpty()) {
		topLeft.x = x;
		topLeft.y = y;
		botRight.x = x+1;
		botRight.y = y+1;
		}
	else {
		topLeft.x = MIN(topLeft.x, x);
		topLeft.y = MIN(topLeft.y, y);
		botRight.x = MAX(botRight.x, x+1);
		botRight.y = MAX(botRight.y, y+1);
		}
	return *this;
}

MGLRect& MGLRect::makeIntersect(int left,int top,int right,int bottom)
/****************************************************************************
*
* Function:		MGLRect::makeIntersect
* Parameters:	left	- Left coordinate of rectangle
*				top		- Top coordinate of rectangle
*				right	- Right coordinate of rectangle
*				bottom	- Bottom coordinate of rectangle
* Returns:      Reference to the current rectangle.
*
* Description:	Computes the intersection of two rectangles.
*
****************************************************************************/
{
	topLeft.x = MAX(topLeft.x, left);
	topLeft.y = MAX(topLeft.y, top);
	botRight.x = MIN(botRight.x, right);
	botRight.y = MIN(botRight.y, bottom);
	if (isEmpty())
		empty();
	return *this;
}

ibool MGLRect::intersect(int left,int top,int right,int bottom) const
/****************************************************************************
*
* Function:		MGLRect::intersect
* Parameters:	left	- Left coordinate of rectangle
*				top		- Top coordinate of rectangle
*				right	- Right coordinate of rectangle
*				bottom	- Bottom coordinate of rectangle
* Returns:		True if the rectangles intersect
*
****************************************************************************/
{
	if (_isEmpty(MAX(topLeft.x, left), MAX(topLeft.y, top),
			MIN(botRight.x, right), MIN(botRight.y, bottom)))
		return false;
	return true;
}

ostream& operator << (ostream& o,const MGLRect& r)
/****************************************************************************
*
* Function:		operator <<
* Parameters:	o	- Stream to send rectangle to
*				r	- Rectangle to display
* Returns:		output stream used
*
****************************************************************************/
{
	return	o << r.topLeft << " - " << r.botRight;
}
