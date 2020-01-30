/****************************************************************************
*
*					 MegaVision Application Framework
*
*			A C++ GUI Toolkit for the MegaGraph Graphics Library
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
* Environment:	IBM PC (MS DOS)
*
* Description:	Module to precompile all of the header file required by
*				the library. This is pointless during development, but
*				can significantly speed up compilation times when using
*				the library.
*
*				You should include this file as the first file in your
*				project or makefile, to ensure that it gets compiled first,
*				with the same set of macros and compilation settings.
*
*
****************************************************************************/

#include "mvis/mvision.hpp"
#pragma	hdrstop

MVHotKey	_MV_noHotKey(kbNoKey,0);
MVRect		_MV_emptyRect(0,0,0,0);
MVRect		_MV_pointRect(0,0,1,1);
int			_MV_systemFont;
int			_MV_systemFontSize;
int 		_MV_sysScrollBarWidth;
int			_MV_iconOffset;
