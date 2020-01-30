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
* Description:	Routines to register all the display device drivers. This
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
Registers all known packed pixel memory drivers to be linked.

HEADER:
mgraph.h

REMARKS:
This function registers all available memory device context drivers to be
linked in, and also registers those drivers in the device detection chain.
This is a useful function to get code up and running quickly.  However, in
order to minimize the size of the resulting executable for release code, you
should only link in those drivers which will actually be used by your
application.

SEE ALSO:
MGL_registerDriver
***************************************************************************/
void MGLAPI MGL_registerAllMemDrivers(void)
{
	/* Register memory context drivers */
	MGL_registerDriver(MGL_PACKED1NAME,PACKED1_driver);
	MGL_registerDriver(MGL_PACKED4NAME,PACKED4_driver);
	MGL_registerDriver(MGL_PACKED8NAME,PACKED8_driver);
	MGL_registerDriver(MGL_PACKED16NAME,PACKED16_driver);
	MGL_registerDriver(MGL_PACKED24NAME,PACKED24_driver);
	MGL_registerDriver(MGL_PACKED32NAME,PACKED32_driver);
}
