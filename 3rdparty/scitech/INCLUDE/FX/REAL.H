/****************************************************************************
*
*				  High Speed Fixed/Floating Point Library
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
* Filename:		$RCSfile: real.h $
*
* Language:     ANSI C/C++
* Environment:	any
*
* Description:	Header file to include either fx/fixed.h or fx/float.h
*				depending on the setting of compile time switches.
*				Whenever it is possible that you will used either
*				floating point or fixed point, include this header file.
*
*				It makes sure that the correct typedef is defined for
*				the 'real' type.
*
* $Id: real.h 1.1 1995/09/26 15:06:33 kjb release $
*
****************************************************************************/

#ifndef	__FX_REAL_H
#define	__FX_REAL_H

#ifndef	__SCITECH_H
#include "scitech.h"
#endif

/*------------------------- Internal Macro's etc --------------------------*/

/* Determine what type to use for floating point numbers, either float
 * double or long double. We default to single precision floating point if
 * nothing is specified.
 */

#if	!defined(FX_FLOAT) && !defined(FX_DOUBLE) && !defined(FX_FIXED)
#define	FX_FLOAT
#endif

#if	defined(FX_FLOAT) || defined(FX_DOUBLE)
#include "fx/float.h"
#endif

#ifdef	FX_FIXED
#include "fx/fixed.h"
typedef	FXFixed		real;
typedef	FXFixed		realdbl;
#undef	_FUDGE
#define	_FUDGE		(4L)
#endif

#ifndef	__FUDGE_H
#include "fx/fudge.h"
#endif

/* Decide what fudge factor to use in equality and tests for single and
 * double precision floating point numbers.
 *
 * We use a default factor defined below if none is set. Another factor
 * can be set by defining the fudge factor before including this file.
 */

#ifndef	_FUDGE
#define	_FUDGE	(real)(1e-6)
#endif

#endif	/* __FX_REAL_H */

