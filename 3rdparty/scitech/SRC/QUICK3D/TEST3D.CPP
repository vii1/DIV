/****************************************************************************
*
*			  Quick3D - A 3D C++ rendering pipeline for the MGL
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
* Environment:	any
*
* Description:	Test program for the 3D extensions library.
*
*
****************************************************************************/

#include "quick3d\quick3d.hpp"
#include "ztimer.h"
#include "getopt.h"
#include <stdio.h>
#include <iomanip.h>
#include <ctype.h>
#include <conio.h>

// Vertices and faces to make up a box

FXPoint3d	p1(REAL(-0.5),REAL(-0.5),REAL(1)),
			p2(REAL(-0.5),REAL(0.5),REAL(1)),
			p3(REAL(0.5),REAL(0.5),REAL(1)),
			p4(REAL(0.5),REAL(-0.5),REAL(1));

FXPoint3d	p5(REAL(-0.5),REAL(-0.5),REAL(-1)),
			p6(REAL(-0.5),REAL(0.5),REAL(-1)),
			p7(REAL(0.5),REAL(0.5),REAL(-1)),
			p8(REAL(0.5),REAL(-0.5),REAL(-1));

FXPoint3d	face[6][4];
FXVec3d		faceNormal[6];
MGLLight	light1,light2;
MGLMaterial	boxMaterial;
char		modename[80];
int			driver = grDETECT;	// Graphics device driver type
int			mode = grDETECT;	// Video mode to use
int			doubleBufferMode = Quick3d::AutoDoubleBuffer;
int			hsrMethod = Quick3d::NoHSR;
int			rgbFlag = false;
int			shadeType = Quick3d::LitFlatShading;
int			twoLights = false;

void waitEvent()
{
	event_t	evt;
	EVT_halt(&evt,EVT_KEYDOWN | EVT_KEYREPEAT | EVT_MOUSEDOWN);
}

ibool checkEvent()
{
	event_t	evt;
	EVT_peekNext(&evt,EVT_KEYDOWN | EVT_KEYREPEAT | EVT_MOUSEDOWN);
	return (evt.what != EVT_NULLEVT);
}

void help(void)
{
	int		i;
	char	buf[255];
	uchar	*modeList;

	MGL_detectGraph(&driver,&mode);
	modeList = MGL_availableModes();

	cout << "Options are:\n";
	cout << "  -VGA      Use standard VGA driver\n";
	cout << "  -VBE      Use unaccelerated VESA VBE driver\n";
	cout << "  -p<value> Set the number of palette entries set at a time (default 256)\n";
	cout << "  -D[h,s]   Force either software (-Ds) or hardware (-Dh) double buffering\n";
	cout << "  -S[d,z]   Use Depth Sorting (-Sd) or Zbuffering (-Sz) (default is no HSR)\n";
	cout << "  -l        Enable two lights in scene\n";
	cout << "  -r        Use RGB color model (dithering in 8 bit modes)\n";
	cout << "  -s        Turn on smooth shading\n";
	cout << "  -M<num>   Set the video mode.\n\n";
	cout << "  -U<div>   Create SubDC 1/'div' the size of display DC\n";
	cout << "Press a key for list of video modes.";
	getch();
	cout << "\n\nAvailable modes are:\n";

	i = 0;
	while (modeList[i] != 0xFF) {
		sprintf(buf,"    %2d - %-26s %2d page (%s)\n",
			modeList[i],MGL_modeName(modeList[i]),MGL_availablePages(modeList[i]),
			MGL_modeDriverName(modeList[i]));
		cout << buf;
		i++;
		}

	cout << "\nVideo Card: " << MGL_driverName(driver) << endl;
	exit(1);
}

void fatalError(int result,int driver = -1,int mode = -1)
{
	MGL_exit();
	cerr << "\nGraphics error: " << MGL_errorMsg(result) << endl;
	if (driver != -1)
		cerr << "Driver: " << driver << ", Mode: " << mode << endl;
	exit(1);
}

void fatalError(const char *msg)
{
	MGL_exit();
	cerr << "FATAL: " << msg << endl;
	exit(1);
}

