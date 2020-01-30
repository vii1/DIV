/****************************************************************************
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
* Language:		ANSI C
* Environment:	any
*
* Description:	Test program for the 3D extensions library color models.
*
*
****************************************************************************/

#include "mgl3d.hpp"
#include <conio.h>
#include <stdlib.h>

#define	TEST

void main(void)
{
	int		driver,mode,err;

	driver = grDETECT;
	MGL_detectGraph(&driver,&mode);

	switch (driver) {
#ifndef	TEST
		case grSVGA_S3:
			mode = grSVGA_640x480x256;
			break;
#endif
		default:
			mode = grVGA_320x200x256;
		}

	MGL_init(&driver,&mode,"..\\mgraph");
	err = MGL_result();
	if (err != grOK) {
		cout << "Graphics error: " << (char*)MGL_errorMsg(err) << endl;
		exit(1);
		}
	MGL3D_init();

	int maxcolor = MGL_maxColor();
	int maxx = MGL_maxx();
	int maxy = MGL_maxy();

	if (maxcolor <= 15) {
		// Simple color demonstration for 16 color displays

		int width = 2 * ((MGL_maxx()+1) / 16);
		int height = 2 * ((MGL_maxy()-10)  / 10);

		int x = width / 2;
		int y = height / 2;
		color_t color = 1;
		for (int j = 0; j < 3; j++) {
			for (int i = 0; i < 5; i++) {
				MGL_setColor(color++);
				MGL_fillRectCoord(x,y,x+width,y+height);
				x += (width/2) * 3;
				}
			y += (height / 2) * 3;
			x = width / 2;
			}
		}
	else {
		// Color demonstration for large numbers of colors

		int width = 2 * ((MGL_maxx()+1) / 46);
		int height = 2 * ((MGL_maxy()-10)  / 47);

		int x = width / 2;
		int y = height / 2;
		color_t color = 0;
		for (int j = 0; j < 16; j++) {
			for (int i = 0; i < 16; i++) {
				MGL_setColor(color++);
				MGL_fillRectCoord(x,y,x+width,y+height);
				x += (width/2) * 3;
				}
			y += (height / 2) * 3;
			x = width / 2;
			}
		}

	getch();
	ThreeColorModel model(Color(REAL(0.7),REAL(0.369),REAL(0.1)),
						  Color(0,REAL(1),0),
						  Color(0,0,REAL(1)),
						  REAL(3));
	model.programPalette();

	getch();
	MGL_clearDevice();

	for (int x = 0; x < maxx; x++) {
		ModelColor modelColor;
		modelColor.rgb.r = (uchar)((x*255.0) / (float)maxx);
		modelColor.rgb.g = modelColor.rgb.b = 0;
		color_t color = model.convertColor(modelColor);
		MGL_setColor(color);
		MGL_lineCoord(x,0,x,maxy);
		}

	getch();
	MGL_clearDevice();

	for (x = 0; x < maxx; x++) {
		ModelColor modelColor;
		modelColor.rgb.g = (uchar)((x*255.0) / (float)maxx);
		modelColor.rgb.r = modelColor.rgb.b = 0;
		color_t color = model.convertColor(modelColor);
		MGL_setColor(color);
		MGL_lineCoord(x,0,x,maxy);
		}

	getch();
	MGL_clearDevice();

	for (x = 0; x < maxx; x++) {
		ModelColor modelColor;
		modelColor.rgb.b = (uchar)((x*255.0) / (float)maxx);
		modelColor.rgb.r = modelColor.rgb.g = 0;
		color_t color = model.convertColor(modelColor);
		MGL_setColor(color);
		MGL_lineCoord(x,0,x,maxy);
		}

	getch();
	MGL3D_exit();
	MGL_exit();
}
