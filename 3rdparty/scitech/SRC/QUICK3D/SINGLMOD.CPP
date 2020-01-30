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
* Description:  Member functions for the MGL1ShadeModel class.
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

static real convertGamma(real v,real intens,real gamma)
{
	return FXmul(FXpow(FXmul(v,intens),gamma),REAL(PALMAX));
}

static uint convertReal(real value)
{
	return (value > REAL(PALMAX) ? PALMAX : FXrealToInt(value));
}

MGL1ShadeModel::MGL1ShadeModel(const MGLDevCtx& dc,const FXColor& color,
	real _gamma,int maxColor)
	: MGLCMappedModel(dc)
/****************************************************************************
*
* Function:		MGL1ShadeModel::MGL1ShadeModel
* Parameters:	dc		- Device context associate with
* 				color	- Value for color channel (Red channel)
*				gamma	- Gamma correction factor to use
*
* Description:	Constructor for the MGL1ShadeModel class. Here we program
*				the top half of the palette for looking up shades of the
*				specified color. The values in the color palette are gamma
*				corrected with the specified gamma correction value.
*
****************************************************************************/
{
	if (dc.maxColor() != maxColor)
		MGL_fatalError("Invalid color depth for class MGL1ShadeModel!");

	// Compute the size and range of the color palette
	int range = index = (maxColor+1) / 2;
	real gamma = FXoneOver(_gamma);

	// In 256 color models, subtract 10 off the starting index so that we
	// can avoid the last 10 system palette entries used by Windows.
	if (maxColor == 255)
    	index -= 10;

	for (int i = 0; i < range; i++) {
		real intens = FXdiv(FXintToReal(i+1),FXintToReal(range+1));

		setPaletteEntry(index+i,
			convertReal(convertGamma(color.r,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color.g,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color.b,intens,gamma) + REAL(0.5)));
		}
}

color_t MGL1Shade4BitModel::convertColor(const FXColor& c)
/****************************************************************************
*
* Function:		MGL1Shade4BitModel::convertColor
* Parameters:	c	- Color value to convert
* Returns:		Converted color value (color map index)
*
* Description:	Converts the color value into a color map index, by looking
*				up the appropriate color channel index. The value in the
*				red color channel is used to do this.
*
****************************************************************************/
{
	return index + (convert(c.r) >> 5);
}

color_t MGL1Shade8BitModel::convertColor(const FXColor& c)
/****************************************************************************
*
* Function:		MGL1Shade8BitModel::convertColor
* Parameters:	c	- Color value to convert
* Returns:		Converted color value (color map index)
*
* Description:	Converts the color value into a color map index, by looking
*				up the appropriate color channel index. The value in the
*				red color channel is used to do this.
*
****************************************************************************/
{
	return index + (convert(c.r) >> 1);
}