MGLDC *initMGL(int argc,char *argv[])
{
	int		i,forcemode = grDETECT;
	uchar	*modeList;
	int		option;
	int		snowlevel,useSubDC;
	char	*argument;
	MGLDC	*dc;

	snowlevel = -1;
	useSubDC = -1;

	// Register display drivers
	MGL_registerAllDispDrivers(true);
	MGL_registerAllMemDrivers();

	do {
		option = getopt(argc,argv,"lV:p:D:C:M:S:U:rs",&argument);
		switch (option) {
			case 'V':
				if (tolower(argument[0]) == 'g')
					driver = grVGA;
				else if (tolower(argument[0]) == 'b')
					driver = grVESA;
				break;
			case 'p':
				snowlevel = atoi(argument);
				break;
			case 'l':
				twoLights = true;
				break;
			case 'D':
				if (argument[0] == 's')
					doubleBufferMode = Quick3d::SoftwareDoubleBuffer;
				else if (argument[0] == 'h')
					doubleBufferMode = Quick3d::HardwareDoubleBuffer;
				break;
			case 'S':
				if (argument[0] == 'd')
					hsrMethod = Quick3d::DepthSortHSR;
				else if (argument[0] == 'z')
					hsrMethod = Quick3d::ZBufferHSR;
				break;
			case 'M':
				forcemode = atoi(argument);
				break;
			case 'U':
				useSubDC = atoi(argument);
				break;
			case 'r':
				rgbFlag = true;
				break;
			case 's':
				shadeType = Quick3d::LitSmoothShading;
				break;
			case PARAMETER:
			case INVALID:
				help();
			}
		} while (option != ALLDONE);

	MGL_detectGraph(&driver,&mode);
	if (forcemode != grDETECT) {
		// Look up the mode number given it's id
		modeList = MGL_availableModes();
		for (i = 0; modeList[i] != 0xFF; i++) {
			if (modeList[i] == forcemode)
				break;
			}
		if (modeList[i] == 0xFF) {
			printf("Invalid video mode selected\n");
			exit(1);
			}
		mode = forcemode;
		}

	if (!MGL_init(&driver,&mode,"\\scitech"))
		fatalError(MGL_result(),driver,mode);
	if ((dc = MGL_createDisplayDC(true)) == NULL)
		fatalError(MGL_result(),driver,mode);

	if (useSubDC > 1) {
		int sizex = (MGL_sizex(dc)+1) / sqrt(useSubDC);
		int sizey = (MGL_sizey(dc)+1) / sqrt(useSubDC);
		int cx = (MGL_sizex(dc)+1) / 2;
		int cy = (MGL_sizey(dc)+1) / 2;
		int left = cx - sizex / 2;
		int top = cy - sizey / 2;
		int right = cx + sizex / 2;
		int bottom = cy + sizey / 2;
		MGL_makeSubDC(dc,left,top,right,bottom);
		}

	if (snowlevel != -1)
		MGL_setPaletteSnowLevel(dc,snowlevel);
	strcpy(modename,MGL_modeName(mode));

	return dc;
}

void exitMGL(void)
{
	MGL_exit();
	cout << "Name of video mode used: \"" << modename << "\"\n";
}

void draw_box(Quick3d& dc)
{
	dc.color(dc.realColor(MGL_WHITE));

	dc.beginLine();
		dc.moveTo(p1);
		dc.lineTo(p2);
		dc.lineTo(p3);
		dc.lineTo(p4);
		dc.lineTo(p1);

		dc.lineTo(p5);
		dc.lineTo(p6);
		dc.lineTo(p7);
		dc.lineTo(p8);
		dc.lineTo(p5);

		dc.moveTo(p2);
		dc.lineTo(p6);

		dc.moveTo(p3);
		dc.lineTo(p7);

		dc.moveTo(p4);
		dc.lineTo(p8);
	dc.endLine();
}

void draw_solidbox(Quick3d& dc)
{
	dc.setShadingType(shadeType);
	dc.material(&boxMaterial);
	dc.setPolygonType(MGL_CONVEX_POLYGON);

	for (int i = 0; i < 6; i++) {
		dc.normal(faceNormal[i]);
		dc.fillPolygon(4,face[i]);
		}
}

