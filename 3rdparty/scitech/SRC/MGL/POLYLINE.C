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
* Description:	Polyline drawing routines
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
 *
 * NOTE: The polyline routines will not work correctly in any write
 * 		 mode other than REPLACE, since the vertices of the polygons will
 *		 be scan converted twice (and interconnecting pixels).
 *
 ***************************************************************************/

/* Macro to save the current fill style and color and replace it with
 * those required for drawing the markers. Note that we do not change the
 * current write mode, so you can XOR the markers if you wish.
 */

#define	preamble_markers()	svStyle = DC.a.penStyle;				\
							DC.a.penStyle = (MGL_BITMAP_SOLID);		\
							svColor = DC.a.color;					\
							MGL_setColor(DC.a.markerColor);			\
							svH = DC.a.penHeight;					\
							svW = DC.a.penWidth;

/* Macro to restore the current fill style and color after drawing some
 * markers.
 */

#define	postamble_markers()	DC.a.penStyle = svStyle;				\
							MGL_setColor(svColor);					\
							DC.a.penHeight = svH;					\
							DC.a.penWidth = svW;

static void drawMarker(point_t p)
/****************************************************************************
*
* Function:		drawMarker
* Parameters:	p	- Point to draw marker on
*
* Description:	Internal routine to plot a marker at point p in the current
*				global attributes.
*
****************************************************************************/
{
	rect_t	r;

	r.left = p.x - DC.a.markerSize + 1;
	r.right = p.x + DC.a.markerSize;
	r.top = p.y - DC.a.markerSize + 1;
	r.bottom = p.y + DC.a.markerSize;
	switch (DC.a.markerStyle) {
		case MGL_MARKER_SQUARE:
			MGL_fillRect(r);
			break;
		case MGL_MARKER_CIRCLE:
			MGL_fillEllipse(r);
			break;
		case MGL_MARKER_X:
			MGL_lineCoord(r.left,r.top,r.right-1,r.bottom-1);
			MGL_lineCoord(r.left,r.bottom-1,r.right-1,r.top);
			break;
		}
}

/****************************************************************************
DESCRIPTION:
Draws a marker at the specified coordinate.

HEADER:
mgraph.h

PARAMETERS:
p	- Coordinate to draw the marker at

REMARKS:
Draws a marker in the current marker color, style and size at the specified location.
Markers can be used to label the vertices in graphs. Refer to the MGL_markerStyleType
type for an enumeration of the types of markers supported.

SEE ALSO:
MGL_setMarkerSize, MGL_getMarkerSize, MGL_setMarkerStyle,
MGL_getMarkerStyle, MGL_polyMarker

****************************************************************************/
void MGLAPI MGL_marker(
	point_t p)
{
	int		svStyle;
	color_t	svColor;
	int		svH,svW;

	preamble_markers();
	drawMarker(p);
	postamble_markers();
}

/****************************************************************************
DESCRIPTION:
Draws a set of pixels.

HEADER:
mgraph.h

PARAMETERS:
count	- Number of vertices in polyline
vArray	- Array of coordinates to draw the pixels at

REMARKS:
This function draws a set of pixels in the current color at the locations passed in the
vArray parameter.

SEE ALSO:
MGL_polyLine, MGL_polyMarker
****************************************************************************/
void MGLAPI MGL_polyPoint(
	int count,
	point_t *vArray)
{
	MGL_beginPixel();
	while (count--) {
		MGL_pixel(*vArray);
		vArray++;
		}
	MGL_endPixel();
}

/****************************************************************************
DESCRIPTION:
Draws a set of markers.

HEADER:
mgraph.h

PARAMETERS:
count	- Number of vertices in polyline
vArray	- Array of coordinates to draw the markers at

REMARKS:
This function draws a set of markers in the current marker color, style and size at
the locations passed in the vArray parameter.

SEE ALSO:
MGL_polyLine, MGL_polyPoint
****************************************************************************/
void MGLAPI MGL_polyMarker(
	int count,
	point_t *vArray)
{
	int		svStyle;
	color_t	svColor;
	int		svH,svW;

	preamble_markers();
	while (count--) {
		drawMarker(*vArray);
		vArray++;
		}
	postamble_markers();
}

/****************************************************************************
DESCRIPTION:
Draws a set of connected lines.

HEADER:
mgraph.h

PARAMETERS:
count	- Number of vertices in polyline
vArray	- Array of vertices in the polyline

REMARKS:
This function draws a set of connected line (a polyline). The coordinates of the
polyline are specified by vArray, and the lines are drawn in the current drawing
attributes.

Note that the polyline is not closed by default, so if you wish to draw the outline of
a polygon, you will need to add the starting point to the end of the vertex array.

SEE ALSO:
MGL_polyMarker, MGL_polyPoint
****************************************************************************/
void MGLAPI MGL_polyLine(
	int count,
	point_t *vArray)
{
	int		i;
	point_t	*p1,*p2;

	p1 = vArray;
	p2 = vArray+1;
	i = count;
	while (--i) {
		MGL_line(*p1,*p2);
		p1 = p2++;
		}

	/* If drawing in XOR mode, draw pixels at the each line connection */
	if (DC.a.writeMode == MGL_XOR_MODE) {
		MGL_beginPixel();
		p2 = vArray+1;
		i = count-1;
		while (--i) {
			MGL_pixel(*p2);
			p2++;
			}
		MGL_endPixel();
		}
}
#endif	/* !MGL_LITE */
