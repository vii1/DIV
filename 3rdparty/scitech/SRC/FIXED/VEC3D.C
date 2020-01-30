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
* Language:     ANSI C
* Environment:	any
*
* Description:	Implementation for 3d vectors routines.
*
*
****************************************************************************/

#include "fx/vec3d.h"

/*------------------------- Implementation --------------------------------*/

#define SELECT	REAL(0.7071)	// selection constant (roughly .5*sqrt(2)

void FXVec3dPerp(FXVec3d *result,const FXVec3d *v1)
/****************************************************************************
*
* Function:		FXVec3d::perp
* Parameters:	result	- Place to store vector perpendicular to v1
*               v1		- Vector to start with
*
* Description:	Finds a vector perpendicular another vector. The method
*				used is to take any vector (say <0,1,0>) and subtract the
*				portion of it pointing in the vectors direction. This
*				doesn't work if the vector IS <0,1,0> or is very near it.
*				So in this case we use <0,0,1> instead.
*
****************************************************************************/
{
	FXVec3d	r,norm;
	real	dot;

	/* Start by trying vector <0,1,0> */
	FXVec3dSet(r,REAL(0),REAL(1),REAL(0));
	FXVec3dCopy(norm,*v1);
	FXVec3dNormalise(norm);

	/* Check to see if the vector is too close to <0,1,0>. If so, use
	 * <0,0,1> instead.
	 */
	dot = FXVec3dDot(norm,r);
	if (dot > SELECT || dot < -SELECT) {
		r.x = REAL(0);
		r.y = REAL(1);
		dot = FXVec3dDot(norm,r);
		}

	/* Subtract off the non-perpendicular part */
	r.x -= FXmul(dot,norm.x);
	r.y -= FXmul(dot,norm.y);
	r.z -= FXmul(dot,norm.z);

	/* Make result unit length */
	FXVec3dNormalise(r);
	FXVec3dCopy(*result,r);
}