void draw_solidbox_tmesh(Quick3d& dc)
{
	dc.setShadingType(shadeType);
	dc.material(&boxMaterial);
	dc.setPolygonType(MGL_CONVEX_POLYGON);

	// Draw the entire box as a two triangular strips, with 6 faces each
	dc.beginTStrip();
		dc.normal(faceNormal[0]);
		dc.vertex(p1);
		dc.vertex(p2);
		dc.vertex(p4);
		dc.vertex(p3);

		dc.normal(faceNormal[3]);
		dc.vertex(p8);
		dc.vertex(p7);

		dc.normal(faceNormal[2]);
		dc.vertex(p5);
		dc.vertex(p6);
	dc.endTStrip();

	dc.beginTStrip();
		dc.normal(faceNormal[4]);
		dc.vertex(p7);
		dc.vertex(p3);
		dc.vertex(p6);
		dc.vertex(p2);

		dc.normal(faceNormal[1]);
		dc.vertex(p5);
		dc.vertex(p1);

		dc.normal(faceNormal[5]);
		dc.vertex(p8);
		dc.vertex(p4);
	dc.endTStrip();
}

void draw_solidbox_qstrip(Quick3d& dc)
{
	dc.setShadingType(shadeType);
	dc.material(&boxMaterial);
	dc.setPolygonType(MGL_CONVEX_POLYGON);

	// Draw the box as two quad meshes with 3 faces each.
	dc.beginQStrip();
		dc.normal(faceNormal[0]);
		dc.vertex(p1);
		dc.vertex(p2);
		dc.vertex(p4);
		dc.vertex(p3);

		dc.normal(faceNormal[3]);
		dc.vertex(p8);
		dc.vertex(p7);

		dc.normal(faceNormal[2]);
		dc.vertex(p5);
		dc.vertex(p6);
	dc.endQStrip();

	dc.beginQStrip();
		dc.normal(faceNormal[4]);
		dc.vertex(p7);
		dc.vertex(p3);
		dc.vertex(p6);
		dc.vertex(p2);

		dc.normal(faceNormal[1]);
		dc.vertex(p5);
		dc.vertex(p1);

		dc.normal(faceNormal[5]);
		dc.vertex(p8);
		dc.vertex(p4);
	dc.endQStrip();
}

void line_test1(Quick3d& dc)
{
	FXPoint3d	p1(REAL(-0.5),REAL(-0.5),0),
				p2(REAL(-0.5),REAL(0.5),0),
				p3(REAL(0.5),REAL(0.5),0),
				p4(REAL(0.5),REAL(-0.5),0);

	dc.clearDevice();
	dc.beginLine();
		dc.color(dc.realColor(1));
		dc.moveTo(p1);
		dc.lineTo(p2);
		dc.color(dc.realColor(2));
		dc.lineTo(p3);
		dc.color(dc.realColor(3));
		dc.lineTo(p4);
		dc.color(dc.realColor(4));
		dc.lineTo(p1);
	dc.endLine();

	waitEvent();

	dc.color(dc.realColor(MGL_WHITE));

	dc.line(p1,p2);
	dc.line(p2,p3);
	dc.line(p3,p4);
	dc.line(p4,p1);
	waitEvent();
}

void line_test2(Quick3d& dc)
{
	FXPoint3d	p1(REAL(-0.5),REAL(-0.5),0),
				p2(REAL(-0.5),REAL(0.5),0),
				p3(REAL(0.5),REAL(0.5),0),
				p4(REAL(0.5),REAL(-0.5),0);

	dc.clearDevice();
	dc.beginLine();
		dc.color(dc.realColor(1));
		dc.moveTo(p3);
		dc.moveRel(p1);
		dc.lineRel(p2);
		dc.color(dc.realColor(2));
		dc.lineRel(p3);
		dc.color(dc.realColor(3));
		dc.lineRel(p4);
		dc.color(dc.realColor(4));
		dc.lineRel(p1);
	dc.endLine();

	waitEvent();
}

void circle_test(Quick3d& dc)
{
	FXPoint3d	o(0,0,REAL(1)),start,end;

	dc.clearDevice();
	draw_box(dc);

	dc.color(dc.realColor(MGL_WHITE));
	dc.arc(o,REAL(0.9),REAL(60),REAL(30));
	dc.getArcCoords(start,end);
	dc.line(o,start);
	dc.line(o,end);
	waitEvent();

	dc.color(dc.realColor(MGL_YELLOW));
	dc.circle(o,REAL(0.9));
	waitEvent();

	int oldBackface = dc.setBackfaceMethod(Quick3d::BackfaceShade);
	dc.color(dc.realColor(MGL_RED));
	dc.fillCircle(o,REAL(0.9));
	waitEvent();
	dc.color(dc.realColor(MGL_WHITE));
	dc.circleOutline();
	waitEvent();

	dc.color(dc.realColor(MGL_BLUE));
	dc.fillArc(o,REAL(0.9),600,300);
	dc.color(dc.realColor(MGL_YELLOW));
	dc.arcOutline();
	waitEvent();
	dc.setBackfaceMethod(oldBackface);
}

