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
* Description:	Text output and control routines.
*
*
****************************************************************************/

#include "mgl.h"

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

/****************************************************************************
DESCRIPTION:
Returns the height of the current font in pixels.

HEADER:
mgraph.h

RETURNS:
Height of the current font in pixels

REMARKS:
Returns the height of the currently active font in pixels. This includes any scaling
transformations that are applied to the font and will be as accurate as possible at the
resolution of the display device.

SEE ALSO:
MGL_textWidth, MGL_drawStr, MGL_getCharMetrics, MGL_getFontMetrics
****************************************************************************/
int MGLAPI MGL_textHeight(void)
{
	font_t *font = DC.a.ts.font;

	if (font == NULL) return 0;

	switch (font->fontType) {
		case MGL_VECTORFONT:
			return (DC.a.ts.szNumery *
					(font->ascent - font->descent + 1 + font->leading))
					/ DC.a.ts.szDenomy;
		case MGL_FIXEDFONT:
		case MGL_PROPFONT:
			return font->ascent - font->descent + 1 + font->leading;
		default:
			return 0;
		}
}

static int charWidth(uchar ch)
/****************************************************************************
*
* Function:		charWidth
* Parameters:	ch	- Character to measure
* Returns:		Width of the character in pixels.
*
****************************************************************************/
{
	font_t *font = DC.a.ts.font;

	if (font == NULL) return 0;

	switch (font->fontType) {
		case MGL_FIXEDFONT:
			return font->fontWidth;
		case MGL_VECTORFONT:
			if (VEC_ISMISSING(font,ch))
				return (DC.a.ts.szNumerx *
						VECFONT(font)->width[MISSINGSYMBOL])
						/ DC.a.ts.szDenomx + DC.a.ts.spaceExtra;
			else
				return (DC.a.ts.szNumerx *
						VECFONT(font)->width[ch])
						/ DC.a.ts.szDenomx + DC.a.ts.spaceExtra;
		case MGL_PROPFONT:
			if (BIT_ISMISSING(font,ch))
				return BITFONT(font)->width[MISSINGSYMBOL] + DC.a.ts.spaceExtra;
			else
				return BITFONT(font)->width[ch] + DC.a.ts.spaceExtra;
		default:
			return 0;
		}
}

/****************************************************************************
DESCRIPTION:
Returns the width of a character in pixels.

HEADER:
mgraph.h

PARAMETERS:
ch	- Character to measure

RETURNS:
Width of the character in pixels (will depend on currently active font)

REMARKS:
Return the width of the specified character, given the currently active font and
attribute settings.

SEE ALSO:
MGL_textWidth, MGL_textHeight, MGL_useFont

****************************************************************************/
int MGLAPI MGL_charWidth(
	char ch)
{
	return charWidth(ch);
}

/****************************************************************************
DESCRIPTION:
Returns the width of the character string in pixels.

HEADER:
mgraph.h

PARAMETERS:
str	- Character string to measure

RETURNS:
Width of the character string in pixels

REMARKS:
Returns the width of the specified character string using the dimensions of the
currently active font in pixels. This includes any scaling transformations that are
applied to the font and will be as accurate as possible at the resolution of the display
device.

SEE ALSO:
MGL_textHeight, MGL_drawStr, MGL_getCharMetrics, MGL_getFontMetrics
****************************************************************************/
int MGLAPI MGL_textWidth(
	const char *str)
{
	int	width = 0;

	if (DC.a.ts.font == NULL) return 0;

	if (str) {
		switch (DC.a.ts.font->fontType) {
			case MGL_FIXEDFONT:
				return strlen(str) * DC.a.ts.font->fontWidth;
			case MGL_VECTORFONT:
			case MGL_PROPFONT:
				while (*str)
					width += charWidth(*str++);
				break;
			default:
				return 0;
			}
		}
	return width;
}

