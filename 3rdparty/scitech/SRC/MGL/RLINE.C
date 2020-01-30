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
* Description:	Line region generation routines.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*--------------------------- Global Variables ----------------------------*/

static	region_t		*rgn;
static	const region_t	*pen;

/*------------------------- Implementation --------------------------------*/

#undef	MGL_rgnLineFX
#undef	MGL_rgnLine

static void _ASMAPI plotPoint(int x,int y)
{ MGL_unionRegionOfs(rgn,pen,x,y); }

/*******************************************************************************
DESCRIPTION:
Generate a fixed point line as a region.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate for first endpoint 
y1	- y coordinate for first endpoint
x2	- x coordinate for second endpoint
y2	- y coordinate for second endpoint
pen	- Region to use as the pen when drawing the line

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
Generates a region as a line starting at the point (x1,y1) and ending at the point
(x2,y2).

SEE ALSO:
MGL_rgnLineFX, MGL_rgnLine
*******************************************************************************/
region_t * MGLAPI MGL_rgnLineCoordFX(
	fix32_t x1,
	fix32_t y1,
	fix32_t x2,
	fix32_t y2,
	const region_t *_pen)
{
	if (!_pen->spans) {
		scanlist	scanList;

		_MGL_scanFatLine(&scanList,x1,y1,x2,y2,
			_pen->rect.bottom - _pen->rect.top-1,
			_pen->rect.right - _pen->rect.left-1);
		return _MGL_createRegionFromBuf(&scanList);
		}
	else {
		rgn = MGL_newRegion();
		pen = _pen;
		MGL_lineEngine(x1,y1,x2,y2,plotPoint);
		return rgn;
		}
}

/*******************************************************************************
DESCRIPTION:
Generate a fixed point line as a region.

HEADER:
mgraph.h

PARAMETERS:
p1	- First endpoint
p2	- Second endpoint
pen	- Region to use as the pen when drawing the line

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function is the same as MGL_rgnLineFX but takes the parameters for the
line as two points instead of coordinates.

SEE ALSO:
MGL_rgnLineCoordFX, MGL_rgnLine
*******************************************************************************/
region_t *MGL_rgnLineFX(
	fxpoint_t p1,
	fxpoint_t p2,
	const region_t *pen);
/* Implemented as a macro */

/****************************************************************************
DESCRIPTION:
Generate a line as a region.

HEADER:
mgraph.h

PARAMETERS:
x1	- x coordinate for first endpoint
y1	- y coordinate for first endpoint
x2	- x coordinate for second endpoint
y2	- y coordinate for second endpoint
pen	- Region to use as the pen when drawing the line

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
Generates a region as a line starting at the point (x1,y1) and ending at the point
(x2,y2). Note that this function takes the coordinates of the lines in integer format.
MGL draws all lines internally in 16.16 fixed point format, so this routine always
converts the coordinates and then calls the MGL_rgnLineFX routine. If you need
maximum performance, you should call the fixed point line drawing functions
instead.

SEE ALSO:
MGL_rgnLine, MGL_rgnLineFX
****************************************************************************/
region_t * MGLAPI MGL_rgnLineCoord(
	int x1,
	int y1,
	int x2,
	int y2,
	const region_t *pen)
{ return MGL_rgnLineCoordFX(MGL_TOFIX(x1),MGL_TOFIX(y1),MGL_TOFIX(x2),MGL_TOFIX(y2),pen); }

/****************************************************************************
DESCRIPTION:
Generate a line as a region.

HEADER:
mgraph.h

PARAMETERS:
p1	- First endpoint
p2	- Second endpoint
pen	- Region to use as the pen when drawing the line

RETURNS:
New region generated, NULL if out of memory.

REMARKS:
This function is the same as MGL_rgnLine but takes the parameters for the
line as two points instead of coordinates.

SEE ALSO:
MGL_rgnLineCoord, MGL_rgnLineFX
****************************************************************************/
region_t *MGL_rgnLine(
	point_t p1,
	point_t p2,
	const region_t *pen);
/* Implemented as a macro */

#endif	/* !MGL_LITE */