void text_test(Quick3d& dc)
{
	FXPoint3d	o(0,0,REAL(10)),start,end;
	font_t 		*font = MGL_loadFont("little.fnt");

	if (!font)
		fatalError(MGL_result());
	dc.clearDevice();

	draw_box(dc);
	dc.push();
	dc.scale(REAL(0.05),REAL(0.05),REAL(0.05));

	dc.useFont(font);
	dc.setTextJustify(MGL_CENTER_TEXT,MGL_CENTER_TEXT);
	dc.drawStr(o,"This is 3D text");

	waitEvent();
	dc.pop();
	MGL_unloadFont(font);
}

void box_test(Quick3d& dc,void (*drawbox)(Quick3d& dc))
{
	dc.clearDevice();
	dc.beginScene();
	drawbox(dc);
	dc.endScene();
	waitEvent();
}

void animate_box(Quick3d& dc,void (*drawbox)(Quick3d& dc))
{
	FXForm3d	rotate;

	rotate.identity();
	rotate.rotatex(REAL(-1));
	rotate.rotatey(REAL(2));
	rotate.rotatez(REAL(1.5));

	// Clear both display pages
	dc.setActivePage(0);
	dc.clearDevice();
	dc.setActivePage(1);
	dc.clearDevice();

	if (!dc.doubleBuffer(doubleBufferMode))
		fatalError("Cannot start double buffered graphics!");

	dc.push();
	while (!checkEvent()) {
		dc.concat(rotate);
		dc.clearDevice();
		dc.beginScene();
		drawbox(dc);
		dc.endScene();
		dc.swapBuffers();
		}
	dc.pop();
	dc.singleBuffer();

	waitEvent();
}

void (*draw_convex_box)(Quick3d& dc);

void draw_convex(Quick3d& dc)
{
	dc.beginConvexObject();
	draw_convex_box(dc);
	dc.endObject();
}

void animate_convex_box(Quick3d& dc,void (*drawbox)(Quick3d& dc))
{
	draw_convex_box = drawbox;
	animate_box(dc,draw_convex);
}

void animate_box_outline(Quick3d& dc,void (*drawbox)(Quick3d& dc))
{
	dc.setOutlineMode(Quick3d::DrawOutline);
	animate_box(dc,drawbox);
	dc.setOutlineMode(Quick3d::NoOutline);
}

void animate_convex_box_outline(Quick3d& dc,void (*drawbox)(Quick3d& dc))
{
	draw_convex_box = drawbox;
	animate_box_outline(dc,draw_convex);
}

void show_colors(Quick3d& dc)
{
	int		color,width,height,x,y,i,j,top,bottom,start,palsize;

	if (dc.maxColor() <= 15) {
		// Simple color demonstration for 16 color displays
		width = 2 * ((dc.maxx()+1) / 16);
		height = 2 * ((dc.maxy()-10)  / 10);

		x = width / 2;
		y = height / 2;
		color = 1;
		for (j = 0; j < 3; j++) {
			for (i = 0; i < 5; i++) {
				dc.setColor(color++);
				dc.fillRect(x,y,x+width,y+height);
				x += (width/2) * 3;
				}
			y += (height / 2) * 3;
			x = width / 2;
			}
		}
	else if (dc.maxColor() == 255) {
		// FXColor demonstration for color mapped displays with 256 colors
		width = 2 * ((dc.maxx()+1) / 47);
		height = 2 * ((dc.maxy()-10)  / 48);
		palsize = dc.getPaletteSize();

		x = width / 2;
		y = height / 2;
		color = 0;
		for (j = 0; j < palsize; j++) {
			dc.setColor(color++);
			dc.fillRect(x,y,x+width,y+height);
			x += (width/2) * 3;
			if (((j+1) % 16) == 0) {
				y += (height / 2) * 3;
				x = width / 2;
				}
			}
		}
	else {
		// FXColor demonstration for HiColor and TrueColor modes
		start = dc.maxx()/8;
		width = dc.maxx() - start*2;
		dc.beginDrawing();

		top = dc.maxy()/8;
		bottom = dc.maxy()/8 + dc.maxy()/5;
		for (x = 0; x < width; x++) {
			dc.setColor(dc.packColor((x * 255L) / width,0,0));
			dc.MGLDevCtx::lineFast(start+x,top,start+x,bottom);
			}

		top += dc.maxy()/5;
		bottom += dc.maxy()/5;
		for (x = 0; x < width; x++) {
			dc.setColor(dc.packColor(0,(x * 255L) / width,0));
			dc.MGLDevCtx::lineFast(start+x,top,start+x,bottom);
			}

		top += dc.maxy()/5;
		bottom += dc.maxy()/5;
		for (x = 0; x < width; x++) {
			dc.setColor(dc.packColor(0,0,(x * 255L) / width));
			dc.MGLDevCtx::lineFast(start+x,top,start+x,bottom);
			}

		top += dc.maxy()/5;
		bottom += dc.maxy()/5;
		for (x = 0; x < width/2; x++) {
			dc.setColor(dc.packColor((((width/2)-x) * 255L) / (width/2),
				(x * 255L) / (width/2),0));
			dc.MGLDevCtx::lineFast(start+x,top,start+x,bottom);
			}
		for (; x < width; x++) {
			dc.setColor(dc.packColor(0,
				( ((width/2) - (x - (width/2))) * 255L) / (width/2),
				((x-width/2) * 255L) / (width/2) ));
			dc.MGLDevCtx::lineFast(start+x,top,start+x,bottom);
			}
		dc.endDrawing();
		}
	waitEvent();
}