/****************************************************************************
DESCRIPTION:
Compute the bounding box for a text string.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate string would be drawn at
y	- y coordinate string would be drawn at
str	- String to measure
bounds	- Place to store the computed bounds

REMARKS:
This function computes the bounding box that fits tightly around a text string drawn
at a specified location on the current device context. This routine correctly
computes the bounding rectangle for the string given the current text justification,
size and direction settings.

SEE ALSO:
MGL_textHeight, MGL_textWidth
****************************************************************************/
void MGLAPI MGL_textBounds(
	int x,
	int y,
	const char *str,
	rect_t *bounds)
{
	text_settings_t	ts = DC.a.ts;
	font_t			*font = ts.font;
	int				newx,newy,width,height;

	if (!str || !font)
		return;

	/* Determine dimensions and offset of bounding rectangle */
	width = MGL_textWidth(str);
	height = (font->ascent - font->descent + 1);
	newx =	(ts.horizJust == MGL_LEFT_TEXT ? 	0 :
			(ts.horizJust == MGL_CENTER_TEXT ? 	width/2 :
												width));
	newy = 	(ts.vertJust == MGL_TOP_TEXT ? 		0 :
			(ts.vertJust == MGL_CENTER_TEXT ?	(height-1)/2 :
			(ts.vertJust == MGL_BOTTOM_TEXT ?	height-1 :
												font->ascent)));
	if (font->fontType == MGL_VECTORFONT)
		newy = (ts.szNumery * newy) / ts.szDenomy;

	/* Build the bounding rectangle depending on text direction */
	switch (ts.dir) {
		case MGL_RIGHT_DIR:
			bounds->left = x - newx;
			bounds->right = bounds->left + width;
			bounds->top = y - newy;
			bounds->bottom = bounds->top + height;
			break;
		case MGL_UP_DIR:
			bounds->left = x - newy;
			bounds->right = bounds->left + height;
			bounds->bottom = y + newx + 1;
			bounds->top = bounds->bottom - width;
			break;
		case MGL_LEFT_DIR:
			bounds->right = x + newx + 1;
			bounds->left = bounds->right - width;
			bounds->bottom = y + newy + 1;
			bounds->top = bounds->bottom - height;
			break;
		case MGL_DOWN_DIR:
			bounds->right = x + newy + 1;
			bounds->left = bounds->right - height;
			bounds->top = y - newx;
			bounds->bottom = bounds->top + width;
			break;
		}
}

/****************************************************************************
DESCRIPTION:
Returns the maximum character width for current font.

HEADER:
mgraph.h

RETURNS:
Maximum character width for current font.

REMARKS:
Returns the maximum character width for the currently active font. You can use this
routine to quickly determine if a character will possibly overlap something else on
the device surface.

SEE ALSO:
MGL_getCharMetrics, MGL_getFontMetrics, MGL_textHeight, MGL_textWidth,
MGL_charWidth

****************************************************************************/
int MGLAPI MGL_maxCharWidth(void)
{
	if (DC.a.ts.font == NULL) return 0;

	switch (DC.a.ts.font->fontType) {
		case MGL_PROPFONT:
		case MGL_FIXEDFONT:
			return DC.a.ts.font->maxWidth;
		case MGL_VECTORFONT:
			return (DC.a.ts.szNumerx * DC.a.ts.font->maxWidth)
					/ DC.a.ts.szDenomx + DC.a.ts.spaceExtra;
		default:
			return 0;
		}
}

/****************************************************************************
DESCRIPTION:
Returns the currently active font metrics.

HEADER:
mgraph.h

PARAMETERS:
metrics	- Place to store the font metrics

REMARKS:
This function computes the font metrics for the current font. The metrics are
computed in pixels and will be as accurate as possible given the current font's
scaling factor (only vector fonts can be scaled however).

SEE ALSO:
MGL_getCharMetrics

****************************************************************************/
void MGLAPI MGL_getFontMetrics(
	metrics_t *m)
{
	font_t	*font = DC.a.ts.font;

	switch (font->fontType) {
		case MGL_VECTORFONT: {
			text_settings_t	ts = DC.a.ts;
			m->width = (ts.szNumerx * font->maxWidth) / ts.szDenomx;
			m->fontWidth = m->width;
			m->fontHeight = (ts.szNumerx * font->fontHeight) / ts.szDenomx;
			m->ascent = (ts.szNumery * font->ascent) / ts.szDenomy;
			m->descent = (ts.szNumery * font->descent) / ts.szDenomy;
			m->leading = (ts.szNumery * font->leading) / ts.szDenomy;
			m->kern = (ts.szNumerx * font->maxKern) / ts.szDenomx;
			break;
			}
		default:
			m->width = font->maxWidth;
			m->fontWidth = font->fontWidth;
			m->fontHeight = font->fontHeight;
			m->ascent = font->ascent;
			m->descent = font->descent;
			m->leading = font->leading;
			m->kern = font->maxKern;
			break;
		}
}

