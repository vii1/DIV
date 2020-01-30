/****************************************************************************
*
*			 Quick 2D - A 2D C++ tranformation class for the MGL
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
* Description:	Test program for the Quick2D library.
*
*
****************************************************************************/

#include "quick2d/quick2d.hpp"
#include <conio.h>
#include <process.h>

void waitEvent(void)
{
	event_t	evt;
	EVT_halt(&evt,EVT_KEYDOWN | EVT_KEYREPEAT | EVT_MOUSEDOWN);
}

void fatalError(int result,int driver = -1,int mode = -1)
{
	MGL_exit();
	cerr << "\nGraphics error: " << MGL_errorMsg(result) << endl;
	if (driver != -1)
		cerr << "Driver: " << driver << ", Mode: " << mode << endl;
	exit(1);
}

MGLDC *initMGL(void)
{
	int		driver,mode;
	MGLDC	*dc;

	// Register all display/mem drivers
	MGL_registerAllDispDrivers(true);
	MGL_registerAllMemDrivers();

	driver = mode = grDETECT;
	if (!MGL_init(&driver,&mode,"\\scitech"))
		fatalError(MGL_result(),driver,mode);
	if ((dc = MGL_createDisplayDC(false)) == NULL)
		fatalError(MGL_result(),driver,mode);
	return dc;
}

void doTests(MGLDC *mgldc)
{
	int			i;
	FXForm2d	m;
	Quick2d		dc(mgldc);
	FXPoint2d	o,start,end;

	o = FXPoint2d(0,0);
//	dc.translate(REAL(0),REAL(-2));
	dc.scale(REAL(1),REAL(0.5));
//	dc.scaleAbout(REAL(1),REAL(0.5),FXPoint2d(REAL(1),REAL(1)));
	dc.rotate(REAL(30));
//	dc.rotateAbout(REAL(30),FXPoint2d(REAL(0.5),REAL(0)));
	dc.marker(o);

	dc.setCirclePrecision(60);
	dc.circle(o,REAL(0.9));
	dc.rect(REAL(-.9),REAL(.9),REAL(.9),REAL(-.9));
	waitEvent();

	dc.setColor(MGL_RED);
	dc.circle(o,REAL(0.9));

	dc.setColor(MGL_RED);
	dc.fillCircle(o,REAL(0.9));
	dc.setColor(MGL_WHITE);
	dc.circleOutline();
	waitEvent();

	dc.setColor(MGL_BLUE);
	dc.fillCircle(o,REAL(0.9));
	dc.setColor(MGL_YELLOW);
	dc.circleOutline();

	dc.setColor(MGL_RED);
	dc.fillArc(o,REAL(0.9),REAL(60),REAL(30));
	dc.setColor(MGL_WHITE);
	dc.arcOutline();

	waitEvent();

	dc.setColor(MGL_BLUE);
	dc.fillArc(o,REAL(0.9),REAL(60),REAL(30));
	dc.setColor(MGL_YELLOW);
	dc.arcOutline();

	o = FXPoint2d(REAL(20),REAL(20));

	dc.worldToView(FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),
		REAL(0),REAL(200),REAL(0),REAL(200));
	dc.scale(REAL(1),REAL(3));
	dc.rotate(REAL(30));

	font_t *font = MGL_loadFont("little.fnt");
	if (!font)
		fatalError(MGL_result());

	dc.useFont(font);
	dc.drawStr(o,"This is a test");
	waitEvent();

	dc.setColor(MGL_RED);
	dc.rotate(REAL(-15));
	dc.drawStr(o,"This is another test");
	waitEvent();

	FXPoint2d	vArray1[] = {FXPoint2d(REAL(-0.5),REAL(-0.5)),FXPoint2d(REAL(-0.5),REAL(0.5)),
						 FXPoint2d(REAL(0.5),REAL(0.5)),FXPoint2d(0,0),FXPoint2d(REAL(0.5),REAL(-0.5))};

	dc.worldToView(FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),
		REAL(-1),REAL(1),REAL(-1),REAL(1));
	dc.rotate(REAL(15));

	dc.clearDevice();
	dc.setColor(MGL_WHITE);
	dc.circle(o,REAL(0.9));
	dc.rect(REAL(-.9),REAL(.9),REAL(.9),REAL(-.9));
	dc.setColor(MGL_RED);
	dc.fillPolygon(5,vArray1);
	waitEvent();

	FXPoint2d	ul(REAL(-0.5),REAL(0.5)),lr(REAL(0.5),REAL(-0.5));

	dc.setColor(MGL_YELLOW);
	dc.rect(ul,lr);
	waitEvent();

	dc.clearDevice();
	dc.setColor(MGL_BLUE);
	dc.fillRect(ul,lr);
	waitEvent();

	dc.worldToView(FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),
		0,REAL(360),REAL(-1),REAL(1));

	FXPoint2d	vArray2[36];

	for (i = 0; i < 36; i++) {
		vArray2[i].x = FXmul(FXintToReal(i),REAL(10));
		vArray2[i].y = FXsin(vArray2[i].x);
		}

	dc.clearDevice();
	dc.setMarkerColor(MGL_MAGENTA);
	dc.setMarkerStyle(MGL_MARKER_SQUARE);
	dc.setMarkerSize(5);

	for (i = 0; i < 100; i++) {
		dc.polyLine(36,vArray2);
		dc.polyMarker(36,vArray2);
		}
	waitEvent();

	dc.setMarkerColor(MGL_GREEN);
	dc.setMarkerStyle(MGL_MARKER_CIRCLE);
	for (i = 0; i < 100; i++)
		dc.polyLineMarker(36,vArray2);
	waitEvent();

	dc.worldToView(FXintToReal(dc.maxx()),FXintToReal(dc.maxy()),
		0,REAL(360),REAL(-1),REAL(1));

	FXPoint2d	p1(0,REAL(-0.5)),p2(0,REAL(0.5)),p3(REAL(360),REAL(0.5)),
			p4(REAL(360),REAL(-0.5)),p;

	dc.clearDevice();
	dc.beginLine();
		dc.setColor(1);
		dc.moveTo(p1);
		dc.lineTo(p2);
		dc.setColor(2);
		dc.lineTo(p3);
		dc.setColor(3);
		dc.lineTo(p4);
		dc.setColor(4);
		dc.lineTo(p1);
	dc.endLine();
	waitEvent();
}

void main(void)
{
	doTests(initMGL());
	MGL_exit();
}