void doTests(MGLDevCtx& mgldc)
{
	int	colorModel;

	// Setup the appropriate color model
	boxMaterial.diffuse.r = REAL(0.766);
	boxMaterial.diffuse.g = REAL(0.575);
	boxMaterial.diffuse.b = REAL(0.217);
	boxMaterial.ambient = boxMaterial.diffuse * FXColor(REAL(0.2),REAL(0.2),REAL(0.2));
//	boxMaterial.specular.r = REAL(0.766);
//	boxMaterial.specular.g = REAL(0.575);
//	boxMaterial.specular.b = REAL(0.217);
	boxMaterial.specular.r = REAL(0.8);
	boxMaterial.specular.g = REAL(0.8);
	boxMaterial.specular.b = REAL(0.8);
	boxMaterial.shininess = REAL(50);

	if (rgbFlag) {
		if (mgldc.getBitsPerPixel() == 8) {
			// Setup for dithered operation with an MGL halfTone palette
			palette_t pal[256];
			MGL_setColorMapMode(mgldc,MGL_DITHER_RGB_MODE);
			MGL_getHalfTonePalette(pal);
			MGL_setPalette(mgldc,pal,256,0);
			MGL_realizePalette(mgldc,256,0,false);
			}
		colorModel = Quick3d::RGBModel;
		}
	else {
		// Setup palette for brown box colors and add the material
		// color entries and shade table for this to work properly.
		boxMaterial.ambientIndex = REAL(16);
		boxMaterial.diffuseIndex = REAL(47);
		boxMaterial.specularIndex = REAL(79);
		if (mgldc.getBitsPerPixel() == 8) {
			palette_t pal[256];
			MGL_getDefaultPalette(mgldc,pal);
			boxMaterial.buildRamp(pal);
			MGL_setPalette(mgldc,pal,256,0);
			MGL_realizePalette(mgldc,256,0,false);
			}
		else boxMaterial.buildShadeTable(mgldc);
		colorModel = Quick3d::ColorIndexModel;
		}

	Quick3d dc(mgldc.getDC(),colorModel);

	FXVec3d	VRP(0,0,0),
			VPN(REAL(0.75),REAL(0.6),REAL(1)),
			VUP(0,REAL(1),0);

//	dc.viewMapping(REAL(-1.5),REAL(-1.5),REAL(1.5),REAL(1.5),FXForm3dPerspective,
//		FXPoint3d(0,0,REAL(10)),REAL(5),REAL(-5),
//		FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),dc.getAspectRatio());
	dc.viewMapping(REAL(-2.5),REAL(-2.5),REAL(2.5),REAL(2.5),FXForm3dPerspective,
		FXPoint3d(0,0,REAL(10)),REAL(5),REAL(-5),
		FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),dc.getAspectRatio());