/****************************************************************************
DESCRIPTION:
Computes the character metrics for a specific character.

HEADER:
mgraph.h

PARAMETERS:
ch	- Character to measure
metrics	- Place to store the resulting metrics
REMARKS:
This function computes the character metrics for a specific character. The character
metrics define specific characters width, height, ascent, descent and other values.
These values can then be used to correctly position the character with pixel precise
positioning.

All values are defined in pixels and will be as accurate as possible given the current
fonts scaling factor (only vector fonts can be scaled).

SEE ALSO:
MGL_getFontMetrics

****************************************************************************/
void MGLAPI MGL_getCharMetrics(
	char ch,
	metrics_t *m)
{
	int				i,j,offset,ascent,descent,fontwidth,kern;
	int				dx,dy,byteWidth,fontAscent;
	text_settings_t	ts = DC.a.ts;
	font_t			*font = ts.font;
	vector			*vec;
	uchar			*bytes;

	if (font == NULL) return;

	switch (font->fontType) {
		case MGL_VECTORFONT:
			if ((offset = VECFONT(font)->offset[(uchar)ch]) == -1)
				offset = VECFONT(font)->offset[MISSINGSYMBOL];
			vec = (vector *)&VECFONT(font)->def[offset];

			/* Perform the sequence of operations to draw the character */
			ascent = descent = fontwidth = kern = 0;
			while (vec->op.op1) {
				dx = vec->op.dx;
				dy = vec->op.dy;
				ascent = MAX(ascent,dy);
				descent = MIN(descent,dy);
				fontwidth = MAX(fontwidth,dx);
				kern = MIN(kern,dx);
				vec++;
				}

			/* Store the computed values */
			m->width = (ts.szNumerx *
						VECFONT(font)->width[(uchar)ch])
						/ ts.szDenomx;
			m->fontWidth = (ts.szNumerx * (fontwidth+1)) / ts.szDenomx;
			m->ascent = (ts.szNumery * ascent) / ts.szDenomy;
			m->descent = (ts.szNumery * descent) / ts.szDenomy;
			m->leading = (ts.szNumery * font->leading) / ts.szDenomy;
			m->fontHeight = m->ascent - m->descent + 1;
			m->kern = (ts.szNumerx * kern) / ts.szDenomx;
			break;
		case MGL_PROPFONT:
		case MGL_FIXEDFONT:
			byteWidth = BITFONT(font)->iwidth[(uchar)ch];
			if ((offset = BITFONT(font)->offset[(uchar)ch]) == -1) {
				offset = BITFONT(font)->offset[MISSINGSYMBOL];
				byteWidth = BITFONT(font)->iwidth[MISSINGSYMBOL];
				}
			bytes = (uchar *)&BITFONT(font)->def[offset];
			byteWidth = (byteWidth + 7) / 8;

			/* Find the ascent and descent values for the character, by
			 * searching for the first non-zero line from the top and
			 * bottom of the bitmap definition
			 */
			offset = 0;
			dx = FALSE;
			fontAscent = font->fontHeight + font->descent - 1;
			for (i = 0; i < fontAscent; i++) {
				for (j = 0; j < byteWidth; j++)
					if (bytes[offset++] != 0) {
						dx = TRUE;
						break;
						}
				if (dx == TRUE)
					break;
				}
			ascent = fontAscent - i;

			offset = (font->fontHeight-1) * byteWidth + byteWidth-1;
			dx = FALSE;
			for (i = font->fontHeight-1; i > fontAscent; i--) {
				for (j = 0; j < byteWidth; j++)
					if (bytes[offset--] != 0) {
						dx = TRUE;
						break;
						}
				if (dx == TRUE)
					break;
				}
			descent = fontAscent - i;

			m->width = BITFONT(font)->width[(uchar)ch];
			m->fontWidth = BITFONT(font)->iwidth[(uchar)ch];
			m->ascent = ascent;
			m->descent = descent;
			m->leading = font->leading;
			m->fontHeight = m->ascent - m->descent + 1;
			m->kern = -BITFONT(font)->loc[(uchar)ch];
			break;
		default:
			m->width = font->maxWidth;
			m->fontWidth = font->fontWidth;
			m->ascent = font->ascent;
			m->descent = font->descent;
			m->leading = font->leading;
			m->fontHeight = m->ascent - m->descent + 1;
			m->kern = font->maxKern;
			break;
		}
}

