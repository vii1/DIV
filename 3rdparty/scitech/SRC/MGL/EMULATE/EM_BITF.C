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
* Description:	C based emulation routines for bitmap font rendering.
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

void _ASMAPI __EMU__drawStrBitmap(int x,int y,const char *str)
/****************************************************************************
*
* Function:		__EMU__drawStrBitmap
* Parameters:	x	- X coordinate to draw at
*				y	- Y coordinate to draw at
*				str	- String to draw
*
* Description:	Draws the specified string at the current position (CP) in
*				the current drawing color, write mode and justification
*				using the current bitmap font. We rely upon the underlying
*				MGL_putMonoImage routine to handling all of the clipping
*				and to trivially reject bitmaps for us.
*
****************************************************************************/
{
	int				i,width,byteWidth,height,ascent,descent,offset;
	int				fontAscent,iwidth,bitmapHeight;
	uchar			*bytes,*buf = _MGL_buf;
	text_settings_t	ts = DC.a.ts;
	font_t			*font = ts.font;

	/* Find the ascent, descent, maxKern and maxWidth values */
	ascent = font->ascent;			descent = font->descent;
	fontAscent = font->fontHeight + descent - 1;
	bitmapHeight = ((font->fontHeight + 7) & ~0x7) - 1;

	/* Adjust the first characters origin depending on the vertical
	 * justification setting.
	 */
	switch (ts.vertJust) {
		case MGL_TOP_TEXT:
			height = fontAscent - ascent;
			y -= (ts.dir == MGL_RIGHT_DIR ? height :
				 (ts.dir == MGL_LEFT_DIR ? font->fontHeight-1 - height : 0));
			x -= (ts.dir == MGL_UP_DIR ? height :
				 (ts.dir == MGL_DOWN_DIR ? bitmapHeight - height : 0));
			break;
		case MGL_CENTER_TEXT:
			height = fontAscent - ascent + (ascent-descent)/2;
			y -= (ts.dir == MGL_RIGHT_DIR ? height :
				 (ts.dir == MGL_LEFT_DIR ? font->fontHeight-1 - height : 0));
			x -= (ts.dir == MGL_UP_DIR ? height :
				 (ts.dir == MGL_DOWN_DIR ? bitmapHeight - height : 0));
			break;
		case MGL_BASELINE_TEXT:
			y -= (ts.dir == MGL_RIGHT_DIR ? fontAscent :
				 (ts.dir == MGL_LEFT_DIR ? font->fontHeight-1 - fontAscent : 0));
			x -= (ts.dir == MGL_UP_DIR ? fontAscent :
				 (ts.dir == MGL_DOWN_DIR ? bitmapHeight - fontAscent : 0));
			break;
		case MGL_BOTTOM_TEXT:
			height = font->fontHeight-1;
			y -= (ts.dir == MGL_RIGHT_DIR ? height :
				 (ts.dir == MGL_LEFT_DIR ? font->fontHeight-1 - height : 0));
			x -= (ts.dir == MGL_UP_DIR ? height :
				 (ts.dir == MGL_DOWN_DIR ? bitmapHeight - height : 0));
			break;
		}

	/* Now draw the string using a different loop for each of the four
	 * text drawing directions. Note that we maintain a character clipping
	 * rectangle through each loop, and check to see if the character can
	 * be entirely rejected.
	 */
	width = MGL_textWidth(str);
	height = font->fontHeight;

	switch (ts.dir) {
		case MGL_RIGHT_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? x :
				(ts.horizJust == MGL_CENTER_TEXT ?
					x - width/2 :
					x - width));

			while (*str) {
				width = BITFONT(font)->width[(uchar)*str];
				iwidth = BITFONT(font)->iwidth[(uchar)*str];
				if ((offset = BITFONT(font)->offset[(uchar)*str++]) == -1) {
					offset = BITFONT(font)->offset[MISSINGSYMBOL];
					width = BITFONT(font)->width[MISSINGSYMBOL];
					iwidth = BITFONT(font)->iwidth[MISSINGSYMBOL];
					}
				bytes = (uchar *)&BITFONT(font)->def[offset];
				byteWidth = (iwidth + 7) >> 3;

				if (iwidth != 0)
					DC.r.putMonoImage(&DC,i,y,byteWidth,height,bytes);

				i += width;
				}
			break;
		case MGL_UP_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? y :
				(ts.horizJust == MGL_CENTER_TEXT ?
					y + width/2 :
					y + width));

			while (*str) {
				width = BITFONT(font)->width[(uchar)*str];
				iwidth = BITFONT(font)->iwidth[(uchar)*str];
				if ((offset = BITFONT(font)->offset[(uchar)*str++]) == -1) {
					offset = BITFONT(font)->offset[MISSINGSYMBOL];
					width = BITFONT(font)->width[MISSINGSYMBOL];
					iwidth = BITFONT(font)->iwidth[MISSINGSYMBOL];
					}
				bytes = (uchar *)&BITFONT(font)->def[offset];
				byteWidth = (iwidth + 7) >> 3;

				if (iwidth != 0) {
					height = font->fontHeight;
					MGL_rotateGlyph(buf,bytes,&byteWidth,&height,MGL_UP_DIR);
					DC.r.putMonoImage(&DC,x,i-height+1,byteWidth,height,buf);
					}

				i -= width;
				}
			break;
		case MGL_DOWN_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? y :
				(ts.horizJust == MGL_CENTER_TEXT ?
					y - width/2 :
					y - width));

			while (*str) {
				width = BITFONT(font)->width[(uchar)*str];
				iwidth = BITFONT(font)->iwidth[(uchar)*str];
				if ((offset = BITFONT(font)->offset[(uchar)*str++]) == -1) {
					offset = BITFONT(font)->offset[MISSINGSYMBOL];
					width = BITFONT(font)->width[MISSINGSYMBOL];
					iwidth = BITFONT(font)->iwidth[MISSINGSYMBOL];
					}
				bytes = (uchar *)&BITFONT(font)->def[offset];
				byteWidth = (iwidth + 7) >> 3;

				if (iwidth != 0) {
					height = font->fontHeight;
					MGL_rotateGlyph(buf,bytes,&byteWidth,&height,MGL_DOWN_DIR);
					DC.r.putMonoImage(&DC,x,i,byteWidth,height,buf);
					}

				i += width;
				}
			break;
		case MGL_LEFT_DIR:
			i = (ts.horizJust == MGL_LEFT_TEXT ? x :
				(ts.horizJust == MGL_CENTER_TEXT ?
					x + width/2 :
					x + width));

			while (*str) {
				width = BITFONT(font)->width[(uchar)*str];
				iwidth = BITFONT(font)->iwidth[(uchar)*str];
				if ((offset = BITFONT(font)->offset[(uchar)*str++]) == -1) {
					offset = BITFONT(font)->offset[MISSINGSYMBOL];
					width = BITFONT(font)->width[MISSINGSYMBOL];
					iwidth = BITFONT(font)->iwidth[MISSINGSYMBOL];
					}
				bytes = (uchar *)&BITFONT(font)->def[offset];
				byteWidth = (iwidth + 7) >> 3;

				if (iwidth != 0) {
					_MGL_rotateBitmap180(buf,bytes,byteWidth,height);
					DC.r.putMonoImage(&DC,i-(byteWidth<<3)+1,y,byteWidth,height,buf);
					}

				i -= width;
				}
			break;
		}
}

#endif	/* !MGL_LITE */
