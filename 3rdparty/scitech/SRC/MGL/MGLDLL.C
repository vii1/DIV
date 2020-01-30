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
* Environment:	any
*
* Description:	Special routines to return the address of global variables
*				in 32 bit DLL's to avoid the need to export data. Watcom
*				C++ does not appear to be able to handle exported global
*				variables correctly at the moment.
*
*
****************************************************************************/

#ifdef	BUILD_DLL
#undef	BUILD_DLL
#define BUILD_MGLDLL
#include "mgraph.h"

void * MGLAPI _export VGA4_getDriverAddr(void)     { return VGA4_driver; }
void * MGLAPI _export VGA8_getDriverAddr(void)     { return VGA8_driver; }
void * MGLAPI _export VGAX_getDriverAddr(void)     { return VGAX_driver; }
void * MGLAPI _export SVGA4_getDriverAddr(void)    { return SVGA4_driver; }
void * MGLAPI _export SVGA8_getDriverAddr(void)    { return SVGA8_driver; }
void * MGLAPI _export SVGA16_getDriverAddr(void)   { return SVGA16_driver; }
void * MGLAPI _export SVGA24_getDriverAddr(void)   { return SVGA24_driver; }
void * MGLAPI _export SVGA32_getDriverAddr(void)   { return SVGA32_driver; }
void * MGLAPI _export LINEAR8_getDriverAddr(void)  { return LINEAR8_driver; }
void * MGLAPI _export LINEAR16_getDriverAddr(void) { return LINEAR16_driver; }
void * MGLAPI _export LINEAR24_getDriverAddr(void) { return LINEAR24_driver; }
void * MGLAPI _export LINEAR32_getDriverAddr(void) { return LINEAR32_driver; }
void * MGLAPI _export ACCEL8_getDriverAddr(void)   { return ACCEL8_driver; }
void * MGLAPI _export ACCEL16_getDriverAddr(void)  { return ACCEL16_driver; }
void * MGLAPI _export ACCEL24_getDriverAddr(void)  { return ACCEL24_driver; }
void * MGLAPI _export ACCEL32_getDriverAddr(void)  { return ACCEL32_driver; }
void * MGLAPI _export DDRAW8_getDriverAddr(void)   { return DDRAW8_driver; }
void * MGLAPI _export DDRAW16_getDriverAddr(void)  { return DDRAW16_driver; }
void * MGLAPI _export DDRAW24_getDriverAddr(void)  { return DDRAW24_driver; }
void * MGLAPI _export DDRAW32_getDriverAddr(void)  { return DDRAW32_driver; }
void * MGLAPI _export OPENGL_getDriverAddr(void)   { return OPENGL_driver; }
void * MGLAPI _export FSOGL8_getDriverAddr(void)   { return FSOGL8_driver; }
void * MGLAPI _export FSOGL16_getDriverAddr(void)  { return FSOGL16_driver; }
void * MGLAPI _export FSOGL24_getDriverAddr(void)  { return FSOGL24_driver; }
void * MGLAPI _export FSOGL32_getDriverAddr(void)  { return FSOGL32_driver; }
void * MGLAPI _export PACKED1_getDriverAddr(void)  { return PACKED1_driver; }
void * MGLAPI _export PACKED4_getDriverAddr(void)  { return PACKED4_driver; }
void * MGLAPI _export PACKED8_getDriverAddr(void)  { return PACKED8_driver; }
void * MGLAPI _export PACKED16_getDriverAddr(void) { return PACKED16_driver; }
void * MGLAPI _export PACKED24_getDriverAddr(void) { return PACKED24_driver; }
void * MGLAPI _export PACKED32_getDriverAddr(void) { return PACKED32_driver; }

#endif