static void drawStrVec(	int x,int y,const char *str)
/****************************************************************************
*
* Function:		drawStrVec
* Parameters:	x	- X coordinate to draw at
*				y	- Y coordinate to draw at
*				str	- String to draw
*
* Description:	Draws the specified string at the position (x,y) in
*				the current drawing color, write mode, text direction
*				and justification using the current vector font.
*
*				In order to draw the vector font in a different direction
*				we rotate the glyphs on the fly.
*
****************************************************************************/
{
	int				i,width,height,ascent,descent,offset,maxWidth;
	int				maxKern,fontAscent,sz_nx,sz_ny,sz_dx,sz_dy;
	vector			*vec;
	text_settings_t	ts = DC.a.ts;
	font_t			*font = ts.font;
	ibool			useSlow;
	rect_t			charRect,d,clipRect;

	/* Compute the scaled ascent, descent, maxKern and maxWidth values */
	ascent = (ts.szNumery * font->ascent) / ts.szDenomy;
	descent = (ts.szNumery * font->descent) / ts.szDenomy;
	maxKern = (ts.szNumerx * font->maxKern) / ts.szDenomx;
	maxWidth = (ts.szNumerx * font->maxWidth) / ts.szDenomx;
	fontAscent = font->fontHeight + font->descent - 1;
	fontAscent = (ts.szNumery * fontAscent) / ts.szDenomy;

	/* If we are drawing with patterned lines, or with a wide pen, then
	 * call the slow drawing routine, which works properly in these
	 * cases
	 */
	useSlow = (DC.a.penStyle != MGL_BITMAP_SOLID ||
			   DC.a.penHeight != 0 || DC.a.penWidth != 0);

	/* Save the clipping rectangle in local variable */
	clipRect = DC.intClipRect;

	/* Determine the current scaling factors to use for scan converting
	 * each character
	 */
	if (ts.dir == MGL_LEFT_DIR || ts.dir == MGL_RIGHT_DIR) {
		sz_nx = ts.szNumerx;		sz_dx = ts.szDenomx;
		sz_ny = ts.szNumery;		sz_dy = ts.szDenomy;
		}
	else {
		/* Swap them over for vertical chars	*/
		sz_nx = ts.szNumery;		sz_dx = ts.szDenomy;
		sz_ny = ts.szNumerx;		sz_dy = ts.szDenomx;
		}

	/* Adjust the first characters origin depending on the vertical
	 * justification setting.
	 */
	switch (ts.vertJust) {
		case MGL_CENTER_TEXT:
			height = (ascent + descent + 1)/2;
			y += (ts.dir == MGL_RIGHT_DIR ? height :
				 (ts.dir == MGL_LEFT_DIR ? -height : 0));
			x += (ts.dir == MGL_UP_DIR ? height :
				 (ts.dir == MGL_DOWN_DIR ? -height : 0));
			break;
		case MGL_TOP_TEXT:
			y += (ts.dir == MGL_RIGHT_DIR ? ascent :
				 (ts.dir == MGL_LEFT_DIR ? -ascent : 0));
			x += (ts.dir == MGL_UP_DIR ? ascent :
				 (ts.dir == MGL_DOWN_DIR ? -ascent : 0));
			break;
		case MGL_BOTTOM_TEXT:
			y += (ts.dir == MGL_RIGHT_DIR ? descent :
				 (ts.dir == MGL_LEFT_DIR ? -descent : 0));
			x += (ts.dir == MGL_UP_DIR ? descent :
				 (ts.dir == MGL_DOWN_DIR ? -descent : 0));
			break;

		/* We dont need to adjust the coordinates for BASELINE_TEXT */
		}

	/* Now draw the string using a different loop for each of the four
	 * text drawing directions. Note that we maintain a character clipping
	 * rectangle through each loop, and check to see if the character must
	 * be clipped, calling a slower clipping routine if need be.
	 */
	width = MGL_textWidth(str);

	switch (ts.dir) {
		case MGL_RIGHT_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? x :
				(ts.horizJust == MGL_CENTER_TEXT ?
					x - width/2 :
					x - width));

			if (!useSlow) {
				charRect.left = i + maxKern;
				charRect.right = i + maxWidth + 1 + DC.a.penWidth;
				charRect.top = y - fontAscent;
				charRect.bottom = y - descent + 1 + DC.a.penHeight;
				}

			while (*str) {
				if ((offset = VECFONT(font)->offset[(uchar)*str]) == -1)
					offset = VECFONT(font)->offset[MISSINGSYMBOL];
				vec = (vector *)&VECFONT(font)->def[offset];
				MGL_unionRect(charRect,clipRect,&d);
				if (useSlow)
					_MGL_drawCharVecSlow(i,y,vec,sz_nx,sz_ny,sz_dx,sz_dy);
				else if (!MGL_equalRect(clipRect,d)) {
					if (!MGL_disjointRect(charRect,clipRect))
						_MGL_drawCharVecSlow(i,y,vec,sz_nx,sz_ny,sz_dx,sz_dy);
					}
				else
					DC.r.drawCharVec(i,y,vec,sz_nx,sz_ny,sz_dx,sz_dy);

				i += (width = charWidth(*str++));
				charRect.left += width;
				charRect.right += width;
				}
			break;
		case MGL_UP_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? y :
				(ts.horizJust == MGL_CENTER_TEXT ?
					y + width/2 :
					y + width));

			if (!useSlow) {
				charRect.left = x - fontAscent;
				charRect.right = x - descent + 1 + DC.a.penWidth;
				charRect.top = i - maxWidth;
				charRect.bottom = i - maxKern + 1 + DC.a.penHeight;
				}

			while (*str) {
				if ((offset = VECFONT(font)->offset[(uchar)*str]) == -1)
					offset = VECFONT(font)->offset[MISSINGSYMBOL];
				vec = (vector *)&VECFONT(font)->def[offset];
				_MGL_rotateVector(vec,MGL_UP_DIR);
				MGL_unionRect(charRect,clipRect,&d);
				if (useSlow)
					_MGL_drawCharVecSlow(x,i,vec,sz_nx,sz_ny,sz_dx,sz_dy);
				else if (!MGL_equalRect(clipRect,d)) {
					if (!MGL_disjointRect(charRect,clipRect))
						_MGL_drawCharVecSlow(x,i,vec,sz_nx,sz_ny,sz_dx,sz_dy);
					}
				else
					DC.r.drawCharVec(x,i,vec,sz_nx,sz_ny,sz_dx,sz_dy);
				_MGL_rotateVector(vec,MGL_DOWN_DIR);

				i -= (width = charWidth(*str++));
				charRect.top -= width;
				charRect.bottom -= width;
				}
			break;
		case MGL_LEFT_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? x :
				(ts.horizJust == MGL_CENTER_TEXT ?
					x + width/2 :
					x + width));

			if (!useSlow) {
				charRect.left = i - maxWidth;
				charRect.right = i - maxKern + 1 + DC.a.penWidth;
				charRect.top = y + descent;
				charRect.bottom = y + fontAscent + 1 + DC.a.penHeight;
				}

			while (*str) {
				if ((offset = VECFONT(font)->offset[(uchar)*str]) == -1)
					offset = VECFONT(font)->offset[MISSINGSYMBOL];
				vec = (vector *)&VECFONT(font)->def[offset];
				_MGL_rotateVector(vec,MGL_LEFT_DIR);
				MGL_unionRect(charRect,clipRect,&d);
				if (useSlow)
					_MGL_drawCharVecSlow(i,y,vec,sz_nx,sz_ny,sz_dx,sz_dy);
				else if (!MGL_equalRect(clipRect,d)) {
					if (!MGL_disjointRect(charRect,clipRect))
						_MGL_drawCharVecSlow(i,y,vec,sz_nx,sz_ny,sz_dx,sz_dy);
					}
				else
					DC.r.drawCharVec(i,y,vec,sz_nx,sz_ny,sz_dx,sz_dy);
				_MGL_rotateVector(vec,MGL_LEFT_DIR);

				i -= (width = charWidth(*str++));
				charRect.left -= width;
				charRect.right -= width;
				}
			break;
		case MGL_DOWN_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? y :
				(ts.horizJust == MGL_CENTER_TEXT ?
					y - width/2 :
					y - width));

			if (!useSlow) {
				charRect.left = x + descent;
				charRect.right = x + fontAscent + 1 + DC.a.penWidth;
				charRect.top = i + maxKern;
				charRect.bottom = i + maxWidth + 1 + DC.a.penHeight;
				}

			while (*str) {
				if ((offset = VECFONT(font)->offset[(uchar)*str]) == -1)
					offset = VECFONT(font)->offset[MISSINGSYMBOL];
				vec = (vector *)&VECFONT(font)->def[offset];
				_MGL_rotateVector(vec,MGL_DOWN_DIR);
				MGL_unionRect(charRect,clipRect,&d);
				if (useSlow)
					_MGL_drawCharVecSlow(x,i,vec,sz_nx,sz_ny,sz_dx,sz_dy);
				else if (!MGL_equalRect(clipRect,d)) {
					if (!MGL_disjointRect(charRect,clipRect))
						_MGL_drawCharVecSlow(x,i,vec,sz_nx,sz_ny,sz_dx,sz_dy);
					}
				else
					DC.r.drawCharVec(x,i,vec,sz_nx,sz_ny,sz_dx,sz_dy);
				_MGL_rotateVector(vec,MGL_UP_DIR);

				i += (width = charWidth(*str++));
				charRect.top += width;
				charRect.bottom += width;
				}
			break;
		}
}

