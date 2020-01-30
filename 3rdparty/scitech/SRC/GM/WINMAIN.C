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
*
*
* Language:   	ANSI C
* Environment:  Win32
*
* Description:  Simple module to call the regular C style main() function
*				for Windows programs.
*
*
****************************************************************************/

#include "gm/gm.h"
#include "getopt.h"

#ifdef	__WINDOWS__
#undef	WINGDIAPI
#undef	APIENTRY
#undef	STRICT
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>

/*--------------------------- Global Variables ----------------------------*/

#define	MAX_ARGV	20
PRIVATE char			*argv[MAX_ARGV];
PRIVATE int				argc;
extern	ibool			__glutDone;

/*------------------------- Implementation --------------------------------*/

extern int main(int argc,char *argv[]);

int PASCAL WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR szCmdLine,int sw)
{
	char		szModuleName[260];
	GetModuleFileName(hInst,szModuleName,sizeof(szModuleName));
	parse_commandline(szModuleName,szCmdLine,&argc,argv,MAX_ARGV);
	return main(argc,argv);
}

#endif	/* __WINDOWS__ */


