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
* Description:	Header file for internal definitions for line clipping.
*
*
****************************************************************************/

#ifndef	__CLIPLINE_H
#define	__CLIPLINE_H

#define	REJECT	false
#define	ACCEPT	true

#define	x1		(*fx1)			/* Access the first x coordinate		*/
#define	y1		(*fy1)			/* Access the first y coordinate		*/
#define	z1		(*fz1)			/* Access the first z coordinate		*/
#define	c1		(*fc1)			/* Access the first c coordinate		*/
#define	r1		(*fr1)			/* Access the first r coordinate		*/
#define	g1		(*fg1)			/* Access the first g coordinate		*/
#define	b1		(*fb1)			/* Access the first b coordinate		*/
#define	x2		(*fx2)			/* Access the second x coordinate		*/
#define	y2		(*fy2)			/* Access the second y coordinate		*/
#define	z2		(*fz2)			/* Access the second z coordinate		*/
#define	c2		(*fc2)			/* Access the second c coordinate		*/
#define	r2		(*fr2)			/* Access the second r coordinate		*/
#define	g2		(*fg2)			/* Access the second g coordinate		*/
#define	b2		(*fb2)			/* Access the second b coordinate		*/

/* Macro to set up the outcodes */

#define	SETOUTCODES(outcode,x,y)		\
	(outcode) = 0;						\
	if ((y) >= bottom) 	(outcode) |= 1;	\
	if ((y) < top) 		(outcode) |= 2;	\
	if ((x) >= right)	(outcode) |= 4;	\
	if ((x) < left) 	(outcode) |= 8

#endif	/* __CLIPLINE_H */
