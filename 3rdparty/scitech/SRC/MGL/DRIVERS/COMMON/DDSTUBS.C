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
* Description:	Module to implement the C versions of the DirectDraw
*				rendering stubs for non-Intel processors.
*
*
****************************************************************************/

#include "mgldd.h"
#pragma hdrstop
#include "drivers\common\gddraw.h"

/*-------------------------- Implementation -------------------------------*/

#ifdef	NO_ASSEMBLER

#define	MKSTUB_INT(name,pname,proto,args)	\
void _ASMAPI DDSTUB_##name##proto			\
{											\
	if (++_MGL_surfLock == 0)				\
		DDRAW_lock();                       \
	_PACKED_vecs.pname##args;				\
	if (--_MGL_surfLock == -1)				\
		DDRAW_unlock();                     \
}

#define	MKSTUB(name,proto,args)				\
	MKSTUB_INT(name,name,proto,args)

#define	MKSTUB_I(name1,name2,proto,args)							\
	MKSTUB_INT(solid##name1,solid.name2,proto,args);				\
	MKSTUB_INT(ropSolid##name1,ropSolid.name2,proto,args);			\
	MKSTUB_INT(patt##name1,patt.name2,proto,args);					\
	MKSTUB_INT(colorPatt##name1,colorPatt.name2,proto,args);		\
	MKSTUB_INT(fatSolid##name1,fatSolid.name2,proto,args);			\
	MKSTUB_INT(fatRopSolid##name1,fatRopSolid.name2,proto,args);	\
	MKSTUB_INT(fatPatt##name1,fatPatt.name2,proto,args);			\
	MKSTUB_INT(fatColorPatt##name1,fatColorPatt.name2,proto,args);	\
	MKSTUB_INT(dither##name1,dither.name2,proto,args);				

#define	MKSTUB_Z(name,proto,args)				\
	MKSTUB_INT(z16_##name,z16.name,proto,args);	\
	MKSTUB_INT(z32_##name,z32.name,proto,args)

#define	MKSTUB_RET(type,name,proto,args)	\
type _ASMAPI DDSTUB_##name##proto			\
{											\
	type ret;								\
	if (++_MGL_surfLock == 0)				\
        DDRAW_lock();                       \
	ret = _PACKED_vecs.name##args;			\
	if (--_MGL_surfLock == -1)				\
        DDRAW_unlock();                     \
	return ret;								\
}

#include "drivers\common\cstubs.c"

#endif