//	dc.viewMapping(REAL(-1),REAL(-1),REAL(1),REAL(1),FXForm3d::parallel,
//		FXPoint3d(0,0,REAL(10)),REAL(5),REAL(-5),
//		FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),dc.getAspectRatio());
	dc.viewOrientation(VRP,VPN,VUP);

//	dc.rotatez(REAL(30));
//	dc.scale(REAL(1),REAL(0.5),REAL(1));

	// Setup the lights in the scene
	light1.dir.x = REAL(-0.5);
	light1.dir.y = REAL(-0.4);
	light1.dir.z = REAL(-0.3);

	light2.dir.x = REAL(-0.1);
	light2.dir.y = REAL(-0.3);
	light2.dir.z = REAL(-1);
	light2.diffuse.r = REAL(0.6);
	light2.diffuse.b = REAL(0.4);
	light2.diffuse.g = REAL(0);
	light2.specular.r = REAL(0.7);
	light2.specular.b = REAL(0.3);
	light2.specular.g = REAL(0);

	dc.addLight(&light1);
	if (twoLights)
		dc.addLight(&light2);

	// Initialise the face array
	face[0][0] = p1; face[0][1] = p2; face[0][2] = p3; face[0][3] = p4;
	face[1][0] = p5; face[1][1] = p6; face[1][2] = p2; face[1][3] = p1;
	face[2][0] = p8; face[2][1] = p7; face[2][2] = p6; face[2][3] = p5;
	face[3][0] = p4; face[3][1] = p3; face[3][2] = p7; face[3][3] = p8;
	face[4][0] = p2; face[4][1] = p6; face[4][2] = p7; face[4][3] = p3;
	face[5][0] = p8; face[5][1] = p5; face[5][2] = p1; face[5][3] = p4;

	faceNormal[0] = ~((face[0][1] - face[0][0]) ^ (face[0][1] - face[0][2]));
	faceNormal[1] = ~((face[1][1] - face[1][0]) ^ (face[1][1] - face[1][2]));
	faceNormal[2] = ~((face[2][1] - face[2][0]) ^ (face[2][1] - face[2][2]));
	faceNormal[3] = ~((face[3][1] - face[3][0]) ^ (face[3][1] - face[3][2]));
	faceNormal[4] = ~((face[4][1] - face[4][0]) ^ (face[4][1] - face[4][2]));
	faceNormal[5] = ~((face[5][1] - face[5][0]) ^ (face[5][1] - face[5][2]));

	show_colors(dc);
	line_test1(dc);
	line_test2(dc);
	circle_test(dc);
	text_test(dc);
	box_test(dc,draw_box);
	if (hsrMethod == Quick3d::NoHSR)
		dc.setBackfaceMethod(Quick3d::BackfaceCull);
	else dc.setBackfaceMethod(Quick3d::BackfaceShade);
	if (dc.setHSRMethod(hsrMethod) == -1)
		fatalError("Could not start Hidden Surface Removal!");
	box_test(dc,draw_solidbox);
	if (!(shadeType & Quick3d::SmoothShading)) {
		// Smooth shading doesn't work with out meshed boxes, because the
		// normals for the boxes will be shared across a 90 degree surface
		// causing incorrect results! Smooth shading of meshes assumes that
		// the surface normals are shared and have been properly specified
		// for each face in the mesh.
		box_test(dc,draw_solidbox_tmesh);
		box_test(dc,draw_solidbox_qstrip);
		}
	animate_box(dc,draw_box);
	animate_box(dc,draw_solidbox);
	animate_box_outline(dc,draw_solidbox);
	if (!(shadeType & Quick3d::SmoothShading)) {
		animate_box(dc,draw_solidbox_tmesh);
		animate_box_outline(dc,draw_solidbox_tmesh);
		animate_box(dc,draw_solidbox_qstrip);
		animate_box_outline(dc,draw_solidbox_qstrip);
		}
	animate_convex_box(dc,draw_box);
	animate_convex_box(dc,draw_solidbox);
	animate_convex_box_outline(dc,draw_solidbox);
	if (!(shadeType & Quick3d::SmoothShading)) {
		animate_convex_box(dc,draw_solidbox_tmesh);
		animate_convex_box_outline(dc,draw_solidbox_tmesh);
		animate_convex_box(dc,draw_solidbox_qstrip);
		animate_convex_box_outline(dc,draw_solidbox_qstrip);
		}
}

void main(int argc,char *argv[])
{
	MGLDevCtx dc(initMGL(argc,argv));
	doTests(dc);
	exitMGL();
}