/****************************************************************************
DESCRIPTION:
Returns the location to begin drawing an underscore for the font.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to be passed to MGL_drawStrXY
y	- y coordinate to be passed to MGL_drawStrXY
str	- String to measure

REMARKS:
This function takes an (x,y) location that would normally be used to draw a string
with MGL_drawStrXY, and adjusts the coordinates to begin at the under score
location for the current font, in the current drawing attributes. Thus the entire
character string can be underlined by drawing a line starting at the computed
underscore location and extending for MGL_textWidth pixels in length.

SEE ALSO:
MGL_drawStrXY, MGL_textWidth
****************************************************************************/
void MGLAPI MGL_underScoreLocation(
	int *x,
	int *y,
	const char *str)
{
	int				newx,newy,adjust;
	int				width = MGL_textWidth(str);
	text_settings_t	ts = DC.a.ts;
	font_t			*font = ts.font;

	if (font == NULL) return;

	newx =	(ts.horizJust == MGL_LEFT_TEXT ? 	0 :
			(ts.horizJust == MGL_CENTER_TEXT ? 	width/2 :
											width));
	switch (font->fontType) {
		case MGL_FIXEDFONT:
		case MGL_PROPFONT:
			newy = 	(ts.vertJust == MGL_TOP_TEXT ? 		font->ascent+1 :
					(ts.vertJust == MGL_CENTER_TEXT ?	(font->ascent+font->descent+1)/2 + 1 :
					(ts.vertJust == MGL_BOTTOM_TEXT ?	(font->descent + 1) :
													1)));
			break;
		default: /* case MGL_VECTORFONT: */
			newy = 	(ts.vertJust == MGL_TOP_TEXT ? 		font->ascent+1 :
					(ts.vertJust == MGL_CENTER_TEXT ?	(font->ascent+font->descent+1)/2 + 1 :
					(ts.vertJust == MGL_BOTTOM_TEXT ?	(font->descent + 1) :
													1)));
			newy = (ts.szNumery * newy) / ts.szDenomy;
			break;
		}

	adjust = font->fontHeight / 10;
	if (adjust < 1) adjust = 1;

	switch (ts.dir) {
		case MGL_RIGHT_DIR:
			*x -= newx;	*y += newy + adjust;
			break;
		case MGL_UP_DIR:
			*x += newy + adjust;	*y += newx;
			break;
		case MGL_LEFT_DIR:
			*x += newx;	*y -= newy + adjust;
			break;
		case MGL_DOWN_DIR:
			*x -= newy + adjust;	*y -= newx;
			break;
		}
}


