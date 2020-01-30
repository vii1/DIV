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
* Description:  C based polygon rendering helper functions. These are
*				normally coded in high performance assembler for the
*				target machine and are used by the software polygon
*				rendering code.
*
*
****************************************************************************/

#include "mgl.h"

#ifndef	MGL_LITE

/*------------------------- Implementation --------------------------------*/

#if	defined(NO_ASSEMBLER)

/* {secret} */
int _ASMAPI _MGL_computeSlope(fxpoint_t *v1,fxpoint_t *v2,fix32_t *slope)
/****************************************************************************
*
* Function:		_MGL_computeSlope
* Parameters:	v1		- First vertex in edge
*				v2		- Second vertex in edge
*               slope	- Pointer to place to store slope
* Returns:      Sign of slope (1 = +ve, 0, -1 = i-ve)
*
* Description:	Compute the slope of the edge and return the sign of the
*				slope.
*
****************************************************************************/
{
	fix32_t	dy;

	if ((dy = v2->y - v1->y) > 0) {
		if (dy <= MGL_FIX_1)
			*slope = v2->x - v1->x;
		else
			*slope = MGL_FixDiv(v2->x - v1->x,dy);
		return 1;
		}
	if (dy < 0)
		return -1;
	return 0;
}

#ifdef	MGL_FIX3D
/* {secret} */
int _ASMAPI _MGL_cComputeSlope(fxpointc_t *v1,fxpointc_t *v2,fix32_t *slope,
	fix32_t *cSlope)
/****************************************************************************
*
* Function:		_MGL_cComputeSlope
* Parameters:	v1		- First vertex in edge
*				v2		- Second vertex in edge
*               slope	- Pointer to place to store slope
*				cslope	- Pointer to place to store color slope
* Returns:      Sign of slope (1 = +ve, 0, -1 = i-ve)
*
* Description:	Compute the coordinate slope and color slope of the edge
*				and return the sign of the coordinate slope.
*
****************************************************************************/
{
	fix32_t	dy;

	if ((dy = v2->p.y - v1->p.y) > 0) {
		if (dy <= MGL_FIX_1) {
			*slope = v2->p.x - v1->p.x;
			*cSlope = v2->c - v1->c;
			}
		else {
			*slope = MGL_FixDiv(v2->p.x - v1->p.x,dy);
			*cSlope = MGL_FixDiv(v2->c - v1->c,dy);
			}
		return 1;
		}
	if (dy < 0)
		return -1;
	return 0;
}

/* {secret} */
int _ASMAPI _MGL_rgbComputeSlope(fxpointrgb_t *v1,fxpointrgb_t *v2,
	fix32_t *slope,fix32_t *rSlope,fix32_t *gSlope,fix32_t *bSlope)
/****************************************************************************
*
* Function:		_MGL_cComputeSlope
* Parameters:	v1		- First vertex in edge
*				v2		- Second vertex in edge
*               slope	- Pointer to place to store slope
*				rSlope	- Pointer to place to store red channel slope
*				gSlope	- Pointer to place to store green channel slope
*				bSlope	- Pointer to place to store blue channel slope
* Returns:      Sign of slope (1 = +ve, 0, -1 = i-ve)
*
* Description:	Compute the coordinate slope and color slope of the edge
*				and return the sign of the coordinate slope.
*
****************************************************************************/
{
	fix32_t	dy;

	if ((dy = v2->p.y - v1->p.y) > 0) {
		if (dy <= MGL_FIX_1) {
			*slope = v2->p.x - v1->p.x;
			*rSlope = v2->c.r - v1->c.r;
			*gSlope = v2->c.g - v1->c.g;
			*bSlope = v2->c.b - v1->c.b;
			}
		else {
			*slope = MGL_FixDiv(v2->p.x - v1->p.x,dy);
			*rSlope = MGL_FixDiv(v2->c.r - v1->c.r,dy);
			*gSlope = MGL_FixDiv(v2->c.g - v1->c.g,dy);
			*bSlope = MGL_FixDiv(v2->c.b - v1->c.b,dy);
			}
		return 1;
		}
	if (dy < 0)
		return -1;
	return 0;
}

