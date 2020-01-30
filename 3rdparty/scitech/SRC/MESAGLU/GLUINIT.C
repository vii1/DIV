/****************************************************************************
*
*						MegaGraph Graphics Library
*
*                   Copyright (C) 1997 SciTech Software.
*							All rights reserved.
*
* Filename:		$Workfile:   GLUINIT.C  $
* Version:		$Revision:   1.0  $
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:  Mesa GLU library initialisation code.
*
* $Date:   11 Jul 1997 12:15:20  $ $Author:   KendallB  $
*
****************************************************************************/

#include "gl\gl.h"
#include "gl\glu.h"
#include <stdlib.h>

/*--------------------------- Global Variables ----------------------------*/

PUBLIC	MGL_glFuncs		_MGL_glFuncs;

/*------------------------- Implementation --------------------------------*/

PUBLIC void APIENTRY gluInitMGL(MGL_glFuncs *glFuncs)
{
	if (glFuncs->dwSize != sizeof(MGL_glFuncs))
    	exit(1);
	_MGL_glFuncs = *glFuncs;
}