/****************************************************************************
DESCRIPTION:
Draws a text string at the specified position.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to begin rasterizing the string at
y	- y coordinate to begin rasterizing the string at
str	- String to display

REMARKS:
Draws a string at the specified (x,y) position in the current drawing color, write
mode, font, text direction and justification.

SEE ALSO:
MGL_drawStr, MGL_textHeight, MGL_textWidth, MGL_useFont

****************************************************************************/
void MGLAPI MGL_drawStrXY(
	int x,
	int y,
	const char *str)
{
	if (DC.a.ts.font == NULL) return;

	switch(DC.a.ts.font->fontType) {
		case MGL_VECTORFONT:
			drawStrVec(x+DC.a.viewPort.left,y+DC.a.viewPort.top,str);
			break;
		case MGL_FIXEDFONT:
		case MGL_PROPFONT:
			DC.r.drawStrBitmap(x+DC.a.viewPort.left,y+DC.a.viewPort.top,str);
			break;
		}
}

/****************************************************************************
DESCRIPTION:
Draws a text string at the current position.

HEADER:
mgraph.h

PARAMETERS:
str	- String to display

REMARKS:
Draws a string at the current position (CP) in the current drawing color, write mode,
font, text direction and justification. The CP is moved so that drawing will begin
directly after the end of the string, only if the horizontal justification is set to
MGL_LEFT_TEXT, otherwise the CP is not moved.

SEE ALSO:
MGL_drawStrXY, MGL_textHeight, MGL_textWidth, MGL_useFont

****************************************************************************/
void MGLAPI MGL_drawStr(
	const char *str)
{
	int	width;

	if (DC.a.ts.font == NULL) return;

	MGL_drawStrXY(DC.a.CP.x,DC.a.CP.y,str);

	if (DC.a.ts.horizJust == MGL_LEFT_TEXT) {
		width = MGL_textWidth(str);
		switch (DC.a.ts.dir) {
			case MGL_RIGHT_DIR:
				DC.a.CP.x += width;
				break;
			case MGL_UP_DIR:
				DC.a.CP.y -= width;
				break;
			case MGL_LEFT_DIR:
				DC.a.CP.x -= width;
				break;
			case MGL_DOWN_DIR:
				DC.a.CP.y += width;
				break;
			}
		}
}

