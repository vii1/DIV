/****************************************************************************
*
*			  Quick3D - A 3D C++ rendering pipeline for the MGL
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
* Description:	Internal header file for the Quick3D library.
*
*
****************************************************************************/

#ifndef	__QUICK3DI_HPP
#define	__QUICK3DI_HPP

#ifndef	__QUICK3D_QUICK3D_HPP
#include "quick3d/quick3d.hpp"
#endif

#ifndef	__DEPTHSRT_HPP
#include "depthsrt.hpp"
#endif

/*-------------------------- Class Definitions ----------------------------*/

/*---------------------- Interal macros and typedefs ----------------------*/

// Macro to swap two values

#define SWAP(a,b)	{ a^=b; b^=a; a^=b; }

// Define the offset for Z to avoid aliasing problems

#define Z_OFS       0x300000L
#define Z_LINEOFS   0x400000L

// Macros to create references to appropriate portions of the fxpointrgbz_t
// structure so that we can intepret it as many different structures

#define	TOP(win)	(*((fxpoint_t*)(&(win).p)))
#define	TOC(win)	(*((fxpointc_t*)(&(win).c.b)))
#define	TORGB(win)	(*((fxpointrgb_t*)(&(win).c)))

#define	TOZ(win)	(*((fxpointz_t*)(&(win).p)))
#define	TOCZ(win)	(*((fxpointcz_t*)(&(win).c.b)))
#define	TORGBZ(win)	(win)

#define	TOC2(v)		(*((fxpointc_t*)(&(v).c)))

// Macros to clamp values to a specified range

#define	CLAMP1I(x,max)		(x > max ? max : x)
#define CLAMP2I(x,min,max)	((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define	CLAMP1(x,max)		CLAMP1I(x,REAL(max))
#define CLAMP2(x,min,max)   CLAMP2I(x,REAL(min),REAL(max))

/*------------------------- Inline member functions -----------------------*/

inline void Quick3d::lightVertices(int backfacing)
{
	if (backfacing && dualShade) {
		if (shading == LitFlatShading) {
			if (rgbFlag)
				lightBackFlatRGB();
			else lightBackFlatIndex();
			}
		else {
			if (rgbFlag)
				lightBackSmoothRGB();
			else lightBackSmoothIndex();
			}
		setShadeTable(bMaterial->shadeTab);
		}
	else {
		if (shading == LitFlatShading) {
			if (rgbFlag)
				lightFrontFlatRGB();
			else lightFrontFlatIndex();
			}
		else {
			if (rgbFlag)
				lightFrontSmoothRGB();
			else lightFrontSmoothIndex();
			}
		setShadeTable(fMaterial->shadeTab);
		}
}

inline void Quick3d::storeVertex(int i,const FXPoint3d& v)
{
	FXPoint4d temp;
	map(TOZ(pipe[i].win),temp,v);
	if (shading == LitSmoothShading)
		pipe[i].normal = cntNormal;
	else if (rgbFlag)
		TORGB(pipe[i].win).c = fxrgbColor;
	else TOC(pipe[i].win).c = fxColor;
}

inline void Quick3d::recycleVertex(int dst,int src)
{
	pipe[dst] = pipe[src];
}

inline ibool Q3D_backfacing(fxpointrgbz_t p0,fxpointrgbz_t p1,fxpointrgbz_t p2,
	int culling)
/****************************************************************************
*
* Function:		Q3D_backfacing
* Parameters:	p0,p1,p2	- Points in the triangle (clockwise order)
*				culling		- Backface culling flag.
* Returns:		True if triangle is backfacing.
*
* Description:	Determines if a triangle with the points p0,p1 and p2 is
*				backfacing, where a backfacing polygon is one where the
*				coordinates defining the polygon are in the order specified
*				by the 'culling' paramater. This is done by examining
*				the sign of the Z coordinate for the cross product of the
*				vectors from p0 to p1 and p1 to p2.
*
*				Note that this routine is intended to work in screen
*				coordinates, after the polygon has been projected into
*				the Normalised Device Coordinate (NDC) system.
*
****************************************************************************/
{
	int	r = MGL_backfacing(
		p1.p.x - p0.p.x,p1.p.y - p0.p.y,
		p2.p.x - p1.p.x,p2.p.y - p1.p.y);
	return culling == Quick3d::ClockWise ? r : !r;
}

/*------------------------- Function Prototypes ---------------------------*/

#endif	// __QUICK3DI_HPP
