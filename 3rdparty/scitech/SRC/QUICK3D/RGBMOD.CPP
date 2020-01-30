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
* Description:  Member functions for the ColorModel base class.
*
*
****************************************************************************/

#include "quick3di.hpp"

/*---------------------------- Implementation -----------------------------*/

inline uint convert(real component)
{
	return (component > REAL(1) ? 255
		: FXrealToInt(FXmul(REAL(255),component) + REAL(0.5)));
}

inline uint convertGamma(real component,real correct)
{
	return (component > REAL(1) ? 255
		: FXmul(REAL(255),FXpow(component,correct)) + REAL(0.5));
}

MGLRGBModel::MGLRGBModel(const MGLDevCtx& dc)
	: MGLColorModel(dc)
/****************************************************************************
*
* Function:		MGLRGBModel::MGLRGBModel
* Parameters:	dc		- Device context associated with
*				gamma	- Gamma correction factor to use
*
* Description:	Pre-calculates the conversion factor for converting the
*				color values with gamma correction.
*
****************************************************************************/
{
	if (dc.getBitsPerPixel() <= 8)
		MGL_fatalError("Cannot use class RGBModel in color mapped modes!");
}

color_t MGLRGBModel::convertColor(const FXColor& c)
/****************************************************************************
*
* Function:		MGLRGBModel::convertColor
* Parameters:	c	- Color value to convert
* Returns:		Converted color value (appropriate display mode RGB tuple)
*
* Description:  Converts the color value into the format required by the
*				underlying video mode.
*
****************************************************************************/
{
	return dc.packColor(convert(c.r),convert(c.g),convert(c.b));
}

MGLRGBGammaModel::MGLRGBGammaModel(const MGLDevCtx& dc,real gamma)
	: MGLColorModel(dc)
/****************************************************************************
*
* Function:		MGLRGBGammaModel::MGLRGBGammaModel
* Parameters:	dc		- Device context associated with
*				gamma	- Gamma correction factor to use
*
* Description:	Pre-calculates the conversion factor for converting the
*				color values with gamma correction.
*
****************************************************************************/
{
	if (dc.getBitsPerPixel() <= 8)
		MGL_fatalError("Cannot use class RGBGammaModel in color mapped modes!");
	correct = FXoneOver(gamma);
}

color_t MGLRGBGammaModel::convertColor(const FXColor& c)
/****************************************************************************
*
* Function:		MGLRGBGammaModel::convertColor
* Parameters:	c	- Color value to convert
* Returns:		Converted color value (appropriate display mode RGB tuple)
*
* Description:  Converts the color value into the format required by the
*				underlying video mode. Each color value is gamma corrected
*				before being converted.
*
****************************************************************************/
{
	return dc.packColor(convertGamma(c.r,correct),
						convertGamma(c.g,correct),
						convertGamma(c.b,correct));
}