/* {secret} */
void _ASMAPI _MGL_preRenderScanLine16(int x0,int minx,int maxx)
/****************************************************************************
*
* Function:		preRenderScanLine16
* Parameters:	x0		- Reference or base X coordinate
*               minx	- Minimum X coordinate in polygon
*				maxx	- Maximum X coordinate in polygon
*
* Description:	Pre-renders the linearly interpolated z offset values
*				into the z offset buffer. We only render z values between
*				minx and maxx inclusive.
*
****************************************************************************/
{
	zfix32_t	z1,dZdX = DC.tr.dZdX;
	zbuf16_t	*z = DC.zOffset;

	/* Compute the initial offset of minx from x0 */
	z1 = MGL_FixMul(dZdX,MGL_TOFIX(minx - x0));

	/* Render the z buffer offsets into the buffer */
	z += minx;
	while (minx < maxx) {
		*z++ = MGL_TOZBUF16(z1);
		z1 += dZdX;
		minx++;
		}
}

/* {secret} */
void _ASMAPI _MGL_preRenderScanLine32(int x0,int minx,int maxx)
/****************************************************************************
*
* Function:		preRenderScanLine32
* Parameters:	x0		- Reference or base X coordinate
*               minx	- Minimum X coordinate in polygon
*				maxx	- Maximum X coordinate in polygon
*
* Description:	Pre-renders the linearly interpolated z offset values
*				into the z offset buffer. We only render z values between
*				minx and maxx inclusive.
*
****************************************************************************/
{
	zfix32_t	z1,dZdX = DC.tr.dZdX;
	zbuf32_t	*z = DC.zOffset;

	/* Compute the initial offset of minx from x0 */
	z1 = MGL_FixMul(dZdX,MGL_TOFIX(minx - x0));

	/* Render the z buffer offsets into the buffer */
	z += minx;
	while (minx < maxx) {
		*z++ = MGL_TOZBUF32(z1);
		z1 += dZdX;
		minx++;
		}
}

#ifdef	__64BIT__
// TODO: Re-code the functions using 64-bit integers as this will be a lot
//		 more efficient that doing it with 32-bit integers.
#endif

#ifndef	__64BIT__
/* {secret} */
void _ASMAPI _MGL_zComputeSlope(fxpointz_t *v1,fxpointz_t *v2,fxpointz_t *v3)
/****************************************************************************
*
* Function:		_MGL_zComputeSlope
* Parameters:   v1,v2,v3	- Three vertices in the polygon
*
* Description:	Compute the z slope across the entire face of the polygon
*				from the following equations (where v1,v2 and v3 are three
*				vertices in the polygon with components x?,y?,z?):
*
*					dZ   (y2-y1)(z3-z1) - (y3-y1)(z2-z1)
*					-- = -------------------------------
*					dY   (y2-y1)(x3-x1) - (y3-y1)(x2-x1)
*
* 				and
*
*					dZ   (z2-z1)(x3-x1) - (z3-z1)(x2-x1)
*					-- = -------------------------------
*					dX   (y2-y1)(x3-x1) - (y3-y1)(x2-x1)
*
* 				Because we are working with screen space coordinates, we
*				are going to have major overflow problems with this
*				calculation in fixed point. If we are doing the code in 386
*				assembler, we can use 64 bit intermediate results to ensure
*				we have the precision we need. For the C version we do it in
*				floating point and convert back to fixed point (slow).
*
*				Note that we store the dZdX and dZdY values into the current
*				DC.tr structure directly.
*
****************************************************************************/
{
#define	FLT(x)	(float)((x)/65536.0F)
#define	FLTZ(x)	(float)((x)/268435456.0F)
#define	FIXZ(x)	(zfix32_t)((x)*268435456.0F)

	float dZ,dX1,dY1,dX2,dY2,dZ1,dZ2;
	dX1 = FLT(v2->p.x - v1->p.x);		/* x2 - x1 */
	dX2 = FLT(v3->p.x - v1->p.x);		/* x3 - x1 */
	dY1 = FLT(v2->p.y - v1->p.y);		/* y2 - y1 */
	dY2 = FLT(v3->p.y - v1->p.y);		/* y3 - y1 */
	dZ1 = FLTZ(v2->z - v1->z);			/* z2 - z1 */
	dZ2 = FLTZ(v3->z - v1->z);			/* z3 - z1 */

	dZ = dY1*dX2 - dY2*dX1;
	DC.tr.dZdX = FIXZ((dY1*dZ2 - dY2*dZ1)/dZ);
	DC.tr.dZdY = FIXZ((dZ1*dX2 - dZ2*dX1)/dZ);
}

