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
* Description:	Header file for error handling module.
*
*
****************************************************************************/

#ifndef	__TCL_ERROR_HPP
#define	__TCL_ERROR_HPP

#ifndef	__TCL_TECHLIB_HPP
#include "tcl/techlib.hpp"
#endif

#define	STK_UNDERFLOW	-1
#define	STK_OVERFLOW	-2

// Pointer to the currently installed error handler

extern void	(*TCL_errorHandler)(int err);
void TCL_defaultErrorHandler(int err);

#endif	// __TCL_ERROR_HPP
