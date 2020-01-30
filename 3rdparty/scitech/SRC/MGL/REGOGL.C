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
* Description:	Routines to register all the OpenGL device drivers. This
*				routine is separate so that the unused display drivers
*				will not be linked in.
*
*
****************************************************************************/

#define	BUILD_MGLDLL
#include "mgraph.h"

/*--------------------------- Global Variables ----------------------------*/

/****************************************************************************
DESCRIPTION:
Registers all OpenGL drivers to be linked in and used for detection

HEADER:
mgraph.h

REMARKS:
This function registers all available OpenGL hardware display drivers to be
linked in, and also registers those drivers in the device detection chain.
This is a useful function to get code up and running quickly.  However, in
order to minimize the size of the resulting executable for release code, you
should only link in those drivers which will actually be used by your
application.

SEE ALSO:
MGL_registerDriver
****************************************************************************/
void MGLAPI MGL_registerAllOpenGLDrivers(void)
{
#ifdef	__WINDOWS__
	MGL_registerDriver(MGL_OPENGLNAME,OPENGL_driver);
	MGL_registerDriver(MGL_FSOGL8NAME,FSOGL8_driver);
	MGL_registerDriver(MGL_FSOGL16NAME,FSOGL16_driver);
	MGL_registerDriver(MGL_FSOGL24NAME,FSOGL24_driver);
	MGL_registerDriver(MGL_FSOGL32NAME,FSOGL32_driver);
#endif
}

