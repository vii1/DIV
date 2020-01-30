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
* Language:		C++ 3.0
* Environment:	any
*
* Description:  3d text drawing routines. The text is drawn using the
*				currently active MGL font, but only if this font is a
*				vector font.
*
*				The text is drawn in the XY plane, with the Z coordinate
*				from the initial starting point.
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

PRIVATE	FXPoint3d	origin;
PRIVATE Quick3d	*dc;

PRIVATE void _ASMAPI move(int x,int y)
{
	FXPoint3d p(FXintToReal(x)+origin.x,FXintToReal(y)+origin.y,origin.z);
	dc->moveTo(p);
}

PRIVATE void _ASMAPI draw(int x,int y)
{
	FXPoint3d p(FXintToReal(x)+origin.x,FXintToReal(y)+origin.y,origin.z);
	dc->lineTo(p);
}

void Quick3d::drawStr(const FXPoint3d& p,char *str)
/****************************************************************************
*
* Function:		Quick3d::drawStr
* Parameters:	p	- Point to start drawing text at
*				str	- String to draw at the location
* Returns:		True if string was drawn, false if not.
*
* Description:	Draws the string in the current world coordinate system.
*				The string is drawn in the current MGL font, with the
*				current justification etc.
*
****************************************************************************/
{
	origin = p;
	validateTotalMapping();
	::dc = this;
	MGL_vecFontEngine(0,0,str,move,draw);
}
