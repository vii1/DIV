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
* Filename:		$Workfile$
* Version:		$Revision$
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	PNG bitmap resource loading/unloading routines. We support
*				loading and saving of 24-bit imagery files, since the PNG
*				format is RGB only. If you load a PNG into an 8-bit device
*				context, it will be dithered. If you save a PNG file from
*				an 8bpp device context it will be converted to a 24-bit
*				file when written to disk. Hence if you plan to edit PNG
*				files and then save the results to disk, do this with
*				a 24-bpp display or memory DC for the best results.
*
*
****************************************************************************/

#include "mgl.h"
#pragma hdrstop

