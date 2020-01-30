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
* Description:	General header file for the Techniques Class library.
*
*
****************************************************************************/

#ifndef	__TCL_TECHLIB_HPP
#define	__TCL_TECHLIB_HPP

#ifndef	__SCITECH_H
#include "scitech.h"
#endif

/*--------------------------- Inline Functions ----------------------------*/

//---------------------------------------------------------------------------
// Set of debugging macros used by the library. If the debug flag is
// set, they are turned on depending on the setting of the flag. If DEBUG
// is set to 2, all checks are turned on, if DEBUG is set to 1, only the
// pre-conditions are enabled. If DEBUG is 0 or undefined, no checks are
// peformed.
//---------------------------------------------------------------------------

extern void (*TCL_checkFail)(char *msg,char *cond,char *file,int line);
void TCL_defaultCheckFail(char *msg,char *cond,char *file,int line);

#undef	PRECONDITION
#undef	CHECK

#ifdef	DEBUG
#       define  PRE(x)  x
#       define  PRECONDITION(p)                                     \
		if (!(p)) TCL_checkFail(									\
			"Precondition failed: '%s', file %s, line %d\n",		\
			#p, __FILE__, __LINE__)
#if		DEBUG > 1
#       define  CHK(x)  x
#		define	CHECK(p)											\
		if (!(p)) TCL_checkFail(									\
			"Check failed: '%s', file %s, line %d\n",				\
			#p, __FILE__, __LINE__)
#else
#       define  CHK(x)
#		define	CHECK(p)		((void)0)
#endif
#else
#		define	PRECONDITION(p)	((void)0)
#		define	CHECK(p)		((void)0)
#       define  PRE(x)
#       define  CHK(x)
#endif

#endif	// __TCL_TECHLIB_HPP