#if !defined(__WATCOMC__)

/****************************************************************************
DESCRIPTION:
Multiplies two fixed point number in 16.16 format

HEADER:
mgraph.h

PARAMETERS:
a	- First number to multiply
b	- Second number to multiply

RETURNS:
Result of the multiplication.

REMARKS:
Multiplies two fixed point number in 16.16 format together
and returns the result. We cannot simply multiply the
two 32 bit numbers together since we need to shift the
64 bit result right 16 bits, but the result of a FXFixed
multiply is only ever 32 bits! Thus we must resort to
computing it from first principles (this is slow and
should ideally be re-coded in assembler for the target
machine).

We can visualise the fixed point number as having two
parts, a whole part and a fractional part:

	FXFixed = (whole + frac * 2^-16)

Thus if we multiply two of these numbers together we
get a 64 bit result:

	(f_whole + f_frac * 2^-16) * (g_whole + g_frac * 2^-16)	=
		(f_whole * g_whole) +
		(f_whole * g_frac)*2^-16 +
		(g_whole * f_frac)*2^-16 +
		(f_frac * g_frac)*2^-32

To convert this back to a 64 bit fixed point number to 32
bit format we simply shift it right by 16 bits (we can round
it by adding 2^-17 before doing this shift). The formula
with the shift integrated is what is used below. Natrually
you can alleviate most of this if the target machine can
perform a native 32 by 32 bit multiplication (since it
will produce a 64 bit result).

SEE ALSO:
MGL_FixDiv, MGL_FixMulDiv
****************************************************************************/
fix32_t	ASMAPI MGL_FixMul(
	fix32_t f,
	fix32_t g)
{
	fix32_t	f_whole,g_whole;
	fix32_t	f_frac,g_frac;

	// Extract the whole and fractional parts of the numbers. We strip the
	// sign bit from the fractional parts but leave it intact for the
	// whole parts. This ensures that the sign of the result will be correct.

	f_frac = f & 0x0000FFFF;
	f_whole = f >> 16;
	g_frac = g & 0x0000FFFF;
	g_whole = g >> 16;

	// We round the result by adding 2^(-17) before we shift the
	// fractional part of the result left 16 bits.

	return	((f_whole * g_whole) << 16) +
			(f_whole * g_frac) +
			(f_frac * g_whole) +
			((f_frac * g_frac + 0x8000) >> 16);
}

/****************************************************************************
DESCRIPTION:
Divides a fixed point number by another.

HEADER:
mgraph.h

PARAMETERS:
a	- Fixed point number to divide
b	- Fixed point number to divide by

RETURNS:
Result of the division.

SEE ALSO:
MGL_FixMul, MGL_FixMulDiv

REMARKS:
Divides a fixed point number by another fixed point number.
The idea is relatively simple; We want to set up a 64 bit
dividend to be divided by our 32 bit divisor, which will
give us a new 32 bit result.
****************************************************************************/
fix32_t	ASMAPI MGL_FixDiv(
	fix32_t f,
	fix32_t g)
{
	unsigned long	quot,rem;
	unsigned long	a,b;
	int				i,apos,bpos,carry;

	// Remember the sign of the operands and divide only positive numbers.

	apos = (f >= 0);
	a = apos ? f : -f;
	bpos = (g >= 0);
	b = bpos ? g : -g;

	// Do a standard shift based FXFixed division to get the result. Since
	// this is fixed point we are essentially dividing a 48 bit number
	// by a 32 bit number where the bottom 16 bits of the 48 bit dividend
	// are zeros (since our 32 bit number goes in the middle of the
	// 64 bit dividend in the figure above).
	//
	// Thus it is just like dividing a 32 bit number by a 32 bit number
	// but we loop around 48 times to get the extra precision that we
	// need!

	quot = a;
	rem = 0;
	for (i = 48; i; i--) {

		// Shift the 64 bits of quotient and remainder left one bit
		carry = quot >> 31;
		quot <<= 1;
		rem = (rem << 1) | carry;

		if (rem >= b) {
			quot |= 1;			// Build the quotient
			rem -= b;
			}
		}

	return ((apos == bpos) ? quot : -quot);
}

