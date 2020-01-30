/****************************************************************************
*
*						  Techniques Class Library
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
* Description:	Module to implement default error handling.
*
*
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "tcl/error.hpp"
#ifdef	__WINDOWS__
#define	STRICT
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/*--------------------------- Global Variables ----------------------------*/

// Pointer to the currently installed error handler

void (*TCL_errorHandler)(int err) = TCL_defaultErrorHandler;
void (*TCL_checkFail)(char *msg,char *cond,char *file,int line) = TCL_defaultCheckFail;

/*---------------------------- Implementation -----------------------------*/

void TCL_defaultErrorHandler(int err)
{
	char	buf[80];

	strcpy(buf,"TECH: ");
	switch (err) {
		case STK_UNDERFLOW:
			strcat(buf,"Stack underflow!");
			break;
		case STK_OVERFLOW:
			strcat(buf,"Stack overflow!");
			break;
		default:
			strcat(buf,"Unknown internal error!");
		}
#ifdef	__WINDOWS__
	MessageBox(NULL, buf,"TechLib Error",MB_ICONEXCLAMATION);
#else
	fprintf(stderr,buf);
#endif
	exit(EXIT_FAILURE);
}

void TCL_defaultCheckFail(char *msg,char *cond,char *file,int line)
{
#ifdef	__WINDOWS__
	char	buf[80];
	sprintf(buf,msg,cond,file,line);
	MessageBox(NULL, buf,"TechLib Error",MB_ICONEXCLAMATION);
#else
	fprintf(stderr,msg,cond,file,line);
#endif
	exit(EXIT_FAILURE);
}
