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
*
* Language:		ANSI C/C++
* Environment:	any
*
* Description:	Header file for defining a number of macros for performing
*				fuzzy arithmetic on single and double precision
*				floating point values.
*
*				To avoid namespace clashes, all inline functions defined
*				in this header are in uppercase. They can be used to
*				replace previous macro versions of the functions (in fact
*				these are automatically #undef'd to avoid problems).
*
* $Id: fudge.h 1.1 1995/09/26 15:06:33 kjb release $
*
****************************************************************************/

#ifndef	__FX_FUDGE_H
#define	__FX_FUDGE_H

/*------------------------- Internal Macro's etc --------------------------*/

/* Undefine any possible macros that may already be defined to avoid
 * complications.
 */

#undef	FZERO
#undef	FEQ
#undef	FLE
#undef	FGE
#undef	FNE
#undef	FLT
#undef	FGT
#undef	LERP

/* Define macros versions of the routines which are quicker when the inline
 * function calls get deeply nested (the optimiser does a better job since
 * the arguments of an inline function call must be fully evaluated before
 * the next call). However the inline function versions are completely
 * side effect free.
 */

#define	_FZERO(a,FUDGE)	((a) < FUDGE && (a) > -FUDGE)

#define	_FEQ(a,b,FUDGE)	_FZERO((a)-(b),FUDGE)
#define	_FLE(a,b,FUDGE)	(((a) < (b)) || _FEQ(a,b,FUDGE))
#define	_FGE(a,b,FUDGE)	(((a) > (b)) || _FEQ(a,b,FUDGE))

#define	_FNE(a,b,FUDGE)	(!_FZERO((a)-(b),FUDGE))
#define	_FLT(a,b,FUDGE)	(((a) < (b)) && _FNE(a,b,FUDGE))
#define	_FGT(a,b,FUDGE)	(((a) > (b)) && _FNE(a,b,FUDGE))
#define	_LERP(lo,hi,alpha)	((lo) + FXmul((hi)-(lo),alpha))
#define	_SIGN(a)		((a) > 0 ? 1 : -1)

#endif	/* __FX_FUDGE_H */
