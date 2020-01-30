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
* Description:  Member functions for the MGL3ShadeModel class.
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

MGL3ShadeModel::MGL3ShadeModel(const MGLDevCtx& dc,const FXColor& color1,
	const FXColor& color2,const FXColor& color3,real _gamma,int maxColor)
	: MGLCMappedModel(dc)
/****************************************************************************
*
* Function:		MGL3ShadeModel::MGL3ShadeModel
* Parameters:	dc		- Device context associated with
*				color1	- Value for first color channel (Red channel)
*				color2	- Value for second color channel (Green channel)
*				color3	- Value for third color channel (Blue channel)
*				gamma	- Gamma correction factor to use
*
* Description:	Constructor for the MGL3ShadeModel class. Here we program
*				the top three quarters of the palette for looking up shades
*				of the three specified colors. The values in the color
*				palette are gamma corrected with the specified gamma
*				correction value.
*
****************************************************************************/
{
	if (dc.maxColor() != maxColor)
		MGL_fatalError("Invalid color depth for class MGL3ShadeModel!");

	// Compute the size of the color palette for each channel, and
	// the range of each color channel
	int range = (maxColor+1) / 4;

	// In 256 color models, subtract 10 off the starting index so that we
	// can avoid the last 10 system palette entries used by Windows.
	if (maxColor == 255)
		index1 = range-10;
	else index1 = range;
	index2 = index1 + range;
	index3 = index2 + range;

	// Compute the values for the three color channels.
	real gamma = FXoneOver(_gamma);

	for (int i = 0; i < range; i++) {
		real intens = FXdiv(FXintToReal(i+1),FXintToReal(range+1));

		setPaletteEntry(index1+i,
			convertReal(convertGamma(color1.r,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color1.g,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color1.b,intens,gamma) + REAL(0.5)));
		setPaletteEntry(index2+i,
			convertReal(convertGamma(color2.r,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color2.g,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color2.b,intens,gamma) + REAL(0.5)));
		setPaletteEntry(index3+i,
			convertReal(convertGamma(color3.r,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color3.g,intens,gamma) + REAL(0.5)),
			convertReal(convertGamma(color3.b,intens,gamma) + REAL(0.5)));
		}
}

color_t MGL3Shade4BitModel::convertColor(const FXColor& color)
/****************************************************************************
*
* Function:		MGL3Shade4BitModel::convertColor
* Parameters:	color	- Color value to convert
* Returns:		Converted color value (color map index)
*
* Description:	Converts the color value into a color map index, by looking
*				up the appropriate color channel index. Remember that only
*				one color channel can be active at a time.
*
****************************************************************************/
{
	if (color.r != 0)
		return index1 + (convert(color.r) >> 6);
	if (color.g != 0)
		return index2 + (convert(color.g) >> 6);
	return index3 + (convert(color.b) >> 6);
}

color_t MGL3Shade8BitModel::convertColor(const FXColor& color)
/****************************************************************************
*
* Function:		MGL3Shade8BitModel::convertColor
* Parameters:	color	- Color value to convert
* Returns:		Converted color value (color map index)
*
* Description:	Converts the color value into a color map index, by looking
*				up the appropriate color channel index. Remember that only
*				one color channel can be active at a time.
*
****************************************************************************/
{
	if (color.r != 0)
		return index1 + (convert(color.r) >> 2);
	if (color.g != 0)
		return index2 + (convert(color.g) >> 2);
	return index3 + (convert(color.b) >> 2);
}
