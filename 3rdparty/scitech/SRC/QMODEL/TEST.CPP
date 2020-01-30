/****************************************************************************
*
*			  	QuickModeler - A Real-Time 3D Modeling System
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
* Description:	Test program for the 3D Modeler
*
*
****************************************************************************/

#include "qmodel.hpp"

#pragma hdrstop

#include "qmodel/cube.hpp"
#include "qmodel/polygon.hpp"
#include "qmodel/bndbox.hpp"
#include "qmodel/xform.hpp"
#include "qmodel/material.hpp"
#include "qmodel/offobjec.hpp"
#include "qmodel/group.hpp"
#include "qmodel/matreg.hpp"

#include "ztimer.h"
#include "getopt.h"
#include <stdio.h>
#include <iomanip.h>
#include <ctype.h>
#include <conio.h>

MGLLight	light1,light2;
MGLMaterial	*mat1;
MGLMaterial	*mat2;
MGLMaterial	*mat3;
MGLMaterial	*mat4;
char		modename[80];
int			driver = grDETECT;	// Graphics device driver type
int			mode = grDETECT;	// Video mode to use
int			doubleBufferMode = Quick3d::AutoDoubleBuffer;
int			hsrMethod = Quick3d::NoHSR;
int			useDither = false;
int			twoLights = false;
int			wireframe = false;
int			flatflag = true;
int         anticlockflag = false;

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
	cout << "  -d        Use dithered rendering in 8 bit modes\n";
	cout << "  -s        Smooth shade the object\n";
	cout << "  -a        Faces are anti-clockwise\n";
	cout << "  -w        Render in wireframe mode\n";
	cout << "  -M<num>   Set the video mode.\n\n";
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
	int		snowlevel;
	char	*argument;
	MGLDC	*dc;

	snowlevel = -1;

	// Register display drivers
	MGL_registerAllDispDrivers(true);
	MGL_registerAllMemDrivers();

	do {
		option = getopt(argc,argv,"lV:p:D:C:M:S:dsaw",&argument);
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
			case 'd':
				useDither = true;
				break;
			case 's':
				flatflag = false;
				break;
			case 'a':
				anticlockflag = true;
				break;
			case 'w':
				wireframe = true;
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

void animateScene(Quick3d& dc,QMObject *scene)
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
	dc.setColor(dc.realColor(MGL_WHITE));
	while (!checkEvent()) {
		dc.concat(rotate);
		dc.clearDevice();
		dc.beginScene();
		if (wireframe)
			scene->renderWireframe(dc);
		else scene->renderShaded(dc);
		dc.endScene();
		dc.swapBuffers();
		}
	dc.pop();
	dc.singleBuffer();

	waitEvent();
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

QMObject *buildScene(void)
{
	FXPoint3d	v[3];
	FXVec3d		n[3];

	v[0] = FXPoint3d(0,0,0);
	v[1] = FXPoint3d(0,1,-0.5);
	v[2] = FXPoint3d(2,1,-0.3);

	QMPolyhedron *o = new QMOFFObject("test.geo",false,
		Quick3d::BackfaceCull,
		anticlockflag ? Quick3d::AntiClockWise : Quick3d::ClockWise);
	if (!flatflag)
		o->makeSmooth(REAL(90),0);
	QMTransform *t = new QMTransform(new QMMaterial(o,mat1,mat2));
	t->scale(3,3,3);;
	return t;
//	t->rotatex(30);
//*/
/*
	QMGroup *g = new QMGroup();
//	g->add(new QMMaterial(c->newInstance(),mat2,mat2));
	QMcomputeNormal(n[0],3,v,Quick3d::ClockWise);
	n[1] = ~(n[0] + FXVec3d(0,0.5,0));
	n[2] = ~(n[0] + FXVec3d(1,0,0));
	QMTransform *t = new QMTransform(new QMSmoothPolygon(3,v,n));
	t->translate(-0.5,-0.5,0);
	g->add(new QMMaterial(t,mat3,mat4));
//	g->add(t);
	return g;
*/
}

void doTests(MGLDevCtx& mgldc)
{
	int	colorModel;
	QMMaterialRegistry	materialReg(16,255);

	// Setup the materials in the scene
	mat1 = new MGLMaterial();
	mat1->diffuse = FXColor(REAL(0.766),REAL(0.575),REAL(0.217));
	mat1->ambient = mat1->diffuse * FXColor(REAL(0.2),REAL(0.2),REAL(0.2));
	mat1->specular = FXColor(REAL(0.3),REAL(0.3),REAL(0.3));
	mat1->shininess = REAL(50);
	materialReg.add(mat1);

	mat2 = new MGLMaterial();
	mat2->diffuse = FXColor(REAL(0.0),REAL(0.8),REAL(0.6));
	mat2->ambient = mat2->diffuse * FXColor(REAL(0.2),REAL(0.2),REAL(0.2));
	mat2->specular = FXColor(REAL(0.3),REAL(0.3),REAL(0.3));
	materialReg.add(mat2);

	mat3 = new MGLMaterial();
	materialReg.add(mat3);

	mat4 = new MGLMaterial();
	mat4->diffuse = FXColor(REAL(0.8),REAL(0),REAL(0.4));
	mat4->ambient = mat4->diffuse * FXColor(REAL(0.2),REAL(0.2),REAL(0.2));
	mat4->specular = FXColor(REAL(0.3),REAL(0.3),REAL(0.3));
	materialReg.add(mat4);

	if (mgldc.getBitsPerPixel() == 8) {
		if (useDither) {
			// Setup for dithered operation with an MGL halfTone palette
			palette_t pal[256];
			MGL_setColorMapMode(mgldc,MGL_DITHER_RGB_MODE);
			MGL_getHalfTonePalette(pal);
			MGL_setPalette(mgldc,pal,256,0);
			MGL_realizePalette(mgldc,256,0,false);
			colorModel = Quick3d::RGBModel;
			}
		else {
			// Setup palette for brown box colors and add the material
			// color entries for this to work properly.
			palette_t pal[256];
			MGL_getDefaultPalette(mgldc,pal);
			materialReg.buildPalette(pal);
			MGL_setPalette(mgldc,pal,256,0);
			MGL_realizePalette(mgldc,256,0,false);
			colorModel = Quick3d::ColorIndexModel;
			}
		}
	else colorModel = Quick3d::RGBModel;

	Quick3d dc(mgldc.getDC(),colorModel);

	FXVec3d	VRP(0,0,0),
			VPN(REAL(0.75),REAL(0.6),REAL(1)),
			VUP(0,REAL(1),0);

	dc.viewMapping(REAL(-2.5),REAL(-2.5),REAL(2.5),REAL(2.5),FXForm3dPerspective,
		FXPoint3d(0,0,REAL(10)),REAL(5),REAL(-5),
		FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),dc.getAspectRatio());
	dc.viewOrientation(VRP,VPN,VUP);

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

	show_colors(dc);
	dc.setBackfaceMethod(Quick3d::BackfaceShade);
	dc.setDualShading(true);
	if (dc.setHSRMethod(hsrMethod) == -1)
		fatalError("Could not start Hidden Surface Removal!");
	QMObject *scene = buildScene();
	animateScene(dc,scene);
	delete scene;
}

void main(int argc,char *argv[])
{
	MGLDevCtx dc(initMGL(argc,argv));
	doTests(dc);
	exitMGL();
}
