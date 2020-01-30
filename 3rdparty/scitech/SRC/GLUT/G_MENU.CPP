/****************************************************************************
*
*                       SciTech MGL Graphics Library
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
* 				 Portions Copyright (c) Mark J. Kilgard, 1994.
*
*
* Language:   	ANSI C
* Environment:  IBM PC (MS DOS)
*
* Description:  SciTech MGL minimal GLUT implementation for fullscreen
*				and windowed operation.
*
*
****************************************************************************/

#include "glutint.h"

/*------------------------- Implementation --------------------------------*/

/* OBSOLETE - use glutMenuStatusFunc instead. */
void glutMenuStateFunc(GLUTmenuStateCB menuStateFunc)
{
	glutMenuStatusFunc((GLUTmenuStatusCB)menuStateFunc);
}

void glutMenuStatusFunc(GLUTmenuStatusCB menuStatusFunc)
{
}

int glutCreateMenu(GLUTselectCB selectFunc)
{
	return 0;
}

void glutDestroyMenu(int menunum)
{
}

int glutGetMenu(void)
{
	return 0;
}

void glutSetMenu(int menuid)
{
}

void glutAddMenuEntry(char *label, int value)
{
}

void glutAddSubMenu(char *label, int menu)
{
}

void glutChangeToMenuEntry(int num, char *label, int value)
{
}

void glutChangeToSubMenu(int num, char *label, int menu)
{
}

void glutRemoveMenuItem(int num)
{
}

void glutAttachMenu(int button)
{
}

void glutDetachMenu(int button)
{
}