/****************************************************************************
DESCRIPTION:
Generates the commands to draw a vectored font.

HEADER:
mgraph.h

PARAMETERS:
x	- x coordinate to start drawing text at
y	- y coordinate to start drawing text at
str	- Character string to draw
move	- Routine to call to perform a move operation
draw	- Routine to call to perform a draw operation

RETURNS:
True if the string is correctly rasterized, false if font is not a vector font.

REMARKS:
This function calls a set of user supplied routines to rasterize the characters in a
vector font. This allows the vector fonts to be drawn in 2D or 3D floating point
coordinate systems by transforming each of the coordinates required to draw each
character by any arbitrary transformation, or in any coordinate system that the users
desires.

The move routine is called to move the cursor to a new location, and the draw
routine is used to perform a draw operation from the current location to the
specified location. Each character in the vector font is started with a move
operation.

Note that the coordinates passed to the move and draw routines will be offset from
the point (x,y), where the point (x,y) is the origin of the first character (i.e. it lies on
its baseline). Note also that the coordinates will be relative to the origin with the
origin at the lower left corner of each character (i.e. inverse of normal device
coordinate y-axis values).

This routine does not honor the standard scaling factors, but simply draws the
characters with a size of (1,1,1,1) (because scaling will be done by the user supplied
move and draw routines).

If the passed font is not a valid vector font, this routine returns false.

SEE ALSO:
MGL_drawStr, MGL_useFont
****************************************************************************/
ibool MGLAPI MGL_vecFontEngine(
	int x,
	int y,
	const char *str,
	void (ASMAPI *move)(int x,int y),
	void (ASMAPI *draw)(int x,int y))
{
	int				offset,dx,dy,missing;
	vector			*vec;
	text_settings_t	ts = DC.a.ts;
	font_t			*font = ts.font;

	/* Ensure that the font is a valid vector font */
	if (font->fontType != MGL_VECTORFONT)
		return false;

	/* Set scaling to (1,1,1,1) */
	DC.a.ts.szNumerx = DC.a.ts.szNumery = 1;
	DC.a.ts.szDenomx = DC.a.ts.szDenomy = 1;

	/* Adjust the first characters origin depending on the vertical
	 * justification setting.
	 */
	y -= (ts.vertJust == MGL_CENTER_TEXT ? (font->ascent + font->descent)/2 :
		 ((ts.vertJust == MGL_TOP_TEXT ? font->ascent :
		  (ts.vertJust == MGL_BOTTOM_TEXT ? font->descent : 0))));
	x -= (ts.horizJust == MGL_CENTER_TEXT ? MGL_textWidth(str)/2 :
		 (ts.horizJust == MGL_RIGHT_TEXT ? MGL_textWidth(str) : 0));

	while (*str) {
		missing = false;
		if ((offset = VECFONT(font)->offset[(uchar)*str]) == -1) {
			offset = VECFONT(font)->offset[MISSINGSYMBOL];
			missing = true;
			}
		vec = (vector *)&VECFONT(font)->def[offset];

		/* Perform the sequence of operations to draw the character */
		while (vec->op.op1) {
			dx = vec->op.dx;
			dy = vec->op.dy;
			if (vec->op.op2)
				move(x + dx,y + dy);
			else
				draw(x + dx,y + dy);
			vec++;
			}

		/* Move to the start of the next character */
		if (missing)
			x += VECFONT(font)->width[MISSINGSYMBOL];
		else
			x += VECFONT(font)->width[(int)*str];
		str++;
		}

	/* Restore original text settings */
	DC.a.ts = ts;
	return true;
}

