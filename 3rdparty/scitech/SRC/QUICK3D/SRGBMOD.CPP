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
* Description:  Member functions for the MGLCMappedRGBModel class.
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

static real convertGamma(int v,int vmax,real gamma)
{
	return FXmul(FXpow(FXdiv(FXintToReal(v),FXintToReal(vmax)),gamma),
		REAL(PALMAX));
}

static uint convertReal(real value)
{
	return (value > REAL(PALMAX) ? PALMAX : FXrealToInt(value));
}

MGLCMappedRGBModel::MGLCMappedRGBModel(const MGLDevCtx& dc,real _gamma,int maxColor)
	: MGLCMappedModel(dc)
/****************************************************************************
*
* Function:		MGLCMappedRGBModel::MGLCMappedRGBModel
* Parameters:	dc		- Device context associate with
*				gamma	- Gamma correction factor to use
*
* Description:	Constructor for the Simple RGB color mode for color mapped
*				devices. This uses the top half of the palette to create
*				a 1:1:1 model in 16 color modes and a 2:3:2 model in
*				256 color modes. Works, but is not very rewarding :-)
*
****************************************************************************/
{
	if (dc.maxColor() != maxColor)
		MGL_fatalError("Invalid color depth for class MGLCMappedRGBModel!");

	// Compute the size and range of the color palette
	int range = index = (maxColor+1) / 2;

	// In 256 color models, subtract 10 off the starting index so that we
	// can avoid the last 10 system palette entries used by Windows.
	if (maxColor == 255)
    	index -= 10;

	// Compute the values for the color channel.
	int red,green,blue,redmax,greenmax,bluemax;
	int redshift,greenshift;
	real gamma = FXoneOver(_gamma);

	if (dc.maxColor() == 255) {
		redshift = 5;			// 2 bits for red
		redmax = 3;
		greenshift = 2;			// 3 bits for green
		greenmax = 7;
		bluemax = 3;			// 2 bits for blue
		}
	else {
		redshift = 2;			// 1 bit for red
		redmax = 1;
		greenshift = 1;			// 1 bit for green
		greenmax = 1;
		bluemax = 1;			// 1 bit for blue
		}

	for (int i = 0; i < range; i++) {
		red = i >> redshift;
		green = (i >> greenshift) & greenmax;
		blue = i & bluemax;
		setPaletteEntry(index+i,
			convertReal(convertGamma(red,redmax,gamma) + REAL(0.5)),
			convertReal(convertGamma(green,greenmax,gamma)+ REAL(0.5)),
			convertReal(convertGamma(blue,bluemax,gamma) + REAL(0.5)));
		}
}

color_t MGLRGB4BitModel::convertColor(const FXColor& c)
/****************************************************************************
*
* Function:		MGLRGB4BitModel::convertColor
* Parameters:	c	- Color value to convert
* Returns:		Converted color value (color map index)
*
* Description:	Converts the color value into a color map index, by looking
*				up the appropriate color channel index. The value in the
*				red color channel is used to do this.
*
****************************************************************************/
{
	return index + ((convert(c.r) >> 7) << 2)
				 + ((convert(c.g) >> 7) << 1)
				 +  (convert(c.b) >> 7);
}

color_t MGLRGB8BitModel::convertColor(const FXColor& c)
/****************************************************************************
*
* Function:		MGLRGB8BitModel::convertColor
* Parameters:	c	- Color value to convert
* Returns:		Converted color value (color map index)
*
* Description:	Converts the color value into a color map index, by looking
*				up the appropriate color channel index. The value in the
*				red color channel is used to do this.
*
****************************************************************************/
{
	return index + ((convert(c.r) >> 6) << 5)
				 + ((convert(c.g) >> 5) << 2)
				 +  (convert(c.b) >> 6);
}