/****************************************************************************
DESCRIPTION:
Multiplies a fixed point number by another and divides by a third number.

HEADER:
mgraph.h

PARAMETERS:
a	- First number to multiply
b	- Second number to multiply
c	- Third number to divide by

RETURNS:
Results of the multiplication and division.

REMARKS:
This function multiplies a 16.16 fixed point number by another producing a 32.32
intermediate result. This 32.32 result is then divided by another 16.16 number to
produce a 16.16 result. Because this routine maintains maximum precision during
the multiplication stage, you can multiply numbers that would normally overflow
the standard MGL_FixMul function.

SEE ALSO:
MGL_FixMul, MGL_FixDiv

****************************************************************************/
fix32_t	ASMAPI MGL_FixMulDiv(
	fix32_t a,
	fix32_t b,
	fix32_t c)
{ return MGL_FixMul(a,MGL_FixDiv(b,c)); }

/****************************************************************************
DESCRIPTION:
Determines if a polygon is backfacing.

HEADER:
mgraph.h

PARAMETERS:
dx1	- change in x along first edge
dy1	- change in y along first edge
dx2	- change in x along second edge
dy2	- change in y along second edge

RETURNS:
1 if the polygon if backfacing, 0 if it is frontfacing

REMARKS:
Determine whether a polygon is backfacing given two fixed point vectors.  The
vectors need to be derived from two consecutive counterclockwise edges of the
polygon in order for this function to return accurate results.

Note that this function is written to correctly calculate the results for screen space
coordinates, which can cause overflow with a normal 16.16 fixed point multiply if
this is calculated directly using calls to MGL_FixMul.

****************************************************************************/
int	ASMAPI MGL_backfacing(
	fix32_t dx1,
	fix32_t dy1,
	fix32_t dx2,
	fix32_t dy2)
{
	// Lose a bit of precision so screen space coordinates will fit within
	// a 32-bit multiply. On 64-bit machines this will be much simpler.
	return ((dx1>>3) * (dy2>>3)) - ((dx2>>3) * (dy1>>3)) < 0;
}

/* {secret} */
zfix32_t _ASMAPI MGL_ZFixDiv(zfix32_t f,fix32_t g)
{ return MGL_FIXTOZ(MGL_FixDiv(MGL_ZTOFIX(f),g)); }

#endif	/* !__WATCOMC__ */

#endif	/* !__64BIT__ */

#endif	/* MGL_FIX3D */

#endif	/* !NO_ASSEMBLER */

/* {secret} */
void _ASMAPI _MGL_preRenderScanLine24(int x0,int minx,int maxx)
/****************************************************************************
*
* Function:		preRenderScanLine24
* Parameters:	x0		- Reference or base X coordinate
*               minx	- Minimum X coordinate in polygon
*				maxx	- Maximum X coordinate in polygon
*
* Description:	Pre-renders the linearly interpolated z offset values
*				into the z offset buffer. We only render z values between
*				minx and maxx inclusive.
*
****************************************************************************/
{
	zfix32_t	z1,z24,dZdX = DC.tr.dZdX;
	uchar		*z = (uchar*)DC.zOffset;

	/* Compute the initial offset of minx from x0 */
	z1 = MGL_FixMul(dZdX,MGL_TOFIX(minx - x0));

	/* Render the z buffer offsets into the buffer */
	z += minx;
	while (minx < maxx) {
		z24 = MGL_TOZBUF24(z1);
		*z++ = (uchar)z24;
		*z++ = (uchar)(z24 >> 8);
		*z++ = (uchar)(z24 >> 16);
		z1 += dZdX;
		minx++;
		}
}

#endif	/* !MGL_LITE */