/* {secret} */
void _ASMAPI _MGL_drawCharVecSlow(int x,int y,vector *vec,int sz_nx,
		int sz_ny,int sz_dx,int sz_dy)
/****************************************************************************
*
* Function:		_MGL_drawCharVecSlow
* Parameters:	x,y		- Coordinate to start drawing text at
*				vec		- Vector font glyph to draw
*				sz_nx	- X scale numerator
*				sz_ny	- Y scale numrtator
*				sz_dx	- X scale denominator
*				sz_dy	- Y scale denominator
*
* Description:	Routine to draw a single vector glyph slowly be calling
*				the MGL_lineCoord routine so that it will be correctly
*				clipped and drawn with normal attributes.
*
****************************************************************************/
{
	int	oldx,oldy,newx,newy;

	/* Convert back to viewport coordinates */
	x -= DC.a.viewPort.left;
	y -= DC.a.viewPort.top;

	/* Perform the sequence of operations to draw the character */
	newx = newy = 0;
	while (vec->op.op1) {
		oldx = newx;
		oldy = newy;
		newx = (sz_nx * vec->op.dx) / sz_dx;
		newy = (sz_ny * vec->op.dy) / sz_dy;
		if (!vec->op.op2)
			MGL_lineCoord(x+oldx,y-oldy,x+newx,y-newy);
		vec++;
		}
}

/* {secret} */
void _ASMAPI _MGL_rotateVector(vector *vec,int dir)
/****************************************************************************
*
* Function:		_MGL_rotateVector
* Parameters:	vec	- Pointer to vector glyph to rotate
*               dir	- Direction rotation in
*
* Description:	Rotates the representation of the vector in the
*				specified direction.
*
****************************************************************************/
{
	int	t;

	if (dir == MGL_UP_DIR) {
		/* Perform an upwards rotation of 90 degrees */
		while (vec->op.op1) {
			t = vec->op.dx;
			vec->op.dx = -vec->op.dy;	/* dx = -dy	*/
			vec->op.dy = t;				/* dy = dx	*/
			vec++;
			}
		}
	else if (dir == MGL_LEFT_DIR) {
		/* Perform a leftwards rotation of 180 degrees */
		while (vec->op.op1) {
			vec->op.dx = -vec->op.dx;	/* dx = -dx	*/
			vec->op.dy = -vec->op.dy;	/* dy = -dy	*/
			vec++;
			}
		}
	else {
		/* Perform a downwards rotation of 90 degrees */
		while (vec->op.op1) {
			t = vec->op.dx;
			vec->op.dx = vec->op.dy;	/* dx = dy	*/
			vec->op.dy = -t;			/* dy = -dx	*/
			vec++;
			}
		}
}

#endif	/* !MGL_LITE */

